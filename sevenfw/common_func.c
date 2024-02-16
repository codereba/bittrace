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

#pragma comment( lib, "psapi.lib" )
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Winmm.lib")

#define MAX_ERROR_MESSAGE_LEN 1024

LRESULT safe_create_dir( LPCTSTR tmp_dir_path, LPCTSTR dir_path )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 
	log_trace( ( MSG_INFO, "destination directory is %s\n", dir_path ) ); 

	ASSERT( dir_path != NULL ); 
	
	if( tmp_dir_path != NULL )
	{
		_ret = CreateDirectoryEx( tmp_dir_path, dir_path, NULL ); 
		if( FALSE == _ret )
		{
			ret = GetLastError(); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "create directory using the extended version function error\n" ) ); 
			goto _return; 
		}
	}
	else
	{
		_ret = CreateDirectory( dir_path, NULL ); 
		if( FALSE == _ret )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "create directory error\n" ) ); 
			ret = GetLastError(); 
			goto _return; 
		}
	}

_return: 
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

INT32 error_handle( LPCSTR msg, DWORD mode, DWORD mb_flags )
{
	INT32 ret;
	DWORD err_code;
	DWORD msg_len; 
	CHAR sys_msg[ MAX_ERROR_MESSAGE_LEN ];
	CHAR output[ MAX_ERROR_MESSAGE_LEN ];

	ret = 0;
	err_code = GetLastError();

	msg_len = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
		NULL, err_code,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // Default language
		sys_msg, MAX_ERROR_MESSAGE_LEN, NULL );

	_snprintf( output, MAX_ERROR_MESSAGE_LEN, "\n  WARNING: %s \n failed with error %d (%s)", msg, err_code, sys_msg );
	if( mode & MODE_DEBUG_OUTPUT )
	{
		OutputDebugStringA( output );
	}

	if( mode & MODE_MESSAGE_BOX )
	{
		ret = MessageBoxA( NULL, output, "error", MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | mb_flags );
	}

	return ret;
}

INLINE LRESULT get_cur_err_msg( LPSTR msg, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD err_code;
	ULONG msg_len; 

	err_code = GetLastError();

	msg_len = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err_code,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // Default language
		msg, MAX_ERROR_MESSAGE_LEN, NULL );

	if( msg_len == 0 )
	{
		ret = GetLastError(); 
		log_trace( ( MSG_INFO, "get last error code message failed, error code %d \n", ret ) ); 		
		goto _return; 
	}

_return:
	return ret; 
}

#ifdef _USE_WINSOCK

#pragma comment( lib, "ws2_32.lib" )
LRESULT init_socket_env()
{
	LRESULT ret = ERROR_SUCCESS; 

	WORD Version = 0x0202; 
	WSADATA WsaData;
	ret = WSAStartup( Version, &WsaData );

	if( ret != 0 )
	{
		log_trace( ( MSG_ERROR, "fatal socket error, can't load windows socket library\n" ) );  
	}

	return ret; 
}

VOID uninit_socket_env()
{
	WSACleanup(); 
}

INLINE LRESULT get_socket_err_msg( LPSTR msg, ULONG buf_len, PULONG ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG err_code; 

	err_code = WSAGetLastError(); 

	_snprintf( msg, buf_len, "socket error: %0x0.8x\n", err_code ); 

	return ret; 
}

#endif //_USE_WINSOCK

#include "wininet.h"
#pragma comment( lib, "wininet.lib" )

INLINE LRESULT get_cur_internet_err_msg( LPSTR msg, ULONG buf_len, PULONG ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD err_code;
	ULONG msg_len; 
	BOOL _ret; 

	err_code = GetLastError(); 

	ASSERT( msg != NULL ); 
	ASSERT( buf_len > 0 ); 

	if( ret_len != NULL )
	{
		*ret_len = 0; 
	}

	*msg = '\0'; 

	if( err_code == ERROR_SUCCESS )
	{
		goto _return; 
	}

	msg_len = buf_len; 

#if 0
	sprintf( msg, "internet error message buffer is 0x%0.8x, buffer len is %d, error code is %d\n", msg, msg_len, err_code ); 

	MessageBoxA( NULL, msg, NULL, NULL ); 
#endif //0 

	_ret = InternetGetLastResponseInfoA( &err_code, msg, &msg_len );
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "get last internet error code message failed\n", ret ) ); 		
	}

	log_trace( ( MSG_INFO, "internet error code is 0x%0.8x\n", err_code ) ); 

	//ASSERT( msg_len != 0 ); 
	if( ret_len != NULL )
	{
		*ret_len = msg_len; 
	}

_return:
	return ret; 
}


#define TITLE_INFO "info"
#define TITLE_WARNING "warning"
#define TITLE_ERROR "error"

INLINE LRESULT get_msg_box_feature( ULONG level, LPCSTR *title, ULONG *flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCSTR _title = TITLE_INFO ; 
	ULONG _flags = MB_ICONINFORMATION; 

	ASSERT( title != NULL ); 
	ASSERT( flags != NULL ); 

	if( MSG_INFO == level )
	{
	}
	else if( MSG_WARNING == level )
	{
		_title = TITLE_WARNING; 
		_flags = MB_ICONEXCLAMATION; 
	}
	else if( MSG_ERROR <= level )
	{
		_title = TITLE_ERROR; 
		_flags = MB_ICONERROR; 
	}

	*title = _title; 
	*flags = _flags;

//_return:
	return ret; 
}

#ifdef LOG_THREAD_SAFE
CRITICAL_SECTION cr_log; 
VOID init_log_context()
{
	init_cs_lock( cr_log ); 
}

VOID uninit_log_context()
{
	uninit_cs_lock( cr_log ); 
}

VOID logger_lock()
{
	lock_cs( cr_log ); 
}

VOID logger_unlock()
{
	unlock_cs( cr_log ); 
}

#endif //LOG_THREAD_SAFE

INT32 s_bLogPathInit = FALSE; 
CHAR	g_bLogSavePath[MAX_PATH] = "C:\\log";
static	CHAR			s_szLogFile[MAX_PATH] = {0};

#ifdef DEBUG_WIN7
ULONG dbg_print_lvl = DBG_LVL_NONE_PRINT; 
#else
ULONG dbg_print_lvl = 0; 
#endif //DEBUG_WIN7

VOID set_dbg_log_level( ULONG lvl )
{
	dbg_print_lvl = lvl; 
}

ULONG get_dbg_log_level()
{
	return dbg_print_lvl; 
}

LRESULT dbg_print(
    ULONG lvl_and_mode,
    LPCSTR dbg_msg,
	... )
{
#define MAX_DBG_OUT_LN_LEN 1024
	
	LRESULT ret = ERROR_SUCCESS; 
    va_list ap; 
	ULONG level; 
	ULONG mode; 
	CHAR output[ MAX_DBG_OUT_LN_LEN ]; 

	mode = lvl_and_mode & DBG_MODE_MASK; 
    level = lvl_and_mode & DBG_LVL_MASK; 

	if( mode == 0 )
	{
		mode = DBG_DBG_BUF_OUT; 
	}

	va_start( ap, dbg_msg );

	if( ( level <= DBG_LVL_VALUE_PART_MASK && level >= ( dbg_print_lvl & DBG_LVL_VALUE_PART_MASK ) ) 
		|| ( ( level > DBG_LVL_VALUE_PART_MASK ) && ( ( level & ( ~DBG_LVL_VALUE_PART_MASK ) ) & dbg_print_lvl ) ) )
	{
#ifdef _UNICODE_
		{
#define MAX_FMT_STR_LEN 512
			ULONG uni_len; 
			WCHAR unicode[ MAX_FMT_STR_LEN ]; 
			ret = mcbs_to_unicode( dbg_msg, unicode, MAX_FMT_STR_LEN, &uni_len ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			_vsntprintf( output, MAX_DBG_OUT_LN_LEN, unicode, ap ); 
		}
#else			
		_vsnprintf( output, MAX_DBG_OUT_LN_LEN, dbg_msg, ap);
#endif
		if( mode & DBG_LOG_OUT )
		{
			FILE* pFile; 

#if defined( LOG_THREAD_SAFE )
			logger_lock(); 
#endif
			if (!s_bLogPathInit)
			{
				SYSTEMTIME stime = {0};			
				GetLocalTime(&stime);
#if defined(UNDER_CE)
				sprintf(s_szLogFile, "%s\\log_%04d%02d%02d_%d.log", g_bLogSavePath, stime.wYear, stime.wMonth, stime.wDay, GetTickCount());
#else
				_snprintf(s_szLogFile, MAX_PATH, "%s\\log_%04d%02d%02d_%d.log", g_bLogSavePath, stime.wYear, stime.wMonth, stime.wDay, GetTickCount());
#endif
				s_bLogPathInit = TRUE;
			}

			pFile = fopen(s_szLogFile, "a");
			if (pFile != NULL)
			{
				long cbSize; 
				fseek(pFile,SEEK_END,0);
				
				cbSize = ftell(pFile);
				if (cbSize > MAX_LOG_FILE_SIZE)
				{
					fclose(pFile);
					{
						SYSTEMTIME stime = {0};			
						GetLocalTime(&stime);
#if defined(UNDER_CE)
						sprintf(s_szLogFile, "%s\\log_%04d%02d%02d_%d.log", g_bLogSavePath, stime.wYear, stime.wMonth, stime.wDay, GetTickCount());
#else
						_snprintf(s_szLogFile, MAX_PATH, "%s\\log_%04d%02d%02d_%d.log", g_bLogSavePath, stime.wYear, stime.wMonth, stime.wDay, GetTickCount());
#endif
						s_bLogPathInit = TRUE;
					}
					pFile = fopen(s_szLogFile, "a");
					if (pFile == NULL)
					{
#if defined( LOG_THREAD_SAFE )
						logger_unlock(); 
#endif
						goto _return; 
					}
				}
				
				vfprintf(pFile, output, ap);
				fflush(pFile);
				fclose(pFile);
				pFile = NULL;
			}

#if defined( LOG_THREAD_SAFE )
			logger_unlock(); 
#endif
		}

		if( mode & DBG_DBG_BUF_OUT )
		{
			OutputDebugStringA( output ); 
		}

		if( mode & DBG_MSG_BOX_OUT )
		{
			LPCSTR title; 
			ULONG flags; 
			get_msg_box_feature( level, &title, &flags ); 

			ret = MessageBoxA( NULL, output, title, MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | flags );
		}
		
		if( mode & DBG_ERROR_MSG )
		{
			if( mode & DBG_INTERNET_ERR )
			{
				ret = get_cur_internet_err_msg( output, MAX_DBG_OUT_LN_LEN, NULL ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				if( *output == '\0' )
				{
					goto _return; 
				}
			}
			else if( mode & DBG_SOCKET_ERR )
			{
#if 0
				ret = get_socket_err_msg( output, MAX_DBG_OUT_LN_LEN, NULL ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}
#endif 
				ret = get_cur_err_msg( output, MAX_DBG_OUT_LN_LEN ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				if( *output == '\0' )
				{
					goto _return; 
				}
			}
			else
			{
				ret = get_cur_err_msg( output, MAX_DBG_OUT_LN_LEN ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}
			}

			if( mode & DBG_DBG_BUF_OUT )
			{
				OutputDebugStringA( output ); 
			}
		}
	}

_return: 
	va_end(ap);
	return ret; 
}

BOOL IsWIN32_NT()
{
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	GetVersionEx(&osv);
	return osv.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

BOOL windows_shutdown(UINT uFlags)
{
	if(IsWIN32_NT())
	{
		HANDLE hProcess, hToken;
		TOKEN_PRIVILEGES Privileges;
		hProcess = GetCurrentProcess();
		OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken);
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &Privileges.Privileges[0].Luid);
		Privileges.PrivilegeCount = 1;
		Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, FALSE, &Privileges, 0, (PTOKEN_PRIVILEGES)NULL, NULL);
	}
	return ExitWindowsEx(uFlags, 0);
}

BOOL windows_hibernate()
{
	static HANDLE hToken;
	static TOKEN_PRIVILEGES tp;
	static LUID luid;
	if( OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
		&hToken))
	{
		LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&luid);
		tp.PrivilegeCount=1;
		tp.Privileges[0].Luid =luid;
		tp.Privileges[0].Attributes =SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),NULL,NULL);
	}

	return SetSystemPowerState(FALSE,TRUE); 
}

LRESULT add_sys_drv_path_to_file_name( LPTSTR file_name_out, ULONG buf_len, LPCTSTR file_name, INT32 chk_exist )
{
	LRESULT ret = ERROR_SUCCESS; 
    DWORD driverLocLen = 0;

	driverLocLen = GetSystemDirectory( file_name_out, buf_len );

    if( driverLocLen == 0 ) 
	{
		ret = GetLastError(); 
		goto _return; 
	}

	_tcsncat( file_name_out, _T( "\\drivers\\" ), buf_len - _tcslen( file_name_out ) );
	_tcsncat( file_name_out, file_name, buf_len - _tcslen( file_name_out ) );

	if( chk_exist == TRUE )
	{
		ret = check_file_exist( file_name_out ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
    return ret;
}  

LRESULT construct_file_path_a( LPSTR file_name_out, ULONG size, LPCSTR dir_path, LPCSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS;
	//INT32 _ret; 

	strncpy( file_name_out, dir_path, size ); 

	if( file_name_out[ strlen( file_name_out ) - 1 ] != '\\' )
	{
		if( strlen( file_name_out ) + 2 > size )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		file_name_out[ strlen( file_name_out ) ] = '\\'; 
	}

	strncat( file_name_out, file_name, size - strlen( file_name_out ) ); 

_return:
	return ret; 
}

LRESULT construct_file_path_w( LPWSTR file_name_out, ULONG size, LPCWSTR dir_path, LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS;
	//INT32 _ret; 

	wcsncpy( file_name_out, dir_path, size ); 

	if( file_name_out[ wcslen( file_name_out ) - 1 ] != '\\' )
	{
		if( wcslen( file_name_out ) + 2 > size )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		file_name_out[ wcslen( file_name_out ) ] = '\\'; 
	}

	wcsncat( file_name_out, file_name, size - wcslen( file_name_out ) ); 

_return:
	return ret; 
}

LRESULT add_app_path_to_file_name( LPTSTR file_name_out, ULONG ccb_buf_len, LPCTSTR file_name, INT32 chk_exist )
{
	LRESULT ret = ERROR_SUCCESS; 
    DWORD driverLocLen = 0;
	LPTSTR path_delim;

	driverLocLen = GetModuleFileName( NULL, 
		file_name_out, 
		ccb_buf_len 
		);

    if( driverLocLen == 0 ) 
	{
        ret = GetLastError(); 
		goto _return; 
    }

	file_name_out[ ccb_buf_len - 1 ] = L'\0'; 

	path_delim = _tcsrchr( file_name_out, '\\' );
	ASSERT( path_delim != NULL ); 

	//*( path_delim + 1 ) = _T( '\0' ); 
	_tcsncpy( path_delim + 1, file_name, ccb_buf_len - ( path_delim - file_name_out + 1 ) );

	if( file_name_out[ ccb_buf_len - 1 ] != L'\0' )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

    //
    // Insure driver file is in the specified directory.
	//

	if( chk_exist == TRUE )
	{
		ret = check_file_exist( file_name_out ); 
	}

_return:

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret;
}   // add_app_path_to_file_name

LRESULT add_app_sub_dir_path_to_file_name( LPTSTR file_name_out, ULONG buf_len, LPCTSTR sub_dir,  LPCTSTR file_name, INT32 chk_exist )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD driverLocLen = 0;
	LPTSTR path_delim;

	driverLocLen = GetModuleFileName( NULL, 
		file_name_out, 
		buf_len );

	if( driverLocLen == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	path_delim = _tcsrchr( file_name_out, '\\' );

	*( path_delim + 1 ) = _T( '\0' ); 

	if( sub_dir != NULL )
	{
		_tcsncpy( path_delim + 1, sub_dir, buf_len - ( path_delim - file_name_out + 1 ) );
	}

	_tcsncat( file_name_out, file_name, buf_len - _tcslen( file_name_out ) ); 

	if( chk_exist == TRUE )
	{
		ret = check_file_exist( file_name_out ); 
	}

_return:
	return ret;
} 

LRESULT delete_file_spec_dir( LPCTSTR file_name, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	TCHAR szOutFullPath[ MAX_PATH * 2 ];

	if( flags == FILE_APP_DIR )
	{
		ret = add_app_path_to_file_name( szOutFullPath, MAX_PATH, file_name, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}
	else if( flags == FILE_SYS_DIR)
	{
		ret = add_sys_drv_path_to_file_name( szOutFullPath, MAX_PATH, file_name, TRUE ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}
	else if( flags == FILE_IS_WHOLE_PATH )
	{
		_ret = DeleteFile( file_name );
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		goto _return; 
	}
	else
	{
		ASSERT( FALSE && "invalid delete file flags" ); 
	}

	// Try to delete the file on disk.

	_ret = DeleteFile( szOutFullPath );
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	return ret;
}

LRESULT file_exists( LPCTSTR file_name, INT32 is_sys_path )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR szOutFullPath[ MAX_PATH ];

	if( is_sys_path == FALSE )
	{
		ret = add_app_path_to_file_name( szOutFullPath, MAX_PATH, file_name, TRUE ); 
	}
	else
	{
		ret = add_sys_drv_path_to_file_name( szOutFullPath, MAX_PATH, file_name, TRUE ); 
	}
	
	return ret; 
}

INT32 get_proc_img_path( DWORD dwProcessId, LPWSTR FileName, DWORD dwFileNameLen, LPWSTR NativeFileName, DWORD dwNativeFileNameLen )
{
	INT32 ret;
	HANDLE hProcess;

	hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId );
	if( NULL == hProcess )
	{
		//error_handle( "OpenProcess Failed\n", 0 );
		ret = FALSE;
		goto RETURN_;
	}

	if( NULL != NativeFileName )
	{
		ret = ( INT32 )GetProcessImageFileNameW( hProcess, NativeFileName, dwNativeFileNameLen );
		if( 0 == ret )
		{
			//error_handle( "GetProcessImageFileNameW failed\n", 0 );
			ret = FALSE;
			goto RETURN_;
		}
	}

	//if( !NT_SUCCESS( ZwQueryInformationProcess( hProcess, 
	//	ProcessImageFileName, 
	//	&NativeFileName, 
	//	dwNativeFileNameLen, 
	//	&dwNativeFileNameLen ) ) )
	//{
	//	ret = FALSE;
	//	goto RETURN_;
	//}

	if( NULL != FileName )
	{
		//ret = QueryFullProcessImageName( hProcess, FileName, dwFileNameLen );
		ret = ( INT32 )GetModuleFileNameExW( hProcess, NULL, FileName, dwFileNameLen );
		if( 0 == ret )
		{
			//error_handle( "GetModuleFileNameExW failed\n", 0 );
			ret = FALSE;
			goto RETURN_;
		}
	}

RETURN_:
	if( NULL != hProcess )
	{
		CloseHandle( hProcess );
	}

	return ret;
}

LRESULT WINAPI adjust_proc_token()
{
	LRESULT ret = ERROR_SUCCESS;
	INT32 _ret; 
	HANDLE hToken = NULL; 
	HANDLE hProcess;
	LUID Luid;
	TOKEN_PRIVILEGES Privileges;

	do 
	{
		hProcess = GetCurrentProcess();

		_ret = OpenProcessToken( hProcess, 0x28, &hToken );

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = LookupPrivilegeValue( NULL, _T( "SeDebugPrivilege" ), &Luid );

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		Privileges.Privileges[0].Luid.LowPart = Luid.LowPart;
		Privileges.Privileges[0].Luid.HighPart = Luid.HighPart;
		Privileges.Privileges[0].Attributes = 2;
		Privileges.PrivilegeCount = 1;

		_ret = AdjustTokenPrivileges( hToken, 0, &Privileges, 0, NULL, NULL );

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE );

	if( hToken != NULL )
	{
		CloseHandle( hToken ); 
	}

	return ret; 
}

INT32 show_popup_menu( HWND hwnd, ULONG menud_id )
{
	HMENU menu; 
	HMENU sub_menu; 
	POINT pos; 

	ASSERT( hwnd != NULL ); 

	menu = LoadMenu( NULL, MAKEINTRESOURCE( menud_id ) ); 
	if( NULL == menu )
	{
		return -1; 
	}

	sub_menu = GetSubMenu( menu, 0 ); 

	SetMenuDefaultItem( sub_menu, 0, TRUE );
	GetCursorPos( &pos );
	//SetForegroundWindow( hMainDlg );
	TrackPopupMenu( sub_menu, 0, pos.x, pos.y, 0, hwnd, NULL );
	PostMessage( hwnd, WM_NULL, 0, 0 );
	DestroyMenu( menu ); 

	return 0; 
}

LRESULT mcbs_to_unicode( LPCSTR mcbs, LPWSTR unicode, ULONG cch_len, PULONG ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD need_len;

	ASSERT( mcbs != NULL );  

	need_len = MultiByteToWideChar( CP_OEMCP, 0, mcbs, -1, NULL, 0 ); 

	if( ret_len != NULL )
	{
		*ret_len = need_len; 
	}
	
	if( need_len > cch_len || unicode == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	need_len = MultiByteToWideChar( CP_OEMCP, 0, mcbs, -1, unicode, cch_len ); 

	if( need_len == 0 )
	{
		ret = ( LRESULT )GetLastError(); 
		dbg_print( MSG_WARNING | DBG_DBG_BUF_OUT, "MultiByteToWideChar failed error code %d \n", GetLastError() ); 
		goto _return; 
	}

	if( ret_len != NULL )
	{
		*ret_len = need_len; 
	}

#ifdef _DEBUG
	if( need_len == cch_len )
	{
		if( unicode[ need_len - 1 ] != L'\0' )
		{
			DBG_BP(); 
			unicode[ need_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_TOO_SMALL; 
		}
	}
#endif //_DEBUG

_return:
	return ret; 
}

LRESULT unicode_to_mcbs( LPCWSTR unicode, LPSTR mcbs, ULONG cch_len, PULONG ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD need_len;
	
	ASSERT( unicode != NULL ); 

	need_len = WideCharToMultiByte( CP_OEMCP, 0, unicode, -1, NULL, 0, NULL, FALSE ); 
	
	if( ret_len != NULL )
	{
		*ret_len = need_len; 
	}

	if( need_len > cch_len || mcbs == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	need_len = WideCharToMultiByte( CP_OEMCP, 0, unicode, -1, mcbs, cch_len, NULL, FALSE ); 

	if( need_len == 0 )
	{
		ret = ( LRESULT )GetLastError(); 
		dbg_print( MSG_WARNING | DBG_DBG_BUF_OUT, "WideCharToMultiByte failed error code %d \n", GetLastError() ); 
		goto _return; 
	}

	if( ret_len != NULL )
	{
		*ret_len = need_len; 
	}

#ifdef _DEBUG
	if( need_len == cch_len )
	{
		if( mcbs[ need_len - 1 ] != '\0' )
		{
			DBG_BP(); 
			mcbs[ need_len - 1 ] = '\0'; 
			ret = ERROR_BUFFER_TOO_SMALL; 
		}
	}
#endif //_DEBUG

_return:
	return ret; 
}

#define DEF_OP_TMP_BUF_LEN 4096
/** @brief ���ɶ�̬������
* @param buf ��̬������ָ�룬���
* @param len ��̬���������ȣ����
* @return �ɹ�����STATUS_SUCCESS
*/
LRESULT init_op_temp_buf( OUT PBYTE *buf, IN ULONG len, OUT PULONG out_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	PBYTE tmp_buf; 

	ASSERT( buf != NULL ); 
	ASSERT( out_len != NULL ); 
	ASSERT( len > 0 ); 

	tmp_buf = ( PBYTE )malloc( len ); 
	if( tmp_buf == NULL )
	{
		*buf = NULL; 
		*out_len = 0; 
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	*buf = tmp_buf; 
	*out_len = len; 

_return:
	return ret; 
}

/** �ͷŶ�̬������
* @param buf ������ָ��
* @param len ����������
* @return ��
*/
VOID uninit_op_temp_buf( IN OUT PBYTE *buf, IN OUT PULONG len )
{
	free( *buf ); 
	*buf = NULL; 
	*len = 0; 

	return;
}

#define REALLOC_INC_LEN 4096

/** @brief ���㻺�����ĳ����Ƿ�Ϻ�Ҫ���粻С��Ҫ�󳤶ȣ����·���
* @param buf ��ǰ�Ļ�����ָ��/����������ɵĻ�����ָ��
* @param len ��ǰ�������Ѿ�ʹ�ó���
* @param max_len ��ǰ�������ķ��䳤��
* @param need_len ��Ҫʹ�õĳ���
* @return ����ɹ� ����STATUS_SUCCESS�����򷵻ش��� �롣
*/
//LRESULT realloc_if_need( PBYTE *buf, ULONG len, PULONG max_len, ULONG need_len )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	ULONG cur_max_len; 
//	PBYTE old_buf; 
//	PBYTE new_buf; 
//
//	ASSERT( NULL != max_len ); 
//
//	old_buf = *buf; 
//	cur_max_len = *max_len; 
//
//	if( ( cur_max_len - len ) >= need_len )
//	{
//		goto _return; 
//	}
//
//	cur_max_len = len + need_len + REALLOC_INC_LEN; ; 
//
//	new_buf = ( PBYTE )malloc( cur_max_len ); 
//	if( new_buf == NULL )
//	{
//		ret = ERROR_NOT_ENOUGH_MEMORY; 
//		goto _return; 
//	}
//
//	memcpy( new_buf, old_buf, len ); 
//
//	free( old_buf ); 
//
//	*max_len = cur_max_len; 
//	*buf = new_buf; 
//
//_return:
//	return ret; 
//}

LRESULT read_file( ULONG pos, PVOID data, ULONG data_len, LPCTSTR file_name )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	LRESULT ret = ERROR_SUCCESS;
	DWORD _ret; 
	DWORD readed;

	hFile = CreateFile( file_name, 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		ret = GetLastError();
		goto _return;
	}

	_ret = SetFilePointer( hFile, pos, 0, FILE_BEGIN ); 
	if( _ret == INVALID_SET_FILE_POINTER )
	{
		ret = GetLastError(); 
		if( ret != NO_ERROR )
		{
			goto _return;
		}
	}

	_ret = ( DWORD )ReadFile( hFile, data, data_len, &readed, NULL ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	if( readed != data_len )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

_return:
	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
	}

	return ret; 
}

LRESULT write_file( ULONG pos, PVOID data, ULONG data_len, LPCTSTR file_name )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	LRESULT ret = ERROR_SUCCESS;
	DWORD _ret;
	ULONG writed; 

	hFile = CreateFile( file_name, 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		hFile = CreateFile( file_name, 
			GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ, 
			NULL, 
			CREATE_ALWAYS, 
			FILE_ATTRIBUTE_NORMAL, 
			NULL );

		if( INVALID_HANDLE_VALUE == hFile )
		{
			ret = GetLastError(); 
			goto _return;
		}
	}

	SetFilePointer( hFile, pos, 0, FILE_BEGIN ); 

	_ret = ( DWORD )WriteFile( hFile, data, data_len, &writed, NULL );
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	if( writed < data_len )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

_return:
	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
	}

	return ret;
}

#include <tlhelp32.h> 

LRESULT KillExe( LPCTSTR szExeName) 
{ 
	LRESULT ret = ERROR_SUCCESS; 
	PROCESSENTRY32 my; 
	HANDLE proc_snapshot ; 
	int lsr; 

	if( szExeName == NULL ) 
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	//if( *szExeName == _T( '\0' ) )
	//{

	//}

	lsr = lstrlen( szExeName ); 
	if ( lsr == 0 ) 
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}
	

	proc_snapshot  = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 ); 
	if( proc_snapshot == INVALID_HANDLE_VALUE ) 
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "open process snap failed \n" ) ); 
		goto _return; 
	}

	my.dwSize = sizeof( my ); 
	if( Process32First( proc_snapshot , &my ) ) 
	{ 
		do 
		{ 
			if ( lstrcmpi( szExeName, my.szExeFile + lstrlen( my.szExeFile ) - lsr ) == 0 ) 
			{ 
				HANDLE hProcessHandle; 
				hProcessHandle = OpenProcess( 0x1F0FFF, TRUE, my.th32ProcessID ); 
				
				if( hProcessHandle != NULL ) 
				{ 
					if( TerminateProcess( hProcessHandle, 0 ) != FALSE ) 
					{ 
						CloseHandle(hProcessHandle); 
					} 
				}

				goto _return; 
			} 
		}while( Process32Next( proc_snapshot, &my ) ); 
	} 

_return:
	if( proc_snapshot != INVALID_HANDLE_VALUE )
	{
		CloseHandle( proc_snapshot ); 
	}

	return 0; 
} 

#include <tlhelp32.h>

//  Forward declarations:
BOOL GetProcessList( );
BOOL ListProcessModules( DWORD dwPID );
BOOL ListProcessThreads( DWORD dwOwnerPID );

LRESULT get_all_proc_list( PROCESSENTRY32* proc_entrys, ULONG max_entry_num, ULONG *entry_num_output )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	HANDLE proc_snap = NULL; 
	ULONG proc_index = 0; 
	//HANDLE hProcess;
	//DWORD dwPriorityClass;

	ASSERT( proc_entrys != NULL ); 
	ASSERT( max_entry_num > 0 ); 
	ASSERT( entry_num_output != NULL ); 

	*entry_num_output = 0; 

	proc_snap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	
	if( proc_snap == INVALID_HANDLE_VALUE )
	{
		log_trace( ( MSG_ERROR, "get currnet system process snap failed\n" ) );
		ret = GetLastError(); 
		goto _return; 
	}

	proc_entrys[ 0 ].dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	
	_ret = Process32First( proc_snap, &proc_entrys[ 0 ] ); 

	if( _ret == FALSE )
	{
		log_trace( ( MSG_ERROR, "get first process information failed\n" ) ); // Show cause of failure

		//   snapshot object!
		ret = GetLastError(); 
		goto _return; 
	}

	do
	{
		//dwPriorityClass = 0;
		//hProcess = OpenProcess(
		//	PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
		//if( hProcess == NULL )
		//	printError( "OpenProcess" );
		//else
		//{
		//	dwPriorityClass = GetPriorityClass( hProcess );
		//	if( !dwPriorityClass )
		//		printError( "GetPriorityClass" );
		//	CloseHandle( hProcess );
		//}


		proc_index ++; 
		if( proc_index >= max_entry_num )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			*entry_num_output = proc_index; 
			goto _return; 
		}

		proc_entrys[ proc_index ].dwSize = sizeof( PROCESSENTRY32 ); 
		_ret = Process32Next( proc_snap, &proc_entrys[ proc_index ] ); 
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			if( ret == ERROR_NO_MORE_FILES )
			{
				ret = ERROR_SUCCESS; 
				break; 
			}
			else
			{
				goto _return; 
			}
		}

	}while( TRUE );

	*entry_num_output = proc_index; 

_return:
	if( proc_snap != NULL )
	{
		CloseHandle( proc_snap );
	}

	return ret;
}


//BOOL ListProcessModules( DWORD dwPID )
//{
//	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
//	MODULEENTRY32 me32;
//
//	// Take a snapshot of all modules in the specified process.
//	hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwPID );
//	if( hModuleSnap == INVALID_HANDLE_VALUE )
//	{
//		printError( "CreateToolhelp32Snapshot (of modules)" );
//		return( FALSE );
//	}
//
//	// Set the size of the structure before using it.
//	me32.dwSize = sizeof( MODULEENTRY32 );
//
//	// Retrieve information about the first module,
//	// and exit if unsuccessful
//	if( !Module32First( hModuleSnap, &me32 ) )
//	{
//		printError( "Module32First" ); // Show cause of failure
//		CloseHandle( hModuleSnap );    // Must clean up the
//		//   snapshot object!
//		return( FALSE );
//	}
//
//	// Now walk the module list of the process,
//	// and display information about each module
//	do
//	{
//		printf( "\n\n     MODULE NAME:     %s",
//			me32.szModule );
//		printf( "\n     executable     = %s",
//			me32.szExePath );
//		printf( "\n     process ID     = 0x%08X",
//			me32.th32ProcessID );
//		printf( "\n     ref count (g)  =     0x%04X",
//			me32.GlblcntUsage );
//		printf( "\n     ref count (p)  =     0x%04X",
//			me32.ProccntUsage );
//		printf( "\n     base address   = 0x%08X",
//			(DWORD) me32.modBaseAddr );
//		printf( "\n     base size      = %d",
//			me32.modBaseSize );
//
//	} while( Module32Next( hModuleSnap, &me32 ) );
//
//	CloseHandle( hModuleSnap );
//	return( TRUE );
//}
//
//BOOL ListProcessThreads( DWORD dwOwnerPID ) 
//{ 
//	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
//	THREADENTRY32 te32; 
//
//	// Take a snapshot of all running threads  
//	hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
//	if( hThreadSnap == INVALID_HANDLE_VALUE ) 
//		return( FALSE ); 
//
//	// Fill in the size of the structure before using it. 
//	te32.dwSize = sizeof(THREADENTRY32 ); 
//
//	// Retrieve information about the first thread,
//	// and exit if unsuccessful
//	if( !Thread32First( hThreadSnap, &te32 ) ) 
//	{
//		printError( "Thread32First" ); // Show cause of failure
//		CloseHandle( hThreadSnap );    // Must clean up the
//		//   snapshot object!
//		return( FALSE );
//	}
//
//	// Now walk the thread list of the system,
//	// and display information about each thread
//	// associated with the specified process
//	do 
//	{ 
//		if( te32.th32OwnerProcessID == dwOwnerPID )
//		{
//			printf( "\n\n     THREAD ID      = 0x%08X",
//				te32.th32ThreadID ); 
//			printf( "\n     base priority  = %d", te32.tpBasePri ); 
//			printf( "\n     delta priority = %d", te32.tpDeltaPri ); 
//		}
//	} while( Thread32Next(hThreadSnap, &te32 ) ); 
//
//	CloseHandle( hThreadSnap );
//	return( TRUE );
//}

//void printError( TCHAR* msg )
//{
//	DWORD eNum;
//	TCHAR sysMsg[256];
//	TCHAR* p;
//
//	eNum = GetLastError( );
//	FormatMessage(
//		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//		NULL, eNum,
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default lang.
//		sysMsg, 256, NULL );
//
//	// Trim the end of the line and terminate it with a null
//	p = sysMsg;
//	while( ( *p > 31 ) || ( *p == 9 ) )
//		++p;
//	do { *p-- = 0; } while( ( p >= sysMsg ) &&
//		( ( *p == '.' ) || ( *p < 33 ) ) );
//
//	// Display the message
//	printf( "\n  WARNING: %s failed with error %d (%s)",
//		msg, eNum, sysMsg );
//}

//void SelfDeleteBat() 
//{
//	// temporary .bat file
//	static char templ[] = 
//		":Repeat\r\n"
//		"del \"%s\"\r\n"
//		"if exist \"%s\" goto Repeat\r\n"
//		"rmdir \"%s\"\r\n"
//		"del \"%s\"" ;
//
//	char modulename[MAX_PATH] ;    // absolute path of calling .exe file
//	char temppath[MAX_PATH] ;      // absolute path of temporary .bat file
//	char folder[MAX_PATH] ;
//	HANDLE hf ;
//
//	GetTempPath(MAX_PATH, temppath) ;
//	strcat(temppath, tempbatname) ;
//
//	GetModuleFileName(NULL, modulename, MAX_PATH) ;
//	strcpy (folder, modulename) ;
//	char *pb = strrchr(folder, '\\');
//	if (pb != NULL)
//		*pb = 0 ;
//
//	hf = CreateFile(temppath, GENERIC_WRITE, 0, NULL, 
//		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
//
//	if (hf != INVALID_HANDLE_VALUE)
//	{
//		DWORD len;
//		char *bat;
//
//		bat = (char*)alloca(strlen(templ) + 
//			strlen(modulename) * 2 + strlen(temppath) + 20) ;
//
//		wsprintf(bat, templ, modulename, modulename, folder, temppath) ;
//
//		WriteFile(hf, bat, strlen(bat), &len, NULL) ;
//		CloseHandle(hf) ;
//
//		ShellExecute(NULL, "open", temppath, NULL, NULL, SW_HIDE);
//	}
//}

//BOOL SelfDeleteClone()
//{
//	if (__argc == 1) {
//		TCHAR szPathOrig[MAX_PATH], szPathClone[MAX_PATH];
//		GetModuleFileName(NULL, szPathOrig, MAX_PATH);
//		GetTempPath(MAX_PATH, szPathClone);
//		GetTempFileName(szPathClone, __TEXT("Del"), 0, szPathClone);
//		CopyFile(szPathOrig, szPathClone, FALSE);
//
//		//***ע����***:
//		// Open the clone EXE using FILE_FLAG_DELETE_ON_CLOSE
//		HANDLE hfile = CreateFile(szPathClone, 0, FILE_SHARE_READ, NULL, 
//			OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
//
//		// Spawn the clone EXE passing it our EXE's process handle
//		// and the full path name to the Original EXE file.
//		TCHAR szCmdLine[512];
//		HANDLE hProcessOrig = OpenProcess(SYNCHRONIZE, TRUE, GetCurrentProcessId());
//
//		wsprintf(szCmdLine, __TEXT("%s %d \"%s\""), szPathClone, hProcessOrig, szPathOrig);
//		STARTUPINFO si;
//		ZeroMemory(&si, sizeof(si));
//		si.cb = sizeof(si);
//		PROCESS_INFORMATION pi;
//		CreateProcess(NULL, szCmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
//		CloseHandle(hProcessOrig);
//		CloseHandle(hfile);
//	}
//	else
//	{
//		// Clone EXE: When original EXE terminates, delete it
//		HANDLE hProcessOrig = (HANDLE)_ttoi(__targv[1]);
//		WaitForSingleObject(hProcessOrig, INFINITE);
//		CloseHandle(hProcessOrig);
//		DeleteFile(__targv[2]);
//
//		return 1;
//	}
//
//	return 1;
//}

BOOL SelfDeleteASM()
{
	TCHAR buf[MAX_PATH];
	HMODULE module = GetModuleHandle(0);
	GetModuleFileName(module, buf, MAX_PATH);
	CloseHandle((HANDLE)4);

	//������δ�����ܺͲ���ϵͳ���(98/2000/xp), ��Щ����Ҫ������
#ifndef _WIN64
	__asm 
	{
		lea eax, buf
			push 0
			push 0
			push eax
			push ExitProcess
			push module
			push DeleteFile
			push UnmapViewOfFile
			ret
	}
#endif //_WIN64

	return TRUE;
}


//BOOL SelfDeleteCMD()
//{
//	SHELLEXECUTEINFO sei;
//
//	TCHAR szModule [MAX_PATH],
//		szComspec[MAX_PATH],
//		szParams [MAX_PATH];
//
//	// get file path names:
//	if((GetModuleFileName(0,szModule,MAX_PATH)!=0) &&
//		(GetShortPathName(szModule,szModule,MAX_PATH)!=0) &&
//		(GetEnvironmentVariable("COMSPEC",szComspec,MAX_PATH)!=0))
//	{
//		// set command shell parameters
//		lstrcpy(szParams,"/c del ");
//		lstrcat(szParams, szModule);
//		lstrcat(szParams, " > nul");
//
//		// set struct members
//		sei.cbSize       = sizeof(sei);
//		sei.hwnd         = 0;
//		sei.lpVerb       = "Open";
//		sei.lpFile       = szComspec;
//		sei.lpParameters = szParams;
//		sei.lpDirectory  = 0;
//		sei.nShow        = SW_HIDE;
//		sei.fMask        = SEE_MASK_NOCLOSEPROCESS;
//
//		// increase resource allocation to program
//		SetPriorityClass(GetCurrentProcess(),
//			REALTIME_PRIORITY_CLASS);
//		SetThreadPriority(GetCurrentThread(),
//			THREAD_PRIORITY_TIME_CRITICAL);
//
//		// invoke command shell
//		if(ShellExecuteEx(&sei))
//		{
//			// suppress command shell process until program exits
//			SetPriorityClass(sei.hProcess,IDLE_PRIORITY_CLASS);
//			SetProcessPriorityBoost(sei.hProcess,TRUE);
//
//			// notify explorer shell of deletion
//			SHChangeNotify(SHCNE_DELETE,SHCNF_PATH,szModule,0);
//			return TRUE;
//		}
//		else // if error, normalize allocation
//		{
//			SetPriorityClass(GetCurrentProcess(),
//				NORMAL_PRIORITY_CLASS);
//			SetThreadPriority(GetCurrentThread(),
//				THREAD_PRIORITY_NORMAL);
//		}
//	}
//
//	return FALSE;
//}

LRESULT rand_string( LPTSTR file_name, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG rand_num; 

	srand( GetTickCount() ); 
	rand_num = rand(); 

	_ret = _sntprintf( file_name, buf_len, _T( "%d" ), rand_num ); 

	if( _ret < 0 )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

_return:
	return ret; 
}

INLINE LRESULT get_tmp_file_name( LPTSTR file_name, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD _ret; 

	_ret = GetTempPath( MAX_PATH, file_name ); 
	if( _ret == 0 )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "get tmp path failed \n" ) ); 

		goto _return;
	}

	if( file_name[ _tcslen( file_name ) - 1 ] != L'\\' ) 
	{
		_tcsncat( file_name, _T( "\\" ), MAX_PATH ); 
	}

	_tcsncat( file_name, UPDATE_FILE_NAME, MAX_PATH ); 

	//_ret = GetTempFileName( file_name, __TEXT( "tmp" ), 0, file_name ); 

	//if( _ret == 0 )
	//{
	//	ret = GetLastError(); 
	//	log_trace( ( DBG_MSG_AND_ERROR_OUT, "get tmp file name failed \n" ) ); 
	//	goto _return; 
	//}

_return:
	return ret; 
}

LRESULT clone_start_self()
{
#define CMD_LINE_LEN 320
#define _TMP_FILE_LEN 64
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR mod_path[ MAX_PATH ]; 
	TCHAR clone_path[ MAX_PATH ]; 
	TCHAR cmd_line[ CMD_LINE_LEN ]; 
	//TCHAR tmp_file_name[ _TMP_FILE_LEN ]; 
	//HANDLE process = NULL; 
	DWORD _ret; 
	HANDLE clone_file = INVALID_HANDLE_VALUE; 
	STARTUPINFO clone_start_info; 
	PROCESS_INFORMATION clone_process_info = { 0 };
	DWORD wait_ret; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	_ret = GetModuleFileName( NULL, mod_path, MAX_PATH ); 
	if( _ret == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	//_tcscpy( clone_path, mod_path ); 

	//ret = rand_string( tmp_file_name, _TMP_FILE_LEN ); 

	//if( ret != ERROR_SUCCESS )
	//{
	//	goto _return; 
	//}

	//_ret = add_app_path_to_file_name( clone_path, MAX_PATH, tmp_file_name, FALSE ); 
	//if( _ret != ERROR_SUCCESS )
	//{
	//	ret = ERROR_ERRORS_ENCOUNTERED; 
	//	goto _return; 
	//}

	ret = get_tmp_file_name( clone_path, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	//_ret = add_app_path_to_file_name( clone_path, MAX_PATH, UPDATE_FILE_NAME, FALSE ); 
	//if( _ret != ERROR_SUCCESS )
	//{
	//	ret = ERROR_ERRORS_ENCOUNTERED; 
	//	goto _return; 
	//}

	_ret = CopyFile( mod_path, clone_path, FALSE );
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "copy clone file %ws failed \n", clone_path ) ); 

	}

	// Open the clone EXE using FILE_FLAG_DELETE_ON_CLOSE
	clone_file = CreateFile( clone_path, 
		0, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_FLAG_DELETE_ON_CLOSE, 
		NULL ); 

	if( clone_file == INVALID_HANDLE_VALUE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "open the clone file failed \n" ) ); 
		goto _return; 
	}

	// Spawn the clone EXE passing it our EXE's process handle
	// and the full path name to the Original EXE file.

	//process = OpenProcess( SYNCHRONIZE, 
	//	TRUE, 
	//	GetCurrentProcessId() ); 

	//if( process == NULL )
	//{
	//	ret = GetLastError(); 
	//	goto _return; 
	//}

	
	ZeroMemory( &clone_start_info, sizeof( clone_start_info ));
	clone_start_info.cb = sizeof( clone_start_info ); 
	_ret = _sntprintf( cmd_line, CMD_LINE_LEN, _T( "%s /u \"%s\"" ), clone_path, mod_path ); 

	if( ( INT32 )_ret < 0 )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	_ret = ( BOOL )CreateProcess( NULL, 
		cmd_line, 
		NULL, 
		NULL, 
		TRUE, 
		0, 
		NULL, 
		NULL, 
		&clone_start_info, 
		&clone_process_info ); 

	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "start the process from the clone file failed \n" ) ); 
		goto _return; 
	}

	wait_ret = WaitForSingleObject( clone_process_info.hProcess, INFINITE ); 

	if( wait_ret != WAIT_TIMEOUT 
		&& wait_ret != WAIT_OBJECT_0 
		&& wait_ret != WAIT_ABANDONED )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!wait clone process failed\n" ) ); 
		ASSERT( FALSE ); 
	}

_return:
	//if( process != NULL )
	//{
	//	CloseHandle( process );
	//}

	if( clone_file != INVALID_HANDLE_VALUE )
	{
		CloseHandle( clone_file ); 
	}

	if( clone_process_info.hProcess != NULL )
	{
		CloseHandle( clone_process_info.hProcess );
		ASSERT( clone_process_info.hThread != NULL ); 
		CloseHandle( clone_process_info.hThread );
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

ULONG CALLBACK clone_start_self_thread( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = clone_start_self(); 
	
	return ( ULONG )ret; 
}

LRESULT start_clone_process()
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE thread_clone_start; 

	thread_clone_start = CreateThread( NULL, 0, clone_start_self_thread, NULL, 0, NULL ); 
	if( thread_clone_start == NULL )
	{
		ret = GetLastError(); 
		ASSERT( ret != ERROR_SUCCESS ); 
	}

	return ret; 
}

LRESULT run_proc( LPCTSTR cmd_line )
{
#define CMD_LINE_LEN 320

	LRESULT ret = ERROR_SUCCESS; 
	//HANDLE process = NULL; 
	DWORD _ret; 
	STARTUPINFO clone_start_info; 
	PROCESS_INFORMATION clone_process_info = { 0 };
	//DWORD wait_ret; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ZeroMemory( &clone_start_info, sizeof( clone_start_info ));
	clone_start_info.cb = sizeof( clone_start_info ); 

	_ret = ( BOOL )CreateProcess( NULL, 
		( LPWSTR )cmd_line, 
		NULL, 
		NULL, 
		TRUE, 
		0, 
		NULL, 
		NULL, 
		&clone_start_info, 
		&clone_process_info ); 

	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "start the process from the command line failed \n" ) ); 
		goto _return; 
	}

	//wait_ret = WaitForSingleObject( clone_process_info.hProcess, INFINITE ); 

	//if( wait_ret != WAIT_TIMEOUT 
	//	&& wait_ret != WAIT_OBJECT_0 
	//	&& wait_ret != WAIT_ABANDONED )
	//{
	//	log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!wait clone process failed\n" ) ); 
	//	ASSERT( FALSE ); 
	//}

_return:

	// Close process and thread handles. 
	if( clone_process_info.hProcess != NULL )
	{
		CloseHandle( clone_process_info.hProcess );
		ASSERT( clone_process_info.hThread != NULL ); 
		CloseHandle( clone_process_info.hThread );
	}

	return ret; 
}

LRESULT run_proc_sync( LPCTSTR cmd_line )
{
#define CMD_LINE_LEN 320
#define _TMP_FILE_LEN 64
	LRESULT ret = ERROR_SUCCESS; 
	//HANDLE process = NULL; 
	DWORD _ret; 
	STARTUPINFO clone_start_info; 
	PROCESS_INFORMATION clone_process_info = { 0 };
	DWORD wait_ret; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ZeroMemory( &clone_start_info, sizeof( clone_start_info ));
	clone_start_info.cb = sizeof( clone_start_info ); 

	_ret = ( BOOL )CreateProcess( NULL, 
		cmd_line, 
		NULL, 
		NULL, 
		TRUE, 
		0, 
		NULL, 
		NULL, 
		&clone_start_info, 
		&clone_process_info ); 

	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "start the process from the command line failed \n" ) ); 
		goto _return; 
	}

	wait_ret = WaitForSingleObject( clone_process_info.hProcess, INFINITE ); 

	if( wait_ret != WAIT_TIMEOUT 
		&& wait_ret != WAIT_OBJECT_0 
		&& wait_ret != WAIT_ABANDONED )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!wait clone process failed\n" ) ); 
		ASSERT( FALSE ); 
	}

_return:

	// Close process and thread handles. 
	if( clone_process_info.hProcess != NULL )
	{
		CloseHandle( clone_process_info.hProcess );
		ASSERT( clone_process_info.hThread != NULL ); 
		CloseHandle( clone_process_info.hThread );
	}

	return ret; 
}

LRESULT create_file_from_res( HINSTANCE hResMod, LPCTSTR pszName, LPCTSTR pszType, LPCTSTR pszSysFile, INT32 sys_path )
{
	// Get the full path of the output file.

	LRESULT ret = ERROR_SUCCESS; 
	TCHAR szOutFullPath[ MAX_PATH * 2 ];
	HRSRC hRes;
	HGLOBAL hgResMem;
	LPVOID pRes;
	DWORD dwResSize;
	FILE* pfDriverFile;
	BOOL bExitWithErr;

	if( sys_path == TRUE )
	{
		ret = add_sys_drv_path_to_file_name( szOutFullPath, MAX_PATH, pszSysFile, FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}
	else
	{
		ret = add_app_path_to_file_name( szOutFullPath, MAX_PATH, pszSysFile, FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

	// Get a memory copy of the Resource.

	hRes = FindResource( hResMod, pszName, pszType );
	if ( hRes == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	hgResMem = LoadResource( hResMod, hRes );
	if ( hgResMem == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	pRes = LockResource( hgResMem );
	if ( pRes == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	dwResSize = SizeofResource( hResMod, hRes );
	if ( dwResSize == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	// Write the Driver File on disk.

	pfDriverFile = _tfopen( szOutFullPath, _T( "wb" ) );
	if ( pfDriverFile == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	// Write the file.

	bExitWithErr = FALSE;

	if ( fwrite( pRes, 1, dwResSize, pfDriverFile ) != dwResSize )
		bExitWithErr = TRUE;

	fclose( pfDriverFile );

	// Exit, eventually, returning error.

	if ( bExitWithErr == TRUE )
	{
		DeleteFile( szOutFullPath );
		
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	// Return to the Caller.
_return:
	return ret;
}

BOOL GetTokenByName( HANDLE *hToken, LPCTSTR lpName )
{
	HANDLE         proc_snap = NULL;
	BOOL           bRet      = FALSE;
	PROCESSENTRY32 pe32      = {0};

	ASSERT( hToken != NULL ); 

	if(!lpName)
	{
		return FALSE;
	}

	proc_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	if (proc_snap == INVALID_HANDLE_VALUE)
		return (FALSE);

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(proc_snap, &pe32))
	{  
		do
		{
			if( _tcsnicmp( pe32.szExeFile, lpName, ARRAY_SIZE( pe32.szExeFile ) ) == 0 )
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
					FALSE,pe32.th32ProcessID);
				bRet = OpenProcessToken(hProcess,TOKEN_ALL_ACCESS,hToken);
				CloseHandle (proc_snap);
				return (bRet);
			}
		}
		while (Process32Next(proc_snap, &pe32));
		bRet = TRUE;
	}
	else
		bRet = FALSE;

	CloseHandle (proc_snap);
	return (bRet);
}

//BOOL RunProcess(LPCSTR lpImage)
//{
//	if(!lpImage)
//	{
//		return FALSE;
//	}
//	HANDLE hToken;
//	if(!GetTokenByName(hToken,"EXPLORER.EXE"))
//	{
//		return FALSE;
//	}
//	STARTUPINFO si;
//	PROCESS_INFORMATION pi;
//
//	ZeroMemory(&si, sizeof(STARTUPINFO));
//	si.cb= sizeof(STARTUPINFO);
//	si.lpDesktop = TEXT("winsta0\\default");
//
//	BOOL bResult = CreateProcessAsUser(hToken,lpImage,NULL,NULL,NULL,
//		FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
//	CloseHandle(hToken);
//	if(bResult)
//	{
//		OutputDebugString("CreateProcessAsUser ok!\r\n");
//	}
//	else
//	{
//		OutputDebugString("CreateProcessAsUser false!\r\n");
//	}
//	return bResult;
//}

void write_log( LPCSTR log_file, char* msg )
{
	FILE* logger = NULL; 

	logger = fopen( log_file, "a" ); 
	if( logger == NULL )
	{
		goto _return; 
	}

	fprintf( logger, msg ); 
	fclose( logger );


_return:
	return; 
}

//LRESULT update_resource()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HRSRC hResLoad;     // handle to loaded resource 
//	HANDLE hExe;        // handle to existing .EXE file 
//	HRSRC hRes;         // handle/ptr. to res. info. in hExe 
//	HANDLE hUpdateRes;  // update resource handle 
//	char *lpResLock;    // pointer to resource data 
//	BOOL result; 
//	// Load the .EXE file that contains the dialog box you want to copy. 
//	hExe = LoadLibrary("hand.exe"); 
//	if (hExe == NULL) 
//	{ 
//		ErrorHandler("Could not load exe."); 
//	} 
//
//	// Locate the dialog box resource in the .EXE file. 
//	hRes = FindResource(hExe, "AboutBox", RT_DIALOG); 
//	if (hRes == NULL) 
//	{ 
//		ErrorHandler("Could not locate dialog box."); 
//	} 
//
//	// Load the dialog box into global memory. 
//	hResLoad = LoadResource(hExe, hRes); 
//	if (hResLoad == NULL) 
//	{ 
//		ErrorHandler("Could not load dialog box."); 
//	} 
//
//	// Lock the dialog box into global memory. 
//	lpResLock = LockResource(hResLoad); 
//	if (lpResLock == NULL) 
//	{ 
//		ErrorHandler("Could not lock dialog box."); 
//	} 
//
//	// Open the file to which you want to add the dialog box resource. 
//	hUpdateRes = BeginUpdateResource("foot.exe", FALSE); 
//	if (hUpdateRes == NULL) 
//	{ 
//		ErrorHandler("Could not open file for writing."); 
//	} 
//
//	// Add the dialog box resource to the update list. 
//	result = UpdateResource(hUpdateRes,       // update resource handle 
//		RT_DIALOG,                   // change dialog box resource 
//		"AboutBox",                  // dialog box name 
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),  // neutral language
//		lpResLock,                   // ptr to resource info 
//		SizeofResource(hExe, hRes)); // size of resource info. 
//	if (result == FALSE) 
//	{ 
//		ErrorHandler("Could not add resource."); 
//	} 
//
//	// Write changes to FOOT.EXE and then close it. 
//	if (!EndUpdateResource(hUpdateRes, FALSE)) 
//	{ 
//		ErrorHandler("Could not write changes to file."); 
//	} 
//
//	// Clean up. 
//	if (!FreeLibrary(hExe)) 
//	{ 
//		ErrorHandler("Could not free executable."); 
//	} 
//
//	return ret; 
//}

//LRESULT create_resource_list()
//{
//	char szBuffer[80]; // print buffer for EnumResourceTypes 
//	DWORD cbWritten;   // number of bytes written to res. info. file 
//	size_t cbString;      // length of string in sprintf
//	LRESULT hResult;
//
//	// Declare callback functions. 
//	BOOL EnumTypesFunc( 
//		HANDLE hModule, 
//		LPTSTR lpType, 
//		LONG lParam); 
//
//	BOOL EnumNamesFunc( 
//		HANDLE hModule, 
//		LPCTSTR lpType, 
//		LPTSTR lpName, 
//		LONG lParam); 
//
//	BOOL EnumLangsFunc( 
//		HANDLE hModule, 
//		LPCTSTR lpType, 
//		LPCTSTR lpName, 
//		WORD wLang, 
//		LONG lParam); 
//
//	// Load the .EXE whose resources you want to list. 
//	hExe = LoadLibrary("hand.exe"); 
//	if (hExe == NULL) 
//	{
//		// Add code to fail as securely as possible.
//		return;
//	} 
//
//	// Create a file to contain the resource info. 
//	hFile = CreateFile("resinfo.txt",      // name of file 
//		GENERIC_READ | GENERIC_WRITE,      // access mode 
//		0,                                 // share mode 
//		(LPSECURITY_ATTRIBUTES) NULL,      // default security 
//		CREATE_ALWAYS,                     // create flags 
//		FILE_ATTRIBUTE_NORMAL,             // file attributes 
//		(HANDLE) NULL);                    // no template 
//	if (hFile == INVALID_HANDLE_VALUE) { 
//		ErrorHandler("Could not open file."); 
//	} 
//
//	// Find all of the loaded file's resources.
//	hResult = StringCchPrintf(szBuffer, 80/sizeof(TCHAR),  
//		// cbString = sprintf(szBuffer, 
//		"The file contains the following resources:\n\n");
//	if (FAILED(hResult))
//	{
//		// Add code to fail as securely as possible.
//		return;
//	}
//	hResult = StringCchLength(szBuffer, 80/sizeof(TCHAR), cbString);
//	if (FAILED(hResult))
//	{
//		// Add code to fail as securely as possible.
//		return;
//	} 
//	WriteFile(hFile,           // file to hold resource info. 
//		szBuffer,              // what to write to the file 
//		(DWORD) cbString,      // number of bytes in szBuffer 
//		&cbWritten,            // number of bytes written 
//		NULL);                 // no overlapped I/O 
//
//	EnumResourceTypes(hExe,              // module handle 
//		(ENUMRESTYPEPROC)EnumTypesFunc,  // callback function 
//		0);                              // extra parameter 
//	// Unload the executable file whose resources were 
//	// enumerated and close the file created to contain 
//	// the resource information. 
//	FreeLibrary(hExe); 
//	CloseHandle(hFile); 
//}

//    FUNCTION: EnumTypesFunc(HANDLE, LPSTR, LONG) 
// 
//    PURPOSE:  Resource type callback 
//BOOL EnumTypesFunc( 
//				   HANDLE hModule,   // module handle 
//				   LPTSTR lpType,    // address of resource type 
//				   LONG lParam)      // extra parameter, could be 
//				   // used for error checking 
//{ 
//	size_t cbString;
//	HRESULT hResult;
//
//	// Write the resource type to a resource information file. 
//	// The type may be a string or an unsigned decimal 
//	// integer, so test before printing. 
//	if ((ULONG)lpType & 0xFFFF0000) 
//	{ 
//		hResult = StringCchPrintf(szBuffer, 80/sizeof(TCHAR), "Type: %s\n", lpType);
//		if (FAILED(hResult))
//		{
//			// Add code to fail as securely as possible.
//			return;
//		}
//		// cbString = sprintf(szBuffer, "Type: %s\n", lpType); 
//	} 
//	else 
//	{
//		hResult = StringCchPrintf(szBuffer, 80/sizeof(TCHAR), "Type: %u\n", (USHORT)lpType);
//		if (FAILED(hResult))
//		{
//			// Add code to fail as securely as possible.
//			return;
//		}
//		//        cbString = sprintf(szBuffer, "Type: %u\n", (USHORT)lpType); 
//	} 
//	hResult = StringCchLength(szBuffer, 80/sizeof(TCHAR), cbString);
//	if (FAILED(hResult))
//	{
//		// Add code to fail as securely as possible.
//		return;
//	} 
//	WriteFile(hFile, szBuffer, (DWORD) cbString, 
//		&cbWritten, NULL); 
//	// Find the names of all resources of type lpType. 
//	EnumResourceNames(hModule, 
//		lpType, 
//		(ENUMRESNAMEPROC)EnumNamesFunc, 
//		0); 
//
//	return TRUE; 
//} 

//    FUNCTION: EnumNamesFunc(HANDLE, LPSTR, LPSTR, LONG) 
// 
//    PURPOSE:  Resource name callback 
//BOOL EnumNamesFunc( 
//				   HANDLE hModule,   // module handle 
//				   LPCTSTR lpType,   // address of resource type 
//				   LPTSTR lpName,    // address of resource name 
//				   LONG lParam)      // extra parameter, could be 
//				   // used for error checking 
//{ 
//	size_t cbString; 
//	HRESULT hResult;
//
//	// Write the resource name to a resource information file. 
//	// The name may be a string or an unsigned decimal 
//	// integer, so test before printing. 
//	if ((ULONG)lpName & 0xFFFF0000) 
//	{
//		hResult = StringCchPrintf(szBuffer, 80/sizeof(TCHAR), "\tName: %s\n", lpName);
//		if (FAILED(hResult))
//		{
//			// Add code to fail as securely as possible.
//			return;
//		}
//		//        cbString = sprintf(szBuffer, "\tName: %s\n", lpName); 
//	} 
//	else 
//	{
//		hResult = StringCchPrintf(szBuffer, 80/sizeof(TCHAR), "\tName: %u\n", (USHORT)lpName);
//		if (FAILED(hResult))
//		{
//			// Add code to fail as securely as possible.
//			return;
//		} 
//		//        cbString = sprintf(szBuffer, "\tName: %u\n", 
//		(USHORT)lpName); 
//	}
//	hResult = StringCchLength(szBuffer, 80/sizeof(TCHAR), cbString);
//	if (FAILED(hResult))
//	{
//		// Add code to fail as securely as possible.
//		return;
//	} 
//
//	WriteFile(hFile, szBuffer, (DWORD) cbString, 
//		&cbWritten, NULL); 
//	// Find the languages of all resources of type 
//	// lpType and name lpName. 
//	EnumResourceLanguages(hModule, 
//		lpType, 
//		lpName, 
//		(ENUMRESLANGPROC)EnumLangsFunc, 
//		0); 
//
//	return TRUE; 
//} 
//
////    FUNCTION: EnumLangsFunc(HANDLE, LPSTR, LPSTR, WORD, LONG) 
//// 
////    PURPOSE:  Resource language callback 
//BOOL EnumLangsFunc( 
//				   HANDLE hModule,  // module handle 
//				   LPCTSTR lpType,  // address of resource type 
//				   LPCTSTR lpName,  // address of resource name 
//				   WORD wLang,      // resource language 
//				   LONG lParam)     // extra parameter, could be 
//				   // used for error checking 
//{ 
//	HANDLE hResInfo; 
//	char szBuffer[80]; 
//	size_t cbString = 0; 
//	HRESULT hResult;
//
//	hResInfo = FindResourceEx(hModule, lpType, lpName, wLang); 
//	// Write the resource language to the resource information file.
//	hResult = StringCchPrintf(szBuffer, 80/sizeof(TCHAR), "\t\tLanguage: %u\n", (USHORT) wLang);
//	if (FAILED(hResult))
//	{
//		// Add code to fail as securely as possible.
//		return;
//	}
//	hResult = StringCchLength(szBuffer, 80/sizeof(TCHAR), cbString);
//	if (FAILED(hResult))
//	{
//		// Add code to fail as securely as possible.
//		return;
//	} 
//	//    cbString = sprintf(szBuffer, "\t\tLanguage: %u\n", (USHORT)wLang); 
//	WriteFile(hFile, szBuffer, (DWORD) cbString, 
//		&cbWritten, NULL); 
//	// Write the resource handle and size to buffer. 
//	cbString = sprintf(szBuffer, 
//		"\t\thResInfo == %lx,  Size == %lu\n\n", 
//		hResInfo, 
//		SizeofResource(hModule, hResInfo)); 
//	WriteFile(hFile, szBuffer, (DWORD) cbString, 
//		&cbWritten, NULL); 
//	return TRUE; 
//} 

INT32 _tcsdigit( LPCTSTR str )
{
	INT32 index = 0; 
	INT32 ret = TRUE; 

	for( ; ; )
	{
		if( str[ index ] == _T( '\0' ) )
		{
			break; 
		}

		if( isdigit( str[ index ] ) == FALSE )
		{
			ret = FALSE; 
			goto _return; 
		}

		index ++; 
	}

_return:
	return ret; 
}

LRESULT str_2_ipv4_addr( LPCTSTR ip_addr, ULONG *output )
{
	LRESULT ret = ERROR_BAD_FORMAT; 
	ULONG _ip_addr = 0; 
	INT32 i; 
	LPCTSTR begin; 
	LPCTSTR end; 
	TCHAR bytes[ 4 ]; 
	register ULONG byte_val; 

	ASSERT( ip_addr != NULL ); 
	ASSERT( output != NULL ); 

	begin = ip_addr; 
	*output = 0; 

	if( _tcscmp( ip_addr, _T( "0" ) ) == 0 )
	{
		_ip_addr = 0; 
		ret = ERROR_SUCCESS; 
		goto _return; 
	}

	for( i = 0; i < 4; i ++ )
	{
		end = _tcschr( begin, _T( '.' ) ); 
		if( end == NULL )
		{
			if( i < 3 )
			{
				goto _return; 
			}

			if( _tcslen( begin ) > 3 )
			{
				goto _return; 
			}

			_tcscpy( bytes, begin ); 

			byte_val = ( ULONG )_tcstoul( bytes, NULL, 10 ); 
			if( ( byte_val & 0xffffff00 ) != 0 )
			{
				goto _return; 
			}

			_ip_addr |= byte_val; 
			break; 
		}
		else 
		{
			if( i == 3 )
			{
				goto _return; 
			}
		}

		if( ( end - begin ) > 3 )
		{
			goto _return; 
		}

		memcpy( bytes, begin, sizeof( TCHAR ) * ( end - begin ) ); 
		bytes[ end - begin ] = '\0'; 

		if( _tcsdigit( bytes ) == FALSE )
		{
			goto _return; 
		}

		byte_val = ( ULONG )_tcstoul( bytes, NULL, 10 ); 
		if( ( byte_val & 0xffffff00 ) != 0 )
		{
			goto _return; 
		}

		_ip_addr |= ( byte_val << ( ( 3 - i ) * 8 ) ); 

		begin = end + 1; 
	}

	*output = _ip_addr; 
	ret = ERROR_SUCCESS; 

#ifdef _DEBUG
	{
		PBYTE bytes; 
		bytes = ( PBYTE )&_ip_addr; 
		log_trace( ( MSG_INFO, "convert the ip address from %s to %d.%d.%d.%d\n", 
			ip_addr, 
			bytes[ 3 ], 
			bytes[ 2 ], 
			bytes[ 1 ], 
			bytes[ 0 ] ) ); 
	}
#endif //_DEBUG
_return:
	return ret; 
}

LRESULT str_2_port( LPCTSTR port, USHORT *output )
{
	LRESULT ret = ERROR_BAD_FORMAT; 
	ULONG port_val; 

	ASSERT( port != NULL ); 
	ASSERT( output != NULL ); 

	*output = 0; 

	if( _tcsdigit( port ) == FALSE )
	{
		goto _return; 
	}

	port_val = ( ULONG )_tcstoul( port, NULL, 10 ); 
	if( ( port_val & 0xffff0000 ) != 0 )
	{
		goto _return; 
	}

	*output = port_val; 
	ret = ERROR_SUCCESS; 

_return:
	return ret; 
}

LRESULT ip_addr_2_str( ULONG ip_addr, TCHAR *buf, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	_sntprintf( buf, buf_len, _T( "%u.%u.%u.%u" ), 
		( ( ip_addr & 0xff000000 ) >> 24 ), 
		( ( ip_addr & 0x00ff0000 ) >> 16), 
		( ( ip_addr & 0x0000ff00 ) >> 8 ), 
		( ip_addr & 0x000000ff )
		);  

	return ret; 
} 

LRESULT port_2_str( USHORT port, TCHAR *buf, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	_sntprintf( buf, buf_len, _T( "%d" ), port ); 

	//_return:
	return ret; 
}

/** @brief ���㻺�����ĳ����Ƿ�Ϻ�Ҫ���粻С��Ҫ�󳤶ȣ����·���
* @param buf ��ǰ�Ļ�����ָ��/����������ɵĻ�����ָ��
* @param len ��ǰ�������Ѿ�ʹ�ó���
* @param max_len ��ǰ�������ķ��䳤��
* @param need_len ��Ҫʹ�õĳ���
* @return ����ɹ� ����STATUS_SUCCESS�����򷵻ش��� �롣
*/
LRESULT realloc_if_need( PBYTE *buf, ULONG len, PULONG max_len, ULONG need_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG cur_max_len; 
	PBYTE old_buf; 
	PBYTE new_buf; 

	ASSERT( NULL != max_len ); 

	old_buf = *buf; 
	cur_max_len = *max_len; 

	if( ( cur_max_len - len ) >= need_len )
	{
		goto _return; 
	}

	cur_max_len = len + need_len + REALLOC_INC_LEN; ; 

	new_buf = ( PBYTE )malloc( cur_max_len ); 
	if( new_buf == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	memcpy( new_buf, old_buf, len ); 

	free( old_buf ); 

	*max_len = cur_max_len; 
	*buf = new_buf; 

_return:
	return ret; 
}

// ���庯������ֵ
//typedef ULONG NTSTATUS;

// ���ֽ��ַ����ṹ����
//typedef struct _UNICODE_STRING {
//	USHORT  Length;
//	USHORT  MaximumLength;
//	PWSTR  Buffer;
//} UNICODE_STRING, *PUNICODE_STRING;

// �������Զ���
typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	UNICODE_STRING *ObjectName;
	ULONG Attributes;
	PSECURITY_DESCRIPTOR SecurityDescriptor;
	PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
} OBJECT_ATTRIBUTES,*POBJECT_ATTRIBUTES;

// ������Ϣ����
typedef struct _DIRECTORY_BASIC_INFORMATION {
	UNICODE_STRING ObjectName;
	UNICODE_STRING ObjectTypeName;
} DIRECTORY_BASIC_INFORMATION, *PDIRECTORY_BASIC_INFORMATION;

// ����ֵ��״̬���Ͷ���
#define OBJ_CASE_INSENSITIVE      0x00000040L
#define DIRECTORY_QUERY           (0x0001)
#define STATUS_SUCCESS            ((NTSTATUS)0x00000000L) // ntsubauth
#define STATUS_MORE_ENTRIES       ((NTSTATUS)0x00000105L)
#define STATUS_NO_MORE_ENTRIES    ((NTSTATUS)0x8000001AL)
#define STATUS_BUFFER_TOO_SMALL   ((NTSTATUS)0xC0000023L)
#define SYMBOLIC_LINK_QUERY       (0x0001)
#define SYMBOLIC_LINK_ALL_ACCESS  (STANDARD_RIGHTS_REQUIRED | 0x1)

// ��ʼ���������Ժ궨��
#define InitializeObjectAttributes( p, n, a, r, s ) { \
	(p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
	(p)->RootDirectory = r;                             \
	(p)->Attributes = a;                                \
	(p)->ObjectName = n;                                \
	(p)->SecurityDescriptor = s;                        \
	(p)->SecurityQualityOfService = NULL;               \
}

// �ַ�����ʼ��
typedef VOID (CALLBACK* RTLINITUNICODESTRING)(PUNICODE_STRING,PCWSTR);
RTLINITUNICODESTRING RtlInitUnicodeString;

// �ַ����Ƚ�
typedef
BOOLEAN
(WINAPI *RTLEQUALUNICODESTRING)(
								const UNICODE_STRING *String1,
								const UNICODE_STRING *String2,
								BOOLEAN CaseInSensitive
								);
RTLEQUALUNICODESTRING RtlEqualUnicodeString;

// �򿪶���
typedef NTSTATUS (WINAPI *ZWOPENDIRECTORYOBJECT)(
	OUT PHANDLE DirectoryHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
	);
ZWOPENDIRECTORYOBJECT ZwOpenDirectoryObject;

// ��ѯ����
typedef
NTSTATUS
(WINAPI *ZWQUERYDIRECTORYOBJECT)(
								 IN HANDLE DirectoryHandle,
								 OUT PVOID Buffer,
								 IN ULONG BufferLength,
								 IN BOOLEAN ReturnSingleEntry,
								 IN BOOLEAN RestartScan,
								 IN OUT PULONG Context,
								 OUT PULONG ReturnLength OPTIONAL
								 );
ZWQUERYDIRECTORYOBJECT ZwQueryDirectoryObject;

// �򿪷������Ӷ���
typedef
NTSTATUS
(WINAPI *ZWOPENSYMBOLICKLINKOBJECT)(
									OUT PHANDLE SymbolicLinkHandle,
									IN ACCESS_MASK DesiredAccess,
									IN POBJECT_ATTRIBUTES ObjectAttributes
									);
ZWOPENSYMBOLICKLINKOBJECT ZwOpenSymbolicLinkObject;

// ��ѯ�������Ӷ���
typedef
NTSTATUS
(WINAPI *ZWQUERYSYMBOLICKLINKOBJECT)(
									 IN HANDLE SymbolicLinkHandle,
									 IN OUT PUNICODE_STRING TargetName,
									 OUT PULONG ReturnLength OPTIONAL
									 );
ZWQUERYSYMBOLICKLINKOBJECT ZwQuerySymbolicLinkObject;

typedef VOID
( WINAPI *RTLCOPYUNICODESTRING )(
								 PUNICODE_STRING DestinationString,
								 PUNICODE_STRING SourceString
								 );

RTLCOPYUNICODESTRING RtlCopyUnicodeString; 
// �ر��Ѿ��򿪵Ķ���
typedef
NTSTATUS
(WINAPI *ZWCLOSE)(
				  IN HANDLE Handle
				  );
ZWCLOSE ZwClose;


LRESULT enum_symbolic_links( WCHAR *sym_name, WCHAR *dev_name, ULONG buf_len )
{
	LRESULT ret = 0; 
	UNICODE_STRING     strDirName;
	UNICODE_STRING sym_find; 
	UNICODE_STRING dev_name_out; 
	OBJECT_ATTRIBUTES  oba;
	NTSTATUS           ntStatus; 
	HANDLE             hDirectory;
	HMODULE hNtdll = NULL;
	UNICODE_STRING symbolicLink;
	UNICODE_STRING targetName; 
	PDIRECTORY_BASIC_INFORMATION  directoryInfo; 
	BYTE           buffer[2048] = {0};
	ULONG          ulLength  = 2048;  
	ULONG          ulContext = 0;
	ULONG          ulRet     = 0; 


	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 
	hNtdll = LoadLibrary( _T( "ntdll.dll" ) ); 
	if ( NULL == hNtdll )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "[%s]--Load ntdll.dll failed.\r\n", __FUNCTION__ ) );
		goto _return;
	}

	RtlInitUnicodeString      = (RTLINITUNICODESTRING)GetProcAddress( hNtdll, "RtlInitUnicodeString");
	RtlEqualUnicodeString     = (RTLEQUALUNICODESTRING)GetProcAddress( hNtdll, "RtlEqualUnicodeString");
	ZwOpenDirectoryObject     = (ZWOPENDIRECTORYOBJECT)GetProcAddress( hNtdll, "ZwOpenDirectoryObject");
	ZwQueryDirectoryObject    = (ZWQUERYDIRECTORYOBJECT)GetProcAddress( hNtdll, "ZwQueryDirectoryObject");
	ZwOpenSymbolicLinkObject  = (ZWOPENSYMBOLICKLINKOBJECT)GetProcAddress( hNtdll, "ZwOpenSymbolicLinkObject");
	ZwQuerySymbolicLinkObject = (ZWQUERYSYMBOLICKLINKOBJECT)GetProcAddress( hNtdll, "ZwQuerySymbolicLinkObject");
	RtlCopyUnicodeString = (RTLCOPYUNICODESTRING)GetProcAddress( hNtdll, "RtlCopyUnicodeString");
	ZwClose                   = (ZWCLOSE)GetProcAddress( hNtdll, "ZwClose");

	ASSERT( sym_name != NULL ); 
	ASSERT( dev_name != NULL ); 
	ASSERT( buf_len > 0 ); 

	RtlInitUnicodeString( &sym_find, sym_name ); 
	dev_name_out.Buffer = dev_name; 
	dev_name_out.Length = 0; 
	dev_name_out.MaximumLength = buf_len; 

	RtlInitUnicodeString( &strDirName, L"\\global??" );
	InitializeObjectAttributes( &oba, &strDirName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	log_trace( ( MSG_INFO, "[%s]--Open directory object now.\r\n", __FUNCTION__ ) );
	ntStatus = ZwOpenDirectoryObject( &hDirectory, DIRECTORY_QUERY, &oba );
	if ( ntStatus != STATUS_SUCCESS )
	{
		ret = -1; 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "[%s]--Open directory object failed.\r\n", __FUNCTION__ ) ); 
		goto _return;
	}

	log_trace( ( MSG_INFO, "[%s]--Open directory object success.\r\n", __FUNCTION__ ) ); 

	RtlInitUnicodeString(&symbolicLink, L"SymbolicLink");

	targetName.Length = 0;
	targetName.Buffer = (PWSTR)calloc(2, 1024);
	if ( targetName.Buffer == NULL )
	{
		ret = -1; 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "[%s]--calloc failed\r\n", __FUNCTION__ ) ); 
		goto _return;
	}

	targetName.MaximumLength = 1024;

	do
	{
		ntStatus = ZwQueryDirectoryObject(hDirectory, buffer, ulLength, TRUE, FALSE, &ulContext, &ulRet);
		if ( (ntStatus != STATUS_SUCCESS) && (ntStatus != STATUS_NO_MORE_ENTRIES) )
		{
			ret = -1; 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "[%s]--ZwQueryDirectoryObject failed.\r\n", __FUNCTION__) ); 
			goto _return;
		}
		else if ( STATUS_NO_MORE_ENTRIES == ntStatus )
		{
			ret = -1; 
			log_trace( ( MSG_INFO, "[%s]--No more object.\r\n", __FUNCTION__ ) ); 
			goto _return;
		}

		directoryInfo = (PDIRECTORY_BASIC_INFORMATION)buffer;
		log_trace( ( MSG_INFO, "ObjectName: [%S]---ObjectTypeName: [%S]\n", 
			directoryInfo->ObjectName.Buffer, directoryInfo->ObjectTypeName.Buffer ) ); 

		if ( RtlEqualUnicodeString(&directoryInfo->ObjectTypeName, &symbolicLink, TRUE) && 
			RtlEqualUnicodeString(&directoryInfo->ObjectName, &sym_find, FALSE ) )
		{
			OBJECT_ATTRIBUTES symbolicLinkAttributes;
			HANDLE            hSymbolicLink;

			// ��ʼ���������Ӷ�������
			InitializeObjectAttributes(&symbolicLinkAttributes, &directoryInfo->ObjectName, 
				OBJ_CASE_INSENSITIVE, hDirectory, NULL);

			// �򿪷������Ӷ���
			ntStatus = ZwOpenSymbolicLinkObject(&hSymbolicLink, SYMBOLIC_LINK_QUERY, &symbolicLinkAttributes);
			if ( ntStatus != STATUS_SUCCESS )
			{
				ret = -1; 
				log_trace( ( MSG_INFO, "[%s]--ZwOpenSymbolicLinkObject failed.\r\n", __FUNCTION__ ) ); 
				goto _return;
			}

			// ��ѯ�������Ӷ���
			ntStatus = ZwQuerySymbolicLinkObject(hSymbolicLink, &targetName, NULL); 
			if ( ntStatus != STATUS_SUCCESS )
			{
				ret = -1; 
				log_trace( ( MSG_INFO, "[%s]--ZwQuerySymbolicLinkObject failed.\r\n", __FUNCTION__ ) ); 
				ZwClose( hSymbolicLink );
				goto _return;
			}

			RtlCopyUnicodeString( &dev_name_out, &targetName ); 
			dev_name_out.Buffer[ dev_name_out.MaximumLength / sizeof( WCHAR ) - 1 ] = L'\0'; 

			// TODO: ������Է������ӵĴ�������

			// ���targetName
			//memset((LPVOID)targetName.Buffer, 0, targetName.Length*sizeof(WCHAR));
			// �رշ������Ӿ��
			ZwClose( hSymbolicLink );
			break; 
		}

	}while(TRUE);


_return:
	if ( hDirectory != NULL )
	{
		ZwClose(hDirectory);
	}

	if( targetName.Buffer != NULL )
	{
		free( targetName.Buffer ); 
	}

	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 

	return ret;
}

DISK_NAME_MAP all_disk_dev_name[ 26 ]; 
LRESULT get_disk_dev_name()
{
	LRESULT ret; 
	INT32 ret_val = 0; 
	INT32 i; 
	WCHAR disk_sign[] = L"A:"; 
	WCHAR dev_name[ 64 ]; 

	for( i = 0; i < 26; i ++ )
	{
		all_disk_dev_name[ i ].sign[ 0 ] = 'A' + i; 
		all_disk_dev_name[ i ].sign[ 1 ] = ':'; 
		all_disk_dev_name[ i ].sign[ 2 ] = '\0'; 

		ret = enum_symbolic_links( disk_sign, dev_name, 64 * sizeof( WCHAR ) ); 
		if( ret == ERROR_SUCCESS )
		{
			ret = unicode_to_mcbs( dev_name, all_disk_dev_name[ i ].dev_name, 64, NULL ); 
			if( ret < 0 )
			{
				ret_val = -1; 
				goto _return; 
			}
		}
		else
		{
			all_disk_dev_name[ i ].dev_name[ 0 ] = '\0'; 
		}

		disk_sign[ 0 ] ++; 
	}

_return:
	return ret_val ; 
}

CHAR *find_dev_name( CHAR *sign_name )
{
	INT32 i;  
	for( i = 0; i < 26; i ++ )
	{
		if( strnicmp( all_disk_dev_name[ i ].sign, sign_name, 2 ) == 0 )
		{
			return all_disk_dev_name[ i ].dev_name; 
		}
	}

	return NULL;  
}

#define CURRENT_USER_PREFIX1 L"\\REGISTRY\\USER\\S"
#define CURRENT_USER_PREFIX2 L"HKU\\S"
#define SHORT_NAME_CURRENT_USER L"HKEY_CURRENT_USER"

root_key current_user[] = {
	{ CURRENT_USER_PREFIX1, SHORT_NAME_CURRENT_USER, CONST_STR_LEN( CURRENT_USER_PREFIX1 ), CONST_STR_LEN( SHORT_NAME_CURRENT_USER ) },
	{ CURRENT_USER_PREFIX2, SHORT_NAME_CURRENT_USER, CONST_STR_LEN( CURRENT_USER_PREFIX2 ), CONST_STR_LEN( SHORT_NAME_CURRENT_USER ) }
};

#define REG_KEY_ROOT_NAME L"\\REGISTRY"

#define ROOT_KEY_USER L"\\REGISTRY\\USER"
#define ROOT_KEY_CURRENT_CONFIG L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\HARDWARE PROFILES\\CURRENT"
#define ROOT_KEY_CLASSES L"\\REGISTRY\\MACHINE\\SOFTWARE\\CLASSES"
#define ROOT_KEY_LOCAL_MACHINE L"\\REGISTRY\\MACHINE"

#define SHORT_NAME_USER L"HKEY_USERS"
#define SHORT_NAME_CURRENT_CONFIG L"HKEY_CURRENT_CONFIG"
#define SHORT_NAME_CLASSES L"HKEY_CLASSES_ROOT"
#define SHORT_NAME_LOCAL_MACHINE L"HKEY_LOCAL_MACHINE"

root_key root_keys[] = {
	{ ROOT_KEY_USER, SHORT_NAME_USER, CONST_STR_LEN( ROOT_KEY_USER ), CONST_STR_LEN( SHORT_NAME_USER ) } ,
	{ ROOT_KEY_CURRENT_CONFIG, SHORT_NAME_CURRENT_CONFIG, CONST_STR_LEN( ROOT_KEY_CURRENT_CONFIG ), CONST_STR_LEN( SHORT_NAME_CURRENT_CONFIG ) },
	{ ROOT_KEY_CLASSES, SHORT_NAME_CLASSES, CONST_STR_LEN( ROOT_KEY_CLASSES ), CONST_STR_LEN( SHORT_NAME_CLASSES ) }, 
	{ ROOT_KEY_LOCAL_MACHINE, SHORT_NAME_LOCAL_MACHINE, CONST_STR_LEN( ROOT_KEY_LOCAL_MACHINE ), CONST_STR_LEN( SHORT_NAME_LOCAL_MACHINE ) }
}; 

void ConvertToUpper( WCHAR *Dest, WCHAR *Source, ULONG Len )
{
	ULONG   i;

	for( i = 0; i < Len; i++ ) {
		if( Source[i] >= L'a' && Source[i] <= L'z' ) {

			Dest[i] = Source[i] - L'a' + L'A';

		} else {

			Dest[i] = Source[i];
		}
	}
}

NTSTATUS reg_path_short_format( LPWSTR reg_key_path, ULONG max_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 i; 
	WCHAR cmpname[ MAX_ROOT_NAME_LEN ]; 
	PWCHAR sub_key_path; 
	ULONG sub_path_len; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	//
	// See if it matches current user
	//
	for( i = 0; i < ARRAY_SIZE( current_user ); i++ ) 
	{
		ConvertToUpper( cmpname, reg_key_path, current_user[ i ].root_name_len ); 

		if( 0 == wcsncmp( cmpname, current_user[ i ].root_name,
			current_user[ i ].root_name_len ) ) 
		{

			log_trace( ( MSG_INFO, "found current user prefix ( %d ) %ws ==> %ws\n", i, 
				reg_key_path, current_user[ i ].root_name ) );

			//
			// Its current user. Process to next slash
			//

			sub_key_path = reg_key_path + current_user[ i ].root_name_len;
			while( *sub_key_path && *sub_key_path != '\\' ) 
			{
				sub_key_path++; 
			}

			sub_path_len = wcslen( sub_key_path ); 
			if( sub_path_len + current_user[ i ].short_name_len + 1 > max_len )
			{
				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				goto _return; 
			}

			memmove( reg_key_path + current_user[ i ].short_name_len, sub_key_path, ( sub_path_len + 1 ) * sizeof( WCHAR ) );
			memcpy( reg_key_path, current_user[ i ].short_name, current_user[ i ].short_name_len * sizeof( WCHAR ) ); 

			log_trace( ( MSG_INFO, "reconstructed the key path: %ws\n", reg_key_path ) ); 

			goto _return; 
		}
	}     

	//
	// Now, see if we can translate a root key name
	//
	for( i = 0; i < ARRAY_SIZE( root_keys ); i++ ) 
	{
		ConvertToUpper( cmpname, reg_key_path, root_keys[ i ].root_name_len ); 

		if( 0 == wcsncmp( cmpname, root_keys[ i ].root_name, 
			root_keys[ i ].root_name_len ) ) 
		{
			log_trace( ( MSG_INFO, "found root key (%d) %ws ==> %ws\n", i, reg_key_path, root_keys[ i ].root_name ) ); 

			sub_key_path = reg_key_path + root_keys[ i ].root_name_len; 
			sub_path_len = wcslen( sub_key_path ); 

			if( root_keys[ i ].short_name_len + sub_path_len + 1 > max_len )
			{
				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				goto _return; 
			}

			memmove( reg_key_path + root_keys[ i ].short_name_len, sub_key_path, ( sub_path_len + 1 ) * sizeof( WCHAR ) ); ;
			memcpy( reg_key_path, root_keys[ i ].short_name, root_keys[ i ].short_name_len * sizeof( WCHAR ) ); 

			log_trace( ( MSG_INFO, "reconstructed the key path: %ws\n", reg_key_path ) ); 
			goto _return;
		}
	}

	log_trace( ( MSG_WARNING, "not found the root key name in the key path %ws\n", reg_key_path ) ); 

_return: 
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__ ) ); 
	return ntstatus; 
}


NTSTATUS reg_path_full_format( LPWSTR reg_key_path, ULONG max_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 i; 
	WCHAR cmpname[ MAX_ROOT_NAME_LEN ]; 
	PWCHAR sub_key_path; 
	ULONG sub_path_len; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	//
	// See if it matches current user
	//
	for( i = 0; i < ARRAY_SIZE( current_user ); i++ ) 
	{
		ConvertToUpper( cmpname, reg_key_path, current_user[ i ].root_name_len ); 

		if( 0 == wcsncmp( cmpname, current_user[ i ].root_name,
			current_user[ i ].root_name_len ) ) 
		{

			log_trace( ( MSG_INFO, "found current user prefix ( %d ) %ws ==> %ws\n", i, 
				reg_key_path, current_user[ i ].root_name ) );

			//
			// Its current user. Process to next slash
			//

			sub_key_path = reg_key_path + current_user[ i ].root_name_len;
			while( *sub_key_path && *sub_key_path != '\\' ) 
			{
				sub_key_path++; 
			}

			sub_path_len = wcslen( sub_key_path ); 
			if( sub_path_len + current_user[ i ].short_name_len + 1 > max_len )
			{
				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				goto _return; 
			}

			memmove( reg_key_path + current_user[ i ].short_name_len, sub_key_path, ( sub_path_len + 1 ) * sizeof( WCHAR ) );
			memcpy( reg_key_path, current_user[ i ].short_name, current_user[ i ].short_name_len * sizeof( WCHAR ) ); 

			log_trace( ( MSG_INFO, "reconstructed the key path: %ws\n", reg_key_path ) ); 

			goto _return; 
		}
	}     

	//
	// Now, see if we can translate a root key name
	//
	for( i = 0; i < ARRAY_SIZE( root_keys ); i++ ) 
	{
		ConvertToUpper( cmpname, reg_key_path, root_keys[ i ].root_name_len ); 

		if( 0 == wcsncmp( cmpname, root_keys[ i ].root_name, 
			root_keys[ i ].root_name_len ) ) 
		{
			log_trace( ( MSG_INFO, "found root key (%d) %ws ==> %ws\n", i, reg_key_path, root_keys[ i ].root_name ) ); 

			sub_key_path = reg_key_path + root_keys[ i ].root_name_len; 
			sub_path_len = wcslen( sub_key_path ); 

			if( root_keys[ i ].short_name_len + sub_path_len + 1 > max_len )
			{
				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				goto _return; 
			}

			memmove( reg_key_path + root_keys[ i ].short_name_len, sub_key_path, ( sub_path_len + 1 ) * sizeof( WCHAR ) ); ;
			memcpy( reg_key_path, root_keys[ i ].short_name, root_keys[ i ].short_name_len * sizeof( WCHAR ) ); 

			log_trace( ( MSG_INFO, "reconstructed the key path: %ws\n", reg_key_path ) ); 
			goto _return;
		}
	}

	log_trace( ( MSG_WARNING, "not found the root key name in the key path %ws\n", reg_key_path ) ); 

_return: 
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__ ) ); 
	return ntstatus; 
}

typedef BOOL ( WINAPI* PtrConvertSidToStringSid )( PSID Sid, LPTSTR* StringSid ); 
LRESULT get_current_user_sid( LPTSTR sid, ULONG max_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hToken; 
	DWORD dwTemp = 0;
	char tagTokenInfoBuf[ 256 ] = {0};
	PTOKEN_USER tagTokenInfo = NULL; 
	PtrConvertSidToStringSid dwPtr = NULL; 
	LPTSTR _sid = NULL; 
	HMODULE mod_handle; 

	if( hProcess == NULL ) 
	{
		ret = GetLastError(); 
		goto _return; 
	}

	_ret = OpenProcessToken( hProcess, TOKEN_QUERY, &hToken ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	if( hToken == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	tagTokenInfo = ( PTOKEN_USER )tagTokenInfoBuf; 
	_ret = GetTokenInformation( hToken, 
		TokenUser, 
		tagTokenInfoBuf, 
		sizeof( tagTokenInfoBuf ), 
		&dwTemp ); 

	if( _ret == FALSE ) 
	{
		ret = GetLastError(); 
		goto _return; 
	}

	mod_handle = LoadLibrary( _T( "Advapi32.dll" ) ); 
	if( mod_handle == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	dwPtr = ( PtrConvertSidToStringSid )GetProcAddress( 
		mod_handle, "ConvertSidToStringSidA" ); 

	_sid = NULL;
	_ret = dwPtr( tagTokenInfo->User.Sid, ( LPTSTR* )&_sid );
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	wcsncpy( sid, _sid, max_len ); 
	if( sid[ max_len - 1 ] != _T( '\0' ) )
	{
		sid[ max_len - 1 ] = _T( '\0' ); 
	}

	LocalFree( (HLOCAL)_sid );

	//RtlFormatCurrentUserKeyPath( )
_return:
	if( hToken != NULL )
	{
		CloseHandle( hToken );
	}

	if( hProcess != NULL )
	{
		CloseHandle( hProcess );
	}

	return ret;
}

BOOL GetLogonSID(HANDLE hToken, PSID *ppsid) 
{
	BOOL bSuccess = FALSE;
	DWORD dwIndex;
	DWORD dwLength = 0;
	PTOKEN_GROUPS ptg = NULL;

	// Verify the parameter passed in is not NULL.
	if (NULL == ppsid)
		goto Cleanup;

	// Get required buffer size and allocate the TOKEN_GROUPS buffer.

	if (!GetTokenInformation(
		hToken,         // handle to the access token
		TokenGroups,    // get information about the token's groups 
		(LPVOID) ptg,   // pointer to TOKEN_GROUPS buffer
		0,              // size of buffer
		&dwLength       // receives required buffer size
		)) 
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
			goto Cleanup;

		ptg = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY, dwLength);

		if (ptg == NULL)
			goto Cleanup;
	}

	// Get the token group information from the access token.

	if (!GetTokenInformation(
		hToken,         // handle to the access token
		TokenGroups,    // get information about the token's groups 
		(LPVOID) ptg,   // pointer to TOKEN_GROUPS buffer
		dwLength,       // size of buffer
		&dwLength       // receives required buffer size
		)) 
	{
		goto Cleanup;
	}

	// Loop through the groups to find the logon SID.

	for (dwIndex = 0; dwIndex < ptg->GroupCount; dwIndex++) 
		if ((ptg->Groups[dwIndex].Attributes & SE_GROUP_LOGON_ID)
			==  SE_GROUP_LOGON_ID) 
		{
			// Found the logon SID; make a copy of it.

			dwLength = GetLengthSid(ptg->Groups[dwIndex].Sid);
			*ppsid = (PSID) HeapAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, dwLength);
			if (*ppsid == NULL)
				goto Cleanup;
			if (!CopySid(dwLength, *ppsid, ptg->Groups[dwIndex].Sid)) 
			{
				HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
				goto Cleanup;
			}
			break;
		}

		bSuccess = TRUE;

Cleanup: 

		// Free the buffer for the token groups.

		if (ptg != NULL)
			HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);

		return bSuccess;
}

VOID FreeLogonSID (PSID *ppsid) 
{
	HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}

LRESULT get_file_version( LPCTSTR file_name, LARGE_INTEGER *version)
{   
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	DWORD version_handle;
	UINT cb; 
	BYTE* version_buf = NULL;
	VS_FIXEDFILEINFO* fixed_file_info = NULL;

	do 
	{
		if( file_name == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		cb = GetFileVersionInfoSize( file_name, &version_handle );
		if( cb > 0 )
		{
			version_buf = ( BYTE* )malloc( cb ); 

			if( version_buf == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}		

			if( GetFileVersionInfo( file_name, 0, cb, version_buf ) )
			{
				_ret = VerQueryValue( version_buf, TEXT( "\\" ), ( VOID** )&fixed_file_info, &cb ); 
				if( _ret == FALSE )
				{
					SAFE_SET_ERROR_CODE( ret ); 
					break; 
				}

				if( fixed_file_info != NULL ) 
				{
					version->HighPart = fixed_file_info->dwFileVersionMS;
					version->LowPart  = fixed_file_info->dwFileVersionLS;
					break; 
				}
			}
		}
	}while( FALSE );

	if( NULL != version_buf )
	{
		free( version_buf ); 
	}

	return ret;
}


LARGE_INTEGER ParseVersion( LPCTSTR szVer )
{
	LARGE_INTEGER ll;	
	DWORD d1 = 0, d2 = 0, d3 = 0, d4 = 0;
	_stscanf( szVer, _T("%u.%u.%u.%u"), &d1, &d2, &d3, &d4 );

	ll.HighPart = MAKELONG( d2, d1 );
	ll.LowPart = MAKELONG( d4, d3 );
	return ll;
}

//void GenVersionStr( LARGE_INTEGER ll, CString &str )
//{
//	str.Format(_T("%u.%u.%u.%u"), HIWORD(ll.HighPart), LOWORD(ll.HighPart), HIWORD(ll.LowPart), LOWORD(ll.LowPart) );
//}

//BOOL VersionInRange(LARGE_INTEGER lVersion, LPCTSTR lpszLowerVersion, LPCTSTR lpszUpperVersion )
//{
//	BOOL bResult = FALSE;
//	if( _tcslen(lpszLowerVersion)>0 
//		&& !(CompareVersion(lVersion, ParseVersion(lpszLowerVersion), _T(">="), bResult ) && bResult) )
//		return FALSE;
//
//	if( _tcslen(lpszUpperVersion)>0 
//		&& !(CompareVersion( lVersion, ParseVersion(lpszUpperVersion), _T("<"), bResult ) && bResult) )
//		return FALSE;
//	return TRUE;
//}


BOOL IsVersionString( LPCTSTR szVer )
{
	unsigned int d1 = 0, d2 = 0, d3 = 0, d4 = 0;
	return 4==_stscanf( szVer, _T("%u.%u.%u.%u"), &d1, &d2, &d3, &d4 );
}

//BOOL EvalCompareResult( INT nRet, LPCTSTR sOpt, BOOL &bResult )
//{
//	if( _tcsicmp(sOpt, _T("<"))==0 )
//		bResult = nRet < 0;
//	else if( _tcsicmp(sOpt, _T("<="))==0 )
//		bResult = nRet <= 0;
//	else if( _tcsicmp(sOpt, _T(">"))==0)
//		bResult = nRet > 0;
//	else if( _tcsicmp(sOpt, _T(">="))==0 )
//		bResult = nRet >= 0;
//	else if( _tcsicmp(sOpt, _T("="))==0 || _tcsicmp(sOpt, _T("=="))==0 )
//		bResult = nRet == 0;
//	else if( _tcsicmp(sOpt, _T("!="))==0 )
//		bResult = nRet != 0;
//	else
//		return FALSE;
//
//	return TRUE;
//}

//BOOL CompareString( LPCTSTR lpLeft, LPCTSTR lpRight, LPCTSTR sOpt, BOOL& bResult ) 
//{
//	INT nRet = 0;
//	if(IsVersionString(lpLeft) && IsVersionString(lpRight))
//	{
//		LARGE_INTEGER llLeft, llRight;
//		llLeft = ParseVersion(lpLeft);
//		llRight = ParseVersion(lpRight);
//
//		nRet = CompareLargeInts(llLeft, llRight);
//	}
//	else
//	{
//		nRet = _tcsicmp(lpLeft, lpRight);
//	}
//
//	return EvalCompareResult(nRet, sOpt, bResult);
//}

//BOOL CompareVersion( const LARGE_INTEGER &v1, const LARGE_INTEGER &v2, LPCTSTR sOpt, BOOL &bResult )
//{
//	INT nRet = CompareLargeInts(v1, v2);
//	return EvalCompareResult(nRet, sOpt, bResult);
//}
//
//void GetDateString( CString &strDate )
//{
//	SYSTEMTIME st;
//	GetLocalTime(&st);
//	strDate.Format(_T("%04d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
//}

//BOOL ParseDateString( LPCTSTR szDate, INT &year, INT &month, INT &day )
//{
//	return _stscanf( szDate, _T("%u-%u-%u"), &year, &month, &day )==3;
//}

//BOOL MyMoveFileA(LPCSTR lpszFileFrom, LPCSTR lpszFileto)
//{
//	if( !PathFileExistsA(lpszFileFrom) )
//		return FALSE;
//	if( PathFileExistsA(lpszFileto) )
//		SetFileAttributesA(lpszFileto, FILE_ATTRIBUTE_NORMAL);
//
//	DWORD flags = GetFileAttributesA( lpszFileFrom );
//	SetFileAttributesA(lpszFileFrom, FILE_ATTRIBUTE_NORMAL);
//	BOOL bRet = MoveFileExA(lpszFileFrom, lpszFileto, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED);
//	if(bRet)
//		SetFileAttributesA(lpszFileto, flags);
//	return bRet;
//}

//BOOL IsFileUsing(LPCTSTR szFilename)
//{
//	DWORD flags = GetFileAttributes( szFilename );
//	SetFileAttributes(szFilename, FILE_ATTRIBUTE_NORMAL);
//
//	HANDLE hfile = CreateFile(szFilename, GENERIC_ALL, 0, 0, OPEN_EXISTING, 0, 0); 
//	BOOL bRet = TRUE;
//	if(hfile!=INVALID_HANDLE_VALUE)
//	{
//		CloseHandle( hfile );
//		bRet = FALSE;
//	}	
//	SetFileAttributes(szFilename, flags);
//	return bRet;
//}

//CString &CompletePathWithModulePath(CString &strPath, LPCTSTR lpszFilename)
//{
//	TCHAR szPath[MAX_PATH] = {0};
//	::GetModuleFileName( (HMODULE)&__ImageBase, szPath, MAX_PATH);
//	PathAppend(szPath, _T("..\\"));
//	PathAppend(szPath, lpszFilename);
//	strPath = szPath;
//	return strPath;
//}

LRESULT create_share_mem( DWORD size, LPCTSTR name, HANDLE *mem_handle, PVOID *mem_addr )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE share_mem = NULL; 
	PVOID share_mem_addr = NULL; 

	ASSERT( mem_handle != NULL ); 
	ASSERT( mem_addr != NULL ); 

	*mem_handle = NULL; 
	*mem_addr = NULL; 

	share_mem = CreateFileMapping( INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, size, name );
	if( share_mem == NULL )
	{
		if( GetLastError() == ERROR_INVALID_HANDLE )
		{
			goto _return; 
		}

		ret = GetLastError(); 
		goto _return; 
	}

	share_mem_addr = MapViewOfFile( share_mem, FILE_MAP_ALL_ACCESS, 0 , 0, 0 ); 
	if( share_mem_addr == NULL )
	{

		ret = GetLastError(); 
		goto _return; 
	}

	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
	}

	ZeroMemory( share_mem_addr, size );  

	*mem_handle = share_mem; 
	*mem_addr = share_mem_addr; 

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( share_mem_addr != NULL )
		{
			UnmapViewOfFile( share_mem_addr );	
		}

		if( share_mem != NULL )
		{
			CloseHandle( share_mem );
		}

		ASSERT( *mem_handle == NULL ); 
		ASSERT( *mem_addr == NULL ); 
	}
	else
	{

		ASSERT( *mem_handle != NULL ); 
		ASSERT( *mem_addr != NULL ); 
	}
	return ret;
}

LRESULT open_share_mem( LPCTSTR name, HANDLE *mem_handle, PVOID *mem_addr )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE share_mem = NULL; 
	PVOID share_mem_addr = NULL; 

	ASSERT( mem_handle != NULL ); 
	ASSERT( mem_addr != NULL ); 

	*mem_handle = NULL; 
	*mem_addr = NULL; 

	share_mem = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, name ); 
	if(share_mem == NULL)
	{	
		ret = GetLastError(); 
		goto _return; 
	}

	share_mem_addr = MapViewOfFile( share_mem, FILE_MAP_ALL_ACCESS, 0, 0, 0 ); 
	if( share_mem_addr == NULL)
	{	
		ret = GetLastError(); 
		goto _return; 
	}

	*mem_handle = share_mem; 
	*mem_addr = share_mem_addr; 

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( share_mem_addr != NULL )
		{
			UnmapViewOfFile( share_mem_addr );	
		}

		if( share_mem != NULL )
		{
			CloseHandle( share_mem );
		}

		ASSERT( *mem_handle == NULL ); 
		ASSERT( *mem_addr == NULL ); 
	}
	else
	{

		ASSERT( *mem_handle != NULL ); 
		ASSERT( *mem_addr != NULL ); 
	}

	return ret;
}

VOID close_share_mem( HANDLE mem_handle, PVOID mem_addr )
{
	ASSERT( mem_handle != NULL ); 
	ASSERT( mem_addr != NULL ); 

	UnmapViewOfFile( mem_addr );	
	CloseHandle( mem_handle );
}

LRESULT del_tmp_file_batch()
{
	LRESULT ret = ERROR_SUCCESS; 
#define DEL_TMP_FILE_CMDS "del *.~*\r\n del *.dcu\r\n Del *.ddp \r\n"
	return ret; 
}

BOOL is_xp_last()
{
	OSVERSIONINFOEX v = { 0 };

	v.dwOSVersionInfoSize = sizeof (v);
	if ( GetVersionEx((LPOSVERSIONINFO)&v) )
	{
		if ( v.dwMajorVersion >= 5 && v.dwMinorVersion != 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL is_win64()
{
	BOOL bResult = FALSE;
	BOOL (WINAPI *pfnIsWow64Process)(HANDLE hProcess, BOOL* bWindows);
	BOOL (WINAPI* pfnGetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);

	HMODULE hDll = GetModuleHandle(_T("kernel32.dll"));
	if ( hDll != NULL )
	{
		( FARPROC )pfnIsWow64Process = GetProcAddress(hDll, "IsWow64Process");
		if ( pfnIsWow64Process != NULL )
		{
			BOOL bWoW64;

			if ( pfnIsWow64Process(GetCurrentProcess(), &bWoW64) && bWoW64 )
			{
				( FARPROC )pfnGetNativeSystemInfo = GetProcAddress(hDll, "GetNativeSystemInfo");
				if ( pfnGetNativeSystemInfo != NULL )
				{
					SYSTEM_INFO si = { 0 };

					if ( pfnGetNativeSystemInfo(&si) )
					{
						bResult = (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
							si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64);
					}
				}
			}
		}
	}

	return bResult;
}

BOOL is_downloaded_file( LPCTSTR lpPath )
{
/*
// �����������ļ�ֵ��IE ���ص��ļ���û�а�ȫ����
[HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\Attachments]
"SaveZoneInformation"=dword:00000001
	*/
	
	BOOL bResult = FALSE;
	TCHAR szStreamPath[MAX_PATH];
	HANDLE hFile; 

	/*StringCchPrintf*/_sntprintf( szStreamPath, MAX_PATH, _T("%s:Zone.Identifier"), lpPath );
	
	hFile = CreateFile(szStreamPath,
		GENERIC_READ,
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING,
		0,
		NULL
		);
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		WCHAR cBuff[100];
		DWORD dwReadBytes = 0;
		LPWSTR lp; 
		int nZoneId; 

		ReadFile(hFile, cBuff, sizeof (cBuff), &dwReadBytes, NULL);
		cBuff[dwReadBytes / sizeof (WCHAR)] = 0;

		lp = wcsstr(cBuff, L"[ZoneTransfer]");
		if ( lp != NULL )
		{
			lp = (LPWSTR)((PBYTE)lp + sizeof (L"[ZoneTransfer]") - 2);
			lp = wcsstr(lp, L"ZoneId=");
			if ( lp != NULL )
			{
				lp = (LPWSTR)((PBYTE)lp + sizeof (L"ZoneId=") - 2);

				nZoneId = _wtoi(lp);
				if ( nZoneId == URLZONE_INTERNET )
				{
					bResult = TRUE;
				}
			}
		}
		
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return bResult;
}

INT32 WINAPI is_vista_or_later()
{
	INT32 ret = FALSE; 
	INT32 _ret; 
	OSVERSIONINFO osInfo;

	do 
	{
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		_ret = GetVersionEx(&osInfo);
		if( _ret == FALSE )
		{
			ASSERT( FALSE ); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "get windows version by GetVersionEx error" ) ); 
			break; 
		}

		ret = osInfo.dwMajorVersion >= 6; 
	}while( FALSE );

	return ret;
}

INT32 WINAPI check_uac_opened()
{
	BOOL bResult = FALSE;

	if( is_vista_or_later() )
	{
		LONG	lResult;
		HKEY	hKey;
		DWORD	dwEnableLUA;
		DWORD	dwType = REG_DWORD;
		DWORD	dwSize = sizeof( DWORD );

		lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\"),
			0,
			KEY_READ,
			&hKey
			);
		if( lResult == ERROR_SUCCESS )
		{
			lResult = RegQueryValueEx(hKey,
				_T("EnableLUA"),
				NULL,
				&dwType,
				(BYTE*)&dwEnableLUA,
				&dwSize
				);
			bResult = (lResult == ERROR_SUCCESS) && (0 != dwEnableLUA);

			RegCloseKey(hKey);
		}
	}

	return bResult;
}

#include <shlobj.h>

LRESULT get_user_path( LPWSTR file_name, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	_ret = SHGetSpecialFolderPath( NULL, file_name, CSIDL_APPDATA, FALSE ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT get_desktop_path( LPWSTR file_name, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	_ret = SHGetSpecialFolderPath( NULL, file_name, CSIDL_DESKTOPDIRECTORY, FALSE ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	return ret; 
}

typedef VOID ( WINAPI *LPFN_GetNativeSystemInfo )( LPSYSTEM_INFO lpSystemInfo ); 

LRESULT get_sys_info( __out LPSYSTEM_INFO sys_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	HMODULE mod_handle; 

	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo; 

	if( NULL == sys_info )
	{
		ASSERT( FALSE && "why input the null parameter" ); 

		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	mod_handle = GetModuleHandle( _T( "kernel32" ) ); 
	if( mod_handle == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	fnGetNativeSystemInfo = ( LPFN_GetNativeSystemInfo )GetProcAddress( mod_handle, "GetNativeSystemInfo" );;

	if( NULL != fnGetNativeSystemInfo )
	{
		fnGetNativeSystemInfo( sys_info );
	}
	else
	{
		GetSystemInfo( sys_info );
	}

_return:
	return ret; 
}

typedef BOOL ( WINAPI *LPFN_ISWOW64PROCESS )( HANDLE, PBOOL );

LRESULT is_wow64_proc( INT32 *is_wow64 )
{
	BOOL _is_wow64;
	LRESULT ret = ERROR_SUCCESS; 
	HMODULE mod_handle; 
	LPFN_ISWOW64PROCESS fnIsWow64Process; 

	ASSERT( is_wow64 != NULL ); 

	*is_wow64 = FALSE; 

	mod_handle = GetModuleHandle( _T( "kernel32" ) ); 
	if( mod_handle == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	fnIsWow64Process = ( LPFN_ISWOW64PROCESS )GetProcAddress(
		mod_handle, "IsWow64Process" ); 


	if( NULL != fnIsWow64Process )
	{
		if( !fnIsWow64Process( GetCurrentProcess(), &_is_wow64 ) )
		{
			log_trace( ( MSG_ERROR, "check the wow64 information failed\n" ) ); 
			ret = GetLastError(); 
			goto _return; 
		}
	}

	*is_wow64 = _is_wow64; 

_return:
	return ret;
}

//BOOL IsWow64Process()    
//{       
//	typedef BOOL (WINAPI *LPFN_IsWow64Process) (HANDLE, PBOOL);
//	LPFN_IsWow64Process fnIsWow64Process;    
//	BOOL bIsWow64Process = FALSE;    
//	fnIsWow64Process = (LPFN_IsWow64Process)GetProcAddress( GetModuleHandleA("kernel32.dll"),"IsWow64Process");    
//	if (NULL != fnIsWow64Process)    
//	{    
//		fnIsWow64Process(GetCurrentProcess(),&bIsWow64Process); 
//	}    
//	return bIsWow64Process;    
//}

LRESULT WINAPI disable_wow_64( LPVOID* dwOldValue )
{
	LRESULT ret = ERROR_NOT_SUPPORTED;
	typedef BOOL (WINAPI *LPFN_WOW64DISABLEWOW64FSREDIRECTION)(LPVOID *);
	LPFN_WOW64DISABLEWOW64FSREDIRECTION _disable_wow_64 = NULL;
	
	do 
	{
		_disable_wow_64 = (LPFN_WOW64DISABLEWOW64FSREDIRECTION) GetProcAddress (GetModuleHandleW (L"kernel32"),"Wow64DisableWow64FsRedirection");
		if( _disable_wow_64 == NULL)
		{
			break; 
		}
		else
		{
			if( !_disable_wow_64( dwOldValue ))
			{
				break;
			}
		}


		ret = ERROR_SUCCESS; 
	}while( FALSE );


	return ret;
}

LRESULT WINAPI revert_wow_64( LPVOID dwOldValue )
{
	LRESULT ret = ERROR_NOT_SUPPORTED; 
	typedef BOOL (WINAPI *LPFN_WOW64REVERTWOW64FSREDIRECTIIO)(LPVOID);
	LPFN_WOW64REVERTWOW64FSREDIRECTIIO _revert_wow_64 = NULL;

	do 
	{
		_revert_wow_64 = (LPFN_WOW64REVERTWOW64FSREDIRECTIIO )GetProcAddress( GetModuleHandleW (L"kernel32"),"Wow64RevertWow64FsRedirection");
		if( _revert_wow_64 == NULL )
		{
			break;
		}
		else
		{
			if( !_revert_wow_64( dwOldValue ) )
			{
				break; 
			}
		}

		ret = ERROR_SUCCESS;
	}while( FALSE );

	return ret;
}

LRESULT get_sys_bits( INT32 *os_bit_out, INT32 *wow64_out )
{
	SYSTEM_INFO si;
	LRESULT ret = ERROR_SUCCESS; 
	INT32 os_bit; 
	INT32 wow64; 

	ASSERT( os_bit_out != NULL ); 

	*os_bit_out = 0; 
	*wow64_out = FALSE; 

	is_wow64_proc( &wow64 ); 

	if( wow64 == TRUE )
	{
		*os_bit_out = 64; 
		*wow64_out = TRUE; 
		goto _return; 
	}

	ret = get_sys_info( &si ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
	{
		os_bit = 64; 
	}
	else
	{
		os_bit = 32; 
	}

	*os_bit_out = os_bit; 
	*wow64_out = wow64; 

_return:
	return ret; 
}

LRESULT get_win_ver_info( OSVERSIONINFOEX *osvi )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	ASSERT( osvi != NULL ); 

	do 
	{
		ZeroMemory( osvi, sizeof( OSVERSIONINFOEX ) );
		osvi->dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );

		_ret = GetVersionEx( ( OSVERSIONINFO* )osvi ); 
		if( _ret == TRUE )
		{
			break; 
		}
		else
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "get windows version error" ) ); 
		}

		osvi->dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		_ret = GetVersionEx ( ( OSVERSIONINFO* )osvi );
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "get windows version error" ) ); 
			break; 
		}
	}while( FALSE );

_return:
	return ret; 
}

LRESULT fix_win_ver_info( OSVERSIONINFOEX *osvi )
{
	LRESULT ret = ERROR_SUCCESS; 
	ASSERT( osvi != NULL ); 

	//LPCTSTR strKernelFile = _T("@SYSTEM");
	//if( ParseSpecialFolder( strKernelFile ) )
	//{
	//	strKernelFile += _T("\\ntoskrnl.exe");
	//	LARGE_INTEGER ll;
	//	if( PathFileExists(strKernelFile) && GetFileVersion(strKernelFile, ll) )
	//	{
	//		DWORD dwMajor = HIWORD(ll.HighPart);
	//		DWORD dwMinor = LOWORD(ll.HighPart);

	//		if( osvi.dwMajorVersion<dwMajor
	//			|| ( osvi.dwMajorVersion==dwMajor && osvi.dwMinorVersion<dwMinor) )
	//		{
	//			osvi.dwPlatformId = VER_PLATFORM_WIN32_NT;
	//			osvi.dwMajorVersion = dwMajor;
	//			osvi.dwMinorVersion = dwMinor;

	//			// OR use ntoskrnl.exe
	//			// MODIFY SP  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Windows@CSDVersion 
	//			DWORD dwCSDVersion = 0;
	//			ReadRegDWord(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Windows"), _T("CSDVersion"), dwCSDVersion);
	//			osvi.wServicePackMajor = (WORD)(dwCSDVersion >> 8);
	//			return TRUE;
	//		}
	//	}
	//}
	return ret;
}

int parse_win_ver( OSVERSIONINFOEX *osvi )
{
	/*
	5.0 	win2k 
	5.1 	winxp 
	5.2 	win2003 
	6.0		vista 
	6.1 	win7 
	*/

	int wVersion = WINUNKNOWN;

	ASSERT( osvi != NULL ); 

	switch (osvi->dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		wVersion = WIN32S;
		break;

	case VER_PLATFORM_WIN32_WINDOWS:
		if ( osvi->dwMinorVersion == 0 )
			wVersion = WIN95;	
		else if ( osvi->dwMinorVersion == 10)
			wVersion = WIN98;
		else if ( osvi->dwMinorVersion == 90)
			wVersion = WINME;
		break;

	case VER_PLATFORM_WIN32_NT:
		if ( osvi->dwMajorVersion <= 4 )
			wVersion = WINNT;
		else if ( osvi->dwMajorVersion == 5 && osvi->dwMinorVersion == 0 )
			wVersion = WIN2K;
		else if ( osvi->dwMajorVersion == 5 && osvi->dwMinorVersion == 1 )
			wVersion = WINXP;
		else if ( osvi->dwMajorVersion == 5 && osvi->dwMinorVersion == 2 )
			wVersion = WIN2003;
		else if ( osvi->dwMajorVersion == 6 )
		{
			if(osvi->dwMinorVersion==0)
			{
				wVersion = osvi->wProductType == VER_NT_WORKSTATION ? WINVISTA : WIN2008;
			}
			else if(osvi->dwMinorVersion==1)
			{
				wVersion = osvi->wProductType == VER_NT_WORKSTATION ? WIN7 : WIN2008R2;
			}
		}
		break;

	default:
		break;
	}
	return wVersion;
}

LRESULT get_win_ver( INT32 *nWindowsVersion, INT32 *nSP )
{
	LRESULT ret = ERROR_SUCCESS; 
	OSVERSIONINFOEX osvi; 

	ASSERT( nWindowsVersion != NULL ); 
	ASSERT( nSP != NULL ); 

	*nWindowsVersion = WINUNKNOWN;
	*nSP = 0;

	ZeroMemory( &osvi, sizeof( osvi ) );
	ret = get_win_ver_info( &osvi ); 

	if( ret == ERROR_SUCCESS )
	{
		fix_win_ver_info( &osvi );
		*nWindowsVersion = parse_win_ver( &osvi );
		*nSP = ( INT32 )osvi.wServicePackMajor;
		goto _return; 
	}

	ret = ERROR_ERRORS_ENCOUNTERED; 

_return:
	return TRUE;
}

/**
* @retval   >0  current OS is greater than compared version
* @retval   <0  current OS is less than compared version
* @retval   0   current OS is equal to compared version
*/
INT32 compare_win_ver(DWORD dwMajorVer, DWORD dwMinorVer)
{
	INT32 ret = 0; 
	OSVERSIONINFOEX osvi; 

	ZeroMemory( &osvi, sizeof( osvi ) );
	ret = get_win_ver_info( &osvi ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( osvi.dwMajorVersion > dwMajorVer)
	{
		ret = 1; 
		goto _return; 
	}
	else if( osvi.dwMajorVersion < dwMajorVer)
	{
		ret = -1; 
		goto _return; 
	}

	ret = osvi.dwMinorVersion - dwMinorVer;

_return:
	return ret; 
}

BOOL is_vista_or_later_ex()
{
	return 0 <= compare_win_ver( em_OS_MajorVer_Vista, 0 );
}

LRESULT create_global_name( LPCWSTR name, HANDLE *name_handle )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE test_sem = NULL; 

	ASSERT( name != NULL ); 
	ASSERT( name_handle != NULL ); 

	*name_handle = NULL; 
	test_sem = CreateSemaphore( NULL, 1, 1, name ); 

	if( NULL == test_sem )
	{
		SAFE_SET_ERROR_CODE( ret ); 
		goto _return; 
	}
	else
	{
		ret = GetLastError(); 
		if( ret == ERROR_ALREADY_EXISTS )
		{
			ret = ERROR_SUCCESS; 
			//CloseHandle( test_sem ); 
			//goto _return; 
		}
		
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "create semaphore have special error code" ); 
			ret = ERROR_SUCCESS; 
		}
		else
		{
			BOOL _ret; 
			_ret = ReleaseSemaphore( test_sem, 1, NULL ); 
			if( FALSE == _ret )
			{
				//ret = GetLastError(); 

				log_trace( ( DBG_MSG_AND_ERROR_OUT, "release the ui log tip error" ) ); 
				//DBG_BP(); 
				//SAFE_SET_ERROR_CODE( ret ); 
				//goto _return; 
			}

			*name_handle = test_sem; 
		}
	}

_return:
	return ret; 
}

LRESULT check_global_name_exist( LPCWSTR name, HANDLE *name_handle )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE test_sem; 

	ASSERT( name != NULL ); 

	test_sem = OpenSemaphore( SEMAPHORE_ALL_ACCESS, TRUE, name ); 
	//test_sem = CreateSemaphore( NULL, 1, 1, name ); 

	if( name_handle != NULL )
	{
		*name_handle = NULL; 
	}

	if( test_sem == NULL )
	{
		ret = GetLastError(); 

		ASSERT( ret != ERROR_SUCCESS ); 

		if( ret != ERROR_FILE_NOT_FOUND 
			&& ret != ERROR_NOT_FOUND )
		{
			log_trace( ( MSG_ERROR, "open semaphore return error is not \"not found \" %u \n", ret ) ); 
			DBG_BP(); 
		}

		goto _return; 
	}

	if( name_handle != NULL )
	{
		*name_handle = test_sem; 
	}
	else
	{
		CloseHandle( test_sem ); 
	}

_return:
	return ret;
}

LRESULT close_global_name( LPCWSTR name, HANDLE name_handle )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	ASSERT( name_handle != NULL ); 

	UNREFERENCED_PARAMETER( name ); 

	log_trace( ( MSG_INFO, "close global name %ws handle 0x%0.8x\n", name, name_handle ) ); 

	_ret = CloseHandle( name_handle ); 

	if( FALSE  == _ret )
	{
		ret = GetLastError(); 
		//goto _return; 
	}

	ASSERT( ERROR_SUCCESS != check_global_name_exist( name, NULL ) ); 

//_return:
	return ret; 
}

LRESULT two_interact_process_protect( LPCWSTR exe_name1, LPCWSTR exe_name2 )
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}

LRESULT find_whole_path( LPCTSTR file_path, file_found_callback on_file_found, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	HANDLE find_file; 
	TCHAR find_path[ MAX_PATH ]; 
	WIN32_FIND_DATA find_data; 

	ASSERT( on_file_found != NULL ); 

	_tcsncpy( find_path, file_path, MAX_PATH ); 
	if( find_path[ MAX_PATH - 1 ] != _T( '\0' ) )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	_tcsncat( find_path, _T( "\\*" ), MAX_PATH - _tcslen( find_path ) ); 
	if( find_path[ MAX_PATH - 1 ] != _T( '\0' ) )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	find_file = FindFirstFile( find_path, &find_data ); 
	if( INVALID_HANDLE_VALUE == find_file )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	for( ; ; )
	{
		on_file_found( file_path, &find_data, context ); 

		_ret = FindNextFile( find_file, &find_data ); 
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			if( ret == ERROR_NO_MORE_FILES )
			{
				ret = ERROR_SUCCESS; 
			}
			break; 
		}
	}

	FindClose( find_file ); 

_return:
	return ret; 
}

#if 0
void dos_to_dev_name( LPCWSTR dos_name, PUNICODE_STRING dev_name )
{
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING _dos_name;
	HANDLE  link_handle;
	ULONG dev_name_len;

	RtlInitUnicodeString( &_dos_name, dos_name ); 

	InitializeObjectAttributes( &oa, 
		&_dos_name, 
		OBJ_CASE_INSENSITIVE, 
		NULL, 
		NULL ); 

	ZwOpenSymbolicLinkObject( &link_handle, GENERIC_READ, &oa );

	ZwQuerySymbolicLinkObject( &link_handle, dev_name, &dev_name_len );

	DbgPrint( "this symbolic link is %wZ",dev_name ); 
}
#endif //0

//typedef ULONG NTSTATUS;
//typedef struct _UNICODE_STRING { 
//	USHORT  Length; 
//	USHORT  MaximumLength; 
//	PWSTR  Buffer; 
//} UNICODE_STRING, *PUNICODE_STRING; 

//typedef struct _OBJECT_ATTRIBUTES { 
//	ULONG Length; 
//	HANDLE RootDirectory; 
//	UNICODE_STRING *ObjectName; 
//	ULONG Attributes; 
//	PSECURITY_DESCRIPTOR SecurityDescriptor; 
//	PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService; 
//} OBJECT_ATTRIBUTES,*POBJECT_ATTRIBUTES;

//typedef struct _DIRECTORY_BASIC_INFORMATION { 
//	UNICODE_STRING ObjectName; 
//	UNICODE_STRING ObjectTypeName; 
//} DIRECTORY_BASIC_INFORMATION, *PDIRECTORY_BASIC_INFORMATION;

////////////////////////////////////////////////////////////////////////// 
#define OBJ_CASE_INSENSITIVE    0x00000040L 
#define DIRECTORY_QUERY            (0x0001) 
#define STATUS_SUCCESS            ((NTSTATUS)0x00000000L) // ntsubauth 
#define STATUS_MORE_ENTRIES        ((NTSTATUS)0x00000105L) 
#define STATUS_BUFFER_TOO_SMALL    ((NTSTATUS)0xC0000023L)
#define InitializeObjectAttributes( p, n, a, r, s ) { \
	(p)->Length = sizeof( OBJECT_ATTRIBUTES ); \
	(p)->RootDirectory = r; \
	(p)->Attributes = a; \
	(p)->ObjectName = n; \
	(p)->SecurityDescriptor = s; \
	(p)->SecurityQualityOfService = NULL; \
}

////////////////////////////////////////////////////////////////////////// 
typedef VOID (CALLBACK* RTLINITUNICODESTRING)(PUNICODE_STRING,PCWSTR); 
RTLINITUNICODESTRING RtlInitUnicodeString;
typedef NTSTATUS (WINAPI *ZWOPENDIRECTORYOBJECT)( 
	OUT PHANDLE DirectoryHandle, 
	IN ACCESS_MASK DesiredAccess, 
	IN POBJECT_ATTRIBUTES ObjectAttributes 
	); 
ZWOPENDIRECTORYOBJECT ZwOpenDirectoryObject;
typedef 
NTSTATUS 
(WINAPI *ZWQUERYDIRECTORYOBJECT)( 
								 IN HANDLE DirectoryHandle, 
								 OUT PVOID Buffer, 
								 IN ULONG BufferLength, 
								 IN BOOLEAN ReturnSingleEntry, 
								 IN BOOLEAN RestartScan, 
								 IN OUT PULONG Context, 
								 OUT PULONG ReturnLength OPTIONAL 
								 ); 
ZWQUERYDIRECTORYOBJECT ZwQueryDirectoryObject;
typedef 
NTSTATUS 
(WINAPI *ZWCLOSE)( 
				  IN HANDLE Handle 
				  ); 
ZWCLOSE ZwClose;
////////////////////////////////////////////////////////////////////////// 
int traverse_all_driver(void) 
{ 
	HMODULE hNtdll; 
	UNICODE_STRING      usDirPath; 
	OBJECT_ATTRIBUTES   oa; 
	NTSTATUS            ntStatus; 
	HANDLE                hDir; 
	PDIRECTORY_BASIC_INFORMATION pBuffer=NULL,pBuffer2; 
	ULONG               uLength = 0x800; 
	ULONG                context,dwRet; 
	hNtdll = LoadLibraryA( "ntdll.dll" ); 
	RtlInitUnicodeString = (RTLINITUNICODESTRING)GetProcAddress( hNtdll, "RtlInitUnicodeString"); 
	ZwOpenDirectoryObject = (ZWOPENDIRECTORYOBJECT)GetProcAddress( hNtdll, "ZwOpenDirectoryObject"); 
	ZwQueryDirectoryObject = (ZWQUERYDIRECTORYOBJECT)GetProcAddress( hNtdll, "ZwQueryDirectoryObject"); 
	ZwClose = (ZWCLOSE)GetProcAddress( hNtdll, "ZwClose"); 

	//��Ŀ¼���� 
	RtlInitUnicodeString(&usDirPath, L"\\Driver"); 
	InitializeObjectAttributes(&oa,&usDirPath,OBJ_CASE_INSENSITIVE,NULL,NULL); 
	printf("ZwOpenDirectoryObject ..."); 
	ntStatus=ZwOpenDirectoryObject(&hDir,DIRECTORY_QUERY,&oa); 
	if (ntStatus!=STATUS_SUCCESS) 
	{ 
		printf("failed! "); 
		goto __exit; 
	} 
	printf("OK! "); 
	printf("ZwQueryDirectoryObject..."); 
	//��ѯĿ¼���� 
	do 
	{ 
		if (pBuffer) free(pBuffer); 
		uLength *= 2; 
		pBuffer=(PDIRECTORY_BASIC_INFORMATION)malloc(uLength); 
		if (!pBuffer) 
		{ 
			printf("malloc failed! "); 
			goto __exit; 
		} 
		ntStatus=ZwQueryDirectoryObject(hDir,pBuffer,uLength,FALSE,TRUE,&context,&dwRet); 
		printf("dwRet:%d ",dwRet); 
	} while (ntStatus == STATUS_MORE_ENTRIES || ntStatus == STATUS_BUFFER_TOO_SMALL);
	if (ntStatus==STATUS_SUCCESS) 
	{ 
		printf("OK! "); 
		//���� 
		pBuffer2=pBuffer; 
		while (pBuffer2->ObjectName.Length!=0&&pBuffer2->ObjectTypeName.Length!=0) 
		{ 
			printf("ObjectName: [%S] ObjectTypeName: [%S] ",pBuffer2->ObjectName.Buffer,pBuffer2->ObjectTypeName.Buffer); 
			pBuffer2++; 
		} 
	} 
	else 
	{ 
		printf("failed! "); 
	} 
__exit: 
	if (pBuffer) free(pBuffer); 
	if (hDir) ZwClose(hDir); 
	return 0; 
}

INT32 compare_str( LPCWSTR src_str, ULONG src_str_len, LPCWSTR dest_str, ULONG dest_str_len )
{
	INT32 j; 
	WCHAR src_ch; 
	WCHAR dest_ch; 
	INT32 ret = 0; 

	do 
	{
		if( src_str_len == 0 
			|| src_str_len != dest_str_len )
		{
			ret = -1; 
			break; 
		}

		for( j = 0; ( ULONG )j < dest_str_len; j ++ )
		{
			dest_ch = dest_str[ j ]; 

			if( dest_ch >= L'a' && dest_ch <= L'z' )
			{
				dest_ch = dest_ch + L'A' - L'a'; 
			}

			src_ch = src_str[ j ]; 

			if(  src_ch >= L'a' && src_ch  <= L'z' )
			{
				src_ch = src_ch + L'A' - L'a'; 
			}

			//printf( "source cahr is %c 0x%0.2x, destination char is %c 0x%0.2x\n", src_ch, src_ch, dest_ch, dest_ch ); 

			if( src_ch != dest_ch )
			{
				ret = -( j + 1 ); 
				ASSERT( ret < 0 ); 
				break; 
			}

			if( src_ch == L'\0' )
			{
				break; 
			}
		}
	} while ( FALSE ); 

	return ret; 
}

VOID dump_data( PVOID data, ULONG len )
{
	INT32 i; 
	INT32 j; 
	BYTE *data_out; 

	ASSERT( data != NULL ); 

	data_out = ( BYTE* )data; 
	for( i = 0; ( ULONG )i < len / 8; i++ )
	{
		printf( "0x%0.8x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x    %c %c %c %c %c %c %c %c\n", 
			i * 8 + 0, 
			data_out[ i * 8 + 0 ], 
			data_out[ i * 8 + 1 ], 
			data_out[ i * 8 + 2 ], 
			data_out[ i * 8 + 3 ], 
			data_out[ i * 8 + 4 ], 
			data_out[ i * 8 + 5 ], 
			data_out[ i * 8 + 6 ], 
			data_out[ i * 8 + 7 ], 

			data_out[ i * 8 + 0 ], 
			data_out[ i * 8 + 1 ], 
			data_out[ i * 8 + 2 ], 
			data_out[ i * 8 + 3 ], 
			data_out[ i * 8 + 4 ], 
			data_out[ i * 8 + 5 ], 
			data_out[ i * 8 + 6 ], 
			data_out[ i * 8 + 7 ] ); 
	}

	if( 0 != ( len % 8 ) )
	{
		printf( "0x%0.8x ", 
			i * 8 ); 
	}

	for( j = 0; ( ULONG )j < len % 8; j ++ )
	{
		printf( "0x%0.2x ", 
			data_out[ i * 8 + j ] ); 
	}

	printf( "    " ); 

	for( j = 0; ( ULONG )j < len % 8; j ++ )
	{
		printf( "%c", 
			data_out[ i * 8 + j ] ); 
	}


	printf( "\n" ); 
	return; 
}

INT32 hex_dump_to_buf( PVOID data, ULONG len, PVOID buf, ULONG buf_len )
{
	INT32 ret = 0; 
	INT32 i; 
	INT32 j; 
	BYTE *data_out; 
	INT32 writed; 

	ASSERT( data != NULL ); 
	ASSERT( buf != NULL ); 
	ASSERT( buf_len > 0 ); 

	do 
	{
		if( len == 0 )
		{
			ret = -EPERM;
			break; 
		}

		data_out = ( BYTE* )data; 
		writed = 0; 
		for( i = 0; ( ULONG )i < len / 8; i++ )
		{
			ret = _snprintf( ( BYTE* )buf + writed, buf_len - writed, "%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x", 
				data_out[ i * 8 + 0 ], 
				data_out[ i * 8 + 1 ], 
				data_out[ i * 8 + 2 ], 
				data_out[ i * 8 + 3 ], 
				data_out[ i * 8 + 4 ], 
				data_out[ i * 8 + 5 ], 
				data_out[ i * 8 + 6 ], 
				data_out[ i * 8 + 7 ] ); 
			if( ret < 0 )
			{
				break; 
			}
			else if( ret == buf_len - writed )
			{
				ret = -EPERM; 
				break; 
			}

			writed += ret; 
		}

		if( ret < 0 )
		{
			break; 
		}

		for( j = 0; ( ULONG )j < len % 8; j ++ )
		{
			ret = _snprintf( ( BYTE* )buf + writed, buf_len - writed, "%0.2x", 
				data_out[ i * 8 + j ] ); 
			if( ret < 0 )
			{
				break; 
			}
			else if( ret == buf_len - writed )
			{
				ret = -EPERM; 
				break; 
			}

			writed += ret; 
		}

		if( ret < 0 )
		{
			break; 
		}

	} while( FALSE );

	return ret; 
}

LRESULT recv_data_from_internet( LPCSTR url, PVOID data, ULONG read_len, ULONG *readed_len, data_download_callback download_tip_func, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
    DWORD flags;

	HINTERNET hOpen = NULL;	
	HINTERNET  hConnect = NULL; 

	CHAR agent[64]; 
	DWORD dwSize;
	CHAR szHead[] = "Accept: */*\r\n\r\n"; 
#define HTTP_READ_TMP_BUF_LEN 16384
	VOID* szTemp[ HTTP_READ_TMP_BUF_LEN ];
	DWORD dwByteToRead = 0;
	DWORD bytes_readed; 
	DWORD dwSizeOfRq = 4;
	DWORD dwBytes = 0;
	DWORD writed = 0; 
	ULONG try_time; 

	ASSERT( data != NULL ); 
	ASSERT( read_len > 0 ); 

	if( readed_len != NULL )
	{
		*readed_len = 0; 
	}

	memset( agent, 0, sizeof( agent ) ); 
	sprintf( agent, "agent %ld", timeGetTime() ); 
	_ret = InternetGetConnectedState( &flags, 0 ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "get internet connection state failed\n" ) ); 

		if( ret == ERROR_SUCCESS )
		{
			//ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}

		goto _return; 
	}

	if( ( flags & INTERNET_CONNECTION_PROXY ) != 0 )
		hOpen = InternetOpenA( agent, INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, NULL, NULL, 0 );
	else
		hOpen = InternetOpenA( agent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	
	if( hOpen == NULL )
	{
		ret = GetLastError();  
		log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "internet open failed 0x%0.8x \n", GetLastError() ) ); 
		goto _return; 
	}

	hConnect = InternetOpenUrlA ( hOpen, 
		url, 
		szHead, 
		lstrlenA( szHead ), 
		INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 
		0 ); 

	if ( hConnect == NULL )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "open url failed 0x%0.8x \n", GetLastError() ) ); 
		goto _return; 
	}

	//ERROR_INSUFFICIENT_BUFFER
	_ret = HttpQueryInfo( hConnect, 
		HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
		( LPVOID )&dwByteToRead, 
		&dwSizeOfRq, 
		NULL ); 

    if( _ret == FALSE )
	{
		_ret = GetLastError(); 
		//ret = GetLastError(); 

		log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "query http info failed 0x%0.8x \n", GetLastError() ) ); 

		dwByteToRead = 0;
	}

	//memset( szTemp, 0, sizeof( szTemp ) ); 

	bytes_readed = 0; 
	try_time = 0; 
#define MAX_TRY_TIME 5

	if( download_tip_func != NULL )
	{
		download_tip_func( data, dwByteToRead, read_len, bytes_readed, 0, context ); 
	}

	do
	{
		_ret = InternetReadFile( hConnect, szTemp, HTTP_READ_TMP_BUF_LEN, &dwSize ); 

		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			
			log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "read data from internet failed \n" ) ); 

			if( ret == ERROR_INTERNET_TIMEOUT 
				|| ret == ERROR_INTERNET_CONNECTION_ABORTED 
				|| ret == ERROR_INTERNET_CONNECTION_RESET )
			{
				DWORD bytes_to_read; 
				DWORD move_to_pos; 

				if( try_time == MAX_TRY_TIME )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					goto _return; 
				}

				InternetCloseHandle( hConnect ); 
				hConnect = InternetOpenUrlA ( hOpen, url, szHead,
					lstrlenA( szHead ), 
					INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 
					0 ); 

				if( hConnect == NULL )
				{
					ret = GetLastError(); 
					log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "open url failed 0x%0.8x \n", GetLastError() ) ); 
					goto _return; 
				}
				
				dwSizeOfRq = 4; 
				_ret = HttpQueryInfo( hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
					( LPVOID )&bytes_to_read, &dwSizeOfRq, NULL ); 

				if( _ret == FALSE )
				{
					ret = GetLastError(); 

					log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "query http info failed 0x%0.8x \n", GetLastError() ) ); 

					bytes_to_read = 0;
				}

				if( bytes_to_read != dwByteToRead )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					log_trace( ( MSG_ERROR, "file content length is not same as previous!\n" ) ); 
					goto _return; 
				}

				move_to_pos = InternetSetFilePointer( hConnect, bytes_readed, NULL, FILE_BEGIN, ( DWORD_PTR )NULL ); 

				if( move_to_pos == ( DWORD )( -1 ) )
				{
					SAFE_SET_ERROR_CODE( ret ); 
					goto _return; 
				}

				if( move_to_pos == bytes_readed )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					goto _return; 
				}

				try_time ++; 
				continue; 
			}
			goto _return; 
		}

		if( dwSize == 0 )
		{
			if( bytes_readed < dwByteToRead )
			{
				ASSERT( FALSE && "data is not all readed " ); 
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "read data length is smaller than the size requested\n" ) ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
			}

			if( readed_len != NULL )
			{
				*readed_len = bytes_readed; 
			}

			break;
		}
		else
		{
			ULONG remain_len; 
			ULONG copy_len; 

			ASSERT( bytes_readed < read_len ); 

			remain_len = read_len - bytes_readed; 
			copy_len = ( remain_len > dwSize ) ? dwSize : remain_len; 

			//ERROR_HTTP_HEADER_NOT_FOUND

			memcpy( ( BYTE* )data + bytes_readed, szTemp, copy_len ); 
		
			if( bytes_readed == read_len )
			{
				ASSERT( ret = ERROR_SUCCESS ); 
				ret = ERROR_SUCCESS; 
				
				if( readed_len != NULL )
				{
					*readed_len = bytes_readed; 
				}

				break; 
			}

			bytes_readed += dwSize; 

			if( download_tip_func != NULL )
			{
				download_tip_func( data, dwByteToRead, read_len, bytes_readed, 1, context ); 
			}
		}
	}while (TRUE);

_return:
	
	if( hConnect != NULL )
	{
		InternetCloseHandle( hConnect ); 
	}

	if( hOpen != NULL )
	{
		InternetCloseHandle( hOpen ); 
	}

	return ret;
}

LRESULT delete_link_file( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}

LRESULT output_file_from_res( HINSTANCE hResMod, LPCTSTR pszName, LPCTSTR pszType, LPCTSTR file_name )
{
	// Get the full path of the output file.

	LRESULT ret = ERROR_SUCCESS; 
	HRSRC hRes;
	HGLOBAL hgResMem;
	LPVOID pRes;
	DWORD dwResSize;
	FILE* pfDriverFile;
	BOOL bExitWithErr;

	// Get a memory copy of the Resource.

	hRes = FindResource( hResMod, pszName, pszType );
	if ( hRes == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	hgResMem = LoadResource( hResMod, hRes );
	if ( hgResMem == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	pRes = LockResource( hgResMem );
	if ( pRes == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	dwResSize = SizeofResource( hResMod, hRes );
	if ( dwResSize == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	// Write the Driver File on disk.

	DeleteFile( file_name ); 

	pfDriverFile = _tfopen( file_name, _T( "wb" ) );
	if ( pfDriverFile == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	// Write the file.

	bExitWithErr = FALSE;

	if ( fwrite( pRes, 1, dwResSize, pfDriverFile ) != dwResSize )
		bExitWithErr = TRUE;

	fclose( pfDriverFile );

	// Exit, eventually, returning error.

	if ( bExitWithErr == TRUE )
	{
		DeleteFile( file_name );

		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	// Return to the Caller.
_return:
	return ret;
}

LPCWSTR find_sub_str( LPCWSTR src_str, ULONG src_str_len, LPCWSTR dest_str, ULONG dest_str_len, ULONG find_time )
{
	LPCWSTR str_find = NULL; 
	INT32 i; 
	INT32 j; 
	INT32 str_matched; 
	WCHAR src_ch; 
	WCHAR dest_ch; 
	ULONG _find_time; 

	do 
	{
		if( src_str_len < dest_str_len )
		{
			break; 
		}

		_find_time = 0; 

		for( i = 0; ( ULONG )i <= src_str_len - dest_str_len; i ++ )
		{
			str_matched = TRUE; 
			for( j = 0; ( ULONG )j < dest_str_len; j ++ )
			{
				dest_ch = dest_str[ j ]; 

				if( dest_ch >= L'a' && dest_ch <= L'z' )
				{
					dest_ch = dest_ch + L'A' - L'a'; 
				}

				src_ch = src_str[ i + j ]; 

				if(  src_ch >= L'a' && src_ch  <= L'z' )
				{
					src_ch = src_ch + L'A' - L'a'; 
				}

				//printf( "source char is %c 0x%0.2x, destination char is %c 0x%0.2x\n", src_ch, src_ch, dest_ch, dest_ch ); 

				if( src_ch != dest_ch )
				{
					str_matched = FALSE; 
					break; 
				}
			}

			if( str_matched == TRUE )
			{
				str_find = &src_str[ i ]; 
				break; 
			}

			_find_time ++; 

			if( _find_time >= find_time )
			{
				break; 
			}
		}

	} while ( FALSE ); 

#ifdef DBG
	if( str_matched == TRUE )
	{
		ASSERT( str_find != NULL ); 
	}
	else
	{
		ASSERT( str_find == NULL ); 
	}
#endif //DBG

	return str_find; 
}

#define FILL_SPACE_TEXT L" " 

#define HEX_CH_10_ADDITION ( L'a' - 10 )

#define BYTE_DUMP_CH_COUNT 2
#ifdef _DEBUG
FORCEINLINE LRESULT dump_byte_in_buf_w( BYTE one_byte, WCHAR *dump_to_buf, ULONG ccb_buf_len )
#else
FORCEINLINE LRESULT dump_byte_in_buf_w( BYTE one_byte, WCHAR *dump_to_buf )
#endif //_DEBUG
{
	LRESULT ret = ERROR_SUCCESS; 
	register BYTE temp_byte; 
	//register BYTE _value; 

	do 
	{
#ifdef _DEBUG
		ASSERT( ccb_buf_len >= BYTE_DUMP_CH_COUNT ); 
#endif //_DEBUG

		temp_byte = one_byte >> 4; 
		if( temp_byte > 9 )
		{
			dump_to_buf[ 0 ] = temp_byte + HEX_CH_10_ADDITION; 
		}
		else
		{
			dump_to_buf[ 0 ] = temp_byte + L'0'; 
		}

		temp_byte = ( one_byte & 0xf ); 

		if( temp_byte > 9 )
		{
			dump_to_buf[ 1 ] = temp_byte + HEX_CH_10_ADDITION; 
		}
		else
		{
			dump_to_buf[ 1 ] = temp_byte + L'0'; 
		}
	}while( FALSE );

	return ret; 
}

#ifdef _DEBUG
#define _dump_byte_in_buf_w dump_byte_in_buf_w

#else
#define _dump_byte_in_buf_w( one_byte, dump_to_buf, ccb_buf_len ) dump_byte_in_buf_w( one_byte, dump_to_buf )
#endif //_DEBUG

#define DWORD_DUMP_CH_COUNT 8
#ifdef _DEBUG
FORCEINLINE LRESULT dump_dword_in_buf_w( DWORD one_dword, WCHAR *dump_to_buf, ULONG ccb_buf_len )
#else
FORCEINLINE LRESULT dump_dword_in_buf_w( DWORD one_dword, WCHAR *dump_to_buf )
#endif //_DEBUG
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
#ifdef _DEBUG
		ASSERT( ccb_buf_len >= DWORD_DUMP_CH_COUNT ); 
#endif //_DEBUG

		_dump_byte_in_buf_w( ( BYTE )( WORD )( one_dword >> 24 ), 
			dump_to_buf, 
			ccb_buf_len ); 

		_dump_byte_in_buf_w( ( ( BYTE )( WORD )( one_dword >> 16 ) & 0x000000ff ), 
			dump_to_buf + BYTE_DUMP_CH_COUNT, 
			ccb_buf_len - BYTE_DUMP_CH_COUNT ); 
		
		_dump_byte_in_buf_w( ( ( BYTE )( WORD )( one_dword >> 8 ) & 0x000000ff ), 
			dump_to_buf + ( BYTE_DUMP_CH_COUNT * 2 ), 
			ccb_buf_len - ( BYTE_DUMP_CH_COUNT * 2 ) ); 

		_dump_byte_in_buf_w( ( BYTE )( WORD )( one_dword & 0x000000ff ), 
			dump_to_buf + ( BYTE_DUMP_CH_COUNT * 3 ), 
			ccb_buf_len - ( BYTE_DUMP_CH_COUNT * 3 ) ); 
	}while( FALSE );

	return ret; 
}


#ifdef _DEBUG
#define _dump_dword_in_buf_w dump_dword_in_buf_w

#else
#define _dump_dword_in_buf_w( one_byte, dump_to_buf, ccb_buf_len ) dump_dword_in_buf_w( one_byte, dump_to_buf )
#endif //_DEBUG

#define QWORD_DUMP_CH_COUNT 16
#ifdef _DEBUG
FORCEINLINE LRESULT dump_qword_in_buf_w( ULONGLONG one_qword, WCHAR *dump_to_buf, ULONG ccb_buf_len )
#else
FORCEINLINE LRESULT dump_qword_in_buf_w( ULONGLONG one_qword, WCHAR *dump_to_buf )
#endif //_DEBUG
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
#ifdef _DEBUG
		ASSERT( ccb_buf_len >= QWORD_DUMP_CH_COUNT ); 
#endif //_DEBUG

		_dump_dword_in_buf_w( ( DWORD )( one_qword >> 32 ), 
			dump_to_buf, 
			ccb_buf_len ); 

		_dump_dword_in_buf_w( ( DWORD )( one_qword & 0x00000000ffffffff ), 
			dump_to_buf + DWORD_DUMP_CH_COUNT, 
			ccb_buf_len - DWORD_DUMP_CH_COUNT ); 
	}while( FALSE );

	return ret; 
}

LRESULT _dump_mem_in_buf_w( WCHAR *dump_to_buf, ULONG ccb_buf_len, USHORT ln_byte, void *mem, int size, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

#define MAX_BYTE_COUNT_IN_LINE  64
#define MIN_BYTE_COUNT_IN_LINE 1

	//WCHAR text[ MAX_BYTE_COUNT_IN_LINE + 1 ]; 
	BYTE *m = ( BYTE* )mem;
	INT32 i,j;
	register INT32 all_outputed = 0; 

	ASSERT( dump_to_buf != NULL ); 
	ASSERT( mem != NULL ); 

	if( ln_byte < MIN_BYTE_COUNT_IN_LINE /*|| ln_byte > MAX_BYTE_COUNT_IN_LINE */)
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ASSERT( ccb_buf_len > 0 ); 

	//text = _alloca( sizeof( WCHAR ) * ( ln_byte + 1 ) ); 

	for( j = 0; j < size / ln_byte; j++)
	{

		if( DUMP_MEM_ADDR == flags )
		{
			//if( sizeof( m ) == 8 )
			//{
			//	dump_qword_in_buf_w( ( ULONGLONG )m, dump_to_buf + outputed ); 
			//}
			//else
			{
				//only support 32 bit pointer now

				if( ccb_buf_len - all_outputed <= DWORD_DUMP_CH_COUNT + 1 )
				{
					ASSERT( ccb_buf_len - all_outputed > 1 ); 
					dump_to_buf[ all_outputed ] = L'\0'; 
					ret = ERROR_BUFFER_OVERFLOW; 
					goto _return; 
				}

				_dump_dword_in_buf_w( ( ULONG )( ULONG_PTR )m, dump_to_buf + all_outputed, ccb_buf_len - all_outputed ); 
				all_outputed += DWORD_DUMP_CH_COUNT; 
				
				dump_to_buf[ all_outputed ] = L' '; 
				all_outputed += 1; 
			}
		}

		for( i = 0; i < ln_byte; i++ ) 
		{
			if( ccb_buf_len - all_outputed <= BYTE_DUMP_CH_COUNT + 1 )
			{
				ASSERT( ccb_buf_len - all_outputed > 1 ); 
				dump_to_buf[ all_outputed ] = L'\0'; 
				ret = ERROR_BUFFER_OVERFLOW; 
				goto _return; 
			}

			_dump_byte_in_buf_w( m[ i ], dump_to_buf + all_outputed, ccb_buf_len - all_outputed ); 
			all_outputed += BYTE_DUMP_CH_COUNT; 

			dump_to_buf[ all_outputed ] = L' '; 
			all_outputed += 1; 

		}

		
		for( i = 0; i < ln_byte; i++ ) 
		{
			if( ccb_buf_len - all_outputed <= 1 + 1 )
			{
				ASSERT( ccb_buf_len - all_outputed > 1 ); 
				dump_to_buf[ all_outputed ] = L'\0'; 
				ret = ERROR_BUFFER_OVERFLOW; 
				goto _return; 
			}

			if( m[ i ] > ' ' && m[ i ] <= '~' )
			{
				dump_to_buf[ all_outputed ] = m[ i ]; 
			}
			else
			{
				dump_to_buf[ all_outputed ] = L'.'; 
			}

			all_outputed += 1; 
		}

		if( ccb_buf_len - all_outputed <= 2 + 1 )
		{
			ASSERT( ccb_buf_len - all_outputed > 1 ); 
			dump_to_buf[ all_outputed ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		dump_to_buf[ all_outputed ] = L'\r'; 
		all_outputed += 1; 

		dump_to_buf[ all_outputed ] = L'\n'; 
		all_outputed += 1; 

		//if( DUMP_MEM_ADDR == flags )
		//{
		//	outputed = _snwprintf( dump_to_buf + all_outputed, buf_ccb_len - all_outputed, L"0x%0.8x:  %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x  %s\r\n",
		//		m, m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], text );
		//}
		//else
		//{

		//	outputed = _snwprintf( dump_to_buf + all_outputed, buf_ccb_len - all_outputed, L"%0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x  %s\r\n",
		//		m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], text );
		//}

		//if( outputed == -1 
		//	|| ( ULONG )outputed + 1 >= buf_ccb_len - all_outputed )
		//{
		//	all_outputed = buf_ccb_len; 
		//	dump_to_buf[ all_outputed - 1 ] = L'\0'; 
		//	goto _return; 
		//}

		//all_outputed += ln_byte; 

		m += ln_byte;
	}

	if( DUMP_MEM_ADDR == flags )
	{
		//if( sizeof( m ) == 8 )
		//{
		//	dump_qword_in_buf_w( ( ULONGLONG )m, dump_to_buf + outputed ); 
		//}
		//else
		{
			//only support 32 bit pointer now

			if( ccb_buf_len - all_outputed <= DWORD_DUMP_CH_COUNT + 1 )
			{
				ASSERT( ccb_buf_len - all_outputed > 1 ); 
				dump_to_buf[ all_outputed ] = L'\0'; 
				ret = ERROR_BUFFER_OVERFLOW; 
				goto _return; 
			}

			_dump_dword_in_buf_w( ( ULONG )( ULONG_PTR )( PVOID )m, dump_to_buf + all_outputed, ccb_buf_len - all_outputed ); 
			all_outputed += DWORD_DUMP_CH_COUNT; 

			dump_to_buf[ all_outputed ] = L' '; 
			all_outputed += 1; 
		}
	}

	for( i = 0; i < ( size % ln_byte ); i++ ) 
	{
		if( ccb_buf_len - all_outputed <= BYTE_DUMP_CH_COUNT + 1 )
		{
			ASSERT( ccb_buf_len - all_outputed > 1 ); 
			dump_to_buf[ all_outputed ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		_dump_byte_in_buf_w( m[ i ], dump_to_buf + all_outputed, ccb_buf_len - all_outputed  ); 
		all_outputed += BYTE_DUMP_CH_COUNT; 

		dump_to_buf[ all_outputed ] = L' '; 
		all_outputed += 1; 
	}

	for( i = 0; i < ln_byte - ( size % ln_byte ); i++ ) 
	{
		if( ccb_buf_len - all_outputed <= BYTE_DUMP_CH_COUNT + 1 )
		{
			ASSERT( ccb_buf_len - all_outputed > 1 ); 
			dump_to_buf[ all_outputed ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		dump_to_buf[ all_outputed ] = L' '; 
		all_outputed += 1; 
		dump_to_buf[ all_outputed ] = L' '; 
		all_outputed += 1; 
		dump_to_buf[ all_outputed ] = L' '; 
		all_outputed += 1; 
	}

	for( i = 0; i < ( size % ln_byte ); i++ ) 
	{
		if( ccb_buf_len - all_outputed <= 1 + 1 )
		{
			ASSERT( ccb_buf_len - all_outputed > 1 ); 
			dump_to_buf[ all_outputed ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		if( m[ i ] > ' ' && m[ i ] <= '~' )
		{
			dump_to_buf[ all_outputed ] = m[ i ]; 
		}
		else
		{
			dump_to_buf[ all_outputed ] = '.'; 
		}

		all_outputed += 1; 
	}

	if( ccb_buf_len - all_outputed <= 2 + 1 )
	{
		ASSERT( ccb_buf_len - all_outputed > 1 ); 
		dump_to_buf[ all_outputed ] = L'\0'; 
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	dump_to_buf[ all_outputed ] = L'\r'; 
	all_outputed += 1; 
	
	dump_to_buf[ all_outputed ] = L'\n'; 
	all_outputed += 1; 

	dump_to_buf[ all_outputed ] = L'\0'; 

	//for( i = 0; i < ( size % ln_byte ); i++ ) 
	//{
	//	if( m[ i ] > ' ' && m[ i ] <= '~' )
	//		text[ i ] = m[ i ];
	//	else
	//		text[ i ] = '.';
	//}

	//text[ ( size % ln_byte ) ] = L'\0'; 

	//if( DUMP_MEM_ADDR == flags )
	//{
	//	outputed = _snwprintf( dump_to_buf + all_outputed, buf_ccb_len - all_outputed, L"0x%0.8x:  ", m );

	//	if( outputed == -1 
	//		|| ( ULONG )outputed + 1 >= buf_ccb_len - all_outputed )
	//	{
	//		all_outputed = buf_ccb_len; 
	//		dump_to_buf[ all_outputed - 1 ] = L'\0'; 
	//		goto _return; 
	//	}

	//	all_outputed += ( ULONG )outputed; 
	//}

	//for( i = 0; i < ( size % ln_byte ); i++ ) 
	//{
	//	outputed = _snwprintf( dump_to_buf + all_outputed, buf_ccb_len - all_outputed, L"%0.2x ", m[ i ] );

	//	if( outputed == -1 
	//		|| ( ULONG )outputed + 1 >= buf_ccb_len - all_outputed )
	//	{
	//		all_outputed = buf_ccb_len; 
	//		dump_to_buf[ all_outputed - 1 ] = L'\0'; 
	//		goto _return; 
	//	}

	//	all_outputed += outputed; 
	//}

	//for( i = 0; i < ln_byte - ( size % ln_byte ); i++ ) 
	//{
	//	if( buf_ccb_len - all_outputed < ( ULONG )outputed + CONST_STR_LEN( FILL_SPACE_TEXT ) + 1 )
	//	{
	//		all_outputed = outputed + 1; 
	//		dump_to_buf[ all_outputed - 1 ] = L'\0'; 
	//		goto _return; 
	//	}

	//	memcpy( dump_to_buf + all_outputed, FILL_SPACE_TEXT, sizeof( FILL_SPACE_TEXT ) - sizeof( FILL_SPACE_TEXT[ 0 ] ) ); 


	//	all_outputed += CONST_STR_LEN( FILL_SPACE_TEXT ); 
	//}

	//outputed = _snwprintf( dump_to_buf + all_outputed, buf_ccb_len - all_outputed, L"%s\r\n", text );

	//if( outputed == -1 
	//	|| ( ULONG )outputed + 1 >= buf_ccb_len - all_outputed )
	//{
	//	all_outputed = buf_ccb_len; 
	//	dump_to_buf[ all_outputed - 1 ] = L'\0'; 
	//	goto _return; 
	//}

	//all_outputed += outputed; 

_return:
	return ret; 
}

LRESULT dump_mem_in_buf( CHAR *dump_to_buf, ULONG buf_len, void *mem, int size )
{
	LRESULT ret = ERROR_SUCCESS; 

	unsigned char str[20];
	unsigned char *m = mem;
	int i,j;
	INT32 outputed; 
	INT32 all_outputed = 0; 

	ASSERT( dump_to_buf != NULL ); 
	ASSERT( mem != NULL ); 

	for (j = 0; j < size / 8; j++)
	{
		memset( str,0,sizeof str );
		for( i = 0; i < 8; i++ ) 
		{
			if( m[i] > ' ' && m[i] <= '~' )
				str[i] = m[i];
			else
				str[i] = '.';
		}

		outputed = _snprintf( dump_to_buf + all_outputed, buf_len - all_outputed, "0x%0.8x  %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x  %s\r\n",
			m, m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], str );

		if( ( ULONG )outputed >= buf_len - all_outputed )
		{
			all_outputed = buf_len; 
			dump_to_buf[ all_outputed - 1 ] = '\0'; 
			goto _return; 
		}

		all_outputed += outputed - 1; 

		m += 8;
	}

	memset( str,0,sizeof str );
	for( i = 0; i < ( size % 8 ); i++ ) 
	{
		if( m[i] > ' ' && m[i] <= '~' )
			str[i] = m[i];
		else
			str[i] = '.';
	}

	outputed = _snprintf( dump_to_buf + all_outputed, buf_len - all_outputed, "0x%08p  ", m );

	if( ( ULONG )outputed >= buf_len - all_outputed )
	{
		all_outputed = buf_len; 
		dump_to_buf[ all_outputed - 1 ] = '\0'; 
		goto _return; 
	}

	all_outputed += ( ULONG )outputed - 1; 

	for( i = 0; i < ( size % 8 ); i++ ) 
	{
		outputed = _snprintf( dump_to_buf + all_outputed, buf_len - all_outputed, "%0.2x ", m[ i ] );

		if( ( ULONG )outputed >= buf_len - all_outputed )
		{
			all_outputed = buf_len; 
			dump_to_buf[ all_outputed - 1 ] = '\0'; 
			goto _return; 
		}

		all_outputed += outputed - 1; 
	}

	outputed = _snprintf( dump_to_buf + all_outputed, buf_len - all_outputed, "%s \r\n", str );

	if( ( ULONG )outputed >= buf_len - all_outputed )
	{
		all_outputed = buf_len; 
		dump_to_buf[ all_outputed - 1 ] = '\0'; 
		goto _return; 
	}

	all_outputed += outputed - 1; 

_return:
	return ret; 
}

#define RISE_TO_PRIORITY_CLASS ABOVE_NORMAL_PRIORITY_CLASS // HIGH_PRIORITY_CLASS BELOW_NORMAL_PRIORITY_CLASS
#define RISE_TO_PRIORITY THREAD_PRIORITY_ABOVE_NORMAL //THREAD_PRIORITY_HIGHEST //THREAD_PRIORITY_TIME_CRITICAL 

INT32 RiseThreadPriority( PRIORITY_INFO *PriorityInfo )
{
	INT32 Ret = 0; 
	ULONG ErrorCode; 
	ULONG OldPriorityClass = 0; 
	ULONG OldPriority = THREAD_PRIORITY_ERROR_RETURN; 
	BOOLEAN PriorityClassSet = FALSE; 
	BOOLEAN PrioritySet = FALSE; 

	PriorityInfo->OldPriority = THREAD_PRIORITY_ERROR_RETURN; 
	PriorityInfo->OldPriorityClass = 0; 

	OldPriorityClass = GetPriorityClass( GetCurrentProcess() ); 
	if( OldPriorityClass == 0 )
	{
		ErrorCode = GetLastError(); 
		dbg_print( MSG_ERROR, "get process priority class error %u\n", ErrorCode ); 
		goto _Exit; 
	}

	OldPriority = GetThreadPriority( GetCurrentThread() ); 
	if( OldPriority == THREAD_PRIORITY_ERROR_RETURN )
	{
		ErrorCode = GetLastError(); 
		dbg_print( MSG_ERROR, "get priority error %u\n", ErrorCode ); 
		goto _Exit; 
	}

	dbg_print( MSG_ERROR, "old priority class is %u\n", OldPriorityClass ); 
	dbg_print( MSG_ERROR, "old priority is %u\n", OldPriority ); 

	if( !SetPriorityClass( GetCurrentProcess(), RISE_TO_PRIORITY_CLASS ) )
	{
		Ret = -EFAULT; 

		ErrorCode = GetLastError(); 
		dbg_print( MSG_ERROR, "set priority error %u\n", ErrorCode ); 
		goto _Exit; 
	}

	PriorityClassSet = TRUE; 

	if( !SetThreadPriority( GetCurrentThread(), RISE_TO_PRIORITY ) )
	{
		Ret = -EFAULT; 

		ErrorCode = GetLastError();

		dbg_print( MSG_ERROR, "set priority error %u\n", ErrorCode ); 

		goto _Exit; 
	} 

	PrioritySet = TRUE; 

_Exit:

	if( Ret != 0 )
	{
		if( PriorityClassSet == TRUE )
		{
			ASSERT( OldPriorityClass != 0 ); 
			if( !SetPriorityClass( GetCurrentProcess(), OldPriorityClass ) )
			{
				ErrorCode = GetLastError(); 
				dbg_print( MSG_ERROR, "set priority error %u\n", ErrorCode ); 
			}
		}

		if( PrioritySet == TRUE )
		{
			ASSERT( OldPriority != THREAD_PRIORITY_ERROR_RETURN ); 

			if( !SetThreadPriority( GetCurrentThread(), OldPriorityClass ) )
			{
				ErrorCode = GetLastError(); 
				dbg_print( MSG_ERROR, "set priority error %u\n", ErrorCode ); 
			}
		}

	}

	return Ret; 
}

INT32 LowerThreadPriority( PRIORITY_INFO *PriorityInfo )
{
	INT32 Ret = 0; 
	ULONG ErrorCode; 
	ULONG OldPriorityClass; 
	ULONG OldPriority; 

	ASSERT( PriorityInfo != NULL ); 

	OldPriorityClass = GetPriorityClass( GetCurrentProcess() ); 
	if( OldPriorityClass == 0 )
	{
		ErrorCode = GetLastError(); 
		dbg_print( MSG_ERROR, "get process priority class error %u\n", ErrorCode ); 
	}

	OldPriority = GetThreadPriority( GetCurrentThread() ); 
	if( OldPriority == THREAD_PRIORITY_ERROR_RETURN )
	{
		ErrorCode = GetLastError(); 
		dbg_print( MSG_ERROR, "get priority error %u\n", ErrorCode ); 
	}

	dbg_print( MSG_ERROR, "old priority class is %u\n", OldPriorityClass ); 
	dbg_print( MSG_ERROR, "old priority is %u\n", OldPriority ); 

	if( PriorityInfo->OldPriority == THREAD_PRIORITY_ERROR_RETURN 
		&& PriorityInfo->OldPriorityClass == 0 )
	{
		Ret = -EINVAL; 
		goto _Exit; 
	}

	if( PriorityInfo->OldPriorityClass != 0 )
	{
		if( !SetPriorityClass( GetCurrentProcess(), OldPriorityClass ) )
		{
			Ret = -EFAULT; 

			ErrorCode = GetLastError(); 
			dbg_print( MSG_ERROR, "set priority eror %u\n", ErrorCode ); 
		}
	}

	if( PriorityInfo->OldPriority != THREAD_PRIORITY_ERROR_RETURN )
	{
		if( !SetThreadPriority(GetCurrentThread(), PriorityInfo->OldPriority) )
		{
			Ret = -EFAULT; 

			ErrorCode = GetLastError(); 
			dbg_print( MSG_ERROR, "set priority eror %u\n", ErrorCode ); 
		}
	}

_Exit:

	return Ret; 
}

//
//  FUNCTION: AddToMessageLog(LPTSTR lpszMsg)
//
//  PURPOSE: Allows any thread to log an error message
//
//  PARAMETERS:
//    lpszMsg - text for message
//    type    - 1 Error 0 Info
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
LRESULT AddToMessageLog( LPTSTR szEventSourceName, LPTSTR lpszMsg, ULONG type )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR   szMsg[256];
	HANDLE  hEventSource;
	LPTSTR  lpszStrings[2]; 
	ULONG dwErr; 

	do 
	{
		dwErr = GetLastError();

		// Use event logging to log the error.
		//
		hEventSource = RegisterEventSource(NULL, szEventSourceName );
		if( hEventSource == NULL ) 
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_stprintf(szMsg, TEXT("%s error: %d"), szEventSourceName, dwErr);
		lpszStrings[0] = szMsg;
		lpszStrings[1] = lpszMsg;


		if( type )
		{
			ReportEvent(hEventSource,         // handle of event source
				EVENTLOG_ERROR_TYPE,  // event type
				0,                    // event category
				0,                    // event ID
				NULL,                 // current user's SID
				2,                    // strings in lpszStrings
				0,                    // no bytes of raw data
				(const char **)lpszStrings,  // array of error
				NULL);                // no raw data
		}
		else
		{
			ReportEvent(hEventSource,               // handle of event
			EVENTLOG_INFORMATION_TYPE,  // event type
			0,                          // event category
			0,                          // event ID
			NULL,                       // current user's SID
			2,                          // strings in
			lpszStrings, 
			0,                          // no bytes of raw data
			(const char **)lpszStrings,        // array of error
			NULL);                      // no raw data
		}

		(void) DeregisterEventSource(hEventSource);
	}while( FALSE );

	return ret; 
}

#include <locale.h>
LRESULT init_std_lib_mbc_local_lang()
{
	#define MAX_CODE_PAGE_TEXT_LEN 16

	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	CHAR code_page[ MAX_CODE_PAGE_TEXT_LEN ];
	INT32 _code_page; 

	do 
	{
		_code_page = GetACP();
		_ret = _snprintf( code_page, ARRAY_SIZE( code_page ), ".%d", _code_page ); 
		if( _ret < 0 || _ret >= ARRAY_SIZE( code_page ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( NULL == setlocale( LC_ALL, code_page ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

	}while( FALSE ); 

	return ret; 
}

INT32 CALLBACK folder_browser_callback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) 
{
	INT32 ret = 0; 

	return ret; 

	//switch(uMsg)  
	//{  
	//case BFFM_INITIALIZED:  //ѡ���ļ��жԻ����ʼ�� 

	//	::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);  
	//	
	//	//��STATUSTEXT������ʾ��ǰ·��   
	//	::SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, lpData);  
	//	
	//	//����ѡ���ļ��жԻ���ı���   
	//	::SetWindowText(hwnd, TEXT("�������ø�����Ŀ¼"));   
	//	break;  
	//case BFFM_SELCHANGED:   //ѡ���ļ��б��ʱ   
	//	{  
	//		TCHAR pszPath[MAX_PATH];  
	//		//��ȡ��ǰѡ��·��   
	//		SHGetPathFromIDList((LPCITEMIDLIST)lParam, pszPath);  
	//		//��STATUSTEXT������ʾ��ǰ·��   
	//		::SendMessage(hwnd, BFFM_SETSTATUSTEXT, TRUE, (LPARAM)pszPath);  
	//	}  
	//	break;  
	//}

	//return 0; 
}  

LRESULT WINAPI open_folder_dlg( LPWSTR folder_path, ULONG cc_buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	BROWSEINFO bi; 
	LPITEMIDLIST pidl; 

	do 
	{
		if( cc_buf_len < MAX_PATH )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			break; 
		}

		bi.hwndOwner      = NULL;
		bi.pidlRoot       = NULL;
		bi.pszDisplayName = NULL; 
		bi.lpszTitle      = NULL; //TEXT( "��ѡ���ļ���" ); 
		bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_USENEWUI; 
		bi.lpfn           = NULL; //folder_browser_callback; //�ص�����
		bi.lParam         = NULL; //(LPARAM)TEXT("D:\\"); //�����ص������Ĳ���,����Ĭ��·��
		bi.iImage         = 0; 

		pidl = SHBrowseForFolder(&bi);
		if( pidl == NULL)
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_ret = SHGetPathFromIDList( pidl, folder_path ); 
		if( _ret == FALSE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

#pragma comment( lib, "strsafe.lib" )

LRESULT WINAPI sprint_time( LPWSTR text, ULONG ccb_buf_len, LARGE_INTEGER time )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG _ret;  
	FILETIME file_time; 
	FILETIME local_file_time; 
	SYSTEMTIME system_time; 

	do 
	{
		ASSERT( text != NULL ); 
		ASSERT( ccb_buf_len > 0 ); 

		file_time.dwHighDateTime = time.HighPart; 
		file_time.dwLowDateTime = time.LowPart; 

		_ret = FileTimeToLocalFileTime( &file_time, &local_file_time ); 
		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = ( LONG )FileTimeToSystemTime( &local_file_time, 
			&system_time ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = ( LONG )StringCchPrintfExW( text, 
			ccb_buf_len, 
			NULL, 
			NULL, 
			0, 
			L"%u/%u/%u %u:%u:%u.%u", 
			system_time.wYear, 
			system_time.wMonth, 
			system_time.wDay, 
			system_time.wHour, 
			system_time.wMinute, 
			system_time.wSecond, 
			( local_file_time.dwLowDateTime % 10000 )
			//system_time.wMilliseconds 
			); 

		if( FAILED( _ret ) )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		text[ 0 ] = L'\0'; 
	}

	return ret; 
}

LRESULT WINAPI sprint_file_time( LPWSTR text, ULONG ccb_buf_len, FILETIME *time )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG _ret;  
	FILETIME local_file_time; 
	SYSTEMTIME system_time; 

	do 
	{
		ASSERT( time != NULL ); 
		ASSERT( text != NULL ); 
		ASSERT( ccb_buf_len > 0 ); 

		_ret = FileTimeToLocalFileTime( time, &local_file_time ); 
		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = ( LONG )FileTimeToSystemTime( &local_file_time, 
			&system_time ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = ( LONG )StringCchPrintfExW( text, 
			ccb_buf_len, 
			NULL, 
			NULL, 
			0, 
			L"%u/%u/%u %u:%u:%u.%u", 
			system_time.wYear, 
			system_time.wMonth, 
			system_time.wDay, 
			system_time.wHour, 
			system_time.wMinute, 
			system_time.wSecond, 
			( local_file_time.dwLowDateTime % 10000 )
			//system_time.wMilliseconds 
			); 

		if( FAILED( _ret ) )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		text[ 0 ] = L'\0'; 
	}

	return ret; 
}

LRESULT WINAPI string_to_time( LPCWSTR text, LARGE_INTEGER *time )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG _ret;  
	FILETIME file_time; 
	FILETIME local_file_time; 
	SYSTEMTIME system_time; 
	ULONG nano_100_seconds; 

	do 
	{
		ASSERT( time != NULL ); 

		time->QuadPart = 0; 

		_ret = ( LONG )swscanf_s( text, 
			L"%04u/%02u/%02u %02u:%02u:%02u.%04u", 
			&system_time.wYear, 
			&system_time.wMonth, 
			&system_time.wDay, 
			&system_time.wHour, 
			&system_time.wMinute, 
			&system_time.wSecond, 
			&nano_100_seconds ); 

#define TIME_FIELD_COUNT 7
		if( _ret != TIME_FIELD_COUNT )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		system_time.wMilliseconds = 0; 
		_ret = ( LONG )SystemTimeToFileTime( &system_time, 
			&local_file_time ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = LocalFileTimeToFileTime( &local_file_time, &file_time ); 
		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		time->HighPart = file_time.dwHighDateTime; 
		time->LowPart = file_time.dwLowDateTime; 

		time->QuadPart += nano_100_seconds; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI string_to_local_time( LPCWSTR text, LARGE_INTEGER *time )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG _ret;  
	FILETIME local_file_time; 
	SYSTEMTIME system_time; 

	do 
	{
		ASSERT( time != NULL ); 

		time->QuadPart = 0; 

		_ret = ( LONG )swscanf_s( text, 
			L"%u/%u/%u %u:%u:%u.%u", 
			&system_time.wYear, 
			&system_time.wMonth, 
			&system_time.wDay, 
			&system_time.wHour, 
			&system_time.wMinute, 
			&system_time.wSecond, 
			&system_time.wMilliseconds ); 

#define TIME_FIELD_COUNT 6
		if( _ret != TIME_FIELD_COUNT )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_ret = ( LONG )SystemTimeToFileTime( &system_time, 
			&local_file_time ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		time->HighPart = local_file_time.dwHighDateTime; 
		time->LowPart = local_file_time.dwLowDateTime; 

	}while( FALSE ); 

	return ret; 
}


LRESULT WINAPI sprint_local_time( LPWSTR text, ULONG ccb_buf_len, LARGE_INTEGER time )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG _ret;  
	FILETIME local_file_time; 
	SYSTEMTIME system_time; 

	do 
	{
		ASSERT( text != NULL ); 
		ASSERT( ccb_buf_len > 0 ); 

		local_file_time.dwHighDateTime = time.HighPart; 
		local_file_time.dwLowDateTime = time.LowPart; 

		_ret = ( LONG )FileTimeToSystemTime( &local_file_time, 
			&system_time ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = ( LONG )StringCchPrintfExW( text, 
			ccb_buf_len, 
			NULL, 
			NULL, 
			0, 
			L"%u/%u/%u %u:%u:%u.%u", 
			system_time.wYear, 
			system_time.wMonth, 
			system_time.wDay, 
			system_time.wHour, 
			system_time.wMinute, 
			system_time.wSecond, 
			( local_file_time.dwLowDateTime % 10000 )
			); 

		if( FAILED( _ret ) )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		text[ 0 ] = L'\0'; 
	}

	return ret; 
}

LRESULT WINAPI check_time_valid( LARGE_INTEGER *time )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	FILETIME file_time; 
	FILETIME local_file_time; 
	SYSTEMTIME sys_time; 
	SYSTEMTIME cur_sys_time; 

	//FILETIME cur_file_time; 

	do 
	{
		file_time.dwHighDateTime = time->HighPart; 
		file_time.dwLowDateTime = time->LowPart; 

		_ret = FileTimeToLocalFileTime( &file_time, &local_file_time ); 
		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		FileTimeToSystemTime( &local_file_time, &sys_time ); 

		GetSystemTime( &cur_sys_time ); 
		//if( _ret == FALSE )
		//{
		//	SAFE_SET_ERROR_CODE( ret ); 
		//	break; 
		//}

		if( sys_time.wYear != cur_sys_time.wYear )
		{
			ret = ERROR_INVALID_PARAMETER; 
			//DBG_BP(); 
			break; 
		}
	}while( FALSE );

	return ret; 
}

#define MAX_DISK_COUNT 64
#define IDE_DISK_DESC_PREFIX "IDE"

INT32 WINAPI FindDiskFromBusType( LPCSTR BusTypeName, PULONG DeviceIndexes, ULONG BufCount, ULONG *RetCount )
{
	INT32 Ret = 0; 
	HRESULT hr; 
	LSTATUS __Ret; 
	ULONG BufSize; 
	ULONG RetSize; 
	CHAR DevDesc[ MAX_PATH ]; 
	CHAR _DiskNum[ 16 ]; 
	ULONG Type; 
	ULONG DevCount; 
	ULONG FoundCount; 
	HKEY RegKey = NULL; 
	ULONG BusTypeNameLen; 
	INT32 i = 0; 

	do 
	{
		if( BusTypeName == NULL )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( DeviceIndexes == NULL )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( RetCount != NULL )
		{
			*RetCount = 0; 
		}

		__Ret = RegOpenKeyExW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum", 0, KEY_READ, &RegKey ); 
		if( __Ret != ERROR_SUCCESS )
		{
			break; 
		}

		BufSize = sizeof( DevCount ); 
		__Ret = RegQueryValueExA( RegKey, "Count",  NULL, &Type, ( BYTE* )&DevCount, &BufSize ); 
		if( __Ret != ERROR_SUCCESS ) 
		{
			break; 
		} 

		BusTypeNameLen = strlen( BusTypeName ); 

		FoundCount = 0; 
		for( i = 0; ( ULONG )i < DevCount; i++ ) 
		{ 
			hr = StringCbPrintfA( _DiskNum, 
				sizeof( _DiskNum ), 
				"%u",
				i ); 

			if( FAILED( hr ) )
			{
				break; 
			}

			RetSize = sizeof( DevDesc ); 

			__Ret = RegQueryValueExA( RegKey, _DiskNum, NULL, &Type, ( LPBYTE )DevDesc, &RetSize ); 

			if( __Ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( 0 == strnicmp( DevDesc, 
				BusTypeName, 
				BusTypeNameLen ) ) 
			{
				//break; 
				if( FoundCount >= BufCount )
				{
					Ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}

				DeviceIndexes[ FoundCount ] = ( ULONG )i; 
				FoundCount ++; 
			} 
		} 

		for( i = FoundCount; ( ULONG )i < BufCount; i ++  )
		{
			DeviceIndexes[ i ] = NULL; 
		}

		if( RetCount != NULL )
		{
			*RetCount = FoundCount; 
		}

	}while( FALSE );

	if( RegKey != NULL )
	{
		RegCloseKey( RegKey ); 
	}

	return Ret; 
}

INT32 CheckDiskUsbBusFromDiskNo( ULONG DiskNo, BOOLEAN *IsUsbBus )
{
	INT32 Ret = 0; 
	INT32 _Ret; 
	STORAGE_PROPERTY_QUERY Query; 
	STORAGE_ADAPTER_DESCRIPTOR AdapterDesc; 
	HRESULT hr; 
#define PHYSICAL_DISK_DEV_SYMBOL_NAME "\\\\.\\PHYSICALDRIVE%u"
	CHAR DiskDevName[ MAX_PATH ]; 
	HANDLE DiskDev = INVALID_HANDLE_VALUE; 
	ULONG RetSize = 0; 
	//ULONG BufSize; 

	do 
	{
		if( NULL == IsUsbBus )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*IsUsbBus = FALSE; 

		hr = StringCbPrintfA( DiskDevName, sizeof( DiskDevName ), PHYSICAL_DISK_DEV_SYMBOL_NAME, DiskNo ); 
		if( FAILED( hr ) )
		{
			Ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		DiskDev = CreateFileA( DiskDevName,
			GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			NULL ); 

		if( DiskDev == NULL 
			|| DiskDev == INVALID_HANDLE_VALUE ) 
		{ 
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		} 

		printf( "open disk %u device successfully\n", DiskNo ); 

		Query.PropertyId = StorageAdapterProperty;  
		Query.QueryType = PropertyStandardQuery;  

		AdapterDesc.Size = sizeof( AdapterDesc ); 

		_Ret = DeviceIoControl( DiskDev, 
			IOCTL_STORAGE_QUERY_PROPERTY, 
			&Query,   
			sizeof( STORAGE_PROPERTY_QUERY ), 
			&AdapterDesc, 
			sizeof( AdapterDesc ), 
			&RetSize, 
			( LPOVERLAPPED )NULL ); 

		if( _Ret == FALSE )
		{
#ifdef _DEBUG
			LPVOID ErrorMsg = NULL; 

			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS, 
				NULL, 
				GetLastError(), 
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language 
				( LPTSTR )&ErrorMsg, 
				0, 
				NULL ); 

			if( ErrorMsg != NULL )
			{
				LocalFree( ErrorMsg ); 
			}
#endif //_DEBUG

			Ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( AdapterDesc.BusType == BusTypeUsb )
		{
			*IsUsbBus = TRUE; 
			break; 
		}
	}while( FALSE ); 

	if( DiskDev != INVALID_HANDLE_VALUE 
		&& DiskDev != NULL )
	{
		CloseHandle( DiskDev ); 
	}

	return Ret; 
}

#if 0
INT32 WINAPI PartIsInDisk( ULONG DiskNo, PARTITION_INFORMATION *PartInfo, BOOLEAN *IsInDisk )
{
	INT32 Ret = FALSE; 
	INT32 _Ret; 
	HRESULT hr; 
#define PHYSICAL_DISK_DEV_SYMBOL_NAME "\\\\.\\PHYSICALDRIVE%u"
	CHAR DiskDevName[ MAX_PATH ]; 
	HANDLE DiskDev = INVALID_HANDLE_VALUE; 
	ULONG RetSize = 0; 
	ULONG BufSize; 
	PDRIVE_LAYOUT_INFORMATION LayoutInfo =  NULL; 

	do 
	{
		if( IsInDisk == NULL )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*IsInDisk = FALSE; 

		if( PartInfo == NULL )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = StringCbPrintfA( DiskDevName, sizeof( DiskDevName ), PHYSICAL_DISK_DEV_SYMBOL_NAME, DiskNo ); 
		if( FAILED( hr ) )
		{
			Ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		BufSize = sizeof( DRIVE_LAYOUT_INFORMATION ) + sizeof( PARTITION_INFORMATION ) * 104; 

		LayoutInfo = ( PDRIVE_LAYOUT_INFORMATION )malloc( BufSize ); 

		if( LayoutInfo == NULL) 
		{ 
			break; 
		} 

		memset( LayoutInfo, 0, BufSize ); 

		DiskDev = CreateFileA( DiskDevName,
			GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			NULL ); 

		if( DiskDev == NULL 
			|| DiskDev == INVALID_HANDLE_VALUE ) 
		{ 
			break; 
		} 

		_Ret = DeviceIoControl( 
			DiskDev, 
			IOCTL_DISK_GET_DRIVE_LAYOUT, 
			NULL, 
			0, 
			( LPVOID )LayoutInfo, 
			BufSize, 
			&RetSize, 
			NULL ); 

		if( FALSE == _Ret ) 
		{ 
#ifdef _DEBUG
			LPVOID ErrorMsg = NULL; 

			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS, 
				NULL, 
				GetLastError(), 
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language 
				( LPTSTR )&ErrorMsg, 
				0, 
				NULL ); 

			if( ErrorMsg != NULL )
			{
				LocalFree( ErrorMsg ); 
			}
#endif //_DEBUG
			break; 
		} 
		{
			PPARTITION_INFORMATION _PartInfo; 
			for( ULONG i = 0; i < LayoutInfo->PartitionCount; i = i + 4 )
			{ 
				_PartInfo = &LayoutInfo->PartitionEntry[ i ]; 

				if( ( _PartInfo->StartingOffset.QuadPart == PartInfo->StartingOffset.QuadPart ) 
					&& ( _PartInfo->PartitionLength.QuadPart == PartInfo->PartitionLength.QuadPart ) 
					&& ( _PartInfo->HiddenSectors == PartInfo->HiddenSectors ) 
					&& ( _PartInfo->PartitionNumber == PartInfo->PartitionNumber ) 
					&& ( _PartInfo->PartitionType == PartInfo->PartitionType ) 
					&& ( _PartInfo->BootIndicator == PartInfo->BootIndicator ) 
					&& ( _PartInfo->RecognizedPartition == PartInfo->RecognizedPartition )
					&& ( _PartInfo->RewritePartition == PartInfo->RewritePartition ) ) 
				{
					*IsInDisk = TRUE; 
					break;
				} 
			}
		}
	}while( FALSE ); 

	if( LayoutInfo != NULL )
	{
		free( LayoutInfo ); 
	}

	if( DiskDev != INVALID_HANDLE_VALUE )
	{
		CloseHandle( DiskDev ); 
	}

	return Ret; 
}
#endif //0

INT32 WINAPI GetDiskNumbers( HANDLE VolumeDev, 
							ULONG *DiskNumbers, 
							ULONG Count, 
							ULONG *RetCount )
{
	INT32 Ret = 0; 
	INT32 _Ret; 
	INT32 i; 
	ULONG BufSize; 
	ULONG RetSize; 
	ULONG ErrorCode; 
	BOOLEAN SizeCorrect = FALSE; 
	PVOLUME_DISK_EXTENTS VolDiskExt = NULL; 

	do 
	{
		if( VolumeDev == INVALID_HANDLE_VALUE 
			|| VolumeDev == NULL )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( DiskNumbers == NULL )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( Count == 0 )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( RetCount == NULL )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		BufSize = sizeof( VOLUME_DISK_EXTENTS ) + ( MAX_DISK_COUNT * sizeof( DISK_EXTENT ) ); 

		//for( ; ; )
		//{
		VolDiskExt = ( PVOLUME_DISK_EXTENTS )malloc( BufSize ); 
		if( VolDiskExt == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		_Ret = DeviceIoControl( VolumeDev, 
			IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, 
			NULL,   
			0, 
			VolDiskExt, 
			BufSize, 
			&RetSize, 
			( LPOVERLAPPED )NULL ); 

		if( _Ret == FALSE )
		{
			ErrorCode = GetLastError(); 

			//DbgPrintEx( "get the volume extens for the volume error %u", ErrorCode ); 

			if( ErrorCode == ERROR_MORE_DATA )
			{

			}
			//	//BufSize += sizeof; 

			//	if( SizeCorrect == TRUE )
			//	{
			//		break; 
			//	}
			//	else
			//	{
			//		SizeCorrect = TRUE; 
			//	}
			//}
			//else
			{
				Ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}
		//else
		//{
		//	break; 
		//}
		//}

		if( Ret != 0 )
		{
			break; 
		}

		*RetCount = VolDiskExt->NumberOfDiskExtents; 

		for( i = 0; ( ULONG )i < VolDiskExt->NumberOfDiskExtents; i ++ )
		{
			DiskNumbers[ i ] = VolDiskExt->Extents[ i ].DiskNumber; 

			if( ( ULONG )i >= Count )
			{
				DiskNumbers[ i ] = ( ULONG )INVALID_DISK_NO; 
			}
		}

		if( VolDiskExt->NumberOfDiskExtents > Count )
		{
			Ret = ERROR_BUFFER_TOO_SMALL; 
			break; 
		}

	}while( FALSE );

	if( VolDiskExt != NULL )
	{
		free( VolDiskExt ); 
	}
	return Ret; 
}

INT32 WINAPI IsUsbDevice( LPCSTR VolumeName ) 
{
	INT32 Ret = FALSE; 
	INT32 _Ret; 
	//LSTATUS __Ret;
	HANDLE VolumeDev = INVALID_HANDLE_VALUE; 
	ULONG RetSize = 0; 
	HRESULT hr; 
	ULONG BufSize; 
	INT32 i; 
	//ULONG ErrorCode; 

#define DEFAULT_INFO_SIZE ( sizeof( DRIVE_LAYOUT_INFORMATION ) + ( 26 * 4 ) * sizeof( PARTITION_INFORMATION ) )

	//PPARTITION_INFORMATION PartInfo; 
	CHAR DevName[ MAX_PATH ]; 
	PSTORAGE_DEVICE_NUMBER DiskNum; 
	PSTORAGE_ADAPTER_DESCRIPTOR AdapterDesc; 
	PVOLUME_DISK_EXTENTS VolDisk;

	//PDRIVE_LAYOUT_INFORMATION DiskLayout; 
	PVOID InfoBuf = NULL; 

	do 
	{
		BufSize = DEFAULT_INFO_SIZE; 

		if( VolumeName == NULL )
		{
			//Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		InfoBuf = malloc( BufSize ); 
		if( InfoBuf == NULL ) 
		{ 
			break; 
		} 

		//PartInfo = ( PPARTITION_INFORMATION )InfoBuf; 
		DiskNum = ( PSTORAGE_DEVICE_NUMBER )InfoBuf; 
		AdapterDesc = ( PSTORAGE_ADAPTER_DESCRIPTOR )InfoBuf; 
		VolDisk = ( PVOLUME_DISK_EXTENTS )InfoBuf; 

		hr = StringCbPrintfA( DevName, sizeof( DevName ), "\\\\.\\%s", VolumeName ); 
		if( FAILED( hr ) )
		{
			break; 
		}

		VolumeDev = CreateFileA( DevName,
			GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			NULL ); 

		if( VolumeDev == NULL 
			|| VolumeDev == INVALID_HANDLE_VALUE ) 
		{
			break; 
		}

		{
			STORAGE_PROPERTY_QUERY Query; 

			Query.PropertyId = StorageAdapterProperty;  
			Query.QueryType = PropertyStandardQuery;  

			AdapterDesc->Size = BufSize; 

			_Ret = DeviceIoControl( VolumeDev, 
				IOCTL_STORAGE_QUERY_PROPERTY, 
				&Query,   
				sizeof( STORAGE_PROPERTY_QUERY ), 
				AdapterDesc, 
				BufSize, 
				&RetSize, 
				( LPOVERLAPPED )NULL ); 

			if( _Ret == TRUE )
			{
				if( AdapterDesc->BusType == BusTypeUsb )
				{
					Ret = TRUE; 
					break; 
				}

				break; 
			}
		}

		do 
		{
			ULONG DiskCount; 
			ULONG DiskNos[ MAX_DISK_COUNT ]; 
			BOOLEAN IsUsbBus; 

			Ret = GetDiskNumbers( VolumeDev, DiskNos, ARRAYSIZE( DiskNos ), &DiskCount ); 
			if( Ret != 0 )
			{
				if( Ret != ERROR_BUFFER_TOO_SMALL )
				{
					break; 
				}
			}

			printf( "volume disk no count is %u\n", DiskCount ); 

			for( ; ; )
			{
				Sleep( 100 ); 
			}

			for( i = 0; i < ARRAYSIZE( DiskNos ); i ++ )
			{
				if( DiskNos[ i ] == INVALID_DISK_NO )
				{
					continue; 
				}

				Ret = CheckDiskUsbBusFromDiskNo( DiskNos[ i ], &IsUsbBus ); 
				if( Ret != 0 )
				{
					continue; 
				}

				if( IsUsbBus == TRUE )
				{
					Ret = TRUE; 
					break; 
				}
			}
		}while( FALSE ); 

		if( Ret == TRUE )
		{
			break; 
		}
		//
		//		{
		//			ULONG DiskIndexes[ MAX_DISK_COUNT ]; 
		//			ULONG UsbDiskCount; 
		//			ULONG i; 
		//			Ret = FindDiskFromBusType( "USBSTOR", DiskIndexes, ARRAYSIZE( DiskIndexes ), &UsbDiskCount ); 
		//			if( Ret != 0 )
		//			{
		//				break; 
		//			}
		//
		//			if( UsbDiskCount == 0 )
		//			{
		//				break; 
		//			}
		//
		//			memset( DiskNum, 0, sizeof( *DiskNum ) ); 
		//
		//			_Ret = DeviceIoControl( 
		//				VolumeDev, 
		//				IOCTL_STORAGE_GET_DEVICE_NUMBER, //IOCTL_DISK_GET_PARTITION_INFO, 
		//				NULL, 
		//				0, 
		//				( LPVOID )DiskNum, 
		//				( ULONG )BufSize, 
		//				( PULONG )&RetSize, 
		//				NULL ); 
		//
		//			if( FALSE == _Ret ) 
		//			{ 
		//#ifdef _DEBUG
		//				LPVOID ErrorMsg = NULL; 
		//
		//				FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		//					FORMAT_MESSAGE_FROM_SYSTEM | 
		//					FORMAT_MESSAGE_IGNORE_INSERTS, 
		//					NULL, 
		//					GetLastError(), 
		//					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language 
		//					( LPTSTR )&ErrorMsg, 
		//					0, 
		//					NULL ); 
		//
		//				if( ErrorMsg != NULL )
		//				{
		//					LocalFree( ErrorMsg ); 
		//				}
		//#endif //_DEBUG
		//				break; 
		//			} 
		//
		//			for( i = 0; i < UsbDiskCount; i ++ )
		//			{
		//				if( DiskIndexes[ i ] == DiskNum->DeviceNumber )
		//				{
		//					Ret = TRUE; 
		//					break; 
		//				}
		//			}
		//
		//			if( Ret == TRUE )
		//			{
		//				break; 
		//			}
		//		}
		//CloseHandle( VolumeDev ); 
		//VolumeDev = NULL; 
	}while( FALSE );

	if( VolumeDev != INVALID_HANDLE_VALUE )
	{
		CloseHandle( VolumeDev ); 
	}

	if( InfoBuf != NULL )
	{
		free( InfoBuf ); 
	}

	return Ret; 
} 

LRESULT WINAPI safe_disable_wow64( PVOID *wow_old_value )
{
	LRESULT ret = ERROR_NOT_SUPPORTED; 
	INT32 is_wow64 = FALSE; 
	//PVOID wow_old_value = NULL; 

	do 
	{
		is_wow64_proc( &is_wow64 ); 
		if( is_wow64 == TRUE )
		{
			ret = disable_wow_64( wow_old_value ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_FATAL_ERROR, "disable wow 64 error\n" ) ); 
			}
		}
		else
		{
			break; 
		}

	}while( FALSE );

	return ret; 
}

INT32 WINAPI get_windows_os_version_build_num()
{
	INT32 version = OSINVALID; 
	OSVERSIONINFO sInfo;
	sInfo.dwOSVersionInfoSize = sizeof(sInfo);
	GetVersionEx(&sInfo);

	switch(sInfo.dwBuildNumber)
	{
	case 2195:
		version = OSWIN2000; 
		break;
	case 2600:
		version = OSWINXP; 
		break;
	case 3790:
		version = OSWIN2003; 
		break;
	case 6000:
	case 6001:
	case 6002:
		version = OSWINVISTA; 
	case 7600:
		version = OSWIN7; 
		break;
	case 7601:
		version = OSWIN2008R2; 
			break;
	}
	return version; 
}

INT32 WINAPI get_windows_os_version() 
{ 
	INT32 nOSVersion = OSINVALID; 
	OSVERSIONINFOEX osversionInfo;

	ZeroMemory( &osversionInfo, sizeof(osversionInfo) );

	osversionInfo.dwOSVersionInfoSize = sizeof(osversionInfo);


	do 
	{
		if ( FALSE == GetVersionEx((LPOSVERSIONINFO)&osversionInfo) )
		{
			break;
		}

		switch( osversionInfo.dwMajorVersion )
		{
		case WIN_MAJOR_NT4_ME_98_95:
			switch( osversionInfo.dwMinorVersion )
			{
			case WIN_MINOR_2K8_VISTA_2K_NT4_95:
				if ( VER_PLATFORM_WIN32_NT == osversionInfo.dwPlatformId )
				{
					nOSVersion = OSOTHER;
				}

				break;

			case WIN_MINOR_98:
				nOSVersion = OSWIN98;
				break;

			case WIN_MINOR_ME:
				nOSVersion = OSOTHER;
				break;
			default:
				break;

			}
			break;

		case WIN_MAJOR_2K3R2_2K3_XP_2K:
			
			switch( osversionInfo.dwMinorVersion )
			{
			
			case WIN_MINOR_2K8_VISTA_2K_NT4_95:
				nOSVersion = OSWIN2000;
				break;
			
			case WIN_MINOR_WIN7_2K8R2_XP:
				nOSVersion = OSWINXP;
				break;

			case WIN_MINOR_2K3R2_2K3_XP64_WIN8_WIN2012:
				if ( VER_NT_WORKSTATION == osversionInfo.wProductType )
				{
					nOSVersion = OSWINXP;
					break;
				}
				if ( 0 != GetSystemMetrics( SM_SERVERR2 ) )
				{
					nOSVersion = OSWIN2003;
				}
				else
				{
					nOSVersion = OSWIN2003R2;
				}
				break;
			default:
				break;
			}
			break;

		case WIN_MAJOR_WIN7_2K8R2_2K8_VISTA_WIN8_WIN2012:
			switch( osversionInfo.dwMinorVersion )
			{
			case WIN_MINOR_2K8_VISTA_2K_NT4_95:
				if ( VER_NT_WORKSTATION == osversionInfo.wProductType )
				{
					nOSVersion = OSWINVISTA;
				}
				else
				{
					nOSVersion = OSWIN2008;
				}
				break;
			case WIN_MINOR_WIN7_2K8R2_XP:
				if ( VER_NT_WORKSTATION == osversionInfo.wProductType )
				{
					nOSVersion = OSWIN7;
				}
				else
				{
					nOSVersion = OSWIN2008R2;
				}
				break;


			case WIN_MINOR_2K3R2_2K3_XP64_WIN8_WIN2012:
				if ( VER_NT_WORKSTATION == osversionInfo.wProductType )
				{
					nOSVersion = OSWIN8;
				}
				else
				{
					nOSVersion = OSWIN2012;
				}
				break;

			default:
				break;
			}
			break;
		default:
			break;
		}
	}while( FALSE );

	return nOSVersion; 
} 
