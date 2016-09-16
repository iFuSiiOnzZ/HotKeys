
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

#define UNUSED(x) (void)(x)

static int  g_KbdHookExit = 0;                    /* Exit app controll */
static JS_NODE *g_pRootNode = 0;                   /* Json hot keys bindings */

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

                if(CreateProcessA(pAppPath, NULL, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi))
                {
                    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
                }
            }
        }

        if(KeyboardData->vkCode == 'E' && (GetAsyncKeyState(VK_CONTROL) & GetAsyncKeyState(VK_MENU) & 0x8000))
        {
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

    char *pFileContent = LoadStringFileIntoMemory("hotkeys.json");
    JS_TOKENIZER Tokenizer = { pFileContent };

    g_pRootNode = json_root();
    json_parser(g_pRootNode, &Tokenizer);

    json_sanitize(g_pRootNode);

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

    if(g_pRootNode) json_clear(g_pRootNode);
    if(pFileContent) free(pFileContent);

    UnhookWindowsHookEx(KbdHook);
    UnregisterClass(CLASS_NAME, hActualInst);

    return 0;
}