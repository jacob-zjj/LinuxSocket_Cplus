#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stddef.h>

#include "wrap.h"

//需要创建客户端和服务端的套接字 因为在连接服务端时需要使用到服务端的套接字结构
#define SERV_ADDR "serv.socket"
#define CLIE_ADDR "clie.socket"

int main(void)
{
	int  cfd, len;
	struct sockaddr_un servaddr, cliaddr;
	char buf[4096];
	cfd = Socket(AF_UNIX, SOCK_STREAM, 0);
	bzero(&cliaddr, sizeof(cliaddr));
	cliaddr.sun_family = AF_UNIX;
	strcpy(cliaddr.sun_path, CLIE_ADDR);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(cliaddr.sun_path);     /* 计算客户端地址结构有效长度 */
	unlink(CLIE_ADDR);
	Bind(cfd, (struct sockaddr *)&cliaddr, len);                                 /* 客户端也需要bind, 不能依赖自动绑定*/
	/*服务端地址初始化*/
	bzero(&servaddr, sizeof(servaddr));                                          /* 构造server 地址 */
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, SERV_ADDR);

	len = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);   /* 计算服务器端地址结构有效长度 */
	//int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	Connect(cfd, (struct sockaddr *)&servaddr, len);
	//fgets(buf, sizeof(buf), stdin) != NULL 等待客户端进行输入 如果不是结束标志则不会停止
	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		write(cfd, buf, strlen(buf));//将字符串写入到套接字中进行传输
		len = read(cfd, buf, sizeof(buf));//读套接字中的内容 到buf中 并返回字符串的 长度
		write(STDOUT_FILENO, buf, len);//将其打印在终端
	}
	close(cfd);
	return 0;
}