#pragma once
#include "Timer.h"
#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <functional>
#include <unistd.h>

class EventLoop;
class TimerNode;
class Channel;

/*这个文件中主要是对http访问的配置*/
//解析http流程状态
enum ProcessState
{
	//枚举类型当第一个值为1时，接下来的依次累加
	STATE_PARSE_URL = 1,
	STATE_PARSE_HEADERS,
	STATE_RECV_BODY,
	STATE_ANALYSIS,
	STATE_FINISH
};
//解析请求地址状态
enum URIState
{
	PARSE_URL_AGAIN = 1,
	PARSE_URL_ERROR,
	PARSE_URL_SUCCESS,
};
//解析请求头状态
enum HeaderState
{
	PARSE_HEADER_SUCCESS = 1,
	PARSE_HEADER_AGAIN,
	PARSE_HEADER_ERROR
};
//分析状态
enum AnalysisState // 1-2
{
	ANALYSIS_SUCCESS = 1,
	ANALYSIS_DIR_SUCCESS = 2,
	ANALYSIS_ERROR
};
//解析状态
enum ParseState//0 - 9
{
	H_START = 0,
	H_KEY,
	H_COLON,
	H_SPACES_AFTER_COLON,
	H_VALUE,
	H_CR,
	H_LF,
	H_END_CR,
	H_END_LF
};
//连接状态
enum ConnnctionState// 0 - 2
{
	H_CONNECTED = 0,
	H_DISCONNECTING,
	H_DISCONNECTED
};
//http请求的方法
enum HttpMethod // 1-3
{
	METHOD_POST = 1,
	METHOD_GET,
	METHOD_HEAD
};
//使用的http的版本 是1.0还是1.1
enum HttpVersion// 1 -2
{
	HTTP_10 = 1,
	HTTP_11
};
//请求文档类型解析
class MimeType
{
private:
	static void init();
	static std::unordered_map<std::string, std::string> mime;//使用未排序的map容器来存储请求文件
	MimeType();
	MimeType(const MimeType &m);//拷贝构造函数
public:
	static std::string getMime(const std::string &suffix);
private:
	static pthread_once_t once_control;//这里也加入了一次调用的方法
};

//2019年10月21日22:46:28  HttpData类型的类 将该类定义为智能指针类 
class HttpData : public std::enable_shared_from_this<HttpData>
{
public:
	HttpData(EventLoop *loop, int connfd);
	~HttpData()
	{
		close(fd_);
	}
	void reset();
	void seperateTimer();
	//连接时间
	void linkTimer(std::shared_ptr<TimerNode> mtimer)
	{
		//shared_ptr 重载了bool 但是weak_ptr没有
		timer_ = mtimer;
	}
	std::shared_ptr<Channel> getChannel() { return channel_; }
	EventLoop *getLoop() { return loop_; }
	void handleClose();
	void newEvent();

private:
	/*将智能指针拿来作为备用 可以在测试时看是否可用
	std::shared_ptr<EventLoop> loop_;
	*/
	EventLoop	*loop_;
	std::shared_ptr<Channel> channel_;
	int fd_;
	std::string inBuffer_;
	std::string outBuffer_;
	bool error_;
	/*以下为http请求各状态值*/
	ConnnctionState connectionState_;

	HttpMethod method_;
	HttpVersion HTTPVersion_;
	std::string fileName_;//请求文件名字
	std::string path_;//请求路径
	int nowReadPos_;//当前读到的位置
	ProcessState state_;//当前进行到的状态
	ParseState hState_;//当前解析的状态

	bool keepAlive_;//是否保持长连接
	std::map<std::string, std::string> headers_;//使用hashmap的方式来存储请求头
	std::weak_ptr<TimerNode> timer_;//时间节点

	/*处理读写连接和错误页面*/
	void handleRead();
	void handleWrite();
	void handleConn();
	void handleError(int fd, int err_num, std::string short_msg);
	//地址解析状态
	URIState parseURI();
	//请求头解析状态
	HeaderState parseHeader();
	//数据分析状态
	AnalysisState analyisRequest();
};