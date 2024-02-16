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
#include "action_check.h"
#include "action_policy_common.h"
#include "action_source.h"
#include "action_policy.h"
#include "action_group_policy.h"

NTSTATUS compare_wild_card( LPCWSTR wild_card, ULONG ccb_wild_card_len, 
						   LPCWSTR name, ULONG ccb_name_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	do 
	{
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS compare_name( LPCWSTR wild_card, ULONG ccb_wild_card_len, 
						   LPCWSTR name, ULONG ccb_name_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	do 
	{
	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_manage_proc( manage_proc_info *proc_info ); 
//old action structures end

static NTSTATUS _check_proc_exec( action_source_info *info, 
								policy_holder *policy, 
								sys_action_info *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	//ULONG target_pid; //Ŀ�����ID 
	//PATH_SIZE_T path_len; 
	//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 
	group_action_policy *_policy; 

	do 
	{	
		ASSERT( info != NULL ); 
		ASSERT( policy != NULL ); 
		ASSERT( action != NULL ); 

		_policy = ( group_action_policy* )&policy->policy; 

		if( _policy->policy.do_proc_exec.target_pid != ( ULONG )INVALID_PROCESS_ID 
			&& _policy->policy.do_proc_exec.target_pid != action->action.do_proc_exec.target_pid )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( _policy->policy.do_proc_exec.path_len == 0 )
		{
			ASSERT( FALSE && "do process exec path length is invalid" ); 
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		if( _policy->policy.do_proc_exec.path_name[ 0 ] != L'*' )
		{
			if( _policy->policy.do_proc_exec.path_len <= 1 )
			{
				ASSERT( FALSE && "do process exec path length is invalid" ); 
				ntstatus = STATUS_INVALID_PARAMETER_3; 
				break; 
			}

			ntstatus = compare_wild_card( _policy->policy.do_proc_exec.path_name + 1, 
				_policy->policy.do_proc_exec.path_len - 1, 
				action->action.do_proc_exec.path_name, 
				action->action.do_proc_exec.path_len ); 
			
			break; 
		}

		ntstatus = compare_name( _policy->policy.do_proc_exec.path_name, 
			_policy->policy.do_proc_exec.path_len, 
			action->action.do_proc_exec.path_name, 
			action->action.do_proc_exec.path_len ); 

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_exec_create( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL ); 

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_exec_destroy( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_exec_module_load( sys_action_info *action, 
									   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_touch( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_open( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_read( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_write( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_modified( sys_action_info *action, 
									action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_readdir( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_remove( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_rename( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_truncate( sys_action_info *action, 
									action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_mklink( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_chmod( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_setsec( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_openkey( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_mkkey( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_rmkey( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_mvkey( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_rmval( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_getval( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_setval( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_loadkey( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_replkey( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_rstrkey( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_setsec( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_exec( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_open( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_debug( sys_action_info *action, 
								 action_reply *resp )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_suspend( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_resume( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_kill( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_job( sys_action_info *action, 
							   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_pgprot( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_freevm( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_writevm( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_readvm( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_remote( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_setctxt( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_suspend( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_resume( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_kill( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_queue_apc( sys_action_info *action, 
									 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_settime( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_link_knowndll( sys_action_info *action, 
										action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_open_physmm( sys_action_info *action, 
									  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_read_physmm( sys_action_info *action, 
									  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_write_physmm( sys_action_info *action, 
									   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS _check_sys_load_kmod( action_source_info *source, 
									 policy_holder *sub_policy, 
									 sys_action_info *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}


static NTSTATUS check_sys_load_kmod( sys_action_info *action, 
									action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_enumproc( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_regsrv( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_opendev( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_postmsg( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_sendmsg( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_findwnd( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_msghook( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_lib_inject( sys_action_info *action, 
									 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_connect( sys_action_info *action, 
								  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_listen( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_accept( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_send( sys_action_info *action, 
							   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_http( sys_action_info *action, 
							   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_extract_hidden( sys_action_info *action, 
										action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_extract_pe( sys_action_info *action, 
									action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_self_copy( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_self_delete( sys_action_info *action, 
									 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_ulterior_exec( sys_action_info *action, 
									   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_invade_process( sys_action_info *action, 
										action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_infect_pe( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_overwrite_pe( sys_action_info *action, 
									  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_register_autorun( sys_action_info *action, 
										  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS CALLBACK check_thread_create( r3_action_notify *action, 
							 action_reply *resp ) 
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	action_source_data source; 

	do 
	{	
		ASSERT( action != NULL ); 
		ASSERT( resp != NULL ); 

		source.proc = ( PVOID )action->action.action.do_thrd_create.target_pid; 
		source.thrd = ( PVOID )action->action.action.do_thrd_create.target_tid; 

		ntstatus = add_action_source( &source ); 

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS CALLBACK check_thread_kill( r3_action_notify *action, 
									action_reply *resp ) 
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	action_source_data source; 

	do 
	{	
		ASSERT( action != NULL ); 
		ASSERT( resp != NULL ); 

		source.proc = ( PVOID )action->action.action.do_thrd_create.target_pid; 
		source.thrd = ( PVOID )action->action.action.do_thrd_create.target_tid; 

		ntstatus = del_action_source( &source ); 

	}while( FALSE );

	return ntstatus; 
}

action_check_dispatch action_dispatch; 

NTSTATUS init_group_policy_dispatchs()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 i; 

	do 
	{
		for( i = 0; i < ARRAY_SIZE( action_dispatch.action_check_functions ); i ++ )
		{
			action_dispatch.action_check_functions[ i ] = ( CHECK_ACTION_FUCTION )NULL; 
		}

		action_dispatch.action_check_functions[ THRD_create ] = check_thread_create; 
		action_dispatch.action_check_functions[ THRD_exit ] = check_thread_kill; 

		ntstatus = register_action_check_dispatch( &action_dispatch ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS uninit_group_policy_dispatchs()
{
	return unregister_action_check_dispatch( &action_dispatch ); 
}

/*********************************************************************************
��Ϊ���������в�ѯ�㷨:
1.��1�汾:
	ͨ����ѯNFA�е����еĲ����飬��ѯ���е�������Ϊ�����Ƿ����ƥ�䣬���ƥ�������С�

*********************************************************************************/
INLINE NTSTATUS _check_group_hitted( policy_group *group, 
							action_source_ctx *source_ctx )
{
	LRESULT ret; 
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LIST_ENTRY *entry; 
	ref_obj *obj; 
	policy_holder *policy; 
	INT32 i; 

	do 
	{
		ASSERT( group != NULL ); 
		ASSERT( source_ctx != NULL ); 

		ret = lock_mutex( group->lock ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		entry = group->sub_polices.Flink; 

		for( ; ; )
		{

			if( entry == &group->sub_polices )
			{
				break; 
			}

			obj = CONTAINING_RECORD( entry, ref_obj, entry ); 
			policy = CONTAINING_RECORD( obj, policy_holder, obj ); 

			for( i = 0; ( ULONG )i < source_ctx->active_state.policy_count; i ++ )
			{
				if( policy == source_ctx->active_state.hitted_polices[ i ] )
				{
					break; 
				}
			}

			if( i == source_ctx->active_state.policy_count )
			{
				ntstatus = STATUS_NOT_FOUND; 
				break; 
			}

			entry = entry->Flink; 
		}

		unlock_mutex( group->lock ); 

	}while( FALSE ); 

	return ntstatus; 
}

/**************************************************************************
����Ϊ����ĳ�ֲ���ʱ�����в�����ļ�飬�鿴���еĲ������������ҳ���Ӧ��
��Ӧ����������1�汾ֻ�������еĵ�1�������顣
**************************************************************************/

NTSTATUS check_group_hitted( group_policy_nfa *nfa, 
							action_source_info *source, 
							group_hitting_callback callback_func )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LIST_ENTRY *entry; 
	policy_group *group; 
	policy_holder *policy; 
	ref_obj *obj; 
	ULONG need_len; 
	INT32 i; 

	do 
	{
		ASSERT( nfa != NULL ); 
		ASSERT( source != NULL ); 
		ASSERT( callback_func != NULL ); 

		ASSERT( nfa->policy_groups.list_num > 0 ); 

		//if( ret_len != NULL )
		//{
		//	*ret_len = 0; 
		//}

		//if( buf_len < sizeof( policy_group_desc ) )
		//{
		//	ASSERT( FALSE ); 

		//	if( ret_len != NULL )
		//	{
		//		*ret_len = sizeof( policy_group_desc ); 
		//	}

		//	ntstatus = STATUS_BUFFER_TOO_SMALL; 
		//	break; 
		//}

		for( i = 0; ( ULONG )i < nfa->policy_groups.list_num; i ++ )
		{
			nfa->policy_groups.r_lock_func( nfa->policy_groups.lock ); 
			entry = nfa->policy_groups.all_ref_node[ i ].Flink; 

			for( ; ; )
			{
				if( entry == &nfa->policy_groups.all_ref_node[ i ] )
				{
					//nfa->policy_groups.unlock_func( nfa->policy_groups.lock ); 
					break; 
				}

				obj = CONTAINING_RECORD( entry, ref_obj, entry ); 
				group = CONTAINING_RECORD( obj, policy_group, obj ); 

				ntstatus = _check_group_hitted( group, &source->ctx ); 
				if( ntstatus == STATUS_SUCCESS )
				{
					NTSTATUS _ntstatus; 

					_ntstatus = callback_func( nfa, source, group ); 
					if( _ntstatus != STATUS_SUCCESS )
					{
						log_trace( ( MSG_FATAL_ERROR, "process hitted group error 0x%0.8x\n", _ntstatus ) ); 
					}
					//_ntstatus = _reference_obj( &nfa->policy_groups, 
					//	&group->obj ); 

					//ASSERT( _ntstatus == STATUS_SUCCESS ); 

					//need_len = sizeof( policy_group_desc ) + ( group->desc.desc_len << 1 ); 
					//if( ret_len != NULL )
					//{
					//	*ret_len = need_len; 
					//}

					//ASSERT( buf_len >= sizeof( policy_group_desc ) ); 

					//if( buf_len < need_len )
					//{
					//	memcpy( desc, &group->desc, buf_len ); 

					//	( *( WCHAR* )( ( BYTE* )desc + buf_len - sizeof( WCHAR ) ) ) = L'\0'; 
					//}
					//else
					//{
					//	memcpy( desc, &group->desc, need_len ); 

					//	( *( WCHAR* )( ( BYTE* )desc + need_len - sizeof( WCHAR ) ) ) = L'\0'; 
					//}

					break; 
				}
			}

			nfa->policy_groups.unlock_func( nfa->policy_groups.lock ); 

			ASSERT( IsListEmpty( entry ) == TRUE ); 

		}

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS group_nfa_check_policy( group_policy_nfa *nfa, 
								action_source_info *source, 
								r3_action_notify *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	NTSTATUS _ntstatus; 
	policy_holder *policy = NULL; 
	policy_state *active_state; 
	ref_obj *obj; 
	ULONG hash_code; 
	LIST_ENTRY *entry; 
	register LIST_ENTRY *policy_list_head; 

	do 
	{
		ASSERT( nfa != NULL ); 
		ASSERT( nfa->check_policy_func != NULL ); 

		active_state = &source->ctx.active_state; 

		//ASSERT( 1 == nfa->policies.list_num ); 

		hash_code = ( ( ULONG )( BYTE )action->action.action.type % nfa->policies.list_num ); 

		policy_list_head = &nfa->policies.all_ref_node[ hash_code ]; 

		//notice: this policies is locked too more time if the policies for this action type is too more.
		//may be need more policies,need fix it (use policy reference).
		nfa->policies.r_lock_func( nfa->policies.lock ); 

		entry = policy_list_head->Flink; 

		for( ; ; )
		{
			if( entry == policy_list_head )
			{
				ntstatus = STATUS_NOT_FOUND; 
				break; 
			}

			ASSERT( entry != NULL ); 

			obj = CONTAINING_RECORD( entry, ref_obj, entry ); 
			policy = CONTAINING_RECORD( obj, policy_holder, obj ); 

			do 
			{
				ntstatus = is_policy_hitted( source, policy ); 
				if( ntstatus == STATUS_SUCCESS )
				{
					break; 
				}

				ntstatus = nfa->check_policy_func( source, 
					policy, 
					action ); 

				if( ntstatus == STATUS_SUCCESS )
				{ 
					_ntstatus = add_hitted_policy( source, policy ); 

					break; 
				}
			}while( FALSE );

			entry = entry->Flink; 
		}

		nfa->policies.unlock_func( nfa->policies.lock ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS CALLBACK on_policy_group_hitted( group_policy_nfa *nfa, action_source_info *source, policy_group *group )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( nfa != NULL ); 
		ASSERT( source != NULL ); 
		ASSERT( group != NULL ); 

		if( group->desc.reply == ACTION_BLOCK )
		{
			//ntstatus = TerminateProcess( )//terminate_proc( ( HANDLE )source->source.proc ); 
		}
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS nfa_action_group_analyze( group_policy_nfa *nfa, 
							action_source_info *source, 
							r3_action_notify *action )
							//policy_group_desc **desc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	//NTSTATUS _ntstatus; 
	policy_reply reply; 
	//policy_group_desc *_desc = NULL; 
	ULONG ret_len; 
	//action_source_ctx *ctx; 

	do
	{
#ifdef DBG
		ASSERT( nfa != NULL ); 
#else
		if( nfa == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}
#endif //DBG

		//ASSERT( desc != NULL ); 

		//*desc = NULL; 

		//if( source->ctx.prev_state != NULL )
		//{
		//	prev_state = &source->ctx.prev_state; 
		//}
		//else
		//{
		//	prev_state = find_proc_action_state( &source->ctx.source ); 
		//}

		//if( prev_state == NULL )
		//{
		//	ntstatus = STATUS_NOT_FOUND; 
		//	break; 
		//}

		//ASSERT( FALSE ); 
		DBG_BP(); 

		ntstatus = group_nfa_check_policy( nfa, 
			source, 
			action ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		//_desc = ALLOC_TAG_POOL( DEFAULT_POLICY_GROUP_LENGTH ); 
		//if( _desc == NULL )
		//{
		//	ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		//	break; 
		//}

		ntstatus = check_group_hitted( nfa, 
			source, 
			on_policy_group_hitted ); 
		
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		//*desc = _desc; 
		//_desc = NULL; 

		//_ntstatus = check_action( source, sub_policy, action ); 

	}while( FALSE ); 

	//if( NULL != _desc )
	//{
	//	FREE_TAG_POOL( _desc ); 
	//}

	return ntstatus; 
}

NTSTATUS check_group_action( action_source_info *source, 
					  sys_action_info *action, 
					  policy_holder *policy )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( source != NULL ); 

		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		switch( action->action.type )
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
		case FILE_getinfo:
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 
		case FILE_setinfo:
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 

			//MT_regmon, 
		case REG_openkey:
			break; //��ע����� ע�����·��  ��ע�����أ� 
		case REG_mkkey:
			break; //����ע����� ע�����·�� 
			//case MODIFY_KEY:
			//	record_size += sizeof( reg_mkkey ); 
			//	break; 
		case REG_rmkey://ɾ��ע����� ע�����·��
			break;
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
			ntstatus = _check_proc_exec( source, policy, action ); 
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
		case PROC_exit:
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
		case THRD_exit:
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
			ntstatus = _check_sys_load_kmod( source, policy, action ); 
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
		case NET_dns:
			break; 
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
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS get_group_policy_param( ref_obj* obj, PVOID *param )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	policy_holder *policy; 

	ASSERT( obj != NULL ); 
	ASSERT( param != NULL ); 

	policy = ( policy_holder* )CONTAINING_RECORD( obj, policy_holder, obj ); 

	*param = ( PVOID )&policy->policy; 

	//_return:
	return ntstatus; 
}

/***************************************************************************************
��Ϊ�����������
1.ϵͳ��Ϊ����Խṹ��
	1.һ����Ϊ���ԣ�����֮��Ĺ�ϵ��
		1.�໥֮��û��˳���ϵ��
		2.�໥֮������а�����ϵ��������С�Ĳ��Ա�����ʱ����Ĳ���Ҳ�����У����Խ�����Ӧ
		�����ݽṹ�����Ż�����1�汾����������
		3.��������ͬ��
2.ϵͳ��Ϊ�ķ�������
	1.����һ��ϵͳ��Ϊ
	2.�Բ������е�����û�����е���Ϊ�����ж�
	3.�������ҵ��Ĳ��ԣ����Ϊ������Ϊ�����Խ����У�����������2���б�
	4.�ж��������е���Ϊ��������Щ��Ϊ�飬ִ����Ϊ��ķ�Ӧ��Ϊ��	
***************************************************************************************/

BOOLEAN check_policy_include( group_action_policy *src_policy, group_action_policy *dst_policy )
{
	BOOLEAN ret = FALSE; 

	/******************************************************************************
	��Ϊ���԰�����ϵ��飬�������°�����ϵ��
	1.IP��ַ������IP��ַ��Χ�еİ�����ϵ��
	2.���Ʋ����ڼ���ͨ�����İ�����ϵ��
	3.���Ʋ������������ʽ�İ�����ϵ��
	******************************************************************************/
	
	do 
	{

	}while( FALSE );

	return ret; 
}

BOOLEAN check_policy_same( group_action_policy *src_policy, group_action_policy *dst_policy )
{
	BOOLEAN ret = FALSE; 

	/******************************************************************************
	��Ϊ���԰�����ϵ��飬�������°�����ϵ��
	1.IP��ַ������IP��ַ��Χ�еİ�����ϵ��
	2.���Ʋ����ڼ���ͨ�����İ�����ϵ��
	3.���Ʋ������������ʽ�İ�����ϵ��
	******************************************************************************/
	
	do 
	{
		if( src_policy->policy.type != dst_policy->policy.type )
		{
			break; 
		}

		switch( src_policy->policy.type )
		{
		case ACCESS_FILE:
			break; 

		case MODIFY_FILE:
			break; 

		case DELETE_FILE: 
			break; 

		case MODIFY_KEY: 
			break; 

		case CREATE_PROC: 
			break; 

		case TERMINATE_PROC: 
			break; 

		case ACCESS_COM: 
			break; 

		case ACCESS_MEM: 
			break; 

		case INSTALL_DRV: 
			break; 

		case INSTALL_HOOK: 
			break; 

		case SOCKET_CREATE: 
			break; 

		case SOCKET_CONNECT: 
			break; 

		case SOCKET_LISTEN: 
			break; 

		case SOCKET_SEND: 
			break; 

		case SOCKET_RECV: 
			break; 

		case SOCKET_ACCEPT: 
			break; 

		case LOCATE_URL: 
			break; 

		case ICMP_SEND: 
			break; 

		case ICMP_RECV: 
			break; 

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
			break; //��ȡ�ļ� �ļ�ȫ·�� 

		case FILE_write:
			break; //д���ļ� �ļ�ȫ·�� 

		case FILE_modified:
			break; //�ļ����޸� �ļ�ȫ·�� 
		case FILE_readdir:

			break; //����Ŀ¼ Ŀ¼ȫ·�� 
		case FILE_remove:
			break; //ɾ���ļ� �ļ�ȫ·�� 

		case FILE_rename:
			break; //�������ļ� �ļ�ȫ·�� 
		case FILE_truncate:
			//ntstatus = collect_file_truncate_param( )
			break; //�ض��ļ� �ļ�ȫ·�� 
		case FILE_mklink:
			break; //�����ļ�Ӳ���� �ļ�ȫ·�� 

		case FILE_chmod:
			break; //�����ļ����� �ļ�ȫ·�� 
		case FILE_setsec:

			break; //�����ļ���ȫ���� �ļ�ȫ·�� 

		case FILE_getinfo:

			break; //�����ļ���ȫ���� �ļ�ȫ·�� 
		case FILE_setinfo:

			break; //�����ļ���ȫ���� �ļ�ȫ·�� 
			//MT_regmon, 
		case REG_openkey:
			break; //��ע����� ע�����·��  ��ע�����أ� 

		case REG_mkkey:
			break; //����ע����� ע�����·�� 
			//case MODIFY_KEY
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

			//break; //����ע�������ȫ���� ע�����·�� 
			//MT_procmon, 
		case PROC_exec:
			//ntstatus = collect_proc_exec
			break; //�������� Ŀ�����·����  �����̼�أ�

		case PROC_open:
			//ntstatus = collect_proc_open
			break; //�򿪽��� Ŀ�����·���� 
		case PROC_debug:
			break; //���Խ��� Ŀ�����·���� 
		case PROC_suspend:
			break; //������� Ŀ�����·���� 
		case PROC_resume:
			break; //�ָ����� Ŀ�����·���� 
		case PROC_exit:
			break; //�������� Ŀ�����·���� 

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
		case THRD_exit:
			break; //����̽����߳� Ŀ�����·���� 
		case THRD_queue_apc:
			break; //������Ŷ�APC Ŀ�����·���� 

			//MT_common
			//case COM_access:
			//	break; 
			//MT_sysmon
		case SYS_settime:

			break; //����ϵͳʱ�� �� 
		case SYS_link_knowndll:
			break; //����KnownDlls���� �����ļ��� 
		case SYS_open_physmm:

			break; //�������ڴ��豸 �� 

		case SYS_read_physmm:

			break; //�������ڴ� �� 
		case SYS_write_physmm:
			//ntstatus = collect_sys_write
			break; //д�����ڴ� �� 
		case SYS_load_kmod:

			break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
			//case INSTALL_DRV:
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

		case W32_lib_inject:
			break; //DLLע�� ע��DLL·���� 

			//MT_netmon
		case NET_create:
			break; 

		case NET_connect:
			break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
			//case SOCKET_CONNECT:
			//	break; 
		case NET_listen:
			break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_LISTEN:
			//	break; 
		case NET_send:
			break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_SEND:
			//	break; 
		case NET_recv:
			break; 
			//case SOCKET_RECV:
			//	break; 
		case NET_accept:
			break; 
			//case SOCKET_ACCEPT:
			//	break; 
		case NET_dns:
			break; 
		case NET_http: 
			break; //HTTP���� HTTP����·������ʽ������/URL�� 

		case NET_icmp_send:
			break; 
		case NET_icmp_recv:
			break; 
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
			//desc = L"BA_other"; 
			//break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
	}while( FALSE );

	return ret; 
}

BOOLEAN compare_group_policy( ref_obj *obj, PVOID param )
{
	BOOLEAN ret = FALSE; 
	policy_holder *policy = NULL; 
	group_action_policy *_policy; 

	//ASSERT( param != NULL ); 
	ASSERT( obj != NULL ); 

	do 
	{
		policy = ( policy_holder* )CONTAINING_RECORD( obj, policy_holder, obj ); 
		_policy = ( group_action_policy* )param; 

		if( policy->policy.policy.type != _policy->policy.type )
		{
			break; 
		}

		//check policy is included by the policy that is added before.

		ret = check_policy_same( &policy->policy, _policy ); 
	}while( FALSE );

	return ret; 
}

NTSTATUS CALLBACK release_group_policy( ref_obj *obj )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	policy_holder *policy; 

	ASSERT( obj != NULL ); 

	policy = ( policy_holder* )CONTAINING_RECORD( obj, policy_holder, obj ); 

	ASSERT( policy->obj.ref_count == 0 ); 
	ASSERT( TRUE == IsListEmpty( &policy->obj.entry ) ); 

	FREE_TAG_POOL( policy ); 

	return ntstatus; 
}

ULONG calc_group_policy_hash( PVOID param, ULONG size )
{
	ULONG hash_code; 
	sys_action_policy *policy; 

	policy = ( sys_action_policy* )param; 

	hash_code = ( ( ULONG )policy->policy.type % size ); 

	return hash_code; 
}

INLINE NTSTATUS is_valid_action_policy( group_action_policy *policy )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( policy != NULL ); 

		if( policy->policy.size == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

	}while( FALSE );

	return ntstatus; 
}

INLINE NTSTATUS set_action_policy( group_action_policy *dst, group_action_policy *src )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( dst != NULL ); 
		ASSERT( src != NULL ); 
		ASSERT( is_valid_action_policy( src ) == STATUS_SUCCESS ); 
		memcpy( &dst->policy, &src->policy, src->policy.size ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS alloc_group_policy( PVOID param, ref_obj **obj_alloc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	policy_holder *policy; 
	group_action_policy *_policy; 

	ASSERT( param != NULL ); 
	ASSERT( obj_alloc != NULL ); 

	*obj_alloc = NULL; 

	_policy = ( group_action_policy* )param; 

	policy = ( policy_holder* )ALLOC_TAG_POOL( sizeof( policy_holder ) + _policy->policy.size - sizeof( action_policy ) ); 
	
	if( policy == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	ntstatus = set_action_policy( &policy->policy, _policy ); 
	if( ntstatus != STATUS_SUCCESS )
	{
		goto _return; 
	}

	*obj_alloc = &policy->obj; 

_return:
	if( ntstatus != STATUS_SUCCESS )
	{
		ASSERT( *obj_alloc == NULL ); 

		if( policy != NULL )
		{
			FREE_TAG_POOL( policy ); 
		}
	}
	return ntstatus; 
}

NTSTATUS get_policy_group_param( ref_obj* obj, PVOID *param )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	policy_group *group; 

	ASSERT( obj != NULL ); 
	ASSERT( param != NULL ); 

	group = ( policy_group* )CONTAINING_RECORD( obj, policy_group, obj ); 

	*param = ( PVOID )&group->desc; 

	//_return:
	return ntstatus; 
}

BOOLEAN compare_policy_group( ref_obj *obj, PVOID param )
{
	BOOLEAN ret = FALSE; 
	policy_group *group = NULL; 
	policy_group_desc *desc; 

	//ASSERT( param != NULL ); 
	ASSERT( obj != NULL ); 
	ASSERT( param != NULL ); 

	desc = ( policy_group_desc* )param; 
	group = CONTAINING_RECORD( obj, policy_group, obj ); 

	if( group->desc.id == desc->id )
	{
		ret = TRUE; 
	}
	else
	{
		ret = FALSE; 
	}
		
	return ret; 
}

NTSTATUS CALLBACK release_policy_group( ref_obj *obj )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	policy_group *group; 

	ASSERT( obj != NULL ); 

	group = ( policy_group* )CONTAINING_RECORD( obj, policy_group, obj ); 

	ASSERT( group->obj.ref_count == 0 ); 
	ASSERT( TRUE == IsListEmpty( &group->obj.entry ) ); 

	FREE_TAG_POOL( group ); 

	return ntstatus; 
}

ULONG calc_policy_group_hash( PVOID param, ULONG size )
{
	ULONG hash_code; 
	policy_group_desc *desc; 

	desc = ( policy_group_desc* )param; 

	hash_code = ( ULONG )( desc->id ) % size; 

	return hash_code; 
}

NTSTATUS alloc_policy_group( PVOID param, ref_obj **obj_alloc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	policy_group *group = NULL; 
	policy_group_desc *desc; 

	ASSERT( obj_alloc != NULL ); 
	ASSERT( param != NULL ); 

	*obj_alloc = NULL; 

	desc = ( policy_group_desc* )param; 

	do 
	{
		group = ALLOC_TAG_POOL( sizeof( policy_group ) + ( ( desc->desc_len + 1 ) << 1 ) ); 
		if( group == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		ntstatus = init_mutex( &group->lock ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		InitializeListHead( &group->sub_polices ); 

		group->desc.id = desc->id; 

		memcpy( group->desc.desc, desc->desc, ( desc->desc_len << 1 ) ); 
		group->desc.desc[ desc->desc_len ] = L'\0'; 
		group->desc.desc_len = desc->desc_len; 

		group->desc.reply = desc->reply; 

		*obj_alloc = &group->obj; 

	}while( FALSE );

	if( ntstatus != STATUS_SUCCESS )
	{
		if( group != NULL )
		{
			FREE_TAG_POOL( group ); 
		}
	}

	return ntstatus; 
}

NTSTATUS CALLBACK _compare_group_policy( struct _policy_holder *src_pol, 
struct _policy_holder *dst_pol )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	
	do 
	{
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS CALLBACK _find_group_policy( struct _policy_holder *pol, 
									 PVOID param )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS CALLBACK _check_group_policy( struct _action_source_info *source, 
	struct _policy_holder *pol, 
	PVOID param )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	action_check_dispatch *dispatch; 
	r3_action_notify *action; 
	CHECK_ACTION_FUCTION check_func; 
	action_reply _reply; 
	INT32 i; 

	do 
	{
		ASSERT( pol != NULL ); 
		ASSERT( param != NULL ); 

		action = ( r3_action_notify* )param; 
		if( pol->policy.policy.type != action->action.action.type )
		{
			ASSERT( FALSE ); 
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		for( i = 0; i < action_check_dispatchs.count; i ++ )
		{
			dispatch = &action_check_dispatchs.dispatchs[ i ]; 
			
			do 
			{
				check_func = dispatch->action_check_functions[ ( ULONG )action->action.action.type ]; 

				if( check_func == NULL )
				{
					break; 
				}
			
				ntstatus = check_func( action, &_reply ); 
			}while( FALSE );
		}
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS init_group_policy_nfa( group_policy_nfa *nfa )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	
	do 
	{
		ASSERT( nfa != NULL ); 
		//ASSERT( compare_policy_func != NULL ); 
		//ASSERT( check_policy_func != NULL ); 
		//ASSERT( find_policy_func != NULL ); 

		//if( nfa == NULL )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER_1; 
		//	break; 
		//}

		//if( compdare_policy_func == NULL )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER_2; 
		//	break; 
		//}

		//if( check_policy_func == NULL )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER_2; 
		//	break; 
		//}

		//if( find_policy_func == NULL )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER_2; 
		//	break; 
		//}

		nfa->check_policy_func = _check_group_policy; 
		//nfa->compare_policy_func = _compare_group_policy; 
		nfa->find_policy_func = _find_group_policy; 

#define POLICIY_HASH_TABLE_SIZE 128
		ntstatus = init_ref_obj_tbl( &nfa->policies, 
			POLICIY_HASH_TABLE_SIZE, 
			compare_group_policy, 
			alloc_group_policy, 
			release_group_policy, 
			calc_group_policy_hash, 
			get_group_policy_param, 
			NULL, 
			NULL, 
			NULL, 
			NULL ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

#define POLICY_GROUP_HASH_TABLE_SIZE 1
		ntstatus = init_ref_obj_tbl( &nfa->policy_groups, 
			POLICY_GROUP_HASH_TABLE_SIZE, 
			compare_policy_group, 
			alloc_policy_group, 
			release_policy_group, 
			calc_policy_group_hash, 
			get_policy_group_param, 
			NULL, 
			NULL, 
			NULL, 
			NULL ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}


		//nfa->group_count = group_count; 

	} while ( FALSE );

	return ntstatus;  
}

NTSTATUS uninit_group_policy_nfa( group_policy_nfa *nfa )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	policy_holder *pol_holder; 
	PLIST_ENTRY entry; 

	ref_obj *obj; 
	policy_holder *_pol_holder; 
	PLIST_ENTRY _entry; 

	PLIST_ENTRY pol_root; 
	PLIST_ENTRY head; 
	BOOLEAN all_dump = FALSE; 
	BOOLEAN lock_held = FALSE; 

#ifdef _DEBUG
	ULONG item_count = 0; 
#endif //_DEBUG

	do 
	{
		ASSERT( nfa != NULL ); 
#if 0
		entry = nfa->policies.all_ref_node[ 0 ].Flink; 
		if( entry == &nfa->policies )
		{
			break; 
		}

		pol_root = &nfa->policies; 

		for( ; ; )
		{
			obj = CONTAINER_INHERIT_ACE( entry, ref_obj, entry ); 

			pol_holder = CONTAINING_RECORD( obj, policy_holder, obj ); 

			//r_lock_policy_holder( pol_holder ); 
			//dump_policy_holder( pol_holder ); 
			//unlock_policy_holder( pol_holder ); 

#ifdef _DEBUG
			item_count += 1; 
#endif //_DEBUG

			entry = pol_holder->sub_policies.Flink; 

			if( entry == &pol_holder->sub_policies )
			{

				entry = &pol_holder->entry; 

				for( ; ; )
				{
					head = pol_holder->head; 

					if( entry->Flink == head )
					{
						if( head == pol_root )
						{
#ifdef _DEBUG
							dbg_print( MSG_IMPORTANT, "all linked graphic item is dumped count %u\n", item_count ); 
#endif //_DEBUG
							all_dump = TRUE; 
							break; 
						}

						_entry = entry; 
						_pol_holder = pol_holder; 

						pol_holder = CONTAINING_RECORD( head, policy_holder, sub_policies ); 
						entry = &pol_holder->entry; 

						RemoveEntryList( _entry ); 
						free( _pol_holder ); 

						//pol_holder = CONTAINING_RECORD( entry, policy_holder, entry ); 
					}
					else
					{
						_entry = entry; 
						_pol_holder = pol_holder; 

						entry = entry->Flink; 
						pol_holder = CONTAINING_RECORD( entry, policy_holder, entry ); 

						RemoveEntryList( _entry ); 
						free( _pol_holder ); 

						break; 
					}
				}

				if( TRUE == all_dump )
				{
					break; 
				}



			}
		}
#endif //0
		ntstatus = uninit_ref_obj_tbl( &nfa->policies ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			log_trace( ( MSG_FATAL_ERROR, "uninitialize the policies table error 0x%0.8x\n", ntstatus ) ); 
		}

		ntstatus = uninit_ref_obj_tbl( &nfa->policy_groups ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			log_trace( ( MSG_FATAL_ERROR, "uninitialize the policy groups table error 0x%0.8x\n", ntstatus ) ); 
		}

	}while( FALSE );

	//if( lock_held == TRUE )
	//{
	//	unlock_mutex( ( HANDLE )nfa->lock ); 
	//}

	return ntstatus; 
}

NTSTATUS del_policy_from_group( )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS del_policy_group( group_policy_nfa *nfa, 
						  policy_group_desc *desc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	NTSTATUS _ntstatus; 
	LIST_ENTRY *entry; 
	ref_obj *obj = NULL; 
	policy_group *group; 
	policy_holder *policy; 

	do 
	{
		ASSERT( nfa != NULL ); 

#ifdef DBG
		if( desc == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}
#else
		ASSERT( desc != NULL ); 
#endif //DBG

		ntstatus = reference_obj( &nfa->policy_groups, 
			( PVOID )desc, 
			&obj, 
			NULL ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		group = CONTAINING_RECORD( obj, policy_group, obj ); 

		for( ; ; )
		{
			entry = group->sub_polices.Flink; 
			if( entry == &group->sub_polices )
			{
				break; 
			}

			RemoveEntryList( entry ); 

			obj = CONTAINING_RECORD( entry, ref_obj, entry ); 
			policy = CONTAINING_RECORD( obj, policy_holder, obj ); 

			_ntstatus = deref_obj( &nfa->policies, 
				&policy->obj ); 

			if( _ntstatus != STATUS_SUCCESS )
			{
				ASSERT( FALSE ); 
				log_trace( ( MSG_FATAL_ERROR, "dereference object 0x%0.8x\n", policy ) ); 
			}
		}

		ntstatus = deref_obj( &nfa->policy_groups, &group->obj ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			ASSERT( FALSE ); 
			log_trace( ( MSG_FATAL_ERROR, "dereference group 0x%0.8x error \n", group ) ); 
		}
		
		ntstatus = _del_ref_obj( &nfa->policy_groups, 
			&group->obj ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			ASSERT( FALSE ); 
			log_trace( ( MSG_FATAL_ERROR, "delete group 0x%0.8x error \n", group ) ); 
		}

	}while( FALSE );

	return ntstatus; 
}

LRESULT dump_policy_holder( policy_holder *pol_holder )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( pol_holder != NULL ); 

	}while( FALSE );

	return ret; 
}

NTSTATUS dump_nfa_policys( group_policy_nfa *nfa )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	policy_holder *pol_holder; 
	PLIST_ENTRY entry; 
	PLIST_ENTRY pol_root; 
	PLIST_ENTRY head; 
	BOOLEAN all_dump = FALSE; 
	BOOLEAN lock_held = FALSE; 

#ifdef _DEBUG
	ULONG item_count = 0; 
#endif //_DEBUG

#if 0

	do 
	{
		ASSERT( nfa != NULL ); 

		entry = nfa->policies.Flink; 
		if( entry == &nfa->policies )
		{
			break; 
		}

		pol_root = &nfa->policies; 

		for( ; ; )
		{
			pol_holder = CONTAINING_RECORD( entry, policy_holder, entry ); 

			dump_policy_holder( pol_holder ); 

#ifdef _DEBUG
			item_count += 1; 
#endif //_DEBUG
			entry = pol_holder->sub_policies.Flink; 

			if( entry == &pol_holder->sub_policies )
			{
				entry = &pol_holder->entry; 

				for( ; ; )
				{
					head = pol_holder->head; 

					if( entry->Flink == head )
					{
						if( head == pol_root )
						{
#ifdef _DEBUG
							dbg_print( MSG_IMPORTANT, "all linked graphic item is dumped count %u\n", item_count ); 
#endif //_DEBUG
							all_dump = TRUE; 
							break; 
						}

						pol_holder = CONTAINING_RECORD( head, policy_holder, sub_policies ); 
						entry = &pol_holder->entry; 
						//pol_holder = CONTAINING_RECORD( entry, policy_holder, entry ); 
					}
					else
					{
						entry = entry->Flink; 
						pol_holder = CONTAINING_RECORD( entry, policy_holder, entry ); 

						break; 
					}
				}

				if( TRUE == all_dump )
				{
					break; 
				}
			}
		}

	}while( FALSE );

	if( lock_held == TRUE )
	{
		unlock_mutex( ( HANDLE )nfa->lock ); 
	}
#endif //0

	return ntstatus; 
}

