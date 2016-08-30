#ifndef NET_H
#define NET_H
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


void clink(char *ipaddr);
void uploadFile(char *filename);
void downloadFile(char *filename);
void quit();

#endif
