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
