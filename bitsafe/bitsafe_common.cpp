/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
 *
 * This file is part of bittrace.
 *
 * bittrace is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bittrace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bittrace.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "_StdAfx.h"
#include "bitsafe_common.h"

inline bool IsOldWindows()
{
	OSVERSIONINFO ovi = { 0 };
	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	BOOL bRet = ::GetVersionEx(&ovi);
	return (!bRet || !((ovi.dwMajorVersion >= 5) || (ovi.dwMajorVersion == 4 && ovi.dwMinorVersion >= 90)));
}

LRESULT fatal_error( LRESULT error_code )
{

	ExitProcess( error_code ); 

	return ERROR_SUCCESS; 
}

LRESULT WINAPI get_app_path( LPWSTR FileName, 
						  ULONG ccBufferLength, 
						  ULONG *ccReturnLength )
{
	INT32 Ret = ERROR_SUCCESS; 
	ULONG _Ret; 
	LPWSTR PathDelim; 

	if( FileName == NULL )
	{
		Ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( ccBufferLength == 0 )
	{
		Ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( ccReturnLength == NULL )
	{
		Ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	*ccReturnLength = 0; 

	_Ret = GetModuleFileName( NULL, FileName, ccBufferLength ); 
	if( _Ret >= ccBufferLength )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}
	else if( _Ret == 0 )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	PathDelim = wcsrchr( FileName, PATH_DELIM_CH ); 

	if( PathDelim == NULL )
	{
		Ret = ERROR_NOT_FOUND; 
		goto _return; 
	}

	if( ( ULONG )( PathDelim - FileName ) + 2 > ccBufferLength )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	PathDelim[ 1 ] = L'\0'; 

	*ccReturnLength = PathDelim - FileName + 1; 

_return: 
	return Ret; 
}

LRESULT WINAPI open_file_dlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
					LPCTSTR lpszDefExt, 
					LPCTSTR lpszFileName,
					LPCTSTR lpszFileTitle, 
					LPCTSTR lpszInitialDir, 
					LPCTSTR lpszFilter,
					HWND hWndParent, 
					DWORD flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	HWND hTmpWnd; 
	OPENFILENAME m_ofn;
	memset(&m_ofn, 0, sizeof(m_ofn)); // initialize structure to 0/NULL
	m_ofn.lStructSize = sizeof(m_ofn);
#if (_WIN32_WINNT >= 0x0500)
	// adjust struct size if running on older version of Windows
	if(IsOldWindows())
	{
		ASSERT(sizeof(m_ofn) > OPENFILENAME_SIZE_VERSION_400);   // must be
		m_ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	}
#endif //(_WIN32_WINNT >= 0x0500)

	m_ofn.lpstrFile = (LPTSTR)lpszFileName;
	m_ofn.nMaxFile = _MAX_PATH;
	m_ofn.lpstrDefExt = lpszDefExt;
	m_ofn.lpstrFileTitle = (LPTSTR)lpszFileTitle;
	m_ofn.nMaxFileTitle = _MAX_FNAME; 
	m_ofn.lpstrInitialDir = lpszInitialDir; 

#ifndef _WIN32_WCE
	m_ofn.Flags = flags | OFN_EXPLORER | OFN_ENABLESIZING;
#else // CE specific
	m_ofn.Flags = flags | OFN_EXPLORER;
#endif //!_WIN32_WCE
	m_ofn.lpstrFilter = lpszFilter;

	m_ofn.hInstance = NULL;

	hTmpWnd = ::GetActiveWindow();
	m_ofn.hwndOwner = hWndParent ? hWndParent : hTmpWnd;

	if(bOpenFileDialog)
	{
		_ret = ::GetOpenFileName(&m_ofn);
	}
	else
	{
		_ret = ::GetSaveFileName(&m_ofn);
	}

	if( _ret == FALSE )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
	}

	return ret;
}

HICON extract_icon( LPCTSTR file_name )
{
	return ExtractIcon( NULL, file_name, -1 ); 
}


LRESULT _set_ctrl_text( CControlUI *ctrl, ALL_LANG_STRINGS_ID id, LPCWSTR def_text )
{
	LPCTSTR tmp_text; 
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( ctrl != NULL ); 

	tmp_text = _get_string_by_id( id, def_text ); 
	if( tmp_text != NULL )
	{
		log_trace( ( MSG_INFO, "add english text %ws to control 0x%0.8x\n", 
			tmp_text, 
			ctrl ) ); 
		ctrl->SetText( tmp_text ); 
	}
	else
	{
		//ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT set_ctrl_text( CPaintManagerUI *pm, LPCTSTR ctrl_name, ALL_LANG_STRINGS_ID id, LPCWSTR def_text )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 

	ASSERT( ctrl_name != NULL ); 
	ASSERT( pm != NULL ); 

	ctrl = pm->FindControl( ctrl_name ); 
	if( ctrl == NULL )
	{
		ret = ERROR_INVALID_PARAMETER; 
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = _set_ctrl_text( ctrl, id, def_text ); 

_return:
	return ret; 
}
