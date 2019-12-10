#include "threadpool.h"
threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
	threadpool_t *pool;
	int i;
	do{
		//MAX_THREADS = 1024;	而实际传的thread_count大小为4;		MAX_QUEUE = 65535;
		if (thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE)
		{
			return NULL;
		}
		//线程池创建错误
		if ((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL)
		{
			break;
		}
		/*初始化线程池*/
		pool->thread_count = 0;
		//线程池队列大小
		pool->queue_size = queue_size;
		//让队列的头部和尾部都指向0这个位置
		pool->head = pool->tail = pool->count = 0;
		pool->shutdown = pool->started = 0;
		//为线程和任务队列分配空间
		pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
		//初始化线程池任务队列 该队列大小根据传入的大小进行分配 实际传入的queue_size大小为65535 即为 usign int 类型的大小
		pool->queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_size);
		//初始化 mutex 和条件变量 pthread_cond_t notify
		/*
		动态初始化：由于该pool->lock没有定义为全局变量 且 没有使用static关键字 因此采用 动态初始化方式:	pthread_mutex_init(&mutex, NULL)
		静态初始化：如果互斥锁是静态分布的 则可直接使用宏初始化 pthead_mutex_t muetx = PTHREAD_MUTEX_INITIALIZER;
		*/
		if ((pthread_mutex_init(&(pool->lock), NULL) != 0) || (pthread_cond_init(&(pool->notify), NULL) != 0) 
			|| (pool->threads == NULL) || (pool->queue == NULL))
		{
			break;
		}
		//启动线程 线程数量控制在传入的thread_count =  4 的范围内
		for (i = 0; i < thread_count; i ++)
		{
			//线程池线程创建回调函数：threadpool_thread线程产生函数
			/*int pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg);*/
			if (pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool) != 0)
			{
				//threadpool_destory释放线程池 如果有一个线程没有没有创建成功则把所有线程池全部销毁和释放
				threadpool_destroy(pool, 0);
				return NULL;
			}
			//线程创建成功 后执行
			pool->thread_count++;//线程池中 工作线程线程数++
			pool->started++;//执行线程数++ 活跃线程数加1
		}
		return pool;
	} while (false);
	//能够执行这一步说明前面必然出现了错误 因此要对已经创建的线程池进行释放
	if (pool != NULL)
	{
		threadpool_free(pool);
	}
	return NULL;
}

//向线程池中的任务队列中添加任务 并将队列的tail指针重新指向尾部 将队列中现有未处理的任务数count + 1
int threadpool_add(threadpool_t *pool, void(*function)(void *), void *argument, int flags)
{
	//加入线程池
	int err = 0;
	int next;
	if (pool == NULL || function == NULL)
	{
		return THREADPOOL_INVALID;//const int THREADPOOL_INVALID = -1;
	}
	if (pthread_mutex_lock(&(pool->lock)) != 0)
	{
		return THREADPOOL_LOCK_FAILURE;
	}
	next = (pool->tail + 1) % pool->queue_size;
	do 
	{
		//判断线程池队列是否已满
		if (pool->count == pool->queue_size)
		{
			err = THREADPOOL_QUEUE_FULL;
			break;
		}
		//判断线程池是否关闭
		if (pool->shutdown)
		{
			err = THREADPOOL_SHUTDOWN;
			break;
		}
		//向线程池中加入队列 并且将其对应回调函数 和参数一并进行传入
		pool->queue[pool->tail].function = function;
		pool->queue[pool->tail].argument = argument;
		pool->tail = next;
		pool->count += 1;
		//唤醒至少一个阻塞在条件变量上的线程
		if (pthread_cond_signal(&(pool->notify)) != 0)
		{
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}
	} while (false);
	//释放锁
	if (pthread_mutex_unlock(&pool->lock) != 0)
	{
		err = THREADPOOL_LOCK_FAILURE;
	}
	return err;
}

int threadpool_destroy(threadpool_t *pool, int flags)
{
	printf("Thread pool destory !\n");
	int i, err = 0;
	if (pool == NULL)
	{
		return THREADPOOL_INVALID;//THREADPOOL_INVALID = -1
	}
	//成功返回0 否则返回一个错误码
	if (pthread_mutex_lock(&(pool->lock)) != 0)
	{
		return THREADPOOL_LOCK_FAILURE;
	}
	do 
	{
		//已经关闭的
		if (pool->shutdown)
		{
			err = THREADPOOL_SHUTDOWN;
			break;
		}
		//根据flag设定关闭方式为优雅还是非优雅 THREADPOOL_GRACEFUL = 1
		//immediate_shutdown = 1, graceful_shutdown = 2 &表示：两边同时为1结果才为1
		pool->shutdown = (flags & THREADPOOL_GRACEFUL) ? graceful_shutdown : immediate_shutdown;
		//唤醒所有线程
		if ((pthread_cond_broadcast(&(pool->notify)) != 0) || (pthread_mutex_unlock(&(pool->lock)) != 0))
		{
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}
		//回收所用工作线程
		for (i = 0; i < pool->thread_count; ++i)
		{
			//extern int pthread_join __P ((pthread_t __th, void **__thread_return));
			//第一个参数为被等待的线程标识符，第二个参数为一个用户定义的指针，它可以用来存储被等待线程的返回值。
			if (pthread_join(pool->threads[i], NULL) != 0)
			{
				err = THREADPOOL_THREAD_FAILURE;//const int THREADPOOL_THREAD_FAILURE = -5;
			}
		}
	} while (false);
	//当所有步骤正确我们开始释放线程池内存
	if (!err)//0和非0 只有!0 = 1(true)
	{
		threadpool_free(pool);
	}
	return err;
}

int threadpool_free(threadpool_t *pool)
{
	if (pool == NULL || pool->started > 0)
	{
		return -1;
	}
	//判断是否已经被回收
	if (pool->threads)
	{
		free(pool->threads);
		free(pool->queue);
		//我们在创建线程池的时候在内部加入了锁机制 因此我们在操作线程池的时候应该先对线程池进行上锁操作
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));//释放锁
		pthread_cond_destroy(&(pool->notify));//释放线程条件
	}
	free(pool);
	return 0;
}

//线程产生函数 线程池中产生线程时所调用的回调函数
static void *threadpool_thread(void *threadpool)
{
	/*任务队列的添加和取操作 都需要加锁 并配合条件变量 跨越了多个线程*/
	threadpool_t *pool = (threadpool_t *)threadpool;
	threadpool_task_t task;
	for (;;)
	{
		//使用lock来作为条件变量等待 --------- 加锁。可理解为将mutex--（或-1）
		pthread_mutex_lock(&(pool->lock));
		//等待条件变量，检查虚假的唤醒。当从pthread_cond_wait()返回时，我们拥有锁
		while ((pool->count == 0) && (!pool->shutdown))
		{
			/*阻塞等待一个条件变量
				int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
			函数作用：
				1.	阻塞等待条件变量cond（参1）满足
				2.	释放已掌握的互斥锁（解锁互斥量）相当于pthread_mutex_unlock(&mutex);
			1.2.两步为一个原子操作。
				3.	当被唤醒，pthread_cond_wait函数返回时，解除阻塞并重新申请获取互斥锁pthread_mutex_lock(&mutex);*/
			pthread_cond_wait(&(pool->notify), &(pool->lock)); //函数返回即可跳出循环
		}
		//immediate_shutdown = 1  graceful_shutdown = 2
		if ((pool->shutdown == immediate_shutdown) ||
			((pool->shutdown == graceful_shutdown) &&
			(pool->count == 0)))
		{
			break;
		}
		/*执行任务 从当前head位置取出任务开始执行*/ 
		task.function = pool->queue[pool->head].function;//队列的头 的任务函数
		task.argument = pool->queue[pool->head].argument;
		pool->head = (pool->head + 1) % pool->queue_size;//将队列的头 重新指向下一个节点 
		//线程中等待执行任务数-1
		pool->count -= 1;

		//解锁。可理解为将mutex ++（或+1）
		pthread_mutex_unlock(&(pool->lock));
		//线程开始工作
		(*(task.function))(task.argument);
	}
	//活跃线程数量减少 1
	--pool->started;
	pthread_mutex_unlock(&(pool->lock));//线程解锁函数
	pthread_exit(NULL);//线程退出函数 函数返回 线程不一定退出 因此必须使用线程退出函数 并且对调用该线程的其他线程不影响
	return(NULL);
}