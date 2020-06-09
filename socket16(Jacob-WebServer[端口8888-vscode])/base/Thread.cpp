#include "Thread.h"
//这里需要实现CurrentThread.h中的void cacheTid();函数 缓存线程id
#include "CurrentThread.h"
#include <memory>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>
using namespace std;

//该命名空间是来自 CurrentThread.h文件中 关于内部变量要进行声明
namespace CurrentThread
{
    __thread int t_cachedTid = 0;
	__thread char t_tidString[32];
	__thread int t_tidStringLength = 6;
	__thread const char* t_threadName = "default";
}
//返回当前线程的id
pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}
//这个函数其实就是获取当前线程号 然后将tidString和线程号合并 更新线程号+tidString的长度t_tidStringLength
void CurrentThread::cacheTid()
{
    if(t_cachedTid == 0)
    {
        t_cachedTid = gettid();
        //将t_tidString 和 t_cachedTid 连接起来的总长度 --> t_tidStringLength
		t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d", t_cachedTid);
    }
}
//在线程中保留name 和 tid这些数据
struct ThreadData
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;//函数
    string name_;
    pid_t* tid_;//线程id
    CountDownLatch* latch_;//计数
    ThreadData(const ThreadFunc& func, const string& name, pid_t *tid, CountDownLatch *latch)
    :   func_(func),
        name_(name),
        tid_(tid),
        latch_(latch)
    {}
    void runInThread()
    {
        *tid_ = CurrentThread::tid();
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;
        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        //PR_SET_NAME :把参数arg2作为调用进程的经常名字。
        prctl(PR_SET_NAME, CurrentThread::t_threadName);
        func_();
        CurrentThread::t_threadName = "finished";
    }
};

//开启线程
void* startThread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

/*
bool started_;
bool joined_;
pthread_t pthreadId_;
pid_t tid_;
ThreadFunc func_;
std::string name_;
CountDownLatch latch_;
*/
//构造函数
Thread::Thread(const ThreadFunc& func, const string& n)
 :  started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(func),
    name_(n),
    latch_(1)
{
    setDefaultName();
}
//析构函数
Thread::~Thread()
{
    if(started_ && !joined_)
    {
        //线程分离 等到回收
        pthread_detach(pthreadId_);
    }
}
//设置默认名字
void Thread::setDefaultName()
{
    if(name_.empty())
    {
        //设置默认名字 Thread
        char buf[32];
        snprintf(buf, sizeof buf, "Thread");
        name_ = buf;
    }
}
/*
assert的作用是先计算表达式 expression ，如果其值为假（即为0），那么它先向stderr打印一条出错信息，
然后通过调用 abort 来终止程序运行。
*/
void Thread::start()
{
    //如果started_为真 说明程序已经启动直接终止
    assert(!started_);
    started_ = true;
    //ThreadData(const ThreadFunc& func, const string& name, pid_t *tid, CountDownLatch *latch)
    ThreadData* data  = new ThreadData(func_, name_, &tid_, &latch_);
    if(pthread_create(&pthreadId_, NULL, &startThread, data))
    {
        started_ = false;
        delete data;
    }
    else
    {
        latch_.wait();
        assert(tid_ > 0);
    }
}
int Thread::join()
{
    //线程已经启动 并且joined为false才能启动join函数就行回收
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}