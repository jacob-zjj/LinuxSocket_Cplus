#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#define SERV_PORT 9527	
//void sys_err(const char *str)
//{
//	perror(str);
//	exit(1);
//}
int main(int argc, char *argv[])
{
	int cfd;
	int counter = 10;
	char buf[BUFSIZ];

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	//将本地需要访问的ip地址 转换 为网络地址
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
	//SOCK_STREAM数据流的方式
	cfd = Socket(AF_INET, SOCK_STREAM, 0);
	//connect和bind的参数形式一致	区别在于bind的参数是自己的地址	而connect的参数是对方的参数
	int ret = Connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	while (--counter)
	{
		Write(cfd, "hello\n", 6);
		ret = Read(cfd, buf, sizeof(buf));
		Write(STDOUT_FILENO, buf, ret);
		sleep(1);
	}
	close(cfd);

	return 0;
}