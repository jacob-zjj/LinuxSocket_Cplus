#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h> //--mkfifo
#include <fcntl.h>
#include <pthread.h>
#include <event2/event.h>
//#include <event2/event.h>
void sys_err(const char *str)
{
	perror(str);
	exit(1);
}

//void (*event_callback_fn)(evutil_socket_t fd, short what, void *arg);
//设置回调函数
void read_cb(evutil_socket_t fd, short what, void *arg)
{
	char buf[BUFSIZ]; //BUFSIZ 1024
	read(fd, buf, sizeof(buf));
	//管道只能单向流动 不同使用一个套接字 同时进行读和写操作
	printf("what = %s, read from write : %s\n", what & EV_READ ? "read yes" : "read no", buf);
	sleep(1);
	return;
}


int main(int argc, char *argv[])
{
	//打开fifo的读端
	int fd = open("testfifo", O_RDONLY | O_NONBLOCK);//设置非阻塞
	if (fd == -1)
	{
		sys_err("open error");
	}
	//创建event_base;
	struct event_base *base = event_base_new();

	//创建事件
	struct event *ev = NULL;

	//添加事件到event_base上 
	//struct event *event_new(struct event_base *base, evutil_socket_t fd, short what, event_callback_fn cb, void *arg);
	ev = event_new(base, fd, EV_READ | EV_PERSIST, read_cb, NULL);

	//添加事件到event_base上 相当于将事件插入到底座上
	//int event_add(struct event *ev, const struct timeval *tv);
	event_add(ev, NULL);

	//启动循环
	//int event_base_dispatch(struct event_base *base,);	
	event_base_dispatch(base);

	//销毁event_base;
	event_base_free(base);
}