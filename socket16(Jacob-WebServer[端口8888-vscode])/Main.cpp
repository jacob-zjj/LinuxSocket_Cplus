//@Author Jacob with reference to https://github.com/linyacool/WebServer and linux 多线程服务端编程(陈硕)
//@Email 1213608068@qq.com
#include "EventLoop.h"
#include "Server.h"
//关于写日志的头文件
#include "base/Logging.h"
//这里主要是用来获取传入参数的个数和类型 启动的时候可以指定端口
#include <getopt.h>
#include <string>
#include <unistd.h>
//切换的目标路径
const char *PATH = "./dir";
/*getopt()函数的使用可以减少对主函数参数处理的繁琐过程
getopt()原型是int getopt( int argc, char *const argv[], const char *optstring ); 调用一次，返回一个选项。 
在命令行选项参数再也检查不到optstring中包含的选项时，返回－1，同时optind储存第一个不包含选项的命令行参数*/
int main(int argc, char *argv[])
{
    /*这里我们自己切换自己的工作路径 因为我们需要访问自己的文件夹中的内容*/
    int ret = chdir(PATH);
    if(ret != 0)
    {
        perror("chdir error!!!");
        exit(1);
    }
    int threadNum = 4;//默认情况下使用4线程 一般是根据自己的主机的线程数来调整
    int port = 8888;
    std::string logPath = "./log/WebServer.log";//日志文件的目录
    int opt;
    const char *str = "t:l:p:";
    //这里也不是关键点 这里我自己已经设置了端口 日志目录 线程数量 
    while((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
            case 't':
            {
                threadNum = atoi(optarg);
                break; 
            }
            case 'l':
            {
                logPath = optarg;
                if(logPath.size() < 2 || optarg[0] != '/')
                {
                    printf("logPath should start with \"/\"\n");
					/*abort()函数首先解除进程对SIGABRT信号的阻止，然后向调用进程发送该信号。
					abort()函数会导致进程的异常终止除非SIGABRT信号被捕捉并且信号处理句柄没有返回。*/
					abort();
                }
                break; 
            }
            case 'p':
            {
                port = atoi(optarg);
                break; 
            }
            default: break;
        }
    }
    //打印当前工作路径 日志文件地址 监听端口号（这里在阿里云服务器上我们新开一个端口）
    std::cout << "working dir: " << PATH << endl;
    std::cout << "The direction of logPath is: " << logPath << endl;
    std::cout << "The port of JacobWebserver is: " << port << endl;
    //先写日志功能的头文件
    Logger::setLogFileName(logPath);
    
    //STL库在多线程上的应用
    //也就要检查是否使用多线程
	#ifndef _PTHREADS
		LOG << "_PTHREADS is not defined !";
	#endif

    EventLoop mainLoop;
	Server myHTTPServer(&mainLoop, threadNum, port);
	myHTTPServer.start();
	mainLoop.loop();
    return 0;
}