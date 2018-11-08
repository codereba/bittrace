
#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#pragma once

#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN	
//#define _CRT_SECURE_NO_DEPRECATE

#include "common_func.h"
#include <objbase.h>
#include <zmouse.h>

#ifndef WIN32_NO_STATUS 
#define WIN32_NO_STATUS 1
#include "..\DuiLib\UIlib.h"
#endif //WIN32_NO_STATUS 

using namespace DuiLib;

#ifdef _DEBUG
#   ifdef _UNICODE
#   ifdef WIN64
#       pragma comment(lib, "..\\bin\\DuiLib_ud64.lib")
#   else
#       pragma comment(lib, "..\\bin\\DuiLib_ud.lib")
#   endif //WIN64
#   else
#   ifdef WIN64
#       pragma comment(lib, "..\\bin\\DuiLib_d64.lib")
#   else
#       pragma comment(lib, "..\\bin\\DuiLib_d.lib")
#   endif //WIN64
#   endif
#else
#   ifdef _UNICODE
#   ifdef WIN64
#       pragma comment(lib, "..\\bin\\DuiLib_u64.lib")
#   else
#       pragma comment(lib, "..\\bin\\DuiLib_u.lib")
#   endif //WIN64
#   else
#   ifdef WIN64
#       pragma comment(lib, "..\\bin\\DuiLib64.lib")
#   else
#       pragma comment(lib, "..\\bin\\DuiLib.lib")
#   endif //WIN64
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

__inline LRESULT set_ctrl_text_and_tooltip_from_name( CPaintManagerUI *pm, LPCWSTR ctrl_name, LPCWSTR text )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 

	ASSERT( NULL != ctrl_name ); 

	ctrl = ( CControlUI* )pm->FindControl( ctrl_name ); 
	ASSERT( ctrl != NULL ); 

	log_trace( ( MSG_INFO, "add english text %ws to control 0x%0.8x\n", 
		text, 
		ctrl ) ); 

	ctrl->SetToolTip( text ); 
	ctrl->SetName( text ); 

	return ret; 
}

__inline LRESULT set_ctrl_text_and_tooltip_from_name_ex( CPaintManagerUI *pm, 
														LPCWSTR ctrl_name, 
														LPCWSTR text, 
														LPCWSTR tooltip )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 

	ASSERT( NULL != ctrl_name ); 

	ctrl = ( CControlUI* )pm->FindControl( ctrl_name ); 
	ASSERT( ctrl != NULL ); 

	log_trace( ( MSG_INFO, "add english text %ws to control 0x%0.8x\n", 
		text, 
		ctrl ) ); 

	ctrl->SetToolTip( tooltip ); 
	ctrl->SetText( text ); 

	return ret; 
}

__inline LRESULT set_ctrl_text_from_name( CPaintManagerUI *pm, LPCWSTR ctrl_name, LPCWSTR text )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 

	ASSERT( NULL != ctrl_name ); 

	ctrl = ( CControlUI* )pm->FindControl( ctrl_name ); 
	ASSERT( ctrl != NULL ); 

	log_trace( ( MSG_INFO, "add english text %ws to control 0x%0.8x\n", 
		text, 
		ctrl ) ); 

	ctrl->SetText( text ); 

	return ret; 
}

__inline LRESULT set_ctrl_tooltip_from_name( CPaintManagerUI *pm, LPCWSTR ctrl_name, LPCWSTR text )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 

	ASSERT( NULL != ctrl_name ); 

	ctrl = ( CControlUI* )pm->FindControl( ctrl_name ); 
	ASSERT( ctrl != NULL ); 

	log_trace( ( MSG_INFO, "add english text %ws to control 0x%0.8x\n", 
		text, 
		ctrl ) ); 

	ctrl->SetToolTip( text ); 

	return ret; 
}

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
