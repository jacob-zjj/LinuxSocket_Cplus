#pragma once
#ifndef REQUESTDATA
#define REQUESTDATA
#include "timer.h"
#include <string>
#include <unordered_map> 
#include <memory>
/*引用头文件(C++11)：#include <unordered_map> -- hashmap表
定义：unordered_map<int,int>、unordered_map<string, double>…
插入：例如将(“ABC” -> 5.45) 插入unordered_map<string, double> hash中，hash[“ABC”]=5.45*/
const int STATE_PARSE_URI = 1;//解析地址
const int STATE_PARSE_HEADERS = 2;//解析头
const int STATE_RECV_BODY = 3;//收到主体内容
const int STATE_ANALYSIS = 4;
const int STATE_FINISH = 5;
//缓冲区最大值
const int MAX_BUFF = 4096;
//有请求出现但是读不到数据，可能是Request Aborted，或者来自网络的数据没有达到等原因 对这样的请求尝试超过一定的次数就抛弃
//尝试次数最多为200
const int AGAIN_MAX_TIMES = 200;
//解析地址
const int PARSE_URI_AGAIN = -1;
const int PARSE_URI_ERROR = -2;
const int PARSE_URI_SUCCESS = 0;
//解析头
const int PARSE_HEADER_AGAIN = -1;
const int PARSE_HEADER_ERROR = -2;
const int PARSE_HEADER_SUCCESS = 0;
//分析数据
const int ANALYSIS_ERROR = -2;
const int ANALYSIS_SUCCESS = 0;
//HTTP请求类型 POST方法还是GET方法
const int METHOD_POST = 1;
const int METHOD_GET = 2;
const int HTTP_10 = 1;
const int HTTP_11 = 2;

const int EPOLL_WAIT_TIME = 500;

//用来判断请求文件后缀名
class MimeType
{
private:
	//static pthread_mutex_t lock;
	static void init();
	static std::unordered_map<std::string, std::string> mime;//hashmap表
	MimeType();
	MimeType(const MimeType &m);//拷贝构造函数 避免析构使得浅拷贝的值失效

public:
	static std::string getMime(const std::string &suffix);

private:
	//使用线程只调用一次方式 这样避免了 使用锁来锁住资源 - 将程序进一步进行优化
	static pthread_once_t once_control;
	//静态变量一定要初始化它 我们在类的实现中即实现了该变量
};

//请求头状态
/*枚举类型中的值 当所有值都未赋值时第一次元素的值为0 后面的值依次加1
如果有赋值的话则 后面的变量则从该赋值位置开始逐渐加1
*/
enum HeaderState
{
	h_start = 0,
	h_key,
	h_colon,
	h_spaces_after_colon,
	h_value,
	h_CR,
	h_LF,
	h_end_CR,
	h_end_LF
};

struct TimerNode;

//请求数据
//enable_shared_from_this 这是一个以基派生类为模板类型实参的基类模板 继承它 this指针就变成了shared_ptr类型
class RequestData : public std::enable_shared_from_this<RequestData>
{
private:
	int againTimes;//监控 观察请求次数 请求次数的阈值AGAIN_MAX_TIMES为200
	std::string path;
	int fd;
	int epollfd;
	std::string content;//content的内容用完就清理
	int method;
	int HTTPversion;//http协议版本
	std::string file_name;
	int now_read_pos;
	int state;
	int h_state;
	bool isfinish;
	bool keep_alive;
	std::unordered_map<std::string, std::string> headers;
	//请求数据内部 加上 定时器 使用weak_ptr虽然不可以控制生命周期 但是可以知道对象是否还活着
	//mytimer *timer;
	std::weak_ptr<TimerNode> timer;

private:
	//解析地址
	int parse_URL();
	//解析请求头
	int parse_Headers();
	//分析请求数据
	int analysisRequest();

public:
	RequestData();
	RequestData(int _epollfd, int _fd, std::string _path);
	~RequestData();
	void linkTimer(std::shared_ptr<TimerNode> mtimer);
	//void addTimer(mytimer *mtimer);
	//void addTimer(std::shared_ptr<TimerNode> mtimer);
	void reset();
	void seperateTimer();
	int getFd();
	void setFd(int _fd);
	void handleRequest();
	void handleError(int fd, int err_num, std::string short_msg);
};

#endif