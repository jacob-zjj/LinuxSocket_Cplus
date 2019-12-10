#pragma once
#include "requestData.h"
#include "nocopyable.h"
#include "MutexLock.h"
#include <unistd.h>
#include <memory>
#include <deque>
#include <queue>

//首先声明这个类
class RequestData;
//定时器结构体
struct TimerNode 
{
	typedef std::shared_ptr<RequestData> SP_ReqData;
private:
	bool deleted;
	//size_t size type 即一种变量的大小 可以转换为int型
	size_t expired_time;//定时器的过期时间 其值为创建时间 + timeout的值
						//requestData *request_data;//将请求数据指针放置于该结构体中的作用是 判断 是否有请求数据-使用定时器
						/*使用智能指针*/
	//std::shared_ptr<requestData> request_data;
	SP_ReqData request_data;

public:
	/*在mytimer中加入智能指针*/
	//mytimer(requestData *_request_data, int timeout);
	TimerNode(SP_ReqData _request_data, int timeout);
	~TimerNode();
	void update(int timeout);
	bool isvalid();
	void clearReq();
	void setDeleted();
	bool isDeleted() const;
	size_t getExpTime() const;
};

//设置定时器比较类 重载括号操作符 如果a的定时器存活时间大于b则返回ture 否则返回false
struct timerCmp
{
	/*将指针统一改为智能指针*/
	bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) const
	{
		return a->getExpTime() > b->getExpTime();
	}
	//bool operator()(const mytimer *a, const mytimer *b) const;
};

class TimerManager
{
	typedef std::shared_ptr<RequestData> SP_ReqData;
	typedef std::shared_ptr<TimerNode> SP_TimerNode;
private:
	std::priority_queue<SP_TimerNode, std::deque<SP_TimerNode>, timerCmp> TimerNodeQueue;
	MutexLock lock;

public:
	TimerManager();
	~TimerManager();
	//加入时间有两种方式 一种是加入时间节点 另外一种是加入请求 并设置过期时间
	void addTimer(SP_ReqData request_data, int timeout);
	void addTimer(SP_TimerNode timer_node);
	void handle_expired_event();
};
