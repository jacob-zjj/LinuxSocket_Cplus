//Logging是对外接口，Logging类内涵一个Logstream对象
//主要是为了每次打log的时候在log之前和之后加上固定的格式化的信息，比如打log的行 文件名等信息
#pragma once
#include "LogStream.h"
#include <pthread.h>
#include <string.h>
#include <string>
#include <stdio.h>

//异步日志
class AsyncLogging;

//日志
class Logger
{
public:
	Logger(const char *fileName, int line);
	~Logger();
	LogStream& stream() { return imp1_.stream_; }
	static void setLogFileName(std::string fileName)
	{
		logFileName_ = fileName;
	}
	static std::string getLogFileName()
	{
		return logFileName_;
	}

private:
	class Imp1
	{
	public:
		Imp1(const char *fileName, int line);
		void formatTime();

		LogStream stream_;
		int line_;
		std::string basename_;
	};
	Imp1 imp1_;
	static std::string logFileName_;
};
#define LOG Logger(__FILE__, __LINE__).stream()