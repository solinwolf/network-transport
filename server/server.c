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


#define PORT 3333
int sockfd;
int sockfdnew;
struct sockaddr_in seraddr;
struct sockaddr_in cliaddr;


void setup()
{
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

void handle(char cmd)
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
	    read(sockfdnew,&namesize,4);
   	    read(sockfdnew,(void*)filename,namesize);
	    filename[namesize] = '\0';
	    if((fd=open(filename,O_RDWR|O_CREAT,0777))<0)
	    {
		perror("Open:");    
            }
	    read(sockfdnew,&filesize,4);
	    while((count=read(sockfdnew,(void *)buf,1024))>0)
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
	    read(sockfdnew,&namesize,4);
	    read(sockfdnew,(void*)filename,namesize);
	    filename[namesize] = '\0';
	    if((fd=open(filename,O_RDONLY))<0)
	    {
		perror("Open:");    
            }
	    if(stat(filename,&fstat)<0)
		return;
	    write(sockfdnew,&fstat.st_size,4);
            
    	    while((count=read(fd,&buf,1024))>0)
	    {
		write(sockfdnew,&buf,count);
  	    }	
  	    close(fd);
     	    printf("Done...\n");
	}
	break;	    
    }
}

int main()
{
    socklen_t addrlen=sizeof(cliaddr);
    char cmd;
    setup();
    while(1)
    {
	if((sockfdnew=accept(sockfd,(struct sockaddr*)&cliaddr,&addrlen))<0)
	{
	    perror("accept:");
	    exit(-1);
	}
        printf("%s connected\n",inet_ntoa(cliaddr.sin_addr));
        while(1)
	{
	    read(sockfdnew,&cmd,1);
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
        close(sockfdnew);
    }
    close(sockfd);
    return 0;
}




















