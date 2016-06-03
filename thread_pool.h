/*
 *        'thread_pool.h'
 *     (C) 2014.4 GordonChen
 *
 *     此处提供线程池相关函数原形定义. 
 *
 */
#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

//用于status
#define START 01
#define END   02

//线程函数指针
typedef void *(*THFUN)(void *);

//任务节点类型
typedef struct task
{
    THFUN fun;
    void *arg;
    struct task *next;
}POOLTASK;

//线程池类型
typedef struct
{
    pthread_mutex_t mymutex;
    pthread_cond_t mycond;
    POOLTASK *task_head;
    pthread_t *pthreadid;
    int thread_num;
    int task_num;
    int status;
}THPOOL;

//初始化线程池;成功返回创建的线程数,失败返回-1.
int pool_init(int thread_num);
//销毁线程池;成功返回0,失败返回-1.
int pool_destroy(void);
//往线程池添加任务;成功返回0,失败返回-1.
int pool_add_task(THFUN fun, void *arg);

#endif

