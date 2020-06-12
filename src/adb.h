#ifndef __ADB_H__
#define __ADB_H__

enum CONNECT_STATUS{
    S_DISCONNECT = 0,
    S_CONNECT = 1,
    
};

struct RUN_STATUS
{
    unsigned char is_adb_connect;
    unsigned char is_mouse_connect;
    unsigned char is_keyboard_connect;
};

int adb_init(void);
int adb_send(unsigned char *buffer, int len);
int send_status(void);

#endif