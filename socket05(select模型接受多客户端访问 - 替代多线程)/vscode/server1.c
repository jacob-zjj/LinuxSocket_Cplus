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
	int i, j, n, maxi;

	int nready, client[FD_SETSIZE];/*自定义数组client 防止遍历1024个文件描述符 FD_SETSIZE默认为1024*/
	int maxfd, listenfd, connfd, sockfd;//connectfd
	char buf[BUFSIZ], str[INET_ADDRSTRLEN];/*#define INET_ADDRSTRLEN 16*/
	
	struct sockaddr_in clie_addr, serv_addr;
	socklen_t clie_ddr_len;
	fd_set rset, allset;//定义读集合 备份集合allset

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

	maxfd = listenfd;//最大文件描述符

	maxi = -1;//首先将maxfd_index最大描述下标指向-1
	for (i = 0; i < FD_SETSIZE; i++)
	{
		client[i] = -1;//将数组中所有位置先都先设置为-1
	}
	FD_ZERO(&allset);//清空监听集合
	FD_SET(listenfd, &allset);//将待监听fd添加到监听集合中

	while (1)
	{
		rset = allset;//备份
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);//使用select监听 
		if (nready < 0)
		{
			perr_exit("select error");
		}
		if (FD_ISSET(listenfd, &rset))//判断listenfd是否在rset中	//listenfd满足读事件
		{
			clie_ddr_len = sizeof(clie_addr);
			connfd = Accept(listenfd, (struct sockaddr*)&clie_addr, &clie_ddr_len);//建立连接 ...不会阻塞
			printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &clie_addr.sin_addr, str, sizeof(str)), ntohs(clie_addr.sin_port));
			for (i = 0; i < FD_SETSIZE; i ++)
			{
				if (client[i] < 0)		/*找到client[]中没有使用的位置*/
				{
					client[i] = connfd;	/*保存accept返回的文件描述符到client[]中*/
					break;
				}
			}
			if(i == FD_SETSIZE)	/*达到select能监听的文件个数*/
			{
				fputs("too many clients\n", stderr);
			}
			FD_SET(connfd, &allset);/*向监控文件描述符集合allset添加新的文件描述符connfd*/
			if (connfd > maxfd)
			{
				maxfd = connfd;/*select第一个参数需要使用最大文件描述符 + 1*/
			}
			if (i > maxi)	/*保证maxi存的总是client[]最后一个元素下标*/
			{
				maxi = i;
			}
			if (--nready == 0)//如果只有一个文件描述符即为listenfd，可以直接进行下一次循环
			{
				continue;
			}
		}
		for (i = 0; i <= maxi; i++)/*检测哪个clients有数据就绪*/
		{
			if ((sockfd = client[i]) < 0)
			{
				continue;
			}
			if (FD_ISSET(sockfd, &rset))//找到满足读事件的fd
			{
				if((n = Read(sockfd, buf, sizeof(buf))) == 0)//检测到客户端已经关闭连接
				{
					Close(sockfd);
					FD_CLR(sockfd, &allset);/*解除select对此文件描述符的监控*/
					client[i] = -1;
				}
				else if (n > 0)
				{
					for (j = 0; j < n; j++)
					{
						buf[j] = toupper(buf[j]);
					}
					Write(sockfd, buf, n);//写给客户端
					Write(STDOUT_FILENO, buf, n);//显示在服务器端
				}
				
			}
		}
	}
	Close(listenfd);
	return 0;
}