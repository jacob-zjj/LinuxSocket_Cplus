#include "EventLoopThread.h"
#include <functional>
EventLoopThread::EventLoopThread()
:   loop_(NULL), 
	exiting_(false), 
	thread_(bind(&EventLoopThread::threadFunc, this), "EventLoopThread"), 
	mutex_(), 
	cond_(mutex_)
{}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != NULL)
    {
        loop_->quit();
        thread_.join();//回收线程
    }
}

EventLoop* EventLoopThread::startLoop()
{
    //如果启动了就不需要再启动了
    assert(!thread_.started());
    thread_.start();
    {
        MutexLockGuard lock(mutex_);
        //一直等到线程开始运行 才能得到loop_ cond_会唤醒阻塞在此处的cond_.wait()
        while(loop_ == NULL)
        {
            cond_.wait();
        }
    }
    return loop_;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }
    loop.loop();
    //assert(exiting_);
    loop_ = NULL;
}