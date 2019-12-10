#include "epoll.h"
#include <sys/epoll.h>
#include <errno.h>
#include "threadpool.h"
struct epoll_event* events;
//以下是对epoll模型的四个函数的封装 分别为 epoll_create epoll_ctl epoll_wait
int epoll_init()
{
	//内核创建指定大小的红黑树 内核同时会创建事件的双向链表
	int epoll_fd = epoll_create(LISTENQ + 1);
	/*epoll创建监听红黑树-成功返回非负 文件描述符 错误返回 -1
	#include <sys/epoll.h>
	int epoll_create(int size);*/
	if (epoll_fd == -1)
	{
		return -1;
	}
	//创建事件数组
	events = new epoll_event[MAXEVENTS];//5000
	return epoll_fd;
}
//注册新描述符
int epoll_add(int epoll_fd, int fd, void *request, __uint32_t events)
{
	struct epoll_event event;
	event.data.ptr = request;
	event.events = events;
	//int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		perror("epoll_add error");
		return -1;
	}
	return 0;
}
//修改描述符状态
int epoll_mod(int epoll_fd, int fd, void *request, __uint32_t events)
{
	struct epoll_event event;
	event.data.ptr = request;
	event.events = events;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0)
	{
		perror("epoll_add error");
		return -1;
	}
	return 0;
}
//从epoll中删除描述符
int epoll_del(int epoll_fd, int fd, void *request, __uint32_t events)
{
	struct epoll_event event;
	event.data.ptr = request;
	event.events = events;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0)
	{
		perror("epoll_add error");
		return -1;
	}
	return 0;
}
//返回活跃事件数
int my_epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout)
{
	int ret_count = epoll_wait(epoll_fd, events, max_events, timeout);
	if (ret_count < 0)
	{
		perror("epoll wait error");
	}
	return ret_count;
}