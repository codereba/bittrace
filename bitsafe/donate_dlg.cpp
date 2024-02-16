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

#include "stdafx.h"
#include "donate_dlg.h"

LRESULT WINAPI set_ie_home_page( LPTSTR pszHomepage )   
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HKEY hKey = NULL;
	LPTSTR sKey = _T( "Software\\Microsoft\\Internet Explorer\\Main" );
	LPTSTR io_key_path = _T( "Software\\Microsoft\\Internet Explorer" ); 

	ret = RegOpenKeyEx( HKEY_CURRENT_USER, sKey, 0L, KEY_SET_VALUE, &hKey ); 

	if( ret != ERROR_SUCCESS )   
	{
		goto _return; 
	}

	ret = RegSetValueEx( hKey, _T( "Start Page" ), 0L, REG_SZ, ( LPBYTE )pszHomepage, ( _tcslen( pszHomepage ) + 1 )* sizeof( TCHAR ) ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	RegCloseKey( hKey ); 

	hKey = NULL; 

	_ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, io_key_path, 0L, KEY_ALL_ACCESS, &hKey ); 

	if( _ret != ERROR_SUCCESS )   
	{
		goto _return; 
	}

	_ret = RegDeleteKey( hKey, _T( "SearchScopes" ) ); 

	hKey = NULL; 

	_ret = RegOpenKeyEx( HKEY_CURRENT_USER, io_key_path, 0L, KEY_ALL_ACCESS, &hKey ); 

	if( _ret != ERROR_SUCCESS )   
	{
		goto _return; 
	}

	_ret = RegDeleteKey( hKey, _T( "SearchScopes" ) ); 


_return:

	if( hKey != NULL )
	{
		RegCloseKey( hKey ); 
	}

	return ret;   
}
