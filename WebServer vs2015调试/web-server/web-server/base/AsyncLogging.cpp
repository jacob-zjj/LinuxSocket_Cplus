#include "AsyncLogging.h"
//2019年10月13日15:56:06 写到此处 这些内容都是借鉴的陈硕的moduo服务器端编程书的内容
#include "LogFile.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <functional>

AsyncLogging::AsyncLogging(std::string logFileName_,int flushInterval)
  : flushInterval_(flushInterval),
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
	/*assert的作用是现计算表达式 expression ，如果其值为假（即为0），那么它先向stderr打印一条出错信息，
	然后通过调用 abort 来终止程序运行。*/
	assert(logFileName_.size() > 1);
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	//vector::reserve函数 是对vector内存的提前分配，不需要每次调用的时候才分配 而且在分配的过程中不产生对象 不会占过多的内存
	buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
	MutexLockGuard lock(mutex_);
	if (currentBuffer_->avail() > len)
	{
		currentBuffer_->append(logline, len);
	}
	else
	{
		//由于采用的是双缓冲机制 因此应该对两个缓冲区同时进行处理
		buffers_.push_back(currentBuffer_);/*说明currentBuffer已经存得差不多了 可以放入缓冲区容器中了*/
		currentBuffer_.reset();/*清空内存 这里不是使用 我们自己定义的reset函数*/
		if (nextBuffer_)
		{
			//使用该函数可以 将nextBuffer_中的内容清空 然后给currentBuffer_
			currentBuffer_ = std::move(nextBuffer_);
		}
		else
		{
			currentBuffer_.reset(new Buffer);//新建一个缓冲区对象传给currentBuffer_ 几乎很少发生
		}
		currentBuffer_->append(logline, len);
		cond_.notify();
	}
}

//线程启动时的回调函数  该函数是非常重要的部分 
void AsyncLogging::threadFunc()
{
	assert(running_ == true);
	latch_.countDown();
	LogFile output(basename_);
	//创建两个缓冲区
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer2->bzero();
	BufferVector buffersToWrite;
	buffersToWrite.reserve(16);
	while (running_)
	{
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
			buffers_.push_back(currentBuffer_);
			currentBuffer_.reset();

			currentBuffer_ = std::move(newBuffer1);
			//将两个缓冲区容器进行交换
			buffersToWrite.swap(buffers_);
			if (!nextBuffer_)
			{
				nextBuffer_ = std::move(newBuffer2);
			}
		}
		assert(!buffersToWrite.empty());

		//当缓冲区容器中的缓冲区大于25 清除 从第三个开始到最后的内容
		if (buffersToWrite.size() > 25)
		{
			/*char buf[256];
			snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
				Timestamp::now().toFormattedString().c_str(),
				buffersToWrite.size() - 2);
			fputs(buf, stderr);
			output.append(buf, static_cast<int>(strlen(buf)));*/
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