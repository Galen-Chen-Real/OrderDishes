/*
 *        'server.c'
 *     (C) 2014.4 GordonChen
 *
 *    点菜系统服务端: Usage: ./server ip port
 *
 *    注意: 本服务端使用的是mysql数据库: 用户名为'root', 主机名'localhost', 无密码, 
 * 数据库名为'dishes', 请使用 mysql -u root -h localhost dishes < dishes.sql
 * 还原数据库, 当然前提是你已经有一个名为'dishes'的mysql数据库.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#include "sql.h"
#include "pipe.h"
#include "sock.h"
#include "itoa.h"
#include "thread_pool.h"

#define THREADNUM 3  //线程池活动线程数
#define TABLEMAX 10  //桌子总数
#define TBNAMEMAX 12 //桌子名称最大长度
#define DISHMAX 64   //菜编号形如'1,2...'组合最大长度
#define OVER -1      //标志结束

int ser_guest_module(int sockfd);      //用户模块
void ser_quit(int signo);              //SIGINT信号触发完全退出 :)
void *ser_process(void *arg);          //和cli_process()对应
int send_dish(int sockfd, char *stb);  //和print_dish()对应

void *pthread_fun_sock(void *arg);     //通信模块

static int table[TABLEMAX];  //桌子占用情况
pthread_mutex_t sql_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Usage: %s ip port\n", argv[0]);
        return 0;
    }

    int res;
    pthread_t pthid;
    bzero(table, sizeof(table));
    
    signal(SIGINT, ser_quit);
    
    if(pipe_init())
    {
        printf("pipe_init() failed!\n");
        return -1;
    }
    
    res = pthread_create(&pthid, NULL, pthread_fun_sock, (void *)argv);
    if(res)
    {
        printf("pthread_create() pthread_fun_sock failed!\n");
        return -1;
    }
    
    int sockfd;
    
    if(pool_init(THREADNUM) != THREADNUM)
    {
        printf("pool_init() failed!\n");
        exit(1);
    }
    
    if(sql_connect())
    {
        printf("sql_connect() failed!\n");
        return -1;
    }
    
    while(1)
    {
        if(pipe_recv(&sockfd, sizeof(sockfd)) < 0)
        {
            printf("pipe_recv() failed!\n");
            continue;
        }
        
        if(ser_guest_module(sockfd))
        {
            printf("ser_guest_module() failed!\n");
            continue;
        }
    }
    
    pthread_join(pthid, NULL);
    
    return 0;
}

void *pthread_fun_sock(void *arg)
{
    int res;
    char **argv = (char **)arg;
    int port = atoi(argv[2]);

    res = open_sock_ser(argv[1], port);
    if(res < 0)
    {
        printf("open_sock_ser() failed!\n");
    }
    else
    {
        printf("PANIC! impossible!\n");
    }
    
    kill(getpid(), SIGINT);
    return NULL;
}

int ser_guest_module(int sockfd)
{
    int res;
    int *psockfd;
    
    psockfd = (int *)malloc(1 * sizeof(int));
    if(psockfd == NULL)
    {
        printf("malloc() psockfd failed!\n");
        return -1;
    }
    *psockfd = sockfd;
    
    res = pool_add_task (ser_process, psockfd); 
    if(res)
    {
        printf("pool_add_task() failed!\n");
        return -1;
    }
    
    return 0;
}

void *ser_process(void *arg)
{
    int sockfd = *(int *)arg;
    int i;
    int sum;
    int ch;
    int itb = -1;
    char tmp[TBNAMEMAX];
    char stb[TBNAMEMAX] = "tb";
    char dish[DISHMAX];
    
    printf("sockfd is %d\n", sockfd); 
    
    while(1)
    {
        if(tcp_recv(sockfd, &ch, sizeof(ch)) < 0)
        {
            printf("tcp_recv() ch failed!\n");
            break;
        }
        
        switch(ch)
        {
            case 1:
                if(tcp_recv(sockfd, &itb, sizeof(int)) < 0)
                {
                    printf("tcp_recv() itb failed!\n");
                    break;
                }
                table[itb] = 1;
                itoa(itb, tmp);
                strcpy(stb, "tb");
                strcat(stb, tmp);
                break;
            case 2:
                send_dish(sockfd, "tbMenu");
                
                bzero(dish, sizeof(dish));
                if(tcp_recv(sockfd, dish, sizeof(dish)) < 0)
                {
                    printf("tcp_recv() sum failed!\n");
                    break;
                }
                if(dish[0] == '0')
                {
                    break;
                }
                
                if(sql_insert_tb(stb, dish))
                {
                    printf("sql_insert_tb() failed!\n");
                }
                break;
            case 3:
                send_dish(sockfd, stb);
                break;
            case 4:
                send_dish(sockfd, stb);
                
                sum = sql_sum_tb(stb);
                if(sum > 1000000)  //为了修复一个已知的BUG.
                {
                    sum = 0;
                }
                if(tcp_send(sockfd, &sum, sizeof(sum)) < 0)
                {
                    printf("tcp_send() sum failed!\n");
                    break;
                }
                
                if(sql_delete_tb(stb))
                {
                    printf("sql_delete_tb() failed!\n");
                }
                table[itb] = 0;
                break;
            case 5:
                table[itb] = 0;
                close_sock_ser(sockfd);
                free(arg);
                return NULL;
                break;
            default:
                printf("Unknow Command: %d\n", ch);
                break;
        } //switch() end
    } //while() end
}

int send_dish(int sockfd, char *stb)
{
    DISHES dish;
    
    pthread_mutex_lock(&sql_mutex);
    if(sql_select(stb, &dish))
    {
        printf("sql_select() failed!\n");
        pthread_mutex_unlock(&sql_mutex);
        dish.id = OVER;
        if(tcp_send(sockfd, &dish, sizeof(dish)) < 0)
        {
            printf("tcp_send() dish over failed!\n");
        }
        return -1;
    }
    
    while(sql_select(NULL, &dish))
    {
        if(tcp_send(sockfd, &dish, sizeof(dish)) < 0)
        {
            printf("tcp_send() dish failed!\n");
        }
    }
    pthread_mutex_unlock(&sql_mutex);
    
    dish.id = OVER;
    if(tcp_send(sockfd, &dish, sizeof(dish)) < 0)
    {
        printf("tcp_send() dish over failed!\n");
    }
    return 0;
}

void ser_quit(int signo)
{
    exit(1);
}
 
