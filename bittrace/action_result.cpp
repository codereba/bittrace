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

#ifdef _DEBUG_MEM_LEAKS
#undef _DEBUG_MEM_LEAKS
#endif //_DEBUG_MEM_LEAKS

#include "common_func.h"
#include "acl_define.h"
#include "action_result.h"
#include "action_log_db.h"
#include "event_memory_store.h"

#include "performance_test.h"
#include "filter_manage.h"
#include "module_info.h"

#include "stack_dump.h"

#include <strsafe.h>
//#include <Error.h>
#include "ui_ctrl.h"

typedef struct _action_cache
{
	thread_manage worker[ 1 ]; 
} action_cache, *paction_cache; 

action_cache event_result_receiver = { 0 }; 

ULONG CALLBACK thread_event_result_receive( LPVOID param ); 


LRESULT WINAPI init_event_result_receiver()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	BOOLEAN work_thread_started = FALSE; 
	BOOLEAN sym_context_inited = FALSE; 

	do 
	{
		ret = init_sym_context(); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		sym_context_inited = TRUE; 

		ret = create_work_thread( &event_result_receiver.worker[ 0 ], thread_event_result_receive, NULL, NULL ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create trace logging thread failed \n" ) ); 
			break; 
		}

		work_thread_started = TRUE; 
	}while( FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		if( FALSE == sym_context_inited )
		{
			uninit_sym_context(); 
		}

		if( work_thread_started == TRUE )
		{
			_ret = stop_work_thread( &event_result_receiver.worker[ 0 ] ); 
			if( _ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				log_trace( ( MSG_ERROR, "stop work thread failed\n" ) ); 
				//ret = _ret; 
			}
		}
	}

	return ret; 
}

LRESULT WINAPI uninit_event_result_receiver()
{
	LRESULT ret = ERROR_SUCCESS; 
	LIST_ENTRY *entry; 

	do 
	{
		ret = stop_work_thread( &event_result_receiver.worker[ 0 ] ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			log_trace( ( MSG_ERROR, "stop work thread failed\n" ) ); 
		}


		ret = uninit_sym_context(); 
		if( ret != ERROR_SUCCESS )
		{

		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI notify_action_result( action_result *result )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( result == NULL )
		{
			break; 
		}


	}while( FALSE );

	return ret; 
}

/*************************************************************************
֪ͨ�¼�����ķ�����
1.˫��Ϣƥ�����
1.�ں��н��¼����¼��Ľ���ֱ𱨸档
2.Ӧ�ò㽫�¼����¼��Ľ���ֱ���н��գ���ͨ�����н��м�¼�����棬���ƥ�䣬
��һ���¼��н��ƥ��֮�󣬽���������Ϣ�����������¼�����ݿ⡣
(3).��ʹ�û��棬ֱ�ӽ������ݱ��ĸ��£����߷ֱ����������ͬ�ı�֮�С�

2.ǰ�󱨸��ж����������¼���������Ϣ�������Ͳ���Ҫ���¼����ݽ��л����ƥ�䴦����

3.ֻ���к󱨸棬û��ǰ���棬ǰ����Ĳ���ƥ�����ں��еĲ��Լ����жϡ����ں��еĲ���
��ͨ���󱨸�������������ϴ����ں��еġ�

�¼�ID��ʾ����:
1.��ΪԴ��¼������������ÿ��������Ϊ��Ԫ���̣߳�����һ�ݼ�¼����¼���浱ǰ��Ϊ��
���ܴ�����������ΪID�š�
��ΪԴ��¼���Ǹ��ӿ�ѧ�ķ�������ʵ�ָ��Ӷȸߣ���Ҫ����һ�׶�ʵ�֡�

2.������ΪID��ʾ������ÿ����Ϊ��һ��ȫ�ֵ���ΪID�š�64λ���͡�

�¼����ݷ��ࣺ
3���෨��
1.�¼�����
2.�¼����
3.ģ���¼�

2���෨:
1.�¼�����
2.�¼���� ( ��ǰʹ��2���෨)

*************************************************************************/

ULONG filtered_action_count = 0; 
ULONG event_notify_count = 0; 

LRESULT WINAPI stop_event_trace()
{
	filtered_action_count = 0; 

	return ERROR_SUCCESS; 
}

ULONG LastUpdateUITime = 0; //GetTickCount(); 

ULONG event_backup_count = 0; 
BOOLEAN sql_transaction_begin = FALSE; 

LRESULT WINAPI notify_action_event_to_ui( ULONG filtered_event_count, ULONG event_count ); 

/*********************************************************************************
��������ķ���ֵ��࣬����ؽ��������ܣ����԰������жȽ�����������ͬʱ�����ǽ��в���
������
*********************************************************************************/
LPCWSTR WINAPI get_result_desc( NTSTATUS ntstatus )
{
	LPCWSTR desc = L""; 

	do 
	{
		switch( ntstatus )
		{
		case STATUS_SUCCESS:
			desc = L"STATUS_SUCCESS"; //L"�ɹ�"; 
			break; 
		case STATUS_UNSUCCESSFUL:
			desc = L"STATUS_UNSUCCESSFUL"; //L"��������"; 
			break; 
		case STATUS_INVALID_PARAMETER:
			desc = L"STATUS_INVALID_PARAMETER"; //L"��Ч����"; 
			break; 
		case STATUS_NOT_FOUND:
			desc = L"STATUS_NOT_FOUND"; //L"����û���ҵ�"; 
			break; 
		case STATUS_NO_MORE_ENTRIES:
			desc = L"STATUS_NO_MORE_ENTRIES"; //L"û�и���"; 
			break; 
		case STATUS_BUFFER_OVERFLOW:
			desc = L"STATUS_BUFFER_OVERFLOW"; //L"���治����ȫװ������"; 
			break; 
		case STATUS_BUFFER_TOO_SMALL:
			desc = L"STATUS_BUFFER_TOO_SMALL"; //L"���泤�ȹ�С"; 
			break; 
		case STATUS_OBJECT_NAME_NOT_FOUND:
			desc = L"STATUS_OBJECT_NAME_NOT_FOUND"; //L"����û���ҵ�"; 
			break; 
		case STATUS_DATA_NOT_ACCEPTED:
			desc = L"STATUS_DATA_NOT_ACCEPTED"; //L"����������"; 
			break; 
		case STATUS_UNKNOWN:
			desc = L""; 
			break; 
		default:
			break; 
		}
	}while( FALSE );

	return desc; 
}

/*****************************************************************************************************************
event logging speed is must more than the event logging speed in procmon.
can testing by remove the load event functions.
*****************************************************************************************************************/

static ULONGLONG event_id = 1; 
LRESULT WINAPI resset_event_id_count()
{
	event_id = 0; 

	return ERROR_SUCCESS; 
}

BOOLEAN process_log_sw = TRUE; 
LRESULT WINAPI process_proc_module_info( r3_action_notify *action ); 
LRESULT WINAPI process_event( r3_action_notify *action )
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 
	INT32 _ret; 

	do 
	{
		if( process_log_sw == FALSE )
		{
			break; 
		}

		action->id.QuadPart = event_id ++; 

		if( action->action.ctx.proc_name[ 0 ] == L'\0' 
			|| action->action.ctx.proc_name_len == 0 )
		{
		}

		ret = time_to_local_time( &action->action.ctx.time ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_FATAL_ERROR, "convert the time to local time error 0xI64x\n", action->action.ctx.time.QuadPart ) ); 
		}

		ret = check_r3_action_notify_output_valid( action ); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			break; 
		}

		if( check_time_valid( &action->action.ctx.time ) )
		{
			DBG_BP(); 
		}

		ret = process_proc_module_info( action ); 

#define EVENT_LOG_TRANSACTION_COUNT ( 800 )
#define MIN_EVENT_LOG_FILTERED_COUNT ( 500 ) 

#ifndef _EVENT_CACHE_BY_MEMORY
		if( 0 == ( event_backup_count % EVENT_LOG_TRANSACTION_COUNT ) )
		{
			if( event_backup_count > 0 )
			{
				if( sql_transaction_begin == TRUE ) 
				{
					_ret = commit_common_log_transaction(); 
					if( _ret != 0 )
					{
						log_trace( ( MSG_ERROR, "commit transaction for the log database error %u\n", _ret ) ); 
					}
					else
					{
						sql_transaction_begin = FALSE; 
					}
				}
				else
				{
					ASSERT( FALSE ); 
				}

				if( filtered_action_count >= MIN_EVENT_LOG_FILTERED_COUNT )
				{
					notify_action_event_to_ui( filtered_action_count ); 

					filtered_action_count = 0; 
				}
			}

			if( sql_transaction_begin == FALSE )
			{
				_ret = begin_common_log_transaction(); 
				if( _ret != 0 )
				{
					log_trace( ( MSG_ERROR, "begin transaction for the log database error %u\n", _ret ) ); 
				}
				else
				{
					sql_transaction_begin = TRUE; 
				}
			}
		}

		{
			PVOID data; 
			data = get_action_output_data( action ); 

			ret = filter_action_log( &action->action.action, 
				&action->action.ctx, 
				data, 
				action->data_size ); 

			if( ret == ERROR_SUCCESS )
			{
				filtered_action_count ++; 
				perf_test_begin( L"event loading to ui time when initial", ( PVOID )action->id.LowPart, NULL ); 
			}
		}

		ret = add_log_ex( action ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "add the event of system to database error %u\n", ret ) ); 
			break; 
		}

		event_backup_count ++; 
#else
		{
			ULONG state; 
			ULONG CurTickCount; 

			ret = add_log_to_store( action, &state ); 
			if( ret == ERROR_BASE_LOG_WORK )
			{
				ASSERT( state == 0 ); 
				break; 
			}

			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "add the event of system to database error %u\n", ret ) ); 

				ASSERT( state == 0 ); 
			}

			if( 0 == ( event_backup_count % EVENT_LOG_TRANSACTION_COUNT ) )
			{
				if( event_backup_count > 0 )
				{
					if( sql_transaction_begin == TRUE ) 
					{
						_ret = commit_common_log_transaction(); 
						if( _ret != 0 )
						{
							log_trace( ( MSG_ERROR, "commit transaction for the log database error %u\n", _ret ) ); 
						}
						else
						{
							sql_transaction_begin = FALSE; 
						}
					}
					else
					{
						ASSERT( FALSE ); 
					}
				}

				if( sql_transaction_begin == FALSE )
				{
					_ret = begin_common_log_transaction(); 
					if( _ret != 0 )
					{
						log_trace( ( MSG_ERROR, "begin transaction for the log database error %u\n", _ret ) ); 
					}
					else
					{
						sql_transaction_begin = TRUE; 
					}
				}

				ret = add_log_to_db_ex( action ); 
				if( ret == ERROR_SUCCESS )
				{
					event_backup_count ++; 
				}
			}

			//����֪ͨ��EVENT COUNT�ͻ����м�¼��COUNT�Ƿ�һ��
			if( state == EVENT_IS_FILTERED )
			{
				filtered_action_count ++; 
			}

			event_notify_count ++; 

			CurTickCount = GetTickCount(); 

			if( ( filtered_action_count >= MIN_EVENT_LOG_FILTERED_COUNT ) 
				|| ( ( CurTickCount - LastUpdateUITime ) > DEFAULT_TRACE_NOTIFY_TIME ) )
			{
				{
					notify_action_event_to_ui( event_notify_count, filtered_action_count ); 

					LastUpdateUITime = CurTickCount; 

					event_notify_count = 0; 
					filtered_action_count = 0; 
				}
			}
		}

#endif //_EVENT_CACHE_BY_MEMORY


	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI recv_event_aio( PVOID input_buf, 
							  ULONG input_size, 
							  PVOID output_buf, 
							  ULONG output_size, 
							  ULONG *ret_len ); 

ULONG CALLBACK thread_event_result_receive( LPVOID param )
{
	LRESULT ret = ERROR_SUCCESS;
	LRESULT _ret; 
	DWORD wait_ret;
	ULONG ret_len; 
	PUI_CTRL ui_ctrl; 
	thread_manage *thread_param;  
	r3_action_notify *action = NULL; 
	event_action_response resp = { 0 }; 
	PRIORITY_INFO priority_info = { 0 }; 

	ASSERT( NULL != param );

	RiseThreadPriority( &priority_info ); 
	thread_param = ( thread_manage* )param; 

	log_trace( ( MSG_INFO, "%s begin receive action result \n", __FUNCTION__  ) ); 

	do 
	{
		action = ( r3_action_notify* )malloc( R3_NOTIFY_BUF_SIZE ); 
		if( action == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		for( ; ; )
		{
			log_trace( ( MSG_INFO, "%s get sys event loop\n", __FUNCTION__ ) ); 

			if( thread_param->stop_run == TRUE )
			{
				break; 
			}

			do 
			{
				ret = recv_event_aio( &resp, 
					sizeof( resp ), 
					action, 
					R3_NOTIFY_BUF_SIZE, 
					&ret_len ); 

				if( thread_param->stop_run == TRUE )
				{
					break; 
				}

				if( ret != ERROR_SUCCESS )
				{}
				else
				{
					ret = process_event( action ); 
					if( ret != ERROR_SUCCESS )
					{}
				}
			} while ( FALSE ); 
		}

	}while( FALSE );

	if( action != NULL )
	{
		free( action ); 
	}

	LowerThreadPriority( &priority_info ); 

	log_trace( ( MSG_INFO, 
		"%s end receive action result 0x%0.8x \n", 
		__FUNCTION__, 
		ret ) ); 

	return ( ULONG )ret; 
}
