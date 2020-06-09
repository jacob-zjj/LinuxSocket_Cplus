//LogStream主要用来格式化输出，重载<<运算符，同时也有自己的一块缓冲区 
//这里缓冲区的存在是为了缓存一行，把多个<<的结果连成一块
//日志流头文件
#pragma once
#include "noncopyable.h"
#include <assert.h>
#include <string.h>
#include <string>

//异步日志
class AsyncLogging;
//使用两种缓冲区来存储日志内容再写入到文件中
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;
//继承非拷贝类 因此该类无法进行拷贝 并且也无法进行赋值
template<int SIZE>
//固定缓冲区
class FixedBuffer : noncopyable
{
public:
    FixedBuffer() : cur_(data_){}
    ~FixedBuffer() {}
    void append(const char* buf, size_t len)
    {
        //判断data缓冲区中的剩余空间大小是否 大于要加入buf的字符串长度
        if(avail() > static_cast<int>(len))
        {
            memcpy(cur_, buf, len);
            cur_ += len;//更新一下当前buffer缓冲区中的cur_指针
        }
    }
    const char* data() const{ return data_;}//返回data缓冲区中字符
    int length() const { return static_cast<int>(cur_ - data_);}//表示整个data_中的实际长度
    char* current() { return cur_;}//返回data_中现在cur_指向的位置
    //表示data_缓冲区中可用的缓冲区长度
    int avail() const { return static_cast<int>(end() - cur_);}
    void add(size_t len) { cur_ += len; }//这里表示加入指定长度 其实也就是直接将cur_后移
    void reset() { cur_ = data_; }//重新将cur_指向data_的首位
    void bzero() { memset(data_, 0, sizeof(data_)); }//清空当前缓冲区的所有内容 用memset来实现
    
private:
    //返回data_中指向末尾的指针
    const char* end() const{ return data_ + sizeof(data_);}
    char data_[SIZE];
    char* cur_;//在函数进行构造的时候就将cur_ 指向 data_字符数组
};

class LogStream : noncopyable
{
public:
    typedef FixedBuffer<kSmallBuffer> Buffer;//定义指定大小的Buffer 4000个长度
    LogStream& operator<<(bool v)
    {
        //如果v为true 向buffer_中的data_缓冲区中加1 false 加入0
        buffer_.append(v ? "1" : "0", 1);
        return *this;//这是重载操作符的标准写法
    }
    LogStream& operator<<(short);
	LogStream& operator<<(unsigned short);
	LogStream& operator<<(int);
	LogStream& operator<<(unsigned int);
	LogStream& operator<<(long);
	LogStream& operator<<(unsigned long);
	LogStream& operator<<(long long);
	LogStream& operator<<(unsigned long long);
    LogStream& operator<<(const void*);
    LogStream& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    LogStream& operator<<(double);
	LogStream& operator<<(long double);
    LogStream& operator<<(char v)
    {
        buffer_.append(&v, 1);//向buffer_中的data_缓冲区中加入v
        return *this;
    }
    LogStream& operator<<(const char* str)
    {
        if(str)
        {
            buffer_.append(str, strlen(str));
        }
        else
        {
            buffer_.append("(null)", 6);
        }
        return *this;
    }
    LogStream& operator<<(const unsigned char* str)
    {
        //将const unsigned char* 类型转换为 const char*
        return operator<<(reinterpret_cast<const char*>(str));
    }
    LogStream& operator<<(const std::string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }
    //这里调用LogStream的append 相当于调用固定缓冲区的append
    void append(const char* data, int len)
    {
        buffer_.append(data, len);
    }
    //获取LogStream中的Buffer缓冲区
    const Buffer& buffer() const
    {
        //返回Buffer类型的buffer_
        return buffer_;
    }
    void resetBuffer()
    {
        //直接调用buffer_中的reset即可
        buffer_.reset();
    }
private:
    void staticCheck();
    //定义模板类型 T 也就是泛型
    template<typename T>
    void formatInteger(T);
    Buffer buffer_;
    static const int kMaxNumericSize = 32;
};