#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stddef.h>
/*本地套接字 即参考的是网络套接字建立方式来进行改写的*/
#include "wrap.h"

//建立本地socket套接字文件
#define SERV_ADDR  "serv.socket"
int main(void)
{
	int lfd, cfd, len, size, i;
	//本地套接字和网络套接字的第一个区别在于服务器端和客户端的结构体地址与网络套接字不同
	struct sockaddr_un servaddr, cliaddr;
	char buf[4096];
	//int socket(int domain, int type, int protocol);
	//第二个区别在于此 AF_UNIX
	lfd = Socket(AF_UNIX, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	//将SERV_ADDR文件地址拷贝到服务器地址结构中去
	strcpy(servaddr.sun_path, SERV_ADDR);
	// offsetof(struct sockaddr_un, sun_path) 该偏移量求得的值即为2
	len = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);
	unlink(SERV_ADDR);/* 确保bind之前serv.sock文件不存在,bind会创建该文件 */
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	Bind(lfd, (struct sockaddr *)&servaddr, len);/* 参3不能是sizeof(servaddr) 这里需要确切的表示出大小*/
	//int listen(int sockfd, int backlog);
	Listen(lfd, 20);
	printf("Accept ...\n");
	while (1)
	{
		len = sizeof(cliaddr);
		//accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
		cfd = Accept(lfd, (struct sockaddr *)&cliaddr, (socklen_t *)&len);
		len -= offsetof(struct sockaddr_un, sun_path);      /* 得到文件名的长度 offsetof(struct sockaddr_un, sun_path) = 2*/
		cliaddr.sun_path[len] = '\0';                       /* 确保打印时,没有乱码出现 */
		printf("client bind filename %s\n", cliaddr.sun_path);
		while ((size = read(cfd, buf, sizeof(buf))) > 0) {
			for (i = 0; i < size; i++)
				buf[i] = toupper(buf[i]);
			//写到套接字中 进行传输
			write(cfd, buf, size);
			write(STDOUT_FILENO, buf, len);//将其打印在终端
		}
		close(cfd);
	}
	close(lfd);
	return 0;
}