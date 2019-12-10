/*
在Logger类中有2个内部类SourceFile和Impl。
类SouceFile用来确定日志文件的名字，而Impl是真正实现日志输出的地方。
在Logger类中可以设置Logger日志级别，以及设置缓存和清空缓存函数。
*/
#pragma once
#include "LogStream.h"
#include <pthread.h>
#include <string.h>
#include <string>
#include <stdio.h>
class AsyncLogging;
class Logger
{
public:
	~Logger();
	Logger(const char *fileName, int line);
	//LogStream重载了 << 运算符，因此可以直接使用
	LogStream& stream()
	{
		return impl_.stream_;
	}

private:
	class Impl
	{
	public:
		Impl(const char *fileName, int line);
		void formatTime();

		LogStream stream_;
		int line_;
		std::string basename_;
	};
	Impl impl_;
};
/*
LogStream.h定义了Buffer类和LogStream类。Buffer类是缓存，日志先缓存到Buffer中，之后再输出。
LogStream类包含buffer，主要实现各种类型数据到字符串的转换，并把数据添加到buffer中。
添加时使用append()函数，如果日志太长，将无法添加，但是每条日志输出都是构建了一个匿名类，不存在日志叠加到一个缓存的问题。
*/
#define LOG Logger(__FILE__, __LINE__).stream()