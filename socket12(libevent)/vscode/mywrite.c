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

void write_cb(evutil_socket_t fd, short what, void *arg)
{
	char buf[] = "hello libevent";
	write(fd, buf, strlen(buf) + 1);
	sleep(1);
	return;
}
//void (*event_callback_fn)(evutil_socket_t fd, short what, void *arg);

int main(int argc, char *argv[])
{
	//创建 fifo
	unlink("testfifo");
	mkfifo("testfifo", 0644);
	//打开fifo的写端
	int fd = open("testfifo", O_WRONLY | O_NONBLOCK);//设置非阻塞
	if (fd == -1)
	{
		sys_err("open error");
	}
	//创建event_base;
	struct event_base *base = event_base_new();

	//创建事件 对象
	struct event *ev = NULL;

	//添加事件到event_base上 //EV_PERSIST持续写
	//struct event *event_new(struct event_base *base, evutil_socket_t fd, short what, event_callback_fn cb, void *arg);
	ev = event_new(base, fd, EV_WRITE | EV_PERSIST, write_cb, NULL);

	//添加事件到event_base上
	//int event_add(struct event *ev, const struct timeval *tv);
	event_add(ev, NULL);

	//启动循环
	//int event_base_dispatch(struct event_base *base,);	
	event_base_dispatch(base);

	//销毁event_base;
	event_base_free(base);
}