//FileUtil是一个底层文件 封装了Log文件的打卡和写入
//并且在类析构的时候关闭文件 底层使用了标准IO
//该append函数直接向文件写入 但是使用write函数直接进行写操作
#pragma once
#include "noncopyable.h"
#include <string>
class AppendFile:noncopyable
{
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();
    //append 会进行写文件操作
    void append(const char* logline, const size_t len);
    void flush();
private:
    size_t write(const char* logline, size_t len);
    FILE* fp_;
    char buffer_[64 * 1024];
};