#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <pthread.h> 


typedef struct task 
{ 
    void *(*process) (void *arg); 
    void *arg;
    struct task *next; 
} Cthread_task; 


/*�̳߳ؽṹ*/ 
typedef struct 
{ 
    pthread_mutex_t queue_lock; 
    pthread_cond_t queue_ready; 

    /*����ṹ���̳߳������еȴ�����*/ 
    Cthread_task *queue_head; 

    /*�Ƿ������̳߳�*/ 
    int shutdown; 
    pthread_t *threadid; 
    
    /*�̳߳����߳���Ŀ*/ 
    int max_thread_num; 
    
    /*��ǰ�ȴ���������*/ 
    int cur_task_size; 

} Cthread_pool; 

static Cthread_pool *pool = NULL; 

void *thread_routine (void *arg); 

void pool_init (int max_thread_num) 
{ 
    int i = 0;
    
    pool = (Cthread_pool *) malloc (sizeof (Cthread_pool)); 

    pthread_mutex_init (&(pool->queue_lock), NULL); 
    /*��ʼ����������*/
    pthread_cond_init (&(pool->queue_ready), NULL); 

    pool->queue_head = NULL; 

    pool->max_thread_num = max_thread_num; 
    pool->cur_task_size = 0; 

    pool->shutdown = 0; 

    pool->threadid = (pthread_t *) malloc (max_thread_num * sizeof (pthread_t)); 

    for (i = 0; i < max_thread_num; i++) 
    {  
        pthread_create (&(pool->threadid[i]), NULL, thread_routine, NULL); 
    } 
} 



/*���̳߳��м�������*/ 
int pool_add_task (void *(*process) (void *arg), void *arg) 
{ 
    /*����һ��������*/ 
    Cthread_task *task = (Cthread_task *) malloc (sizeof (Cthread_task)); 
    task->process = process; 
    task->arg = arg; 
    task->next = NULL;

    pthread_mutex_lock (&(pool->queue_lock)); 
    /*��������뵽�ȴ�������*/ 
    Cthread_task *member = pool->queue_head; 
    if (member != NULL) 
    { 
        while (member->next != NULL) 
            member = member->next; 
        member->next = task; 
    } 
    else 
    { 
        pool->queue_head = task; 
    } 

    pool->cur_task_size++; 
    pthread_mutex_unlock (&(pool->queue_lock)); 
    
    pthread_cond_signal (&(pool->queue_ready)); 
    
    return 0; 
} 



/*�����̳߳أ��ȴ������е����񲻻��ٱ�ִ�У������������е��̻߳�һֱ 
����������������˳�*/ 
int pool_destroy () 
{ 
    if (pool->shutdown) 
        return -1;/*��ֹ���ε���*/ 
    pool->shutdown = 1; 

    /*�������еȴ��̣߳��̳߳�Ҫ������*/ 
    pthread_cond_broadcast (&(pool->queue_ready)); 

    /*�����ȴ��߳��˳�������ͳɽ�ʬ��*/ 
    int i; 
    for (i = 0; i < pool->max_thread_num; i++) 
        pthread_join (pool->threadid[i], NULL); 
    free (pool->threadid); 

    /*���ٵȴ�����*/ 
    Cthread_task *head = NULL; 
    while (pool->queue_head != NULL) 
    { 
        head = pool->queue_head; 
        pool->queue_head = pool->queue_head->next; 
        free (head); 
    } 
    /*���������ͻ�����Ҳ����������*/ 
    pthread_mutex_destroy(&(pool->queue_lock)); 
    pthread_cond_destroy(&(pool->queue_ready)); 
     
    free (pool); 
    /*���ٺ�ָ���ÿ��Ǹ���ϰ��*/ 
    pool=NULL; 
    return 0; 
} 



void * thread_routine (void *arg) 
{ 
    printf ("starting thread 0x%x\n", pthread_self ()); 
    while (1) 
    { 
        pthread_mutex_lock (&(pool->queue_lock)); 

        while (pool->cur_task_size == 0 && !pool->shutdown) 
        { 
            printf ("thread 0x%x is waiting\n", pthread_self ()); 
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock)); 
        } 

        /*�̳߳�Ҫ������*/ 
        if (pool->shutdown) 
        { 
            /*����break,continue,return����ת��䣬ǧ��Ҫ�����Ƚ���*/ 
            pthread_mutex_unlock (&(pool->queue_lock)); 
            printf ("thread 0x%x will exit\n", pthread_self ()); 
            pthread_exit (NULL); 
        } 

        printf ("thread 0x%x is starting to work\n", pthread_self ()); 

         
        /*�����������1����ȡ�������е�ͷԪ��*/ 
        pool->cur_task_size--; 
        Cthread_task *task = pool->queue_head; 
        pool->queue_head = task->next; 
        pthread_mutex_unlock (&(pool->queue_lock)); 

        /*���ûص�������ִ������*/ 
        (*(task->process)) (task->arg); 
        free (task); 
        task = NULL; 
    } 
    /*��һ��Ӧ���ǲ��ɴ��*/ 
    pthread_exit (NULL); 
}

void * myprocess (void *arg) 
{ 
    printf ("threadid is 0x%x, working on task %d\n", pthread_self (),*(int *) arg); 
    sleep (1);/*��Ϣһ�룬�ӳ������ִ��ʱ��*/ 
    return NULL; 
} 

int main (int argc, char **argv) 
{ 
    pool_init (3);/*�̳߳������������߳�*/ 
     
    /*���������Ͷ��10������*/ 
    int *workingnum = (int *) malloc (sizeof (int) * 10); 
    int i; 
    for (i = 0; i < 10; i++) 
    { 
        workingnum[i] = i; 
        pool_add_task (myprocess, &workingnum[i]); 
    } 
    /*�ȴ������������*/ 
    sleep (5); 
    /*�����̳߳�*/ 
    pool_destroy (); 

    free (workingnum); 
    
    return 0; 
}
