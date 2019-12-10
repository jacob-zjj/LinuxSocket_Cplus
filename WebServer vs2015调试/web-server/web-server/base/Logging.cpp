#include "Logging.h"
#include "CurrentThread.h"
#include "Thread.h"
#include "AsyncLogging.h"
#include <assert.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_;

std::string Logger::logFileName_ = "./log/jacob_WebServer.log";

void once_init()
{
	//2019.10.11晚 实现AsyncLogging.h 和 .cpp
	AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
	AsyncLogger_->start();
}

void output(const char* msg, int len)
{
	/*在多线程环境中，有些事仅需要执行一次。通常当初始化应用程序时，可以比较容易地将其放在main函数中。
	但当你写一个库时，就不能在main里面初始化了，你可以用静态初始化，但使用一次初始化（pthread_once）会比较容易些。
	功能：本函数使用初值为PTHREAD_ONCE_INIT的once_control变量保证init_routine()函数在本进程执行序列中仅执行一次。*/

	//设置线程只初始化一次 回调函数 int pthread_once(pthread_once_t *once_control, void (*init_routine) (void))；
	pthread_once(&once_control_, once_init);
	AsyncLogger_->append(msg, len);
}

//Logger类中的类Impl中的方法
Logger::Imp1::Imp1(const char *fileName, int line) 
:	stream_(), 
	line_(line), 
	basename_(fileName)
{
	formatTime();
}

//定义输出的日志的时间形式
void Logger::Imp1::formatTime()
{
	struct timeval tv;
	time_t time;
	char str_t[26] = { 0 };
	gettimeofday(&tv, NULL);
	time = tv.tv_sec;
	struct tm* p_time = localtime(&time);//获取本地时间
	strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
	stream_ << str_t;//LogFile已经重载了 << 将时间信息整理成一行作为日志输入
}

//2019年10月20日16:59:20
Logger::Logger(const char *fileName, int line) : imp1_(fileName, line)
{}

Logger::~Logger()
{
	imp1_.stream_ << "--" << imp1_.basename_ << ":" << imp1_.line_ << '\n';
	const LogStream::Buffer& buf(stream().buffer());
	//通过启动线程 来写日志文件
	output(buf.data(), buf.length());
}

