//CountDownLatch的主要作用是确保Thread中传出去的func真的启动了以后 
//外层的start才返回
#pragma once
#include "Condition.h"
#include "MutexLock.h"
#include "noncopyable.h"

class CountDownLatch : noncopyable
{
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();

private:
    mutable MutexLock mutex_;/*使得被mutable所修饰的变量 即使是在const修饰的函数中也可以进行访问*/
    Condition condition_;
    int count_;
};