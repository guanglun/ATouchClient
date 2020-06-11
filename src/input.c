#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>

#ifdef __linux__
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <linux/input.h>
#endif
#include <pthread.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "log.h"
#include "version.h"

#include "adb.h"
#include "scmd.h"

#define LOG_HEAD "INPUT"

#define XINPUT_KEYBOARD "keyboard:"
#define XINPUT_MOUSE "pointer:"

#define KEY_STATUS_UP 0
#define KEY_STATUS_DOWN 1
#define KEY_STATUS_KEEP 2

extern struct RUN_STATUS status;

enum INPUT_CLASS
{
    INPUT_CLASS_UNKNOW = 0,
    INPUT_CLASS_MOUSE,
    INPUT_CLASS_KEYBOARD
};

struct INPUT_DEVICE
{
    bool enable;
    int fd;
    char path[20];
    char name[80];
};

struct INPUT_DEVICE mouse, keyboard;

#ifdef __linux__
static unsigned char kbd_keycodes[256] = {
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_A,
    KEY_B, KEY_C, KEY_D, KEY_E, KEY_F,
    KEY_G, KEY_H, KEY_I, KEY_J, KEY_K,
    KEY_L, KEY_M, KEY_N, KEY_O, KEY_P,
    KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U,
    KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB, KEY_SPACE,
    KEY_MINUS, KEY_EQUAL, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_BACKSLASH,
    KEY_RESERVED, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_COMMA,
    KEY_DOT, KEY_SLASH, KEY_CAPSLOCK, KEY_F1, KEY_F2,
    KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7,
    KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_SYSRQ, KEY_SCROLLLOCK, KEY_PAUSE, KEY_INSERT, KEY_HOME,
    KEY_PAGEUP, KEY_DELETE, KEY_END, KEY_PAGEDOWN, KEY_RIGHT,
    KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NUMLOCK, KEY_KPSLASH,
    KEY_KPASTERISK, KEY_KPMINUS, KEY_KPPLUS, KEY_KPENTER, KEY_KP1,
    KEY_KP2, KEY_KP3, KEY_KP4, KEY_KP5, KEY_KP6,
    KEY_KP7, KEY_KP8, KEY_KP9, KEY_KP0, KEY_KPDOT,
    KEY_102ND, KEY_COMPOSE, KEY_POWER, KEY_KPEQUAL, KEY_F13,
    KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18,
    KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23,
    KEY_F24, KEY_OPEN, KEY_HELP, KEY_PROPS, KEY_FRONT,
    KEY_STOP, KEY_AGAIN, KEY_UNDO, KEY_CUT, KEY_COPY,
    KEY_PASTE, KEY_FIND, KEY_MUTE, KEY_VOLUMEUP, KEY_VOLUMEDOWN,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_KPCOMMA, KEY_RESERVED,
    KEY_RO, KEY_KATAKANAHIRAGANA, KEY_YEN, KEY_HENKAN, KEY_MUHENKAN,
    KEY_KPJPCOMMA, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_HANGUEL,
    KEY_HANJA, KEY_KATAKANA, KEY_HIRAGANA, KEY_ZENKAKUHANKAKU, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_LEFTCTRL,
    KEY_LEFTSHIFT, KEY_LEFTALT, KEY_LEFTMETA, KEY_RIGHTCTRL, KEY_RIGHTSHIFT,
    KEY_RIGHTALT, KEY_RIGHTMETA, KEY_PLAYPAUSE, KEY_STOPCD, KEY_PREVIOUSSONG,
    KEY_NEXTSONG, KEY_EJECTCD, KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_MUTE,
    KEY_WWW, KEY_BACK, KEY_FORWARD, KEY_STOP, KEY_FIND,
    KEY_SCROLLUP, KEY_SCROLLDOWN, KEY_EDIT, KEY_SLEEP, KEY_COFFEE,
    KEY_REFRESH, KEY_CALC, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
    KEY_RESERVED};
#endif

static enum INPUT_CLASS get_input_class(int fd)
{
#ifdef __linux__

    uint8_t *bits = NULL;
    ssize_t bits_size = 0;
    int i, j, k;
    int res;

    int mouse_feature_count = 0;
    int keyboard_feature_count = 0;

    for (i = EV_KEY; i <= EV_MAX; i++)
    { // skip EV_SYN since we cannot query its available codes
        int count = 0;
        while (1)
        {
            res = ioctl(fd, EVIOCGBIT(i, bits_size), bits);
            if (res < bits_size)
                break;
            bits_size = res + 16;
            bits = realloc(bits, bits_size * 2);
            if (bits == NULL)
            {
                fprintf(stderr, "failed to allocate buffer of size %d\n", (int)bits_size);
                return INPUT_CLASS_UNKNOW;
            }
        }
        for (j = 0; j < res; j++)
        {
            for (k = 0; k < 8; k++)
                if (bits[j] & 1 << k)
                {

                    if (j * 8 + k == REL_X && i == EV_REL)
                    {
                        mouse_feature_count++;
                    }
                    else if (j * 8 + k == REL_Y && i == EV_REL)
                    {
                        mouse_feature_count++;
                    }
                    else if (j * 8 + k == REL_WHEEL && i == EV_REL)
                    {
                        mouse_feature_count++;
                    }

                    if (i == EV_KEY)
                    {
                        keyboard_feature_count++;
                    }

                    count++;
                }
        }
    }
    free(bits);

    if (mouse_feature_count >= 3)
    {
        return INPUT_CLASS_MOUSE;
    }
    else if (keyboard_feature_count >= 10)
    {
        return INPUT_CLASS_KEYBOARD;
    }

#endif

    return INPUT_CLASS_UNKNOW;
}

static void set_input_enable(char *name, char *class)
{
    char cmd_str[1024];
    sprintf(cmd_str, "xinput enable \"%s%s\"", class, name);
    system(cmd_str);
}

static void set_input_disable(char *name, char *class)
{
    char cmd_str[1024];
    sprintf(cmd_str, "xinput disable \"%s%s\"", class, name);
    system(cmd_str);
}

int input_exit(void)
{
    if (mouse.enable == true)
    {
        set_input_enable(mouse.name, XINPUT_MOUSE);
    }

    if (keyboard.enable == true)
    {
        set_input_enable(keyboard.name, XINPUT_KEYBOARD);
    }

    return 0;
}

static int input_scan(void)
{
#ifdef __linux__
    int fd;
    char name[80];
    char path[20];
    for (int i = 0; i < 20; i++)
    {
        sprintf(path, "/dev/input/event%d", i);

        fd = open(path, O_RDWR);

        if (fd > 0)
        {

            memset(name, '\0', sizeof(name));
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1)
            {
                name[0] = '\0';
            }

            enum INPUT_CLASS input_class = get_input_class(fd);

            switch (input_class)
            {
            case INPUT_CLASS_UNKNOW:
                //LOG("%s INPUT_UNKNOW \t%s \r\n",path,name);
                break;
            case INPUT_CLASS_MOUSE:
                //LOG("%s INPUT_MOUSE \t%s \r\n",path,name);

                if (mouse.enable == false)
                {
                    LOG("%s INPUT_MOUSE    \t%s \r\n", path, name);
                    mouse.fd = fd;
                    memcpy(mouse.path, path, sizeof(path));
                    memcpy(mouse.name, name, sizeof(name));
                    mouse.enable = true;
                    status.is_mouse_connect = true;
                    set_input_disable(name, XINPUT_MOUSE);
                }

                break;
            case INPUT_CLASS_KEYBOARD:
                //LOG("%s INPUT_KEYBOARD \t%s \r\n",path,name);

                if (keyboard.enable == false)
                {
                    LOG("%s INPUT_KEYBOARD \t%s \r\n", path, name);
                    keyboard.fd = fd;
                    memcpy(keyboard.path, path, sizeof(path));
                    memcpy(keyboard.name, name, sizeof(name));
                    keyboard.enable = true;
                    status.is_keyboard_connect = true;
                    set_input_disable(name, XINPUT_KEYBOARD);
                }

                break;
            }

            if (fd != mouse.fd && fd != keyboard.fd)
            {
                close(fd);
            }
        }
    }

#endif

    return 0;
}

static int select_input(void)
{
#ifdef __linux__
    int fd;
    int selectid = 0;
    char name[80];
    char path[20];

    printf("Input Device List :\r\n");

    for (int i = 0; i < 20; i++)
    {
        sprintf(path, "/dev/input/event%d", i);

        fd = open(path, O_RDWR);

        if (fd > 0)
        {

            memset(name, '\0', sizeof(name));
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1)
            {
                name[0] = '\0';
            }

            printf("  %s\tid=%d\t%s\r\n", path, i, name);
        }
        close(fd);
    }

    printf("Select Mouse id:");
    scanf("%d", &selectid);

    sprintf(path, "/dev/input/event%d", selectid);
    fd = open(path, O_RDWR);
    if (fd > 0)
    {
        memset(name, '\0', sizeof(name));
        if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1)
        {
            name[0] = '\0';
        }
        mouse.fd = fd;
        memcpy(mouse.path, path, sizeof(path));
        memcpy(mouse.name, name, sizeof(name));
        mouse.enable = true;
        status.is_mouse_connect = true;
        set_input_disable(name, XINPUT_MOUSE);
    }
    else
    {
        input_exit();
        printf("open fail,exit\r\n");
    }

    printf("Select Keyboard id:");
    scanf("%d", &selectid);

    sprintf(path, "/dev/input/event%d", selectid);
    fd = open(path, O_RDWR);
    if (fd > 0)
    {
        memset(name, '\0', sizeof(name));
        if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1)
        {
            name[0] = '\0';
        }
        keyboard.fd = fd;
        memcpy(keyboard.path, path, sizeof(path));
        memcpy(keyboard.name, name, sizeof(name));
        keyboard.enable = true;
        status.is_keyboard_connect = true;
        set_input_disable(name, XINPUT_KEYBOARD);
    }
    else
    {
        input_exit();
        printf("open fail,exit\r\n");
    }

    LOG("%s INPUT_MOUSE    \t%s \r\n", mouse.path, mouse.name);
    LOG("%s INPUT_KEYBOARD \t%s \r\n", keyboard.path, keyboard.name);

#endif
    return 0;
}

void *mouse_fun_thread(void *arg)
{
#ifdef __linux__
    int ret;
    bool report = false;
    unsigned char buf[4];
    struct input_event event;
    LOG("mouse thread start\r\n");
    memset(buf, 0, 4);

    while (1)
    {
        if (mouse.enable == true)
        {
            ret = read(mouse.fd, &event, sizeof(event));
            if (ret < 0)
            {
                close(mouse.fd);
                mouse.fd = 0;
                mouse.enable = false;
                status.is_mouse_connect = false;
                set_input_enable(mouse.name, XINPUT_MOUSE);
            }
            else
            {
                if (event.type == EV_SYN && event.code == SYN_REPORT)
                {
                    report = true;
                }
                else if (event.type == EV_KEY && event.code == BTN_LEFT)
                {
                    if (event.value)
                    {
                        buf[0] |= 0x01;
                    }
                    else
                    {
                        buf[0] &= (~0x01);
                    }
                }
                else if (event.type == EV_KEY && event.code == BTN_RIGHT)
                {
                    if (event.value)
                    {
                        buf[0] |= 0x02;
                    }
                    else
                    {
                        buf[0] &= (~0x02);
                    }
                }
                else if (event.type == EV_KEY && event.code == BTN_MIDDLE)
                {
                    if (event.value)
                    {
                        buf[0] |= 0x04;
                    }
                    else
                    {
                        buf[0] &= (~0x04);
                    }
                }
                else if (event.type == EV_REL && event.code == REL_X)
                {
                    buf[1] = (char)event.value;
                }
                else if (event.type == EV_REL && event.code == REL_Y)
                {
                    buf[2] = (char)event.value;
                }
                else if (event.type == EV_REL && event.code == REL_WHEEL)
                {
                    buf[3] = (char)event.value;
                }

                if (report == true)
                {
                    //log_byte(buf,4);
                    mouse_cmd_send(buf, 4);
                    memset(buf + 1, 0, 3);
                    report = false;
                }
                //LOG("[mouse]%04x %04x %08x\r\n", event.type, event.code, event.value);
            }
        }
    }
#endif
}

void *keyboard_fun_thread(void *arg)
{
#ifdef __linux__
    int ret;
    bool report = false;

    struct input_event event;
    unsigned char buf[8];
    LOG("keyboard thread start\r\n");
    bool is_keep_only = true;
    unsigned char i, m, n;
    while (1)
    {
        if (keyboard.enable == true)
        {
            ret = read(keyboard.fd, &event, sizeof(event));
            if (ret < 0)
            {
                close(keyboard.fd);
                keyboard.fd = 0;
                keyboard.enable = false;
                status.is_keyboard_connect = false;
                set_input_enable(keyboard.name, XINPUT_KEYBOARD);
            }
            else
            {
                if (event.type == EV_SYN && event.code == SYN_REPORT)
                {
                    report = true;
                }
                else if (event.type == EV_KEY && event.value == KEY_STATUS_DOWN)
                {
                    is_keep_only = false;
                    if (event.code == KEY_LEFTCTRL)
                    {
                        buf[0] |= 0x01;
                    }
                    else if (event.code == KEY_LEFTSHIFT)
                    {
                        buf[0] |= 0x02;
                    }
                    else if (event.code == KEY_LEFTALT)
                    {
                        buf[0] |= 0x04;
                    }
                    else if (event.code == KEY_LEFTMETA)
                    {
                        buf[0] |= 0x08;
                    }
                    else if (event.code == KEY_RIGHTCTRL)
                    {
                        buf[0] |= 0x10;
                    }
                    else if (event.code == KEY_RIGHTSHIFT)
                    {
                        buf[0] |= 0x20;
                    }
                    else if (event.code == KEY_RIGHTALT)
                    {
                        buf[0] |= 0x40;
                    }
                    else if (event.code == KEY_RIGHTMETA)
                    {
                        buf[0] |= 0x80;
                    }
                    else
                    {
                        for (i = 0; i < 255; i++)
                        {
                            if (kbd_keycodes[i] == event.code)
                            {
                                break;
                            }
                        }

                        for (m = 2; m < 8; m++)
                        {
                            if (buf[m] == 0)
                            {
                                buf[m] = i;
                                break;
                            }
                        }
                    }
                }
                else if (event.type == EV_KEY && event.value == KEY_STATUS_UP)
                {
                    is_keep_only = false;
                    if (event.code == KEY_LEFTCTRL)
                    {
                        buf[0] &= (~0x01);
                    }
                    else if (event.code == KEY_LEFTSHIFT)
                    {
                        buf[0] &= (~0x02);
                    }
                    else if (event.code == KEY_LEFTALT)
                    {
                        buf[0] &= (~0x04);
                    }
                    else if (event.code == KEY_LEFTMETA)
                    {
                        buf[0] &= (~0x08);
                    }
                    else if (event.code == KEY_RIGHTCTRL)
                    {
                        buf[0] &= (~0x10);
                    }
                    else if (event.code == KEY_RIGHTSHIFT)
                    {
                        buf[0] &= (~0x20);
                    }
                    else if (event.code == KEY_RIGHTALT)
                    {
                        buf[0] &= (~0x40);
                    }
                    else if (event.code == KEY_RIGHTMETA)
                    {
                        buf[0] &= (~0x80);
                    }
                    else
                    {
                        for (i = 0; i < 255; i++)
                        {
                            if (kbd_keycodes[i] == event.code)
                            {
                                break;
                            }
                        }

                        for (m = 2; m < 8; m++)
                        {
                            if (buf[m] == i)
                            {
                                break;
                            }
                        }

                        for (n = m; n < 7; n++)
                        {
                            if (buf[n + 1] != 0)
                            {
                                buf[n] = buf[n + 1];
                            }
                            else
                            {
                                buf[n] = 0;
                                break;
                            }
                        }
                    }
                }

                if (buf[0] == 0x01 && buf[2] == 0x06)
                {
                    input_exit();
                    _exit(0);
                }

                if (report == true)
                {
                    if (is_keep_only == false)
                    {
                        log_byte(buf, 8);
                        keyboard_cmd_send(buf, 8);
                        is_keep_only = true;
                    }
                    report = false;
                }

                //LOG("[keyboard]%04x %04x %08x\r\n", event.type, event.code, event.value);
            }
        }
    }
#endif
}

void *scan_fun_thread(void *arg)
{
    LOG("scan thread start\r\n");

    while (1)
    {
        if (mouse.enable == true && keyboard.enable == true)
        {
        }
        else
        {
            input_scan();
        }

        sleep(1);
    }
}

static int windows_creat(void)
{

    return 0;
}

pthread_t mouse_thread, keyboard_thread, scan_thread;
int input_init(int select)
{
    mouse.enable = false;
    keyboard.enable = false;

#ifdef __linux__
    if (select == 0)
    {
        pthread_create(&scan_thread, NULL, scan_fun_thread, NULL);
    }
    else
    {
        select_input();
    }
#endif

    pthread_create(&mouse_thread, NULL, mouse_fun_thread, NULL);
    pthread_create(&keyboard_thread, NULL, keyboard_fun_thread, NULL);

    // ShowCursor(false);


    // windows_creat();

//     RECT rect;
//     rect.bottom = 100;
//     rect.right = 100;
// //-----------add------
//     rect.left = 100;
//     rect.top = 100;
// //-----------end-----
//     ClipCursor(&rect);

//     while(1)
//     {
//         POINT p;
//         GetCursorPos(&p);
//         LOG("%d %d\r\n",p.x,p.y);
//         //SetCursorPos(100,100);
//     }


    return 0;
}

