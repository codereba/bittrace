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

机器的内存的容量决定了事件最多可以多少条数。

使用完全的内存缓存是最快的内容显示方式。

高速显示要求的原因：

1.事件产生的速度非常快，产生速度>数据库加载事件速度，如果是这样的话，则不能进行实时显示。
实时显示先决条件：
事件产生速度<加载显示速度

2.缓存加载算法极限：
缓存加载的最大时间*缓存加载速度

3.设计的结构：
1.缓存最大量(无限制速度效果最好)
2.加载缓存机制(无间断加载速度效果最好)
3.缓存IO速度(内存速度最好，硬盘速度次之(无压缩平面文件整度2，有压缩，格式定义文件速度3)

大型事件缓存设计：

1.使用LIST结构记录所有的事件的记录，但这种方法在LIST列表扩展时，会有一次复制之前所有记录的操作，性能比较低。
2.

WINDOWS事件发生速度分析:

19:15:05.1089577-19:15:21.4452447=6.3362870 7357条事件

与PROCMON比较，BITTRAACE的跟踪量必须大于或等于，才可以有实时跟踪的可能。
BITTRACE内存占用量分析:7357*6000=44142000=44.142MB 
PROCMON内存占用量分析:15.512K=15.512000 / 7357=2108 
缩小单个事件内存占用量将会提升事件IO，显示速度，内存拷贝总量将会成比例减少。

6000/2108相对而PROCMON的速度是BITTRACE的2.8倍左右

实时事件显示总IO量，计算方法:
1秒钟发生的事件总数
平均一个事件所需要的内存量
事件从源到显示，分析输出总共的拷贝次数

运算量计算方法:
包括IO总量*拷贝运行时间
单条事件输出的额外运算次数

对于PROCMON来说，如果需要进行实时的显示，缓存机制必须达到2.5MB秒
对于BITTRACE来说，如果需要进行实时的显示，缓存机制必须达到7MB秒

内存拷贝最大速度应该是>100MB
硬盘IO(WINDOWS NTFS文件系统)最大速度(sata)可以达到27MB

输出目标文件为数据库的话，最大速度将会受到非常多的因素的影响。很可能无法达到实时。

缓存架构设计:
更精确的缓存加构设计需要进行相关的细致的实验来完成，当前使用估计法来进行设计:
1.内存缓存为主，因为当前通过分析，SQLITE数据库很难达到实时跟踪的要求。
2.平面文件为2（可能不使用）
3.数据库文件为2或3
**************************************************************************************************************/

/***********************************************************************************************
通过实际测试:
PROCMON 73968 = 12.333M
BITTRACE 48525 = 180.20M
2X TIME 2014/11/24
***********************************************************************************************/

#define EVENT_IS_FILTERED 0x00000001

/***************************************************************************************************
通过实际测试，发现在BITTRACE用掉系统总内存量的2/3时（还有550M），系统运行非常慢，可能是由于PAGED\
页面文件非常大，使用频繁导致。
可能应该动计算出一个内存量上限。
分析PROCMON的日志内存量小的原因。
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
				如果添加失败，会出现无法加载相应的事件到界面的问题。
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
				如果添加失败，会出现无法加载相应的事件到界面的问题。
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
循环日志缓存法:
设定最大的日志缓存区大小，如果使用超界，则进行循环使用。
这种设计是否可行?
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