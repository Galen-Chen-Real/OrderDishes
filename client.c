/*
 *        'client.c'
 *     (C) 2014.4 GordonChen
 *
 *    点菜系统客户端: Usage: ./client ip port
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#include "sock.h"
#include "sql.h"

#define NOMAX 5    //点菜时编号最大长度
#define DISHMAX 64 //菜编号形如'1,2...'组合最大长度
#define TABLEMAX 10//桌子总数
#define OVER -1    //标志结束
#define IDMAX 32   //菜单的最大菜数

void cli_process(int sockfd);  //和ser_process()对应
int check(char *no);           //检查用户输入是否正确;正确返回0,不正确返回-1.
int print_dish(int sockfd);    //接收并打印菜单
int print_dish_np(int sockfd); //同上
void cli_quit(int signo);      //SIGINT信号触发退出

static int globe_sockfd;       //为了更好的退出 :)
static int check_no[IDMAX];    //check()

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Usage: %s ip port\n", argv[0]);
        return 0;
    }
    int port = atoi(argv[2]);
    int sockfd;
    signal(SIGINT, cli_quit);
    
    if((sockfd = open_sock_cli(argv[1], port)) < 0)
    {
        printf("open_sock_cli() failed!\n");
        return -1;
    }
    globe_sockfd = sockfd;
    
    cli_process(sockfd);
    
    return 0;
}

void cli_process(int sockfd)
{
    int i;
    int ch;
    char ret[4];
    int sum;
    int itb = -1;;
    int ctb;
    char no[NOMAX];
    char dish[DISHMAX];
    
    printf("sockfd = %d\n", sockfd);
    
    while(1)
    {
        system("clear");
        printf("\t欢迎光临!\n餐桌编号:%d\n", itb + 1);
        printf("1.选桌子\n2.点菜\n3.查看\n4.结算\n5.退出\n请选择:");
        scanf("%d", &ch);
        
        switch(ch)
        {
            case 1:
                if(tcp_send(sockfd, &ch, sizeof(int)) < 0)
                {
                    printf("tcp_send() ch failed!\n");
                    break;
                }
                
                system("clear");
                printf("\t选桌子\n");
                for(i = 0; i < TABLEMAX; i++)
                {
                    printf("%d  ", i + 1);
                }
                printf("\n");
                while(1)
                {
                    printf("请选择(负数返回):");
                    scanf("%d", &ctb);
                    if(ctb > TABLEMAX || ctb == 0)
                    {
                        printf("输入错误!\n");
                        continue;
                    }
                    if(ctb < 0)
                    {
                        itb = -1;
                    }
                    else
                    {
                        itb = ctb - 1;
                    }
                    break;
                }
                if(tcp_send(sockfd, &itb, sizeof(int)) < 0)
                {
                    printf("tcp_send() itb failed!\n");
                    break;
                }
                if(itb + 1 > 0)
                {
                    printf("\n恭喜选桌成功! 餐桌编号:%d\n", itb + 1);
                }
                printf("输入任意字符返回...\n");
                scanf("%s", &ret);
                break;
            case 2:
                if(itb < 0)
                {
                    printf("您尚未选定餐桌,输入任意字符返回...\n");
                    scanf("%s", &ret);
                    break;
                }             
                if(tcp_send(sockfd, &ch, sizeof(int)) < 0)
                {
                    printf("tcp_send() ch failed!\n");
                    break;
                }
                
                bzero(dish, sizeof(dish));
                bzero(check_no, sizeof(check_no));
                system("clear");
                printf("\t点菜\n");
                printf("ID\tNAME\tPRICE\tLAVE\n");
                print_dish(sockfd);
                while(1)
                {
                    printf("请选择(负数返回):");
                    scanf("%s", no);
                    if(no[0] == '-')
                    {
                        break;
                    }
                    if(check(no))
                    {
                        printf("输入错误!\n");
                        continue;
                    }
                    strcat(no, ",");
                    strcat(dish, no);
                }
                if(!strlen(dish))
                {
                    dish[0] = '0';
                }
                else
                {
                    dish[strlen(dish) - 1] = '\0';
                }
                
                if(tcp_send(sockfd, dish, strlen(dish)) < 0)
                {
                    printf("tcp_send() dish failed!\n");
                }
                break;
                
            case 3:
                if(itb < 0)
                {
                    printf("您尚未选定餐桌,输入任意字符返回...\n");
                    scanf("%s", &ret);
                    break;
                }  
                if(tcp_send(sockfd, &ch, sizeof(int)) < 0)
                {
                    printf("tcp_send() ch failed!\n");
                    break;
                }
                
                system("clear");
                printf("\t查看\n");
                printf("ID\tNAME\tNUM\n");
                print_dish_np(sockfd);
                
                printf("输入任意字符返回...\n");
                scanf("%s", &ret);
                break;
                
            case 4:
                if(itb < 0)
                {
                    printf("您尚未选定餐桌,输入任意字符返回...\n");
                    scanf("%s", &ret);
                    break;
                }  
                if(tcp_send(sockfd, &ch, sizeof(int)) < 0)
                {
                    printf("tcp_send() ch failed!\n");
                    break;
                }
                
                system("clear");
                printf("\t结算\n");
                printf("ID\tNAME\tPRICE\tNUM\n");
                print_dish(sockfd);
                
                if(tcp_recv(sockfd, &sum, sizeof(sum)) < 0)
                {
                    printf("tcp_recv() sum failed!\n");
                    break;
                }
                printf("\tSUM : %d\n", sum);
                itb = -1;
                printf("输入任意字符返回...\n");
                scanf("%s", &ret);
                break;
                
            case 5:
                cli_quit(SIGINT);
                break;
                
            default:
                printf("输入有误! 输入任意字符返回...\n");
                scanf("%s", &ret);
                break;
        } // switch(ch) end
    } // while(1) end
}

int print_dish(int sockfd)
{
    DISHES dish;
    
    while(1)
    {
        if(tcp_recv(sockfd, &dish, sizeof(dish)) < 0)
        {
            printf("tcp_recv() dish failed!\n");
        }
        if(dish.id == OVER)
        {
            break;
        }
        if(dish.num > 0)
        {
            printf("%d\t%s\t%d\t%d\n", dish.id, dish.name, dish.price, dish.num);
            check_no[dish.id] = 1;
        }
    }
    
    return 0;
}

int print_dish_np(int sockfd)
{
    DISHES dish;
    
    while(1)
    {
        if(tcp_recv(sockfd, &dish, sizeof(dish)) < 0)
        {
            printf("tcp_recv() dish failed!\n");
        }
        if(dish.id == OVER)
        {
            break;
        }
        printf("%d\t%s\t%d\n", dish.id, dish.name, dish.num);
        check_no[dish.id] = 1;
    }
    
    return 0;
}

int check(char *no)
{
    return check_no[atoi(no)] ? 0 : -1;
}

void cli_quit(int signo)
{
    int ch = 5;
    if(tcp_send(globe_sockfd, &ch, sizeof(int)) < 0)
    {
        printf("tcp_send() ch failed!\n");
    }
    exit(1);
}

