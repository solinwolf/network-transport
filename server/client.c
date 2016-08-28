#include <strings.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sqlite3.h> 

int sockfd;
struct sockaddr_in serveraddr;
#define PORT 3333
SSL_CTX *ctx;
SSL *ssl;

void clink(char *ipaddr)
{
    int flag;
    //创建套接字 
    sockfd = socket(AF_INET,SOCK_STREAM,0);

    //链接到服务器
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr(ipaddr);
    if(flag = connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))==-1)
    {
	perror("connect:");
	exit(0);
    }

    //创建ssl 套接字
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl,sockfd);
    SSL_connect(ssl);
    return ;

    //建立ssl套接字
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl,sockfd);
    SSL_connect(ssl);
}

void uploadFile(char *filename)
{
    int fd;
    char cmd='U';
    char buf[1024];
    int count=0;
    int namesize=strlen(filename);
    struct stat fileStat;
    //打开文件
    if((fd = open(filename,O_RDONLY))<0)
    {
	perror("open:");
	exit(0);
    }
    //发送文件属性
  //  write(sockfd,&cmd,1);
  //  write(sockfd,(void*)&namesize,4);
   // write(sockfd,filename,namesize);
    SSL_write(ssl,&cmd,1);
    SSL_write(ssl,(void*)&namesize,4);
    SSL_write(ssl,filename,namesize);
    if(stat(filename,&fileStat)==-1)
	return;
    //write(sockfd,(void*)&(fileStat.st_size),4);
    SSL_write(ssl,(void*)&(fileStat.st_size),4);
    //发送文件内容
    while((count=read(fd,(void *)buf,1024))>0)
    {
	SSL_write(ssl,&buf,count);	
    }		
	
    close(fd);
}

void downloadFile(char *filename)
{
    int fd;
    char cmd='D';
    char buf[1024];
    int count=0;
    int namesize=strlen(filename);
    int filesize=0;
    int tmpsize=0;
    //打开文件

    //文件属性
    //write(sockfd,&cmd,1);
    //write(sockfd,(void*)&namesize,4);
    //write(sockfd,filename,namesize);
    //read(sockfd,&filesize,4);
    SSL_write(ssl,&cmd,1);
    SSL_write(ssl,(void*)&namesize,4);
    SSL_write(ssl,filename,namesize);
    if((fd = open(filename,O_RDWR|O_CREAT,0777))<0)
    {
	perror("open:");
	exit(0);
    }

    SSL_read(ssl,&filesize,4);
   // while((count=SSL_read(ssl,(void*)buf,1024))>0)
    //read file propertes
    //write(sockfd,&cmd,1);
    //write(sockfd,(void*)&namesize,4);
    //write(sockfd,filename,namesize);
    //read(sockfd,&filesize,4);
    //读取文件内容
    while((count=SSL_read(ssl,(void*)&buf,1024))>0)
    {
	write(fd,&buf,count);
	tmpsize += count;
	if(tmpsize==filesize)
	    break;
    }
    close(fd);
}
void quitS()
{
    char cmd='Q';
    SSL_write(ssl,&cmd,1);
    system("clear");
    SSL_shutdown(ssl);
    SSL_free(ssl);
    

    //4. disconnect 
    close(sockfd);
    SSL_CTX_free(ctx);
    exit(0);
}
void menu() 
{
    char command;
    char file_u[30];
    char file_d[30];
    char c;
    while(1)
    {
	printf("\n     /***-------------------- 1. Updload file----------------***/\n");
        printf("       /***-------------------- 2. Download file---------------***/\n");
        printf("       /***-------------------- 3. Quit App--------------------***/\n");
        printf("Input command:");
        command=getchar();
        switch(command)
	{
	    case '1':
   	    {
		printf("Upload file:");
		while ((c=getchar()) != '\n' && c != EOF);
		fgets(file_u,30,stdin);
		file_u[strlen(file_u)-1]='\0';
		uploadFile(file_u);
	    }
	    break;

  	    case '2':
	    {
		printf("Download file:");
		while ((c=getchar()) != '\n' && c != EOF);
		fgets(file_d,30,stdin);
   		file_d[strlen(file_d)-1] = '\0';
		downloadFile(file_d);
	    }
	    break;
		
	    case '3':
  		printf("Quiting....\n");
	//	while ((c=getchar()) != '\n' && c != EOF);
        	quitS();
		
		break;
	
	    default:
		printf("Input right command...");
		break;
	}
    }
}

char passwd_d[10];

static int callback(void *NotUsed, int argc, char **argv, char **azColName) 
{ 
      int i; 
     for(i=0; i<argc; i++) 
     { 
         strcpy(passwd_d,argv[i]);
     } 
     printf("\n"); 
    return 0;

} 

int login()
{
     char username[10];
     char passwd[10];
     sqlite3 *db;
     char sql[50];
     
     int success;
     
     printf("User name: ");
     scanf("%s",username);
     
     printf("Password: ");
     scanf("%s",passwd);
     getchar();
     
     
     sprintf(sql, "select passwd from tb0 where name='%s';",username); 
     
     sqlite3_open("user.db", &db); 
     
     sqlite3_exec(db, sql, callback, 0, NULL);
     
     sqlite3_close(db); 
     
     
     
     success = strcmp(passwd,passwd_d);
     	
     return success;	
}

int main(int argc,char* argv[])
{
    //检查参数
    if(argc!=2)
    {
         printf("usage: ./client 192.168.1.100(ip address)\n");
	 exit(0);
    }
	//登陆到客户端系统
    if (login()!=0)
    {
         printf("wrong username or password!\n");
         exit(0);	
    }
    //OpenSSL初始化
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx=SSL_CTX_new(SSLv23_client_method());
    //链接到服务器
    clink(argv[1]);
    //执行用户的指令
    menu();

    return 0;
}

