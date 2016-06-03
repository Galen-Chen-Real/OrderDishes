/*
 *        'sock.h'
 *     (C) 2014.4 GordonChen
 *
 */
#ifndef _TF_SOCKET_H
#define _TF_SOCKET_H

#define LISNUM 4

//开启客户端并连接服务器;成功返回sockfd,失败返回-1.
int  open_sock_cli(char *ip, int port);
//TCP发送数据
int  tcp_send(int sockfd, const void *buf, int len);
//TCP接收数据
int  tcp_recv(int sockfd, void *buf, int len);
//关闭客户端
void close_sock_cli(int sockfd);

#endif

