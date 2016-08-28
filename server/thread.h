#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>
//定义线程池任务结构
typedef struct task 
{ 
	//任务处理函数
    void *(*process) (int arg); 
	//处理函数的参数
    int arg;
	//指向下一个任务的指针
    struct task *next; 
} Cthread_task; 

//定义线程池结构
typedef struct 
{ 
	//定义互斥锁和条件变量
    pthread_mutex_t queue_lock; 
    pthread_cond_t queue_ready; 

	//指向任务链表的指针
    Cthread_task *queue_head; 
	
	//线程池销毁的标志
    int shutdown; 
	//指向线程ID的指针
    pthread_t *threadid; 
    
	//线程池线程个数
    int max_thread_num; 
    
	//当前线程池中任务的个数
    int cur_task_size; 

} Cthread_pool; 

void * thread_routine (void *arg) ;
void pool_init (int max_thread_num) ;
int pool_add_task (void *(*process) (int arg), int arg) ;


#endif
