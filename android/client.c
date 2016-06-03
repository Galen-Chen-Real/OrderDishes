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

//数据库数据组成的结构体类型
typedef struct
{
    int id;
    char name[24];
    int price;
    int num;
}DISHES;

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
        printf("\tWELCOM\nTable-no:%d\n", itb + 1);
        printf("1.Choice Table\n2.Choice Dishes\n3.Watch\n4.Pay\n5.Quit\nPlease choice:");
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
                printf("\tChoice Table\n");
                for(i = 0; i < TABLEMAX; i++)
                {
                    printf("%d  ", i + 1);
                }
                printf("\n");
                while(1)
                {
                    printf("Please choice(negative return):");
                    scanf("%d", &ctb);
                    if(ctb > TABLEMAX || ctb == 0)
                    {
                        printf("Input error!\n");
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
                    printf("\nCongratulate choice table successed! Table-no:%d\n", itb + 1);
                }
                printf("Any input character return...\n");
                scanf("%s", &ret);
                break;
            case 2:
                if(itb < 0)
                {
                    printf("You have not selected table,Any input character return...\n");
                    scanf("%s", &ret);
                    break;
                }             
                if(tcp_send(sockfd, &ch, sizeof(int)) < 0)
                {
                    printf("tcp_send() ch failed!\n");
                    break;
                }
                
                memset(dish, '\0', sizeof(dish));
                memset(check_no, '\0', sizeof(check_no));
                system("clear");
                printf("\tChoice Dishes\n");
                printf("ID\tNAME\tPRICE\tLAVE\n");
                print_dish(sockfd);
                while(1)
                {
                    printf("Please choice(negative return):");
                    scanf("%s", no);
                    if(no[0] == '-')
                    {
                        break;
                    }
                    if(check(no))
                    {
                        printf("Input error!\n");
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
                    printf("You have not selected table,Any input character return...\n");
                    scanf("%s", &ret);
                    break;
                }  
                if(tcp_send(sockfd, &ch, sizeof(int)) < 0)
                {
                    printf("tcp_send() ch failed!\n");
                    break;
                }
                
                system("clear");
                printf("\tWatch\n");
                printf("ID\tNAME\tNUM\n");
                print_dish_np(sockfd);
                
                printf("Any input character return...\n");
                scanf("%s", &ret);
                break;
                
            case 4:
                if(itb < 0)
                {
                    printf("You have not selected table,Any input character return...\n");
                    scanf("%s", &ret);
                    break;
                }  
                if(tcp_send(sockfd, &ch, sizeof(int)) < 0)
                {
                    printf("tcp_send() ch failed!\n");
                    break;
                }
                
                system("clear");
                printf("\tPay\n");
                printf("ID\tNAME\tPRICE\tNUM\n");
                print_dish(sockfd);
                
                if(tcp_recv(sockfd, &sum, sizeof(sum)) < 0)
                {
                    printf("tcp_recv() sum failed!\n");
                    break;
                }
                printf("\tSUM : %d\n", sum);
                itb = -1;
                printf("Any input character return...\n");
                scanf("%s", &ret);
                break;
                
            case 5:
                cli_quit(SIGINT);
                break;
                
            default:
                printf("Input error! Any input character return...\n");
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

