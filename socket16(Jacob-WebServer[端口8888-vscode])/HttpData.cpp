#include "HttpData.h"
#include "time.h"
#include "Channel.h"
#include "Util.h"
#include "EventLoop.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>
#include <time.h>
#include <sys/socket.h>
//打开显示文件内容
#include <dirent.h>//打开文件目录
using namespace std;

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime;

const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;//默认事件
const int DEFAULT_EXPIRED_TIME = 2000; // 默认过期时间
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000; // 设置长连接的时间

//网站图片 默认搜索
char favicon[555] = {
	'\x89', 'P', 'N', 'G', '\xD', '\xA', '\x1A', '\xA', '\x0', '\x0', '\x0', '\xD', 'I', 'H', 'D', 'R',
	'\x0', '\x0', '\x0', '\x10', '\x0', '\x0', '\x0', '\x10', '\x8', '\x6', '\x0', '\x0', '\x0', '\x1F', '\xF3', '\xFF',
	'a', '\x0', '\x0', '\x0', '\x19', 't', 'E', 'X', 't', 'S', 'o', 'f', 't', 'w', 'a', 'r', 'e', '\x0', 'A', 'd', 'o', 'b', 'e', '\x20',
	'I', 'm', 'a', 'g', 'e', 'R', 'e', 'a', 'd', 'y', 'q', '\xC9', 'e', '\x3C', '\x0', '\x0',
	'\x1', '\xCD', 'I', 'D', 'A', 'T', 'x', '\xDA', '\x94', '\x93', '9', 'H', '\x3', 'A', '\x14', '\x86',
	'\xFF', '\x5D', 'b', '\xA7', '\x4', 'R', '\xC4', 'm', '\x22', '\x1E', '\xA0', 'F', '\x24', '\x8', '\x16', '\x16',
	'v', '\xA', '6', '\xBA', 'J', '\x9A', '\x80', '\x8', 'A', '\xB4', 'q', '\x85', 'X', '\x89', 'G', '\xB0',
	'I', '\xA9', 'Q', '\x24', '\xCD', '\xA6', '\x8', '\xA4', 'H', 'c', '\x91', 'B', '\xB', '\xAF', 'V', '\xC1',
	'F', '\xB4', '\x15', '\xCF', '\x22', 'X', '\x98', '\xB', 'T', 'H', '\x8A', 'd', '\x93', '\x8D', '\xFB', 'F',
	'g', '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f', 'v', 'f', '\xDF', '\x7C', '\xEF', '\xE7', 'g', 'F', '\xA8',
	'\xD5', 'j', 'H', '\x24', '\x12', '\x2A', '\x0', '\x5', '\xBF', 'G', '\xD4', '\xEF', '\xF7', '\x2F', '6', '\xEC',
	'\x12', '\x20', '\x1E', '\x8F', '\xD7', '\xAA', '\xD5', '\xEA', '\xAF', 'I', '5', 'F', '\xAA', 'T', '\x5F', '\x9F',
	'\x22', 'A', '\x2A', '\x95', '\xA', '\x83', '\xE5', 'r', '9', 'd', '\xB3', 'Y', '\x96', '\x99', 'L', '\x6',
	'\xE9', 't', '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T', '\xA7', '\xC4', 'b', '1', '\xB5', '\x5E', '\x0', '\x3',
	'h', '\x9A', '\xC6', '\x16', '\x82', '\x20', 'X', 'R', '\x14', 'E', '6', 'S', '\x94', '\xCB', 'e', 'x',
	'\xBD', '\x5E', '\xAA', 'U', 'T', '\x23', 'L', '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0', '\x9E', '\xBC', '\x9',
	'A', '\x7C', '\x3E', '\x1F', '\x83', 'D', '\x22', '\x11', '\xD5', 'T', '\x40', '\x3F', '8', '\x80', 'w', '\xE5',
	'3', '\x7', '\xB8', '\x5C', '\x2E', 'H', '\x92', '\x4', '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g', '\x98',
	'\xE9', '6', '\x1A', '\xA6', 'g', '\x15', '\x4', '\xE3', '\xD7', '\xC8', '\xBD', '\x15', '\xE1', 'i', '\xB7', 'C',
	'\xAB', '\xEA', 'x', '\x2F', 'j', 'X', '\x92', '\xBB', '\x18', '\x20', '\x9F', '\xCF', '3', '\xC3', '\xB8', '\xE9',
	'N', '\xA7', '\xD3', 'l', 'J', '\x0', 'i', '6', '\x7C', '\x8E', '\xE1', '\xFE', 'V', '\x84', '\xE7', '\x3C',
	'\x9F', 'r', '\x2B', '\x3A', 'B', '\x7B', '7', 'f', 'w', '\xAE', '\x8E', '\xE', '\xF3', '\xBD', 'R', '\xA9',
	'd', '\x2', 'B', '\xAF', '\x85', '2', 'f', 'F', '\xBA', '\xC', '\xD9', '\x9F', '\x1D', '\x9A', 'l', '\x22',
	'\xE6', '\xC7', '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15', '\x90', '\x7', '\x93', '\xA2', '\x28', '\xA0', 'S', 'j',
	'\xB1', '\xB8', '\xDF', '\x29', '5', 'C', '\xE', '\x3F', 'X', '\xFC', '\x98', '\xDA', 'y', 'j', 'P', '\x40',
	'\x0', '\x87', '\xAE', '\x1B', '\x17', 'B', '\xB4', '\x3A', '\x3F', '\xBE', 'y', '\xC7', '\xA', '\x26', '\xB6', '\xEE',
	'\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD', '\xA', '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X', '\x0',
	'\x27', '\xEB', 'n', 'V', 'p', '\xBC', '\xD6', '\xCB', '\xD6', 'G', '\xAB', '\x3D', 'l', '\x7D', '\xB8', '\xD2',
	'\xDD', '\xA0', '\x60', '\x83', '\xBA', '\xEF', '\x5F', '\xA4', '\xEA', '\xCC', '\x2', 'N', '\xAE', '\x5E', 'p', '\x1A',
	'\xEC', '\xB3', '\x40', '9', '\xAC', '\xFE', '\xF2', '\x91', '\x89', 'g', '\x91', '\x85', '\x21', '\xA8', '\x87', '\xB7',
	'X', '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N', 'N', 'b', 't', '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
	'\xEC', '\x86', '\x2', 'H', '\x26', '\x93', '\xD0', 'u', '\x1D', '\x7F', '\x9', '2', '\x95', '\xBF', '\x1F', '\xDB',
	'\xD7', 'c', '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF', '\x22', 'J', '\xC3', '\x87', '\x0', '\x3', '\x0', 'K',
	'\xBB', '\xF8', '\xD6', '\x2A', 'v', '\x98', 'I', '\x0', '\x0', '\x0', '\x0', 'I', 'E', 'N', 'D', '\xAE', 'B', '\x60', '\x82',
};
void MimeType::init()
{
	mime[".html"] = "text/html; charset=utf-8";
	mime[".htm"] = "text/html; charset=utf-8";
	mime[".css"] = "text/css";
	mime[".avi"] = "video/x-msvideo";
	mime[".au"] = "audio/basic";
	mime[".bmp"] = "image/bmp";
	mime[".c"] = "text/plain";
	mime[".doc"] = "application/msword";
	mime[".gif"] = "image/gif";
	mime[".gz"] = "application/x-gzip";
	mime[".ico"] = "application/x-ico";
	mime[".jpg"] = "image/jpeg;";
	mime[".png"] = "image/png";
	mime[".txt"] = "text/plain";
	mime[".mp3"] = "audio/mp3";
	mime[".mpeg"] = "video/mpeg";
	mime[".mpe"] = "video/mpeg";
	mime[".vrml"] = "model/vrml";
	mime[".wrl"] = "model/vrml";
	mime[".midi"] = "audio/midi";
	mime[".mid"] = "audio/midi";
	mime[".mov"] = "video/quicktime";
	mime[".wav"] = "audio/wav";
	mime[".qt"] = "video/quicktime";
	mime[".wav"] = "audio/wav";
	mime[".avi"] = "video/x-msvideo";
	mime["default"] = "text/html; charset=utf-8";
}

std::string MimeType::getMime(const std::string &suffix)
{
	pthread_once(&once_control, MimeType::init);
	if(mime.find(suffix) == mime.end())
	{
		return mime["default"];
	}
	else
	{
		return mime[suffix];
	}
}

HttpData::HttpData(EventLoop *loop, int connfd)
: 	loop_(loop),
	channel_(new Channel(loop, connfd)),
	fd_(connfd),
	error_(false),
	connectionState_(H_CONNECTED),
	method_(METHOD_GET),
	HTTPVersion_(HTTP_11),
	nowReadPos_(0),
	state_(STATE_PARSE_URL),
	hState_(H_START),
	keepAlive_(false)
{
	//变量初始化顺序应该要按照定义的顺序来
	channel_->setReadHandler(bind(&HttpData::handleRead, this));
	channel_->setWriteHandler(bind(&HttpData::handleWrite, this));
	channel_->setConnHandler(bind(&HttpData::handleConn, this));
}

//重置请求对象
void HttpData::reset()
{
	fileName_.clear();
	path_.clear();
	nowReadPos_ = 0;
	state_ = STATE_PARSE_URL;
	hState_ = H_START;
	headers_.clear();
	//重置时间节点对象
	if (timer_.lock())
	{
		shared_ptr<TimerNode> my_timer(timer_.lock());
		my_timer->clearReq();
		timer_.reset();
	}
}
//定时器和请求分离
void HttpData::seperateTimer()
{
	if(timer_.lock())
	{
		shared_ptr<TimerNode> my_timer(timer_.lock());
		my_timer->clearReq();
		timer_.reset();
	}
}

//处理读事件 但是就算是post请求 我也要进行读事件处理
void HttpData::handleRead(){
	__uint32_t &events_ =  channel_->getEvents();
	do
	{
		/* code */
		bool zero = false;
		int read_num = readn(fd_, inBuffer_, zero);
		LOG << "Request: " << inBuffer_;
		if(connectionState_ == H_DISCONNECTING)
		{
			inBuffer_.clear();
			break;
		}
		if(read_num < 0)
		{
			perror("1");
			error_ = true;
			handleError(fd_, 400, "Bad Request");
			break;
		}
		else if(zero)
		{
			//有请求但是读不到数据，可能是Request Aborted, 或者来自网络的数据没有达到等原因
			//最有可能是对端已经关闭了，统一按照对端已经关闭处理 error_true;
			connectionState_ = H_DISCONNECTING;
			if(read_num == 0)
			{
				break;
			}
		}
		if(state_ == STATE_PARSE_URL)
		{
			URIState flag = this->parseURI();
			if(flag == PARSE_URL_AGAIN)
			{
				break;
			}
			else if(flag == PARSE_URL_ERROR)
			{
				perror("2");
				LOG << "FD = " << fd_ << "," << inBuffer_ << "*******";//将出错的套接字和请求输出到日志中
				inBuffer_.clear();
				error_ = true;
				handleError(fd_, 400, "Bad Request");
				break;
			}
			else
			{
				state_ = STATE_PARSE_HEADERS;//解析了地址 接下来该解析请求头
			}
		}
		if(state_ == STATE_PARSE_HEADERS)
		{
			HeaderState flag = this->parseHeader();
			if(flag == PARSE_HEADER_AGAIN)
			{
				break;
			}
			else if(flag == PARSE_HEADER_ERROR)
			{
				perror("3");
				error_ = true;
				handleError(fd_, 400, "Bad Request");
				break;
			}
			if(method_ == METHOD_POST)
			{//如果是post请求的话 代表已将上传文件 因此我们应该把状态切换到上传状态
				state_ = STATE_RECV_BODY;
			}
			else
			{
				state_ = STATE_ANALYSIS;//分析数据状态
			}
		}
		if (state_ == STATE_RECV_BODY)
		{
			int content_length = -1;
			if(headers_.find("Content-length") != headers_.end())
			{
				content_length = stoi(headers_["Content-length"]);//讲字符串转换成数字
			}
			else
			{
				error_ = true;
				handleError(fd_, 400, "Bad Request: Lack of argument (Content-length)");
				break;
			}
			if(static_cast<int>(inBuffer_.size()) < content_length)
			{
				break;
			}
			state_ = STATE_ANALYSIS;
		}
		//当前状态为STATE_ANALYSIS 也是最重要的解析状态
		if (state_ == STATE_ANALYSIS)
		{
			AnalysisState flag = this->analyisRequest();
			if(flag == ANALYSIS_SUCCESS)
			{
				state_ = STATE_FINISH;
				break;
			}
			else
			{
				error_ = true;
				break;
			}
		}
	} while (false);
	//如果error_为false 代表前面有一个流程出现错误
	if(!error_)
	{
		if(outBuffer_.size() > 0)
		{
			handleWrite();
		}
		if(!error_ && state_ == STATE_FINISH)
		{
			this->reset();
			//如果依然有请求在inBuffer_中 则继续处理 直到inBuffer 中为空
			if(inBuffer_.size() > 0)
			{
				if(connectionState_ != H_DISCONNECTING)
				{
					handleRead();
				}
			}
		}
		else if(!error_ && connectionState_ != H_DISCONNECTED)
		{
			//只有当该套接字对端的数据可读时才会触发，触发一次后需要不断读取对端的数据直到返回EAGAIN错误码为止。
			events_ |= EPOLLIN;
		}
	}
}

void HttpData::handleWrite()
{
	if(!error_ && connectionState_ != H_DISCONNECTED)
	{
		__uint32_t &events_ = channel_->getEvents();
		if(writen(fd_, outBuffer_) < 0)
		{
			perror("writen");
			events_ = 0;
			error_ = true;
		}
		if(outBuffer_.size() > 0)
		{
			events_ |= EPOLLOUT;
		}
	}
}

void HttpData::handleConn()
{
	seperateTimer();
	__uint32_t &events_ = channel_->getEvents();
	//error_为false 表示没有出现错误。如果有错误的话error_就会是true;
	if(!error_ && connectionState_ == H_CONNECTED)
	{
		if(events_ != 0)
		{
			int timeout = DEFAULT_EXPIRED_TIME;
			if(keepAlive_)
			{
				timeout = DEFAULT_KEEP_ALIVE_TIME;
			}
			if((events_ & EPOLLIN) && (events_ & EPOLLOUT))
			{
				events_ = __uint32_t(0);
				events_ |= EPOLLOUT;
			}
			events_ |= EPOLLET;
			loop_->updatePoller(channel_, timeout);
		}
		else if(keepAlive_)
		{
			events_ |= (EPOLLIN | EPOLLET);
			int timeout = DEFAULT_KEEP_ALIVE_TIME;
			loop_->updatePoller(channel_, timeout);
		}
		else
		{
			events_ |= (EPOLLIN | EPOLLET);
			//如果发现不是长连接的话 将默认的超时时间设置为default的时间的一半
			int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
			loop_->updatePoller(channel_, timeout);
		}
	}
	else if(!error_ && connectionState_ == H_DISCONNECTING && (events_ & EPOLLOUT))
	{
		events_ = (EPOLLOUT | EPOLLET);
	}
	else
	{
		loop_->runInLoop(bind(&HttpData::handleClose, shared_from_this()));
	}
}

URIState HttpData::parseURI()
{
	string &str = inBuffer_;
	string cop = str;
	//读到完整的请求行再开始解析
	size_t pos = str.find('\r', nowReadPos_);
	if (pos < 0)
	{
		return PARSE_URL_AGAIN;
	}
	//去掉请求地址所占的空间 节省空间
	string request_line = str.substr(0, pos);
	if(str.size() > pos + 1)
	{
		str = str.substr(pos + 1);//将剩余的内容保存下来
	}
	else
	{
		str.clear();//去除所有的请求行
	}
	int posGet = request_line.find("GET");
	int posPost = request_line.find("POST");
	int posHead = request_line.find("HEAD");

	if(posGet >= 0)
	{
		pos = posGet;
		method_ = METHOD_GET;
	}
	else if(posPost >= 0)
	{
		pos = posPost;
		method_ = METHOD_POST;
	}
	else if(posHead >= 0)
	{
		pos = posHead;
		method_ = METHOD_HEAD;
	}
	else
	{
		return PARSE_URL_ERROR;
	}
	//filename
	pos = request_line.find("/", pos);
	if(pos < 0)
	{
		//这里当没有加 / 时，默认是将filName_定义为index.html 如果没有index.html就会返回404页面
		//但是一般的网站其实都是这样设计的，在处理没有/ 和 文件时候将访问文件设置为index.html，也就是我们常说的主页
		fileName_ = "index.html";
		HTTPVersion_ = HTTP_11;
		return PARSE_URL_SUCCESS;
	}
	else
	{
		size_t _pos = request_line.find(' ', pos);
		if(_pos < 0)
		{
			return PARSE_URL_ERROR;
		}
		else
		{
			if (_pos - pos > 1)
			{
				//截取pos + 1到_pos - pos + 1之间的字符串
				fileName_ = request_line.substr(pos + 1, _pos - pos - 1);
				
				/*不管是否为中文文件，都对其进行解码，虽然解码，但是其实如果是英文字符我们不会对其进行处理*/
				char* tmp = (char*)fileName_.c_str();
				decode_str(tmp, tmp);
				fileName_ = (string)tmp;

				size_t __pos = fileName_.find('?');
				if (__pos >= 0)
				{
					fileName_ = fileName_.substr(0, __pos);
				}
			}
			//表示
			else
				fileName_ = "./";
				//fileName_ = "index.html";
		}
		pos = _pos;//将下标向后移
	}
	//HTTP 版本号
	pos = request_line.find("/", pos);
	if(pos < 0)
	{
		return PARSE_URL_ERROR;
	}
	else
	{
		if(request_line.size() - pos <= 3)
		{
			return PARSE_URL_ERROR;
		}
		else
		{
			string ver = request_line.substr(pos + 1, 3);
			if(ver == "1.0")
			{
				HTTPVersion_ = HTTP_10;
			}
			else if(ver == "1.1")
			{
				HTTPVersion_ = HTTP_11;
			}
			else
			{
				return PARSE_URL_ERROR;
			}
		}
	}
	return PARSE_URL_SUCCESS;
}

HeaderState HttpData::parseHeader()
{
	string &str = inBuffer_;
	int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
	int now_read_line_begin = 0;
	bool notFinish = true;
	size_t i = 0;
	for(; i < str.size() && notFinish; ++i)
	{
		switch(hState_)
		{
			case H_START:
			{
				if(str[i] == '\n' || str[i] == '\r')
				{
					break;
				}
				//找到请求头的第一个关键位置
				hState_ = H_KEY;
				key_start = i;
				now_read_line_begin = i;
				break;
			}
			case H_KEY:
			{
				if(str[i] == ':')
				{
					key_end = i;
					if(key_end - key_start <= 0)
					{
						return PARSE_HEADER_ERROR;
					}
					hState_ = H_COLON;
				}
				else if(str[i] == '\n' && str['i'] == '\r')
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
			case H_COLON:
			{
				if(str[i] == ' ')
				{
					hState_ = H_SPACES_AFTER_COLON;
				}
				else
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
			case H_SPACES_AFTER_COLON:
			{
				hState_ = H_VALUE;
				value_start = i;
				break;
			}
			case H_VALUE:
			{
				if(str[i] == '\r')
				{
					hState_ = H_CR;
					value_end = i;
					if(value_end - value_start <= 0)
					{
						return PARSE_HEADER_ERROR;
					}
				}
				else if(i - value_start > 255)
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
			case H_CR:
			{
				if (str[i] == '\n')
				{
					hState_ = H_LF;
					string key(str.begin() + key_start, str.begin() + key_end);
					string value(str.begin() + value_start, str.begin() + value_end);
					headers_[key] = value;
					now_read_line_begin = i;
				}
				else
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
			case H_LF:
			{
				if(str[i] == '\r')
				{
					hState_ = H_END_CR;
				}
				else
				{
					key_start = i;
					hState_ = H_KEY;
				}
				break;
			}
			case H_END_CR:
			{
				if (str[i] == '\n')
				{
					hState_ = H_END_LF;
				}
				else
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
			case H_END_LF:
			{
				notFinish = false;
				key_start = i;
				now_read_line_begin = i;
				break;
			}
		}
	}
	if (hState_ == H_END_LF)
	{
		str = str.substr(i);
		return PARSE_HEADER_SUCCESS;
	}
	str = str.substr(now_read_line_begin);
	return PARSE_HEADER_AGAIN;
}

AnalysisState HttpData::analyisRequest()
{
	if(method_ == METHOD_POST)
	{
		return ANALYSIS_ERROR;
		//method = post 内容没有写
	}
	else if (method_ == METHOD_GET || method_ == METHOD_HEAD)
	{
		string header;
		header += "HTTP/1.1 200 OK\r\n";
		if (headers_.find("Connection") != headers_.end() && (headers_["Connection"] == "Keep-Alive" || headers_["Connection"] == "keep-alive"))
		{
			keepAlive_ = true;
			header += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" + to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
		}
		int dot_pos = fileName_.find('.');
		string filetype;
		if (dot_pos < 0)
			filetype = MimeType::getMime("default");
		else
			filetype = MimeType::getMime(fileName_.substr(dot_pos));
		// echo test
		if (fileName_ == "hello")
		{
			outBuffer_ = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
			return ANALYSIS_SUCCESS;
		}
		if (fileName_ == "favicon.ico")
		{
			header += "Content-Type: image/png\r\n";
			header += "Content-Length: " + to_string(sizeof favicon) + "\r\n";
			header += "Server: jacob's Web Server\r\n";

			header += "\r\n";
			outBuffer_ += header;
			outBuffer_ += string(favicon, favicon + sizeof favicon);;
			return ANALYSIS_SUCCESS;
		}

		struct stat sbuf;
		if (stat(fileName_.c_str(), &sbuf) < 0)
		{
			header.clear();
			handleError(fd_, 404, "Not Found!");
			return ANALYSIS_ERROR;
		}
		header += "Content-Type: " + filetype + "\r\n";
		header += "Content-Length: " + to_string(sbuf.st_size) + "\r\n";
		header += "Server: jacob's Web Server\r\n";
		// 头部结束
		header += "\r\n";
		outBuffer_ += header;
		if (method_ == METHOD_HEAD)
			return ANALYSIS_SUCCESS;
		//这里有一个潜在的问题就是以前我们其实是在这个这个地方发送给客户端数据
		//而现在我们是将数据统一放入缓冲区中进行发送主要是在outBuffer_中
		if (S_ISDIR(sbuf.st_mode))
		{
			//使用发送错误文件的处理方式来进行改进
			outBuffer_.clear();
			//但是需要先发头文件再发送
			writen(fd_, header);
			//按照以前成功的经验，我们先发一个头文件出去，然后在发送body文件

			//判断如果是目录，需要将目录中的信息拼接成一个网页发给客户端
			/*由于重写发送机制因此我们只需要将信息放入outBuffer_即可，不需要重复发送--接下来---拼接一个html压面发给客户端*/
			char buf[4094] = { 0 };
			int i;
			sprintf(buf, "<html><head><title>Jacob_WebServer(%s)</title></head>", fileName_.c_str());

			sprintf(buf + strlen(buf), "<body><h1>当前目录：%s</h1>\n<h3><a href=\"http://121.199.60.131:8000/\">一个开源python-django博客项目</a></h3>\n<table>", fileName_.c_str());
			char enstr[1024] = { 0 };
			char path[1024] = { 0 };
			//目录项二级指针
			struct dirent** ptr;
			int num = scandir(fileName_.c_str(), &ptr, NULL, alphasort);
			//遍历
			for (i = 0; i < num; ++i)
			{
				char* name = ptr[i]->d_name;
				//拼接文件的完整路径
				sprintf(path, "%s/%s", fileName_.c_str(), name);
				//printf("path = %s ==================\n", path);
				struct stat st;
				stat(path, &st);
				//编码生成  %E5 %A7
				encode_str(enstr, sizeof(enstr), name);
				//如果是文件
				if (S_ISREG(st.st_mode))
				{
					sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", enstr, name, (long)st.st_size);
				}
				else if (S_ISDIR(st.st_mode))
				{
					sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s/</a></td><td>%ld</td></tr>", enstr, name, (long)st.st_size);
				}
				//memset(buf, 0, sizeof(buf));
				//字符串拼接
			}
			sprintf(buf + strlen(buf), "</table></body></html>");
			//将buf转换成string 放入outBuffer_中
			//outBuffer_ += (string)buf;
			writen(fd_, buf, strlen(buf));

			//这里我想打印当前请求目录的时间，方便随时查看是否服务器还在运行
			//获取系统时间  
			time_t now_time = time(NULL);
			//获取本地时间  
			tm*  t_tm = localtime(&now_time);
			printf("Port:8888 -> Request Dir: %s", asctime(t_tm));
			return ANALYSIS_DIR_SUCCESS;
			//return ANALYSIS_ERROR;
			//return ANALYSIS_SUCCESS;
		}
		else 
		{
			//普通文件的处理方式，就是直接将文件返回给客户端，显示出来
			int src_fd = open(fileName_.c_str(), O_RDONLY, 0);
			if (src_fd < 0)
			{
				outBuffer_.clear();
				handleError(fd_, 404, "Not Found!");
				return ANALYSIS_ERROR;
			}
			void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
			close(src_fd);
			if (mmapRet == (void *)-1)
			{
				munmap(mmapRet, sbuf.st_size);
				outBuffer_.clear();
				handleError(fd_, 404, "Not Found!");
				return ANALYSIS_ERROR;
			}
			char *src_addr = static_cast<char*>(mmapRet);
			outBuffer_ += string(src_addr, src_addr + sbuf.st_size);;
			munmap(mmapRet, sbuf.st_size);
			return ANALYSIS_SUCCESS;
		}
	}
	else
	{
		return ANALYSIS_ERROR;
	}
}

void HttpData::handleError(int fd, int err_num, string short_msg)
{
	short_msg = " " + short_msg;
    char send_buff[4096];
    string body_buff, header_buff;
    body_buff += "<html><head><meta http-equiv=Content-Type content=\"text/html; charset=utf-8\"><title>哎~出错了</title></head>";
    body_buff += "<body bgcolor=\"ffffff\">";
    body_buff += to_string(err_num) + short_msg;
    body_buff += "<hr><em> jacob's Web Server</em>\n</body></html>";

    header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
    header_buff += "Content-Type: text/html\r\n";
    header_buff += "Connection: Close\r\n";
    header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: jacob's Web Server\r\n";;
    header_buff += "\r\n";
    // 错误处理不考虑writen不完的情况
    sprintf(send_buff, "%s", header_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));
    sprintf(send_buff, "%s", body_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));
}

void HttpData::handleClose()
{
	connectionState_ = H_DISCONNECTED;
	shared_ptr<HttpData> guard(shared_from_this());
	loop_->removeFromPoller(channel_);//从epoll中移除事件
}

void HttpData::newEvent()
{
	//添加新的事件
	channel_->setEvents(DEFAULT_EVENT);
	//将事件挂上epoll树
	loop_->addToPoller(channel_, DEFAULT_EXPIRED_TIME);
}
