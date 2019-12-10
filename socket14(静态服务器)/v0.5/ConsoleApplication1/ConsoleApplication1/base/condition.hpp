#pragma once
#include "nocopyable.hpp"
#include "mutexLock.hpp"
#include <pthread.h>
class Condition : noncopyable
{
public:
	explicit Condition(MutexLock &_mutex) : mutex(_mutex)
	{
		pthread_cond_init(cond, NULL);
	}
	~Condition()
	{
		pthread_cond_destroy(cond);
	}
	void wait()
	{
	/*int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
		函数作用：
		1.	阻塞等待条件变量cond（参1）满足
		2.	释放已掌握的互斥锁（解锁互斥量）相当于pthread_mutex_unlock(&mutex);
		 1.2.两步为一个原子操作。
		3.	当被唤醒，pthread_cond_wait函数返回时，解除阻塞并重新申请获取互斥锁pthread_mutex_lock(&mutex);
	*/
		pthread_cond_wait(&cond, mutex.get());
	}
	void notify()
	{
		//利用条件变量的方式进行唤醒 随机唤醒至少一个
		pthread_cond_signal(&cond);
	}
	void notifyAll()
	{
		//利用广播的方式就行唤醒 全部唤醒阻塞在条件变量中的线程
		pthread_cond_broadcast(&cond);
	}
private:
	MutexLock &mutex;
	pthread_cond_t cond;
};