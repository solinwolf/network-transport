
#include "net.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
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
#define PORT 3333
int sockfd;

struct sockaddr_in seraddr;

SSL_CTX *ctx;
SSL *ssl;
//做好TCP链接准备
void clink(char *ipaddr)
{	
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
	    perror("socket");	
	    exit(0);
	}
		
	memset(&seraddr,0,sizeof(seraddr));
	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = inet_addr(ipaddr);
	seraddr.sin_port = htons(PORT);
	
	if(connect(sockfd,(struct sockaddr* )&seraddr,sizeof(seraddr))==-1)
	{
	    perror("connect");
	    exit(0);
	}
	
	/*ŽŽœšSSL*/
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sockfd);
        SSL_connect(ssl);

	return ;
}
void uploadFile(char *filename)
{	
	int fd;
	char buf[1024];
	int count=0;
	int size = strlen(filename);
	char cmd = 'U';

	struct stat fstat;
		
	if((fd=open(filename,O_RDONLY))==-1)
	{
		perror("open: ");
		return;
	}
	
	//write(sockclient,&cmd,1);
	SSL_write(ssl,&cmd,1);
	
	//write(sockclient,(void *)&size,4);
	//write(sockclient,filename,size);
	SSL_write(ssl,(void *)&size,4);
	SSL_write(ssl,filename,size);
	
	if(stat(filename,&fstat)==-1)
		return;
	
	//write(sockclient,(void *)&(fstat.st_size),4);
	SSL_write(ssl,(void *)&(fstat.st_size),4);
	
	while((count=read(fd,(void *)buf,1024))>0)
	{
		//write(sockclient,&buf,count);	
		SSL_write(ssl,&buf,count);	
		
	}		
	
	close(fd);

}
//~~~~~~~~~~~~~~~~~~~~~~~ÏÂÔØÎÄŒþ~~~~~~~~~~~~~~~~~~~~~~~~~

void downloadFile(char *filename)
{
	int fd;
	char buf[1024];
	int count=0;
	int filesize = 0;
	int tmpsize = 0;
	int namesize = 0;
	char cmd = 'D';
	
	int size = strlen(filename);
	
	//write(sockclient,(void *)&cmd,1);
	SSL_write(ssl,(void *)&cmd,1);
	
	//write(sockclient,&size,4);
	//write(sockclient,filename,size);
	SSL_write(ssl,&size,4);
	SSL_write(ssl,filename,size);
	
	if((fd=open(filename,O_RDWR|O_CREAT,0777))<0)
	{
		perror("open error:\n");	
	}
	
	//read(sockclient,&filesize,4);	
	SSL_read(ssl,&filesize,4);	

	while((count=SSL_read(ssl,(void *)buf,1024))>0)
	{
		write(fd,&buf,count);
		tmpsize += count;
		if(tmpsize==filesize)
			break;	

	}
	
	close(fd);	
}

void quit()
{
	char cmd = 'Q';
	
	//write(sockclient,(void *)&cmd,1);
	SSL_write(ssl,(void *)&cmd,1);
	
	system("clear");
	
	/*SSLÍË³ö*/
	SSL_shutdown(ssl);
    	SSL_free(ssl);
    	close(sockfd);
    	SSL_CTX_free(ctx);
				
	exit(0);	
}

