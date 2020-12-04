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
int DOUBLE_KEYPRESS_TIMER = 1;
INPUT Input = { 0 };
bool overlayIsVisible = false;
bool isDoubleKeypress = false;
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
	CBitmap bg;
	CTrayNotifyIcon m_TrayIcon;
	HICON m_hIcon;
	HBRUSH hbrWhite, hbrBlack;
	CFont	formFont;
	CFilterEdit enemyDistance;
	CFilterEdit enemyAzimuth;
	CFilterEdit gunnerDistance;
	CFilterEdit gunnerAzimuth;
	CEdit edit;
	POINT windowPos = {};
	POINT point = {};
	POINT curpoint = {};
	CMainFrame();

	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL OnIdle() noexcept override;

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnClose();
	LRESULT OnTrayMenu(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnHotKey(int nHotKeyID, UINT uModifiers, UINT uVirtKey);
	void OnTimer(UINT_PTR /*nIDEvent*/);
	void OnMouseMove(UINT /*nFlags*/, CPoint point);
	void OnMouseUp(UINT /*nFlags*/, CPoint /*point*/);
	void OnMouseDown(UINT /*nFlags*/, CPoint point);
	BOOL OnEraseBkgnd(CDCHandle dc);
	HBRUSH OnCtlColorStatic(CDCHandle dc, CStatic wndStatic);
	void OnExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow wnd);

	BEGIN_MSG_MAP_EX(CMainFrame)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_HOTKEY(OnHotKey)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_LBUTTONDOWN(OnMouseDown)
		MSG_WM_LBUTTONUP(OnMouseUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MESSAGE_HANDLER_EX(WM_NOTIFYCALLBACK, OnTrayMenu)
		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnExit)
		COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnAbout)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
	END_MSG_MAP()
};

#endif // FOXHOLETOOL_H
