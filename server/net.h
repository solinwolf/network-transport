#ifndef NET_H
#define NET_H
#include <openssl/ssl.h>
#include <openssl/err.h>
void setup();
void handle(char cmd,SSL *ssl);
void * process(int arg);
#endif

