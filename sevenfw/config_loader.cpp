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
#include "config_loader.h"

CHAR filter_text_pool[ TEXT_POOL_LEN ]; 
CHAR filter_text_ln[ TEXT_POOL_LEN ]; 
ULONG filter_text_pool_readed;

LONG memmem( LPCSTR mem_find, LONG mem_find_len, LPCSTR mem, LONG memlen )
{
	INT32 i, j;

	if( mem_find_len > memlen )
		return -1;

	for( i = 0; i < memlen - mem_find_len + 1; i ++ )
	{
		for( j = 0; j < mem_find_len; j++)
		{
			if( mem_find[j] != mem[i + j])
			{
				goto _continue; 
			}
		}
		
		return i;
_continue:
		continue; 
	}
	return -1;
}

INT32 read_ln( HANDLE file, LPSTR lpszLine, LPSTR lpszRecvBuff, DWORD *pBufBegin, DWORD dwBufLen)
{	
	ULONG Readed; 
	CHAR *lpszTerm;	
	INT32 ret;
	DWORD dwSize = 0;
	INT32 nTermPos = -1;
	DWORD dwLnEndIndex;
	DWORD dwRecvedLen = *pBufBegin;
	DWORD dwMaxLnLen = dwBufLen;
	INT32 nLnEndIndex;

	ASSERT(file != INVALID_HANDLE_VALUE && pBufBegin != NULL);
	ASSERT(dwBufLen != 0);

	lpszTerm = ( LPSTR )LINE_TERM;

	do
	{
		if(dwRecvedLen > 0)
		{
			nTermPos = memmem( ( LPCSTR )lpszTerm, LINE_TERM_LEN, ( LPCSTR )lpszRecvBuff + dwSize, dwRecvedLen - dwSize);
			if(nTermPos >= 0)
			{
				nTermPos += dwSize;
			}

			if(lpszRecvBuff[dwRecvedLen - 1] == lpszTerm[0])
				dwSize = dwRecvedLen - 1;
			else
				dwSize = dwRecvedLen;

		}

		if(dwRecvedLen <= 0 || nTermPos < 0)
		{
			if((dwMaxLnLen - dwRecvedLen) > 0 )
			{

				ret = ReadFile( file, lpszRecvBuff + dwRecvedLen, dwMaxLnLen - dwRecvedLen, &Readed, NULL );
				if( ret == FALSE || Readed == 0 )
				{
					*lpszLine = 0;
					return ERR_DISCONNECT;
				}
				else
				{
					dwRecvedLen += Readed;
				}
			}
			else
			{
				*lpszLine = 0;
				return ERR_EXCEED_MAX_LN_LEN;
			}
		}
	}while(nTermPos < 0);

	memcpy(lpszLine, lpszRecvBuff, nTermPos);
	*( TCHAR* )( lpszLine + nTermPos )= 0;

	nLnEndIndex = nTermPos + LINE_TERM_LEN;

	memcpy(lpszRecvBuff, lpszRecvBuff + nLnEndIndex, dwRecvedLen - nLnEndIndex );
	memset( lpszRecvBuff + dwRecvedLen - nLnEndIndex, 0, nLnEndIndex ); 

	*pBufBegin = dwRecvedLen - nLnEndIndex;
	
	return nTermPos;
}

#define MAX_TEXT_LEN 1024
#define FILTER_TEXT_DELIM ':'

INT32 get_text_and_type( LPTSTR text_ln, LPTSTR text, ULONG text_len, ULONG *text_type )
{
	TCHAR *_type; 
	TCHAR *_type_end; 
	ULONG_PTR _text_len; 

	ASSERT( text != NULL ); 
	ASSERT( text_type != NULL ); 
	ASSERT( text_len > 0 );

	_type = _tcsrchr( text_ln, FILTER_TEXT_DELIM );
	if( _type == NULL )
	{
		return FORMAT_ERROR; 
	}

	if( _type - text_ln  + sizeof( TCHAR ) > text_len )
	{
		_text_len = text_len; 
	}
	else
	{
		_text_len = _type - text_ln  + sizeof( TCHAR ); 
	}

	memcpy( text, text_ln, _text_len - sizeof( TCHAR ) );
	text[ _text_len - 1 ] = _T( '\0' ); 
	_type += sizeof( CHAR ); 
	if( text_ln + _tcslen( text_ln ) - _type < 4 || 
		*_type < '0' || 
		*_type > '9' )
	{
		return FORMAT_ERROR; 
	}

	*text_type = _tcstol( _type, &_type_end, 10 ); 

	return 0; 
}

LRESULT del_filter_text_from_file( LPTSTR text, ULONG text_len, ULONG type, LPCTSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD _ret; 
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR filer_text_file_or_buff[ MAX_PATH ];
	DWORD Readed;
	TCHAR filt_rule[ MAX_TEXT_LEN ]; 
	ULONG text_type; 
	ULONG load_error; 

	ASSERT( NULL != text ); 
	ASSERT( NULL != file_name ); 

	ret = add_app_path_to_file_name( filer_text_file_or_buff, MAX_PATH, file_name, FALSE );

	if( ret != ERROR_SUCCESS )
	{
		goto _return;
	}

	hFile = ::CreateFile( filer_text_file_or_buff, 
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

	_ret = SetFilePointer( hFile, 0, 0, FILE_BEGIN ); 

		
	if( INVALID_SET_FILE_POINTER != _ret )
	{
		ret = GetLastError(); 
		if( ret != NO_ERROR )
		{
			goto _return;
		}
	}

	text_len = _sntprintf( filer_text_file_or_buff, 
		MAX_TEXT_LEN, 
		_T( "%s:%0.4d\r\n" ), 
		text, 
		type ); 

	load_error = 0; 
	for( ; ; )
	{
		ret = read_ln( hFile, 
			filter_text_ln, 
			filter_text_pool, 
			&filter_text_pool_readed, 
			TEXT_POOL_LEN ); 

		if( ret < 0 )
		{
			break; 
		}

		//ret = get_text_and_type( filter_text_ln, filt_rule, MAX_TEXT_LEN, &text_type ); 
		//if( ret < 0 )
		//{
		//	load_error ++; 
		//	continue; 
		//}
	}

	if( load_error > 0 )
	{
		ret = FORMAT_ERROR; 
		goto _return; 
	}

_return:
	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
	}
	return ret; 
}

LRESULT load_filter_text_from_file( treat_filter_text treat_func, PVOID param, LPCTSTR file_name )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	LRESULT ret;
	TCHAR filter_text_file[ MAX_PATH ];
	DWORD Readed;
	TCHAR filt_rule[ MAX_TEXT_LEN ]; 
	ULONG text_type; 
	ULONG load_error; 

	ASSERT( NULL != treat_func ); 

	ret = add_app_path_to_file_name( filter_text_file, MAX_PATH, file_name, FALSE );

	if( ERROR_SUCCESS != ret )
	{
		goto _return;
	}

	hFile = ::CreateFile( filter_text_file, 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		SAFE_SET_ERROR_CODE( ret );
		goto _return;
	}

	if( 0 != ::SetFilePointer( hFile, 0, 0, FILE_BEGIN ) )
	{
		SAFE_SET_ERROR_CODE( ret ); 
		goto _return;
	}

	load_error = 0; 
	for( ; ; )
	{
		ret = ( LRESULT )read_ln( hFile, 
			filter_text_ln, 
			filter_text_pool, 
			&filter_text_pool_readed, 
			TEXT_POOL_LEN ); 

		if( ret < 0 )
		{
			break; 
		}

		//Ret = get_text_and_type( filter_text_ln, filt_rule, MAX_TEXT_LEN, &text_type ); 
		//if( Ret < 0 )
		//{
		//	load_error ++; 
		//	continue; 
		//}

		treat_func( ( TCHAR* )filter_text_ln, 0, 0, param ); 
	}

	if( load_error > 0 )
	{
		ret = FORMAT_ERROR; //ERROR_FILE_INVALID; 
		goto _return; 
	}

_return:
	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
	}
	return ret; 
}

INT32 save_filter_to_file( LPCTSTR text, ULONG type, LPCTSTR file_name )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	INT32 ret = 0;
	BOOL _ret;
	TCHAR filer_text_file_or_buff[ MAX_TEXT_LEN + 4 ];
	DWORD Writed;
	ULONG text_len; 


	ASSERT( NULL != file_name ); 

	ret = add_app_path_to_file_name( filer_text_file_or_buff, MAX_TEXT_LEN + 4, file_name, FALSE  );
	if( ERROR_SUCCESS != ret )
	{
		goto _return;
	}

	hFile = ::CreateFile( filer_text_file_or_buff, 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{	
		hFile = ::CreateFile( filer_text_file_or_buff, 
			GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ, 
			NULL, 
			CREATE_ALWAYS, 
			FILE_ATTRIBUTE_NORMAL, 
			NULL );

		if( INVALID_HANDLE_VALUE == hFile )
		{
			ret = -1; 
			goto _return;
		}
	}

	SetFilePointer( hFile, 0, 0, FILE_END ); 

	text_len = ( ULONG )_tcslen( text ); 
	if( text_len > MAX_TEXT_LEN )
	{
		( ( LPTSTR )text )[ MAX_TEXT_LEN - 1 ] = _T( '\0' ); 
	}

	text_len = _sntprintf( filer_text_file_or_buff, 
		MAX_TEXT_LEN, 
		_T( "%s:%0.4d\r\n" ), 
		text, 
		type ); 

	if( text_len < 0 )
	{
		ret = text_len; 
		goto _return; 
	}

	_ret = WriteFile( hFile, filer_text_file_or_buff, text_len, &Writed, NULL );
	if( _ret == FALSE || Writed < sizeof( text_len ) )
	{
		ret = -1; 
		goto _return; 
	}
_return:
	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
	}
	return ret;
}

INT32 reset_filter_text_file( LPCTSTR file_name )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR filer_text_file_or_buff[ MAX_TEXT_LEN + 4 ];

	ASSERT( NULL != file_name ); 
	
	ret = add_app_path_to_file_name( filer_text_file_or_buff, MAX_TEXT_LEN + 4, file_name, FALSE );
	if( ERROR_SUCCESS != ret )
	{
		goto _return;
	}

	hFile = ::CreateFile( filer_text_file_or_buff, 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		ret = -1; 
		goto _return;
	}

	ret = 0; 
	goto _return; 

_return:
	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
	}
	return ret; 
}