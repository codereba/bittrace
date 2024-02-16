/*
 * Copyright 2010-2024 JiJie.Shi.
 *
 * This file is part of bittrace.
 * Licensed under the Gangoo License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
