#pragma once
/*类static关键字*/
#ifndef EVENTPOLL
#define EVENTPOLL
#include "requestData.h"
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>
//将Epoll方法封装成一个类
class Epoll 
{
private:
	static epoll_event *events;
	//将智能指针加入类型的变量 加入到hash表中
	static std::unordered_map<int, std::shared_ptr<requestData>> fd2req;
	static int epoll_fd;
	static const std::string PATH;

public:
	//9.2修改至此
	static int epoll_init(int maxevents, int listen_num);
	static int epoll_add(int fd, std::shared_ptr<requestData> request, __uint32_t events);
	static int epoll_mod(int fd, std::shared_ptr<requestData> request, __uint32_t events);
	static int epoll_del(int fd, __uint32_t events);
	static void my_epoll_wait(int listen_fd, int max_events, int timeout);
	static void acceptConnection(int listen_fd, int epoll_fd, const std::string path);
	//智能指针vector集合 智能指针的对象是requestData
	static std::vector<std::shared_ptr<requestData>> getEventsRequest(int listen_fd, int events_num, const std::string path);
};
#endif