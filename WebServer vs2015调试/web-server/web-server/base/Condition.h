//设置条件变量 类的实现
#pragma once
#include "noncopyable.h"
#include "MutexLock.h"
#include <pthread.h>
#include <errno.h>
#include <cstdint>
#include <time.h>
/*在C++中，我们有时可以将构造函数用作自动类型转换函数。但这种自动特性并非总是合乎要求的，
有时会导致意外的类型转换，因此，C++新增了关键字explicit，用于关闭这种自动特性。
即被explicit关键字修饰的类构造函数，不能进行自动地隐式类型转换，只能显式地进行类型转换。
注意：只有一个参数的构造函数，或者构造函数有n个参数，但有n-1个参数提供了默认值，这样的情况才能进行类型转换。
详情请参加博客https://www.cnblogs.com/gklovexixi/p/5622681.html
构造函数只有一个参数，可以进行类型转换，如：Demo test; test = 12.2;这样的调用就相当于把12.2隐式转换为Demo类型。
*/
class Condition : noncopyable
{
public:
	explicit Condition(MutexLock &_mutex) : mutex(_mutex)
	{
		pthread_cond_init(&cond, NULL);//初始化 锁的 条件变量
	}

	~Condition()
	{
		pthread_cond_destroy(&cond);
	}

	void wait()
	{
		pthread_cond_wait(&cond, mutex.get());
		//阻塞等待一个条件变量 条件满足时 释放已掌握的互斥锁 当被唤醒时，解除阻塞 重新申请互斥锁
	}

	void notify()
	{
		//唤醒至少一个阻塞在条件变量上的线程
		pthread_cond_signal(&cond);
	}

	void notifyAll()
	{
		pthread_cond_broadcast(&cond);//唤醒所有阻塞在条件变量上的线程
	}

	bool waitForSecond(int seconds)
	{
		/*
		struct timespec
		{
			time_t tv_sec;  // Seconds - >= 0 秒
			long   tv_nsec; // Nanoseconds - [0, 999999999] 纳秒
		};
		*/
		struct timespec abstime;
		clock_gettime(CLOCK_REALTIME, &abstime);//获取系统当前时间
		//CLOCK_REALTIME 系统当前时间
		abstime.tv_sec += static_cast<time_t>(seconds);
		return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
		//设置超时时间 如果定时时间到了就会返回true 没有到将会返回flase
	}
private:
	MutexLock &mutex;
	pthread_cond_t cond;
};