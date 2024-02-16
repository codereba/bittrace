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

#include "common_func.h"
#include <tchar.h>
#include "inst_im.h"

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
