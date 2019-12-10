#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <arpa/inet.h>
void read_cb(struct bufferevent *bev, void *arg)
{
	char buf[1024] = { 0 };
	//size_t  bufferevent_read(struct bufferevent *bev, void *buf, size_t bufsize);
	bufferevent_read(bev, buf, sizeof(buf));
	printf("fwq say: %s\n", buf);
	bufferevent_write(bev, buf, strlen(buf) + 1);
	sleep(1);
}

void write_cb(struct bufferevent *bev, void *arg)
{
	printf("-------------I'm a client call back!!!\n");
}

void event_cb(struct bufferevent *bev,short events, void *arg)
{
	if (events & BEV_EVENT_EOF)
	{
		printf("Connection closed\n");
	}
	else if (events & BEV_EVENT_ERROR)
	{
		printf("some other error\n");
	}
	else if (events & BEV_EVENT_CONNECTED)
	{
		printf("connected server successfully!\n");
		return;
	}
	//释放资源
	bufferevent_free(bev);
	printf("bufferevent 资源已经被释放...\n");
}

void read_terminal(evutil_socket_t fd, short what, void *arg)
{
	//读数据
	char buf[1024] = { 0 };
	int len = read(fd, buf, sizeof(buf));

	struct bufferevent* bev = (struct bufferevent*)arg;
	//发送数据
	bufferevent_write(bev, buf, len + 1);
}

int main(int argc, const char *argv[])
{
	struct event_base* base = NULL;
	base = event_base_new();
	
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	//通信的fd放到bufferevent中
	struct bufferevent* bev = NULL;
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

	//init server info
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(9876);
	inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr.s_addr);
	
	//连接服务器
	bufferevent_socket_connect(bev, (struct sockaddr*)&serv, sizeof(serv));
	
	//设置回调
	bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);

	//设置回调生效
	bufferevent_enable(bev, EV_READ);
	//该函数使得读回调可以被触发

	//创建事件
	struct event* ev = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST,
		read_terminal, bev);
	
	//添加事件
	event_add(ev, NULL);
	
	event_base_dispatch(base);

	event_free(ev);

	event_base_free(base); 
	
	return 0;
}