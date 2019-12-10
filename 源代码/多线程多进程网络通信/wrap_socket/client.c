#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "wrap.h"

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666

int main(void)
{
    int sfd, len;
    struct sockaddr_in serv_addr;
    char buf[BUFSIZ]; 

    sfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serv_addr, sizeof(serv_addr));                       
    serv_addr.sin_family = AF_INET;                             
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);    
    serv_addr.sin_port = htons(SERV_PORT);                      

    Connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    while (1) {
        fgets(buf, sizeof(buf), stdin);
        int r = Write(sfd, buf, strlen(buf));       
        printf("Write r ======== %d\n", r);
        len = Read(sfd, buf, sizeof(buf));
        printf("Read len ========= %d\n", len);
        Write(STDOUT_FILENO, buf, len);
    }

    Close(sfd);

    return 0;
}

