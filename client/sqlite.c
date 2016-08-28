
#include "sqlite.h"
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
