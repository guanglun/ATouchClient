#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include <pthread.h>
#include <unistd.h>

#include "adb.h"
#include "input.h"
#include "log.h"
#include "version.h"

#define LOG_HEAD "MAIN"

#ifdef __linux__
void Stop(int signo)
{
    input_exit();
    //LOG("oops! stop!!!\r\n");
    _exit(0);
}
#endif

static void usage(char *name)
{
    printf("ATouchClient %s\r\n", VERSION);
    fprintf(stderr, "Usage: %s [-s] [-h] [-v]\n", name);
    fprintf(stderr, "    -s: select device manually\n");
    fprintf(stderr, "    -v: print version\n");
    fprintf(stderr, "    -h: print help\n");
}

int main(int argc, char *argv[])
{
    int ret;
    int c;
    int select = 0;

    do
    {
        c = getopt(argc, argv, "shv");
        if (c == EOF)
            break;
        switch (c)
        {
        case 's':
            select = 1;
            break;
        case 'v':
            printf("ATouchClient %s\r\n", VERSION);
            exit(1);
            break;
        case 'h':
            usage(argv[0]);
            exit(1);
            break;
        }
    } while (1);

    LOG("\r\n==============start=============\r\n");
    LOG("\r\n==============%s=============\r\n", VERSION);
    LOG("ATouchClient is runing\r\n");

#ifdef __linux__
    signal(SIGINT, Stop);
#endif

    ret = input_init(select);
    if (ret < 0)
    {
        LOG("input init fail,exit\r\n");
        goto exit;
    }

    ret = adb_init();
    if (ret < 0)
    {
        LOG("adb init fail,exit\r\n");
        goto exit;
    }

    while (1)
    {

        send_status();
        sleep(1);
    }

exit:
    LOG("==============exit=============\r\n");
    return 0;
}