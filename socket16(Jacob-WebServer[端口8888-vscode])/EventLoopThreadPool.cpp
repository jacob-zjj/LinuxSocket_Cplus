#include "EventLoopThreadPool.h"
EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, int numThreads)
:   baseloop_(baseloop),
    started_(false),
    numThreads_(numThreads),
    next_(0)
{
    if(numThreads_ <= 0)
    {
        LOG << "numThreads_ <= 0";
        abort();
    }
}

void EventLoopThreadPool::start()
{
    baseloop_->assertInLoopThread();
    started_ = true;
    for(int i = 0; i < numThreads_; i++)
    {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    baseloop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseloop_;
    //检查当前是否开启了指定个数的eventLoop循环
    if(!loops_.empty())
    {
        loop = loops_[next_];
        //这里使用了Roubin Robin的方式（也就是轮询的方式进行了负载均衡 使得指定个数的线程都能够得到充分利用）
        //不会造成某一个线程过忙 另一个线程空闲的情况
        next_ = (next_ + 1) % numThreads_;
    }
    return loop;
}