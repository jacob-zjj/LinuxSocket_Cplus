#include "CountDownLatch.h"
CountDownLatch::CountDownLatch(int count) : mutex_(), condition_(mutex_), count_(count)
{
}

void CountDownLatch::wait()
{
	MutexLockGuard lock(mutex_);
	while (count_ > 0)
	{
		condition_.wait();
	}
}

//计数结束 计数--
void CountDownLatch::countDown()
{
	MutexLockGuard lock(mutex_);
	--count_;
	if (count_ == 0)
	{
		condition_.notifyAll();
	}
}

//返回Count值
int CountDownLatch::getCount() const
{
	MutexLockGuard lock(mutex_);
	return count_;
}