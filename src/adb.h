#ifndef __ADB_H__
#define __ADB_H__

#include <stdbool.h>

enum ADB_STATUS
{
    ADB_STATUS_NOTCONNECT,
    ADB_STATUS_CONNECT
};

struct RUN_STATUS
{
    bool is_adb_connect;
    bool is_mouse_connect;
    bool is_keyboard_connect;
};

int adb_init(void);
int adb_send(unsigned char *buffer, int len);
int send_status(void);

#endif