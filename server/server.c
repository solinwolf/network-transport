#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h> 
#define PORT 3333
int sockfd;
int sockfdnew;
struct sockaddr_in seraddr;
struct sockaddr_in cliaddr;
SSL_CTX *ctx;
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
    //发出信号，让系统知道已经有如任务可以执行
    pthread_cond_signal (&(pool->queue_ready)); 
    
    return 0; 
}

//做好TCP链接准备
void setup()
{
    char pwd[100];
    //initial openssl
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_server_method());
    getcwd(pwd,100);
  	if(strlen(pwd)==1)
    		pwd[0]='\0';
    if (!SSL_CTX_use_certificate_file(ctx, strcat(pwd,"/cacert.pem"), SSL_FILETYPE_PEM))  
    {  
        ERR_print_errors_fp(stdout);  
        exit(1);  
    }  
    getcwd(pwd,100);
  	if(strlen(pwd)==1)
    		pwd[0]='\0';
    if (!SSL_CTX_use_PrivateKey_file(ctx, strcat(pwd,"/privkey.pem"), SSL_FILETYPE_PEM) )  
    {  
        ERR_print_errors_fp(stdout);  
        exit(1);  
    }  
    if (!SSL_CTX_check_private_key(ctx))  
    {  
        ERR_print_errors_fp(stdout);  
        exit(1);  
    }  
    //create socket 
    if((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1)
    {
		perror("socket:");
		exit(-1);
    }
    //bind
    bzero(&seraddr,sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(PORT);
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sockfd,(struct sockaddr*)&seraddr,sizeof(struct sockaddr))<0)
    {
		perror("bind:");
		exit(1);
    }
    //listen
    if(listen(sockfd,5)<0)
    {
		perror("listen:");
		exit(1);
    }
}
//解析指令函数，服务器根据不用的客户端指令进行不同的操作
void handle(char cmd,SSL *ssl)
{
    char filename[100];
    int namesize=0;
    int fd;
    int filesize=0;
    int count=0;
    int tmpsize=0;
    char buf[1024];
    struct stat fstat;
    switch(cmd)
    {
	//客户端发送“发送文件”指令，服务器将接收客户端的文件
	case 'U':
        {
	    printf("client %s sending file...\n",inet_ntoa(cliaddr.sin_addr));
	    SSL_read(ssl,&namesize,4);
   	    SSL_read(ssl,(void*)filename,namesize);
	    filename[namesize] = '\0';
	    if((fd=open(filename,O_RDWR|O_CREAT,0777))<0)
	    {
			perror("Open:");    
        }
	    SSL_read(ssl,&filesize,4);
	    while((count=SSL_read(ssl,(void *)buf,1024))>0)
	    {
			write(fd,&buf,count);
			tmpsize += count;
			if(tmpsize==filesize)
				break;	
	    }
     	close(fd);
	    printf("Done...\n");
	}
	break;
	//客户端发送“下载文件”指令，服务器将发送本地文件给客户端
	case 'D':
	{
	    printf("client %s fetching file...\n",inet_ntoa(cliaddr.sin_addr));
	    SSL_read(ssl,&namesize,4);
	    SSL_read(ssl,(void*)filename,namesize);
	    filename[namesize] = '\0';
	    if((fd=open(filename,O_RDONLY))<0)
	    {
			perror("Open:");    
        }
	    if(stat(filename,&fstat)<0)
			return;
	    SSL_write(ssl,&fstat.st_size,4);
            
    	while((count=read(fd,(void *)buf,1024))>0)
	    {
			SSL_write(ssl,&buf,count);
  	    }	
  	    close(fd);
     	printf("Done...\n");
	}
	break;	    
    }
}
//处理用户链接的函数
void * process(int arg)
{
	int tmp_fd = arg;
	SSL *ssl;
	char cmd;
	//建立ssl传输环境
   	ssl = SSL_new(ctx);
   	SSL_set_fd(ssl, tmp_fd);
   	if (SSL_accept(ssl) == -1)
   	{
      	perror("accept");
      	close(tmp_fd);
   	}				
		
	while(1)
	{
		//读回客户端发送的指令
		SSL_read(ssl,&cmd,1);
		//如果客户端发送退出指令Q,则服务器将端开此次链接
		if(cmd == 'Q')
		{

			SSL_shutdown(ssl);
    		SSL_free(ssl);
			close(tmp_fd);	
			break;
		}
		//如果客户端发送了“退出以外的命令”，则交给处理函数进一步判断命令
		else
		{
			handle(cmd,ssl);
		}
	}
	
	return NULL;
}
int main(int argc,char* argv[])
{
    if(argc!=1)
	{
		printf("usage:./server \n");
		exit(0);
 	}
    socklen_t addrlen=sizeof(cliaddr);
    char cmd;
    //建立socket 通信准备
    setup();
    //创建线程池，此线程池有5个工作线程
	pool_init(5);
    
    while(1)
    {
		//服务器一直等待客户端的链接
		if((sockfdnew=accept(sockfd,(struct sockaddr*)&cliaddr,&addrlen))<0)
		{
			perror("accept:");
			exit(-1);
		}

		printf("%s connected\n",inet_ntoa(cliaddr.sin_addr));
		//如果客户端链接成功，则将此任务交给线程池处理
        pool_add_task(process,sockfdnew);
    }
    close(sockfd);
    return 0;
}




















