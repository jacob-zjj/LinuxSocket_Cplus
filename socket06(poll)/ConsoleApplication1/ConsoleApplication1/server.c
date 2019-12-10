#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <ctype.h>

#include "wrap.h"

#define SRV_PORT 8000
#define OPEN_MAX 1024
#define MAXLINE 80

int main(int argc, char *argv[])
{
	int i, j, maxi, listenfd, connfd, sockfd;
	int nready;
	ssize_t n;

	char buf[MAXLINE], str[INET_ADDRSTRLEN];/*#define INET_ADDRSTRLEN 16*/
	socklen_t clilen;
	struct pollfd client[OPEN_MAX];
	struct sockaddr_in cliaddr, serv_addr;
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	//端口复用 避免先关闭服务端后关闭客户端无法重启
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SRV_PORT);//指定端口号
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);//指定本地任意IP
	Bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));//绑定
	Listen(listenfd, 128);//设置同一时刻服务器连接上限
	printf("Accept client connecting！！！\n");

	client[0].fd = listenfd;
	client[0].events = POLLIN;
	//将其他的文件描述符都赋值为-1
	for (i = 1; i < OPEN_MAX; i ++)
	{
		client[i].fd = -1;
	}
	maxi = 0;//指向listenfd;
	for (;;)
	{
		nready = poll(client, maxi + 1, -1);
		//处理listenfd
		if (client[0].revents & POLLIN)//判断是否为读事件
		{
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);//建立连接 ...不会阻塞
			printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
			for (i = 1; i < OPEN_MAX; i++)
			{
				if (client[i].fd < 0)		/*找到client[]中没有使用的位置--换句话说数组中闲置的空间*/
				{
					client[i].fd = connfd;	/*保存accept返回的文件描述符到client[]中*/
					break;
				}
			}
			if (i == OPEN_MAX)				/*说明达到了最大客户端数量*/
			{
				perr_exit("too many clients!!!");
			}
			client[i].events = POLLIN;/*设置刚刚返回的connfd,监控读事件*/
			if (i > maxi)
			{
				maxi = i;/*更新client[]中最大的元素下标*/
			}
			if (--nready <= 0)
			{
				continue;/*没有更多就绪事件时，继续回到poll阻塞*/
			}
		}
		for (i = 1; i <= maxi; i++)/*检测哪个clients有数据就绪*/
		{
			if ((sockfd = client[i].fd) < 0)
			{
				continue;/*异常处理*/
			}
			if (client[i].revents & POLLIN)//找到满足读事件的fd
			{
				if ((n = Read(sockfd, buf, MAXLINE)) < 0)//检测到客户端已经关闭连接
				{
					//当读函数返回值小于0的时候有四中情况需要考虑
					if (errno == ECONNABORTED)/*收到RST标志*/
					{
						printf("client[%d] aborted connection\n", i);
						Close(sockfd);
						client[i].fd = -1;/*poll中不监控该文件描述符 直接置为-1即可 不用像select中那样移除*/
					}
					else
					{
						perr_exit("read error!!!");
					}
				}
				else if (n == 0)	/*说明客户端先关闭连接*/
				{
					printf("client[%d] closed connection\n", i);
					Close(sockfd);
					client[i].fd = -1;
				}
				else
				{
					for (j = 0; j < n; j++)
					{
						buf[j] = toupper(buf[j]);
					}
					Writen(sockfd, buf, n);//回写数据给客户端
					//将数据进行包装显示
					Write(STDOUT_FILENO, buf, n);//显示在服务器端
				}
				if (--nready <= 0)
				{
					break;
				}
			}
		}
	}
	return 0;
}