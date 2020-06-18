#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
//指定读多少数据
ssize_t readn(int fd, void *buff, size_t n)
{
	size_t nleft = n;
	ssize_t nread = 0;
	ssize_t readSum = 0;
	char *ptr = (char*)buff;
	while (nleft > 0)
	{
		if ((nread = read(fd, ptr, nleft)) < 0)
		/*ssize_t read(int fd, void *buf, size_t count);
		On success, the number of bytes read is returned (zero indicates end of  file),  and  the  file
		position  is  advanced  by  this number.(成功返回读到的位数 并且将文件描述符进行移动)
		*/ 
		{
			if (errno == EINTR)
				nread = 0;
			else if (errno == EAGAIN)
			{
				return readSum;
			}
			else
			{
				return -1;
			}
		}
		else if (nread == 0)
			break;
		readSum += nread;
		nleft -= nread;
		ptr += nread;
	}
	return readSum;
}
//指定写多少数据
ssize_t writen(int fd, void *buff, size_t n)
{
	size_t nleft = n;
	ssize_t nwritten = 0;
	ssize_t writeSum = 0;
	char *ptr = (char*)buff;
	while (nleft > 0)
	{
		if ((nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (nwritten < 0)
			{
				if (errno == EINTR || errno == EAGAIN)
				{
					nwritten = 0;
					continue;
				}
				else
					return -1;
			}
		}
		writeSum += nwritten;
		nleft -= nwritten;
		ptr += nwritten;
	}
	return writeSum;
}

void handle_for_sigpipe()
{
	/*收藏的博客有提到这一问题
	struct sigaction {
		void(*sa_handler)(int);
		void(*sa_sigaction)(int, siginfo_t *, void *);
		sigset_t sa_mask;
		int sa_flags;
		void(*sa_restorer)(void);
	}
	sa_handler此参数和signal()的参数handler相同，代表新的信号处理函数
	sa_mask 用来设置在处理该信号时暂时将sa_mask 指定的信号集搁置
	sa_flags 用来设置信号处理的其他相关操作，下列的数值可用。 
	SA_RESETHAND：当调用信号处理函数时，将信号的处理函数重置为缺省值SIG_DFL
	SA_RESTART：如果信号中断了进程的某个系统调用，则系统自动启动该系统调用
	SA_NODEFER ：一般情况下， 当信号处理函数运行时，内核将阻塞该给定信号。
	但是如果设置了 SA_NODEFER标记， 那么在该信号处理函数运行时，内核将不会阻塞该信号
	*/
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;//ignore
	sa.sa_flags = 0;
	//int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
	//signum参数指出要捕获的信号类型，act参数指定新的信号处理方式，oldact参数输出先前信号的处理方式（如果不为NULL的话）。
	//SIGPIPE 13
	//由于tcp连接 四次握手的过程是全双工的 因此会存在两次调用write() 而第二次调用的时候将会导致 内核发送信号 SIGPIPE 因此
	//忽略这个信号 是高并发服务器的一个好的处理方式---详细请见收藏的博客
	if (sigaction(SIGPIPE, &sa, NULL))
	{
		return;
	}
	/*
	//忽略SIGPIPE信号，这常用于并发服务器的性能的一个技巧
	//因为并发服务器常常fork很多子进程，子进程终结之后需要
	//服务器进程去wait清理资源。如果将此信号的处理方式设为
	//忽略，可让内核把僵尸子进程转交给init进程去处理，省去了
	//大量僵尸进程占用系统资源。
	*/
}
//设置非阻塞
int setSockNonBlocking(int fd)
{
	/*文件状态标志：F_GETFL F_SETFL
	F_GETFL: 获取文件访问模式和文件状态标志;参数被忽略。
	(O_APPEND、O_ASYNC、O_DIRECT、O_NOATIME和O_NONBLOCK)---flag
	*/
	int flag = fcntl(fd, F_GETFL, 0);
	if (flag == -1)
	{
		return -1;
	}
	flag |= O_NONBLOCK;
	//int fcntl(int fd, int cmd, ... /* arg */);
	/*文件状态标志：F_GETFL F_SETFL
	F_SETFL: 将文件状态标志设置为arg指定的值。
	文件访问模式(O_RDONLY, O_WRONLY, O_RDWR)和文件创建标志(即、O_CREAT、O_EXCL、O_NOCTTY、O_TRUNC)在arg中被忽略。
	在Linux上，这个命令只能更改O_APPEND、O_ASYNC、O_DIRECT、O_NOATIME和O_NONBLOCK标志。
	*/
	if (fcntl(fd, F_SETFL, flag) == -1)
	{
		return -1;
	}
	return 0;
}
