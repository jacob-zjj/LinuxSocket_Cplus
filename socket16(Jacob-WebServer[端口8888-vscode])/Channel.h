/*
	1> 关注一个fd。一般关注的fd事件就是读事件和写事件。
	2> 为fd增加读事件。
	3> 为fd增加写事件。
	4> 为fd删除读事件。
	5> 为fd删除写事件。
	6> 对外提供事件处理接口。
*/
//channel中主要是对事件的管理
//channel可以通过EventLoop将自己关注的fd的事件添加、删除、更改到epoll.cpp
#pragma once
#include "Timer.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include <functional>

class EventLoop;
class HttpData;

class Channel
{
private:
	typedef std::function<void()> CallBack;
	EventLoop *loop_;
	int fd_;
	__uint32_t events_;
	__uint32_t revents_;
	__uint32_t lastEvents_;
	//方便找到上层持有该Channel的对象 使用weak_ptr 不会像shared_ptr的use_count()改变，
	//因此可以用来查看某个shared_ptr指向的对象
	/*
		当我们reset了shared_ptr之后，weak_ptr会自动“到期”（expired），不会产生悬空指针。
		同时，因为weak_ptr指向某个对象，并不会使该对象的shared_ptr的use_count()改变，
		因此weak_ptr用的比较多的场景就是查看某个shared_ptr指向的对象。
	*/
	std::weak_ptr<HttpData> holder_;

private:
    int parse_URL();
    int parse_Headers();
    int analysisRequest();
    //回调句柄
    CallBack readHandler_;
	CallBack writeHandler_;
	CallBack errorHandler_;
	CallBack connHandler_;

public:
    Channel(EventLoop* loop);
	Channel(EventLoop* loop, int fd);
	~Channel();
	int getFd();
	void setFd(int fd);
	void setHolder(std::shared_ptr<HttpData> holder)
	{
		holder_ = holder;
	}
    std::shared_ptr<HttpData> getHolder()
    {
        //通过weak_ptr的lock函数 返回其所指对象的shared_ptr智能指针
        std::shared_ptr<HttpData> ret(holder_.lock());
        return ret;
    }

    //设置回调函数句柄
    void setReadHandler(CallBack &&readHandler)
	{
		readHandler_ = readHandler;
	}
	void setWriteHandler(CallBack &&writeHandler)
	{
		writeHandler_ = writeHandler;
	}
	void setErrorHandler(CallBack &&errorHandler)
	{
		errorHandler_ = errorHandler;
	}
	void setConnHandler(CallBack &&connHandler)
	{
		connHandler_ = connHandler;
	}

    //进行事件事件处理
    void handleEvents()
    {
       events_ = 0;
		if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
		{
			events_ = 0;
			return;
		}
		if (revents_ & EPOLLERR)
		{
			if (errorHandler_)
			{
				errorHandler_();
			}
			events_ = 0;
			return;
		}
		if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
		{
			handleRead();
		}
		if (revents_ & EPOLLOUT)
		{
			handleWrite();
		}
		handleConn();
    }
    void handleRead();
    void handleWrite();

    //未在此处实现
	// void handleError(int fd, int err_num, std::string short_msg);
	
	void handleConn();

    void setRevents(__uint32_t ev)
	{
		revents_ = ev;
	}
	void setEvents(__uint32_t ev)
	{
		events_ = ev;
	}
	__uint32_t& getEvents()
	{
		return events_;
	}
    __uint32_t getLastEvents()
	{
		return lastEvents_;
	}
    bool EqualAndUpdateLasteVents()
	{
		bool ret = (lastEvents_ == events_);
		lastEvents_ = events_;
		return ret;
	}
};

typedef std::shared_ptr<Channel> SP_Channel;