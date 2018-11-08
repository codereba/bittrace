/*
 * Copyright 2010 JiJie Shi
 *
 * This file is part of NetMonitor.
 *
 * NetMonitor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NetMonitor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NetMonitor.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  2007.6.10	Ji Jie Shi modified it for this ui lib. 
 */

#include "common_func.h"
#include <tchar.h>
#include "inst_im.h"

//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

INT32 GetAppDir( TCHAR *Path, DWORD PathLen )
{
	TCHAR AppPath[ MAX_PATH ];
	TCHAR *path_delim;

	if( 0 == GetModuleFileName( NULL, AppPath, MAX_PATH ) )
	{
		goto _ERROR;
	}

	path_delim = _tcsrchr( AppPath, '\\' );

	if( NULL == path_delim )
	{
		goto _ERROR;
	}

	*path_delim = _T( '\0' );

	if( _tcslen( AppPath ) + 1 > PathLen )
	{
		goto _ERROR;
	}

	_tcscpy( Path, AppPath );
	return 0;

_ERROR:
	*Path = _T( '\0' );
	return -1;
}

HRESULT CopySysFile( TCHAR* FileName )
{
	HRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	TCHAR SysFilePath[ MAX_PATH ];
	TCHAR AppFilePath[ MAX_PATH ];

	GetSystemDirectory( SysFilePath, MAX_PATH );
	GetAppDir( AppFilePath, MAX_PATH );

	_tcsncat( SysFilePath, _T( "\\drivers\\" ), MAX_PATH - _tcslen( SysFilePath ) );
	_tcsncat( AppFilePath, _T( "\\" ), MAX_PATH - _tcslen( AppFilePath ) );
	
	_tcsncat( SysFilePath, FileName, MAX_PATH - _tcslen( SysFilePath ) );
	_tcsncat( AppFilePath, FileName, MAX_PATH - _tcslen( AppFilePath ) );

	_ret = CopyFile( AppFilePath, SysFilePath, FALSE );
	if( _ret == FALSE )	
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "copy the file to system dir failed \n" ) ); 
		goto _return; 
	}

_return:
	return ret;
}

HRESULT DelSysFile( TCHAR* FileName )
{
	HRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	TCHAR SysFilePath[ MAX_PATH ];
	TCHAR AppFilePath[ MAX_PATH ];

	GetSystemDirectory( SysFilePath, MAX_PATH );
	GetAppDir( AppFilePath, MAX_PATH );

	_tcsncat( SysFilePath, _T( "\\drivers\\" ), MAX_PATH - _tcslen( SysFilePath ) );
	_tcsncat( AppFilePath, _T( "\\" ), MAX_PATH - _tcslen( AppFilePath ) );
	
	_tcsncat( SysFilePath, FileName, MAX_PATH - _tcslen( SysFilePath ) );
	_tcsncat( AppFilePath, FileName, MAX_PATH - _tcslen( AppFilePath ) );

	_ret = DeleteFile( SysFilePath );
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "delete the file in system dir failed \n" ) ); 
		goto _return; 
	}

_return:
	return ret; 
}
