#include "threadpool.h"
//将类中的属性初始化
//初始化 mutex 和条件变量 pthread_cond_t notify
/*
动态初始化：由于该pool->lock没有定义为全局变量 且 没有使用static关键字 因此采用 动态初始化方式:	pthread_mutex_init(&mutex, NULL)
静态初始化：如果互斥锁是静态分布的 则可直接使用宏初始化 pthead_mutex_t muetx = PTHREAD_MUTEX_INITIALIZER;
*/
pthread_mutex_t ThreadPool::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::notify = PTHREAD_COND_INITIALIZER;
std::vector<pthread_t> ThreadPool::threads;
//线程池中的任务队列
std::vector<ThreadPoolTask> ThreadPool::queue;
int ThreadPool::thread_count = 0;//当前线程池中的线程数
int ThreadPool::queue_size = 0;
int ThreadPool::head = 0;
int ThreadPool::tail = 0;
int ThreadPool::count = 0;//表示当前队列中的任务数
int ThreadPool::shutdown = 0;
int ThreadPool::started = 0;

int ThreadPool::threadpool_create(int _thread_count, int _queue_size)
{
	bool err = false;
	do{
		//MAX_THREADS = 1024;	而实际传的thread_count大小为4;		MAX_QUEUE = 65535;
		if (_thread_count <= 0 || _thread_count > MAX_THREADS || _queue_size <= 0 || _queue_size > MAX_QUEUE)
		{
			_thread_count = 4;
			_queue_size = 1024;
		}
		/*初始化线程池 表示当前线程池中的线程数*/
		thread_count = 0;
		//线程池队列大小
		queue_size = _queue_size;
		//让队列的头部和尾部都指向0这个位置
		head = tail = count = 0;
		shutdown = started = 0;

		//由于设置为vector 因此可以使用resize方法
		threads.resize(_thread_count);
		queue.resize(_queue_size);
		
		//启动线程 线程数量控制在传入的thread_count =  4 的范围内
		for (int i = 0; i < _thread_count; ++i)
		{
			//线程池线程创建回调函数：threadpool_thread线程产生函数
			/*int pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg);*/
			if (pthread_create(&threads[i], NULL, threadpool_thread, (void*)(0)) != 0)
			{
				//threadpool_destory释放线程池 如果有一个线程没有没有创建成功则把所有线程池全部销毁和释放
				//threadpool_destroy(pool, 0);
				return -1;
			}
			//线程创建成功 后执行
			thread_count++;//线程池中 工作线程线程数++
			started++;//执行线程数++ 活跃线程数加1
		}
	} while (false);
	//能够执行这一步说明前面必然出现了错误 因此要对已经创建的线程池进行释放
	if (err)
	{
		return -1;
	}
	return 0;
}

void myHandler(std::shared_ptr<void> req)
{
	//使用智能指针时 涉及到向下转型 可以使用static_pointer_cast 和 dynamic_pointer_cast 但是dynamic_pointer_cast相对来说不是很安全
	std::shared_ptr<RequestData> request = std::static_pointer_cast<RequestData>(req);
	request->handleRequest();
}

//向线程池中的任务队列中添加任务 并将队列的tail指针重新指向尾部 将队列中现有未处理的任务数count + 1
int ThreadPool::threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun)
{
	//加入线程池
	int next, err = 0;
	if (pthread_mutex_lock(&lock) != 0)
	{
		return THREADPOOL_LOCK_FAILURE;
	}
	do 
	{
		next = (tail + 1) % queue_size;
		//判断线程池队列是否已满
		if (count == queue_size)
		{
			err = THREADPOOL_QUEUE_FULL;
			break;
		}
		//判断线程池是否关闭
		if (shutdown)
		{
			err = THREADPOOL_SHUTDOWN;
			break;
		}
		//并且在队列中注入回调函数和参数 
		queue[tail].fun = fun;
		queue[tail].args = args;
		tail = next;
		++count;

		//唤醒至少一个阻塞在条件变量上的线程
		if (pthread_cond_signal(&notify) != 0)
		{
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}
	} while (false);
	//释放锁
	if (pthread_mutex_unlock(&lock) != 0)
	{
		err = THREADPOOL_LOCK_FAILURE;
	}
	return err;
}

int ThreadPool::threadpool_destroy(ShutDownOption shutdown_option)
{
	printf("Thread pool destory !\n");
	int i, err = 0;

	//成功返回0 否则返回一个错误码
	if (pthread_mutex_lock(&lock) != 0)
	{
		return THREADPOOL_LOCK_FAILURE;
	}
	do 
	{
		//已经关闭的
		if (shutdown)
		{
			err = THREADPOOL_SHUTDOWN;
			break;
		}
		//根据flag设定关闭方式为优雅还是非优雅 THREADPOOL_GRACEFUL = 1
		//immediate_shutdown = 1, graceful_shutdown = 2 &表示：两边同时为1结果才为1
		//pool->shutdown = (flags & THREADPOOL_GRACEFUL) ? graceful_shutdown : immediate_shutdown;
		shutdown = shutdown_option;
		//唤醒所有线程
		if ((pthread_cond_broadcast(&notify) != 0) || (pthread_mutex_unlock(&lock) != 0))
		{
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}
		//回收所用工作线程
		for (i = 0; i < thread_count; ++i)
		{
			//extern int pthread_join __P ((pthread_t __th, void **__thread_return));
			//第一个参数为被等待的线程标识符，第二个参数为一个用户定义的指针，它可以用来存储被等待线程的返回值。
			if (pthread_join(threads[i], NULL) != 0)
			{
				err = THREADPOOL_THREAD_FAILURE;//const int THREADPOOL_THREAD_FAILURE = -5;
			}
		}
	} while (false);
	//当所有步骤正确我们开始释放线程池内存
	if (!err)//0和非0 只有!0 = 1(true)
	{
		threadpool_free();
	}
	return err;
}

int ThreadPool::threadpool_free()
{
	if (started > 0)
	{
		//表示还有活着的线程
		return -1;
	}
	pthread_mutex_lock(&lock);
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&notify);
	return 0;
	//if (pool == NULL || pool->started > 0)
	//{
	//	return -1;
	//}
	////判断是否已经被回收
	//if (pool->threads)
	//{
	//	free(pool->threads);
	//	free(pool->queue);
	//	//我们在创建线程池的时候在内部加入了锁机制 因此我们在操作线程池的时候应该先对线程池进行上锁操作
	//	pthread_mutex_lock(&(pool->lock));
	//	pthread_mutex_destroy(&(pool->lock));//释放锁
	//	pthread_cond_destroy(&(pool->notify));//释放线程条件
	//}
	//free(pool);
	//return 0;
}

//线程产生函数 线程池中产生线程时所调用的回调函数
void *ThreadPool::threadpool_thread(void *args)
{
	while (true)
	{
		ThreadPoolTask task;
		pthread_mutex_lock(&lock);
		while ((count == 0) && (!shutdown))
		{
			/*阻塞等待一个条件变量
			int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
			函数作用：
			1.	阻塞等待条件变量cond（参1）满足
			2.	释放已掌握的互斥锁（解锁互斥量）相当于pthread_mutex_unlock(&mutex);
			1.2.两步为一个原子操作。
			3.	当被唤醒，pthread_cond_wait函数返回时，解除阻塞并重新申请获取互斥锁pthread_mutex_lock(&mutex);*/
			pthread_cond_wait(&notify, &lock); //函数返回即可跳出循环
		}

		//immediate_shutdown = 1  graceful_shutdown = 2
		if ((shutdown == immediate_shutdown) ||
			((shutdown == graceful_shutdown) && (count == 0)))
		{
			break;
		}

		/*执行任务 从当前head位置取出任务开始执行*/
		task.fun = queue[head].fun;//队列的头 的任务函数
		task.args = queue[head].args;
		queue[head].fun = NULL;
		queue[head].args.reset();
		head = (head + 1) % queue_size;//将队列的头 重新指向下一个节点 //线程中等待执行任务数-1
		--count;/*处理完了一件任务后将队列中的任务数减一*/
		//解锁。可理解为将mutex ++（或+1）
		pthread_mutex_unlock(&lock);
		//线程开始工作 -- 换句话说线程变成空闲 任务开始执行
		(task.fun)(task.args);
	}
	--started;//退出一个线程
	pthread_mutex_unlock(&lock);//线程解锁函数 如果在循环中是break出来的 可能没有释放锁 因此在这里释放锁也是一种非常安全的做法
	printf("This threadpool thread finishs!\n");
	pthread_exit(NULL);//线程退出函数 函数返回 线程不一定退出 因此必须使用线程退出函数 并且对调用该线程的其他线程不影响
	return(NULL);
}