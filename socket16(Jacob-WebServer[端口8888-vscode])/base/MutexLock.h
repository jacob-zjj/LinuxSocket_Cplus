#pragma once
#include "noncopyable.h"
#include <pthread.h>
#include <cstdio>
//用Mutex来专门定义 程序中用到的锁
class MutexLock : noncopyable
{
public:
    MutexLock()
    {
        pthread_mutex_init(&mutex, NULL);//新建锁
    }
    ~MutexLock()
    {
        pthread_mutex_lock(&mutex);//先获得锁
        pthread_mutex_destroy(&mutex);//销毁锁
    }
    void lock()
    {
        pthread_mutex_lock(&mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
    //得到这把锁
    pthread_mutex_t *get()
    {
        return &mutex;
    }
    
private:
    pthread_mutex_t mutex;

//友元类不受访问权限影响
private:
    friend class Condition;
};
class MutexLockGuard : noncopyable
{
public:
    //避免隐式类型转换
    explicit MutexLockGuard(MutexLock &_mutex) : mutex(_mutex)
    {
        mutex.lock();
    }
    ~MutexLockGuard()
    {
        mutex.unlock();
    }
private:
    MutexLock &mutex;
};
