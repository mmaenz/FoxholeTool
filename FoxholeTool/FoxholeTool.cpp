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

CMainFrame::CMainFrame() {
	m_hIcon = CTrayNotifyIcon::LoadIcon(IDI_FoxholeTool_white);
	formFont.CreateFont(-12, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("MS Sans Serif"));
	hbrWhite = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	hbrBlack = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() noexcept {
	return FALSE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	SetFont((HFONT)formFont);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	REGISTER_HOTKEY_F2 = GlobalAddAtomA("REGISTER_HOTKEY_F2");
	REGISTER_HOTKEY_F3 = GlobalAddAtomA("REGISTER_HOTKEY_F3");
	RegisterHotkeyF2(this->operator HWND());
	RegisterHotkeyF3(this->operator HWND());
	SetWindowStyle(this->operator HWND());
	if (!m_TrayIcon.Create(this, IDR_TRAYPOPUP, _T("FoxholeTool\n\nF2 - use hammer\nF3 - artillery calculator"), m_hIcon, WM_NOTIFYCALLBACK, IDM_CONTEXTMENU, true)) {
		ATLTRACE(_T("Failed to create tray icon 1\n"));
		return -1;
	}
	return 0;
}

void CMainFrame::OnClose() {
	if (overlayIsVisible) {
		overlayIsVisible = !overlayIsVisible;
		ShowWindow(overlayIsVisible);
	}
	else {
		m_TrayIcon.Delete(true);
		UnregisterHotKey(this->operator HWND(), REGISTER_HOTKEY_F2);
		UnregisterHotKey(this->operator HWND(), REGISTER_HOTKEY_F3);
		SetMsgHandled(false);
		PostQuitMessage(0);
	}
}

LRESULT CMainFrame::OnTrayMenu(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (LOWORD(lParam)) {
	case WM_CONTEXTMENU: {
		ShowContextMenu(this->operator HWND(), hInstance);
		break;
	}
	}
	return 0L;
}

void CMainFrame::OnHotKey(int nHotKeyID, UINT uModifiers, UINT uVirtKey) {
	if (nHotKeyID == REGISTER_HOTKEY_F2) {
		SendInput(1, &Input, sizeof(INPUT));
	}
	else if (nHotKeyID == REGISTER_HOTKEY_F3) {
		overlayIsVisible = !overlayIsVisible;
		ShowWindow(overlayIsVisible);
		if (overlayIsVisible && (windowPos.left == 0 && windowPos.top == 0 && windowPos.right == 0 && windowPos.bottom == 0)) {
			RECT screenRect = {};
			::GetClientRect(GetDesktopWindow(), &screenRect);
			RECT windowRect = {};
			GetClientRect(&windowRect);
			ClientToScreen(&windowRect);
			int windowWidth = windowRect.right - windowRect.left;
			int windowHeight = windowRect.bottom - windowRect.top;
			windowPos.left = (screenRect.right / 2) - (windowWidth / 2);
			windowPos.top = (screenRect.bottom / 2) - (windowHeight / 2);
			windowPos.right = windowPos.left + windowWidth;
			windowPos.bottom = windowPos.top + windowHeight;
			ClientToScreen(&windowPos);
			MoveWindow(&windowPos, true);
		}
		else {
			MoveWindow(&windowPos, true);
		}
	}
}

void CMainFrame::OnMouseMove(UINT /*nFlags*/, CPoint point) {
	if (dragWindow == true && (point.x != oldMousePos.x || point.y != oldMousePos.y)) {
		int dx = point.x - oldMousePos.x;
		int dy = point.y - oldMousePos.y;
		oldMousePos.x = point.x;
		oldMousePos.y = point.y;

		RECT mainWindowRect;
		int windowWidth, windowHeight;
		GetWindowRect(&mainWindowRect);
		windowHeight = mainWindowRect.bottom - mainWindowRect.top;
		windowWidth = mainWindowRect.right - mainWindowRect.left;

		/*
		CPoint pos(mainWindowRect.left, mainWindowRect.top);
		pos.x = pos.x + dx;
		pos.y = pos.y + dy;
		*/

		ClientToScreen(&point);
		MoveWindow(point.x, point.y, windowWidth, windowHeight, TRUE);
	}
}

void CMainFrame::OnMouseUp(UINT /*nFlags*/, CPoint /*point*/) {
	dragWindow = false;
	ReleaseCapture();
}

void CMainFrame::OnMouseDown(UINT /*nFlags*/, CPoint point) {
	oldMousePos.x = point.x;
	oldMousePos.y = point.y;
	dragWindow = true;
	SetCapture();
}

BOOL CMainFrame::OnEraseBkgnd(CDCHandle dc) {
	RECT rc;
	GetClientRect(&rc);
	SetMapMode(dc, MM_ANISOTROPIC);
	SetWindowExtEx(dc, 100, 100, NULL);
	SetViewportExtEx(dc, rc.right, rc.bottom, NULL);
	FillRect(dc, &rc, hbrBlack);
	return true;
}

HBRUSH CMainFrame::OnCtlColorStatic(CDCHandle dc, CStatic wndStatic) {
	SetTextColor(dc, RGB(255, 255, 255));
	SetBkColor(dc, RGB(0, 0, 0));
	return hbrWhite;
}
void CMainFrame::OnExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/) {
	overlayIsVisible = false;
	PostMessage(WM_CLOSE);
}


void CMainFrame::OnAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow wnd) {
	MessageBoxW(L"Made by [3SP] Ben Button\nhttps://github.com/mmaenz/foxholetool\n\nUse F2 for automatic hammer\nUse F3 for artillery calculator\n(Set Foxhole to windowed fullscreen for overlay)\n", L"FoxholeTool v0.0.1", MB_OK);
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_HIDE) {
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CMainFrame wndMain;
    HWND hWnd = wndMain.CreateEx();
    if (hWnd == NULL) {
        ATLTRACE(_T("Main window creation failed!\n"));
        return 0;
    }

    wndMain.ShowWindow(nCmdShow);

    int nRet = theLoop.Run();
    _Module.RemoveMessageLoop();
    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE /*hPrevInst*/, LPTSTR lpstrCmdLine, int nCmdShow) {
    hInstance = hInst;
    HRESULT hRes = _Module.Init(NULL, hInst);
    hRes;
    ATLASSERT(SUCCEEDED(hRes));

    int nRet = Run(lpstrCmdLine, SW_HIDE);

    _Module.Term();
    return nRet;
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

void SetWindowStyle(HWND hWnd) {
    RECT rect;
    GetWindowRect(hWnd, &rect);
    SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_NOSIZE);
    LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
    lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(hWnd, GWL_STYLE, lStyle);
    SetWindowRgn(hWnd, CreateRoundRectRgn(0, 0, 562, 257, 20, 20), true);
}

