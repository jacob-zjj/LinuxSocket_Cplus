#pragma once
#include "HttpData.h"
#include "base/noncopyable.h"
#include "base/MutexLock.h"
#include <unistd.h>
#include <memory>
#include <queue>
#include <deque>
class HttpData;
class TimerNode
{
public:
	TimerNode(std::shared_ptr<HttpData> requestData, int timeout);//要传入超时时间
	~TimerNode();
	TimerNode(TimerNode &tn);//拷贝构造函数
	void update(int timeout);
	bool isValid();
	void clearReq();
	void setDeleted() { deleted_ = true; };//将该时间节点设置为删除标志
	bool isDeleted() const { return deleted_; };//查看该时间节点是否删除
	size_t getExpTime() const { return expiredTime_; };//获取到期超时时间

private:
	bool deleted_;
	size_t expiredTime_;//到期时间
	std::shared_ptr<HttpData> SPHttpData;//关联关系
};

struct TimeCmp
{
	bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) const
	{
		//重载()操作符 比较当前调用者 和 传入时间的过期时间大小
		/*这里主要是一个回调函数 为了放入优先级队列中时所使用的函数*/
		return a->getExpTime() > b->getExpTime();
	}
};

//时间节点管理
class TimerManager
{
public:
	TimerManager();
	~TimerManager();
	void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);
	void handleExpiredEvent();

private:
	typedef std::shared_ptr<TimerNode> SPTimerNode;
	//优先级时间队列 其底层是用堆来实现的  具体大顶堆还是小顶堆 需要看后面的回调函数
	//只能通过top来访问堆首元素
	std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimeCmp> timerNodeQueue;
	/*深入理解优先级队列 priority_queue
	1、priority_queue<int,vector<int>,greater<int>>q;
		greater表示数字小的优先级高 相当于cmp函数的大于 >
	2、priority_queue<int,vector<int>,less<int>>q;
		less表示数字大的优先级高  相当于cmp函数中的小于 <
	3、priority_queue<数据类型,容器+数据类型vector<fruit>,回调函数cmp>q;
		cmp中是小于符号表示 越大优先级越高 反之越小优先级越高
	4、该项目中的priority_queue内的cmp是表示的比较是 > ，表示值越小的优先级越高
	*/
};
