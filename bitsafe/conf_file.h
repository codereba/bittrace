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
 
 #pragma once

#include "acl_define.h"
#include <time.h>

#define PARAM_DEFINE_DB_NAME _T( "data0.dat" )
#define FW_LOG_DB_NAME _T( "data1.dat" )
#define DEFENSE_LOG_DB_NAME _T( "data2.dat" )
#define ACTION_RULE_DB_NAME _T( "data3.dat" )

#define SEVENT_CONF_DB_NAME _T( "data10.dat" )

#define TEST_REG_RULE 0x00000001
#define TEST_SOCKET_RULE 0x00000002
#define TEST_COMMON_DRIVER_RULE 0x00000004

extern LPCTSTR flag_descs[]; 

//typedef enum _RULE_TYPE
//{
//	FW_APP_NAME_RULE, 
//	FW_APP_CLS_NAME_RULE, 
//	FW_URL_NAME_RULE, 
//	FW_URL_CLS_NAME_RULE, 
//	FW_IP_REGION_RULE, 
//	FW_IP_REGION_CLS_RULE, 
//
//	DEFENSE_APP_NAME_RULE, 
//	DEFENSE_APP_CLS_NAME_RULE, 
//	DEFENSE_COM_NAME_RULE, 
//	DEFENSE_COM_CLS_NAME_RULE, 
//	DEFENSE_FILE_NAME_RULE, 
//	DEFENSE_FILE_CLS_NAME_RULE, 
//	DEFENSE_REG_NAME_RULE, 
//	DEFENSE_REG_CLS_NAME_RULE
//} RULE_TYPE; 

typedef struct _general_conf
{
	TCHAR *val_name; 
	TCHAR *val_data; 
} general_conf, *pgeneral_conf; 

typedef struct _url_param_record
{
	LPCWSTR url; 
	ULONG id; 
	LPCWSTR class_name; 
	LPCWSTR desc; 
	ULONG flag; 
} url_param_record, *purl_param_record; 

typedef struct _ip_param_record
{
	ULONG ip_begin; 
	ULONG ip_end; 
	ULONG id; 
	LPCWSTR class_name; 
	LPCWSTR desc; 
	ULONG flag; 
} ip_param_record, *pip_param_record; 

typedef struct _port_param_record
{
	ULONG port_begin; 
	ULONG port_end; 
	ULONG id; 
	LPCWSTR class_name; 
	prot_type type; 
	LPCWSTR desc; 
	ULONG flag; 
} port_param_record, *pport_param_record; 

typedef struct _app_param_record
{
	LPCWSTR app_name; 
	ULONG id; 
	LPCWSTR class_name; 
	LPCWSTR desc; 
	ULONG flag; 
} app_param_record, *papp_param_record; 

typedef struct _common_param_record
{
	LPCWSTR name; 
	ULONG id; 
	LPCWSTR class_name; 
	LPCWSTR desc; 
	ULONG flag; 
} common_param_record, *pcommon_param_record; 

typedef struct _file_param_record
{
	LPCWSTR file_path; 
	LPCWSTR class_name; 
	ULONG id; 
	LPCWSTR desc; 
	ULONG flag; 
} file_param_record, *pfile_param_record; 

typedef struct _com_param_record
{
	LPCWSTR com_name; 
	LPCWSTR class_name; 
	ULONG id; 
	LPCWSTR desc; 
	ULONG flag; 
} com_param_record, *pcom_param_record; 

typedef struct _reg_param_record
{
	LPCWSTR reg_path; 
	LPCWSTR class_name; 
	ULONG id; 
	LPCWSTR desc; 
	ULONG flag; 
} reg_param_record, *preg_param_record; 

typedef union _param_record
{
	app_param_record app; 
	reg_param_record reg; 
	file_param_record file; 
	com_param_record com; 
	ip_param_record ip; 
	url_param_record url; 
	port_param_record port; 
	common_param_record common; 
} param_record, *pparam_record; 

typedef struct _action_rule_record
{
	LPCWSTR class_names[ MAX_RULE_PARAM_NUM ];  
	ULONG ids[ MAX_RULE_PARAM_NUM ]; 
	ULONG action; 
	ULONG type; 
	LPCWSTR desc; 
	ULONG flag; 
} action_rule_record, *paction_rule_record; 

typedef struct _fw_log
{
	ULONG id; 
	WCHAR proc_name[ _MAX_FILE_NAME_LEN ]; 
	action_response_type action; 
	sys_action_type type; 
	prot_type prot; 
	ULONG src_ip; 
	ULONG dest_ip; 
	ULONG dest_port; 
	ULONG src_port; 
	WCHAR desc[ _MAX_DESC_LEN ]; 
	time_t time; 
} fw_log, *pfw_log; 

typedef struct _defense_log
{
	ULONG id; 
	WCHAR proc_name[ _MAX_FILE_NAME_LEN ]; 
	sys_action_type flag; 
	action_response_type action; 
	WCHAR target_desc[ _MAX_TEXT_LEN ]; 
	time_t time; 
} defense_log, *pdefense_log; 

typedef struct _param_define_id
{
	ULONG id; 
	WCHAR class_name[ _MAX_CLASS_NAME_LEN ]; 
} param_define_id, *pparam_define_id; 

typedef enum _sql_action
{
	CREATE_TABLE, 
	DROP_TABLE, 
	ALTER_TABLE, 
	INSERT_ROW, 
	DELETE_ROW, 
	ALTER_ROW, 
	SELECT_ROW, 
	SQL_ACTION_BEGIN = INSERT_ROW, 
	SQL_ACTION_END = SELECT_ROW, 
	MAX_SQL_ACITON
} sql_action, *psql_action; 

#define DELETE_CMD L"DELETE FROM "
#define SELECT_CMD L"SELECT "
#define INSERT_CMD L"INSERT INTO "
#define UPDATE_CMD L"UPDATE "

#define INSERT_CMD_LEN CONST_STR_LEN( INSERT_CMD )
#define DELETE_CMD_LEN CONST_STR_LEN( DELETE_CMD )
#define SELECT_CMD_LEN CONST_STR_LEN( SELECT_CMD )
#define UPDATE_CMD_LEN CONST_STR_LEN( UPDATE_CMD )

#define is_valid_sql_action( action ) ( action >= SQL_ACTION_BEGIN && action <= SQL_ACTION_END )

#define MAX_TABLE_NAME_LEN 128
#define MAX_CLOUMN_NAME_LEN 128
#define MAX_COLUMN_VAL_LEN _MAX_REG_PATH_LEN
#define MAX_COLUMN_NUM 12

typedef enum _column_type
{
	NUMBER_TYPE, 
	STRING_TYPE, 
	TIME_TYPE, 
	COLUMN_TYPE_BEIGN = NUMBER_TYPE, 
	COLUMN_TYPE_END = TIME_TYPE, 
	MAX_COLUMN_TYPE 
} column_type; 

#define is_valid_column_type( type ) ( type >= COLUMN_TYPE_BEIGN && type <= COLUMN_TYPE_END )

#define _MAX_COLUMN_NAME_LEN 80

typedef struct _column_name_value
{
	column_type type; 
	WCHAR name[ _MAX_COLUMN_NAME_LEN ]; 
	WCHAR val[ MAX_COLUMN_VAL_LEN ]; 
} column_name_value, *pcolumn_name_value; 

typedef struct _sql_stmt_desc
{
	sql_action action; 
	ULONG col_val_num; 
	ULONG sel_col_num; 
	WCHAR table_name[ MAX_TABLE_NAME_LEN ]; 
	column_name_value name_val[ MAX_COLUMN_NUM ]; 
	WCHAR sel_cloumn_name[ MAX_COLUMN_NUM ][ MAX_CLOUMN_NAME_LEN ]; 
	LPWSTR sql; 

} sql_stmt_desc, *psql_stmt_desc; 

typedef struct _access_rule_store
{
	access_rule_desc rule; 
	TCHAR desc[ _MAX_DESC_LEN ]; 
} access_rule_store, *paccess_rule_store; 

#define SUPER_RULE_MASK 0x80000000 
#define is_super_rule( flag ) ( !!( flag & SUPER_RULE_MASK ) )

typedef struct _action_rule_buffered
{
#ifndef STL_RULE_BUFFER
	OFFSET_LIST_ENTRY entry; 
#endif //STL_RULE_BUFFER

	access_rule_desc rule; 
	WCHAR desc[ _MAX_DESC_LEN ]; 
	ULONG flags; 
} action_rule_buffered, *paction_rule_buffered; 

typedef INT32 ( CALLBACK *read_param_record_callback )( param_define_type type, param_record *record, PVOID param ); 
typedef LRESULT ( CALLBACK *read_action_rule_callback)( access_rule_type type, action_rule_record *record, PVOID param ); 
typedef INT32 ( CALLBACK *fw_log_callback)( pfw_log log, PVOID param ); 
typedef INT32 ( CALLBACK *defense_log_callback )( pdefense_log log, PVOID param ); 
typedef LRESULT ( CALLBACK *drv_state_callback )( bitsafe_function_type type, bitsafe_function_state state, PVOID param ); 
typedef LRESULT ( CALLBACK *general_conf_callback )( LPCTSTR val_name, PVOID val_data, PVOID param ); 

typedef LRESULT ( CALLBACK *defines_of_rule_callback )( access_rule_type rule_type, param_define_type param_type, param_record *param, ULONG index, action_rule_record *record ); 

typedef LRESULT ( CALLBACK *alloc_record_callback )( PVOID *record, PVOID param ); 

LRESULT set_col_val( sql_stmt_desc *sql_stmt, ULONG index, LPCWSTR col_name, LPCWSTR col_val ); 
LRESULT add_col_val( sql_stmt_desc *sql_stmt, LPCWSTR col_name, LPCWSTR col_val ); 
LRESULT alloc_sql_stmt_desc( sql_action action, LPCWSTR table_name, sql_stmt_desc **sql_stmt ); 
LRESULT set_sel_col( sql_stmt_desc *sql_stmt, ULONG index, LPCWSTR col_name ); 
LRESULT add_sel_col( sql_stmt_desc *sql_stmt, LPCWSTR col_name ); 
LRESULT add_col_val_int( sql_stmt_desc *sql_stmt, LPCWSTR col_name, ULONG col_val ); 
LRESULT generate_sql_stmt( sql_stmt_desc *sql_stmt );VOID release_sql_stmt_desc( sql_stmt_desc *sql_stmt ); 

typedef enum _RULE_CTRL_MODE
{
	APP_ACTION_RULE, 
	APP_ACTION_TYPE_RULE, 
	APP_DEFINE_RULE, 
	MAX_RULE_CTRL_MODE
}RULE_CTRL_MODE, *PRULE_CTRL_MODE; 

#define is_valid_common_rule_flag( flags ) ( flags >= APP_ACTION_RULE || flags <= APP_DEFINE_RULE )

LRESULT input_rule( access_rule_ptr_desc *rule, ULONG flags ); 
LRESULT input_rule_to_config( access_rule_desc *rule, ULONG flags ); 
LRESULT del_rule_from_config( access_rule_desc *rule ); 
LRESULT get_defense_log( defense_log_callback output_func, alloc_record_callback alloc_func, PVOID param ); 
LRESULT get_rule( access_rule_type type, read_action_rule_callback output_func, alloc_record_callback alloc_func, PVOID param ); 
LRESULT get_fw_log( fw_log_callback output_func, alloc_record_callback alloc_func, PVOID param ); 

LRESULT add_fw_log( pfw_log log ); 
LRESULT add_defense_log( pdefense_log log );

INLINE LRESULT get_fw_rule( read_action_rule_callback output_func, alloc_record_callback alloc_func, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	_ret = get_rule( URL_RULE_TYPE, output_func, alloc_func, param ); 
	if( _ret != ERROR_SUCCESS )
	{
		ret = _ret; 
	}

	_ret = get_rule( SOCKET_RULE_TYPE, output_func, alloc_func, param ); 
	if( _ret != ERROR_SUCCESS )
	{
		ret = _ret; 
	}

//_return:
	return ret; 
}

INLINE LRESULT get_defense_rule( read_action_rule_callback output_func, alloc_record_callback alloc_func, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	_ret = get_rule( COM_RULE_TYPE, output_func, alloc_func, param ); 
	if( _ret != ERROR_SUCCESS )
	{
		ret = _ret; 
	}

	_ret = get_rule( FILE_RULE_TYPE, output_func, alloc_func, param ); 
	if( _ret != ERROR_SUCCESS )
	{
		ret = _ret; 
	}

	_ret = get_rule( REG_RULE_TYPE, output_func, alloc_func, param ); 
	if( _ret != ERROR_SUCCESS )
	{
		ret = _ret; 
	}

//_return:
	return ret; 
}

//INLINE LRESULT _check_action_rule_valid( access_rule_ptr_desc *rule_input )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	ULONG param_num; 
//	param_define_type param_type; 
//	INT32 i; 
//
//	ASSERT( rule_input != NULL ); 
//
//	ASSERT( is_valid_access_rule_type( rule_input->type ) ); 
//	ASSERT( is_valid_response_type( rule_input->resp ) ); 
//
//	//if( wcslen( rule_input->addition ) > _MAX_DESC_LEN )
//	//{
//	//	ret = ERROR_BUFFER_OVERFLOW; 
//	//	goto _return; 
//	//}
//
//	param_num = get_access_rule_param_count( rule_input->type ); 
//
//	for( i = 0; i < param_num; i ++ )
//	{
//		param_type = get_rule_param_type( rule_input->type, i ); 
//		if( param_type == INVALID_PARAM_DEFINE_TYPE )
//		{
//			ASSERT( FALSE ); 
//			break; 
//		}
//
//		ret = _check_param_valid( rule_input->type, param_type, ( param_all_desc* )&rule_input->desc.params[ i ] ); 
//		if( ret != ERROR_SUCCESS )
//		{
//			goto _return; 
//		}
//	}
//_return:
//	return ret; 
//}

INLINE LRESULT copy_rule_input_param( access_rule_desc *dest_rule, access_rule_ptr_desc *input_param )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG param_num; 
	param_define_type type; 
	INT32 i; 

	ASSERT( input_param != NULL ); 
	ASSERT( dest_rule != NULL ); 

	ASSERT( check_access_rule_ptr_input_valid( input_param, FALSE ) == ERROR_SUCCESS ); 

	//wcscpy( input->addition, input_param->addition ); 
	dest_rule->resp = input_param->resp; 
	dest_rule->type = input_param->type; 

	param_num = get_access_rule_param_count( input_param->type ); 
	for( i = 0; ( ULONG )i < param_num; i ++ )
	{
		type = dest_rule->desc.params[ i ]->type; 

		if( type == IP_DEFINE )
		{
			dest_rule->desc.params[ i ]->ip.ip_begin = input_param->desc.params[ i ].ip.ip_begin; 
			dest_rule->desc.params[ i ]->ip.ip_end = input_param->desc.params[ i ].ip.ip_end; 
		}
		else if( type == PORT_DEFINE )
		{
			dest_rule->desc.params[ i ]->port.port_begin = input_param->desc.params[ i ].port.port_begin; 
			dest_rule->desc.params[ i ]->port.port_end = input_param->desc.params[ i ].port.port_end; 
			dest_rule->desc.params[ i ]->port.type = input_param->desc.params[ i ].port.type; 
		}
		else
		{
			if( input_param->desc.params[ i ].common.name != NULL )
			{
				StringCbCopyW( dest_rule->desc.params[ i ]->common.name, 
					sizeof( dest_rule->desc.params[ i ]->common.name ), 
					input_param->desc.params[ i ].common.name ); 
			}
			else
			{
				*dest_rule->desc.params[ i ]->common.name = L'\0'; 
			}
		}
	}
	
//_return:
	return ret; 
}

INLINE VOID _set_define_type_in_rule( access_rule_ptr_desc *rule_input )
{
	ASSERT( rule_input != NULL ); 

	switch ( rule_input->type )
	{
	case URL_RULE_TYPE:
		rule_input->desc.params[ 0 ].type = APP_DEFINE; 
		rule_input->desc.params[ 1 ].type = URL_DEFINE; 

		break; 
	case SOCKET_RULE_TYPE:
		rule_input->desc.params[ 0 ].type = APP_DEFINE; 
		rule_input->desc.params[ 1 ].type = IP_DEFINE; 
		rule_input->desc.params[ 2 ].type = PORT_DEFINE; 
		rule_input->desc.params[ 3 ].type = IP_DEFINE; 
		rule_input->desc.params[ 4 ].type = PORT_DEFINE; 
		break; 
	case FILE_RULE_TYPE:
		rule_input->desc.params[ 0 ].type = APP_DEFINE; 
		rule_input->desc.params[ 1 ].type = FILE_DEFINE; 
		break; 
	case REG_RULE_TYPE:
		rule_input->desc.params[ 0 ].type = APP_DEFINE; 
		rule_input->desc.params[ 1 ].type = REG_DEFINE; 
		break; 
	case COM_RULE_TYPE:
		rule_input->desc.params[ 0 ].type = APP_DEFINE; 
		rule_input->desc.params[ 1 ].type = COM_DEFINE; 
		break; 
	case COMMON_RULE_TYPE: 
		rule_input->desc.params[ 0 ].type = APP_DEFINE; 
		rule_input->desc.params[ 1 ].type = COMMON_DEFINE; 
		break; 
	}
}


//extern drv_state all_drv_open_state[ MAX_DRIVER_TYPE ]; 
//extern CRITICAL_SECTION drv_open_state_lock; 
LRESULT config_general( TCHAR* val_name, TCHAR *val_data ); 

LRESULT _load_driver_conf( read_param_record_callback callback_func, 
						 alloc_record_callback alloc_func, 
						 PVOID param ); 

#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus

	LRESULT init_bitsafe_function_state(); 
	LRESULT uninit_bitsafe_function_state(); 
	LRESULT config_bitsafe_function_state(  bitsafe_function_type type, bitsafe_function_state load_state ); 
	LRESULT check_bitsafe_function_state( bitsafe_function_type type, bitsafe_function_state *load_state ); 
#ifdef __cplusplus
}
#endif //__cplusplus

LRESULT config_all_driver(); 

LRESULT load_general_conf( read_param_record_callback callback_func, 
						  alloc_record_callback alloc_func, 
						  PVOID param ); 

LRESULT init_name_access_rule_ptr_desc( access_rule_ptr_desc *rule_input, action_response_type resp, access_rule_type type, LPCWSTR app_name, LPCWSTR name_param ); 
LRESULT input_rule_from_conf_file( LPCTSTR file_name ); 

LRESULT open_bitsafe_conf(); 
LRESULT open_function_conf(); 
LRESULT test_sqlite3_ext_stack_overflow(); 

LRESULT close_bitsafe_conf(); 
LRESULT close_function_conf(); 

LRESULT generate_test_action( sys_action_desc *cur_action, ULONG param, ULONG mode ); 

LRESULT load_rule_from_record( access_rule_type type, 
							  action_rule_record *record, 
							  access_rule_desc * rule_input ); 

LRESULT init_access_rule_ptr_desc( access_rule_ptr_desc *rule_input ); 

INLINE VOID dump_action_rule( access_rule_desc *action_rule )
{
	ULONG src_ip; 
	ULONG dest_ip; 
	PBYTE src_ip_byte; 
	PBYTE dest_ip_byte; 

	ASSERT( action_rule != NULL ); 

	log_trace( ( MSG_INFO, "dump action rule:\n" ) ); 

	if( is_name_rule_type( action_rule->type ) == TRUE )
	{
		log_trace( ( MSG_INFO, "name is %s \n", 
			action_rule->desc.params[ 1 ]->common.name ) ); 
	}
	else if( is_socket_rule_type( action_rule->type ) == TRUE )
	{
		src_ip = ( ULONG )action_rule->desc.socket.src_ip.ip.ip_begin; 
		dest_ip = ( ULONG )action_rule->desc.socket.dest_ip.ip.ip_begin; 

		src_ip_byte = ( PBYTE )&src_ip; 
		dest_ip_byte =  ( PBYTE )&dest_ip; 

		log_trace( ( MSG_INFO, "source ip is %d.%d.%d.%d port is %d, destination ip is %d.%d.%d.%d port is %d\n", 
			src_ip_byte[ 3 ], 
			src_ip_byte[ 2 ], 
			src_ip_byte[ 1 ], 
			src_ip_byte[ 0 ], 
			action_rule->desc.socket.src_port, 
			dest_ip_byte[ 3 ], 
			dest_ip_byte[ 2 ], 
			dest_ip_byte[ 1 ], 
			dest_ip_byte[ 0 ], 
			action_rule->desc.socket.dest_port 
			) ); 
	}

	log_trace( ( MSG_INFO, "dump action rule end\n" ) ); 
}

#ifdef _DEBUG


LRESULT add_test_url_rule(); 

INT32 test_conf_function( ULONG mode ); 
LRESULT stop_test_conf_function(); 
#endif //_DEBUG

#define FW_LOG_OP 0x00000001
#define DEFENSE_LOG_OP 0x00000002
#define MAX_LOG_ROW_COUNT 35000
#define DEL_LOG_NUM_ONCE 10000

#define CHECK_DB_OVERFLOW_TIME 1
#define CHECK_LOG_DB_OVERFLOW_TIME 10 

LRESULT truncate_log_db( ULONG flags ); 
LRESULT truncate_db( PVOID target_db, LPCTSTR table_name, ULONG max_num, ULONG del_once ); 

LRESULT action_action_log( pfw_log log ); 

INLINE LRESULT add_actions_log( sys_action_record *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

INLINE LRESULT add_log( sys_action_desc *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	fw_log socket_log; 
	defense_log name_log; 
	access_rule_type rule_type; 

	*socket_log.desc = L'\0'; 
	//*name_log.target_desc = L'\0'; 

	rule_type = acl_type( action->type ); 
	if( rule_type == SOCKET_RULE_TYPE )
	{
		socket_log.action = action->resp; 
		//wcsncpy( socket_log.desc, action->addition, _MAX_DESC_LEN ); 
		StringCbCopyW( socket_log.proc_name, 
			sizeof( socket_log.proc_name ), 
			action->desc.socket.app.app.app_name ); 

		socket_log.dest_port = action->desc.socket.dest_port.port.port_begin; 
		socket_log.src_port = action->desc.socket.src_port.port.port_begin; 
		socket_log.src_ip = action->desc.socket.src_ip.ip.ip_begin; 
		socket_log.dest_ip = action->desc.socket.dest_ip.ip.ip_begin; 
		socket_log.prot = action->desc.socket.src_port.port.type; 
		time( &socket_log.time ); 

		ret = add_fw_log( &socket_log ); 
	}
	else
	{
		name_log.flag = action->type; 
		name_log.action = action->resp; 
		ASSERT( wcsnlen( action->desc.common.app.app.app_name, _MAX_FILE_NAME_LEN ) < _MAX_FILE_NAME_LEN ); 

		StringCbCopyW( name_log.proc_name, 
			sizeof( name_log.proc_name ), 
			action->desc.common.app.app.app_name ); 
		
		time( &name_log.time ); //action->time; 
		
		StringCbCopyW( name_log.target_desc, 
			sizeof( name_log.target_desc ), 
			action->desc.common.param0.common.name ); 

		ret = add_defense_log( &name_log ); 
	}

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	return ret; 
}

INLINE LPCWSTR get_protocol_text( prot_type type )
{
	LPCWSTR desc = _T( "" ); 

	switch( type )
	{
	case PROT_TCP: 
		desc = _get_string_by_id( TEXT_IP_EDIT_PROT_TCP, 
			_T( "TCP协议" ) ); 
		break; 
	case PROT_UDP:
		desc = _get_string_by_id( TEXT_IP_EDIT_PROT_UDP, 
			_T( "UDP协议" ) ); 
		break; 
	case ALL_PROT:
		desc = _get_string_by_id( TEXT_IP_EDIT_PROT_ALL, 
			_T( "*" ) ); 
		break; 
	}

	return desc; 
}

INLINE LPCWSTR get_sys_action_desc( sys_action_type type )
{
	LPCWSTR desc = _T( "" ); 
	switch( type )
	{
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	case INSTALL_DRV:
		desc = _get_string_by_id( TEXT_SYS_ACTION_LOAD_DRIVER_DESC, 
			_T( "安装驱动" ) ); 
		break; 
	case ACCESS_COM:
		desc = _get_string_by_id( TEXT_SYS_ACTION_INSTALL_COM_DESC, 
			_T( "访问COM组件" ) ); 
		break; 
	case MODIFY_KEY:
		desc = _get_string_by_id( TEXT_SYS_ACTION_MODIFY_REGISTRY_DESC, 
			_T( "修改注册表键值" ) ); 
		break; 
	case MODIFY_FILE: 
		desc = _get_string_by_id( TEXT_SYS_ACTION_MODIFY_FILE_DESC, 
			_T( "修改文件" ) ); 
		break; 
	case DELETE_FILE: 
		desc = _get_string_by_id( TEXT_SYS_ACTION_DELETE_FILE_DESC, 
			_T( "删除文件" ) ); 
		break; 
	case LOCATE_URL: 
		desc = _get_string_by_id( TEXT_SYS_ACTION_LOCATE_TO_URL_DESC, 
			_T( "访问URL链接" ) ); 
		break; 
	case INSTALL_HOOK: 
		desc = _get_string_by_id( TEXT_SYS_ACTION_INSTALL_HOOK_DESC, 
			_T( "安装钩子" ) ); 
		break; 
	case CREATE_PROC: 
		desc = _get_string_by_id( TEXT_SYS_ACTION_CREATE_PROCESS_DESC, 
			_T( "生成进程" ) ); 
		break; 
	case ACCESS_MEM:
		desc = _get_string_by_id( TEXT_SYS_ACTION_ACCESS_REMOTE_MEMORY_DESC, 
			_T( "访问其它进程内存" ) ); 
		break; 
	case OTHER_ACTION: 
		desc = _get_string_by_id( TEXT_SYS_ACTION_OTHER_PROCTECTED_ACTION, 
			_T( "其它操作" ) ); 
		break; 
	case SOCKET_CONNECT: 
		desc = _get_string_by_id( TEXT_SYS_ACTION_CREATE_CONNECTION_DESC, 
			_T( "建立网络连接" ) ); 
		break; 
	case SOCKET_SEND:
		desc = _get_string_by_id( TEXT_SYS_ACTION_SEND_DATA_DESC, 
			_T( "发送网络数据" ) ); 
		break; 
	case SOCKET_RECV:
		desc = _get_string_by_id( TEXT_SYS_ACTION_RECEIVE_DATA_DESC, 
			_T( "接收网络数据" ) ); 
		break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE
	}

	return desc; 
}

INLINE LPCWSTR get_response_desc( action_response_type type )
{
	LPCWSTR desc = _T( "" ); 

	switch( type )
	{
	case ACTION_ALLOW: 
		desc = _get_string_by_id( TEXT_RESPONSE_ALLOW, _T( "允许" ) );
		break; 
	case ACTION_BLOCK:
		desc = _get_string_by_id( TEXT_RESPONSE_BLOCK, _T( "阻止" ) ); 
		break; 
	case ACTION_LEARN:
		desc = _get_string_by_id( TEXT_RESPONSE_LEARN, _T( "询问" ) ); 
		break; 
	default:
		break; 
	}

	return desc; 
}

INLINE LRESULT input_rule_from_action( sys_action_desc *sys_action, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
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

	//ntstatus = correct_params_of_rule( rule_input ); 

	ret = input_rule_to_config( rule_input, flags );

_return:

	if( rule_input != NULL )
	{
		free( rule_input );  
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT input_r3_rule_from_action( sys_action_desc *sys_action, ULONG flags ); 

LRESULT r3_check_sys_action( r3_action_notify *action, event_action_response *resp ); 

//LRESULT r3_check_sys_action( sys_action_desc *action, event_action_response *resp ); 

LRESULT load_acl_to_ring3(); 
LRESULT release_acl_from_ring3(); 

LRESULT init_debug_actions(); 
LRESULT _set_socket_debug_info( ULONG ip, ULONG port ); 
LRESULT set_socket_debug_info( LPCSTR ip, ULONG port ); 
LRESULT check_socket_debug_info( ULONG ip, ULONG port ); 
LRESULT debug_sys_action( sys_action_desc *action ); 