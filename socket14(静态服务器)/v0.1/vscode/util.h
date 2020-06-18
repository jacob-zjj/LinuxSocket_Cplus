#ifndef UTIL
#define UTIL
//c++中的<cstdlib> 相当于c语言中的stdlib标库
#include <cstdlib>
ssize_t readn(int fd, void *buff, size_t n);
ssize_t writen(int fd, void *buff, size_t n);
void handle_for_sigpipe();
//设置非阻塞
int setSockNonBlocking(int fd);
#endif
