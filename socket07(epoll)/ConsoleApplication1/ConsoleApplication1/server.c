/*主要掌握epoll模型*/
/*
主要包含三个函数
	int epoll_create(int size);
	int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
	int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>

#include "wrap.h"

#define SRV_PORT 8000
/*打开文件上限 如果需要修改则需要修改配置文件*/
#define OPEN_MAX 1024
#define MAXLINE 8192

int main(int argc, char *argv[])
{
	int i, j, listenfd, connfd, sockfd;
	int n, num = 0;
	ssize_t nready, efd, res;
	char buf[MAXLINE], str[INET_ADDRSTRLEN];/*#define INET_ADDRSTRLEN 16*/
	socklen_t clilen;
	
	struct sockaddr_in cliaddr, servaddr;
	/*epoll中独有的 根据属性设置变量*/
	struct epoll_event tep, ep[OPEN_MAX];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	//端口复用 避免先关闭服务端后关闭客户端无法重启
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SRV_PORT);//指定端口号
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//指定本地任意IP
	Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));//绑定
	Listen(listenfd, 20);//设置同一时刻服务器连接上限
	printf("Accept client connecting！！！\n");

	efd = epoll_create(OPEN_MAX);//创建epoll模型 efd指向红黑树根节点
	if (efd == -1)
	{
		perr_exit("epoll_create error");
	}

	/*初始化*/
	tep.events = EPOLLIN;//指定lfd的监听时间为"读"
	tep.data.fd = listenfd;

	res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);//将lfd及对应的结构体设置到树上，efd可找到该树
	if (res == -1)
	{
		perr_exit("epoll_ctl error");
	}

	for (;;)
	{
		/*epoll为server阻塞监听事件 ep为struct epoll_envent 类型数组，OPEN_MAX为数组容量，-1表示永久阻塞*/
		nready = epoll_wait(efd, ep, OPEN_MAX, -1);
		if (nready == -1)
		{
			perr_exit("epoll_wait error");
		}
		for (i = 0; i < nready; i ++)
		{
			if (!(ep[i].events & EPOLLIN))//如果不是读事件 继续循环
			{
				continue;
			}
			if (ep[i].data.fd == listenfd)//判断是满足事件的fd是否为lfd
			{
				clilen = sizeof(cliaddr);
				connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);//接受连接

				printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
				printf("cfd %d---client %d\n", connfd, ++num);

				tep.events = EPOLLIN;
				tep.data.fd = connfd;
				res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
				if (res == -1)
				{
					perr_exit("epoll_ctl error");
				}
			}
			else
			{
				sockfd = ep[i].data.fd;
				n = Read(sockfd, buf, MAXLINE);

				if (n == 0)//读到0，说明客户端关闭连接
				{
					res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);//将该文件描述符从红黑树摘除
					if (res == -1)
					{
						perr_exit("epoll_ctl error");
					}
					Close(sockfd);//关闭与该客户端的连接
					printf("client[%d] closed connection\n", sockfd);
				}
				else if (n < 0)
				{
					perror("read n < 0 error: ");//出错
					res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);//摘除节点
					Close(sockfd);
				}
				else
				{
					for (j = 0; j < n; j ++)
					{
						buf[j] = toupper(buf[j]);
					}
					Write(STDOUT_FILENO, buf, n);
					Write(sockfd, buf, n);
				}
			}
		}
	}
	Close(listenfd);
	Close(efd);
	return 0;
}