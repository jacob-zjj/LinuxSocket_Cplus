#include "FileUtil.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
//FILE * fopen(const char * filename, const char * mode); mode表示以什么样的方式去读取文件
// 用"ae"的目的是为了 使用O_CLOEXEC 即当调用exec（）函数成功后，文件描述符会自动关闭 linux内核版本2.6.23以上加入的
AppendFile::AppendFile(string filename) : fp_(fopen(filename.c_str(), "ae"))
{
	//用户提供缓冲区
	//void setbuffer(FILE *stream, char *buf, size_t size);用文件流 指针 指向用户定义的缓冲区
	setbuffer(fp_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile()
{
	fclose(fp_);
}

//append 会向文件写
void AppendFile::append(const char* logline, const size_t len)
{
	size_t n = this->write(logline, len);
	size_t remain = len - n;
	while (remain > 0)
	{
		size_t x = this->write(logline + n, remain);
		if (x == 0)//判断是已经将指定内容写完
		{
			int err = ferror(fp_);
			if (err)
			{
				fprintf(stderr, "AppendFile::append() failed !\n");
			}
			break;
		}
		n += x;//该方法是为了表示当前文件已经写了多少，设置文件指针的偏移量
		remain = len - n;//remian  -= x
	}
}

void AppendFile::flush()
{
	//刷新FILE*类型指针 fp_ 指向的缓冲区 其实也是直接清空
	fflush(fp_);
}

size_t AppendFile::write(const char*logline, size_t len)
{
	//size_t fwrite_unlocked(const void *ptr, size_t size, size_t n, FILE *stream);
	//线程安全的写文件内容的方式 ---- 当多个进程文件流进行写入时，可能造成死锁 所以采用这个函数
	return fwrite_unlocked(logline, 1, len, fp_);
}