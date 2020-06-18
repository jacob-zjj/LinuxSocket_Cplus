#include "epoll.h"
#include "threadpool.h"
#include "util.h"
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <deque>
#include <queue>

int TIMER_TIME_OUT = 500;

//extern std::priority_queue<std::shared_ptr<mytimer>, std::deque<std::shared_ptr<mytimer>>, timerCmp> myTimerQueue;

/*务必记住静态变量必须进行初始化*/
epoll_event *Epoll::events;
//MAXFDS = 1000表示最多放多少个请求
Epoll::SP_ReqData Epoll::fd2req[MAXFDS];
//std::unordered_map<int, std::shared_ptr<RequestData>> Epoll::fd2req;
int Epoll::epoll_fd = 0;
const std::string Epoll::PATH = "/home/jacob/dir";

TimerManager Epoll::timer_manager;

//以下是对epoll模型的四个函数的封装 分别为 epoll_create epoll_ctl epoll_wait
int Epoll::epoll_init(int maxevents, int listen_num)
{
	//内核创建指定大小的红黑树 内核同时会创建事件的双向链表
	epoll_fd = epoll_create(listen_num + 1);
	/*epoll创建监听红黑树-成功返回非负 文件描述符 错误返回 -1
	#include <sys/epoll.h>
	int epoll_create(int size);*/
	if (epoll_fd == -1)
	{
		return -1;
	}
	//events.reset(new epoll_event[maxevents], [](epoll_event *data){delete [] data;});
	//创建事件数组
	events = new epoll_event[maxevents];//5000
	return 0;
}

//注册新描述符
int Epoll::epoll_add(int fd, SP_ReqData request, __uint32_t events)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	fd2req[fd] = request;
	//int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		perror("epoll_add error(epoll)");
		return -1;
	}
	return 0;
}

//修改描述符状态
int Epoll::epoll_mod(int fd, SP_ReqData request, __uint32_t events)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	fd2req[fd] = request;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0)
	{
		perror("epoll_mod error");
		/*将shared_ptr指针类型的request 进行清空 由于这次的修改事件没有成功*/
		fd2req[fd].reset();
		return -1;
	}
	return 0;
}

//从epoll中删除描述符
int Epoll::epoll_del(int fd, __uint32_t events)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0)
	{
		perror("epoll_add error");
		return -1;
	}
	//c++11中特有的 auto类型
	/*auto fd_ite = fd2req.find(fd);
	if (fd_ite != fd2req.end())
	{
		fd2req.erase(fd_ite);
	}*/
	fd2req[fd].reset();
	return 0;
}

//返回活跃事件数
void Epoll::my_epoll_wait(int listen_fd, int max_events, int timeout)
{
	//timeout == -1表示epoll_wait一直阻塞 0表示直接返回，即使没有事件的情况下
	int event_count = epoll_wait(epoll_fd, events, max_events, timeout);
	if (event_count < 0)
	{
		perror("epoll wait error");
	}
	std::vector<SP_ReqData> req_data = getEventsRequest(listen_fd, event_count, PATH);
	if (req_data.size() > 0)
	{
		for (auto &req : req_data)
		{
			if (ThreadPool::threadpool_add(req) < 0)
			{
				//线程池满了或者关闭了等原因，抛弃本次监听到的请求
				break;
			}
		}
	}
	//及时处理过期事件 将定时器从小顶堆的优先级定时器队列中弹出 
	timer_manager.handle_expired_event();
}

#include <iostream>
#include <arpa/inet.h>
using namespace std;
void Epoll::acceptConnection(int listen_fd, int epoll_fd, const std::string path)
{
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	//socklen_t client_addr_len = 0; 修改这里的地址长度 在读取IP地址和端口时就不是0
	socklen_t client_addr_len = sizeof(client_addr);
	int accept_fd = 0;
	//char client_IP[1024];
	while ((accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
	{
		cout << inet_ntoa(client_addr.sin_addr) << endl;
		cout << ntohs(client_addr.sin_port) << endl;
		/*可以使用以下方式进行打印*/
		//printf("client ip: %s port: %d\n",
		//	inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
		//	/*将网络端口 转换成  本地网络端口*/
		//	ntohs(client_addr.sin_port)
		//);
		/*
		// TCP的保活机制默认是关闭的
		int optval = 0;
		socklen_t len_optval = 4;
		getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
		cout << "optval ==" << optval << endl;
		*/
		//限制服务器的最大并发连接数
		if (accept_fd >= MAXFDS)
		{
			close(accept_fd);
			continue;
		}
		//设置非阻塞模式 这个方法是来自 util.h中的
		int ret = setSockNonBlocking(accept_fd);
		if (ret < 0)
		{
			perror("Set non block failed");
			return;
		}
		//这里与以前版本的区别在于只是将此处的裸指针改为智能指针而已
		SP_ReqData req_info(new RequestData(epoll_fd, accept_fd, path));
		/*2019年9月11日09:50:32 补充说明 新建*/
		//requestData *req_info = new requestData(epoll_fd, accept_fd, path);

		//文件描述符可以读 边缘触发模式 保证一个socket链接在任一时刻只被一个线程处理
		//即使设置EPOLLET模式还是有可能一个套接字上的事件被多次触发 因此采用 EPOLLONESHOT的思想
		__uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
		//int Epoll::epoll_add(int fd, std::shared_ptr<requestData> request, __uint32_t events)
		Epoll::epoll_add(accept_fd, req_info, _epo_event);
		
		timer_manager.addTimer(req_info, TIMER_TIME_OUT);
		//新增时间信息 替换裸指针为智能指针
		//std::shared_ptr<mytimer> mtimer(new mytimer(req_info, TIMER_TIME_OUT));
		//mytimer *mtimer = new mytimer(req_info, TIMER_TIME_OUT);
		//req_info->addTimer(mtimer);
		//pthread_mutex_lock(&qlock);
		//MutexLockGuard lock;
		//myTimerQueue.push(mtimer);
		//pthread_mutex_unlock(&qlock);
	}
}

//分发处理函数
std::vector<std::shared_ptr<RequestData>> Epoll::getEventsRequest(int listen_fd, int events_num, const std::string path)
{
	std::vector<SP_ReqData> req_data;
	for (int i = 0; i < events_num; i++)
	{
		//获取有事件产生的描述符
		int fd = events[i].data.fd;

		//有事件发生的描述符为监听描述符
		if (fd == listen_fd)
		{
			acceptConnection(listen_fd, epoll_fd, path);
		}
		else if (fd < 3)
		{
			printf("fd < 3\n");
			break;
		}
		else
		{
			//排除错误事件
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
			{
				printf("error event\n");
				if (fd2req[fd])
				{
					//将定时器和事件请求进行分离
					fd2req[fd]->seperateTimer();
				}
				//清空该事件请求
				fd2req[fd].reset();
				/*auto fd_ite = fd2req.find(fd);
				if (fd_ite != fd2req.end())
				{
					fd2req.erase(fd_ite);
				}*/
				//printf("error event\n");
				//delete request;
				continue;
			}

			//将请求任务加入到线程池任务队列中
			//加入线程池之前将Timer和request分离 简单说就是删除时间
			SP_ReqData cur_req = fd2req[fd];
			//SP_ReqData cur_req(fd2req[fd]);

			//如果对应fd中有请求的话
			if (cur_req)
			{
				//判断事件是否可读
				if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI))
				{
					cur_req->enableRead();
				}
				//判断事件是否可写
				else 
				{
					cur_req->enableWrite();
				}
				cur_req->seperateTimer();
				req_data.push_back(cur_req);
				fd2req[fd].reset();
			}
			else
			{
				cout << "SP cur_req is invalid" << endl;
			}
			/*cur_req->seperateTimer();
			req_data.push_back(cur_req);
			auto fd_ite = fd2req.find(fd);
			if (fd_ite != fd2req.end())
			{
				fd2req.erase(fd_ite);
			}*/

			//int threadpool_add(threadpool_t *pool, void(*function)(void *), void *argument, int flags)
			/*这个回调函数即为处理请求数据的回调函数  myHandler -》 handleRequest*/
			//int rc = threadpool_add(tp, myHandler, events[i].data.ptr, 0);
		}
	}
	return req_data;
}
void Epoll::add_timer(SP_ReqData request_data, int timeout)
{
	timer_manager.addTimer(request_data, timeout);
}
