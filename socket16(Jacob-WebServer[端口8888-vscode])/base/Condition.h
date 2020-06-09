//这个类的实现其实是锁唤醒的条件 封装成了类
#pragma once
#include "noncopyable.h"
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
        //初始化锁的条件变量
        pthread_cond_init(&cond, NULL);
    }
    ~Condition()
    {
        pthread_cond_destroy(&cond);
    }
    void wait()
    {
        //阻塞等待一个条件变量 条件满足时 释放已掌握的互斥锁 当被唤醒时，解除阻塞 重新申请互斥锁
        pthread_cond_wait(&cond, mutex.get());
    }
    void notify()
    {
        //唤醒中至少一个阻塞在条件变量上的线程
        pthread_cond_signal(&cond);
    }
    void notifyAll()
    {
        //唤醒所有阻塞在条件变量上的线程
        pthread_cond_broadcast(&cond);
    }
    //设置一定时间的阻塞
    bool waitForSecond(int seconds)
    {
        /* 使用结构体 类型
		struct timespec
		{
			time_t tv_sec;  // Seconds - >= 0 秒
			long   tv_nsec; // Nanoseconds - [0, 999999999] 纳秒
		};
		*/
        struct timespec abstime;
        //CLOCK_REALTIME 表示获取当前系统时间
        clock_gettime(CLOCK_REALTIME, &abstime);//获取系统当前时间
        abstime.tv_sec += static_cast<time_t>(seconds);
        //设置超时时间 如果定时时间到了就会返回true 没有到将会返回flase
        return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
    }

private:
    MutexLock& mutex;
    pthread_cond_t cond;
};