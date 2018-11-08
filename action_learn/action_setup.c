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
 #ifdef TEST_IN_RING3

#include "common_func.h"
//#define WIN32_NO_STATUS
#include "ring0_2_ring3.h"
#else
#include "common.h"
#endif //TEST_IN_RING3

#include "acl_define.h"
#include "action_type_parse.h"
#include "action_check.h"
#include "action_setup.h"

#if 0
NTSTATUS exec_create_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action_info = NULL; 
	ULONG action_info_size; 
	action_response_type resp; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( param_count < EXEC_CREATE_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}


		action_info = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( exec_create ) 
			+ ( MAX_PATH << 1 ) 
			+ ( MAX_PATH << 1 ) ); 

		if( action_info == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action_info->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action_info->action.type = EXEC_create; 

		action_info->action.do_exec_create.pid = all_params[ 0 ].data.ptr_val; //目标进程ID 
		action_info->action.do_exec_create.parent_pid = all_params[ 0 ].data.ptr_val; //动作发起者进程ID 
		action_info->action.do_exec_create.image_base = all_params[ 0 ].data.int32_val; 
		action_info->action.do_exec_create.path_len = all_params[ 3 ].data.int32_val;  
		action_info->action.do_exec_create.cmd_len = all_params[ 4 ].data.int32_val; 

		if( action_info->action.do_exec_create.path_len > MAX_PATH )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		if( action_info->action.do_exec_create.cmd_len > MAX_PATH )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}
		
		memcpy( action_info->action.do_exec_create.path_name 
			+ action_info->action.do_exec_create.path_len + 1, 
			all_params[ 6 ].data.wstring_val, 
			( ( action_info->action.do_exec_create.cmd_len + 1 ) << 1 ) ); 

		memcpy( action_info->action.do_exec_create.path_name, 
			all_params[ 5 ].data.wstring_val, 
			( action_info->action.do_exec_create.path_len + 1 ) << 1 ); 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS exec_module_load_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( param_count < EXEC_MODULE_LOAD_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( exec_module_load ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_module_load; 

		action->action.do_exec_module_load.base = all_params[ 0 ].data.uint32_val; //目标进程ID 
		action->action.do_exec_module_load.size = all_params[ 1 ].data.uint32_val; //动作发起者进程ID 
		action->action.do_exec_module_load.path_len = all_params[ 2 ].data.uint16_val; 
				
		if( action->action.do_exec_module_load.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_exec_module_load.path_name, 
			all_params[ 3 ].data.wstring_val, 
			( ( action->action.do_exec_module_load.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS exec_destroy_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( param_count < EXEC_DESTROY_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( exec_destroy ) 
			+ ( ( MAX_PATH + PROCESS_CMD_LEN ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}


		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_proc_action_context( ParentId, 
				&action_info->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_exec_destroy.pid = all_params[ 0 ].data.uint32_val; //目标进程ID 
		action->action.do_exec_destroy.parent_pid = all_params[ 1 ].data.uint32_val; //动作发起者进程ID 
		action->action.do_exec_destroy.path_len = all_params[ 2 ].data.uint16_val; 
		action->action.do_exec_destroy.cmd_len = all_params[ 3 ].data.uint16_val; 
		
		if( action->action.do_exec_destroy.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action->action.do_exec_destroy.cmd_len + 1 > MAX_CMD_LEN )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_exec_destroy.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_exec_destroy.cmd_len + 1 ) << 1 ) ); 

		memcpy( ( action->action.do_exec_destroy.path_name
			+ action->action.do_exec_destroy.path_len + 1 ), 
			all_params[ 5 ].data.wstring_val, 
			( ( action->action.do_exec_destroy.cmd_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}


NTSTATUS file_touch_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( param_count < EXEC_DESTROY_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		//ULONG access; //文件访问权限 
		//ULONG alloc_size; //文件初始长度 
		//ULONG attrib; //文件属性 
		//ULONG share; //文件共享属性 
		//ULONG disposition; //文件打开/创建选项 
		//ULONG options; //文件打开/创建选项 
		//ULONG result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //文件全路径 

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_touch ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = FILE_touch; 

		action->action.do_file_touch.alloc_size = all_params[ 0 ].data.uint32_val; //文件初始长度 
		action->action.do_file_touch.attrib = all_params[ 1 ].data.uint32_val; //文件属性 
		action->action.do_file_touch.share = all_params[ 2 ].data.uint32_val; //文件共享属性 
		action->action.do_file_touch.disposition = all_params[ 3 ].data.uint32_val; //文件打开/创建选项 
		action->action.do_file_touch.options = all_params[ 4 ].data.uint32_val; //文件打开/创建选项 
		//action->action.do_file_touch.result = all_params[ 5 ].data.uint32_val; //动作完成结果(NTSTATUS) 
		action->action.do_file_touch.path_len = all_params[ 5 ].data.uint32_val; 

		if( action->action.do_file_touch.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}		

		memcpy( action->action.do_file_touch.path_name, 
			all_params[ 6 ].data.wstring_val, 
			( ( action->action.do_file_touch.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}


NTSTATUS file_open_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( param_count < FILE_OPEN_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_open ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//ULONG access; //文件访问权限 
		//ULONG alloc_size; //文件初始长度 
		//ULONG attrib; //文件属性 
		//ULONG share; //文件共享属性 
		//ULONG disposition; //文件打开/创建选项 
		//ULONG options; //文件打开/创建选项 
		////ULONG result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = FILE_open; 

		action->action.do_file_open.access = all_params[ 0 ].data.uint32_val; //文件访问权限 
		action->action.do_file_open.alloc_size = all_params[ 1 ].data.uint32_val; //文件初始长度 
		action->action.do_file_open.attrib = all_params[ 2 ].data.uint32_val; //文件属性 
		action->action.do_file_open.share = all_params[ 3 ].data.uint32_val; //文件共享属性 
		action->action.do_file_open.disposition = all_params[ 4 ].data.uint32_val; //文件打开/创建选项 
		action->action.do_file_open.options = all_params[ 5 ].data.uint32_val; //文件打开/创建选项 
		action->action.do_file_open.path_len = all_params[ 6 ].data.uint32_val; 

		if( action->action.do_file_open.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_open.path_name, 
			all_params[ 7 ].data.wstring_val, 
			( ( action->action.do_file_open.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}

//NTSTATUS file_open_setup( param_info all_params[ ], ULONG param_count )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	sys_action_info *action = NULL; 
//	action_response_type resp = ACTION_ALLOW; 
//
//	//ULONG access; //文件访问权限 
//	//ULONG alloc_size; //文件初始长度 
//	//ULONG attrib; //文件属性 
//	//ULONG share; //文件共享属性 
//	//ULONG disposition; //文件打开/创建选项 
//	//ULONG options; //文件打开/创建选项 
//	////ULONG result; //动作完成结果(NTSTATUS) 
//	//PATH_SIZE_T path_len; 
//	//union
//	//{
//	//	WCHAR path_name[ 1 ]; //文件全路径 
//
//	//	struct 
//	//	{
//	//		WCHAR *path_name_ptr; 
//	//	};
//	//};
//
//	do
//	{
//		if( all_params == NULL )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_1; 
//			break; 
//		}
//
//		if( param_count < FILE_OPEN_PARAM_COUNT )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_2;  
//			break; 
//		}
//
//		action = alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_open ) 
//			+ ( MAX_PATH << 1 ) ); 
//
//		if( action == NULL )
//		{
//			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
//			break; 
//		}
//
//		ntstatus = collect_action_context( &action->ctx ); 
//		if( ntstatus != STATUS_SUCCESS )
//		{
//			break; 
//		}
//
//		action->action.do_file_open.path_len = all_params[ 6 ].data.uint16_val; 
//
//		if( action->action.do_file_open.path_len + 1 > MAX_NATIVE_NAME_SIZE )
//		{
//			ntstatus = STATUS_BUFFER_TOO_SMALL; 
//			break; 
//		}
//
//		action->action.type = FILE_open; 
//		action->action.do_file_open.access = all_params[ 0 ].data.uint32_val; 
//		action->action.do_file_open.alloc_size = all_params[ 1 ].data.uint32_val; 
//		action->action.do_file_open.attrib = all_params[ 2 ].data.uint32_val; 
//		action->action.do_file_open.share = all_params[ 3 ].data.uint32_val; 
//		action->action.do_file_open.disposition = all_params[ 4 ].data.uint32_val; 
//		action->action.do_file_open.options = all_params[ 5 ].data.uint32_val; 
//
//		memcpy( action->action.do_file_open.path_name, 
//			all_params[ 7 ].data.wstring_val, 
//			( ( action->action.do_file_open.path_len + 1 ) << 1 ) ); 
//
//	}while( FALSE ); 
//
//	return ntstatus; 
//}


NTSTATUS file_read_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( param_count < FILE_READ_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		//ULONG data_len; //数据长度 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( _file_read ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//{
		//	ULONG pid; 
		//	ULONG parent_pid; 
		//	PATH_SIZE_T path_len; 
		//	PATH_SIZE_T cmd_len; 

		//	union
		//	{
		//		WCHAR path_name[ 1 ]; //进程全路径 

		//		struct 
		//		{
		//			WCHAR *path_name_ptr; 
		//			WCHAR *cmd_line_ptr; 
		//		};
		//	};
		//}

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_file_read.offset = all_params[ 0 ].data.uint32_val; //目标进程ID 
		action->action.do_file_read.data_len = all_params[ 1 ].data.uint32_val; //动作发起者进程ID 
		//action->action.do_file_read.result = all_params[ 2 ].data.uint16_val; 
		action->action.do_file_read.path_len = all_params[ 2 ].data.uint16_val; 
		
		if( action->action.do_file_read.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_read.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_read.path_len + 1 ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}


NTSTATUS file_write_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( param_count < FILE_WRITE_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		//ULONG offset; //文件内偏移 
		//ULONG data_len; //数据长度 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 

		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( _file_write ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_file_write.offset = all_params[ 0 ].data.uint32_val; //目标进程ID 
		action->action.do_file_write.data_len = all_params[ 1 ].data.uint32_val; //动作发起者进程ID 
		action->action.do_file_write.path_len= all_params[ 2 ].data.uint16_val; 
		
		if( action->action.do_file_write.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_write.path_name, 
			all_params[ 3 ].data.wstring_val, 
			( ( action->action.do_file_write.path_len ) << 1 ) ); 

	}while( FALSE ); 

	return ntstatus; 
}


NTSTATUS file_modified_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_MODIFIED_PARAM_COUNT 2
		if( param_count < FILE_MODIFIED_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_modified ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_file_modified.path_len = all_params[ 0 ].data.uint16_val; 
		
		if( action->action.do_file_modified.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_modified.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_modified.path_len ) << 1 ) ); 

		action->action.do_file_modified.path_name[ action->action.do_file_modified.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS file_read_dir_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_READ_DIR_PARAM_COUNT 3
		if( param_count < FILE_READ_DIR_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_readdir ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 
		action->action.do_file_readdir.filter = all_params[ 0 ].data.uint32_val; 
		action->action.do_file_readdir.path_len = all_params[ 1 ].data.uint16_val; 
		
		if( action->action.do_file_readdir.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_readdir.path_name, 
			all_params[ 2 ].data.wstring_val, 
			( ( action->action.do_file_readdir.path_len ) << 1 ) ); 

		action->action.do_file_readdir.path_name[ action->action.do_file_readdir.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS file_remove_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_MODIFIED_PARAM_COUNT 2
		if( param_count < FILE_MODIFIED_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_remove ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_file_remove.path_len = all_params[ 0 ].data.uint16_val; 
		
		if( action->action.do_file_remove.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_remove.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_remove.path_len ) << 1 ) ); 

		action->action.do_file_remove.path_name[ action->action.do_file_modified.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS file_rename_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_RENAME_PARAM_COUNT 2
		if( param_count < FILE_RENAME_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_rename ) 
			+ ( ( MAX_PATH + MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//ULONG replace_existing; //是否覆盖已存在文件 
		//ULONG result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T new_name_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//		WCHAR *new_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = FILE_rename; 

		action->action.do_file_rename.replace_existing = all_params[ 0 ].data.uint32_val; 
		action->action.do_file_rename.path_len = all_params[ 0 ].data.uint16_val; 
		action->action.do_file_rename.new_name_len = all_params[ 0 ].data.uint16_val; 
		
		if( action->action.do_file_rename.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action->action.do_file_rename.new_name_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_rename.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_rename.path_len ) << 1 ) ); 

		action->action.do_file_rename.path_name[ action->action.do_file_rename.path_len ] = L'\0'; 

		memcpy( action->action.do_file_rename.path_name 
			+ action->action.do_file_rename.path_len + 1, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_rename.new_name_len ) << 1 ) ); 

		action->action.do_file_rename.path_name[ action->action.do_file_rename.path_len ] = L'\0'; 
		action->action.do_file_rename.path_name[ action->action.do_file_rename.path_len + 1 
			+ action->action.do_file_rename.new_name_len ] = L'\0'; 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS file_truncate_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_TRUNCATE_PARAM_COUNT 2
		if( param_count < FILE_TRUNCATE_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_truncate ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//ULONG eof; //截断后的文件长度 
		////NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		////WCHAR path_name[ 1 ]; //文件全路径 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_file_truncate.eof = all_params[ 0 ].data.uint32_val; 
		action->action.do_file_truncate.path_len = all_params[ 0 ].data.uint16_val; 

		if( action->action.do_file_truncate.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_truncate.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_truncate.path_len ) << 1 ) ); 

		action->action.do_file_truncate.path_name[ action->action.do_file_truncate.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS file_mklink_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_MKLINK_PARAM_COUNT 2
		if( param_count < FILE_MKLINK_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_mklink ) 
			+ ( ( MAX_PATH + MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//ULONG eof; //截断后的文件长度 
		////NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		////WCHAR path_name[ 1 ]; //文件全路径 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		//ULONG flags; //like replace existing; //是否覆盖已存在文件 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T link_name_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//		WCHAR *link_name_ptr; 
		//	};
		//};
		//WCHAR path_name[ 1

		action->action.do_file_mklink.flags = all_params[ 0 ].data.uint16_val; 
		action->action.do_file_mklink.path_len = all_params[ 0 ].data.uint16_val; 
		action->action.do_file_mklink.link_name_len = all_params[ 0 ].data.uint16_val; 

		if( action->action.do_file_mklink.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( action->action.do_file_mklink.link_name_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_mklink.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_mklink.path_len ) << 1 ) ); 

		action->action.do_file_mklink.path_name[ action->action.do_file_modified.path_len ] = L'\0'; 
	
		memcpy( action->action.do_file_mklink.path_name
			+ action->action.do_file_mklink.path_len + 1, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_mklink.link_name_len ) << 1 ) ); 

		action->action.do_file_mklink.path_name[ action->action.do_file_modified.path_len + 1 
			+ action->action.do_file_mklink.link_name_len ] = L'\0'; 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS file_chmod_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_CHMOD_PARAM_COUNT 2
		if( param_count < FILE_CHMOD_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_chmod ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//ULONG attrib; //文件属性 
		////NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //文件全路径 

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_file_chmod.path_len = all_params[ 0 ].data.uint16_val; 
		
		if( action->action.do_file_chmod.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_modified.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_modified.path_len ) << 1 ) ); 

		action->action.do_file_modified.path_name[ action->action.do_file_modified.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS file_setsec_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_MODIFIED_PARAM_COUNT 2
		if( param_count < FILE_MODIFIED_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_setsec ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_file_setsec.path_len = all_params[ 0 ].data.uint16_val; 
		
		if( action->action.do_file_setsec.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_file_setsec.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_file_setsec.path_len ) << 1 ) ); 

		action->action.do_file_setsec.path_name[ action->action.do_file_modified.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS reg_openkey_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define REG_OPEN_KEY_PARAM_COUNT 2
		if( param_count < FILE_MODIFIED_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( reg_openkey ) 
			+ ( ( MAX_REG_PATH_LEN ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//ULONG access; //注册表打开/创建权限 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		////WCHAR path_name[ 1 ]; //注册表键路径 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_reg_openkey.access = all_params[ 0 ].data.uint32_val; 
		action->action.do_reg_openkey.path_len = all_params[ 1 ].data.uint16_val; 
		
		if( action->action.do_reg_openkey.path_len + 1 > MAX_REG_PATH_LEN )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_reg_openkey.path_name, 
			all_params[ 2 ].data.wstring_val, 
			( ( action->action.do_reg_openkey.path_len ) << 1 ) ); 

		action->action.do_reg_openkey.path_name[ action->action.do_reg_openkey.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS reg_mkkey_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define REG_MK_KEY_PARAM_COUNT 3
		if( param_count < REG_MK_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( reg_mkkey ) 
			+ ( ( MAX_REG_PATH_LEN ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_reg_mkkey.path_len = all_params[ 0 ].data.uint16_val; 
		
		if( action->action.do_reg_mkkey.path_len + 1 > MAX_REG_PATH_LEN )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_reg_mkkey.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_reg_mkkey.path_len ) << 1 ) ); 

		action->action.do_reg_mkkey.path_name[ action->action.do_reg_mkkey.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS reg_rmkey_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define REG_RM_KEY_PARAM_COUNT 2
		if( param_count < REG_RM_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( reg_rmkey ) 
			+ ( ( MAX_REG_PATH_LEN ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = REG_rmkey; 

		action->action.do_reg_rmkey.path_len = all_params[ 0 ].data.uint16_val; 
		
		if( action->action.do_reg_rmkey.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_reg_rmkey.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_reg_rmkey.path_len ) << 1 ) ); 

		action->action.do_reg_rmkey.path_name[ action->action.do_reg_rmkey.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS reg_mvkey_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define REG_MV_KEY_PARAM_COUNT 2
		if( param_count < REG_MV_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( reg_mvkey ) 
			+ ( ( MAX_REG_PATH_LEN ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T new_name_len; 
		////WCHAR path_name[ 1 ]; //注册表键路径 
		////WCHAR new_keyname[ 0 ]; //目标注册表键名 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //注册表键路径

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//		WCHAR *new_key_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_reg_mvkey.path_len = all_params[ 0 ].data.uint16_val; 
		
		if( action->action.do_reg_mvkey.path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		action->action.do_reg_mvkey.new_name_len = all_params[ 1 ].data.uint16_val; 

		if( action->action.do_reg_mvkey.new_name_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		memcpy( action->action.do_reg_mvkey.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_reg_mvkey.path_len ) << 1 ) ); 

		action->action.do_reg_mvkey.path_name[ action->action.do_reg_mvkey.path_len ] = L'\0'; 

		memcpy( action->action.do_reg_mvkey.path_name
			I+ action->action.do_reg_mvkey.path_len + 1, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_reg_mvkey.path_len ) << 1 ) ); 

		action->action.do_reg_mvkey.path_name[ action->action.do_reg_mvkey.path_len + 1 
		action->action.do_reg_mvkey.new_name_len ] = L'\0'; 
	
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS reg_rmval_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	PATH_SIZE_T path_len; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define REG_RM_KEY_PARAM_COUNT 2
		if( param_count < REG_RM_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		path_len = all_params[ 0 ].data.uint16_val; 

		if( path_len + 1 > MAX_REG_PATH_LEN )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( reg_rmval ) 
			+ ( path_len << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_reg_rmval.path_len = path_len; 

		memcpy( action->action.do_reg_rmval.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_reg_rmval.path_len ) << 1 ) ); 

		action->action.do_reg_rmval.path_name[ action->action.do_reg_rmval.path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS reg_setval_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	PATH_SIZE_T val_name_len; 
	PATH_SIZE_T key_name_len; 


	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define REG_SETVAL_PARAM_COUNT 6
		if( param_count < REG_SETVAL_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		val_name_len = all_params[ 0 ].data.uint16_val; 
		key_name_len = all_params[ 0 ].data.uint16_val; 

		if( key_name_len + 1 > MAX_REG_PATH_LEN )
		{
			break; 
		}

		if( val_name_len + 1 > MAX_PATH )
		{
			break; 
		}

		//ULONG type; //注册表值类型 
		//ULONG data_len; //数据长度 
		////NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T val_name_len; 
		//PATH_SIZE_T key_name_len; 
		//WCHAR path_name[ 1 ]; //注册表值路径 
		////WCHAR path_name[ 0 ]; //注册表键路径名 

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( reg_setval ) 
			+ ( ( key_name_len + 1 + val_name_len + 1 ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 
		action->action.do_reg_setval.data_len = key_name_len; 
		action->action.do_reg_setval.key_name_len = val_name_len; 
		
		memcpy( action->action.do_reg_setval.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( action->action.do_reg_setval.path_len ) << 1 ) ); 

		action->action.do_reg_setval.path_name[ action->action.do_reg_setval.path_len ] = L'\0'; 

		memcpy( action->action.do_reg_setval.path_name 
			+ key_name_len + 1, 
			all_params[ 4 ].data.wstring_val, 
			( val_name_len << 1 ) ); 

		action->action.do_reg_setval.path_name[ key_name_len + 1 + val_name_len ] = L'\0'; 
	
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS reg_loadkey_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	PATH_SIZE_T key_name_len; 
	PATH_SIZE_T hive_name_len; 


	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define REG_LOAD_KEY_PARAM_COUNT 4
		if( param_count < REG_LOAD_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		key_name_len = all_params[ 0 ].data.uint16_val; 
		val_name_len = all_params[ 1 ].data.uint16_val; 

		if( key_name_len + 1 > MAX_REG_PATH_LEN )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( hive_name_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( reg_loadkey ) 
			+ ( ( key_name_len + 1 + val_name_len + 1 ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//PATH_SIZE_T key_name_len; 
		//PATH_SIZE_T hive_name_len; 
		//WCHAR path_name[ 1 ]; //注册表键路径名 

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_reg_loadkey.key_name_len= key_name_len
		action->action.do_reg_loadkey.hive_name_len = hive_name_len; 
		
		memcpy( action->action.do_reg_loadkey.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( key_name_len ) << 1 ) ); 

		action->action.do_reg_loadkey.path_name[ key_name_len ] = L'\0'; 

		memcpy( action->action.do_reg_loadkey.path_name + key_name_len + 1, 
			all_params[ 4 ].data.wstring_val, 
			( ( key_name_len ) << 1 ) ); 

		action->action.do_reg_loadkey.path_name[ key_name_len + 1 + val_name_len ] = L'\0'; 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS reg_replkey_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	PATH_SIZE_T key_name_len; 
	PATH_SIZE_T hive_name_len; 
	PATH_SIZE_T new_hive_name_len; 


	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define REG_REPLKEY_PARAM_COUNT 5
		if( param_count < REG_REPLKEY_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		//PATH_SIZE_T key_name_len; 
		//PATH_SIZE_T hive_name_len; 
		//PATH_SIZE_T new_hive_name_len; 
		//WCHAR path_name[ 1 ]; //注册表键路径名 

		key_name_len = all_params[ 0 ].data.uint16_val; 
		hive_name_len = all_params[ 1 ].data.uint16_val; 
		new_hive_name_len = all_params[ 3 ].data.uint16_val; 

		if( key_name_len + 1 > MAX_REG_PATH_LEN )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( hive_name_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		if( new_hive_name_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_3; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_modified ) 
			+ ( ( key_name_len + 1 
			+ hive_name_len + 1 
			+ new_hive_name_len + 1 ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}


		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_reg_replkey.key_name_len = key_name_len; 
		action->action.do_reg_replkey.hive_name_len = hive_name_len; 
		action->action.do_reg_replkey.new_hive_name_len = new_hive_name_len; 

		memcpy( action->action.do_reg_replkey.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( key_name_len ) << 1 ) ); 

		action->action.do_reg_replkey.path_name[ key_name_len ] = L'\0'; 
	
		memcpy( action->action.do_reg_replkey.path_name 
			+ key_name_len + 1, 
			all_params[ 4 ].data.wstring_val, 
			( ( hive_name_len ) << 1 ) ); 

		action->action.do_reg_replkey.path_name[ key_name_len + 1 + hive_name_len ] = L'\0'; 

		memcpy( action->action.do_reg_replkey.path_name
			+ key_name_len + 1 + hive_name_len, 
			all_params[ 4 ].data.wstring_val, 
			( ( new_hive_name_len ) << 1 ) ); 

		action->action.do_reg_replkey.path_name[ hive_name_len + 1 + hive_name_len + 1 + new_hive_name_len ] = L'\0'; 

	}while( FALSE ); 

	return ntstatus; 
}

//#define reg_rstrkey_setup reg_loadkey_setup 

NTSTATUS reg_rstrkey_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	return reg_loadkey_setup( all_params, param_count, flags ); 
}

NTSTATUS reg_setsec_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	PATH_SIZE_T key_name_len; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_MODIFIED_PARAM_COUNT 2
		if( param_count < FILE_MODIFIED_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		key_name_len = all_params[ 0 ].data.uint16_val; 
		if( key_name_len > MAX_REG_PATH_LEN )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( reg_setsec ) 
			+ ( ( key_name_len + 1 ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_reg_setsec.path_len = key_name_len; 
		
		memcpy( action->action.do_reg_setsec.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( key_name_len ) << 1 ) ); 

		action->action.do_reg_setsec.path_name[ key_name_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS proc_exec_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	PATH_SIZE_T path_len; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define PROC_EXEC_PARAM_COUNT 2
		if( param_count < FILE_MODIFIED_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		//ULONG target_pid; //目标进程ID 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 
	
		path_len = all_params[ 1 ].data.uint16_val; 
		if( path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( proc_exec ) 
			+ ( ( path_len + 1 ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		action->action.do_proc_exec.target_pid = all_params[ 0 ].data.uint32_val; 
		action->action.do_proc_exec.path_len = path_len; 

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = PROC_exec; 

		memcpy( action->action.do_proc_exec.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( path_len ) << 1 ) ); 

		action->action.do_proc_exec.path_name[ path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS proc_open_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	PATH_SIZE_T path_len; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define PROC_OPEN_PARAM_COUNT 2
		if( param_count < PROC_OPEN_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		path_len = all_params[ 0 ].data.uint16_val; 
		if( path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_3; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( proc_open ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//ULONG target_pid; //目标进程ID 
		//ULONG access; //进程打开权限 
		////NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = PROC_open; 

		action->action.do_proc_open.target_pid = all_params[ 0 ].data.uint32_val; 
		action->action.do_proc_open.access = all_params[ 1 ].data.uint16_val; 

		action->action.do_proc_open.path_len = path_len; 
		
		memcpy( action->action.do_proc_open.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( path_len ) << 1 ) ); 

		action->action.do_proc_open.path_name[ path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS proc_debug_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	ULONG path_len; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define PROC_DEBUG_PARAM_COUNT 4
		if( param_count < PROC_DEBUG_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		path_len = all_params[ 3 ].data.uint16_val; 

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( proc_debug ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //文件全路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = PROC_debug; 

		action->action.do_proc_debug.target_pid = all_params[ 0 ].data.uint32_val; 
		//action->action.do_proc_debug.access = all_params[ 0 ].data.uint32_val; 

		action->action.do_proc_debug.path_len = path_len; 
		
		memcpy( action->action.do_proc_debug.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( path_len ) << 1 ) ); 

		action->action.do_proc_debug.path_name[ path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS proc_suspend_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	ULONG path_len; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define PROC_SUSPEND_PARAM_COUNT 3
		if( param_count < PROC_SUSPEND_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		path_len = all_params[ 1 ].data.uint16_val; 

		if( path_len + 1 > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_3; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( proc_suspend ) 
			+ ( ( path_len + 1 ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_proc_suspend.target_pid = all_params[ 0 ].data.uint32_val; 
		action->action.do_proc_suspend.path_len = path_len; 
		
		memcpy( action->action.do_proc_suspend.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( path_len ) << 1 ) ); 

		action->action.do_proc_suspend.path_name[ path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS proc_resume_setup( param_info all_params[ ], 
						   ULONG param_count, 
						   ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	sys_action_info *action = NULL; 
	ULONG path_len; 

	do 
	{
		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define FILE_MODIFIED_PARAM_COUNT 2
		if( param_count < FILE_MODIFIED_PARAM_COUNT )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		path_len = all_params[ 2 ].data.uint16_val; 
		if( path_len > MAX_PATH )
		{
			ntstatus = STATUS_INVALID_PARAMETER_3; 
			break; 
		}

		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( file_modified ) 
			+ ( ( MAX_PATH ) << 1 ) ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}


		if( COLLECT_CURRENT_PROCESS_CONTEXT & flags )
		{
			ntstatus = collect_action_context( &action->ctx ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}
		}

		action->action.type = EXEC_destroy; 

		action->action.do_proc_resume.target_pid = all_params[ 0 ].data.uint32_val; 
		action->action.do_proc_resume.path_len = path_len; 
		
		memcpy( action->action.do_proc_resume.path_name, 
			all_params[ 4 ].data.wstring_val, 
			( ( path_len ) << 1 ) ); 

		action->action.do_proc_resume.path_name[ path_len ] = L'\0'; 
	}while( FALSE ); 

	return ntstatus; 
}

#endif //0 

ULONG get_param_data_type_len( param_info *param )
{
	ULONG data_type_len; 

	switch( param->type )
	{
	case INT8_TYPE:
		data_type_len = sizeof( CHAR ); 
		break; 
	case INT16_TYPE: 
		data_type_len = sizeof( SHORT ); 
		break; 
	case INT32_TYPE: 
		data_type_len = sizeof( INT32 );  
		break; 
	case INT64_TYPE:
		data_type_len = sizeof( LONGLONG ); 
		break; 
	case UINT8_TYPE:
		data_type_len = sizeof( BYTE ); 
		break; 
	case UINT16_TYPE:
		data_type_len = sizeof( USHORT ); 
		break; 
	case UINT32_TYPE: 
		data_type_len = sizeof( UINT32 ); 
		break; 
	case UINT64_TYPE: 
		data_type_len = sizeof( ULONGLONG ); 
		break; 
	case PTR_TYPE:
		data_type_len = sizeof( PVOID ); 
		break; 
	case STRING_PARAM:
		if( param->data.string_val == NULL )
		{
			data_type_len = 0; 
			break; 
		}

		data_type_len = strlen( param->data.string_val ) + sizeof( CHAR ); 
		break; 
	case WSTRING_TYPE:
		if( param->data.wstring_val == NULL )
		{
			data_type_len = 0; 
			break; 
		}

		data_type_len = ( ( wcslen( param->data.wstring_val ) + 1 ) << 1 ); 
		break; 
	case ANSI_STRING_TYPE:
		if( param->data.ansi_val.Buffer == NULL )
		{
			ASSERT( FALSE ); 
			data_type_len = 0; 
			break; 
		}

		data_type_len = param->data.ansi_val.Length + sizeof( CHAR ); 

		break; 
	case UNICODE_STRING_TYPE:
		if( param->data.unicode_val.Buffer == NULL )
		{
			ASSERT( FALSE ); 
			data_type_len = 0; 
			break; 
		}

		data_type_len = param->data.unicode_val.Length + sizeof( WCHAR ); 

		break; 
	case DATA_BLOB_TYPE:
		data_type_len = 0; 
		break; 
	default:
		data_type_len = 0;  
		break;
	}

	return data_type_len; 
}

NTSTATUS copy_param_data( BYTE *data_buf, 
						 ULONG buf_len, 
						 param_info *param, 
						 ULONG *param_data_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( data_buf != NULL ); 
		ASSERT( param_data_len != NULL ); 

		ASSERT( buf_len > 0 ); 
		ASSERT( buf_len >= get_param_data_type_len( param ) ); 

		*param_data_len = 0; 
		switch( param->type )
		{
		case INT8_TYPE:
			*( INT8* )data_buf = param->data.int8_val; 
			*param_data_len = sizeof( param->data.int8_val ); 
			break; 
		case INT16_TYPE: 
			*( SHORT* )data_buf = param->data.int16_val; 
			*param_data_len = sizeof( param->data.int16_val ); 
			break; 
		case INT32_TYPE: 
			*( INT32* )data_buf = param->data.int32_val; 
			*param_data_len = sizeof( param->data.int32_val ); 
			break; 
		case INT64_TYPE:
			*( INT64* )data_buf = param->data.int64_val; 
			*param_data_len = sizeof( param->data.int64_val ); 
			break; 
		case UINT8_TYPE:
			*( UINT8* )data_buf = param->data.uint8_val; 
			*param_data_len = sizeof( param->data.uint8_val ); 
			break; 
		case UINT16_TYPE:
			*( UINT16* )data_buf = param->data.uint16_val; 
			*param_data_len = sizeof( param->data.uint16_val ); 
			break; 
		case UINT32_TYPE: 
			*( UINT32* )data_buf = param->data.uint32_val; 
			*param_data_len = sizeof( param->data.uint32_val ); 
			break; 
		case UINT64_TYPE: 
			*( ULONGLONG* )data_buf = param->data.uint64_val; 
			*param_data_len = sizeof( param->data.uint64_val ); 
			break; 
		case PTR_TYPE:
			*( PVOID* )data_buf = ( PVOID )( ULONG_PTR )param->data.ptr_val; 
			*param_data_len = sizeof( param->data.ptr_val ); 
			break; 
		case STRING_PARAM:
			if( param->data.string_val == NULL )
			{
				ASSERT( FALSE ); 
				ntstatus = STATUS_INVALID_PARAMETER_2; 
				break; 
			}

			*param_data_len = sizeof( param->data.string_val ) + sizeof( CHAR ); 

			memcpy( data_buf, param->data.string_val, *param_data_len ); 
			break; 
		case WSTRING_TYPE:
			if( param->data.wstring_val == NULL )
			{
				ASSERT( FALSE ); 
				ntstatus = STATUS_INVALID_PARAMETER_2; 
				break; 
			}

			*param_data_len = ( wcslen( param->data.wstring_val ) << 1 ) + sizeof( WCHAR ); 
			memcpy( data_buf, param->data.wstring_val, *param_data_len ); 
			break; 
		case ANSI_STRING_TYPE:
			if( param->data.ansi_val.Buffer == NULL )
			{
				ASSERT( FALSE ); 
				ntstatus = STATUS_INVALID_PARAMETER_3; 
				break; 
			}

			memcpy( data_buf, param->data.ansi_val.Buffer, param->data.ansi_val.Length ); 

			*( CHAR* )( data_buf + param->data.ansi_val.Length ) = '\0'; 
			*param_data_len = param->data.ansi_val.Length + sizeof( CHAR ); 

			break; 

		case UNICODE_STRING_TYPE:

			if( param->data.unicode_val.Buffer == NULL )
			{
				ASSERT( FALSE ); 
				ntstatus = STATUS_INVALID_PARAMETER_3; 
				break; 
			}

			memcpy( data_buf, param->data.unicode_val.Buffer, param->data.unicode_val.Length ); 

			*( WCHAR* )( data_buf + param->data.unicode_val.Length ) = L'\0'; 
			*param_data_len = param->data.unicode_val.Length + sizeof( WCHAR ); 

			break; 

		case DATA_BLOB_TYPE:
			ntstatus = STATUS_NOT_SUPPORTED; 
			break; 
		default:
			ntstatus = STATUS_NOT_IMPLEMENTED; 
			break;
		}

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS construct_param_struct_data( param_info all_params[ ], 
						   ULONG param_count, 
						   PVOID struct_data_buf, 
						   ULONG buf_len,  
						   ULONG *struct_data_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	BYTE *param_data_buf = NULL; 
	ULONG data_copied_len; 
	ULONG struct_data_buf_len; 
	ULONG param_data_len; 
	ULONG i; 

	do 
	{
		ASSERT( struct_data_len != 0 ); 

		*struct_data_len = 0;

		if( all_params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( param_count == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		struct_data_buf_len = 0; 
		
		for( i = 0; i < param_count; i ++ )
		{
			param_data_len = get_param_data_type_len( &all_params[ i ] ); 
			if( param_data_len == 0 )
			{
				ntstatus = STATUS_INVALID_PARAMETER_3; 
				break; 
			}

			struct_data_buf_len += param_data_len; 
		}

		if( struct_data_len != NULL )
		{
			*struct_data_len = struct_data_buf_len; 
		}

		if( buf_len < struct_data_buf_len )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		param_data_buf = ( BYTE* )struct_data_buf; 
		//param_data_buf = ( BYTE* )ALLOC_TAG_POOL( struct_data_buf_len ); 
		
		if( param_data_buf == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		data_copied_len = 0; 
		for( i = 0; i < param_count; i ++ )
		{
			ntstatus = copy_param_data( param_data_buf + data_copied_len, 
				buf_len - data_copied_len, 
				&all_params[ i ], 
				&param_data_len ); 

			if( ntstatus !=	STATUS_SUCCESS )
			{
				break; 
			}

			data_copied_len += param_data_len; 
		}

		ASSERT( data_copied_len == struct_data_buf_len ); 

		//*struct_data_len = data_copied_len; 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS test_action_data_setup()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	param_info all_params[ MAX_PARAMS_COUNT ]; 
	sys_action_info *action = NULL; 
	ULONG ret_len; 

	do 
	{
		action = ( sys_action_info* )alloc_sys_action_info( ACTION_RECORD_SIZE_BY_TYPE( exec_create ) + ( ( MAX_PATH + MAX_PATH ) << 1 ) ); 
		if( action == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		//INT8_TYPE, 
		//	INT16_TYPE,
		//	INT32_TYPE, 
		//	INT64_TYPE, 
		//	UINT8_TYPE, 
		//	UINT16_TYPE, 
		//	UINT32_TYPE, 
		//	UINT64_TYPE, 
		//	PTR_TYPE, 
		//	STRING_PARAM, 
		//	WSTRING_TYPE, 
		//	ANSI_STRING_TYPE, 
		//	UNICODE_STRING_TYPE, 
		//	DATA_BLOB_TYPE, 
		//	MAX_PARAM_TYPE,

		//ULONG pid; // 进程ID 
		//ULONG parent_pid; // 父进程ID 
		//PVOID image_base; //process virtual address space // 主映像基址 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T cmd_len; 
		//union 
		//{
		//	WCHAR path_name[ 1 ]; //进程全路径 
		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//		WCHAR *cmd_line_ptr; 
		//	};
		//};

#define TEST_PROC_ID 0
#define TEST_PARENT_PROC_ID 10
#define TEST_IMAGE_BASE 0x80000001

		all_params[ 0 ].type = UINT32_TYPE; 
		all_params[ 0 ].data.uint32_val = TEST_PROC_ID; 

		all_params[ 1 ].type = UINT32_TYPE; 
		all_params[ 1 ].data.uint32_val = TEST_PARENT_PROC_ID; 

		all_params[ 2 ].type = PTR_TYPE; 
		all_params[ 2 ].data.ptr_val = ( POINTER_TYPE )( ULONG_PTR )( ULONG )0x80003210; 

#define TEST_EXEC_CREATE_PATH L"test_proc"
#define TEST_EXEC_CREATE_CMD L"test_cmd"

		all_params[ 3 ].type = UINT16_TYPE; 
		all_params[ 3 ].data.uint32_val = CONST_STR_LEN( TEST_EXEC_CREATE_PATH ); 

		all_params[ 4 ].type = UINT16_TYPE; 
		all_params[ 4 ].data.uint32_val = CONST_STR_LEN( TEST_EXEC_CREATE_CMD ); 

		all_params[ 5 ].type = WSTRING_TYPE; 
		all_params[ 5 ].data.wstring_val = TEST_EXEC_CREATE_PATH; 

		all_params[ 6 ].type = WSTRING_TYPE; 
		all_params[ 6 ].data.wstring_val = TEST_EXEC_CREATE_CMD; 

		//ntstatus = collect_action_context( action&->ctx ); 
		//if( ntstatus != STATUS_SUCCESS )
		//{
		//	break; 
		//}

		action->action.type = EXEC_create; 

		ntstatus = construct_param_struct_data( all_params, 
			7, 
			( BYTE* )&action->action.do_exec_create, 
			sizeof( exec_create ) + ( ( MAX_PATH + MAX_PATH ) << 1 ), 
			&ret_len ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		if( action->action.do_exec_create.pid != all_params[ 0 ].data.uint32_val )
		{
			ASSERT( FALSE ); 
		}

		if( action->action.do_exec_create.parent_pid != all_params[ 1 ].data.uint32_val )
		{
			ASSERT( FALSE ); 
		}

		if( action->action.do_exec_create.image_base != all_params[ 2 ].data.ptr_val )
		{
			ASSERT( FALSE ); 
		}

		if( action->action.do_exec_create.path_len != all_params[ 3 ].data.uint16_val )
		{
			ASSERT( FALSE ); 
		}

		if( action->action.do_exec_create.cmd_len != all_params[ 4 ].data.uint16_val )
		{
			ASSERT( FALSE ); 
		}

		if( wcscmp( action->action.do_exec_create.path_name, 
			all_params[ 5 ].data.wstring_val ) != 0 )
		{
			ASSERT( FALSE ); 
		}

		if( wcscmp( action->action.do_exec_create.path_name 
			+ all_params[ 3 ].data.uint16_val + 1, 
			all_params[ 6 ].data.wstring_val ) != 0 )
		{
			ASSERT( FALSE ); 
		}

	}while( FALSE );

	if( action != NULL )
	{
		FREE_TAG_POOL( action ); 
	}

	return ntstatus; 
}

NTSTATUS analyze_action_meaning( sys_action_info *action, sys_action_type *action_mean )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		if( action_mean == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		*action_mean = BA_other; 
	}while( FALSE ); 

	return ntstatus; 
}