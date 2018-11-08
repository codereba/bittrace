/*
 *
 * Copyright 2010 JiJie Shi
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
 
 #include "common_func.h"
#include "crush_report.h"
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

INLINE LONG WINAPI mini_dump_handler( struct _EXCEPTION_POINTERS *lpExceptionInfo )
{
	LONG ret = EXCEPTION_EXECUTE_HANDLER; 
	BOOL _ret; 
	HANDLE hFile; 
	CHAR szFileName[64];
	SYSTEMTIME st;
	MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	log_trace( ( MSG_INFO, "exception information is 0x%0.8x\n", lpExceptionInfo ) ); 

	GetLocalTime( &st );
	wsprintfA( szFileName, "%04d-%02d-%02d-%02d-%02d-%02d-%02d-%02d.dmp", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, rand() %100 );

	hFile = ::CreateFileA( szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if( hFile == INVALID_HANDLE_VALUE )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "create target file for mini dump failed\n" ) ); 
		goto _return; 
	}

	ExInfo.ThreadId = ::GetCurrentThreadId();
	ExInfo.ExceptionPointers = lpExceptionInfo;
	ExInfo.ClientPointers = FALSE;

	// write the dump

	_ret = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );

	if( _ret )
	{
		log_trace( ( MSG_INFO, "create dump file successfully\n" ) );
	}
	else
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "create mini dump failed\n" ) ); 
	}

_return:
	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle(hFile);
	}

	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 
	return ret;
}

LPTOP_LEVEL_EXCEPTION_FILTER prev_flt_func = NULL; 
LRESULT init_crush_report()
{
	prev_flt_func = SetUnhandledExceptionFilter( mini_dump_handler );
	return ERROR_SUCCESS; 
}

LRESULT uninit_crush_report()
{
	if( prev_flt_func != NULL )
	{
		SetUnhandledExceptionFilter( prev_flt_func ); 
	}

	return ERROR_SUCCESS; 
}