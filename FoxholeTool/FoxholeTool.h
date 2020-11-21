#ifndef FOXHOLETOOL_H
#define FOXHOLETOOL_H
#pragma once

#include "resource.h"
#include <windows.h>

// ATL
#include <atlframe.h>
#include <atlcrack.h>

#include "NTray.h"
#include "EditFilter.h"

#include <shellapi.h>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

class __declspec(uuid("9D0B8B92-4E1C-488e-A1E1-2331AFCE2CB5")) NotificationIcon;

// Globals
HINSTANCE hInstance = NULL;	// current instance
int REGISTER_HOTKEY_F2 = 1;
int REGISTER_HOTKEY_F3 = 2;
INPUT Input = { 0 };
bool overlayIsVisible = false;
bool dragWindow = false;
CPoint oldMousePos;

// forwards
void ShowContextMenu(HWND hWnd, HINSTANCE hInstance);
void RegisterHotkeyF2(HWND hWnd);
void RegisterHotkeyF3(HWND hWnd);
void UnregisterHotkey(HWND hWnd, int hotkey);
void SetWindowStyle(HWND hWnd);

class CMainFrame :	public CFrameWindowImpl<CMainFrame>,
					public CMessageFilter,
					public CIdleHandler {
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CTrayNotifyIcon m_TrayIcon;
	HICON m_hIcon;
	HBRUSH hbrWhite, hbrBlack;
	CFont	formFont;
	CFilterEdit enemyDistance;
	CFilterEdit enemyAzimuth;
	CFilterEdit gunnerDistance;
	CFilterEdit gunnerAzimuth;


	CMainFrame() {
		m_hIcon = CTrayNotifyIcon::LoadIcon(IDI_FoxholeTool_white);
		formFont.CreateFont(-12, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("MS Sans Serif"));
		hbrWhite = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
		hbrBlack = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	}

	BEGIN_MSG_MAP_EX(CMainFrame)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_HOTKEY(OnHotKey)
		MSG_WM_LBUTTONDOWN(OnMouseDown)
		MSG_WM_LBUTTONUP(OnMouseUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MESSAGE_HANDLER_EX(WM_NOTIFYCALLBACK, OnTrayMenu)
		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnExit)
		COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnAbout)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
	END_MSG_MAP()

	BOOL PreTranslateMessage(MSG* pMsg) override {
		return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
	}

	BOOL OnIdle() noexcept override	{
		return FALSE;
	}

	int OnCreate(LPCREATESTRUCT lpCreateStruct)	{
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

		CRect rectED(10, 40, 60, 60);
		ClientToScreen(&rectED);
		enemyDistance.Create(this->operator HWND(), ((_U_RECT)rectED), _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP, 0, IDC_ENEMY_DISTANCE);
		enemyDistance.SetFont(formFont);
		enemyDistance.SetIncludeMask(_T("1234567890."));
		enemyDistance.SetLimitText(6);

		CRect rectEA(70, 40, 120, 60);
		ClientToScreen(&rectEA);
		enemyAzimuth.Create(this->operator HWND(), ((_U_RECT)rectEA), _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP, 0, IDC_ENEMY_DISTANCE);
		enemyAzimuth.SetFont(formFont);
		enemyAzimuth.SetIncludeMask(_T("1234567890."));
		enemyAzimuth.SetLimitText(6);

		CRect rectGD(10, 70, 60, 90);
		ClientToScreen(&rectGD);
		gunnerDistance.Create(this->operator HWND(), ((_U_RECT)rectGD), _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP, 0, IDC_ENEMY_DISTANCE);
		gunnerDistance.SetFont(formFont);
		gunnerDistance.SetIncludeMask(_T("1234567890."));
		gunnerDistance.SetLimitText(6);

		CRect rectGA(70, 70, 120, 90);
		ClientToScreen(&rectGA);
		gunnerAzimuth.Create(this->operator HWND(), ((_U_RECT)rectGA), _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP, 0, IDC_ENEMY_DISTANCE);
		gunnerAzimuth.SetFont(formFont);
		gunnerAzimuth.SetIncludeMask(_T("1234567890."));
		gunnerAzimuth.SetLimitText(6);

		RECT screenRect;
		::GetClientRect(GetDesktopWindow(), &screenRect);
		RECT windowRect;
		GetClientRect(&windowRect);
		RECT windowPos;
		int windowWidth = windowRect.right - windowRect.left;
		int windowHeight = windowRect.bottom - windowRect.top;
		windowPos.left = (screenRect.right / 2) - (windowWidth / 2);
		windowPos.top = (screenRect.bottom / 2) - (windowHeight / 2);
		windowPos.right = windowPos.left + windowWidth;
		windowPos.bottom = windowPos.top + windowHeight;
		ClientToScreen(&windowPos);
		MoveWindow(&windowPos, true);
		return 0;
	}

	void OnClose() {
		if (overlayIsVisible) {
			overlayIsVisible = !overlayIsVisible;
			ShowWindow(overlayIsVisible);
		} else {
			m_TrayIcon.Delete(true);
			UnregisterHotKey(this->operator HWND(), REGISTER_HOTKEY_F2);
			UnregisterHotKey(this->operator HWND(), REGISTER_HOTKEY_F3);
			SetMsgHandled(false);
			PostQuitMessage(0);
		}
	}

	LRESULT OnTrayMenu(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (LOWORD(lParam)) {
		case WM_CONTEXTMENU: {
			ShowContextMenu(this->operator HWND(), hInstance);
			break;
		}
		}
		return 0L;
	}

	void OnHotKey(int nHotKeyID, UINT uModifiers, UINT uVirtKey) {
		if (nHotKeyID == REGISTER_HOTKEY_F2) {
			SendInput(1, &Input, sizeof(INPUT));
		}
		else if (nHotKeyID == REGISTER_HOTKEY_F3) {
			overlayIsVisible = !overlayIsVisible;
			ShowWindow(overlayIsVisible);
		}
	}

	void OnMouseMove(UINT /*nFlags*/, CPoint point) {
		if (dragWindow == true && (point.x != oldMousePos.x || point.y != oldMousePos.y))	{
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

	void OnMouseUp(UINT /*nFlags*/, CPoint /*point*/) {
		dragWindow = false;
		ReleaseCapture();
	}

	void OnMouseDown(UINT /*nFlags*/, CPoint point) {
		oldMousePos.x = point.x;
		oldMousePos.y = point.y;
		dragWindow = true;
		SetCapture();
	}
	
	BOOL OnEraseBkgnd(CDCHandle dc) {
		RECT rc;
		GetClientRect(&rc);
		SetMapMode(dc, MM_ANISOTROPIC);
		SetWindowExtEx(dc, 100, 100, NULL);
		SetViewportExtEx(dc, rc.right, rc.bottom, NULL);
		FillRect(dc, &rc, hbrBlack);
		return true;
	}

	HBRUSH OnCtlColorStatic(CDCHandle dc, CStatic wndStatic) {
		SetTextColor(dc, RGB(255, 255, 255));
		SetBkColor(dc, RGB(0, 0, 0));
		return hbrWhite;
	}
	void OnExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)	{
		overlayIsVisible = false;
		PostMessage(WM_CLOSE);
	}


	void OnAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow wnd)	{
		MessageBoxW(L"Made by [3SP] Ben Button\nhttps://github.com/mmaenz/foxholetool\n\nUse F2 for automatic hammer\nUse F3 for artillery calculator\n(Set Foxhole to windowed fullscreen for overlay)\n", L"FoxholeTool v0.0.1", MB_OK);
	}
};

#endif // FOXHOLETOOL_H
