#include "Epoll.h"
#include "Util.h"
#include "base/Logging.h"
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <queue>
#include <deque>
#include <assert.h>

#include <arpa/inet.h>
#include <iostream>
using namespace std;

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;
//使用epoll_create1的优点是它允许你指定标志,我认为这些标志目前仅限于close-on-exec(因此在执行另一个进程时文件描述符会自动关闭).
/*
int epoll_create1(int flags);
	功能：创建一个多路复用的实例 参数：flags
	flags：
		0:	如果这个参数是0，这个函数等价于epoll_create（0）
		EPOLL_CLOEXEC：	这是这个参数唯一的有效值，如果这个参数设置为这个。那么当进程替换映像的时候会关闭这个文件描述符，
						这样新的映像中就无法对这个文件描述符操作，适用于多进程编程+映像替换的环境里
	返回值：
		success：返回一个非0 的未使用过的最小的文件描述符
		error：-1 errno被设置
*/
Epoll::Epoll() 
:	epollFd_(epoll_create1(EPOLL_CLOEXEC)), 
	events_(EVENTSNUM)
{
	//检查epollfd是否创建成功
	assert(epollFd_ > 0);
}

Epoll::~Epoll(){}

//添加新的描述符
void Epoll::epoll_add(SP_Channel request, int timeout)
{
    int fd = request->getFd();
	//设置超时时间
	if (timeout > 0)
	{
		add_timer(request, timeout);
		fd2http_[fd] = request->getHolder();
	}
	struct epoll_event event;
	event.data.fd = fd;
	event.events = request->getEvents();

	request->EqualAndUpdateLasteVents();

	fd2chan_[fd] = request;
	//fd为通信套接字 epollFd_为新创建的epoll类型的套接字 需要与fd相关联 然后挂上红黑树
	if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		perror("epoll_add eror");
		fd2chan_[fd].reset();
	}
}

//修改描述符
void Epoll::epoll_mod(SP_Channel request, int timeout)
{
    if (timeout > 0)
	{
		add_timer(request, timeout);
	}
	int fd = request->getFd();
	//判断是否和上次事件相同 如果不相同才新建事件 把事件加入到红黑树上的节点中
	if (!request->EqualAndUpdateLasteVents())
	{
		struct epoll_event event;
		event.data.fd = fd;
		event.events = request->getEvents();
		if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0)
		{
			perror("epoll_mod error");
			fd2chan_[fd].reset();
		}
	}
}

//从epoll中删除描述符
void Epoll::epoll_del(SP_Channel request)
{
    int fd = request->getFd();
	struct epoll_event event;
	event.data.fd = fd;
	event.events = request->getLastEvents();
	if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0)
	{
		perror("epoll_del error");
	}
	fd2chan_[fd].reset();
	fd2http_[fd].reset();
}

//返回活跃事件集合
std::vector<SP_Channel> Epoll::poll()
{
    while(true)
    {
        int event_count = epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
		if (event_count < 0)
		{
			perror("epoll wait error：");
		}
		std::vector<SP_Channel> req_data = getEventRequest(event_count);
		if (req_data.size() > 0)
		{
			return req_data;
		}
    }
}

//处理过期事件
void Epoll::handleExpired()
{
    timerManager_.handleExpiredEvent();
}

//分发处理函数
std::vector<SP_Channel> Epoll::getEventRequest(int events_num)
{
    std::vector<SP_Channel> req_data;
	for (int i = 0; i < events_num; ++i)
	{
		//获取有事件产生的描述符
		int fd = events_[i].data.fd;
		SP_Channel cur_req = fd2chan_[fd];
		if (cur_req)
		{
			cur_req->setRevents(events_[i].events);
			cur_req->setEvents(0);
			req_data.push_back(cur_req);
		}
		else
		{
			LOG << "SP cur_req is invalid";
		}
	}
	return req_data;
}

//加入超时时间
void Epoll::add_timer(SP_Channel request_data, int timeout)
{
    shared_ptr<HttpData> t = request_data->getHolder();
	if (t)
	{
		timerManager_.addTimer(t, timeout);
	}
	else
	{
		LOG << "timer add fail";
	}
}