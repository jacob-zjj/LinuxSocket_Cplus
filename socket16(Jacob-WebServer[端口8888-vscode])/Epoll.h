/*
1> EventModule在这里就是对 socket 的事件监听，socket下面就用 epollFd_ 代替了。
2> 为fd添加事件   对应 EPOLL_CTL_ADD
3> 为fd更改事件   对应 EPOLL_CTL_MOD
4> 为fd删除事件   对应 EPOLL_CTL_DEL
5> 对外提供获取当前关注的活动的fd及其事件。
*/
#pragma once
#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>

class Epoll
{
public:
    Epoll();
    ~Epoll();
    void epoll_add(SP_Channel request, int timeout);
	void epoll_mod(SP_Channel request, int timeout);
	void epoll_del(SP_Channel request);
    std::vector<std::shared_ptr<Channel>> poll();
	std::vector<std::shared_ptr<Channel>> getEventRequest(int events_num);
    void add_timer(SP_Channel request_data, int timeout);
    int getEpollFd()
    {
        return epollFd_;
    }
    void handleExpired();

private:
    static const int MAXFDS = 100000;
	//当前epoll套接字
    int epollFd_;
    //用于存储事件的vector容器 事件类型为epoll_event事件类型
	std::vector<epoll_event> events_;
	//存储通道
	std::shared_ptr<Channel> fd2chan_[MAXFDS];
	//存储请求数据
	std::shared_ptr<HttpData> fd2http_[MAXFDS];
	TimerManager timerManager_;
};