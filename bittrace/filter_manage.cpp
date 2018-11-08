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
#include "filter_manage.h"
#include "sqlite_supp.h"
#include "action_log_db.h"
#include "action_log_sql.h"
#include "stack_dump.h"
#include "filter_cond_db.h"
#include "action_filter_cond_sql.h"
#include "action_log_sql.h"
#include "action_display.h"
#include "r3_rw_lock.h"

#pragma comment(lib, "Delayimp.lib")
#pragma comment(linker, "/DelayLoad:d3d9.dll")

CRITICAL_SECTION filter_info_lock; 
action_filter_infos filter_infos; 


LRESULT WINAPI r_lock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		lock_cs( filter_info_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI r_unlock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		unlock_cs( filter_info_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI w_lock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		lock_cs( filter_info_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI w_unlock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		unlock_cs( filter_info_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI w_try_lock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		_ret = TryEnterCriticalSection( &filter_info_lock ); 

		if( FALSE == _ret )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "try enter cs error" ) ); 

			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

/*****************************************************************************
will make more database for caching events which is filtered.so need a function 
set to manage the data structure for caching.
caching database id:
1.timestamp
2.base database information.
3.filtered target database and table.
4.filtered sql

is when remove the temporary database for filtering? 
1.change the filtered condition.( accumulated filtering is best )
*****************************************************************************/

/*************************************************************
filter value type: 
1. value from ring 3. 
2. value from ring 0. 
*************************************************************/

/*************************************************************
time
main type
*************************************************************/
HANDLE WINAPI get_parent_pid( HANDLE pid )
{
	HANDLE parent_pid = INVALID_PROCESS_ID; 

	return parent_pid; 
} 

ULONG WINAPI get_action_main_type( sys_action_type action_type )
{
	ULONG main_type = 0; 

	switch( action_type )
	{

		//MT_execmon
	case EXEC_create:   //进程启动 进程路径名 （执行监控） 
	case EXEC_destroy:  //进程退出 进程路径名 
	case EXEC_module_load:  //模块加载 模块路径名 
		main_type = MT_execmon; 
		break; 
		//MT_filemon:  
	case FILE_touch:  //创建文件 文件全路径 （文件监控） 
	case FILE_open:  //打开文件 文件全路径 
	case FILE_read:  //读取文件 文件全路径 
	case FILE_write:  //写入文件 文件全路径 
	case FILE_modified:  //文件被修改 文件全路径 
	case FILE_readdir:  //遍历目录 目录全路径 
	case FILE_remove:  //删除文件 文件全路径 
	case FILE_rename:  //重命名文件 文件全路径 
	case FILE_truncate:  //截断文件 文件全路径 
	case FILE_mklink:  //建立文件硬链接 文件全路径 
	case FILE_chmod:  //设置文件属性 文件全路径 
	case FILE_setsec:  //设置文件安全属性 文件全路径 
	case FILE_getinfo:
	case FILE_setinfo:
	case FILE_close:
		main_type = MT_filemon; 
		break; 
		//MT_regmon:  
	case REG_openkey:  //打开注册表键 注册表键路径  （注册表监控） 
	case REG_mkkey:  //创建注册表键 注册表键路径 
	case REG_rmkey:  //删除注册表键 注册表键路径 
	case REG_mvkey:  //重命名注册表键 注册表键路径 
	case REG_rmval:  //删除注册表键 注册表键路径 
	case REG_getval:  //获取注册表值 注册表值路径 
	case REG_setval:  //设置注册表值 注册表值路径 
	case REG_loadkey:  //挂载注册表Hive文件 注册表键路径 
	case REG_replkey:  //替换注册表键 注册表键路径 
	case REG_rstrkey:  //导入注册表Hive文件 注册表键路径 
	case REG_setsec:  //设置注册表键安全属性 注册表键路径 
	case REG_closekey:
		main_type = MT_regmon; 
		break; 

		//MT_procmon:  
	case PROC_exec:  //创建进程 目标进程路径名  （进程监控）
	case PROC_open:  //打开进程 目标进程路径名 
	case PROC_debug:  //调试进程 目标进程路径名 
	case PROC_suspend:  //挂起进程 目标进程路径名 
	case PROC_resume:  //恢复进程 目标进程路径名 
	case PROC_kill:  
	case PROC_exit:  //结束进程 目标进程路径名 
	case PROC_job:  //将进程加入工作集 目标进程路径名 
	case PROC_pgprot:  //跨进程修改内存属性 目标进程路径名 
	case PROC_freevm:  //跨进程释放内存 目标进程路径名 
	case PROC_writevm:  //跨进程写内存 目标进程路径名 
	case PROC_readvm:  //跨进程读内存 目标进程路径名 
	case THRD_remote:  //创建远程线程 目标进程路径名 
	case THRD_create:  //创建线程
	case THRD_setctxt:  //跨进程设置线程上下文 目标进程路径名 
	case THRD_suspend:  //跨进程挂起线程 目标进程路径名 
	case THRD_resume:  //跨进程恢复线程 目标进程路径名 
	case THRD_kill:  //跨进程结束线程 目标进程路径名 
	case THRD_exit:  
	case THRD_queue_apc:  //跨进程排队APC 目标进程路径名 
		main_type = MT_procmon; 
		break; 

		//MT_common
	case COM_access:
		main_type = MT_common; 
		break;

	case PORT_read:
	case PORT_write:
	case PORT_urb:
		main_type = MT_peripheral; 
		break; 

		//MT_sysmon
	case SYS_settime:  //设置系统时间 无 
	case SYS_link_knowndll:  //建立KnownDlls链接 链接文件名 
	case SYS_open_physmm:  //打开物理内存设备 无 
	case SYS_read_physmm:  //读物理内存 无 
	case SYS_write_physmm:  //写物理内存 无 
	case SYS_load_kmod:  //加载内核模块 内核模块全路径 
	case SYS_load_mod:  //加载模块 内核模块全路径 
	case SYS_unload_mod:  //卸载模块 内核模块全路径 
	case SYS_enumproc:  //枚举进程 无 
	case SYS_regsrv:  //注册服务 服务进程全路径 
	case SYS_opendev:  //打开设备 设备名 
		main_type = MT_sysmon; 
		break; 

		//MT_w32mon
	case W32_postmsg:  //发送窗口消息（Post） 目标进程路径名 
	case W32_sendmsg:  //发送窗口消息（Send） 目标进程路径名 
	case W32_findwnd:  //查找窗口 无 
	case W32_msghook:  //设置消息钩子 无 
	case W32_lib_inject:  //DLL注入 注入DLL路径名 
		main_type = MT_w32mon; 
		break; 

		//MT_netmon:  
	case NET_create:  
	case NET_connect:  //网络连接 远程地址（格式：IP：端口号） （网络监控） 
	case NET_listen:  //监听端口 本机地址（格式：IP：端口号） 
	case NET_send:  //发送数据包 远程地址（格式：IP：端口号） 
	case NET_recv:  
	case NET_accept:  

	case NET_dns:  
	case NET_http:  //HTTP请求 HTTP请求路径（格式：域名/URL） 
	case NET_icmp_send:  
	case NET_icmp_recv:  
		main_type = MT_netmon; 
		break; 

		//MT_behavior:  
	case BA_extract_hidden:  //释放隐藏文件 释放文件路径名 （行为监控） 
	case BA_extract_pe:  //释放PE文件 释放文件路径名 
	case BA_self_copy:  //自我复制 复制目标文件路径名 
	case BA_self_delete:  //自我删除 删除文件路径名 
	case BA_ulterior_exec:  //隐秘执行 被执行映像路径名 
	case BA_invade_process:  //入侵进程 目标进程路径名 
	case BA_infect_pe:  //感染PE文件 目标文件路径名 
	case BA_overwrite_pe:  //覆写PE文件 目标文件路径名 
	case BA_register_autorun:  //注册自启动项 自启动文件路径名 
	case BA_other:  
		main_type = MT_behavior; 
	default:
		main_type = MT_unknown; 
		break; 
	}

	return main_type; 
}

/**********************************************************************************************************************
速度慢的解决方案：
大量的使用内存，通过内存来实现实时显示日志。
如果需要进行分析，则是立足于数据库，一定将所有能加载的日志全部加载，最大量由内存容量决定。

内存加载法，高速显示所有的信息。

多级缓存法：
第1级缓存是数据库
第2级缓存是内存，直接加载所有数据库中的内容。全部放在内存中。
第3级缓存是过滤后的数据，加载内存中经过过滤的信息。

速度的重要性：


显示一个完整的描述信息好，还是将操作与对像，细节分别显示好：

**********************************************************************************************************************/
BOOLEAN WINAPI filter_action_time_value( LARGE_INTEGER *event_time, time_region *duration, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 

	do 
	{
		switch( flt_mode )
		{
		case FLT_CONTAINS: 
			if( event_time->QuadPart >=
				duration->begin_time.QuadPart 
				&& event_time->QuadPart <= duration->end_time.QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_EXCLUDES: 
			if( event_time->QuadPart <
				duration->begin_time.QuadPart 
				&& event_time->QuadPart > duration->end_time.QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_EQUALS: 
			if( event_time->QuadPart >=
				duration->begin_time.QuadPart 
				&& event_time->QuadPart <= duration->end_time.QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 

		case FLT_NOT_EQUALS: 
			if( event_time->QuadPart <
				duration->begin_time.QuadPart 
				&& event_time->QuadPart > duration->end_time.QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		default: 
			ASSERT( FALSE ); 
			break; 
		}
	}while( FALSE );

	return filtered; 
}

BOOLEAN WINAPI _filter_action_time_value( LARGE_INTEGER *event_time, LARGE_INTEGER *dst_time, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 

	do 
	{
		event_time->QuadPart = event_time->QuadPart - ( event_time->QuadPart % ( 10000000 ) ); 

		switch( flt_mode )
		{
		case FLT_CONTAINS: 
			if( event_time->QuadPart == dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_EXCLUDES: 
			if( event_time->QuadPart != dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_EQUALS: 
			if( event_time->QuadPart == dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 

		case FLT_NOT_EQUALS: 
			if( event_time->QuadPart != dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_GREATER_THAN: 
			if( event_time->QuadPart > dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_LOWER_THAN: 
			if( event_time->QuadPart < dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_START_WITH: 
			if( event_time->QuadPart >= dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_END_WITH: 
			if( event_time->QuadPart <= dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 

		default: 
			ASSERT( FALSE ); 
			break; 
		}
	}while( FALSE );

	return filtered; 
}

BOOLEAN WINAPI filter_action_ulong_value( ULONG event_type, ULONG policy_type, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 

	switch( flt_mode )
	{
	case FLT_CONTAINS: 

		if( event_type == ( event_type & policy_type ) )
		{
			filtered = TRUE; 
			break; 
		}

		break; 
	case FLT_EXCLUDES: 
		if( 0 == ( event_type & policy_type ) )
		{
			filtered = TRUE; 
			break; 
		}
		break; 
	case FLT_EQUALS: 
		if( event_type == policy_type )
		{
			filtered = TRUE; 
			break; 
		}
		break; 

	case FLT_NOT_EQUALS: 
		if( event_type != policy_type )
		{
			filtered = TRUE; 
			break; 
		}
		break; 
	default: 
		ASSERT( FALSE ); 
		break; 
	}

	return filtered; 
}

BOOLEAN WINAPI filter_action_ulonglong_value( ULONGLONG source_value, ULONGLONG dest_value, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 

	switch( flt_mode )
	{
	case FLT_CONTAINS: 

		if( source_value == dest_value )
		{
			filtered = TRUE; 
			break; 
		}

		break; 
	case FLT_EXCLUDES: 
		if( source_value != dest_value )
		{
			filtered = TRUE; 
			break; 
		}
		break; 
	case FLT_EQUALS: 
		if( source_value == dest_value )
		{
			filtered = TRUE; 
			break; 
		}
		break; 

	case FLT_NOT_EQUALS: 
		if( source_value != dest_value )
		{
			filtered = TRUE; 
			break; 
		}
		break; 
	default: 
		ASSERT( FALSE ); 
		break; 
	}

	return filtered; 
}

BOOLEAN WINAPI filter_action_string_value( LPCWSTR source_value, ULONG cc_src_len, LPCWSTR dest_value, ULONG cc_dest_len, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 
	INT32 _ret; 
	LPWSTR text_value; 

	switch( flt_mode )
	{
	case FLT_CONTAINS: 
		text_value = wcsstr( ( LPWSTR )source_value, 
			dest_value ); //ctx->proc_name_len  
		if( text_value != NULL )
		{
			filtered = TRUE; 
		}
		break; 
	case FLT_EXCLUDES: 
		text_value = wcsstr( ( LPWSTR )source_value, 
			dest_value ); //ctx->proc_name_len  
		if( text_value == NULL )
		{
			filtered = TRUE; 
		}
		break; 
	case FLT_EQUALS: 
		_ret = wcsnicmp( source_value, 
			dest_value, 
			cc_dest_len ); 

		if( _ret == 0 )
		{
			filtered = TRUE; 
		}
		break; 
	case FLT_NOT_EQUALS: 
		_ret = wcsnicmp( source_value, 
			dest_value, 
			cc_dest_len ); 

		if( _ret != 0 )
		{
			filtered = TRUE; 
		}
		break; 
	default: 
		ASSERT( FALSE ); 
		break; 
	}

	return filtered; 
}

ULONG WINAPI get_action_io_type( sys_action_type action_type )
{
	ULONG io_type = 0; 
	return io_type; 
}

/***********************************************************************************
将策略定义成2种类型:
1.可排除的部分，当策略达到了一定的状态时，这些策略将会被排除。
2.不可排除部分，必须进行过滤并且判断可排除部分是否可以执行。

当前不可排除部分的定义: 
从策略集开始到结束，当遇到过滤条件不是事件主类型的加入过滤条件时，则对排除可排除部分
策略。

当前可排除部分的定义:
所有的排除方式的过滤条件都是可排除条件。
***********************************************************************************/
LRESULT WINAPI filter_action( action_filter_info *filter_info, 
										  sys_action_record *action, 
										  action_context *ctx, 
										  PVOID data, 
										  ULONG data_size )
{
	LRESULT ret = ERROR_NOT_FOUND; 
	LRESULT _ret; 
	BOOLEAN found = FALSE; 

	do 
	{
		switch( filter_info->cond )
		{
		case FLT_PATH:
			{
				WCHAR obj_path[ _MAX_REG_PATH_LEN ]; 
				LPCWSTR _obj_path; 
				ULONG obj_path_len; 
				
				_ret = get_object_path( action, 
					ctx, 
					obj_path, 
					ARRAYSIZE( obj_path ), 
					&_obj_path, 
					0, 
					&obj_path_len ); 
				
				if( _ret != ERROR_SUCCESS )
				{
					break; 
				}

				found = filter_action_string_value( _obj_path, 
					obj_path_len, 
					filter_info->value.text.string_value, 
					filter_info->value.text.cc_string_len, 
					filter_info->compare_mode ); 
				break; 			
			}
			break; 
		case FLT_PROCESS_NAME: 
			{
				LPCWSTR proc_name; 
				ULONG proc_name_len; 

				proc_name = wcsrchr( ctx->proc_name, PATH_DELIM ); 
				if( proc_name == NULL )
				{
					proc_name = ctx->proc_name; 
					proc_name_len = ( ULONG )wcslen( ctx->proc_name ); 

					ASSERT( wcsicmp( proc_name, L"SYSTEM" ) == 0 ); 
				}
				else
				{
					proc_name += 1;  
					proc_name_len = ( ULONG )( ULONG_PTR )( ( ctx->proc_name + ctx->proc_name_len ) - proc_name ); 
				
#ifdef _DEBUG
					if( proc_name[ proc_name_len - 1 ] != L'e' 
						&& proc_name[ proc_name_len - 1 ] != L'E' ) 
					{
						ASSERT( FALSE ); 
					}
#endif //_DEBUG
				}

				found = filter_action_string_value( proc_name, 
					proc_name_len, 
					filter_info->value.text.string_value, 
					filter_info->value.text.cc_string_len, 
					filter_info->compare_mode ); 

				break; 
			}
			break; 
		case FLT_COMMAND: 
			{
				//WCHAR *cmd_line; 

				ret = ERROR_INVALID_PARAMETER; 

				break; 
			}
			break; 
		case FLT_PID: 
			{
				found = filter_action_ulong_value( ctx->proc_id, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_PARENT_PID: 
			{
				ULONG parent_pid; 

				ret = ERROR_NOT_SUPPORTED; 
				break; 
				parent_pid = ( ULONG )( ULONG_PTR )get_parent_pid( ( HANDLE )( ULONG_PTR )ctx->proc_id ); 
				
				found = filter_action_ulong_value( parent_pid, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_TID: 
			{
				ULONG tid; 

				tid = ctx->thread_id; 

				found = filter_action_ulong_value( tid, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_MAIN_TYPE: 
			{
				ULONG main_type; 

				main_type = get_action_main_type( action->type ); 
				found = filter_action_ulong_value( main_type, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_ACTION_TYPE: 
			{
				found = filter_action_ulong_value( action->type, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_READ_WRITE: 
			{
				ULONG io_type; 
				
				io_type = get_action_io_type( action->type ); 

				found = filter_action_ulong_value( io_type, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_DESCRITION: 
			{
				ret = ERROR_NOT_SUPPORTED; 
				break;
			}
			break; 
		case FLT_DESC_DETAIL: 
			break; 
		case FLT_ACTION_RESULT: 
			{
				found = filter_action_ulong_value( ctx->last_result, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_DURATION: 
			{
				//ASSERT( filter_info->value.size >= sizeof( time_region ) ); 

				found = filter_action_time_value( &ctx->time, &filter_info->value.data.duration, filter_info->compare_mode ); 
			}
			break; 
		case FLT_DATE_TIME:  
			{
				found = _filter_action_time_value( &ctx->time, &filter_info->value.data.time_value, filter_info->compare_mode ); 
			}
			break; 
		case FLT_RELATIVE_TIME: 
			//{
			//	found = filter_action_time_value( &ctx->time, &filter_info->value.data.time_value ); 
			//}
			break; 
		case FLT_SESSION:  
			break; 
		case FLT_USER:  
			break; 
		case FLT_AUTH_ID:  
			break; 
		case FLT_ACTION_NO: 
			break; 
		case FLT_CORPORATION:  
			break; 
		case FLT_VERSION: 
			{
				LARGE_INTEGER file_version; 
				
				_ret = get_file_version( ctx->proc_name, &file_version ); 
				if( _ret != ERROR_SUCCESS )
				{
					break; 
				}

				found = filter_action_ulonglong_value( ( ULONGLONG )file_version.QuadPart, filter_info->value.data.ulonglong_value, filter_info->compare_mode ); 
			}
			break; 
		case FLT_VIRTUAL_TECH: 
			break; 
		case FLT_CPU_ARCH: 
			break; 
		default: 
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			break; 
		}

		if( found != TRUE )
		{
			break; 
		}

		//if( filter_info->result != FLT_INCLUDE )
		//{
		ret = ERROR_SUCCESS; 
		//}

	}while( FALSE ); 

	return ret; 
}

/******************************************************************************************
当前的过滤的方法的问题：
1.过滤速度比较慢，尤其过滤出量大的时候。
处理此问题有两种方法：
1.大量使用内存缓存，像PROCMON一样的工作方式。
2.控制分析的时域。
******************************************************************************************/
LRESULT WINAPI init_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	init_cs_lock( filter_info_lock );  

	return ret; 
}

LRESULT WINAPI uninit_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	uninit_cs_lock( filter_info_lock ); 
	return ret; 
}

#ifdef _DEBUG
ULONG dbg_thread_id = ( ULONG )( ULONG_PTR )INVALID_THREAD_ID - 1; 
ULONG dbg_proc_id = ( ULONG )( ULONG_PTR )INVALID_PROCESS_ID; 
ULONG test_proc_id = ( ULONG )( ULONG_PTR )INVALID_PROCESS_ID; 
#endif //_DEBUG

LRESULT WINAPI filter_action_log( sys_action_record *action, 
								 action_context *ctx,  
								 PVOID data, 
								 ULONG data_size )
{
	LRESULT ret; 
	LRESULT _ret; 
	BOOLEAN lock_held = FALSE; 
	BOOLEAN exclude_secondary_cond = FALSE; 
	ULONG i; 

	do 
	{
		ret = ERROR_SUCCESS; 

		_ret = w_try_lock_filter_infos(); 

		if( ERROR_SUCCESS != _ret )
		{
			break; 
		}

		lock_held = TRUE; 

#ifdef _DEBUG
		if( dbg_thread_id == ctx->thread_id )
		{
			DBG_BP(); 
		}

		if( dbg_proc_id == ctx->proc_id )
		{
			DBG_BP(); 
		}

#endif //_DEBUG

		_ret = ERROR_NOT_FOUND; 

		for( i = 0; i < filter_infos.filter_infos.size(); i ++ )
		{
			do 
			{
				if( filter_infos.filter_infos[ i ]->filter_mode != FLT_INCLUDE )
				{
					break; 
				}

				if( exclude_secondary_cond == FALSE 
					&& filter_infos.filter_infos[ i ]->cond != FLT_MAIN_TYPE ) 
				{
					exclude_secondary_cond = TRUE; 
				}

				_ret = filter_action( filter_infos.filter_infos[ i ], 
					action, 
					ctx, 
					data, 
					data_size ); 

				if( _ret == ERROR_NOT_FOUND )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

			}while( FALSE ); 
		}

		if( FALSE != exclude_secondary_cond 
			|| ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = ERROR_SUCCESS; 
		_ret = ERROR_NOT_FOUND; 
		for( i = 0; i < filter_infos.filter_infos.size(); i ++ )
		{
			do 
			{
				if( filter_infos.filter_infos[ i ]->filter_mode != FLT_EXCLUDE )
				{
					break; 
				}

				_ret = filter_action( filter_infos.filter_infos[ i ], 
					action, 
					ctx, 
					data, 
					data_size ); 

			}while( FALSE );

			if( _ret != ERROR_NOT_FOUND )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}

	}while( FALSE );

#ifdef _DEBUG
	if( test_proc_id != ( ULONG )INVALID_PROCESS_ID 
		&& test_proc_id != ctx->proc_id 
		&& ret == ERROR_SUCCESS )
	{
		DBG_BP(); 
	}
#endif //_DEBUG

	if( TRUE == lock_held )
	{
		w_unlock_filter_infos(); 
	}

	return ret; 
}

LRESULT WINAPI filter_log_for_ui( sys_action_record *action, 
								 action_context *ctx,  
								 PVOID data, 
								 ULONG data_size, 
								 filter_ui_info *ui_info )
{
	LRESULT ret; 
	LRESULT _ret; 
	BOOLEAN lock_held = FALSE; 
	ULONG i; 

	do 
	{
		ret = ERROR_INVALID_PARAMETER; 

		if( ui_info == NULL )
		{
			break; 
		}

		_ret = w_try_lock_filter_infos(); 

		if( ERROR_SUCCESS != _ret )
		{
			break; 
		}

		lock_held = TRUE; 

		_ret = ERROR_NOT_FOUND; 

		for( i = 0; i < filter_infos.filter_infos.size(); i ++ )
		{
			do 
			{
				if( filter_infos.filter_infos[ i ]->ui_info.bk_clr == INVALID_UI_FILTERED_BK_COLOR )
				{
					break; 
				}

				_ret = filter_action( filter_infos.filter_infos[ i ], 
					action, 
					ctx, 
					data, 
					data_size ); 

			}while( FALSE );

			if( _ret != ERROR_NOT_FOUND )
			{
				ui_info->bk_clr = filter_infos.filter_infos[ i ]->ui_info.bk_clr; 
				ui_info->cond = filter_infos.filter_infos[ i ]->cond; 

				ret = ERROR_SUCCESS; 
				break; 
			}
		}
		
	}while( FALSE );

	if( TRUE == lock_held )
	{
		w_unlock_filter_infos(); 
	}

	if( ret != ERROR_SUCCESS )
	{
		ui_info->bk_clr = INVALID_UI_FILTERED_BK_COLOR; 
		ui_info->cond = FLT_TYPE_INVALID; 
	}

	return ret; 
}


/**********************************************************************
过滤方式： 
条件与列名对应方式
1对1: 直接取出列名

优点：
构造条件语句比较简单 *查询速度更快*

缺点：
数据库需要更多的列，需要保存更多的值，占用更多的空间。*查询速度更慢*


1对多: 需要取出列名组

优点：
占用更多的空间，更多的列。

缺点：
构造条件语句比较复杂。

条件与列值对应方式
1对单一模式: 加入一条SQL WHERE 条件等式语句，*查询速度快*

优点：
构造条件语句比较简单

缺点：
数据库需要更多的列，需要保存更多的值，占用更多的空间。

1对多模式: 加入多条SQL WHERE 条件等式语句 需要使用 AND或OR进行连接

优点：
占用更多的空间，更多的列。

缺点：
构造条件语句比较复杂，*查询速度会变慢*。

**********************************************************************/

