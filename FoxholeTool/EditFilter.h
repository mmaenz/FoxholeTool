#if !defined(AFX_FILTERINGEDIT_H__20010604_1C88_3217_03F4_0080AD509054__INCLUDED_)
#define AFX_FILTERINGEDIT_H__20010604_1C88_3217_03F4_0080AD509054__INCLUDED_

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CFilterEdit - Filtering Edit control
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001 Bjarke Viksoe.
//
// Thanks to Yarp (Senosoft.com) for the Alt+Gr fix.
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLCTRLS_H__
  #error FilterEdit.h requires atlctrls.h to be included first
#endif

#if (WINVER >= 0x0500)
  #include <shlwapi.h>
  #pragma comment(lib, "shlwapi.lib")
#elif defined(_ATL_MIN_CRT)
  #error No minimal build because of tchar string operations
#endif


/////////////////////////////////////////////////////////////////////////////
// CFilterEdit

#define ES_EX_JUMPY           0x00000001
#define ES_EX_INCLUDEMASK     0x00000002
#define ES_EX_EXCLUDEMASK     0x00000004
#define WM_CHANGE             (WM_USER + 99)

class CFilterEdit : 
   public CWindowImpl< CFilterEdit, CEdit, CControlWinTraits >
{
public:
   DECLARE_WND_SUPERCLASS(_T("WTL_FilterEdit"), CEdit::GetWndClassName())  

   BEGIN_MSG_MAP(CFilterEdit)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_CHAR, OnChar)
      MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
      MESSAGE_HANDLER(WM_SETTEXT, OnSetText)
      MESSAGE_HANDLER(WM_PASTE, OnPaste)
   END_MSG_MAP()

   enum { MAX_MASK_LEN = 64 };

   DWORD m_dwExtStyle;
   TCHAR m_szExclude[MAX_MASK_LEN+1];
   TCHAR m_szInclude[MAX_MASK_LEN+1];
   // Operations

   BOOL SubclassWindow(HWND hWnd)
   {
      ATLASSERT(m_hWnd == NULL);
      ATLASSERT(::IsWindow(hWnd));
      BOOL bRet = CWindowImpl< CFilterEdit, CEdit, CControlWinTraits >::SubclassWindow(hWnd);
      if( bRet ) _Init();
      return bRet;
   }

   DWORD GetExtendedEditStyle() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return m_dwExtStyle;
   }

   void SetExtendedEditStyle(DWORD dwStyle)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      m_dwExtStyle = dwStyle;
      Invalidate();
   }

   void SetExcludeMask(LPCTSTR pstrMask)
   {
      ATLASSERT(!::IsBadStringPtr(pstrMask, MAX_MASK_LEN));
      m_dwExtStyle |= ES_EX_EXCLUDEMASK;
      ::lstrcpyn(m_szExclude, pstrMask, MAX_MASK_LEN);
   }

   void SetIncludeMask(LPCTSTR pstrMask)
   {
      ATLASSERT(!::IsBadStringPtr(pstrMask, MAX_MASK_LEN));
      m_dwExtStyle |= ES_EX_INCLUDEMASK;
      ::lstrcpyn(m_szInclude, pstrMask, MAX_MASK_LEN);
   }

   // Implementation

   void _Init()
   {
      ATLASSERT(::IsWindow(m_hWnd));

#ifdef _DEBUG
      // Check class
      TCHAR szBuffer[16];
      if( ::GetClassName(m_hWnd, szBuffer, (sizeof(szBuffer)/sizeof(TCHAR))-1) ) {
         ATLASSERT(::lstrcmpi(szBuffer, CEdit::GetWndClassName())==0);
         ATLASSERT(GetStyle() & WS_CHILD);
         ATLASSERT((GetStyle() & (ES_LOWERCASE|ES_UPPERCASE|ES_PASSWORD|ES_READONLY))==0);
      }
#endif // _DEBUG

      ::ZeroMemory(m_szExclude, sizeof(m_szExclude)/sizeof(TCHAR));
      ::ZeroMemory(m_szInclude, sizeof(m_szInclude)/sizeof(TCHAR));
      m_dwExtStyle = 0;
   }

   BOOL _NextDlgItem()
   {
      POINT pt;
      ::GetCaretPos(&pt);
      if( CharFromPos(pt)==(int)GetLimitText() ) {
          CEdit next = ::GetNextDlgTabItem(GetParent(), m_hWnd, FALSE);
          ::SetFocus( next );
          next.SetSel(0, -1);
         return TRUE;
      }
      return FALSE;
   }

   BOOL _CheckText(LPCTSTR pstr)
   {
      if( pstr==NULL ) return TRUE;
      ATLASSERT(!::IsBadStringPtr(pstr, -1));
      if( m_dwExtStyle & ES_EX_EXCLUDEMASK ) {
#if (WINVER >= 0x0500)
         if( ::StrPBrk(pstr, m_szExclude)!=NULL ) return FALSE;
#else
         if( _tcspbrk(pstr, m_szExclude)!=NULL ) return FALSE;
#endif
      }
      if( m_dwExtStyle & ES_EX_INCLUDEMASK ) {
         while( *pstr ) {
#if (WINVER >= 0x0500)
            if( ::StrChr(m_szInclude, *pstr)==NULL ) return FALSE;
#else
            if( _tcschr(m_szInclude, *pstr)==NULL ) return FALSE;
#endif
            pstr = ::CharNext(pstr);
         }
      }
      return TRUE;
   }

   // Message Handlers

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
      _Init();
      return lRes;
   }

   LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      TCHAR ch = (TCHAR)wParam;
      bool bCombi = (::GetKeyState(VK_CONTROL) & 0x8000) || (::GetKeyState(VK_MENU) & 0x8000);
      if( bCombi && (::GetKeyState(VK_CONTROL) & 0x8000) && (::GetKeyState(VK_MENU) & 0x8000) ) bCombi = false; // Alt+Gr key is ok
      if( !bCombi && ch>=VK_SPACE ) {
         if( m_dwExtStyle & ES_EX_EXCLUDEMASK ) {
#if (WINVER >= 0x0500)
            if( ::StrChr(m_szExclude, ch)!=NULL ) {
#else
            if( _tcschr(m_szExclude, ch)!=NULL ) {
#endif
               //::MessageBeep((UINT)-1);
               return 0;
            }
         }
         if( m_dwExtStyle & ES_EX_INCLUDEMASK ) {
#if (WINVER >= 0x0500)
            if( ::StrChr(m_szInclude, ch)==NULL ) {
#else
            if( _tcschr(m_szInclude, ch)==NULL ) {
#endif
               //::MessageBeep((UINT)-1);
               return 0;
            }
         }
      }

      LRESULT lResult = DefWindowProc(uMsg, wParam, lParam);
      ::PostMessage(GetParent(), WM_CHANGE, 0, 0);
      if( m_dwExtStyle & ES_EX_JUMPY ) _NextDlgItem();

      return lResult;
   }

   LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {
      if( wParam==VK_RIGHT && (m_dwExtStyle & ES_EX_JUMPY) ) {
         if( _NextDlgItem() ) return 0;
      }
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnSetText(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
   {
      LPCTSTR pstr = (LPCTSTR)lParam;
      if( _CheckText(pstr)==FALSE ) {
         //::MessageBeep((UINT)-1);
         return 0;
      }
      bHandled = false;
      return 0;
   }

   LRESULT OnPaste(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      // Instead of trying to check the data on the clipboard (which
      // could be in any strange clipboard format) we let the paste succeed,
      // check if the new text is valid and revert to the original text if not.

      int len = GetWindowTextLength()+1;
      LPTSTR pstrOld = (LPTSTR)_alloca(len*sizeof(TCHAR));
      if( pstrOld==NULL ) return 0;
      GetWindowText(pstrOld, len);
      
      LRESULT lResult = DefWindowProc(uMsg, wParam, lParam);

      len = GetWindowTextLength()+1;
      LPTSTR pstrNew = (LPTSTR)_alloca(len*sizeof(TCHAR));
      if( pstrNew==NULL ) return 0;
      GetWindowText(pstrNew, len);
      if( _CheckText(pstrNew)==FALSE ) {
         SetWindowText(pstrOld);
         SetSel(0,0);
         //::MessageBeep((UINT)-1);
      }
      
      return lResult;
   }
};


#endif // !defined(AFX_FILTERINGEDIT_H__20010604_1C88_3217_03F4_0080AD509054__INCLUDED_)
