//LogStream主要用来格式化输出，重载<<运算符，同时也有自己的一块缓冲区 
//这里缓冲区的存在是为了缓存一行，把多个<<的结果连成一块
#pragma once
#include "noncopyable.h"
#include <assert.h>
#include <string.h>
#include <string>

//异步日志
class AsyncLogging;
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

//继承非拷贝类 因此该类无法进行拷贝 并且也无法进行赋值构造
template<int SIZE>
class FixedBuffer : noncopyable
{
public:
	FixedBuffer() : cur_(data_){}

	~FixedBuffer() {}

	void append(const char* buf, size_t len)
	{
		//判断data缓冲区中的剩余空间大小是否 大于要加入buf缓冲区中的内容大小
		if (avail() > static_cast<int>(len))
		{
			memcpy(cur_, buf, len);
			cur_ += len;//将cur_的指针指向的位置进行更新
		}
	}
	const char* data() const { return data_; }//返回data_
	int length() const { return static_cast<int>(cur_ - data_); }//data_指向的是开始位置，cur_ - data_表示的是整个长度

	char* current() { return cur_; }//返回cur_当前指向的位置
	int avail() const { return static_cast<int>(end() - cur_); }/*表示data缓冲区中剩余的可用缓冲区长度*/
	void add(size_t len) { cur_ += len; }//表示加入指定长度 或许是用来表示空格作用

	void reset() { cur_ = data_; }//重新将cur_指向data_的首位
	void bzero() { memset(data_, 0, sizeof(data_)); }//清空data_缓冲区中的内容

private:
	//返回 data_中指向末尾的指针
	const char* end() const { return data_ + sizeof data_; }

	char data_[SIZE];
	char* cur_;//在函数进行构造的时候就将cur_指向 data_字符数组
};

class LogStream : noncopyable
{
public:
	typedef FixedBuffer<kSmallBuffer> Buffer;//定义指定大小的Bufer

	LogStream& operator << (bool v)
	{
		//如果v为true 向buffer_中的data_中加入1 错误加入0
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
		buffer_.append(&v, 1);//向buffer_中的data_中加入v
		return *this;
	}

	LogStream& operator<<(const char* str)
	{
		if (str)
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
		//将const unsigned char* 类型转换为 char* 类型 最终会去调用operator<<(const char* str) 已在上面进行了定义
		return operator<<(reinterpret_cast<const char*>(str));
	}

	LogStream& operator<<(const std::string& v)
	{
		//向buffer_中加入长度一定的字符串
		buffer_.append(v.c_str(), v.size());
		return *this;
	}

	void append(const char* data, int len)
	{
		buffer_.append(data, len);
	}

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

	template<typename T>
	void formatInteger(T);

	Buffer buffer_;

	static const int kMaxNumericSize = 32;
};