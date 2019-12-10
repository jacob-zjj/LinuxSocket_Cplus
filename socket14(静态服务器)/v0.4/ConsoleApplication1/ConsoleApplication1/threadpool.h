//线程池
#pragma once
#ifndef THREADPOOL
#define THREADPOOL
#include "requestData.h"
#include <pthread.h>
#include <functional>
#include <memory>
#include <vector>

const int THREADPOOL_INVALID = -1;
const int THREADPOOL_LOCK_FAILURE = -2;
const int THREADPOOL_QUEUE_FULL = -3;
const int THREADPOOL_SHUTDOWN = -4;
const int THREADPOOL_THREAD_FAILURE = -5;
const int THREADPOOL_GRACEFUL = 1;

const int MAX_THREADS = 1024;
const int MAX_QUEUE = 65535;

//关闭方式 为直接关闭 还是优雅关闭
typedef enum
{
	immediate_shutdown = 1,
	graceful_shutdown = 2
}ShutDownOption;

typedef struct ThreadPoolTask
{
	std::function<void(std::shared_ptr<void>)> fun;
	std::shared_ptr<void> args;
	//将这两个指针全部修改为智能指针
	//void(*function)(void*);
	//void *argument;
};
/**
*  @struct threadpool
*  @brief The threadpool struct
*
*  @var notify       Condition variable to notify worker threads.
*  @var threads      Array containing worker threads ID.
*  @var thread_count Number of threads
*  @var queue        Array containing the task queue.
*  @var queue_size   Size of the task queue.
*  @var head         Index of the first element.
*  @var tail         Index of the next element.
*  @var count        Number of pending tasks
*  @var shutdown     Flag indicating if the pool is shutting down
*  @var started      Number of started threads
*/
void myHandler(std::shared_ptr<void> req);
class ThreadPool
{
private:
	//pthread_mutex_t 类型，其本质是一个结构体。为简化理解，应用时可忽略其实现细节，简单当成整数看待。
	static pthread_mutex_t lock;
	//用于定义条件变量 pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	//线程阻塞的条件变量
	static pthread_cond_t notify;
	//线程指针 数组 
	//pthread_t *threads;修改为容器
	static std::vector<pthread_t> threads;
	//线程池任务队列 任务队列中包含回调函数及参数
	static std::vector<ThreadPoolTask> queue;
	static int thread_count;//线程池线程数
	static int queue_size;//队列大小
	static int head;//队列的头
	//tail指向尾节点的下一节点
	static int tail;//队列的尾部
	static int count;//此时队列的容量 线程任务队列的任务数量
	static int shutdown;
	static int started;

public:
	static int threadpool_create(int _thread_count, int _queue_size);
	static int threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun = myHandler);
	//默认使用优雅的方式销毁线程池
	static int threadpool_destroy(ShutDownOption shutdown_option = graceful_shutdown);
	static int threadpool_free();
	static void *threadpool_thread(void *args);
};
#endif
