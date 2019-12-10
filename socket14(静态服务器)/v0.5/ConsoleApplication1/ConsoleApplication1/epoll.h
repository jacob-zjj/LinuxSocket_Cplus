#pragma once
/*类static关键字*/
#ifndef EVENTPOLL
#define EVENTPOLL
#include "requestData.h"
#include "timer.h"
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>
//将Epoll方法封装成一个类
class Epoll 
{
public:
	//使用SP_ReqData 替换 std::shared_ptr<RequestData>
	typedef std::shared_ptr<RequestData> SP_ReqData;
private:
	static const int MAXFDS = 1000;
	static epoll_event *events;
	//将智能指针加入类型的变量 加入到hash表中 该hash表中的存储是 fd(套接字) --> 对应请求
	//static std::unordered_map<int, SP_ReqData> fd2req;
	/*用数组的方式来存储请求 shared_ptr<RequestData>类型的请求*/
	static SP_ReqData fd2req[MAXFDS];
	static int epoll_fd;
	static const std::string PATH;

	static TimerManager timer_manager;
public:
	//9.2修改至此
	static int epoll_init(int maxevents, int listen_num);
	static int epoll_add(int fd, SP_ReqData request, __uint32_t events);
	static int epoll_mod(int fd, SP_ReqData request, __uint32_t events);
	//将事件类型直接设置为边沿触发模式 和一次触发
	static int epoll_del(int fd, __uint32_t events = (EPOLLIN | EPOLLET | EPOLLONESHOT));
	static void my_epoll_wait(int listen_fd, int max_events, int timeout);
	static void acceptConnection(int listen_fd, int epoll_fd, const std::string path);
	//智能指针vector集合 智能指针的对象是requestData
	static std::vector<SP_ReqData> getEventsRequest(int listen_fd, int events_num, const std::string path);

	static void add_timer(SP_ReqData request_data, int timeout);
};
#endif