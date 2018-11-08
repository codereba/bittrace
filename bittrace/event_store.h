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

#ifndef __EVENT_STORE_H__
#define __EVENT_STORE_H__

INLINE ULONG calc_sys_action_record_size( sys_action_type type )
{
	ULONG record_size; 

	record_size = ACTION_RECORD_PARAMETERS_OFFSET; 

	switch( type )
	{
	case EXEC_create:
		record_size += sizeof( exec_create ); 
		break; //进程启动 进程路径名 （执行监控） 

	case EXEC_destroy:
		record_size += sizeof( exec_destroy ); 
		break; //进程退出 进程路径名 

	case EXEC_module_load:
		record_size += sizeof( exec_module_load ); 
		break; //模块加载 模块路径名 

		//MT_filemon, 
	case FILE_touch:
		record_size += sizeof( file_touch ); 
		break; //创建文件 文件全路径 （文件监控） 	
	case FILE_open:
		record_size += sizeof( file_open ); 
		break; //打开文件 文件全路径 
		//case ACCESS_FILE:
		//	record_size += sizeof( file_open ); 
		//	break; 
	case FILE_read:
		record_size += sizeof( _file_read ); 
		break; //读取文件 文件全路径 
	case FILE_write:
		record_size += sizeof( _file_write ); 
		break; //写入文件 文件全路径 
		//case MODIFY_FILE:
		//	record_size += sizeof( file_modified ); 
		//	break; 
	case FILE_modified:
		record_size += sizeof( file_modified ); 
		break; //文件被修改 文件全路径 
	case FILE_readdir:
		record_size += sizeof( file_readdir ); 
		break; //遍历目录 目录全路径 
	case FILE_remove:
		record_size += sizeof( file_remove ); 
		break; //删除文件 文件全路径 
		//case DELETE_FILE:
		//	record_size += sizeof( file_remove ); 
		//	break; 
	case FILE_rename:
		record_size += sizeof( file_rename ); 
		break; //重命名文件 文件全路径 
	case FILE_truncate:
		record_size += sizeof( file_truncate ); 
		break; //截断文件 文件全路径 
	case FILE_mklink:
		record_size += sizeof( file_mklink ); 
		break; //建立文件硬链接 文件全路径 
	case FILE_chmod:
		record_size += sizeof( file_chmod ); 
		break; //设置文件属性 文件全路径 
	case FILE_setsec:
		record_size += sizeof( file_setsec ); 
		break; //设置文件安全属性 文件全路径 

		//MT_regmon, 
	case REG_openkey:
		record_size += sizeof( reg_openkey ); 
		break; //打开注册表键 注册表键路径  （注册表监控） 
	case REG_mkkey:
		record_size += sizeof( reg_mkkey ); 
		break; //创建注册表键 注册表键路径 
		//case MODIFY_KEY:
		//	record_size += sizeof( reg_mkkey ); 
		//	break; 
	case REG_rmkey:
		record_size += sizeof( reg_rmkey ); 
		break; //删除注册表键 注册表键路径 
	case REG_mvkey:
		record_size += sizeof( reg_mvkey ); 
		break; //重命名注册表键 注册表键路径 
	case REG_getinfo: 
		record_size += sizeof( reg_getinfo ); 
		break; 
	case REG_setinfo: 
		record_size += sizeof( reg_setinfo ); 
		break; 
	case REG_rmval:
		record_size += sizeof( reg_rmval ); 
		break; //删除注册表键 注册表键路径 
	case REG_getval:
		record_size += sizeof( reg_getval ); 
		break; //获取注册表值 注册表值路径 
	case REG_setval:
		record_size += sizeof( reg_setval ); 
		break; //设置注册表值 注册表值路径 
	case REG_loadkey:
		record_size += sizeof( reg_loadkey ); 
		break; //挂载注册表Hive文件 注册表键路径 
	case REG_replkey:
		record_size += sizeof( reg_replkey ); 
		break; //替换注册表键 注册表键路径 
	case REG_rstrkey:
		record_size += sizeof( reg_rstrkey ); 
		break; //导入注册表Hive文件 注册表键路径 
	case REG_setsec:
		record_size += sizeof( reg_setsec ); 
		break; //设置注册表键安全属性 注册表键路径 

		//MT_procmon, 
	case PROC_exec:
		record_size += sizeof( proc_exec ); 
		break; //创建进程 目标进程路径名  （进程监控）
		//case CREATE_PROC:
		//	record_size += sizeof( proc_exec ); 
		//	break; 
	case PROC_open:
		record_size += sizeof( proc_open ); 
		break; //打开进程 目标进程路径名 
	case PROC_debug:
		record_size += sizeof( proc_debug ); 
		break; //调试进程 目标进程路径名 
	case PROC_suspend:
		record_size += sizeof( proc_suspend ); 
		break; //挂起进程 目标进程路径名 
	case PROC_resume:
		record_size += sizeof( proc_resume ); 
		break; //恢复进程 目标进程路径名 
	case PROC_exit:
		record_size += sizeof( proc_kill ); 
		break; //结束进程 目标进程路径名 
		//case TERMINATE_PROC:
		//	record_size += sizeof( proc_kill ); 
		//	break; 
	case PROC_job:
		record_size += sizeof( proc_job ); 
		break; //将进程加入工作集 目标进程路径名 
	case PROC_pgprot:
		record_size += sizeof( proc_pgprot ); 
		break; //跨进程修改内存属性 目标进程路径名 
	case PROC_freevm:
		record_size += sizeof( proc_freevm ); 
		break; //跨进程释放内存 目标进程路径名 
	case PROC_writevm:
		record_size += sizeof( proc_writevm ); 
		break; //跨进程写内存 目标进程路径名 
	case PROC_readvm:
		record_size += sizeof( proc_readvm ); 
		break; //跨进程读内存 目标进程路径名 
	case THRD_remote:
		record_size += sizeof( thrd_remote ); 
		break; //创建远程线程 目标进程路径名 
	case THRD_setctxt:
		record_size += sizeof( thrd_setctxt ); 
		break; //跨进程设置线程上下文 目标进程路径名 
	case THRD_suspend:
		record_size += sizeof( thrd_suspend ); 
		break; //跨进程挂起线程 目标进程路径名 
	case THRD_resume:
		record_size += sizeof( thrd_resume ); 
		break; //跨进程恢复线程 目标进程路径名 
	case THRD_exit:
		record_size += sizeof( thrd_kill ); 
		break; //跨进程结束线程 目标进程路径名 
	case THRD_queue_apc:
		record_size += sizeof( thrd_queue_apc ); 
		break; //跨进程排队APC 目标进程路径名 

	case SYS_settime:
		record_size += sizeof( sys_settime ); 
		break; //设置系统时间 无 
	case SYS_link_knowndll:
		record_size += sizeof( sys_link_knowndll ); 
		break; //建立KnownDlls链接 链接文件名 
	case SYS_open_physmm:
		record_size += sizeof( sys_open_physmm ); 
		break; //打开物理内存设备 无 
		//case ACCESS_MEM:
		//	record_size += sizeof( sys_open_physmm ); 
		//	break; 
	case SYS_read_physmm:
		record_size += sizeof( sys_read_physmm ); 
		break; //读物理内存 无 
	case SYS_write_physmm:
		record_size += sizeof( sys_write_physmm ); 
		break; //写物理内存 无 
	case SYS_load_kmod:
		record_size += sizeof( sys_load_kmod ); 
		break; //加载内核模块 内核模块全路径 
		//case INSTALL_DRV:
		//	record_size += sizeof( sys_load_kmod ); 
		//	break; 
	case SYS_enumproc:
		record_size += sizeof( sys_enumproc ); 
		break; //枚举进程 无 
	case SYS_regsrv:
		record_size += sizeof( sys_regsrv ); 
		break; //注册服务 服务进程全路径 
	case SYS_opendev:
		record_size += sizeof( sys_opendev ); 
		break; //打开设备 设备名 

		//MT_w32mon
	case W32_postmsg:
		record_size += sizeof( w32_postmsg ); 
		break; //发送窗口消息（Post） 目标进程路径名 
	case W32_sendmsg:
		record_size += sizeof( w32_sendmsg ); 
		break; //发送窗口消息（Send） 目标进程路径名 
	case W32_findwnd:
		record_size += sizeof( w32_findwnd ); 
		break; //查找窗口 无 
	case W32_msghook:
		record_size += sizeof( w32_msghook ); 
		break; //设置消息钩子 无 
		//case INSTALL_HOOK:
		//	record_size += sizeof( w32_msghook ); 
		//	break; 
	case W32_lib_inject:
		record_size += sizeof( w32_lib_inject ); 
		break; //DLL注入 注入DLL路径名 

		//MT_netmon
	case NET_create:
		record_size += sizeof( net_create ); 
		break; 
		//case SOCKET_CREATE:
		//	record_size += sizeof( net_create ); 
		//	break; 
	case NET_connect:
		record_size += sizeof( net_connect ); 
		break; //网络连接 远程地址（格式：IP：端口号） （网络监控） 
		//case SOCKET_CONNECT:
		//	record_size += sizeof( net_connect ); 
		//	break; 
	case NET_listen:
		record_size += sizeof( net_listen ); 
		break; //监听端口 本机地址（格式：IP：端口号） 
		//case SOCKET_LISTEN:
		//	record_size += sizeof( net_listen ); 
		//	break; 
	case NET_send:
		record_size += sizeof( net_send ); 
		break; //发送数据包 远程地址（格式：IP：端口号） 
		//case SOCKET_SEND:
		//	record_size += sizeof( net_send ); 
		//	break; 
	case NET_recv:
		record_size += sizeof( net_recv ); 
		break; 
		//case SOCKET_RECV:
		//	record_size += sizeof( net_recv ); 
		//	break; 
	case NET_accept:
		record_size += sizeof( net_accept ); 
		break; 
		//case SOCKET_ACCEPT:
		//	record_size += sizeof( net_accept ); 
		//	break; 
	case NET_dns:
		record_size += sizeof( net_dns ); 
		break; //
	case NET_http:
		record_size += sizeof( net_http ); 
		break; //HTTP请求 HTTP请求路径（格式：域名/URL） 
		//case LOCATE_URL:
		//	record_size += sizeof( ); 
		//	break; 
	//case NET_icmp_send:
	//	record_size += sizeof( ipv4_icmp_action ); 
	//	break; 
	//	//case ICMP_SEND:
	//	//	record_size += sizeof( ); 
	//	//	break; 
	//case NET_icmp_recv:
	//	record_size += sizeof( ipv4_icmp_action ); 
	//	break; 
		//case ICMP_RECV:
		//	record_size += sizeof( ); 
		//	break; 


		//MT_behavior, 
	case BA_extract_hidden:
		record_size += sizeof( ba_extract_hidden ); 
		break; //释放隐藏文件 释放文件路径名 （行为监控） 
	case BA_extract_pe:
		record_size += sizeof( ba_extract_pe ); 
		break; //释放PE文件 释放文件路径名 
	case BA_self_copy:
		record_size += sizeof( ba_self_copy ); 
		break; //自我复制 复制目标文件路径名 
	case BA_self_delete:
		record_size += sizeof( ba_self_delete ); 
		break; //自我删除 删除文件路径名 
	case BA_ulterior_exec:
		record_size += sizeof( ba_ulterior_exec ); 
		break; //隐秘执行 被执行映像路径名 
	case BA_invade_process:
		record_size += sizeof( ba_invade_process ); 
		break; //入侵进程 目标进程路径名 
	case BA_infect_pe:
		record_size += sizeof( ba_infect_pe ); 
		break; //感染PE文件 目标文件路径名 
	case BA_overwrite_pe:
		record_size += sizeof( ba_overwrite_pe ); 
		break; //覆写PE文件 目标文件路径名 
	case BA_register_autorun:
		record_size += sizeof( ba_register_autorun ); 
		break; //注册自启动项 自启动文件路径名 

		//case BA_other:
		//record_size += sizeof( ); 
		//break; 

	default:
		ASSERT( FALSE ); 
		record_size = 0; 
		break; 
	}

	return record_size; 
}

//r3_action_notify
typedef struct _event_log_compressed
{
	ULONG size; 
	ULONG data_size; 
	ULARGE_INTEGER id; 
	ULONG frame_count; 
	sys_action_info action; 
	BYTE append[ 1 ]; 
	/*************************
	the data that's saved in the append: 
	1.append data in action record (can compressed)
	2.stack frame (can compressed)
	3.data
	4.the context of the *can compress* too, but process it that is more complex.
	*************************/

	/****************************************************
	2 steps to resolve the store size problem:
	1.correct the size that's allocated in the kernel.
	2.compress the size that's save in ring 3.
	****************************************************/

} event_log_compressed, *pevent_log_compressed; 

#define EVENT_SEARCH_RESULT 0x00000001

typedef struct _r3_action_notify_buf
{
	ULONG status; 
	//LPWSTR stack_dump; 
	r3_action_notify action; 
} r3_action_notify_buf, *pr3_action_notify_buf; 

ULONG WINAPI calc_correct_record_append_size( r3_action_notify *event ); 

ULONG WINAPI calc_correct_all_append_size( r3_action_notify *event ); 

LRESULT WINAPI compress_event_log( r3_action_notify *action, r3_action_notify_buf **event_out ); 

LRESULT WINAPI compress_event_log_ex( r3_action_notify *action, event_log_compressed ** event_out ); 

#endif //__EVENT_STORE_H__