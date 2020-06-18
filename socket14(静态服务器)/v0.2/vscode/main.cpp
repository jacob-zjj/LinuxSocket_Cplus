//自己封装的函数库
#include "requestData.h"
#include "epoll.h"
#include "threadpool.h"
#include "util.h"
//引用的系统库
#include <sys/epoll.h>
#include <queue>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;

//线程池中线程数量设置为4
const int THREADPOOL_THREAD_NUM = 4;
//线程池中的任务队列大小
const int QUEUE_SIZE = 65535;

const int PORT = 8888;
const int ASK_STATIC_FILE = 1;
const int ASK_IMAGE_STITCH = 2;

//通过输入路径改变进程的执行路径 从而达到访问指定文件的作用
const char *PATH = "/home/jacob/dir";
//const string PATH = "/home/jacob/dir";
//const string PATH = "/";

const int TIMER_TIME_OUT = 500;

//extern一般是使用在多文件之间需要共享某些代码时。
//extern pthread_mutex_t qlock; 因为使用了RALL锁机制 因此不用再单独创建锁

extern struct epoll_event* events;
void acceptConnection(int listen_fd, int epoll_fd, const string &path);

//优先级队列
extern priority_queue<mytimer*, deque<mytimer*>, timerCmp> myTimerQueue;

int socket_bind_listen(int port)
{
	//检查port的值 取正确区间范围
	if (port < 1024 || port > 65535)
	{
		return -1;
	}
	//创建socket(IPV4 + TCP)，返回监听套接字描述符
	int listen_fd = 0;
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return -1;
	}
	//建立端口复用操作 现在网络编程十分常用
	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		return -1;
	}
	//设置服务器IP和Port 以及监听描述符绑定
	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));//清空结构体内存
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);
	if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		return -1;
	}
	//开始监听 最大等待队列长度为 LISTENQ 且在epoll_create中已经创建了LISTENQ大小的红黑树
	//#define LISTENQ 1024 //  int listen(int sockfd, int backlog);
	if (listen(listen_fd, LISTENQ) == -1)/*成功返回0 失败返回-1*/
	{
		return -1;
	}
	//无效的监听描述符
	if (listen_fd == -1)
	{
		close(listen_fd);
		return -1;
	}
	return listen_fd;
}

void myHandler(void *args)
{
	requestData *req_data = (requestData*)args;
	req_data->handleRequest();
}

void acceptConnection(int listen_fd, int epoll_fd, const string &path)
{
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client_addr_len = 0;
	int accept_fd = 0;
	while ((accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
	{
		/*
		// TCP的保活机制默认是关闭的
		int optval = 0;
		socklen_t len_optval = 4;
		getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
		cout << "optval ==" << optval << endl;
		*/
		//设置非阻塞模式
		int ret = setSockNonBlocking(accept_fd);
		if (ret < 0)
		{
			perror("Set non block failed");
			return;
		}
		requestData *req_info = new requestData(epoll_fd, accept_fd, path);
		//文件描述符可以读 边缘触发模式 保证一个socket链接在任一时刻只被一个线程处理
		//即使设置EPOLLET模式还是有可能一个套接字上的事件被多次触发
		__uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
		epoll_add(epoll_fd, accept_fd, static_cast<void*>(req_info), _epo_event);
		//新增时间信息
		mytimer *mtimer = new mytimer(req_info, TIMER_TIME_OUT);
		req_info->addTimer(mtimer);
		//pthread_mutex_lock(&qlock);
		{
			MutexLockGuard();
		}
		myTimerQueue.push(mtimer);
		//pthread_mutex_unlock(&qlock);
	}
}
//分发处理函数
void handle_events(int epoll_fd, int listen_fd, struct epoll_event* events, int events_num, const string &path, threadpool_t* tp)
{
	for (int i = 0; i < events_num; i ++)
	{
		//获取有事件产生的描述符
		requestData* request = (requestData*)(events[i].data.ptr);
		int fd = request->getFd();
		//有事件发生的描述符为监听描述符
		if (fd == listen_fd)
		{
			acceptConnection(listen_fd, epoll_fd, path);
		}
		else
		{
			//排除错误事件
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
			{
				printf("error event\n");
				delete request;
				continue;
			}
			//将请求任务加入到线程中
			//加入线程池之前将Timer和request分离
			request->seperateTimer();
			//int threadpool_add(threadpool_t *pool, void(*function)(void *), void *argument, int flags)
			/*这个回调函数即为处理请求数据的回调函数  myHandler -》 handleRequest*/
			int rc = threadpool_add(tp, myHandler, events[i].data.ptr, 0);
		}
	}
}
/*处理逻辑
因为(1)优先队列不支持随机访问
(2)即使支持 随机删除某节点后破坏了堆的结构 需要重新更新堆结构
所以对于被置为deleted的时间节点 会延迟到它(1)超时或者(2)前面的节点都被删除 它才会删除
一个点被置为deleted 它最迟在TIMER_TIMER_OUT时间后被删除
这样做有两个好处
(1)第一个好处是不需要遍历优先级队列 省时
(2)第二个好处是给超时时间一个容忍时间 就是设定的超时时间是删除的下限（并不是一到超时时间就立即删除）
如果监听的请求在超时后的下一次请求中又出现了一次就不用重新申请requestData节点了，这样可以继续重复利用
前面的requestData 减少了一次delete和一次new的时间
*/
//处理过期事件 由于定时器优先级队列中为小顶堆，我们只需要判断最顶端是否过去即可，如果最顶端没有过期 那么说明所有定时器都不会过期
void handle_expired_event()
{
	MutexLockGuard();
	//pthread_mutex_lock(&qlock);
	while (!myTimerQueue.empty())
	{
		mytimer *ptimer_now = myTimerQueue.top();
		if (ptimer_now->isDeleted())
		{
			myTimerQueue.pop();
			delete ptimer_now;
		}
		else if (ptimer_now->isvalid() == false)
		{
			myTimerQueue.pop();
			delete ptimer_now;
		}
		else
		{
			break;
		}
	}
	//pthread_mutex_unlock(&qlock);
}
int main()
{
	//根据PATH指定的工作路径 改变进程的执行路径
	int ret = chdir(PATH);
	if (ret != 0)
	{
		perror("chdir error");
		exit(1);
	}
	//信号处理函数
	handle_for_sigpipe();
	//创建epoll红黑树模型 套接字
	int epoll_fd = epoll_init();
	if (epoll_fd < 0)
	{
		perror("epoll init failed");
		return 1;
	}
	//创建线程池
	threadpool_t *threadpool = threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE, 0);
	//创建监听套接字
	int listen_fd = socket_bind_listen(PORT);
	if (listen_fd < 0)
	{
		perror("socket bind failed");
		return 1;
	}
	//设置非阻塞监听模式
	if (setSockNonBlocking(listen_fd) < 0)
	{
		perror("set socket non block failed");
		return 1;
	}
	__uint32_t event = EPOLLIN | EPOLLET;
	requestData *req = new requestData();
	req->setFd(listen_fd);//设定套接字
	//将监听事件加入到epoll树 同时加入req请求
	epoll_add(epoll_fd, listen_fd, static_cast<void*>(req), event);
	while (true)
	{
		//int my_epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout)
		int events_num = my_epoll_wait(epoll_fd, events, MAXEVENTS, -1);
		if (events_num == 0)
		{
			continue;
		}
		printf("%d\n", events_num);
		//遍历events数组，根据监听种类及描述符类型分发操作
		handle_events(epoll_fd, listen_fd, events, events_num, PATH, threadpool);
		//处理
		handle_expired_event();
	}
	return 0;
}