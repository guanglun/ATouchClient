#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h> 
#include <stdbool.h>

#include "log.h"
#include "version.h"

#define LOG_HEAD "INPUT"

enum INPUT_CLASS{
    INPUT_UNKNOW = 0,
    INPUT_MOUSE,
    INPUT_KEYBOARD
};

struct INPUT_DEVICE{
    bool enable;
    int fd;
    char path[20];
};

struct INPUT_DEVICE mouse,keyboard;

static enum INPUT_CLASS get_input_class(int fd)
{
    uint8_t *bits = NULL;
    ssize_t bits_size = 0;
    int i, j, k;
    int res;

    int mouse_feature_count = 0;
    int keyboard_feature_count = 0;

    for(i = EV_KEY; i <= EV_MAX; i++) { // skip EV_SYN since we cannot query its available codes
        int count = 0;
        while(1) {
            res = ioctl(fd, EVIOCGBIT(i, bits_size), bits);
            if(res < bits_size)
                break;
            bits_size = res + 16;
            bits = realloc(bits, bits_size * 2);
            if(bits == NULL) {
                fprintf(stderr, "failed to allocate buffer of size %d\n", (int)bits_size);
                return INPUT_UNKNOW;
            }
        }
        for(j = 0; j < res; j++) {
            for(k = 0; k < 8; k++)
                if(bits[j] & 1 << k) {

                    if(j * 8 + k == REL_X && i == EV_REL)
                    {
                        mouse_feature_count++;
                    }else if(j * 8 + k == REL_Y && i == EV_REL)
                    {
                        mouse_feature_count++;
                    }else if(j * 8 + k == REL_WHEEL && i == EV_REL)
                    {
                        mouse_feature_count++;
                    }

                    if(i == EV_KEY)
                    {
                        keyboard_feature_count++;
                    }

                    count++;
                }
        }
    }
    free(bits);

    if(mouse_feature_count >= 3)
    {
        return INPUT_MOUSE;
    }else if(keyboard_feature_count >= 10)
    {
        return INPUT_KEYBOARD;
    }

    return INPUT_UNKNOW;
}

int input_scan(void)
{
    int fd;
    char name[80];
    char path[20];
    for(int i = 5 ;i<10;i++)
    {
        sprintf(path,"/dev/input/event%d",i);

        fd = open(path, O_RDWR);

        if(fd > 0) {

            memset(name,'\0',sizeof(name));
            if(ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }

            enum INPUT_CLASS input_class = get_input_class(fd);

            switch(input_class)
            {
                case INPUT_UNKNOW:
                    //LOG("%s INPUT_UNKNOW \t%s \r\n",path,name);
                    break;
                case INPUT_MOUSE:
                    //LOG("%s INPUT_MOUSE \t%s \r\n",path,name);

                    if(mouse.enable == false)
                    {
                        LOG("%s INPUT_MOUSE \t%s \r\n",path,name);
                        mouse.fd = fd;
                        memcpy(mouse.path,path,sizeof(path));
                        mouse.enable = true;
                    }

                    break;
                case INPUT_KEYBOARD:
                    //LOG("%s INPUT_KEYBOARD \t%s \r\n",path,name);

                    if(keyboard.enable == false)
                    {
                        LOG("%s INPUT_KEYBOARD \t%s \r\n",path,name);
                        keyboard.fd = fd;
                        memcpy(keyboard.path,path,sizeof(path));
                        keyboard.enable = true;
                    }

                    break;                        
            }   


            if(fd != mouse.fd && fd != keyboard.fd)
            {
                close(fd);
            }
        }

     
    }
    return 0;
}

void *mouse_fun_thread(void *arg)
{
    int ret;
    bool report = false;
    unsigned char buf[4];
    struct input_event event;
    LOG("mouse thread start\r\n");
    memset(buf,0,4);

    while(1)
    {
        if(mouse.enable == true)
        {
            ret = read(mouse.fd, &event, sizeof(event));
            if(ret < 0)
            {
                close(mouse.fd);
                mouse.fd = 0;
                mouse.enable = false;
            }else{

                report = true;
                memset(buf+1,0,3);
                if(event.type == EV_KEY && event.code == BTN_LEFT)
                {
                    
                }
                // switch(event.code)
                // {
                //     case BTN_LEFT:
                //         if(event.value)
                //         {
                //             buf[0] |= 0x01;
                //         }
                //         else
                //         {
                //             buf[0] &= (~0x01);
                //         }  
                //     break;
                //     case BTN_RIGHT:
                //         if(event.value)
                //         {
                //             buf[0] |= 0x02;
                //         }
                //         else
                //         {
                //             buf[0] &= (~0x02);
                //         }  
                //     break;     
                //     case BTN_MIDDLE:
                //         if(event.value)
                //         {
                //             buf[0] |= 0x04;
                //         }
                //         else
                //         {
                //             buf[0] &= (~0x04);
                //         }  
                //     break;    
                //     case REL_X:
                //         buf[1] = (char)event.value;
                //     break;    
                //     case REL_Y:
                //         buf[2] = (char)event.value;
                //     break;            
                //     case REL_WHEEL:
                //         buf[3] = (char)event.value;
                //     break;                                           
                //     default:
                //         report = false;
                //     break;
                // }

                if(report == true)
                {
                    log_byte(buf,4);
                }
                //LOG("[mouse]%04x %04x %08x\r\n", event.type, event.code, event.value);
            }
            
        }
    }
}

void *keyboard_fun_thread(void *arg)
{
    int ret;
    struct input_event event;
    LOG("keyboard thread start\r\n");

    while(1)
    {
        if(keyboard.enable == true)
        {
            ret = read(keyboard.fd, &event, sizeof(event));
            if(ret < 0)
            {
                close(keyboard.fd);
                keyboard.fd = 0;
                keyboard.enable = false;
            }else{
                LOG("[keyboard]%04x %04x %08x\r\n", event.type, event.code, event.value);
            }
            
        }
    }
}

void *scan_fun_thread(void *arg)
{
    LOG("scan thread start\r\n");

    while(1)
    {
        if(mouse.enable == true && keyboard.enable == true)
        {
            
        }else{
            input_scan();
        }
        
        sleep(1);
    }
}

pthread_t mouse_thread,keyboard_thread,scan_thread;
int input_init(void)
{
    mouse.enable = false;
    keyboard.enable = false;

    

    pthread_create(&scan_thread, NULL, scan_fun_thread, NULL);
    pthread_create(&mouse_thread, NULL, mouse_fun_thread, NULL);
    pthread_create(&keyboard_thread, NULL, keyboard_fun_thread, NULL);

    return 0;
}