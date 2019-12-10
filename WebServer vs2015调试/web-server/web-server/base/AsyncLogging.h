//AsyncLogging负责(定时到或被填满时)将缓冲区中的数据写入LogFile中
#pragma once
#include "CountDownLatch.h"
#include "MutexLock.h"
#include "Thread.h"
#include "LogStream.h"
#include "noncopyable.h"
#include <functional>
#include <string>
#include <vector>

//2019年10月12日14:46:03 写到此 缓冲日志的所有代码均参考陈硕的moduo服务器编程一书中代码部分
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
		running_ = true;
		thread_.start();
		latch_.wait();
	}
	void stop()
	{
		running_ = false;
		cond_.notify();
		thread_.join();
	}

private:
	void threadFunc();
	//FixedBuffer 是对于缓冲区的是作用
	typedef FixedBuffer<kLargeBuffer> Buffer;
	/*前面的版本我们开始引入这个智能指针这个概念 目的是为了对对象生命周期的管理 我们知道智能指针的使用使得我们在
	利用对象指针的时候不需要重复的去创造对象指针 我们只需要将其计数 + 1即可*/
	typedef std::vector<std::shared_ptr<Buffer>> BufferVector;//智能指针类型的vector容器  缓冲区容器
	typedef std::shared_ptr<Buffer> BufferPtr;//缓冲区指针
	const int flushInterval_;
	bool running_;
	std::string basename_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
	//因为使用双缓冲机制 其实是四个缓冲 在线程的回调函数中还会建立两个缓冲区
	//所以存在以下的 现在缓冲区 和 接下来的缓冲区  大小为 4000x1000
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	//缓冲区容器 内容包含包含双缓冲区
	BufferVector buffers_;
	//统计线程启动数量 当线程启动一个countDown()函数对latch_值进行修改 -1
	CountDownLatch latch_;
};