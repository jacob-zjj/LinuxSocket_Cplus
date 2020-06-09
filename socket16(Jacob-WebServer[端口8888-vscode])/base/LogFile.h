#pragma once
#include "FileUtil.h"
#include "MutexLock.h"
//logFIle进一步封装了FileUtil，并设置了一个循环次数，每当过了1024次就刷新一次
#include "noncopyable.h"
#include <memory>
#include <string>
//TODO 提供自动归档功能
class LogFile : noncopyable
{
public:
    //每被append flushEveryN次 flush一下 会往文件中写
    //只不过文件是带缓冲区的
    LogFile(const std::string& basename, int flushEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    //这部分代码没有实现
    void rollFile();

private:
    void append_unlocked(const char* logline, int len);

    const std::string basename_;
    const int flushEveryN_;

    int count_;
    //使用两个智能指针
	std::unique_ptr<MutexLock> mutex_;
	std::unique_ptr<AppendFile> file_;
};