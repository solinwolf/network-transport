

#include <pthread.h> 
#include "net.h"
#include "thread.h"
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
extern int sockfd;
extern int sockfdnew;
extern struct sockaddr_in seraddr;
extern struct sockaddr_in cliaddr;
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




















