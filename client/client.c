#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <strings.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

int sockfd;
struct sockaddr_in serveraddr;
#define PORT 3333

void clink(char *ipaddr)
{
    int flag;
    //create socket 
    sockfd = socket(AF_INET,SOCK_STREAM,0);

    //connect to server
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr(ipaddr);
    if(flag = connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))==-1)
    {
	perror("connect:");
	exit(0);
    }
}

void uploadFile(char *filename)
{
    int fd;
    char cmd='U';
    char buf[1024];
    int count=0;
    int namesize=strlen(filename);
    struct stat fileStat;
    //open file
    if((fd = open(filename,O_RDONLY))<0)
    {
	perror("open:");
	exit(0);
    }
    //send file propertes
    write(sockfd,&cmd,1);
    write(sockfd,(void*)&namesize,4);
    write(sockfd,filename,namesize);
    
    if(stat(filename,&fileStat)==-1)
	return;
    write(sockfd,(void*)&(fileStat.st_size),4);

    //send file content
    while((count=read(fd,(void *)&buf,1024))>0)
    {
	write(sockfd,&buf,count);	
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
    //open file
    if((fd = open(filename,O_RDWR|O_CREAT,0777))<0)
    {
	perror("open:");
	exit(0);
    }
    //read file propertes
    write(sockfd,&cmd,1);
    write(sockfd,(void*)&namesize,4);
    write(sockfd,filename,namesize);
    read(sockfd,&filesize,4);
    
    //read file content
    while((count=read(sockfd,(void*)&buf,1024))>0)
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
    write(sockfd,&cmd,1);
    system("clear");
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
	printf("\n      /***-------------------- 1. Updload file----------------***/\n");
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


int main(int argc,char* argv[])
{
    //1. check args
    if(argc!=2)
    {
         printf("usage: ./client 192.168.1.100(ip address)\n");
	 exit(0);
    }
    //2. connect to server
    clink(argv[1]);
    //3. update and download
    menu();
    //4. disconnect 
    close(sockfd);
    return 0;
}

