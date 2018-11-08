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

#include "common_func.h"
#include "sqlite3.h"
#include "sql_stmt_def.h"
#include "sqlite_supp.h"

#define BITSAFE_DB_KEY "*111013#$BitSafe^" 
PVOID *pwd_variable = NULL; 

#define SQL_STMT_LEN 1024
#define LOG_SQL_STMT_LEN 4096
#define DRV_CONF_SQL_STMT_LEN 256
#define GENERAL_SQL_STMT_LEN 512

INT32 CALLBACK check_result_count( sqlite3_stmt *stmt, ULONG type, PVOID callback_func, PVOID alloc_record_callback, PVOID param )
{
	ASSERT( param != NULL ); 

	return *( PINT32 )param = sqlite3_column_int( stmt, 0 ); 
}

INT32 CALLBACK output_result_string( sqlite3_stmt *stmt, ULONG type, PVOID callback_func, PVOID alloc_record_callback, PVOID param )
{
	INT32 ret = ERROR_SUCCESS; 
	HRESULT hr; 
	string_output *output; 
	LPCWSTR colume_text; 

	ASSERT( param != NULL ); 

	do 
	{
		output = ( string_output* )param; 

		colume_text = ( LPCWSTR )sqlite3_column_text16( stmt, 0 ); 

		if( colume_text == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = StringCbCopyW( output->buf, output->buf_size, colume_text ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_table_columns_exist( sqlite3* db, LPCTSTR table_name, LPCTSTR colume_names[], ULONG colume_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HRESULT hr; 
	TCHAR colume_name[ MAX_PATH ]; 

#define MAX_COLUMES_NAME_LEN 512 
#define MAX_SELECT_COLUME_SQL_STRING_LEN 512
	TCHAR all_collumes[ MAX_COLUMES_NAME_LEN ]; 
	TCHAR sql_stmt[ MAX_SELECT_COLUME_SQL_STRING_LEN ]; 

	data_process data_proc = { 0 }; 
	string_output output; 
	INT32 i; 

	do 
	{

		if( db == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( table_name == NULL || table_name[ 0 ] == _T( '0' ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( NULL == colume_names 
			|| colume_count == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = StringCbPrintf( sql_stmt, sizeof( sql_stmt ), _T("select sql from sqlite_master where tbl_name=\'%s\' and  type=\'table\'"), table_name ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			break; 
		}

		all_collumes[ 0 ] = L'\0'; 
		output.buf = all_collumes; 
		output.buf_size = sizeof( all_collumes ); 

		data_proc.data_proc_func = output_result_string; 
		data_proc.param = &output; 

		_ret = sql_exec( db, sql_stmt, &data_proc ); 
		if( ret != SQLITE_OK )
		{
			dbg_print( MSG_FATAL_ERROR, "%s insert new record failed: %d, reason %s\n", __FUNCTION__, ret, sqlite3_errmsg16( db ) );

			ret = ERROR_DB_INSERT_ROW_ERROR; 
			break; 
		}

		if( L'\0' == all_collumes[ 0 ] )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_tcslwr( all_collumes ); 

		for( i = 0; ( ULONG )i < colume_count; i ++ )
		{
			hr = StringCbCopy( colume_name, sizeof( colume_name ), colume_names[ i ] ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_OVERFLOW; 
				break; 
			}

			_tcslwr( colume_name ); 
			if( NULL == wcsstr( all_collumes, colume_name ) )
			{
				ret = ERROR_NOT_FOUND; 
				break; 
			}
		}

	}while( FALSE );

	return ret; 
}

INT32 WINAPI check_table_exist( sqlite3 *db, LPCTSTR table_name, INT32 *exist )
{
	INT32 ret = SQLITE_OK; 
	TCHAR sql_stmt[ SQL_STMT_LEN ]; 
	data_process data_proc = { 0 }; 

	ASSERT( db != NULL ); 
	ASSERT( table_name != NULL ); 
	ASSERT( exist != NULL ); 

	*exist = 0; 

	ret = _sntprintf( sql_stmt, SQL_STMT_LEN, SQL_CHECK_TABLE_EXIST, table_name ); 
	if( ret <= 0 )
	{
		ret = SQLITE_ERROR; 
		goto _return; 
	}

	data_proc.data_proc_func = check_result_count; 
	data_proc.param = exist; 

	ret = sql_exec( db, sql_stmt, &data_proc ); ; 

_return:
	return ret; 
}

INT32 WINAPI sql_exec( sqlite3 *db, LPCTSTR sql, pdata_process data_proc )
{
	INT32 ret = SQLITE_OK; 
	sqlite3_stmt *stmt = NULL; 

	ASSERT( db != NULL ); 

	ret = sqlite3_prepare16( db, sql, ( INT32 )_tcslen( sql ) * sizeof( WCHAR ), &stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
		log_trace( ( MSG_ERROR, "sqlite3_prepare( %d ): %ws\n", 
			ret, 
			sqlite3_errmsg16( db ) ) ); 
		
		goto _return; 
	}
	ret = sqlite3_step(stmt);

	if( ( ret != SQLITE_OK ) 
		&& ( ret != SQLITE_DONE ) 
		&& ( ret != SQLITE_ROW ) ) 
	{
		log_trace( ( MSG_ERROR, "sqlite3_step( %d ): %ws %ws \n", 
			ret, 
			sql, 
			sqlite3_errmsg16( db ) ) ); 
		
		goto _return; 
	}


	while( ret == SQLITE_ROW )
	{
		if( data_proc != NULL 
			&& data_proc->data_proc_func != NULL )
		{
			ret = data_proc->data_proc_func( stmt, data_proc->type, data_proc->callback_func, data_proc->alloc_func, data_proc->param ); 
			if( ret == SQLITE_ABORT )
			{
				break; 
			}
		} 

		ret = sqlite3_step( stmt );
	}

_return: 
	if( stmt != NULL )
	{
		ret = sqlite3_finalize( stmt );
		if( ret != SQLITE_OK ) 
		{
			log_trace( ( MSG_ERROR, "sqlite3_finalize( %d ): %ws\n", 
				ret, 
				sqlite3_errmsg16( db ) ) );
		}
	}

	return ret;
}

INT32 WINAPI set_sqlite3_basic_mode( sqlite3 *db )
{
	INT32 ret = SQLITE_OK; 
	INT32 _ret; 

	do 
	{
		_ret = sql_exec( db, _T( "PRAGMA default_cache_size=9012" ), NULL ); 
		if( _ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "set default_cache_size error %u %ws\n", 
				_ret, 
				sqlite3_errmsg16( db ) ) ); 
		}

		_ret = sql_exec( db, _T( "PRAGMA temp_store=2" ), NULL ); 
		if( _ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "set temp_store error %u %ws\n", 
				_ret, 
				sqlite3_errmsg16( db ) ) ); 
		}

	} while ( FALSE );

	return ret; 
}

/********************************************************************************************
1.支持64位问题：
	1.性能方面比较好的选择是直接编译为64位版本。但需要将所有的关联项目都编译为64位版本，需要
	投入大量的处理平台支持兼容性问题的编译时间。

	2.比较简单的处理方法是使用关闭64位WOW重定向的方法，优点是更改简单，缺点是不能发挥64位系统
	的性能优势，有些细节功能需要做出大的调整。(比如InstallHinfSection如果在32位程序下运行是会
	直接重定向至WOW目录下的)。
2.调试版本使用关闭WOW64重定向的方法不处理。

********************************************************************************************/
#ifndef _WIN64
static void __declspec( naked ) pwd_func( void )
{
	__asm
	{
		mov		eax, dword ptr[ esp ]
		mov		ecx, eax; 
		shr		eax, 0x10; 
		and		ecx, 0xFFFF; 
		mov		dword ptr[ esp ], eax; 
		lea		eax, [ offset pwd_variable + ecx * 4 ];
		call	dword ptr[ eax ]; 
		pop		ecx; 
		cmp		ecx, 0x1000; 
		ja		callOriginalHandler; 
		pop		edx; 
		add		esp, ecx; 
		jmp		edx; 

callOriginalHandler:

		jmp		ecx
	}
}
#else

static VOID pwd_func( void )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	CHAR print_msg[] = "index = %u\n"; 

	do 
	{
		for( i = 0; i < 100; i ++ )
		{
			dbg_print( MSG_FATAL_ERROR, print_msg, i ); 
		}

	}while( FALSE );

	return; 
}

#endif //_WIN64

//#define ENCODE_LOG_DB 1
INT32 create_unicode_db( LPCTSTR file_path, sqlite3 **handle )
{
	INT32 ret; 
	sqlite3 *db; 

	*handle = NULL; 

#ifdef _UNICODE 
	ret = sqlite3_open16( file_path, &db ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}
#else
	ret = sqlite3_open( file_path, &db ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}
#endif //_UNICODE

#ifdef ENCODE_LOG_DB
	ret = sqlite3_key( db, ( CHAR* )pwd_func, 20 ); 
	if( ret != SQLITE_OK )
	{
		log_trace( ( MSG_INFO, "set db key failed %d \n", ret ) ); 
		goto _return; 
	}


	{
		INT32 test_rekey = FALSE; 
		if( test_rekey == TRUE )
		{
			ret = sqlite3_rekey( db, NULL, 0 ); 
			if( ret != SQLITE_OK )
			{
				log_trace( ( MSG_INFO, "set db key failed %d %ws\n", ret, sqlite3_errmsg16( db ) ) ); 
				goto _return; 
			}

			ret = sqlite3_key( db, BITSAFE_DB_KEY, CONST_STR_LEN( BITSAFE_DB_KEY ) ); 
			if( ret != SQLITE_OK )
			{
				log_trace( ( MSG_INFO, "set db key failed %d %ws\n", ret, sqlite3_errmsg16( db ) ) ); 
				goto _return; 
			}
		}
	}

#endif //ENCODE_LOG_DB

	ret = sqlite3_extended_result_codes( db, TRUE ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	ret = set_db_to_unicode( db ); 
	if( ret!= SQLITE_OK )
	{
		goto _return; 
	}

	ret = set_sqlite3_basic_mode( db ); 
	if( ret!= SQLITE_OK )
	{
		goto _return; 
	}
	
	*handle = db; 

_return:
	return ret; 
}

INT32 WINAPI begin_transaction( sqlite3 *db )
{
	INT32 ret; 
	ret = sql_exec( db, _T( "BEGIN TRANSACTION" ), NULL ); 

	return ret; 
}

INT32 WINAPI commit_transaction( sqlite3 *db )
{
	INT32 ret; 
	ret = sql_exec( db, _T("COMMIT TRANSACTION"), NULL );
	return ret; 
}

INT32 WINAPI rollback_transaction( sqlite3 *db )
{
	INT32 ret; 
	ret = sql_exec( db, _T("ROLLBACK TRANSACTION"), NULL ); 
	;
	return ret;
}