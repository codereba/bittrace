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
#include "acl_define.h"
#include "action_type_parse.h"
#include "sqlite_supp.h"
#include "action_log_db.h"
#include "action_log_sql.h"
#include "stack_dump.h"

#define MAX_COND_STRING_LEN MAX_PATH 

LRESULT WINAPI check_table_columns_exist( sqlite3* db, LPCTSTR table_name, LPCTSTR colume_names[], ULONG colume_count ); 

INT32 CALLBACK read_action_log_ex( sqlite3_stmt *stmt, 
								  ULONG type, 
								  PVOID read_callback_func, 
								  PVOID alloc_func, 
								  PVOID param ); 

LRESULT WINAPI check_log_db_struct( sqlite3 *db )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	INT32 _ret; 
	WCHAR sql_stmt[ MAX_PATH ]; 
	LPCWSTR colume_names[] = { L"id", 
		L"type", 
		L"proc_name", 
		L"proc_id", 
		L"thread_id", 
		L"param0", 
		L"param1", 
		L"param2", 
		L"param3", 
		L"param4", 
		L"param5", 
		L"param6", 
		L"param7", 
		L"stack", 
		L"data", 
		L"data_len", 
		L"result", 
		L"time" }; 

	do 
	{
		if( NULL == db )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = check_table_columns_exist( db, COMMON_ACTION_TABLE_NAME, colume_names, ARRAYSIZE( colume_names ) ); 
		if( ret == ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ret;  
}

LRESULT WINAPI filter_action_log( sys_action_record *action, 
								 action_context *ctx, 
								 PVOID data, 
								 ULONG data_size ); 

/******************************************************************************
���¼�ͨ��ģ�ͽ������¼ܹ�:
1.�¼�����ֻ�����¼�Դ�ı�־����,����ID,�߳�ID(�����ټ���Ψһ����:����ʱ���,������ʾΨһ��)
2.���������¼�Դ������,���¼�������,�ڷ����¼�ʱ,ͨ���¼���Դ��ID����λ�¼�Դ��

NATIVE·�������·���е��̷�·���Ĺ�����ʽ,���ں���ʵ��:
���ļ�ϵͳ�������������еĶ�Ӧ��ϵ������TRACELOG,TRACELOG
�н�������·��ӳ���,������¼���־ʱ,����·��ת��.

******************************************************************************/
sqlite3 *common_log_db = NULL;  

static NTSTATUS WINAPI read_blob_data( sqlite3_blob *blob, PVOID buf, ULONG buf_len, ULONG *ret_len ); 

NTSTATUS CALLBACK read_action_from_db_row( sqlite3_stmt *stmt, sys_action_record *action ); 

INT32 CALLBACK read_action_log( sqlite3_stmt *stmt, 
							   ULONG type, 
							   PVOID read_callback_func, 
							   PVOID alloc_func, 
							   PVOID param ); 

NTSTATUS WINAPI restore_action_log_db_backup( LPCWSTR file_path )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( file_path != NULL ); 
	}while( FALSE );

	return ntstatus; 
}

void sqlite_db_log( PVOID context, INT32 result_code, LPCSTR error_msg )
{


}

void sqlite_db_profile( PVOID context, LPCSTR sql, ULONGLONG delay )
{
	SYSTEMTIME cur_time; 
	GetSystemTime( &cur_time ); 
	
	dbg_print( MSG_IMPORTANT, "%04u-%02u-%02u %02u:%02u:%02u.%03u sql %s use time %I64u\n", 
		cur_time.wYear, 
		cur_time.wMonth, 
		cur_time.wDay, 
		cur_time.wHour, 
		cur_time.wMinute, 
		cur_time.wSecond, 
		cur_time.wMilliseconds, 
		sql, 
		delay ); 
}

WCHAR log_db_file_name[ MAX_PATH ] = { 0 }; 
ULONG log_db_file_name_len = 0; 
ULONG log_db_dir_name_len = 0; 
HANDLE bittrace_proc_id = INVALID_PROCESS_ID; 

NTSTATUS WINAPI open_action_log_db( LPCWSTR file_path, PVOID *handle )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret = 0; 
	HRESULT hr; 
	INT32 _ret; 
	sqlite3 *db = NULL; 

	do 
	{

		ASSERT( file_path != NULL ); 
		ASSERT( handle != NULL ); 

		*handle = NULL; 

		ret = create_unicode_db( file_path, &db ); 
		if( ret != SQLITE_OK )
		{
			break; 
		}

		/**************************************************************
		PostgreSQL:    4.373 
		MySQL:    0.114 
		SQLite 2.7.6:    13.061 
		SQLite 2.7.6 (nosync):    0.223 
		**************************************************************/

		do 
		{
			WCHAR sql_stmt[ MAX_PATH ]; 

			ret = check_log_db_struct( db ); 
			if( ret == ERROR_SUCCESS )
			{
				break; 
			}

			// ALTER TABLE table_name
			// ADD column_name datatype

			dbg_print( MSG_FATAL_ERROR, "log database structure is not correct\n" ); 
#if 0
			{
				WCHAR backup_file_name[ MAX_PATH ]; 

				ret = sqlite3_close( db ); 
				if( ret != SQLITE_OK )
				{
					log_trace( ( MSG_ERROR, "close log data base failed error code: %d, reason:%s\n", 
						ret, 
						sqlite3_errmsg16( db ) ) ); 

					ntstatus = STATUS_UNSUCCESSFUL; 
					break; 
				}

				do 
				{
					hr = StringCbCopyW( backup_file_name, sizeof( backup_file_name ), file_path ); 
					if( FAILED( hr ) )
					{
						break; 
					}

					hr = StringCbCatW( backup_file_name, sizeof( backup_file_name ), L".bak" ); 
					if( FAILED( hr ) )
					{
						break; 
					}

					_ret = DeleteFileW( backup_file_name ); 
					if( FALSE == _ret )
					{

					}

					_ret = MoveFileW( file_path, backup_file_name ); 
					if( FALSE == _ret )
					{
						break; 
					}

				}while( FALSE );


				ret = create_unicode_db( file_path, &db ); 
				if( ret != SQLITE_OK )
				{
					break; 
				}
			}
#endif //0
			
		}while( FALSE );

		ret = safe_create_table( db, COMMON_ACTION_TABLE_NAME, SQL_CREATE_COMMON_ACTION_TABLE ); 
		if( ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "create log table for common action logging failed error code: %d, reason:%s\n", 
				ret, 
				sqlite3_errmsg16( db ) ) ); 
			
			break; 
		}

		/*********************************************************************************************
			ID�����ᵽ��ROWID����ROWID�������Դ��������ģ�����������ܲ�����Ҫ����������
		*********************************************************************************************/
		_ret = sql_exec( db, L"create index " COMMON_ACTION_TABLE_ID_INDEX_NAME L" on " COMMON_ACTION_TABLE_NAME L"(id)", NULL ); 
		if( _ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "create log table for common action logging failed error code: %d, reason:%ws\n", 
				ret, 
				sqlite3_errmsg16( db ) ) ); 
		}

		_ret = sql_exec( db, L"create index " COMMON_ACTION_TABLE_TIME_INDEX_NAME L" on " COMMON_ACTION_TABLE_NAME L"(time)", NULL  ); 
		if( _ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "create log table for common action logging failed error code: %d, reason:%s\n", 
				ret, 
				sqlite3_errmsg16( db ) ) ); 
		}

		_ret = sql_exec( db, L"create index " COMMON_ACTION_TABLE_COMMON_INDEX_NAME L" on " COMMON_ACTION_TABLE_NAME L"(type,proc_name,proc_id,thread_id,param0,param1,param2,param3,param4,param5,param6,param7,stack,data,data_len)", NULL  ); 
		if( _ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "create log table for common action logging failed error code: %d, reason:%s\n", 
				ret, 
				sqlite3_errmsg16( db ) ) ); 
		}

#ifdef _DEBUG
		sqlite3_set_profile( db, sqlite_db_profile, NULL ); 
		//sqlite3_config( SQLITE_CONFIG_LOG, sqlite_db_log, NULL ); 
#endif //_DEBUG
	
		{
			HRESULT hr; 
			LPCWSTR file_name; 

			file_name = wcsrchr( ( WCHAR* )file_path, L'\\' ); 
			if( file_name == NULL )
			{
				DBG_BP(); 
				file_name = file_path; 
			}

			hr = StringCbCopy( log_db_file_name, sizeof( log_db_file_name ), file_name ); 
			if( FAILED( hr ) )
			{
				log_trace( ( MSG_IMPORTANT, "get the file name of the log database error %u\n", hr ) ); 
				DBG_BP(); 
				break; 
			}

			log_db_dir_name_len = ( ULONG )( file_name - file_path ); 
			log_db_file_name_len = wcslen( log_db_file_name ); 
			bittrace_proc_id = ( HANDLE )GetCurrentProcessId(); 
		}

		*handle = db; 

	}while( FALSE );

	if( ret != SQLITE_OK )
	{
		if( db != NULL )
		{
			sqlite3_close( db ); 
		}
	}

	return ntstatus; 
}

NTSTATUS WINAPI close_action_log_db( PVOID handle )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ntstatus = _close_db( handle ); 

	}while( FALSE );

	return ntstatus; 
}

LPCWSTR param_sel_mode_string[ PARAM_SEL_MODE_COUNT ] = {
	L"or", 
		L"and", 
		L"not", 
}; 

NTSTATUS WINAPI create_param_fmt_string( sys_action_record *action, 
								 LPWSTR fmt_string, 
								 ULONG ccb_buf_len, 
								 ULONG *ccb_ret_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	
	do 
	{

	}while( FALSE );
	
	return ntstatus; 
}

LPCWSTR WINAPI get_table_name_from_action_type( sys_action_type type )
{
	LPCWSTR table_name = L""; 

	switch( type )
	{
		
	default:
		table_name = COMMON_ACTION_TABLE_NAME; 
		break; 
	}

	return table_name; 
}

NTSTATUS WINAPI get_sql_param_string( param_info *param, 
						  LPWSTR param_text, 
						  ULONG ccb_buf_len, 
						  ULONG *ccb_ret_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 
	ULONG _ccb_ret_len = 0; 

	do 
	{
		ASSERT( param != NULL ); 
		ASSERT( param_text != NULL ); 
		ASSERT( ccb_buf_len > 0 ); 
		ASSERT( ccb_ret_len != NULL ); 

		switch( param->type )
		{
		case INT8_TYPE:
			ret = _snwprintf( param_text,  
				ccb_buf_len, 
				L"%d", 
				param->data.int8_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}

			break; 
		case INT16_TYPE:
			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"%d", 
				param->data.int16_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}

			break; 
		case INT32_TYPE:
			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"%d", 
				param->data.int32_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}
			break; 
		case INT64_TYPE:
			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"%I64d", 
				param->data.int64_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}
			break; 
		case UINT8_TYPE:
			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"%u", 
				param->data.uint8_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}
			break; 
		case UINT16_TYPE:
			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"%u", 
				param->data.uint16_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}
			break; 
		case UINT32_TYPE:
			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"%u", 
				param->data.uint32_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}
			break; 
		case UINT64_TYPE:
			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"%I64u", 
				param->data.uint32_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}
			break; 
		case PTR_TYPE:
			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"%I64u", 
				param->data.ptr_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}
			break; 
		case STRING_PARAM:
			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"'%s'", 
				param->data.string_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}
			break; 
		case WSTRING_TYPE:
			if( L'\0' == *param->data.wstring_val )
			{
			}

			ret = _snwprintf( param_text, 
				ccb_buf_len, 
				L"'%s'", 
				param->data.wstring_val ); 

			if( ret < 0 )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
			else if( ret == ccb_buf_len )
			{
				param_text[ ccb_buf_len - 1 ] = L'\0'; 

				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}
			else
			{
				_ccb_ret_len = ( ULONG )ret; 
			}
			break; 
		case DATA_BLOB_TYPE:

			if( ccb_buf_len < 2 )
			{
				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}

			param_text[ 0 ] = L'?'; 
			param_text[ 1 ] = L'\0'; 

			ccb_buf_len = 2; 
			break; 
		default:
			ASSERT( FALSE ); 
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}
		
		*ccb_ret_len = _ccb_ret_len; 

	}while( FALSE );

	return ntstatus; 
}

LPCWSTR WINAPI get_param_sel_cond_string( param_sel_mode sel_mode )
{
	LPCWSTR cond_string = L""; 

	do 
	{
		if( FALSE == invalid_sel_mode( sel_mode ) )
		{
			break; 
		}

		cond_string = param_sel_mode_string[ ( ULONG )sel_mode ]; 
	}while( FALSE );

	return cond_string; 
}

NTSTATUS WINAPI construct_select_sql( LPWSTR sql_out, 
							  ULONG ccb_buf_len, 
							  action_context *ctx, 
							  sys_action_type type, 
							  param_info all_params[], 
							  param_sel_mode all_conds[], 
							  ULONG count, 
							  ULONG offset, 
							  ULONG limit )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 
	INT32 i; 
	LPCWSTR table_name = L""; 
#define MAX_PARAM_TEXT_LEN 2048
	WCHAR *param_text = NULL; 
	ULONG param_text_len; 
	LPCWSTR cond_desc; 
#define MAX_COND_STRING_LEN MAX_PATH 
	WCHAR cond_string[ MAX_COND_STRING_LEN ]; 
	
	ULONG output_len; 
	ULONG param_output; 
	BOOLEAN where_added = FALSE;  

	do 
	{
		do 
		{
			table_name = get_table_name_from_action_type( type ); 

			if( *table_name == L'\0' )
			{
				ntstatus = STATUS_INVALID_PARAMETER_1; 
				break; 
			}

			sql_out[ ccb_buf_len - 1 ] = L'\0';  

			ret = _snwprintf( sql_out, ccb_buf_len, L"select * from %s", table_name ); 
			if( ret < 0 )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
			else if( ( ULONG )ret >= ccb_buf_len )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			output_len = 0; 

			param_text = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_PARAM_TEXT_LEN ); 
			if( param_text == NULL )
			{
				ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
				break; 
			}

			param_output = 0; 

			if( type != INVALID_SYS_ACTION_TYPE )
			{
				if( where_added == FALSE )
				{
					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 

					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
					{
						ntstatus = STATUS_BUFFER_OVERFLOW; 
						break; 
					}

					where_added = TRUE; 
				}

				wcsncat( sql_out, L" type = ", ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				ret = _snwprintf( param_text, 
					MAX_PARAM_TEXT_LEN, 
					L"%u", 
					type ); 

				if( ret < 0 )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_UNSUCCESSFUL; 
					break; 
				}
				else if( ret == MAX_PARAM_TEXT_LEN )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_BUFFER_TOO_SMALL; 
					break; 
				}

				wcsncat( sql_out, 
					param_text, 
					ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				param_output += 1; 
			}

			if( ctx != NULL && *ctx->proc_name != L'\0' )
			{
				if( where_added == FALSE )
				{
					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 

					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
					{
						ntstatus = STATUS_BUFFER_OVERFLOW; 
						break; 
					}

					where_added = TRUE; 

					ASSERT( param_output == 0 ); 
				}

				if( param_output > 0 )
				{
					wcsncat( sql_out, L" and proc_name = ", ccb_buf_len - wcslen( sql_out ) ); 
				}
				else
				{
					wcsncat( sql_out, L" proc_name = ", ccb_buf_len - wcslen( sql_out ) ); 
				}

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				ret = _snwprintf( param_text, 
					MAX_PARAM_TEXT_LEN, 
					L"'%s'", 
					ctx->proc_name ); 

				if( ret < 0 )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_UNSUCCESSFUL; 
					break; 
				}
				else if( ret == MAX_PARAM_TEXT_LEN )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_BUFFER_TOO_SMALL; 
					break; 
				}

				wcsncat( sql_out, 
					param_text, 
					ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				param_output += 1; 
			}

			if( ctx != NULL && ctx->proc_id != 0 )
			{
				if( where_added == FALSE )
				{
					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 

					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
					{
						ntstatus = STATUS_BUFFER_OVERFLOW; 
						break; 
					}

					where_added = TRUE; 

					ASSERT( param_output == 0 ); 
				}

				if( param_output > 0 )
				{
					wcsncat( sql_out, L" and proc_id = ", ccb_buf_len - wcslen( sql_out ) ); 
				}
				else
				{
					wcsncat( sql_out, L" proc_id = ", ccb_buf_len - wcslen( sql_out ) ); 
				}

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				ret = _snwprintf( param_text, 
					MAX_PARAM_TEXT_LEN, 
					L"%u", 
					ctx->proc_id ); 

				if( ret < 0 )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_UNSUCCESSFUL; 
					break; 
				}
				else if( ret == MAX_PARAM_TEXT_LEN )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_BUFFER_TOO_SMALL; 
					break; 
				}

				wcsncat( sql_out, 
					param_text, 
					ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				param_output += 1; 
			}

			if( ctx != NULL && ctx->thread_id != 0 )
			{
				if( where_added == FALSE )
				{
					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 

					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
					{
						ntstatus = STATUS_BUFFER_OVERFLOW; 
						break; 
					}

					where_added = TRUE; 

					ASSERT( param_output == 0 ); 
				}

				if( param_output > 0 )
				{
					wcsncat( sql_out, L" and thread_id = ", ccb_buf_len - wcslen( sql_out ) ); 
				}
				else
				{
					wcsncat( sql_out, L" thread_id = ", ccb_buf_len - wcslen( sql_out ) ); 
				}

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				ret = _snwprintf( param_text, 
					MAX_PARAM_TEXT_LEN, 
					L"%u", 
					ctx->thread_id ); 

				if( ret < 0 )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_UNSUCCESSFUL; 
					break; 
				}
				else if( ret == MAX_PARAM_TEXT_LEN )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_BUFFER_TOO_SMALL; 
					break; 
				}

				wcsncat( sql_out, 
					param_text, 
					ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				param_output += 1; 
			}

			if( ctx != NULL && ctx->last_result != RESULT_IGNORE )
			{
				if( where_added == FALSE )
				{
					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 

					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
					{
						ntstatus = STATUS_BUFFER_OVERFLOW; 
						break; 
					}

					where_added = TRUE; 

					ASSERT( param_output == 0 ); 
				}

				if( param_output > 0 )
				{
					wcsncat( sql_out, L" and result = ", ccb_buf_len - wcslen( sql_out ) ); 
				}
				else
				{
					wcsncat( sql_out, L" result = ", ccb_buf_len - wcslen( sql_out ) ); 
				}

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				ret = _snwprintf( param_text, 
					MAX_PARAM_TEXT_LEN, 
					L"%u", 
					ctx->last_result ); 

				if( ret < 0 )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_UNSUCCESSFUL; 
					break; 
				}
				else if( ret == MAX_PARAM_TEXT_LEN )
				{
					ASSERT( FALSE ); 
					ntstatus = STATUS_BUFFER_TOO_SMALL; 
					break; 
				}

				wcsncat( sql_out, 
					param_text, 
					ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				param_output += 1; 
			}

			if( count == 0 )
			{
				break; 
			}
			else
			{
				if( where_added == FALSE )
				{
					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 

					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
					{
						ntstatus = STATUS_BUFFER_OVERFLOW; 
						break; 
					}

					where_added = TRUE; 

					ASSERT( param_output == 0 ); 
				}
			}

			if( param_output > 0 )
			{
				wcsncat( sql_out, 
					L" and ", 
					ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}
			}

			for( i = 0; ( ULONG )i < count; i ++ )
			{
				if( all_conds[ i ] == SELECT_NOT )
				{
					continue; 
				}

				if( i != 0 )
				{
					cond_desc = get_param_sel_cond_string( all_conds[ i ] ); 
					if( *cond_desc == L'\0' )
					{
						ASSERT( FALSE && "input invalid parameter condition?" ); 
						continue; 
					}
				}
				else
				{
					cond_desc = L""; 
				}

				ntstatus = get_sql_param_string( &all_params[ i ], 
					param_text, 
					MAX_PARAM_TEXT_LEN, 
					&param_text_len ); 

				if( ntstatus != STATUS_SUCCESS )
				{
					log_trace( ( MSG_ERROR, "can't get the parameter text of the type %u\n", all_params[ i ].type ) ); 
					continue; 
				}

				_snwprintf( cond_string, 
					ARRAY_SIZE( cond_string ), 
					L"%s param%u = ", 
					cond_desc, 
					i ); 

				wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				output_len = ( ULONG )wcslen( sql_out ); 

				if( output_len + 2 >= ccb_buf_len )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				sql_out[ output_len ] = L' '; 
				sql_out[ output_len + 1 ] = L'\0'; 
			}

		}while( FALSE ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		if( limit > 0 )
		{
#define MAX_INT_TEXT_LEN 12
			//WCHAR offset[ MAX_INT_TEXT_LEN ]; 

			if( where_added == FALSE )
			{
				wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}

				where_added = TRUE; 

				ASSERT( param_output == 0 ); 
			}
			else
			{
				wcsncat( sql_out, L" and", ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}
			}

			ret = _snwprintf( cond_string, 
				ARRAY_SIZE( cond_string ), 
				L" rowid >= %u", 
				offset ); 

			wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 

			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			if( limit != LIMIT_INFINITE)
			{
				ret = _snwprintf( cond_string, 
					ARRAY_SIZE( cond_string ), 
					L" and rowid < %u", 
					offset + limit ); 

				wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 

				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}
			}
		}
	}while( FALSE );


	if( param_text != NULL )
	{
		free( param_text ); 
	}

	return ntstatus; 
}

NTSTATUS WINAPI construct_delete_sql( LPWSTR sql_out, 
							  ULONG ccb_buf_len, 
							  action_context *ctx, 
							  sys_action_type type, 
							  param_info all_params[], 
							  param_sel_mode all_conds[], 
							  ULONG count )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 
	INT32 i; 
	LPCWSTR table_name = L""; 
#define MAX_PARAM_TEXT_LEN 2048
	WCHAR *param_text = NULL; 
	ULONG param_text_len; 
	LPCWSTR cond_desc; 
#define MAX_COND_STRING_LEN MAX_PATH 
	WCHAR cond_string[ MAX_COND_STRING_LEN ]; 
	ULONG output_len; 

	do 
	{
		table_name = get_table_name_from_action_type( type ); 

		if( *table_name == L'\0' )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		sql_out[ ccb_buf_len - 1 ] = L'\0';  

		ret = _snwprintf( sql_out, ccb_buf_len, L"delete from %s where", table_name ); 
		if( ret < 0 )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}
		else if( ( ULONG )ret >= ccb_buf_len )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}

		output_len = 0; 

		param_text = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_PARAM_TEXT_LEN ); 
		if( param_text == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		for( i = 0; ( ULONG )i < count; i ++ )
		{
			if( all_conds[ i ] == SELECT_NOT )
			{
				continue; 
			}

			if( i != 0 )
			{
				cond_desc = get_param_sel_cond_string( all_conds[ i ] ); 
				if( *cond_desc == L'\0' )
				{
					ASSERT( FALSE && "input invalid parameter condition?" ); 
					continue; 
				}
			}
			else
			{
				cond_desc = L""; 
			}

			ntstatus = get_sql_param_string( &all_params[ i ], 
				param_text, 
				MAX_PARAM_TEXT_LEN, 
				&param_text_len ); 

			if( ntstatus != STATUS_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "can't get the parameter text of the type %u\n", all_params[ i ].type ) ); 
				continue; 
			}

			_snwprintf( cond_string, 
				ARRAY_SIZE( cond_string ), 
				L"%s param%u = ", 
				cond_desc, 
				i ); 

			wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 

			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 

			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			output_len = wcslen( sql_out ); 

			if( output_len + 2 >= ccb_buf_len )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			sql_out[ output_len ] = L' '; 
			sql_out[ output_len + 1 ] = L'\0'; 
		}

	}while( FALSE );

	if( param_text != NULL )
	{
		free( param_text ); 
	}
	return ntstatus; 
}

NTSTATUS WINAPI construct_insert_sql( LPWSTR sql_out, 
							  ULONG ccb_buf_len, 
							  action_context *ctx, 
							  sys_action_type type, 
							  param_info all_params[], 
							  param_colume_info all_columes[], 
							  param_sel_mode all_conds[], 
							  ULONG count )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 
	INT32 i; 
	LPCWSTR table_name = L""; 
	WCHAR *param_text = NULL; 
	ULONG param_text_len; 
	WCHAR fmt_string[ MAX_COND_STRING_LEN ]; 
	WCHAR *_sql_out; 
	size_t remain_len; 
	ULONG output_len; 
	HRESULT hr; 

	do 
	{
		table_name = get_table_name_from_action_type( type ); 

		if( *table_name == L'\0' )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( ctx == NULL )
		{
		}

		sql_out[ ccb_buf_len - 1 ] = L'\0';  

		param_text = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_PARAM_TEXT_LEN ); 
		if( param_text == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		hr = StringCchPrintfExW( sql_out, ccb_buf_len, &_sql_out, &remain_len, 0, L"insert into %s ( ", table_name ); 
		if( FAILED( hr ) 
			|| ( remain_len <= 1 ) )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}

		ASSERT( _sql_out != NULL ); 

		hr = StringCchCatExW( _sql_out, remain_len, L"type, ", &_sql_out, &remain_len, 0 ); 

		if( FAILED( hr ) 
			|| ( remain_len <= 1 ) )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}
		
		ASSERT( _sql_out != NULL ); 

		if( ctx != NULL && *ctx->proc_name != L'\0' )
		{
			hr = StringCchCatExW( _sql_out, remain_len, L"proc_name, ", &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
		}

		if( ctx != NULL && ctx->proc_id != 0 )
		{
			hr = StringCchCatExW( _sql_out, remain_len, L"proc_id, ", &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
		}

		if( ctx != NULL && ctx->thread_id != 0 )
		{
			hr = StringCchCatExW( _sql_out, remain_len, L"thread_id, ", &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
		}

		for( i = 0; ( ULONG )i < count; i ++ )
		{
			if( all_conds[ i ] == SELECT_NOT )
			{
				continue; 
			}

			if( all_params[ i ].type != DATA_BLOB_TYPE )
			{
				if( all_columes[ i ].colume_name != NULL )
				{
					if( ( ULONG )i < count - 1 )
					{
						hr = StringCchPrintfExW( fmt_string, 
							ARRAY_SIZE( fmt_string ), 
							NULL, 
							NULL, 
							0, 
							L"%s, ", 
							all_columes[ i ].colume_name ); 
					}
					else
					{
						hr = StringCchPrintfExW( fmt_string, 
							ARRAY_SIZE( fmt_string ), 
							NULL, 
							NULL, 
							0, 
							L"%s", 
							all_columes[ i ].colume_name ); 
					}
				}
				else
				{
					if( ( ULONG )i < count - 1 )
					{
						hr = StringCchPrintfExW( fmt_string, 
							ARRAY_SIZE( fmt_string ), 
							NULL, 
							NULL, 
							0, 
							L"param%u, ", 
							i ); 
					}
					else
					{
						hr = StringCchPrintfExW( fmt_string, 
							ARRAY_SIZE( fmt_string ), 
							NULL, 
							NULL, 
							0, 
							L"param%u", 
							i ); 
					}
				}
			}
			else
			{
				hr = StringCchPrintfExW( fmt_string, 
					ARRAY_SIZE( fmt_string ), 
					NULL, 
					NULL, 
					0, 
					L"data" ); 
			}

			if( FAILED( hr ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			hr = StringCchCatExW( _sql_out, remain_len, fmt_string, &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			ASSERT( _sql_out != NULL ); 
		}

		if( ctx != NULL && ctx->last_result != STATUS_UNSUCCESSFUL )
		{
			hr = StringCchCatExW( _sql_out, remain_len, L", result ", &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
		}

		if( ctx != NULL && ctx->time.QuadPart != 0 )
		{
			hr = StringCchCatExW( _sql_out, remain_len, L", time ", &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
		}

		hr = StringCchCatExW( _sql_out, remain_len, L" ) VALUES ( ", &_sql_out, &remain_len, 0 ); 

		if( FAILED( hr ) 
			|| ( remain_len <= 1 ) )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}

		hr = StringCchPrintfExW( param_text, 
			MAX_PARAM_TEXT_LEN, 
			NULL, 
			NULL, 
			0, 
			L"%u, ", 
			type ); 

		if( FAILED( hr ) )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}

		hr = StringCchCatExW( _sql_out, remain_len, param_text, &_sql_out, &remain_len, 0 ); 

		if( FAILED( hr ) 
			|| ( remain_len <= 1 ) )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}

		if( ctx != NULL && *ctx->proc_name != L'\0' )
		{
			hr = StringCchPrintfExW( param_text, 
				MAX_PARAM_TEXT_LEN, 
				NULL, 
				NULL, 
				0, 
				L"'%s', ", 
				ctx->proc_name ); 

			if( FAILED( hr ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			hr = StringCchCatExW( _sql_out, remain_len, param_text, &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
		}

		if( ctx != NULL && ctx->proc_id != 0 )
		{
#ifdef _DEBUG
			if( ctx->proc_id > 65535 )
			{
				DBG_BP(); 
			}
#endif //_DEBUG

			hr = StringCchPrintfExW( param_text, 
				MAX_PARAM_TEXT_LEN, 
				NULL, 
				NULL, 
				0, 
				L"%u, ", 
				ctx->proc_id ); 


			if( FAILED( hr ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			hr = StringCchCatExW( _sql_out, remain_len, param_text, &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
		}

		if( ctx != NULL && ctx->thread_id != 0 )
		{
#ifdef _DEBUG
			if( ctx->thread_id > 65535 )
			{
				DBG_BP(); 
			}
#endif //_DEBUG

			if( count > 0 )
			{
				hr = StringCchPrintfExW( param_text, 
					MAX_PARAM_TEXT_LEN, 
					NULL, 
					NULL, 
					0, 
					L"%u, ", 
					ctx->thread_id ); 
			}
			else
			{
				hr = StringCchPrintfExW( param_text, 
					MAX_PARAM_TEXT_LEN, 
					NULL, 
					NULL, 
					0, 
					L"%u", 
					ctx->thread_id ); 
			}

			if( FAILED( hr ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			hr = StringCchCatExW( _sql_out, remain_len, param_text, &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
		}

		for( i = 0; ( ULONG )i < count; i ++ )
		{
			if( all_conds[ i ] == SELECT_NOT )
			{
				continue; 
			}

#ifdef _DEBUG
			switch( all_params[ i ].type )
			{

			case INT32_TYPE: 
			case INT64_TYPE: 
			case UINT32_TYPE: 
			case UINT64_TYPE: 
			case PTR_TYPE: 
				if( type != EXEC_module_load 
					&& all_params[ i ].data.uint32_val >= 0x80000000 )
				{
				}
				break; 
			default:
				break; 
			}
#endif //_DEBUG

			ntstatus = get_sql_param_string( &all_params[ i ], 
				param_text, 
				MAX_PARAM_TEXT_LEN, 
				&param_text_len ); 

			if( ntstatus != STATUS_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "can't get the parameter text of the type %u\n", all_params[ i ].type ) ); 

				if( ntstatus == STATUS_BUFFER_TOO_SMALL )
				{
					if( all_params[ i ].type == WSTRING_TYPE 
						|| all_params[ i ].type == STRING_PARAM 
						|| all_params[ i ].type == UNICODE_STRING_TYPE 
						|| all_params[ i ].type == ANSI_STRING_TYPE )
					{
						param_text[ MAX_PARAM_TEXT_LEN - 2 ] = L'\''; 
						param_text[ MAX_PARAM_TEXT_LEN - 1 ] = L'\0'; 
					}
				}
				else
				{
					break; 
				}
			}

			hr = StringCchCatExW( _sql_out, remain_len, param_text, &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}

			if( ( ULONG )i < count - 1 )
			{
				hr = StringCchCatExW( _sql_out, remain_len, L", ", &_sql_out, &remain_len, 0 ); 

				if( FAILED( hr ) 
					|| ( remain_len <= 1 ) )
				{
					ntstatus = STATUS_BUFFER_OVERFLOW; 
					break; 
				}
			}
		}

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		if( ctx != NULL && ctx->last_result != STATUS_UNSUCCESSFUL )
		{
			INT32 _ret; 
			HRESULT hr; 


			hr = StringCbPrintf (param_text, MAX_PARAM_TEXT_LEN * sizeof( WCHAR ),
				L", %u", 
				ctx->last_result ); 
		}

		if( FAILED( hr ) )
		{
			ASSERT( FALSE ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}
		hr = StringCchCatExW( _sql_out, remain_len, param_text, &_sql_out, &remain_len, 0 ); 

		if( FAILED( hr ) 
			|| ( remain_len <= 1 ) )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}

		if( ctx != NULL && ctx->time.QuadPart != 0 )
		{
			SYSTEMTIME sys_time; 
			FILETIME file_time; 

			INT32 _ret; 
			HRESULT hr; 

			file_time.dwHighDateTime = ctx->time.HighPart; 
			file_time.dwLowDateTime = ctx->time.LowPart; 


			_ret = FileTimeToSystemTime( &file_time, &sys_time ); 
			if( _ret == FALSE )
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "convert the log time to system time error %u\n" ) ); 
				break; 
			}

			if( count > 0 )
			{
				hr = StringCbPrintf (param_text, MAX_PARAM_TEXT_LEN * sizeof( WCHAR ),
					L", '%04u/%02u/%02u %02u:%02u:%02u.%04u'",
					sys_time.wYear,
					sys_time.wMonth,
					sys_time.wDay,
					sys_time.wHour,
					sys_time.wMinute,
					sys_time.wSecond, 
					( file_time.dwLowDateTime % 10000 ) ); 

				//ret = _snwprintf( param_text, 
				//	MAX_PARAM_TEXT_LEN, 
				//	L"%u, ", 
				//	ctx->thread_id ); 
			}
			else
			{
				hr = StringCbPrintf (param_text, MAX_PARAM_TEXT_LEN * sizeof( WCHAR ),
					L", '%04u/%02u/%02u %02u:%02u:%02u.%04u'", 
					sys_time.wYear,
					sys_time.wMonth,
					sys_time.wDay,
					sys_time.wHour,
					sys_time.wMinute,
					sys_time.wSecond, 
					( file_time.dwLowDateTime % 10000 ) );

				//ret = _snwprintf( param_text, 
				//	MAX_PARAM_TEXT_LEN, 
				//	L"%u", 
				//	ctx->thread_id ); 
			}

			if( FAILED( hr ) )
			{
				ASSERT( FALSE ); 
				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}

			hr = StringCchCatExW( _sql_out, remain_len, param_text, &_sql_out, &remain_len, 0 ); 

			if( FAILED( hr ) 
				|| ( remain_len <= 1 ) )
			{
				ntstatus = STATUS_BUFFER_OVERFLOW; 
				break; 
			}
		}

		hr = StringCchCatExW( _sql_out, remain_len, L" ) ", &_sql_out, &remain_len, 0 ); 

		if( FAILED( hr ) 
			|| ( remain_len <= 1 ) )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}

	}while( FALSE );

	if( param_text != NULL )
	{
		free( param_text ); 
	}

	return ntstatus; 
}

NTSTATUS WINAPI collect_exec_module_load_param( sqlite3_stmt *stmt, exec_module_load *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG


		action->base = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->size = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_exec_module_load_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG


		action->do_exec_module_load.base = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->do_exec_module_load.size = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->do_exec_module_load.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( exec_module_load ) 
			+ ( ( action->do_exec_module_load.path_len + 1 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}
		
		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		memcpy( action->do_exec_module_load.path_name, text, 
			( ( action->do_exec_module_load.path_len + 1 ) << 1 ) ); 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI collect_file_touch_param( sqlite3_stmt *stmt, file_touch *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //�ļ�����Ȩ�� 
		action->alloc_size = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //�ļ���ʼ���� 
		action->attrib = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //�ļ����� 
		action->share = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�ļ��������� 
		action->disposition = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //�ļ���/����ѡ�� 
		action->options = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //�ļ���/����ѡ�� 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 6 ); //������ɽ��NTSTATUS
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 7 ); //�ļ�ȫ·�� 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_file_touch_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_file_touch.access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //�ļ�����Ȩ�� 
		action->do_file_touch.alloc_size = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //�ļ���ʼ���� 
		action->do_file_touch.attrib = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //�ļ����� 
		action->do_file_touch.share = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�ļ��������� 
		action->do_file_touch.disposition = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //�ļ���/����ѡ�� 
		action->do_file_touch.options = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //�ļ���/����ѡ�� 
		action->do_file_touch.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 6 ); //������ɽ��NTSTATUS
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( file_touch ) + ( ( action->do_file_touch.path_len + 1 ) << 1 ); 

		if( action->size > buf_len )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 7 ); //�ļ�ȫ·�� 

		memcpy( action->do_file_touch.path_name, 
			text, 
			( action->do_file_touch.path_len + 1 ) << 1 ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI collect_file_open_param( sqlite3_stmt *stmt, file_open *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG


		action->access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //�ļ�����Ȩ�� 
		action->alloc_size = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //�ļ���ʼ���� 
		action->attrib = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //�ļ����� 
		action->share = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�ļ��������� 
		action->disposition = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //�ļ���/����ѡ�� 
		action->options = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //�ļ���/����ѡ��  
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 6 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 7 ); //�ļ�ȫ·�� 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_file_open_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG


		action->do_file_open.access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //�ļ�����Ȩ�� 
		action->do_file_open.alloc_size = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //�ļ���ʼ���� 
		action->do_file_open.attrib = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //�ļ����� 
		action->do_file_open.share = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�ļ��������� 
		action->do_file_open.disposition = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //�ļ���/����ѡ�� 
		action->do_file_open.options = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //�ļ���/����ѡ�� 
		action->do_file_open.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 6 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( file_open ) + ( ( action->do_file_open.path_len + 1 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 7 ); //�ļ�ȫ·�� 
		
		memcpy( action->do_file_open.path_name, 
			text, 
			( action->do_file_open.path_len + 1 ) << 1 ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI collect_exec_destroy_param( sqlite3_stmt *stmt, exec_destroy *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->cmd_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		action->cmd_line_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_exec_destroy_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_exec_destroy.pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->do_exec_destroy.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->do_exec_destroy.cmd_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( exec_destroy ) 
			+ ( ( action->do_exec_destroy.path_len + action->do_exec_destroy.cmd_len + 2 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		ASSERT( text[ action->do_exec_destroy.path_len ] == L'\0' ); 
		memcpy( action->do_exec_destroy.path_name, text, ( action->do_exec_destroy.path_len + 1 ) << 1 ); 
		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 
		ASSERT( text[ action->do_exec_destroy.cmd_len ] == L'\0' ); 
		memcpy( action->do_exec_destroy.path_name + action->do_exec_destroy.path_len + 1, text, ( ( action->do_exec_destroy.cmd_len + 1 ) << 1 ) ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI collect_file_read_param( sqlite3_stmt *stmt, _file_read *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->offset = ( ULONG )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_file_read_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_file_read.offset = ( ULONG )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->do_file_read.data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->do_file_read.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( _file_read ) 
			+ ( ( action->do_file_read.path_len + 1 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

		memcpy( action->do_file_read.path_name, 
			text, 
			( action->do_file_read.path_len + 1 ) << 1 ); 
			
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_file_write_param( sqlite3_stmt *stmt, _file_write *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->offset = ( ULONG )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_file_write_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_file_write.offset = ( ULONG )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->do_file_write.data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->do_file_write.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( _file_write ) + ( ( action->do_file_write.path_len + 1 ) << 1 ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

		memcpy( action->do_file_write.path_name, 
			text, 
			( action->do_file_write.path_len + 1 ) << 1 ); 
		\
	}while( FALSE );

	return ntstatus; 
}

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
NTSTATUS WINAPI collect_modify_file_param( sqlite3_stmt *stmt, access_file_info *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX ); 

	}while( FALSE );

	return ntstatus; 
}

#define collect_access_file_param collect_modify_file_param
#define collect_delete_file_param collect_modify_file_param
#define collect_access_com_param collect_modify_file_param

NTSTATUS WINAPI collect_modify_key_param( sqlite3_stmt *stmt, 
								  access_reg_info *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI collect_create_proc_param( sqlite3_stmt *stmt, 
								  manage_proc_info *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX ); 

	}while( FALSE );

	return ntstatus; 
}

#define collect_terminate_proc_param collect_create_proc_param
#define collect_install_hook_param collect_create_proc_param
#define collect_access_mem_param collect_create_proc_param

NTSTATUS WINAPI collect_install_drv_param( sqlite3_stmt *stmt, 
								   manage_driver_info *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI collect_socket_create_param( sqlite3_stmt *stmt, 
								   ipv4_socket_action *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->src_ip = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->dest_ip = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->src_port = ( USHORT )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->dest_port = ( USHORT )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->prot = ( prot_type )( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->result = ( socket_action_ret )( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
	}while( FALSE );

	return ntstatus; 
}

#define collect_socket_connect_param collect_socket_create_param
#define collect_socket_listen_param collect_socket_create_param
#define collect_socket_send_param collect_socket_create_param
#define collect_socket_recv_param collect_socket_create_param
#define collect_socket_accept_param collect_socket_create_param

NTSTATUS WINAPI collect_locate_url_param( sqlite3_stmt *stmt, 
									 access_url_info *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( FALSE ); 

		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->domain_name = ( LPCSTR )sqlite3_column_text( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->path_name = ( LPCSTR )sqlite3_column_text( stmt, ACTION_PARAM_FIELD_INDEX ); 
	}while( FALSE );

	return ntstatus; 
}

#define collect_icmp_recv_param collect_icmp_send_param 

NTSTATUS WINAPI collect_icmp_send_param( sqlite3_stmt *stmt, 
								  ipv4_icmp_action *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->src_ip = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->dest_ip = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->type = ( BYTE )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->code = ( BYTE )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
	}while( FALSE );

	return ntstatus; 
}

#endif //COMPATIBLE_OLD_ACTION_DEFINE

NTSTATUS WINAPI collect_file_modified_param( sqlite3_stmt *stmt, file_modified *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		//ULONG pid; //���������߽���ID 
		//ULONG tid; //�����������߳�ID 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX ); //�ļ�ȫ·�� 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_file_modified_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_file_modified.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 

		action->size = ACTION_RECORD_SIZE_BY_TYPE( file_modified ) + ( ( action->do_file_modified.path_len + 1 ) << 1 ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX ); //�ļ�ȫ·�� 

		memcpy( action->do_file_modified.path_name, 
				text, 
				( ( action->do_file_modified.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_file_remove_param( sqlite3_stmt *stmt, file_remove *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //�ļ�ȫ·�� 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_file_remove_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_file_remove.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 

		action->size = ACTION_RECORD_SIZE_BY_TYPE( file_modified ) + ( ( action->do_file_remove.path_len + 1 ) << 1 ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX ); //�ļ�ȫ·�� 

		memcpy( action->do_file_remove.path_name, 
			text, 
			( ( action->do_file_remove.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_file_rename_param( sqlite3_stmt *stmt, file_rename *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG
		action->replace_existing = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //�Ƿ񸲸��Ѵ����ļ� 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->new_name_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		action->new_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_file_rename_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_file_rename.replace_existing = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //�Ƿ񸲸��Ѵ����ļ� 
		action->do_file_rename.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->do_file_rename.new_name_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( file_rename ) 
			+ ( ( action->do_file_rename.path_len + action->do_file_rename.new_name_len + 2 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		
		memcpy( action->do_file_rename.path_name, 
			text, 
			( action->do_file_rename.path_len + 1 ) << 1 ); 
		
		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 

		memcpy( action->do_file_rename.path_name + action->do_file_rename.path_len + 1, 
			text, 
			( action->do_file_rename.new_name_len + 1 ) << 1 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_reg_open_key_param( sqlite3_stmt *stmt, reg_openkey *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //ע�����/����Ȩ�� 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //ע�����·�� 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_reg_open_key_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_reg_openkey.access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //ע�����/����Ȩ�� 
		action->do_reg_openkey.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( reg_openkey ) + ( ( action->do_reg_openkey.path_len + 1 ) << 1 ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //ע�����·�� 

		memcpy( action->do_reg_openkey.path_name, 
			text, 
			( action->do_reg_openkey.path_len + 1 ) << 1 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_reg_mk_key_param( sqlite3_stmt *stmt, reg_mkkey *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //ע�����/����Ȩ�� 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //ע�����·�� 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_reg_mk_key_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_reg_mkkey.access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //ע�����/����Ȩ�� 
		action->do_reg_mkkey.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( reg_mkkey ) + ( ( action->do_reg_mkkey.path_len + 1 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //ע�����·�� 

		memcpy( action->do_reg_mkkey.path_name, 
			text, 
			( action->do_reg_mkkey.path_len + 1 ) << 1 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_reg_rm_key_param( sqlite3_stmt *stmt, reg_rmkey *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //ע�����·�� 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_reg_rm_key_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG
		action->do_reg_rmkey.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 

		action->size = ACTION_RECORD_SIZE_BY_TYPE( reg_rmkey ) + ( ( action->do_reg_rmkey.path_len + 1 ) << 1 );  
		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //ע�����·�� 

		memcpy( action->do_reg_rmkey.path_name, 
			text, 
			( ( action->do_reg_rmkey.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_reg_mv_key_param( sqlite3_stmt *stmt, reg_mvkey *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->new_name_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->new_key_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_reg_mv_key_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG
		action->do_reg_mvkey.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->do_reg_mvkey.new_name_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( reg_mvkey ) 
			+ ( ( action->do_reg_mvkey.path_len + action->do_reg_mvkey.new_name_len + 2 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		memcpy( action->do_reg_mvkey.path_name, 
			text, 
			( action->do_reg_mvkey.path_len + 1 ) << 1 ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		memcpy( action->do_reg_mvkey.path_name + ( action->do_reg_mvkey.path_len + 1 ) , 
			text, 
			( action->do_reg_mvkey.new_name_len + 1 ) << 1 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_reg_rm_val_param( sqlite3_stmt *stmt, reg_rmval *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->key_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //ע�����·�� 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_reg_rm_val_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG


		action->do_reg_rmval.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( reg_rmval ) + ( ( action->do_reg_rmval.path_len + 1 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //ע�����·�� 

		memcpy( action->do_reg_rmval.path_name, 
			text, 
			( action->do_reg_rmval.path_len + 1 ) << 1 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_proc_exec_param( sqlite3_stmt *stmt, proc_exec *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //���ݳ��� 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //ע���ֵ·�� 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_proc_exec_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_proc_exec.target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //���ݳ��� 

		action->do_proc_exec.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( proc_exec ) + ( ( action->do_proc_exec.path_len + 1 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //ע���ֵ·�� 

		memcpy( action->do_proc_exec.path_name, text, ( action->do_proc_exec.path_len + 1 ) << 1 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_reg_set_val_param( sqlite3_stmt *stmt, reg_setval *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->type = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //ע���ֵ���� 
		action->data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //���ݳ��� 
		action->val_name_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		action->key_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //ע���ֵ·�� 
		action->value_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //ע�����·���� 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_reg_set_val_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_reg_setval.type = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //ע���ֵ���� 
		action->do_reg_setval.data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //���ݳ��� 
		action->do_reg_setval.val_name_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->do_reg_setval.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

		action->size = ACTION_RECORD_SIZE_BY_TYPE( reg_setval ) 
			+ ( ( action->do_reg_setval.path_len + action->do_reg_setval.val_name_len + 2 ) << 1 ); 
		
		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //ע���ֵ·�� 
		memcpy( action->do_reg_setval.path_name, 
			text, 
			( action->do_reg_setval.path_len + 1 ) << 1 ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //ע�����·���� 

		memcpy( action->do_reg_setval.path_name + action->do_reg_setval.path_len + 1, 
			text, 
			( action->do_reg_setval.val_name_len + 1 ) << 1 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_reg_get_val_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->type = ( sys_action_type )( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //ע���ֵ���� 
		action->do_reg_getval.info_size = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //���ݳ��� 
		action->do_reg_getval.val_name_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->do_reg_getval.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( reg_getval ) + ( ( action->do_reg_getval.path_len + action->do_reg_getval.val_name_len + 2 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //ע���ֵ·�� 
		memcpy( action->do_reg_getval.path_name, 
			text, 
			( ( action->do_reg_getval.path_len + 1 ) << 1 ) ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //ע�����·���� 
		memcpy( action->do_reg_getval.path_name + action->do_reg_getval.path_len + 1, 
			text, 
			( ( action->do_reg_getval.val_name_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_reg_get_val_param( sqlite3_stmt *stmt, 
										  reg_getval *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->type = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //ע���ֵ���� 
		action->info_size = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //���ݳ��� 
		action->val_name_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		action->key_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //ע���ֵ·�� 
		action->value_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //ע�����·���� 

	}while( FALSE ); 

	return ntstatus; 
}

//#define collect_reg_get_val_param( __stmt, __action ) collect_reg_set_val_param( __stmt, ( reg_getval* )__action ) 

NTSTATUS WINAPI collect_proc_write_vm( sqlite3_stmt *stmt, proc_writevm *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->base = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //д���ڴ��ַ 
		action->data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //��ͼд�볤�� 
		action->bytes_written = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�ɹ�д�볤�� 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_proc_write_vm( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_proc_writevm.target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->do_proc_writevm.base = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //д���ڴ��ַ 
		action->do_proc_writevm.data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //��ͼд�볤�� 
		action->do_proc_writevm.bytes_written = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�ɹ�д�볤�� 
		action->do_proc_writevm.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( proc_writevm ) + ( ( action->do_proc_writevm.path_len + 1 ) << 1 ); 
		
		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); 

		memcpy( action->do_proc_writevm.path_name, 
			text, 
			( ( action->do_proc_writevm.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_proc_read_vm( sqlite3_stmt *stmt, proc_readvm *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->base = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //д���ڴ��ַ 
		action->data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //��ͼд�볤�� 
		action->bytes_read = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�ɹ�д�볤�� 
		action->result = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //������ɽ��(NTSTATUS) 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 6 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_proc_read_vm( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_proc_readvm.target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->do_proc_readvm.base = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //д���ڴ��ַ 
		action->do_proc_readvm.data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //��ͼд�볤�� 
		action->do_proc_readvm.bytes_read = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�ɹ�д�볤�� 
		action->do_proc_readvm.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( proc_readvm ) + ( ( action->do_proc_readvm.path_len + 1 ) << 1 ); 
		
		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 6 ); 
		
		memcpy( action->do_proc_readvm.path_name, 
			text, 
			( ( action->do_proc_readvm.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_thrd_remote( sqlite3_stmt *stmt, thrd_remote *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->target_tid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //Ŀ���߳�ID 
		action->access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //�̴߳���Ȩ�� 
		action->suspended = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�Ƿ����ʽ���� 
		action->start_vaddr = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //�߳���ʼ��ַ 
		action->thread_param = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //�̲߳��� 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 6 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 7 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_thrd_remote( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_thrd_remote.target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->do_thrd_remote.target_tid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //Ŀ���߳�ID 
		action->do_thrd_remote.access = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //�̴߳���Ȩ�� 
		action->do_thrd_remote.suspended = ( BOOLEAN )( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); //�Ƿ����ʽ���� 
		action->do_thrd_remote.start_vaddr = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //�߳���ʼ��ַ 
		action->do_thrd_remote.thread_param = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); //�̲߳��� 
		action->do_thrd_remote.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 6 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( thrd_remote ) + ( ( action->do_thrd_remote.path_len + 1 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 7 ); 
		memcpy( action->do_thrd_remote.path_name, 
			text, 
			( ( action->do_thrd_remote.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_thrd_set_ctxt( sqlite3_stmt *stmt, thrd_setctxt *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->target_tid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //Ŀ���߳�ID 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_thrd_set_ctxt( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_thrd_setctxt.target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->do_thrd_setctxt.target_tid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //Ŀ���߳�ID 
		action->do_thrd_setctxt.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( thrd_setctxt ) + ( ( action->do_thrd_setctxt.path_len + 1 ) << 1 ); 
		
		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

		memcpy( action->do_thrd_setctxt.path_name, 
			text, 
			( ( action->do_thrd_setctxt.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_w32_lib_inject( sqlite3_stmt *stmt, w32_lib_inject *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->result = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //������ɽ��(NTSTATUS) 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		action->lib_path_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_w32_lib_inject( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR  text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_w32_lib_inject.target_pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->do_w32_lib_inject.result = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //������ɽ��(NTSTATUS) 
		action->do_w32_lib_inject.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( w32_lib_inject ) + ( ( action->do_w32_lib_inject.path_len + 1 ) << 1 ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

		memcpy( action->do_w32_lib_inject.path_name, 
			text, 
			( ( action->do_w32_lib_inject.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_net_create_param( sqlite3_stmt *stmt, net_create *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		//IP_ADDR_T local_ip_addr; 
		//PORT_T local_port; 
		//ULONG protocol; //Э�� 
		////NTSTATUS result; //������ɽ��(NTSTATUS) 
		//IP_ADDR_T ip_addr; 
		//PORT_T port; 

		action->local_ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->local_port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //Ŀ�����ID 
		action->protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //Ŀ�����ID 
		action->ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		action->port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_net_create_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		//IP_ADDR_T local_ip_addr; 
		//PORT_T local_port; 
		//ULONG protocol; //Э�� 
		////NTSTATUS result; //������ɽ��(NTSTATUS) 
		//IP_ADDR_T ip_addr; 
		//PORT_T port; 

		action->do_net_create.local_ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->do_net_create.local_port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //Ŀ�����ID 
		action->do_net_create.protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //Ŀ�����ID 
		action->do_net_create.ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		action->do_net_create.port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_net_connect_param( sqlite3_stmt *stmt, net_connect *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG


		action->protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_net_connect_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG


		action->do_net_connect.protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->do_net_connect.ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->do_net_connect.port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_net_listen_param( sqlite3_stmt *stmt, net_listen *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG


		action->protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI _collect_net_listen_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG


		action->do_net_listen.protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Ŀ�����ID 
		action->do_net_listen.ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->do_net_listen.port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS WINAPI collect_net_send_param( sqlite3_stmt *stmt, net_send *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_net_send_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_net_send.protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->do_net_send.data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->do_net_send.ip_addr = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->do_net_send.port = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 

	}while( FALSE );

	return ntstatus; 
}

#define collect_net_recv_param collect_net_send_param 
#define collect_net_accept_param collect_net_recv_param
#define _collect_net_recv_param _collect_net_send_param 
#define _collect_net_accept_param _collect_net_recv_param

NTSTATUS WINAPI collect_net_http_param( sqlite3_stmt *stmt, net_http *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Э�� 
		action->cmd = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //HTTP���� 
		action->data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //�������ݳ��� 
		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		action->path_name_ptr  = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //URL 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_net_http_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_net_http.protocol = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); //Э�� 
		action->do_net_http.cmd = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //HTTP���� 
		action->do_net_http.data_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //�������ݳ��� 
		action->do_net_http.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		action->do_net_http.path_name_ptr  = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); //URL 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI collect_ba_extract_hidden_param( sqlite3_stmt *stmt, ba_extract_hidden *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->path_name_ptr  = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //URL 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_ba_extract_hidden_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_ba_extract_hidden.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 
		action->size = ACTION_RECORD_SIZE_BY_TYPE( ba_extract_hidden ) + ( ( action->do_ba_extract_hidden.path_len + 1 ) << 1 ); 

		if( action->size > buf_size )
		{
			ntstatus + STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); //URL 
	
		memcpy( action->do_ba_extract_hidden.path_name, 
			text, 
			( ( action->do_ba_extract_hidden.path_len + 1 ) << 1 ) ); 
	}while( FALSE );

	return ntstatus; 
}

#define collect_ba_extract_pe_param collect_ba_extract_hidden_param
#define collect_ba_self_copy_param collect_ba_extract_hidden_param
#define collect_ba_self_delete_param collect_ba_extract_hidden_param
#define collect_ba_infect_pe_param collect_ba_extract_hidden_param

#define _collect_ba_extract_pe_param _collect_ba_extract_hidden_param
#define _collect_ba_self_copy_param _collect_ba_extract_hidden_param
#define _collect_ba_self_delete_param _collect_ba_extract_hidden_param
#define _collect_ba_infect_pe_param _collect_ba_extract_hidden_param

NTSTATUS WINAPI collect_ba_register_autorun_param( sqlite3_stmt *stmt, ba_register_autorun *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->type = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 

		action->path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->path_name_ptr  = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //URL 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_ba_register_autorun_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_ba_register_autorun.type = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX ); 

		action->do_ba_register_autorun.path_len = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		
		action->size = ACTION_RECORD_SIZE_BY_TYPE( ba_register_autorun ) + ( ( action->do_ba_register_autorun.path_len + 1 ) << 1 ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); //URL 

		memcpy( action->do_ba_register_autorun.path_name, 
			text, 
			( ( action->do_ba_register_autorun.path_len + 1 ) << 1 ) ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI collect_exec_create_param( sqlite3_stmt *stmt, exec_create *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX  ); 
		action->image_base = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->path_len = ( PATH_SIZE_T )( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->cmd_len = ( PATH_SIZE_T )( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		
		action->path_name_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 
		action->cmd_line_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI _collect_exec_create_param( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPCWSTR text; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( action != NULL ); 
		ASSERT( buf_size > 0 ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		action->do_exec_create.pid = ( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX  ); 
		action->do_exec_create.image_base = ( POINTER_TYPE )( ULONG_PTR )sqlite3_column_int64( stmt, ACTION_PARAM_FIELD_INDEX + 1 ); 
		action->do_exec_create.path_len = ( PATH_SIZE_T )( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 2 ); 
		action->do_exec_create.cmd_len = ( PATH_SIZE_T )( ULONG )sqlite3_column_int( stmt, ACTION_PARAM_FIELD_INDEX + 3 ); 
		
		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 4 ); 
		ASSERT( text[ action->do_exec_create.path_len ] == L'\0' ); 

		memcpy( action->do_exec_create.path_name, 
			text, 
			( action->do_exec_create.path_len + 1 ) << 1 ); 

		text = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_PARAM_FIELD_INDEX + 5 ); 
		memcpy( action->do_exec_create.path_name + action->do_exec_create.path_len + 1, text, ( action->do_exec_create.cmd_len + 1 ) << 1 ); 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS CALLBACK read_action_from_db_row( sqlite3_stmt *stmt, sys_action_record *action )
{
	NTSTATUS ntstatus = STATUS_INVALID_PARAMETER_1; 

	do 
	{
		action->size = 0; 
		action->type =( sys_action_type )sqlite3_column_int( stmt, 1 ); 

		switch( action->type )
		{
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
		case ACCESS_FILE:
			ntstatus = collect_access_file_param( stmt, &action->file_info ); 
			break; 

		case MODIFY_FILE:
			ntstatus = collect_modify_file_param( stmt, &action->file_info ); 
			break; 

		case DELETE_FILE: 
			ntstatus = collect_delete_file_param( stmt, &action->file_info ); 
			break; 

		case MODIFY_KEY: 
			ntstatus = collect_modify_key_param( stmt, &action->reg_info ); 
			break; 

		case CREATE_PROC: 
			ntstatus = collect_create_proc_param( stmt, &action->proc_info ); 

			break; 

		case TERMINATE_PROC: 
			ntstatus = collect_terminate_proc_param( stmt, &action->proc_info ); 

			break; 

		case ACCESS_COM: 
			ntstatus = collect_access_com_param( stmt, &action->file_info ); 

			break; 

		case ACCESS_MEM: 
			ntstatus = collect_access_mem_param( stmt, &action->proc_info ); 

			break; 

		case INSTALL_DRV: 
			ntstatus = collect_install_drv_param( stmt, &action->driver_info ); 

			break; 

		case INSTALL_HOOK: 
			ntstatus = collect_install_hook_param( stmt, &action->proc_info ); 

			break; 

		case SOCKET_CREATE: 
			ntstatus = collect_socket_create_param( stmt, &action->socket_info ); 

			break; 

		case SOCKET_CONNECT: 
			ntstatus = collect_socket_connect_param( stmt, &action->socket_info ); 

			break; 

		case SOCKET_LISTEN: 
			ntstatus = collect_socket_listen_param( stmt, &action->socket_info ); 

			break; 

		case SOCKET_SEND: 
			ntstatus = collect_socket_send_param( stmt, &action->socket_info ); 

			break; 

		case SOCKET_RECV: 
			ntstatus = collect_socket_recv_param( stmt, &action->socket_info ); 

			break; 

		case SOCKET_ACCEPT: 
			ntstatus = collect_socket_accept_param( stmt, &action->socket_info ); 

			break; 

		case LOCATE_URL: 
			ntstatus = collect_locate_url_param( stmt, &action->url_info ); 

			break; 

		case ICMP_SEND: 
			ntstatus = collect_icmp_send_param( stmt, &action->icmp_info ); 
			break; 

		case ICMP_RECV: 
			ntstatus = collect_icmp_recv_param( stmt, &action->icmp_info ); 
			break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE

		case EXEC_create:
			ntstatus = collect_exec_create_param( stmt, &action->do_exec_create ); 
			break; //�������� ����·���� ��ִ�м�أ� 

		case EXEC_destroy:
			ntstatus = collect_exec_destroy_param( stmt, &action->do_exec_destroy ); 
			break; //�����˳� ����·���� 

		case EXEC_module_load:
			ntstatus = collect_exec_module_load_param( stmt, &action->do_exec_module_load ); 
			break; //ģ����� ģ��·���� 

			//MT_filemon, 
		case FILE_touch:
			ntstatus = collect_file_touch_param( stmt, &action->do_file_open ); 
			break; //�����ļ� �ļ�ȫ·�� ���ļ���أ� 	
		case FILE_open:
			ntstatus = collect_file_open_param( stmt, &action->do_file_open ); 
			break; //��ȡ�ļ� �ļ�ȫ·�� 

		case FILE_read:
			ntstatus = collect_file_read_param( stmt, &action->do_file_read ); 
			break; //д���ļ� �ļ�ȫ·�� 

		case FILE_write:
			ntstatus = collect_file_write_param( stmt, &action->do_file_write ); 
			break; //д���ļ� �ļ�ȫ·�� 

		case FILE_modified:
			ntstatus = collect_file_modified_param( stmt, &action->do_file_modified ); 
			break; //�ļ����޸� �ļ�ȫ·�� 
		case FILE_readdir:

			break; //����Ŀ¼ Ŀ¼ȫ·�� 
		case FILE_remove:
			ntstatus = collect_file_remove_param( stmt, &action->do_file_remove ); 
			break; //ɾ���ļ� �ļ�ȫ·�� 

		case FILE_rename:
			ntstatus = collect_file_rename_param( stmt, &action->do_file_rename ); 
			break; //�������ļ� �ļ�ȫ·�� 
		case FILE_truncate:
			//ntstatus = collect_file_truncate_param( )
			break; //�ض��ļ� �ļ�ȫ·�� 
		case FILE_mklink:
			break; //�����ļ�Ӳ���� �ļ�ȫ·�� 

		case FILE_chmod:
			break; //�����ļ����� �ļ�ȫ·�� 

		case FILE_getinfo:
			break; //�����ļ����� �ļ�ȫ·�� 

		case FILE_setinfo:
			break; //�����ļ����� �ļ�ȫ·�� 

		case FILE_setsec:

			break; //�����ļ���ȫ���� �ļ�ȫ·�� 
			//MT_regmon, 
		case REG_openkey:
			ntstatus = collect_reg_open_key_param( stmt, &action->do_reg_openkey ); 
			break; //��ע����� ע�����·��  ��ע�����أ� 

		case REG_mkkey:
			ntstatus = collect_reg_mk_key_param( stmt, &action->do_reg_mkkey ); 
			break; //����ע����� ע�����·�� 
			//case MODIFY_KEY
			//	break; 
		case REG_rmkey:
			ntstatus = collect_reg_rm_key_param( stmt, &action->do_reg_rmkey ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_mvkey:
			ntstatus = collect_reg_mv_key_param( stmt, &action->do_reg_mvkey ); 
			break; //������ע����� ע�����·�� 

		case REG_rmval:
			ntstatus = collect_reg_rm_val_param( stmt, &action->do_reg_rmval ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_getval:
			ntstatus = collect_reg_get_val_param( stmt, &action->do_reg_getval ); 
			break; //��ȡע���ֵ ע���ֵ·�� 

		case REG_setval:
			ntstatus = collect_reg_set_val_param( stmt, &action->do_reg_setval ); 
			break; //����ע���ֵ ע���ֵ·�� 

		case REG_loadkey:
			
			break; //����ע���Hive�ļ� ע�����·�� 

		case REG_replkey:

			break; //�滻ע����� ע�����·�� 

		case REG_rstrkey:
			break; //����ע���Hive�ļ� ע�����·�� 

			//break; //����ע�������ȫ���� ע�����·�� 
			//MT_procmon, 
		case PROC_exec:
			ntstatus = collect_proc_exec_param( stmt, &action->do_proc_exec ); 

			break; //�������� Ŀ�����·����  �����̼�أ�

		case PROC_open:
			//ntstatus = collect_proc_open
			break; //�򿪽��� Ŀ�����·���� 
		case PROC_debug:
			break; //���Խ��� Ŀ�����·���� 
		case PROC_suspend:
			break; //������� Ŀ�����·���� 
		case PROC_resume:
			break; //�ָ����� Ŀ�����·���� 
		case PROC_exit:
			break; //�������� Ŀ�����·���� 

		case PROC_job:
			break; //�����̼��빤���� Ŀ�����·���� 
		case PROC_pgprot:
			break; //������޸��ڴ����� Ŀ�����·���� 
		case PROC_freevm:
			break; //������ͷ��ڴ� Ŀ�����·���� 
		case PROC_writevm:
			ntstatus = collect_proc_write_vm( stmt, &action->do_proc_writevm ); 
			break; //�����д�ڴ� Ŀ�����·���� 
		case PROC_readvm:
			ntstatus = collect_proc_read_vm( stmt, &action->do_proc_readvm ); 
			break; //����̶��ڴ� Ŀ�����·���� 
		case THRD_remote:
			ntstatus = collect_thrd_remote( stmt, &action->do_thrd_remote ); 
			break; //����Զ���߳� Ŀ�����·���� 
		case THRD_setctxt:
			break; //����������߳������� Ŀ�����·���� 
		case THRD_suspend:
			break; //����̹����߳� Ŀ�����·���� 
		case THRD_resume:
			break; //����ָ̻��߳� Ŀ�����·���� 
		case THRD_exit:
			break; //����̽����߳� Ŀ�����·���� 
		case THRD_queue_apc:
			break; //������Ŷ�APC Ŀ�����·���� 

			//MT_common
			//case COM_access:
			//	break; 
			//MT_sysmon
		case SYS_settime:

			break; //����ϵͳʱ�� �� 
		case SYS_link_knowndll:
			break; //����KnownDlls���� �����ļ��� 
		case SYS_open_physmm:

			break; //�������ڴ��豸 �� 

		case SYS_read_physmm:

			break; //�������ڴ� �� 
		case SYS_write_physmm:
			//ntstatus = collect_sys_write
			break; //д�����ڴ� �� 
		case SYS_load_kmod:

			break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
			//case INSTALL_DRV:
			//	break; 
		case SYS_enumproc:
			break; //ö�ٽ��� �� 
		case SYS_regsrv:

			break; //ע����� �������ȫ·�� 
		case SYS_opendev:

			break; //���豸 �豸�� 

			//MT_w32mon
		case W32_postmsg:

			break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
		case W32_sendmsg:

			break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
		case W32_findwnd:

			break; //���Ҵ��� �� 
		case W32_msghook:

			break; //������Ϣ���� �� 

		case W32_lib_inject:

			break; //DLLע�� ע��DLL·���� 

			//MT_netmon
		case NET_create:
			ntstatus = collect_net_create_param( stmt, &action->do_net_create ); 
			break; 

		case NET_connect:
			ntstatus = collect_net_connect_param( stmt, &action->do_net_connect ); 
			break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
			//case SOCKET_CONNECT:
			//	break; 
		case NET_listen:
			ntstatus = collect_net_listen_param( stmt, &action->do_net_listen ); 
			break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_LISTEN:
			//	break; 
		case NET_send:
			ntstatus = collect_net_send_param( stmt, &action->do_net_send ); 
			break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_SEND:
			//	break; 
		case NET_recv:
			ntstatus = collect_net_recv_param( stmt, &action->do_net_recv ); 
			break; 
			//case SOCKET_RECV:
			//	break; 
		case NET_accept:
			ntstatus = collect_net_accept_param( stmt, &action->do_net_accept ); 
			break; 
			//case SOCKET_ACCEPT:
			//	break; 

		//case NET_dns: 
		//	ntstatus = collect_net_dns_param( stmt, &action->do_net_dns ); 
		//	break; //HTTP���� HTTP����·������ʽ������/URL��
		case NET_http: 
			ntstatus = collect_net_http_param( stmt, &action->do_net_http ); 
			break; //HTTP���� HTTP����·������ʽ������/URL�� 

		case NET_icmp_send:
			break; 
		case NET_icmp_recv:
			break; 
			//MT_behavior, 
		case BA_extract_hidden:
			break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
		case BA_extract_pe:
			ntstatus = collect_ba_extract_pe_param( stmt, &action->do_ba_overwrite_pe ); 
			break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
		case BA_self_copy:

			break; //���Ҹ��� ����Ŀ���ļ�·���� 
		case BA_self_delete:

			break; //����ɾ�� ɾ���ļ�·���� 
		case BA_ulterior_exec:

			break; //����ִ�� ��ִ��ӳ��·���� 
		case BA_invade_process:
			
			break; //���ֽ��� Ŀ�����·���� 
		case BA_infect_pe:
			ntstatus = collect_ba_infect_pe_param( stmt, &action->do_ba_infect_pe ); 

			break; //��ȾPE�ļ� Ŀ���ļ�·���� 
		case BA_overwrite_pe:
			break; //��дPE�ļ� Ŀ���ļ�·���� 
		case BA_register_autorun:
			ntstatus = collect_ba_register_autorun_param( stmt, &action->do_ba_register_autorun ); 
			break; //ע���������� �������ļ�·���� 

			//case BA_other:
			//desc = L"BA_other"; 
			//break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}

		if( ntstatus != STATUS_SUCCESS )
		{
			DBG_BP(); 
			break; 
		}

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS CALLBACK __read_action_from_db_row( sqlite3_stmt *stmt, sys_action_record *action, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_INVALID_PARAMETER_1; 

	do 
	{
		action->size = 0; 
		action->type =( sys_action_type )sqlite3_column_int( stmt, 1 ); 

		switch( action->type )
		{
		case EXEC_create:
			ntstatus = _collect_exec_create_param( stmt, action, buf_size ); 
			break; //�������� ����·���� ��ִ�м�أ� 

		case EXEC_destroy:
			ntstatus = _collect_exec_destroy_param( stmt, action, buf_size ); 
			break; //�����˳� ����·���� 

		case EXEC_module_load:
			ntstatus = _collect_exec_module_load_param( stmt, action, buf_size ); 
			break; //ģ����� ģ��·���� 

			//MT_filemon, 
		case FILE_touch:
			ntstatus = _collect_file_touch_param( stmt, action, buf_size ); 
			break; //�����ļ� �ļ�ȫ·�� ���ļ���أ� 	
		case FILE_open:
			ntstatus = _collect_file_open_param( stmt, action, buf_size ); 
			break; //��ȡ�ļ� �ļ�ȫ·�� 

		case FILE_read:
			ntstatus = _collect_file_read_param( stmt, action, buf_size ); 
			break; //д���ļ� �ļ�ȫ·�� 

		case FILE_write:
			ntstatus = _collect_file_write_param( stmt, action, buf_size ); 
			break; //д���ļ� �ļ�ȫ·�� 

		case FILE_modified:
			ntstatus = _collect_file_modified_param( stmt, action, buf_size ); 
			break; //�ļ����޸� �ļ�ȫ·�� 
		case FILE_readdir:

			break; //����Ŀ¼ Ŀ¼ȫ·�� 
		case FILE_remove:
			ntstatus = _collect_file_remove_param( stmt, action, buf_size ); 
			break; //ɾ���ļ� �ļ�ȫ·�� 

		case FILE_rename:
			ntstatus = _collect_file_rename_param( stmt, action, buf_size ); 
			break; //�������ļ� �ļ�ȫ·�� 
		case FILE_truncate:
			//ntstatus = _collect_file_truncate_param( )
			break; //�ض��ļ� �ļ�ȫ·�� 
		case FILE_mklink:
			break; //�����ļ�Ӳ���� �ļ�ȫ·�� 

		case FILE_chmod:
			break; //�����ļ����� �ļ�ȫ·�� 

		case FILE_getinfo:
			break; //�����ļ����� �ļ�ȫ·�� 

		case FILE_setinfo:
			break; //�����ļ����� �ļ�ȫ·�� 

		case FILE_setsec:

			break; //�����ļ���ȫ���� �ļ�ȫ·�� 
			//MT_regmon, 
		case REG_openkey:
			ntstatus = _collect_reg_open_key_param( stmt, action, buf_size ); 
			break; //��ע����� ע�����·��  ��ע�����أ� 

		case REG_mkkey:
			ntstatus = _collect_reg_mk_key_param( stmt, action, buf_size ); 
			break; //����ע����� ע�����·�� 
			//case MODIFY_KEY
			//	break; 
		case REG_rmkey:
			ntstatus = _collect_reg_rm_key_param( stmt, action, buf_size ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_mvkey:
			ntstatus = _collect_reg_mv_key_param( stmt, action, buf_size ); 
			break; //������ע����� ע�����·�� 

		case REG_rmval:
			ntstatus = _collect_reg_rm_val_param( stmt, action, buf_size ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_getval:
			ntstatus = _collect_reg_get_val_param( stmt, action, buf_size ); 
			break; //��ȡע���ֵ ע���ֵ·�� 

		case REG_setval:
			ntstatus = _collect_reg_set_val_param( stmt, action, buf_size ); 
			break; //����ע���ֵ ע���ֵ·�� 

		case REG_loadkey:

			break; //����ע���Hive�ļ� ע�����·�� 

		case REG_replkey:

			break; //�滻ע����� ע�����·�� 

		case REG_rstrkey:
			break; //����ע���Hive�ļ� ע�����·�� 

			//break; //����ע�������ȫ���� ע�����·�� 
			//MT_procmon, 
		case PROC_exec:
			ntstatus = _collect_proc_exec_param( stmt, action, buf_size ); 

			break; //�������� Ŀ�����·����  �����̼�أ�

		case PROC_open:
			//ntstatus = _collect_proc_open
			break; //�򿪽��� Ŀ�����·���� 
		case PROC_debug:
			break; //���Խ��� Ŀ�����·���� 
		case PROC_suspend:
			break; //������� Ŀ�����·���� 
		case PROC_resume:
			break; //�ָ����� Ŀ�����·���� 
		case PROC_exit:
			break; //�������� Ŀ�����·���� 

		case PROC_job:
			break; //�����̼��빤���� Ŀ�����·���� 
		case PROC_pgprot:
			break; //������޸��ڴ����� Ŀ�����·���� 
		case PROC_freevm:
			break; //������ͷ��ڴ� Ŀ�����·���� 
		case PROC_writevm:
			ntstatus = _collect_proc_write_vm( stmt, action, buf_size ); 
			break; //�����д�ڴ� Ŀ�����·���� 
		case PROC_readvm:
			ntstatus = _collect_proc_read_vm( stmt, action, buf_size ); 
			break; //����̶��ڴ� Ŀ�����·���� 
		case THRD_remote:
			ntstatus = _collect_thrd_remote( stmt, action, buf_size ); 
			break; //����Զ���߳� Ŀ�����·���� 
		case THRD_setctxt:
			break; //����������߳������� Ŀ�����·���� 
		case THRD_suspend:
			break; //����̹����߳� Ŀ�����·���� 
		case THRD_resume:
			break; //����ָ̻��߳� Ŀ�����·���� 
		case THRD_exit:
			break; //����̽����߳� Ŀ�����·���� 
		case THRD_queue_apc:
			break; //������Ŷ�APC Ŀ�����·���� 

			//MT_common
			//case COM_access:
			//	break; 
			//MT_sysmon
		case SYS_settime:

			break; //����ϵͳʱ�� �� 
		case SYS_link_knowndll:
			break; //����KnownDlls���� �����ļ��� 
		case SYS_open_physmm:

			break; //�������ڴ��豸 �� 

		case SYS_read_physmm:

			break; //�������ڴ� �� 
		case SYS_write_physmm:
			//ntstatus = _collect_sys_write
			break; //д�����ڴ� �� 
		case SYS_load_kmod:

			break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
			//case INSTALL_DRV:
			//	break; 
		case SYS_enumproc:
			break; //ö�ٽ��� �� 
		case SYS_regsrv:

			break; //ע����� �������ȫ·�� 
		case SYS_opendev:

			break; //���豸 �豸�� 

			//MT_w32mon
		case W32_postmsg:

			break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
		case W32_sendmsg:

			break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
		case W32_findwnd:

			break; //���Ҵ��� �� 
		case W32_msghook:

			break; //������Ϣ���� �� 

		case W32_lib_inject:

			break; //DLLע�� ע��DLL·���� 

			//MT_netmon
		case NET_create:
			ntstatus = _collect_net_create_param( stmt, action, buf_size ); 
			break; 

		case NET_connect:
			ntstatus = _collect_net_connect_param( stmt, action, buf_size ); 
			break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
			//case SOCKET_CONNECT:
			//	break; 
		case NET_listen:
			ntstatus = _collect_net_listen_param( stmt, action, buf_size ); 
			break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_LISTEN:
			//	break; 
		case NET_send:
			ntstatus = _collect_net_send_param( stmt, action, buf_size ); 
			break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_SEND:
			//	break; 
		case NET_recv:
			ntstatus = _collect_net_recv_param( stmt, action, buf_size ); 
			break; 
			//case SOCKET_RECV:
			//	break; 
		case NET_accept:
			ntstatus = _collect_net_accept_param( stmt, action, buf_size ); 
			break; 
			//case SOCKET_ACCEPT:
			//	break; 

		//case NET_dns: 
		//	//ntstatus = _collect_net_dns_param( stmt, action, buf_size ); 
		//	break; //HTTP���� HTTP����·������ʽ������/URL�� 

		case NET_http: 
			ntstatus = _collect_net_http_param( stmt, action, buf_size ); 
			break; //HTTP���� HTTP����·������ʽ������/URL�� 

		case NET_icmp_send:
			break; 
		case NET_icmp_recv:
			break; 
			//MT_behavior, 
		case BA_extract_hidden:
			break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
		case BA_extract_pe:
			ntstatus = _collect_ba_extract_pe_param( stmt, action, buf_size ); 
			break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
		case BA_self_copy:

			break; //���Ҹ��� ����Ŀ���ļ�·���� 
		case BA_self_delete:

			break; //����ɾ�� ɾ���ļ�·���� 
		case BA_ulterior_exec:

			break; //����ִ�� ��ִ��ӳ��·���� 
		case BA_invade_process:

			break; //���ֽ��� Ŀ�����·���� 
		case BA_infect_pe:
			ntstatus = _collect_ba_infect_pe_param( stmt, action, buf_size ); 

			break; //��ȾPE�ļ� Ŀ���ļ�·���� 
		case BA_overwrite_pe:
			break; //��дPE�ļ� Ŀ���ļ�·���� 
		case BA_register_autorun:
			ntstatus = _collect_ba_register_autorun_param( stmt, action, buf_size ); 
			break; //ע���������� �������ļ�·���� 

			//case BA_other:
			//desc = L"BA_other"; 
			//break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}

		if( ntstatus != STATUS_SUCCESS )
		{
			DBG_BP(); 
			break; 
		}

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS CALLBACK _read_action_from_db_row( sqlite3_stmt *stmt, sys_action_record *action )
{
	return __read_action_from_db_row( stmt, action, BASE_MAX_R3_NOTIFY_VARIABLE_SIZE ); 
}

NTSTATUS WINAPI read_action_context( sqlite3_stmt *stmt, action_context *ctx )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret; 
	WCHAR *proc_name; 
	ULONG proc_name_len; 
	LPCWSTR time_text; 

	do 
	{
		ASSERT( ctx != NULL ); 


		ctx->last_result = ( ULONG )sqlite3_column_int( stmt, ACTION_RESULT_PARAM_FIELD_INDEX ); 

		proc_name = ( WCHAR* )sqlite3_column_text16( stmt, PROC_NAME_COLUME_INDEX ); 

		time_text = ( LPWSTR )sqlite3_column_text16( stmt, ACTION_TIME_COLUME_INDEX ); 

		ret = string_to_time( time_text, &ctx->time ); 

		if( ret != ERROR_SUCCESS )
		{
			
		}
		
		ctx->proc_id = ( ULONG )sqlite3_column_int( stmt, PROC_ID_COLUME_INDEX ); 
		ctx->thread_id = ( ULONG )sqlite3_column_int( stmt, THREAD_ID_COLUME_INDEX ); 

		if( proc_name == NULL )
		{
			if( ctx->proc_id == 4 )
			{
#define SYSTEM_PROCESS_NAME L"SYSTEM"
				memcpy( ctx->proc_name, SYSTEM_PROCESS_NAME, sizeof( SYSTEM_PROCESS_NAME ) ); 
				ctx->proc_name_len = ARRAYSIZE( SYSTEM_PROCESS_NAME ) - 1; 
			}
			else if( ctx->proc_id == 0 )
			{
#define IDLE_PROCESS_NAME L"IDLE"
				memcpy( ctx->proc_name, IDLE_PROCESS_NAME, sizeof( IDLE_PROCESS_NAME ) );  
				ctx->proc_name_len = ARRAYSIZE( IDLE_PROCESS_NAME ) - 1; 
			}
			else
			{
				//check if process id is insane. maybe max process id is lower or equal than 65535
				ASSERT( FALSE && "why trace process name is 0" ); 
				*ctx->proc_name = L'\0'; 
				ctx->proc_name_len = 0; 
			}

			break; 
		}

		proc_name_len = ( ULONG )wcslen( proc_name ); 
		if( proc_name_len >= ARRAY_SIZE( ctx->proc_name ) )
		{
			proc_name_len = ARRAY_SIZE( ctx->proc_name ) - 1; 

			memcpy( ctx->proc_name, proc_name, proc_name_len << 1 ); 
			ctx->proc_name[ proc_name_len ] = L'\0'; 
		}
		else
		{
			memcpy( ctx->proc_name, proc_name, ( proc_name_len + 1 ) << 1 ); 
		}

		ctx->proc_name_len = proc_name_len; 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI read_action_data( sqlite3_stmt *stmt, PVOID data, ULONG buf_len, ULONG *ret_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sqlite3_blob *blob; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( data != NULL ); 
		ASSERT( ret_len != NULL ); 
		ASSERT( buf_len > 0 ); 

		blob = ( sqlite3_blob* )sqlite3_column_blob( stmt, ACTION_DATA_PARAM_FIELD_INDEX ); 
		
		if( blob == NULL )
		{
			*ret_len = 0; 
			ntstatus = STATUS_NOT_FOUND; 
			break; 
		}

		ntstatus = read_blob_data( blob, data, buf_len, ret_len ); 
		if( ntstatus != STATUS_SUCCESS )
		{

		}

	}while( FALSE );

	return ntstatus; 
}

INT32 CALLBACK read_action_log( sqlite3_stmt *stmt, 
							   ULONG type, 
							   PVOID read_callback_func, 
							   PVOID alloc_func, 
							   PVOID param )
{
	NTSTATUS ntstatus; 
	INT32 ret = SQLITE_OK; 
	ULONG state; 
	sys_action_record action; 
	action_context ctx; 
	WCHAR *stack_dump; 
	PVOID data = NULL; 
	ULONG data_len; 
	ULONG action_log_id; 

	ASSERT( stmt != NULL ); 
	ASSERT( read_callback_func != NULL ); 

	do 
	{

		ASSERT( param != NULL ); 

		if( FALSE == is_valid_action_type( ( sys_action_type )type ) )
		{
			ASSERT( FALSE && "why got the invalid driver type value from config db" ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( NULL == read_callback_func )
		{
			ASSERT( FALSE && "why got the invalid driver state value from config db" ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		data = malloc( PAGE_SIZE /*MAX_ACTION_DATA_SIZE */); 
		if( data == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}
		
		action_log_id = ( ULONG )sqlite3_column_int( stmt, ACTION_LOG_ID_INDEX ); 

		ntstatus = read_action_from_db_row( stmt, &action ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load action event information from data base error %u\n", ntstatus ) ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ntstatus = read_action_context( stmt, &ctx ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		stack_dump = ( LPWSTR )sqlite3_column_text16( stmt, ACTION_STACK_PARAM_FIELD_INDEX ); 

		data_len = 0; 

		ntstatus = ( ( action_log_load_callback )read_callback_func )( action_log_id, 
			&action, 
			&ctx, 
			data, 
			data_len, 
			stack_dump, 
			NULL, 
			param, 
			&state ); 

		if( state & ( LOG_IS_ALL_LOADED | ACTION_COUNT_IS_REACH_MAX ) )
		{
			ret = SQLITE_ABORT; 
		}
		else
		{
			if( ntstatus != STATUS_SUCCESS )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

	}while( FALSE ); 

	if( data != NULL )
	{
		free( data ); 
	}

	return ret; 
}

INT32 CALLBACK read_action_log_ex( sqlite3_stmt *stmt, 
							   ULONG type, 
							   PVOID read_callback_func, 
							   PVOID alloc_func, 
							   PVOID param )
{
	NTSTATUS ntstatus; 
	INT32 ret = SQLITE_OK; 
	ULONG state; 
	WCHAR *stack_dump; 
	PVOID data; 
	ULONG data_size; 
	r3_action_notify *action; 

	ASSERT( stmt != NULL ); 
	ASSERT( read_callback_func != NULL ); 

	do 
	{
		ASSERT( param != NULL ); 

		if( FALSE == is_valid_action_type( ( sys_action_type )type ) )
		{
			ASSERT( FALSE && "why got the invalid driver type value from config db" ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( NULL == read_callback_func )
		{
			ASSERT( FALSE && "why got the invalid driver state value from config db" ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( NULL == alloc_func )
		{
			ASSERT( FALSE && "allocate buffer function for reading log from db is null" ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		action = ( r3_action_notify* )( ( alloc_log_buffer )alloc_func )( param ); 

		ASSERT( action != NULL ); 

		action->id.QuadPart = ( ULONGLONG )sqlite3_column_int( stmt, ACTION_LOG_ID_INDEX ); 

		ntstatus = _read_action_from_db_row( stmt, &action->action.action ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load action event information from data base error %u\n", ntstatus ) ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		data = ( BYTE* )get_action_output_data_ptr( action ); 

		ASSERT( BASE_MAX_R3_NOTIFY_VARIABLE_SIZE >= action->action.action.size ); 

		action->data_inited = FALSE; 
		action->real_data_size = 0; 
		action->data_size = 0; //data_size; 

		ntstatus = read_action_context( stmt, &action->action.ctx ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		stack_dump = ( LPWSTR )sqlite3_column_text16( stmt, ACTION_STACK_PARAM_FIELD_INDEX ); 

		action->size = get_r3_action_notify_size( action ); 

		ntstatus = ( ( _action_log_load_callback )read_callback_func )( action, 
			stack_dump, 
			param, 
			&state ); 

		if( state & ( LOG_IS_ALL_LOADED | ACTION_COUNT_IS_REACH_MAX ) )
		{
			ret = SQLITE_ABORT; 
		}
		else
		{
			if( ntstatus != STATUS_SUCCESS )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

	}while( FALSE ); 

	return ret; 
}

NTSTATUS WINAPI input_action_log_to_db_transact( action_context *ctx, 
									   sys_action_record *action, 
									   PVOID data, 
									   ULONG data_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	} while ( FALSE ); 

	return ntstatus; 
}

LRESULT WINAPI _filtered_index_2_real_index( LONG filtered_index, 
											LONG search_filtered_index, 
											LONG search_real_index, 
											LONG *real_index, 
											ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	WCHAR *sql_stmt = NULL; 
	calc_filtered_index_work work_context; 
	data_process data_proc = { 0 }; 

	do 
	{
		ASSERT( real_index != NULL ); 
		ASSERT( filtered_index >= 0 ); 
		ASSERT( search_filtered_index >= 0 ); 
		ASSERT( search_real_index >= 0 ); 
		ASSERT( search_real_index >= search_filtered_index ); 

		*real_index = INVALID_ITEM_REAL_INDEX; 

		sql_stmt = ( WCHAR* )malloc( sizeof( WCHAR ) * LOG_SQL_STMT_LEN ); 
		if( sql_stmt == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		data_proc.data_proc_func = read_action_log; 
		data_proc.callback_func = ( PVOID )_filter_event_log; 

		work_context.search_filtered_index = search_filtered_index; 
		work_context.search_real_index = search_real_index; 
		work_context.target_filtered_index = filtered_index; 
		work_context.target_real_index = INVALID_ITEM_REAL_INDEX; 
		work_context.flags = flags; 

		data_proc.param = &work_context; 

		if( flags != LOG_FILTER_DECENDING )
		{
			_ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, _T( "SELECT * FROM " ) COMMON_ACTION_TABLE_NAME _T( " WHERE rowid>%u" ), search_real_index ); 
		}
		else
		{
			_ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, _T( "SELECT * FROM " ) COMMON_ACTION_TABLE_NAME _T( " WHERE rowid<%u" ) _T( " ORDER BY rowid DESC" ), search_real_index ); 
		}

		if( _ret <= 0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_ret = sql_exec( ( sqlite3* )common_log_db, sql_stmt, &data_proc ); 
		if( _ret != SQLITE_OK )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			log_trace( ( MSG_ERROR, "read the action log count failed %u\n", _ret ) ); 
		}


		log_trace( ( MSG_INFO, "the action log count is %u \n", work_context.target_real_index ) ); 

		if( work_context.target_real_index == INVALID_ITEM_REAL_INDEX )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ASSERT( filtered_index == work_context.target_filtered_index ); 

		*real_index = work_context.target_real_index; 

	}while( FALSE );

	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

NTSTATUS WINAPI output_action_log_from_db( action_context *ctx, 
								   sys_action_type type, 
								   param_sel_mode all_cond[], 
								   param_info all_param[], 
								   ULONG param_count, 
								   ULONG offset, 
								   ULONG limit, 
								   PVOID context, 
								   action_log_load_callback filter_log_func )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 
	data_process db_process; 
	LPCWSTR table_name; 
	sqlite3 *db; 
	LPWSTR select_sql = NULL; 

	do 
	{

		if( all_cond == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( all_param == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		db = common_log_db; 

		db_process.alloc_func = NULL; 
		db_process.callback_func = ( PVOID )filter_log_func; 
		db_process.data_proc_func = read_action_log; 
		db_process.param = context; 
		db_process.type = ( ULONG )0; 

#define MAX_SELECT_SQL_STRING_LEN PAGE_SIZE 
		select_sql = ( LPWSTR )malloc( sizeof( WCHAR ) * MAX_SELECT_SQL_STRING_LEN ); 

		if( select_sql == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		ntstatus = construct_select_sql( select_sql, 
			MAX_SELECT_SQL_STRING_LEN, 
			ctx, 
			type, 
			all_param, 
			all_cond, 
			param_count, 
			offset, 
			limit ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}


		ret = sql_exec( db, select_sql, &db_process ); 
		if( ret != SQLITE_OK )
		{
			ret = ERROR_DB_SELECT_ERROR; 
			break; 
		}

	}while( FALSE );

	if( select_sql != NULL )
	{
		free( select_sql );
	}
	return ntstatus; 
}

NTSTATUS WINAPI output_action_log_from_db_ex( PVOID db_handle, 
											 action_context *ctx, 
											 sys_action_type type, 
											 param_sel_mode all_cond[], 
											 param_info all_param[], 
											 ULONG param_count, 
											 ULONG offset, 
											 ULONG limit, 
											 PVOID context, 
											 PVOID alloc_func, 
											 PVOID filter_log_func )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 
	data_process db_process; 
	LPCWSTR table_name; 
	sqlite3 *db; 
	LPWSTR select_sql = NULL; 

	do 
	{
		ASSERT( db_handle != NULL ); 
		if( all_cond == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( all_param == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		db = ( sqlite3* )db_handle; 

		db_process.alloc_func = alloc_func; 
		db_process.callback_func = ( PVOID )filter_log_func; 
		db_process.data_proc_func = read_action_log_ex; 
		db_process.param = context; 
		db_process.type = ( ULONG )0; 

#define MAX_SELECT_SQL_STRING_LEN PAGE_SIZE 
		select_sql = ( LPWSTR )malloc( sizeof( WCHAR ) * MAX_SELECT_SQL_STRING_LEN ); 

		if( select_sql == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		ntstatus = construct_select_sql( select_sql, 
			MAX_SELECT_SQL_STRING_LEN, 
			ctx, 
			type, 
			all_param, 
			all_cond, 
			param_count, 
			offset, 
			limit ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		ret = sql_exec( db, select_sql, &db_process ); 
		if( ret != SQLITE_OK )
		{
			ret = ERROR_DB_SELECT_ERROR; 
			break; 
		}
	}while( FALSE );

	if( select_sql != NULL )
	{
		free( select_sql );
	}
	return ntstatus; 
}

/****************************************************************************************
������Ҫ�����Ŀ��λ�õ���־����ʵ��ţ������ڸ�Ƶ�ʵĽ��ж�λʱ����ռ�úܶ�����ܡ�
��Ҫ���ǼӾ����ܶ�ļ��ؽ϶����־�������У��������и��õ�����Ч����
****************************************************************************************/

NTSTATUS CALLBACK _filter_event_log( ULONG log_id, 
									sys_action_record *action, 
									action_context *ctx, 
									PVOID data, 
									ULONG data_len, 
									LPCWSTR stack_dump,
									PVOID work_context, 
									ULONG *state_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret; 
	calc_filtered_index_work *context; 

	do 
	{
		*state_out = 0; 

		context = ( calc_filtered_index_work* )work_context; 

		ret = filter_action_log( action, 
			ctx, 
			data, 
			data_len ); 

		if( ret != ERROR_SUCCESS )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		if( context->flags == LOG_FILTER_DECENDING )
		{
			context->search_filtered_index --; 
		}
		else
		{
			context->search_filtered_index ++; 
		}

		if( context->search_filtered_index == context->target_filtered_index )
		{
			*state_out = LOG_IS_ALL_LOADED; 

			//ntstatus = ( NTSTATUS )SQLITE_ABORT; 
			context->target_real_index = log_id; 
			break; 
		}

	}while( FALSE );

	return ntstatus; 
}

#ifndef _EVENT_CACHE_BY_MEMORY
NTSTATUS CALLBACK filter_event_log( ULONG log_id, 
										sys_action_record *action, 
										action_context *ctx, 
										PVOID data, 
										ULONG data_len, 
										LPCWSTR stack_dump,
										//ULONG frame_count, 
										PVOID work_context, 
										ULONG *state_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret; 
	ULONG *log_count; 

	do 
	{

		log_count = ( ULONG* )work_context; 

		ret = filter_action_log( action, 
			ctx, 
			data, 
			data_len ); 

		if( ret != ERROR_SUCCESS )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		*log_count += 1; 

	}while( FALSE );

	return ntstatus; 
}

LRESULT WINAPI get_filtered_action_log_count( ULONG *log_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	ULONG count; 
	WCHAR *sql_stmt = NULL; 
	data_process data_proc = { 0 }; 

	do 
	{
		ASSERT( common_log_db != NULL ); 

		ASSERT( log_count != NULL ); 

		*log_count = 0; 

		sql_stmt = ( WCHAR* )malloc( sizeof( WCHAR ) * LOG_SQL_STMT_LEN ); 
		if( sql_stmt == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		count = 0; 
		data_proc.data_proc_func = read_action_log; 
		data_proc.callback_func = ( PVOID )filter_event_log; 
		data_proc.param = &count; 

		_ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, _T( "SELECT * FROM " ) COMMON_ACTION_TABLE_NAME ); 

		if( _ret <= 0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}


		_ret = sql_exec( ( sqlite3* )common_log_db, sql_stmt, &data_proc ); 
		if( _ret != SQLITE_OK )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			log_trace( ( MSG_ERROR, "read the action log count failed %u\n", _ret ) ); 
		}

		log_trace( ( MSG_INFO, "the action log count is %u \n", count ) ); 
		*log_count = count; 

	}while( FALSE ); 

	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

#endif //_EVENT_CACHE_BY_MEMORY

LRESULT WINAPI get_action_log_count( ULONG *log_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	ULONG count; 
	WCHAR *sql_stmt = NULL; 
	data_process data_proc = { 0 }; 

	do 
	{
		ASSERT( common_log_db != NULL ); 

		ASSERT( log_count != NULL ); 

		*log_count = 0; 

		sql_stmt = ( WCHAR* )malloc( sizeof( WCHAR ) * LOG_SQL_STMT_LEN ); 
		if( sql_stmt == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		data_proc.data_proc_func = check_result_count; 
		data_proc.param = &count; 

		_ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, _T( "SELECT COUNT(*) FROM " ) COMMON_ACTION_TABLE_NAME ); 

		if( _ret <= 0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_ret = sql_exec( ( sqlite3* )common_log_db, sql_stmt, &data_proc ); 
		if( _ret != SQLITE_OK )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			log_trace( ( MSG_ERROR, "read the action log count failed %u\n", _ret ) ); 
		}

		log_trace( ( MSG_INFO, "the action log count is %u \n", count ) ); 
		*log_count = count; 

	}while( FALSE ); 

	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

NTSTATUS WINAPI update_blob_size( )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

VOID WINAPI free_blob_buf( PVOID buf )
{
	if( buf != NULL )
	{
		free( buf ); 
	}
	else
	{
		ASSERT( FALSE ); 
	}
}

NTSTATUS WINAPI write_action_log_data( INT64 row_index, 
							   PBYTE buf, 
							   ULONG buf_len, 
							   ULONG *ret_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 
	sqlite3_blob *blob = NULL; 
	INT32 blob_size; 

	do 
	{
		if( ret_len != NULL )
		{
			*ret_len = 0; 
		}

		if( row_index == INVALID_ROW_INDEX )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( buf_len > MAX_ACTION_DATA_SIZE )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		_ret = sqlite3_blob_open( common_log_db, 
			"main", 
			COMMON_ACTION_TABLE_NAMEA, 
			"data", 
			row_index, 
			1, 
			&blob ); 

		if( _ret != SQLITE_OK )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		blob_size = sqlite3_blob_bytes( blob ); 

		if( buf_len > ( ULONG )blob_size )
		{
			update_blob_size( ); 
		}

		_ret = sqlite3_blob_write( blob, buf, buf_len, 0 ); 
		if( _ret != SQLITE_OK )
		{
			break; 
		}

	}while( FALSE );

	{
		sqlite3_blob_close( blob ); 
	}

	return ntstatus; 
}

NTSTATUS WINAPI read_blob_data( sqlite3_blob *blob, PVOID buf, ULONG buf_len, ULONG *ret_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG copy_len; 
	INT32 _ret; 

	do 
	{

		ASSERT( blob != NULL ); 
		ASSERT( buf != NULL ); 
		ASSERT( buf_len > 0 ); 

		copy_len = sqlite3_blob_bytes( blob ); 

		if( ret_len != NULL )
		{
			*ret_len = copy_len; 
		}

		if( copy_len == 0 )
		{
			break; 
		}

		if( buf_len < copy_len )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			copy_len = buf_len; 
			break; 
		}

		_ret = sqlite3_blob_read( blob, buf, copy_len, 0 ); 
		if( _ret != SQLITE_OK )
		{
			break; 
		}
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI read_action_log_data( INT64 row_index, 
							  PBYTE buf, 
							  ULONG buf_len, 
							  ULONG *ret_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 
	sqlite3_blob *blob = NULL; 

	do 
	{
		if( ret_len != NULL )
		{
			*ret_len = 0; 
		}

		if( row_index == INVALID_ROW_INDEX )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		_ret = sqlite3_blob_open( common_log_db, 
			"main", 
			COMMON_ACTION_TABLE_NAMEA, 
			"data", 
			row_index, 
			0, 
			&blob ); 

		if( _ret != SQLITE_OK )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		ntstatus = read_blob_data( blob, buf, buf_len, ret_len ); 
		if( ntstatus != STATUS_SUCCESS )
		{

		}

	}while( FALSE );

	{
		sqlite3_blob_close( blob ); 
	}

	return ntstatus; 
}

#define MAX_ACTION_LOG_SQL_SIZE ( ULONG )( 4096 ) 

NTSTATUS WINAPI commit_common_log_transaction()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 

	do 
	{
		_ret = commit_transaction( common_log_db ); 
		if( _ret != SQLITE_OK )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			log_trace( ( MSG_ERROR, "commit transaction for the log database error %u\n", _ret ) ); 
		}

	} while ( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI begin_common_log_transaction()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 

	do 
	{
		_ret = begin_transaction( common_log_db ); 
		if( _ret != SQLITE_OK )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			log_trace( ( MSG_ERROR, "begin transaction for the log database error %u\n", _ret ) ); 
		}

	} while ( FALSE );

	return ntstatus; 
}

NTSTATUS WINAPI input_action_log_to_db( action_context *ctx, 
								sys_action_record *action, 
								ULONGLONG call_frame[], 
								ULONG frame_count, 
								PVOID data, 
								ULONG data_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 
	LRESULT __ret; 
	INT64 cur_row_index; 
	sqlite3_stmt *stmt = NULL; 
	WCHAR *prepared_sql = NULL; 
	param_info all_params[ MAX_PARAMS_COUNT ]; 
	param_sel_mode all_conds[ MAX_PARAMS_COUNT ]; 
	param_colume_info all_columes[ MAX_PARAMS_COUNT ] = { 0 }; 
	WCHAR stack_dump_text[ MAX_STACK_DUMP_TEXT_LEN ]; 
	ULONG ccb_ret_len; 
	ULONG param_count; 
	INT32 i; 
	BOOLEAN have_data_colume = FALSE; 
	BOOLEAN have_stack_dump_colume = FALSE; 

	do 
	{
		if( FALSE == is_valid_action_type( action->type ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( common_log_db != NULL ); 

		prepared_sql = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_ACTION_LOG_SQL_SIZE ); 
		if( prepared_sql == NULL )
		{
			ntstatus = STATUS_INSUFF_SERVER_RESOURCES; 
			break; 
		}

		ntstatus = get_params_from_action( action, all_params, ARRAY_SIZE( all_params ), &param_count ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		if( param_count + 1 > ARRAY_SIZE( all_params ) )
		{
			break; 
		}

		do 
		{
			if( frame_count == 0 )
			{
				break; 
			}

			if( call_frame == NULL )
			{
				break; 
			}

			stack_dump_text[ 0 ] = L'\0'; 

#ifdef _DEBUG
			if( stack_dump_text[ 0 ] == 0xcdcd 
				&& stack_dump_text[ 1 ] == 0xcdcd )
			{
				DBG_BP(); 
			}
#endif //_DEBUG

			all_params[ param_count ].type = WSTRING_TYPE; 
			all_params[ param_count ].data.wstring_val = stack_dump_text; 
			all_columes[ param_count ].colume_name = L"stack"; 

			param_count += 1; 
			have_stack_dump_colume = TRUE; 

		} while ( FALSE );

		if( data != NULL /*&& data_len > 0*/ )
		{
			all_params[ param_count ].type = DATA_BLOB_TYPE; 
			all_params[ param_count ].data.uint64_val = 0; 
			
			param_count += 1; 
			have_data_colume = TRUE; 
		}
		else
		{
			ASSERT( FALSE ); 
		}

		for( i = 0; ( ULONG )i < param_count; i ++ )
		{
			all_conds[ i ] = SELECT_AND; 
		}

		ntstatus = construct_insert_sql( prepared_sql, 
			MAX_ACTION_LOG_SQL_SIZE, 
			ctx, 
			action->type, 
			all_params, 
			all_columes, 
			all_conds, 
			param_count ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

#ifdef _DEBUG
		if( wcslen( prepared_sql ) > MAX_ACTION_LOG_SQL_SIZE )
		{
			DBG_BP(); 
		}
#endif //_DEBUG

		_ret = sqlite3_prepare16_v2( common_log_db,  
			prepared_sql, 
			-1, 
			&stmt,
			NULL ); 

		if( _ret != SQLITE_OK )
		{
			LPCSTR err_msg; 
			_ret = sqlite3_errcode( common_log_db ); 
			err_msg = sqlite3_errmsg( common_log_db ); 
			log_trace( ( MSG_FATAL_ERROR, "input the event to the db errro %s\n", err_msg ) ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		if( data != NULL /*&& data_len > 0*/ )
		{
			ASSERT( have_data_colume == TRUE ); 

			_ret = sqlite3_bind_blob( stmt, 1, data, data_len, NULL ); 
			if( _ret != SQLITE_OK )
			{
				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
		}
		else
		{
			ASSERT( FALSE ); 
		}

		_ret = sqlite3_step( stmt ); 
		if( _ret != SQLITE_OK 
			&& _ret != SQLITE_DONE )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}


	}while( FALSE );

	if( prepared_sql != NULL )
	{
		free( prepared_sql ); 
	}

	if( stmt != NULL )
	{
		sqlite3_finalize( stmt ); 
	}
	return ntstatus; 
}

NTSTATUS WINAPI input_action_log_to_db_ex( PVOID db_handle, 
										  action_context *ctx, 
										  sys_action_record *action, 
										  ULONGLONG call_frame[], 
										  ULONG frame_count, 
										  PVOID data, 
										  ULONG data_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 
	LRESULT __ret; 
	INT64 cur_row_index; 
	sqlite3 *db; 
	sqlite3_stmt *stmt = NULL; 
	WCHAR *prepared_sql = NULL; 
	param_info all_params[ MAX_PARAMS_COUNT ]; 
	param_sel_mode all_conds[ MAX_PARAMS_COUNT ]; 
	param_colume_info all_columes[ MAX_PARAMS_COUNT ] = { 0 }; 
	WCHAR stack_dump_text[ MAX_STACK_DUMP_TEXT_LEN ]; 
	ULONG ccb_ret_len; 
	ULONG param_count; 
	INT32 i; 
	BOOLEAN have_data_colume = FALSE; 
	BOOLEAN have_stack_dump_colume = FALSE; 

	do 
	{
		if( FALSE == is_valid_action_type( action->type ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( db_handle != NULL ); 

		db = ( sqlite3* )db_handle; 

		prepared_sql = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_ACTION_LOG_SQL_SIZE ); 
		if( prepared_sql == NULL )
		{
			ntstatus = STATUS_INSUFF_SERVER_RESOURCES; 
			break; 
		}

		ntstatus = get_params_from_action( action, all_params, ARRAY_SIZE( all_params ), &param_count ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		if( param_count + 1 > ARRAY_SIZE( all_params ) )
		{
			break; 
		}

		do 
		{
			if( frame_count == 0 )
			{
				break; 
			}

			if( call_frame == NULL )
			{
				break; 
			}

			stack_dump_text[ 0 ] = L'\0'; 

			all_params[ param_count ].type = WSTRING_TYPE; 
			all_params[ param_count ].data.wstring_val = stack_dump_text; 
			all_columes[ param_count ].colume_name = L"stack"; 

			param_count += 1; 
			have_stack_dump_colume = TRUE; 

		} while ( FALSE );

		if( data != NULL /*&& data_len > 0*/ )
		{
			all_params[ param_count ].type = DATA_BLOB_TYPE; 
			all_params[ param_count ].data.uint64_val = 0; 

			param_count += 1; 
			have_data_colume = TRUE; 
		}
		else
		{
			ASSERT( FALSE ); 
		}

		for( i = 0; ( ULONG )i < param_count; i ++ )
		{
			all_conds[ i ] = SELECT_AND; 
		}

		ntstatus = construct_insert_sql( prepared_sql, 
			MAX_ACTION_LOG_SQL_SIZE, 
			ctx, 
			action->type, 
			all_params, 
			all_columes, 
			all_conds, 
			param_count ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}


		_ret = sqlite3_prepare16_v2( db,  
			prepared_sql, 
			-1, 
			&stmt,
			NULL ); 

		if( _ret != SQLITE_OK )
		{
			LPCSTR err_msg; 
			_ret = sqlite3_errcode( db ); 
			err_msg = sqlite3_errmsg( db ); 
			log_trace( ( MSG_FATAL_ERROR, "input the event to the db errro %s\n", err_msg ) ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		if( data != NULL /*&& data_len > 0*/ )
		{
			ASSERT( have_data_colume == TRUE ); 

			_ret = sqlite3_bind_blob( stmt, 1, data, data_len, NULL ); 
			if( _ret != SQLITE_OK )
			{
				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}
		}
		else
		{
			ASSERT( FALSE ); 
		}

		_ret = sqlite3_step( stmt ); 
		if( _ret != SQLITE_OK 
			&& _ret != SQLITE_DONE )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}


	}while( FALSE );

	if( prepared_sql != NULL )
	{
		free( prepared_sql ); 
	}

	if( stmt != NULL )
	{
		sqlite3_finalize( stmt ); 
	}
	return ntstatus; 
}

LRESULT WINAPI backup_action_info_to_db()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI read_action_info_cache()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE ); 

	return ret; 
}
