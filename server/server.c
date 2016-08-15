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
<<<<<<< HEAD
#include <pthread.h> 
=======
>>>>>>> 11346f9bdab6a43b9a4fcb9c2a81802459910fff

#define PORT 3333
int sockfd;
int sockfdnew;
struct sockaddr_in seraddr;
struct sockaddr_in cliaddr;
SSL_CTX *ctx;
SSL *ssl;
<<<<<<< HEAD
typedef struct task 
{ 
    void *(*process) (int arg); 
    int arg;
    struct task *next; 
} Cthread_task; 

/*Ïß³Ì³Øœá¹¹*/ 
typedef struct 
{ 
    pthread_mutex_t queue_lock; 
    pthread_cond_t queue_ready; 

    /*ÁŽ±íœá¹¹£¬Ïß³Ì³ØÖÐËùÓÐµÈŽýÈÎÎñ*/ 
    Cthread_task *queue_head; 

    /*ÊÇ·ñÏú»ÙÏß³Ì³Ø*/ 
    int shutdown; 
    pthread_t *threadid; 
    
    /*Ïß³Ì³ØÖÐÏß³ÌÊýÄ¿*/ 
    int max_thread_num; 
    
    /*µ±Ç°µÈŽýµÄÈÎÎñÊý*/ 
    int cur_task_size; 

} Cthread_pool; 

Cthread_pool *pool = NULL; 

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

        /*Ïß³Ì³ØÒªÏú»ÙÁË*/ 
        if (pool->shutdown) 
        { 
            /*Óöµœbreak,continue,returnµÈÌø×ªÓïŸä£¬Ç§Íò²»ÒªÍüŒÇÏÈœâËø*/ 
            pthread_mutex_unlock (&(pool->queue_lock)); 
            printf ("thread 0x%x will exit\n", pthread_self ()); 
            pthread_exit (NULL); 
        } 

        printf ("thread 0x%x is starting to work\n", pthread_self ()); 

         
        /*ŽýŽŠÀíÈÎÎñŒõ1£¬²¢È¡³öÁŽ±íÖÐµÄÍ·ÔªËØ*/ 
        pool->cur_task_size--; 
        Cthread_task *task = pool->queue_head; 
        pool->queue_head = task->next; 
        pthread_mutex_unlock (&(pool->queue_lock)); 

        /*µ÷ÓÃ»Øµ÷º¯Êý£¬ÖŽÐÐÈÎÎñ*/ 
        (*(task->process)) (task->arg); 
        free (task); 
        task = NULL; 
    } 
    /*ÕâÒ»ŸäÓŠžÃÊÇ²»¿ÉŽïµÄ*/ 
    pthread_exit (NULL); 
}

void pool_init (int max_thread_num) 
{ 
    int i = 0;
    
    pool = (Cthread_pool *) malloc (sizeof (Cthread_pool)); 

    pthread_mutex_init (&(pool->queue_lock), NULL); 
    /*³õÊŒ»¯ÌõŒþ±äÁ¿*/
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

int pool_add_task (void *(*process) (int arg), int arg) 
{ 
    /*¹¹ÔìÒ»žöÐÂÈÎÎñ*/ 
    Cthread_task *task = (Cthread_task *) malloc (sizeof (Cthread_task)); 
    task->process = process; 
    task->arg = arg; 
    task->next = NULL;

    pthread_mutex_lock (&(pool->queue_lock)); 
    /*œ«ÈÎÎñŒÓÈëµœµÈŽý¶ÓÁÐÖÐ*/ 
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

=======
>>>>>>> 11346f9bdab6a43b9a4fcb9c2a81802459910fff

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
void * process(int arg)
{
	int tmp_fd = arg;
	SSL *ssl;
	char cmd;
	
	/*ŽŽœšSSL*/	
       	ssl = SSL_new(ctx);
       	SSL_set_fd(ssl, tmp_fd);
       	if (SSL_accept(ssl) == -1)
       	{
          	perror("accept");
          	close(tmp_fd);
       	}				
		
	while(1)
	{
		/*¶ÁÈ¡ÃüÁî*/
		//read(new_fd,&cmd,1);
		SSL_read(ssl,&cmd,1);

<<<<<<< HEAD
		if(cmd == 'Q')
		{
			/*SSLÍË³ö*/
			SSL_shutdown(ssl);
    			SSL_free(ssl);
			close(tmp_fd);	
			break;
		}
		else
		{
			handle(cmd,ssl);
		}
	}
	
	return NULL;
}
=======
>>>>>>> 11346f9bdab6a43b9a4fcb9c2a81802459910fff
int main(int argc,char* argv[])
{
    if(argc!=1)
	{
		printf("usage:./server \n");
		exit(0);
 	}
    socklen_t addrlen=sizeof(cliaddr);
    char cmd;
    setup();
    //创建线程池
	pool_init(5);
    
    while(1)
    {
		if((sockfdnew=accept(sockfd,(struct sockaddr*)&cliaddr,&addrlen))<0)
		{
			perror("accept:");
			exit(-1);
		}
<<<<<<< HEAD
        pool_add_task(process,sockfdnew);
=======
		printf("%s connected\n",inet_ntoa(cliaddr.sin_addr));
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl,sockfdnew);
   		if (SSL_accept(ssl) == -1)
   		{
      		perror("accept");
      		close(sockfdnew);
   		}	
		
		while(1)
		{
			SSL_read(ssl,&cmd,1);
			if(cmd == 'Q')
		    {
				printf("%s disconnected \n",inet_ntoa(cliaddr.sin_addr));
	   			close(sockfdnew);
				break;
			}
			else
			{
				handle(cmd);
			}
		}
		SSL_shutdown(ssl);
		SSL_free(ssl);
		close(sockfdnew);
>>>>>>> 11346f9bdab6a43b9a4fcb9c2a81802459910fff
    }
    close(sockfd);
    return 0;
}




















