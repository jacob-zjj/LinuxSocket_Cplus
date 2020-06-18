#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define SERV_PORT 8000

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	int sockfd, n;//n用来表示的就是所调用sendto和recvfrom的返回值
	char buf[BUFSIZ];//512
	//int socket(int domain, int type, int protocol);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	//int inet_pton(int af, const char *src, void *dst);
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT);
	//udp通信时不使用bind函数进行绑定
	//char *fgets(char *s, int size, FILE *stream);
	while (fgets(buf, BUFSIZ, stdin) != NULL)
	{
		//ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if (n == -1)
			perror("sendto error");
		//替换掉TCP中的read函数
		//ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);
		n = recvfrom(sockfd, buf, BUFSIZ, 0, NULL, 0);
		printf("received from %s at PORT %d\n", "127.0.0.1", SERV_PORT);
		if (n == -1)
			perror("recvfrom error");
		write(STDOUT_FILENO, buf, n);
	}
	close(sockfd);
	return 0;
}