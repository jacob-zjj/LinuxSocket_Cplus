#include "Logging.h"
#include "CurrentThread.hpp"
#include "Thread.h"
#include <assert.h>

#include "AsyncLogging.h"

#include <iostream>
using namespace std;

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_;

void once_init()
{
	AsyncLogger_ = new AsyncLogging(std::string("/jacob_web_server.log"));
	AsyncLogger_->start();
}

void output(const char* msg, int len)
{
	pthread_once(&once_control_, once_init);
	AsyncLogger_->append(msg, len);
}

Logger::Impl::Impl(const char *fileName, int line):stream_(), basename_(fileName), line_(line)
{
}

void Logger::Impl::formatTime()
{
}

Logger::Logger(const char *fileName, int line) : impl_(fileName, line)
{

}

Logger::~Logger()
{
	impl_.stream_ << " - " << impl_.basename_ << ":" << impl_.line_ << '\n';
	const LogStream::Buffer& buf(stream().buffer());
	output(buf.data(), buf.length());
}


