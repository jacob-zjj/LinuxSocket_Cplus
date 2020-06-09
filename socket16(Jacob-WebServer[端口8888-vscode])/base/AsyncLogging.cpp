#include "AsyncLogging.h"
#include "LogFile.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <functional>

AsyncLogging::AsyncLogging(std::string logFileName_, int flushInterval)
 :  flushInterval_(flushInterval),
    running_(false),
    basename_(logFileName_),
    thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
    mutex_(),
    cond_(mutex_),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer),
    buffers_(),
    latch_(1)
{
    //先判断logFileName的长度
    assert(logFileName_.size() > 1),
    //清空当前缓冲区 和 接下来的缓冲区
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    //vector::reserve函数 是对vector内存的提前分配，不需要每次调用的时候才分配 
    //而且在分配的过程中不产生对象 不会占过多的内存
	buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
    MutexLockGuard lock(mutex_);
    if(currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline, len);
    }
    //这里如果是多线程进行写缓冲 那么当avail()的长度小于等于len的时候 线程会阻塞在这里
    else
    {
        //由于采用的是双缓冲机制 如果当前缓冲区已经装满 那么需要把它放入到缓冲区容器中
        buffers_.push_back(currentBuffer_);
        currentBuffer_.reset();//使用自己定义的方法来清空内存 只需要把内存的指针指向缓冲区开始位置即可
        //对智能对象进行判断 其实就是看这个对象是否还存在 或者已经被销毁
        if(nextBuffer_)
        {
            //使用该函数可以将 nextBuffer_中的内容清空 然后给currentBuffer_
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            //新建一个缓冲区对象传给currentBuffer_ 几乎很少发生
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);
        cond_.notify();
    }
}
//启动线程的回调函数 这个函数非常关键
void AsyncLogging::threadFunc()
{
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_);
    //创建两个缓冲区
    BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
    //初始化清空缓冲区
    newBuffer1->bzero();
	newBuffer2->bzero();
    //装缓冲的容器
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while(running_)
    {
        //先进行判断
        assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());
        {
            MutexLockGuard lock(mutex_);
            if (buffers_.empty())//unusual usage!
			{
				//如果buffers_不为空 等两秒
				cond_.waitForSecond(flushInterval_);
			}
            //首先将当前缓冲区放入缓冲区集合中
            buffers_.push_back(currentBuffer_);
            currentBuffer_.reset();
            currentBuffer_ = std::move(newBuffer1);
            //将两个缓冲区容器进行交换 将生产者容器和消费者容器进行交换
            buffersToWrite.swap(buffers_);
            //如果nextBuffer为空
            if(!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }
        //如果buffersToWrite不为空 那么才能进行正常写
        assert(!buffersToWrite.empty());
        //当缓冲区容器中的缓冲区大于25 清除 从第三个开始到最后的内容
		if (buffersToWrite.size() > 25)
		{
			buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
		}   
        for (size_t i = 0; i < buffersToWrite.size(); ++i)
		{
			//向文件中写入内容
			output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
		}
        //因为已经将 buffersTowrite中的内容写入 因此直接重置为2即可
		if (buffersToWrite.size() > 2)
		{
			// drop non-bzero-ed buffers, avoid trashing
			buffersToWrite.resize(2);
		}
        //当newBuffer1为空时 需要将buffersToWrite中的最后一个缓冲区放入其中
		if (!newBuffer1)
		{
			assert(!buffersToWrite.empty());
			newBuffer1 = buffersToWrite.back();
			buffersToWrite.pop_back();
			newBuffer1->reset();
		}

		//当newBuffer2为空时 需要将buffersToWrite中的最后一个缓冲区放入其中
		if (!newBuffer2)
		{
			assert(!buffersToWrite.empty());
			newBuffer2 = buffersToWrite.back();
			buffersToWrite.pop_back();
			newBuffer2->reset();
		}
		buffersToWrite.clear();
		output.flush();
    }
    output.flush();
}