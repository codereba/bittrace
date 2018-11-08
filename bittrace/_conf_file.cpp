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

#include "StdAfx.h"
#include "ring0_2_ring3.h"
#include "bitsafe_common.h"
#include "UIUtil.h"
#include "sqlite3.h"  
#include "sql_stmt_def.h"
#include "datetime.h"
#include "trace_log_api.h"
#include "sqlite_supp.h"
#include "ui_ctrl.h"
#include "_conf_file.h"

#include "rbtree.h"
#include "hash_table.h"
#ifdef DEBUG_DRIVER
#include "mem_map_io.h"
#include "sys_event.h"
#endif //DEBUG_DRIVER

#include "sys_event_define.h"
#include "volume_name_map.h"
#include "action_log_db.h"
#include "filter_cond_db.h"
#include <strsafe.h>

#define AUTO_ADD_PARAM_RECORD 0x00000001

#define SQL_STMT_LEN 1024
#define LOG_SQL_STMT_LEN 4096
#define DRV_CONF_SQL_STMT_LEN 256
#define GENERAL_SQL_STMT_LEN 512

#define MAX_DEFINE_NUM 5000
#define DEL_DEFINE_ONCE 500

#define MAX_RULE_NUM 20000
#define DEL_RULE_ONCE 500

LPCWSTR flag_descs[] = { 
	INSTALL_DRV_DESC, 
		INSTALL_HOOK_DESC, 
		CREATE_PROC_DESC, 
		ACCESS_MEM_DESC, 
		ACCESS_COM_DESC, 
		MODIFY_KEY_DESC, 
		MODIFY_FILE_DESC 
}; 

LPCTSTR action_descs[] = { ACTION_BLOCK_DESC, 
ACTION_ALLOW_DESC }; 

LPCTSTR prot_descs[] = { PROT_TCP_DESC, PROT_UDP_DESC }; 

LRESULT get_context_for_rule( access_rule_type type, LPCTSTR *tbl_name, row_data_callback *proc_data_func, sqlite3 **db ); 

#define sqlite3_column_text16 ( WCHAR* )sqlite3_column_text16

sqlite3* action_rule_db = NULL; 
sqlite3* fw_log_db = NULL; 
sqlite3* param_define_db = NULL; 
sqlite3* defense_log_db = NULL; 
sqlite3* conf_db = NULL; 
extern sqlite3* common_log_db; 
extern sqlite3* cond_db; 

INLINE LRESULT action_action_log( pfw_log log ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0;
	//ASSERT( handle != NULL ); 


	if( ( add_time % CHECK_LOG_DB_OVERFLOW_TIME )== 0 )
	{
		truncate_log_db( FW_LOG_OP ); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_sntprintf( sql_stmt, LOG_SQL_STMT_LEN, 
		SQL_INSERT_FW_LOG, 
		log->proc_name, 
		log->action, 
		log->type, 
		log->prot, 
		log->src_ip, 
		log->src_port, 
		log->dest_ip, 
		log->dest_port, 
		log->desc, 
		log->time ); 

	ret = sql_exec( ( sqlite3* )fw_log_db, sql_stmt, NULL );
	if( ret != SQLITE_OK )
	{
		ret = ERROR_DB_INSERT_ROW_ERROR; 
		__Trace( _T( "insert new firewall trusted app failed: %d, reason %s\n" ), ret, sqlite3_errmsg16(( sqlite3* )fw_log_db ) );

		goto _return; 
	}
	else
	{
		add_time ++; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

INT32 CALLBACK process_fw_log( sqlite3_stmt *stmt, ULONG type, PVOID callback_func, PVOID alloc_func, PVOID param )
{
	INT32 ret = SQLITE_OK; 
	INT32 alloc_local = FALSE;
	fw_log *log = NULL; 

	if( alloc_func != NULL )
	{
		ret = ( ( alloc_record_callback )( alloc_func ) )( ( PVOID* )&log, param ); 
	}

	if( ret < 0 || log == NULL )
	{
		log = ( fw_log* )malloc( sizeof( fw_log ) ); 
		if( log == NULL )
		{
			ret = SQLITE_NOMEM; 
			goto _return; 
		}

		alloc_local = TRUE; 
	}

	log->id = ( ULONG )sqlite3_column_int( stmt, 0 ); 
	wcsncpy( log->proc_name, sqlite3_column_text16( stmt, 1 ), _MAX_FILE_NAME_LEN ); 
	
	log->action = ( action_response_type )sqlite3_column_int( stmt, 2 ); 
	log->type = ( sys_action_type )sqlite3_column_int( stmt, 3 ); 

	log->prot = ( prot_type )sqlite3_column_int( stmt, 4 ); 

	log->src_ip = sqlite3_column_int( stmt, 5 ); 
	
	log->src_port = sqlite3_column_int( stmt, 6 ); 

	log->dest_ip = sqlite3_column_int( stmt, 7 ); 

	log->dest_port = sqlite3_column_int( stmt, 8 ); 

#define FW_LOG_COL_DESC L"desc"
	ASSERT( wcscmp( ( WCHAR* )sqlite3_column_name16( stmt, 9 ), FW_LOG_COL_DESC ) == 0 ); 

	wcsncpy( log->desc, sqlite3_column_text16( stmt, 9 ), _MAX_DESC_LEN ); 

	ret = str2time( sqlite3_column_text16( stmt, 10 ), &log->time ); 

	ASSERT( ret == 0 ); 

	ret = ( ( fw_log_callback )callback_func )( log, param ); 

_return: 
	if( alloc_local == TRUE )
	{
		if( log != NULL )
		{
			free( log ); 
		}
	}

	return ret; 
}

INT32 CALLBACK read_rule_record( sqlite3_stmt *stmt, ULONG type,  PVOID read_callback, PVOID alloc_func, PVOID param )
{
	INT32 ret = SQLITE_OK; 
	INT32 alloc_local = FALSE;
	action_rule_record record; 


	ASSERT( stmt != NULL ); 
	ASSERT( read_callback != NULL ); 
	ASSERT( is_valid_access_rule_type( type ) ); 
	ASSERT( sqlite3_column_count( stmt ) == 14 ); 
	ASSERT( wcscmp(	( WCHAR* )sqlite3_column_name16( stmt, 0 ), L"type" ) == 0 ); 

	record.type = sqlite3_column_int( stmt, 0 ); 
	record.class_names[ 0 ] = sqlite3_column_text16( stmt, 1 ); 
	record.ids[ 0 ]= sqlite3_column_int( stmt, 2 ); 
	record.class_names[ 1 ] = sqlite3_column_text16( stmt, 3 ); 
	record.ids[ 1 ]= sqlite3_column_int( stmt, 4 ); 
	record.class_names[ 2 ] = sqlite3_column_text16( stmt, 5 ); 
	record.ids[ 2 ]= sqlite3_column_int( stmt, 6 ); 
	record.class_names[ 3 ] = sqlite3_column_text16( stmt, 7 ); 
	record.ids[ 3 ]= sqlite3_column_int( stmt, 8 ); 
	record.class_names[ 4 ] = sqlite3_column_text16( stmt, 9 ); 
	record.ids[ 4 ]= sqlite3_column_int( stmt, 10 );

	record.action = sqlite3_column_int( stmt, 11 ); 
	record.desc = sqlite3_column_text16( stmt, 12 ); 
	record.flag = sqlite3_column_int( stmt, 13 ); 

	ret = ( ( read_action_rule_callback )read_callback )( ( access_rule_type )type, &record, param ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "configure this rule error type is %ws configure flags is %u\n", 
			get_rule_type_desc( ( access_rule_type )type ), 
			record.flag ) ); 
		
		//DBG_BP(); 

		//ASSERT( FALSE ); 
		ret = SQLITE_ERROR; 
	}

_return: 
	return ret; 
}

LRESULT load_param_callback( param_define_type type, param_record *record, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_all_desc *param_load; 

	ASSERT( record != NULL ); 
	ASSERT( param != NULL ); 
	ASSERT( is_valid_param_define_type( type ) ); 

	param_load = ( param_all_desc* )param; 

	param_load->type = type; 
	param_load->is_cls = FALSE; 

	switch( type )
	{
	case URL_DEFINE: 
		ASSERT( wcslen( record->url.url ) < _MAX_URL_LEN ); 
		wcscpy( param_load->url.url, record->url.url ); 
		break; 
	case IP_DEFINE: 
		param_load->ip.ip_begin = record->ip.ip_begin; 
		param_load->ip.ip_end = record->ip.ip_end; 
		break; 
	case PORT_DEFINE: 
		param_load->port.port_begin = record->port.port_begin; 
		param_load->port.port_end = record->port.port_end; 
		break; 
	case FILE_DEFINE: 
		ASSERT( wcslen( record->file.file_path ) < _MAX_FILE_NAME_LEN ); 
		wcscpy( param_load->file.file_path, record->file.file_path ); 
		break; 
	case REG_DEFINE: 
		ASSERT( wcslen( record->reg.reg_path ) < _MAX_FILE_NAME_LEN ); 
		wcscpy( param_load->reg.reg_path, record->reg.reg_path ); 
		break; 
	case COM_DEFINE: 
		ASSERT( wcslen( record->com.com_name ) < _MAX_COM_NAME_LEN ); 
		wcscpy( param_load->com.com_name, record->com.com_name ); 
		break; 
	case APP_DEFINE: 
		ASSERT( wcslen( record->app.app_name ) < _MAX_COM_NAME_LEN ); 
		wcscpy( param_load->app.app_name, record->app.app_name ); 
		break; 
	default:
		ASSERT( FALSE ); 
		break; 
	}

_return:
	return ret; 
}

INT32 CALLBACK read_param_record( sqlite3_stmt *stmt, ULONG type, PVOID read_callback_func, PVOID alloc_func, PVOID param )
{
	INT32 ret = SQLITE_OK; 
	param_record record; 

	ASSERT( stmt != NULL ); 
	ASSERT( is_valid_param_define_type( type ) ); 
	ASSERT( read_callback_func != NULL ); 

	switch( type )
	{
	case COM_DEFINE:
		ASSERT( sqlite3_column_count( stmt ) == 5 ); 
		ASSERT( wcscmp(	( WCHAR* )sqlite3_column_name16( stmt, 1 ), L"com_name" ) == 0 ); 

		record.com.id = sqlite3_column_int( stmt, 0 ); 
		record.com.com_name = sqlite3_column_text16( stmt, 1 ); 
		record.com.class_name = sqlite3_column_text16( stmt, 2 ); 
		record.com.desc = sqlite3_column_text16( stmt, 3 ); 
		record.com.flag = sqlite3_column_int( stmt, 4 ); 

		break; 

	case FILE_DEFINE:
		ASSERT( sqlite3_column_count( stmt ) == 5 ); 
		ASSERT( wcscmp(	( WCHAR* )sqlite3_column_name16( stmt, 1 ), L"file_path" ) == 0 ); 

		record.file.id = sqlite3_column_int( stmt, 0 ); 
		record.file.file_path = sqlite3_column_text16( stmt, 1 ); 
		record.file.class_name = sqlite3_column_text16( stmt, 2 ); 
		record.file.desc = sqlite3_column_text16( stmt, 3 ); 
		record.file.flag = sqlite3_column_int( stmt, 4 ); 

		break; 

	case REG_DEFINE:
		ASSERT( sqlite3_column_count( stmt ) == 5 ); 
		ASSERT( wcscmp(	( WCHAR* )sqlite3_column_name16( stmt, 1 ), L"reg_path" ) == 0 ); 
		record.reg.id = sqlite3_column_int( stmt, 0 ); 		record.reg.reg_path = sqlite3_column_text16( stmt, 1 ); 

		record.reg.class_name = sqlite3_column_text16( stmt, 2 ); 
		record.reg.desc = sqlite3_column_text16( stmt, 3 ); 
		record.reg.flag = sqlite3_column_int( stmt, 4 ); 

		break; 

	case APP_DEFINE:
		ASSERT( sqlite3_column_count( stmt ) == 5 ); 
		ASSERT( wcscmp(	( WCHAR* )sqlite3_column_name16( stmt, 1 ), L"name" ) == 0 ); 

		record.app.id = sqlite3_column_int( stmt, 0 )
; 
		record.app.app_name = sqlite3_column_text16( stmt, 1 ); 

		record.app.class_name = sqlite3_column_text16( stmt, 2 ); 
		record.app.desc = sqlite3_column_text16( stmt, 3 ); 
		record.app.flag = sqlite3_column_int( stmt, 4 ); 

		break; 

	case PORT_DEFINE:
		ASSERT( sqlite3_column_count( stmt ) == 7 ); 
		ASSERT( wcscmp(	( WCHAR* )sqlite3_column_name16( stmt, 1 ), L"port_begin" ) == 0 ); 

		record.port.id = sqlite3_column_int( stmt, 0 ); 
		record.port.port_begin = sqlite3_column_int64( stmt, 1 ); 
		record.port.port_end = sqlite3_column_int64( stmt, 2 ); 
		record.port.class_name = sqlite3_column_text16( stmt, 3 ); 
		record.port.type = ( prot_type )sqlite3_column_int( stmt, 4 ); 
		record.port.desc = sqlite3_column_text16( stmt, 5 ); 
		record.port.flag = sqlite3_column_int( stmt, 6 ); 

		break; 

	case IP_DEFINE:
		ASSERT( sqlite3_column_count( stmt ) == 6 ); 
		ASSERT( wcscmp(	( WCHAR* )sqlite3_column_name16( stmt, 1 ), L"ip_begin" ) == 0 ); 

		record.ip.id = sqlite3_column_int( stmt, 0 );
		record.ip.ip_begin = sqlite3_column_int64( stmt, 1 ); 
		record.ip.ip_end = sqlite3_column_int64( stmt, 2 );  
		record.ip.class_name = sqlite3_column_text16( stmt, 3 ); 
		record.ip.desc = sqlite3_column_text16( stmt, 4 ); 
		record.ip.flag = sqlite3_column_int( stmt, 5 ); 

		break; 

	case URL_DEFINE:
		ASSERT( sqlite3_column_count( stmt ) == 5 ); 
		ASSERT( wcscmp(	( WCHAR* )sqlite3_column_name16( stmt, 1 ), L"url" ) == 0 ); 

		__Trace( _T( "loading the url define, which class name is %s \n" ), sqlite3_column_text16( stmt, 2 ) ); 

		record.url.id = sqlite3_column_int( stmt, 0 ); 
		record.url.url = sqlite3_column_text16( stmt, 1 ); 
		record.url.class_name = sqlite3_column_text16( stmt, 2 ); 
		; 
		record.url.desc = sqlite3_column_text16( stmt, 3 ); 
		record.url.flag = sqlite3_column_int( stmt, 4 ); 

		break; 

	case COMMON_DEFINE:
		record.common.id = sqlite3_column_int( stmt, 0 )
			; 
		record.common.name = sqlite3_column_text16( stmt, 1 ); 

		record.common.class_name = sqlite3_column_text16( stmt, 2 ); 
		record.common.desc = sqlite3_column_text16( stmt, 3 ); 
		record.common.flag = sqlite3_column_int( stmt, 4 ); 
		break; 
	default:
		ASSERT( FALSE ); 
		break; 
	}

	ret = ( ( read_param_record_callback )read_callback_func )( ( param_define_type )type, &record, param ); 
_return: 
	return ret; 
}

INT32 CALLBACK read_drv_type( sqlite3_stmt *stmt, ULONG type, PVOID read_callback_func, PVOID alloc_func, PVOID param )
{
	INT32 ret = SQLITE_OK; 
	ULONG drv_type; 
	ULONG state; 

	ASSERT( stmt != NULL ); 
	ASSERT( read_callback_func != NULL ); 

	drv_type =( ULONG ) sqlite3_column_int( stmt, 0 ); 
	state = ( ULONG )sqlite3_column_int( stmt, 1 ); 

	if( FALSE == is_valid_bitsafe_function_type( ( bitsafe_function_type )drv_type ) )
	{
		ASSERT( FALSE && "why got the invalid driver type value from config db" ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	if( FALSE == is_valid_bitsafe_function_state( ( bitsafe_function_state )state ) )
	{
		ASSERT( FALSE && "why got the invalid driver state value from config db" ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	ret = ( ( drv_state_callback )read_callback_func )( ( bitsafe_function_type )drv_type, ( bitsafe_function_state )state, param ); 
_return: 
	return ret; 
}

INT32 CALLBACK read_common_conf( sqlite3_stmt *stmt, ULONG type, PVOID read_callback_func, PVOID alloc_func, PVOID param )
{
	INT32 ret = SQLITE_OK; 
	general_conf conf_val; 

	ASSERT( stmt != NULL ); 
	ASSERT( read_callback_func != NULL ); 

	conf_val.val_name = sqlite3_column_text16( stmt, 0 ); 
	conf_val.val_data = sqlite3_column_text16( stmt, 1 ); 

	if( L'\0' == conf_val.val_name[ 0 ] )
	{
		ASSERT( FALSE && "why got the invalid driver type value from config db" ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	//if( 0 == conf_val.val_data[ 0 ] )
	//{
	//	ASSERT( FALSE && "why got the invalid driver state value from config db" ); 
	//	ret = ERROR_ERRORS_ENCOUNTERED; 
	//	goto _return; 
	//}

	ret = ( ( general_conf_callback )read_callback_func )( 0, &conf_val, param ); 
_return: 
	return ret; 
}

INT32 _get_fw_log( PVOID handle, fw_log_callback output_func, alloc_record_callback alloc_func, PVOID param )
{
	INT32 ret = 0; 
	data_process data_proc = { 0 }; 

	ASSERT( handle != NULL ); 
	ASSERT( output_func != NULL ); 

	data_proc.data_proc_func = process_fw_log; 
	data_proc.callback_func = output_func; 
	data_proc.alloc_func = alloc_func; 
	data_proc.param = param; 

	ret = sql_exec( ( sqlite3* )handle, _T( "select * from " ) FW_LOG_TABLE, &data_proc );
	return ret; 
}

#define CLASS_COLUMN_NAME _T( "class_name" )
#define ID_COLUMN_NAME _T( "id" )

LRESULT get_table_name_for_define( param_define_type type, sqlite3 **db, LPCWSTR *table_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR _table_name = NULL; 
	sqlite3 *_db = NULL; 

	//ASSERT( db != NULL ); 
	ASSERT( table_name != NULL ); 

	ASSERT( is_valid_param_define_type( type ) ); 

	if( db != NULL )
	{
		*db = NULL; 
	}

	*table_name = NULL; 

	switch( type )
	{
	case URL_DEFINE:
		_table_name = FW_URL_DEFINE_TABLE; 
		_db = param_define_db; 
		break; 
	case IP_DEFINE: 
		_table_name = FW_IP_DEFINE_TABLE; 
		_db = param_define_db; 
		break; 
	case PORT_DEFINE:
		_table_name = FW_PORT_DEFINE_TABLE; 
		_db = param_define_db; 
		break; 
	case FILE_DEFINE:
		_table_name = DEFENSE_FILE_PROTECT_TABLE; 
		_db = param_define_db; 
		break; 
	case REG_DEFINE: 
		_table_name = DEFENSE_REG_PROTECT_TABLE; 
		_db = param_define_db; 
		break; 
	case COM_DEFINE: 
		_table_name = DEFENSE_COM_PROTECT_TABLE; 
		_db = param_define_db; 
		break; 
	case APP_DEFINE: 
		_table_name = APP_DEFINE_TABLE; 
		_db = param_define_db; 
		break; 
	case COMMON_DEFINE: 
		_table_name = COMMON_DEFINE_TABLE; 
		_db = param_define_db; 
		break; 
	default:
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		break; 
	}

	*table_name = _table_name; 
	if( db != NULL )
	{
		*db = _db; 
	}
	return ret; 
}

INT32 CALLBACK load_param_from_record( param_define_type type, param_record *record, PVOID param  )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_all_desc *param_load; 

	ASSERT( is_valid_param_define_type( type ) );
	ASSERT( param != NULL ); 
	ASSERT( record != NULL ); 

	param_load = ( param_all_desc* )param; 

	ASSERT( param_load->type == type ); 
	ASSERT( param_load->is_cls == TRUE ? param_load->id == INVALID_ID_VALUE : param_load->id != INVALID_ID_VALUE ); 

	switch( type )
	{
	case COM_DEFINE:

		if( wcslen( record->com.com_name ) >= _MAX_COM_NAME_LEN )
		{
			ret = ERROR_DB_DATA_CURRUPT; 
			goto _return; 
		}

		wcscpy( param_load->com.com_name, record->com.com_name ); 
		break; 

	case FILE_DEFINE:

		if( wcslen( record->file.file_path ) >= _MAX_FILE_NAME_LEN )
		{
			ret = ERROR_DB_DATA_CURRUPT; 
			goto _return; 
		}

		wcscpy( param_load->file.file_path, record->file.file_path ); 
		break; 

	case REG_DEFINE:

		if( wcslen( record->reg.reg_path ) >= _MAX_REG_PATH_LEN )
		{
			ret = ERROR_DB_DATA_CURRUPT; 
			goto _return; 
		}

		wcscpy( param_load->reg.reg_path, record->reg.reg_path ); 
		break; 

	case APP_DEFINE:
		if( wcslen( record->app.app_name ) >= _MAX_FILE_NAME_LEN )
		{
			ret = ERROR_DB_DATA_CURRUPT; 
			goto _return; 
		}

		wcscpy( param_load->app.app_name, record->app.app_name ); 
		break; 

	case PORT_DEFINE:
		param_load->port.port_begin = record->port.port_begin; 
		param_load->port.port_end = record->port.port_end; 
		param_load->port.type = record->port.type; 
		break; 

	case IP_DEFINE:
		param_load->ip.ip_begin = record->ip.ip_begin; 
		param_load->ip.ip_end = record->ip.ip_end; 
		break; 

	case URL_DEFINE:
		if( wcslen( record->url.url ) >= _MAX_URL_LEN )
		{
			ret = ERROR_DB_DATA_CURRUPT; 
			goto _return; 
		}

		wcscpy( param_load->url.url, record->url.url ); 
		break; 

	case COMMON_DEFINE: 
		if( wcslen( record->common.name ) >= _MAX_FILE_NAME_LEN )
		{
			ret = ERROR_DB_DATA_CURRUPT; 
			goto _return; 
		}

		wcscpy( param_load->common.name, record->common.name ); 
		break; 
	default:
		ASSERT( FALSE ); 
		break; 
	}

_return:
	return ret; 
}

LRESULT _load_param_define( param_define_type type, ULONG id, LPCWSTR class_name, read_param_record_callback callback_func, alloc_record_callback alloc_func, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	sql_stmt_desc *sql_stmt = NULL; 
	WCHAR id_val[ 64 ]; 
	LPCWSTR table_name; 
	sqlite3 *db; 
	data_process db_process; 

	get_table_name_for_define( type, &db, &table_name ); 
	ret = alloc_sql_stmt_desc( SELECT_ROW, table_name, &sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( class_name != NULL )
	{	
		ret = add_col_val( sql_stmt, CLASS_COLUMN_NAME, class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}
	else
	{ 

		ret = add_col_val_int( sql_stmt, ID_COLUMN_NAME, id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

	ret = generate_sql_stmt( sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	db_process.alloc_func = alloc_func; 
	db_process.callback_func = callback_func; 
	db_process.data_proc_func = read_param_record; 
	db_process.param = param; 
	db_process.type = ( ULONG )type; 

	ret = sql_exec( db, sql_stmt->sql, &db_process ); 
	if( ret != SQLITE_OK )
	{
		ret = ERROR_DB_SELECT_ERROR; 
		goto _return; 
	}

_return:

	if( sql_stmt != NULL )
	{
		release_sql_stmt_desc( sql_stmt ); 
	}
	return ret; 
}

#define SQL_SELECT_RULE _T( "select * from %s where type=%d;" )
LRESULT get_rule( access_rule_type type, read_action_rule_callback output_func, alloc_record_callback alloc_func, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	data_process data_proc = { 0 }; 
	LPCTSTR tbl_name; 
	sqlite3 *db; 
	row_data_callback proc_data_func; 
	TCHAR sql_stmt[ SQL_STMT_LEN ]; 

	ASSERT( output_func != NULL ); 

	ASSERT( is_valid_access_rule_type( type ) == TRUE ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 
	ret = get_context_for_rule( type, &tbl_name, &proc_data_func, &db ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ASSERT( db != NULL ); 
	ret = _sntprintf( sql_stmt, SQL_STMT_LEN, SQL_SELECT_RULE, tbl_name, type ); 
	if( ret <= 0 )
	{
		ret = SQLITE_ERROR; 
		goto _return; 
	}

	log_trace( ( MSG_INFO, "select rule sql is %ws \n", sql_stmt) ); 

	data_proc.data_proc_func = proc_data_func; 
	data_proc.alloc_func = alloc_func; 
	data_proc.callback_func = output_func; 
	data_proc.param = param; 
	data_proc.type = type; 

	ret = sql_exec( ( sqlite3* )db, sql_stmt, &data_proc );
	if( ret != SQLITE_OK )
	{
		ret = ERROR_DB_SELECT_ERROR; ; 
		goto _return; 
	}

_return:

	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 
	return ret; 
}


LRESULT load_param_define( param_all_desc *param_load )
{
	ULONG id; 
	LPCWSTR class_name; 
	LRESULT ret = ERROR_SUCCESS;

	if( param_load->is_cls == TRUE )
	{
		id = INVALID_ID_VALUE ; 
		class_name = param_load->cls.class_name; 
	}
	else 
	{
		class_name = NULL; 
		id = param_load->id; 
	}

	ASSERT( ( class_name != NULL && *class_name != L'\0' ) ? id == INVALID_ID_VALUE : id != INVALID_ID_VALUE ); 

	ret = _load_param_define( param_load->type, id, class_name, load_param_from_record, NULL, ( PVOID )param_load ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT construct_sql_stmt_by_param( sql_action action, param_input_desc *param_input, sql_stmt_desc **sql_stmt_alloc, sqlite3 **db )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR tbl_name; 
	LPCWSTR col_name = NULL; 
	sqlite3 *_db; 
	sql_stmt_desc *sql_stmt = NULL; 
	PARAM_ID id; 

	ASSERT( is_valid_sql_action( action ) ); 
	ASSERT( param_input != NULL ); 
	ASSERT( sql_stmt_alloc != NULL ); 

	if( db != NULL )
	{
		*db = NULL; 
	}

	*sql_stmt_alloc = NULL; 

	ret = get_table_name_for_define( param_input->type, &_db, &tbl_name ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

#define CLASS_COL_NAME _T( "class" )
#define URL_COL_NAME _T( "url" )
#define BEGIN_IP_COL_NAME _T( "ip_begin" )
#define END_IP_COL_NAME _T( "ip_end" )
#define BEGIN_PORT_COL_NAME _T( "port_begin" )
#define END_PORT_COL_NAME _T( "port_end" )
#define PORT_TYPE_COL_NAME _T( "type" )
#define APP_COL_NAME _T( "name" )
#define NAME_COL_NAME _T( "name" )
#define REG_COL_NAME _T( "reg_path" )
#define FILE_COL_NAME _T( "file_path" )
#define COM_COL_NAME _T( "com_name" )
#define ID_COL_NAME _T( "id" )

	ret = alloc_sql_stmt_desc( action, tbl_name, &sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	id = param_input->id; 

	if( param_input->is_cls == TRUE )
	{
		if( param_input->id != INVALID_ID_VALUE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
		col_name = CLASS_COL_NAME; 
	}
	else
	{
		switch( param_input->type )
		{
		case URL_DEFINE:
			col_name = URL_COL_NAME; 
			break; 
		case IP_DEFINE: 
			if( param_input->ip.ip_end != 0 || param_input->ip.ip_begin != 0 )
			{
				ret = add_col_val_int( sql_stmt, BEGIN_IP_COL_NAME, param_input->ip.ip_begin ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				ret = add_col_val_int( sql_stmt, END_IP_COL_NAME, param_input->ip.ip_end ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}
			}

			break; 
		case PORT_DEFINE:
			if( param_input->port.port_begin != 0 || param_input->port.port_end != 0 )
			{
				ret = add_col_val_int( sql_stmt, BEGIN_PORT_COL_NAME, param_input->port.port_begin ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				ret = add_col_val_int( sql_stmt, END_PORT_COL_NAME, param_input->port.port_end ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}
			}

			if( is_valid_prot_type( param_input->port.type ) )
			{
				ret = add_col_val_int( sql_stmt, PORT_TYPE_COL_NAME, param_input->port.type ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}
			}
			break; 
		case FILE_DEFINE:
			col_name = FILE_COL_NAME; 
			break; 
		case REG_DEFINE: 
			col_name = REG_COL_NAME; 
			break; 
		case COM_DEFINE: 
			col_name = COM_COL_NAME; 
			break; 
		case APP_DEFINE:
			col_name = APP_COL_NAME; 
			break; 
		case COMMON_DEFINE:
			col_name = NAME_COL_NAME; 
			break; 
		default:
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}

	if( id != INVALID_ID_VALUE )
	{
			ret = add_col_val_int( sql_stmt, ID_COL_NAME, id ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
	}
	else if( col_name != NULL )
	{
		if( param_input->common.name != NULL 
			/*&& *param_input->common.name != L'\0'*/ )
		{
			ret = add_col_val( sql_stmt, col_name, param_input->common.name ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}
	}

	ret = generate_sql_stmt( sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	*db = _db; 

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( sql_stmt != NULL )
		{
			release_sql_stmt_desc( sql_stmt ); 
		}
	}
	else
	{
		*sql_stmt_alloc = sql_stmt; 
	}
	return ret; 
}

LRESULT _construct_sql_stmt_by_param( sql_action action, param_all_desc *param_input, sql_stmt_desc **sql_stmt_alloc, sqlite3 **db )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR tbl_name; 
	LPCWSTR col_name = NULL; 
	sqlite3 *_db; 
	sql_stmt_desc *sql_stmt = NULL; 
	PARAM_ID id; 

	ASSERT( is_valid_sql_action( action ) ); 
	ASSERT( param_input != NULL ); 
	ASSERT( sql_stmt_alloc != NULL ); 

	if( db != NULL )
	{
		*db = NULL; 
	}

	*sql_stmt_alloc = NULL; 

	ret = get_table_name_for_define( param_input->type, &_db, &tbl_name ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

#define CLASS_COL_NAME _T( "class" )
#define URL_COL_NAME _T( "url" )
#define BEGIN_IP_COL_NAME _T( "ip_begin" )
#define END_IP_COL_NAME _T( "ip_end" )
#define BEGIN_PORT_COL_NAME _T( "port_begin" )
#define END_PORT_COL_NAME _T( "port_end" )
#define PORT_TYPE_COL_NAME _T( "type" )
#define APP_COL_NAME _T( "name" )
#define REG_COL_NAME _T( "reg_path" )
#define FILE_COL_NAME _T( "file_path" )
#define COM_COL_NAME _T( "com_name" )
#define ID_COL_NAME _T( "id" )

	ret = alloc_sql_stmt_desc( action, tbl_name, &sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	id = param_input->id; 

	if( param_input->is_cls == TRUE )
	{
		if( param_input->id != INVALID_ID_VALUE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
		col_name = CLASS_COL_NAME; 
	}
	else
	{
		switch( param_input->type )
		{
		case URL_DEFINE:
			col_name = URL_COL_NAME; 
			break; 
		case IP_DEFINE: 
			if( param_input->ip.ip_end != 0 
				&& param_input->ip.ip_end <  param_input->ip.ip_begin  )
			{
				ret = ERROR_INVALID_PARAMETER; 
				goto _return; 
			}

			ret = add_col_val_int( sql_stmt, BEGIN_IP_COL_NAME, param_input->ip.ip_begin ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			ret = add_col_val_int( sql_stmt, END_IP_COL_NAME, param_input->ip.ip_end ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			break; 
		case PORT_DEFINE:
			if( param_input->port.port_end != 0 
				&& param_input->port.port_end < param_input->port.port_begin )
			{
				ret = ERROR_INVALID_PARAMETER; 
				goto _return; 
			}

			ret = add_col_val_int( sql_stmt, BEGIN_PORT_COL_NAME, param_input->port.port_begin ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			ret = add_col_val_int( sql_stmt, END_PORT_COL_NAME, param_input->port.port_end ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			if( is_valid_prot_type( param_input->port.type ) )
			{
				ret = add_col_val_int( sql_stmt, PORT_TYPE_COL_NAME, param_input->port.type ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}
			}
			break; 
		case FILE_DEFINE:
			col_name = FILE_COL_NAME; 
			break; 
		case REG_DEFINE: 
			col_name = REG_COL_NAME; 
			break; 
		case COM_DEFINE: 
			col_name = COM_COL_NAME; 
			break; 
		case APP_DEFINE:
			col_name = APP_COL_NAME; 
			break; 
		case COMMON_DEFINE:
			col_name = NAME_COL_NAME; 
			break; 
		default:
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}

	if( id != INVALID_ID_VALUE )
	{
		ret = add_col_val_int( sql_stmt, ID_COL_NAME, id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}
	else if( col_name != NULL )
	{
		if( param_input->common.name != NULL )
		{
			ASSERT( is_name_param_define_type( param_input->type ) == TRUE ); 
			//if( is_name_param_define_type( param_input->type ) == TRUE 
			//	&& *param_input->common.name == L'\0' )
			//{
			//	ret = ERROR_INVALID_PARAMETER; 
			//	goto _return; 
			//}

			ret = add_col_val( sql_stmt, col_name, param_input->common.name ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}
	}

	ret = generate_sql_stmt( sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( db != NULL )
	{
		*db = _db; 
	}

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( sql_stmt != NULL )
		{
			release_sql_stmt_desc( sql_stmt ); 
		}
	}
	else
	{
		*sql_stmt_alloc = sql_stmt; 
	}

	return ret; 
}

LRESULT get_param_define( param_input_desc *param_input, read_param_record_callback output_func, alloc_record_callback alloc_func, PVOID param )
{
	INT32 ret = ERROR_SUCCESS; 
	data_process data_proc = { 0 }; 
	//LPCTSTR tbl_name; 
	sqlite3 *db; 
	row_data_callback proc_data_func; 
	sql_stmt_desc *sql_stmt; 

	ASSERT( output_func != NULL ); 
	ASSERT( param_input != NULL ); 

	proc_data_func = read_param_record; 

	ret = construct_sql_stmt_by_param( SELECT_ROW, param_input, &sql_stmt, &db ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ASSERT( db != NULL ); 

	data_proc.data_proc_func = proc_data_func; 
	data_proc.alloc_func = alloc_func; 
	data_proc.callback_func = output_func; 
	data_proc.param = param; 
	data_proc.type = param_input->type; 

	ret = sql_exec( ( sqlite3* )db, sql_stmt->sql, &data_proc );

_return:
	return ret; 
}

INT32 CALLBACK process_defense_log( sqlite3_stmt *stmt, ULONG type, PVOID callback_func, PVOID alloc_func, PVOID param )
{
	INT32 ret = 0; 
	defense_log *log = NULL; 
	INT32 alloc_local = FALSE; 

	ASSERT( callback_func != NULL ); 

	if( alloc_func != NULL )
	{
		ret = ( ( alloc_record_callback )alloc_func )( ( PVOID* )&log, param ); 
	}

	if( ret < 0 || log == NULL )
	{
		log = ( pdefense_log )malloc( sizeof( defense_log ) ); 
		if( log == NULL )
		{
			ret = SQLITE_NOMEM; 
			goto _return; 
		}

		alloc_local = TRUE; 
	}

	log->id = sqlite3_column_int( stmt, 0 ); 
	wcsncpy( log->proc_name, sqlite3_column_text16( stmt, 1 ), _MAX_FILE_NAME_LEN ); 

	ASSERT( wcscmp( ( WCHAR* )sqlite3_column_name16( stmt, 1 ), L"proc_info" ) == 0 ); 

	log->flag = ( sys_action_type )sqlite3_column_int( stmt, 2 ); 

	log->action = ( action_response_type )sqlite3_column_int( stmt, 3 ); 

	wcsncpy( log->target_desc, sqlite3_column_text16( stmt, 4 ), _MAX_FILE_NAME_LEN ); 

	str2time( sqlite3_column_text16( stmt, 5 ), &log->time ); 

	ret = ( ( defense_log_callback )callback_func )( 
		log, 
		param ); 

_return:
	if( alloc_local == TRUE )
	{
		free( log ); 
	}
	return ret; 
}

INT32 _get_defense_log( PVOID handle, defense_log_callback output_func, alloc_record_callback alloc_func, PVOID param )
{
	INT32 ret = 0;  
	data_process data_proc = { 0 }; 

	ASSERT( handle != NULL ); 
	ASSERT( output_func != NULL ); 

	data_proc.param = param; 
	data_proc.callback_func = output_func; 
	data_proc.alloc_func = alloc_func; 
	data_proc.data_proc_func = process_defense_log; 
	ret = sql_exec( ( sqlite3* )handle, _T( "select * from " ) DEFENSE_LOG_TABLE, &data_proc ); 
	return ret; 
}

LRESULT CALLBACK alloc_defense_log_debug( PVOID *log, PVOID param )
{
	INT32 ret = SQLITE_OK; 
	defense_log *_log; 

	_log = ( defense_log* )malloc( sizeof( defense_log ) ); 
	if( _log != NULL )
	{
		ret = SQLITE_NOMEM; 
		goto _return; 
	}

	*log = ( PVOID )_log; 
_return:

	return ret; 
}

LRESULT get_defense_log( defense_log_callback output_func, alloc_record_callback alloc_func, PVOID param )
{
	INT32 ret = 0;  

	ASSERT( output_func != NULL ); 

	ret = _get_defense_log( defense_log_db, output_func, alloc_defense_log_debug, param ); 
	
	return ret; 
}

INT32 add_url_param_record( PVOID handle, LPCTSTR url, LPCTSTR class_name, LPCTSTR desc, ULONG flag )
{
	INT32 ret = 0; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0; 

	ASSERT( handle != NULL ); 
	//ASSERT( url != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	if( url == NULL )
	{
		url = _T( "" ); 
	}

	if( class_name == NULL )
	{
		class_name = _T( "" );  
	}

	if( desc == NULL )
	{
		desc = _T( "" ); 
	}

	if( ( add_time % CHECK_DB_OVERFLOW_TIME ) == 0 )
	{
		truncate_db( ( sqlite3* )handle, 
			FW_URL_DEFINE_TABLE, 
			MAX_DEFINE_NUM, 
			DEL_DEFINE_ONCE
			); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = SQLITE_NOMEM; 
		goto _return; 
	}

	ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_INSERT_URL_DEFINE, url, class_name, desc, flag ); 

	if( ret <= 0 )
	{
		ret = SQLITE_ERROR; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )handle, sql_stmt, NULL ); 
	if( ret == SQLITE_OK )
	{
		add_time ++; 
	}
	else if( ret == SQLITE_CONSTRAINT )
	{
#ifdef _DEBUG
		LRESULT _ret; 
		ULONG count; 
		data_process data_proc = { 0 }; 
		data_proc.data_proc_func = check_result_count; 
		data_proc.param = &count; 

		_ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, _T( "SELECT COUNT(*) FROM " ) FW_URL_DEFINE_TABLE _T( " WHERE url='%s'" ), url ); 

		if( _ret <= 0 )
		{
			ret = SQLITE_ERROR; 
			goto _return; 
		}

		_ret = sql_exec( ( sqlite3* )handle, sql_stmt, &data_proc ); 
		if( _ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "other database contraint failed \n" ) ); 
		}

		log_trace( ( MSG_INFO, "check the failed insert row, this count is %d \n", count ) ); 
#endif //_DEBUG

		ret = SQLITE_OK; 
	}

	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s insert new record failed: %d, sql %ws extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )handle ), sqlite3_errmsg16( ( sqlite3* )handle ) );
	}

_return:
 	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT add_ip_param_record( PVOID handle, ULONGLONG ip_begin, ULONGLONG ip_end, LPCTSTR class_name, LPCTSTR desc, ULONG flag )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0; 

	ASSERT( handle != NULL ); 
	
	if( ip_end != 0 && ip_end < ip_begin )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( class_name == NULL )
	{
		class_name = _T( "" );  
	}

	if( desc == NULL )
	{
		desc = _T( "" ); 
	}

	if( ( add_time % CHECK_DB_OVERFLOW_TIME ) == 0 )
	{
		truncate_db( ( sqlite3* )handle, 
			FW_IP_DEFINE_TABLE, 
			MAX_DEFINE_NUM, 
			DEL_DEFINE_ONCE
			); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_INSERT_IP_DEFINE, ip_begin, ip_end, class_name, desc, flag ); 

	if( ret <= 0 )
	{
		ret = ERROR_SQL_STMT_BUFFER_OVERFLOW; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )handle, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s insert new record failed: %d, reason %s\n" ), __TFUNCTION__, ret, sqlite3_errmsg16( ( sqlite3* )handle ) );

		if( ret == SQLITE_CONSTRAINT )
		{
			ret = ERROR_SUCCESS; 
			goto _return; 
		}
		
		ret = ERROR_DB_INSERT_ROW_ERROR; 
		goto _return; 
	}
	else 
	{
		add_time ++; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

LRESULT add_port_param_record( PVOID handle, ULONG port_begin, ULONG port_end, ULONG type, LPCTSTR class_name, LPCTSTR desc, ULONG flag )
{
	INT32 ret = 0; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0; 

	ASSERT( handle != NULL ); 
	ASSERT( is_valid_prot_type( type ) == TRUE  ); 

	if( port_end != 0 && port_end < port_begin )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( class_name == NULL )
	{
		class_name = _T( "" );  
	}

	if( desc == NULL )
	{
		desc = _T( "" ); 
	}

	if( ( add_time % CHECK_DB_OVERFLOW_TIME ) == 0 )
	{
		truncate_db( ( sqlite3* )handle, 
			FW_PORT_DEFINE_TABLE, 
			MAX_DEFINE_NUM, 
			DEL_DEFINE_ONCE
			); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_INSERT_PORT_DEFINE, port_begin, port_end, type, class_name, desc, flag ); 

	if( ret <= 0 )
	{
		ret = ERROR_SQL_STMT_BUFFER_OVERFLOW; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )handle, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s insert new record failed: %d, extended error code is %d reason %s\n" ), __TFUNCTION__, ret, sqlite3_extended_errcode( ( sqlite3* )handle ), sqlite3_errmsg16( ( sqlite3* )handle ) );
		if( ret == SQLITE_CONSTRAINT )
		{
			ret = SQLITE_OK; 
		}
		else 
		{
			ret = ERROR_DB_INSERT_ROW_ERROR; 
		}
	}
	else
	{
		add_time ++; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

INT32 add_app_param_record( PVOID handle, LPCTSTR name, LPCTSTR class_name, LPCTSTR desc, ULONG flag )
{
	INT32 ret = 0; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0; 

	ASSERT( handle != NULL ); 

	if( class_name == NULL )
	{
		class_name = _T( "" );  
	}

	if( desc == NULL )
	{
		desc = _T( "" ); 
	}

	if( name == NULL )
	{
		//ASSERT( FALSE ); 
		name = _T( "" ); 
	}

	if( ( add_time % CHECK_DB_OVERFLOW_TIME ) == 0 )
	{
		truncate_db( ( sqlite3* )handle, 
			APP_DEFINE_TABLE, 
			MAX_DEFINE_NUM, 
			DEL_DEFINE_ONCE
			); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = SQLITE_NOMEM; 
		goto _return; 
	}

	ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_INSERT_APP_DEFINE, name, class_name, desc, flag ); 
	if( ret <= 0 )
	{
		ret = SQLITE_ERROR; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )handle, sql_stmt, NULL ); 
	if( ret == SQLITE_OK )
	{
		add_time ++; 
	}
	else if( ret == SQLITE_CONSTRAINT )
	{
		__Trace( _T( "%s insert new record failed: %d, extented error code %d reason %s\n" ), __TFUNCTION__, ret, sqlite3_extended_errcode( ( sqlite3* )handle ), sqlite3_errmsg16( ( sqlite3* )handle ) );
		ret = SQLITE_OK; 
	}
	
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s insert new record failed: %d, extented error code %d reason %s\n" ), __TFUNCTION__, ret, sqlite3_extended_errcode( ( sqlite3* )handle ), sqlite3_errmsg16( ( sqlite3* )handle ) );
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

INT32 add_rule( LPCTSTR class_name, 
				  ULONG id, 
				  LPCTSTR class_name2, 
				  ULONG id2, 
				  LPCTSTR class_name3, 
				  ULONG id3, 
				  LPCTSTR class_name4, 
				  ULONG id4, 
				  LPCTSTR class_name5, 
				  ULONG id5, 
				  action_response_type action, 
				  access_rule_type type, 
				  LPCTSTR desc, 
				  ULONG flag )
{
	INT32 ret = 0; 
	TCHAR *sql_stmt = NULL; 
	TCHAR *sql_fmt = NULL; 
	//TCHAR *table_name; 
	sqlite3 *db; 
	static ULONG fw_rule_add_time = 0; 
	static ULONG defense_rule_add_time = 0; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ASSERT( is_valid_access_rule_type( type ) ); 

	if( class_name == NULL )
	{
		class_name = L""; 
	}

	if( class_name2 == NULL )
	{
		class_name2 = L""; 
	}

	if( class_name3 == NULL )
	{
		class_name3 = L""; 
	}

	if( class_name4 == NULL )
	{
		class_name4 = L""; 
	}

	if( class_name5 == NULL )
	{
		class_name5 = L""; 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = SQLITE_NOMEM; 
		goto _return; 
	}

	switch( type )
	{
	case URL_RULE_TYPE: 
	case SOCKET_RULE_TYPE:  
		sql_fmt = SQL_INSERT_FW_RULE; 
		//table_name = FW_RULE_TABLE; 

		if( ( fw_rule_add_time & CHECK_DB_OVERFLOW_TIME ) == 0 )
		{
			truncate_db( action_rule_db, 
				FW_RULE_TABLE, 
				MAX_RULE_NUM, 
				DEL_RULE_ONCE
				); 
		}
		break; 
	case FILE_RULE_TYPE: 
	case REG_RULE_TYPE: 
	case COM_RULE_TYPE: 
	case COMMON_RULE_TYPE: 
		sql_fmt = SQL_INSERT_DEFENSE_RULE; 
		//table_name = DEFENSE_RULE_TABLE; 

		if( ( defense_rule_add_time & CHECK_DB_OVERFLOW_TIME ) == 0 )
		{
			truncate_db( action_rule_db, 
				DEFENSE_RULE_TABLE, 
				MAX_RULE_NUM, 
				DEL_RULE_ONCE
				); 
		}
		break; 
	default:
		ASSERT( "invalid rule type" && FALSE ); 
	}

	db = action_rule_db; 
	ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, sql_fmt, class_name, id, class_name2, id2, 
		class_name3, id3, class_name4, id4, class_name5, id5, action, type, desc, flag ); 

	if( ret <= 0 )
	{
		ret = SQLITE_ERROR; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )db, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
#define SQL_CHECK_FW_RULE _T( "SELECT COUNT(*) FROM " ) FW_RULE_TABLE _T( " WHERE class_name='%s' and id=%d and class_name2='%s' and id2=%d and class_name3='%s' and id3=%d and class_name4='%s' and id4=%d and class_name5='%s' and id5=%d " )

#define SQL_CHECK_DEFENSE_RULE _T( "SELECT COUNT(*) FROM " ) DEFENSE_RULE_TABLE _T( " WHERE class_name='%s' and id=%d and class_name2='%s' and id2=%d and class_name3='%s' and id3=%d and class_name4='%s' and id4=%d and class_name5='%s' and id5=%d " )

		__Trace( _T( "%s insert new rule failed: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );

		if( ret == SQLITE_CONSTRAINT )
		{
#ifdef _DEBUG
			LRESULT _ret; 
			ULONG count; 
			data_process data_proc = { 0 }; 
			
			data_proc.data_proc_func = check_result_count; 
			data_proc.param = &count; 

			switch( type )
			{

			case URL_RULE_TYPE: 
			case SOCKET_RULE_TYPE:  
				_sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_CHECK_FW_RULE, class_name, id, class_name2, id2, 
					class_name3, id3, class_name4, id4, class_name5, id5 ); 
				break; 
			case FILE_RULE_TYPE: 
			case REG_RULE_TYPE: 
			case COM_RULE_TYPE: 
			case COMMON_RULE_TYPE: 
				_sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_CHECK_DEFENSE_RULE, class_name, id, class_name2, id2, 
					class_name3, id3, class_name4, id4, class_name5, id5 ); 
				break; 
			}

			__Trace( _T( "%s sql %s extented error code %d reason %s\n" ), __TFUNCTION__, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );

			ret = sql_exec( ( sqlite3* )db, sql_stmt, &data_proc );  
			if( ret != SQLITE_OK )
			{
				__Trace( _T( "%s check rule failed: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );
			}

			log_trace( ( MSG_INFO, "check the insert operation failed reason, check the row to input already exist in the database %d\n", count ) ); 

			if( count == 0 )
			{
				ASSERT( FALSE && "insert row failed,but not is aleady exist error\n" ); 
			}
#endif //_DEBUG
			ret = SQLITE_OK; 
		}
	}
	else
	{
		switch( type )
		{

		case URL_RULE_TYPE: 
		case SOCKET_RULE_TYPE:  
			fw_rule_add_time ++; 
			break; 
		case FILE_RULE_TYPE: 
		case REG_RULE_TYPE: 
		case COM_RULE_TYPE: 
		case COMMON_RULE_TYPE: 
			defense_rule_add_time ++; 
			break; 
		}
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ret; 
}

INT32 check_rule( LPCTSTR class_name, 
			   ULONG id, 
			   LPCTSTR class_name2, 
			   ULONG id2, 
			   LPCTSTR class_name3, 
			   ULONG id3, 
			   LPCTSTR class_name4, 
			   ULONG id4, 
			   LPCTSTR class_name5, 
			   ULONG id5, 
			   action_response_type action, 
			   access_rule_type type, 
			   LPCTSTR desc, 
			   ULONG flag )
{
	INT32 ret = 0; 
	LRESULT _ret; 
	TCHAR *sql_stmt = NULL; 
	TCHAR *sql_fmt = NULL; 
	//TCHAR *table_name; 
	sqlite3 *db; 
	static ULONG fw_rule_add_time = 0; 
	static ULONG defense_rule_add_time = 0; 
	ULONG count = 0; 
	data_process data_proc = { 0 }; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ASSERT( is_valid_access_rule_type( type ) ); 

	if( class_name == NULL )
	{
		class_name = L""; 
	}

	if( class_name2 == NULL )
	{
		class_name2 = L""; 
	}

	if( class_name3 == NULL )
	{
		class_name3 = L""; 
	}

	if( class_name4 == NULL )
	{
		class_name4 = L""; 
	}

	if( class_name5 == NULL )
	{
		class_name5 = L""; 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = SQLITE_NOMEM; 
		goto _return; 
	}

	db = action_rule_db; 

#define SQL_CHECK_FW_RULE _T( "SELECT COUNT(*) FROM " ) FW_RULE_TABLE _T( " WHERE class_name='%s' and id=%d and class_name2='%s' and id2=%d and class_name3='%s' and id3=%d and class_name4='%s' and id4=%d and class_name5='%s' and id5=%d " )

#define SQL_CHECK_DEFENSE_RULE _T( "SELECT COUNT(*) FROM " ) DEFENSE_RULE_TABLE _T( " WHERE class_name='%s' and id=%d and class_name2='%s' and id2=%d and class_name3='%s' and id3=%d and class_name4='%s' and id4=%d and class_name5='%s' and id5=%d " )

	__Trace( _T( "%s insert new rule failed: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) ); 

	data_proc.data_proc_func = check_result_count; 
	data_proc.param = &count; 

	switch( type )
	{

	case URL_RULE_TYPE: 
	case SOCKET_RULE_TYPE:  
		_sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_CHECK_FW_RULE, class_name, id, class_name2, id2, 
			class_name3, id3, class_name4, id4, class_name5, id5 ); 
		break; 
	case FILE_RULE_TYPE: 
	case REG_RULE_TYPE: 
	case COM_RULE_TYPE: 
	case COMMON_RULE_TYPE: 
		_sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_CHECK_DEFENSE_RULE, class_name, id, class_name2, id2, 
			class_name3, id3, class_name4, id4, class_name5, id5 ); 
		break; 
	}

	__Trace( _T( "%s sql %s extented error code %d reason %s\n" ), __TFUNCTION__, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );

	ret = sql_exec( ( sqlite3* )db, sql_stmt, &data_proc );  
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s check rule failed: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );
	}

	log_trace( ( MSG_INFO, "check the insert operation failed reason, check the row to input already exist in the database %d\n", count ) ); 

	if( count == 0 )
	{
		ASSERT( FALSE && "check row failed,but not is aleady exist error\n" ); 
		ret = SQLITE_ERROR; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ret; 
}

LRESULT get_context_for_rule( access_rule_type type, LPCTSTR *tbl_name, row_data_callback *proc_data_func, sqlite3 **db )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( tbl_name != NULL ); 
	//ASSERT( proc_data_func != NULL ); 
	ASSERT( db != NULL ); 

	switch( type )
	{
	case URL_RULE_TYPE:
	case SOCKET_RULE_TYPE:
		*db = ( sqlite3* )action_rule_db; 
		*tbl_name = FW_RULE_TABLE; 

		if( proc_data_func != NULL )
		{
			*proc_data_func = read_rule_record; 
		}
		break; 

	case FILE_RULE_TYPE:
	case REG_RULE_TYPE:
	case COM_RULE_TYPE:
	case COMMON_RULE_TYPE:
		*db = ( sqlite3* )action_rule_db; 
		*tbl_name = DEFENSE_RULE_TABLE; 
		if( proc_data_func != NULL )
		{
			*proc_data_func = read_rule_record; 
		}
		break; 
	default:
		ASSERT( "invalid action rule type" && FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		break; 
	}

_return:
	return ret; 
}

#define EQUAL_SIGN _T( " = " )

VOID add_rule_sel_param( LPTSTR condition, ULONG id, ULONG index, LPCTSTR class_name, INT32 *added )
{
	INT32 _added = FALSE; 
	TCHAR id_desc[ 32 ]; 

	ASSERT( condition != NULL ); 
	ASSERT( added != NULL ); 
	ASSERT( index >= 1 ); 

	if( id != INVALID_ID_VALUE )
	{
		if( *added == TRUE )
		{
			_tcsncat( condition, _T( " and " ), LOG_SQL_STMT_LEN - _tcslen( condition ) ); 
		}

		_tcsncat( condition, ID_COLUMN_NAME, LOG_SQL_STMT_LEN - _tcslen( condition ) ); 
		if( index > 1 )
		{
			_itot( index, id_desc, 10 ); 
			_tcsncat( condition, id_desc, LOG_SQL_STMT_LEN - _tcslen( condition ) ); 
		}

		_tcsncat( condition, EQUAL_SIGN, LOG_SQL_STMT_LEN - _tcslen( condition ) ); 
		_itot( id, id_desc, 10 ); 
		_tcsncat( condition, id_desc, LOG_SQL_STMT_LEN - _tcslen( condition ) ); 
		_added = TRUE; 
	}
	else if( class_name != NULL 
		&& *class_name != _T( '\0' ) )
	{
		if( *added == TRUE )
		{
			_tcsncat( condition, _T( " and " ), LOG_SQL_STMT_LEN - _tcslen( condition ) ); 
		}

		_tcsncat( condition, CLASS_COLUMN_NAME, LOG_SQL_STMT_LEN - _tcslen( condition ) ); 
		_tcsncat( condition, EQUAL_SIGN, LOG_SQL_STMT_LEN - _tcslen( condition ) ); 
		_tcsncat( condition, class_name, LOG_SQL_STMT_LEN - _tcslen( condition ) ); 
		_added = TRUE; 
	}

	if( *added == FALSE )
	{
		*added = _added; 
	}
	
	return; 
}

LRESULT del_rule_from_db( LPCTSTR class_name, 
			   ULONG id, 
			   LPCTSTR class_name2, 
			   ULONG id2, 
			   LPCTSTR class_name3, 
			   ULONG id3, 
			   LPCTSTR class_name4, 
			   ULONG id4, 
			   LPCTSTR class_name5, 
			   ULONG id5, 
			   action_response_type action, 
			   access_rule_type type, 
			   LPCTSTR desc, 
			   ULONG flag )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR *sql_stmt = NULL; 
	sqlite3 *db; 
	LPCTSTR table_name; 
	TCHAR id_desc[ 32 ]; 
	TCHAR *condition; 
	INT32 param_index = 1; 
	INT32 param_added = FALSE; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ASSERT( is_valid_access_rule_type( type ) == TRUE ); 

	log_trace( ( MSG_INFO, "rule type is %ws \n", get_rule_type_desc( type ) ) ); 

	ret = get_context_for_rule( type, &table_name, NULL, &db ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; ; 
		goto _return; 
	}

	condition = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( TCHAR ) ); 
	if( condition == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_tcsncpy( sql_stmt, DELETE_CMD, LOG_SQL_STMT_LEN ); 
	_tcsncat( sql_stmt, table_name, LOG_SQL_STMT_LEN - _tcslen( sql_stmt ) ); 

	*condition = _T( '\0' ); 

	add_rule_sel_param( condition, id, param_index ++, class_name, &param_added ); 
	add_rule_sel_param( condition, id2, param_index ++, class_name2, &param_added ); 
	add_rule_sel_param( condition, id3, param_index ++, class_name3, &param_added ); 
	add_rule_sel_param( condition, id4, param_index ++, class_name4, &param_added ); 
	add_rule_sel_param( condition, id5, param_index ++, class_name5, &param_added ); 
	
	if( *condition == L'\0' )
	{
		ASSERT( FALSE && "this sql statement will delete all row in the table" ); 
	}
	else
	{
		_tcsncat( sql_stmt, _T( " WHERE " ), LOG_SQL_STMT_LEN - _tcslen( sql_stmt )  ); 
		_tcsncat( sql_stmt, condition, LOG_SQL_STMT_LEN - _tcslen( sql_stmt ) ); 
	}

	ret = sql_exec( ( sqlite3* )db, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s delete rule failed: %d, extented error code %d reason %s\n" ), __TFUNCTION__, ret, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ret; 
}

INT32 add_reg_param_record( PVOID handle, LPCTSTR reg_path, LPCTSTR class_name, LPCTSTR desc, ULONG flag )
{
	INT32 ret = 0; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0; 

	ASSERT( handle != NULL ); 

	if( class_name == NULL )
	{
		class_name = _T( "" );  
	}

	if( desc == NULL )
	{
		desc = _T( "" ); 
	}

	if( ( add_time % CHECK_DB_OVERFLOW_TIME ) == 0 )
	{
		truncate_db( ( sqlite3* )handle, 
			DEFENSE_REG_PROTECT_TABLE, 
			MAX_DEFINE_NUM, 
			DEL_DEFINE_ONCE
			); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = SQLITE_NOMEM; 
		goto _return; 
	}

	ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_INSERT_REG_DEFINE, reg_path, class_name, desc, flag ); 

	if( ret <= 0 )
	{
		ret = SQLITE_ERROR; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )handle, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s insert new record failed: %d, reason %s\n" ), __TFUNCTION__, ret, sqlite3_errmsg16( ( sqlite3* )handle ) ); 
		if( ret == SQLITE_CONSTRAINT )
		{
			ret = SQLITE_OK; 
		}
	}
	else
	{
		add_time ++; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

INT32 add_file_param_record( PVOID handle, LPCTSTR file_path, LPCTSTR class_name, LPCTSTR desc, ULONG flag )
{
	INT32 ret = 0; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0; 

	ASSERT( handle != NULL ); 


	if( class_name == NULL )
	{
		class_name = _T( "" );  
	}

	if( desc == NULL )
	{
		desc = _T( "" ); 
	}

	if( ( add_time % CHECK_DB_OVERFLOW_TIME ) == 0 )
	{
		truncate_db( ( sqlite3* )handle, 
			DEFENSE_FILE_PROTECT_TABLE, 
			MAX_DEFINE_NUM, 
			DEL_DEFINE_ONCE
			); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = SQLITE_NOMEM; 
		goto _return; 
	}

	ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_INSERT_FILE_DEFINE, file_path, class_name, desc, flag ); 

	if( ret <= 0 )
	{
		ret = SQLITE_ERROR; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )handle, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s insert new record failed: %d, reason %s\n" ), __TFUNCTION__, ret, sqlite3_errmsg16( ( sqlite3* )handle ) );

		if( ret == SQLITE_CONSTRAINT )
		{
			ret = SQLITE_OK; 
			goto _return; 
		}

		ret = ERROR_DB_INSERT_ROW_ERROR; 
		goto _return; 
	}
	else
	{
		add_time ++; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

LRESULT add_common_param_record( PVOID handle, LPCTSTR common_name, LPCTSTR class_name, LPCTSTR desc, ULONG flag )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0; 

	ASSERT( handle != NULL ); 

	if( class_name == NULL )
	{
		class_name = _T( "" );  
	}

	if( desc == NULL )
	{
		desc = _T( "" ); 
	}

	if( common_name == NULL )
	{
		common_name = _T( "" ); 
	}

	if( ( add_time % CHECK_DB_OVERFLOW_TIME ) == 0 )
	{
		truncate_db( ( sqlite3* )handle, 
			COMMON_DEFINE_TABLE, 
			MAX_DEFINE_NUM, 
			DEL_DEFINE_ONCE
			); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = SQLITE_NOMEM; 
		goto _return; 
	}

	ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_INSERT_COMMON_DEFINE, common_name, class_name, desc, flag ); 

	if( ret <= 0 )
	{
		ret = SQLITE_ERROR; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )handle, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s insert new record failed: %d, reason %s\n" ), __TFUNCTION__, ret, sqlite3_errmsg16( ( sqlite3* )handle ) );

		if( ret == SQLITE_CONSTRAINT )
		{
			ret = SQLITE_OK; 
			goto _return; 
		}

		ret = ERROR_DB_INSERT_ROW_ERROR; 
		goto _return; 
	}
	else
	{
		add_time ++; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

INT32 add_com_param_record( PVOID handle, LPCTSTR com_name, LPCTSTR class_name, LPCTSTR desc, ULONG flag )
{
	INT32 ret = 0; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0; 

	ASSERT( handle != NULL ); 

	if( class_name == NULL )
	{
		class_name = _T( "" );  
	}

	if( desc == NULL )
	{
		desc = _T( "" ); 
	}

	if( ( add_time % CHECK_DB_OVERFLOW_TIME ) == 0 )
	{
		truncate_db( ( sqlite3* )handle, 
			DEFENSE_COM_PROTECT_TABLE, 
			MAX_DEFINE_NUM, 
			DEL_DEFINE_ONCE
			); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = SQLITE_NOMEM; 
		goto _return; 
	}

	ret = _sntprintf( sql_stmt, LOG_SQL_STMT_LEN, SQL_INSERT_COM_DEFINE, com_name, class_name, desc, flag ); 

	if( ret <= 0 )
	{
		ret = SQLITE_ERROR; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )handle, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "%s insert new record failed: %d, reason %s\n" ), __TFUNCTION__, ret, sqlite3_errmsg16( ( sqlite3* )handle ) );

		if( ret == SQLITE_CONSTRAINT )
		{
			ret = SQLITE_OK; 
			goto _return; 
		}

		ret = ERROR_DB_INSERT_ROW_ERROR; 
	}
	else
	{
		add_time ++; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

INT32 open_action_rule_db( LPCTSTR file_path, sqlite3 **handle )
{
	INT32 ret = 0; 
	sqlite3 *db = NULL; 

	ASSERT( file_path != NULL ); 

	*handle = NULL; 

	ret = create_unicode_db( file_path, &db ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	ret = safe_create_table( db, FW_RULE_TABLE, SQL_CREATE_FW_RULE_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the defense failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	ret = safe_create_table( db, DEFENSE_RULE_TABLE, SQL_CREATE_DEFENSE_RULE_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the firewall failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	*handle = db; 

_return:
	if( ret != SQLITE_OK )
	{
		if( db != NULL )
		{
			sqlite3_close( db ); 
		}
	}

	return ret; 
}

INT32 open_fw_log_db( LPCTSTR file_path, sqlite3 **handle )
{
	INT32 ret = 0; 
	sqlite3 *db = NULL; 

	ASSERT( file_path != NULL ); 

	*handle = NULL; 

	ret = create_unicode_db( file_path, &db ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	ret = safe_create_table( db, FW_LOG_TABLE, SQL_CREATE_FW_LOG_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the firewall failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	*handle = db; 

_return:
	if( ret != SQLITE_OK )
	{
		if( db != NULL )
		{
			sqlite3_close( db ); 
		}
	}

	return ret; 
}

INT32 open_defense_log_db( LPCTSTR file_path, sqlite3 **handle )
{
	INT32 ret = 0; 
	sqlite3 *db = NULL; 

	ASSERT( file_path != NULL ); 

	*handle = NULL; 

	ret = create_unicode_db( file_path, &db ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	ret = safe_create_table( db, DEFENSE_LOG_TABLE, SQL_CREATE_DEFNESE_LOG_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the defense failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	*handle = db; 

_return:
	if( ret != SQLITE_OK )
	{
		if( db != NULL )
		{
			sqlite3_close( db ); 
		}
	}

	return ret; 
}

INT32 open_param_define_db( LPCTSTR file_path, sqlite3 **handle )
{
	INT32 ret = 0; 
	sqlite3 *db = NULL; 

	ASSERT( file_path != NULL ); 

	*handle = NULL; 

	ret = create_unicode_db( file_path, &db ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	ret = safe_create_table( db, FW_IP_DEFINE_TABLE, SQL_CREATE_FW_IP_DEFINE_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the firewall failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	ret = safe_create_table( db, FW_PORT_DEFINE_TABLE, SQL_CREATE_FW_PORT_DEFINE_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the defense failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	ret = safe_create_table( db, FW_URL_DEFINE_TABLE, SQL_CREATE_FW_URL_DEFINE_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the defense failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	ret = safe_create_table( db, DEFENSE_REG_PROTECT_TABLE, SQL_CREATE_PROTECT_REG_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the firewall failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	ret = safe_create_table( db, DEFENSE_FILE_PROTECT_TABLE, SQL_CREATE_PROTECT_FILE_TABLE ); 
	if( ret != SQLITE_OK )
	{
		//__Trace( _T( "create log table of the firewall failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16() );
		goto _return; 
	}

	ret = safe_create_table( db, DEFENSE_COM_PROTECT_TABLE, SQL_CREATE_PROTECT_COM_TABLE ); 
	if( ret != SQLITE_OK )
	{
		//__Trace( _T( "create log table of the firewall failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16() );
		goto _return; 
	}

	ret = safe_create_table( db, APP_DEFINE_TABLE, SQL_CREATE_APP_DEFINE_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the defense failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	ret = safe_create_table( db, COMMON_DEFINE_TABLE, SQL_CREATE_COMMON_DEFINE_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the defense failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	*handle = db; 

_return:
	if( ret != SQLITE_OK )
	{
		if( db != NULL )
		{
			sqlite3_close( db ); 
		}
	}

	return ret; 
}

#include "bitsafe_conf.h"
LRESULT open_bitsafe_conf()
{
	LRESULT ret; 
	CStdString conf_file_path; 

	ASSERT( sqlite3_threadsafe() != FALSE ); 

	conf_file_path = CPaintManagerUI::GetInstancePath(); 
	ASSERT( conf_file_path != _T( "" ) ); 

	conf_file_path += ACTION_RULE_DB_NAME; 

	ret = open_action_rule_db( conf_file_path, &action_rule_db ); 
	if( ret != SQLITE_OK )
	{
		if( ret == SQLITE_NOTADB )
		{
			ret = delete_file_spec_dir( conf_file_path, FILE_IS_WHOLE_PATH ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "delete corrupt db file error\n" ) ); 
				ASSERT( FALSE && "delete corrupt db file error\n" ); 
				goto _return; 
			}

			ret = safe_create_file_from_res( ACTION_RULE_DB_NAME ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "create access rule db failed \n" ) );
				goto _return; 
			}

			ret = open_action_rule_db( conf_file_path, &action_rule_db ); 
			if( ret != SQLITE_OK )
			{
				ret = ERROR_DB_CREATE_FAILED; 
				goto _return; 
			}
		}
		else
		{
			log_trace( ( MSG_ERROR, "open db file %ws error %u\n", conf_file_path.GetData(), ret ) ); 
			ASSERT( FALSE && "open db file error\n" ); 
			ret = ERROR_DB_CREATE_FAILED; 
			goto _return; 
		}
	}

	conf_file_path = CPaintManagerUI::GetInstancePath(); 
	ASSERT( conf_file_path != _T( "" ) ); 

	conf_file_path += FW_LOG_DB_NAME; 

	ret = open_fw_log_db( conf_file_path, &fw_log_db ); 
	if( ret != SQLITE_OK )
	{
		if( ret == SQLITE_NOTADB )
		{
			ret = delete_file_spec_dir( conf_file_path, FILE_IS_WHOLE_PATH ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "delete corrupt db file error\n" ) ); 
				ASSERT( FALSE && "delete corrupt db file error\n" ); 
				goto _return; 
			}

			ret = safe_create_file_from_res( FW_LOG_DB_NAME ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "create access rule db failed \n" ) );
				goto _return; 
			}

			ret = open_fw_log_db( conf_file_path, &fw_log_db ); 
			if( ret != SQLITE_OK )
			{
				ret = ERROR_DB_CREATE_FAILED; 
				goto _return; 
			}
		}
		else
		{
			log_trace( ( MSG_ERROR, "open db file %ws error %u\n", conf_file_path.GetData(), ret ) ); 
			ASSERT( FALSE && "open db file error\n" ); 
			ret = ERROR_DB_CREATE_FAILED; 
			goto _return; 
		}
	}

	conf_file_path = CPaintManagerUI::GetInstancePath(); 
	ASSERT( conf_file_path != _T( "" ) ); 

	conf_file_path += DEFENSE_LOG_DB_NAME; 

	ret = open_defense_log_db( conf_file_path, &defense_log_db ); 
	if( ret != SQLITE_OK )
	{
		if( ret == SQLITE_NOTADB )
		{
			ret = delete_file_spec_dir( conf_file_path, FILE_IS_WHOLE_PATH ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "delete corrupt db file error\n" ) ); 
				ASSERT( FALSE && "delete corrupt db file error\n" ); 
				goto _return; 
			}

			ret = safe_create_file_from_res( DEFENSE_LOG_DB_NAME ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "create access rule db failed \n" ) );
				goto _return; 
			}

			ret = open_defense_log_db( conf_file_path, &defense_log_db ); 
			if( ret != SQLITE_OK )
			{
				ret = ERROR_DB_CREATE_FAILED; 
				goto _return; 
			}
		}
		else
		{
			log_trace( ( MSG_ERROR, "open db file %ws error %u\n", conf_file_path.GetData(), ret ) ); 
			ASSERT( FALSE && "open db file error\n" ); 
			ret = ERROR_DB_CREATE_FAILED; 
			goto _return; 
		}
	}

	conf_file_path = CPaintManagerUI::GetInstancePath(); 
	ASSERT( conf_file_path != _T( "" ) ); 

	conf_file_path += PARAM_DEFINE_DB_NAME; 

	ret = open_param_define_db( conf_file_path, &param_define_db ); 
	if( ret != SQLITE_OK )
	{
		if( ret == SQLITE_NOTADB )
		{
			ret = delete_file_spec_dir( conf_file_path, FILE_IS_WHOLE_PATH ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "delete corrupt db file error\n" ) ); 
				ASSERT( FALSE && "delete corrupt db file error\n" ); 
				goto _return; 
			}

			ret = safe_create_file_from_res( PARAM_DEFINE_DB_NAME ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "create access rule db failed \n" ) );
				goto _return; 
			}

			ret = open_param_define_db( conf_file_path, &param_define_db ); 
			if( ret != SQLITE_OK )
			{
				ret = ERROR_DB_CREATE_FAILED; 
				goto _return; 
			}
		}
		else
		{
			log_trace( ( MSG_ERROR, "open db file %ws error %u\n", conf_file_path.GetData(), ret ) ); 
			ASSERT( FALSE && "open db file error\n" ); 
			ret = ERROR_DB_CREATE_FAILED; 
			goto _return; 
		}
	}

	conf_file_path = CPaintManagerUI::GetInstancePath(); 
	ASSERT( conf_file_path != _T( "" ) ); 

	conf_file_path += ACTION_LOG_DB_NAME; 

	ret = open_action_log_db( conf_file_path, ( PVOID* )&common_log_db ); 
	if( ret != SQLITE_OK )
	{
		if( ret == SQLITE_NOTADB )
		{
			ret = delete_file_spec_dir( conf_file_path, FILE_IS_WHOLE_PATH ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "delete corrupt db file error\n" ) ); 
				ASSERT( FALSE && "delete corrupt db file error\n" ); 
				goto _return; 
			}

			ret = restore_action_log_db_backup( ACTION_LOG_DB_NAME ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "create access rule db failed \n" ) );
				goto _return; 
			}

			ret = open_action_log_db( conf_file_path, ( PVOID* )&common_log_db ); 
			if( ret != SQLITE_OK )
			{
				ret = ERROR_DB_CREATE_FAILED; 
				goto _return; 
			}
		}
		else
		{
			log_trace( ( MSG_ERROR, "open db file %ws error %u\n", conf_file_path.GetData(), ret ) ); 
			ASSERT( FALSE && "open db file error\n" ); 
			ret = ERROR_DB_CREATE_FAILED; 
			goto _return; 
		}
	}

	conf_file_path = CPaintManagerUI::GetInstancePath(); 
	ASSERT( conf_file_path != _T( "" ) ); 

	conf_file_path += ACTION_FILTER_COND_DB_NAME; 

	ret = open_action_filter_cond_db( conf_file_path, ( PVOID* )&cond_db ); 
	if( ret != SQLITE_OK )
	{
		if( ret == SQLITE_NOTADB )
		{
			ret = delete_file_spec_dir( conf_file_path, FILE_IS_WHOLE_PATH ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "delete corrupt db file error\n" ) ); 
				ASSERT( FALSE && "delete corrupt db file error\n" ); 
				goto _return; 
			}

			//ret = restore_action_filter_db_backup( ACTION_FILTER_COND_DB_NAME ); 
			//if( ret != ERROR_SUCCESS )
			//{
			//	log_trace( ( MSG_ERROR, "create access rule db failed \n" ) );
			//	goto _return; 
			//}

			ret = open_action_filter_cond_db( conf_file_path, ( PVOID* )&cond_db ); 
			if( ret != SQLITE_OK )
			{
				ret = ERROR_DB_CREATE_FAILED; 
				goto _return; 
			}
		}
		else
		{
			log_trace( ( MSG_ERROR, "open db file %ws error %u\n", conf_file_path.GetData(), ret ) ); 
			ASSERT( FALSE && "open db file error\n" ); 
			ret = ERROR_DB_CREATE_FAILED; 
			goto _return; 
		}
	}

_return:
	if( ret != SQLITE_OK )
	{
		DBG_BP(); 

		if( param_define_db != NULL )
		{
			close_db( param_define_db ); 
			param_define_db = NULL; 
		}

		if( fw_log_db != NULL )
		{
			close_db( fw_log_db ); 
			fw_log_db = NULL; 
		}

		if( action_rule_db != NULL )
		{
			close_db( action_rule_db ); 
			action_rule_db = NULL; 
		}

		if( defense_log_db != NULL )
		{
			close_db( defense_log_db ); 
			defense_log_db = NULL; 
		}

		if( common_log_db != NULL )
		{
			close_db( common_log_db ); 
			common_log_db = NULL; 
		}

		if( cond_db != NULL )
		{
			close_db( cond_db ); 
		}
	}

	return ret; 
}

LRESULT open_conf_db( LPCTSTR file_path, sqlite3 **handle )
{
	INT32 ret = 0; 
	sqlite3 *db = NULL; 

	ASSERT( file_path != NULL ); 
	ASSERT( handle != NULL ); 

	*handle = NULL; 

	ret = create_unicode_db( file_path, &db ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	ret = safe_create_table( db, DRIVER_LOAD_CONF_TABLE, SQL_CREATE_DRIVER_LOAD_CONF_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the firewall failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	ret = safe_create_table( db, GENERAL_CONF_TABLE, SQL_CREATE_GENERAL_CONF_TABLE ); 
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "create log table of the firewall failed error code: %d, reason:%s\n" ), ret, sqlite3_errmsg16(db) );
		goto _return; 
	}

	*handle = db; 

_return:
	if( ret != SQLITE_OK )
	{
		if( db != NULL )
		{
			sqlite3_close( db ); 
		}
	}

	return ret; 
}

LRESULT test_sqlite3_ext_stack_overflow()
{
	LRESULT ret = 0; 
	sqlite3* test_db = NULL; 

	INT32 _ret; 
	CStdString db_file_path; 

	db_file_path = L"C:\\"; 

	//db_file_path = CPaintManagerUI::GetInstancePath(); 
	//ASSERT( db_file_path != _T( "" ) ); 

	db_file_path += SEVENT_CONF_DB_NAME; 

	_ret = open_conf_db( db_file_path, &test_db ); 
	if( _ret != SQLITE_OK )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 

		if( _ret == SQLITE_NOTADB )
		{
			log_trace( ( MSG_ERROR, "open db file %ws error: not a db\n", db_file_path.GetData() ) ); 
			ASSERT( FALSE && "open db file error\n" ); 
			ret = ERROR_DB_CREATE_FAILED; 
			goto _return; 
		}
		else
		{
			log_trace( ( MSG_ERROR, "open db file %ws error %u\n", db_file_path.GetData(), _ret ) ); 
			ASSERT( FALSE && "open db file error\n" ); 
			ret = ERROR_DB_CREATE_FAILED; 
			goto _return; 
		}
	}


_return:
	if( ret != ERROR_SUCCESS )
	{
		if( test_db != NULL )
		{
			close_db( test_db ); 
			test_db = NULL; 
		}
	}

	return ret; 
}

LRESULT open_function_conf()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	CStdString conf_file_path; 

	conf_file_path = CPaintManagerUI::GetInstancePath(); 
	ASSERT( conf_file_path != _T( "" ) ); 

	conf_file_path += SEVENT_CONF_DB_NAME; 

	_ret = open_conf_db( conf_file_path, &conf_db ); 
	if( _ret != SQLITE_OK )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 

		if( _ret == SQLITE_NOTADB )
		{
			ret = delete_file_spec_dir( conf_file_path, FILE_IS_WHOLE_PATH ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "delete corrupt db file error\n" ) ); 
				ASSERT( FALSE && "delete corrupt db file error\n" ); 
				goto _return; 
			}

			ret = safe_create_file_from_res( SEVENT_CONF_DB_NAME ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "create access rule db failed \n" ) );
				goto _return; 
			}

			_ret = open_conf_db( conf_file_path, &conf_db ); 
			if( _ret != SQLITE_OK )
			{
				ret = ERROR_DB_CREATE_FAILED; 
				goto _return; 
			}
		}
		else
		{
			log_trace( ( MSG_ERROR, "open db file %ws error %u\n", conf_file_path.GetData(), _ret ) ); 
			ASSERT( FALSE && "open db file error\n" ); 
			ret = ERROR_DB_CREATE_FAILED; 
			goto _return; 
		}
	}


_return:
	if( ret != ERROR_SUCCESS )
	{
		if( conf_db != NULL )
		{
			close_db( conf_db ); 
			conf_db = NULL; 
		}
	}

	return ret; 
}

LRESULT get_fw_log( fw_log_callback output_func, alloc_record_callback alloc_func, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	//static INT32 log_load_time = 0; 

	ASSERT( output_func != NULL ); 

	ret = _get_fw_log( fw_log_db, output_func, alloc_func, param ); 

	//log_load_time ++; 

	return ret; 
}

LRESULT close_function_conf()
{
	INT32 _ret; 
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( conf_db != NULL ); 

	_ret = close_db( conf_db ); 
	if( _ret != SQLITE_OK )
	{
		ret = ERROR_DB_CLOSE_ERROR; 
		__Trace( _T( "insert new firewall trusted app failed: %d, reason %s\n" ), ret, sqlite3_errmsg( ( sqlite3* )action_rule_db ) );
	}

	return ret; 
}

LRESULT close_bitsafe_conf()
{
	INT32 _ret; 
	LRESULT ret = ERROR_SUCCESS; 
	_ret = close_db( action_rule_db ); 
	if( _ret != SQLITE_OK )
	{
		ret = ERROR_DB_CLOSE_ERROR; 
		__Trace( _T( "insert new firewall trusted app failed: %d, reason %s\n" ), ret, sqlite3_errmsg( ( sqlite3* )action_rule_db ) );
	}

	_ret = close_db( fw_log_db ); 
	if( _ret != SQLITE_OK )
	{
		ret = ERROR_DB_CLOSE_ERROR; 
		__Trace( _T( "insert new firewall trusted app failed: %d, reason %s\n" ), ret, sqlite3_errmsg( ( sqlite3* )fw_log_db ) );
	}

	_ret = close_db( param_define_db ); 
	if( _ret != SQLITE_OK )
	{
		ret = ERROR_DB_CLOSE_ERROR; 
		__Trace( _T( "insert new firewall trusted app failed: %d, reason %s\n" ), ret, sqlite3_errmsg( ( sqlite3* )param_define_db ) );
	}

	_ret = close_db( defense_log_db ); 
	if( _ret != SQLITE_OK )
	{
		ret = ERROR_DB_CLOSE_ERROR; 
		__Trace( _T( "insert new firewall trusted app failed: %d, reason %s\n" ), ret, sqlite3_errmsg( ( sqlite3* )defense_log_db ) );
	}

	ASSERT( common_log_db != NULL ); 

	_ret = close_db( common_log_db ); 
	if( _ret != SQLITE_OK )
	{
		ret = ERROR_DB_CLOSE_ERROR; 
		__Trace( _T( "close common action log db error: %d, reason %s\n" ), ret, sqlite3_errmsg( ( sqlite3* )common_log_db ) );
	}

	//ASSERT( common_log_db != NULL ); 

	//_ret = close_db( common_log_db ); 
	//if( _ret != SQLITE_OK )
	//{
	//	ret = ERROR_DB_CLOSE_ERROR; 
	//	__Trace( _T( "close common action log db error: %d, reason %s\n" ), ret, sqlite3_errmsg( ( sqlite3* )common_log_db ) );
	//}

	ASSERT( cond_db != NULL ); 

	_ret = close_db( cond_db ); 
	if( _ret != SQLITE_OK )
	{
		ret = ERROR_DB_CLOSE_ERROR; 
		__Trace( _T( "close action filter condition db error: %d, reason %s\n" ), ret, sqlite3_errmsg( ( sqlite3* )cond_db ) ); 
	}

	return ret; 
}

LRESULT truncate_log_db( ULONG flags )
{
	LRESULT _ret = ERROR_SUCCESS; 
	sqlite3 *db; 
	ULONG count; 
	data_process data_proc = { 0 }; 
	TCHAR sql_stmt[ SQL_STMT_LEN ]; 

#define DEL_LOG_SQL_STMT _T( "delete from %s where rowid in ( select rowid from %s order by time limit %d )" ) 

	ASSERT( fw_log_db != NULL ); 
	ASSERT( defense_log_db != NULL ); 

	data_proc.data_proc_func = check_result_count; 
	data_proc.param = &count; 

	if( flags == FW_LOG_OP )
	{
		_sntprintf( sql_stmt, 
			SQL_STMT_LEN, 
			_T( "SELECT COUNT(*) FROM %s" ), 
			FW_LOG_TABLE ); 
		db = fw_log_db; 
	}
	else
	{
		_sntprintf( sql_stmt, 
			SQL_STMT_LEN, 
			_T( "SELECT COUNT(*) FROM %s" ), 
			DEFENSE_LOG_TABLE ); 
		db = defense_log_db; 
	}

	_ret = sql_exec( db, sql_stmt, &data_proc ); 
	if( _ret != SQLITE_OK )
	{
		log_trace( ( MSG_ERROR, "get log count failed\n" ) ); 
		goto _return; 
	}

	if( count > MAX_LOG_ROW_COUNT )
	{
		if( flags == FW_LOG_OP )
		{
			_ret = _sntprintf( sql_stmt, 
				SQL_STMT_LEN, 
				DEL_LOG_SQL_STMT, 
				FW_LOG_TABLE, 
				FW_LOG_TABLE, 
				DEL_LOG_NUM_ONCE ); 
		}
		else
		{
			_ret = _sntprintf( sql_stmt, 
				SQL_STMT_LEN, 
				DEL_LOG_SQL_STMT, 
				DEFENSE_LOG_TABLE, 
				DEFENSE_LOG_TABLE, 
				DEL_LOG_NUM_ONCE ); 
		}

		_ret = sql_exec( db, sql_stmt, &data_proc ); 
		if( _ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "delete log failed\n" ) ); 
		}
	}

_return:
	return _ret; 
}

LRESULT truncate_db( PVOID target_db, LPCTSTR table_name, ULONG max_num, ULONG del_once )
{
	LRESULT _ret = ERROR_SUCCESS; 
	sqlite3 *db; 
	ULONG count; 
	data_process data_proc = { 0 }; 
	TCHAR sql_stmt[ SQL_STMT_LEN ]; 

#define DEL_DB_SQL_STMT _T( "delete from %s where rowid in ( select rowid from %s order by rowid limit %d )" ) 

	ASSERT( fw_log_db != NULL ); 
	ASSERT( defense_log_db != NULL ); 

	data_proc.data_proc_func = check_result_count; 
	data_proc.param = &count; 

	_sntprintf( sql_stmt, 
		SQL_STMT_LEN, 
		_T( "SELECT COUNT(*) FROM %s" ), 
		table_name ); 
	db = ( sqlite3* )target_db; 

	_ret = sql_exec( db, sql_stmt, &data_proc ); 
	if( _ret != SQLITE_OK )
	{
		log_trace( ( MSG_ERROR, "get log count failed\n" ) ); 
		goto _return; 
	}

	if( count > max_num )
	{
		_ret = _sntprintf( sql_stmt, 
			SQL_STMT_LEN, 
			DEL_DB_SQL_STMT, 
			table_name, 
			table_name, 
			del_once ); 

		_ret = sql_exec( db, sql_stmt, &data_proc ); 
		if( _ret != SQLITE_OK )
		{
			log_trace( ( MSG_ERROR, "delete log failed\n" ) ); 
		}
	}

_return:
	return _ret; 
}

LRESULT add_fw_log( pfw_log log ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR *sql_stmt = NULL; 
	static ULONG add_time = 0;
	//ASSERT( handle != NULL ); 


	if( ( add_time % CHECK_LOG_DB_OVERFLOW_TIME )== 0 )
	{
		truncate_log_db( FW_LOG_OP ); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( WCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_sntprintf( sql_stmt, LOG_SQL_STMT_LEN, 
		SQL_INSERT_FW_LOG, 
		log->proc_name, 
		log->action, 
		log->type, 
		log->prot, 
		log->src_ip, 
		log->src_port, 
		log->dest_ip, 
		log->dest_port, 
		log->desc, 
		log->time ); 

	ret = sql_exec( ( sqlite3* )fw_log_db, sql_stmt, NULL );
	if( ret != SQLITE_OK )
	{
		ret = ERROR_DB_INSERT_ROW_ERROR; 
		__Trace( _T( "insert new firewall trusted app failed: %d, reason %s\n" ), ret, sqlite3_errmsg16(( sqlite3* )fw_log_db ) );

		goto _return; 
	}
	else
	{
		add_time ++; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

LRESULT add_defense_log( pdefense_log log )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR *sql_stmt = NULL; 
	INT32 exist; 
	static ULONG add_time = 0; 

#ifdef _DEBUG
	ret = check_table_exist( ( sqlite3* )defense_log_db, DEFENSE_LOG_TABLE, &exist ); 
	ASSERT( exist > 0 ); 
#endif //_DEBUG

	if( ( add_time % CHECK_LOG_DB_OVERFLOW_TIME )== 0 )
	{
		truncate_log_db( DEFENSE_LOG_OP ); 
	}

	sql_stmt = ( TCHAR* )malloc( LOG_SQL_STMT_LEN * sizeof( TCHAR ) ); 
	if( sql_stmt == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_sntprintf( sql_stmt, SQL_STMT_LEN, SQL_INSERT_DEFENSE_LOG, 
		log->proc_name, 
		log->flag, 
		log->action, 
		log->target_desc, 
		log->time ); 

	ret = sql_exec( ( sqlite3* )defense_log_db, sql_stmt, NULL );
	if( ret != SQLITE_OK )
	{
		__Trace( _T( "insert new firewall trusted app failed: %d, reason %s\n" ), ret, sqlite3_errmsg16(( sqlite3* )defense_log_db ) );
		ret = ERROR_DB_INSERT_ROW_ERROR; 
		goto _return; 
	}
	else
	{
		add_time ++; 
	}

_return:
	if( sql_stmt != NULL )
	{
		free( sql_stmt ); 
	}

	return ret; 
}

//#ifdef _DEBUG

INT32 CALLBACK check_fw_trust( LPCTSTR app_path, PVOID param )
{
	INT32 ret = 0; 

	OutputDebugString( app_path ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

INT32 CALLBACK check_fw_log( pfw_log log, PVOID param )
{
	INT32 ret = 0; 

	OutputDebugString( log->proc_name ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

INT32 CALLBACK check_defense_log( pdefense_log log, PVOID param )
{
	INT32 ret = 0; 

	OutputDebugString( log->proc_name ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

INT32 CALLBACK check_file_define( param_define_type type, param_record *rule, PVOID param  )
{
	INT32 ret = 0; 
	file_param_record *_rule; 

	_rule = ( file_param_record* )rule; 
	OutputDebugString( _rule->file_path ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

INT32 CALLBACK check_url_define( param_define_type type, param_record *rule, PVOID param  )
{
	INT32 ret = 0; 
	url_param_record *_rule; 

	_rule = ( url_param_record* )rule; 
	OutputDebugString( _rule->url ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

INT32 CALLBACK check_com_define( param_define_type type, param_record *rule, PVOID param  )
{
	INT32 ret = 0; 
	com_param_record *_rule; 

	_rule = ( com_param_record* )rule; 
	OutputDebugString( _rule->com_name ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

INT32 CALLBACK check_app_define( param_define_type type, param_record *rule, PVOID param  )
{
	INT32 ret = 0; 
	app_param_record *_rule; 

	_rule = ( app_param_record* )rule; 

	OutputDebugString( _rule->app_name ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

INT32 CALLBACK check_reg_define( param_define_type type, param_record *rule, PVOID param  )
{
	INT32 ret = 0; 
	reg_param_record *_rule; 

	_rule = ( reg_param_record* )rule; 
	OutputDebugString( _rule->reg_path ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

INT32 CALLBACK check_ip_define( param_define_type type, param_record *rule, PVOID param  )
{
	INT32 ret = 0; 
	CStdString dbg_out; 
	ip_param_record *_rule; 

	_rule = ( ip_param_record* )rule; 

	dbg_out.Format( _T( "%d" ), _rule->ip_begin ); 
	OutputDebugString( dbg_out.GetData() ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

INT32 CALLBACK check_port_define( param_define_type type, param_record *rule, PVOID param  )
{
	INT32 ret = 0; 
	CStdString dbg_out; 
	port_param_record *_rule; 

	_rule = ( port_param_record* )rule; 

	dbg_out.Format( _T( "%d" ), _rule->port_begin ); 
	OutputDebugString( dbg_out.GetData() ); 
	OutputDebugString( _T( "\n" ) ); 

	return ret; 
}

LRESULT load_rule( access_rule_desc *rule_load )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	ULONG param_count; 

	ASSERT( rule_load != NULL ); 

	param_count = get_access_rule_param_count( rule_load->type ); 

	for( i = 0; i < param_count; i ++ )
	{
		ret = load_param_define( rule_load->desc.params[ i ] ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}
	
_return:
	return ret; 
}

INLINE LRESULT load_url_rule( access_rule_desc *rule_desc )
{
	return load_rule( rule_desc ); 
}

INLINE LRESULT load_socket_rule( access_rule_desc *rule_desc )
{
	return load_rule( rule_desc ); 
}

INLINE LRESULT load_file_rule( access_rule_desc *rule_desc )
{
	return load_rule( rule_desc ); 
}

INLINE LRESULT load_reg_rule( access_rule_desc *rule_desc )
{
	return load_rule( rule_desc ); 
}

INLINE LRESULT load_com_rule( access_rule_desc *rule_desc )
{
	return load_rule( rule_desc ); 
}

INLINE LRESULT load_common_rule( access_rule_desc *rule_desc )
{
	return load_rule( rule_desc ); 
}

LRESULT load_rule_defines( access_rule_desc *rule_desc )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT(rule_desc != NULL ); 
	
	switch( rule_desc->type )
	{
	case URL_RULE_TYPE:
		ret = load_url_rule( rule_desc ); 
		break; 
	case SOCKET_RULE_TYPE:
		ret = load_socket_rule( rule_desc ); 
		break; 
	case FILE_RULE_TYPE: 
		ret = load_file_rule( rule_desc ); 
		break; 
	case REG_RULE_TYPE:  
		ret = load_reg_rule( rule_desc ); 
		break; 
	case COM_RULE_TYPE: 
		ret = load_com_rule( rule_desc ); 
		break; 
	case COMMON_RULE_TYPE:
		ret = load_common_rule( rule_desc ); 
		break; 
	default:
		ASSERT( FALSE );
		break; 
	}


	return ret; 
}

LRESULT CALLBACK load_action_rule( PVOID rule, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	sys_action_desc *rule_desc; 

	ASSERT( rule != NULL ); 
	rule_desc = ( sys_action_desc* )rule; 

	ASSERT( is_valid_access_rule_type( rule_desc->type ) ); 

//_return:
	return ret; 
}

LRESULT load_rule_from_record( access_rule_type type, 
							  action_rule_record *record, 
							  access_rule_desc *rule_input )
{
	INT32 i; 
	LRESULT ret = ERROR_SUCCESS; ; 
	action_rule_record *_rule; ; 
	//action_rule_define rule_define; 
	ULONG param_count; 

	ASSERT( rule_input != NULL ); 

	_rule = ( action_rule_record* )record; 

	//wcsncpy( rule_input->addition, record->desc, _MAX_DESC_LEN );
	ret = init_access_rule( type, rule_input ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	rule_input->resp = ( action_response_type )record->action; 

	rule_input->type = ( access_rule_type )record->type; 

	param_count = get_access_rule_param_count( rule_input->type ); 

	for( i = 0; ( ULONG )i < param_count; i ++ )
	{
		if( *record->class_names[ i ] == L'\0' )
		{
			rule_input->desc.params[ i ]->is_cls = FALSE; 
			if( record->ids[ i ] == INVALID_ID_VALUE )
			{
				ret = ERROR_DB_DATA_CURRUPT; 
				ASSERT( FALSE ); 
				goto _return; 
			}
		}
		else
		{
			if( wcslen( record->class_names[ i ] ) >= _MAX_CLASS_NAME_LEN )
			{
				ret = ERROR_DB_DATA_CURRUPT; 
				ASSERT( FALSE ); 
				goto _return; 
			}

			wcscpy( rule_input->desc.params[ i ]->cls.class_name, record->class_names[ i ] ); 

			if( record->ids[ i ] != INVALID_ID_VALUE )
			{

				ret = ERROR_DB_DATA_CURRUPT; 
				ASSERT( FALSE ); 
				goto _return; 
			}
		}

		rule_input->desc.params[ i ]->id = record->ids[ i ]; 

	}

	if( rule_input->type == COMMON_RULE_TYPE )
	{
		ASSERT( is_valid_action_type( record->flag ) == TRUE ); 
		rule_input->desc.common.action_type = ( sys_action_type )record->flag; 
	}

	set_param_type_in_rule( rule_input ); 

	ret = load_rule( rule_input ); 
	if( ret < 0 )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

_return:
	return ret; 
}

//#define DEBUG_DRIVER 

LRESULT test_trace_log(); 

sys_action_type get_action_type_from_rule_type( access_rule_type type )
{
	sys_action_type action_type = INVALID_SYS_ACTION_TYPE; 
	ASSERT( is_valid_access_rule_type( type ) ); 

	switch( type )
	{
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	//case INSTALL_DRV: 
	//case INSTALL_HOOK:
	//case CREATE_PROC:
	//case ACCESS_MEM: 
	case COMMON_RULE_TYPE: 
		action_type = INSTALL_DRV; 
		break; 
	//case ACCESS_COM:
	case COM_RULE_TYPE:
		action_type = ACCESS_COM; 
		break; 
	//case MODIFY_KEY:
	case REG_RULE_TYPE:
		action_type = MODIFY_KEY; 
		break; 
	//case MODIFY_FILE:
	case FILE_RULE_TYPE:
		action_type = MODIFY_FILE; 
		break; 
	//case SOCKET_CONNECT:
	//case SOCKET_SEND:
		//case SOCKET_RECV:
	case SOCKET_RULE_TYPE: 
		action_type = SOCKET_CONNECT; 
		break; 
	//case LOCATE_URL:
	case URL_RULE_TYPE: 
		action_type = LOCATE_URL; 
		break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE
	default:
		ASSERT( "invalid sys action type" && FALSE ); 
		break; 
	}

	return action_type; 
}

LRESULT generate_test_action( sys_action_desc *cur_action, ULONG param, ULONG mode )
{
	LRESULT ret = ERROR_SUCCESS; 
	CStdString test_app_name; 
	CStdString test_reg_path; 

	test_app_name.Format( _T( "test_app_%d" ), ( param % 2 ) ); 

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	if( mode == TEST_REG_RULE )
	{
		ret = init_sys_action( MODIFY_KEY, cur_action ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		if( ( param % 3 ) == 0 )
		{
			test_reg_path.Format( _T( "HKEY_LOCAL_MACHINE\\SYSTEM\\test%d\\test_value" ), param ); 
		}
		else if( ( param % 4 ) == 0 )
		{
			//param_str.Format( _T( "sys action generated %d" ), param ); 
			//cur_action->type = ( sys_action_type )( param % MAX_SYS_ACTION_TYPE ); 

			//if( cur_action->type == SOCKET_CONNECT 
			//	|| cur_action->type == SOCKET_SEND
			//	|| cur_action->type == SOCKET_RECV )
			//{
			//	cur_action->type == OTHER_ACTION; 
			//}
			test_reg_path.Format( _T( "HKEY_LOCAL_MACHINE\\SYSTEM\\test%d\\" ), param ); 

		}
		else if( ( param % 5 ) == 0 )
		{
			test_reg_path.Format( _T( "HKEY_LOCAL_MACHINE\\SYSTE" ) ); 
		}
		else if( ( param & 7 ) == 0 )
		{
			test_reg_path.Format( _T( "HKEY_LOCAL_MACHINE\\") ); 
		}
		else if( ( param & 11 ) == 0 )
		{
			test_reg_path.Format( _T( "HEKY_LOCAL_MACHINE\\DATA" ) ); 
		}
		else
		{
			test_reg_path.Format( _T( "HKEY_LOCAL_MAchine\\SYSTEM\\test%d" ), param ); 
		}

		//*cur_action->addition = L'\0'; 
		cur_action->id = INVALID_ID_VALUE; 

		cur_action->desc.params[ 0 ]->id = INVALID_ID_VALUE; 
		cur_action->desc.params[ 0 ]->is_cls = FALSE; 

		wcscpy( cur_action->desc.params[ 0 ]->app.app_name, test_app_name.GetData() ); 

		cur_action->desc.params[ 1 ]->id = INVALID_ID_VALUE; 
		cur_action->desc.params[ 1 ]->is_cls = FALSE; 

		wcscpy( cur_action->desc.params[ 1 ]->app.app_name, test_reg_path.GetData() ); 

		cur_action->type = get_action_type_from_rule_type( REG_RULE_TYPE ); 
	}
	else if( mode == TEST_SOCKET_RULE )
	{
		init_sys_action( SOCKET_SEND, cur_action ); 
		cur_action->id = INVALID_ID_VALUE; 

		wcscpy( cur_action->desc.socket.app.app.app_name, test_app_name.GetData() ); 
		cur_action->desc.socket.app.is_cls = FALSE; 
		cur_action->desc.socket.app.id = INVALID_ID_VALUE; 

		cur_action->desc.socket.dest_ip.type = IP_DEFINE; 
		cur_action->desc.socket.src_ip.type = IP_DEFINE; 
		cur_action->desc.socket.dest_port.type = PORT_DEFINE; 
		cur_action->desc.socket.src_port.type = PORT_DEFINE; 
		cur_action->desc.socket.src_port.port.type = ALL_PROT; 
		cur_action->desc.socket.dest_port.port.type = ALL_PROT; 

		ret = str_2_ipv4_addr( _T( "192.168.10.102" ), &cur_action->desc.socket.dest_ip.ip.ip_begin ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		ret = str_2_ipv4_addr( _T( "0.0.0.0" ), &cur_action->desc.socket.dest_ip.ip.ip_end ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}
	}
	else if( mode == TEST_COMMON_DRIVER_RULE )
	{
		ret = init_sys_action( INSTALL_DRV, cur_action ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		cur_action->id = INVALID_ID_VALUE; 

		cur_action->desc.common.action_type = INSTALL_DRV; 
		//wcscpy( cur_action->desc.socket.app.app.app_name, test_app_name.GetData() ); 
		cur_action->desc.common.app.is_cls = FALSE; 
		cur_action->desc.common.app.id = INVALID_ID_VALUE; 
		*cur_action->desc.common.app.app.app_name = L'\0'; 

		cur_action->desc.common.param0.is_cls = FALSE; 
		cur_action->desc.common.param0.id = INVALID_ID_VALUE; 
		wcscpy( cur_action->desc.common.param0.common.name, 
			L"C:\\Windows\\system32\\drivers\\sevenfw.sys" ); 

	}
#endif //COMPATIBLE_OLD_ACTION_DEFINE

	set_define_type_in_action( cur_action ); 

_return:
	return ret; 
}

LRESULT init_name_access_rule_ptr_desc( access_rule_ptr_desc *rule_input, action_response_type resp, access_rule_type type, LPCWSTR app_name, LPCWSTR name_param )
{
	LRESULT ret = ERROR_SUCCESS; 
	//access_rule_ptr_desc rule_input; 
	INT32 i; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	rule_input->type = type; 
	rule_input->resp = resp; 

	//rule_input->desc.params[ 0 ].type = APP_DEFINE; 

	for( i = 0; i < MAX_RULE_PARAM_NUM; i ++ )
	{
		rule_input->desc.params[ i ].type = INVALID_PARAM_DEFINE_TYPE; 
		rule_input->desc.params[ i ].id = INVALID_ID_VALUE; 
		rule_input->desc.params[ i ].is_cls = FALSE; 
		rule_input->desc.params[ i ].common.name = NULL; 
	}

	rule_input->desc.params[ 0 ].app.app_name = app_name; 
	rule_input->desc.params[ 1 ].common.name = name_param; 

	_set_define_type_in_rule( rule_input ); 

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ret; 
}

LRESULT init_access_rule_ptr_desc( access_rule_ptr_desc *rule_input )
{
	LRESULT ret = ERROR_SUCCESS; 
	//access_rule_ptr_desc rule_input; 
	INT32 i; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	rule_input->type = INVALID_RULE_TYPE; 

	//rule_input->desc.params[ 0 ].type = APP_DEFINE; 

	for( i = 0; i < MAX_RULE_PARAM_NUM; i ++ )
	{
		rule_input->desc.params[ i ].type = INVALID_PARAM_DEFINE_TYPE; 
		rule_input->desc.params[ i ].id = INVALID_ID_VALUE; 
		rule_input->desc.params[ i ].is_cls = FALSE; 
		memset( &rule_input->desc.params[ i ].common, 0, sizeof( param_define_input ) ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ret; 
}

LRESULT add_test_url_rule()
{
	LRESULT ret = ERROR_SUCCESS; 
	access_rule_ptr_desc rule_input; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	return ret; 

	rule_input.resp = ACTION_BLOCK; 
	//*rule_input.addition = L'\0'; 
	rule_input.type = URL_RULE_TYPE; 

	rule_input.desc.params[ 0 ].type = APP_DEFINE; 
	rule_input.desc.params[ 0 ].id = INVALID_ID_VALUE; 
	rule_input.desc.params[ 0 ].is_cls = FALSE; 

	rule_input.desc.params[ 0 ].app.app_name = L""; 

	rule_input.desc.params[ 1 ].type = URL_DEFINE; 
	rule_input.desc.params[ 1 ].id = INVALID_ID_VALUE; 
	rule_input.desc.params[ 1 ].is_cls = FALSE; 

	rule_input.desc.params[ 1 ].app.app_name = L"www.balancesoft.com";  

	_set_define_type_in_rule( &rule_input ); 

	ret = input_rule( &rule_input, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "input the url 1 failed 0x%0.8x \n", ret ) ); 
	}

	rule_input.desc.params[ 1 ].app.app_name = L"www.baidu.com";  

	_set_define_type_in_rule( &rule_input ); 

	ret = input_rule( &rule_input, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "input the url 2 failed 0x%0.8x \n", ret ) ); 
	}

	rule_input.desc.params[ 1 ].app.app_name = L"www.sina.com";  

	_set_define_type_in_rule( &rule_input ); 

	ret = input_rule( &rule_input, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "input the url 3 failed 0x%0.8x \n", ret ) ); 
	}

	rule_input.desc.params[ 1 ].app.app_name = L"www.google.com";  

	_set_define_type_in_rule( &rule_input ); 

	ret = input_rule( &rule_input, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "input the url 2 failed 0x%0.8x \n", ret ) ); 
	}

	rule_input.desc.params[ 1 ].app.app_name = L"www.simpai.net";  

	_set_define_type_in_rule( &rule_input ); 

	ret = input_rule( &rule_input, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "input the url 3 failed 0x%0.8x \n", ret ) ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ret; 
}

#include "config_loader.h"

INT32 add_url_rule_from_config( LPCTSTR text, ULONG text_type, ULONG flags, PVOID param )
{

	LRESULT ret = ERROR_SUCCESS; 
	access_rule_ptr_desc rule_input; 
	CHAR output[ MAX_PATH ]; 
#ifdef _UNICODE
	LPSTR text_out = NULL; 
	ULONG need_len; 
#endif //_UNICODE


	log_trace( ( MSG_INFO, "enter %s url is %ws\n", __FUNCTION__, text ) ); 

	init_name_access_rule_ptr_desc( &rule_input, ACTION_BLOCK, URL_RULE_TYPE, L"", text ); 

	ret = input_rule( &rule_input, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "input the url 1 failed 0x%0.8x \n", ret ) ); 
	}

_return:

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

#ifdef LOAD_RULE_FROM_FILE
LRESULT input_rule_from_conf_file( LPCTSTR _file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR file_name[ MAX_PATH ]; 

	ret = load_filter_text_from_file( add_url_rule_from_config, NULL, _file_name );
	if( ret < 0 )
	{
		if( ret == FORMAT_ERROR )
		{
			::MessageBox( NULL, _T( "配置文件格式不正确!" ), NULL, MB_OK ); 
			goto _return; 
		}

		ret = 0; 
	}

_return:
	return ret; 
}
#endif //LOAD_RULE_FROM_FILE

bitsafe_function_state all_drv_open_state[ MAX_FUNCTION_TYPE ] = { { BITSAFE_FUNCTION_DISABLED }, //SEVENFW_DRIVER
														{ BITSAFE_FUNCTION_DISABLED }, //HTTP_TEXT_FLT_DRIVER
														{ BITSAFE_FUNCTION_DISABLED }, //HTTP_URL_FLT_DRIVER 
														{ BITSAFE_FUNCTION_DISABLED }, //ANTI_ARP_DRIVER
														{ BITSAFE_FUNCTION_DISABLED }, //NET_MON_DRIVER
//														{ BITSAFE_FUNCTION_DISABLED }, //SYS_MON_DRIVER
														{ BITSAFE_FUNCTION_DISABLED }, //TRACE_LOG_DRIVER 
//#ifndef DELAY_WIN7_SUPPORT
//														{ BITSAFE_FUNCTION_DISABLED }, //NET_FW_DRIVER
//#endif //DELAY_WIN7_SUPPORT
//														{ BITSAFE_FUNCTION_DISABLED }, //SEVEN_FW_WIN7_DRIVER
														{ BITSAFE_FUNCTION_DISABLED }, //REG_MNG_DRIVER
														{ BITSAFE_FUNCTION_DISABLED } }; //FS_MNG_DRIVER 



CRITICAL_SECTION drv_open_state_lock; 
BOOLEAN drv_state_lock_inited = FALSE; 

LRESULT update_driver_conf( bitsafe_function_type type, bitsafe_function_state load_state )
{
	LRESULT ret = ERROR_SUCCESS; 
	sqlite3 *db; 
	TCHAR sql_stmt[ DRV_CONF_SQL_STMT_LEN ]; 

	db = conf_db; 
	ret = _sntprintf( sql_stmt, DRV_CONF_SQL_STMT_LEN, SQL_UPDATE_DRIVER_LOAD_CONF, load_state, type ); 
	if( ret <= 0 )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )db, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
#define SQL_CHECK_DRIVER_LOAD_CONF _T( "SELECT COUNT(*) FROM " ) DRIVER_LOAD_CONF_TABLE _T( " WHERE drv_id=%u " )

		__Trace( _T( "%s insert driver conf state error: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );

		if( ret == SQLITE_ERROR )
		{
#ifdef _DEBUG
			LRESULT _ret; 
			ULONG count; 
			data_process data_proc = { 0 }; 

			data_proc.data_proc_func = check_result_count; 
			data_proc.param = &count; 

			_sntprintf( sql_stmt, DRV_CONF_SQL_STMT_LEN, SQL_CHECK_DRIVER_LOAD_CONF, type ); 

			__Trace( _T( "%s sql %s extented error code %d reason %s\n" ), __TFUNCTION__, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );

			_ret = sql_exec( ( sqlite3* )db, sql_stmt, &data_proc );  
			if( _ret != SQLITE_OK )
			{
				__Trace( _T( "%s check rule failed: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );
			}

			log_trace( ( MSG_INFO, "check the insert operation failed reason, check the row to input already exist in the database %d\n", count ) ); 

			if( count != 0 )
			{
				ASSERT( FALSE && "insert row failed,but not is aleady exist error\n" ); 
				goto _return; 
			}
#endif //_DEBUG
		}

		ret = ERROR_ERRORS_ENCOUNTERED; 
	}

_return:

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT input_driver_state( bitsafe_function_type type, bitsafe_function_state load_state )
{
	LRESULT ret = ERROR_SUCCESS; 
	sqlite3 *db; 
	TCHAR sql_stmt[ DRV_CONF_SQL_STMT_LEN ];

	db = conf_db; 
	ret = _sntprintf( sql_stmt, DRV_CONF_SQL_STMT_LEN, SQL_INSERT_DRIVER_LOAD_CONF, type, load_state ); 
	if( ret <= 0 )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )db, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
#define SQL_CHECK_DRIVER_LOAD_CONF _T( "SELECT COUNT(*) FROM " ) DRIVER_LOAD_CONF_TABLE _T( " WHERE drv_id=%u " )

		__Trace( _T( "%s insert driver conf state error: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );

		if( ret == SQLITE_CONSTRAINT )
		{
#ifdef _DEBUG
			LRESULT _ret; 
			ULONG count; 
			data_process data_proc = { 0 }; 

			data_proc.data_proc_func = check_result_count; 
			data_proc.param = &count; 

			_sntprintf( sql_stmt, DRV_CONF_SQL_STMT_LEN, SQL_CHECK_DRIVER_LOAD_CONF, type ); 
			
			__Trace( _T( "%s sql %s extented error code %d reason %s\n" ), __TFUNCTION__, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );

			_ret = sql_exec( ( sqlite3* )db, sql_stmt, &data_proc );  
			if( _ret != SQLITE_OK )
			{
				__Trace( _T( "%s check rule failed: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );
			}

			log_trace( ( MSG_INFO, "check the insert operation failed reason, check the row to input already exist in the database %d\n", count ) ); 

			if( count == 0 )
			{
				ASSERT( FALSE && "insert row failed,but not is aleady exist error\n" ); 
				//goto _return; 
			}
#endif //_DEBUG
		}

		ret = ERROR_ERRORS_ENCOUNTERED; 
	}

_return:

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT config_driver( bitsafe_function_type type, bitsafe_function_state load_state )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		DBG_BP(); 
		ret = input_driver_state( type, load_state ); 
		if( ret == ERROR_SUCCESS )
		{
			break; 
		}

		ret = update_driver_conf( type, load_state ); 
	} while ( FALSE );

	return ret; 
}

LRESULT config_general( CHAR* val_name, CHAR *val_data )
{
	LRESULT ret = ERROR_SUCCESS; 
	sqlite3 *db; 
	TCHAR *sql_stmt = NULL; //[ GENERAL_SQL_STMT_LEN ]; 

	ASSERT( val_name != NULL ); 
	ASSERT( val_data != NULL ); 

	sql_stmt = ( WCHAR* )malloc( sizeof( GENERAL_SQL_STMT_LEN ) * sizeof( WCHAR ) ); 

	if( sql_stmt == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	db = conf_db; 
	ret = _sntprintf( sql_stmt, GENERAL_SQL_STMT_LEN, SQL_INSERT_GENERAL_CONF, val_name, val_data ); 
	if( ret <= 0 )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	ret = sql_exec( ( sqlite3* )db, sql_stmt, NULL ); 
	if( ret != SQLITE_OK )
	{
#define SQL_CHECK_GENERAL_CONF _T( "SELECT COUNT(*) FROM " ) GENERAL_CONF_TABLE _T( " WHERE val_name=%s and val_data=%s" )

		__Trace( _T( "%s insert driver conf state error: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );

		if( ret == SQLITE_CONSTRAINT )
		{
#ifdef _DEBUG
			LRESULT _ret; 
			ULONG count; 
			data_process data_proc = { 0 }; 

			data_proc.data_proc_func = check_result_count; 
			data_proc.param = &count; 


			_sntprintf( sql_stmt, DRV_CONF_SQL_STMT_LEN, SQL_CHECK_DRIVER_LOAD_CONF, val_name, val_data ); 

			__Trace( _T( "%s sql %s extented error code %d reason %s\n" ), __TFUNCTION__, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );

			_ret = sql_exec( ( sqlite3* )db, sql_stmt, &data_proc );  
			if( _ret != SQLITE_OK )
			{
				__Trace( _T( "%s check rule failed: %d, sql %s extented error code %d reason %s\n" ), __TFUNCTION__, ret, sql_stmt, sqlite3_extended_errcode( ( sqlite3* )db ), sqlite3_errmsg16( ( sqlite3* )db ) );
			}

			log_trace( ( MSG_INFO, "check the insert operation failed reason, check the row to input already exist in the database %d\n", count ) ); 

			if( count == 0 )
			{
				ASSERT( FALSE && "insert row failed,but not is aleady exist error\n" ); 
			}
#endif
		}

		ret = ERROR_ERRORS_ENCOUNTERED; 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT config_all_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	_ret = config_driver( NET_PACKET_FLT_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "write driver config error 0x%0.8x\n", _ret ) ); 

		ret = _ret; 
	}

	_ret = config_driver( HTTP_TEXT_FLT_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "write driver config error 0x%0.8x\n", _ret ) ); 
		ret = _ret; 
	}

	_ret = config_driver( HTTP_URL_FLT_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "write driver config error 0x%0.8x\n", _ret ) ); 
		ret = _ret; 
	}

	_ret = config_driver( ANTI_ARP_ATTACK_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "write driver config error 0x%0.8x\n", _ret ) ); 
		ret = _ret; 
	}

	_ret = config_driver( NET_COMM_MANAGE_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "write driver config error 0x%0.8x\n", _ret ) ); 
		ret = _ret; 
	}

	_ret = config_driver( TRACE_LOG_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "write driver config error 0x%0.8x\n", _ret ) ); 
		ret = _ret; 
	}

	_ret = config_driver( FILE_MANAGE_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "write driver config error 0x%0.8x\n", _ret ) ); 
		ret = _ret; 
	}

	return ret; 
}

LRESULT _load_driver_conf( drv_state_callback callback_func, 
						 alloc_record_callback alloc_func, 
						 PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	//TCHAR *sql_stmt; 
	LPCWSTR table_name; 
	sqlite3 *db; 
	data_process db_process; 

#define SELECT_ALL_DRIVER_CONF _T( "select * from " ) DRIVER_LOAD_CONF_TABLE _T( ";" )

	db = conf_db; 

	db_process.alloc_func = alloc_func; 
	db_process.callback_func = callback_func; 
	db_process.data_proc_func = read_drv_type; 
	db_process.param = param; 
	db_process.type = ( ULONG )0; 

	ret = sql_exec( db, SELECT_ALL_DRIVER_CONF, &db_process ); 
	if( ret != SQLITE_OK )
	{
		ret = ERROR_DB_SELECT_ERROR; 
		goto _return; 
	}

_return:

	return ret; 
}

LRESULT CALLBACK read_drv_state_callback( bitsafe_function_type type, bitsafe_function_state state, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	bitsafe_function_state *input; 

	input = ( bitsafe_function_state* )param; 

	if( FALSE == is_valid_bitsafe_function_type( type ) )
	{
		ASSERT( FALSE && "invalid driver type to input driver state" ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( FALSE == is_valid_bitsafe_function_state( state ) )
	{
		ASSERT( FALSE && "invalid driver state to input driver state" ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	input[ type ] = state; 

_return:
	return ret; 
}

LRESULT load_driver_conf( bitsafe_function_state all_drv_state[ MAX_FUNCTION_TYPE ] )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	for( i = 0; i < MAX_FUNCTION_TYPE; i ++ )
	{
		all_drv_state[ i ] = BITSAFE_FUNCTION_DEFAULT; 
	}

	ret = _load_driver_conf( read_drv_state_callback, NULL, all_drv_state ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 

		log_trace( ( MSG_ERROR, "get the driver load state error 0x%0.8x\n", ret ) ); 
	}

	return ret; 
}

LRESULT load_general_conf( read_param_record_callback callback_func, 
						 alloc_record_callback alloc_func, 
						 PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	//TCHAR *sql_stmt; 
	LPCWSTR table_name; 
	sqlite3 *db; 
	data_process db_process; 

#define SELECT_ALL_DRIVER_CONF _T( "select * from " ) GENERAL_CONF_TABLE _T( ";" )

	db = conf_db; 

	db_process.alloc_func = alloc_func; 
	db_process.callback_func = callback_func; 
	db_process.data_proc_func = read_common_conf; 
	db_process.param = param; 
	db_process.type = ( ULONG )0; 

	ret = sql_exec( db, SELECT_ALL_DRIVER_CONF, &db_process ); 
	if( ret != SQLITE_OK )
	{
		ret = ERROR_DB_SELECT_ERROR; 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT init_bitsafe_function_state()
{
	LRESULT ret; 
	init_cs_lock( drv_open_state_lock ); 
	drv_state_lock_inited = TRUE; 

	lock_cs( drv_open_state_lock ); 
	ret = load_driver_conf( all_drv_open_state ); ; 
	unlock_cs( drv_open_state_lock ); 

	return ret; 
}

LRESULT uninit_bitsafe_function_state()
{
	LRESULT ret = ERROR_SUCCESS; 
	uninit_cs_lock( drv_open_state_lock ); 
	drv_state_lock_inited = FALSE; 

	return ret; 
}

LRESULT config_bitsafe_function_state(  bitsafe_function_type type, bitsafe_function_state load_state )
{
	LRESULT ret = ERROR_SUCCESS; 

	DBG_BP(); 
	if( FALSE == is_valid_bitsafe_function_type( type ) )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = config_driver( type, load_state ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "config driver state error" ); 
		log_trace( ( MSG_ERROR, "configure the driver %u state %u error 0x%0.8x\n", type, load_state, ret ) ); 
	}

	if( TRUE == drv_state_lock_inited )
	{
		lock_cs( drv_open_state_lock ); 
		all_drv_open_state[ type ] = load_state; 
		unlock_cs( drv_open_state_lock ); 
	}
	else
	{
		DBG_BP(); 
		all_drv_open_state[ type ] = load_state; 
	}

_return:
	return ret; 
}

LRESULT check_bitsafe_function_state( bitsafe_function_type type, bitsafe_function_state *load_state )
{
	LRESULT ret = ERROR_SUCCESS; 
	bitsafe_function_state _load_state; 

	if( TRUE == drv_state_lock_inited )
	{
		lock_cs( drv_open_state_lock ); 
		_load_state = all_drv_open_state[ type ]; 
		unlock_cs( drv_open_state_lock ); 
	}
	else
	{
		_load_state = all_drv_open_state[ type ]; 
	}

	*load_state = _load_state; 

	return ret; 
}

#define SEVEN_FW_SYSMODULE
#include "seven_fw_api.h"
#include "Acl_Domain.h"
#include "bitsafe_rule_conf.h"
#include "url_hash_table.h"
#include "http_parse.h"
#include "rbtree.h"
#include "socket_rule.h"
ULONG trace_log_id = INVALID_ID_VALUE; 

#ifdef TEST_SYS_EVENT_KERNEL
LRESULT stop_test_conf_function()
{
	LRESULT ret = ERROR_SUCCESS; 
	uninit_action_manage(); 
	release_sys_events_list( HAVE_PENDING_EVENT ); 
	release_data_flow_trace(); 
	UninitAllAclList(); 
	release_http_filter_list(); 

	return ret; 
}
#endif //TEST_SYS_EVENT_KERNEL

#include <winsock.h>
#pragma comment( lib, "ws2_32.lib" )

INLINE LRESULT fixup_up_addr_encode()
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ip_addr; 
	ULONG ip_addr2; 

	ret = str_2_ipv4_addr( _T( "192.168.101.2" ),  &ip_addr ); 
	ip_addr2 = inet_addr( "192.168.101.2" ); 

	if( ip_addr != ip_addr2 )
	{
		if( htonl( ip_addr ) != ip_addr2 )
		{
			ASSERT( FALSE ); 
		}
	}

	PrintIPAddr( &ip_addr ); 

	ret = str_2_ipv4_addr( _T( "192.168.101.2" ),  &ip_addr ); 
	ret = str_2_ipv4_addr( _T( "192.168.101.3" ), &ip_addr2 ); 

	if( ip_addr2 <= ip_addr )
	{
		ASSERT( FALSE ); 
	}

_return:
	return ret; 
}

#ifdef DEBUG_DRIVER
#define TEST_REGION_NUM 1000
LRESULT test_socket_rule()
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG region_len; 
	ULONG region_begin; 
	INT32 i; 

	access_rule_desc rule_input; 
	sys_action_record socket_action; 
	socket_rule_item *rule_found; 
	rb_node *node_out; 

	init_access_rule( SOCKET_RULE_TYPE, &rule_input ); 

	fixup_up_addr_encode();

	region_len = 0xFFFFFFFF / ( TEST_REGION_NUM * 2 ); 

	for( i = 0; i < TEST_REGION_NUM; i ++ )
	{	
		rule_input.type = SOCKET_RULE_TYPE; 

		rule_input.desc.socket.app.type = APP_DEFINE; 
		rule_input.desc.socket.dest_ip.type = IP_DEFINE; 
		rule_input.desc.socket.src_ip.type = IP_DEFINE; 
		rule_input.desc.socket.dest_port.type = PORT_DEFINE; 
		rule_input.desc.socket.src_port.type = PORT_DEFINE; 
		rule_input.desc.socket.src_port.port.type = ALL_PROT; 
		rule_input.desc.socket.dest_port.port.type = ALL_PROT; 

		region_begin = i * region_len; 

		rule_input.desc.socket.dest_ip.ip.ip_begin = region_begin; 
		rule_input.desc.socket.dest_ip.ip.ip_end = region_begin + region_len; 

		rule_input.desc.socket.src_ip.ip.ip_begin = 0; 
		rule_input.desc.socket.src_ip.ip.ip_begin = 0; 
		rule_input.resp = ( action_response_type )( i % MAX_RESPONSE_TYPE ); 

		ntstatus = rb_insert_socket_rule( &rule_input, &node_out ); 
		if( !NT_SUCCESS( ntstatus ) )
		{
			if( ntstatus == STATUS_LIST_ITEM_ALREADY_EXIST )
			{
				log_trace( ( MSG_INFO, "insert the socket rule region ( %u-%u ) %s 0x%0.8x\n", 
					i * region_len, 
					i * region_len + region_len, 
					"no meaning because sam meaning rule already exist", 
					node_out ) ); 

			}
			else
			{
				log_trace( ( MSG_INFO, "insert the socket rule region ( %u-%u ) %s 0x%0.8x\n", 
					i * region_len, 
					i * region_len + region_len, 
					"failed", 
					node_out ) ); 

				goto _return; 
			}
		}

		log_trace( ( MSG_INFO, "insert the socket rule region ( %u-%u ) %s 0x%0.8x\n", 
			i * region_len, 
			i * region_len + region_len, 
			"successfully", 
			node_out ) ); 
	}

	for( i = 0; i < TEST_REGION_NUM; i ++ )
	{
		action_response_type resp; 

		socket_action.type = SOCKET_SEND; 

		socket_action.socket_info.prot = PROT_TCP; 

		ret = str_2_ipv4_addr( _T( "192.168.102.165" ), &socket_action.socket_info.src_ip ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		socket_action.socket_info.dest_ip = ( i + TEST_REGION_NUM ) * region_len + ( i + ( region_len / TEST_REGION_NUM ) );  

		socket_action.socket_info.src_port = 19; 
		socket_action.socket_info.dest_port = 30; 

		ntstatus = rb_check_socket_action( &socket_rule_rbt, NULL, &socket_action.socket_info, NULL, &resp ); 

		if( !NT_SUCCESS( ntstatus ) )
		{
			log_trace( ( MSG_INFO, "can't found the ip address %d\n", socket_action.socket_info.dest_ip ) ); 

			continue; 
		}

		ASSERT( FALSE ); 
		break; 

		ASSERT( is_valid_response_type( resp ) == TRUE ); 

		if( resp == ACTION_BLOCK )
		{
			ret = ERROR_ACCESS_DENIED; 
		}
		else if( resp == ACTION_ALLOW )
		{
			ret = ERROR_SUCCESS;
		}

		log_trace( ( MSG_INFO, "socket rule found the response is %s\n", get_response_desc( resp ) ) ); 
	}

_return:

	return ret; 
}
#endif //DEBUG_DRIVER

#ifdef DEBUG_DRIVER
INT32 test_conf_function( ULONG mode )
{
	LRESULT ret = 0; 
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 i; 
	CStdString test_str; 
	CStdString test_reg_path; 
	sqlite3 *test_db; 
	time_t cur_time; 
	defense_log _defense_log; 
	sys_action_desc action_desc; 
	fw_log _fw_log; 
	INT32 test_add = TRUE; 
	access_rule_ptr_desc rule_input; 
	access_rule_desc rule_config; 
	INT32 rule_manage_inited = FALSE; 
	INT32 sys_events_inited = FALSE; 
	INT32 data_flow_trace_inited = FALSE; 
	INT32 all_acl_inited = FALSE; 
	INT32 http_flt_context_inited = FALSE; 
	action_response_type response; 
	MSG_FLT flt_setting; 
	INT32 need_log = FALSE; 

	//ret = test_trace_log(); 

	ntstatus = init_action_manage(); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

	rule_manage_inited = TRUE; 

	ntstatus = init_sys_events_list( HAVE_PENDING_EVENT ); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}
	sys_events_inited = TRUE; 

	ntstatus = init_data_flow_trace(); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

	data_flow_trace_inited = TRUE; 

	ntstatus = InitAllAclList(); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

	all_acl_inited = TRUE; 

	ntstatus = init_http_filter_list(); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

	test_socket_rule(); 

	release_socket_rule_rbt(); 

	goto _return; 

	http_flt_context_inited = TRUE; 

	flt_setting.lvl = detail_flt; 
	flt_setting.name = TRACE_LOGGER_SYSTEM; 
	flt_setting.release_notify_func = NULL; 
	flt_setting.setting_num = 0; 
	flt_setting.settings[ 0 ].mode = proc_name_flt; 
	memset( &flt_setting.settings[ 0 ].value, 0, sizeof( FLT_TYPE ) ); 

	add_trace_logger( &flt_setting, sizeof( flt_setting ), &trace_log_id ); 

	//get_fw_trust_apps( check_trust_app, NULL ); 
	//get_fw_log( check_fw_log, NULL, NULL ); 
	if( test_add == TRUE )
	{
		for( i = 0; i < 100; i ++ )
		{
			if( mode & TEST_REG_RULE )
			{
				test_str.Format( _T( "test_app %d" ), i ); 
				test_reg_path.Format( _T( "HKEY_LOCAL_MACHINE\\SYSTEM\\test%d" ), i  ); 

				time( &cur_time ); 

				init_name_access_rule_ptr_desc( &rule_input, ACTION_LEARN, REG_RULE_TYPE, L"", test_reg_path.GetData() ); 

				ret = input_rule( &rule_input, 0 ); 
			}

			__Trace( _T( "add registry path %s rule return 0x%0.8x\n" ), test_reg_path.GetData(), ret ); 
			
			//ASSERT( FALSE ); 
			//goto _return; 
		}
	}

	if( mode & TEST_SOCKET_RULE )
	{
		access_rule_desc _rule_input; 
		init_access_rule_ptr_desc( &rule_input ); 
		rule_input.type = SOCKET_RULE_TYPE; 

		rule_input.desc.socket.app.type = APP_DEFINE; 
		//rule_input.desc.socket.app.app.app_name = app_path.GetData(); 
		rule_input.desc.socket.dest_ip.type = IP_DEFINE; 
		rule_input.desc.socket.src_ip.type = IP_DEFINE; 
		rule_input.desc.socket.dest_port.type = PORT_DEFINE; 
		rule_input.desc.socket.src_port.type = PORT_DEFINE; 
		rule_input.desc.socket.src_port.port.type = ALL_PROT; 
		rule_input.desc.socket.dest_port.port.type = ALL_PROT; 


		ret = str_2_ipv4_addr( _T( "0.0.0.0" ), &rule_input.desc.socket.dest_ip.ip.ip_begin ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		ret = str_2_ipv4_addr( _T( "255.255.255.255" ), &rule_input.desc.socket.dest_ip.ip.ip_end ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		rule_input.resp = ACTION_LEARN; 

		ret = input_rule( &rule_input, APP_DEFINE_RULE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "add the socket action rule failed %ws \n", "0 - 255.255.255.255" ) ); 
		}

		init_access_rule( rule_input.type, &_rule_input ); 

		ret = copy_rule_input_param( &_rule_input, &rule_input ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = config_rule( &_rule_input, NULL, ADD_RULE_DEFINE, APP_ACTION_RULE ); 
		if( ret == ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "configure the socket action rule failed %ws \n", "0 - 255.255.255.255" ) ); 
		}
	}
	
	if( mode & TEST_COMMON_DRIVER_RULE )
	{
		ret = init_driver_manage_rule( L"C:\\Windows\\system32\\drivers\\sevenfw.sys", ACTION_LEARN, &rule_config ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		ret = input_rule_to_config( &rule_config, INSTALL_DRV ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = config_rule( &rule_config, NULL, ADD_RULE_DEFINE, APP_ACTION_RULE ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( http_flt_context_inited == TRUE )
		{
			release_http_filter_list(); 
		}

		if( all_acl_inited == TRUE )
		{
			UninitAllAclList(); 
		} 

		if( rule_manage_inited = TRUE )
		{
			uninit_action_manage(); 
		}
		if( sys_events_inited == TRUE )
		{
			release_sys_events_list( HAVE_PENDING_EVENT ); 
		}
		if( data_flow_trace_inited == TRUE )
		{
			release_data_flow_trace(); 
		}
	}

	return ret; 
}

#endif //DEBUG_DRIVER;

LRESULT CALLBACK check_fw_rule( access_rule_type type, action_rule_record *record, PVOID param )
{
	INT32 i; 
	LRESULT ret = ERROR_SUCCESS; 
	CStdString dbg_out; 
	action_rule_record *_rule; 
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	access_rule_desc rule_input; 
	action_rule_define rule_define; 
	sys_action_desc cur_action_desc; 
	action_response_type response; 
	ULONG param_count;  

_return:
	return ret; 
}

LRESULT load_acl_to_ring3()
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 i; 
	CStdString test_str; 
	CStdString test_reg_path; 
	sqlite3 *test_db; 
	time_t cur_time; 
	defense_log _defense_log; 
	sys_action_desc action_desc; 
	fw_log _fw_log; 
	INT32 test_add = TRUE; 
	access_rule_ptr_desc rule_input; 
	access_rule_desc rule_config; 
	INT32 rule_manage_inited = FALSE; 
	INT32 sys_events_inited = FALSE; 
	//INT32 data_flow_trace_inited = FALSE; 
	INT32 all_acl_inited = FALSE; 
	INT32 http_flt_context_inited = FALSE; 
	action_response_type response; 
	//MSG_FLT flt_setting; 
	INT32 need_log = FALSE; 
	return ret; 
}

LRESULT release_acl_from_ring3()
{
	LRESULT ret = ERROR_SUCCESS; 

	DBG_BP(); 

#ifdef DRIVER_DEBUG
	release_sys_events_list( HAVE_PENDING_EVENT ); 
#endif //DRIVER_DEBUG

	return ret; 
}

INLINE VOID init_param_id( param_define_id *param )
{
	ASSERT( param != NULL ); 

	param->id = INVALID_ID_VALUE; 
	*param->class_name = L'\0'; 
}

LRESULT get_param_define_id( param_input_desc *param, param_define_id *param_id )
{
	LRESULT ret = ERROR_SUCCESS; 
	sql_stmt_desc *sql_stmt = NULL; 
	sqlite3 *db; 
	data_process data_proc = { 0 }; 

	ASSERT( param_id != NULL ); 

	init_param_id( param_id ); 

	ret = construct_sql_stmt_by_param( SELECT_ROW, param, &sql_stmt, &db ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ASSERT( db != NULL );
	ASSERT( sql_stmt != NULL ); 
	ASSERT( sql_stmt->sql != NULL ); 

	data_proc.alloc_func = NULL; 
	data_proc.data_proc_func = check_result_count; 
	data_proc.callback_func = NULL; 
	data_proc.param = &param_id->id; 
	data_proc.type = param->type; 

	ret = sql_exec( db, sql_stmt->sql, &data_proc ); 
	if( ret != SQLITE_OK )
	{
		ret = ERROR_DB_SELECT_ERROR; 
		goto _return; 
	}

_return:

	if( sql_stmt != NULL )
	{
		release_sql_stmt_desc( sql_stmt ); 
	}
	return ret; 
}

LRESULT _get_param_define_id( param_all_desc *param, param_define_id *param_id )
{
	LRESULT ret = ERROR_SUCCESS; 
	sql_stmt_desc *sql_stmt = NULL; 
	sqlite3 *db; 
	data_process data_proc = { 0 }; 

	init_param_id( param_id ); 

	ret = _construct_sql_stmt_by_param( SELECT_ROW, param, &sql_stmt, &db ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ASSERT( db != NULL );
	ASSERT( sql_stmt != NULL ); 
	ASSERT( sql_stmt->sql != NULL ); 

	data_proc.alloc_func = NULL; 
	data_proc.data_proc_func = check_result_count; 
	data_proc.callback_func = NULL; 
	param_id->id = INVALID_ID_VALUE; 

	data_proc.param = &param_id->id; 
	data_proc.type = param->type; 

	ret = sql_exec( db, sql_stmt->sql, &data_proc ); 
	if( ret != SQLITE_OK )
	{
		ret = ERROR_DB_SELECT_ERROR; 
		goto _return; 
	}

	if( param_id->id == INVALID_ID_VALUE )
	{
		ret = ERROR_DB_SELECT_ERROR; 
		goto _return; 
	}

_return:

	if( sql_stmt != NULL )
	{
		release_sql_stmt_desc( sql_stmt ); 
	}
	return ret; 
}

LRESULT alloc_action_rule( access_rule_type type, action_response_type action, access_rule_desc **rule_alloc )
{
	LRESULT ret = ERROR_SUCCESS; 

	access_rule_desc *_rule_alloc; 

	*rule_alloc = NULL; 

	_rule_alloc = ( access_rule_desc* )malloc( sizeof( access_rule_desc ) ); 

	if( _rule_alloc == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	memset( _rule_alloc, 0, sizeof( access_rule_desc ) ); 

	_rule_alloc->type = type; 
	_rule_alloc->resp = action; 

	*rule_alloc = _rule_alloc; 

_return:
	return ret; 
}

LRESULT set_com_name( LPCTSTR class_name, LPCTSTR com_name, access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 
	ASSERT( rule->type == COM_RULE_TYPE ); 

	rule->desc.com.com.type = COM_DEFINE; 

	if( class_name != NULL )
	{
		if( wcslen( class_name ) > _MAX_CLASS_NAME_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.com.com.cls.class_name, class_name ); 

		rule->desc.com.com.is_cls = TRUE; 
	}
	else
	{
		if( wcslen( com_name ) > _MAX_COM_NAME_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.com.com.com.com_name, com_name ); 
		rule->desc.com.com.is_cls = FALSE; 
	}

_return:
	return ret; 
}

LRESULT set_url_name( LPCTSTR class_name, LPCTSTR url_name, access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 
	ASSERT( rule->type == URL_RULE_TYPE ); 

	rule->desc.url.url.type = URL_DEFINE; 

	if( class_name != NULL )
	{
		if( wcslen( class_name ) > _MAX_CLASS_NAME_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.url.url.cls.class_name, class_name ); 

		rule->desc.url.url.is_cls = TRUE; 
	}
	else
	{
		if( wcslen( url_name ) > _MAX_URL_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.url.url.url.url, url_name ); 
		rule->desc.url.url.is_cls = FALSE; 
	}

_return:
	return ret; 
}

LRESULT set_file_name( LPCTSTR class_name, LPCTSTR file_name, access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 
	ASSERT( rule->type == FILE_RULE_TYPE ); 

	rule->desc.file.file_path.type = FILE_DEFINE; 

	if( class_name != NULL )
	{
		if( wcslen( class_name ) > _MAX_CLASS_NAME_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.file.file_path.cls.class_name, class_name ); 

		rule->desc.file.file_path.is_cls = TRUE; 
	}
	else
	{
		if( wcslen( file_name ) > _MAX_URL_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.file.file_path.file.file_path, file_name ); 
		rule->desc.file.file_path.is_cls = FALSE; 
	}

_return:
	return ret; 
}

LRESULT set_reg_name( LPCTSTR class_name, LPCTSTR reg_name, access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 
	ASSERT( rule->type == REG_RULE_TYPE ); 

	rule->desc.reg.reg_path.type = REG_DEFINE; 

	if( class_name != NULL )
	{
		if( wcslen( class_name ) > _MAX_CLASS_NAME_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.reg.reg_path.cls.class_name, class_name ); 

		rule->desc.reg.reg_path.is_cls = TRUE; 
	}
	else
	{
		if( wcslen( reg_name ) > _MAX_URL_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.reg.reg_path.reg.reg_path, reg_name ); 
		rule->desc.reg.reg_path.is_cls = FALSE; 
	}

_return:
	return ret; 
}

LRESULT set_src_ip( LPCTSTR class_name, ULONGLONG ip_begin, ULONGLONG ip_end, access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == SOCKET_RULE_TYPE ); 

	rule->desc.socket.src_ip.type = IP_DEFINE; 

	if( class_name != NULL )
	{
		if( wcslen( class_name ) > _MAX_CLASS_NAME_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.socket.src_ip.cls.class_name, class_name ); 
		
		rule->desc.socket.src_ip.is_cls = TRUE; 
	}
	else
	{
		if( ip_end < ip_begin )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		rule->desc.socket.src_ip.is_cls = FALSE; 
		rule->desc.socket.src_ip.ip.ip_begin = ip_begin; 
		rule->desc.socket.src_ip.ip.ip_end = ip_end; 
	}

_return:
	return ret; 
}

LRESULT set_dest_ip( LPCTSTR class_name, ULONGLONG ip_begin, ULONGLONG ip_end, access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 

	if( rule->type != SOCKET_RULE_TYPE )
	{
		ret = ERROR_INVALID_HANDLE; 
		goto _return; 
	}

	rule->desc.socket.dest_ip.type = IP_DEFINE; 

	if( class_name != NULL )
	{
		if( wcslen( class_name ) > _MAX_CLASS_NAME_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.socket.dest_ip.cls.class_name, class_name ); 

		rule->desc.socket.dest_ip.is_cls = TRUE; 
	}
	else
	{
		if( ip_end < ip_begin )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		rule->desc.socket.dest_ip.is_cls = FALSE; 
		rule->desc.socket.dest_ip.ip.ip_begin = ip_begin; 
		rule->desc.socket.dest_ip.ip.ip_end = ip_end; 
	}

_return:
	return ret; 
}

LRESULT set_dest_port( LPCTSTR class_name, ULONGLONG port_begin, ULONGLONG port_end, access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 

	if( rule->type != SOCKET_RULE_TYPE )
	{
		ret = ERROR_INVALID_HANDLE; 
		goto _return; 
	}

	rule->desc.socket.dest_port.type = PORT_DEFINE; 

	if( class_name != NULL )
	{
		if( wcslen( class_name ) > _MAX_CLASS_NAME_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.socket.dest_port.cls.class_name, class_name ); 

		rule->desc.socket.dest_port.is_cls = TRUE; 
	}
	else
	{
		if( port_end < port_begin )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		rule->desc.socket.dest_port.is_cls = FALSE; 
		rule->desc.socket.dest_port.port.port_begin = port_begin; 
		rule->desc.socket.dest_port.port.port_end = port_end; 
	}

_return:
	return ret; 
}

LRESULT set_src_port( LPCTSTR class_name, ULONGLONG port_begin, ULONGLONG port_end, access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 

	if( rule->type != SOCKET_RULE_TYPE )
	{
		ret = ERROR_INVALID_HANDLE; 
		goto _return; 
	}

	rule->desc.socket.src_port.type = PORT_DEFINE; 

	if( class_name != NULL )
	{
		if( wcslen( class_name ) > _MAX_CLASS_NAME_LEN - 1 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		wcscpy( rule->desc.socket.src_port.cls.class_name, class_name ); 

		rule->desc.socket.src_port.is_cls = TRUE; 
	}
	else
	{
		if( port_end < port_begin )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		rule->desc.socket.src_port.is_cls = FALSE; 
		rule->desc.socket.src_port.port.port_begin = port_begin; 
		rule->desc.socket.src_port.port.port_end = port_end; 
	}

_return:
	return ret; 
}

VOID release_sql_stmt_desc( sql_stmt_desc *sql_stmt )
{
	ASSERT( sql_stmt != NULL ); 
	if( sql_stmt->sql != NULL )
	{
		free( sql_stmt->sql ); 
		//sql_stmt->sql = NULL; 
	}

	free( sql_stmt ); 
}

LRESULT set_col_val( sql_stmt_desc *sql_stmt, ULONG index, column_type type, LPCWSTR col_name, LPCWSTR col_val )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( sql_stmt != NULL ); 
	ASSERT( col_name != NULL ); 
	ASSERT( col_val != NULL ); 
	ASSERT( is_valid_column_type( type ) );  

	if( index >= MAX_COLUMN_NUM )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	if( wcslen( col_name ) > MAX_CLOUMN_NAME_LEN - 1 )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	if( wcslen( col_val ) > MAX_COLUMN_VAL_LEN - 1 )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	wcscpy( sql_stmt->name_val[ index ].name, col_name ); 
	wcscpy( sql_stmt->name_val[ index ].val, col_val ); 
	sql_stmt->name_val[ index ].type = type; 

_return:
	return ret; 
}

LRESULT add_col_val( sql_stmt_desc *sql_stmt, LPCWSTR col_name, LPCWSTR col_val )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( sql_stmt != NULL ); 

	if( sql_stmt->col_val_num >= MAX_COLUMN_NUM )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	ret = set_col_val( sql_stmt, sql_stmt->col_val_num++, COLUME_STRING_TYPE, col_name, col_val ); 

_return:
	return ret; 
}

#define MAX_LONG_VAL_LEN 12
LRESULT add_col_val_int( sql_stmt_desc *sql_stmt, LPCWSTR col_name, ULONG col_val )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR long_val[ MAX_LONG_VAL_LEN ]; 

	ASSERT( sql_stmt != NULL ); 

	if( sql_stmt->col_val_num >= MAX_COLUMN_NUM )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	ret = set_col_val( sql_stmt, sql_stmt->col_val_num++, COLUME_NUMBER_TYPE, col_name, _ultot( col_val, long_val, 10 ) ); 

_return:
	return ret; 
}

LRESULT alloc_sql_stmt_desc( sql_action action, LPCWSTR table_name, sql_stmt_desc **sql_stmt )
{
	LRESULT ret = ERROR_SUCCESS; 
	sql_stmt_desc *_sql_stmt; 

	ASSERT( table_name != NULL ); 
	ASSERT( sql_stmt != NULL ); 

	if( wcslen( table_name ) > MAX_TABLE_NAME_LEN - 1 )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	_sql_stmt = ( sql_stmt_desc* )malloc( sizeof( sql_stmt_desc ) ); 
	if( _sql_stmt == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	memset( _sql_stmt, 0, sizeof( sql_stmt_desc ) ); 

	_sql_stmt->col_val_num = 0; 
	_sql_stmt->sel_col_num = 0; 
	_sql_stmt->action = action; 
	wcscpy( _sql_stmt->table_name, table_name ); 

	*sql_stmt = _sql_stmt; 

_return:
	return ret; 
}

LRESULT set_sel_col( sql_stmt_desc *sql_stmt, ULONG index, LPCWSTR col_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( sql_stmt != NULL ); 
	ASSERT( col_name != NULL ); 

	if( index >= MAX_COLUMN_NUM )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	if( wcslen( col_name ) > MAX_CLOUMN_NAME_LEN - 1 )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	wcscpy( sql_stmt->sel_cloumn_name[ sql_stmt->col_val_num ], col_name ); 

_return:
	return ret; 
}

LRESULT add_sel_col( sql_stmt_desc *sql_stmt, LPCWSTR col_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( sql_stmt != NULL ); 

	if( sql_stmt->sel_col_num >= MAX_COLUMN_NUM )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	ret = set_sel_col( sql_stmt, sql_stmt->sel_col_num++, col_name ); 

_return:
	return ret; 
}

VOID free_sql_stmt_buf( sql_stmt_desc *sql_stmt )
{
	ASSERT( sql_stmt != NULL ); 
	
	if( sql_stmt->sql != NULL )
	{
		free( sql_stmt->sql ); 
		sql_stmt->sql = NULL; 
	}
}

LRESULT alloc_sql_stmt_buf( sql_stmt_desc *sql_stmt, ULONG size )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	ASSERT( sql_stmt != NULL ); 
	free_sql_stmt_buf( sql_stmt ); 

	sql_stmt->sql = ( WCHAR* )malloc( size ); 
	
	if( sql_stmt->sql == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT generate_sql_stmt( sql_stmt_desc *sql_stmt )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPWSTR _sql; 
	ULONG sql_len; 
	INT32 i; 

	ASSERT( sql_stmt != NULL ); 
	ASSERT( is_valid_sql_action( sql_stmt->action ) ); 

	switch( sql_stmt->action )
	{
	case INSERT_ROW: 
		sql_len = INSERT_CMD_LEN; 
		break; 
	case DELETE_ROW:
		sql_len = DELETE_CMD_LEN; 
		break; 
	case ALTER_ROW:
		sql_len = UPDATE_CMD_LEN; 
		break;
	case SELECT_ROW:
		sql_len = SELECT_CMD_LEN; 
		break; 
	default:
		ASSERT( "invalid sql action type" && FALSE ); 
		break; 
	}

	sql_len += wcslen( sql_stmt->table_name ); 
	sql_len += 1; 

	for( i = 0; i < sql_stmt->col_val_num; i ++ )
	{
		sql_len += wcslen( sql_stmt->name_val[ i ].name ); 
		sql_len += wcslen( sql_stmt->name_val[ i ].val ); 
		sql_len += 10; 
	}

	for( i = 0; i < sql_stmt->sel_col_num; i ++ )
	{
		sql_len += wcslen( sql_stmt->sel_cloumn_name[ i ] ); 
		sql_len += 2; 
	}

	sql_len += 20; 


	free_sql_stmt_buf( sql_stmt ); 

	_sql = ( WCHAR* )malloc( sql_len * sizeof( WCHAR ) ); 


	if( _sql == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ASSERT( is_valid_sql_action( sql_stmt->action ) ); 

	*_sql = L'\0'; 

	switch( sql_stmt->action )
	{
	case INSERT_ROW: 
		wcsncat( _sql, INSERT_CMD, sql_len ); 
		break; 
	case DELETE_ROW:
		wcsncat( _sql, DELETE_CMD, sql_len ); 
		break; 
	case ALTER_ROW:
		wcsncat( _sql, UPDATE_CMD, sql_len ); 
		break;
	case SELECT_ROW:
		wcsncat( _sql, SELECT_CMD, sql_len ); 
		if( sql_stmt->sel_col_num == 0 )
		{
			wcsncat( _sql, L"*", sql_len - wcslen( _sql ) ); 
		}
		else
		{
			for( i = 0; i < ( INT32 )sql_stmt->sel_col_num; i ++ )
			{
				wcsncat( _sql, sql_stmt->sel_cloumn_name[ i ], sql_len - wcslen( _sql ) ); 
				if( i < sql_stmt->sel_col_num - 1 )
				{
					wcsncat( _sql, L", ", sql_len - wcslen( _sql ) ); 
				}
			}
		}

		wcsncat( _sql, L" FROM ", sql_len - wcslen( _sql ) ); 
		break; 
	default:
		ASSERT( "invalid sql action type" && FALSE ); 
		break; 
	}

	wcsncat( _sql, sql_stmt->table_name, sql_len - wcslen( _sql ) ); 

	//if( sql_stmt->action == INSERT_ROW 
	//	|| sql_stmt->action == DELETE_ROW )
	{
		if( sql_stmt->col_val_num > 0 )
		{
			wcsncat( _sql, L" WHERE ", sql_len - wcslen( _sql ) ); 
			for( i = 0; ( ULONG )i < sql_stmt->col_val_num; i ++ )
			{
				wcsncat( _sql, sql_stmt->name_val[ i ].name, sql_len - wcslen( _sql ) );
				wcsncat( _sql, L"=", sql_len - wcslen( _sql ) ); 
				if( sql_stmt->name_val[ i ].type == STRING_PARAM )
				{
					wcsncat( _sql, L"'", sql_len - wcslen( _sql ) ); 
				}

				wcsncat( _sql, sql_stmt->name_val[ i ].val, sql_len - wcslen( _sql ) ); 

				if( sql_stmt->name_val[ i ].type == STRING_PARAM )
				{
					wcsncat( _sql, L"'", sql_len - wcslen( _sql ) ); 
				}

				if( ( ULONG )i < sql_stmt->col_val_num - 1 )
				{
					wcsncat( _sql, L" and ", sql_len - wcslen( _sql ) ); 
				}
			}
		}
	}

	wcsncat( _sql, L";", sql_len - wcslen( _sql ) );

	sql_stmt->sql = _sql; 

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( _sql != NULL )
		{
			free( _sql ); 
		}
	}

	return ret; 
}

LRESULT check_class_exist( param_define_type type, sqlite3 *db, LPCWSTR class_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR table_name; 
	sql_stmt_desc *sql_stmt = NULL; 
	data_process db_process; 
	ULONG row_count; 

	ASSERT( db != NULL ); 
	ASSERT( is_valid_param_define_type( type ) ); 

	ret = get_table_name_for_define( type, NULL, &table_name ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = alloc_sql_stmt_desc( SELECT_ROW, table_name, &sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_sel_col( sql_stmt, L"COUNT(*)" ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( class_name == NULL )
	{
		class_name = L""; 
	}

	ret = add_col_val( sql_stmt, L"class_name", class_name ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = generate_sql_stmt( sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	db_process.alloc_func = NULL; 
	db_process.data_proc_func = NULL; 
	db_process.callback_func = check_result_count; 
	db_process.param = &row_count; 

	ret = sql_exec( db, sql_stmt->sql, &db_process ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	if( row_count == 0 )
	{
		ret = ERROR_NOT_FOUND; 
	}

_return:
	if( sql_stmt != NULL )
	{
		release_sql_stmt_desc( sql_stmt ); 
	}
	return ret; 
}

LRESULT _check_class_exist( param_define_type type, sqlite3 *db, LPCWSTR class_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR table_name; 
	sql_stmt_desc *sql_stmt = NULL; 
	data_process db_process; 
	ULONG row_count; 

	ASSERT( db != NULL ); 
	ASSERT( is_valid_param_define_type( type ) ); 

	ret = get_table_name_for_define( type, NULL, &table_name ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = alloc_sql_stmt_desc( SELECT_ROW, table_name, &sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_sel_col( sql_stmt, L"COUNT(*)" ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_col_val( sql_stmt, L"class_name", class_name ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = generate_sql_stmt( sql_stmt ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	db_process.alloc_func = NULL; 
	db_process.data_proc_func = NULL; 
	db_process.callback_func = check_result_count; 
	db_process.param = &row_count; 

	ret = sql_exec( db, sql_stmt->sql, &db_process ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	if( row_count == 0 )
	{
		ret = ERROR_NOT_FOUND; 
	}

_return:
	if( sql_stmt != NULL )
	{
		release_sql_stmt_desc( sql_stmt ); 
	}
	return ret; 
}

LRESULT __check_ip_define_exist( param_all_desc *param, param_define_id *param_id, ULONG flags )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == IP_DEFINE ); 
	ASSERT( param_id != NULL ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 

	}
	else
	{
		if( flags == AUTO_ADD_PARAM_RECORD )
		{
			ret = add_ip_param_record( ( PVOID )param_define_db, 
				param->ip.ip_begin, 
				param->ip.ip_end,  
				NULL, NULL, 0 ); 

			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}

		ret = _get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0'; 
	}

_return:
	return ret; 
}

LRESULT _check_ip_define_exist( param_all_desc *param, param_define_id *param_id )
{
	return __check_ip_define_exist( param, param_id, AUTO_ADD_PARAM_RECORD ); 
}

LRESULT check_ip_define_exist( param_input_desc *param, param_define_id *param_id )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == IP_DEFINE ); 
	ASSERT( param_id != NULL ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 

	}
	else
	{
		ret = add_ip_param_record( ( PVOID )param_define_db, 
			param->ip.ip_begin, 
			param->ip.ip_end, 
			NULL, NULL, 0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0'; 
	}

_return:
	return ret; 
}

LRESULT __check_port_define_exist( param_all_desc *param, param_define_id *param_id, ULONG flags )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == PORT_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		if( flags == AUTO_ADD_PARAM_RECORD )
		{
			ret = add_port_param_record( ( PVOID )param_define_db, 
				param->port.port_begin, 
				param->port.port_end, 
				param->port.type, 
				NULL, NULL, 0 ); 

			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				goto _return; 
			}
		}
		
		ret = _get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';  
	}

_return:
	return ret; 
}

FORCEINLINE LRESULT _check_port_define_exist( param_all_desc *param, param_define_id *param_id )
{
	return __check_port_define_exist( param, param_id, AUTO_ADD_PARAM_RECORD ); 
}

LRESULT check_port_define_exist( param_input_desc *param, param_define_id *param_id )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == PORT_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		ret = add_port_param_record( ( PVOID )param_define_db, 
			param->port.port_begin, 
			param->port.port_end, 
			param->port.type, 
			NULL, NULL, 0 ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		ret = get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';  
	}

_return:
	return ret; 
}

LRESULT __check_app_define_exist( param_all_desc *param, param_define_id *param_id, ULONG flags )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == APP_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		if( flags == AUTO_ADD_PARAM_RECORD )
		{
			ret = add_app_param_record( ( PVOID )param_define_db, 
				param->app.app_name, 
				NULL, NULL, 0 ); 

			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}

		ret = _get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';  
	}

_return:
	return ret; 
}

FORCEINLINE LRESULT _check_app_define_exist( param_all_desc *param, param_define_id *param_id )
{
	return __check_app_define_exist( param, param_id, AUTO_ADD_PARAM_RECORD ); 
}

LRESULT check_app_define_exist( param_input_desc *param, param_define_id *param_id )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == APP_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		ret = add_app_param_record( ( PVOID )param_define_db, 
			param->app.app_name, 
			NULL, NULL, 0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';  
	}

_return:
	return ret; 
}

LRESULT __check_common_define_exist( param_all_desc *param, param_define_id *param_id, ULONG flags )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == COMMON_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		if( flags == AUTO_ADD_PARAM_RECORD )
		{
			ret = add_common_param_record( ( PVOID )param_define_db, 
				param->common.name, 
				NULL, 
				NULL, 
				0 ); 

			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}

		ret = _get_param_define_id( param, param_id ); 
	}
_return:
	return ret; 
}

FORCEINLINE LRESULT _check_common_define_exist( param_all_desc *param, param_define_id *param_id )
{
	return __check_common_define_exist( param, param_id, AUTO_ADD_PARAM_RECORD ); 
}

LRESULT check_common_define_exist( param_input_desc *param, param_define_id *param_id )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == COMMON_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		ret = add_common_param_record( ( PVOID )param_define_db, 
			param->common.name, 
			NULL, 
			NULL, 0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = get_param_define_id( param, param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
	}
_return:
	return ret; 
}

LRESULT __check_com_define_exist( param_all_desc *param, param_define_id *param_id, ULONG flags )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == COM_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		if( flags == AUTO_ADD_PARAM_RECORD )
		{
			ret = add_com_param_record( ( PVOID )param_define_db, 
				param->com.com_name, 
				NULL, NULL, 0 ); 

			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}
		
		ret = _get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';   
	}
_return:
	return ret; 
}

FORCEINLINE LRESULT _check_com_define_exist( param_all_desc *param, param_define_id *param_id )
{
	return __check_com_define_exist( param, param_id, AUTO_ADD_PARAM_RECORD ); 
}

LRESULT check_com_define_exist( param_input_desc *param, param_define_id *param_id )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == COM_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		ret = add_com_param_record( ( PVOID )param_define_db, 
			param->com.com_name, 
			NULL, NULL, 0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';   
	}
_return:
	return ret; 
}

LRESULT __check_file_define_exist( param_all_desc *param, param_define_id *param_id, ULONG flags )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == FILE_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		if( flags == AUTO_ADD_PARAM_RECORD )
		{
			ret = add_file_param_record( ( PVOID )param_define_db, 
				param->file.file_path, 
				NULL, NULL, 0 ); 

			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}

		ret = _get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';   
	}

_return:
	return ret; 
}

FORCEINLINE LRESULT _check_file_define_exist( param_all_desc *param, param_define_id *param_id )
{
	return __check_file_define_exist( param, param_id, AUTO_ADD_PARAM_RECORD ); 
}

LRESULT check_file_define_exist( param_input_desc *param, param_define_id *param_id )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == FILE_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		ret = add_file_param_record( ( PVOID )param_define_db, 
			param->file.file_path, 
			NULL, NULL, 0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';   
	}

_return:
	return ret; 
}

LRESULT check_reg_define_exist( param_input_desc *param, param_define_id *param_id )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == REG_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		ret = add_reg_param_record( ( PVOID )param_define_db, 
			param->reg.reg_path, 
			NULL, NULL, 0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';   
	}

_return:
	return ret; 
}

LRESULT __check_reg_define_exist( param_all_desc *param, param_define_id *param_id, ULONG flags )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == REG_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		if( flags == AUTO_ADD_PARAM_RECORD )
		{
			ret = add_reg_param_record( ( PVOID )param_define_db, 
				param->reg.reg_path, 
				NULL, NULL, 0 ); 

			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}

		ret = _get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';   
	}

_return:
	return ret; 
}

FORCEINLINE LRESULT _check_reg_define_exist( param_all_desc *param, param_define_id *param_id )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == REG_DEFINE ); 

	ret = __check_reg_define_exist( param, param_id, AUTO_ADD_PARAM_RECORD ); 

_return:
	return ret; 
}

LRESULT __check_url_define_exist( param_all_desc *param, param_define_id *param_id, ULONG flags )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == URL_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		if( flags == AUTO_ADD_PARAM_RECORD )
		{
			ret = add_url_param_record( ( PVOID )param_define_db, 
				param->url.url,  NULL, NULL, 0 ); 

			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}

		ret = _get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';   
	}

_return:
	return ret; 
}

FORCEINLINE LRESULT _check_url_define_exist( param_all_desc *param, param_define_id *param_id )
{
	return __check_url_define_exist( param, param_id, AUTO_ADD_PARAM_RECORD ); 
}

LRESULT check_url_define_exist( param_input_desc *param, param_define_id *param_id )
{
	LRESULT ret; 

	ASSERT( param != NULL ); 
	ASSERT( param->type == URL_DEFINE ); 

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		param_id->id = INVALID_ID_VALUE; 
		ASSERT( wcslen( param->cls.class_name ) < MAX_CLASS_NAME ); 

		wcscpy( param_id->class_name, param->cls.class_name ); 
	}
	else
	{
		ret = add_url_param_record( ( PVOID )param_define_db, 
			param->url.url,  NULL, NULL, 0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
		ret = get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0';   
	}

_return:
	return ret; 
}

LRESULT add_socket_rule( access_rule_ptr_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id src_ip_id; 
	param_define_id src_port_id; 
	param_define_id dest_ip_id; 
	param_define_id dest_port_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == SOCKET_RULE_TYPE ); 

	ret = check_app_define_exist( &rule->desc.socket.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_ip_define_exist( &rule->desc.socket.src_ip, &src_ip_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_ip_define_exist( &rule->desc.socket.dest_ip, &dest_ip_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_port_define_exist( &rule->desc.socket.src_port, &src_port_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_port_define_exist( &rule->desc.socket.dest_port, &dest_port_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, src_ip_id.class_name, src_ip_id.id, src_port_id.class_name, src_port_id.id, dest_ip_id.class_name, dest_ip_id.id, dest_port_id.class_name, dest_port_id.id, rule->resp, rule->type, _T( "" ), flags ); 

_return:
	return ret; 
}

LRESULT _add_socket_rule( access_rule_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id src_ip_id; 
	param_define_id src_port_id; 
	param_define_id dest_ip_id; 
	param_define_id dest_port_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == SOCKET_RULE_TYPE ); 

	ret = _check_app_define_exist( ( param_all_desc* )&rule->desc.socket.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = _check_ip_define_exist( ( param_all_desc* )&rule->desc.socket.src_ip, &src_ip_id ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = _check_ip_define_exist( ( param_all_desc* )&rule->desc.socket.dest_ip, &dest_ip_id ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = _check_port_define_exist( ( param_all_desc* )&rule->desc.socket.src_port, &src_port_id ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = _check_port_define_exist( ( param_all_desc* )&rule->desc.socket.dest_port, &dest_port_id ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, src_ip_id.class_name, src_ip_id.id, src_port_id.class_name, src_port_id.id, dest_ip_id.class_name, dest_ip_id.id, dest_port_id.class_name, dest_port_id.id, rule->resp, rule->type, _T( "" ), flags ); 

	ASSERT( ret == ERROR_SUCCESS ); 

_return:
	return ret; 
}

LRESULT check_socket_rule( sys_action_desc *action, event_action_response *resp )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id src_ip_id; 
	param_define_id src_port_id; 
	param_define_id dest_ip_id; 
	param_define_id dest_port_id; 

	ASSERT( action != NULL ); 

	ASSERT( acl_type( action->type ) == SOCKET_RULE_TYPE ); 

	ret = __check_app_define_exist( ( param_all_desc* )&action->desc.socket.app, &app_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = __check_ip_define_exist( ( param_all_desc* )&action->desc.socket.src_ip, &src_ip_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = __check_ip_define_exist( ( param_all_desc* )&action->desc.socket.dest_ip, &dest_ip_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = __check_port_define_exist( ( param_all_desc* )&action->desc.socket.src_port, &src_port_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = __check_port_define_exist( ( param_all_desc* )&action->desc.socket.dest_port, &dest_port_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ASSERT( ret == ERROR_SUCCESS ); 

	ret = check_rule( app_id.class_name, 
		app_id.id, 
		src_ip_id.class_name, 
		src_ip_id.id, 
		src_port_id.class_name, 
		src_port_id.id, 
		dest_ip_id.class_name, 
		dest_ip_id.id, 
		dest_port_id.class_name, 
		dest_port_id.id, 
		MAX_RESPONSE_TYPE, 
		acl_type( action->type ), 
		NULL, 
		0 ); 

_return:
	return ret; 
}

LRESULT add_url_rule( access_rule_ptr_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id url_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == URL_RULE_TYPE ); 

	ret = check_url_define_exist( &rule->desc.url.url, &url_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_app_define_exist( &rule->desc.url.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, url_id.class_name, url_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T("" ), flags ); 
_return:
	return ret; 
}

LRESULT _add_url_rule( access_rule_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id url_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == URL_RULE_TYPE ); 

	init_param_id( &app_id ); 
	init_param_id( &url_id ); 

	ret = _check_url_define_exist( ( param_all_desc* )&rule->desc.url.url, &url_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = _check_app_define_exist( ( param_all_desc* )&rule->desc.url.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, url_id.class_name, url_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T( "" ), flags ); 
_return:
	return ret; 
}

LRESULT check_url_rule( sys_action_desc *action, event_action_response *resp )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id url_id; 

	ASSERT( action != NULL ); 

	ASSERT( acl_type( action->type ) == URL_RULE_TYPE ); 

	init_param_id( &app_id ); 
	init_param_id( &url_id ); 

	ret = __check_url_define_exist( ( param_all_desc* )&action->desc.url.url, &url_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = __check_app_define_exist( ( param_all_desc* )&action->desc.url.app, &app_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}
	
	ret = check_rule( app_id.class_name, 
		app_id.id, 
		url_id.class_name, 
		url_id.id, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, 
		INVALID_ID_VALUE, 
		MAX_RESPONSE_TYPE, 
		acl_type( action->type ), 
		NULL, 
		0 );  
_return:
	return ret; 
}

LRESULT add_file_rule( access_rule_ptr_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id file_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == FILE_RULE_TYPE ); 

	ret = check_file_define_exist( &rule->desc.file.file_path, &file_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_app_define_exist( &rule->desc.file.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, file_id.class_name, file_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T( "" ), flags ); 
_return:
	return ret; 
}

LRESULT _add_file_rule( access_rule_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id file_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == FILE_RULE_TYPE ); 

	ret = _check_file_define_exist( ( param_all_desc* )&rule->desc.file.file_path, &file_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = _check_app_define_exist( ( param_all_desc* )&rule->desc.file.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, file_id.class_name, file_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T("" ), flags ); 
_return:
	return ret; 
}

LRESULT check_file_rule( sys_action_desc *action, event_action_response *resp )
{
	LRESULT ret = ERROR_SUCCESS; 

	param_define_id app_id; 
	param_define_id file_id; 

	ASSERT( action != NULL ); 

	ASSERT( acl_type( action->type ) == FILE_RULE_TYPE ); 

	ret = __check_file_define_exist( ( param_all_desc* )&action->desc.file.file_path, &file_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = __check_app_define_exist( ( param_all_desc* )&action->desc.file.app, &app_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_rule( app_id.class_name, 
		app_id.id, 
		file_id.class_name, 
		file_id.id, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, 
		INVALID_ID_VALUE, 
		MAX_RESPONSE_TYPE, 
		acl_type( action->type ), 
		NULL, 
		0 ); 

_return:
	return ret;  
}

LRESULT add_reg_rule( access_rule_ptr_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id reg_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == REG_RULE_TYPE ); 

	ret = check_reg_define_exist( &rule->desc.reg.reg_path, &reg_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_app_define_exist( &rule->desc.reg.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, reg_id.class_name, reg_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T( "" ), flags ); 
_return:
	return ret; 
}

LRESULT _add_reg_rule( access_rule_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id reg_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == REG_RULE_TYPE ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ret = _check_reg_define_exist( ( param_all_desc* )&rule->desc.reg.reg_path, &reg_id ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "insert registry key path failed \n" ) ); 
		goto _return; 
	}

	ret = _check_app_define_exist( ( param_all_desc* )&rule->desc.reg.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "insert application path failed \n" ) ); 
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, reg_id.class_name, reg_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T( "" ), flags ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "insert rule failed\n" ) ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT check_reg_rule( sys_action_desc *action, event_action_response *resp )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id reg_id; 

	ASSERT( action != NULL ); 

	ASSERT( acl_type( action->type ) == REG_RULE_TYPE ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ret = __check_reg_define_exist( ( param_all_desc* )&action->desc.reg.reg_path, &reg_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "insert registry key path failed \n" ) ); 
		goto _return; 
	}

	ret = __check_app_define_exist( ( param_all_desc* )&action->desc.reg.app, &app_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "insert application path failed \n" ) ); 
		goto _return; 
	}

	ret = check_rule( app_id.class_name, 
		app_id.id, 
		reg_id.class_name, 
		reg_id.id, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, 
		INVALID_ID_VALUE, 
		MAX_RESPONSE_TYPE, 
		acl_type( action->type ), 
		NULL, 
		0 ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "insert rule failed\n" ) ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT add_com_rule( access_rule_ptr_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id com_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == COM_RULE_TYPE ); 

	ret = check_com_define_exist( &rule->desc.com.com, &com_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_app_define_exist( &rule->desc.reg.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, com_id.class_name, com_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T( "" ), flags );
_return:
	return ret; 
}

LRESULT _add_com_rule( access_rule_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id com_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == COM_RULE_TYPE ); 

	ret = _check_com_define_exist( ( param_all_desc* )&rule->desc.com.com, &com_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = _check_app_define_exist( ( param_all_desc* )&rule->desc.reg.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, com_id.class_name, com_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T( "" ), flags );
_return:
	return ret; 
}

LRESULT check_com_rule( sys_action_desc *action, event_action_response *resp )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id com_id; 

	ASSERT( action != NULL ); 

	ASSERT( acl_type( action->type ) == COM_RULE_TYPE ); 

	ret = __check_com_define_exist( ( param_all_desc* )&action->desc.com.com, &com_id, 0); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = __check_app_define_exist( ( param_all_desc* )&action->desc.reg.app, &app_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_rule( app_id.class_name, 
		app_id.id, 
		com_id.class_name, 
		com_id.id, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, 
		INVALID_ID_VALUE, 
		MAX_RESPONSE_TYPE, 
		acl_type( action->type ), 
		NULL, 
		0 );

_return:
	return ret; 
}

LRESULT _add_common_rule( access_rule_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id param_id; 

	ASSERT( rule != NULL ); 

	ASSERT( rule->type == COMMON_RULE_TYPE ); 

	ASSERT( is_valid_action_type( flags ) == TRUE ); 

	ret = _check_app_define_exist( ( param_all_desc* )&rule->desc.reg.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = _check_common_define_exist( ( param_all_desc* )&rule->desc.common.param0, &param_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = add_rule( app_id.class_name, app_id.id, param_id.class_name, param_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T( "" ), flags );

_return:
	return ret; 
}

LRESULT add_common_rule( access_rule_ptr_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id param_id; 

	ASSERT( rule != NULL ); 
	ASSERT( is_valid_common_rule_flag( flags ) == TRUE ); 
	ASSERT( rule->type == COMMON_RULE_TYPE ); 

	ASSERT( is_valid_action_type( flags ) == TRUE ); 

	ret = check_common_define_exist( &rule->desc.common.param0, &param_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_app_define_exist( &rule->desc.common.app, &app_id ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ASSERT( app_id.id != INVALID_ID_VALUE ); 

	ret = add_rule( app_id.class_name, app_id.id, NULL, param_id.id, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, NULL, INVALID_ID_VALUE, rule->resp, rule->type, _T( "" ), flags );

_return:
	return ret; 
}

LRESULT check_common_rule( sys_action_desc *action, event_action_response *resp )
{	
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id app_id; 
	param_define_id param_id; 

	ASSERT( action != NULL ); 
	//ASSERT( is_valid_common_rule_flag( flags ) == TRUE ); 
	ASSERT( acl_type( action->type ) == COMMON_RULE_TYPE ); 

	ASSERT( is_valid_action_type( action->type ) == TRUE ); 

	ret = __check_common_define_exist( ( param_all_desc* )&action->desc.common.param0, &param_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = __check_app_define_exist( ( param_all_desc* )&action->desc.common.app, &app_id, 0 ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_rule( app_id.class_name, 
		app_id.id, 
		NULL, 
		param_id.id, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, 
		INVALID_ID_VALUE, 
		NULL, INVALID_ID_VALUE, 
		MAX_RESPONSE_TYPE, 
		acl_type( action->type ), 
		NULL, 
		0 ); 

_return:
	return ret; 
}

LRESULT del_param( param_all_desc *param, param_define_id *param_id )
{
	LRESULT ret = ERROR_SUCCESS; 
	sql_stmt_desc *sql_stmt = NULL; 

	ASSERT( param != NULL ); 
	ASSERT( param_id != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	if( _check_param_desc_valid( param ) != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "input invalid param" ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( param->is_cls == TRUE )
	{
		ret = check_class_exist( param->type, param_define_db, param->cls.class_name ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		*param_id->class_name = L'\0'; 
		param->id = INVALID_ID_VALUE; 
		goto _return; 
	}
	else
	{
		ret = _get_param_define_id( param, 
			param_id ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		ASSERT( param_id->id != INVALID_ID_VALUE ); 
		*param_id->class_name = L'\0'; 
	}

_return:
	if( sql_stmt != NULL )
	{
		release_sql_stmt_desc( sql_stmt ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT del_rule_from_config( access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 
	param_define_id param_id[ 5 ]; 
	ULONG param_num; 
	INT32 i; 

	ASSERT( rule != NULL ); 

	init_access_desc_param_ptr( rule->type, &rule->desc ); 

	param_num = get_access_rule_param_count( rule->type ); 

	for( i = 0; i < ARRAY_SIZE( param_id ); i ++ )
	{
		param_id[ i ].id = INVALID_ID_VALUE; 
		*param_id[ i ].class_name = _T( '\0' ); 
	}

	for( i = 0; ( ULONG )i < param_num; i ++ )
	{
		ret = del_param( rule->desc.params[ i ], &param_id[ i ] ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

	ret = del_rule_from_db( 
		param_id[ 0 ].class_name, 
		param_id[ 0 ].id, 
		param_id[ 1 ].class_name, 
		param_id[ 1 ].id, 
		param_id[ 2 ].class_name, 
		param_id[ 2 ].id, 
		param_id[ 3 ].class_name, 
		param_id[ 3 ].id, 
		param_id[ 4 ].class_name, 
		param_id[ 4 ].id, 
		rule->resp, 
		rule->type, 
		_T( "" ), 
		0 );

_return:
	return ret; 
}

LRESULT input_rule( access_rule_ptr_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 

	ASSERT( check_access_rule_ptr_input_valid( rule, FALSE ) == ERROR_SUCCESS ); 

	if( rule->type == URL_RULE_TYPE )
	{
		ret = add_url_rule( rule, flags ); 
	}
	else if( rule->type == SOCKET_RULE_TYPE )
	{
		ret = add_socket_rule( rule, flags ); 
	}
	else if( rule->type == FILE_RULE_TYPE )
	{
		ret = add_file_rule( rule, flags ); 
	}
	else if( rule->type == REG_RULE_TYPE )
	{
		ret = add_reg_rule( rule, flags ); 
	}
	else if( rule->type == COM_RULE_TYPE )
	{
		ret = add_com_rule( rule, flags ); 
	}
	else if( rule->type == COMMON_RULE_TYPE )
	{
		ret = add_common_rule( rule, flags ); 
	}

_return:
	return ret; 
}

#ifdef _DEBUG
BOOLEAN debugging_action = FALSE; 
#endif //_DEBUG

#include "action_type.h"

action_response_type default_response = ACTION_ALLOW; 

/*
find rule for one action, process design:
1.one rule type for one action type, rule type will be more,maintain will be complex.
2.one rule type for some action types
3.don't use rule, use other data structure for acl checking.
*/

#include "action_check.h"

LRESULT input_r3_rule_from_action( sys_action_desc *sys_action, ULONG flags )
{
#if 0 
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	NTSTATUS ntstatus; 

	access_rule_desc *rule_input = NULL; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	rule_input = ( access_rule_desc* )malloc( sizeof( access_rule_desc ) ); 
	if( rule_input == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	rule_input->resp = sys_action->resp; 
	rule_input->type = acl_type( sys_action->type ); 

	ret = init_access_rule( rule_input->type, rule_input ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = copy_params( rule_input->type, &sys_action->desc, &rule_input->desc ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	_ret = input_rule_to_config( rule_input, flags ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "input the action rule to database of config error 0x%0.8x\n", ntstatus ) ); 
		ret = _ret; 
		goto _return; 
	}

_return:

	if( rule_input != NULL )
	{
		free( rule_input );  
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
#endif //0
	return ERROR_NOT_SUPPORTED; 
}


LRESULT input_rule_to_config( access_rule_desc *rule, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 

	ASSERT( is_valid_access_rule_type( rule->type ) ); 

	if( rule->type == URL_RULE_TYPE )
	{
		ret = _add_url_rule( rule, flags ); 
	}
	else if( rule->type == SOCKET_RULE_TYPE )
	{
		ret = _add_socket_rule( rule, flags ); 
	}
	else if( rule->type == FILE_RULE_TYPE )
	{
		ret = _add_file_rule( rule, flags ); 
	}
	else if( rule->type == REG_RULE_TYPE )
	{
		ret = _add_reg_rule( rule,flags ); 
	}
	else if( rule->type == COM_RULE_TYPE )
	{
		ret = _add_com_rule( rule, flags ); 
	}
	else if( rule->type == COMMON_RULE_TYPE )
	{
		ret = _add_common_rule( rule, flags ); 
	}

_return:
	return ret; 
}

VOID release_action_rule( access_rule_desc *rule )
{
	ASSERT( rule != NULL ); 
	free( rule ); 
}

#ifdef _DEBUG_DRIVER
LRESULT test_trace_log()
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	access_rule_desc rule_input; 
	action_rule_define rule_define; 
	sys_action_desc cur_action_desc; 
	action_response_type response; 
	data_trace_context trace_option = { 0 }; 

	cur_action_desc.id = 0; 
	cur_action_desc.resp = ACTION_ALLOW; 
	cur_action_desc.type = INSTALL_DRV; 
	cur_action_desc.desc.file.app.is_cls = FALSE; 
	*cur_action_desc.desc.file.app.app.app_name = L'\0'; 
	wcscpy( cur_action_desc.desc.file.file_path.file.file_path, L"C:\\testfile_txt" ); 

	ntstatus = init_action_manage(); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

#define TEST_FILE_NAME L"C:\\test_file.txt"
	rule_input.resp = ACTION_ALLOW; 
	//*rule_input.addition = L'\0'; 
	rule_input.type = FILE_RULE_TYPE; 
	rule_input.desc.file.app.type = APP_DEFINE; 
	rule_input.desc.file.app.is_cls = FALSE; 
	*rule_input.desc.file.app.app.app_name = L'\0'; 
	rule_input.desc.file.file_path.is_cls = FALSE; 
	rule_input.desc.file.file_path.type = FILE_DEFINE; 

	memcpy( rule_input.desc.file.file_path.file.file_path, TEST_FILE_NAME, sizeof( TEST_FILE_NAME ) ); 
	ntstatus = add_action_rule( &rule_input, MODIFY_RULE ); 
	ntstatus = find_defines_of_rule( &rule_define, &rule_input ); 
	ntstatus = check_sys_acl( &cur_action_desc, &trace_option, &response ); 

	_del_action_rule( &rule_input ); 
	ntstatus = uninit_action_manage(); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

_return:
	return ret; 
}
#endif //_DEBUG_DRIVER

INLINE LRESULT load_params_of_url_rule( access_rule_type rule_type, param_define_type param_type, param_record *param, ULONG index, action_rule_record *record )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR *url; 

	ASSERT( record != NULL ); 
	ASSERT( param != NULL ); 
	ASSERT( rule_type == URL_RULE_TYPE ); 

	log_trace( ( MSG_INFO, "enter %s config url rule \n", __FUNCTION__ ) ); 

#define NAME_PARAM_INDEX 1
	if( index != NAME_PARAM_INDEX )
	{
		goto _return; 
	}

	ASSERT( param_type == URL_DEFINE ); 

	if( *param->url.url == '\0' )
	{
		log_trace( ( MSG_INFO, "url length is 0\n" ) ); 
		goto _return; 
	}

	url = ( WCHAR* )param->url.url; 

_return:
	log_trace( ( MSG_INFO, "leave %s config url rule \n", __FUNCTION__ ) ); 

	return ret; 
}

sys_action_desc socket_action_debug; 

LRESULT init_debug_actions()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		memcpy( &socket_action_debug, 0, sizeof( socket_action_debug ) ); 
	}while( FALSE );

	return ret; 
}

LRESULT set_socket_debug_info( LPCSTR ip, ULONG port )
{
	ULONG _ip = inet_addr( ( CHAR* )ip ); 

	return _set_socket_debug_info( _ip, port ); 
}

LRESULT _set_socket_debug_info( ULONG ip, ULONG port )
{
	LRESULT ret = ERROR_SUCCESS; 

	if( ip != NULL )
	{
		socket_action_debug.desc.socket.dest_ip.ip.ip_begin = ip; 
	}
	
	if( port != 0 )
	{
		socket_action_debug.desc.socket.dest_port.port.port_begin = port; 
	}

	return ret; 
}

LRESULT check_socket_debug_info( ULONG ip, ULONG port )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( socket_action_debug.desc.socket.dest_ip.ip.ip_begin != 0 )
		{
			if( socket_action_debug.desc.socket.dest_ip.ip.ip_begin != ip )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}
		else
		{
			ret = ERROR_INVALID_PARAMETER; 
		}

		if( socket_action_debug.desc.socket.dest_port.port.port_begin != 0 )
		{
			if( socket_action_debug.desc.socket.dest_port.port.port_begin != port )
			{
				ret = ERROR_INVALID_PARAMETER_2; 
				break; 
			}
		}
		else
		{
			ret = ERROR_INVALID_PARAMETER_2; 
		}
	}while( FALSE );

	if( ret == ERROR_SUCCESS )
	{
		DBG_BP(); 
	}

	return ret; 
}

LRESULT debug_sys_action( sys_action_desc *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}
