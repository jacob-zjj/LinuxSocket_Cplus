#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#define SERV_PORT 8000
int main(void)
{
	struct sockaddr_in serv_addr, clie_addr;
	socklen_t clie_addr_len;
	int sockfd;
	char buf[BUFSIZ];//BUFSIZ 大小为512
	char str[INET_ADDRSTRLEN];//大小为16个字节
	int i, n;
	//int socket(int domain, int type, int protocol);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);//protocol为0表示使用默认协议 及SOCK_DGRAM所代表的UDP协议
	bzero(&serv_addr, sizeof(serv_addr));//清空地址内存
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_PORT);
	//int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
	bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));//绑定服务端地址结构，套接字等信息
	printf("Accepting connections ...\n");
	while (1)
	{
		clie_addr_len = sizeof(clie_addr);//客户端地址结构
		//替换了TCP网络通信中的read 和 accept函数
		//ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);
		n = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *)&clie_addr, &clie_addr_len);
		if (n == -1)
			perror("recvfrom error");
		//将客户端网络地址转换为本地地址const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
		//将客户端端口转换为本地端口uint16_t ntohs(uint16_t netshort);
		printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &clie_addr.sin_addr, str, sizeof(str)),ntohs(clie_addr.sin_port));
		for (i = 0; i < n; i++)
			buf[i] = toupper(buf[i]);//将收集到的字符串转换为大写
		//ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
		n = sendto(sockfd, buf, n, 0, (struct sockaddr *)&clie_addr, sizeof(clie_addr));
		if (n == -1)
			perror("sendto error");
	}
	close(sockfd);
	return 0;
}