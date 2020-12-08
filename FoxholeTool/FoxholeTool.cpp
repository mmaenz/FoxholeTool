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
#include <sstream>
#include "IOConsole.h"
#include "FoxholeTool.h"

CAppModule _Module;

CMainFrame::CMainFrame() {
	m_hIcon = CTrayNotifyIcon::LoadIcon(IDI_FoxholeTool_white);
	formFont.CreateFont(-12, 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("MS Sans Serif"));
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainFrame::OnIdle() noexcept {
	return FALSE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	//SetMsgHandled(false);
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
	
	hbrWhite = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	hbrBlack = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));

	CRect rect;
	MapDialogRect(&rect);
	windowWidth = rect.right;
	windowHeight = rect.bottom;
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
				SetForegroundWindow(this->operator HWND());
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
}

void CMainFrame::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
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

	CDC mdc;
	mdc.CreateCompatibleDC(dc);
	mdc.SelectBitmap(background);
	CRect rClient;
	GetClientRect(rClient);
	dc.FillSolidRect(rClient, RGB(255, 255, 255));
	
	BITMAP bmp;
	background.GetBitmap(&bmp); // Get bitmap info
	CRect rCenterBmp = {};
	rCenterBmp.left = 0; // center horizontal
	rCenterBmp.right = bmp.bmWidth;
	rCenterBmp.bottom = bmp.bmHeight;
	rCenterBmp.top = 0;
	dc.StretchBlt(0, 0, rClient.right, rClient.bottom, mdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	return TRUE; // CBaseClass::OnEraseBkgnd(pDC); Don't let the baseclass do this -&gt; it may also draw the background
}


HBRUSH CMainFrame::OnCtlColorStatic(CDCHandle dc, CStatic wndStatic) {
	if (wndStatic.GetDlgCtrlID() == IDC_LABEL_STATIC) {
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(RGB(255, 255, 255));
		dc.SelectFont(formFont);
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}
	return (HBRUSH)::GetStockObject(WHITE_BRUSH);
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
	editEnemyDistance.SetLimitText(3);
	editEnemyDistance.SetExtendedEditStyle(ES_EX_JUMPY);
	editEnemyDistance.SetIncludeMask(_T("1234567890"));
	editEnemyAzimuth.SubclassWindow(GetDlgItem(IDC_ENEMY_AZIMUTH));
	editEnemyAzimuth.SetLimitText(3);
	editEnemyAzimuth.SetExtendedEditStyle(ES_EX_JUMPY);
	editEnemyAzimuth.SetIncludeMask(_T("1234567890"));
	editGunnerDistance.SubclassWindow(GetDlgItem(IDC_GUNNER_DISTANCE));
	editGunnerDistance.SetLimitText(3);
	editGunnerDistance.SetExtendedEditStyle(ES_EX_JUMPY);
	editGunnerDistance.SetIncludeMask(_T("1234567890"));
	editGunnerAzimuth.SubclassWindow(GetDlgItem(IDC_GUNNER_AZIMUTH));
	editGunnerAzimuth.SetLimitText(3);
	editGunnerAzimuth.SetExtendedEditStyle(ES_EX_JUMPY);
	editGunnerAzimuth.SetIncludeMask(_T("1234567890"));
	editResultDistance.SubclassWindow(GetDlgItem(IDC_RESULT_DISTANCE));
	editResultAzimuth.SubclassWindow(GetDlgItem(IDC_RESULT_AZIMUTH));
}

LRESULT CMainFrame::OnChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CString sWindowText;
	editEnemyDistance.GetWindowText(sWindowText);
	float eD = toFloat(sWindowText);
	if (eD == -1) {
		editResultDistance.SetWindowText(_T(""));
		editResultAzimuth.SetWindowText(_T(""));
		copyToClipboard(_T(""));
		return LRESULT();
	}

	editEnemyAzimuth.GetWindowText(sWindowText);
	float eA = toFloat(sWindowText);
	if (eA == -1 || eA > 360) {
		editResultDistance.SetWindowText(_T(""));
		editResultAzimuth.SetWindowText(_T(""));
		copyToClipboard(_T(""));
		return LRESULT();
	}

	editGunnerDistance.GetWindowText(sWindowText);
	float gD = toFloat(sWindowText);
	if (gD == -1) {
		editResultDistance.SetWindowText(_T(""));
		editResultAzimuth.SetWindowText(_T(""));
		copyToClipboard(_T(""));
		return LRESULT();
	}

	editGunnerAzimuth.GetWindowText(sWindowText);
	float gA = toFloat(sWindowText);
	if (gA == -1 || gA > 360) {
		editResultDistance.SetWindowText(_T(""));
		editResultAzimuth.SetWindowText(_T(""));
		copyToClipboard(_T(""));
		return LRESULT();
	}

	if (gA == eA && gD == eD) {
		editResultDistance.SetWindowText(_T(""));
		editResultAzimuth.SetWindowText(_T(""));
		copyToClipboard(_T(""));
		return LRESULT();
	}

	calculate(eD, eA, gD, gA);

	return LRESULT();
}

float CMainFrame::toFloat(CString toFloat) {
	std::istringstream iss(CStringA(toFloat).GetString());
	float f;
	iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
	// Check the entire string was consumed and if either failbit or badbit is set
	if (iss.eof() && !iss.fail()) {
		if (f <= 360 && f >= 0) {
			return (float)(std::ceilf(f * 100.0f) / 100.0f);
		}
	}
	return -1;
}

void CMainFrame::calculate(float eD, float eA, float gD, float gA) {
	float aD = 0.f;
	float rD = 0.f;
	float aS = 0.f;
	float rA = 0.f;

	aD = (float)((eA > gA) ? ToRadian(eA - gA) : ToRadian(gA - eA));

	rD = std::sqrt(eD * eD + gD * gD - 2 * eD * gD * std::cos(aD));

	if (rD >= 45 && eD != 0) {
		aS = (float)std::roundf((float)ToDegree(std::acosf((-(eD * eD) + gD * gD + rD * rD) / (2 * gD * rD))));
		if (Angle(ToDegree(aD)) > 180) {
			rA = (eA > gA) ? gA + 180 + aS : gA + 180 - aS;
		} else {
			if (eA > gA) {
				rA = gA + 180 - aS;
			} else {
				rA = gA + 180 + aS;
			}
		}
		
		rA = Angle(std::roundf(rA));
		
		CString resultDistance;
		resultDistance.Format(_T("%.0f"), rD);
		CString resultAzimuth;
		resultAzimuth.Format(_T("%.0f"), rA);
		editResultDistance.SetWindowText(resultDistance);
		editResultAzimuth.SetWindowText(resultAzimuth);
		CString result;
		result.Format(_T("Distance %.0f / Azimuth %.0f"), rD, rA);
		copyToClipboard(result);
	}
}

void CMainFrame::copyToClipboard(CString data) {
	if (!OpenClipboard()) {
		return;
	}
	// Remove the current Clipboard contents
	if (!EmptyClipboard()) {
		return;
	}

	size_t size = sizeof(TCHAR) * (1 + data.GetLength());
	HGLOBAL hResult = GlobalAlloc(GMEM_MOVEABLE, size);
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hResult);
	memcpy(lptstrCopy, data.GetBuffer(), size);
	GlobalUnlock(hResult);
#ifndef _UNICODE
	if (::SetClipboardData(CF_TEXT, hResult) == NULL)
#else
	if (::SetClipboardData(CF_UNICODETEXT, hResult) == NULL)
#endif
	{
		GlobalFree(hResult);
		CloseClipboard();
		return;
	}
	CloseClipboard();
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
	int nRet = 0;
	try {
		HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, _T("FoxholeTool_is_running"));
		if (!hMutex) {
			hMutex = CreateMutex(0, 0, _T("FoxholeTool_is_running"));
		}
		else {
			return 0;
		}

		hInstance = hInst;
		HRESULT hRes = _Module.Init(NULL, hInst);
		ATLASSERT(SUCCEEDED(hRes));

		nRet = Run(lpstrCmdLine, SW_HIDE);

		_Module.Term();
		ReleaseMutex(hMutex);
	}
	catch (...) {
		MessageBoxA(NULL, "Something went horribly wrong!", "I'm tired", MB_OK);
	}
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

