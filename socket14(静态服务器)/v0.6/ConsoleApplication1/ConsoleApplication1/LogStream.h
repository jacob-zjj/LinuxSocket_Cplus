#pragma once
/*assert.h是c标准库的一个头文件，该头文件的主要目的就是提供一个assert的宏定义。
assert只是对所给的表达式求值，就像if判断语句中一样，然后如果该值为真则正常运行，
否则报错，并调用abort(),产生异常中断，exit出来。*/
#include <assert.h>
#include <string.h>
#include <string>
#include "nocopyable.h"

class AsyncLogging;
//使用双缓冲的方式 将数据进行处理
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer : noncopyable
{
public:
	FixedBuffer() : cur_(data_)
	{}
	~FixedBuffer() 
	{}
	void append(const char* buf, size_t len)
	{
		if (avail() > len)
		{
			memcpy(cur_, buf, len);
			cur_ += len;
		}
	}
	const char* data() const
	{
		return data_;
	}
	int length() const
	{
		return static_cast<int>(cur_ - data_);
	}
	char* current()
	{
		return cur_;
	}
	int avail() const
	{
		return static_cast<int>(end() - cur_);
	}
	void add(size_t len)
	{
		cur_ += len;
	}
	void reset()
	{
		cur_ = data_;
	}
	void bzero()
	{
		memset(data_, 0, sizeof data_);
	}
private:
	const char* end() const 
	{ 
		return data_ + sizeof data_; 
	}
	char data_[SIZE];
	char* cur_;
};

class LogStream : noncopyable
{
	typedef LogStream self;
public:
	typedef FixedBuffer<kSmallBuffer> Buffer;
	self& operator << (bool v)
	{
		buffer_.append(v ? "1" : "0", 1);
		return *this;
	}
	self& operator<<(short);
	self& operator<<(unsigned short);
	self& operator<<(int);
	self& operator<<(unsigned int);
	self& operator<<(long);
	self& operator<<(unsigned long);
	self& operator<<(long long);
	self& operator<<(unsigned long long);

	self& operator<<(const void*);

	self& operator<<(float v)
	{
		*this << static_cast<double>(v);
		return *this;
	}
	self& operator<<(double);
	self& operator<<(char v)
	{
		buffer_.append(&v, 1);
		return *this;
	}
	self& operator<<(const char* str)
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

	self& operator<<(const unsigned char* str)
	{
		return operator<<(reinterpret_cast<const char*>(str));
	}

	self& operator<<(const std::string& v)
	{
		buffer_.append(v.c_str(), v.size());
		return *this;
	}

	void append(const char* data, int len) 
	{ 
		buffer_.append(data, len); 
	}

	const Buffer& buffer() const 
	{ 
		return buffer_; 
	}

	void resetBuffer() 
	{ 
		buffer_.reset(); 
	}

private:
	void staticCheck();
	template<typename T>
	void formatInteger(T);
	Buffer buffer_;
	static const int kMaxNumericSize = 32;
};