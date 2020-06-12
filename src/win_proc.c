#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include "win_proc.h"
#endif

#include <pthread.h>
#include <unistd.h>

#include "adb.h"
#include "input.h"
#include "log.h"
#include "version.h"
#include "scmd.h"
#include "keyboard.h"

#define LOG_HEAD "WINPROC"

#ifdef _WIN32

POINT centerp, centerpc, pd;
unsigned char GameEnable = 0;

extern struct RUN_STATUS status;

static unsigned char win_kbd_keycodes[256] = {

    [VK_0] = KB_0,
    [VK_1] = KB_1,
    [VK_2] = KB_2,
    [VK_3] = KB_3,
    [VK_4] = KB_4,
    [VK_5] = KB_5,
    [VK_6] = KB_6,
    [VK_7] = KB_7,
    [VK_8] = KB_8,
    [VK_9] = KB_9,

    [VK_A] = KB_A,
    [VK_B] = KB_B,
    [VK_C] = KB_C,
    [VK_D] = KB_D,
    [VK_E] = KB_E,
    [VK_F] = KB_F,
    [VK_G] = KB_G,
    [VK_H] = KB_H,
    [VK_I] = KB_I,
    [VK_J] = KB_J,
    [VK_K] = KB_K,
    [VK_L] = KB_L,
    [VK_M] = KB_M,
    [VK_N] = KB_N,
    [VK_O] = KB_O,
    [VK_P] = KB_P,
    [VK_Q] = KB_Q,
    [VK_R] = KB_R,
    [VK_S] = KB_S,
    [VK_T] = KB_T,
    [VK_U] = KB_U,
    [VK_V] = KB_V,
    [VK_W] = KB_W,
    [VK_X] = KB_X,
    [VK_Y] = KB_Y,
    [VK_Z] = KB_Z,

    [VK_F1] = KB_F1,
    [VK_F2] = KB_F2,
    [VK_F3] = KB_F3,
    [VK_F4] = KB_F4,
    [VK_F5] = KB_F5,
    [VK_F6] = KB_F6,
    [VK_F7] = KB_F7,
    [VK_F8] = KB_F8,
    [VK_F9] = KB_F9,
    [VK_F10] = KB_F10,
    [VK_F11] = KB_F11,
    [VK_F12] = KB_F12,

    [VK_RETURN] = KB_ENTER,
    [VK_ESCAPE] = KB_ESCAPE,
    [VK_BACK] = KB_BSPACE,
    [VK_TAB] = KB_TAB,
    [VK_SPACE] = KB_SPACE,
    [VK_OEM_MINUS] = KB_MINUS,
    [VK_OEM_PLUS] = KB_EQUAL,
    [VK_OEM_4] = KB_LBRACKET,
    [VK_OEM_6] = KB_RBRACKET,
    [VK_OEM_5] = KB_BSLASH,
    [VK_OEM_1] = KB_SCOLON,
    [VK_OEM_7] = KB_QUOTE,
    [VK_OEM_3] = KB_GRAVE,
    [VK_OEM_COMMA] = KB_COMMA,
    [VK_OEM_PERIOD] = KB_DOT,
    [VK_OEM_2] = KB_SLASH,
    [VK_CAPITAL] = KB_CAPSLOCK,

    [VK_INSERT] = KB_INSERT,
    [VK_HOME] = KB_HOME,
    [VK_PRIOR] = KB_PGUP,
    [VK_DELETE] = KB_DELETE,
    [VK_END] = KB_END,
    [VK_NEXT] = KB_PGDOWN,

    [VK_RIGHT] = KB_RIGHT,
    [VK_LEFT] = KB_LEFT,
    [VK_DOWN] = KB_DOWN,
    [VK_UP] = KB_UP,

    [VK_NUMLOCK] = KB_NUMLOCK,
    [VK_DIVIDE] = KB_KP_SLASH,
    [VK_MULTIPLY] = KB_KP_ASTERISK,
    [VK_SUBTRACT] = KB_KP_MINUS,
    [VK_ADD] = KB_KP_PLUS,

    [VK_NUMPAD0] = KB_KP_0,
    [VK_NUMPAD1] = KB_KP_1,
    [VK_NUMPAD2] = KB_KP_2,
    [VK_NUMPAD3] = KB_KP_3,
    [VK_NUMPAD4] = KB_KP_4,
    [VK_NUMPAD5] = KB_KP_5,
    [VK_NUMPAD6] = KB_KP_6,
    [VK_NUMPAD7] = KB_KP_7,
    [VK_NUMPAD8] = KB_KP_8,
    [VK_NUMPAD9] = KB_KP_9,
    [VK_DECIMAL] = KB_KP_DOT,

};

void reload_window_size(HWND hwnd)
{
    //ShowCursor(FALSE);

    status.is_keyboard_connect = S_CONNECT;
    status.is_mouse_connect = S_CONNECT;

    RECT r;
    GetWindowRect(hwnd, &r);

    r.left += 40;
    r.top += 40;
    r.right -= 40;
    r.bottom -= 40;

    ClipCursor(&r);

    centerp.x = (r.right + r.left) / 2;
    centerp.y = (r.bottom + r.top) / 2;

    centerpc.x = centerp.x;
    centerpc.y = centerp.y;

    ScreenToClient(hwnd, &centerpc);
    //SetCursorPos(centerp.x, centerp.y);

    GameEnable = 1;
}

void window_out(HWND hwnd)
{
    //ShowCursor(TRUE);
    GameEnable = 0;
    ReleaseCapture();
}

unsigned char kb_buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char kb_buf_back[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char mouse_buf[4] = {0, 0, 0, 0};
unsigned char mouse_buf_back[4] = {0, 0, 0, 0};
char key_name[200];
int i,m,n;

unsigned char activate = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    
    if(wParam==SC_KEYMENU) 
    {
        return 0;
    }

    switch (msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd,&ps);
            DrawText(ps.hdc,TEXT("WIN键返回界面操作"),strlen(TEXT("WIN键返回界面操作")),&(ps.rcPaint),DT_CENTER);
            EndPaint(hwnd,&ps);
        }
        return 0;
    case WM_ACTIVATE:
        if(wParam == WA_CLICKACTIVE)
        {
            activate = 1;
        }
        break;
    case WM_CLOSE:
        
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_MOUSEMOVE:
        if(GameEnable)
        {
            pd.x = (LOWORD(lParam) - centerpc.x)/1.5;
            pd.y = (HIWORD(lParam) - centerpc.y)/1.5;

            SetCursorPos(centerp.x, centerp.y);

            mouse_buf[1] = (char)pd.x;
            mouse_buf[2] = (char)pd.y;
        }
        break;
    case WM_LBUTTONDOWN:
        mouse_buf[0] |= 0x01;
        break;
    case WM_LBUTTONUP:
        if(activate == 1)
        {
            reload_window_size(hwnd);
        }
        mouse_buf[0] &= (~0x01);
        break;
    case WM_RBUTTONDOWN:
        mouse_buf[0] |= 0x02;
        break;
    case WM_RBUTTONUP:
        mouse_buf[0] &= (~0x02);
        break;
    case WM_MBUTTONDOWN:
        mouse_buf[0] |= 0x04;
        break;
    case WM_MBUTTONUP:
        mouse_buf[0] &= (~0x04);
        break;

    case WM_MOVE:
        reload_window_size(hwnd);
        break;

    case WM_KEYDOWN:

        //LOG("KEY DW : %d %X\r\n", wParam, wParam);
        switch (wParam)
        {
        case VK_LWIN:
            window_out(hwnd);
            break;            
        case VK_SHIFT:
            kb_buf[0] |= 0x02;
            break;
        case VK_CONTROL:
            kb_buf[0] |= 0x01;
            break;

        default:

            for(i = 2;i < 8;i++)
            {
                if(kb_buf[i] == win_kbd_keycodes[wParam])
                {
                    break;
                }else if(kb_buf[i] == 0)
                {
                    kb_buf[i] = win_kbd_keycodes[wParam];
                    break;
                }
            }

            break;
        }
        break;
    case WM_KEYUP:

        //LOG("KEY UP : %d %X\r\n", wParam, wParam);
        switch (wParam)
        {

        case VK_SHIFT:
            kb_buf[0] &= (~0x02);
            break;
        case VK_CONTROL:
            kb_buf[0] &= (~0x01);
            break;

        default:

            for(i = 2;i < 8;i++)
            {
                if(kb_buf[i] == win_kbd_keycodes[wParam])
                {
                    kb_buf[i] = 0;
                    break;
                }
            }
            
            if(i < 7)
            {
                for(m = i;m < 7;m++)
                {
                    if(kb_buf[m+1] != 0)
                    {
                        kb_buf[m] = kb_buf[m+1];
                        kb_buf[m+1] = 0;
                    }else{
                        break;
                    }
                }
            }


            break;
        }
        break;        
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
        break;
    }

    if(GameEnable)
    {
        if (memcmp(mouse_buf_back, mouse_buf, 4) != 0)
        {
            mouse_cmd_send(mouse_buf, 4);
            //log_byte(mouse_buf,4);
        }
        memcpy(mouse_buf_back, mouse_buf, 8);


        if (memcmp(kb_buf_back, kb_buf, 8) != 0)
        {
            keyboard_cmd_send(kb_buf, 8);
            //log_byte(kb_buf,8);
        }
        memcpy(kb_buf_back, kb_buf, 8);
    }

    return 0;
}

#endif

