/*
1> 持有一个Epoll的对象。
2> Channel可以通过EventLoop将自己关注的fd的事件添加、删除、更改到 Epoll
3> 执行 Epoll的事件探测(epoll_wait)，获取活动的fd的Channel，调用Channel的事件处理函数。
*/
#pragma once
#include "base/Thread.h"
#include "Epoll.h"
#include "base/Logging.h"
#include "Channel.h"
#include "base/CurrentThread.h"
#include "Util.h"
#include <vector>
#include <memory>
#include <functional>
#include <iostream>
using namespace std;

//事件循环 单线程运行时不会阻塞的一种机制，也就是我们经常使用异步的原理。
class EventLoop
{
public:

    typedef std::function<void()> Functor;
	EventLoop();
	~EventLoop();
	void loop();
	void quit();
	void runInLoop(Functor&& cb);
	void queueInLoop(Functor&& cb);
	//获取当前线程id 可以得到是否为同一线程
	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    //判断线程是否在循环线程中 
    void assertInLoopThread()
	{
		//如果从别的线程调用该方法 isInLoopThread 将会返回false
		assert(isInLoopThread());
	}
    //关闭epoll事件
	void shutdown(shared_ptr<Channel> channel)
	{
		shutDownWR(channel->getFd());
	}
	//移除epoll事件
	void removeFromPoller(shared_ptr<Channel> channel)
	{
		poller_->epoll_del(channel);
	}
	//更新epoll事件
	void updatePoller(shared_ptr<Channel> channel, int timeout = 0)
	{
		poller_->epoll_mod(channel, timeout);
	}
	//添加epoll事件
	void addToPoller(shared_ptr<Channel> channel, int timeout = 0)
	{
		poller_->epoll_add(channel, timeout);
	}

private:
	//声明顺序 wakeupFd_ > pwakeupChannel_
	bool looping_;
	shared_ptr<Epoll> poller_;
	int wakeupFd_;
	bool quit_;
	bool eventHandling_;
	mutable MutexLock mutex_;
	//用来放回调函数的集合
	std::vector<Functor> pendingFunctors_;
	bool callingPendingFunctors_;
	const pid_t threadId_;
	shared_ptr<Channel> pwakeupChannel_;

	void wakeup();
	void handleRead();
	void doPendingFunctions();
	void handleConn();
};