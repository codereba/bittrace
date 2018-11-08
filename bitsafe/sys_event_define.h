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

#ifndef __DEFENSE_RULES_H__
#define __DEFENSE_RULES_H__

#include "action_check.h"
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define NTSTATUS_SUCCESS_CODE 0x20000000
#define NTSTATUS_INFO_CODE 0x60000000
#define NTSTATUS_WARNING_CODE 0xa0000000
#define NTSTATUS_ERROR_CODE 0xe0000000

#define FACILITY_TRACE_LOG_CODE 0x00320000

#define NAME_EXACT_MATCH 0x00000008

typedef INT32 ( CALLBACK *check_info_callback )( ULONG flag, PLIST_ENTRY list_entry, PVOID param1, PVOID param2 ); 

#define URL_DEFINE_TABLE_SIZE 1024
#define IP_DEFINE_TABLE_SIZE 1024
#define PORT_DEFINE_TABLE_SIZE 1024
#define FILE_DEFINE_TABLE_SIZE 1024
#define REG_DEFINE_TABLE_SIZE 1024
#define COM_DEFINE_TABLE_SIZE 1024
#define APP_DEFINE_TABLE_SIZE 1024
#define COMMON_DEFINE_TABLE_SIZE 64


#define CLASS_URL_DEFINE_TABLE_SIZE 128
#define CLASS_IP_DEFINE_TABLE_SIZE 128
#define CLASS_PORT_DEFINE_TABLE_SIZE 128
#define CLASS_FILE_DEFINE_TABLE_SIZE 128
#define CLASS_REG_DEFINE_TABLE_SIZE 128
#define CLASS_COM_DEFINE_TABLE_SIZE 128
#define CLASS_APP_DEFINE_TABLE_SIZE 128
#define CLASS_COMMON_DEFINE_TABLE_SIZE 64

#define URL_RULE_DEFINE_TABLE_SIZE 128 
#define IP_PORT_RULE_DEFINE_TABLE_SIZE 1 
#define FILE_RULE_DEFINE_TABLE_SIZE 128 
#define REG_RULE_DEFINE_TABLE_SIZE 128
#define COM_RULE_DEFINE_TABLE_SIZE 128 
#define COMMON_RULE_TABLE_SIZE 128 

#define CLASS_PARAM 0x00000001
#define COMMON_PARAM 0x00000000

typedef struct _param_define_item
{
	ULONG ref_count; 
	LIST_ENTRY entry; 
	union
	{
		struct _LIST_ENTRY define_list; 
		common_hash_table *define_table; 
	};

	param_all_desc param; 
} param_define_item, *pparam_define_item; 

typedef struct _socket_rule_define
{
	param_define_item *app; 
	param_define_item *src_ip; 
	param_define_item *src_port; 
	param_define_item *dest_ip; 
	param_define_item *dest_port; 
} socket_rule_define, *psocket_rule_define; 

typedef struct _url_rule_define
{
	param_define_item *app; 
	param_define_item *url; 
} url_rule_define, *purl_rule_define; 

typedef struct _file_rule_define
{
	param_define_item *app; 
	param_define_item *file_path; 
} file_rule_define, *pfile_rule_define; 

typedef struct _reg_rule_define
{
	param_define_item *app; 
	param_define_item *reg_path; 
} reg_rule_define, *preg_rule_define; 

typedef struct _com_rule_define
{
	param_define_item *app; 
	param_define_item *com_name; 
} com_rule_define, *pcom_rule_define; 

typedef struct _app_rule_define
{
	param_define_item *app;  
} app_rule_define, *papp_rule_define; 

typedef struct _common_rule_define
{
	param_define_item *app; 
	param_define_item *param0; 
	param_define_item *param1; 
	param_define_item *param2; 
	param_define_item *param3; 
	sys_action_type action_type; 
} common_rule_define, *pcommon_rule_define; 

typedef union _action_rule_define
{
	socket_rule_define socket; 
	url_rule_define url; 
	file_rule_define file; 
	reg_rule_define reg; 
	com_rule_define com; 
	app_rule_define app; 
	common_rule_define common; 
} action_rule_define; 

typedef struct _action_rule_desc
{
	access_rule_type type; 
	action_response_type action; 
	data_trace_option trace_option; 
	action_rule_define rule; 
} action_rule_desc, *paction_rule_desc; 

typedef struct _action_rule_item
{
	ULONG ref_count; 
	union
	{
		LIST_ENTRY entry; 
		rb_node rb_node; 
	};

	action_rule_desc rule; 
} action_rule_item, *paction_rule_item; 

typedef struct _sys_action_rule_item
{
	LIST_ENTRY entry; 
	sys_action_desc rule; 
} sys_action_rule_item, *psys_action_rule_item; 

typedef struct _fs_style_interator
{
	LPCWSTR org_path; 
	LPWSTR iterator; 
	ULONG length; 
	ULONG index; 
} fs_style_interator, *pfs_style_interator; 

typedef struct _path_char_interator
{
	LPCSTR org_path; 
	LPSTR iterator; 
	ULONG length; 
	ULONG index; 
} path_char_interator, *ppath_char_interator; 

#define PATH_DELIM L'\\'
#define PATH_DELIM_CHAR '\\'

#ifndef INVALID_HASH_CODE 
#define INVALID_HASH_CODE ( ULONG )( - 1 )
#endif //INVALID_HASH_CODE

#define calc_com_name_hash_code calc_name_hash_code
#define calc_reg_name_hash_code calc_name_hash_code
#define calc_app_name_hash_code calc_name_hash_code 
#define calc_file_name_hash_code calc_name_hash_code

typedef NTSTATUS ( CALLBACK *uninit_iteration_callback )( PVOID context ); 
typedef NTSTATUS ( CALLBACK *init_iteration_callback )( PVOID param, /*ULONG buf_len, */ PULONG hash_code, ULONG tbl_size, PVOID *param_out, PVOID *context ); 
typedef NTSTATUS ( CALLBACK *iterate_name_callback )( PVOID param, PULONG hash_code, ULONG tbl_size, PVOID *param_out ); 

typedef INT32 ( CALLBACK* compare_hash_table_item_callback )( PVOID param, PLIST_ENTRY item, PVOID param_iteration ); 
typedef ULONG ( CALLBACK *calc_hash_code_callback )( PVOID param, ULONG table_size ); 

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

NTSTATUS alloc_param_define( param_define_type type, param_define *param_input, INT32 flag, param_define_item **param_alloc ); 
NTSTATUS _alloc_param_define( param_all_desc *param_input, INT32 flag, param_define_item **param_alloc ); 

INT32 compare_define_name_no_case( LPCWSTR define_name, LPCWSTR name_compare ); 
INT32 compare_define_name_no_case_len( LPCWSTR define_name, LPCWSTR name_compare, ULONG len ); 
ULONG CALLBACK calc_class_name_hash_code( PVOID param, ULONG table_size ); 
INT32 CALLBACK compare_class_define( PVOID param, PLIST_ENTRY entry, PVOID param_iteration ); 
NTSTATUS find_in_hash_table_lock_free( PVOID param, init_iteration_callback init_iteration_func, uninit_iteration_callback uninit_iteration_func, iterate_name_callback iteration_func, calc_hash_code_callback hash_code_func, compare_hash_table_item_callback compare_func, common_hash_table *table, PLIST_ENTRY *item_found ); 
NTSTATUS find_in_hash_table( PVOID param, init_iteration_callback init_iteration_func, uninit_iteration_callback uninit_iteration_func, iterate_name_callback iteration_func, calc_hash_code_callback hash_code_func, compare_hash_table_item_callback compare_func, common_hash_table *table, PLIST_ENTRY *item_found ); 

NTSTATUS add_param_define_to_class( param_define_item *param_input, LPCWSTR class_name ); 
NTSTATUS add_param_define( param_define_item *param_input, ULONG flag, LPCWSTR class_name ); 
INLINE NTSTATUS add_common_param_define( param_define_type type, param_define *param_input, LPCWSTR class_name, param_define_item **param_alloc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	param_define_item *param_item = NULL; 

	ASSERT( param_input != NULL ); 
	ASSERT( is_valid_param_define_type( type ) ); 

	*param_alloc = NULL; 
	ntstatus = alloc_param_define( type, param_input, COMMON_PARAM, &param_item ); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

	ASSERT( param_item != NULL ); 

	ntstatus = add_param_define( param_item, COMMON_PARAM, class_name ); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

_return:
	if( !NT_SUCCESS( ntstatus ) )
	{
		ASSERT( param_item == NULL ); 
		if( ntstatus == STATUS_PARAM_NO_MEANING )
		{
			ntstatus = STATUS_SUCCESS; 
		}
	}
	else
	{
		ASSERT( param_item != NULL ); 
		*param_alloc = param_item; 
	}
	return ntstatus; 
}

INLINE NTSTATUS add_com_define( com_param_define *com_define, LPCWSTR class_name, param_define_item **param_item  )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( com_define != NULL ); 

	ntstatus = add_common_param_define( COM_DEFINE, ( param_define* )com_define, class_name, param_item ); 

	return ntstatus; 
}

INLINE NTSTATUS add_file_define( file_param_define *file_define, LPCWSTR class_name, param_define_item **param_item  )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( file_define != NULL ); 

	ntstatus = add_common_param_define( FILE_DEFINE, ( param_define* )file_define, class_name, param_item ); 

	//_return:
	return ntstatus; 
}

INLINE NTSTATUS add_key_define( reg_param_define *key_define, LPCWSTR class_name, param_define_item **param_item )
{
	NTSTATUS ntstatus = STATUS_SUCCESS;  

	ASSERT( key_define != NULL ); 

	ntstatus = add_common_param_define( REG_DEFINE, ( param_define* )key_define, class_name, param_item ); 

	//_return:
	return ntstatus; 
}

INLINE NTSTATUS add_app_define( app_param_define *app_define, LPCWSTR class_name, param_define_item **param_item )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( app_define != NULL ); 

	ntstatus = add_common_param_define( APP_DEFINE, ( param_define* )app_define, class_name, param_item ); 

//_return:
	return ntstatus; 
}

INLINE NTSTATUS add_class_param_define( param_define_type type, param_other_desc *cls_define, param_define_item **param_alloc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	param_define_item *param_item = NULL; 

	ASSERT( cls_define != NULL ); 
	ASSERT( param_alloc != NULL ); 

	*param_alloc = NULL; 
	ntstatus = alloc_param_define( type, ( param_define* )cls_define, CLASS_PARAM, &param_item ); 

	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

	ASSERT( param_item != NULL ); 

	ntstatus = add_param_define( param_item, CLASS_PARAM, NULL ); 

_return:
	if( !NT_SUCCESS( ntstatus ) )
	{
		ASSERT( param_item == NULL ); 
		
		if( ntstatus == STATUS_PARAM_NO_MEANING )
		{
			ntstatus = STATUS_SUCCESS; 
		}
	}
	else
	{
		ASSERT( param_item != NULL ); 
		*param_alloc = param_item; 
	}

	return ntstatus; 
}

INT32 compare_url_define( LPCWSTR url_param, LPCWSTR url ); 
ULONG CALLBACK calc_url_name_hash_code( PVOID param, ULONG table_size ); 
ULONG CALLBACK calc_name_hash_code( PVOID param, ULONG table_size ); 
ULONG CALLBACK _calc_ip_hash_code( PVOID param, ULONG table_size ); 
ULONG CALLBACK calc_port_hash_code( PVOID param, ULONG table_size ); 
INT32 CALLBACK compare_app_name( PVOID param, PLIST_ENTRY item, PVOID param_iteration ); 
INT32 CALLBACK compare_com_name( PVOID param, PLIST_ENTRY item, PVOID param_iteration ); 
INT32 CALLBACK compare_file_name( PVOID param, PLIST_ENTRY item, PVOID param_iteration ); 
INT32 CALLBACK compare_reg_name( PVOID param, PLIST_ENTRY item, PVOID param_iteration ); 
INT32 CALLBACK compare_url_name( PVOID param, PLIST_ENTRY item, PVOID param_iteration ); 
INT32 CALLBACK compare_ip_define( PVOID param, PLIST_ENTRY item, PVOID param_iteration ); 
INT32 CALLBACK compare_port_define( PVOID param, PLIST_ENTRY item, PVOID param_iteration ); 
NTSTATUS get_param_define( param_define_type type, param_all_desc *param_input, param_define_item **item_found ); 
NTSTATUS get_class_param_define( param_define_type type, param_all_desc *param_input, param_define_item **item_found ); 

NTSTATUS get_rule_define( param_define_type type, 
						 access_rule_desc *rule_input, 
						 ULONG param_index, 
						 INT32 alloc_new, 
						 param_define_item **param ); 
NTSTATUS find_defines_of_rule( action_rule_define *rule, access_rule_desc *rule_input ); 
NTSTATUS alloc_access_rule( access_rule_desc *rule_input, action_rule_item **rule_alloc ); 
common_hash_table *get_action_rule_table( access_rule_type type ); 
ULONG CALLBACK calc_url_rule_hash_code( PVOID param, ULONG table_size ); 
ULONG CALLBACK calc_socket_rule_hash_code( PVOID param, ULONG table_size ); 
ULONG CALLBACK calc_file_rule_hash_code( PVOID param, ULONG table_size ); 
ULONG CALLBACK calc_reg_rule_hash_code( PVOID param, ULONG table_size ); 
ULONG CALLBACK calc_com_rule_hash_code( PVOID param, ULONG table_size ); 
ULONG CALLBACK calc_common_rule_hash_code( PVOID param, ULONG table_size ); 
NTSTATUS _add_action_rule( access_rule_desc *rule_input, action_rule_item *rule_add ); 
NTSTATUS _add_action_rule_lock_free( access_rule_desc *rule_input, action_rule_item *rule_add ); 

#define MODIFY_RULE 0x00000001

VOID _get_action_rule_func_and_param( access_rule_type rule_type, 
									 access_rule_desc *rule_input, 
									 init_iteration_callback *init_iteration_func, 
									 uninit_iteration_callback *uninit_iteration_func, 
									 iterate_name_callback *iterate_func, 
									 calc_hash_code_callback *hash_code_func, 
									 compare_hash_table_item_callback *compare_func, 
									 PVOID *param ); 

NTSTATUS add_action_rule( access_rule_desc *rule_input, ULONG flags ); 

NTSTATUS add_app_response_record( action_response_type resp, LPWSTR app_name, ULONG name_len ); 
NTSTATUS add_action_response_record( access_rule_type rule_type, action_response_type resp, LPWSTR app_name, ULONG name_len ); 
NTSTATUS _add_action_response_record( sys_action_type type, action_response_type resp, LPWSTR app_name, ULONG name_len ); 

INLINE NTSTATUS add_app_rule_define( access_rule_desc *access_rule )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( access_rule != NULL ); 

	if( check_access_rule_input_valid( access_rule, FALSE ) != STATUS_SUCCESS )
	{
		ntstatus = STATUS_INVALID_PARAMETER_1; 
		goto _return; 
	}

	ntstatus = add_app_response_record( access_rule->resp, 
		access_rule->desc.common.app.app.app_name, 
		( ULONG )wcslen( access_rule->desc.common.app.app.app_name ) + 1 ); 

_return:
	return ntstatus; 
}

INLINE NTSTATUS add_app_action_type_rule( access_rule_desc *access_rule )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( access_rule != NULL ); 

	if( check_access_rule_input_valid( access_rule, FALSE ) != STATUS_SUCCESS )
	{
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	ntstatus = add_action_response_record( access_rule->type, 
		access_rule->resp, 
		access_rule->desc.common.app.app.app_name, 
		( ULONG )wcslen( access_rule->desc.common.app.app.app_name ) + 1 ); 

_return:
	return ntstatus; 
}


NTSTATUS input_action_rule_from_desc( sys_action_desc *sys_action, ULONG flags ); 
INT32 CALLBACK compare_reg_rule( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration  ); 
INT32 CALLBACK compare_com_rule( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration  ); 
INT32 CALLBACK compare_file_rule( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration  ); 
INT32 _compare_socket_info( action_rule_define *socket_define, action_desc *socket ); 
INT32 CALLBACK compare_socket_rule( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration  ); 
INT32 CALLBACK compare_common_rule( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration  ); 
INT32 CALLBACK compare_url_rule( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration  ); 

NTSTATUS del_param_define( param_all_desc *param_input ); 
NTSTATUS del_class_param_define( param_all_desc *param ); 
NTSTATUS init_param_defines(); 
NTSTATUS init_class_param_defines(); 
NTSTATUS init_action_rule_defines(); 
NTSTATUS init_response_records(); 
NTSTATUS uninit_param_defines(); 
NTSTATUS uninit_class_param_defines(); 
NTSTATUS uninit_action_rule_defines(); 
NTSTATUS uninit_response_records(); 
VOID clear_response_records(); 

NTSTATUS init_action_manage(); 
NTSTATUS uninit_action_manage(); 

INLINE BOOLEAN is_untrusted_flag( sys_action_type flag )
{
	return FALSE; 
}

INLINE LPCWSTR get_rule_action_desc( action_response_type type )
{
	LPCWSTR desc = L""; 
	switch( type )
	{
	case ACTION_ALLOW:
		desc = L"ACTION_ALLOW"; 
		break; 
	case ACTION_BLOCK:
		desc = L"ACTION_BLOCK"; 
		break; 
	case ACTION_LEARN:
		desc = L"ACTION_LEARN"; 
		break; 
	default:
		ASSERT( "invalid action rule type" && FALSE ); 
		break;
	}

	return desc; 
}

NTSTATUS _get_defines_of_rule( action_rule_define *rule, access_rule_desc *rule_input, INT32 auto_add ); 

INLINE NTSTATUS get_defines_of_rule( action_rule_define *rule, access_rule_desc *rule_input )
{
	return _get_defines_of_rule( rule, rule_input, TRUE ); 
}

INLINE NTSTATUS find_defines_of_rule( action_rule_define *rule, access_rule_desc *rule_input )
{
	return _get_defines_of_rule( rule, rule_input, FALSE ); 
}

NTSTATUS del_action_rule( sys_action_desc *cur_action ); 
NTSTATUS _del_action_rule( access_rule_desc *rule_input ); 
NTSTATUS modify_action_rule( access_rule_desc *dest_rule, 
							access_rule_desc *rule_setting ); 

NTSTATUS check_sys_acl( sys_action_desc *cur_action, data_trace_option *trace_option, action_response_type *response ); 

INLINE NTSTATUS check_name_acl( sys_action_type action_type, LPWSTR app_name, LPWSTR param, data_trace_option *trace_option, action_response_type *response ,sys_action_desc **action_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_desc *cur_action = NULL; 
	BOOLEAN action_outputed = FALSE; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 
	ASSERT( response != NULL ); 

	if( action_out != NULL )
	{
		*action_out = NULL; 
	}

	cur_action = ( sys_action_desc* )ALLOC_TAG_POOL( sizeof( sys_action_desc ) ); 
	if( cur_action == NULL )
	{
		log_trace( ( MSG_ERROR, "!!!allocate action desc failed \n" ) ); 
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	ntstatus = generate_name_action_desc( action_type, app_name, param, cur_action ); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		log_trace( ( MSG_ERROR, "!!!generate the name action desc failed 0x%0.8x\n", ntstatus ) ); 
		goto _return;
	}

	ntstatus = check_sys_acl( cur_action, trace_option, response ); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		log_trace( ( MSG_ERROR, "!!!check sys acl failed 0x%0.8x\n", ntstatus ) ); 
		goto _return; 
	}

	if( action_out != NULL )
	{
		*action_out = cur_action; 
		action_outputed = TRUE; 
	}

_return:

	if( !NT_SUCCESS( ntstatus ) )
	{
		ASSERT( action_outputed == FALSE ); 

		if( cur_action != NULL )
		{
			FREE_TAG_POOL( cur_action ); 
		}
	}
	else
	{
		if( action_outputed == FALSE )
		{
			if( cur_action != NULL )
			{
				FREE_TAG_POOL( cur_action ); 
			}
		}
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x \n", __FUNCTION__, ntstatus ) ); 
	return ntstatus; 
}

INLINE NTSTATUS check_socket_acl( sys_action_type action_type, 
								 LPWSTR app_name, 
								 ULONG src_ip, 
								 USHORT src_port, 
								 ULONG dest_ip, 
								 USHORT dest_port, 
								 prot_type prot, 
								 data_trace_option *trace_option, 
								 action_response_type *response, 
								 sys_action_desc **action_alloc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_desc *cur_action = NULL;

	ASSERT( response != NULL ); 
	ASSERT( action_alloc != NULL ); 

	*action_alloc = NULL; 
	*response = ACTION_ALLOW; 

	cur_action = ( sys_action_desc* )ALLOC_TAG_POOL( sizeof( sys_action_desc ) ); 
	if( cur_action == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	ntstatus = generate_socket_action_desc( action_type, app_name, src_ip, src_port, dest_ip, dest_port, prot, cur_action );
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return;
	}

	ntstatus = check_sys_acl( cur_action, trace_option, response ); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

	*action_alloc = cur_action; 

_return:
	if( !NT_SUCCESS( ntstatus ) )
	{
		if( cur_action != NULL )
		{
			FREE_TAG_POOL( cur_action ); 
		}
	}

	return ntstatus; 
}

INLINE NTSTATUS get_socket_action( sys_action_type action_type, LPWSTR app_name, ULONG src_ip, USHORT src_port, ULONG dest_ip, USHORT dest_port, prot_type prot, sys_action_desc **action_alloc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_desc *cur_action = NULL;

	ASSERT( action_alloc != NULL ); 

	*action_alloc = NULL; 

	cur_action = ( sys_action_desc* )ALLOC_TAG_POOL( sizeof( sys_action_desc ) ); 
	if( cur_action == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	ntstatus = generate_socket_action_desc( action_type, app_name, src_ip, src_port, dest_ip, dest_port, prot, cur_action );
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return;
	}

	*action_alloc = cur_action; 

_return:
	if( !NT_SUCCESS( ntstatus ) )
	{
		if( cur_action != NULL )
		{
			FREE_TAG_POOL( cur_action ); 
		}
	}

	return ntstatus; 
}

INLINE INT32 compare_value_region_define( ULONG src_val_begin, ULONG src_val_end, ULONG dest_val_begin, ULONG dest_val_end )
{
	INT32 ret = FALSE; 

	if( src_val_end >= src_val_begin
		&& dest_val_end >= dest_val_begin )
	{
		if( src_val_begin < dest_val_begin )
		{
			goto _return; 
		}

		if( src_val_end > dest_val_end )
		{
			goto _return; 
		}

		if( src_val_begin > dest_val_end )
		{
			goto _return; 
		}

		if( src_val_end < dest_val_begin )
		{
			goto _return; 
		}
	}
	else
	{
		//ASSERT( FALSE ); 
	}

	ret = TRUE; 
_return:
	return ret;
}

INLINE NTSTATUS generate_action_desc_from_action( sys_action_info *action_info, sys_action_desc **action_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_desc *cur_action = NULL; 
	LPCWSTR name_param = NULL; 

	do 
	{
		ASSERT( action_info != NULL ); 
		ASSERT( action_out != NULL ); 

		//*action_out = NULL; 

		switch( action_info->action.type )
		{
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
		case SOCKET_CONNECT:
		case SOCKET_SEND:
		case SOCKET_RECV:
			ntstatus = get_socket_action( action_info->action.type, 
				action_info->ctx.proc_name, 
				action_info->action.socket_info.src_ip, 
				action_info->action.socket_info.src_port, 
				action_info->action.socket_info.dest_ip, 
				action_info->action.socket_info.dest_port, 
				action_info->action.socket_info.prot, 
				&cur_action ); 
			break; 
		case LOCATE_URL:
			ASSERT( FALSE && "can't reach here check url rule "); 
			//ntstatus = check_name_acl( action_info->action.type, action_info->ctx.proc_name, action_info->action.url_info.file_path, action_info->action.url_info.file_path, &need_log ); 

			//ntstatus = url_filter( action_info->action.url_info.domain_name, action_info->action.url_info.file_path ); 

			//if( NT_SUCCESS( ntstatus ) )
			//{
			//	response = ACTION_BLOCK; 
			//}
			//else
			//{
			//	response = ACTION_ALLOW;
			//}

			ntstatus = STATUS_INVALID_PARAMETER; 

			break; 
		case INSTALL_DRV:
			name_param = action_info->action.driver_info.path_name; 
			break; 
		case ACCESS_COM:
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		case MODIFY_KEY:
			name_param = action_info->action.reg_info.path_name; 
			break; 
		case MODIFY_FILE: 
		case DELETE_FILE: 
			name_param = action_info->action.file_info.path_name; 
			break; 
		case INSTALL_HOOK: 
			break; 
		case CREATE_PROC:
			name_param = action_info->action.proc_info.path_name; 
			break; 
		case TERMINATE_PROC:
			name_param = action_info->action.proc_info.path_name; 
			break; 
		case ACCESS_MEM:
			//break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE
		case OTHER_ACTION: 
			//ntstatus = STATUS_INVALID_PARAMETER; 
			//break; 
		default:
			ASSERT( FALSE && "unknown sys action" ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			goto _return; 	
			break; 	
		}

		if( name_param == NULL )
		{
			break; 
		}

		cur_action = ( sys_action_desc* )ALLOC_TAG_POOL( sizeof( sys_action_desc ) ); 
		if( cur_action == NULL )
		{
			log_trace( ( MSG_ERROR, "!!!allocate action desc failed \n" ) ); 
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			goto _return; 
		}

		ntstatus = generate_name_action_desc( action_info->action.type, action_info->ctx.proc_name, name_param, cur_action ); 
		if( !NT_SUCCESS( ntstatus ) )
		{
			FREE_TAG_POOL( cur_action ); 
			cur_action = NULL; 
			log_trace( ( MSG_ERROR, "!!!generate the name action desc failed 0x%0.8x\n", ntstatus ) ); 
			break; 
		}
	} while ( FALSE );

_return:
#ifdef DBG
	if( ntstatus != STATUS_SUCCESS )
	{
		ASSERT( cur_action == NULL ); 
	}
#endif //DBG
	*action_out = cur_action; 

	return ntstatus; 
}

#define HOST_APP_NAME_PREFIX L"SEVENFW"

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
INLINE NTSTATUS dump_socket_action( ipv4_socket_action *ipv4_action )
{
	ASSERT( ipv4_action != NULL ); 

	log_trace( ( MSG_INFO, "dump ip version 4 socket action:\n" ) ); 

	log_trace( ( MSG_INFO, "source ip address is : " ) ); 
	PrintIPAddr( &ipv4_action->src_ip ); 
	log_trace( ( MSG_INFO, "source prot is %u\n", ipv4_action->src_port ) ); 

	log_trace( ( MSG_INFO, "dest ip address is : " ) ); 
	PrintIPAddr( &ipv4_action->dest_ip ); 
	log_trace( ( MSG_INFO, "dest prot is %u\n", ipv4_action->dest_port ) ); 

	log_trace( ( MSG_INFO, "transfer protocol is %u\n", ipv4_action->prot ) ); 

	return STATUS_SUCCESS;
}
#endif //COMPATIBLE_OLD_ACTION_DEFINE

NTSTATUS check_system_key_proc( LPCWSTR proc_name ); 
NTSTATUS find_response_record( sys_action_type type, LPWSTR app_name, ULONG name_len, data_trace_option *trace_option, action_response_type *resp ); 
NTSTATUS init_app_record_lock(); 
NTSTATUS uninit_app_record_lock(); 
NTSTATUS init_app_records(); 
NTSTATUS uninit_app_response_records(); 
VOID clear_app_response_records(); 
NTSTATUS add_app_response_record( action_response_type resp, LPWSTR app_name, ULONG name_len ); 
NTSTATUS find_app_response_record( LPWSTR app_name, ULONG name_len, data_trace_option *trace_option, action_response_type *resp ); 

NTSTATUS check_socket_acl_direct( LPWSTR app_name, sys_action_record *action_record, data_trace_option *trace_option, action_response_type *response ); 

NTSTATUS CALLBACK notify_dns_packet_info( CHAR* dns_name, 
										 ULONG dns_name_len, 
										 USHORT prot, 
										 PVOID context, 
										 PVOID *param_out ); 

NTSTATUS CALLBACK check_domain_rule( CHAR* dns_name, 
									ULONG dns_name_len, 
									USHORT prot, 
									PVOID *param_out ); 

NTSTATUS pre_process_record_rule( access_rule_desc *rule_input, sys_action_type action_type ); 


INT32 compare_name_define( LPCWSTR define_name, LPCWSTR check_name ); 

NTSTATUS CALLBACK interate_url_path( PVOID param, PULONG hash_code, ULONG tbl_size, PVOID *param_out ); 
NTSTATUS CALLBACK iterate_fs_style_path( PVOID param, PULONG hash_code, ULONG tbl_size, PVOID *param_out ); 

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__DEFENSE_RULES_H__