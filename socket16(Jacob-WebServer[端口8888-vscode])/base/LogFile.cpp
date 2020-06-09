#include "LogFile.h"
#include "FileUtil.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace std;

LogFile::LogFile(const std::string& basename, int flushEveryN)
:   basename_(basename),
    flushEveryN_(flushEveryN),
    count_(0),
    mutex_(new MutexLock)
{
    file_.reset(new AppendFile(basename));   
}

LogFile::~LogFile(){}

void LogFile::append(const char* logline, int len)
{
    MutexLockGuard lock(*mutex_);
    append_unlocked(logline, len);
}

void LogFile::flush()
{
    //这里直接调用fileUtil中的flush方法 直接将缓存清空即可
    MutexLockGuard lock(*mutex_);
    file_->flush();
}

void LogFile::append_unlocked(const char* logline, int len)
{
    //该函数 其实是对append函数的进一步封装 主要是为了控制append 的次数，每1024次就刷新一次缓冲区
	//加入 日志文件 当加入次数达到 1024 次 刷新用户定义的文件缓冲区
    file_->append(logline, len);
    ++count_;
    if(count_ >= flushEveryN_)
    {
        count_ = 0;
        file_->flush();
    }
}