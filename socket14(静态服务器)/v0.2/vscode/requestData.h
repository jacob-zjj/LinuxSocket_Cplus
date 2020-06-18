#ifndef REQUESTDATA
#define REQUESTDATA
#include <string>
#include <unordered_map> 
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
	static pthread_mutex_t lock;
	static std::unordered_map<std::string, std::string> mime;//hashmap表
	MimeType();
	MimeType(const MimeType &m);//拷贝构造函数 避免析构使得浅拷贝的值失效
public:
	static std::string getMime(const std::string &suffix);
};
//请求头状态
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
struct mytimer;
struct requestData;
//请求数据
struct requestData
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
	//请求数据内部 加上 定时器
	mytimer *timer;
private:
	//解析地址
	int parse_URL();
	//解析请求头
	int parse_Headers();
	//分析请求数据
	int analysisRequest();
public:
	requestData();
	requestData(int _epollfd, int _fd, std::string _path);
	~requestData();
	void addTimer(mytimer *mtimer);
	void reset();
	void seperateTimer();
	int getFd();
	void setFd(int _fd);
	void handleRequest();
	void handleError(int fd, int err_num, std::string short_msg);
};

//定时器结构体
struct mytimer
{
	bool deleted;
	//size_t size type 即一种变量的大小 可以转换为int型
	size_t expired_time;//定时器的过期时间 其值为创建时间 + timeout的值
	requestData *request_data;//将请求数据指针放置于该结构体中的作用是 判断 是否有请求数据-使用定时器

	mytimer(requestData *_request_data, int timeout);
	~mytimer();
	void update(int timeout);
	bool isvalid();
	void clearReq();
	void setDeleted();
	bool isDeleted() const;
	size_t getExpTime() const;
};
//设置定时器比较类 重载括号操作符 如果a的定时器存活时间大于b则返回ture 否则返回false
struct timerCmp
{
	bool operator()(const mytimer *a, const mytimer *b) const;
};

/*RAII的做法是使用一个对象，在其构造时获取对应的资源，在对象生命期内控制对资源的访问，
使之始终保持有效，最后在对象析构的时候，释放构造时获取的资源。*/
class MutexLockGuard//管理资源
{
public:
	//避免在对象初始化时隐式调用该构造函数 只能通过显示调用 及不支持隐式类型转换
	explicit MutexLockGuard();
	~MutexLockGuard();

private:
	static pthread_mutex_t lock;

private:
	MutexLockGuard(const MutexLockGuard&);
	MutexLockGuard& operator=(const MutexLockGuard&);
};
#endif