#pragma once

#include "CountDownLatch.h"
#include "MutexLock.h"
#include "Thread.h"
#include "LogStream.h"
#include "nocopyable.h"
#include <functional>
#include <string>
#include <vector>

class AsyncLogging : noncopyable
{
public:
	AsyncLogging(const std::string basename, int flushInterval = 2);
	~AsyncLogging()
	{
		if (runing_)
		{
			stop();
		}
	}
	void append(const char* logline, int len);
	void start()
	{
		runing_ = true;
		thread_.start();
		latch_.wait();
	}
	void stop()
	{
		runing_ = false;
		cond_.notify();
		thread_.join();
	}
private:
	AsyncLogging(const AsyncLogging&);
	void operator= (const AsyncLogging&);
	
	void threadFunc();

	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
	typedef std::shared_ptr<Buffer> BufferPtr;

	const int flushInterval_;
	bool runing_;
	std::string basename_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
	CountDownLatch latch_;
};
