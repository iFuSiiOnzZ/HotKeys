#include <sys/stat.h> /* struct stat, stat */
#include <stdio.h>    /* FILE, fopen_s, fread, fclose */
#include <utils.h>

unsigned int GetFileSize(char *pFileName)
{
    struct stat stat_buf = { 0 };
    int rc = stat(pFileName, &stat_buf);

    return rc == 0 ? stat_buf.st_size : 0;
}

char *LoadStringFileIntoMemory(char *pFileName)
{
    FILE *pFileHandle = NULL;
    char *pFileContent = NULL;

    fopen_s(&pFileHandle, pFileName, "r");
    if (!pFileHandle) return pFileContent;

    unsigned int FileSize = GetFileSize(pFileName);
    pFileContent = (char *) malloc(sizeof(char) * (FileSize + 1));

    size_t r = fread(pFileContent, 1, FileSize, pFileHandle);
    pFileContent[r] = '\0';

    fclose(pFileHandle);
    return pFileContent;
}

FILETIME GetLastWriteTime(char *pFileName)
{
    FILETIME LastWriteTime = { 0 };
    WIN32_FILE_ATTRIBUTE_DATA Data = { 0 };

    if(GetFileAttributesEx(pFileName, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return LastWriteTime;
}

void ShowError(char *pUserMessage)
{
    char s[256] = { 0 };
    sprintf_s(s, "Error: %d", GetLastError());
    MessageBoxA(NULL, pUserMessage, s, MB_ICONEXCLAMATION | MB_OK);
}

void EnableTrayIcon(NOTIFYICONDATA *pTrayIcon, HWND hWnd, int ID)
{
    pTrayIcon->hWnd = hWnd;
    pTrayIcon->uCallbackMessage = ID;

    pTrayIcon->cbSize = sizeof(NOTIFYICONDATA);
    pTrayIcon->uFlags = NIF_MESSAGE | NIF_ICON;

    pTrayIcon->hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(100));
    Shell_NotifyIcon(NIM_ADD, pTrayIcon);
}

void DisableTrayIcon(NOTIFYICONDATA *pTrayIcon)
{
    Shell_NotifyIcon(NIM_DELETE, pTrayIcon);
}
