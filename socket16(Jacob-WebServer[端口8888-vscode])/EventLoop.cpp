#include "EventLoop.h"
#include "base/Logging.h"
#include "Util.h"
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <iostream>
using namespace std;

//加上__thread使用变量在每一个线程中都有一个变量实体
__thread EventLoop* t_loopInThisThread = 0;
int createEventfd()
{
	int evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evfd < 0)
	{
		LOG << "Failed in eventfd";
		abort();
	}
	return evfd;
}

EventLoop::EventLoop()
:  looping_(false),
    poller_(new Epoll()),
    wakeupFd_(createEventfd()),
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    pwakeupChannel_(new Channel(this, wakeupFd_))
{  
    if(t_loopInThisThread)
    {
    }
    else
	{
		t_loopInThisThread = this;
	}
	pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
	pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
	pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
	poller_->epoll_add(pwakeupChannel_, 0);
}

EventLoop::~EventLoop()
{
	close(wakeupFd_);
	t_loopInThisThread = NULL;
}

void EventLoop::handleConn()
{
	updatePoller(pwakeupChannel_, 0);
}

void EventLoop::handleRead()
{
   uint64_t one = 1;
	ssize_t n = readn(wakeupFd_, &one, sizeof one);
	if (n != sizeof one)
	{
		LOG << "EventLop::handleRead() reads " << n << "bytes instead of 8";
	}
	//设置事件为边沿触发模式
	pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
	ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
	if (n != sizeof one)
	{
		LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
	}
}

void EventLoop::runInLoop(Functor&& cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor&& cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
	//如果不是在当前Loop线程循环 或者callingpendingFunctors_为ture的时候开始写数据
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

//进入循环
void EventLoop::loop()
{
	assert(!looping_);
	assert(isInLoopThread());
	looping_ = true;
	quit_ = false;
	std::vector<SP_Channel> ret;
	while (!quit_)
	{
		ret.clear();
		ret = poller_->poll();
		eventHandling_ = true;
		for (auto &it : ret)
		{
			it->handleEvents();
		}
		eventHandling_ = false;
		doPendingFunctions();
		poller_->handleExpired();
	}
	looping_ = false;
}

void EventLoop::doPendingFunctions()
{
    std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	{
		MutexLockGuard lock(mutex_);
        //交换 回调函数集合
		functors.swap(pendingFunctors_);
	}
	for (size_t i = 0; i < functors.size(); ++i)
	{
        //进行回调
		functors[i]();
	}
	callingPendingFunctors_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
	if (!isInLoopThread())
	{
		wakeup();
	}
}