
#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#pragma once

#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN	
#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <objbase.h>
#include <zmouse.h>
//#ifdef WIN32_NO_STATUS
//#undef WIN32_NO_STATUS
//#include <ntstatus.h>
//#endif //WIN32_NO_STATUS
#include "common_func.h"
#include "ui_ctrl.h"

#include "..\DuiLib\UIlib.h"

using namespace DuiLib;

#ifdef _DEBUG
#   ifdef _UNICODE
#       pragma comment(lib, "..\\bin\\DuiLib_ud.lib")
#   else
#       pragma comment(lib, "..\\bin\\DuiLib_d.lib")
#   endif
#else
#   ifdef _UNICODE
#       pragma comment(lib, "..\\bin\\DuiLib_u.lib")
#   else
#       pragma comment(lib, "..\\bin\\DuiLib.lib")
#   endif
#endif

#ifdef _DEBUG
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif


#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "comctl32.lib" )

#if _MSC_VER <= 1310
#define _stprintf_s _sntprintf
#define _vstprintf_s _vsntprintf
#endif

#include "local_strings.h"
LRESULT _set_ctrl_text( CControlUI *ctrl, ALL_LANG_STRINGS_ID id, LPCWSTR def_text = NULL ); 
LRESULT set_ctrl_text( CPaintManagerUI *pm, LPCTSTR ctrl_name, ALL_LANG_STRINGS_ID id, LPCWSTR def_text = NULL ); 

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
