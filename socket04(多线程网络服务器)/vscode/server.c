#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "wrap.h"
#define MAXLINE 8192
#define SRV_PORT 8000

struct s_info
{
	struct sockaddr_in cliaddr;
	int connfd;
};
void *do_work(void *arg)
{
	int n, i;
	struct s_info *ts = (struct s_info*)arg;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN]; //这个值等于16
	while (1)
	{
		n = Read(ts->connfd, buf, MAXLINE);//读客户端
		if (n == 0)
		{
			printf("the client %d closed...\n", ts->connfd);
			break;//跳出循环 关闭cfd
		}
		printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)), ntohs((*ts).cliaddr.sin_port));
		for (i = 0; i < n; i ++)
		{
			buf[i] = toupper(buf[i]);
		}
		Write(STDOUT_FILENO, buf, n);
		Write(ts->connfd, buf, n);
	}
	Close(ts->connfd);
	return (void *)0;
	//pthread_exit(0);效果一样
}
int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int listenfd, connfd;
	pthread_t tid;

	struct s_info ts[256];//创建结构体数组
	int i = 0;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);//创建一个socket 得到监听套接字	

	//端口复用 避免异常关闭服务端后无法重启
	int opt = 1;

	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));

	//memset(&srv_addr, 0, sizeof(srv_addr));	//将地址结构清零
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SRV_PORT);//指定端口号
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//指定本地任意IP

	Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));//绑定

	Listen(listenfd, 128);//设置同一时刻服务器连接上限
	printf("Accepting client connect....\n");

	while (1)
	{
		cliaddr_len = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddr_len);//阻塞监听客户端链接请求
		ts[i].cliaddr = cliaddr;
		ts[i].connfd = connfd;
		
		pthread_create(&tid, NULL, do_work, (void*)&ts[i]);
		pthread_detach(tid);//子线程分离 防止僵尸线程产生
		i++;
	}
	return 0;
}