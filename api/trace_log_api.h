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

#ifndef __TARCE_LOG_API_H__
#define __TARCE_LOG_API_H__

#include "acl_define.h"

#define TRACE_LOG_INTERFACE_NAME 'artb'

#define RING3_INTERFACE_PORT_NAME ( L"\\RING3_PORT" )

#define MIN_MSG_SIZE 1

#define SYSTEM_ACTION_MESSAGE_ID 21
#define MODULE_ACTIVITY_MESSAGE_ID 22

#define LOG_MODE_ALLOW 0x00000001
#define LOG_MODE_BLOCK 0x00000002
#define LOG_MODE_LEARN 0x00000004

#define LOG_MODE_ALL ( LOG_MODE_LEARN | LOG_MODE_BLOCK | LOG_MODE_ALLOW ) 

typedef enum _R3_SHARED_CBUF_TYPE
{
	SYS_LOG_BUF = 0, 
	MAX_R3_CBUFFER_TYPE, 
} R3_SHARED_CBUF_TYPE, *PR3_SHARED_CBUF_TYPE; 

#define is_valid_r3_cbuffer_type( type ) ( type >= SYS_LOG_BUF && type < MAX_R3_CBUFFER_TYPE ) 

typedef enum _R3_SHARED_ARRAY_TYPE
{
	SYS_ACTION_BUF_ARRAY, 
	R3_NOTIFY_BUF_ARRAY, 
	MAX_R3_ARRAY_TYPE
} R3_SHARED_ARRAY_TYPE, *PR3_SHARED_ARRAY_TYPE; 

#define is_valid_r3_array_type( type ) ( type >= SYS_ACTION_BUF_ARRAY && type < MAX_R3_ARRAY_TYPE ) 

#pragma pack( push )
#pragma pack( 1 )
#define DEFAULT_MAX_TRACE_DATA_SIZE 512
#define MAX_TRACE_DATA_SIZE 8192
#define MIN_TRACE_DATA_SIZE 32

#ifndef EVENT_TRACE_CONFIG_DEFINED
#define EVENT_TRACE_CONFIG_DEFINED
typedef struct _event_trace_config
{
	ULONG proc_id; 
	ULONG trace_data_size; 
} event_trace_config, *pevent_trace_config; 
#endif //EVENT_TRACE_CONFIG_DEFINED

typedef struct _ring3_shared_vm_out
{
	PVOID r3_addr; 
	ULONG vm_size; 
} ring3_shared_vm_out, *pring3_shared_vm_out; 

typedef struct _all_shared_vm_out
{
	ring3_shared_vm_out all_shared_vm[ MAX_R3_CBUFFER_TYPE + MAX_R3_ARRAY_TYPE ]; 
} all_shared_vm_out, *pall_shared_vm_out; 

#pragma pack( pop )

#pragma pack( push, 4 )
typedef struct _DATA_FLOW_CONDITIONS
{
	HANDLE proc_id; 
	HANDLE thread_id; 
	sys_action_type type; 
	PVOID object; 
	ULONG cc_object_path_len; 
	WCHAR object_path[ MAX_NATIVE_NAME_SIZE ]; 
} DATA_FLOW_CONDITIONS, *PDATA_FLOW_CONDITIONS; 
#pragma pack( pop )

#define IOCTL_GET_TRACE_MSG CTL_CODE( FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_GET_TRACE_LOG CTL_CODE( FILE_DEVICE_UNKNOWN, 0x802, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )

#define IOCTL_SEVEN_FW_SET_FILTER_IP				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0804, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_SET_WORK_MODE				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0805, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_ADD_FILTER_NAME				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0806, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_DELETE_FILTER_NAME			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0807, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_DELETE_ALL_FILTER_NAME		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0808, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_FIND_FILTER_NAME_TEST		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0809, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_SET_HTTP_FILTER_NAME			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080A, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_UNSET_HTTP_FILTER_NAME		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080B, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_GET_HTTP_FILTER_NAMES		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080C, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_SET_HTTP_FILTER_URL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080D, METHOD_BUFFERED, FILE_ANY_ACCESS ) 
#define IOCTL_SEVEN_FW_UNSET_HTTP_FILTER_URL		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080F, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_SEVEN_FW_GET_HTTP_FILTER_URLS			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0810, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_SET_HTTP_FILTER_URLS			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0811, METHOD_BUFFERED, FILE_ANY_ACCESS ) 
#define IOCTL_SEVEN_FW_UNSET_HTTP_FILTER_URLS		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0812, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_BITSAFE_SET_BP_MATCH_NAME CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0813, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_ADD_PARAM_DEFINE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x866, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_ADD_RULE_DEFINE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x864, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_DEL_RULE_DEFINE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x865, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_MODIFY_RULE_DEFINE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x869, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_ADD_APP_RULE_DEFINE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x867, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_DEL_APP_RULE_DEFINE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x868, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_ADD_APP_ACTION_TYPE_RULE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x86a, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_DEL_APP_ACTION_TYPE_RULE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x86b, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_ASYNC_GET_ACTION_EVENT CTL_CODE( FILE_DEVICE_UNKNOWN, 0x86c, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )
#define IOCTL_ADD_ACTION_EVENT CTL_CODE( FILE_DEVICE_UNKNOWN, 0x861, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_GET_ACTION_EVENT CTL_CODE( FILE_DEVICE_UNKNOWN, 0x862, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )
#define IOCTL_RESPONSE_ACTION_EVENT CTL_CODE( FILE_DEVICE_UNKNOWN, 0x863, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_NOTIFY_DRIVER_INTERFACE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x851, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_REMOVE_DRIVER_INTERFACE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x852, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_REGISTER_DRIVER_INTERFACE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x853, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_DEREGISTER_DRIVER_INTERFACE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x854, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_DRIVER_INTERFACE_CANCEL CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0891, METHOD_BUFFERED, FILE_ANY_ACCESS ) 

#define IOCTL_TRACE_LOG_SET_VOLUME_PATH_MAPPING CTL_CODE( FILE_DEVICE_UNKNOWN, 0x855, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_TRACE_LOG_DEL_VOLUME_PATH_MAPPING CTL_CODE( FILE_DEVICE_UNKNOWN, 0x856, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_NETWORK_BLOCK_ALL	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0870, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_GET_BLOCK_COUNT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0871, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_SEVEN_FW_SET_NOTIFY_EVENTS			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0873, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SEVEN_FW_SET_NOTIFY_EVENT			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0874, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_SEVEN_FW_GET_WORK_MODE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0872, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_SEVENFW_SET_BP_MATCH_NAME CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0875, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_SEVEN_FW_SET_WORK_TIME CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0876, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_START_TRACE_LOG_UI_INTERACT CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0877, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_STOP_TRACE_LOG_UI_INTERACT CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0878, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_BLOCK_PING CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0879, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_RESET_LEARNED_RULE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x087A, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_SET_LOG_MODE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x087B, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_GET_LOG_MODE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x087C, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_GET_USER_DIRECT_MEM CTL_CODE( FILE_DEVICE_UNKNOWN, 0x087D, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_SET_WAIT_RING3_REPLY_TIME CTL_CODE( FILE_DEVICE_UNKNOWN, 0x087E, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_INIT_RING3_R3_VM CTL_CODE( FILE_DEVICE_UNKNOWN, 0x087F, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_UNINIT_RING3_VM CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0880, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_EVENT_TRACE_CONFIG CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0881, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_EVENT_DATA_FLOW_TRACE_CONFIG CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0882, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_GET_EVENTMON_DRIVER_VERSION CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0901, METHOD_BUFFERED, FILE_ANY_ACCESS )

//#define IOCTL_INIT_RING3_LOG_BUF_ARRAY CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0881, METHOD_BUFFERED, FILE_ANY_ACCESS )
//#define IOCTL_UNINIT_RING3_LOG_BUF_ARRAY CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0882, METHOD_BUFFERED, FILE_ANY_ACCESS )

#include "common_api.h"
//#define IOCTL_TRACE_LOG_GET_TRACE_LOG_IF CTL_CODE( FILE_DEVICE_UNKNOWN, 0x852, METHOD_BUFFERED, FILE_ANY_ACCESS )

//typedef struct __TRACE_LOG_DEV_EXTENSION
//{
//	add_new_trace_func trace_log_add_func; 
//	input_trace_log_func trace_log_input_func; 
//} TRACE_LOG_DEV_EXTENSION, *PTRACE_LOG_DEV_EXTENSION; 

#define TRACE_MSG_LENGTH 256 
#define NT_PROCNAMELEN 260
#define PROC_ID_NONE ( ULONG )( -1 )
#define THREAD_ID_NONE ( ULONG )( -1 )
#define MAX_TRACE_MSG_COUNT 4096
#define FLT_IF_NAME_LEN 32

#define LOGGER_NAME_NONE 0

//#ifdef TEST_IN_RING3
//
//#ifndef STATUS_INVALID_PARAMETER 
////#define STATUS_INVALID_PARAMETER ERROR_INVALID_PARAMETER
//#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
//#endif //STATUS_INVALID_PARAMETER 
//
//#endif //TEST_IN_RING3

#define TRACE_LOGGER_SYSTEM 'sysa'
#define TRACE_LOGGER_NETWORK 'lawf'
#define TRACE_LOGGER_TRACE 'cart'

typedef enum __FLT_MODE
{
	proc_name_flt = 0x0001, 
	proc_id_flt = 0x0002, 
	thread_id_flt = 0x0004 
} FLT_MODE, *PFLT_MODE;

typedef enum __FLT_LEVEL
{
	common_flt, 
	detail_flt
} FLT_LEVEL, *PFLT_LEVEL;

typedef enum _notify_event_type
{
	SYS_LOG_EVENT, 
	SYS_ACTION_EVENT, 
	BLOCK_COUNT_EVENT, 
	WORK_MODE_EVENT, 
	MAX_EVENT_TYPE
} notify_event_type; 

typedef enum _TRACING_LEVEL
{
	MESSAGE_TRACING = 0, 
	DATA_TRACING, 
	MAX_TRACING_LEVEL, 
} TRACING_LEVEL, *PTRACING_LEVEL; 

#pragma pack( push )
#pragma pack( 4 )

#pragma warning( push )
#pragma warning( disable:4200 )

typedef struct __TRACE_INFO_OUTPUT
{
	ULONG Size;
	BYTE Msgs[ 0 ];
} TRACE_INFO_OUTPUT, *PTRACE_INFO_OUTPUT;

typedef union __FLT_TYPE
{
	ULONG proc_id; 
	ULONG thread_id; 
	CHAR proc_name[ NT_PROCNAMELEN ]; 
} FLT_TYPE;

typedef struct _sys_log_output
{
	ULONG size;
	sys_log_unit trace_logs[ 0 ]; 
} sys_log_output, *psys_log_output;

#pragma warning( pop)

typedef struct __FLT_SETTING
{
	FLT_MODE mode; 
	FLT_TYPE value; 
} FLT_SETTING, *PFLT_SETTING;

typedef struct _lookup_host_name_request
{
	ULONG ip; 
	CHAR name[ MAX_DOMAIN_NAME_LEN ]; 
} lookup_host_name_request, *plookup_host_name_request; 

typedef enum _relation_info_request
{
	LOOKUP_HOST_NAME, 
	ANALYZE_DNS_PACK
} relation_info_request_type; 

typedef struct _dns_analyze_request
{
	PVOID data; 
	ULONG data_len; 
	INT32 is_send; 
	PVOID context; 
} dns_analyze_request, *pdns_analyze_request; 

typedef struct _action_info_request
{
	union
	{
		lookup_host_name_request name_request; 	
		dns_analyze_request dns_request; 
	}; 
} action_info_request, *paction_info_request; 

typedef ULONG ACTION_ID; 
typedef NTSTATUS ( *release_flt_notify_func )( ULONG logger_id, ULONG logger_name ); 

typedef struct _event_to_notify
{
	notify_event_type type; 
	HANDLE event_handle; 
} event_to_notify, *pevent_to_notify;

typedef struct _notify_events_set
{
	ULONG event_num; 
	event_to_notify events[ 1 ]; 
} notify_events_set, *pnotify_events_set; 

//#define MAX_TRACE_LOGGER_REF_NUM 6
//typedef enum _MSG_TYPE
//{
//	MSG_ACTION_INFO = 0x00000001, 
//	MSG_ACTION_DATA = 0x00000002,  
//} MSG_TYPE, *PMSG_TYPE; 
//
//typedef struct __MSG_FLT_SETTINGS
//{
//	ULONG ref_count; 
//	release_flt_notify_func release_notify[ MAX_TRACE_LOGGER_REF_NUM ]; 
//	FLT_LEVEL flt_lvl; 
//
//	ULONG proc_id; 
//	ULONG thread_id; 
//	WCHAR proc_name[ NT_PROCNAMELEN ]; 
//} MSG_FLT_SETTINGS, *PMSG_FLT_SETTINGS;

//typedef struct __MSG_FLT
//{
//	release_flt_notify_func release_notify_func; 
//	FLT_LEVEL lvl; 
//	ULONG name; 
//	ULONG setting_num; 
//	FLT_SETTING settings[ 1 ]; 
//} MSG_FLT, *PMSG_FLT; 

#define is_valid_tracing_level( level ) ( level >= MESSAGE_TRACING && level < MAX_TRACING_LEVEL )

#define SHA1_HASH_LEN 20 
typedef struct __FLT_SETTINGS
{
	FLT_LEVEL flt_lvl; 
	ULONG proc_id; 
	ULONG thread_id; 
	BYTE proc_name[ NT_PROCNAMELEN ]; 
	//BYTE image_hash_id[ SHA1_HASH_LEN ]; 
	//ULONG other_info_len; 
	//BYTE other_info[ 1 ]; 
} FLT_SETTINGS, *PFLT_SETTINGS; 

#pragma pack( pop )

//#include "driver_interface.h"

//#define ACTION_RECORD_SIZE_BY_TYPE( type ) ( sizeof( type ) + FIELD_OFFSET( sys_action_info, action ) + sizeof( sys_action_type ) ) //FIELD_OFFSET( sys_action_record,  )
//#define ACTION_RECORD_SIZE_BY_TYPE_APPEND( type, append_size ) ( ACTION_RECORD_SIZE_BY_TYPE( type ) + append_size ) 

#if 0
INLINE VOID free_action_context( action_context *ctx )
{
	ASSERT( ctx != NULL ); 

	if( ctx->sid_a != NULL )
	{
		FREE_TAG_POOL( ctx->sid_a );
	}

	FREE_TAG_POOL( ctx ); 
}
#endif // 0

INLINE LPCWSTR get_notify_event_type( notify_event_type type )
{
	LPCWSTR desc = L"UNKNOWN_NOTIFY_EVENT"; 

	switch( type )
	{
	case SYS_LOG_EVENT:
		desc = L"SYS_LOG_EVENT"; 
		break; 
	case SYS_ACTION_EVENT:
		desc = L"SYS_ACTION_EVENT"; 
		break; 
	case BLOCK_COUNT_EVENT:
		desc = L"BLOCK_COUNT_EVENT"; 
		break; 
	case WORK_MODE_EVENT:
		desc = L"WORK_MODE_EVENT"; 
		break; 
	default:
		break; 
	}
	return desc; 
}

#define is_valid_notify_event_type( type ) ( type >= SYS_LOG_EVENT && type <= WORK_MODE_EVENT )

#define NETWORK_DATA_INBOUND 0x00000001
#define NETWORK_DATA_OUTBOUND 0x00000002

#define TRACE_LOG_DEV_DOS_NAME L"\\DosDevices\\TraceLog"
#define TRACE_LOG_DEV_NAME L"\\Device\\TraceLog"
#define TRACE_LOG_DEV_NAME_APP _T( "\\\\.\\TraceLog" )

#pragma warning( pop )

#endif //__TARCE_LOG_API_H__