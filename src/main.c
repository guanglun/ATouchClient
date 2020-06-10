#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h> 

#include "adb.h"
#include "input.h"
#include "log.h"
#include "version.h"

#define LOG_HEAD "MAIN"
int main(int argc, char *argv[])
{
    int ret;
    LOG("\r\n==============start=============\r\n");\
    LOG("\r\n==============%s=============\r\n",VERSION);
    LOG("ATouchClient is runing\r\n");

    ret = adb_init();
    if(ret < 0)
    {
        LOG("adb init fail,exit\r\n");
        goto exit;
    }

    ret = input_init();
    if(ret < 0)
    {
        LOG("input init fail,exit\r\n");
        goto exit;
    }

    while(1)
    {
        sleep(1000);
    }

exit:
    LOG("==============exit=============\r\n");
    return 0;    
}