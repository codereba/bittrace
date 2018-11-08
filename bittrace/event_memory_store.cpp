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
#include "action_log_db.h"
#include "event_memory_store.h"
#include "r3_rw_lock.h"
#include "sqlite_supp.h"

ULONG event_filtering_flags = 0; 

ULONG set_event_filter_flags( ULONG flags )
{
	ULONG old_flags; 

	old_flags = event_filtering_flags; 
	event_filtering_flags = flags; 

	return old_flags; 
}

ULONG get_event_filter_flags()
{
	ULONG old_flags; 

	return event_filtering_flags; 
}

std::vector<r3_action_notify_buf*> all_actions; 
std::vector<r3_action_notify_buf*> all_filtered_actions; 

BOOLEAN filtering_event = FALSE; 

CRITICAL_SECTION events_store_lock; 
CRITICAL_SECTION filtered_events_store_lock; 
CRITICAL_SECTION filtering_tip_lock; 

LRESULT WINAPI init_event_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		init_cs_lock( events_store_lock ); 
		init_cs_lock( filtered_events_store_lock ); 
		init_cs_lock( filtering_tip_lock ); 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI uninit_event_store()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = clear_all_events(); 

		uninit_cs_lock( events_store_lock ); 
		uninit_cs_lock( filtered_events_store_lock ); 
		uninit_cs_lock( filtering_tip_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI clear_all_events()
{
	LRESULT ret = ERROR_SUCCESS; 

	w_lock_event_filter_store(); 
	all_filtered_actions.clear(); 
	w_unlock_event_filter_store(); 

	w_lock_event_store(); 
	ret = release_all_events_memory_cache(); 
	w_unlock_event_store(); 

	return ret; 
}

INLINE LRESULT WINAPI free_action_notify_buf( r3_action_notify_buf *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI filter_all_events()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	r3_action_notify_buf *action; 
	INT32 i; 
	BOOLEAN no_more_item = FALSE; 
	do 
	{
		if( all_actions.size() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
		for( i = 0; ; i ++ )
		{
			r_lock_event_store(); 
			if( ( ULONG )i >= all_actions.size() )
			{
				no_more_item = TRUE; 
			}
			r_unlock_event_store(); 

			if( TRUE == no_more_item )
			{
				break; 
			}

			action = all_actions[ i ]; 

			_ret = filter_action_log( &action->action.action.action, 
				&action->action.action.ctx, 
				get_action_output_data( &action->action ), 
				action->action.data_size ); 

			if( _ret == ERROR_SUCCESS )
			{
				ULONG event_count; 

				w_lock_event_filter_store(); 
				event_count = ( ULONG )all_filtered_actions.size(); 
				all_filtered_actions.push_back( action ); 
				w_unlock_event_filter_store(); 

				if( all_filtered_actions.size() <= event_count )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI release_all_events_memory_cache()
{
	std::vector<r3_action_notify_buf*>::iterator it; 

	for( it = all_actions.begin(); it != all_actions.end(); it ++ )
	{
		free_action_notify_buf( *it ); 

		free( *it ); 
	}

	all_actions.clear(); 

	return ERROR_SUCCESS; 
}


LRESULT WINAPI _clear_filtered_events()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		all_filtered_actions.clear(); 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI clear_filtered_events()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		w_lock_event_filter_store(); 

		all_filtered_actions.clear(); 

		w_unlock_event_filter_store(); 
	}while( FALSE );

	return ret; 
}

LONG WINAPI get_store_event_count()
{
	return ( LONG )all_actions.size(); 
}


LRESULT WINAPI get_filtered_action_log_count( ULONG *log_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	//LRESULT _ret; 
	ULONG count; 

	do 
	{
		ASSERT( log_count != NULL ); 

		*log_count = 0; 

		count = all_filtered_actions.size(); 

		log_trace( ( MSG_INFO, "the action log count is %u \n", count ) ); 
		*log_count = count; 

	}while( FALSE ); 

	return ret; 
}

/**************************************************************************************
使用文件处理日志的情况分析:
1.对日志做保存分析
2.在用户需要使用文件实时缓存日志时。(当前的实现方法性能达不到实时状态.)需要优化结构，
但文件系统上限是否可以满足要求?符号处理也是问题。
但这样可以小内存机器中低精确度的工作。(低精确性价值有限，很难使用)
**************************************************************************************/
LRESULT WINAPI save_log_to_db( LPCWSTR file_name, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 
	PVOID db_handle = NULL; 
	std::vector<r3_action_notify_buf*>::iterator it; 
	r3_action_notify_buf *log_buf; 

	ASSERT( file_name != NULL ); 

	do 
	{
		ntstatus = open_action_log_db( file_name, &db_handle ); 
		if( STATUS_SUCCESS != ntstatus )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( ONLY_SAVE_FILTERED_LOG_DB == flags )
		{
			r_lock_event_filter_store(); 

			ret = begin_transaction( ( sqlite3* )db_handle ); 
			if( ret != SQLITE_OK )
			{
				ntstatus = STATUS_UNSUCCESSFUL; 
				log_trace( ( MSG_ERROR, "begin transaction for the log database error %u\n", ret ) ); 
			}

			for( it = all_filtered_actions.begin(); it != all_filtered_actions.end(); it ++ )
			{
				log_buf = ( *it ); 

				ret = input_action_log_to_db_ex( db_handle, 
					&log_buf->action.action.ctx, 
					&log_buf->action.action.action, 
					log_buf->action.stack_frame, 
					log_buf->action.frame_count, 
					(PVOID )( ( BYTE* )&log_buf->action.action.action + log_buf->action.action.action.size ), 
					log_buf->action.data_size ); 

				if( ERROR_SUCCESS != ret )
				{

				}
			}

			ret = commit_transaction( ( sqlite3* )db_handle ); 
			if( ret != SQLITE_OK )
			{
				ntstatus = STATUS_UNSUCCESSFUL; 
				log_trace( ( MSG_ERROR, "commit transaction for the log database error %u\n", ret ) ); 
			}

			r_unlock_event_filter_store(); 
		}
		else
		{
			r_lock_event_store(); 

			ret = begin_transaction( ( sqlite3* )db_handle ); 
			if( ret != SQLITE_OK )
			{
				ntstatus = STATUS_UNSUCCESSFUL; 
				log_trace( ( MSG_ERROR, "begin transaction for the log database error %u\n", ret ) ); 
			}

			for( it = all_actions.begin(); it != all_actions.end(); it ++ )
			{
				log_buf = ( *it ); 

				ret = input_action_log_to_db_ex( db_handle, 
					&log_buf->action.action.ctx, 
					&log_buf->action.action.action, 
					log_buf->action.stack_frame, 
					log_buf->action.frame_count, 
					(PVOID )( ( BYTE* )&log_buf->action.action.action + log_buf->action.action.action.size ), 
					log_buf->action.data_size ); 

				if( ERROR_SUCCESS != ret )
				{

				}
			}

			ret = commit_transaction( ( sqlite3* )db_handle ); 
			if( ret != SQLITE_OK )
			{
				ntstatus = STATUS_UNSUCCESSFUL; 
				log_trace( ( MSG_ERROR, "commit transaction for the log database error %u\n", ret ) ); 
			}

			r_unlock_event_store(); 
		}

	}while( FALSE );

	if( db_handle != NULL )
	{
		close_action_log_db( db_handle ); 
	}

	return ret; 
}

LRESULT WINAPI resset_event_id_count(); 
LRESULT WINAPI process_event( r3_action_notify *action ); 

typedef struct _read_event_from_db
{
	r3_action_notify *log_buf; 
} read_event_from_db, *pread_event_from_db; 

PVOID CALLBACK get_static_log_buffer( PVOID param ) 
{
	ASSERT( param != NULL ); 
	return ( ( read_event_from_db* )param )->log_buf; 
}

NTSTATUS CALLBACK cache_event_log_to_cache( r3_action_notify *action, 
										LPCWSTR stack_dump,
										PVOID work_context, 
										ULONG *state_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret; 
	r3_action_notify *action_notify; 
	//read_event_from_db *context; 

	do 
	{
		if( state_out != NULL )
		{
			*state_out = 0; 
		}

		ASSERT( work_context != NULL ); 

		action_notify = action; 

		ret = process_event( action_notify ); 

	}while( FALSE );

	return ntstatus; 
}

LRESULT WINAPI load_log_from_db( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 
	INT32 i; 
	action_context ctx = { 0 }; 
	param_sel_mode all_conds[ MAX_PARAMS_COUNT ]; 
	param_info all_params[ MAX_PARAMS_COUNT ]; 
	ULONG param_count; 
	PVOID db_handle = NULL; 
	BYTE *action_notify_buf = NULL; 
	r3_action_notify *action; 
	read_event_from_db context; 

#ifdef _DEBUG_PERFORMANCE
	ULONG load_count = 0; 
	ULONG load_offset = 0; 
	ULONG BeginTime = 0; 
	ULONG EndTime = 0; 
#endif //_DEBUG_PERFORMANCE

	ASSERT( file_name != NULL ); 

	do 
	{
		action_notify_buf = ( BYTE* )malloc( R3_NOTIFY_BUF_SIZE ); 
		if( action_notify_buf == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		action = ( r3_action_notify* )action_notify_buf; 

		context.log_buf = action; 

		ntstatus = open_action_log_db( file_name, &db_handle ); 
		if( STATUS_SUCCESS != ntstatus )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

#ifdef _DEBUG_PERFORMANCE
		BeginTime = GetTickCount(); 
		BeginTime = GetTickCount(); 
#endif //_DEBUG_PERFORMANCE

			ctx.last_result = RESULT_IGNORE; 

			memset( &all_params, 0, sizeof( all_params ) ); 
			param_count = 0; 

			for( i = 0; ( ULONG )i < param_count; i ++ )
			{
				all_conds[ i ] = SELECT_OR; 
			}

			/***************************************************************************************
			过滤可以通过直接使用数据库查询语言来实现，但这样如果再加上索引(ID)定位的时间，会比较慢，
			其中字符串比较索引是数据库查询速度的难点。(EVERYTHING是如何实现高速查询索引的?)
			所以使用了select * + 代码过滤的方法。
			***************************************************************************************/

			resset_event_id_count(); 

			ret = output_action_log_from_db_ex( 
				db_handle, 
				NULL, 
				SYS_ACTION_NONE, 
				all_conds, 
				all_params, 
				param_count, 
				0, 
				0xffffffff, 
				( PVOID )&context, 
				get_static_log_buffer, 
				cache_event_log_to_cache ); 

			if( ret != 0 )
			{
				log_trace( ( MSG_FATAL_ERROR, "show action from db error %u\n", ret ) ); 
			}
	}while( FALSE ); 

#ifdef _DEBUG_PERFORMANCE
	EndTime = GetTickCount(); 

	//if( EndTime - BeginTime > 100 )
	//{
	//	__asm int 3; 
	//}

	dbg_print( MSG_IMPORTANT, "%s offset %u count %u time %u\n", __FUNCTION__, load_offset, load_count, EndTime - BeginTime ); 
#endif //_DEBUG_PERFORMANCE

	if( db_handle != NULL )
	{
		close_action_log_db( db_handle ); 
	}

	if( NULL != action_notify_buf )
	{
		free( action_notify_buf ); 
	}

	return ret; 
}
