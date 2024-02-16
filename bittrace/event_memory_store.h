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

#ifndef __EVENT_MEMORY_STORE_H__
#define __EVENT_MEMORY_STORE_H__

#include "action_type_parse.h"
#include "filter_manage.h"
#include "event_store.h"

LRESULT WINAPI init_event_store(); 

LRESULT WINAPI uninit_event_store(); 

LRESULT WINAPI _clear_filtered_events(); 
LRESULT WINAPI clear_filtered_events(); 

LONG WINAPI get_store_event_count(); 
//LONG WINAPI get_store_filter_events_count(); 

#ifdef new
#undef new
#endif //new
#include <vector>

extern std::vector<r3_action_notify_buf*> all_actions; 
extern std::vector<r3_action_notify_buf*> all_filtered_actions; 

extern CRITICAL_SECTION events_store_lock; 
extern CRITICAL_SECTION filtered_events_store_lock; 
extern CRITICAL_SECTION filtering_tip_lock; 

extern BOOLEAN filtering_event; 

INLINE LRESULT WINAPI r_lock_event_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		lock_cs( events_store_lock ); 
		//filter_info_lock.LockReader(); 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI r_unlock_event_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		unlock_cs( events_store_lock ); 
		//filter_info_lock.UnlockReader(); 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI w_lock_event_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		lock_cs( events_store_lock ); 
		//filter_info_lock.LockWriter(); 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI w_unlock_event_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		unlock_cs( events_store_lock ); 
		//filter_info_lock.UnlockWriter(); 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI w_try_lock_event_store()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		_ret = TryEnterCriticalSection( &events_store_lock ); 

		//try_unlock_cs( filter_info_lock ); 
		//_ret = filter_info_lock.TryLockWriter(); 
		if( FALSE == _ret )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "try enter cs error" ) ); 

			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI r_lock_event_filter_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		lock_cs( filtered_events_store_lock ); 
		//filter_info_lock.LockReader(); 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI r_unlock_event_filter_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		unlock_cs( filtered_events_store_lock ); 
		//filter_info_lock.UnlockReader(); 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI w_lock_event_filter_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		lock_cs( filtered_events_store_lock ); 
		//filter_info_lock.LockWriter(); 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI w_unlock_event_filter_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		unlock_cs( filtered_events_store_lock ); 
		//filter_info_lock.UnlockWriter(); 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI w_try_lock_event_filter_store()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		_ret = TryEnterCriticalSection( &filtered_events_store_lock ); 

		//try_unlock_cs( filter_info_lock ); 
		//_ret = filter_info_lock.TryLockWriter(); 
		if( FALSE == _ret )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "try enter cs error" ) ); 

			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI w_lock_filtering_tip_lock()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		lock_cs( filtering_tip_lock ); 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI w_unlock_filtering_tip_lock()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		unlock_cs( filtering_tip_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI release_all_events_memory_cache(); 
LRESULT WINAPI filter_all_events(); 

LRESULT WINAPI clear_all_events(); 

/**************************************************************************************************************

�������ڴ�������������¼������Զ���������

ʹ����ȫ���ڴ滺��������������ʾ��ʽ��

������ʾҪ���ԭ��

1.�¼��������ٶȷǳ��죬�����ٶ�>���ݿ�����¼��ٶȣ�����������Ļ������ܽ���ʵʱ��ʾ��
ʵʱ��ʾ�Ⱦ�������
�¼������ٶ�<������ʾ�ٶ�

2.��������㷨���ޣ�
������ص����ʱ��*��������ٶ�

3.��ƵĽṹ��
1.���������(�������ٶ�Ч�����)
2.���ػ������(�޼�ϼ����ٶ�Ч�����)
3.����IO�ٶ�(�ڴ��ٶ���ã�Ӳ���ٶȴ�֮(��ѹ��ƽ���ļ�����2����ѹ������ʽ�����ļ��ٶ�3)

�����¼�������ƣ�

1.ʹ��LIST�ṹ��¼���е��¼��ļ�¼�������ַ�����LIST�б���չʱ������һ�θ���֮ǰ���м�¼�Ĳ��������ܱȽϵ͡�
2.

WINDOWS�¼������ٶȷ���:

19:15:05.1089577-19:15:21.4452447=6.3362870 7357���¼�

��PROCMON�Ƚϣ�BITTRAACE�ĸ�����������ڻ���ڣ��ſ�����ʵʱ���ٵĿ��ܡ�
BITTRACE�ڴ�ռ��������:7357*6000=44142000=44.142MB 
PROCMON�ڴ�ռ��������:15.512K=15.512000 / 7357=2108 
��С�����¼��ڴ�ռ�������������¼�IO����ʾ�ٶȣ��ڴ濽����������ɱ������١�

6000/2108��Զ�PROCMON���ٶ���BITTRACE��2.8������

ʵʱ�¼���ʾ��IO�������㷽��:
1���ӷ������¼�����
ƽ��һ���¼�����Ҫ���ڴ���
�¼���Դ����ʾ����������ܹ��Ŀ�������

���������㷽��:
����IO����*��������ʱ��
�����¼�����Ķ����������

����PROCMON��˵�������Ҫ����ʵʱ����ʾ��������Ʊ���ﵽ2.5MB��
����BITTRACE��˵�������Ҫ����ʵʱ����ʾ��������Ʊ���ﵽ7MB��

�ڴ濽������ٶ�Ӧ����>100MB
Ӳ��IO(WINDOWS NTFS�ļ�ϵͳ)����ٶ�(sata)���Դﵽ27MB

���Ŀ���ļ�Ϊ���ݿ�Ļ�������ٶȽ����ܵ��ǳ�������ص�Ӱ�졣�ܿ����޷��ﵽʵʱ��

����ܹ����:
����ȷ�Ļ���ӹ������Ҫ������ص�ϸ�µ�ʵ������ɣ���ǰʹ�ù��Ʒ����������:
1.�ڴ滺��Ϊ������Ϊ��ǰͨ��������SQLITE���ݿ���Ѵﵽʵʱ���ٵ�Ҫ��
2.ƽ���ļ�Ϊ2�����ܲ�ʹ�ã�
3.���ݿ��ļ�Ϊ2��3
**************************************************************************************************************/

/***********************************************************************************************
ͨ��ʵ�ʲ���:
PROCMON 73968 = 12.333M
BITTRACE 48525 = 180.20M
2X TIME 2014/11/24
***********************************************************************************************/

#define EVENT_IS_FILTERED 0x00000001

/***************************************************************************************************
ͨ��ʵ�ʲ��ԣ�������BITTRACE�õ�ϵͳ���ڴ�����2/3ʱ������550M����ϵͳ���зǳ���������������PAGED\
ҳ���ļ��ǳ���ʹ��Ƶ�����¡�
����Ӧ�ö������һ���ڴ������ޡ�
����PROCMON����־�ڴ���С��ԭ��
***************************************************************************************************/

INLINE LRESULT load_from_memory_cache( ULONG item_index, r3_action_notify_buf **action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 

		r_lock_event_store(); 
		if( item_index >= all_actions.size() )
		{
			ret = ERROR_INVALID_PARAMETER; 
		}
		r_unlock_event_store(); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		*action = all_actions[ item_index ]; 

	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI add_to_event_filtered_store( r3_action_notify_buf *event_buf )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG event_count; 

	do 
	{
		w_lock_filtering_tip_lock(); 
		do 
		{
			if( filtering_event == TRUE )
			{
				ret = ERROR_NOT_READY; 
				break; 
			}
		} while ( FALSE ); 
		w_unlock_filtering_tip_lock(); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		w_lock_event_filter_store(); 

		do 
		{
			event_count = ( ULONG )all_filtered_actions.size(); 
			all_filtered_actions.push_back( event_buf ); 
		}while( FALSE );

		w_unlock_event_filter_store(); 

		if( all_filtered_actions.size() <= event_count )
		{
			//free( _action ); 
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI add_to_event_store( r3_action_notify_buf *event_buf )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG event_count; 

	do 
	{
		w_lock_filtering_tip_lock(); 
		do 
		{
			if( filtering_event == TRUE )
			{
				ret = ERROR_NOT_READY; 
				break; 
			}
		} while ( FALSE ); 
		w_unlock_filtering_tip_lock(); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		w_lock_event_store(); 

		do 
		{
			event_count = ( ULONG )all_filtered_actions.size(); 
			all_filtered_actions.push_back( event_buf ); 
		}while( FALSE );

		w_unlock_event_store(); 

		if( all_filtered_actions.size() <= event_count )
		{
			//free( _action ); 
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}
	}while( FALSE );

	return ret; 
}


INLINE LRESULT add_to_memory_cache( r3_action_notify *action, ULONG *state_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	r3_action_notify_buf *_action = NULL; 
	ULONG event_count; 
	PVOID data; 
	BOOLEAN event_filtered = FALSE; 

	do 
	{
		if( action->size == 0 
			|| action->size > R3_NOTIFY_BUF_SIZE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = compress_event_log( action, &_action ); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			break; 
		}

		ASSERT( _action != NULL ); 

		//_action = ( r3_action_notify* )malloc( action->size ); 
		//if( _action == NULL )
		//{
		//	ret = ERROR_NOT_ENOUGH_MEMORY; 
		//	break; 
		//}

		//memcpy( _action, action, action->size ); 

		//ret = prepare_action_to_load( _action ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	free( _action ); 
		//	break; 
		//}

		//_action->id.QuadPart = all_actions.size() + 1; 

		//{
		//	PVOID data; 
		//	ULONG data_size; 
		//	WCHAR text_buf[ 1024 ]; 

		//	data = get_action_output_data( &_action->action ); 
		//	data_size = _action->action.data_size; 

		//	dump_mem_in_buf_w( text_buf, ARRAYSIZE( text_buf ), data, data_size, 0 ); 

		//	dbg_print( MSG_IMPORTANT, "filtered:\n%ws\n", text_buf ); 

		//	//DebugBreak(); 
		//}

		data = get_action_output_data( &_action->action ); 

		_ret = filter_action_log( &_action->action.action.action, 
			&_action->action.action.ctx, 
			data, 
			_action->action.data_size ); 

		if( _ret == ERROR_SUCCESS )
		{
			event_filtered = TRUE; 
		}

		if( event_filtering_flags == DROP_FILTERED_EVENTS 
			&& event_filtered == FALSE )
		{
			free( _action ); 
			//ret = ERROR_SUCCESS; 
			break; 
		}

		w_lock_event_store(); 
		event_count = ( ULONG )all_actions.size(); 
		all_actions.push_back( _action ); 
		w_unlock_event_store(); 

		if( all_actions.size() <= event_count )
		{
			free( _action ); 
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		{
			if( event_filtered == TRUE )
			{
				/***********************************************************************************
				�������ʧ�ܣ�������޷�������Ӧ���¼�����������⡣
				***********************************************************************************/
				_ret = add_to_event_filtered_store( _action ); 

				if( _ret == ERROR_SUCCESS )
				{
					*state_out = EVENT_IS_FILTERED; 
				}
			}
		}

	}while( FALSE );

	return ret; 
}

INLINE LRESULT add_to_memory_cache_ex( sys_action_record *action, 
									  action_context *ctx, 
									  PVOID data, 
									  ULONG data_len, 
									  LPCWSTR stack_dump )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	r3_action_notify_buf *_action = NULL; 
	ULONG event_count; 
	static ULONGLONG log_id; 

	do 
	{
		if( action->size == 0 
			|| action->size > R3_NOTIFY_BUF_SIZE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( _action != NULL ); 

#define R3_ACTION_NOTIFY_BUF_FIXED_LENGTH ( FIELD_OFFSET( r3_action_notify_buf, action ) + FIELD_OFFSET( r3_action_notify, action ) + FIELD_OFFSET( sys_action_info, action ) ) 
		_action = ( r3_action_notify_buf* )malloc( R3_ACTION_NOTIFY_BUF_FIXED_LENGTH + action->size + data_len ); 

		if( _action == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		memcpy( &_action->action.action.ctx, ctx, sizeof( *ctx ) ); 
		memcpy( &_action->action.action.action, action, action->size ); 

		_action->action.id.QuadPart = log_id ++; 

		w_lock_event_store(); 
		event_count = ( ULONG )all_actions.size(); 
		all_actions.push_back( _action ); 
		w_unlock_event_store(); 

		if( all_actions.size() <= event_count )
		{
			free( _action ); 
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		{
			PVOID data; 
			data = get_action_output_data( &_action->action ); 

			_ret = filter_action_log( &_action->action.action.action, 
				&_action->action.action.ctx, 
				data, 
				_action->action.data_size ); 

			if( _ret == ERROR_SUCCESS )
			{
				/***********************************************************************************
				�������ʧ�ܣ�������޷�������Ӧ���¼�����������⡣
				***********************************************************************************/
				_ret = add_to_event_filtered_store( _action ); 

				if( _ret == ERROR_SUCCESS )
				{
				}
			}
		}

	}while( FALSE );

	return ret; 
}

/*********************************************************************************
ѭ����־���淨:
�趨������־��������С�����ʹ�ó��磬�����ѭ��ʹ�á�
��������Ƿ����?
*********************************************************************************/

INLINE LRESULT WINAPI add_log_to_store( r3_action_notify *action, ULONG *state_out )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 

		ASSERT( state_out != NULL ); 
		*state_out = 0; 

		//if( action->action.action.type == FILE_write )
		//{
		//	DBG_BP(); 
		//}

		ret = filter_trace_base_work( action ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( action->action.action.type == FILE_write )
		{
			//DBG_BP(); 
		}

		{
			ret = add_to_memory_cache( action, state_out ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{

	}

	return ret; 
}

INLINE ULONG WINAPI get_store_filtered_events_count()
{
	return ( ULONG )all_filtered_actions.size(); 
}

#define ONLY_SAVE_FILTERED_LOG_DB 0x00000001
LRESULT WINAPI save_log_to_db( LPCWSTR file_name, ULONG flags ); 
LRESULT WINAPI load_log_from_db( LPCWSTR file_name ); 

#endif //__EVENT_MEMORY_STORE_H__