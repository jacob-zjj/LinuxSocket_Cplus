#include "Server.h"
#include "base/Logging.h"
#include "Util.h"
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
using namespace std;

Server::Server(EventLoop* loop, int threadNum, int port)
:   loop_(loop),
    threadNum_(threadNum),
    eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
    started_(false),
    acceptChannel_(new Channel(loop_)),
    port_(port),
    listenFd_(socket_bind_listen(port_))
{
    acceptChannel_->setFd(listenFd_);
    //处理信号管道 
    handle_for_sigpipe();
    //设置非阻塞监听
    if(setSocketNonBlocking(listenFd_) < 0)
    {
        perror("set socket non block failed");
        abort();
    }
}

void Server::start()
{
    //先开启EventLoop线程池
    eventLoopThreadPool_->start();
    //acceptChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    acceptChannel_->setReadHandler(bind(&Server::handNewConn, this));
    acceptChannel_->setConnHandler(bind(&Server::handThisConn, this));
    loop_->addToPoller(acceptChannel_, 0);
    started_ = true;
}

void Server::handNewConn()
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    //如果这里的accept的监听套接字设置为阻塞的话 存在一个客户端断掉或者单方面发送RST那么程序将会阻塞在这里直到这个客户端重新连接
    //如果将listenFd_设置为非阻塞类型  那么每次进行接收客户端连接就会直接进行返回 不会阻塞在这里 如果大量的客户端同时连接也可以按着排队的思想统一进行处理
    while((accept_fd = accept(listenFd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
    {
        EventLoop *loop = eventLoopThreadPool_->getNextLoop();
        LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port);
        //限制服务器的最大并发连接数
        if(accept_fd >= MAXFDS)
        {
            close(accept_fd);
            continue;
        }
        //设置为非阻塞模式
        if(setSocketNonBlocking(accept_fd) < 0)
        {
            LOG << "Set non block failed!";
            return;
        }
        //设置网络不适用Nagle算法
        setSocketNodelay(accept_fd);
		shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
        req_info->getChannel()->setHolder(req_info);
        //调用HttpData类中的newEvent方法 创建新的事件 将事件和过期时间传入epoll类创建新的epoitem挂上epollfd红黑树
        //然后放入loop循环 因为我们使用one loop per thread 因此这里要判断是否为当前线程
        loop->queueInLoop(std::bind(&HttpData::newEvent, req_info));
    }
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}