#pragma once
#include <stdint.h>
namespace CurrentThread
{
    //如果函数的声明中带有关键字extern 仅仅是暗示这个变量会在外部实现
    //__thread变量每一个线程有一份独立实体，各个线程的值互不干扰
    extern __thread int t_cachedTid;
	extern __thread char t_tidString[32];
	extern __thread int t_tidStringLength;
	extern __thread const char* t_threadName;
    void cacheTid();//该函数会在Thread.cpp中实现
    //在c/c++中，为了解决一些频繁调用的小函数大量消耗栈空间（栈内存）的问题，特别的引入了inline修饰符，表示为内联函数。
    //内联函数能够减少时常调用的函数对栈空间大量消耗
    inline int tid()
    {
        /*
        让cpu可以预先取出下一条指令，减少cpu等待取指令的耗时，从而可以提供cpu的效率。
        减少跳的次数 减少指令跳转带来的性能上的下降, 达到优化程序的目的
			long __builtin_expect(long exp, long c);
        exp == c 表示值为c的概率很大 这样避免跳转
        */
        if(__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
    inline const char* tidString()
    {
        return t_tidString;
    }
    inline int tidStringLength()
    {
        return t_tidStringLength;
    }
    inline const char* name()
    {
        return t_threadName;
    }
}