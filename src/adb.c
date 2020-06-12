#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <pthread.h>
#include <unistd.h>

#include "adb.h"
#include "log.h"
#include "version.h"
#include "scmd.h"

#define LOG_HEAD "ADB"

#define ADB_FORWARD_PORT 5555

struct RUN_STATUS status;

char cmd_str[4096], recv_str[4096];
FILE *fp;
int sockfd;

static int adb_check_exist(void)
{
    LOG("check adb\r\n");
    sprintf(cmd_str, "adb version");
    LOG("%s\r\n", cmd_str);

    memset(recv_str, 0, sizeof(recv_str));
    fp = popen(cmd_str, "r");
    fgets(recv_str, sizeof(recv_str), fp);
    LOG("%s", recv_str);

    if (strstr(recv_str, "Android Debug Bridge") != NULL)
    {
        LOG("adb exist\r\n");
        return 0;
    }
    else
    {
        LOG("adb does not exist\r\n");
        return -1;
    }
    return -1;
}

static int adb_start_server(void)
{
    sprintf(cmd_str, "adb kill-server");
    LOG("%s\r\n", cmd_str);

    memset(recv_str, 0, sizeof(recv_str));
    fp = popen(cmd_str, "r");
    fgets(recv_str, sizeof(recv_str), fp);
    LOG("%s", recv_str);

    sprintf(cmd_str, "adb start-server");
    LOG("%s\r\n", cmd_str);

    memset(recv_str, 0, sizeof(recv_str));
    fp = popen(cmd_str, "r");
    fgets(recv_str, sizeof(recv_str), fp);
    LOG("%s", recv_str);

    memset(recv_str, 0, sizeof(recv_str));
    fgets(recv_str, sizeof(recv_str), fp);
    LOG("%s", recv_str);

    if (strstr(recv_str, "daemon started successfully") != NULL)
    {
        LOG("adb server start success\r\n");
        return 0;
    }
    else
    {
        LOG("adb server start fail\r\n");
        return -1;
    }
    return -1;
}

static int adb_start_remote_server(void)
{
    unsigned int count = 0, pid_count = 0;
    char search_flag = 0;
    char pid_str[9];

    sprintf(cmd_str, "adb shell \"ps -A | grep ATouchService\"");
    //LOG("%s\r\n", cmd_str);

    memset(recv_str, 0, sizeof(recv_str));
    fp = popen(cmd_str, "r");
    fgets(recv_str, sizeof(recv_str), fp);
    //LOG("%s", recv_str);

    if (strlen(recv_str) > 2)
    {
        for (; count < strlen(recv_str); count++)
        {
            switch (search_flag)
            {
            case 0:
                if (recv_str[count] == 0x20)
                {
                    search_flag = 1;
                }
                break;
            case 1:
                if (recv_str[count] >= '0' && recv_str[count] <= '9')
                {
                    pid_str[pid_count++] = recv_str[count];
                    search_flag = 2;
                }
                break;
            case 2:
                if (recv_str[count] >= '0' && recv_str[count] <= '9')
                {
                    pid_str[pid_count++] = recv_str[count];
                }
                else
                {
                    pid_str[pid_count++] = '\0';
                    search_flag = 3;
                    count = strlen(recv_str);
                }
                break;
            default:
                break;
            }
        }
    }

    if (search_flag == 3)
    {
        LOG("found pid:%s\r\n", pid_str);
        sprintf(cmd_str, "adb shell \"kill -s 9 %s\"", pid_str);

        memset(recv_str, 0, sizeof(recv_str));
        fp = popen(cmd_str, "r");
        fgets(recv_str, sizeof(recv_str), fp);
    }
    else
    {
        LOG("not found pid\r\n");
    }

    sprintf(cmd_str, "adb shell \"cp /mnt/sdcard/ATouch/ATouchService /data/local/tmp\"");
    system(cmd_str);

    // sprintf(cmd_str, "adb shell \"cp /mnt/sdcard/ATouch/ATouchService /data/local/tmp\"");

    // memset(recv_str, 0, sizeof(recv_str));
    // fp = popen(cmd_str, "r");
    // fgets(recv_str, sizeof(recv_str), fp);

#ifdef __linux__
    sprintf(cmd_str, "adb shell \"/data/local/tmp/ATouchService &\" &");
#endif

#ifdef _WIN32
    sprintf(cmd_str, "start /b adb shell \"/data/local/tmp/ATouchService &\"", pid_str);
#endif

    system(cmd_str);

    LOG("server start\r\n");

    return 0;
}

static int adb_version(void)
{
    sprintf(cmd_str, "adb version");

    memset(recv_str, 0, sizeof(recv_str));
    fp = popen(cmd_str, "r");
    fgets(recv_str, sizeof(recv_str), fp);
    LOG("%s", recv_str) ;
}

static int adb_devices(void)
{
    sprintf(cmd_str, "adb devices");
    //LOG("%s\r\n", cmd_str) ;

    memset(recv_str, 0, sizeof(recv_str));
    fp = popen(cmd_str, "r");
    fgets(recv_str, sizeof(recv_str), fp);
    //LOG("%s",recv_str);

    memset(recv_str, 0, sizeof(recv_str));
    fgets(recv_str, sizeof(recv_str), fp);
    //LOG("%s",recv_str);

    if (strlen(recv_str) > 1)
    {
        adb_start_remote_server();
        return 0;
    }

    return -1;
}

static int adb_forward(void)
{

    sprintf(cmd_str, "adb forward tcp:%d tcp:1989", ADB_FORWARD_PORT);
    //LOG("%s\r\n", cmd_str) ;

    memset(recv_str, 0, sizeof(recv_str));
    fp = popen(cmd_str, "r");
    fgets(recv_str, sizeof(recv_str), fp);
}

static int adb_socket(void)
{
    char recv_buf[1024];
    int recv_size;

#ifdef _WIN32
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(1, 1), &wsadata) == SOCKET_ERROR)
    {
        LOG("WSAStartup() fail\n");
        exit(0);
    }
#endif

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return -1;
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(ADB_FORWARD_PORT);

#ifdef __linux__
    inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr);
#endif

#ifdef _WIN32
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif

    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        //LOG("socket connect fail\r\n");
        return -1;
    }
    
    status.is_adb_connect = S_CONNECT;

    LOG("socket connect %d\r\n",status.is_adb_connect);

    while (1)
    {
#ifdef __linux__
        recv_size = read(sockfd, recv_buf, sizeof(recv_buf));
#endif

#ifdef _WIN32
        recv_size = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
#endif
        if (recv_size < 0)
        {
            break;
        }
        else if (recv_size > 0)
        {
            recv_buf[recv_size] = '\0';
            LOG("%s\r\n", recv_buf);
        }
    }

    LOG("socket disconnect\r\n");
    //     if(write(STDOUT_FILENO, recv_buf, recv_size) != recv_size)
    //     {
    // 		perror("write error");
    //     }

#ifdef __linux__
    close(sockfd);
#endif

#ifdef _WIN32
    closesocket(sockfd);
#endif
}

int send_status(void)
{
    unsigned char status_buf[4] = {0, 0, 0, 0};
    status_buf[0] = status.is_adb_connect;
    status_buf[1] = status.is_keyboard_connect;
    status_buf[2] = status.is_mouse_connect;
    status_cmd_send(status_buf, 4);
}

int adb_send(unsigned char *buffer, int len)
{
    //LOG("%d\n",status.is_adb_connect);
    if (status.is_adb_connect == S_DISCONNECT)
    {
        return -1;
    }

#ifdef __linux__
    if (write(sockfd, buffer, len) != len)
    {
        close(sockfd);
    }
#endif

#ifdef _WIN32

    //log_byte(buffer, len);

    if (send(sockfd, buffer, len, 0) != len)
    {
        LOG("adb socket send fail\r\n");
        closesocket(sockfd);
    }
#endif

    return 0;
}

pthread_t adb_thread;

void *adb_fun_thread(void *arg)
{

    int is_found_device = 0;

    LOG("adb thread start\r\n");

    adb_version();

    while (1)
    {
        while (!is_found_device)
        {
            if (adb_devices() == 0)
            {

                is_found_device = 1;
            }
            else
            {
                sleep(2);
            }
        }

        adb_forward();

        sleep(1);
        LOG("start connect socket\r\n");

        //for (int try = 0; try < 100; try ++)
        {
            adb_socket();
        }

        status.is_adb_connect = S_DISCONNECT;
        is_found_device = 0;
        //LOG("restart\r\n");
        sleep(2);
    }

    return 0;
}

int adb_init(void)
{
    int ret;

    status.is_adb_connect = S_DISCONNECT;
    status.is_mouse_connect = S_DISCONNECT;
    status.is_keyboard_connect = S_DISCONNECT;

    // ret = adb_check_exist();
    // if(ret < 0)
    //     return -1;

    // ret = adb_start_server();
    // if(ret < 0)
    //     return -1;

    pthread_create(&adb_thread, NULL, adb_fun_thread, NULL);

    return 0;
}