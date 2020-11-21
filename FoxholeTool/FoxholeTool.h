#ifndef FOXHOLETOOL_H
#define FOXHOLETOOL_H
#pragma once

#include "resource.h"

#include <windows.h>

// ATL
#include <atlframe.h>
#include <atlcrack.h>

#include <shellapi.h>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

class __declspec(uuid("9D0B8B92-4E1C-488e-A1E1-2331AFCE2CB5")) NotificationIcon;

// Globals
HINSTANCE hInstance;	// current instance
int REGISTER_HOTKEY_F2 = 1;
int REGISTER_HOTKEY_F3 = 2;
INPUT Input = { 0 };

// forwards
bool AddNotificationIcon(HWND hWnd, HINSTANCE hInstance);
bool DeleteNotificationIcon();
void ShowContextMenu(HWND hWnd, HINSTANCE hInstance);
void RegisterHotkeyF2(HWND hWnd);
void RegisterHotkeyF3(HWND hWnd);
void UnregisterHotkey(HWND hWnd, int hotkey);

class CMainFrame :	public CFrameWindowImpl<CMainFrame>, 
					public CMessageFilter,
					public CIdleHandler {
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	BEGIN_MSG_MAP_EX(CMainFrame)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_HOTKEY(OnHotKey)
		MESSAGE_HANDLER_EX(WM_NOTIFYCALLBACK, OnTrayMenu)
		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnExit)
		COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnAbout)
	END_MSG_MAP()

	BOOL PreTranslateMessage(MSG* pMsg) override {
		return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
	}

	BOOL OnIdle() noexcept override	{
		return FALSE;
	}

	int OnCreate(LPCREATESTRUCT lpCreateStruct)	{
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);
		
		REGISTER_HOTKEY_F2 = GlobalAddAtomA("REGISTER_HOTKEY_F2");
		REGISTER_HOTKEY_F3 = GlobalAddAtomA("REGISTER_HOTKEY_F3");
		RegisterHotkeyF2(this->operator HWND());
		RegisterHotkeyF3(this->operator HWND());
		AddNotificationIcon(this->operator HWND(), hInstance);
		return 1;
	}

	void OnClose() {
		DeleteNotificationIcon();
		UnregisterHotKey(this->operator HWND(), REGISTER_HOTKEY_F2);
		UnregisterHotKey(this->operator HWND(), REGISTER_HOTKEY_F3);
		SetMsgHandled(false);
		PostQuitMessage(0);
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

		}
	}

	void OnExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)	{
		PostMessage(WM_CLOSE);
	}


	void OnAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow wnd)	{
		//CSimpleDialog<IDD_ABOUTBOX> dlg;
		//dlg.DoModal(this->operator HWND());
		MessageBoxW(L"Made by Ben Button\nhttps://github.com/mmaenz/foxholetool\n\nUse F2 for automatic hammer\nUse F3 for artillery calculator\n(Set Foxhole to windowed fullscreen for overlay)\n", L"FoxholeTool v0.0.1", MB_OK);
	}
};

#endif // FOXHOLETOOL_H
