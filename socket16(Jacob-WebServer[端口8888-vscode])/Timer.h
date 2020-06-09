#pragma once
#include "HttpData.h"
#include "base/noncopyable.h"
#include "base/MutexLock.h"
#include <unistd.h>
#include <memory>
#include <queue>
#include <deque>
class HttpData;
//时间节点 对HttpRequest的管理 过期管理
class TimerNode
{
public:
    TimerNode(std::shared_ptr<HttpData> requestData, int timeout);//传入HttpData 并传入超时时间
    ~TimerNode();
    TimerNode(TimerNode &tn);//拷贝构造函数
    void update(int timeout);
    bool isValid();
    void clearReq();
    void setDeleted(){deleted_ = true;};
    bool isDeleted() const { return deleted_; };
    size_t getExpTime() const { return expiredTime_; };

private:
	bool deleted_;
	size_t expiredTime_;//到期时间
	std::shared_ptr<HttpData> SPHttpData;//关联关系
};

struct TimeCmp
{
    bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) const
    {
        return a->getExpTime() > b->getExpTime();
    }
};

//时间节点管理 使用优先级队列来存储时间节点
class TimerManager
{
public:
    TimerManager();
    ~TimerManager();
    void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);
    void handleExpiredEvent();

private:
    typedef std::shared_ptr<TimerNode> SPTimerNode;
    std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimeCmp> timerNodeQueue;
};