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
    if(!loops_.empty())
    {
        loop = loops_[next_];
        next_ = (next_ + 1) % numThreads_;
    }
    return loop;
}