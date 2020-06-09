#include "Util.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>//互联网连接地址族 设置ipv4 和 端口等
#include <netinet/tcp.h>
const int MAX_BUFF = 4096;

//设置读函数的几种类型 这里是针对char*类型
ssize_t readn(int fd, void *buff, size_t n)
{
    /*记录当前读到的位置和剩余的字符个数*/
    size_t nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    char *ptr = (char*)buff;
    while(nleft > 0)
    {
        /*调用系统的读函数 返回读到的字符个数*/
        if((nread = read(fd, ptr, nleft)) < 0)
        {
            //表明此次没有读到数据 那此次的nread就为0
            if (errno == EINTR)
			{
				nread = 0;
			}
            //表示无数据可读 直接返回已经读到的数据长度
            else if (errno == EAGAIN)
			{
				return readSum;
			}
			else
			{
				return -1;
			}
        }
        //数据已经读完
        else if (nread == 0)
		{
			break;
		}
		readSum += nread;
		nleft -= nread;
		ptr += nread;
    }
    return readSum;//返回总共读到的字符个数
}
ssize_t readn(int fd, std::string &inBuffer, bool &zero)
{
    ssize_t nread = 0;
	ssize_t readSum = 0;
	while (true)
	{
		//MAX_BUFF = 4096
		char buff[MAX_BUFF];
		if ((nread = read(fd, buff, MAX_BUFF)) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EAGAIN)
			{
				return readSum;
			}
			else
			{
				perror("read error");
				return -1;
			}
		}
		else if (nread == 0)
		{
			zero = true;
			break;
		}
		readSum += nread;
		inBuffer += std::string(buff, buff + nread);//将inBuffer的指针从定位
	}
	return readSum;
}
ssize_t readn(int fd, std::string &inBuffer)
{
    ssize_t nread = 0;
	ssize_t readSum = 0;
	while (true)
	{
		char buff[MAX_BUFF];
		if ((nread = read(fd, buff, MAX_BUFF)) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EAGAIN)
			{
				return readSum;
			}
			else
			{
				perror("read error");
				return -1;
			}
		}
		else if (nread == 0)
		{
			break;
		}
		readSum += nread;
		inBuffer += std::string(buff, buff + nread);
	}
	return readSum;
}
//设置写函数的几种类型
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
				if (errno == EINTR)
				{
					nwritten = 0;
					continue;
				}
				else if(errno == EAGAIN)
				{
					return writeSum;
				}
				else
				{
					return -1;
				}
			}
		}
		//直接向对面写即可 不需要判断是否写入为0
		writeSum += nwritten;
		nleft -= nwritten;
		ptr += nwritten;
	}
	return writeSum;
}
ssize_t writen(int fd, std::string &sbuff)
{
    size_t nleft = sbuff.size();
	ssize_t nwritten = 0;
	ssize_t writeSum = 0;
	const char *ptr = sbuff.c_str();
	while (nleft > 0)
	{
		if ((nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (nwritten < 0)
			{
				if (errno == EINTR)
				{
					nwritten = 0;
					continue;
				}
				else if (errno == EAGAIN)
					break;
				else
					return -1;
			}
		}
		writeSum += nwritten;
		nleft -= nwritten;
		ptr += nwritten;
	}
	if (writeSum == static_cast<int>(sbuff.size()))
		sbuff.clear();
	else
		sbuff = sbuff.substr(writeSum);
	return writeSum;
}
//处理信号管道
void handle_for_sigpipe()
{
    struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	//捕捉SIGPIPE信号
	if (sigaction(SIGPIPE, &sa, NULL))
	{
		return;
	}
}
//设置非阻塞
int setSocketNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
	if (flag == -1)
	{
		return -1;
	}
	flag |= O_NONBLOCK;
	//设置为非阻塞 如果成功返回 0  如果失败返回 -1
	if (fcntl(fd, F_SETFL, flag) == -1)
	{
		return -1;
	}
	return 0;
}
//设置网络 不使用Nagle算法
void setSocketNodelay(int fd)
{
    int enable = 1;
    //IPPROTO_TCP TCP选项 TCP_NODELAY 不使用Nagle算法
	/*
	默认情况下Nagle算法是默认开启的，
	在默认的情况下,Nagle算法是默认开启的，Nagle算法比较适用于发送方发送大批量的小数据，并且接收方作出及时回应的场合，
	这样可以降低包的传输个数。
	
	同时协议也要求提供一个方法给上层来禁止掉Nagle算法当你的应用不是连续请求+应答的模型的时候，
	而是需要实时的单项的发送数据并及时获取响应，这种case就明显不太适合Nagle算法，明显有delay的。

	不用等待ACK 可以连续发送
	*/
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}
//设置优雅退出还是 强制退出
void setSocketNoLinger(int fd)
{
    /*三种断开方式： Linux下tcp连接断开的时候调用close()函数，有优雅断开和强制断开两种方式。
	1. l_onoff = 0; l_linger忽略
		close()立刻返回，底层会将未发送完的数据发送完成后再释放资源，即优雅退出。

	2. l_onoff != 0; l_linger = 0;
		close()立刻返回，但不会发送未发送完成的数据，而是通过一个REST包强制的关闭socket描述符，即强制退出。

	3. l_onoff != 0; l_linger > 0;
		close()不会立刻返回，内核会延迟一段时间，这个时间就由l_linger的值来决定。如果超时时间到达之前，
		发送完未发送的数据(包括FIN包)并得到另一端的确认，close()会返回正确，socket描述符优雅性退出。
		否则，close()会直接返回错误值，未发送数据丢失，socket描述符被强制性退出。
		需要注意的是，如果socket描述符被设置为非堵塞型，则close()会直接返回值。
	*/
    struct linger linger_;
	linger_.l_onoff = 1;
	linger_.l_linger = 30;
    //SO_LINGER 设置端口延迟关闭
    //SOL_SOCKET 通用套接字选项
    setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&linger_, sizeof(linger_));
}
//函数原型int shutdown(int sockfd, int how)
/*参数说明：
(1) sockfd是要关闭的套接字
(2) 该函数的行为依赖于how参数的值

a) SHUT_RD，关闭连接的读这一半，套接字中不再有数据可接收，而且套接字接收缓冲区中的现有数据都被丢弃，
			进程不能再对这样的套接字调用任何读函数。该套接字接收的来自对端的任何数据都被确认，然后悄然丢弃。
b) SHUT_WR，关闭连接的写这一半，当前留在套接字发送缓冲区的数据都将被发送掉，后跟TCP的正常连接终止序列，
			进程不能再对这样的套接字调用任何写函数。
c) SHUT_RDWR，连接的读半部与写半部都关闭。*/
//关闭 套接字读写
//这里 使用半关闭的方法 关闭写端
void shutDownWR(int fd)
{
    //其终止网络连接的方式 还是需要调用系统的close()函数
    shutdown(fd, SHUT_WR);//将当前套接字中发送缓冲区的数据发送掉 然后进程不能再对套接字调用任何写函数 套接字关闭
}
//sokcet绑定和监听函数
int socket_bind_listen(int port)
{
    //检查port 取正常值范围
	if (port < 0 || port > 65535)
	{
		return -1;
	}
	//创建socket(IPV4 + TCP) 返回监听描述符
	int listen_fd = 0;
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return -1;
	}
	//消除bind时出现端口已经使用 因此我们使用端口复用 设置端口复用
	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		return -1;
	}
	//设置服务器IP和Port，和监听描述符绑定
	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);
	if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		return -1;
	}
	//开始监听 最大等待队列长为LISTENQ 这里我们设置为2048
	if (listen(listen_fd, 2048) == -1)
	{
		return -1;
	}
	//无效监听描述符
	if (listen_fd == -1)
	{
		close(listen_fd);
		return -1;
	}
	return listen_fd;
}
//-------------------------------------该部分的功能是将地址栏的中文字符进行转码 及编码-------------------------------------------
//16进制转换成10进制
int hexit(char c)
{
    if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}
	return 0;
}
//这里主要是用来处理 当请求的路径中包含中文字符 那就进行编码
//解码
void decode_str(char *to, char *from)
{
	for (; *from != '\0'; ++to, ++from)
	{
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
		{
			*to = hexit(from[1]) * 16 + hexit(from[2]);
			from += 2;
		}
		else
		{
			*to = *from;
		}
	}
	*to = '\0';
}
//编码 编码为16进制
void encode_str(char *to, int tosize, const char* from)
{
	int tolen;
	for(tolen = 0; *from != '\0' && tolen + 4 < tosize; ++ from)
	{
		if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0)
		{
			*to = *from;
			++to;
			++tolen;
		}
		else
		{
			//%%表示转义的意思
			sprintf(to, "%%%02x", (int)*from & 0xff);
			to += 3;
			tolen += 3;
		}
	}
	*to = '\0';
}
//--以上服务于对地址进行解码和编码，由于输入的地址是中文时，前台传过来的地址数据需要进行解码才能识别-------