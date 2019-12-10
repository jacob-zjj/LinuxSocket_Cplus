#pragma once
#include <stdint.h>

namespace CurrentThread
{
	//如果函数的声明中带有关键字extern，仅仅是暗示这个函数可能在别的源文件里定义，没有其它作用
	extern __thread int t_cachedTid;
	extern __thread char t_tidString[32];
	extern __thread int t_tidStringLength;
	extern __thread const char* t_threadName;
	void cacheTid();//该函数在thread.cpp中实现
	//在c/c++中，为了解决一些频繁调用的小函数大量消耗栈空间（栈内存）的问题，特别的引入了inline修饰符，表示为内联函数。
	inline int tid()
	{
		/*
		减少跳的次数 减少指令跳转带来的性能上的下降, 达到优化程序的目的
			long __builtin_expect(long exp, long c);

			你期望 exp 表达式的值等于常量 c, 看 c 的值, 如果 c 的值为0(即期望的函数返回值),
			那么 执行 if 分支的的可能性小, 否则执行 else 分支的可能性小(函数的返回值等于第一个参数 exp).

			#define likely(x)      __builtin_expect(!!(x), 1)
			#define unlikely(x)    __builtin_expect(!!(x), 0)
		*/
		if (__builtin_expect(t_cachedTid == 0, 0))
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