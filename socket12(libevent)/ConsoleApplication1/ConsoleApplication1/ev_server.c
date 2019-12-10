#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
//读缓冲区回调
//void read_cb(struct bufferevent *bev, void *cbarg)
void read_cb(struct bufferevent *bev, void *arg)
{
	char buf[1024] = { 0 };
	//size_t  bufferevent_read(struct bufferevent *bev, void *buf, size_t bufsize);
	bufferevent_read(bev, buf, sizeof(buf));
	char *p = "I'm a server, I have received your send-date!";
	
	//写数据给客户端
	bufferevent_write(bev, p, strlen(p) + 1);

	sleep(1);
}

//写缓冲回调
//void write_cb(struct bufferevent *bev, void *cbarg)
void write_cb(struct bufferevent *bev, void *arg)
{
	printf("I'm a server, I have send the date to client successful, function write_cb have been used...\n");
}

//事件回调
//void event_cb(struct bufferevent *bev,short events, void *cbarg)
void event_cb(struct bufferevent *bev, short events, void *arg)
{
	if (events & BEV_EVENT_EOF)
	{
		printf("Connection closed\n");
	}
	else if (events & BEV_EVENT_ERROR)
	{
		printf("some other error\n");
	}
	bufferevent_free(bev);
	printf("bufferevent 资源已经被释放...\n");
}

//监听回调
void cb_listener(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int len, void *ptr)
{
	//调用回调函数说明与客户端连接成功
	printf("connect new client\n");
	struct event_base* base = (struct event_base*)ptr;

	//客户端成功连接 应该 添加新bufferevent事件
	struct bufferevent *bev;
	//struct bufferevent *bufferevent_socket_new(struct event_base *base,evutil_socket_t fd,enum bufferevent_options options);
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	
	//给bufferevent缓冲区设置回调
	//void bufferevent_setcb(struct bufferevent *bufev,bufferevent_data_cb readcb, bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void *cbarg);
	bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);

	//启用 bufferevent的 读缓冲 默认是disable的
	bufferevent_enable(bev, EV_READ);
}

int main(int argc, const char* argv[])
{
	//init server 绑定端口等信息
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(9876);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);

	//创建event_base
	struct event_base *base;
	base = event_base_new();//创建一个新的底座
	//创建套接字
	//绑定
	//接收连接请求
	struct evconnlistener* listener;//监听器
	listener = evconnlistener_new_bind(base, 
										cb_listener, 
										base, 
										LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
										36, 
										(struct sockaddr*)&serv, 
										sizeof(serv));
	//启动循环监听
	event_base_dispatch(base);
	//释放监听
	evconnlistener_free(listener);
	//释放底座
	event_base_free(base);
	return 0;
}