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
#include "acl_define.h"
#include "action_type_parse.h"
#include "sqlite_supp.h"
#include "action_log_db.h"
#include "action_log_sql.h"
#include "stack_dump.h"
#include "filter_cond_db.h"
#include "action_filter_cond_sql.h"

sqlite3 *cond_db = NULL;  

static NTSTATUS output_action_filter_cond( sqlite3_stmt *stmt, action_filter_info *filter_info ); 

static INT32 CALLBACK read_action_cond( sqlite3_stmt *stmt, 
								PVOID read_callback_func, 
								PVOID alloc_func, 
								PVOID param ); 
static NTSTATUS read_blob_data( sqlite3_blob *blob, PVOID buf, ULONG buf_len, ULONG *ret_len ); 

NTSTATUS CALLBACK read_action_from_db_row( sqlite3_stmt *stmt, sys_action_record *action ); 

INT32 CALLBACK read_action_filter_cond( sqlite3_stmt *stmt, 
							   ULONG type, 
							   PVOID read_callback_func, 
							   PVOID alloc_func, 
							   PVOID param ); 

//typedef struct _exec_create_manage
//{
//
//	NTSTATUS get_exec_create_insert_sql( LPWSTR sql_out, ULONG ccb_buf_len, ULONG *ccb_ret_len )
//	{
//		NTSTATUS ntstatus = STATUS_SUCCESS; 
//		ULONG _ret; 
//
//		ASSERT( sql_out != NULL ); 
//		ASSERT( ccb_buf_len > 0 ); 
//
//#define MIN_INSERT_SQL_SQL_SIZE ARRAY_SIZE( L"insert to" ) 
//
//		if( ccb_buf_len < MIN_INSERT_SQL_SQL_SIZE )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_1; 
//			break; 
//		}
//
//		_ret = _snwprintf( sql_out, "insert to " )
//	}
//
//}exec_create_manage, *pexec_create_manage;

LRESULT WINAPI check_filter_info_db_struct( sqlite3* db )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR colume_names[] = { L"cond", 
		L"mode", 
		L"value_type", 
		L"value", 
		L"result", 
		L"bk_color", 
		L"disabled" }; 

	do 
	{
		if( NULL == db )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = check_table_columns_exist( db, ACTION_FILTER_COND_TABLE_NAME, colume_names, ARRAYSIZE( colume_names ) ); 
		if( ret == ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ret;  
}

NTSTATUS open_action_filter_cond_db( LPCWSTR file_path, PVOID *handle )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret = 0; 
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

		do 
		{
			WCHAR sql_stmt[ MAX_PATH ]; 

			ret = check_filter_info_db_struct( db ); 
			if( ret == ERROR_SUCCESS )
			{
				break; 
			}

			// ALTER TABLE table_name
			// ADD column_name datatype
			hr = StringCbPrintfW( sql_stmt, sizeof( sql_stmt ), L"DROP TABLE %s ", ACTION_FILTER_COND_TABLE_NAME ); 
			if( FAILED( hr ))
			{
				ret = ERROR_BUFFER_OVERFLOW; 
				break; 
			}
			_ret = sql_exec( db, sql_stmt, NULL ); 
			if( _ret != SQLITE_OK )
			{
				dbg_print( MSG_FATAL_ERROR, "%s execute sql error %d, reason %s\n", __FUNCTION__, ret, sqlite3_errmsg( db ) );

				//ret = ERROR_DB_CONFIG_ERROR; 
				break; 
			}
		}while( FALSE );

		ret = safe_create_table( db, ACTION_FILTER_COND_TABLE_NAME, SQL_CREATE_ACTION_FILTER_COND_TABLE ); 
		if( ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "create log table for common action logging failed error code: %d, reason:%s\n", 
				ret, 
				sqlite3_errmsg16( db ) ) ); 
			
			break; 
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

NTSTATUS close_action_cond_db( PVOID handle )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ntstatus = _close_db( handle ); 

	}while( FALSE );

	return ntstatus; 
}
//
//LPCWSTR param_sel_mode_string[ PARAM_SEL_MODE_COUNT ] = {
//	L"or", 
//		L"and", 
//		L"not", 
//}; 
//
//NTSTATUS create_param_fmt_string( sys_action_record *action, 
//								 LPWSTR fmt_string, 
//								 ULONG ccb_buf_len, 
//								 ULONG *ccb_ret_len )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	
//	do 
//	{
//
//	}while( FALSE );
//	
//	return ntstatus; 
//}

//LPCWSTR get_param_fmt_string(  param_type type )
//{
//	LPCWSTR fmt_string = L""; 
//
//	do 
//	{
//		if( is_valid_param_type( type ) == FALSE )
//		{
//			break; 
//		}
//
//		fmt_string = param_type_fmts[ ( ULONG )type ]; 
//	}while( FALSE );
//
//	return fmt_string; 
//}

//NTSTATUS get_param_string( param_info *param, 
//						  LPWSTR param_text, 
//						  ULONG ccb_buf_len, 
//						  ULONG *ccb_ret_len )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	INT32 ret; 
//	ULONG _ccb_ret_len = 0; 
//
//	do 
//	{
//		ASSERT( param != NULL ); 
//		ASSERT( param_text != NULL ); 
//		ASSERT( ccb_buf_len > 0 ); 
//		ASSERT( ccb_ret_len != NULL ); 
//		
//		//*ccb_ret_len = 0; 
//
//		//param_fmt = get_param_fmt_string( param->type ); 
//
//		switch( param->type )
//		{
//		case INT8_TYPE:
//			ret = _snwprintf( param_text,  
//				ccb_buf_len, 
//				L"%d", 
//				param->data.int8_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//
//			break; 
//		case INT16_TYPE:
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"%d", 
//				param->data.int16_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//
//			break; 
//		case INT32_TYPE:
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"%d", 
//				param->data.int32_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//			break; 
//		case INT64_TYPE:
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"%I64d", 
//				param->data.int64_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//			break; 
//		case UINT8_TYPE:
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"%u", 
//				param->data.uint8_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//			break; 
//		case UINT16_TYPE:
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"%u", 
//				param->data.uint16_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//			break; 
//		case UINT32_TYPE:
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"%u", 
//				param->data.uint32_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//			break; 
//		case UINT64_TYPE:
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"%I64u", 
//				param->data.uint32_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//			break; 
//		case PTR_TYPE:
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"%p", 
//				param->data.ptr_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//			break; 
//		case STRING_PARAM:
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"'%s'", 
//				param->data.string_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//			break; 
//		case WSTRING_TYPE:
//			if( L'\0' == *param->data.wstring_val )
//			{
//				//ASSERT( FALSE ); 
//			}
//
//			ret = _snwprintf( param_text, 
//				ccb_buf_len, 
//				L"'%s'", 
//				param->data.wstring_val ); 
//
//			if( ret < 0 )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == ccb_buf_len )
//			{
//				param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//			else
//			{
//				_ccb_ret_len = ( ULONG )ret; 
//			}
//			break; 
//		case DATA_BLOB_TYPE:
//
//			if( ccb_buf_len < 2 )
//			{
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//
//			param_text[ 0 ] = L'?'; 
//			param_text[ 1 ] = L'\0'; 
//
//			ccb_buf_len = 2; 
//
//			//ret = _snwprintf( param_text, 
//			//	ccb_buf_len, 
//			//	L"?" 
//			//	); 
//
//			//if( ret < 0 )
//			//{
//			//	param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//			//	ntstatus = STATUS_UNSUCCESSFUL; 
//			//	break; 
//			//}
//			//else if( ret == ccb_buf_len )
//			//{
//			//	param_text[ ccb_buf_len - 1 ] = L'\0'; 
//
//			//	ntstatus = STATUS_BUFFER_TOO_SMALL; 
//			//	break; 
//			//}
//			//else
//			//{
//			//	_ccb_ret_len = ( ULONG )ret; 
//			//}
//			break; 
//		//case MAX_PARAM_TYPE:
//		//	break; 
//		default:
//			ASSERT( FALSE ); 
//			ntstatus = STATUS_INVALID_PARAMETER_1; 
//			break; 
//		}
//		
//		*ccb_ret_len = _ccb_ret_len; 
//
//	}while( FALSE );
//
//	return ntstatus; 
//}
//
//LPCWSTR get_param_sel_cond_string( param_sel_mode sel_mode )
//{
//	LPCWSTR cond_string = L""; 
//
//	do 
//	{
//		if( FALSE == invalid_sel_mode( sel_mode ) )
//		{
//			break; 
//		}
//
//		cond_string = param_sel_mode_string[ ( ULONG )sel_mode ]; 
//	}while( FALSE );
//
//	return cond_string; 
//}

//NTSTATUS construct_select_sql( LPWSTR sql_out, 
//							  ULONG ccb_buf_len, 
//							  action_context *ctx, 
//							  sys_action_type type, 
//							  param_info all_params[], 
//							  param_sel_mode all_conds[], 
//							  ULONG count, 
//							  ULONG offset, 
//							  ULONG limit )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	INT32 ret; 
//	INT32 i; 
//	LPCWSTR table_name = L""; 
//#define MAX_PARAM_TEXT_LEN 2048
//	WCHAR *param_text = NULL; 
//	ULONG param_text_len; 
//	LPCWSTR cond_desc; 
//#define MAX_COND_STRING_LEN MAX_PATH 
//	WCHAR cond_string[ MAX_COND_STRING_LEN ]; 
//	
//	ULONG output_len; 
//	ULONG param_output; 
//	BOOLEAN where_added = FALSE;  
//
//	do 
//	{
//		do 
//		{
//			table_name = get_table_name_from_action_type( type ); 
//
//			if( *table_name == L'\0' )
//			{
//				ntstatus = STATUS_INVALID_PARAMETER_1; 
//				break; 
//			}
//
//			sql_out[ ccb_buf_len - 1 ] = L'\0';  
//
//			//wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 
//
//			ret = _snwprintf( sql_out, ccb_buf_len, L"select * from %s", table_name ); 
//			if( ret < 0 )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//			else if( ( ULONG )ret >= ccb_buf_len )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//
//			output_len = 0; 
//
//			param_text = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_PARAM_TEXT_LEN ); 
//			if( param_text == NULL )
//			{
//				ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
//				break; 
//			}
//
//			param_output = 0; 
//
//			if( type != INVALID_SYS_ACTION_TYPE )
//			{
//				if( where_added == FALSE )
//				{
//					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 
//
//					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//					{
//						ntstatus = STATUS_BUFFER_OVERFLOW; 
//						break; 
//					}
//
//					where_added = TRUE; 
//				}
//
//				wcsncat( sql_out, L" type = ", ccb_buf_len - wcslen( sql_out ) ); 
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				ret = _snwprintf( param_text, 
//					MAX_PARAM_TEXT_LEN, 
//					L"%u", 
//					type ); 
//
//				if( ret < 0 )
//				{
//					ASSERT( FALSE ); 
//					ntstatus = STATUS_UNSUCCESSFUL; 
//					break; 
//				}
//				else if( ret == MAX_PARAM_TEXT_LEN )
//				{
//					ASSERT( FALSE ); 
//					ntstatus = STATUS_BUFFER_TOO_SMALL; 
//					break; 
//				}
//
//				wcsncat( sql_out, 
//					param_text, 
//					ccb_buf_len - wcslen( sql_out ) ); 
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				param_output += 1; 
//			}
//
//			if( ctx != NULL && *ctx->proc_name != L'\0' )
//			{
//				if( where_added == FALSE )
//				{
//					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 
//
//					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//					{
//						ntstatus = STATUS_BUFFER_OVERFLOW; 
//						break; 
//					}
//
//					where_added = TRUE; 
//
//					ASSERT( param_output == 0 ); 
//				}
//
//				if( param_output > 0 )
//				{
//					wcsncat( sql_out, L" and proc_name = ", ccb_buf_len - wcslen( sql_out ) ); 
//				}
//				else
//				{
//					wcsncat( sql_out, L" proc_name = ", ccb_buf_len - wcslen( sql_out ) ); 
//				}
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				ret = _snwprintf( param_text, 
//					MAX_PARAM_TEXT_LEN, 
//					L"'%s'", 
//					ctx->proc_name ); 
//
//				if( ret < 0 )
//				{
//					ASSERT( FALSE ); 
//					ntstatus = STATUS_UNSUCCESSFUL; 
//					break; 
//				}
//				else if( ret == MAX_PARAM_TEXT_LEN )
//				{
//					ASSERT( FALSE ); 
//					ntstatus = STATUS_BUFFER_TOO_SMALL; 
//					break; 
//				}
//
//				wcsncat( sql_out, 
//					param_text, 
//					ccb_buf_len - wcslen( sql_out ) ); 
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				param_output += 1; 
//			}
//
//			if( ctx != NULL && ctx->proc_id != 0 )
//			{
//				if( where_added == FALSE )
//				{
//					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 
//
//					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//					{
//						ntstatus = STATUS_BUFFER_OVERFLOW; 
//						break; 
//					}
//
//					where_added = TRUE; 
//
//					ASSERT( param_output == 0 ); 
//				}
//
//				if( param_output > 0 )
//				{
//					wcsncat( sql_out, L" and proc_id = ", ccb_buf_len - wcslen( sql_out ) ); 
//				}
//				else
//				{
//					wcsncat( sql_out, L" proc_id = ", ccb_buf_len - wcslen( sql_out ) ); 
//				}
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				ret = _snwprintf( param_text, 
//					MAX_PARAM_TEXT_LEN, 
//					L"%u", 
//					ctx->proc_id ); 
//
//				if( ret < 0 )
//				{
//					ASSERT( FALSE ); 
//					ntstatus = STATUS_UNSUCCESSFUL; 
//					break; 
//				}
//				else if( ret == MAX_PARAM_TEXT_LEN )
//				{
//					ASSERT( FALSE ); 
//					ntstatus = STATUS_BUFFER_TOO_SMALL; 
//					break; 
//				}
//
//				wcsncat( sql_out, 
//					param_text, 
//					ccb_buf_len - wcslen( sql_out ) ); 
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				param_output += 1; 
//			}
//
//			if( ctx != NULL && ctx->thread_id != 0 )
//			{
//				if( where_added == FALSE )
//				{
//					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 
//
//					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//					{
//						ntstatus = STATUS_BUFFER_OVERFLOW; 
//						break; 
//					}
//
//					where_added = TRUE; 
//
//					ASSERT( param_output == 0 ); 
//				}
//
//				if( param_output > 0 )
//				{
//					wcsncat( sql_out, L" and thread_id = ", ccb_buf_len - wcslen( sql_out ) ); 
//				}
//				else
//				{
//					wcsncat( sql_out, L" thread_id = ", ccb_buf_len - wcslen( sql_out ) ); 
//				}
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				ret = _snwprintf( param_text, 
//					MAX_PARAM_TEXT_LEN, 
//					L"%u", 
//					ctx->thread_id ); 
//
//				if( ret < 0 )
//				{
//					ASSERT( FALSE ); 
//					ntstatus = STATUS_UNSUCCESSFUL; 
//					break; 
//				}
//				else if( ret == MAX_PARAM_TEXT_LEN )
//				{
//					ASSERT( FALSE ); 
//					ntstatus = STATUS_BUFFER_TOO_SMALL; 
//					break; 
//				}
//
//				wcsncat( sql_out, 
//					param_text, 
//					ccb_buf_len - wcslen( sql_out ) ); 
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				param_output += 1; 
//			}
//
//			if( count == 0 )
//			{
//				break; 
//			}
//			else
//			{
//				if( where_added == FALSE )
//				{
//					wcsncat( sql_out, L" where", ccb_buf_len - wcslen( sql_out ) ); 
//
//					if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//					{
//						ntstatus = STATUS_BUFFER_OVERFLOW; 
//						break; 
//					}
//
//					where_added = TRUE; 
//
//					ASSERT( param_output == 0 ); 
//				}
//			}
//
//			if( param_output > 0 )
//			{
//				wcsncat( sql_out, 
//					L" and ", 
//					ccb_buf_len - wcslen( sql_out ) ); 
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//			}
//
//			for( i = 0; ( ULONG )i < count; i ++ )
//			{
//				if( all_conds[ i ] == SELECT_NOT )
//				{
//					continue; 
//				}
//
//				if( i != 0 )
//				{
//					cond_desc = get_param_sel_cond_string( all_conds[ i ] ); 
//					if( *cond_desc == L'\0' )
//					{
//						ASSERT( FALSE && "input invalid parameter condition?" ); 
//						continue; 
//					}
//				}
//				else
//				{
//					cond_desc = L""; 
//				}
//
//				ntstatus = get_param_string( &all_params[ i ], 
//					param_text, 
//					MAX_PARAM_TEXT_LEN, 
//					&param_text_len ); 
//
//				if( ntstatus != STATUS_SUCCESS )
//				{
//					log_trace( ( MSG_ERROR, "can't get the parameter text of the type %u\n", all_params[ i ].type ) ); 
//					continue; 
//				}
//
//				_snwprintf( cond_string, 
//					ARRAY_SIZE( cond_string ), 
//					L"%s param%u = ", 
//					cond_desc, 
//					i ); 
//
//				wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 
//
//				if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				output_len = wcslen( sql_out ); 
//
//				if( output_len + 2 >= ccb_buf_len )
//				{
//					ntstatus = STATUS_BUFFER_OVERFLOW; 
//					break; 
//				}
//
//				sql_out[ output_len ] = L' '; 
//				sql_out[ output_len + 1 ] = L'\0'; 
//			}
//
//		}while( FALSE ); 
//
//		if( ntstatus != STATUS_SUCCESS )
//		{
//			break; 
//		}
//
//		if( limit > 0 )
//		{
//#define MAX_INT_TEXT_LEN 12
//			//WCHAR offset[ MAX_INT_TEXT_LEN ]; 
//
//			ret = _snwprintf( cond_string, 
//				ARRAY_SIZE( cond_string ), 
//				L" limit %u", 
//				limit ); 
//
//			wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//
//			ret = _snwprintf( cond_string, 
//				ARRAY_SIZE( cond_string ), 
//				L" offset %u", 
//				offset ); 
//
//			wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//		}
//	}while( FALSE );
//
//
//	if( param_text != NULL )
//	{
//		free( param_text ); 
//	}
//
//	return ntstatus; 
//}
//
//NTSTATUS construct_delete_sql( LPWSTR sql_out, 
//							  ULONG ccb_buf_len, 
//							  action_context *ctx, 
//							  sys_action_type type, 
//							  param_info all_params[], 
//							  param_sel_mode all_conds[], 
//							  ULONG count )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	INT32 ret; 
//	INT32 i; 
//	LPCWSTR table_name = L""; 
//#define MAX_PARAM_TEXT_LEN 2048
//	WCHAR *param_text = NULL; 
//	ULONG param_text_len; 
//	LPCWSTR cond_desc; 
//#define MAX_COND_STRING_LEN MAX_PATH 
//	WCHAR cond_string[ MAX_COND_STRING_LEN ]; 
//	ULONG output_len; 
//
//	do 
//	{
//		table_name = get_table_name_from_action_type( type ); 
//
//		if( *table_name == L'\0' )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_1; 
//			break; 
//		}
//
//		sql_out[ ccb_buf_len - 1 ] = L'\0';  
//
//		//wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 
//
//		ret = _snwprintf( sql_out, ccb_buf_len, L"delete from %s where", table_name ); 
//		if( ret < 0 )
//		{
//			ntstatus = STATUS_BUFFER_OVERFLOW; 
//			break; 
//		}
//		else if( ( ULONG )ret >= ccb_buf_len )
//		{
//			ntstatus = STATUS_BUFFER_OVERFLOW; 
//			break; 
//		}
//
//		output_len = 0; 
//
//		param_text = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_PARAM_TEXT_LEN ); 
//		if( param_text == NULL )
//		{
//			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
//			break; 
//		}
//
//		for( i = 0; ( ULONG )i < count; i ++ )
//		{
//			if( all_conds[ i ] == SELECT_NOT )
//			{
//				continue; 
//			}
//
//			if( i != 0 )
//			{
//				cond_desc = get_param_sel_cond_string( all_conds[ i ] ); 
//				if( *cond_desc == L'\0' )
//				{
//					ASSERT( FALSE && "input invalid parameter condition?" ); 
//					continue; 
//				}
//			}
//			else
//			{
//				cond_desc = L""; 
//			}
//
//			ntstatus = get_param_string( &all_params[ i ], 
//				param_text, 
//				MAX_PARAM_TEXT_LEN, 
//				&param_text_len ); 
//
//			if( ntstatus != STATUS_SUCCESS )
//			{
//				log_trace( ( MSG_ERROR, "can't get the parameter text of the type %u\n", all_params[ i ].type ) ); 
//				continue; 
//			}
//
//			_snwprintf( cond_string, 
//				ARRAY_SIZE( cond_string ), 
//				L"%s param%u = ", 
//				cond_desc, 
//				i ); 
//
//			wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//
//			wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//
//			output_len = wcslen( sql_out ); 
//
//			if( output_len + 2 >= ccb_buf_len )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//
//			sql_out[ output_len ] = L' '; 
//			sql_out[ output_len + 1 ] = L'\0'; 
//		}
//
//	}while( FALSE );
//
//	if( param_text != NULL )
//	{
//		free( param_text ); 
//	}
//	return ntstatus; 
//}
//
//NTSTATUS construct_insert_sql( LPWSTR sql_out, 
//							  ULONG ccb_buf_len, 
//							  action_context *ctx, 
//							  sys_action_type type, 
//							  param_info all_params[], 
//							  param_colume_info all_columes[], 
//							  param_sel_mode all_conds[], 
//							  ULONG count )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	INT32 ret; 
//	INT32 i; 
//	LPCWSTR table_name = L""; 
//	WCHAR *param_text = NULL; 
//	ULONG param_text_len; 
//	WCHAR fmt_string[ MAX_COND_STRING_LEN ]; 
//	ULONG output_len; 
//
//	do 
//	{
//		table_name = get_table_name_from_action_type( type ); 
//
//		if( *table_name == L'\0' )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_1; 
//			break; 
//		}
//
//		if( ctx == NULL )
//		{
//			DBG_BP(); 
//			//break; 
//		}
//
//		sql_out[ ccb_buf_len - 1 ] = L'\0';  
//
//		//wcsncat( sql_out, cond_string, ccb_buf_len - wcslen( sql_out ) ); 
//
//		ret = _snwprintf( sql_out, ccb_buf_len, L"insert into %s ( ", table_name ); 
//		if( ret < 0 )
//		{
//			ntstatus = STATUS_BUFFER_OVERFLOW; 
//			break; 
//		}
//		else if( ( ULONG )ret >= ccb_buf_len )
//		{
//			ntstatus = STATUS_BUFFER_OVERFLOW; 
//			break; 
//		}
//
//		output_len = 0; 
//
//		param_text = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_PARAM_TEXT_LEN ); 
//		if( param_text == NULL )
//		{
//			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
//			break; 
//		}
//
//		wcsncat( sql_out, L"type, ", ccb_buf_len - wcslen( sql_out ) ); 
//
//		if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//		{
//			ntstatus = STATUS_BUFFER_OVERFLOW; 
//			break; 
//		}
//
//		if( ctx != NULL && *ctx->proc_name != L'\0' )
//		{
//			wcsncat( sql_out, L"proc_name, ", ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//		}
//
//		if( ctx != NULL && ctx->proc_id != 0 )
//		{
//			wcsncat( sql_out, L"proc_id, ", ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//		}
//
//		if( ctx != NULL && ctx->thread_id != 0 )
//		{
//			wcsncat( sql_out, L"thread_id, ", ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//		}
//
//		//if( ctx != NULL && ctx->time.QuadPart != 0 )
//		//{
//		//	wcsncat( sql_out, L"time, ", ccb_buf_len - wcslen( sql_out ) ); 
//
//		//	if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//		//	{
//		//		ntstatus = STATUS_BUFFER_OVERFLOW; 
//		//		break; 
//		//	}			
//		//}
//
//		for( i = 0; ( ULONG )i < count; i ++ )
//		{
//			if( all_conds[ i ] == SELECT_NOT )
//			{
//				continue; 
//			}
//
//			//ntstatus = get_param_string( &all_params[ i ], 
//			//	param_text, 
//			//	MAX_PARAM_TEXT_LEN, 
//			//	&param_text_len ); 
//
//			//if( ntstatus != STATUS_SUCCESS )
//			//{
//			//	log_trace( ( MSG_ERROR, "can't get the parameter text of the type %u\n", all_params[ i ].type ) ); 
//			//	continue; 
//			//}
//
//			if( all_params[ i ].type != DATA_BLOB_TYPE )
//			{
//				if( all_columes[ i ].colume_name != NULL )
//				{
//					if( ( ULONG )i < count - 1 )
//					{
//						ret = _snwprintf( fmt_string, 
//							ARRAY_SIZE( fmt_string ), 
//							L"%s, ", 
//							all_columes[ i ].colume_name ); 
//					}
//					else
//					{
//						ret = _snwprintf( fmt_string, 
//							ARRAY_SIZE( fmt_string ), 
//							L"%s", 
//							all_columes[ i ].colume_name ); 
//					}
//				}
//				else
//				{
//					if( ( ULONG )i < count - 1 )
//					{
//						ret = _snwprintf( fmt_string, 
//							ARRAY_SIZE( fmt_string ), 
//							L"param%u, ",  
//							i ); 
//					}
//					else
//					{
//						ret = _snwprintf( fmt_string, 
//							ARRAY_SIZE( fmt_string ), 
//							L"param%u",  
//							i ); 
//					}
//				}
//			}
//			else
//			{
//				ret = _snwprintf( fmt_string, 
//					ARRAY_SIZE( fmt_string ), 
//					L"data",  
//					i ); 
//			}
//
//			if( ret < 0 )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//			else if( ( ULONG )ret >= ccb_buf_len )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//
//			wcsncat( sql_out, fmt_string, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//		}
//
//		if( ctx != NULL && ctx->time.QuadPart != 0 )
//		{
//			wcsncat( sql_out, L", time ", ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//		}
//
//		wcsncat( sql_out, L" ) VALUES ( ", ccb_buf_len - wcslen( sql_out ) ); 
//
//		if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//		{
//			ntstatus = STATUS_BUFFER_OVERFLOW; 
//			break; 
//		}
//
//		ret = _snwprintf( param_text, 
//			MAX_PARAM_TEXT_LEN, 
//			L"%u, ", 
//			type ); 
//
//		if( ret < 0 )
//		{
//			ASSERT( FALSE ); 
//			ntstatus = STATUS_UNSUCCESSFUL; 
//			break; 
//		}
//		else if( ret == MAX_PARAM_TEXT_LEN )
//		{
//			ASSERT( FALSE ); 
//			ntstatus = STATUS_BUFFER_TOO_SMALL; 
//			break; 
//		}
//
//		wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 
//
//		if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//		{
//			ntstatus = STATUS_BUFFER_OVERFLOW; 
//			break; 
//		}
//
//		if( ctx != NULL && *ctx->proc_name != L'\0' )
//		{
//			ret = _snwprintf( param_text, 
//				MAX_PARAM_TEXT_LEN, 
//				L"'%s', ", 
//				ctx->proc_name ); 
//
//			if( ret < 0 )
//			{
//				ASSERT( FALSE ); 
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == MAX_PARAM_TEXT_LEN )
//			{
//				ASSERT( FALSE ); 
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//
//			wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//		}
//
//		if( ctx != NULL && ctx->proc_id != 0 )
//		{
//			ret = _snwprintf( param_text, 
//				MAX_PARAM_TEXT_LEN, 
//				L"%u, ", 
//				ctx->proc_id ); 
//
//			if( ret < 0 )
//			{
//				ASSERT( FALSE ); 
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == MAX_PARAM_TEXT_LEN )
//			{
//				ASSERT( FALSE ); 
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//
//			wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//		}
//
//		if( ctx != NULL && ctx->thread_id != 0 )
//		{
//			if( count > 0 )
//			{
//				ret = _snwprintf( param_text, 
//					MAX_PARAM_TEXT_LEN, 
//					L"%u, ", 
//					ctx->thread_id ); 
//			}
//			else
//			{
//				ret = _snwprintf( param_text, 
//					MAX_PARAM_TEXT_LEN, 
//					L"%u", 
//					ctx->thread_id ); 
//			}
//
//			if( ret < 0 )
//			{
//				ASSERT( FALSE ); 
//				ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			else if( ret == MAX_PARAM_TEXT_LEN )
//			{
//				ASSERT( FALSE ); 
//				ntstatus = STATUS_BUFFER_TOO_SMALL; 
//				break; 
//			}
//
//			wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//		}
//
//		for( i = 0; ( ULONG )i < count; i ++ )
//		{
//			if( all_conds[ i ] == SELECT_NOT )
//			{
//				continue; 
//			}
//
//			ntstatus = get_param_string( &all_params[ i ], 
//				param_text, 
//				MAX_PARAM_TEXT_LEN, 
//				&param_text_len ); 
//
//			if( ntstatus != STATUS_SUCCESS )
//			{
//				log_trace( ( MSG_ERROR, "can't get the parameter text of the type %u\n", all_params[ i ].type ) ); 
//				continue; 
//			}
//
//			wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//
//			output_len = wcslen( sql_out ); 
//
//			if( output_len + 3 >= ccb_buf_len )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//
//			if( ( ULONG )i < count - 1 )
//			{
//				sql_out[ output_len ] = L','; 
//				sql_out[ output_len + 1 ] = L' ';  
//				sql_out[ output_len + 2 ] = L'\0'; 
//			}
//		}
//
////		if( ctx != NULL && ctx->time.QuadPart != 0 )
////		{
//////#ifdef _DEBUG
////			SYSTEMTIME sys_time; 
////			FILETIME file_time; 
////			FILETIME local_file_time; 
////			LARGE_INTEGER _local_file_time; 
////			INT32 _ret; 
////			//TCHAR tszDateTime[64] = {0};
////
////			//do 
////			//{
////			file_time.dwHighDateTime = ctx->time.HighPart; 
////			file_time.dwLowDateTime = ctx->time.LowPart; 
////
////			_ret = FileTimeToLocalFileTime( &file_time, &local_file_time ); 
////			if( _ret == FALSE )
////			{
////				SAFE_SET_ERROR_CODE( ret ); 
////				break; 
////			}
////
////			_ret = FileTimeToSystemTime( &local_file_time, &sys_time ); 
////			if( _ret == FALSE )
////			{
////				SAFE_SET_ERROR_CODE( ret ); 
////				break; 
////			}
////
//////#endif //_DEBUG
////
////			_local_file_time.HighPart = local_file_time.dwHighDateTime; 
////			_local_file_time.LowPart = local_file_time.dwLowDateTime; 
////			
////			//if( count > 0 )
////			//{
////			//	ret = _snwprintf( param_text, 
////			//		MAX_PARAM_TEXT_LEN, 
////			//		L", datetime( %I64u, 'localtime' ) ", //'unixepoch' 
////			//		_local_file_time.QuadPart ); 
////			//}
////			//else
////			{
////				ret = _snwprintf( param_text, 
////					MAX_PARAM_TEXT_LEN, 
////					L", datetime( %I64u, 'unixepoch', 'localtime' ) ", //'unixepoch' 
////					_local_file_time.QuadPart ); 
////			}
////
////			if( ret < 0 )
////			{
////				ASSERT( FALSE ); 
////				ntstatus = STATUS_UNSUCCESSFUL; 
////				break; 
////			}
////			else if( ret == MAX_PARAM_TEXT_LEN )
////			{
////				ASSERT( FALSE ); 
////				ntstatus = STATUS_BUFFER_TOO_SMALL; 
////				break; 
////			}
////
////			wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 
////
////			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
////			{
////				ntstatus = STATUS_BUFFER_OVERFLOW; 
////				break; 
////			}
////		}
//
//		if( ctx != NULL && ctx->time.QuadPart != 0 )
//		{
//			SYSTEMTIME sys_time; 
//			FILETIME file_time; 
//			FILETIME file_local_time; 
//			INT32 _ret; 
//			HRESULT hr; 
//			//TCHAR tszDateTime[64] = {0};
//
//			//do 
//			//{
//			file_time.dwHighDateTime = ctx->time.HighPart; 
//			file_time.dwLowDateTime = ctx->time.LowPart; 
//
//			_ret = FileTimeToLocalFileTime( &file_time, &file_local_time ); 
//			if( _ret == FALSE )
//			{
//				log_trace( ( DBG_MSG_AND_ERROR_OUT, "convert the log time to system time error %u\n" ) ); 
//				break; 
//			}
//
//			_ret = FileTimeToSystemTime( &file_local_time, &sys_time ); 
//			if( _ret == FALSE )
//			{
//				log_trace( ( DBG_MSG_AND_ERROR_OUT, "convert the log time to system time error %u\n" ) ); 
//				break; 
//			}
//
//			if( count > 0 )
//			{
//				hr = StringCbPrintf (param_text, MAX_PARAM_TEXT_LEN * sizeof( WCHAR ),
//					L", '%04u-%02u-%02u %02u:%02u:%02u.%03u'",
//					sys_time.wYear,
//					sys_time.wMonth,
//					sys_time.wDay,
//					sys_time.wHour,
//					sys_time.wMinute,
//					sys_time.wSecond, 
//					sys_time.wMilliseconds ); 
//
//				//ret = _snwprintf( param_text, 
//				//	MAX_PARAM_TEXT_LEN, 
//				//	L"%u, ", 
//				//	ctx->thread_id ); 
//			}
//			else
//			{
//				hr = StringCbPrintf (param_text, MAX_PARAM_TEXT_LEN * sizeof( WCHAR ),
//					L", '%04u-%02u-%02u %02u:%02u:%02u.%03u'", 
//					sys_time.wYear,
//					sys_time.wMonth,
//					sys_time.wDay,
//					sys_time.wHour,
//					sys_time.wMinute,
//					sys_time.wSecond, 
//					sys_time.wMilliseconds );
//
//				//ret = _snwprintf( param_text, 
//				//	MAX_PARAM_TEXT_LEN, 
//				//	L"%u", 
//				//	ctx->thread_id ); 
//			}
//
//			if( FAILED( hr ) )
//			{
//				ASSERT( FALSE ); 
//				//ntstatus = STATUS_UNSUCCESSFUL; 
//				break; 
//			}
//			//else if( ret == MAX_PARAM_TEXT_LEN )
//			//{
//			//	ASSERT( FALSE ); 
//			//	ntstatus = STATUS_BUFFER_TOO_SMALL; 
//			//	break; 
//			//}
//
//			wcsncat( sql_out, param_text, ccb_buf_len - wcslen( sql_out ) ); 
//
//			if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//			{
//				ntstatus = STATUS_BUFFER_OVERFLOW; 
//				break; 
//			}
//			//} while ( FALSE );
//		}
//
//		wcsncat( sql_out, L" ) ", ccb_buf_len - wcslen( sql_out ) ); 
//
//		if( sql_out[ ccb_buf_len - 1 ] != L'\0' )
//		{
//			ntstatus = STATUS_BUFFER_OVERFLOW; 
//			break; 
//		}
//	}while( FALSE );
//
//	if( param_text != NULL )
//	{
//		free( param_text ); 
//	}
//
//	return ntstatus; 
//}

LRESULT WINAPI adjust_filter_cond_value( action_filter_info *filter_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR text_value; 
	ULONG ulong_value; 
	INT32 int32_value; 
	time_region duration; 
	LPWSTR tmp_text; 

	do 
	{
		ASSERT( NULL != filter_info ); 

		if( filter_info->value.text.text_is_ptr == TRUE ) 
		{
			text_value = filter_info->value.text.text_mode_value_ptr; 
		}
		else
		{
			//ASSERT( FALSE ); 
			text_value = filter_info->value.text.text_mode_value; 
		}

		ASSERT( filter_info->value.text.cc_string_len > 0 
			&& text_value[ 0 ] != L'\0' ); 

		switch( filter_info->cond )
		{
		case FLT_PATH: 
		case FLT_PROCESS_NAME: 
		case FLT_COMMAND: 
		case FLT_DESCRITION: 
		case FLT_DESC_DETAIL: 
		case FLT_USER: 
		case FLT_AUTH_ID: 
		case FLT_CORPORATION: 
			{
				//if( STRING_POINTER_VALUE_TYPE == filter_info->value.type )
				//{
				//	HRESULT hr; 

				//	text_value = filter_info->value.text.text_mode_value_ptr; 

				//	hr = StringCbCopyW( filter_info->value.text.string_value, 
				//		sizeof( filter_info->value.text.string_value ), 
				//		text_value ); 

				//	if( hr != NULL )
				//	{
				//		ret = ERROR_INVALID_PARAMETER; 
				//		break; 
				//	}
				//}
			}
			break; 
		case FLT_MAIN_TYPE: 
		case FLT_ACTION_TYPE:  
		case FLT_PID: 
		case FLT_PARENT_PID: 
		case FLT_TID: 
		case FLT_ACTION_RESULT: 
		case FLT_SESSION: 
			{
				ulong_value = wcstoul( text_value, 
					&tmp_text, 
					10 ); 

				filter_info->value.data.ulong_value = ulong_value; 
				filter_info->value.type = ULONG_VALUE_TYPE; 
			}
			break; 
		case FLT_READ_WRITE: 
		case FLT_CPU_ARCH: 
		case FLT_VIRTUAL_TECH: 
			{
				int32_value = wcstol( text_value, 
					&tmp_text, 
					10 ); 

				filter_info->value.data.ulong_value = int32_value; 
				filter_info->value.type = INT32_VALUE_TYPE; 
			}
			break; 
		case FLT_DURATION: 
			{
				_ret = wscanf_s( text_value, 
					L"%I64-%I64", 
					&duration.begin_time.QuadPart, 
					&duration.end_time.QuadPart ); 

				filter_info->value.data.duration.begin_time.QuadPart = duration.begin_time.QuadPart; 
				filter_info->value.data.duration.end_time.QuadPart = duration.end_time.QuadPart; 

				filter_info->value.type = DURATION_VALUE_TYPE; 
			}
			break; 
		case FLT_DATE_TIME: 
		case FLT_RELATIVE_TIME: 
			{
				//duration.begin_time.QuadPart = _wcstoui64( text_value, 
				//	&tmp_text, 
				//	10 ); 

				_ret = string_to_local_time( text_value, &filter_info->value.data.time_value ); 
				if( _ret != ERROR_SUCCESS )
				{}

				filter_info->value.type = TIME_VALUE_TYPE; 
			}
			break; 
		case FLT_ACTION_NO: 
		case FLT_VERSION: 
			{
				duration.begin_time.QuadPart = _wcstoui64( text_value, 
					&tmp_text, 
					10 ); 

				filter_info->value.data.ulonglong_value = duration.begin_time.QuadPart; 

				filter_info->value.type = ULONGLONG_VALUE_TYPE; 
			}
			break; 
		default: 
			break; 
		}
	}while( FALSE );

	return ret; 
}

NTSTATUS output_action_filter_cond( sqlite3_stmt *stmt, action_filter_info *filter_info )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret; 

	do 
	{
		ASSERT( stmt != NULL ); 
		ASSERT( filter_info != NULL ); 

#ifdef _DEBUG
		if( stmt == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( filter_info == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		//{
		//	LPWSTR column_name; 
		//	INT32 i; 

		//	for( i = 0; i <= ACTION_FILTER_COND_CREATE_TIME_INDEX; i ++ )
		//	{
		//		column_name = ( LPWSTR )sqlite3_column_name16( stmt, i ); 
		//		
		//		dbg_print( MSG_IMPORTANT, "column %u name is %ws\n", i , column_name ); 
		//	}
		//}

		filter_info->cond = ( action_filter_cond )sqlite3_column_int( stmt, ACTION_FILTER_COND_COLUME_INDEX  ); 
		filter_info->compare_mode = ( action_compare_mode )sqlite3_column_int( stmt, ACTION_FILTER_MODE_COLUME_INDEX ); 
		filter_info->value.text.text_mode_value_ptr = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_FILTER_VALUE_COLUME_INDEX ); 
		//filter_info->value.type = ( action_filter_value_type )sqlite3_column_text16( stmt, ACTION_FILTER_VALUE_TYPE_COLUME_INDEX ); 
		filter_info->filter_mode = ( action_filter_mode )sqlite3_column_int( stmt, ACTION_FILTER_RESULT_COLUME_INDEX ); 
		filter_info->ui_info.bk_clr = ( action_filter_mode )sqlite3_column_int( stmt, ACTION_FILTER_BACKGROUD_COLOR_COLUME_INDEX ); 

		if( NULL == filter_info->value.text.text_mode_value_ptr )
		{
			ASSERT( FALSE ); 
			filter_info->value.text.cc_string_len = wcslen( filter_info->value.text.text_mode_value_ptr ); 
		}

		filter_info->value.text.text_is_ptr = TRUE; 
		filter_info->value.type = STRING_VALUE_TYPE; 

		ret = adjust_filter_cond_value( filter_info ); 
		if( ret != ERROR_SUCCESS )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 

		}
	}while( FALSE );

	return ntstatus; 
}

INT32 CALLBACK read_action_cond( sqlite3_stmt *stmt, 
								ULONG type, 
								PVOID read_callback_func, 
								PVOID alloc_func, 
								PVOID param )
{
	NTSTATUS ntstatus; 
	INT32 ret = SQLITE_OK; 
	action_filter_info filter_info; 
	ULONG state; 
	//ULONG action_cond_id; 
	LPCWSTR time; 

	ASSERT( stmt != NULL ); 
	ASSERT( read_callback_func != NULL ); 

	do 
	{
		if( NULL == read_callback_func )
		{
			ASSERT( FALSE && "why got the invalid driver state value from config db" ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		//action_cond_id = ( ULONG )sqlite3_column_int( stmt, ACTION_FILTER_COND_ID_INDEX ); 
		time = ( WCHAR* )sqlite3_column_text16( stmt, ACTION_FILTER_COND_CREATE_TIME_INDEX ); 

		ntstatus = output_action_filter_cond( stmt, &filter_info ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load action event information from data base error %u\n", ntstatus ) ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ntstatus = ( ( action_filter_cond_callback )read_callback_func )( 0, 
			&filter_info, 
			time, 
			param, 
			&state ); 
		
		//if( state & ACTION_COUNT_IS_REACH_MAX )
		//{
		//	ret = SQLITE_ABORT; 
		//}
		//else
		//{
		//	if( ntstatus != STATUS_SUCCESS )
		//	{
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}
		//}

	}while( FALSE ); 

	return ret; 
}

NTSTATUS output_action_cond_from_db( ULONG offset, 
								   ULONG limit, 
								   PVOID context, 
								   action_filter_cond_callback filter_cond_func )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 
	HRESULT _ret; 
	data_process db_process; 
	sqlite3 *db; 
	LPWSTR select_sql = NULL; 

	do 
	{

		db = cond_db; 

		db_process.alloc_func = NULL; 
		db_process.callback_func = ( PVOID )filter_cond_func; 
		db_process.data_proc_func = read_action_cond; 
		db_process.param = context; 
		db_process.type = ( ULONG )0; 

#define MAX_SELECT_SQL_STRING_LEN PAGE_SIZE 
		select_sql = ( LPWSTR )malloc( sizeof( WCHAR ) * MAX_SELECT_SQL_STRING_LEN ); 

		if( select_sql == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		_ret = ( LONG )StringCchPrintfExW( select_sql, 
			MAX_SELECT_SQL_STRING_LEN, 
			NULL, 
			NULL, 
			0, 
			SQL_SELECT_ACTION_COND ); 

		if( FAILED( _ret ) )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		//_ret = sqlite3_prepare16_v2( cond_db,  
		//	prepared_sql, 
		//	-1, 
		//	&stmt,
		//	NULL ); 

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

LRESULT get_action_filter_cond_count( ULONG *cond_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	ULONG count; 
	WCHAR *sql_stmt = NULL; 
	//sqlite3 *db; 
	data_process data_proc = { 0 }; 

	do 
	{
		ASSERT( cond_db != NULL ); 

		ASSERT( cond_count != NULL ); 

		*cond_count = 0; 

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

		//db = common_log_db; 

		_ret = sql_exec( ( sqlite3* )cond_db, sql_stmt, &data_proc ); 
		if( _ret != SQLITE_OK )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			log_trace( ( MSG_ERROR, "read the action log count failed %u\n", _ret ) ); 
		}

		log_trace( ( MSG_INFO, "the action log count is %u \n", count ) ); 
		*cond_count  = count; 

	}while( FALSE ); 

	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

#define MAX_ACTION_COND_SQL_SIZE ( ULONG )( 2048 ) //PAGE_SIZE

NTSTATUS remove_action_cond_from_db( action_filter_info *filter_info )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 
	HRESULT __ret; 
	sqlite3_stmt *stmt = NULL; 
	WCHAR *prepared_sql = NULL; 
	//ULONG ccb_ret_len; 

	do 
	{
		if( NULL == filter_info )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( cond_db != NULL ); 

		prepared_sql = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_ACTION_COND_SQL_SIZE ); 
		if( prepared_sql == NULL )
		{
			ntstatus = STATUS_INSUFF_SERVER_RESOURCES; 
			break; 
		}

		__ret = ( LONG )StringCchPrintfExW( prepared_sql, 
			MAX_ACTION_COND_SQL_SIZE, 
			NULL, 
			NULL, 
			0, 
			SQL_DELETE_ACTION_COND, 
			filter_info->cond, 
			filter_info->compare_mode, 
			filter_info->value.type, 
			filter_info->value.text.text_mode_value, 
			filter_info->filter_mode ); 

		if( FAILED( __ret ) )
		{
			//SAFE_SET_ERROR_CODE( ret ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		_ret = sqlite3_prepare16_v2( cond_db,  
			prepared_sql, 
			-1, 
			&stmt,
			NULL ); 

		if( _ret != SQLITE_OK )
		{
			LPCSTR err_msg; 
			_ret = sqlite3_errcode( cond_db ); 
			err_msg = sqlite3_errmsg( cond_db ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		_ret = sqlite3_step( stmt ); 
		if( _ret != SQLITE_OK 
			&& _ret != SQLITE_DONE )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

	}while( FALSE );

	if( stmt != NULL )
	{
		sqlite3_finalize( stmt ); 
	}

	return ntstatus; 
}

NTSTATUS input_action_cond_to_db( action_filter_info *filter_info )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 
	HRESULT __ret; 
	sqlite3_stmt *stmt = NULL; 
	WCHAR *prepared_sql = NULL; 
	//ULONG ccb_ret_len; 

	do 
	{
		if( NULL == filter_info )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( cond_db != NULL ); 

		prepared_sql = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_ACTION_COND_SQL_SIZE ); 
		if( prepared_sql == NULL )
		{
			ntstatus = STATUS_INSUFF_SERVER_RESOURCES; 
			break; 
		}

		__ret = ( LONG )StringCchPrintfExW( prepared_sql, 
			MAX_ACTION_COND_SQL_SIZE, 
			NULL, 
			NULL, 
			0, 
			SQL_INSERT_ACTION_COND, 
			filter_info->cond, 
			filter_info->compare_mode, 
			filter_info->value.type, 
			filter_info->value.text.text_mode_value, 
			filter_info->filter_mode, 
			filter_info->ui_info.bk_clr ); 

		if( FAILED( __ret ) )
		{
			//SAFE_SET_ERROR_CODE( ret ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		_ret = sqlite3_prepare16_v2( cond_db,  
			prepared_sql, 
			-1, 
			&stmt,
			NULL ); 

		if( _ret != SQLITE_OK )
		{
			LPCSTR err_msg; 
			_ret = sqlite3_errcode( cond_db ); 
			err_msg = sqlite3_errmsg( cond_db ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		_ret = sqlite3_step( stmt ); 
		if( _ret != SQLITE_OK 
			&& _ret != SQLITE_DONE )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

	}while( FALSE );

	if( stmt != NULL )
	{
		sqlite3_finalize( stmt ); 
	}

	return ntstatus; 
}

NTSTATUS update_action_cond_to_db( action_filter_info *filter_info, 
								  action_filter_info *old_filter_info )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 
	HRESULT __ret; 
	sqlite3_stmt *stmt = NULL; 
	WCHAR *prepared_sql = NULL; 
	//ULONG ccb_ret_len; 

	do 
	{
		if( NULL == filter_info )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( cond_db != NULL ); 

		prepared_sql = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_ACTION_COND_SQL_SIZE ); 
		if( prepared_sql == NULL )
		{
			ntstatus = STATUS_INSUFF_SERVER_RESOURCES; 
			break; 
		}

		__ret = ( LONG )StringCchPrintfExW( prepared_sql, 
			MAX_ACTION_COND_SQL_SIZE, 
			NULL, 
			NULL, 
			0, 
			SQL_UPDATE_ACTION_COND, 
			filter_info->cond, 
			filter_info->compare_mode, 
			filter_info->value.type, 
			filter_info->value.text.text_mode_value, 
			filter_info->filter_mode, 
			filter_info->ui_info.bk_clr, 
			old_filter_info->cond, 
			old_filter_info->compare_mode, 
			old_filter_info->value.type, 
			old_filter_info->value.text.text_mode_value, 
			old_filter_info->filter_mode, 
			old_filter_info->ui_info.bk_clr ); 

		if( FAILED( __ret ) )
		{
			//SAFE_SET_ERROR_CODE( ret ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		_ret = sqlite3_prepare16_v2( cond_db, 
			prepared_sql, 
			-1, 
			&stmt,
			NULL ); 

		if( _ret != SQLITE_OK )
		{
			LPCSTR err_msg; 
			_ret = sqlite3_errcode( cond_db ); 
			err_msg = sqlite3_errmsg( cond_db ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		_ret = sqlite3_step( stmt ); 
		if( _ret != SQLITE_OK 
			&& _ret != SQLITE_DONE ) 
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		//{ this is not a correct method.
		//	ULONG count; 
		//	count =( ULONG ) sqlite3_column_int( stmt, 0 ); 
		//	if( 0 == count )
		//	{
		//		dbg_print( MSG_IMPORTANT, "have not update any filter conditions\n" ); 
		//	}
		//}
	}while( FALSE );

	if( stmt != NULL )
	{
		sqlite3_finalize( stmt ); 
	}

	return ntstatus; 
}

NTSTATUS delete_action_cond_from_db( action_filter_info *filter_info )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _ret; 
	HRESULT __ret; 
	sqlite3_stmt *stmt = NULL; 
	WCHAR *prepared_sql = NULL; 
	//ULONG ccb_ret_len; 

	do 
	{
		//if( NULL == filter_info )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER; 
		//	break; 
		//}

		ASSERT( cond_db != NULL ); 

		prepared_sql = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_ACTION_COND_SQL_SIZE ); 
		if( prepared_sql == NULL )
		{
			ntstatus = STATUS_INSUFF_SERVER_RESOURCES; 
			break; 
		}

		if( filter_info == NULL )
		{
			__ret = ( LONG )StringCchPrintfExW( prepared_sql, 
				MAX_ACTION_COND_SQL_SIZE, 
				NULL, 
				NULL, 
				0, 
				SQL_DELETE_ALL_ACTION_COND ); 
		}
		else
		{
			__ret = ( LONG )StringCchPrintfExW( prepared_sql, 
				MAX_ACTION_COND_SQL_SIZE, 
				NULL, 
				NULL, 
				0, 
				SQL_DELETE_ACTION_COND, 
				filter_info->cond, 
				filter_info->compare_mode, 
				filter_info->value.type, 
				filter_info->value.text.text_mode_value, 
				filter_info->filter_mode ); 
		}

		if( FAILED( __ret ) )
		{
			//SAFE_SET_ERROR_CODE( ret ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		_ret = sqlite3_prepare16_v2( cond_db,  
			prepared_sql, 
			-1, 
			&stmt,
			NULL ); 

		if( _ret != SQLITE_OK )
		{
			LPCSTR err_msg; 
			_ret = sqlite3_errcode( cond_db ); 
			err_msg = sqlite3_errmsg( cond_db ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		_ret = sqlite3_step( stmt ); 
		if( _ret != SQLITE_OK 
			&& _ret != SQLITE_DONE )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

	}while( FALSE );

	if( stmt != NULL )
	{
		sqlite3_finalize( stmt ); 
	}

	return ntstatus; 
}

//LRESULT backup_action_filter_cond_to_db()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	do 
//	{
//	}while( FALSE );
//
//	return ret; 
//}

//LRESULT read_action_filter_cond_cache()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	do 
//	{
//	}while( FALSE ); 
//
//	return ret; 
//}

