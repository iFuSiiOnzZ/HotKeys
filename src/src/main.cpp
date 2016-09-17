
#include <Windows.h> /* CreateProcessA, CloseHandle */
#include <shlobj.h>  /* SHGetFolderPathA, CSIDL_MYDOCUMENTS */

#include <string.h>  /* strcat_s, strlen, strtok_s*/

#include <js_reader.h> /* JS_NODE, JS_TOKENIZER, json_root, json_parser, json_sanitize, json_clear */
#include <utils.h>     /* LoadStringFileIntoMemory, LogToFile */

// https://msdn.microsoft.com/en-us/library/ms644990(VS.85).aspx                        WH_KEYBOARD_DL
// https://msdn.microsoft.com/en-us/library/ms644985(v=vs.85).aspx                      LowLevelKeyboardProc
// https://msdn.microsoft.com/es-es/library/windows/desktop/ms644967(v=vs.85).aspx      KBDLLHOOKSTRUCT


#define CLASS_NAME "KeyboardHook"
#define WINDOW_NAME "KeyboardHook"

#define TRY_ICON_ID 1024
#define UNUSED(x) (void)(x)

static HMENU g_hTrayWnd = 0;        /* Tray menu */
static int g_KbdHookExit    = 0;    /* Exit app controll */
static JS_NODE *g_pRootNode = 0;    /* Json hot keys bindings */

void AddMenu(HMENU hMenu, int Id, char *WndText)
{
    MENUITEMINFO MenuItem = { 0 };
    MenuItem.cbSize = sizeof(MenuItem);

    MenuItem.fMask = MIIM_ID | MIIM_STATE | MIIM_DATA | MIIM_TYPE;
    MenuItem.fState =  MFS_UNCHECKED | MFS_ENABLED;
    MenuItem.fType = MFT_STRING;

    MenuItem.dwTypeData = (char *)WndText;
    MenuItem.wID = Id;

    InsertMenuItem(hMenu, GetMenuItemCount(hMenu), true, &MenuItem);
}

LRESULT CALLBACK LowLevelKeyboardProc(int action, WPARAM wp, LPARAM lp)
{
    if (action == HC_ACTION && (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN))
    {
        KBDLLHOOKSTRUCT *KeyboardData = (KBDLLHOOKSTRUCT *)lp;
        int js = json_size(g_pRootNode, "root");
       
        for(int i = 0; i < js; ++i)
        {
            int pArrays[1] = { i };
            char pHotKeysTemp[32] = { 0 };

            char *pAppPath = json_value(g_pRootNode, "root.path", pArrays, 1);
            char *pHotKeys = json_value(g_pRootNode, "root.hotkeys", pArrays, 1);
            char *pStartIn = json_value(g_pRootNode, "root.startin", pArrays, 1);

            for(int j = 0; j < 31 && pHotKeys[j]; ++j)
            {
                pHotKeysTemp[j] = pHotKeys[j];
            }

            char *pContext = 0; int Execute = 1;
            char *pToken = strtok_s(pHotKeysTemp, "+", &pContext);
            
            while(pToken && Execute)
            {
                if(!strcmp(pToken, "CTRL"))
                {
                    Execute = GetAsyncKeyState(VK_CONTROL) & 0x8000;
                }
                else if(!strcmp(pToken, "ALT"))
                {
                    Execute = GetAsyncKeyState(VK_MENU) & 0x8000;
                }
                else if(!strcmp(pToken, "SHIFT"))
                {
                    Execute = GetAsyncKeyState(VK_SHIFT) & 0x8000;
                }
                else if(*pToken >= 'A' && *pToken <= 'Z')
                {
                    Execute = KeyboardData->vkCode == (unsigned char)*pToken;
                }
                else 
                {
                    Execute = 0;
                }

                pToken = strtok_s(0, "+", &pContext);
            }

            if(Execute)
            {
                STARTUPINFO si = { sizeof(si),  0 };
                PROCESS_INFORMATION pi = { 0 };

                if(CreateProcessA(pAppPath, NULL, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, pStartIn, &si, &pi))
                {
                    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
                }
            }
        }
    }

    return CallNextHookEx(NULL, action, wp, lp);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wp, LPARAM lp)
{
    if(Msg == WM_CREATE)
    {
        g_hTrayWnd = CreatePopupMenu();

        AddMenu(g_hTrayWnd, 0, "Close this menu");
        AddMenu(g_hTrayWnd, 1, "Exit KeyBindings");

        return 0;
    }
    else if(Msg == WM_CLOSE || Msg == WM_DESTROY)
    {
        g_KbdHookExit = 1;
        return 0;
    }
    else if(Msg == TRY_ICON_ID && lp == WM_RBUTTONDOWN)
    {
        POINT CursorPos = { 0 };
        GetCursorPos(&CursorPos);

        int MenuClicked = TrackPopupMenu(g_hTrayWnd, TPM_RETURNCMD | TPM_NONOTIFY, CursorPos.x, CursorPos.y, 0, hWnd, NULL);
        if(MenuClicked == 1){ DestroyMenu(g_hTrayWnd); g_KbdHookExit = 1; }

        return 0;
    }

    return DefWindowProcA(hWnd, Msg, wp, lp);
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

int WINAPI WinMain(HINSTANCE hActualInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow)
{
    UNUSED(hPrevInst);
    UNUSED(cmdLine);
    UNUSED(cmdShow);

    WNDCLASSEXA WndCls = { 0 };

    WndCls.hInstance        = hActualInst;
    WndCls.hCursor          = LoadCursor(NULL, IDC_ARROW);
    WndCls.hIconSm          = LoadIcon(NULL, IDI_APPLICATION);
    WndCls.hIcon            = LoadIcon(NULL, IDI_APPLICATION);

    WndCls.lpfnWndProc      = WndProc;
    WndCls.lpszClassName    = CLASS_NAME;

    WndCls.cbSize           = sizeof(WNDCLASSEXA);
    WndCls.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);

    if(RegisterClassExA(&WndCls) == 0)
    {
        ShowError("Window Registration Failed");
        return 1;
    }

    HWND hWnd = CreateWindowExA
    (
        WS_EX_CLIENTEDGE, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, 0, 0,
        300, 50, HWND_DESKTOP, NULL, hActualInst, NULL
    );

    if(hWnd == NULL)
    {
        ShowError("Window Creation Failed");
        return 1;
    }

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    HHOOK KbdHook = SetWindowsHookExA(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, hActualInst, NULL);
    MSG hMsg = { 0 };

    char *pFileContent = LoadStringFileIntoMemory("hotkeys.json");
    JS_TOKENIZER Tokenizer = { pFileContent };

    g_pRootNode = json_root();
    json_parser(g_pRootNode, &Tokenizer);
    json_sanitize(g_pRootNode);

    NOTIFYICONDATA TrayIcon = { 0 };
    EnableTrayIcon(&TrayIcon, hWnd, TRY_ICON_ID);

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

    DisableTrayIcon(&TrayIcon);

    if(pFileContent) free(pFileContent);
    if(g_pRootNode) json_clear(g_pRootNode);

    DestroyWindow(hWnd);
    UnhookWindowsHookEx(KbdHook);
    UnregisterClass(CLASS_NAME, hActualInst);

    return 0;
}