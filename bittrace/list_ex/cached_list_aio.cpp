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

#include "stdafx.h"

#include "UIListEx.h"
#ifndef _UI_DEBUG
#include "common_func.h"
#include "acl_define.h"
#include "list_ex_common.h"
#endif //_UI_DEBUG
#include "bitsafe_common.h"

#include "UIListSubElementEx.h"
#include "UIDlgBuilder.h"

#ifdef _UI_DEBUG
#include "ListCache.h"
#else
#include "list_log.h"
#endif //_UI_DEBUG

#ifndef _UI_DEBUG
#include "action_display.h"
#include "stack_dump.h"
#include "bittrace_common.h"
#include "volume_name_map.h"
#include "action_log_db.h"
#include "event_log_loader.h"
#endif //_UI_DEBUG

#include "ui_ctrl.h"
#include "cached_list_aio.h"
#include "sqlite3.h"

cache_work_context cache_worker = { 0 }; 

LRESULT WINAPI launch_load_action_log_work( LONG begin_index, LONG count, LONG ring_index, LONG max_item_count, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	work_item *work = NULL; 

	do 
	{
		work = ( work_item* )malloc( sizeof( work_item ) ); 
		if( work == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

//#ifdef _DEBUG
//		if( flags & LOAD_ITEM_DECREMENTAL )
//		{
//			
//		}
//#endif //_DEBUG

		cache_worker.context.break_work = TRUE; 

		work->begin_index = begin_index; 
		work->count = count; 
		work->ring_begin_index = ring_index; 
		work->max_count = max_item_count; 
		work->flags = flags; 
		//work->loaded_count = 0; 

		lock_cs( cache_worker.work_lock ); 
		InsertHeadList( &cache_worker.all_work_item, &work->entry ); 
		unlock_cs( cache_worker.work_lock ); 

		SetEvent( cache_worker.worker.notify ); 

		work = NULL; 
	}while( FALSE );

	if( work != NULL )
	{
		free( work ); 
	}
	return ret; 
}

NTSTATUS CALLBACK send_event_log_to_ui( ULONG log_id, 
									   sys_action_record *action, 
									   action_context *ctx, 
									   PVOID data, 
									   ULONG data_len, 
									   LPCWSTR stack_dump,
									   r3_action_notify_buf *action_buf, 
									   //ULONG frame_count, 
									   PVOID work_context, 
									   ULONG *state_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret; 
	ULONG ring_index; 
	cache_work_help *log_context; 
	//CControlUI **ui_items; 
	//CPaintManagerUI *manager; 
	//ULONG state; 
	ULONG index; 
	event_log_notify event_log; 

	do 
	{
		if( state_out != NULL )
		{
			*state_out = 0; 
		}

		if( work_context == NULL )
		{
			ASSERT( FALSE && "ui cache is null" ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		log_context = ( cache_work_help* )work_context; 

		if( log_context->notify_wnd == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		if( log_context->break_work == TRUE )
		{
			*state_out = ACTION_COUNT_IS_REACH_MAX; 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		if( action ==  NULL )
		{
			ASSERT( FALSE && "input null action" ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( log_id  >= 1 ); 

		index = log_context->work->loaded_count; 

		ring_index = GetRingItemIndex( log_context->work->ring_begin_index + index, log_context->work->max_count ); 

		if( ring_index == INVALID_ITEM_INDEX )
		{
			ASSERT( FALSE ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		if( log_context->break_work == TRUE )
		{
			*state_out = ACTION_COUNT_IS_REACH_MAX; 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		event_log.log_id = log_id; 
		event_log.action = action; 
		event_log.cached_item_index = ring_index; 
		event_log.stack_dump = stack_dump; 
		event_log.ctx = ctx; 
		event_log.data = data; 
		event_log.data_len = data_len; 

		ret = ( LRESULT )::SendMessage( log_context->notify_wnd, WM_EVENT_LOG_RECEIVE, ( WPARAM )work_context, ( LPARAM )&event_log ); 
		if( ret != ERROR_SUCCESS )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		if( log_context->break_work == TRUE )
		{
			*state_out = ACTION_COUNT_IS_REACH_MAX; 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		log_context->work->loaded_count += 1; 

	}while( FALSE );

	return ntstatus; 
}

ULONG CALLBACK thread_cache_work( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD wait_ret; 
	thread_manage *thread_param; 
	cache_work_help *_work_help; 
	work_item *work; 
	PLIST_ENTRY entry; 
	PVOID _param; 
	ULONG begin_time; 
	ULONG end_time; 

	ASSERT( param != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	thread_param = ( thread_manage* )param; 

	ASSERT( thread_param->context != NULL ); 

	_work_help = ( cache_work_help* )thread_param->context; 

	_param = thread_param->param; 

	do 
	{
		if( _work_help->notify_wnd == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		for( ; ; )
		{
			log_trace( ( MSG_INFO, "%s get block count loop\n", __FUNCTION__ ) ); 

			if( thread_param->stop_run == TRUE )
			{
				break; 
			}

			wait_ret = wait_event( thread_param->notify, INFINITE ); 
			if( wait_ret != WAIT_TIMEOUT 
				&& wait_ret != WAIT_OBJECT_0 
				&& wait_ret != WAIT_ABANDONED )
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait syncronous event failed will exit\n" ) ); 
				break; 
			}

			if( thread_param->stop_run == TRUE )
			{
				break; 
			}


			lock_cs( cache_worker.work_lock ); 

			entry = RemoveTailList( &cache_worker.all_work_item ); 

			unlock_cs( cache_worker.work_lock ); 

			if( entry == NULL )
			{
				log_trace( ( MSG_ERROR, "why is work item don't exist\n" ) ); 
				
				goto _continue; 
			}

			work = CONTAINING_RECORD( entry, work_item, entry ); 

			begin_time = GetTickCount(); 

			work->loaded_count = 0; 
			_work_help->work = work; 
			_work_help->break_work = FALSE; 

			ret = load_action_log_work( work->begin_index, work->count, _work_help ); 

			end_time = GetTickCount(); 

			__Trace( _T( "fill the ui content of %u controls elapse %u\n" ), work->count, end_time - begin_time ); 

			if( ret == ERROR_SUCCESS )
			{
				log_trace( ( MSG_INFO, "%s get load item cache error 0x%0.8x \n", __FUNCTION__, ret ) ); ; 
			}

			ret = ( LRESULT )::SendMessage( _work_help->notify_wnd, WM_EVENT_LOG_RECEIVE_DONE, ( WPARAM )_work_help, ( LPARAM )NULL ); 
			if( ret == FALSE )
			{
				goto _continue; 
			}

_continue:
			continue; 
		}
	}while( FALSE ); 

	return ( ULONG )ret; 
}

LRESULT CListBodyExUI::fill_cached_item_aio( LONG ItemBeginIndex, LONG ItemCount, LONG *ItemLoaded )
{
	LRESULT ret = ERROR_SUCCESS; 
//#ifndef _UI_DEBUG
//	ui_cache_context context; 
//#else
//	LONG ItemRingIndex; 
//#endif //_UI_DEBUG

	LONG RealIndexEnd; 
	LONG NeedLoadItemCount; 
	LONG _ItemLoaded = 0; 
	//LONG i; 

	do 
	{

		//if( m_pContainerCallback == NULL )
		//{
		//	ret = ERROR_NOT_READY; 
		//	break; 
		//}

		//ret = m_pContainerCallback->LoadSubItemsContent( ItemBeginIndex, ItemCount ); 

		if( ItemLoaded != NULL )
		{
			*ItemLoaded = 0; 
		}

		if( ItemBeginIndex < 0 )
		{
			DBG_BP(); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ItemCount > MaxItemCacheCount )
		{
			ItemCount = MaxItemCacheCount; 
			//ret = ERROR_BUFFER_OVERFLOW; 
			//break; 
		}

		RealIndexEnd = ItemBeginIndex + ItemCount; 

		if( RealIndexEnd < ItemBeginIndex )
		{
			DBG_BP(); 
			__Trace( _T( "item index to loading is overflow %d:%d\n" ), ItemBeginIndex, ItemCount ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

#ifdef _DEBUG
		if( ( RealIndexEnd > ( ItemCacheBeginIndex + ItemCacheCount ) )

			&& ( ItemBeginIndex < ItemCacheBeginIndex ) )
		{
			//ASSERT( FALSE && "item to load too more" ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
#endif //_DEBUG

		{
			LONG OverlapBegin; 
			LONG OverlapEnd; 
			

			/****************************************************************************************
			3      10      13      30

			region1:3-10 region2:13-30 bigger left:13 smaller right:10 SR-BL=10-13=-3
			region1:3-13 region2:10-30 bigger left:10 smaller right:13 SR-BL=13-10=3
			region1:3-30 region2:10-13 bigger left:10 smaller right:13 SR-BL=10-13=3
			****************************************************************************************/
			if( ItemBeginIndex > ItemCacheBeginIndex )
			{
				OverlapBegin = ItemBeginIndex; 
			}
			else
			{
				OverlapBegin = ItemCacheBeginIndex; 
			}

			if( RealIndexEnd > ItemCacheBeginIndex + ItemCacheCount )
			{
				OverlapEnd = ItemCacheBeginIndex + ItemCacheCount; 
			}
			else
			{
				OverlapEnd = RealIndexEnd; 
			}
			
			if( OverlapEnd > OverlapBegin )
			{
				_ItemLoaded += OverlapEnd - OverlapBegin; 
			}
#ifdef _DEBUG
			else //if( OverlapEnd <= OverlapBegin )
			{
				__Trace( _T( "cached item region:%d:%d is not crossed with the loading region:%d:%d" ), 
					ItemCacheBeginIndex, 
					ItemCacheBeginIndex + ItemCacheCount, 
					ItemBeginIndex, 
					ItemBeginIndex + ItemCount ); 

				//DBG_BP(); 
				//_ItemLoaded += OverlapBegin - OverlapEnd; 
			}
#endif //_DEBUG
		}

		do 
		{
			if( ItemBeginIndex < ItemCacheBeginIndex )
			{
				NeedLoadItemCount = ( ItemCacheBeginIndex - ItemBeginIndex ); 

				if( NeedLoadItemCount > MaxItemCacheCount )
				{
					NeedLoadItemCount = MaxItemCacheCount; 
					//ItemCacheCount = 0; 
					ItemCacheBeginIndex = ItemBeginIndex; 
					ItemRingBegin = 0; 
				}

#ifndef _UI_DEBUG
				ret = launch_load_action_log_work( ItemBeginIndex, 
					NeedLoadItemCount, 
					ItemRingBegin - ( LONG )NeedLoadItemCount, 
					MaxItemCacheCount, 
					LOAD_ITEM_DECREMENTAL ); 

				if( ret != ERROR_SUCCESS )
				{
					dbg_print( MSG_FATAL_ERROR, "launch the loading log work error %0.8x\n", ret ); 
				}
#else
				for( i = 0; i < NeedLoadItemCount; i ++ )
				{
					ItemRingIndex = GetRingItemIndex( ItemRingBegin - i - 1, MaxItemCacheCount ); 
					ASSERT( ItemRingIndex != INVALID_ITEM_INDEX ); 

					if( 0 > ( ItemBeginIndex + ( NeedLoadItemCount - i ) - 1 ) )
					{
						DBG_BP(): 
					}

					SetTempControlContent( ( ItemBeginIndex + ( NeedLoadItemCount - i ) - 1 ), ItemRingIndex ); 
				}
#endif //_UI_DEBUG
			}
		}while( FALSE );


		do 
		{
			if( RealIndexEnd > ( ItemCacheBeginIndex + ItemCacheCount ) )
			{
				NeedLoadItemCount = RealIndexEnd - ( ItemCacheBeginIndex + ItemCacheCount ); 

				if( NeedLoadItemCount == 0 )
				{
					//DBG_BP(); 
					break; 
				}

				if( NeedLoadItemCount > MaxItemCacheCount )
				{
					NeedLoadItemCount = MaxItemCacheCount; 
					//ItemCacheCount = 0; 
					ItemCacheBeginIndex = RealIndexEnd - MaxItemCacheCount; 
					ItemRingBegin = 0; 
				}
#ifndef _UI_DEBUG
				//context.manager = m_pManager; 
				//context.loaded_count = 0; 
				//context.ring_load_index = ItemRingBegin + ItemCacheCount; 
				//context.max_count = MaxItemCacheCount; 
				//context.ui_cache = ControlUICache; 
				//context.item_real_index = RealIndexEnd - NeedLoadItemCount; 

				ret = launch_load_action_log_work( RealIndexEnd - NeedLoadItemCount, 
					NeedLoadItemCount, 
					ItemRingBegin + ItemCacheCount, 
					MaxItemCacheCount, 
					0 ); 

				if( ret != ERROR_SUCCESS )
				{
					dbg_print( MSG_FATAL_ERROR, "launch the loading log work error %0.8x\n", ret ); 
				}

				//ret = load_action_log( , &context ); 

				//if( context.loaded_count != NeedLoadItemCount )
				//{
				//	log_trace( ( MSG_ERROR, "load the event log to ui is not completely successful" ) ); 
				//}
#else
				//static LONG PrevItemIndex = 0; 
				for( i = 0; i < NeedLoadItemCount; i ++ )
				{
					ItemRingIndex = GetRingItemIndex( ItemRingBegin + ItemCacheCount + i, MaxItemCacheCount ); 
					ASSERT( ItemRingIndex != INVALID_ITEM_INDEX ); 
					
					//PrevItemIndex = ItemBeginIndex + ItemCacheCount + i; 

					SetTempControlContent( ( RealIndexEnd - NeedLoadItemCount ) + i, ItemRingIndex ); 
				}
#endif //_UI_DEBUG
			}
		}while( FALSE ); 

	}while( FALSE ); 

	return ret; 
}

/**************************************************************
优化预加载算法，在正常运行时计算加载可能性，进行加载。
**************************************************************/
LRESULT CListBodyExUI::InitItemCacheWorker()
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE cache_work_notify = NULL; 
	BOOLEAN cs_lock_inited = FALSE; 

	do
	{
		//#define CACHE_WORK_EVENT_NAME "CACHE_WORK_EVENT"

		cache_worker.context.list = NULL; 
		cache_worker.context.notify_wnd = m_pManager->GetPaintWindow(); 
		if( cache_worker.context.notify_wnd == NULL )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		InitializeListHead( &cache_worker.all_work_item ); 

		init_cs_lock( cache_worker.work_lock ); 
		cs_lock_inited = TRUE; 

		cache_worker.context.break_work = FALSE; 

		cache_work_notify = CreateEventW( NULL, FALSE, FALSE, NULL ); 
		if( cache_work_notify == NULL )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		ret = _create_work_thread( &cache_worker.worker, thread_cache_work, cache_work_notify, &cache_worker.context, NULL ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create trace logging thread failed \n" ) ); 
		}

		cache_worker.worker.self_notify = FALSE; 

	}while( FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		if( cs_lock_inited == TRUE )
		{
			uninit_cs_lock( cache_worker.work_lock ); 
		}

		if( cache_work_notify != NULL )
		{
			CloseHandle( cache_work_notify ); 
		}
	}

	return ret; 
}

LRESULT CListBodyExUI::UninitItemCacheWorker()
{
	LRESULT ret = ERROR_SUCCESS; 

	do
	{
		{
			stop_work_thread( &cache_worker.worker ); 
		}

		if( NULL != cache_worker.worker.notify )
		{
			CloseHandle( cache_worker.worker.notify ); 
			cache_worker.worker.notify = NULL; 
		}

		init_cs_lock( cache_worker.work_lock ); 

	}while( FALSE ); 

	return ret; 
}

//LRESULT CListBodyExUI::GetItemLock( LONG ItemCacheIndex, CRITICAL_SECTION )
//{
//	
//}

//LRESULT CListBodyExUI::LockCachedItem( LONG ItemCacheIndex )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//
//	do 
//	{
//		ASSERT( ItemCacheIndex >= 0 ); 
//
//		ItemCacheIndex
//	}while( FALSE );
//
//	return ret; 
//}

LRESULT CListBodyExUI::AdjustItemCacheIndexes( LONG item_count, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( item_count > 0 ); 

		if( flags & LOAD_ITEM_DECREMENTAL )
		{

#ifdef _UI_DEBUG
			ItemCacheCount += NeedLoadItemCount; 
#else
			ItemCacheCount += item_count; 
#endif //_UI_DEBUG

			if( ItemCacheCount > MaxItemCacheCount )
			{
				ItemCacheCount = MaxItemCacheCount; 
			}

#ifdef _UI_DEBUG
			ItemCacheBeginIndex -= NeedLoadItemCount; 
#else
			ItemCacheBeginIndex -= item_count; 
#endif //_DEBUG

			//#ifdef _DEBUG
			if( ItemCacheBeginIndex < 0 )
			{
				DBG_BP(); 
				ItemCacheBeginIndex = 0; 
			}
			//#endif //_DEBUG

#ifdef _UI_DEBUG
			ItemRingBegin -= NeedLoadItemCount; 
#else
			ItemRingBegin += item_count;
#endif //_UI_DEBUG

			if( ItemRingBegin < 0 )
			{
				ItemRingBegin = ( MaxItemCacheCount + ItemRingBegin ); //ItemRingIndex
			}
		}
		else
		{
#ifdef _UI_DEBUG
			ItemCacheCount += NeedLoadItemCount;
#else
			ItemCacheCount += item_count; 
#endif //_UI_DEBUG


			if( ItemCacheCount > MaxItemCacheCount )
			{
				//DBG_BP(); 
				ItemRingBegin = ( ( ItemRingBegin + ItemCacheCount ) % MaxItemCacheCount ); 
				//ItemRingBegin += ( ItemCacheCount - MaxItemCacheCount ); 
				ItemCacheBeginIndex += ( ItemCacheCount - MaxItemCacheCount ); 
				ItemCacheCount = MaxItemCacheCount; 
			}

			//if( ( ItemRingBegin + ItemCacheCount ) > MaxItemCacheCount )
			//{
			//	DBG_BP(); 
			//	
			//}
		}
	}while( FALSE );

	return ret; 
}

LRESULT load_event_log_to_ui( CControlUI *ui_item, 
							 CPaintManagerUI *manager, 
							 LONG log_id, 
							 LONG filtered_index, 
							 sys_action_record *action, 
							 action_context *ctx, 
							 PVOID data, 
							 ULONG data_len, 
							 LPCWSTR stack_frame, 
							 r3_action_notify_buf *action_notify, 
							 filter_ui_info *ui_info, 
							 PVOID param ); 

LRESULT CListBodyExUI::check_and_update_ui( LONG ItemIndex, ULONG loaded_count, LONG ItemCount, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
#define MIN_LOAD_ITEM_COUNT 16
		if( 0 == ( CACHE_WORK_LOADED & flags ) 
			&& loaded_count < MIN_LOAD_ITEM_COUNT )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		if( ItemIndex >= ItemCacheBeginIndex 
			&& ItemIndex < ItemCacheBeginIndex + ItemCacheCount )
		{
			CControlUI *Item; 
			LRESULT _ret;  
			LONG DisplayItemIndex; 

			DisplayItemIndex = ItemIndex - ItemCacheBeginIndex; 

			ItemDisplayBegin = GetRingItemIndex( DisplayItemIndex + ItemRingBegin, MaxItemCacheCount ); //from 0

			_ret = GetDisplayItemAt( 0, &Item ); 
			if( _ret == ERROR_SUCCESS )
			{
				DisplayItemIndex =( ( CListContainerElementExUI *)Item )->GetIndex(); 

				if( DisplayItemIndex != ItemIndex )
				{
					DisplayItemIndex = DisplayItemIndex; 
				}
			}

#ifdef _DEBUG

			if( DisplayRegionFilled( ItemDisplayBegin ) != ERROR_SUCCESS )
			{
				DBG_BP(); 
			}

#endif //_DEBUG

		}
		else
		{
			/**************************************************************************
			对容器尾部滚动的处理效果不好,通过测试发现SQLITE在读ID更大的数据时，速度越慢。

			**************************************************************************/
			__Trace( _T( "can't load the display items (%u-%u).\n" ), ItemIndex, ItemIndex + ItemDisplayCount ); 
		}

		//CCachedItemManager::OnScrollPosChanged( PosX, PosY, Height ); 
		NeedUpdate(); 

		if( m_pManager != NULL )
		{
			m_pManager->NeedUpdate(); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT CListBodyExUI::on_cache_item_loaded_done( PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	//event_log_notify *event_log; 
	cache_work_help *work_context; 

	do 
	{
		ASSERT( context != NULL ); 

		work_context = ( cache_work_help* )context; 
		ret = check_and_update_ui( work_context->work->begin_index, work_context->work->loaded_count, work_context->work->count, CACHE_WORK_LOADED ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT CListBodyExUI::on_cache_item_loaded( PVOID item, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	event_log_notify *event_log; 
	cache_work_help *work_context; 

	do 
	{
		ASSERT( item != NULL ); 
		event_log = ( event_log_notify* )item; 

		ASSERT( event_log != NULL ); 
		ASSERT( event_log->cached_item_index < MaxItemCacheCount ); 
	
		work_context = ( cache_work_help* )context; 

		ASSERT( work_context != NULL ); 
		ASSERT( work_context->work != NULL ); 

		ret = load_event_log_to_ui( CachedControlUI[ event_log->cached_item_index ], 
			m_pManager, 
			event_log->log_id, 
			0, 
			event_log->action, 
			event_log->ctx, 
			event_log->data, 
			event_log->data_len, 
			event_log->stack_dump, 
			NULL, 
			NULL, 
			NULL ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = AdjustItemCacheIndexes( 1, work_context->work->flags ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "adjust the cache indexes error %u\n", ret ); 
			break; 
		}

		ret = check_and_update_ui( work_context->work->begin_index, work_context->work->loaded_count, work_context->work->count, 0 ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE );

	return ret; 
}

//LRESULT CListBodyExUI::LockDisplayCacheItem( cache_item *CachedItem )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	do 
//	{
//		ASSERT( CachedItem != NULL ); 
//
//		//if( CachedItem == NULL )
//		//{
//		//	ret = ERROR_INVALID_PARAMETER; 
//		//}
//
//		if( CachedItem->lock == NULL )
//		{
//			ASSERT( FALSE ); 
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		CachedItem->lock
//	}while( FALSE );
//
//	return ret; 
//}