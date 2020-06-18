#ifndef __WRAP_H_
#define	__WRAP_H_
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

void perr_exit(const char *s);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
int Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
int Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
int Close(int fd);
//指定读取多少个字节
ssize_t Readn(int fd, void *vptr, size_t n);
//指定写出n个字符
ssize_t	Writen(int fd, const void *vptr, size_t n);
//只在当前模块中有用
static ssize_t my_read(int fd, char *ptr);
//指定读一行数据
ssize_t Readline(int fd, void *vptr, size_t maxlen);
#endif