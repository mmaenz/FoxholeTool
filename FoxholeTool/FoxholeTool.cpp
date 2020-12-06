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

#include <iostream>
#include "IOConsole.h"
#include "FoxholeTool.h"

CAppModule _Module;

CMainFrame::CMainFrame() {
	m_hIcon = CTrayNotifyIcon::LoadIcon(IDI_FoxholeTool_white);
	formFont.CreateFont(-12, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("MS Sans Serif"));
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainFrame::OnIdle() noexcept {
	return FALSE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	hbrWhite = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	hbrBlack = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	SetMsgHandled(false);
	return 0;
}

LRESULT CMainFrame::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	background.LoadBitmap(MAKEINTRESOURCE(IDB_BACKGROUND));
	background.GetBitmapDimension(&bgSize);
	SetFont((HFONT)formFont);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	REGISTER_HOTKEY_F2 = GlobalAddAtomA("REGISTER_HOTKEY_F2");
	REGISTER_HOTKEY_F3 = GlobalAddAtomA("REGISTER_HOTKEY_F3");
	RegisterHotkeyF2(this->operator HWND());
	RegisterHotkeyF3(this->operator HWND());
	SetWindowStyle(this->operator HWND(), windowWidth, windowHeight);
	if (!m_TrayIcon.Create(this, IDR_TRAYPOPUP, _T("FoxholeTool\n\nF2 - use hammer (click to stop)\nF3 - show/focus artillery calculator (2x hide)"), m_hIcon, WM_NOTIFYCALLBACK, IDM_CONTEXTMENU, true)) {
		ATLTRACE(_T("Failed to create tray icon 1\n"));
		return -1;
	}

	CenterWindow();
	this->InitializeControls();
	return LRESULT();
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
		if (isDoubleKeypress) {
			overlayIsVisible = !overlayIsVisible;
			ShowWindow(overlayIsVisible);
			isDoubleKeypress = false;
			//KillTimer(DOUBLE_KEYPRESS_TIMER);
		}
		else {
			if (!overlayIsVisible) {
				overlayIsVisible = !overlayIsVisible;
				ShowWindow(overlayIsVisible);
			}
			else {
				SetActiveWindow();
				SetFocus();
				//KillTimer(DOUBLE_KEYPRESS_TIMER);
				isDoubleKeypress = true;
				SetTimer(DOUBLE_KEYPRESS_TIMER, GetDoubleClickTime());
			}
		}
	}
}

void CMainFrame::OnTimer(UINT_PTR timerId) {
	KillTimer(timerId);
	isDoubleKeypress = false;
	std::cout << "Timer off" << std::endl;
}

void CMainFrame::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
	std::cout << "keypressed" << std::endl;
}

void CMainFrame::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/) {
	if (dragWindow == true) {
		GetCursorPos(&curpoint);
		windowPos.x = curpoint.x - point.x;
		windowPos.y = curpoint.y - point.y;
		SetWindowPos(HWND_TOP, windowPos.x, windowPos.y, 0, 0, SWP_NOSIZE);
	}
}

void CMainFrame::OnMouseUp(UINT /*nFlags*/, CPoint /*point*/) {
	dragWindow = false;
	ReleaseCapture();
}

void CMainFrame::OnMouseDown(UINT /*nFlags*/, CPoint point) {
	SetCapture();
	//save current cursor coordinate
	GetCursorPos(&point);
	ScreenToClient(&point);
	dragWindow = true;
}

BOOL CMainFrame::OnEraseBkgnd(CDCHandle dc) {
	if (!background.m_hBitmap) {
		return true;
	}
	CRect rc;
	GetClientRect(&rc);
	SetMapMode(dc, MM_ANISOTROPIC);
	SetWindowExtEx(dc, 100, 100, NULL);
	SetViewportExtEx(dc, rc.right, rc.bottom, NULL);
	FillRect(dc, &rc, hbrBlack);

	/*
	CDC dcMem;
	dcMem.CreateCompatibleDC(dc);
	HBITMAP cache = dcMem.SelectBitmap(background.m_hBitmap);

	dc.BitBlt(0, 0, bgSize.cx, bgSize.cy, dcMem, 0, 0, SRCCOPY);
	dc.SelectBitmap(cache); 
	*/
	return true;
}


HBRUSH CMainFrame::OnCtlColorStatic(CDCHandle dc, CStatic wndStatic) {
	SetTextColor(dc, RGB(255, 255, 255));
	SetBkColor(dc, RGB(0, 0, 0));
	SetBkMode(dc, TRANSPARENT);
	return (HBRUSH)::GetStockObject(NULL_BRUSH);
}
void CMainFrame::OnExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/) {
	overlayIsVisible = false;
	PostMessage(WM_CLOSE);
}


void CMainFrame::OnAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow wnd) {
	MessageBoxW(L"Made by [3SP] Ben Button\nhttps://github.com/mmaenz/foxholetool\n\nUse F2 for automatic hammer\nUse F3 to show/refocus artillery calculator\nDouble F3 to hide\n(Set Foxhole to windowed fullscreen for overlay)\n", L"FoxholeTool v0.0.1", MB_OK);
}

void CMainFrame::InitializeControls(void) {
	editEnemyDistance.SubclassWindow(GetDlgItem(IDC_ENEMY_DISTANCE));
	editEnemyDistance.SetLimitText(6);
	editEnemyDistance.SetExtendedEditStyle(ES_EX_JUMPY);
	editEnemyDistance.SetIncludeMask(_T("1234567890."));
	editEnemyAzimuth.SubclassWindow(GetDlgItem(IDC_ENEMY_AZIMUTH));
	editEnemyAzimuth.SetLimitText(6);
	editEnemyAzimuth.SetExtendedEditStyle(ES_EX_JUMPY);
	editEnemyAzimuth.SetIncludeMask(_T("1234567890."));
	editGunnerDistance.SubclassWindow(GetDlgItem(IDC_GUNNER_DISTANCE));
	editGunnerDistance.SetLimitText(6);
	editGunnerDistance.SetExtendedEditStyle(ES_EX_JUMPY);
	editGunnerDistance.SetIncludeMask(_T("1234567890."));
	editGunnerAzimuth.SubclassWindow(GetDlgItem(IDC_GUNNER_AZIMUTH));
	editGunnerAzimuth.SetLimitText(6);
	editGunnerAzimuth.SetExtendedEditStyle(ES_EX_JUMPY);
	editGunnerAzimuth.SetIncludeMask(_T("1234567890."));
}

void CMainFrame::OnChar(TCHAR chChar, UINT nRepCnt, UINT nFlags) {
	std::cout << "keypress" << std::endl;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_HIDE) {
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CMainFrame wndMain;
    
	HWND hWnd = wndMain.Create(NULL);
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
#ifdef _DEBUG
	RedirectConsoleIO();
#endif
	hInstance = hInst;
    HRESULT hRes = _Module.Init(NULL, hInst);
    hRes;
    ATLASSERT(SUCCEEDED(hRes));

    int nRet = Run(lpstrCmdLine, SW_HIDE);

    _Module.Term();
    return nRet;
}

int main() {
	return _tWinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOWNORMAL);
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

void SetWindowStyle(HWND hWnd, int width, int height) {
    //RECT rect;
    //GetWindowRect(hWnd, &rect);
    //SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_NOSIZE);
    LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
    lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(hWnd, GWL_STYLE, lStyle);
    SetWindowRgn(hWnd, CreateRoundRectRgn(0, 0, width, height, 20, 20), true);
}

