
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
int sockfdnew;
struct sockaddr_in seraddr;
struct sockaddr_in cliaddr;
SSL_CTX *ctx;
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
