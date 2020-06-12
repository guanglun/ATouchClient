#ifndef __WIN_PROC_H__
#define __WIN_PROC_H__

#ifdef _WIN32
#include <windows.h>
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void reload_window_size(HWND hwnd);

#endif

#endif
