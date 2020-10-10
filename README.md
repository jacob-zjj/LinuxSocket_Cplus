# Linux C++ socket网络编程
## 【高性能静态网络服务器 一步步搭建过程】
Reference chenshuo && GitHub-linyacool
## 编译环境：

	socket14(静态服务器)之前使用ubuntu18.04等环境均可

	socket14(静态服务器)使用ubuntu14.04 g++4.8 进行编译

	socket15和socket16为静态服务器最终版本 使用ubuntu14.04 g++4.8编译即可

## socket14/15/16 Introduction  

 该项目为c++11(智能指针)编写的Web静态高并发网络服务器，使用socket网络编程相关知识，使用状态机的方式解析了get、head请求，主要是获取目的静态资源在前端网页进行展示，可以理解为B/S模式，同时通过解析确认是否为长连接，编程中加入的了支持长连接的代码，在此基础同时支持管线化请求；最后再加入日志系统将服务器运行状态写入日志文件，记录服务器的运行状态。  

测试页：http://121.199.60.131/

## Technical points（项目中的技术点）
* 使用Epoll ET的IO多路复用技术，非阻塞IO，使用Reactor模式
* 使用多线程充分利用多核CPU，并使用线程池（线程池的大小根据不同主机CPU的核心数）避免线程频繁创建销毁的开销
* 使用基于小根堆（priority_queue）的定时器对每个请求设置过期时间 关闭超时请求
* 主线程只负责accept请求，并以Round Robin的方式分发给其它IO线程(兼计算线程)，锁的争用只会出现在主线程和某一特定线程中
* 使用eventfd实现了线程的异步唤醒
* 使用双缓冲区技术实现了简单的异步日志系统
* 为减少内存泄漏的可能，使用智能指针等RAII机制
* 使用状态机解析了HTTP请求,支持管线化
* 支持优雅关闭连接 先关闭写端再关闭读端

## 静态网络服务器编译环境
* OS: Ubuntu 14.04
* Complier: g++ 4.8	

## 开启服务器

	./WebServer [-t thread_numbers] [-p port] [-l log_file_path(should begin with '/')]

## 并发模型

并发模型为Reactor+非阻塞IO+线程池，新连接Round Robin分配

## 代码统计
使用ubuntu中的cloc工具可以对项目代码进行统计


## Others
可以采用Webbench来对项目进行压力测试

## 库结构  
socket01 - socket13是学习Linux网络编程的整个流程 
socket14可以在先学习socket13的基础上来学习
socket14中分为了6个版本，v0.6版本启动myserver时需要进入root权限
每个版本都较前一个版本有了更新 在socket13的基础上加入了线程池，定时器，请求任务队列，同时加入了RALL锁，智能指针，非拷贝赋值类；
将epoll 等用类进行封装

socket15 16主要是web静态高并发网络服务器




