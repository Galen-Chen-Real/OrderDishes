/*
 *        'thread_pool.c'
 *     (C) 2014.4 GordonChen
 *
 *     此处提供'thread_pool.h'中函数的实现. 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "thread_pool.h"

static THPOOL thpool;

static void *thread_routine(void *arg);

int pool_init(int thread_num)
{
    int i;
    int res;

    if(thread_num <= 0 || thpool.status == START)
    {
        return -1;
    }
        
    pthread_mutex_init(&thpool.mymutex, NULL);
    pthread_cond_init(&thpool.mycond, NULL);
    
    if((thpool.task_head = (POOLTASK *)malloc(1 * sizeof(POOLTASK))) == NULL)
    {
        perror("malloc() task_head failed");
        pthread_mutex_destroy(&thpool.mymutex);
        pthread_cond_destroy(&thpool.mycond);
        thpool.status = END;
        return -1;
    }
    thpool.task_head->fun = thpool.task_head->arg = thpool.task_head->next = NULL;
    
    if((thpool.pthreadid = (pthread_t *)malloc(thread_num * sizeof(pthread_t))) == NULL)
    {
        perror("malloc() pthreadid failed");
        pthread_mutex_destroy(&thpool.mymutex);
        pthread_cond_destroy(&thpool.mycond);
        free(thpool.task_head);
        thpool.task_head = NULL;
        thpool.status = END;
        return -1;
    }
    
    thpool.thread_num = thread_num;
    for(i = 0; i < thread_num; i++)
    {
        res = pthread_create(thpool.pthreadid + i, NULL, (THFUN)thread_routine, NULL);
        if(res)
        {
            perror("pthread_create() failed");
            pthread_mutex_destroy(&thpool.mymutex);
            pthread_cond_destroy(&thpool.mycond);
            free(thpool.task_head);
            thpool.task_head = NULL;
            free(thpool.pthreadid);
            thpool.pthreadid = NULL;
            thpool.status = END;
            return -1;
        }
    }
    
    thpool.task_num = 0;
    thpool.status = START;
    
    return i;
}

static void *thread_routine(void *arg)
{
    POOLTASK *tmp;
    printf ("starting thread 0x%x\n", pthread_self ()); 
    while(1)
    {
        pthread_mutex_lock(&thpool.mymutex);
        while(thpool.task_num == 0 && thpool.status != END)
        {
            printf ("thread 0x%x is waiting\n", pthread_self ());
            pthread_cond_wait(&thpool.mycond, &thpool.mymutex);
        }
        
        if(thpool.status == END)
        {
            pthread_mutex_unlock(&thpool.mymutex);
            printf ("thread 0x%x will exit\n", pthread_self ()); 
            pthread_exit(NULL);
        }
        
        tmp = thpool.task_head->next;
        if(tmp == NULL)
        {
            printf("PANIC! tmp == NULL\n");
            pthread_mutex_unlock(&thpool.mymutex);
            pthread_exit(NULL);
        }
        
        thpool.task_head->next = tmp->next;
        thpool.task_num--;
        printf ("thread 0x%x is starting to work\n", pthread_self ()); 
        pthread_mutex_unlock(&thpool.mymutex);
        
        (*(tmp->fun))(tmp->arg);
        free(tmp);
        tmp = NULL;
    }
    pthread_mutex_exit(NULL);
}

int pool_destroy(void)
{
    int i;
    POOLTASK *tmp;
    
    if(thpool.status != START)
    {
        return -1;
    }
    thpool.status = END;
    
    pthread_cond_broadcast(&thpool.mycond);
    
    for(i = 0; i < thpool.thread_num; i++)
    {
        pthread_join(thpool.pthreadid[i], NULL);
    }
    
    pthread_mutex_destroy(&thpool.mymutex);
    pthread_cond_destroy(&thpool.mycond);
    
    free(thpool.pthreadid);
    thpool.pthreadid = NULL;
    
    tmp = thpool.task_head->next;
    while(tmp != NULL)
    {
        thpool.task_head->next = tmp->next;
        free(tmp);
        tmp = thpool.task_head->next;
    }
    free(thpool.task_head);
    thpool.task_head = NULL;
    
    thpool.thread_num = 0;
    thpool.task_num = 0;
    
    return 0;
}

int pool_add_task(THFUN fun, void *arg)
{
    POOLTASK *tmp, *new_task;
    
    if(thpool.status != START)
    {
        return -1;
    }
    
    new_task = (POOLTASK *)malloc(1 * sizeof(POOLTASK));
    if(new_task == NULL)
    {
        perror("malloc() new_task failed");
        return -1;
    }
    
    new_task->fun = fun;
    new_task->arg = arg;
    new_task->next = NULL;  // :(
    
    pthread_mutex_lock(&thpool.mymutex);
    tmp = thpool.task_head;
    while(tmp->next != NULL)
    {
        tmp = tmp->next;
    }
    tmp->next = new_task;
    thpool.task_num++;
    pthread_mutex_unlock(&thpool.mymutex);
    
    pthread_cond_signal(&thpool.mycond);
    
    return 0;
}

