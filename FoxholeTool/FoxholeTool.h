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
HWND hWnd = NULL;
int REGISTER_HOTKEY_F2 = 1;
int REGISTER_HOTKEY_F3 = 2;
int REGISTER_HOTKEY_F4 = 3;
int REGISTER_HOTKEY_SHIFTF4 = 4;
int DOUBLE_KEYPRESS_TIMER = 1;
int AUTOCLICK_TIMER = 2;
int AUTOCLICK_TIMER_INTERVAL = 1000;
INPUT MouseInput = { 0 };
INPUT KeyboardInput = { 0 };
bool isShiftF4 = false;
bool isAutoclickerRunning = false;
bool overlayIsVisible = false;
bool isDoubleKeypress = false;
bool dragWindow = false;
CPoint oldMousePos;
int windowWidth = 260;
int windowHeight = 150;

// forwards
void ShowContextMenu(HWND hWnd, HINSTANCE hInstance);
void RegisterHotkeyF2(HWND hWnd);
void RegisterHotkeyF3(HWND hWnd);
void RegisterHotkeyF4(HWND hWnd);
void RegisterHotkeyShiftF4(HWND hWnd);
void UnregisterHotkey(HWND hWnd, int hotkey);
void SetWindowStyle(HWND hWnd, int width, int height);

#include <corecrt_math_defines.h>
#define ToRadian(angleDegrees) ((angleDegrees) * M_PI / 180.0)
#define ToDegree(angleRadians) ((angleRadians) * 180.0 / M_PI)
#define Angle(angle) ((angle > 360) ? angle - 360 : angle)

typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	WS_EX_APPWINDOW> CMainFrameTraits;

class CMainFrame :	public CDialogImpl<CMainFrame>,
					public CMessageFilter,
					public CIdleHandler {
public:
	enum { IDD = IDD_FoxholeTool_DIALOG	};
	CBitmap background;
	SIZE bgSize = {};
	CTrayNotifyIcon m_TrayIcon;
	HICON m_hIcon;
	HBRUSH hbrWhite, hbrBlack;
	CFont	formFont;
	CEdit edit;
	POINT windowPos = {};
	POINT point = {};
	POINT curpoint = {};
	CMainFrame();

	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL OnIdle() noexcept override;

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void OnClose();
	LRESULT OnTrayMenu(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnHotKey(int nHotKeyID, UINT uModifiers, UINT uVirtKey);
	void OnTimer(UINT_PTR /*nIDEvent*/);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnMouseMove(UINT /*nFlags*/, CPoint point);
	void OnMouseUp(UINT /*nFlags*/, CPoint /*point*/);
	void OnMouseDown(UINT /*nFlags*/, CPoint point);
	BOOL OnEraseBkgnd(CDCHandle dc);
	HBRUSH OnCtlColorStatic(CDCHandle dc, CStatic wndStatic);
	void OnExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/);
	void OnAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow wnd);
	void InitializeControls(void);
	LRESULT OnChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	float toFloat(CString string);
	void calculate(float eD, float eA, float gD, float gA);
	void copyToClipboard(CString data);

	BEGIN_MSG_MAP_EX(CMainFrame)
		MSG_WM_CREATE(OnCreate)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_HOTKEY(OnHotKey)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER(WM_CHANGE, OnChange)
		MSG_WM_LBUTTONDOWN(OnMouseDown)
		MSG_WM_LBUTTONUP(OnMouseUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MESSAGE_HANDLER_EX(WM_NOTIFYCALLBACK, OnTrayMenu)
		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnExit)
		COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnAbout)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
	END_MSG_MAP()

private:
	CFilterEdit editEnemyDistance;
	CFilterEdit editEnemyAzimuth;
	CFilterEdit editGunnerDistance;
	CFilterEdit editGunnerAzimuth;
	CFilterEdit editResultDistance;
	CFilterEdit editResultAzimuth;
};

#endif // FOXHOLETOOL_H
