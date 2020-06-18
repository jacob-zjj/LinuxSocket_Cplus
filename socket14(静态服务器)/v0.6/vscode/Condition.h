#pragma once
#include "nocopyable.h"
#include "MutexLock.h"
#include <pthread.h>
#include <errno.h>
#include <cstdint>
#include <time.h>

class Condition : noncopyable
{
public:
	explicit Condition(MutexLock &_mutex) : mutex(_mutex)
	{
		pthread_cond_init(&cond, NULL);
	}
	~Condition()
	{
		pthread_cond_destroy(&cond);
	}
	void wait()
	{
	/*int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
		函数作用：
		1.	阻塞等待条件变量cond（参1）满足
		2.	释放已掌握的互斥锁（解锁互斥量）相当于pthread_mutex_unlock(&mutex);
		 1.2.两步为一个原子操作。
		3.	当被唤醒，pthread_cond_wait函数返回时，解除阻塞并重新申请获取互斥锁pthread_mutex_lock(&mutex);
	*/
		pthread_cond_wait(&cond, mutex.get());
	}
	void notify()
	{
		//利用条件变量的方式进行唤醒 随机唤醒至少一个
		pthread_cond_signal(&cond);
	}
	void notifyAll()
	{
		//利用广播的方式就行唤醒 全部唤醒阻塞在条件变量中的线程
		pthread_cond_broadcast(&cond);
	}
	bool waitForSeconds(int seconds)
	{
		struct timespec abstime;
		//clock_gettime( ) 提供了纳秒级的精确度  
		/*
		int clock_gettime(clockid_t clk_id, struct timespec *tp);参数说明：
		
		clockid_t clk_id 用于指定计时时钟的类型，有以下4种：
		CLOCK_REALTIME:系统实时时间,随系统实时时间改变而改变,即从UTC1970-1-1 0:0:0开始计时,
		中间时刻如果系统时间被用户该成其他,则对应的时间相应改变
		CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
		CLOCK_PROCESS_CPUTIME_ID:本进程到当前代码系统CPU花费的时间
		CLOCK_THREAD_CPUTIME_ID:本线程到当前代码系统CPU花费的时间
		struct timespec *tp用来存储当前的时间，其结构如下：
		struct timespec
		{
			time_t tv_sec; //seconds
			long tv_nsec; //nanoseconds
		};
		返回值。0成功， - 1失败
		*/
		//首先应该获取当前时间 然后将当前时间加上等待时间 一起传给pthread_cond_timedwait
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec += static_cast<time_t>(seconds);
		//限时等待条件变量
		/*int pthread_cond_timedwait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, const struct timespec *restrict abstime);*/
		return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
	}

private:
	MutexLock &mutex;
	pthread_cond_t cond;
};