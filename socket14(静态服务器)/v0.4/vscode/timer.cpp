#include "timer.h"
#include "epoll.h"
#include <unordered_map>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <deque>
#include <queue>
#include <iostream>
using namespace std;

//mytimer::mytimer(requestData *_request_data, int timeout):deleted(false), request_data(_request_data)
TimerNode::TimerNode(SP_ReqData _request_data, int timeout) : deleted(false), request_data(_request_data)
{
	cout << "TimeNode()" << endl;
	/*struct timeval
	{
	__time_t tv_sec;        //Seconds. 秒
	__suseconds_t tv_usec;  // Microseconds. 微秒
	};*/
	struct timeval now;
	// int gettimeofday(struct timeval *tv, struct timezone *tz);成功返回 0  失败返回 -1
	gettimeofday(&now, NULL);
	// 以毫秒计算
	expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

TimerNode::~TimerNode()
{
	cout << "~TimeNode()" << endl;//析构函数
	if (request_data)
	{
		//int Epoll::epoll_del(int fd, __uint32_t events)
		//我们已经在epoll类中定义了该函数参数，其中设置 __uint32_t events = (EPOLLIN | EPOLLET | EPOLLONESHOT) 默认参数
		Epoll::epoll_del(request_data->getFd());
		//cout << "request_data = " << request_data << endl;
		//delete request_data;
		//request_data = NULL;//避免野指针
	}
}
//更新定时器的过期时间
void TimerNode::update(int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	//以毫秒计算
	expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}
//判断定时器是否有效 统一使用毫秒计数
bool TimerNode::isvalid()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	//size_t其实就是一种unsigned int类型
	size_t temp = ((now.tv_sec * 1000) + (now.tv_usec / 1000));
	if (temp < expired_time)
	{
		return true;
	}
	else
	{
		this->setDeleted();
		return false;
	}

}
void TimerNode::clearReq()
{
	//request_data = NULL;
	request_data.reset();
	this->setDeleted();
}
void TimerNode::setDeleted()
{
	deleted = true;
}
bool TimerNode::isDeleted() const
{
	return deleted;
}
size_t TimerNode::getExpTime() const
{
	return expired_time;
}

//以下是TimerManager的实现
TimerManager::TimerManager()
{

}
TimerManager::~TimerManager()
{

}
//加入时间有两种方式 一种是加入时间节点 另外一种是加入请求 并设置过期时间
void TimerManager::addTimer(SP_ReqData request_data, int timeout)
{
	//新建一个时间节点 将时间节点加入到时间节点队列中 该时间节点带有request请求
	SP_TimerNode new_node(new TimerNode(request_data, timeout));
	{
		MutexLockGuard locker(lock);
		TimerNodeQueue.push(new_node);
	}
	//将请求类对象 和 时间节点对象进行关联
	request_data->linkTimer(new_node);
}

void TimerManager::addTimer(SP_TimerNode timer_node)
{}

/*处理逻辑
因为(1)优先队列不支持随机访问
(2)即使支持 随机删除某节点后破坏了堆的结构 需要重新更新堆结构
所以对于被置为deleted的时间节点 会延迟到它(1)超时或者(2)前面的节点都被删除 它才会删除
一个点被置为deleted 它最迟在TIMER_TIMER_OUT时间后被删除
这样做有两个好处
(1)第一个好处是不需要遍历优先级队列 省时
(2)第二个好处是给超时时间一个容忍时间 就是设定的超时时间是删除的下限（并不是一到超时时间就立即删除）
如果监听的请求在超时后的下一次请求中又出现了一次就不用重新申请requestData节点了，这样可以继续重复利用
前面的requestData 减少了一次delete和一次new的时间
*/

//处理过期事件 由于定时器优先级队列中为小顶堆，我们只需要判断最顶端是否过去即可，如果最顶端没有过期 那么说明所有定时器都不会过期
void TimerManager::handle_expired_event()
{
	MutexLockGuard locker(lock);
	//pthread_mutex_lock(&qlock);
	while (!TimerNodeQueue.empty())
	{
		SP_TimerNode ptimer_now = TimerNodeQueue.top();
		if (ptimer_now->isDeleted())
		{
			TimerNodeQueue.pop();
			//delete ptimer_now;有了智能指针就不需要再进行 delete释放堆资源了
		}
		else if (ptimer_now->isvalid() == false)
		{
			TimerNodeQueue.pop();
			//delete ptimer_now;
		}
		else
		{
			//如果没有过期的时间定时器 则 跳出循环
			break;
		}
	}
	//pthread_mutex_unlock(&qlock);
}