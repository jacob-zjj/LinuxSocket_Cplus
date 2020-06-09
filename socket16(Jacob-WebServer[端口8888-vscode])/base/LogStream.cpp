#include "LogStream.h"
#include <algorithm>
#include <limits>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

const char digits[] = "9876543210123456789";
//指向digits的0 方便了后面的转换
const char* zero = digits + 9;

//参考 muduo
template<typename T>
//这是一个很有效的数字向字符串的转换 借鉴了陈硕的muduo
size_t convert(char buf[], T value)
{
	T i = value;
	char *p = buf;
	do
	{
		int lsd = static_cast<int>(i % 10);
		i /= 10;
		*p++ = zero[lsd];
	} while (i != 0);
	if (value < 0)
	{
		*p++ = '-';//这里不用考虑是否在最后面 因为我们最终会将数字字符串进行反转
	}
	*p = '\0';//结束标记
	std::reverse(buf, p);//buf代表的是首位置 p代表的是字符串的末尾位置
	return p - buf;//尾部位置 - 首部位置即为整个字符串的长度
}
template class FixedBuffer<kSmallBuffer>;//4000
template class FixedBuffer<kLargeBuffer>;//4000 x 1000
template<typename T>//kMaxNumericSize == 32
void LogStream::formatInteger(T v)
{
    //buffer容不下kMaxNumericSize个字符的话会被直接丢弃
    /*原理也很容易就想通 由于我们设定 不管是int long long long 的数字的长度都不应该超过32位，因此转换成字符串也不应该超过32位*/
    if(buffer_.avail() >= kMaxNumericSize)
    {
        //buffer_.current()表示buffer_中的cur_当前指向的data_中的位置 v表示要转换的整数值 并且已经将转换后的值放入了data_中
		//在convert函数中已经将v值进行插入
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);//移动指针 到后面插入的长度
    }
}
LogStream& LogStream::operator<<(short v)
{
    //所有转换成int类型 然后调用int类型的输出 最后都会去调用formatInteger
    *this << static_cast<int>(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned short v)
{
    //所有的unsigned short 类型先转换成usigned int类型 然后在调用formatInteger
    *this << static_cast<unsigned int>(v);
    return *this;
}
LogStream& LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned int v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(double v)
{
    if(buffer_.avail() >= kMaxNumericSize)
    {
        //求出所放入double类型v的长度
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}
LogStream& LogStream::operator<<(long double v)
{
    if (buffer_.avail() >= kMaxNumericSize)
	{
		//函数的返回值为实际加入字符串的长度 %.12g表示取v的精度问题
		int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12Lg", v);
		buffer_.add(len);
	}
	return *this;
}