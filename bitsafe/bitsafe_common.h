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

#pragma once

#include <time.h>
#include <SHELLAPI.H>

#ifdef _DEBUG_MEM_LEAKS
#ifdef new
#undef new
#endif //new 
#endif //_DEBUG_MEM_LEAKS

#include <string>
#include <vector>

#ifdef _DEBUG_MEM_LEAKS
#ifdef DBG_NORMAL_BLOCK
#define new DBG_NORMAL_BLOCK
#endif //DBG_NORMAL_BLOCK
#endif //_DEBUG_MEM_LEAKS

#include "sevenfw_err_code.h"
#include "ui_config.h"
#include "mem_region_list.h"

using namespace std; 

#ifdef _UNICODE
typedef std::wstring record_string; 
#else
typedef std::string record_string; 
#endif //_UNICODE

#define _MAX_URL_LEN 1024 

#ifndef NO_UILIB_SUPP
INLINE LRESULT get_themes_path( CStdString &theme_path )
{
	LRESULT ret = ERROR_SUCCESS; 

	theme_path = CPaintManagerUI::GetInstancePath(); 

	if( TRUE == theme_path.IsEmpty() )
	{
		ret = ERROR_NOT_READY; 
	}

	theme_path.Append( _T( "themes" ) ); 

	return ret; 
}
#endif //NO_UILIB_SUPP

LRESULT fatal_error( LRESULT error_code ); 

LRESULT WINAPI open_file_dlg( BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
					LPCTSTR lpszDefExt = NULL,
					LPCTSTR lpszFileName = NULL,
					LPCTSTR lpszFileTitle = NULL,
					LPCTSTR lpszInitialDir = NULL, 
					LPCTSTR lpszFilter = NULL, 
					HWND hWndParent = NULL, 
					DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT ); 

#define PATH_DELIM_CH L'\\'
LRESULT WINAPI get_app_path( LPWSTR FileName, 
							ULONG ccBufferLength, 
							ULONG *ccReturnLength ); 

INLINE LRESULT convert_bool_to_ret( BOOL ret_val )
{
	INT32 ret; 
	if( ret_val == FALSE )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
	}
	else
	{
		ret = 0; 
	}

	return ret; 
}
