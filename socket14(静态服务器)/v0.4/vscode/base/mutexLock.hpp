/*.hpp 文件表示 .cpp和.h文件都放在同一个文件中 表示为将定义和实现都放在同一个文件	这样编译时就可以不用再进行编译 直接生成.obj
文件进行调用即可*/
#pragma once
#include "nocopyable.hpp"
#include <pthread.h>
#include <cstdio>

//nocopyable为非拷贝 非赋值类 因此继承该类可以拥有这种属性
class MutexLock : noncopyable
{
public:
	MutexLock()
	{
		pthread_mutex_init(&mutex, NULL);
	}
	~MutexLock()
	{
		//在锁析构的时候进行加锁 是防止此时锁被使用
		pthread_mutex_lock(&mutex);
		pthread_mutex_destroy(&mutex);
	}
	void lock()
	{
		pthread_mutex_lock(&mutex);
	}
	void unlock()
	{
		pthread_mutex_unlock(&mutex);
	}
private:
	pthread_mutex_t mutex;
};

/*详细来看 RALL锁机制 其实可以理解为我们在调用函数时 其实是入栈 在函数中调用锁对象 锁对象也是在栈中
因此在函数调用结束时 牵涉到出栈 出栈就会释放资源 因此我们不用担心另外的线程来使用资源
使用RALL方法能够成功的将对象和资源进行绑定
*/

/*RAII的做法是使用一个对象，在其构造时获取对应的资源，在对象生命期内控制对资源的访问，
使之始终保持有效，最后在对象析构的时候，释放构造时获取的资源。*///管理资源
class MutexLockGuard : noncopyable
{
public:
	//避免在对象初始化时隐式调用该构造函数 只能通过显示调用 即不支持隐式类型转换
	/*在调用该构造函数时 如果不是类似A a(19)就会报错 如果使用 a = 19就相当于使用隐式类型转换*/
	explicit MutexLockGuard(MutexLock &_mutex):mutex(_mutex)
	{		mutex.lock();
	}
	~MutexLockGuard()
	{
		mutex.unlock();
	}
private:
	MutexLock &mutex;
};