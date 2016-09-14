
#include <Windows.h> /*  */
#include <shlobj.h>  /* SHGetFolderPathA, CSIDL_MYDOCUMENTS */

#include <string.h>  /* strcat_s, strlen */
#include <time.h>    /* struct tm, time, localtime_s, strftime */

// https://msdn.microsoft.com/en-us/library/ms644990(VS.85).aspx                        WH_KEYBOARD_DL
// https://msdn.microsoft.com/en-us/library/ms644985(v=vs.85).aspx                      LowLevelKeyboardProc
// https://msdn.microsoft.com/es-es/library/windows/desktop/ms644967(v=vs.85).aspx      KBDLLHOOKSTRUCT

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646300(v=vs.85).aspx      GetKeyNameTextA
// https://msdn.microsoft.com/es-es/library/windows/desktop/dd375731(v=vs.85).aspx      Virual Keys

// https://msdn.microsoft.com/en-us/library/bb762181(VS.85).aspx                        User Folder data
// https://msdn.microsoft.com/en-us/library/bb762494(v=vs.85).aspx                      CSIDL

#define CLASS_NAME "KeyboardHook"
#define WINDOW_NAME "KeyboardHook"

#define UNUSED(x) (void)(x)
#define NEWLINE "\r\n"

static int  g_KbdHookExit = 0;                    /* Exit app controll */
static char g_LogFilePath[MAX_PATH] = { 0 };      /* Logging user actions */
static char g_UserProfileDir[MAX_PATH] = { 0 };   /* Current user home directory */

void LogToFile(char *pMessage, char *pFile)
{
    HANDLE hFile = CreateFileA(pFile, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);

    if(hFile)
    {
        DWORD NumberOfBytesWritten = 0;

        time_t RawTime = 0 ;
        struct tm s_TimeInfo = { 0 };

        time (&RawTime);
        localtime_s(&s_TimeInfo, &RawTime);

        char pBuffer[80] = { 0 };
        strftime(pBuffer, 80 , "%d/%m/%y %H:%M:%S", &s_TimeInfo);

        WriteFile(hFile, pBuffer, (DWORD) strlen(pBuffer), &NumberOfBytesWritten, NULL);
        WriteFile(hFile,  "    ",                       4, &NumberOfBytesWritten, NULL);

        WriteFile(hFile, pMessage, (DWORD) strlen(pMessage), &NumberOfBytesWritten, NULL);
        WriteFile(hFile,  NEWLINE,  (DWORD) strlen(NEWLINE), &NumberOfBytesWritten, NULL);
        
        CloseHandle(hFile);
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(int action, WPARAM wp, LPARAM lp)
{
    if (action == HC_ACTION && (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN))
    {
        KBDLLHOOKSTRUCT *KeyboardData = (KBDLLHOOKSTRUCT *)lp;
        unsigned long Msg = (KeyboardData->scanCode << 16) + (KeyboardData->flags << 24);

        char KeyStr[256] = { 0 };
        GetKeyNameTextA(Msg, KeyStr, 256);

        OutputDebugStringA(KeyStr);

        if(KeyboardData->vkCode == 0x54 /*T*/ && (GetAsyncKeyState(VK_CONTROL) & GetAsyncKeyState(VK_MENU) & 0x8000))
        {
            STARTUPINFO si = { sizeof(si),  0 };
            PROCESS_INFORMATION pi = { 0 };

            if(CreateProcessA("c:\\windows\\system32\\cmd.exe", NULL, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi))
            {
                CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
                LogToFile("Create terminal process", g_LogFilePath);
            }
        }

        if(KeyboardData->vkCode == 0x45 /*E*/ && (GetAsyncKeyState(VK_CONTROL) & GetAsyncKeyState(VK_MENU) & 0x8000))
        {
            LogToFile("Ending HotKeys application", g_LogFilePath);
            g_KbdHookExit = 1;
        }
    }

    return CallNextHookEx(NULL, action, wp, lp);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wp, LPARAM lp)
{
    if(Msg == WM_CLOSE || Msg == WM_DESTROY)
    {
        OutputDebugStringA("Exit KeyBindigs");
        g_KbdHookExit = 1;
        return 0;
    }

    return DefWindowProcA(hWnd, Msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hActualInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow)
{
    UNUSED(hPrevInst);
    UNUSED(cmdLine);
    UNUSED(cmdShow);

    WNDCLASSEXA wndCls = { 0 };

    wndCls.cbSize           = sizeof(WNDCLASSEXA);
    wndCls.cbClsExtra       = 0;
    wndCls.cbWndExtra       = 0;

    wndCls.hInstance        = hActualInst;
    wndCls.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wndCls.hIconSm          = LoadIcon(NULL, IDI_APPLICATION);
    wndCls.hIcon            = LoadIcon(NULL, IDI_APPLICATION);

    wndCls.lpfnWndProc      = WndProc;
    wndCls.lpszMenuName     = NULL;
    wndCls.lpszClassName    = CLASS_NAME;

    wndCls.style            = 0;
    wndCls.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);

    if(RegisterClassExA(&wndCls) == 0)
    {
        MessageBoxA(NULL, "Window Registration Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    HWND hWnd = CreateWindowExA
    (
        WS_EX_CLIENTEDGE, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, 0, 0,
        300, 50, HWND_DESKTOP, NULL, hActualInst, NULL
    );

    if(hWnd == NULL)
    {
        MessageBoxA(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    HHOOK KbdHook = SetWindowsHookExA(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, hActualInst, NULL);
    MSG hMsg = { 0 };

    if(SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, g_UserProfileDir) != S_OK)
    {
        OutputDebugStringA("[ERROR] Can't get user profile directory");
    }

    if(g_UserProfileDir[0])
    {
        strcat_s(g_UserProfileDir, "\\HotKeys");
        CreateDirectoryA(g_UserProfileDir, NULL);

        strcat_s(g_LogFilePath, g_UserProfileDir);
        strcat_s(g_LogFilePath, "\\log.txt");
    }

    while(!g_KbdHookExit)
    { 
        while(PeekMessage(&hMsg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&hMsg);
            DispatchMessageA(&hMsg);
        }

        // NOTE(Andrei): Don't waste CPU time
        Sleep(20);
    }

    UnhookWindowsHookEx(KbdHook);
    UnregisterClass(CLASS_NAME, hActualInst);

    return 0;
}