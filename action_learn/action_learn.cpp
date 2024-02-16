/*
 * Copyright 2010-2024 JiJie.Shi.
 *
 * This file is part of bittrace.
 * Licensed under the Gangoo License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
break; //�������� ����·���� ��ִ�м�أ� 

case EXEC_destroy:
break; //�����˳� ����·���� 

case EXEC_module_load:
break; //ģ����� ģ��·���� 

//MT_filemon, 
case FILE_touch:
break; //�����ļ� �ļ�ȫ·�� ���ļ���أ� 	
case FILE_open:
break; //���ļ� �ļ�ȫ·�� 
//case ACCESS_FILE:
//	break; 
case FILE_read:
break; //��ȡ�ļ� �ļ�ȫ·�� 
case FILE_write:
break; //д���ļ� �ļ�ȫ·�� 
//case MODIFY_FILE:
//	break; 
case FILE_modified:
break; //�ļ����޸� �ļ�ȫ·�� 
case FILE_readdir:
break; //����Ŀ¼ Ŀ¼ȫ·�� 
case FILE_remove:
break; //ɾ���ļ� �ļ�ȫ·�� 
//case DELETE_FILE:
//	break; 
case FILE_rename:
break; //�������ļ� �ļ�ȫ·�� 
case FILE_truncate:
break; //�ض��ļ� �ļ�ȫ·�� 
case FILE_mklink:
break; //�����ļ�Ӳ���� �ļ�ȫ·�� 
case FILE_chmod:
break; //�����ļ����� �ļ�ȫ·�� 
case FILE_setsec:
break; //�����ļ���ȫ���� �ļ�ȫ·�� 

//MT_regmon, 
case REG_openkey:
break; //��ע����� ע�����·��  ��ע�����أ� 
case REG_mkkey:
break; //����ע����� ע�����·�� 
//case MODIFY_KEY:
//	record_size += sizeof( reg_mkkey ); 
//	break; 
case REG_rmkey:
break; //ɾ��ע����� ע�����·�� 
case REG_mvkey:
break; //������ע����� ע�����·�� 
case REG_rmval:
break; //ɾ��ע����� ע�����·�� 
case REG_getval:
break; //��ȡע���ֵ ע���ֵ·�� 
case REG_setval:
break; //����ע���ֵ ע���ֵ·�� 
case REG_loadkey:
break; //����ע���Hive�ļ� ע�����·�� 
case REG_replkey:
break; //�滻ע����� ע�����·�� 
case REG_rstrkey:
break; //����ע���Hive�ļ� ע�����·�� 
case REG_setsec:
break; //����ע�������ȫ���� ע�����·�� 

//MT_procmon, 
case PROC_exec:
break; //�������� Ŀ�����·����  �����̼�أ�
//case CREATE_PROC:
//	record_size += sizeof( proc_exec ); 
//	break; 
case PROC_open:
break; //�򿪽��� Ŀ�����·���� 
case PROC_debug:
break; //���Խ��� Ŀ�����·���� 
case PROC_suspend:
break; //������� Ŀ�����·���� 
case PROC_resume:
break; //�ָ����� Ŀ�����·���� 
case PROC_kill:
break; //�������� Ŀ�����·���� 
//case TERMINATE_PROC:
//	record_size += sizeof( proc_kill ); 
//	break; 
case PROC_job:
break; //�����̼��빤���� Ŀ�����·���� 
case PROC_pgprot:
break; //������޸��ڴ����� Ŀ�����·���� 
case PROC_freevm:
break; //������ͷ��ڴ� Ŀ�����·���� 
case PROC_writevm:
break; //�����д�ڴ� Ŀ�����·���� 
case PROC_readvm:
break; //����̶��ڴ� Ŀ�����·���� 
case THRD_remote:
break; //����Զ���߳� Ŀ�����·���� 
case THRD_setctxt:
break; //����������߳������� Ŀ�����·���� 
case THRD_suspend:
break; //����̹����߳� Ŀ�����·���� 
case THRD_resume:
break; //����ָ̻��߳� Ŀ�����·���� 
case THRD_kill:
break; //����̽����߳� Ŀ�����·���� 
case THRD_queue_apc:
break; //������Ŷ�APC Ŀ�����·���� 

//MT_common
//case COM_access:
//	record_size += sizeof( ); 
//	break; 
//case ACCESS_COM:
//	record_size += sizeof( ); 
//	break; 

//MT_sysmon
case SYS_settime:
break; //����ϵͳʱ�� �� 
case SYS_link_knowndll:
break; //����KnownDlls���� �����ļ��� 
case SYS_open_physmm:
break; //�������ڴ��豸 �� 
//case ACCESS_MEM:
//	break; 
case SYS_read_physmm:
break; //�������ڴ� �� 
case SYS_write_physmm:
break; //д�����ڴ� �� 
case SYS_load_kmod:
break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
//case INSTALL_DRV:
//	record_size += sizeof( sys_load_kmod ); 
//	break; 
case SYS_enumproc:
break; //ö�ٽ��� �� 
case SYS_regsrv:
break; //ע����� �������ȫ·�� 
case SYS_opendev:
break; //���豸 �豸�� 

//MT_w32mon
case W32_postmsg:
break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
case W32_sendmsg:
break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
case W32_findwnd:
break; //���Ҵ��� �� 
case W32_msghook:
break; //������Ϣ���� �� 
//case INSTALL_HOOK:
//	record_size += sizeof( w32_msghook ); 
//	break; 
case W32_lib_inject:
break; //DLLע�� ע��DLL·���� 

//MT_netmon
case NET_create:
break; 
//case SOCKET_CREATE:
//	record_size += sizeof( net_create ); 
//	break; 
case NET_connect:
break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
//case SOCKET_CONNECT:
//	record_size += sizeof( net_connect ); 
//	break; 
case NET_listen:
break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
//case SOCKET_LISTEN:
//	record_size += sizeof( net_listen ); 
//	break; 
case NET_send:
break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
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
break; //HTTP���� HTTP����·������ʽ������/URL�� 
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
break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
case BA_extract_pe:
break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
case BA_self_copy:
break; //���Ҹ��� ����Ŀ���ļ�·���� 
case BA_self_delete:
break; //����ɾ�� ɾ���ļ�·���� 
case BA_ulterior_exec:
break; //����ִ�� ��ִ��ӳ��·���� 
case BA_invade_process:
break; //���ֽ��� Ŀ�����·���� 
case BA_infect_pe:
break; //��ȾPE�ļ� Ŀ���ļ�·���� 
case BA_overwrite_pe:
break; //��дPE�ļ� Ŀ���ļ�·���� 
case BA_register_autorun:
break; //ע���������� �������ļ�·���� 

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
ע�⣺
����Ϊ��Ϊ3�֣�
1.δ֪��ΪԴ��Ϊ��������Ϊ��Ҫ����SANDBOX
2.��ΪԴ��������Ϊ��������Ϊ����ֱ�ӷŹ���
3.��ΪԴ��������Ϊ��������Ϊ����ֱ����ֹ��

��ʹ�ü򵥵ķ������жϵĻ�����Ԫ����ΪԴ����������Ϊ��
1.δ֪��ΪԴ
2.��������ΪԴ
3.��������ΪԴ��

��˼���Ƕ�ͬһ��ָ�����У���ͬһ����Ϊ��ֻ����һ�Ρ�
��ΪSANDBOX���л��ƻ�ռ��һ��������Դ��������ͬʱ�Դ�����ָ������
���д�����
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
				//LPCWSTR pathname; //�ļ�ȫ·��
				//ULONG pid; //���������߽���ID 
				//ULONG tid; //�����������߳�ID 
				//ULONG offset; //�ļ���ƫ�� 
				//ULONG data_len; //���ݳ��� 
				//NTSTATUS result; //������ɽ��(NTSTATUS) 
				all_test_policy[ i ].policy.type = FILE_modified; 
				*all_test_policy[ i ].policy.do_file_write.path_name = L'\0'; 
				all_test_policy[ i ].policy.do_file_write.offset = 100; 
				all_test_policy[ i ].policy.do_file_write.data_len = 1000; 
				//all_test_policy[ i ].policy.do_file_write.result = STATUS_SUCCESS; 
				break; 
			case 2:
				//LPCWSTR pathname; //Ŀ�����ȫ·�� 
				//ULONG pid; //���������߽���ID 
				//ULONG tid; //�����������߳�ID 
				//ULONG target_pid; //Ŀ�����ID 
				//ULONG exitcode; //�����Ƴ��� 
				//NTSTATUS result; //������ɽ��(NTSTATUS) 
				
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
				//LPCWSTR pathname; //�ļ�ȫ·��
				//ULONG pid; //���������߽���ID 
				//ULONG tid; //�����������߳�ID 
				//ULONG offset; //�ļ���ƫ�� 
				//ULONG data_len; //���ݳ��� 
				//NTSTATUS result; //������ɽ��(NTSTATUS) 
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