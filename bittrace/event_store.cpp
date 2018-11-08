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
#include "event_store.h"

/**********************************************************************************
procmon跟踪功能的分析：
1.其中日志的1/3是PROFILING事件，这个事件是定时对系统的所有的线程的用时情况，和CONTEXT
SWITCH进行跟踪分析。（价值是什么？）

bittrace与procmon的关键差别：
1.注册表键的关闭事件PROCMON非常多，其中的原因是什么？(都使用CALLBACK应该，那么差别为什么
这么大？)
**********************************************************************************/
ULONG WINAPI calc_correct_record_append_size( r3_action_notify *event )
{
	ULONG record_append_size = 0; 
	sys_action_record *_action; 
	
	_action = &event->action.action; 
		
	switch( _action->type )
	{
	case EXEC_create:
		record_append_size += ( ( event->action.action.do_exec_create.path_len + 1 ) << 1 ); 
		break; //进程启动 进程路径名 （执行监控） 

	case EXEC_destroy:
		record_append_size += ( ( event->action.action.do_exec_destroy.path_len + 1 ) << 1 ); 
		break; //进程退出 进程路径名 

	case EXEC_module_load:
		record_append_size += ( ( event->action.action.do_exec_module_load.path_len + 1 ) << 1 ); 
		break; //模块加载 模块路径名 

		//MT_filemon, 
	case FILE_touch:
		record_append_size += ( ( event->action.action.do_file_touch.path_len + 1 ) << 1 ); 
		break; //创建文件 文件全路径 （文件监控） 	
	case FILE_open:
		record_append_size += ( ( event->action.action.do_file_open.path_len + 1 ) << 1 ); 
		break; //打开文件 文件全路径 
		//case ACCESS_FILE:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case FILE_read:
		record_append_size += ( ( event->action.action.do_file_read.path_len + 1 ) << 1 ); 
		break; //读取文件 文件全路径 
	case FILE_write:
		record_append_size += ( ( event->action.action.do_file_write.path_len + 1 ) << 1 ); 
		break; //写入文件 文件全路径 
		//case MODIFY_FILE:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case FILE_modified:
		record_append_size += ( ( event->action.action.do_file_modified.path_len + 1 ) << 1 ); 
		break; //文件被修改 文件全路径 
	case FILE_readdir:
		record_append_size += ( ( event->action.action.do_file_readdir.path_len + 1 ) << 1 ); 
		break; //遍历目录 目录全路径 
	case FILE_remove:
		record_append_size += ( ( event->action.action.do_file_remove.path_len + 1 ) << 1 ); 
		break; //删除文件 文件全路径 
		//case DELETE_FILE:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case FILE_rename:
		record_append_size += ( ( event->action.action.do_file_rename.path_len + 1 ) << 1 ); 
		break; //重命名文件 文件全路径 
	case FILE_truncate:
		record_append_size += ( ( event->action.action.do_file_truncate.path_len + 1 ) << 1 ); 
		break; //截断文件 文件全路径 
	case FILE_mklink:
		record_append_size += ( ( event->action.action.do_file_mklink.path_len + 1 ) << 1 ); 
		break; //建立文件硬链接 文件全路径 
	case FILE_chmod:
		record_append_size += ( ( event->action.action.do_file_chmod.path_len + 1 ) << 1 ); 
		break; //设置文件属性 文件全路径 
	case FILE_setsec:
		record_append_size += ( ( event->action.action.do_file_setsec.path_len + 1 ) << 1 ); 
		break; //设置文件安全属性 文件全路径 

	case FILE_getinfo:
		record_append_size += ( ( event->action.action.do_file_getinfo.path_len + 1 ) << 1 ); 
		break; //设置文件安全属性 文件全路径 

	case FILE_setinfo:
		record_append_size += ( ( event->action.action.do_file_setinfo.path_len + 1 ) << 1 ); 
		break; //设置文件安全属性 文件全路径 

		//MT_regmon, 
	case REG_openkey:
		record_append_size += ( ( event->action.action.do_reg_openkey.path_len + 1 ) << 1 ); 
		break; //打开注册表键 注册表键路径  （注册表监控） 
	case REG_mkkey:
		record_append_size += ( ( event->action.action.do_reg_mkkey.path_len + 1 ) << 1 ); 
		break; //创建注册表键 注册表键路径 
		//case MODIFY_KEY:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case REG_rmkey:
		record_append_size += ( ( event->action.action.do_reg_rmkey.path_len + 1 ) << 1 ); 
		break; //删除注册表键 注册表键路径 
	case REG_mvkey:
		record_append_size += ( ( event->action.action.do_reg_mvkey.path_len + 1 ) << 1 ); 
		break; //重命名注册表键 注册表键路径 
	case REG_rmval:
		record_append_size += ( ( event->action.action.do_reg_rmval.path_len + 1 
			+ event->action.action.do_reg_rmval.val_len + 1 ) << 1 ); 
		break; //删除注册表键 注册表键路径 
	case REG_getval:
		record_append_size += ( ( event->action.action.do_reg_getval.path_len + 1 
			+ event->action.action.do_reg_getval.val_name_len + 1 ) << 1 ) 
			+ event->action.action.do_reg_getval.info_size; 
		break; //获取注册表值 注册表值路径 
	case REG_setval:
		record_append_size += ( ( event->action.action.do_reg_getval.path_len + 1 
			+ event->action.action.do_reg_getval.val_name_len + 1 ) << 1 ) 
			+ event->action.action.do_reg_getval.info_size; 
		break; //设置注册表值 注册表值路径 
	case REG_loadkey:
		record_append_size += ( ( event->action.action.do_reg_loadkey.path_len + 1 ) << 1 ); 
		break; //挂载注册表Hive文件 注册表键路径 
	case REG_replkey:
		record_append_size += ( ( event->action.action.do_reg_replkey.path_len + 1 ) << 1 ); 
		break; //替换注册表键 注册表键路径 
	case REG_rstrkey:
		record_append_size += ( ( event->action.action.do_reg_rstrkey.path_len + 1 ) << 1 ); 
		break; //导入注册表Hive文件 注册表键路径 
	case REG_setsec:
		record_append_size += ( ( event->action.action.do_reg_setsec.path_len + 1 ) << 1 ); 
		break; //设置注册表键安全属性 注册表键路径 

		//MT_procmon, 
	case PROC_exec:
		record_append_size += ( ( event->action.action.do_proc_exec.path_len + 1 ) << 1 ); 
		break; //创建进程 目标进程路径名  （进程监控）
		//case CREATE_PROC:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case PROC_open:
		record_append_size += ( ( event->action.action.do_proc_open.path_len + 1 ) << 1 ); 
		break; //打开进程 目标进程路径名 
	case PROC_debug:
		record_append_size += ( ( event->action.action.do_proc_debug.path_len + 1 ) << 1 ); 
		break; //调试进程 目标进程路径名 
	case PROC_suspend:
		record_append_size += ( ( event->action.action.do_proc_suspend.path_len + 1 ) << 1 ); 
		break; //挂起进程 目标进程路径名 
	case PROC_resume:
		record_append_size += ( ( event->action.action.do_proc_resume.path_len + 1 ) << 1 ); 
		break; //恢复进程 目标进程路径名 
	case PROC_exit:
		record_append_size += ( ( event->action.action.do_proc_exit.path_len + 1 ) << 1 ); 
		break; //结束进程 目标进程路径名 
		//case TERMINATE_PROC:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case PROC_job:
		record_append_size += ( ( event->action.action.do_proc_job.path_len + 1 ) << 1 ); 
		break; //将进程加入工作集 目标进程路径名 
	case PROC_pgprot:
		record_append_size += ( ( event->action.action.do_proc_pgprot.path_len + 1 ) << 1 ); 
		break; //跨进程修改内存属性 目标进程路径名 
	case PROC_freevm:
		record_append_size += ( ( event->action.action.do_proc_freevm.path_len + 1 ) << 1 ); 
		break; //跨进程释放内存 目标进程路径名 
	case PROC_writevm:
		record_append_size += ( ( event->action.action.do_proc_writevm.path_len + 1 ) << 1 ); 
		break; //跨进程写内存 目标进程路径名 
	case PROC_readvm:
		record_append_size += ( ( event->action.action.do_proc_readvm.path_len + 1 ) << 1 ); 
		break; //跨进程读内存 目标进程路径名 
	case THRD_remote:
		record_append_size += ( ( event->action.action.do_thrd_remote.path_len + 1 ) << 1 ); 
		break; //创建远程线程 目标进程路径名 
	case THRD_setctxt:
		record_append_size += ( ( event->action.action.do_thrd_setctxt.path_len + 1 ) << 1 ); 
		break; //跨进程设置线程上下文 目标进程路径名 
	case THRD_suspend:
		record_append_size += ( ( event->action.action.do_thrd_suspend.path_len + 1 ) << 1 ); 
		break; //跨进程挂起线程 目标进程路径名 
	case THRD_resume:
		record_append_size += ( ( event->action.action.do_thrd_resume.path_len + 1 ) << 1 ); 
		break; //跨进程恢复线程 目标进程路径名 
	case THRD_exit:
		record_append_size += ( ( event->action.action.do_thrd_exit.path_len + 1 ) << 1 ); 
		break; //跨进程结束线程 目标进程路径名 
	case THRD_queue_apc:
		record_append_size += ( ( event->action.action.do_thrd_queue_apc.path_len + 1 ) << 1 ); 
		break; //跨进程排队APC 目标进程路径名 

		//MT_common
		//case COM_access:
		//	record_append_size += ( ( event->action.action.do_com_access.path_len + 1 ) << 1 ); 
		//	break; 
		//case ACCESS_COM:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 

		//MT_sysmon
	case SYS_settime:
		break; //设置系统时间 无 
	case SYS_link_knowndll:
		record_append_size += ( ( event->action.action.do_sys_link_knowndll.path_len + 1 ) << 1 ); 
		break; //建立KnownDlls链接 链接文件名 
	case SYS_open_physmm:
		break; //打开物理内存设备 无 
		//case ACCESS_MEM:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case SYS_read_physmm:
		break; //读物理内存 无 
	case SYS_write_physmm:
		break; //写物理内存 无 
	case SYS_load_kmod:
		record_append_size += ( ( event->action.action.do_sys_load_kmod.path_len + 1 ) << 1 ); 
		break; //加载内核模块 内核模块全路径 
		//case INSTALL_DRV:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case SYS_enumproc:
		break; //枚举进程 无 
	case SYS_regsrv:
		record_append_size += ( ( event->action.action.do_sys_regsrv.path_len + 1 ) << 1 ); 
		break; //注册服务 服务进程全路径 
	case SYS_opendev:
		record_append_size += ( ( event->action.action.do_sys_opendev.path_len + 1 ) << 1 ); 
		break; //打开设备 设备名 

		//MT_w32mon
	case W32_postmsg:
		record_append_size += ( ( event->action.action.do_w32_postmsg.path_len + 1 ) << 1 ); 
		break; //发送窗口消息（Post） 目标进程路径名 
	case W32_sendmsg:
		record_append_size += ( ( event->action.action.do_w32_sendmsg.path_len + 1 ) << 1 ); 
		break; //发送窗口消息（Send） 目标进程路径名 
	case W32_findwnd:
		record_append_size = 0; 
		break; //查找窗口 无 
	case W32_msghook:
		record_append_size += ( ( event->action.action.do_w32_msghook.path_len + 1 ) << 1 ); 
		break; //设置消息钩子 无 
		//case INSTALL_HOOK:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case W32_lib_inject:
		record_append_size += ( ( event->action.action.do_w32_lib_inject.path_len + 1 ) << 1 ); 
		break; //DLL注入 注入DLL路径名 

		//MT_netmon
	case NET_create:
		break; 
		//case SOCKET_CREATE:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_connect:
		break; //网络连接 远程地址（格式：IP：端口号） （网络监控） 
		//case SOCKET_CONNECT:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_listen:
		break; //监听端口 本机地址（格式：IP：端口号） 
		//case SOCKET_LISTEN:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_send:
		break; //发送数据包 远程地址（格式：IP：端口号） 
		//case SOCKET_SEND:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_recv:
		break; 
		//case SOCKET_RECV:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_accept:
		break; 
		//case SOCKET_ACCEPT:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break;
	case NET_http:
		break; //HTTP请求 HTTP请求路径（格式：域名/URL） 
		//case LOCATE_URL:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	case NET_icmp_send:
		record_append_size += ( ( event->action.action.do_net_icmp_send.path_len + 1 ) << 1 ); 
		break; 
		//case ICMP_SEND:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_icmp_recv:
		record_append_size += ( ( event->action.action.do_net_icmp_recv.path_len + 1 ) << 1 ); 
		break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE
		//case ICMP_RECV:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 


		//MT_behavior, 
	case BA_extract_hidden:
		record_append_size += ( ( event->action.action.do_ba_extract_hidden.path_len + 1 ) << 1 ); 
		break; //释放隐藏文件 释放文件路径名 （行为监控） 
	case BA_extract_pe:
		record_append_size += ( ( event->action.action.do_ba_extract_pe.path_len + 1 ) << 1 ); 
		break; //释放PE文件 释放文件路径名 
	case BA_self_copy:
		record_append_size += ( ( event->action.action.do_ba_self_copy.path_len + 1 ) << 1 ); 
		break; //自我复制 复制目标文件路径名 
	case BA_self_delete:
		record_append_size += ( ( event->action.action.do_ba_self_delete.path_len + 1 ) << 1 ); 
		break; //自我删除 删除文件路径名 
	case BA_ulterior_exec:
		record_append_size += ( ( event->action.action.do_ba_ulterior_exec.path_len + 1 ) << 1 ); 
		break; //隐秘执行 被执行映像路径名 
	case BA_invade_process:
		record_append_size += ( ( event->action.action.do_ba_invade_process.path_len + 1 ) << 1 ); 
		break; //入侵进程 目标进程路径名 
	case BA_infect_pe:
		record_append_size += ( ( event->action.action.do_ba_infect_pe.path_len + 1 ) << 1 ); 
		break; //感染PE文件 目标文件路径名 
	case BA_overwrite_pe:
		record_append_size += ( ( event->action.action.do_ba_overwrite_pe.path_len + 1 ) << 1 ); 
		break; //覆写PE文件 目标文件路径名 
	case BA_register_autorun:
		record_append_size += ( ( event->action.action.do_ba_register_autorun.path_len + 1 ) << 1 ); 
		break; //注册自启动项 自启动文件路径名 

		//case BA_other:
		//record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//break; 

	default:
		ASSERT( FALSE ); 
		record_append_size = 0; 
		break; 
	}

	return record_append_size; 
}

ULONG WINAPI calc_correct_all_append_size( r3_action_notify *event )
{
	ULONG append_size; 

	do 
	{
		append_size = 0; 
		append_size += event->data_size; 
		append_size += event->action.action.size; 

		ASSERT( event->frame_count >= 0 
			&& event->frame_count <= ARRAYSIZE( event->stack_frame ) ); 
		
		append_size += event->frame_count * sizeof( ULONGLONG ); 
		
	}while( FALSE );

	return append_size; 
}

LRESULT WINAPI compress_event_log_ex( r3_action_notify *action, event_log_compressed ** event_out )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI init_event_buf_other_part( r3_action_notify_buf *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		action->status = 0; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI compress_event_log( r3_action_notify *action, r3_action_notify_buf **event_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG record_size; 
	ULONG event_size; 
	r3_action_notify_buf *_action = NULL; 
	PVOID src_data; 
	PVOID dest_data; 

	do 
	{
		ASSERT( event_out != NULL ); 
		ASSERT( action != NULL ); 

		*event_out = NULL; 

#ifdef _DEBUG
		if( STATUS_SUCCESS != check_r3_action_notify_output_valid( action ) )
		{
			DBG_BP(); 
		}
#endif //_DEBUG

		record_size = action->action.action.size; 
		event_size = record_size; 
		
		event_size += ACTION_RECORD_OFFEST; 

		if( action->data_inited == FALSE )
		{
		}
		else
		{
			event_size += action->data_size; 
		}

		event_size += FIELD_OFFSET( r3_action_notify_buf, action ); 

		_action = ( r3_action_notify_buf* )malloc( event_size ); 
		if( _action == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		memcpy( &_action->action, action, ACTION_RECORD_OFFEST + record_size ); 

		if( action->data_size > 0 )
		{
			src_data = get_action_output_data( action ); 
			dest_data = ( PVOID )( ( BYTE* )&_action->action.action.action + record_size );  

			memcpy( dest_data, src_data, action->data_size ); 
		}

		_action->action.action.action.size = record_size; 
		_action->action.size = ACTION_RECORD_OFFEST + record_size + action->data_size; 

		ret = init_event_buf_other_part( _action ); 
		if( ret != ERROR_SUCCESS )
		{

		}
		*event_out = _action; 

	}while( FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		free( _action ); 
		ASSERT( *event_out == NULL ); 
	}

	return ret; 
}
