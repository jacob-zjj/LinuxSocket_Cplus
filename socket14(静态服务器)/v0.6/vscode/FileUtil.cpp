#include "FileUtil.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

AppendFile::AppendFile(std::string filename) : fp_(fopen(filename.c_str(), "ae"))
{
	//该函数包含在stdio.h的头文件中 是基于linux系统的
	//void setbuffer(FILE * stream, char * buf, size_t size);
	/*在打开文件流后, 读取内容之前, 调用setbuffer()可用来设置文件流的缓冲区. 
	参数stream 为指定的文件流, 参数buf 指向自定的缓冲区起始地址, 参数size 为缓冲区大小.*/
	setbuffer(fp_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile()
{
	fclose(fp_);
}

void AppendFile::append(const char* logline, const size_t len)
{
	size_t n = write(logline, len);
	size_t remain = len - n;
	//直到将logline中的数据写完为止
	while (remain > 0)
	{
		size_t x = write(logline + n, remain);
		if (x == 0)
		{
			int err = ferror(fp_);
			if (err)
			{
				fprintf(stderr, "AppendFile::append() failed! \n");
			}
			break;
		}
		n += x;
		remain = len - n;
	}
}

void AppendFile::flush()
{
	fflush(fp_);
}

size_t AppendFile::write(const char* logfile, const size_t len)
{
	// #undef fwrite_unlocked
	return ::fwrite_unlocked(logfile, 1, len, fp_);
}