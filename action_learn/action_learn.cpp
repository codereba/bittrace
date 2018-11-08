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
 #include "common_func.h"
#include "ring0_2_ring3.h"
#include "acl_define.h"
#include "hash_table.h"
#include "ref_node.h"
#include "rbtree.h"
#include "sys_event_define.h"
#include "action_source.h"
#include "action_check.h"
#include "action_group_policy.h"
#include "action_learn.h"

#include <conio.h>

group_policy_nfa global_nfa = { 0 }; 

LRESULT check_path_policy_ex( sys_action_link *actions )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	} while ( FALSE ); 

	return ret; 
}

/****************************************************************
switch action type template
switch( type )
{
case EXEC_create:
break; //进程启动 进程路径名 （执行监控） 

case EXEC_destroy:
break; //进程退出 进程路径名 

case EXEC_module_load:
break; //模块加载 模块路径名 

//MT_filemon, 
case FILE_touch:
break; //创建文件 文件全路径 （文件监控） 	
case FILE_open:
break; //打开文件 文件全路径 
//case ACCESS_FILE:
//	break; 
case FILE_read:
break; //读取文件 文件全路径 
case FILE_write:
break; //写入文件 文件全路径 
//case MODIFY_FILE:
//	break; 
case FILE_modified:
break; //文件被修改 文件全路径 
case FILE_readdir:
break; //遍历目录 目录全路径 
case FILE_remove:
break; //删除文件 文件全路径 
//case DELETE_FILE:
//	break; 
case FILE_rename:
break; //重命名文件 文件全路径 
case FILE_truncate:
break; //截断文件 文件全路径 
case FILE_mklink:
break; //建立文件硬链接 文件全路径 
case FILE_chmod:
break; //设置文件属性 文件全路径 
case FILE_setsec:
break; //设置文件安全属性 文件全路径 

//MT_regmon, 
case REG_openkey:
break; //打开注册表键 注册表键路径  （注册表监控） 
case REG_mkkey:
break; //创建注册表键 注册表键路径 
//case MODIFY_KEY:
//	record_size += sizeof( reg_mkkey ); 
//	break; 
case REG_rmkey:
break; //删除注册表键 注册表键路径 
case REG_mvkey:
break; //重命名注册表键 注册表键路径 
case REG_rmval:
break; //删除注册表键 注册表键路径 
case REG_getval:
break; //获取注册表值 注册表值路径 
case REG_setval:
break; //设置注册表值 注册表值路径 
case REG_loadkey:
break; //挂载注册表Hive文件 注册表键路径 
case REG_replkey:
break; //替换注册表键 注册表键路径 
case REG_rstrkey:
break; //导入注册表Hive文件 注册表键路径 
case REG_setsec:
break; //设置注册表键安全属性 注册表键路径 

//MT_procmon, 
case PROC_exec:
break; //创建进程 目标进程路径名  （进程监控）
//case CREATE_PROC:
//	record_size += sizeof( proc_exec ); 
//	break; 
case PROC_open:
break; //打开进程 目标进程路径名 
case PROC_debug:
break; //调试进程 目标进程路径名 
case PROC_suspend:
break; //挂起进程 目标进程路径名 
case PROC_resume:
break; //恢复进程 目标进程路径名 
case PROC_kill:
break; //结束进程 目标进程路径名 
//case TERMINATE_PROC:
//	record_size += sizeof( proc_kill ); 
//	break; 
case PROC_job:
break; //将进程加入工作集 目标进程路径名 
case PROC_pgprot:
break; //跨进程修改内存属性 目标进程路径名 
case PROC_freevm:
break; //跨进程释放内存 目标进程路径名 
case PROC_writevm:
break; //跨进程写内存 目标进程路径名 
case PROC_readvm:
break; //跨进程读内存 目标进程路径名 
case THRD_remote:
break; //创建远程线程 目标进程路径名 
case THRD_setctxt:
break; //跨进程设置线程上下文 目标进程路径名 
case THRD_suspend:
break; //跨进程挂起线程 目标进程路径名 
case THRD_resume:
break; //跨进程恢复线程 目标进程路径名 
case THRD_kill:
break; //跨进程结束线程 目标进程路径名 
case THRD_queue_apc:
break; //跨进程排队APC 目标进程路径名 

//MT_common
//case COM_access:
//	record_size += sizeof( ); 
//	break; 
//case ACCESS_COM:
//	record_size += sizeof( ); 
//	break; 

//MT_sysmon
case SYS_settime:
break; //设置系统时间 无 
case SYS_link_knowndll:
break; //建立KnownDlls链接 链接文件名 
case SYS_open_physmm:
break; //打开物理内存设备 无 
//case ACCESS_MEM:
//	break; 
case SYS_read_physmm:
break; //读物理内存 无 
case SYS_write_physmm:
break; //写物理内存 无 
case SYS_load_kmod:
break; //加载内核模块 内核模块全路径 
//case INSTALL_DRV:
//	record_size += sizeof( sys_load_kmod ); 
//	break; 
case SYS_enumproc:
break; //枚举进程 无 
case SYS_regsrv:
break; //注册服务 服务进程全路径 
case SYS_opendev:
break; //打开设备 设备名 

//MT_w32mon
case W32_postmsg:
break; //发送窗口消息（Post） 目标进程路径名 
case W32_sendmsg:
break; //发送窗口消息（Send） 目标进程路径名 
case W32_findwnd:
break; //查找窗口 无 
case W32_msghook:
break; //设置消息钩子 无 
//case INSTALL_HOOK:
//	record_size += sizeof( w32_msghook ); 
//	break; 
case W32_lib_inject:
break; //DLL注入 注入DLL路径名 

//MT_netmon
case NET_create:
break; 
//case SOCKET_CREATE:
//	record_size += sizeof( net_create ); 
//	break; 
case NET_connect:
break; //网络连接 远程地址（格式：IP：端口号） （网络监控） 
//case SOCKET_CONNECT:
//	record_size += sizeof( net_connect ); 
//	break; 
case NET_listen:
break; //监听端口 本机地址（格式：IP：端口号） 
//case SOCKET_LISTEN:
//	record_size += sizeof( net_listen ); 
//	break; 
case NET_send:
break; //发送数据包 远程地址（格式：IP：端口号） 
//case SOCKET_SEND:
//	record_size += sizeof( net_send ); 
//	break; 
case NET_recv:
break; 
//case SOCKET_RECV:
//	record_size += sizeof( net_recv ); 
//	break; 
case NET_accept:
break; 
//case SOCKET_ACCEPT:
//	record_size += sizeof( net_accept ); 
//	break; 
case NET_http:
break; //HTTP请求 HTTP请求路径（格式：域名/URL） 
//case LOCATE_URL:
//	record_size += sizeof( ); 
//	break; 
case NET_icmp_send:
break; 
//case ICMP_SEND:
//	record_size += sizeof( ); 
//	break; 
case NET_icmp_recv:
break; 
//case ICMP_RECV:
//	record_size += sizeof( ); 
//	break; 


//MT_behavior, 
case BA_extract_hidden:
break; //释放隐藏文件 释放文件路径名 （行为监控） 
case BA_extract_pe:
break; //释放PE文件 释放文件路径名 
case BA_self_copy:
break; //自我复制 复制目标文件路径名 
case BA_self_delete:
break; //自我删除 删除文件路径名 
case BA_ulterior_exec:
break; //隐秘执行 被执行映像路径名 
case BA_invade_process:
break; //入侵进程 目标进程路径名 
case BA_infect_pe:
break; //感染PE文件 目标文件路径名 
case BA_overwrite_pe:
break; //覆写PE文件 目标文件路径名 
case BA_register_autorun:
break; //注册自启动项 自启动文件路径名 

//case BA_other:
//record_size += sizeof( ); 
//break; 

default:
ASSERT( FALSE ); 
break; 
}
*************************************************************/

LRESULT action_is_match_learning_target( sys_action_record *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	} while ( FALSE ); 

	return ret; 
}

LRESULT study_this_action( sys_action_record *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	} while ( FALSE );

	return ret; 
}

LRESULT action_is_match_learing_target( sys_action_record *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	} while ( FALSE );

	return ret; 	
}

LRESULT action_study( sys_action_record *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do
	{
		ret = action_is_match_learing_target( action ); 
		if( ret == ERROR_SUCCESS )
		{
			ret = study_this_action( action );
		}
	}while( FALSE ); 

	return ret; 
}

NTSTATUS CALLBACK lock_mutex_func( PVOID lock )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG wait_ret; 
	ULONG err_code; 

	do 
	{
		ASSERT( lock != NULL ); 

		wait_ret = WaitForSingleObject( ( HANDLE )lock, INFINITE ); 

		if( wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED )
		{
			ASSERT( wait_ret == WAIT_FAILED ); 

			err_code = GetLastError(); 

			if( err_code != ERROR_SUCCESS )
			{
				ntstatus = STATUS_UNSUCCESSFUL; //ERROR_ERRORS_ENCOUNTERED; 
			}
			else
			{
				dbg_print( MSG_IMPORTANT, "why wait lock return error, but last error is success %u\n", err_code ); 
				ASSERT( FALSE ); 
			}
			break; 
		}

	} while ( FALSE );

	return ntstatus; 
}

NTSTATUS CALLBACK unlock_mutex_func( PVOID mutex )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	ASSERT( mutex != NULL ); 

	do 
	{
		_ret = ReleaseMutex( ( HANDLE )mutex ); 
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			break; 
		}

	} while ( FALSE );

	return ( NTSTATUS )ret; 
}

NTSTATUS on_thread_created( PVOID thread_obj )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

/*****************************************************************
action checking process:
1.add the action source.
2.the action source do one action, then encapsolute the action to 
the structure of the action description, then input it to action 
checking nfa. 
3.the nfa first got the action source of the action.
4.check nfa check the current state of the action source.
5.nfa get the policies series of the corresponding action series.
6.nfa check the policies from the action and the current action state
of the action source.
7.nfa get the response of the action series, or change action state 
of the action source, return.
*****************************************************************/

/*****************************************************************
注意：
将行为分为3种：
1.未知行为源行为，对其行为需要进行SANDBOX
2.行为源白名单行为，对其行为进行直接放过。
3.行为源黑名单行为，对其行为进行直接阻止。

或使用简单的方法，判断的基本单元是行为源，而不是行为：
1.未知行为源
2.白名单行为源
3.黑名单行为源。

意思就是对同一组指令序列（或同一个行为）只分析一次。
因为SANDBOX运行机制会占用一定量的资源，不合适同时对大量的指令序列
进行处理。
*****************************************************************/
/******************************************************************
check the policy for the action, get the response for action series.
or change the action source state to correct state when action occur 
in the action state now.
******************************************************************/
NTSTATUS source_do_action( sys_action_record *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	} while ( FALSE );

	return ntstatus; 
}


ULONG number_policies[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }; 


NTSTATUS find_thread_action_state( ULONG thread_id, policy_state **state_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	
	do 
	{
		ASSERT( state_out != NULL ); 

		*state_out = NULL; 

	}while( FALSE );

	return ntstatus; 
}

LRESULT CALLBACK _check_action_policy( policy_holder *src_pol, PVOID param )
{
	NTSTATUS ntstatus; 
	data_trace_option trace_option; 
	sys_action_desc *action; 
	action_response_type reply; 
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		action = ( sys_action_desc* )param; 

		ntstatus = check_sys_acl( action, &trace_option, &reply ); 

	}while( FALSE );

	return ret; 
}

NTSTATUS compound_action_policy( policy_holder *src_policy, policy_holder *dest_policy )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS _compound_action_policy( policy_holder *src_policy, PVOID dest_policy )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

LRESULT CALLBACK _compare_number_policy( policy_holder *src_pol, PVOID policy )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( &src_pol->policy == policy )
		{
			break; 
		}
		else
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT CALLBACK compare_number_policy( policy_holder *src_pol, policy_holder *dst_pol )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( &src_pol->policy == &dst_pol->policy )
		{
			break; 
		}
		else
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT CALLBACK check_number_policy( PVOID context, policy_holder *pol_holder, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( pol_holder != NULL ); 

		if( ( ULONG )&pol_holder->policy == ( ULONG )param )
		{
			break; 
		}
		else
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE ); 

	return ret; 
}

INLINE NTSTATUS __collect_action_context( ULONG proc_id, action_source_info *source )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	} while ( FALSE ); 
	
	return ntstatus; ;
}

NTSTATUS get_cur_action_context( action_source_info *proc_sub, 
								action_context *ctx )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	//action_context *_ctx = NULL; 
	//proc_info *_proc_sub = NULL; 

	do 
	{
		//ASSERT( action != NULL );  
		ASSERT( proc_sub != NULL ); 

		ntstatus = _collect_action_context( ctx ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

	}while( FALSE ); 
	return ntstatus; 
}

#if 0
#ifdef SELF_TEST

#define TEST_ACTION_POLICY_COUNT 3
#define TEST_ACTION_COUNT 2

sys_action_policy all_test_policy[ TEST_ACTION_POLICY_COUNT ] = { SYS_ACTION_END, { 0 }, { ACTION_ALLOW } }; 
//sys_action_record all_test_action[ TEST_ACTION_COUNT ] = { SYS_ACTION_END, { 0 } }; 
sys_action_record all_test_action[ TEST_ACTION_COUNT ] = { 0 }; 

NTSTATUS init_test_action_policies()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG i; 

	do 
	{
		for( i = 0; i < ARRAY_SIZE( all_test_action ); i ++ )
		{
			switch( i )
			{
			case 0:
				all_test_policy[ i ].policy.type = NET_accept; 
				all_test_policy[ i ].policy.do_net_accept.data_len = 0; 
				all_test_policy[ i ].policy.do_net_accept.protocol = PROT_TCP; 
				all_test_policy[ i ].policy.do_net_accept.ip_addr = 0; 
				all_test_policy[ i ].policy.do_net_accept.port = 8001; 
				break; 
			case 1:
				//LPCWSTR pathname; //文件全路径
				//ULONG pid; //动作发起者进程ID 
				//ULONG tid; //动作发起者线程ID 
				//ULONG offset; //文件内偏移 
				//ULONG data_len; //数据长度 
				//NTSTATUS result; //动作完成结果(NTSTATUS) 
				all_test_policy[ i ].policy.type = FILE_modified; 
				*all_test_policy[ i ].policy.do_file_write.path_name = L'\0'; 
				all_test_policy[ i ].policy.do_file_write.offset = 100; 
				all_test_policy[ i ].policy.do_file_write.data_len = 1000; 
				//all_test_policy[ i ].policy.do_file_write.result = STATUS_SUCCESS; 
				break; 
			case 2:
				//LPCWSTR pathname; //目标进程全路径 
				//ULONG pid; //动作发起者进程ID 
				//ULONG tid; //动作发起者线程ID 
				//ULONG target_pid; //目标进程ID 
				//ULONG exitcode; //进程推出码 
				//NTSTATUS result; //动作完成结果(NTSTATUS) 
				
				all_test_policy[ i ].policy.type = PROC_kill; 
				*all_test_policy[ i ].policy.do_proc_kill.path_name = L'\0'; 
				all_test_policy[ i ].policy.do_proc_kill.target_pid = 50; 
				all_test_policy[ i ].policy.do_proc_kill.exitcode = 100; 
				//all_test_policy[ i ].policy.do_proc_kill.result = STATUS_SUCCESS; 
				break; 
			case 3:
				break; 
			case 4:
				break; 
			case 5:
				break; 
			case 6:
				break; 
			case 7:
				break; 
			case 8:
				break; 
			case 9:
				break; 
			case 10:
				break; 
			case 11:
				break; 
			default:
				ASSERT( FALSE ); 
				memset( &all_test_policy[ i ], 0, sizeof( all_test_policy[ 0 ] ) ); 
				break; 
			}
		}

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS init_test_actions()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG i; 

	do 
	{
		for( i = 0; i < ARRAY_SIZE( all_test_action ); i ++ )
		{
			switch( i )
			{
			case 0:
				all_test_action[ i ].type = NET_accept; 
				all_test_action[ i ].do_net_accept.data_len = 0; 
				all_test_action[ i ].do_net_accept.protocol = SELF_TEST;  
				break; 
			case 1:
				//LPCWSTR pathname; //文件全路径
				//ULONG pid; //动作发起者进程ID 
				//ULONG tid; //动作发起者线程ID 
				//ULONG offset; //文件内偏移 
				//ULONG data_len; //数据长度 
				//NTSTATUS result; //动作完成结果(NTSTATUS) 
				all_test_action[ i ].type = FILE_modified; 
				all_test_action[ i ].do_file_write.path_name[ 0 ] = L'\0'; 
				all_test_action[ i ].do_file_write.offset = 100; 
				all_test_action[ i ].do_file_write.data_len = 1000; 
				//all_test_action[ i ].do_file_write.result = STATUS_SUCCESS; 
				break; 
			case 2:
				break; 
			case 3:
				break; 
			case 4:
				break; 
			case 5:
				break; 
			case 6:
				break; 
			case 7:
				break; 
			case 8:
				break; 
			case 9:
				break; 
			case 10:
				break; 
			case 11:
				break; 
			default:
				ASSERT( FALSE ); 
				memset( &all_test_action[ i ], 0, sizeof( all_test_action[ 0 ] ) ); 
				break; 
			}
		}

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS test_action_policy()
{
	LRESULT ret; 
	INT32 i; 
	policy_state cur_state = { 0 }; 
	event_action_response reply; 
	//policy_reply reply; 
	action_source_info proc_sub; 
	BOOLEAN action_replied = FALSE; 
	action_context cur_action_ctx = { 0 }; 
	data_trace_option trace_option; 
	//policy_holder *pol_holder = NULL; 
	//ret = ( 1 == 1 ? ( ret == 0, TRUE ) : FALSE ); 

	do 
	{
		ret = init_test_actions(); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = init_test_action_policies(); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = init_policy_nfa( &global_nfa, 
			compare_action_policy, 
			_compare_action_policy, 
			check_action_policy ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = insert_policy_to_nfa( &global_nfa, 
			( PVOID )&all_test_policy[ 0 ], 
			ACTION_ALLOW, 
			&cur_state ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		for( i = 1; i < ARRAYSIZE( all_test_policy ); i ++ )
		{
			if( i == ARRAYSIZE( all_test_policy ) - 1 )
			{
				ret = add_policy_to_path( &global_nfa, 
					&cur_state, 
					( PVOID )&all_test_policy[ i ], 
					ACTION_BLOCK, 
					&cur_state ); 
			}
			else
			{
				ret = add_policy_to_path( &global_nfa, 
					&cur_state, 
					( PVOID )&all_test_policy[ i ], 
					ACTION_ALLOW, 
					&cur_state ); 
			}

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		dump_nfa_policys( &global_nfa ); 

		cur_state.id = 0; 
		cur_state.pol_locator = NULL; 

		ret = get_cur_action_context( &proc_sub, &cur_action_ctx ); 
		if( ret != STATUS_SUCCESS )
		{
			break; 
		}

		for( i = 0; i < ARRAYSIZE( all_test_action ); i ++ )
		{
			ret = nfa_action_analyze( 
				&proc_sub, 
				&cur_action_ctx, 
				&all_test_action[ i ],  
				&trace_option, 
				&reply ); 

			if( ret != ERROR_SUCCESS )
			{
				if( ret == ERROR_NOT_FOUND )
				{
					break; 
				}

				if( cur_state.pol_locator == NULL )
				{
					break; 
				}
			}

			switch( reply.action )
			{
			case ACTION_ALLOW:
				break; 
			case ACTION_BLOCK:
				MessageBeep( 0 ); 
				MessageBox( NULL, _T( "action deny" ), NULL, 0 ); 
				action_replied = TRUE; 
				break; 
			default:
				ASSERT( FALSE ); 
				action_replied = TRUE; 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( action_replied == TRUE )
			{
				break; 
			}
		}

		uninit_policy_nfa( &global_nfa ); 


	}while( FALSE );

	printf( "return %u\n", ret ); 
	getch(); 

	return ( INT32 )ret; 
}

NTSTATUS test_number_policy()
{
	LRESULT ret; 
	INT32 i; 
	
	//policy_state cur_state = { 0 }; 
	policy_reply reply; 
	BOOLEAN action_replied = FALSE; 
	//policy_holder *pol_holder = NULL; 
	//ret = ( 1 == 1 ? ( ret == 0, TRUE ) : FALSE ); 
	
	do 
	{
		ret = init_policy_nfa( &global_nfa, 
			compare_number_policy, 
			_compare_number_policy, 
			check_number_policy ); 
		
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = insert_policy_to_nfa( &global_nfa, 
			( PVOID )number_policies[ 0 ], 
			ACTION_ALLOW, 
			&cur_state ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		for( i = 1; i < ARRAYSIZE( number_policies ); i ++ )
		{
			if( i == ARRAYSIZE( number_policies ) - 1 )
			{
				ret = add_policy_to_path( &global_nfa, 
					&cur_state, 
					( PVOID )number_policies[ i ], 
					ACTION_BLOCK, 
					&cur_state ); 
			}
			else
			{
				ret = add_policy_to_path( &global_nfa, 
					&cur_state, 
					( PVOID )number_policies[ i ], 
					ACTION_ALLOW, 
					&cur_state ); 
			}

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		dump_nfa_policys( &global_nfa ); 

		cur_state.id = 0; 
		cur_state.pol_locator = NULL; 

		for( i = 0; i < ARRAYSIZE( number_policies ); i ++ )
		{
			ret = nfa_check_policy( &global_nfa, 
				( PVOID )number_policies[ i ],  
				&cur_state, 
				&cur_state, 
				&reply ); 

			if( ret != ERROR_SUCCESS )
			{
				if( ret == ERROR_NOT_FOUND )
				{
					break; 
				}

				if( cur_state.pol_locator == NULL )
				{
					break; 
				}
			}

			switch( reply.reply )
			{
			case ACTION_ALLOW:
				break; 
			case ACTION_BLOCK:
				MessageBeep( 0 ); 
				MessageBox( NULL, _T( "action deny" ), NULL, 0 ); 
				action_replied = TRUE; 
				break; 
			default:
				ASSERT( FALSE ); 
				action_replied = TRUE; 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( action_replied == TRUE )
			{
				break; 
			}
		}

		uninit_policy_nfa( &global_nfa ); 


	}while( FALSE );

	printf( "return %u\n", ret ); 
	getch(); 

	return ( INT32 )ret; 
}
#endif //0

#include "action_type_parse.h"
#include "action_setup.h"

INT32 main()
{
	NTSTATUS ntstatus; 
	INT32 ret = 0; 
	exec_create creat = { 0 }; 
	param_info all_params[ 10 ]; 
	ULONG param_count; 

	creat.cmd_len = 10; 
	creat.image_base = ( PVOID )1000; 
	creat.path_len = 100; 
	creat.path_name[ 0 ] = L'\0'; 
	creat.pid = 1001; 

	do 
	{
		ntstatus = test_action_data_setup(); 

		//parse_exec_create( &creat, all_params, ARRAY_SIZE( all_params ), &param_count ); 

		//test_action_policy(); 
	}while( FALSE );

	return ret; 
}
#else
INT32 main( INT32 argc, CHAR *argv[] )
{
	INT32 ret = 0; 

	do 
	{
	} while ( FALSE );

	return ret; 
}

#endif //SELF_TEST