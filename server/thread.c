
#include "thread.h"
#include <pthread.h> 
#include <stdio.h>
#include <malloc.h>
//定义一个指向线程池的全局指针
static Cthread_pool *pool = NULL; 
//每个线程执行的函数
//线程创建后执行此函数，并且在这个函数里调用用户的处理函数
void * thread_routine (void *arg) 
{ 
    printf ("starting thread 0x%x\n", pthread_self ()); 
    while (1) 
    { 
		//给互斥锁上锁
        pthread_mutex_lock (&(pool->queue_lock)); 
		//如果线程刚刚创建，，则会在这里等待任务的加入
        while (pool->cur_task_size == 0 && !pool->shutdown) 
        { 
            printf ("thread 0x%x is waiting\n", pthread_self ()); 
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock)); 
        } 

		//如果要销毁线程池，则退出本线程
        if (pool->shutdown) 
        { 

            pthread_mutex_unlock (&(pool->queue_lock)); 
            printf ("thread 0x%x will exit\n", pthread_self ()); 
            pthread_exit (NULL); 
        } 
		//当有任务加入线程池时，从这里开始执行任务
        printf ("thread 0x%x is starting to work\n", pthread_self ()); 
		//先从任务链表中取出一个任务
        pool->cur_task_size--; 
        Cthread_task *task = pool->queue_head; 
        pool->queue_head = task->next; 
        pthread_mutex_unlock (&(pool->queue_lock)); 

		//然后执行任务（调用任务加入线程池时指定的函数）
        (*(task->process)) (task->arg); 
		//将这个任务的结构释放掉
        free (task); 
        task = NULL; 
    } 
	//按理这里是不会执行到的
    pthread_exit (NULL); 
}
//线程池初始化函数
//本函数将创建含有max_thread_num 个线程的线程池，并且每个线程都在等待任务的到来
void pool_init (int max_thread_num) 
{ 
    int i = 0;
    //分配线程池结构
    pool = (Cthread_pool *) malloc (sizeof (Cthread_pool)); 
	//初始化互斥锁
    pthread_mutex_init (&(pool->queue_lock), NULL); 
	//初始化条件变量
    pthread_cond_init (&(pool->queue_ready), NULL); 
	//将任务链表指针置空
    pool->queue_head = NULL; 
	//初始化最大线程数目和当前任务个数
    pool->max_thread_num = max_thread_num; 
    pool->cur_task_size = 0; 
	//初始化线程池销毁状态
    pool->shutdown = 0; 
	//分配空间来存储所有线程的线程ID
    pool->threadid = (pthread_t *) malloc (max_thread_num * sizeof (pthread_t)); 
	//循环创建线程，并且每个线程都在执行同一个函数
    for (i = 0; i < max_thread_num; i++) 
    {  
        pthread_create (&(pool->threadid[i]), NULL, thread_routine, NULL); 
    } 
} 
//任务加入函数，将任务加入线程池
int pool_add_task (void *(*process) (int arg), int arg) 
{ 
	//创建一个新的任务，并将其用参数初始化
    Cthread_task *task = (Cthread_task *) malloc (sizeof (Cthread_task)); 
    task->process = process; 
    task->arg = arg; 
    task->next = NULL;
	//给互斥锁上锁
    pthread_mutex_lock (&(pool->queue_lock)); 
 	//将任务加入到已经存在的任务链表中
    Cthread_task *member = pool->queue_head; 
	//如果已经有任务存在，加入队列后
    if (member != NULL) 
    { 
        while (member->next != NULL) 
            member = member->next; 
        member->next = task; 
    } 
	//当已存在任务为0时，直接加入
    else 
    { 
        pool->queue_head = task; 
    } 
	//将任务个数加一
    pool->cur_task_size++; 
	//解锁
    pthread_mutex_unlock (&(pool->queue_lock)); 
    //发出条件变量信号，让线程知道已经有有任务可以执行
    pthread_cond_signal (&(pool->queue_ready)); 
    
    return 0; 
}

