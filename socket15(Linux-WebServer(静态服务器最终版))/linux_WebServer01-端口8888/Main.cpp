//@Author Jacob with reference to https://github.com/linyacool/WebServer and linux 多线程服务端编程(陈硕)
//@Email 1213608068@qq.com
#include "EventLoop.h"
#include "Server.h"
//导入一个日志.h文件即可
#include "base/Logging.h"
#include <getopt.h>
#include <string>
#include <unistd.h>

//切换目标路径
const char *PATH = "./dir";

/*getopt()函数的使用可以减少对主函数参数处理的繁琐过程
getopt()原型是int getopt( int argc, char *const argv[], const char *optstring ); 调用一次，返回一个选项。 
在命令行选项参数再也检查不到optstring中包含的选项时，返回－1，同时optind储存第一个不包含选项的命令行参数*/
int main(int argc, char *argv[])
{
	/*切换一下工作路径*/
	
	int ret = chdir(PATH);
	if (ret != 0)
	{
		perror("chdir error!!!");
		exit(1);
	}
	//打印当前路径
	cout << "working dir: " << PATH << endl;
	int threadNum = 4;//默认情况下使用的是4线程 但是可以根据不同的电脑调整该参数 比如我的电脑是6线程 可以输入6 
	int port = 8888;//默认情况下的端口设置为8888
	//std::string logPath = "./log/WebServer.log";/*默认情况下将日志文件存储位置在当前文件夹下的log文件夹中WebServer.log文件中*/
	std::string logPath = "./log/WebServer.log";
	int opt;
	const char *str = "t:l:p:";
	while ((opt = getopt(argc, argv, str)) != -1)
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
				if (logPath.size() < 2 || optarg[0] != '/')
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
	std::cout << "The direction of logPath is: " << logPath << endl;
	Logger::setLogFileName(logPath);
	//STL库在多线程上的应用
	#ifndef _PTHREADS
		LOG << "_PTHREADS is not defined !";
	#endif
	
	EventLoop mainLoop;
	//std::cout << "sdsddddddddddd: " << endl;
	Server myHTTPServer(&mainLoop, threadNum, port);
	myHTTPServer.start();
	mainLoop.loop();
	return 0;
}