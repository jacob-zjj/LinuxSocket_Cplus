#pragma once
#include "Timer.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include <functional>

class EventLoop;
class HttpData;

//Channel通道 表示一个请求 或者是一个链接
class Channel
{
private:
	typedef std::function<void()> CallBack;
	EventLoop *loop_;
	int fd_;
	__uint32_t events_;
	__uint32_t revents_;
	__uint32_t lastEvents_;
	//方便找到上层持有该Channel的对象 使用weak_ptr 不会像shared_ptr的use_count()改变，因此可以用来查看某个shared_ptr指向的对象
	/*
	当我们reset了shared_ptr之后，weak_ptr会自动“到期”（expired），不会产生悬空指针。
	同时，因为weak_ptr指向某个对象，并不会使该对象的shared_ptr的use_count()改变，
	因此weak_ptr用的比较多的场景就是查看某个shared_ptr指向的对象。
	*/
	std::weak_ptr<HttpData> holder_;

private:
	//2019年10月23日22:30:11
	//未在此处实现
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
	//处理事件
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
	void handleError(int fd, int err_num, std::string short_msg);
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
	bool EqualAndUpdateLasteVents()
	{
		bool ret = (lastEvents_ == events_);
		lastEvents_ = events_;
		return ret;
	}
	__uint32_t getLastEvents()
	{
		return lastEvents_;
	}
};

typedef std::shared_ptr<Channel> SP_Channel;