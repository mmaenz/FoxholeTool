// FoxholeTool.cpp : Defines the entry point for the application.
//
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include "stdatl.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atlmisc.h>
#include <atlctrlw.h>
#include <atlprint.h>
#include <atlfind.h>

#include "FoxholeTool.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_HIDE) {
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CMainFrame wndMain;

    if (wndMain.CreateEx() == NULL)
    {
        ATLTRACE(_T("Main window creation failed!\n"));
        return 0;
    }

    wndMain.ShowWindow(nCmdShow);

    int nRet = theLoop.Run();

    _Module.RemoveMessageLoop();
    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE /*hPrevInst*/, LPTSTR lpstrCmdLine, int nCmdShow) {
/*
    INITCOMMONCONTROLSEX iccx;
    iccx.dwSize = sizeof(iccx);
    iccx.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
    BOOL bRet = ::InitCommonControlsEx(&iccx);
    bRet;
    ATLASSERT(bRet);
*/
    hInstance = hInst;
    HRESULT hRes = _Module.Init(NULL, hInst);
    hRes;
    ATLASSERT(SUCCEEDED(hRes));

    int nRet = Run(lpstrCmdLine, SW_HIDE);

    _Module.Term();
    return nRet;
}

bool AddNotificationIcon(HWND hwnd, HINSTANCE hInstance) {
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = hwnd;
    // add the icon, setting the icon, tooltip, and callback message.
    // the icon will be identified with the GUID
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_GUID;
    nid.guidItem = __uuidof(NotificationIcon);
    nid.uCallbackMessage = WM_NOTIFYCALLBACK;
    LoadIconMetric(hInstance, MAKEINTRESOURCE(IDI_FoxholeTool_white), LIM_SMALL, &nid.hIcon);
    LoadString(hInstance, IDS_APP_TITLE, nid.szTip, ARRAYSIZE(nid.szTip));
    Shell_NotifyIcon(NIM_ADD, &nid);
    // NOTIFYICON_VERSION_4 is prefered
    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

bool DeleteNotificationIcon() {
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.uFlags = NIF_GUID;
    nid.guidItem = __uuidof(NotificationIcon);
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowContextMenu(HWND hwnd, HINSTANCE hInstance) {
    HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDM_CONTEXTMENU));
    if (hMenu) {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu) {
            // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
            SetForegroundWindow(hwnd);
            POINT pt;
            GetCursorPos(&pt);
            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON | TPM_BOTTOMALIGN;
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
                uFlags |= TPM_RIGHTALIGN;
            } else {
                uFlags |= TPM_LEFTALIGN;
            }
            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}

void RegisterHotkeyF2(HWND hWnd) {
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    RegisterHotKey(
        hWnd,
        REGISTER_HOTKEY_F2,
        MOD_NOREPEAT,
        VK_F2);
}

void RegisterHotkeyF3(HWND hWnd) {
    RegisterHotKey(
        hWnd,
        REGISTER_HOTKEY_F3,
        MOD_NOREPEAT,
        VK_F3);
}

void UnregisterHotkey(HWND hWnd, int hotkey) {
    UnregisterHotKey(hWnd, hotkey);
}

/*

#define MAX_LOADSTRING 100

// Use a guid to uniquely identify our icon
class __declspec(uuid("9D0B8B92-4E1C-488e-A1E1-2331AFCE2CB5")) NotificationIcon;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int REGISTERHOLD = 1;
INPUT Input = { 0 };
UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL                AddNotificationIcon(HWND hwnd);
BOOL                DeleteNotificationIcon();
void                ShowContextMenu(HWND hwnd, POINT pt);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    REGISTERHOLD = GlobalAddAtomA("REGISTER_HOTKEY");
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_FoxholeTool, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FoxholeTool));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDS_FoxholeTool);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL AddNotificationIcon(HWND hwnd)
{
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = hwnd;
    // add the icon, setting the icon, tooltip, and callback message.
    // the icon will be identified with the GUID
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_GUID;
    nid.guidItem = __uuidof(NotificationIcon);
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    LoadIconMetric(hInst, MAKEINTRESOURCE(IDI_FoxholeTool_white), LIM_SMALL, &nid.hIcon);
    LoadString(hInst, IDS_APP_TITLE, nid.szTip, ARRAYSIZE(nid.szTip));
    Shell_NotifyIcon(NIM_ADD, &nid);

    // NOTIFYICON_VERSION_4 is prefered
    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL DeleteNotificationIcon()
{
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.uFlags = NIF_GUID;
    nid.guidItem = __uuidof(NotificationIcon);
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}


void ShowContextMenu(HWND hwnd, POINT pt) {
    HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDM_CONTEXTMENU));
    if (hMenu)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu)
        {
            // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
            SetForegroundWindow(hwnd);
            GetCursorPos(&pt);
            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON | TPM_BOTTOMALIGN;
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
                uFlags |= TPM_RIGHTALIGN;
            } else {
                uFlags |= TPM_LEFTALIGN;
            }
            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   RegisterHotKey(
       hWnd,
       REGISTERHOLD,
       MOD_NOREPEAT,
       VK_F2);
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WMAPP_NOTIFYCALLBACK:
        switch (LOWORD(lParam)) {
            case WM_CONTEXTMENU: {
                POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
                ShowContextMenu(hWnd, pt);
            }
            break;
        }
        break;    
    case WM_CREATE:
        if (!AddNotificationIcon(hWnd))
        {
            MessageBox(hWnd,
                L"Please read the ReadMe.txt file for troubleshooting",
                L"Error adding icon", MB_OK);
            return -1;
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
            case IDM_EXIT: {
                DestroyWindow(hWnd);
                break;
            default: break;
            }
        }
        break;
    case WM_HOTKEY:        
        SendInput(1, &Input, sizeof(INPUT));
        break;
    case WM_DESTROY:
        UnregisterHotKey(hWnd, REGISTERHOLD);
        DeleteNotificationIcon();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

*/