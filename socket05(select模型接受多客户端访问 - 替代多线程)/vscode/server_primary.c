#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "wrap.h"

#define SRV_PORT 8000

int main(int argc, char *argv[])
{
	int listenfd, connfd;//connectfd

	struct sockaddr_in clie_addr, serv_addr;
	socklen_t clie_ddr_len;

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
	fd_set rset, allset;//定义读集合 备份集合allset
	int ret, maxfd = 0, n, i, j;
	char buf[BUFSIZ];
	maxfd = listenfd;//最大文件描述符

	FD_ZERO(&allset);//清空监听集合
	FD_SET(listenfd, &allset);//将待监听fd添加到监听集合中
	
	while (1)
	{
		rset = allset;//备份
		ret = select(maxfd + 1, &rset, NULL, NULL, NULL);//使用select监听
		if (ret < 0)
		{
			perr_exit("select error");
		}
		/*
			函数在这里的处理不是一次性的将众多描述符加入到集合中	
			而是每次循环的进行 把描述符加入到集合中 每次只加入一个
		*/
		if (FD_ISSET(listenfd, &rset))//判断listenfd是否在rset中	//listenfd满足读事件
		{
			clie_ddr_len = sizeof(clie_addr);
			connfd = Accept(listenfd, (struct sockaddr*)&clie_addr, &clie_ddr_len);//建立连接 ...不会阻塞
			FD_SET(connfd, &allset);//将新产生的fd添加到监听集合中
			if (maxfd < connfd)
			{
				maxfd = connfd;//修改maxfd
			}
			if (ret == 1)	//说明select只返回一个 并且是listenfd 后续指令无需执行
			{
				continue;//跳出一次while循环
			}
		}
		/*
			每次都会循环的查找查看是否有客户端向服务器发送数据
		*/
		for (i = listenfd + 1; i <= maxfd; i ++)//处理满足读事件的fd
		{
			if (FD_ISSET(i, &rset))//找到满足读事件的fd
			{
				n = Read(i, buf, sizeof(buf));
				if (n == 0)//检测到客户端已经关闭连接
				{
					Close(i);
					FD_CLR(i, &allset);//将关闭的fd,移除出监听集合
				}
				else if(n == -1)
				{ 
					perr_exit("read error");
				}
				for (j = 0; j < n; j ++)
				{
					buf[j] = toupper(buf[j]);
				}
				Write(i, buf, n);//写给客户端
				Write(STDOUT_FILENO, buf, n);//显示在服务器端
			}
		}
	}

	Close(listenfd);
}