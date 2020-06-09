#include "Timer.h"
#include <sys/time.h>
#include <unistd.h>
//要传入超时时间
TimerNode::TimerNode(std::shared_ptr<HttpData> requestData, int timeout) 
: 	deleted_(false),
	SPHttpData(requestData)
{
	struct timeval now;
	gettimeofday(&now, NULL);//获取当天时间
	//统一以毫秒来计算 和 存储 先将当前时间转换成毫秒 加上超时时间毫秒 得到 从系统启动到设置结束的时间
	expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}
TimerNode::~TimerNode()
{
    if (SPHttpData)
	{
		SPHttpData->handleClose();
	}
}
//拷贝构造函数
TimerNode::TimerNode(TimerNode &tn)
:   SPHttpData(tn.SPHttpData)
{}

//更新超时时间
void TimerNode::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);//获取当天时间
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

//查看该时间节点是否还有效 主要是通过过期时间来判断
bool TimerNode::isValid()
{
    struct timeval now;
	gettimeofday(&now, NULL);
	size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
	if (temp < expiredTime_)
	{
		return true;
	}
	else
	{
		this->setDeleted();
		return false;
	}
}
//清空RequestData对象
void TimerNode::clearReq()
{
    SPHttpData.reset();
    this->setDeleted();
}

TimerManager::TimerManager()
{}

TimerManager::~TimerManager()
{}

void TimerManager::addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout)
{
    SPTimerNode new_node(new TimerNode(SPHttpData, timeout));
    //优先级队列 小顶堆 过期时间近的越靠前
    timerNodeQueue.push(new_node);
    //SPHttpData与定时器时间节点相互连接 这里其实就是将HtttpData对象中的 TimerNode指针指向 new_node
    SPHttpData->linkTimer(new_node);
}

/* 处理逻辑是这样的~
因为(1) 优先队列不支持随机访问
    (2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构。
所以对于被置为deleted的时间节点，会延迟到它(1)超时 或 (2)它前面的节点都被删除时，它才会被删除。
一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。
这样做有两个好处：
    (1) 第一个好处是不需要遍历优先队列，省时。
    (2) 第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请RequestData节点了，这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间。
*/
void TimerManager::handleExpiredEvent()
{
	while (!timerNodeQueue.empty())
	{
		SPTimerNode ptimer_now = timerNodeQueue.top();
		//时间节点是否为删除状态
		if (ptimer_now -> isDeleted())
		{
			timerNodeQueue.pop();
		}
		//时间节点是否为有效状态
		else if (ptimer_now->isValid() == false)
		{
			timerNodeQueue.pop();
		}
		else
		{
			break;
		}
	}
}
