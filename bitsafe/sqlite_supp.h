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

#ifndef __SQLITE_SUPP_H__
#define __SQLITE_SUPP_H__

#include "sqlite3.h"

#define SQL_STMT_LEN 1024
#define LOG_SQL_STMT_LEN 4096
#define DRV_CONF_SQL_STMT_LEN 256
#define GENERAL_SQL_STMT_LEN 512

typedef INT32 ( CALLBACK *row_data_callback )( sqlite3_stmt *stmt, ULONG type, PVOID callback_func, PVOID alloc_record_callback, PVOID param ); 

typedef struct _data_process
{
	row_data_callback data_proc_func; 
	PVOID callback_func; 
	PVOID alloc_func; 
	PVOID param; 
	ULONG type; 
} data_process, *pdata_process; 

typedef struct _string_output
{
	ULONG buf_size; 
	WCHAR *buf; 
} string_output, *pstring_output; 

INT32 CALLBACK output_result_string( sqlite3_stmt *stmt, ULONG type, PVOID callback_func, PVOID alloc_record_callback, PVOID param ); 

INT32 CALLBACK check_result_count( sqlite3_stmt *stmt, ULONG type, PVOID callback_func, PVOID alloc_record_callback, PVOID param ); 

LRESULT WINAPI check_table_columns_exist( sqlite3* db, LPCTSTR table_name, LPCTSTR colume_names[], ULONG colume_count ); 

INT32 WINAPI check_table_exist( sqlite3 *db, LPCTSTR table_name, INT32 *exist ); 

INT32 WINAPI sql_exec( sqlite3 *db, LPCTSTR sql, pdata_process data_proc ); 

INT32 WINAPI begin_transaction( sqlite3 *db ); 

INT32 WINAPI commit_transaction( sqlite3 *db ); 

INT32 WINAPI rollback_transaction( sqlite3 *db ); 

INT32 WINAPI set_sqlite3_basic_mode( sqlite3 *db ); 

INLINE INT32 set_db_to_unicode( sqlite3 *db )
{
	INT32 ret; 

	ret = sql_exec( ( sqlite3* )db, _T( "PRAGMA encoding = 'UTF-16';" ), NULL ); 
	if( ret != SQLITE_OK )
	{
		ret = ERROR_DB_CONFIG_ERROR; 
		goto _return; 
	}

_return:
	return ret; 
}

INLINE INT32 safe_create_table( sqlite3 *db, LPCTSTR tbl_name, LPCTSTR creat_sql )
{
	INT32 ret; 
	INT32 tbl_exist; 

	ret = check_table_exist( db, tbl_name, &tbl_exist ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	if( tbl_exist == FALSE )
	{
		ret = sql_exec( db, creat_sql, NULL ); 
		if( ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "create blocked table of the firewall failed error code: %d, reason:%ws\n", 
				ret, 
				sqlite3_errmsg16( db ) ) );

			goto _return; 
		}
	}

_return:
	return ret; 
}

INT32 create_unicode_db( LPCTSTR file_path, sqlite3 **handle ); 

INLINE INT32 close_db( PVOID handle )
{
	INT32 ret; 
	ASSERT( handle != NULL ); 
	ret = sqlite3_close( ( sqlite3* )handle ); 
	return ret; 
}

INLINE NTSTATUS _close_db( PVOID handle )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 

	do 
	{
		if( handle == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		ret = sqlite3_close( ( sqlite3* )handle ); 
		if( ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "close log data base failed error code: %d, reason:%s\n", 
				ret, 
				sqlite3_errmsg16( ( sqlite3* )handle ) ) ); 

			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}
	}while( FALSE );

	return ntstatus; 
}

#endif //__SQLITE_SUPP_H__