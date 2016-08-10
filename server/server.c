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
SSL *ssl;

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
    while(1)
    {
		if((sockfdnew=accept(sockfd,(struct sockaddr*)&cliaddr,&addrlen))<0)
		{
			perror("accept:");
			exit(-1);
		}
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
    }
    close(sockfd);
    return 0;
}




















