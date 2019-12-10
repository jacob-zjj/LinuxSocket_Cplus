/*Util.h文件中包含了项目中所用到的所有功能函数  字符串读写 信号处理  设置非阻塞 绑定监听端口等*/
#pragma once
#include <string>
#include <cstdlib>
//设置读函数的几种类型
ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
ssize_t readn(int fd, std::string &inBuffer);
//设置写函数的几种类型
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);
//处理信号管道
void handle_for_sigpipe();
//设置非阻塞
int setSocketNonBlocking(int fd);
//设置非延迟
void setSocketNodelay(int fd);
//设置优雅退出还是 强制退出
void setSocketNoLinger(int fd);
//关闭
void shutDownWR(int fd);
//sokcet绑定和监听函数
int socket_bind_listen(int port);
//16进制转换
int hexit(char c);
//解码
void decode_str(char *to, char *from);
//编码
void encode_str(char *to, int tosize, const char* from);