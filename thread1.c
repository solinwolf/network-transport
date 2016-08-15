

//任务结构，每个任务占一项
typedef struct task{
	//回调函数，任务运行时调用此函数
	void *(*process)(void* arg);
    //回调函数的参数
	void *arg;
    //
	struct task *next;

}CThread_task;


typedef struct{
	//互斥锁
    pthread_mutex_t queue_lock; 
    pthread_cond_t queue_ready; 
	//链表结构，链接所有等待任务
	CThread_task *queue_head;
	//销毁线程标志
    int shutdown;
	pthread_t *threadid;
	//线程池允许活动任务数目
    int max_thread_size;
	//当前等待队列的任务数目
	int cur_queue_size;

}CThread_pool;
static CThread_pool *pool=NULL;

void* thread_routine(void* arg);

void pool_init(int max_thread_num){
    pool=(CThread_pool*)malloc(sizeof(CThread_pool));

	pthread_mutex_init(&(pool->queue_head));
	pthread_cond_init(%(pool->queue_ready));
	
	pool->max_thread_size = max_thread_num;
	pool->cur_queue_size = 0;
	pool->shutdown = 0;
	pool->threadid = (pthread_t*)malloc(sizeof(pthread_t)*max_thread_num);
	int i=0;
	for(i=0;i<max_thread_num;i++){
		pthread_create(&(pool->threadid[i]),NULL,thread_routine,NULL);
	}
}

int pool_add_task(void* (*process)(void* arg),void* arg){
    //创建新的任务，并将其用参数初始化
	CThread_task *task = (CThread_task*)malloc(sizeof(CThread_task));
	task->process = process;
	task->arg = arg;
	task->next = NULL;
	//将新任务加入线程池的链表中
    pthread_mutex_lock (&(pool->queue_lock));
	CThread_task* member = pool->queue_head;
	if(member!=NULL){
		while(member->next!=NULL)
			member = member->next;
		member->next = task;	
	}
	else{
		pool->queue_head = task;
	}	 
	task->cur_queue_size++;
    pthread_mutex_unlock (&(pool->queue_lock)); 
    pthread_cond_signal (&(pool->queue_ready)); 
}

void *myprocess(void* arg){
    printf("threadid %x is working on task:%d\n",pthread_self(),*(int*)arg);
	sleep(1);
	return NULL;
}

void *thread_routine(void* arg){
	printf("starting thread 0x%x\n",pthread_self());
    while(1){
		pthread_mutex_lock(&(pool->queue_lock));
		while(pool->cur_queue_size == 0 && !pool->shutdown){
			printf ("thread 0x%x is waiting\n", pthread_self ()); 
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock)); 
		}
		if(pool->shutdown){
            pthread_mutex_unlock (&(pool->queue_lock)); 
            printf ("thread 0x%x will exit\n", pthread_self ()); 
            pthread_exit (NULL); 
		}
		printf("thread 0x%x is starting to work...\n",pthread_self());
		pool->cur_queue_size--;
		CThread_task* task = pool->queue_head;
		pool->queue_head = task->next;
        pthread_mutex_unlock (&(pool->queue_lock)); 

		(*(task->process))(task->arg);
		free(task);
		task=NULL;
	} 
	pthread_exit (NULL);   
}
int main(int argc,char *argv[]){
    int i=0;
	pool_init(5);
	int *worknum = (int*)malloc(sizeof(int)*10);
	for(i=0;i<10;i++){
		worknum[i]=i;
		pool_add_work(myprocess,&(worknum[i]));
	}
    sleep (5); 
    pool_destroy (); 
    free (workingnum); 
    return 0; 
}














