
#ifndef SQLITE_H
#define SQLITE_H
#include <stdio.h>
#include <string.h>
#include <sqlite3.h> 
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
int login();

#endif

