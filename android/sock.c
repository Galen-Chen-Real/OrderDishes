/*
 *        'sock.c'
 *     (C) 2014.4 GordonChen
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "sock.h"

int open_sock_cli(char *ip, int port)
{
    int len, fd, size;
    struct sockaddr_in dest;
    int sockfd;

    /* 创建一个 socket 用于 tcp 通信 */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket");
        return -1;
    }

    /* 初始化服务器端（对方）的地址和端口信息 */
    memset((void *)&dest, '\0', sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = port;
    if (inet_aton(ip, (struct in_addr *) &dest.sin_addr.s_addr) == 0)
    {
        perror(ip);
        return -1;
    }

     /* 连接服务器 */
    if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0)
    {
        perror("Connect ");
        return -1;
    }
    
    return sockfd;
}

int  tcp_send(int sockfd, const void *buf, int len)
{
    return send(sockfd, buf, len, 0);
}

int  tcp_recv(int sockfd, void *buf, int len)
{
    return recv(sockfd, buf, len, 0);
}

void close_sock_cli(int sockfd)
{
    /* 关闭监听的 socket */
    close(sockfd);
}


