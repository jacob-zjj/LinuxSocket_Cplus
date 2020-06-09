//CountDownLatch的主要作用是确保Thread中传出去的func真的启动了
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
    /*使得被mutable所修饰的变量 即使是在const修饰的函数中也可以进行访问*/
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};