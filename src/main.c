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

#ifdef _WIN32
#include <windows.h>
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

static int main_init(int select)
{
    int ret;

    LOG("\r\n==============start=============\r\n");
    LOG("\r\n==============%s=============\r\n", VERSION);
    LOG("ATouchClient is runing\r\n");

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
    return 0;
}

#ifdef __linux__
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

#ifdef __linux__
    signal(SIGINT, Stop);
#endif


    main_init(select);

exit:
    LOG("==============exit=============\r\n");
    return 0;
}

#endif

#ifdef _WIN32


POINT centerp,centerpc,pd;
pthread_t main_thread;

void *main_fun_thread(void *arg)
{
    main_init(0);
}

static void reload_window_size(HWND hwnd)
{
    ShowCursor(false);
    
    RECT r;
    GetWindowRect(hwnd,&r);

    r.left+=40;
    r.top+=40;
    r.right-=40;
    r.bottom-=40;

    ClipCursor(&r);

    centerp.x = (r.right+r.left)/2;
    centerp.y = (r.bottom+r.top)/2;

    centerpc.x = centerp.x;
    centerpc.y = centerp.y;

    ScreenToClient(hwnd,&centerpc);
    SetCursorPos(centerp.x,centerp.y);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{   
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_MOUSEMOVE:
            pd.x = LOWORD(lParam)-centerpc.x;
            pd.y = HIWORD(lParam)-centerpc.y;
            if(pd.x != 0 || pd.y != 0)
            printf("%d %d\r\n",pd.x,pd.y );
            SetCursorPos(centerp.x,centerp.y);
            //OnMouseMove(hwnd, LOWORD(lParam), HIWORD(lParam), (int)wParam);
            break;
        case WM_MOVE:
            reload_window_size(hwnd);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;  // 更多详细都可以去百度的 http://baike.baidu.com/view/1750396.htm
    HWND hwnd;
    MSG Msg;
    char text[30];
 
    const char szClassName[] = "myWindowClass";
 
    // 1. 设置注册窗口结构体
    wc.cbSize        = sizeof(WNDCLASSEX);              // 注册窗口结构体的大小
    wc.style         = 0;                               // 窗口的样式
    wc.lpfnWndProc   = WndProc;                         // 指向窗口处理过程的函数指针
    wc.cbClsExtra    = 0;                               // 指定紧跟在窗口类结构后的附加字节数
    wc.cbWndExtra    = 0;                               // 指定紧跟在窗口事例后的附加字节数
    wc.hInstance     = hInstance;                       // 本模块的实例句柄
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION); // 图标的句柄
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);     // 光标的句柄
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);        // 背景画刷的句柄
    wc.lpszMenuName  = NULL;                            // 指向菜单的指针
    wc.lpszClassName = szClassName;                     // 指向类名称的指针
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION); // 和窗口类关联的小图标
 
    // 2. 使用【窗口结构体】注册窗口
    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, TEXT("窗口注册失败！"), TEXT("错误"), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
 
    // 3. 创建窗口
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,       // 窗口的扩展风格
        szClassName,            // 指向注册类名的指针
        "ATouch "VERSION,       // 指向窗口名称的指针
        WS_OVERLAPPEDWINDOW,    // 窗口风格
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 240, // 窗口的 x,y 坐标以及宽高
        NULL,                   // 父窗口的句柄
        NULL,                   // 菜单的句柄
        hInstance,              // 应用程序实例的句柄
        NULL                    // 指向窗口的创建数据
        );
 
    if(hwnd == NULL)
    {
        MessageBox(NULL, "窗口创建失败", "错误",MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
 
    // 4. 显示窗口
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    pthread_create(&main_thread, NULL, main_fun_thread, NULL);

    reload_window_size(hwnd);

    // 6. 消息循环
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    
    return Msg.wParam;
}

#endif

