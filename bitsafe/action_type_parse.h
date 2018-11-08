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

#ifndef __ACTION_TYPE_PARSE_H__
#define __ACTION_TYPE_PARSE_H__

/***********************************************************************************
must know the type of the member.
***********************************************************************************/

typedef enum _param_type
{
	INT8_TYPE, 
	INT16_TYPE,
	INT32_TYPE, 
	INT64_TYPE, 
	UINT8_TYPE, 
	UINT16_TYPE, 
	UINT32_TYPE, 
	UINT64_TYPE, 
	PTR_TYPE, 
	STRING_PARAM, 
	WSTRING_TYPE, 
	ANSI_STRING_TYPE, 
	UNICODE_STRING_TYPE, 
	DATA_BLOB_TYPE, 
	TIME_PARAM_TYPE, 
	MAX_PARAM_TYPE, 
} param_type, *pparam_type; 

INLINE BOOLEAN is_valid_param_type( param_type type )
{
	return ( type >= INT8_TYPE && type < MAX_PARAM_TYPE ); 
}

#ifndef DRIVER
#ifndef __field_bcount_part_opt
#define __field_bcount_part_opt( ___max, ___len )
#endif //__field_bcount_part_opt

//////////////////////////////////////
typedef USHORT RTL_STRING_LENGTH_TYPE;

typedef struct _STRING {
	__maybevalid USHORT Length;
	__maybevalid USHORT MaximumLength;
#ifdef MIDL_PASS
	[size_is(MaximumLength), length_is(Length) ]
#endif // MIDL_PASS
	__field_bcount_part_opt(MaximumLength, Length) PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;
typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;

typedef STRING OEM_STRING;
typedef PSTRING POEM_STRING;
typedef CONST STRING* PCOEM_STRING;

typedef struct _CSTRING {
	USHORT Length;
	USHORT MaximumLength;
	CONST char *Buffer;
} CSTRING;
typedef CSTRING *PCSTRING;
#define ANSI_NULL ((CHAR)0)     // winnt

typedef STRING CANSI_STRING;
typedef PSTRING PCANSI_STRING;
#endif //DRIVER

/*************************************************************************
PVOID类型与ULONG，ULONGLONG类型关系：
1.PVOID类型的长度是由于编译器编译时定的，但运行时必须有相应
WINDOWS系统位数与硬件平台的支持。
2.所以当驱动与应用编译的位数是一样的，这时对内存和ULONGLONG的处理最高效的。
3.而当驱动与应用的编译的位数是不一样的，就需要一种相应的约定好的转换方式，
或者都定义为兼容性最好的ULONGLONG型。
*************************************************************************/

typedef union _param_data
{
	INT8 int8_val; 
	INT16 int16_val; 
	INT32 int32_val; 
	INT64 int64_val; 
	UINT8 uint8_val; 
	UINT16 uint16_val; 
	UINT32 uint32_val; 
	UINT64 uint64_val; 
	POINTER_TYPE ptr_val; 
	LPSTR string_val; 
	LPWSTR wstring_val; 
	ANSI_STRING ansi_val; 
	UNICODE_STRING unicode_val; 
} param_data, *pparam_data; 

typedef struct _param_info
{
	param_type type; 
	param_data data; 
} param_info, *pparam_info; 
	
#define MAX_PARAMS_COUNT 12 //10 //12

#define set_param_info( __param, __type, member, val ) ( __param ).type = ( __type ); ( __param ).data.member## = ( val );

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern LPCWSTR param_type_fmts[ MAX_PARAM_TYPE ]; 

NTSTATUS parse_exec_create( exec_create *creat, param_info params[], ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_exec_module_load_param( exec_module_load *module_load, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_file_touch_param( file_touch *action, 
								param_info *params, 
								ULONG max_count, 
								ULONG *count_out ); 

NTSTATUS parse_file_open_param( file_open *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_file_read_param( _file_read *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_file_write_param( _file_write *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_thread_remote_param( thrd_remote *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_thread_set_context_param( thrd_setctxt *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_thread_suspend_param( thrd_suspend *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_thread_kill_param( thrd_kill *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_thread_queue_apc_param( thrd_queue_apc *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_open_phys_mm_param( sys_open_physmm *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_read_phys_mm_param( sys_read_physmm *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_load_kmod_param( sys_load_kmod *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_reg_srv_param( sys_regsrv *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_open_dev_param( sys_opendev *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_w32_msg_hook( w32_msghook *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_w32_lib_inject_hook( w32_lib_inject *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_net_connect( net_connect *action, param_info *params, ULONG max_count, ULONG *count_out ); 

#define parse_net_create parse_net_connect
#define parse_net_listen parse_net_connect

NTSTATUS parse_net_send( net_send *action, param_info *params, ULONG max_count, ULONG *count_out ); 

#define parse_net_recv parse_net_send
#define parse_net_accept parse_net_send

NTSTATUS parse_net_http( net_http *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_extract_hidden( ba_extract_hidden *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_extract_pe( ba_extract_pe *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_self_copy( ba_self_copy *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_self_delete( ba_self_delete *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_invade_process( ba_invade_process *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_infect_pe( ba_infect_pe *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_overwrite_pe( ba_overwrite_pe *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_register_autorun( ba_register_autorun *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS get_params_from_action( sys_action_record *action, param_info params[], ULONG max_count, ULONG *count_out ); 

NTSTATUS analyze_action_meaning( sys_action_info *action, sys_action_type *action_mean ); 

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__ACTION_TYPE_PARSE_H__
