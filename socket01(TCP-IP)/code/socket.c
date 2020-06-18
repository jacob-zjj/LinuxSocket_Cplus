#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#define SERV_PORT 9527
void sys_err(const char *str)
{
	perror(str);
	exit(1);
}
int main(int argc, char *argv[])
{
	int lfd = 0, cfd = 0;
	int ret;
	int i;
	char buf[BUFSIZ], client_IP[1024];//BUFSIZ 4096
	struct sockaddr_in serv_addr, clit_addr;
	socklen_t clit_addr_len;
	/*服务端需要将本机设置的端口和IP地址转换成网络字节序*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);//htons 表示16位短整型
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);//htons 表示32位长整型
	//新建一个网络套接字
	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		sys_err("socket error!!!");
	}
	//sockaddr_in  地址结构 是一种结构体类型
	bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	listen(lfd, 128);

	clit_addr_len = sizeof(clit_addr);
	//接受客户端的
	cfd = accept(lfd, (struct sockaddr*)&clit_addr, &clit_addr_len);
	if (cfd == -1)
	{
		sys_err("accept error");
	}
	/*inet_ntop 将网络ip地址转换成本地ip地址  由于网络ip地址和本机ip地址类型是不同的两种类型*/
	printf("client ip: %s port: %d\n",
		inet_ntop(AF_INET, &clit_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
		/*将网络端口 转换成  本地网络端口*/
		ntohs(clit_addr.sin_port)
	);
	while (1)
	{
		ret = read(cfd, buf, sizeof(buf));
		//写给本地缓冲区
		write(STDOUT_FILENO, buf, ret);
		for (i = 0; i < ret; i++)
		{
			buf[i] = toupper(buf[i]);
		}
		//通过文件描述符写给客户端
		write(cfd, buf, ret);
	}	
	close(lfd);
	close(cfd);
	return 0;
}