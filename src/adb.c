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
#include "log.h"
#include "version.h"
#include "scmd.h"

#define LOG_HEAD "ADB"

struct RUN_STATUS status = {
    .is_adb_connect = false,
    .is_mouse_connect = false,
    .is_keyboard_connect = false
};

char cmd_str[4096],recv_str[4096];
FILE *fp; 
int sockfd;

static int adb_check_exist(void)
{
    LOG("check adb\r\n") ; 
    sprintf(cmd_str,"adb version");
    LOG("%s\r\n", cmd_str) ;

    memset(recv_str,0,sizeof(recv_str));
    fp=popen(cmd_str, "r"); 
    fgets(recv_str,sizeof(recv_str),fp); 
    LOG("%s",recv_str); 

    if(strstr(recv_str,"Android Debug Bridge") != NULL)
    {
        LOG("adb exist\r\n") ;
        return 0; 
    }else{
        LOG("adb does not exist\r\n") ; 
        return -1;
    }
    return -1;
}

static int adb_start_server(void)
{
    sprintf(cmd_str,"adb kill-server");
    LOG("%s\r\n", cmd_str) ;

    memset(recv_str,0,sizeof(recv_str));
    fp=popen(cmd_str, "r"); 
    fgets(recv_str,sizeof(recv_str),fp); 
    LOG("%s",recv_str); 

    sprintf(cmd_str,"adb start-server");
    LOG("%s\r\n", cmd_str) ;

    memset(recv_str,0,sizeof(recv_str));
    fp=popen(cmd_str, "r"); 
    fgets(recv_str,sizeof(recv_str),fp); 
    LOG("%s",recv_str); 

    memset(recv_str,0,sizeof(recv_str));
    fgets(recv_str,sizeof(recv_str),fp); 
    LOG("%s",recv_str); 

    if(strstr(recv_str,"daemon started successfully") != NULL)
    {
        LOG("adb server start success\r\n") ;
        return 0; 
    }else{
        LOG("adb server start fail\r\n") ; 
        return -1;
    }
    return -1;   
}

static int adb_devices(void)
{
    sprintf(cmd_str,"adb devices");
    //LOG("%s\r\n", cmd_str) ;

    memset(recv_str,0,sizeof(recv_str));
    fp=popen(cmd_str, "r"); 
    fgets(recv_str,sizeof(recv_str),fp); 
    //LOG("%s",recv_str); 

    memset(recv_str,0,sizeof(recv_str));
    fgets(recv_str,sizeof(recv_str),fp); 
    //LOG("%s",recv_str); 

    if(strlen(recv_str) > 1)
    {
        return 0;
    }

    return -1;
}

static int adb_forward(void)
{
    sprintf(cmd_str,"adb forward tcp:7680 tcp:1989");
    //LOG("%s\r\n", cmd_str) ;

    memset(recv_str,0,sizeof(recv_str));
    fp=popen(cmd_str, "r"); 
    fgets(recv_str,sizeof(recv_str),fp); 
}

static int adb_socket(void)
{
    char recv_buf[1024];
    size_t recv_size;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
		return -1;
    }


    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(7680);
    inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr);
 
    if(connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {
        //LOG("socket connect fail\r\n");
	    return -1;
    }
    status.is_adb_connect = true;
    LOG("socket connect success\r\n");
    while(1)
    {
        if((recv_size = read(sockfd, recv_buf, sizeof(recv_buf))) < 0)
        {
            return -1;
        }else{
            if(recv_size > 0)
            {
                recv_buf[recv_size] = '\0';
                LOG("%s\r\n",recv_buf);
            }else{
                return -1;
            }
        }

    }
//     if(write(STDOUT_FILENO, recv_buf, recv_size) != recv_size)
//     {
// 		perror("write error");
//     }
//     close(sockfd);

}

int adb_send(unsigned char *buffer,int len)
{
    if(status.is_adb_connect == false)
    {
        return -1;
    }

    if(write(sockfd, buffer, len) != len)
    {
		close(sockfd);
    }

    return 0;
}

pthread_t adb_thread;

void *adb_fun_thread(void *arg)
{
    int is_found_device = 0;

    LOG("adb thread start\r\n");

    while(1)
    {
        while(!is_found_device)
        {
            if(adb_devices() == 0)
            {
                
                is_found_device = 1;
            }else{
                sleep(2);
            }
        }
        
        adb_forward();

        //sleep(2);
        //LOG("start connect socket\r\n");

        adb_socket();

        status.is_adb_connect = false;
        is_found_device = 0;
        //LOG("restart\r\n");
        sleep(2);
    }

    return 0;
}

int adb_init(void)
{
    int ret;

    
    // ret = adb_check_exist();
    // if(ret < 0)
    //     return -1;

    // ret = adb_start_server();
    // if(ret < 0)
    //     return -1;


    pthread_create(&adb_thread, NULL, adb_fun_thread, NULL);



    return 0;
}