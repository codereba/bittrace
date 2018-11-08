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

#ifndef __ACL_DEFINE_H__
#define __ACL_DEFINE_H__

#include "sevenfw_err_code.h"
#include "action_type.h"


#define INVALID_PROCESS_ID ( HANDLE )( -1 )
#define INVALID_THREAD_ID ( HANDLE )( -1 )

#define INVALID_ID_VALUE ( ULONG )( -1 ) 

#pragma warning( push )
#pragma warning( disable:4995 )

#ifndef _WCSNCPY_S
#define wcsncpy_s( _dest_str, _dest_size, _src_str, _src_size ) wcsncpy( _dest_str, _src_str, _dest_size ) 
#endif //_WCSNCPY_S

#ifdef _DRIVER
typedef NTSTATUS RESULT; 
#define ERROR_SUCCESS STATUS_SUCCESS
#define ERROR_BUFFER_OVERFLOW STATUS_BUFFER_OVERFLOW
#define ERROR_SOCKET_SOURCE_DEST_PROT_NOT_SAME STATUS_SOCKET_SOURCE_DEST_PROT_NOT_SAME
#define ERROR_PARAM_NO_MEANING STATUS_PARAM_NO_MEANING
#define ERROR_RULE_NO_MEANING STATUS_RULE_NO_MEANING 
#define ERROR_INVALID_PARAMETER STATUS_INVALID_PARAMETER
#define ERROR_ERRORS_ENCOUNTERED STATUS_UNSUCCESSFUL 
#define ERROR_BUFFER_TOO_SMALL STATUS_BUFFER_TOO_SMALL
#define ERROR_INVALID_PARAMETER_2 STATUS_INVALID_PARAMETER_2 
#else
typedef LRESULT RESULT; 
#define ERROR_INVALID_PARAMETER_2 ((NTSTATUS)0xC00000F0L) 
#endif //_DRIVER

#ifdef _DRIVER
#define ACTION_BLOCK_DESC _T( "block" )
#define ACTION_ALLOW_DESC _T( "allow" )
#define ALL_LEARN_DESC _T( "learn" )
#else
#define ACTION_BLOCK_DESC L"block" 
#define ACTION_ALLOW_DESC L"allow" 
#define ALL_LEARN_DESC  L"learn"
#endif //_DRIVER

typedef enum _ACTION_RECORD_TYPE
{
	RECORD_NONE, 
	RECORD_APP_ACTION, 
	RECORD_APP_ACTION_TYPE, 
	RECORD_APP 
} ACTION_RECORD_TYPE, *PACTION_RECORD_TYPE; 


typedef enum _param_compare_type
{
	COMPARE_APP_AND_PARAM = RECORD_APP_ACTION, 
	COMPARE_APP_AND_TYPE = RECORD_APP_ACTION_TYPE, 
	COMPARE_APP = RECORD_APP, 
} param_compare_type; 

#define is_valid_record_mode( mode ) ( mode >= RECORD_NONE && mode <= RECORD_APP )

typedef enum _work_mode
{
	WORK_FREE_MODE, //自由
	WORK_BLOCK_MODE, //禁止
	WORK_ACL_MODE,  //绿色
	WORK_LEARN_MODE, 
	MAX_WORK_MODE
} work_mode;

#define is_valid_work_mode( mode ) ( mode < MAX_WORK_MODE )

INLINE LPCWSTR get_work_mode_desc( ULONG mode )
{
	LPCWSTR mode_desc = L"UNKNOWN_WORK_MODE"; 

	switch( mode )
	{
	case WORK_FREE_MODE:
		mode_desc = L"WORK_FREE_MODE"; 
		break; 
	case WORK_BLOCK_MODE:
		mode_desc = L"WORK_BLOCK_MODE"; 
		break; 
	case WORK_ACL_MODE:
		mode_desc = L"WORK_ACL_MODE"; 
		break; 
	default:
	break; 
	}

	return mode_desc; 
}

#define PROT_TCP_DESC L"tcp"
#define PROT_UDP_DESC L"udp" 

INLINE LPCWSTR get_prot_type_desc( prot_type type )
{
	LPCWSTR desc = L""; 

	switch( type )
	{
	case PROT_TCP:
		desc = PROT_TCP_DESC; 
		break; 
	case PROT_UDP:
		desc = PROT_UDP_DESC; 
		break; 
	default:
		ASSERT( FALSE && "invalid protocol type" ); 
		break; 
	}

	return desc; 
}

#define _MAX_CLASS_NAME_LEN 64 
#define _MAX_DESC_LEN 256
#define NAME_PARAM_INDEX 1
#define _MAX_FILE_NAME_LEN 260
#define _MAX_COM_NAME_LEN 260 
#define _MAX_COMMAND_LEN 1024

#define _MAX_URL_LEN 1024 
#define _MAX_TEXT_LEN _MAX_REG_PATH_LEN 
#define MAX_RULE_PARAM_NUM 5
#define MAX_DOMAIN_NAME_LEN 256
#define MAX_DATA_FLOW_COUNT 4096


#ifdef _DRIVER
#define INSTALL_DRV_DESC L"安装驱动" 
#define INSTALL_HOOK_DESC L"安装钩子" 
#define CREATE_PROC_DESC L"运行其它程序" 
#define ACCESS_MEM_DESC L"访问其它程序内存" 
#define ACCESS_COM_DESC L"访问COM组件功能" 
#define MODIFY_KEY_DESC L"修改注册表键值" 
#define MODIFY_FILE_DESC L"修改被保护目录下文件" 
#else
#define INSTALL_DRV_DESC _T( "安装驱动" )
#define INSTALL_HOOK_DESC _T( "安装钩子") 
#define CREATE_PROC_DESC _T( "运行其它程序") 
#define ACCESS_MEM_DESC _T( "访问其它程序内存" )
#define ACCESS_COM_DESC _T( "访问COM组件功能")
#define MODIFY_KEY_DESC _T( "修改注册表键值")
#define MODIFY_FILE_DESC _T( "修改被保护目录下文件" )
#endif //_DRIVER

#define is_valid_access_rule_type( type ) ( type >= ACTION_RULE_BEGIN && type <= ACTION_RULE_END ) 
#define is_rbtree_link_rule( type ) ( type == SOCKET_RULE_TYPE )

typedef enum _param_define_type
{
	URL_DEFINE, 
	IP_DEFINE, 
	PORT_DEFINE, 
	FILE_DEFINE, 
	REG_DEFINE, 
	COM_DEFINE, 
	APP_DEFINE, 
	COMMON_DEFINE, 
	BEGIN_PARAM_DEFINE_TYPE = URL_DEFINE, 
	END_PARAM_DEFINE_TYPE = COMMON_DEFINE, 
	MAX_PARAM_DEFINE_TYPE, 
	INVALID_PARAM_DEFINE_TYPE = -1 
} param_define_type; 

#define is_name_param_define_type( type ) ( type == COM_DEFINE || type == REG_DEFINE || type == FILE_DEFINE || type == URL_DEFINE || type == APP_DEFINE || type == COMMON_DEFINE ) 

#define is_fw_param_define_type( type ) ( type == IP_DEFINE || type == PORT_DEFINE || type == URL_DEFINE )

#define must_not_mull_param( type ) FALSE

INLINE INT32 is_must_meaningful_param( param_define_type type )
{
	INT32 ret = FALSE; 

	if( type == URL_DEFINE )
	{
		ret = TRUE; 
	}

	return ret; 
}

#define is_valid_param_define_type( type ) ( type >= BEGIN_PARAM_DEFINE_TYPE && type <= END_PARAM_DEFINE_TYPE ) 

#define is_path_name_param_define_type( type ) ( type == REG_DEFINE || type == FILE_DEFINE )

typedef enum _action_rule_type
{
	SOCKET_RULE_TYPE, 
	URL_RULE_TYPE, 
	FILE_RULE_TYPE, 
	REG_RULE_TYPE, 
	COM_RULE_TYPE, 
	COMMON_RULE_TYPE, 
	ACTION_RULE_BEGIN = SOCKET_RULE_TYPE, 
	ACTION_RULE_END = COMMON_RULE_TYPE, 
	NAME_RULE_BEGIN = URL_RULE_TYPE, 
	NAME_RULE_END = COMMON_RULE_TYPE, 
	MAX_ACTION_RULE_TYPE = ACTION_RULE_END + 1, 
	INVALID_RULE_TYPE = -1 
} access_rule_type, *paction_rule_type; 

#define is_name_rule_type( rule_type ) ( rule_type >= NAME_RULE_BEGIN && rule_type <= NAME_RULE_END )
#define is_socket_rule_type( rule_type ) ( rule_type == SOCKET_RULE_TYPE )
#define is_fw_rule_type( rule_type ) ( rule_type == SOCKET_RULE_TYPE || rule_type == URL_RULE_TYPE )

typedef enum _action_response_type
{
	ACTION_ALLOW, 
	ACTION_BLOCK, 
	ACTION_LEARN, 
	RULE_ACTION_BEGIN = ACTION_BLOCK, 
	RULE_ACTION_END = ACTION_LEARN,
	MAX_RESPONSE_TYPE
} action_response_type; 

typedef enum _action_source_response_type
{
	BLOCK_SOURCE_ACTIONS, 
	ALLOW_SOURCE_ACTIONS, 
	DELETE_SOURCE_FILE, 
	DELETE_SOURCE_VIRUS_CODE, 

	SOURCE_ACTION_BEGIN = BLOCK_SOURCE_ACTIONS, 
	SOURCE_ACTION_END = DELETE_SOURCE_VIRUS_CODE,
	MAX_ACTION_SOURCE_RESPONSE_TYPE, 
} action_source_response_type; 

typedef action_response_type action_reply; 

INLINE LPCWSTR get_action_resp_desc( action_response_type type )
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
		break; 
	}

	return desc; 
}

typedef enum _action_access_type
{
	WRITE_ACCESS, 
	READ_ACCESS, 
	DELETE_ACCESS
} action_access_type; 

#define is_valid_response_type( type ) ( type >= RULE_ACTION_BEGIN && type <= RULE_ACTION_END ) 

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif //PAGE_SIZE

#pragma pack( push )
#pragma pack( 1 )

typedef struct _action_context
{
	NTSTATUS last_result; 
	ULONG proc_id; 
	ULONG thread_id; 
	WCHAR proc_name[ _MAX_FILE_NAME_LEN ]; 
	ULONG proc_name_len; 

#if 0 
	WCHAR cmd_line[ _MAX_COMMAND_LEN ]; 
	ULONG cmd_line_len; 
#endif //0 

#ifdef SID_FILTER_SUPPORT
	ULONG sid_a_size; 
	BYTE sid_a[ MAX_SID_SIZE ]; //struct _SID_AND_ATTRIBUTES *
#endif //SID_FILTER_SUPPORT

#ifdef DRIVER
	LARGE_INTEGER time; 
#else
	union
	{
		LARGE_INTEGER time; 
		LPWSTR time_text; 
	}; 
#endif //DRIVER
} action_context, *paction_context; 

typedef struct _sys_action_info
{
	action_context ctx; 
	sys_action_record action; 
} sys_action_info, *psys_action_info; 

#define FLAGS_EVENT_INSERTED ( LONGLONG )( 0xffffffffffffffff )
#define MAX_TRACE_FRAME_COUNT 64
typedef struct _r3_action_notify
{
	ULONG size; 
	BOOLEAN data_inited; 
	ULONG data_size; 
	ULONG real_data_size; 
	ULARGE_INTEGER id; 
	ULONG frame_count; 

	ULONGLONG stack_frame[ MAX_TRACE_FRAME_COUNT ]; 

	sys_action_info action; 
	BYTE append[ 1 ]; 
#ifdef R3_REMAPPED_VM_DEFINED
	r3_remapped_vm data_buf; 
#else
	//BYTE data_buf[ 1 ]; 
#endif //R3_REMAPPED_VM_DEFINED

} r3_action_notify, *pr3_action_notify; 

#pragma pack( pop )

#define ACTION_RECORD_OFFEST ( FIELD_OFFSET( r3_action_notify, action ) + FIELD_OFFSET( sys_action_info, action ) ) 

#define DEFAULT_ACTION_RECORD_REGION_SIZE ( sizeof( sys_action_record ) + DEFAULT_PARAMETERS_VARIABLE_SIZE )
#define R3_NOTIFY_BUF_SIZE ( sizeof( r3_action_notify ) + DEFAULT_PARAMETERS_VARIABLE_SIZE + DEFAULT_OUTPUT_DATA_REGION_SIZE )

#define BASE_MAX_R3_NOTIFY_VARIABLE_SIZE ( ULONG )( R3_NOTIFY_BUF_SIZE - ACTION_RECORD_OFFEST )
#define MAX_R3_NOTIFY_VARIABLE_SIZE( __buffer_size__ ) ( ULONG )( ( __buffer_size__ ) - ACTION_RECORD_OFFEST )

#define SYS_ACTION_DATA_END_SIGN ( ULONG )'SEAS'

#define ACTION_RECORD_SIZE_BY_TYPE( _type_ ) ( sizeof( _type_ ) + ACTION_RECORD_PARAMETERS_OFFSET ) 

#define ACTION_RECORD_SIZE_BY_TYPE_APPEND( type, append_size ) ( ACTION_RECORD_SIZE_BY_TYPE( type ) + append_size ) 

INLINE BOOLEAN action_can_be_blocked( sys_action_type type )
{
	BOOLEAN ret = TRUE; 

	do 
	{
		if( is_socket_action( type ) == FALSE )
		{
			break; 
		}

		ret = FALSE; 
	}while( FALSE );

	return ret; 
}

INLINE sys_action_info *_alloc_sys_action_info( ULONG info_size )
{
	sys_action_info *action = NULL; 

	do 
	{
#define MAX_ACTION_INFO_SIZE ( sizeof( sys_action_info ) + DEFAULT_PARAMETERS_VARIABLE_SIZE )
		if( info_size > MAX_ACTION_INFO_SIZE )
		{
			ASSERT( FALSE ); 
			break; 
		}

		if( info_size == 0 )
		{
			ASSERT( FALSE ); 
			break; 
		}

#ifndef DRIVER
#define ALLOC_TAG_POOL( size ) malloc( size )
#endif //DRIVER

		action = ( sys_action_info* )ALLOC_TAG_POOL( info_size ); 
		if( action == NULL )
		{
			break; 
		}

		action->action.size = ( USHORT )info_size - ( FIELD_OFFSET( sys_action_info, action ) ); 
	}while( FALSE );

	return action; 
}

#define ACTION_RECORD_PARAMETERS_OFFSET ( ULONG )( FIELD_OFFSET( sys_action_record, do_exec_create ) ) 

#define ACTION_RECORD_PARAMETERS_SIZE( __action_info_size ) ( ULONG )( ( __action_info_size ) - ACTION_RECORD_PARAMETERS_OFFSET )

INLINE BOOLEAN is_intresting_event_type( sys_action_type type )
{
	BOOLEAN ret = FALSE; 

	do
	{
		//if( TRUE == is_network_action( type ) )
		//{
		//	ret = TRUE; 
		//	break; 
		//}

		if( type == FILE_read 
			|| type == FILE_write )
		{
			ret = TRUE; 
			break; 
		}

	}while( FALSE ); 

	return ret; 
}


INLINE ULONG get_sys_action_output_size( sys_action_info *action_info )
{
	ULONG record_size = 0; 

	do 
	{
		/***************************************************************************
		can use the r3 max output size subtract the action output front members to be the 
		max size of the data,that's maximum value.
		***************************************************************************/

		if( BASE_MAX_R3_NOTIFY_VARIABLE_SIZE < action_info->action.size )
		{
			break; 
		}

		record_size = FIELD_OFFSET( sys_action_info, action ) + action_info->action.size; 

	}while( FALSE ); 

	return record_size; 
}

INLINE sys_action_info *alloc_sys_action_info( ULONG info_size )
{
	sys_action_info *action = NULL; 

	do 
	{
#define MAX_ACTION_INFO_SIZE ( sizeof( sys_action_info ) + DEFAULT_PARAMETERS_VARIABLE_SIZE )
		if( ACTION_RECORD_PARAMETERS_OFFSET + info_size > MAX_ACTION_INFO_SIZE )
		{
			ASSERT( FALSE ); 
			break; 
		}

		if( info_size == 0 )
		{
			ASSERT( FALSE ); 
			break; 
		}		

		action = _alloc_sys_action_info( ACTION_RECORD_PARAMETERS_OFFSET + info_size ); 

	}while( FALSE );

	return action; 
}

#ifdef R3_REMAPPED_VM_DEFINED

#pragma pack( push )
#pragma pack( 4 )

typedef struct _r3_remapped_vm
{
	PVOID r3_addr; 
	ULONG vm_size; 
#ifdef _DRIVER
	PMDL mdl; 
#else
	PVOID mdl; 
#endif //_DRIVER
} r3_remapped_vm, *pr3_remapped_vm; 

#pragma pack( pop )

#endif //R3_REMAPPED_VM_DEFINED

INLINE PVOID get_action_output_data( r3_action_notify *action )
{
	return ( PVOID )( ( BYTE* )&action->action.action + action->action.action.size ); 
}

#define get_action_output_data_ptr get_action_output_data

INLINE ULONG get_r3_action_notify_size( r3_action_notify *action )
{
	return ( action->data_size 
		+ ACTION_RECORD_OFFEST
		+ action->action.action.size ); 
}

INLINE RESULT check_r3_action_notify_valid( r3_action_notify *action )
{
	RESULT ret = ERROR_SUCCESS; 

	do 
	{

		if( get_r3_action_notify_size( action ) > R3_NOTIFY_BUF_SIZE )
		{
			ret = ERROR_INVALID_PARAMETER_2; 
			break; 
		}

		if( action->data_inited == FALSE )
		{
			if( action->data_size != 0 )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}
		else
		{
			if( action->data_size > action->real_data_size )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}

	}while( FALSE );
	
	return ret; 
}

INLINE RESULT check_r3_action_notify_output_valid( r3_action_notify *action )
{
	RESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = check_r3_action_notify_valid( action ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( action->size != get_r3_action_notify_size( action ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

#define DEF_TRACE_DATA_SIZE 0

#define DATA_TRACE_RET_SUCCESS 0x00000001
#define DATA_TRACE_RET_ERROR 0x00000002
#define DATA_TRACE_RET_ALL 0x00000003
#define DATA_TRACE_RET_LEARN 0x80000000

#define DATA_TRACE_RET_BY_SETTING 0x00000004

#define DATA_TRACE_RET_MASK 0x0000000f

#define DATA_TRACE_MODE_NONE 0x00000000
#define DATA_TRACE_MODE_ACTION 0x00000010
#define DATA_TRACE_MODE_DATA 0x00000020
#define DATA_TRACE_MODE_MASK 0x00000030

#define DEF_TRACE_MODE DATA_TRACE_MODE_NONE 

#define is_valid_trace_mode( mode ) ( mode == DATA_TRACE_MODE_NONE || mode == DATA_TRACE_MODE_ACTION || mode == DATA_TRACE_MODE_DATA ) 

#define DATA_TRACE_RET_TYPE( trace_flags ) ( trace_flags & DATA_TRACE_RET_MASK )
#define DATA_TRACE_MODE( trace_flags ) ( trace_flags & DATA_TRACE_MODE_MASK )

typedef enum __data_context_type
{
	DATA_CONTEXT_NULL = 0, 
	DATA_CONTEXT_IRP, 
	DATA_CONTEXT_MDL, 
	DATA_CONTEXT_BUF, 
	DATA_MAX_TYPE,
} data_context_type, *pdata_context_type; 

#define is_valid_data_context_type( data_type ) ( data_type >= DATA_CONTEXT_IRP && data_type < DATA_MAX_TYPE ) 

typedef struct _url_param_define
{
	WCHAR url[ _MAX_URL_LEN ]; 
	USHORT domain_name_len; 
	USHORT domain_name_off; 
	USHORT file_path_len; 
	USHORT file_path_off; 
} url_param_define, *purl_param_define; 

typedef struct _ip_param_define
{
	ULONG ip_begin; 
	ULONG ip_end; 
} ip_param_define, *pip_param_define; 

typedef struct _port_param_define
{
	ULONG port_begin; 
	ULONG port_end; 
	prot_type type; 
} port_param_define, *pport_param_define; 

typedef struct _app_param_define
{
	WCHAR app_name[ _MAX_FILE_NAME_LEN ]; 
} app_param_define, *papp_param_define; 

#define MAX_SIDS_COUNT		128

typedef struct _sid_param_define
{
	UCHAR sid_mask[ MAX_SIDS_COUNT / 8 ];	/* SIDs bitmask */
} sid_param_define, *p_sid_param_define; 


typedef struct _file_param_define
{
	WCHAR file_path[ _MAX_FILE_NAME_LEN ]; 
} file_param_define, *pfile_param_define; 

typedef struct _com_param_define
{
	WCHAR com_name[ _MAX_COM_NAME_LEN ]; 
} com_param_define, *pcom_param_define; 

typedef struct _reg_param_define
{
	WCHAR reg_path[ _MAX_REG_PATH_LEN ]; 
} reg_param_define, *preg_param_define; 

typedef struct _class_param_define
{
	WCHAR class_name[ _MAX_CLASS_NAME_LEN ]; 
} class_param_define, *pclass_param_define; 

typedef struct _common_param_define
{
	WCHAR name[ _MAX_FILE_NAME_LEN ]; 
} common_param_define, *pcommon_param_define; 

typedef ULONG PARAM_ID; 

typedef union
{
	reg_param_define reg; 
	com_param_define com; 
	file_param_define file; 
	app_param_define app; 
	port_param_define port; 
	ip_param_define ip; 
	url_param_define url; 
	class_param_define cls; 
	common_param_define common; 
} param_define; 

typedef struct _param1_desc
{
	param_define_type type; 
	ULONG id; 
	BOOLEAN is_cls; 
	USHORT size; 

	union
	{
		class_param_define cls; 
		app_param_define app; 
		com_param_define com; 
	};

} param1_desc, *pparam1_desc; 

typedef struct _param_all_desc
{
	param_define_type type; 
	ULONG id; 
	BOOLEAN is_cls; 
	USHORT size; 

	union
	{
		app_param_define app; 
		reg_param_define reg; 
		com_param_define com; 
		file_param_define file; 
		port_param_define port; 
		ip_param_define ip; 
		url_param_define url; 
		class_param_define cls; 
		common_param_define common; 
	};
} param_all_desc, *pparam_all_descc; 

typedef struct _param2_desc
{
	param_define_type type; 
	ULONG id; 
	BOOLEAN is_cls; 
	USHORT size; 

	union
	{
		reg_param_define reg; 
		com_param_define com; 
		file_param_define file; 
		port_param_define port; 
		ip_param_define ip; 
		url_param_define url; 
		class_param_define cls; 
		common_param_define common; 
	};
} param2_desc, *pparam2_descc; 

typedef struct _param_common_desc
{
	param_define_type type; 
	ULONG id; 
	BOOLEAN is_cls; 
	USHORT size; 

	union
	{
		common_param_define common; 
	};
} param_common_desc, *pparam_common_desc; 

typedef struct _param_other_desc
{
	param_define_type type; 
	ULONG id; 
	BOOLEAN is_cls; 
	USHORT size; 

	union
	{
		ip_param_define ip; 
		port_param_define port; 
		class_param_define cls; 
	};

} param_other_desc, *pparam_other_desc; 

#define MAX_RING3_REPLY_WAIT_TIME ( LONGLONG )( -(20) * (10 * 1000 * 1000) )
#define DEF_RING3_REPLY_WAIT_TIME ( LONGLONG )( -(1) * ( 100 ) )
#define DEBUG_RING3_REPLY_WAIT_TIME ( LONGLONG )( -( 60 ) * ( 10 * 1000 * 1000 ) )

typedef struct _event_action_response
{
	ULONG id; 
	USHORT action; 
	BYTE need_record; 
} event_action_response, *pevent_action_response; 

typedef struct _socket_action_desc
{
	param1_desc app; 
	param_other_desc src_ip; 
	param_other_desc src_port; 
	param_other_desc dest_ip; 
	param_other_desc dest_port; 
} socket_action_desc, *psocket_action_desc; 

typedef struct _url_action_desc
{
	param1_desc app; 
	param2_desc url; 
	LIST_ENTRY ip_addrs; 
} url_action_desc, *purl_action_desc; 

typedef struct _file_action_desc
{
	param1_desc app; 
	param2_desc file_path; 
} file_action_desc, *pfile_action_desc; 

typedef struct _reg_action_desc
{
	param1_desc app; 
	param2_desc reg_path; 
} reg_action_desc, *preg_action_desc; 

typedef struct _com_action_desc
{
	param1_desc app; 
	param1_desc com; 
} com_action_desc, *pcom_action_desc; 

typedef struct _common_action_desc
{
	param1_desc app; 
	param2_desc param0; 
	param_other_desc param1; 
	param_other_desc param2; 
	sys_action_type action_type; 
} common_action_desc, *pcommon_action_desc; 

typedef struct _action_desc
{
	union
	{
		socket_action_desc socket; 
		url_action_desc url; 
		file_action_desc file; 
		reg_action_desc reg; 
		com_action_desc com; 
		common_action_desc common;
	};

	param_all_desc *params[ MAX_RULE_PARAM_NUM ]; 
} action_desc, *paction_desc; 

#define INVALID_ACTION_ID ( ULONG )( -1 )

typedef enum __data_alloc_mode
{
	DATA_BUF_INVALID_ALLOC_MODE = 0x00000000, 
	DATA_BUF_MAPPED_TO_USER = 0x00000001, 
	DATA_BUF_ALLOCATE_IN_KERNEL_PAGED, 
	DATA_BUF_ALLOCATE_IN_KERNEL_NOPAGED,
	DATA_BUF_NONE, 
	MAX_DATA_ALLOCATE_MODE, 
} data_alloc_mode, *pdata_mode; 

#define is_valid_data_alloc_mode( alloc_mode ) ( alloc_mode >= DATA_BUF_MAPPED_TO_USER && alloc_mode < MAX_DATA_ALLOCATE_MODE )

C_ASSERT( ( sizeof( ULONG_PTR ) / sizeof( WCHAR ) ) >= 2 ); 

typedef struct _relating_data
{
	data_alloc_mode mode; 
	ULONG data_len; 

	union
	{
		PVOID data; 
		ULONG_PTR data_off; 
		WCHAR data_buf[ sizeof( ULONG_PTR ) /sizeof( WCHAR ) ]; 
	};
} relating_data, *prelating_data; 

#define is_valid_proc_handle( proc_handle ) ( proc_handle != NULL && proc_handle != ZwCurrentProcess() ) 


#pragma pack( push )
#pragma pack( 1 )

typedef struct _sys_action_desc
{
	ULONG id; 
	sys_action_type type; 
	action_response_type resp; 
	action_desc desc; 

#ifdef SUPPORT_MDL_REMAPPING
	r3_remapped_vm data; 
#endif //SUPPORT_MDL_REMAPPING

} sys_action_desc, *psys_action_desc; 

#define SYS_ACTION_OUTPUT_MAGIC ( ULONG )'tcas'
#define SYS_ACTION_PLACEHOLDER_MAGIC ( ULONG )'dlhp'

#define MAX_SYS_ACTION_UNIT_SIZE ( sizeof( sys_action_output ) + DEFAULT_OUTPUT_DATA_REGION_SIZE )

typedef struct _sys_action_output
{
	ULONG size; 
	ULONG magic; 
	sys_action_desc action; 
	BYTE data[ 1 ]; 
}sys_action_output, *psys_action_output; 

typedef struct _sys_log_unit
{
	sys_action_output action_log; 
	BYTE data[ DEFAULT_OUTPUT_DATA_REGION_SIZE ]; 
}sys_log_unit, *psys_log_unit; 

typedef struct _sys_action_and_data_output
{
	sys_action_info *action; 
	BYTE *data_buf; 
	ULONG data_size; 
}sys_action_and_data_output, *psys_action_and_data_output; 

FORCEINLINE ULONG calc_action_output_size( ULONG data_len )
{
	ULONG size; 

#ifdef DBG
	if( /*data_len == 0 || */ data_len > DEFAULT_OUTPUT_DATA_REGION_SIZE )
	{
		ASSERT( FALSE && "invalid data size" ); 
	}
#endif //DBG

	size = data_len; 
	size += sizeof( sys_action_output ); 

	return size; 
}

FORCEINLINE RESULT is_valid_action_output( sys_action_output *action_output )
{
	RESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( action_output != NULL ); 
		if( action_output->magic != SYS_ACTION_OUTPUT_MAGIC )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
		}

		if( action_output->size < FIELD_OFFSET( sys_action_output, data ) 
			|| action_output->size > sizeof( sys_action_output ) + DEFAULT_OUTPUT_DATA_REGION_SIZE )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER_2; 
		}
	}while( FALSE );

	return ret; 
}

typedef struct _sys_action_output_map
{
	sys_action_output *action; 
} sys_action_output_map, *psys_action_output_map; 

INLINE ULONG get_action_output_data_size( sys_action_output *output )
{
	return output->size - sizeof( sys_action_output ); 
}

typedef struct _sys_action_map
{
	sys_action_desc *desc; 
	ULONG desc_len; 
} sys_action_map, *psys_action_map; 

#define RULE_ID_SIZE		32
typedef struct _access_rule_desc
{
	access_rule_type type; 
	action_response_type resp; 
	action_desc desc; 
} access_rule_desc, *paccess_rule_desc; 

typedef struct _access_rule_modify_info
{
	access_rule_desc dest_rule; 
	access_rule_desc rule_setting; 
} access_rule_modify_info, *paccess_rule_modify_info; 

typedef struct _url_param_input
{
	LPCWSTR url; 
} url_param_input, *purl_param_input; 

typedef struct _ip_param_input
{
	ULONG ip_begin; 
	ULONG ip_end; 
} ip_param_input, *pip_param_input; 

typedef struct _port_param_input
{
	ULONG port_begin; 
	ULONG port_end; 
	prot_type type; 
} port_param_input, *pport_param_input; 

typedef struct _app_param_input
{
	LPCWSTR app_name; 
} app_param_input, *papp_param_input; 

typedef struct _file_param_input
{
	LPCWSTR file_path; 
} file_param_input, *pfile_param_input; 

typedef struct _com_param_input
{
	LPCWSTR com_name; 
} com_param_input, *pcom_param_input; 

typedef struct _reg_param_input
{
	LPCWSTR reg_path; 
} reg_param_input, *preg_param_input; 

typedef struct _class_param_input
{
	LPCWSTR class_name; 
} class_param_input, *pclass_param_input; 

typedef struct _common_param_input
{
	LPCWSTR name; 
} common_param_input, *pcommon_param_input; 

typedef union
{
	reg_param_input reg; 
	com_param_input com; 
	file_param_input file; 
	app_param_input app; 
	port_param_input port; 
	ip_param_input ip; 
	url_param_input url; 
	class_param_input cls; 
	common_param_input common; 
} param_define_input; 

typedef struct _param_input_desc
{
	param_define_type type; 
	PARAM_ID id; 
	BOOLEAN is_cls; 
	
	union
	{
		reg_param_input reg; 
		com_param_input com; 
		file_param_input file; 
		app_param_input app; 
		port_param_input port; 
		ip_param_input ip; 
		url_param_input url; 
		class_param_input cls; 
		common_param_input common; 
	};
} param_input_desc, *pparam_input_desc; 


typedef struct _socket_action_input
{
	param_input_desc app; 
	param_input_desc src_ip; 
	param_input_desc src_port; 
	param_input_desc dest_ip; 
	param_input_desc dest_port; 
} socket_action_input, *psocket_action_input; 
typedef struct _url_action_input
{
	param_input_desc app; 
	param_input_desc url; 
} url_action_input, *purl_action_input; 

typedef struct _file_action_input
{
	param_input_desc app; 
	param_input_desc file_path; 
} file_action_input, *pfile_action_input; 

typedef struct _reg_action_input
{
	param_input_desc app; 
	param_input_desc reg_path; 
} reg_action_input, *preg_action_input; 

typedef struct _com_action_input
{
	param_input_desc app; 
	param_input_desc com; 
} com_action_input, *pcom_action_input; 


typedef struct _common_action_input
{
	param_input_desc app; 
	param_input_desc param0; 
	param_input_desc param1; 
	param_input_desc param2; 
	param_input_desc param3; 
} common_action_input, *pcommon_action_input; 

typedef union _action_ptr_desc
{
	socket_action_input socket; 
	url_action_input url; 
	file_action_input file; 
	reg_action_input reg; 
	com_action_input com; 
	common_action_input common;
	param_input_desc params[ MAX_RULE_PARAM_NUM ]; 
} action_ptr_desc; 

typedef struct access_rule_ptr_desc
{
	action_response_type resp; 
	access_rule_type type; 
	action_ptr_desc desc; 
} access_rule_ptr_desc, *paccess_rule_ptr_desc;  

typedef struct _data_trace_option
{
	ULONG trace_mode; 
	ULONG tracing_size; 
} data_trace_option, *pdata_trace_option; 

#pragma pack( pop )

#define TRACE_SIZE_MAX_LIMIT 0x7fffffff

#define DEF_ACTION_TRACE_FLAGS ( DATA_TRACE_RET_ERROR | DATA_TRACE_RET_LEARN )

#ifdef _DRIVER
typedef struct _data_trace_context
{
	//data_trace_option; 
	//HANDLE proc_handle; 
	//ULONG trace_mode; 
	data_context_type data_type; 
	IO_DIRECTION data_dir; 
	ULONG data_size; 

	union
	{
		PIRP irp; 
		PMDL mdl; 
		PVOID data; 
	} data_context; 

	//ULONG action_id; 
	//ULONGLONG action_id; 
} data_trace_context, *pdata_trace_context; 

INLINE BOOLEAN is_valid_trace_context( data_trace_context *trace_context )
{
	BOOLEAN ret = TRUE; 

	do 
	{
		ASSERT( trace_context != NULL ); 

		if( FALSE == is_valid_data_context_type( trace_context->data_type ) )
		{
			ret = FALSE; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

INLINE NTSTATUS validate_data_cont( PVOID data_cont, data_context_type data_type )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( data_cont != NULL ); 
		ASSERT( TRUE == is_valid_data_context_type( data_type ) ); 

		switch( data_type )
		{
#ifdef DRIVER
		case DATA_CONTEXT_IRP:
			if( ( ( IRP* )data_cont )->Type != IO_TYPE_IRP )
			{
				ntstatus = STATUS_INVALID_PARAMETER_1; 
			}
			break; 
		case DATA_CONTEXT_MDL:
			if( ( CSHORT )( sizeof( MDL ) + sizeof( PFN_NUMBER ) ) < ( ( MDL* )data_cont )->Size )
			{
				ntstatus = STATUS_INVALID_PARAMETER_2; 
			}
			break; 
#endif //DRIVER
			//注意用户层缓存区的访问异常问题。
		case DATA_CONTEXT_BUF:
			if( data_cont == NULL )
			{
				ntstatus = STATUS_INVALID_PARAMETER_3; 
			}
			break; 
		default:
			ASSERT( FALSE && "invalid data context type" ); 
			ntstatus = STATUS_INVALID_PARAMETER_5; 
			break; 
		}
	}while( FALSE ); 

	return ntstatus; 
}

INLINE NTSTATUS safe_copy_data( PVOID dst, PVOID src, ULONG size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{			
		if( ExGetPreviousMode() != UserMode )
		{
			memcpy( dst, src, size ); 
			break; 
		}

		try
		{

			//ProbeForRead( src, size, 1 ); 
			memcpy( dst, src, size ); 
		}
		_except( EXCEPTION_EXECUTE_HANDLER )
		{
#if 0
			DbgBreakPoint(); 
#endif //0
			ntstatus = GetExceptionCode(); 
		}

	} while (FALSE );

	return ntstatus; 
}

INLINE NTSTATUS init_data_trace_context( ULONG data_size, 
										IO_DIRECTION dir, 
										PVOID data_cont, 
										data_context_type data_type, 
										data_trace_context *trace_context )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( data_cont != NULL ); 
		ASSERT( trace_context != NULL ); 
		//ASSERT( data_type != DATA_CONTEXT_NULL ); 
		ASSERT( TRUE == is_valid_io_direction( dir ) ); 
		ASSERT( TRUE == is_valid_data_context_type( data_type ) ); 

		trace_context->data_context.data = NULL; 
		trace_context->data_dir = dir; 
		trace_context->data_size = 0; 
		trace_context->data_type = DATA_CONTEXT_NULL; 

		ntstatus = validate_data_cont( data_cont, data_type ); 
		if( STATUS_SUCCESS != ntstatus )
		{
			break; 
		}

		trace_context->data_context.data = data_cont; 
		trace_context->data_dir = dir; 
		trace_context->data_type = data_type; 
		trace_context->data_size = data_size; 

	}while( FALSE ); 

	return ntstatus; 
}

INLINE NTSTATUS get_action_data_from_context( data_trace_context *context, PVOID *data_out, ULONG *data_len_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	PVOID data_buf; 
	ULONG data_len; 

	ASSERT( context != NULL ); 
	ASSERT( data_out != NULL ); 
	ASSERT( data_len_out != NULL ); 

	do 
	{
		//if( context == NULL )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER_1; 
		//	break; 
		//}

		switch( context->data_type )
		{
		case DATA_CONTEXT_IRP:
			ASSERT( context->data_context.irp != NULL ); 

			ntstatus = get_data_buf_from_irp( context->data_context.irp, 
				context->data_dir, 
				0, 
				&data_buf, 
				&data_len ); 

			if( context->data_size < data_len )
			{
				data_len = context->data_size; 
			}
			break; 
		case DATA_CONTEXT_MDL: 
			ntstatus = get_data_buf_from_mdl( context->data_context.mdl, &data_buf, &data_len ); 

			if( context->data_size < data_len )
			{
				data_len = context->data_size; 
			}
			break; 
		case DATA_CONTEXT_BUF: 
			data_buf = context->data_context.data; 
			data_len = context->data_size; 
			break; 
		default:
			//data_buf = NULL; 
			//data_len = 0; 
			ntstatus = STATUS_INVALID_PARAMETER; 
			ASSERT( FALSE && "invalid data context type" ); 
			break; 
		}

		if( ntstatus != STATUS_SUCCESS )
		{
			ASSERT( data_buf == NULL ); 
			ASSERT( data_len == 0 ); 
			//ASSERT( data_buf == NULL || data_size == 0 ); 
			*data_out = NULL; 
			*data_len_out = 0; 

			log_trace( ( MSG_ERROR, "get the io data error 0x%0.8x\n", ntstatus ) ); 
			break; 
		}

		ASSERT( data_buf != NULL ); 
		ASSERT( data_len != 0 ); 

		*data_out = data_buf; 
		*data_len_out = data_len; 

	} while ( FALSE ); 

	return ntstatus; 
}

typedef struct _sys_action_filter_info
{
	action_response_type resp; 
	data_trace_context data_trace; 
}sys_action_filter_info, *psys_action_filter_info; 

extern ULONG action_trace_flags; 

INLINE NTSTATUS set_global_trace_flags( ULONG trace_flags )
{
	action_trace_flags = trace_flags; 
	return STATUS_SUCCESS; 
}

INLINE BOOLEAN check_action_need_log( action_response_type response, ULONG rule_trace_mode, ULONG trace_mode )
{
	BOOLEAN need_log = FALSE; 

	do 
	{
		if( FALSE == is_valid_trace_mode( trace_mode ) )
		{
			ASSERT( FALSE && "how can input the invalid trace mode to checking." ); 
			break; 
		}

		if( response == ACTION_LEARN )
		{
			if( 0 != ( DATA_TRACE_RET_LEARN & action_trace_flags ) )
			{
				need_log = TRUE; 
				break; 
			}
		}

		switch( DATA_TRACE_RET_TYPE ( action_trace_flags ) )
		{
		case DATA_TRACE_RET_ALL:
			need_log = TRUE; 
			break; 
		case DATA_TRACE_RET_ERROR:
			if( response == ACTION_BLOCK )
			{
				need_log = TRUE; 
				break; 
			}
			break; 

		case DATA_TRACE_RET_SUCCESS:
			if( response == ACTION_ALLOW)
			{
				need_log = TRUE; 
				break; 
			}
			break; 

		case DATA_TRACE_RET_BY_SETTING:
			{
				if( 0 == ( DATA_TRACE_MODE( rule_trace_mode ) & trace_mode ) )
				{
					break; 
				}

				switch( DATA_TRACE_RET_TYPE( rule_trace_mode ) )
				{
				case DATA_TRACE_RET_ALL:
					need_log = TRUE; 
					break; 
				case DATA_TRACE_RET_ERROR:
					if( response == ACTION_BLOCK )
					{
						need_log = TRUE; 
						break; 
					}
					break; 

				case DATA_TRACE_RET_SUCCESS:
					if( response == ACTION_ALLOW)
					{
						need_log = TRUE; 
						break; 
					}
					break; 
				default:
					ASSERT( FALSE && "unknown data trace return type" ); 
					break; 
				}
			}
			break; 
		default:
			ASSERT( FALSE && "unknown data trace return type for global data trace setting" ); 
			break; 
		}
	}while( FALSE );

	return need_log; 
}

INLINE RESULT init_def_trace_option( data_trace_option *trace_option )
{
	RESULT ret = ERROR_SUCCESS; 
	access_rule_type rule_type; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	do 
	{
		ASSERT( trace_option  != NULL ); 

		trace_option->trace_mode = DEF_TRACE_MODE; 
		trace_option->tracing_size = DEF_TRACE_DATA_SIZE; 
	}while( FALSE );

	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 

	return ret; 
}

INLINE NTSTATUS __allocate_relating_data( HANDLE proc_handle, /*ULONG protect, */data_alloc_mode mode, ULONG data_len, relating_data *data_buf_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	PVOID data_buf = NULL; 

	do 
	{
		ASSERT( data_buf_out != NULL ); 
		ASSERT( data_len > 0 ); 

		data_buf_out->data = NULL; 
		data_buf_out->data_len = 0; 
		data_buf_out->mode = DATA_BUF_INVALID_ALLOC_MODE; 

		switch( mode )
		{
		case DATA_BUF_MAPPED_TO_USER: 
			{
				SIZE_T _data_len; 

				if( is_valid_proc_handle( proc_handle ) == FALSE )
				{
					ntstatus = STATUS_INVALID_PARAMETER; 
					break; 
				}

				_data_len = data_len; 

				data_buf_out->data = NULL; 
				data_buf_out->data_len = 0; 

				ntstatus = ZwAllocateVirtualMemory( proc_handle, &data_buf, 0, &_data_len, MEM_COMMIT /*| MEM_TOP_DOWN | MEM_RESERVE | MEM_RESET */, PAGE_EXECUTE_READWRITE /*| PAGE_WRITECOMBINE*/ ); 
				if( ntstatus != STATUS_SUCCESS )
				{
					break; 
				}

				ASSERT( data_buf != NULL ); 
				ASSERT( _data_len != 0 ); 

				data_buf_out->data_len = ( ULONG )data_len; 
				data_buf_out->data = data_buf; 
				data_buf_out->mode = mode; 
			}
			break; 
		case DATA_BUF_ALLOCATE_IN_KERNEL_PAGED:
		case DATA_BUF_ALLOCATE_IN_KERNEL_NOPAGED:
			{
				data_buf = ( BYTE* )alloc_pool( mode == DATA_BUF_ALLOCATE_IN_KERNEL_PAGED ? PagedPool : NonPagedPool, 
					data_len );

				if( data_buf == NULL )
				{
					ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
					break; 
				} 

				data_buf_out->data_len = ( ULONG )data_len; 
				data_buf_out->data = data_buf; 
				data_buf_out->mode = mode; 
			}
			break; 
		default:
			ASSERT( FALSE && "invalid data allocating mode" ); 
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		//data_buf_out->proc_id = proc_id; 

		//data_buf_out->data_len = ( ULONG )_data_len; 
	} while ( FALSE ); 

	return ntstatus; 
}

INLINE NTSTATUS release_relating_data_buf( HANDLE proc_handle, relating_data *data_buf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		//ASSERT( proc_handle != NULL ); 
		ASSERT( data_buf != NULL ); 
		ASSERT( TRUE == is_valid_data_alloc_mode( data_buf->mode ) ); 

		if( data_buf->data == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		if( data_buf->data_len == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			ASSERT( FALSE && "why data length is 0" ); 
			//break; 
		}

		switch( data_buf->mode )
		{
		case DATA_BUF_MAPPED_TO_USER:
			{
				ASSERT( TRUE == is_valid_proc_handle( proc_handle ) ); 

				ntstatus = ZwFreeVirtualMemory( proc_handle, data_buf->data, 0, MEM_RELEASE ); 
			}
			break; 

		case DATA_BUF_ALLOCATE_IN_KERNEL_PAGED: 
		case DATA_BUF_ALLOCATE_IN_KERNEL_NOPAGED:
			{
				free_pool( data_buf->data ); 
			}
			break; 
		default:
			ASSERT( FALSE && "invalid data allocating mode" ); 
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

	}while( FALSE );

	return ntstatus; 
}

INLINE NTSTATUS release_sys_action_info( sys_action_info *action_info )
{

	ASSERT( action_info != NULL ); 

	//if( action_info->ctx.sid_a != NULL )
	//{
	//	FREE_TAG_POOL( action_info->ctx.sid_a );
	//}

	FREE_TAG_POOL( action_info ); 

	return STATUS_SUCCESS; 
}

#endif //_DRIVER

INLINE RESULT is_valid_trace_option( data_trace_option *trace_option )
{
	RESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( trace_option != NULL ); 

		if( FALSE == is_valid_trace_mode( trace_option->trace_mode ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( TRACE_SIZE_MAX_LIMIT < trace_option->tracing_size )
		{
			ret = ERROR_INVALID_PARAMETER_2; 
			break; 
		}

	} while ( FALSE );

	return ret; 
}

INLINE ULONG get_access_rule_param_count( access_rule_type type )
{
	ULONG param_count = 0; 
	switch( type )
	{
	case URL_RULE_TYPE:
		param_count = 2; 
		break; 
	case SOCKET_RULE_TYPE:
		param_count = 5; 
		break; 
	case FILE_RULE_TYPE:
		param_count = 2; 
		break; 
	case REG_RULE_TYPE:
		param_count = 2; 
		break; 
	case COM_RULE_TYPE:
		param_count = 2; 
		break; 
	case COMMON_RULE_TYPE: 
		param_count = 2; 
		break; 
	default:
		ASSERT( FALSE ); 
	}

	return param_count; 
}

INLINE param_define_type get_rule_param_type( access_rule_type type, ULONG index )
{
	param_define_type param_type = INVALID_PARAM_DEFINE_TYPE; 

	switch( type )
	{
	case URL_RULE_TYPE:
		{
			switch( index )
			{
			case 0:
				param_type = APP_DEFINE; 
				break; 
			case 1:
				param_type = URL_DEFINE; 
				break; 
			default:
				break; 
			}
		}
		break; 
	case SOCKET_RULE_TYPE:
		{
			switch( index )
			{
			case 0:
				param_type = APP_DEFINE; 
				break; 
			case 1:
				param_type = IP_DEFINE; 
				break; 
			case 2:
				param_type = PORT_DEFINE; 
				break; 
			case 3:
				param_type = IP_DEFINE; 
				break; 
			case 4:
				param_type = PORT_DEFINE;  
				break; 
			default:
				break; 
			}
		}
		break; 
	case FILE_RULE_TYPE: 
		{
			switch( index )
			{
			case 0:
				param_type = APP_DEFINE; 
				break; 
			case 1:
				param_type = FILE_DEFINE; 
				break; 
			default:
				break; 
			}
		}
		break; 
	case REG_RULE_TYPE:
		{
			switch( index )
			{
			case 0:
				param_type = APP_DEFINE; 
				break; 
			case 1:
				param_type = REG_DEFINE; 
				break; 
			default:
				break; 
			}
		}
		break; 
	case COM_RULE_TYPE:
		{
			switch( index )
			{
			case 0:
				param_type = APP_DEFINE; 
				break; 
			case 1:
				param_type = COM_DEFINE; 
				break; 
			default:
				break; 
			}
		}
		break; 
	case COMMON_RULE_TYPE:
		{
			switch( index )
			{
			case 0:
				param_type = APP_DEFINE; 
				break; 
			case 1:
				param_type = COMMON_DEFINE; 
			default:
				break; 
			}
		}
		break; 
	default:
		ASSERT( FALSE );  
		break; 
	}

	return param_type; 
}

INLINE ULONG calc_name_param_len_wchar_unit( param_define_type type, WCHAR *name )
{
	ULONG name_len = 0; 

	ASSERT( name != NULL ); 

	ASSERT( is_name_param_define_type( type ) == TRUE ); 

#ifdef URL_CHAR_UNIT
	if( type == URL_RULE_TYPE )
	{
		name_len = strlen( ( CHAR* )name ) + sizeof( CHAR ); 
		name_len = ( name_len + sizeof( CHAR ) ) / sizeof( WCHAR ); 
	}
	else
#endif //URL_CHAR_UNIT
	{
		name_len = ( ULONG )wcslen( name ) + ( ULONG )sizeof( WCHAR ); 
	}

	goto _return; 

_return:
	return name_len; 
}

INLINE ULONG get_name_param_len_wchar_unit( param_define_type type )
{
	ULONG buf_len = 0;

	if( type == APP_DEFINE )
	{
		buf_len = _MAX_FILE_NAME_LEN; 
	}
	else if( type == COM_DEFINE )
	{
		buf_len = _MAX_COM_NAME_LEN; 
	}
	else if( type == URL_DEFINE )
	{
#ifdef URL_CHAR_UNIT
		buf_len = _MAX_URL_LEN / sizeof( WCHAR ); 
#else
		buf_len = _MAX_URL_LEN; 
#endif //URL_CHAR_UNIT

	}
	else if( type == FILE_DEFINE )
	{
		buf_len = _MAX_FILE_NAME_LEN; 
	}
	else if( type == REG_DEFINE )
	{
		buf_len = _MAX_REG_PATH_LEN; 
	}	
	else if( type == COMMON_DEFINE )
	{
		buf_len = _MAX_FILE_NAME_LEN; 
	}
	else 
	{
		ASSERT( FALSE && "invalid param define type " ); 
	}

	return buf_len; 
}

#define INVALID_PARAM_OFF ( USHORT )0 
#define TO_PARAM_PTR( action, param_off ) ( ( param_all_desc* )( ( BYTE* )action + param_off ) )

INLINE RESULT get_rule_param_off_from_index( access_rule_type type, action_desc *desc, USHORT *param_out, ULONG index )
{
	RESULT ret = ERROR_SUCCESS; 
	//param_all_desc *param_desc = NULL; 
	USHORT param_off; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( param_out != NULL ); 

	*param_out = 0; 
	switch( type )
	{
	case FILE_RULE_TYPE: 
	case REG_RULE_TYPE:
	case COM_RULE_TYPE:
	case COMMON_RULE_TYPE:
	case URL_RULE_TYPE:
		{
			switch( index )
			{
			case 0:
				param_off = FIELD_OFFSET( action_desc, common.app ); 
				break; 
			case 1:
				param_off = FIELD_OFFSET( action_desc, common.param0 ); 
				break; 
			default:
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}
		break; 
	case SOCKET_RULE_TYPE:
		{
			switch( index )
			{
			case 0:
				param_off = FIELD_OFFSET( action_desc, socket.app ); 
				break; 
			case 1:
				param_off = FIELD_OFFSET( action_desc, socket.src_ip ); 
				break; 
			case 2:
				param_off = FIELD_OFFSET( action_desc, socket.src_port ); 
				break; 
			case 3:
				param_off = FIELD_OFFSET( action_desc, socket.dest_ip ); 
				break; 
			case 4:
				param_off = FIELD_OFFSET( action_desc, socket.dest_port ); 
				break; 
			default:
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}
		break; 

	default:
		ret = ERROR_INVALID_PARAMETER; 
		ASSERT( FALSE );  
		break; 
	}

	*param_out = param_off; 

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

INLINE RESULT get_rule_param_from_index( access_rule_type type, action_desc *desc, param_all_desc **param_out, ULONG index )
{
	RESULT ret = ERROR_SUCCESS; 
	param_all_desc *param_desc = NULL; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( param_out != NULL ); 

	*param_out = NULL; 
	switch( type )
	{
	case FILE_RULE_TYPE: 
	case REG_RULE_TYPE:
	case COM_RULE_TYPE:
	case COMMON_RULE_TYPE:
	case URL_RULE_TYPE:
		{
			switch( index )
			{
			case 0:
				param_desc = ( param_all_desc* )&desc->common.app; 
				break; 
			case 1:
				param_desc = ( param_all_desc* )&desc->common.param0; 
				break; 
			default:
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}
		break; 
	case SOCKET_RULE_TYPE:
		{
			switch( index )
			{
			case 0:
				param_desc = ( param_all_desc* )&desc->socket.app; 
				break; 
			case 1:
				param_desc = ( param_all_desc* )&desc->socket.src_ip; 
				break; 
			case 2:
				param_desc = ( param_all_desc* )&desc->socket.src_port; 
				break; 
			case 3:
				param_desc = ( param_all_desc* )&desc->socket.dest_ip; 
				break; 
			case 4:
				param_desc = ( param_all_desc* )&desc->socket.dest_port; 
				break; 
			default:
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}
		break; 

	default:
		ret = ERROR_INVALID_PARAMETER; 
		ASSERT( FALSE );  
		break; 
	}

	*param_out = param_desc; 

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

INLINE ULONG get_param_define_size( param_define_type type, INT32 is_cls )
{
	ULONG size = 0; 

	if( is_cls == TRUE )
	{
		size = sizeof( class_param_define ); 
		goto _return; 
	}

	switch( type )
	{
	case URL_DEFINE: 
		size = sizeof( url_param_define );  
		break; 
	case IP_DEFINE:
		size = sizeof( ip_param_define ); 
		break; 
	case PORT_DEFINE: 
		size = sizeof( port_param_define ); 
		break; 
	case FILE_DEFINE:
		size = sizeof( file_param_define ); 
		break; 
	case REG_DEFINE:
		size = sizeof( reg_param_define ); 
		break; 
	case COM_DEFINE:
		size = sizeof( com_param_define ); 
		break; 
	case APP_DEFINE: 
		size = sizeof( app_param_define ); 
		break; 
	case COMMON_DEFINE: 
		size = sizeof( common_param_define ); 
		break; 
	default:
		ASSERT( FALSE && "invalid param define type " ); 
		break; 
	}

_return:
	return size; 
}

#define HTTP_PROT_DELIM_UNICODE L"://" 

INLINE RESULT get_domain_name_in_url( LPCWSTR url, LPCWSTR *domain_name, ULONG *domain_name_len )
{
	RESULT ret = ERROR_SUCCESS; 
	LPCWSTR _domain_name; 
	ULONG _domain_name_len; 
	LPCWSTR  _file_path; 

	ASSERT( url != NULL ); 
	ASSERT( domain_name != NULL ? domain_name_len != NULL : domain_name_len == NULL ); 

	if( domain_name != NULL )
	{
		*domain_name = NULL; 
		*domain_name_len = 0; 
	}
	
	_domain_name = wcsstr( url, HTTP_PROT_DELIM_UNICODE ); 
	if( _domain_name == NULL )
	{
		_domain_name = url; 
	}
	else
	{
		_domain_name += CONST_STR_LEN( HTTP_PROT_DELIM_UNICODE ); 
	}

	_file_path = wcschr( _domain_name, '/' ); 

	if( _file_path == NULL )
	{
		_domain_name_len = ( ULONG )wcslen( _domain_name ); 
	}
	else
	{
		_domain_name_len = ( ULONG )( _file_path - _domain_name ); 
	}

	if( _domain_name_len == 0 )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}


	if( domain_name != NULL )
	{
		*domain_name = _domain_name; 
		*domain_name_len = _domain_name_len; 
	}

_return:
	return ret; 
}

INLINE RESULT check_param_define_valid( param_define *param, INT32 is_cls, param_define_type type, ULONG size )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG buf_len; 

	ASSERT( param != NULL ); 
	ASSERT( is_valid_param_define_type( type ) == TRUE ); 

	if( size != 0 
		&& size != get_param_define_size( type, is_cls ) )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( is_cls == TRUE )
	{
		if( *param->cls.class_name == L'\0' )
		{
			log_trace( ( MSG_ERROR, "parameter is class but class name is null\n" ) );
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			goto _return; 
		}

		if( wcsnlen( param->cls.class_name, _MAX_CLASS_NAME_LEN ) == _MAX_CLASS_NAME_LEN )
		{
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			goto _return; 
		}
	}

	if( is_name_param_define_type( type ) == TRUE ) 
	{

		if( *param->common.name == L'\0' )
		{
			if( must_not_mull_param( type ) == TRUE )
			{
				ret = ERROR_INVALID_PARAMETER; 
			}
			else
			{
				ret = ERROR_PARAM_NO_MEANING; 
			}
			goto _return; 
		}

		if( type == URL_DEFINE )
		{
#ifdef URL_CHAR_UNIT
			if( strnlen( param->common.name, buf_len * sizeof( WCHAR ) ) == buf_len * sizeof( WCHAR ) )
			{
				log_trace( ( MSG_ERROR, "parameter length is overflow \n" ) );
				ret = ERROR_INVALID_PARAMETER; 
				goto _return; 
			}

#endif //URL_CHAR_UNIT
			ret = get_domain_name_in_url( ( LPCWSTR )param->url.url, NULL, NULL ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}
		else
		{
			buf_len = get_name_param_len_wchar_unit( type ); 

			if( wcsnlen( param->common.name, buf_len ) == buf_len )
			{
				log_trace( ( MSG_ERROR, "parameter length is overflow \n" ) );
				ret = ERROR_INVALID_PARAMETER; 
				goto _return; 
			}
		}
	}
	else if( type == IP_DEFINE )
	{
		if( param->ip.ip_end == 0 && param->ip.ip_begin == 0 )
		{
			ret = ERROR_PARAM_NO_MEANING; 
			goto _return; 
		}

		if( param->ip.ip_end != 0 
			&& param->ip.ip_end < param->ip.ip_begin )
		{
			log_trace( ( MSG_ERROR, "begin ip greater than end ip\n" ) ); 

			ASSERT( FALSE ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
	}
	else if( type == PORT_DEFINE )
	{
		if( param->port.port_begin == 0 && param->port.port_end == 0 )
		{
			ret = ERROR_PARAM_NO_MEANING; 
			goto _return; 
		}

		if( param->port.port_end != 0 
			&& param->port.port_end < param->port.port_begin )
		{
			log_trace( ( MSG_ERROR, "begin port greater than end port\n" ) ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		if( is_valid_prot_type( param->port.type ) == FALSE ) 
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			log_trace( ( MSG_ERROR, "port have unknown protocol %d\n", param->port.type ) ); 
			goto _return; 
		}
	}
	else
	{
		ASSERT( "invalid param define type" && FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

_return:
	return ret; 
}

INLINE void correct_path_name( LPWSTR path_name, ULONG name_len )
{
	if( path_name[ name_len - 1 ] == L'\\' 
		|| path_name[ name_len - 1 ] == L'/' )
	{
		path_name[ name_len - 1 ] = L'\0'; 
	}
}

INLINE RESULT copy_action_param( param_all_desc *param_src, param_all_desc *param_dest )
{
	RESULT ret = ERROR_SUCCESS;
	ULONG buf_len; 

	param_dest->is_cls = param_src->is_cls; 
	param_dest->type = param_src->type; 
	param_dest->id = param_src->id; 

	if( is_name_param_define_type( param_src->type ) == TRUE 
		|| param_src->is_cls == TRUE )
	{
		ULONG name_len; 
		buf_len = get_name_param_len_wchar_unit( param_src->type ); 
		name_len = ( ULONG )wcsnlen( param_src->common.name, buf_len ); 
		if( name_len == buf_len )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		wcsncpy_s( param_dest->common.name,  
			ARRAYSIZE( param_dest->common.name ), 
			param_src->common.name, 
			_TRUNCATE ); 

		if( is_path_name_param_define_type( param_src->type ) == TRUE )
		{
			correct_path_name( param_dest->common.name, name_len ); 
		}
	}
	else if( param_src->type == IP_DEFINE )
	{
		param_dest->ip.ip_begin = param_src->ip.ip_begin; 
		param_dest->ip.ip_end = param_src->ip.ip_end; 
	}
	else if( param_src->type == PORT_DEFINE )
	{
		param_dest->port.type = param_src->port.type; 
		param_dest->port.port_begin = param_src->port.port_begin; 
		param_dest->port.port_end = param_src->port.port_end; 
	}
	else
	{
		ASSERT( FALSE && "unknown param define type" ); 
	}

_return:
	return ret; 
}

INLINE RESULT copy_action_params( param_all_desc **param_src, param_all_desc **param_dest, ULONG param_num )
{
	RESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	for( i = 0; ( ULONG )i < param_num; i ++ )
	{
		ret = copy_action_param( param_src[ i ], param_dest[ i ] ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

_return:
	return ret; 
}

INLINE RESULT check_ptr_param_desc_valid( param_input_desc* param )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG buf_len; 

	ASSERT( param != NULL ); 
	ASSERT( is_valid_param_define_type( param->type ) == TRUE ); 

	if( param->is_cls == TRUE )
	{
		if( param->cls.class_name == NULL 
			|| *param->cls.class_name == L'\0' )
		{
			log_trace( ( MSG_ERROR, "parameter is class but class name is null\n" ) );
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			goto _return; 
		}

		if( wcsnlen( param->cls.class_name, _MAX_CLASS_NAME_LEN ) == _MAX_CLASS_NAME_LEN )
		{
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			goto _return; 
		}
	}


	if( is_name_param_define_type( param->type ) == TRUE ) 
	{
		if( param->common.name == NULL || *param->common.name == L'\0' )
		{
			if( must_not_mull_param( param->type ) == TRUE )
			{
				ret = ERROR_INVALID_PARAMETER; 
			}
			else
			{
				ret = ERROR_PARAM_NO_MEANING; 
			}
			goto _return; 
		}

		buf_len = get_name_param_len_wchar_unit( param->type ); 

		if( wcsnlen( param->common.name, buf_len ) == buf_len )
		{
			log_trace( ( MSG_ERROR, "parameter length is overflow \n" ) );
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
	}
	else if( param->type == IP_DEFINE )
	{
		if( param->ip.ip_end == 0 && param->ip.ip_begin == 0 )
		{
			ret = ERROR_PARAM_NO_MEANING; 
			goto _return; 
		}

		if( param->ip.ip_end != 0 
			&& param->ip.ip_end < param->ip.ip_begin )
		{
			log_trace( ( MSG_ERROR, "begin ip greater than end ip\n" ) ); 

			ASSERT( FALSE ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
	}
	else if( param->type == PORT_DEFINE )
	{
		if( param->port.port_begin == 0 && param->port.port_end == 0 )
		{
			ret = ERROR_PARAM_NO_MEANING; 
			goto _return; 
		}

		if( param->port.port_end != 0 
			&& param->port.port_end < param->port.port_begin )
		{
			log_trace( ( MSG_ERROR, "begin port greater than end port\n" ) ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		if( is_valid_prot_type( param->port.type ) == FALSE ) 
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			log_trace( ( MSG_ERROR, "port have unknown protocol %d\n", param->port.type ) ); 
			goto _return; 
		}
	}
	else
	{
		ASSERT( "invalid param define type" && FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

_return:
	return ret; 
}

INLINE RESULT check_param_desc_valid( param_all_desc* param )
{
	return check_param_define_valid( ( param_define* )&param->common, param->is_cls, param->type, param->size ); 
}

INLINE RESULT _check_param_desc_valid( param_all_desc* param )
{
	RESULT ret; 
	ret = check_param_define_valid( ( param_define* )&param->common, param->is_cls, param->type, param->size ); 
	if( ret == ERROR_PARAM_NO_MEANING )
	{
		ret = ERROR_SUCCESS; 
	}

	return ret; 
}

INLINE RESULT copy_params( access_rule_type type, action_desc *src_desc, action_desc *dest_desc )
{
	RESULT ret = ERROR_SUCCESS; 
	param_all_desc *src_param; 
	param_all_desc *dest_param; 
	ULONG param_num; 
	INT32 i; 

	param_num = get_access_rule_param_count( type ); 

	for( i = 0; ( ULONG )i < param_num; i ++ )
	{
		ret = get_rule_param_from_index( type, src_desc, &src_param, ( ULONG )i ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = get_rule_param_from_index( type, dest_desc, &dest_param, ( ULONG )i ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

#ifdef DBG
		{
			RESULT _ret; 
			_ret = check_param_desc_valid( src_param ); 
			if( _ret != ERROR_SUCCESS && _ret != ERROR_PARAM_NO_MEANING )
			{
				ASSERT( FALSE ); 
			}
		}
#endif //DBG
		ret = copy_action_param( src_param, dest_param ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

_return:
	return ret;
}

INLINE RESULT check_param_of_ptr_rule_valid( access_rule_type rule_type, param_define_type correct_type, param_input_desc *param, INT32  from_db )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( param != NULL ); 

	if( is_valid_param_define_type( param->type ) == FALSE )
	{
		log_trace( ( MSG_ERROR, "invalid parameter type %d\n", param->type ) ); 
		ret = ERROR_INVALID_PARAMETER; 
		ASSERT( FALSE ); 
		goto _return; 
	}

	if( param->type != correct_type )
	{
		log_trace( ( MSG_ERROR, "parameter type is not correct this is: %d correct is %d\n", param->type, correct_type ) ); 
		ret = ERROR_INVALID_PARAMETER; 
		ASSERT( FALSE ); 
		goto _return; 
	}

	if( from_db == TRUE )
	{
		if( param->id == INVALID_ID_VALUE && param->is_cls == FALSE )
		{
			log_trace( ( MSG_ERROR, "parameter readed from database,but id and class name both is null\n", param->type, correct_type ) );

			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			goto _return; 
		}
	}

	ret = check_ptr_param_desc_valid( param ); 
	if( ret != ERROR_SUCCESS && ret != ERROR_PARAM_NO_MEANING )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

_return:
	return ( RESULT )ret; 
}

INLINE RESULT check_param_of_rule_valid( access_rule_type rule_type, param_define_type correct_type, param_all_desc *param, INT32  from_db )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( param != NULL ); 

	if( is_valid_param_define_type( param->type ) == FALSE )
	{
		log_trace( ( MSG_ERROR, "invalid parameter type %d\n", param->type ) ); 
		ret = ERROR_INVALID_PARAMETER; 
		ASSERT( FALSE ); 
		goto _return; 
	}

	if( param->type != correct_type )
	{
		log_trace( ( MSG_ERROR, "parameter type is not correct this is: %d correct is %d\n", param->type, correct_type ) ); 
		ret = ERROR_INVALID_PARAMETER; 
		ASSERT( FALSE ); 
		goto _return; 
	}

	if( from_db == TRUE )
	{
		if( param->id == INVALID_ID_VALUE && param->is_cls == FALSE )
		{
			log_trace( ( MSG_ERROR, "parameter readed from database,but id and class name both is null\n", param->type, correct_type ) );

			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			goto _return; 
		}
	}

	ret = check_param_desc_valid( param ); 
	if( ret != ERROR_SUCCESS && ret != ERROR_PARAM_NO_MEANING )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

_return:
	return ( RESULT )ret; 
}

INLINE access_rule_type acl_type( sys_action_type action_type )
{
	access_rule_type rule_type = COMMON_RULE_TYPE; 

	ASSERT( is_valid_action_type( action_type ) ); 
	if( is_valid_action_type( action_type ) == FALSE )
	{
	}

	switch( action_type )
	{
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	case INSTALL_DRV: 
	case INSTALL_HOOK:
	case CREATE_PROC:
	case ACCESS_MEM: 
#endif //COMPATIBLE_OLD_ACTION_DEFINE
	
	case OTHER_ACTION:
	case EXEC_create://进程启动 进程路径名 （执行监控） 
	case EXEC_destroy://进程退出 进程路径名 
	case EXEC_module_load: //模块加载 模块路径名 
		//MT_procmon: 
	case PROC_exec: //创建进程 目标进程路径名  （进程监控）
	case PROC_open: //打开进程 目标进程路径名 
	case PROC_debug: //调试进程 目标进程路径名 
	case PROC_suspend: //挂起进程 目标进程路径名 
	case PROC_resume: //恢复进程 目标进程路径名 
	case PROC_exit: //结束进程 目标进程路径名 
	case PROC_job: //将进程加入工作集 目标进程路径名 
	case PROC_pgprot: //跨进程修改内存属性 目标进程路径名 
	case PROC_freevm: //跨进程释放内存 目标进程路径名 
	case PROC_writevm: //跨进程写内存 目标进程路径名 
	case PROC_readvm: //跨进程读内存 目标进程路径名 
	case THRD_remote: //创建远程线程 目标进程路径名 
	case THRD_setctxt: //跨进程设置线程上下文 目标进程路径名 
	case THRD_suspend: //跨进程挂起线程 目标进程路径名 
	case THRD_resume: //跨进程恢复线程 目标进程路径名 
	case THRD_exit: //跨进程结束线程 目标进程路径名 
	case THRD_queue_apc: //跨进程排队APC 目标进程路径名 

		//MT_sysmon
	case SYS_settime: //设置系统时间 无 
	case SYS_link_knowndll: //建立KnownDlls链接 链接文件名 
	case SYS_open_physmm: //打开物理内存设备 无 
	case SYS_read_physmm: //读物理内存 无 
	case SYS_write_physmm: //写物理内存 无 
	case SYS_load_kmod: //加载内核模块 内核模块全路径 
	case SYS_enumproc: //枚举进程 无 
	case SYS_regsrv: //注册服务 服务进程全路径 
	case SYS_opendev: //打开设备 设备名 

		//MT_w32mon
	case W32_postmsg: //发送窗口消息（Post） 目标进程路径名 
	case W32_sendmsg: //发送窗口消息（Send） 目标进程路径名 
	case W32_findwnd: //查找窗口 无 
	case W32_msghook: //设置消息钩子 无 
	case W32_lib_inject: //DLL注入 注入DLL路径名 

		//MT_behavior: 
	case BA_extract_hidden: //释放隐藏文件 释放文件路径名 （行为监控） 
	case BA_extract_pe: //释放PE文件 释放文件路径名 
	case BA_self_copy: //自我复制 复制目标文件路径名 
	case BA_self_delete: //自我删除 删除文件路径名 
	case BA_ulterior_exec: //隐秘执行 被执行映像路径名 
	case BA_invade_process: //入侵进程 目标进程路径名 
	case BA_infect_pe: //感染PE文件 目标文件路径名 
	case BA_overwrite_pe: //覆写PE文件 目标文件路径名 
	case BA_register_autorun: //注册自启动项 自启动文件路径名 
	//case BA_other: 

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
		ASSERT( action_type == INSTALL_DRV ); 
#endif //COMPATIBLE_OLD_ACTION_DEFINE

		rule_type = COMMON_RULE_TYPE; 
		break; 
	//MT_common
	case COM_access: 

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	case ACCESS_COM:
		rule_type = COM_RULE_TYPE; 
		break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	case MODIFY_KEY:
#endif //COMPATIBLE_OLD_ACTION_DEFINE

	//MT_regmon: 
	case REG_openkey: //打开注册表键 注册表键路径  （注册表监控） 
	case REG_mkkey: //创建注册表键 注册表键路径 
	case REG_rmkey: //删除注册表键 注册表键路径 
	case REG_mvkey: //重命名注册表键 注册表键路径 
	case REG_getinfo: 
	case REG_setinfo: 
	case REG_rmval: //删除注册表键 注册表键路径 
	case REG_getval: //获取注册表值 注册表值路径 
	case REG_setval: //设置注册表值 注册表值路径 
	case REG_loadkey: //挂载注册表Hive文件 注册表键路径 
	case REG_replkey: //替换注册表键 注册表键路径 
	case REG_rstrkey: //导入注册表Hive文件 注册表键路径 
	case REG_setsec: //设置注册表键安全属性 注册表键路径 
	case REG_closekey:
		rule_type = REG_RULE_TYPE; 
		break; 
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	case MODIFY_FILE:
	case DELETE_FILE:
#endif //COMPATIBLE_OLD_ACTION_DEFINE

		//MT_filemon: 
	case FILE_touch: //创建文件 文件全路径 （文件监控） 
	case FILE_open: //打开文件 文件全路径 
	case FILE_read: //读取文件 文件全路径 
	case FILE_write: //写入文件 文件全路径 
	case FILE_modified: //文件被修改 文件全路径 
	case FILE_readdir: //遍历目录 目录全路径 
	case FILE_remove: //删除文件 文件全路径 
	case FILE_rename: //重命名文件 文件全路径 
	case FILE_truncate: //截断文件 文件全路径 
	case FILE_mklink: //建立文件硬链接 文件全路径 
	case FILE_chmod: //设置文件属性 文件全路径 
	case FILE_setsec: //设置文件安全属性 文件全路径 
	case FILE_getinfo:
	case FILE_setinfo:
		rule_type = FILE_RULE_TYPE; 
		break; 

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	case SOCKET_CONNECT:
	case SOCKET_SEND:
	case SOCKET_RECV:
#endif //COMPATIBLE_OLD_ACTION_DEFINE

		//MT_netmon: 
	case NET_create: 
	case NET_connect: //网络连接 远程地址（格式：IP：端口号） （网络监控） 
	case NET_listen: //监听端口 本机地址（格式：IP：端口号） 
	case NET_send: //发送数据包 远程地址（格式：IP：端口号） 
	case NET_recv: 
	case NET_accept: 

	case NET_icmp_send: 
	case NET_icmp_recv: 
		rule_type = SOCKET_RULE_TYPE; 

		break; 
	case NET_http: //HTTP请求 HTTP请求路径（格式：域名/URL） 
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	case LOCATE_URL:
		rule_type = URL_RULE_TYPE; 
		break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE

	default:
		ASSERT( "invalid sys action type" && FALSE ); 
		rule_type = COMMON_RULE_TYPE; 
		break; 
	}

	return rule_type; 
}

INLINE LPCWSTR get_rule_type_desc( access_rule_type type )
{
	WCHAR *type_desc = L""; 
	switch( type )
	{
	case URL_RULE_TYPE:
		type_desc = L"URL_RULE_TYPE"; 
		break; 
	case SOCKET_RULE_TYPE:
		type_desc = L"SOCKET_RULE_TYPE"; 
		break; 
	case FILE_RULE_TYPE:
		type_desc = L"FILE_RULE_TYPE"; 
		break; 
	case REG_RULE_TYPE:
		type_desc = L"REG_RULE_TYPE"; 
		break; 
	case COM_RULE_TYPE:
		type_desc = L"COM_RULE_TYPE"; 
		break; 
	case COMMON_RULE_TYPE:
		type_desc = L"COMMON_RULE_TYPE"; 
		break; 
	default:
		ASSERT( FALSE ); 
		type_desc = L"UNKOWN_RULE_TYPE"; 
		break; 
	}
	return type_desc; 
}

INLINE LPCWSTR get_param_type_desc( param_define_type type )
{
	LPCWSTR type_desc = L""; 

	switch( type )
	{
	case URL_DEFINE: 
		type_desc = L"URL_DEFINE"; 
		break; 
	case IP_DEFINE: 
		type_desc = L"IP_DEFINE"; 
		break; 
	case PORT_DEFINE:
		type_desc = L"PORT_DEFINE"; 
		break; 
	case FILE_DEFINE: 
		type_desc = L"FILE_DEFINE"; 
		break; 
	case REG_DEFINE: 
		type_desc = L"REG_DEFINE"; 
		break; 
	case COM_DEFINE: 
		type_desc = L"COM_DEFINE"; 
		break; 
	case APP_DEFINE: 
		type_desc = L"APP_DEFINE"; 
		break; 
	default:
		ASSERT( FALSE ); 
		break; 
	}

	return type_desc; 
}

INLINE RESULT _check_param_valid( access_rule_type rule_type, param_define_type correct_type, param_all_desc *param )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG buf_len; 

	ASSERT( param != NULL ); 

	if( is_valid_param_define_type( param->type ) == FALSE )
	{
		log_trace( ( MSG_ERROR, "invalid parameter type %d\n", param->type ) );

		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( param->type != correct_type )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( param->id == INVALID_ID_VALUE && param->is_cls == TRUE )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( param->is_cls == TRUE && *param->cls.class_name == L'\0' )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	buf_len = get_name_param_len_wchar_unit( param->type ); 
	if( calc_name_param_len_wchar_unit( param->type, param->common.name ) >= buf_len )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	if( rule_type != COMMON_RULE_TYPE 
		&& rule_type != SOCKET_RULE_TYPE  
		&& param->type != APP_DEFINE )
	{
		if( *param->common.name == L'\0' )
		{
			ret = ERROR_PARAM_NO_MEANING; 
			goto _return; 
		}
	}

_return:
	return ret; 
}

INLINE RESULT check_params_ptr_of_rule_valid( access_rule_type rule_type, action_desc *desc )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG param_num; 
	param_define_type param_type; 
	param_all_desc *param_desc; 
	INT32 i; 

	ASSERT( desc != NULL ); 
	param_num = get_access_rule_param_count( rule_type ); 

	for( i = 0; ( ULONG )i < param_num; i ++ )
	{
		param_type = get_rule_param_type( rule_type, i ); 
		if( param_type == INVALID_PARAM_DEFINE_TYPE )
		{
			ASSERT( FALSE ); 
			break; 
		}

		ret = get_rule_param_from_index( rule_type, desc, &param_desc, i ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}

		ASSERT( param_desc != NULL ); 

		if( param_desc == NULL 
			|| desc->params[ i ] != param_desc )
		{
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			break; 
		}
	}

	return ret; 
};

INLINE RESULT check_action_ptr_params_valid( access_rule_type rule_type, action_ptr_desc *desc, INT32 from_db )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG param_num; 
	ULONG no_meaning_param_num; 
	param_define_type param_type; 
	INT32 i; 

	if( is_valid_access_rule_type( rule_type ) == FALSE )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( rule_type == SOCKET_RULE_TYPE )
	{
		if( desc->socket.dest_port.port.type != desc->socket.src_port.port.type )
		{
			log_trace( ( MSG_ERROR, "the socket rule source protocol type and destination protocol type is not same\n") ); 
			ret = ERROR_SOCKET_SOURCE_DEST_PROT_NOT_SAME; 
			goto _return; 
		}
	}

	param_num = get_access_rule_param_count( rule_type ); 

	no_meaning_param_num = 0; 

	for( i = 0; ( ULONG )i < MAX_RULE_PARAM_NUM; i ++ )
	{
		if( ( ULONG )i < param_num )
		{
			param_type = get_rule_param_type( rule_type, i ); 
			if( param_type == INVALID_PARAM_DEFINE_TYPE )
			{
				ASSERT( FALSE ); 
				break; 
			}

			ret = check_param_of_ptr_rule_valid( rule_type, param_type, &desc->params[ i ], from_db ); 
			if( ret != ERROR_SUCCESS )
			{
				if( ret == ERROR_PARAM_NO_MEANING )
				{
					no_meaning_param_num ++; 
				}
				else
				{
					log_trace( ( MSG_ERROR, "parameter %d is invalid \n", i ) ); 
					ASSERT( FALSE ); 
					goto _return; 
				}
			}
		}
		else
		{
			if( desc->params[ i ].id != INVALID_ID_VALUE )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				goto _return; 
			}

			if( desc->params[ i ].is_cls == TRUE )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				goto _return; 
			}

			if( desc->params[ i ].type != INVALID_PARAM_DEFINE_TYPE )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				goto _return; 
			}

			if( desc->params[ i ].common.name != NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				goto _return; 
			}
		}
	}

	if( no_meaning_param_num == param_num )
	{
		ret = ERROR_RULE_NO_MEANING; 
		goto _return; 
	}

	ret = ERROR_SUCCESS; 
_return:
	return ret; 
}

INLINE RESULT check_action_params_valid( access_rule_type rule_type, action_desc *desc, INT32 from_db )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG param_num; 
	ULONG no_meaning_param_num; 
	INT32 meaning_param_error; 
	param_define_type param_type; 
	param_all_desc *param_desc; 
	INT32 i; 

	if( is_valid_access_rule_type( rule_type ) == FALSE )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( rule_type == SOCKET_RULE_TYPE )
	{
		if( desc->socket.dest_port.port.type != desc->socket.src_port.port.type )
		{
			log_trace( ( MSG_ERROR, "the socket rule source protocol type and destination protocol type is not same\n") ); 
			ret = ERROR_SOCKET_SOURCE_DEST_PROT_NOT_SAME; 
			goto _return; 
		}
	}

	param_num = get_access_rule_param_count( rule_type ); 

	no_meaning_param_num = 0; 
	meaning_param_error = FALSE; 
	for( i = 0; ( ULONG )i < param_num; i ++ )
	{
		param_type = get_rule_param_type( rule_type, i ); 
		if( param_type == INVALID_PARAM_DEFINE_TYPE )
		{
			ASSERT( FALSE ); 
			break; 
		}

		ret = get_rule_param_from_index( rule_type, desc, &param_desc, i ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}

		ASSERT( param_desc != NULL ); 

		if( param_desc == NULL 
			|| desc->params[ i ] != param_desc )
		{
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			break; 
		}

		ret = check_param_of_rule_valid( rule_type, param_type, param_desc, from_db ); 
		if( ret != ERROR_SUCCESS )
		{
			if( ret == ERROR_PARAM_NO_MEANING )
			{
				if( TRUE == is_must_meaningful_param( param_type ) )
				{
					meaning_param_error = TRUE; 
					break; 
				}

				no_meaning_param_num ++; 
			}
			else
			{
				log_trace( ( MSG_ERROR, "parameter %d is invalid \n", i ) ); 
				ASSERT( FALSE ); 
				goto _return; 
			}
		}
	}

	if( meaning_param_error == TRUE 
		|| no_meaning_param_num == param_num )
	{
		ret = ERROR_RULE_NO_MEANING; 
		goto _return; 
	}

	ret = ERROR_SUCCESS; 

_return:
	return ret; 
}

INLINE HRESULT init_param( param_define_type type, ULONG size, LPCWSTR cls_name, param_all_desc *param )
{
	HRESULT ret = ERROR_SUCCESS; 
	
	ASSERT( param != NULL ); 

	param->id = INVALID_ID_VALUE; 
	param->type = type; 
	
	if( cls_name == NULL || *cls_name == L'\0' )
	{
		param->is_cls = FALSE; 
		memset( &param->common.name, 0, size ); 
	}
	else
	{
		param->is_cls = TRUE; 
		if( wcsnlen( cls_name, _MAX_CLASS_NAME_LEN ) == _MAX_CLASS_NAME_LEN )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		wcsncpy_s( param->cls.class_name,  
			ARRAYSIZE( param->cls.class_name ), 
			cls_name, 
			_TRUNCATE ); 
	}
	
_return:
	return ret; 
}


INLINE RESULT init_access_desc_param_ptr( access_rule_type type, action_desc *desc )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG param_num; 
	INT32 i; 

	param_num = get_access_rule_param_count( type ); 

	for( i = 0; ( ULONG )i < param_num; i ++ )
	{
		ret = get_rule_param_from_index( type, desc, &desc->params[ i ], i ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}
	}

	return ret; 
}

INLINE RESULT _init_action_desc_param_ptr( sys_action_type type, sys_action_desc *desc )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG param_num; 
	INT32 i; 
	access_rule_type rule_type; 

	ASSERT( TRUE == is_valid_action_type( type ) ); 
	rule_type = acl_type( type ); 

	ASSERT( TRUE == is_valid_access_rule_type( rule_type ) ); 

	param_num = get_access_rule_param_count( rule_type ); 

	for( i = 0; ( ULONG )i < param_num; i ++ )
	{
		ret = get_rule_param_from_index( rule_type, &desc->desc, &desc->desc.params[ i ], i ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}
	}

	return ret; 
}

INLINE RESULT set_action_desc_param_offset( access_rule_type type, action_desc *desc )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG param_num; 
	INT32 i; 

	param_num = get_access_rule_param_count( type ); 

	for( i = 0; ( ULONG )i < param_num; i ++ )
	{
		ret = get_rule_param_from_index( type, desc, &desc->params[ i ], i ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}
	}

	return ret; 
}

INLINE RESULT init_action_desc( access_rule_type type, action_desc *desc )
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG param_num; 
	param_define_type param_type; 
	INT32 i; 

	param_num = get_access_rule_param_count( type ); 

	for( i = 0; ( ULONG )i < ARRAY_SIZE( desc->params ); i ++ )
	{
		if( ( ULONG )i < param_num )
		{
			ret = get_rule_param_from_index( type, desc, &desc->params[ i ], i ); 

			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				break; 
			}

			param_type = get_rule_param_type( type, i ); 

			desc->params[ i ]->id = INVALID_ID_VALUE; 
			desc->params[ i ]->size = ( USHORT )get_param_define_size( param_type, FALSE ); 
			desc->params[ i ]->is_cls = FALSE; 
			desc->params[ i ]->type = param_type; 

			ret = init_param( param_type, desc->params[ i ]->size, NULL, desc->params[ i ] ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				break; 
			}
		}
		else
		{
			desc->params[ i ] = NULL; 
		}
	}

	return ret; 
}

INLINE RESULT init_access_rule( access_rule_type type, access_rule_desc *access_rule )
{
	RESULT ret = ERROR_SUCCESS; 

	ASSERT( access_rule != NULL ); 

	access_rule->type = type; 
	access_rule->resp = ACTION_ALLOW; 

	if( is_valid_access_rule_type( type ) == FALSE )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = init_action_desc( type, &access_rule->desc ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!initialize the action description failed\n" ) ); 
	}
_return:
	return ret; 
}

INLINE RESULT init_sys_action( sys_action_type type, sys_action_desc *sys_action )
{
	RESULT ret = ERROR_SUCCESS; 
	access_rule_type rule_type; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( sys_action  != NULL ); 

	sys_action->type = type; 
	sys_action->resp = ACTION_ALLOW; 
	sys_action->id = INVALID_ACTION_ID; 

#ifdef ACTION_LOG_ALLOC_FROM_R3_MAPPED_BUF
	sys_action->data = NULL; 
	sys_action->data.data_len = 0; 
	sys_action->data.mode = DATA_BUF_INVALID_ALLOC_MODE; 
#endif //ACTION_LOG_ALLOC_FROM_R3_MAPPED_BUF

	//init_def_trace_option( &sys_action->trace_option ); 

	if( is_valid_action_type( type ) == FALSE )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	rule_type = acl_type( type ); 

	ret = init_action_desc( rule_type, &sys_action->desc ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!initialize the action description failed\n" ) ); 
	}

_return: 
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}


INLINE RESULT check_sys_action_input_valid( sys_action_desc *cur_action )
{
	RESULT ret = ERROR_SUCCESS; 
	access_rule_type rule_type; 

	ASSERT( cur_action != NULL ); 

	if( is_valid_action_type( cur_action->type ) == FALSE )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	rule_type = acl_type( cur_action->type ); 

	ret = check_action_params_valid( rule_type, &cur_action->desc, FALSE ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "action params is invalid\n" ) ); 
	}

_return:
	return ret; 
}

INLINE RESULT check_access_rule_input_valid( access_rule_desc *rule_input, INT32 from_db )
{
	RESULT ret = ERROR_SUCCESS; 

	ASSERT( rule_input != NULL ); 

	if( is_valid_access_rule_type( rule_input->type ) == FALSE )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	if( is_valid_response_type( rule_input->resp ) == FALSE )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}; 

#ifdef DBG
	//init_access_desc_param_ptr( rule_input->type, &rule_input->desc ); 
#endif //DBG

	ret = check_action_params_valid( rule_input->type, &rule_input->desc, from_db ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "action params is invalid\n" ) ); 
	}
_return:
	return ret; 
}

INLINE RESULT check_access_rule_ptr_input_valid( access_rule_ptr_desc *rule_input, INT32 from_db )
{
	RESULT ret = ERROR_SUCCESS; 

	ASSERT( rule_input != NULL ); 

	if( is_valid_access_rule_type( rule_input->type ) == FALSE )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	if( is_valid_response_type( rule_input->resp ) == FALSE )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}; 

	ret = check_action_ptr_params_valid( rule_input->type, &rule_input->desc, from_db ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "action params is invalid\n" ) ); 
	}
_return:
	return ret; 
}

INLINE ULONG get_param_buf_len( access_rule_type type, ULONG index )
{
	param_define_type param_type; 
	param_type = get_rule_param_type( type, index ); 

	return get_name_param_len_wchar_unit( param_type ); 
}

INLINE RESULT check_struct_align()
{
	RESULT ret = ERROR_SUCCESS; 
	ULONG offset1; 
	ULONG offset2; 
	ULONG offset3; 

	offset1 = FIELD_OFFSET( param_all_desc, app ); 
	offset2 = FIELD_OFFSET( param1_desc, app ); 
	offset3 = FIELD_OFFSET( param2_desc, common ); 
	
	if( offset1 != offset2 || offset1 != offset3 )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		ASSERT( FALSE ); 
	}

	return ret; 
}

INLINE RESULT _validate_url( LPCWSTR url, ULONG url_len )
{
	RESULT ret = ERROR_SUCCESS; 

	return ret; 
}

INLINE RESULT parse_url( LPWSTR url, 
				  ULONG url_len, 
				  ULONG buf_len, 
				  USHORT *domain_name, 
				  USHORT *domain_name_len, 
				  USHORT *file_path, 
				  USHORT *file_path_len )
{
	RESULT ret = ERROR_SUCCESS; 
	WCHAR *_domain_name; 
	ULONG _domain_name_len; 
	WCHAR *_file_path; 
	ULONG _file_path_len; 

	ASSERT( url != NULL ); 
	ASSERT( url_len > 0 ); 
	ASSERT( url_len < 65535 ); 
	ASSERT( domain_name != NULL ); 
	ASSERT( domain_name_len != NULL ); 
	ASSERT( file_path != NULL ); 
	ASSERT( file_path_len != NULL ); 

	log_trace( ( MSG_INFO, "enter %s url %ws \n", __FUNCTION__, url ) ); 

	*file_path = 0; 
	*file_path_len = 0; 
	*domain_name = 0; 
	*domain_name_len = 0; 

#ifdef _DEBUG
	_validate_url( url, url_len ); 
#endif
	if( *( url + url_len - 1 ) != L'\0' )
	{
		*( url + url_len - 1 ) = L'\0'; 
	}

	_domain_name = wcsstr( url, HTTP_PROT_DELIM_UNICODE ); 
	if( _domain_name == NULL )
	{
		_domain_name = url; 
	}
	else
	{
		_domain_name += CONST_STR_LEN( HTTP_PROT_DELIM_UNICODE ); 
	}

	_file_path = wcschr( _domain_name, '/' ); 

	if( _file_path == NULL )
	{
		_domain_name_len = ( ULONG )wcslen( _domain_name ) + 1; 
		_file_path_len = 0; 
	}
	else
	{
		_domain_name_len = ( ULONG )( _file_path - _domain_name + 1 ); 
		_file_path_len = ( ULONG )wcslen( _domain_name ) - ( ULONG )( _file_path - _domain_name ) + 1; 
	}

	if( _domain_name_len <= 2 )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	log_trace( ( MSG_INFO, "domain name %ws, file path %ws \n", _domain_name, _file_path != NULL ? _file_path : L"NULL" ) ); 


	if( _domain_name_len + _file_path_len > buf_len ) 
	{
		ret = ERROR_BUFFER_TOO_SMALL; 
		goto _return; 
	}

	memmove( url, _domain_name, ( _domain_name_len - 1 ) * sizeof( WCHAR ) ); 
	*( url + _domain_name_len - 1 ) = L'\0'; 

	log_trace( ( MSG_INFO, "domain name %ws ", url ) ); 
	if( _file_path_len > 0 )
	{
		memmove( url + _domain_name_len, _file_path, _file_path_len * sizeof( WCHAR ) ); 
		ASSERT( *( url + _domain_name_len + _file_path_len - 1 ) == L'\0' ); 

		log_trace( ( MSG_INFO, "file path %ws \n", url + _domain_name_len ) ); 
	}
	else
	{
		log_trace( ( MSG_INFO, "file path is NULL\n" ) ); 
	}

	ASSERT( _domain_name_len > 1 ); 
	ASSERT( _domain_name_len < 0xffff ); 
	ASSERT( _file_path_len < 0xffff ); 

	*domain_name = 0; 
	*domain_name_len = ( USHORT )_domain_name_len - 1; 
	
	if( _file_path_len == 0 )
	{
		*file_path = ( USHORT )( -1 ); 
		*file_path_len = 0; 
	}
	else
	{
		ASSERT( _file_path_len >= 2 ); 
		*file_path = ( USHORT )_domain_name_len; 
		*file_path_len = ( USHORT )_file_path_len - 1; 
	}

_return:

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

INLINE RESULT parse_url_param( param_all_desc *param )
{
	RESULT ret = ERROR_SUCCESS; 

	ASSERT( param != NULL ); 

	ASSERT( param->type == URL_DEFINE ); 
	ret = parse_url( param->url.url, 
		( ULONG )wcslen( param->url.url ) + 1, 
		_MAX_URL_LEN, 
		&param->url.domain_name_off, 
		&param->url.domain_name_len, 
		&param->url.file_path_off, 
		&param->url.file_path_len 
		); 

	return ret; 
}

INLINE INT32 check_access_rule_size_limit()
{
	INT32 ret = FALSE; 
	if( sizeof( sys_action_desc ) > PAGE_SIZE )
	{
		ASSERT( FALSE && "sys_action_desc size greater than page size" ); 
		goto _return; 
	}

	if( sizeof( access_rule_desc ) > PAGE_SIZE )
	{
		ASSERT( FALSE && "access_rule_desc size greater than page size" ); 		
		goto _return; 
	}

	ret = TRUE; 
_return:
	return ret; 
}

INLINE VOID set_param_type_of_action( access_rule_type type, action_desc *action_input )
{
	ASSERT( action_input != NULL ); 

#ifdef DBG
	{
		NTSTATUS ntstatus; 
		ntstatus = check_params_ptr_of_rule_valid( type, action_input ); 
		if( ntstatus != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
		}
	}
#endif //DBG

	action_input->params[ 0 ]->type = APP_DEFINE; 

	switch ( type )
	{
	case URL_RULE_TYPE:
		action_input->params[ 1 ]->type = URL_DEFINE; 
		break; 
	case SOCKET_RULE_TYPE:
		action_input->params[ 1 ]->type = IP_DEFINE; 
		action_input->params[ 2 ]->type = PORT_DEFINE; 
		action_input->params[ 3 ]->type = IP_DEFINE; 
		action_input->params[ 4 ]->type = PORT_DEFINE; 
		break; 
	case FILE_RULE_TYPE:		
		action_input->params[ 1 ]->type = FILE_DEFINE; 
		break; 
	case REG_RULE_TYPE:
		action_input->params[ 1 ]->type = REG_DEFINE; 
		break; 
	case COM_RULE_TYPE:
		action_input->params[ 1 ]->type = COM_DEFINE; 
		break; 
	case COMMON_RULE_TYPE: 
		action_input->params[ 1 ]->type = COMMON_DEFINE; 
		break; 
	}
}

INLINE VOID set_param_type_in_rule( access_rule_desc *rule_input )
{
	ASSERT( rule_input != NULL ); 
	set_param_type_of_action( rule_input->type, &rule_input->desc ); 
}

INLINE VOID set_define_type_in_action( sys_action_desc *action_input )
{
	ASSERT( action_input != NULL ); 

	set_param_type_of_action( acl_type( action_input->type ), &action_input->desc ); 
}

#ifndef _DRIVER
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif

#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
#endif //STATUS_INVALID_PARAMETER

#endif //_DRIVER

#ifndef DRIVER

#ifndef NT_SUCCESS
#define NT_SUCCESS( x ) ( x >= 0 )
#endif //NT_SUCCESS

#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
#endif //STATUS_INVALID_PARAMETER

#endif //DRIVER

#pragma warning( push )
#pragma warning( disable:4995 )

INLINE NTSTATUS generate_socket_action_desc( sys_action_type action_type, LPCWSTR app_name, ULONG src_ip, USHORT src_port, ULONG dest_ip, USHORT dest_port, prot_type prot, sys_action_desc *desc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG buf_len; 

	ASSERT( is_socket_action( action_type ) ); 
	ASSERT( desc != NULL ); 

	ntstatus = init_sys_action( action_type, desc ); 
	if( !NT_SUCCESS( ntstatus  ) )
	{
		goto _return; 
	}

	buf_len = get_param_buf_len( acl_type( action_type ), 0 ); 

	if( wcsnlen( app_name, buf_len ) == buf_len )
	{
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	wcscpy( desc->desc.socket.app.app.app_name, app_name ); 

	desc->desc.socket.src_ip.ip.ip_begin = desc->desc.socket.src_ip.ip.ip_end = src_ip; 
	desc->desc.socket.dest_ip.ip.ip_begin = desc->desc.socket.dest_ip.ip.ip_end = dest_ip; 

	desc->desc.socket.src_port.port.port_begin = desc->desc.socket.src_port.port.port_end = src_port; 
	desc->desc.socket.dest_port.port.port_begin = desc->desc.socket.dest_port.port.port_end = dest_port; 
	desc->desc.socket.dest_port.port.type = desc->desc.socket.src_port.port.type = prot; 

	set_define_type_in_action( desc ); 

_return:
	return ntstatus; 
}

INLINE NTSTATUS generate_name_action_desc( sys_action_type action_type, LPCWSTR app_name, LPCWSTR param, sys_action_desc *desc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG buf_len; 

	ASSERT( desc != NULL ); 
	ASSERT( is_name_action( action_type ) ); 
	ASSERT( app_name != NULL ); 
	ASSERT( is_common_action( action_type )? TRUE : param != NULL ); 

	if( is_common_action( action_type ) == FALSE && param == NULL )
	{
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	ntstatus = init_sys_action( action_type, desc ); 
	if( !NT_SUCCESS( ntstatus  ) )
	{
		goto _return; 
	}

	if( wcsnlen( app_name, ARRAY_SIZE( desc->desc.common.app.app.app_name ) ) 
		== ARRAY_SIZE( desc->desc.common.app.app.app_name ) )
	{
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	wcsncpy_s( desc->desc.common.app.app.app_name, 
		ARRAYSIZE( desc->desc.common.app.app.app_name ), 
		app_name, 
		_TRUNCATE ); 

	if( param != NULL )
	{
		buf_len = get_param_buf_len( acl_type( action_type ), 1 ); 

		if( wcsnlen( param, buf_len ) == buf_len )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			goto _return; 
		}

		wcsncpy_s( desc->desc.common.param0.common.name, 
			ARRAYSIZE( desc->desc.common.param0.common.name ), 
			param, 
			_TRUNCATE ); 
	}

	set_define_type_in_action( desc ); 

_return:
	return ntstatus; 
}

#pragma warning( pop )

#ifndef _WCSNCPY_S
#undef wcsncpy_s 
#endif //_WCSNCPY_S

#endif //__ACL_DEFINE_H__