//AsyncLogging负责(定时到或被填满时)将缓冲区中的数据写入LogFile中
//用一个背景线程专门用来收集日志 并写入日志文件 其他线程只是负责向这个线程发送日志消息即可
#pragma once
#include "CountDownLatch.h"
#include "MutexLock.h"
#include "Thread.h"
#include "LogStream.h"
#include "noncopyable.h"
#include <functional>
#include <string>
#include <vector>
class AsyncLogging : noncopyable
{
public:
    //构造函数在传入basename 即为 filename时 刷新间歇设置为2
    AsyncLogging(const std::string basename, int flushInterval = 2);
    ~AsyncLogging()
	{
		if (running_)
		{
			stop();
		}
	}
    void append(const char* logline, int len);
    void start()
	{
		running_ = true;//说明在运行
		thread_.start();//启动线程
		latch_.wait();//阻塞等待在条件上
	}
    void stop()
    {
        running_ = false;
        cond_.notify();//这里只需要唤醒一个在阻塞条件上等待的线程
        thread_.join();//使用join会在 当前线程结束之后再 结束析构结束掉当前线程 回收
    }

private:
    void threadFunc();
    //FixedBuffer 其实就是缓冲区 因为在对缓冲区进行定义时使用了模板类 
    typedef FixedBuffer<kLargeBuffer> Buffer;
    //使用智能指针对vector中的每一个对象进行管理
    typedef std::vector<std::shared_ptr<Buffer>> BufferVector;//智能指针类型的vector容器  缓冲区容器
    typedef std::shared_ptr<Buffer> BufferPtr;//缓冲区指针
    const int flushInterval_;
    bool running_;
    std::string basename_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    //因为使用双缓冲机制  其实是四个缓冲 在线程的回调函数中还会建立两个缓冲区
    //所以存在以下的 现在缓冲区 和 接下来的缓冲区  大小为 4000x1000
    BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
    //缓冲区容器 内容包含双缓冲区
    BufferVector buffers_;
    //统计线程启动数量 当线程启动一个countDown()函数对latch_值进行修改 -1
	CountDownLatch latch_;
};