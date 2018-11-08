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

#ifndef __ACTION_CHECK_H__
#define __ACTION_CHECK_H__

/****
record more actions, the limit is max actions count of its policy,
need match n rules of the when one action occur.
****/

/******
action analyze by nfa:
constructing a nfa when one rule added, then just match once 
when one action occur.
******/

/**************************************************************************************
use nfa mode find the balance pointer, if the actions match the base rule, then record 
the action and the result to one rule.
***************************************************************************************/

#include "acl_define.h"
#include "action_type.h"
#include "ref_node.h"

typedef struct _sys_action_link
{
	HANDLE lock; 
	LIST_ENTRY actions; 
} sys_action_link, *psys_action_link; 

//typedef enum _action_reply
//{
//	ALLOW_ACTION, 
//	DENY_ACTION, 
//	MAX_ACTION_REPLY, 
//} action_reply; 

#define DEF_ACTION_REPLY ACTION_ALLOW 
typedef struct _policy_reply
{
	action_reply reply; 
} policy_reply, *ppolicy_reply; 

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct _sys_action_policy
{
	action_policy policy; 
	policy_reply reply; 
}sys_action_policy, *psys_action_policy; 

#ifdef __cplusplus
}
#endif //__cplusplus

typedef enum _action_source_type 
{
	ACTION_SOURCE_PROCESS, 
	ACTION_SOURCE_SYSTEM, 
} action_source_type, *paction_source_type ; 

struct _policy_holder; 

typedef NTSTATUS ( CALLBACK *compare_policy_callback )( struct _policy_holder *src_pol, 
struct _policy_holder *dst_pol ); 
typedef NTSTATUS ( CALLBACK *find_policy_callback )( struct _policy_holder *pol, PVOID param ); 
typedef NTSTATUS ( CALLBACK *check_policy_callback )( struct _action_source_info *source, struct _policy_holder *pol, PVOID param ); 

/********************************************************************************
group policy nfa 结构:
1.实体策略集，保存了所有的策略，之中所有的策略都不会重复。
2.组策略集，保存了所有的策略组，通过策略连接器连接至所有子策略。
********************************************************************************/
typedef struct _group_action_policy
{
	action_policy policy; 
}group_action_policy, *pgroup_action_policy; 

typedef struct _policy_holder
{
	ref_obj obj; 

	PVOID lock; 

	group_action_policy policy; 
} policy_holder, *ppolicy_holder; 

/**************************************************************
policy for learning have not action unit( one process now ) name,just learning action.
every action unit for learning checking will be binding with one 
instance of this struct.
***************************************************************/

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

INLINE LPCWSTR get_action_desc( sys_action_type type )
{
	LPCWSTR	desc = L""; 
	switch( type )
	{
	case OTHER_ACTION: 
		desc = L"OTHER_ACTION"; 
		break; 
	case EXEC_create:
		desc = L"EXEC_create"; 
		break; //进程启动 进程路径名 （执行监控） 

	case EXEC_destroy:
		desc = L"EXEC_destroy"; 
		break; //进程退出 进程路径名 

	case EXEC_module_load:
		desc = L"EXEC_module_load"; 
		break; //模块加载 模块路径名 

		//MT_filemon, 
	case FILE_touch:
		desc = L"FILE_touch"; 
		break; //创建文件 文件全路径 （文件监控） 	
	case FILE_open:
		desc = L"FILE_open"; 
		break; //打开文件 文件全路径 
		//case ACCESS_FILE:
		//	desc = L"ACCESS_FILE"; 
		//	break; 
	case FILE_read:
		desc = L"FILE_read"; 
		break; //读取文件 文件全路径 
	case FILE_write:
		desc = L"FILE_write"; 
		break; //写入文件 文件全路径 
		//case MODIFY_FILE:
		//	desc = L"MODIFY_FILE"; 
		//	break; 
	case FILE_modified:
		desc = L"FILE_modified"; 
		break; //文件被修改 文件全路径 
	case FILE_readdir:
		desc = L"FILE_readdir"; 
		break; //遍历目录 目录全路径 
	case FILE_remove:
		desc = L"FILE_remove"; 
		break; //删除文件 文件全路径 
		//case DELETE_FILE:
		//	desc = L"DELETE_FILE"; 
		//	break; 
	case FILE_rename:
		desc = L"FILE_rename"; 
		break; //重命名文件 文件全路径 
	case FILE_truncate:
		desc = L"FILE_truncate"; 
		break; //截断文件 文件全路径 
	case FILE_mklink:
		desc = L"FILE_mklink"; 
		break; //建立文件硬链接 文件全路径 
	case FILE_chmod:
		desc = L"FILE_chmod"; 
		break; //设置文件属性 文件全路径 
	case FILE_setsec:
		desc = L"FILE_setsec"; 
		break; //设置文件安全属性 文件全路径 
	case FILE_getinfo:
		desc = L"FILE_getinfo"; 
		break; //设置文件安全属性 文件全路径 
	case FILE_setinfo:
		desc = L"FILE_setinfo"; 
		break; //设置文件安全属性 文件全路径 
		//MT_regmon, 
	case REG_openkey:
		desc = L"REG_openkey"; 
		break; //打开注册表键 注册表键路径  （注册表监控） 
	case REG_mkkey:
		desc = L"REG_mkkey"; 
		break; //创建注册表键 注册表键路径 
		//case MODIFY_KEY:
		//desc = L"MODIFY_KEY"; 
		//	break; 
	case REG_rmkey:
		desc = L"REG_rmkey"; 
		break; //删除注册表键 注册表键路径 
	case REG_mvkey:
		desc = L"REG_mvkey"; 
		break; //重命名注册表键 注册表键路径 
	case REG_getinfo: 
		desc = L"REG_getinfo"; 
		break; 
	case REG_setinfo: 
		desc = L"REG_setinfo"; 
		break; 
	case REG_rmval:
		desc = L"REG_rmval"; 
		break; //删除注册表键 注册表键路径 
	case REG_getval:
		desc = L"REG_getval"; 
		break; //获取注册表值 注册表值路径 
	case REG_setval:
		desc = L"REG_setval"; 
		break; //设置注册表值 注册表值路径 
	case REG_loadkey:
		desc = L"REG_loadkey"; 
		break; //挂载注册表Hive文件 注册表键路径 
	case REG_replkey:
		desc = L"REG_replkey"; 
		break; //替换注册表键 注册表键路径 
	case REG_rstrkey:
		desc = L"REG_rstrkey"; 
		break; //导入注册表Hive文件 注册表键路径 
	case REG_setsec:
		desc = L"REG_setsec"; 
		break; //设置注册表键安全属性 注册表键路径 
		//MT_procmon, 
	case PROC_exec:
		desc = L"PROC_exec"; 
		break; //创建进程 目标进程路径名  （进程监控）
		//case CREATE_PROC:
		//desc = L"CREATE_PROC"; 
		//	break; 
	case PROC_open:
		desc = L"PROC_open"; 
		break; //打开进程 目标进程路径名 
	case PROC_debug:
		desc = L"PROC_debug"; 
		break; //调试进程 目标进程路径名 
	case PROC_suspend:
		desc = L"PROC_suspend"; 
		break; //挂起进程 目标进程路径名 
	case PROC_resume:
		desc = L"PROC_resume"; 
		break; //恢复进程 目标进程路径名 
	case PROC_exit:
		desc = L"PROC_kill"; 
		break; //结束进程 目标进程路径名 
		//case TERMINATE_PROC:
		//desc = L"TERMINATE_PROC"; 
		//	break; 
	case PROC_job:
		desc = L"PROC_job"; 
		break; //将进程加入工作集 目标进程路径名 
	case PROC_pgprot:
		desc = L"PROC_pgprot"; 
		break; //跨进程修改内存属性 目标进程路径名 
	case PROC_freevm:
		desc = L"PROC_freevm"; 
		break; //跨进程释放内存 目标进程路径名 
	case PROC_writevm:
		desc = L"PROC_writevm"; 
		break; //跨进程写内存 目标进程路径名 
	case PROC_readvm:
		desc = L"PROC_readvm"; 
		break; //跨进程读内存 目标进程路径名 
	case THRD_remote:
		desc = L"THRD_remote"; 
		break; //创建远程线程 目标进程路径名 
	case THRD_setctxt:
		desc = L"THRD_setctxt"; 
		break; //跨进程设置线程上下文 目标进程路径名 
	case THRD_suspend:
		desc = L"THRD_suspend"; 
		break; //跨进程挂起线程 目标进程路径名 
	case THRD_resume:
		desc = L"THRD_resume"; 
		break; //跨进程恢复线程 目标进程路径名 
	case THRD_exit:
		desc = L"THRD_kill"; 
		break; //跨进程结束线程 目标进程路径名 
	case THRD_queue_apc:
		desc = L"THRD_queue_apc"; 
		break; //跨进程排队APC 目标进程路径名 

		//MT_common
	case COM_access:
		desc = L"COM_access"; 
		break; 
	
	case PORT_read: 
		desc = L"PORT_read"; 
		break; 

	case PORT_write: 
		desc = L"PORT_write"; 
		break; 
	case PORT_urb:
		desc = L"PORT_urb"; 
		//case ACCESS_COM:
		//desc = L"ACCESS_COM"; 
		//	break; 

		//MT_sysmon
	case SYS_settime:
		desc = L"SYS_settime"; 
		break; //设置系统时间 无 
	case SYS_link_knowndll:
		desc = L"SYS_link_knowndll"; 
		break; //建立KnownDlls链接 链接文件名 
	case SYS_open_physmm:
		desc = L"SYS_open_physmm"; 
		break; //打开物理内存设备 无 
		//case ACCESS_MEM:
		//desc = L"ACCESS_MEM"; 
		//	break; 
	case SYS_read_physmm:
		desc = L"SYS_read_physmm"; 
		break; //读物理内存 无 
	case SYS_write_physmm:
		desc = L"SYS_write_physmm"; 
		break; //写物理内存 无 
	case SYS_load_kmod:
		desc = L"SYS_load_kmod"; 
		break; //加载内核模块 内核模块全路径 
		//case INSTALL_DRV:
		//desc = L"INSTALL_DRV"; 
		//	break; 
	case SYS_enumproc:
		desc = L"SYS_enumproc"; 
		break; //枚举进程 无 
	case SYS_regsrv:
		desc = L"SYS_regsrv"; 
		break; //注册服务 服务进程全路径 
	case SYS_opendev:
		desc = L"SYS_opendev"; 
		break; //打开设备 设备名 

		//MT_w32mon
	case W32_postmsg:
		desc = L"W32_postmsg"; 
		break; //发送窗口消息（Post） 目标进程路径名 
	case W32_sendmsg:
		desc = L"W32_sendmsg"; 
		break; //发送窗口消息（Send） 目标进程路径名 
	case W32_findwnd:
		desc = L"W32_findwnd"; 
		break; //查找窗口 无 
	case W32_msghook:
		desc = L"W32_msghook"; 
		break; //设置消息钩子 无 
		//case INSTALL_HOOK:
		//desc = L"INSTALL_HOOK"; 
		//	break; 
	case W32_lib_inject:
		desc = L"W32_lib_inject"; 
		break; //DLL注入 注入DLL路径名 

		//MT_netmon
	case NET_create:
		desc = L"NET_create"; 
		break; 
	case NET_connect:
		desc = L"NET_connect"; 
		break; //网络连接 远程地址（格式：IP：端口号） （网络监控） 
	case NET_listen:
		desc = L"NET_listen"; 
		break; //监听端口 本机地址（格式：IP：端口号） 
	case NET_send:
		desc = L"NET_send"; 
		break; //发送数据包 远程地址（格式：IP：端口号） 
	case NET_recv:
		desc = L"NET_recv"; 
		break; 
	case NET_accept:
		desc = L"NET_accept"; 
		break; 
	case NET_dns:
		desc = L"NET_dns"; 
		break; 

	case NET_http:
		desc = L"NET_http"; 
		break; //HTTP请求 HTTP请求路径（格式：域名/URL） 
	case NET_icmp_send:
		desc = L"NET_icmp_send"; 
		break; 
		//case ICMP_SEND:
		//desc = L"ICMP_SEND"; 
		//	break; 
	case NET_icmp_recv:
		desc = L"NET_icmp_recv"; 
		break; 

		//MT_behavior, 
	case BA_extract_hidden:
		desc = L"BA_extract_hidden"; 
		break; //释放隐藏文件 释放文件路径名 （行为监控） 
	case BA_extract_pe:
		desc = L"BA_extract_pe"; 
		break; //释放PE文件 释放文件路径名 
	case BA_self_copy:
		desc = L"BA_self_copy"; 
		break; //自我复制 复制目标文件路径名 
	case BA_self_delete:
		desc = L"BA_self_delete"; 
		break; //自我删除 删除文件路径名 
	case BA_ulterior_exec:
		desc = L"BA_ulterior_exec"; 
		break; //隐秘执行 被执行映像路径名 
	case BA_invade_process:
		desc = L"BA_invade_process"; 
		break; //入侵进程 目标进程路径名 
	case BA_infect_pe:
		desc = L"BA_infect_pe"; 
		break; //感染PE文件 目标文件路径名 
	case BA_overwrite_pe:
		desc = L"BA_overwrite_pe"; 
		break; //覆写PE文件 目标文件路径名 
	case BA_register_autorun:
		desc = L"BA_register_autorun"; 
		break; //注册自启动项 自启动文件路径名 
	default:
		ASSERT( FALSE ); 
		break; 
	}

	return desc; 
}

NTSTATUS init_action_check_context(); 
NTSTATUS uninit_action_check_context(); 

#ifdef TEST_IN_RING3
INLINE NTSTATUS _collect_action_context( action_context *info )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG name_len; 

	ASSERT( info != NULL ); 

	info->proc_id = 0; 
	*info->proc_name = '\0'; 
	info->thread_id = 0; 

	do 
	{
		info->proc_id = GetCurrentProcessId(); 
		name_len = GetModuleFileNameW( NULL, ( WCHAR* )info->proc_name, sizeof( info->proc_name ) >> 1 ); 
		if( name_len == 0 )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		info->proc_name_len = name_len >> 1; 

		if( info->proc_name[ info->proc_name_len - 1 ] != L'\0' )
		{
			info->proc_name[ info->proc_name_len - 1 ] = L'\0'; 
			info->proc_name_len -= 1; 
		}

		info->thread_id = ( ULONG )GetCurrentThreadId(); 
	}while( FALSE );

	return ntstatus; 
}
#endif //TEST_IN_RING3


NTSTATUS check_sys_action( r3_action_notify *action, 
						  action_reply *resp ); 

NTSTATUS load_action_policies(); 


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__ACTION_CHECK_H__