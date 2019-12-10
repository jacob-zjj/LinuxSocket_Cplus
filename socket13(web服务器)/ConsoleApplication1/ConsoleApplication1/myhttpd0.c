#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXSIZE 2048

int init_listen_fd(int port, int epfd)
{
	//创建监听的套接字 lfd
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		perror("socket error");
		exit(1);
	}
	//创建服务器地址结构 IP + port
	struct sockaddr_in srv_addr;

	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(port);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//端口复用
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt);

	//给lfd 绑定地址结构
	int ret = bind(lfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if (ret == -1)
	{
		perror("bind error");
		exit(1);
	}
	//设置监听上限
	ret = listen(lfd, 128);
	if (ret == -1)
	{
		perror("listen error");
		exit(1);
	}
	//lfd 添加到 epoll树上
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = lfd;

	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
	if (ret == -1)
	{
		perror("epoll_ctl add lfd error");
		exit(1);
	}
	return lfd;
}

void do_accept(int lfd, int epfd)
{
	struct sockaddr_in clt_addr;
	socklen_t clt_addr_len = sizeof(clt_addr);

	int cfd = accept(lfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
	if (cfd == -1)
	{
		perror("accept error");
		exit(1);
	}

	//打印客户端IP + port
	char client_ip[64] = { 0 };
	printf("New Client IP: %s, Port: %d, cfd = %d\n",
		inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
			ntohs(clt_addr.sin_port), cfd
		);

	//设置cfd 非阻塞
	int flag = fcntl(cfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(cfd, F_SETFL, flag);

	//将新节点cfd挂到epoll监听树上
	struct epoll_event ev;
	ev.data.fd = cfd;

	//边沿非阻塞模式 即ET模式
	ev.events = EPOLLIN | EPOLLET;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, &ev);
	if (ret == -1)
	{
		perror("epoll_ctl add cfd error");
		exit(1);
	}
}

void do_read(int cfd, int epfd)
{
	//read cfd 小--大 write回
	//读取一行http协议 拆分 获取get文件名 协议号
}

void epoll_run(int port)
{
	int i = 0;
	struct epoll_event all_events[MAXSIZE];

	//创建一个epoll监听树根
	int epfd = epoll_create(MAXSIZE);
	if (epfd == -1)
	{
		perror("epoll_create error");
		exit(1);
	}

	//创建lfd,并添加至监听树
	int lfd = init_listen_fd(port, epfd);

	while (1)
	{
		//监听节点对应事件
		int ret = epoll_wait(epfd, all_events, MAXSIZE, -1);//阻塞的方式 非阻塞就是轮询
		if (ret == -1)
		{
			perror("epoll_wait error");
			exit(1);
		}
		for (i = 0; i < ret; i ++)
		{
			//只处理读事件 其他事件默认不处理
			struct epoll_event *pev = &all_events[i];

			//不是读事件
			if (!(pev->events & EPOLLIN))
			{
				continue;
			}
			if (pev->data.fd == lfd)	//接受连接请求
			{
				do_accept(lfd, epfd);
			}
			else//读数据
			{
				do_read(pev->data.fd, epfd);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	//命令行参数获取端口 和 server提供的目录
	if (argc < 3)
	{
		printf("./server port path\n");
	}
	//获取用户输入的端口
	int port = atoi(argv[1]);
	//改进进程工作目录
	int ret = chdir(argv[2]);
	if (ret != 0)
	{
		perror("chdir error");
		exit(1);
	}
	//启动 epoll监听
	epoll_run(port);
	return 0;
}