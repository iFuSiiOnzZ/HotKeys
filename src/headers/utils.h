#ifndef UTILS_H
#define UTILS_H

#include <Windows.h> /* FILETIME, NOTIFYICONDATA, GetFileAttributesEx, GetLastError, LoadIcon, GetModuleHandle, MAKEINTRESOURCE, Shell_NotifyIcon*/

unsigned int GetFileSize(char *pFileName);
char *LoadStringFileIntoMemory(char *pFileName);

FILETIME GetLastWriteTime(char *pFileName);
void ShowError(char *pUserMessage);

void EnableTrayIcon(NOTIFYICONDATA *pTrayIcon, HWND hWnd, int ID);
void DisableTrayIcon(NOTIFYICONDATA *pTrayIcon);


#endif
