#include "wrap.h"
#define SRV_PORT 9999
//回收子进程
void catch_child(int signum)
{

	while (waitpid(0, NULL, WNOHANG) > 0);
	return;
}
int main(int argc, char *argv[])
{
	int lfd, cfd;
	pid_t pid;
	int ret, i;
	char buf[BUFSIZ];
	struct sockaddr_in srv_addr, clt_addr;
	socklen_t clt_addr_len;

	//memset(&srv_addr, 0, sizeof(srv_addr));	//将地址结构清零
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(SRV_PORT);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	lfd = Socket(AF_INET, SOCK_STREAM, 0);
	Bind(lfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

	Listen(lfd, 128);
	clt_addr_len = sizeof(clt_addr);

	while (1)
	{
		cfd = Accept(lfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
		pid = fork();
		if (pid < 0)
		{
			perr_exit("forke error");
		}
		else if (pid == 0) 
		{
			close(lfd);
			break;
		}
		else
		{
			//清理僵尸进程
			struct sigaction act;
			act.sa_handler = catch_child;
			sigemptyset(&act.sa_mask);
			act.sa_flags = 0;
			ret = sigaction(SIGCHLD,&act, NULL);
			if (ret != 0)
			{
				perr_exit("sigaction error");
			}
			close(cfd);
			continue;
		}
	}
	if (pid == 0)
	{
		for (;;) {
			ret = Read(cfd, buf, sizeof(buf));
			if (ret == 0)
			{
				close(cfd);
				exit(1);
			}
			for (int i = 0; i < ret; i++)
			{
				buf[i] = toupper(buf[i]);
			}
			Write(cfd, buf, ret);
			Write(STDOUT_FILENO, buf, ret);
		}
	}
	return 0;
}