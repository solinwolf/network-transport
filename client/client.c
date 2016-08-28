
#include "net.h"
#include "sqlite.h"
extern SSL_CTX *ctx;

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

