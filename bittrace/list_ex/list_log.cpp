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
#include "filter_menu.h"

#define VERBOSE_INFORMATION_MODE_TIP L"Press SHIFT or ATL to browser information quickly" //L"按住SHIFT或ATL键快速显示详细信息" 
#define _DEBUG_PERFORMANCE 1 

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

#include "action_result.h"
#include "filter_manage.h"
#include "event_memory_store.h"
#include "event_list_item.h"
#include "property_dlg.h"
#include "process_property_dlg.h"
#include "event_property_dlg.h"

#include "property_dlg.h"
#include "tooltip.h"

#define SQLITE_ABORT        4   /* Callback routine requested an abort */
#define FILL_CACHED_ITEM_SYNC 1

//#define _DEBUG_SQLITE_PERFORMANCE 1

//#ifdef __cplusplus
//extern "C" {
//#endif //__cplusplus
//
//	NTSTATUS CALLBACK cache_event_log_to_ui(sys_action_record *action, 
//		action_context *ctx, 
//		PVOID data, 
//		ULONG data_len, 
//		//LPCWSTR stack_frame[],
//		//ULONG frame_count, 
//		PVOID work_context, 
//		ULONG *state_out ); 
//
//#ifdef __cplusplus
//}
//#endif //__cplusplus

#ifdef _UI_DEBUG
#define ITEM_TEXT_BUF_LEN 260
#endif //_UI_DEBUG

//namespace DuiLib
//{
//	double CalculateDelay( double state )
//	{
//		return pow( state, 2 );
//	}
//}

CTooltipWindow event_tooltip; 
event_properies_dlg event_dlg; 
process_properies_dlg process_dlg; 
EVENT_BROWSER_MODE browser_mode = BROWSER_SIMPLE_MODE; 

LRESULT WINAPI stop_event_trace(); 

/********************************************************************************************
加载事件信息的效率可以从3个方面入手处理：
1.sqlite的加载机制的效率
2.将信息加载到界面的效率
3.加载时机，方式的速度

1 bittrace_ud.exe:sqlite3OsWrite + 00000027
2 bittrace_ud.exe:write32bits + 00000030
3 bittrace_ud.exe:pager_write + 00000141
4 bittrace_ud.exe:sqlite3PagerWrite + 0000022c
5 bittrace_ud.exe:balance + 000000f6
6 bittrace_ud.exe:sqlite3BtreeInsert + 000002ae
7 bittrace_ud.exe:sqlite3VdbeExec + 000049d5
********************************************************************************************/

CDialogBuilder DlgBuilder; 

const TCHAR* const ProcIconLabelName = _T("proc_icon");
const TCHAR* const EventIconLabelName = _T("event_icon"); 
const TCHAR* const EventNameLabelName = _T("event_name"); 
const TCHAR* const TimeLableName = _T("time");
const TCHAR* const IdLableName = _T("action_id");
const TCHAR* const ProcIdLabelName = _T("proc_id");
const TCHAR* const ThreadIdLabelName = _T("thread_id");
const TCHAR* const ExeFileNameLabelName = _T("exe_file");
const TCHAR* const EventDescLableName = _T("event_desc");
const TCHAR* const EventObjLableName = _T("event_obj");
const TCHAR* const ResultLableName = _T("result");
const TCHAR* const DataLabelName = _T("data");
const TCHAR* const CallStackLabelName = _T("call_stack");

const TCHAR* const EventInfoLayoutContainerName = EVENT_INFO_LAYOUT_CONTAINER_NAME;
const TCHAR* const AllStackLayoutContainerName = _T( "all_stack_layout" );

class event_list_item_builder: public IDialogBuilderCallback
{
public:
	event_list_item_builder( )
	{ 
	}

public:
	CControlUI* CreateControl(LPCTSTR pstrClass, CPaintManagerUI *pManager ) 
	{
		if( _tcscmp(pstrClass, _T("EventListElement")) == 0 ) 
		{
			return new CEventListItemUI(); 
		}
		else
		{
			ASSERT( FALSE ); 
		}

		return NULL;
	}
};

LRESULT WINAPI get_filter_info_from_event_ui( CControlUI *ctrl, 
											 LPCWSTR filter_mode_name, 
											 action_filter_info *filter_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR ctrl_name; 
	//LPCWSTR _ctrl_name; 
	LPCWSTR ctrl_text; 
	LPCWSTR filter_val; 
	HRESULT hr; 

	do 
	{
		ASSERT( ctrl != NULL ); 
		ASSERT( filter_info != NULL ); 

		//_ctrl_name = ctrl->GetUserData(); 
		ctrl_text = ctrl->GetUserData(); 
		if( ctrl_text[ 0 ] == L'\0' )
		{
			ctrl_text = ctrl->GetText(); 
		}

		ctrl_name = ctrl->GetName(); 
			
		if( wcscmp( ctrl_name, L"event_icon" ) == 0 )
		{
			filter_info->cond = FLT_MAIN_TYPE; 
		}
		else if( wcscmp( ctrl_name, L"event_name" ) == 0 )
		{
			filter_info->cond = FLT_ACTION_TYPE; 
		}
		else if( wcscmp( ctrl_name, L"time" ) == 0 )
		{
			filter_info->cond = FLT_DATE_TIME; 
		}
		else if( wcscmp( ctrl_name, L"action_id" ) == 0 )
		{
			filter_info->cond = FLT_ACTION_NO; 
		}
		else if( wcscmp( ctrl_name, L"proc_icon" ) == 0 )
		{
			filter_info->cond = FLT_PROCESS_NAME; 
		}
		else if( wcscmp( ctrl_name, L"proc_id" ) == 0 )
		{
			filter_info->cond = FLT_PID; 
		}
		else if( wcscmp( ctrl_name, L"thread_id" ) == 0 )
		{
			filter_info->cond = FLT_TID; 
		}
		else if( wcscmp( ctrl_name, L"exe_file" ) == 0 )
		{
			filter_info->cond = FLT_PROCESS_NAME; 
			filter_val = wcsrchr( ctrl_text, PATH_DELIM ); 
			if( filter_val != NULL )
			{
				filter_val += 1; 
				ctrl_text = filter_val; 
			}
		}
		else if( wcscmp( ctrl_name, L"event_obj" ) == 0 )
		{
			filter_info->cond = FLT_PATH; 
		}
		else if( wcscmp( ctrl_name, L"event_desc" ) == 0 )
		{
			filter_info->cond = FLT_DESC_DETAIL; 
		}
		else if( wcscmp( ctrl_name, L"data" ) == 0 )
		{
			filter_info->cond = FLT_ACTION_DATA; 
		}
		else if( wcscmp( ctrl_name, L"result" ) == 0 )
		{
			filter_info->cond = FLT_ACTION_RESULT; 
		}
		else if( wcscmp( ctrl_name, L"call_stack" ) == 0 )
		{
			filter_info->cond = FLT_CALL_STACK; 
		}
		else
		{
			ASSERT( FALSE ); 

			filter_info->cond = FLT_TYPE_INVALID; 
			break; 
		}

		filter_info->compare_mode = FLT_EQUALS;
		if( wcscmp( filter_mode_name, L"menu_include_it" ) == 0 )
		{
			filter_info->filter_mode = FLT_INCLUDE; 
		}
		else if( wcscmp( filter_mode_name, L"menu_exclude_it" ) == 0 )
		{
			filter_info->filter_mode = FLT_EXCLUDE; 
		}
		//else if( wcscmp( _ctrl_name, L"menu_contain_it" ) == 0 )
		//{
		//	filter_info->mode = FLT_CONTAINS; 
		//}
		else if( wcscmp( filter_mode_name, L"menu_begin_time" ) == 0 )
		{
			CEventListItemUI *list_item; 
			filter_info->filter_mode = FLT_EXCLUDE; 
			filter_info->cond = FLT_DATE_TIME; 
			
			list_item = ( CEventListItemUI* )( PVOID )ctrl->GetTag(); 
			ctrl_text = list_item->items.time->GetText(); 
		
			filter_info->compare_mode = FLT_START_WITH; 
		}
		else if( wcscmp( filter_mode_name, L"menu_end_time" ) == 0 )
		{
			CEventListItemUI *list_item; 
			filter_info->filter_mode = FLT_INCLUDE; 
			filter_info->cond = FLT_DATE_TIME; 

			list_item = ( CEventListItemUI* )( PVOID )ctrl->GetTag(); 
			ctrl_text = list_item->items.time->GetText(); 

			filter_info->compare_mode = FLT_END_WITH; 
		}
		else if( wcscmp( filter_mode_name, L"menu_hilight_it" ) == 0 )
		{
			filter_info->filter_mode = FLT_DISPLAY; 
		}
		else
		{
			filter_info->filter_mode = FLT_INVALID_FILTER_MODE; 
			ASSERT( FALSE ); 
			break; 
		} 

		hr = StringCbCopyW( filter_info->value.text.text_mode_value, 
			sizeof( filter_info->value.text.text_mode_value ), 
			ctrl_text ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		filter_info->value.text.cc_string_len = ( ULONG )wcslen( filter_info->value.text.text_mode_value ); 
	
		filter_info->value.type = STRING_VALUE_TYPE; 
		filter_info->value.text.text_is_ptr = FALSE; 

		ret = adjust_filter_cond_value( filter_info ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		filter_info->ui_info.bk_clr = EVENT_HILIGHT_BK_COLOR; 

		//init_filter_ui_info( &filter_info->ui_info ); 

	}while( FALSE ); 

	return ret; 
}

bool CEventListItemUI::Select(bool bSelect)
{
	if( !IsEnabled() ) 
	{
		return false;
	}

	//if( bSelect == m_bSelected ) 
	//{
	//	return true;
	//}

	//m_bSelected = !m_bSelected; 

	if( m_pOwner != NULL ) 
	{
		m_pOwner->SelectListItem( m_iIndex );
	}

	Invalidate();

	return true;
}

bool CEventListItemUI::IsSelected() const
{
	bool ret; 
	//IListItemUI pListItem; 
	//pListItem = static_cast<IListItemUI*>(GetInterface(UI_LIST_ITEM_INTERFACE_NAME)); 
	ASSERT( m_pOwner != NULL ); 
	if( m_pOwner->GetCurSel() == real_index )
	{
		ret = true; 
	}
	else
	{
		ret = false; 
	}

	return ret;
}

LONG GetRingItemIndex( LONG index, LONG item_count )
{
	LONG item_index; 

	do 
	{
		if( item_count == 0 )
		{
			item_index = INVALID_ITEM_INDEX; 
			break; 
		}

		item_index = ( index % item_count ); 
		if( item_index < 0 )
		{
			item_index = item_count + item_index; 
		}
	}while( FALSE );

	return item_index; 
}

LONG GetCorrectRingItemIndex( LONG index, LONG item_count )
{
	LONG item_index; 

	do 
	{
		if( item_count == 0 )
		{
			DBG_BP(); 
			item_index = INVALID_ITEM_INDEX; 
			break; 
		}

		if( index >= item_count * 2 )
		{
			DBG_BP(); 
			item_index = INVALID_ITEM_INDEX; 
			break; 
		}

		item_index = ( index % item_count ); 
		if( item_index < 0 )
		{
			item_index = item_count + item_index; 
		}
	}while( FALSE );

	return item_index; 
}

#include "action_ui_support.h"

LRESULT CEventListItemUI::init_sub_item_holder()
{
	LRESULT ret = ERROR_SUCCESS;  
	CListSubElementExUI *sub_item; 

	do 
	{
		sub_item = static_cast< CListSubElementExUI* >( m_pManager->FindSubControlByName( this, EventIconLabelName ) ); 
		ASSERT( sub_item != NULL ); 
		items.event_icon = sub_item; 
		items.event_icon->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast< CListSubElementExUI* >( m_pManager->FindSubControlByName( this, ProcIconLabelName ) ); 
		ASSERT( sub_item != NULL ); 
		items.proc_icon = sub_item; 
		items.proc_icon->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast< CListSubElementExUI* >( m_pManager->FindSubControlByName( this, EventNameLabelName ) ); 
		ASSERT( sub_item != NULL ); 
		items.event_name = sub_item; 
		items.event_name->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast< CListSubElementExUI* >( m_pManager->FindSubControlByName( this, IdLableName ) ); 
		ASSERT( sub_item != NULL ); 
		items.action_id = sub_item; 
		items.action_id->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast< CListSubElementExUI* >( m_pManager->FindSubControlByName( this, ResultLableName ) ); 
		ASSERT( sub_item != NULL ); 
		items.result = sub_item; 
		items.result->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast< CListSubElementExUI* >( m_pManager->FindSubControlByName( this, TimeLableName ) ); 
		ASSERT( sub_item != NULL ); 
		items.time = sub_item; 
		items.time->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName( this,  ProcIdLabelName ) );
		ASSERT( sub_item != NULL ); 
		items.proc_id = sub_item; 
		items.proc_id->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName( this,  ThreadIdLabelName ) );
		ASSERT( sub_item != NULL ); 
		items.thread_id = sub_item; 
		items.thread_id->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName( this,  ExeFileNameLabelName ) );
		ASSERT( sub_item != NULL ); 
		items.exe_file = sub_item; 
		items.exe_file->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName( this,  EventObjLableName ) );
		ASSERT( sub_item != NULL ); 
		items.event_obj = sub_item; 
		items.event_obj->SetTag( ( UINT_PTR )( PVOID )this ); 

		sub_item = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName( this,  EventDescLableName ) );
		ASSERT( sub_item != NULL ); 
		items.event_desc = sub_item; 
		items.event_desc->SetTag( ( UINT_PTR )( PVOID )this ); 
		
		sub_item = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName( this,  DataLabelName ) );
		ASSERT( sub_item != NULL ); 
		items.data = sub_item; 
		items.data->SetTag( ( UINT_PTR )( PVOID )this ); 
		
		sub_item = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName( this,  CallStackLabelName ) );
		ASSERT( sub_item != NULL ); 
		items.call_stack = sub_item; 
		items.call_stack->SetTag( ( UINT_PTR )( PVOID )this ); 
	}while( FALSE ); 
	return ret; 
}

#ifndef _UI_DEBUG
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
							 PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	TCHAR szBuf[ ITEM_TEXT_BUF_LEN ]; // = {0}; 
	WCHAR file_dos_path[ MAX_PATH ]; 
	ULONG ccb_ret_len; 

#ifdef _DEBUG
	ULONG begin_time; 
	ULONG end_time; 
#endif //_DEBUG

	//IListOwnerUI *ListOwner; 
	CEventListItemUI* pListElement = NULL;

	do 
	{
		if( ui_item == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( manager == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

#ifdef _DEBUG
		if( 0 != wcscmp( ui_item->GetClass(), UI_EVENT_LIST_ELEMENT_CLASS_NAME ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		begin_time = GetTickCount(); 
#endif //_DEBUG

		pListElement = ( CEventListItemUI* )ui_item; 
		pListElement->SetVisible( true ); 

		pListElement->SetIndex( filtered_index ); 
		pListElement->set_real_index( filtered_index ); 
		pListElement->set_filtered_index( filtered_index ); 
		pListElement->SetTag( ( UINT_PTR )( PVOID )action_notify ); 

		//CContainerUI* main_layout = static_cast< CContainerUI* >( manager->FindSubControlByName( pListElement, EventInfoLayoutContainerName ) ); 

		//if( main_layout != NULL )
		//{
		//	main_layout->SetMaxWidth( LIST_ROW_WIDTH ); 
		//	main_layout->SetFixedWidth( LIST_ROW_WIDTH ); 
		//	main_layout->SetVisible( true );
		//}

		do 
		{
			HRESULT hr; 

			if( pListElement->items.event_icon != NULL)
			{
				LPWSTR main_type_desc; 
				UI_PARAM main_type_ui; 
				action_main_type main_type; 
				_ret = _get_action_main_type( action->type, &main_type ); 
				if( _ret != ERROR_SUCCESS )
				{
					DBG_BP(); 
					break; 
				}

				_ret = get_action_main_type_desc( main_type, &main_type_desc, &main_type_ui ); 

				if( _ret != ERROR_SUCCESS )
				{
					break; 
				}

				ASSERT( main_type_desc != NULL 
					&& main_type_ui != NULL ); 

				hr = StringCbPrintfW( szBuf, sizeof( szBuf ), L"file=\'%s\' source=\'0,0,16,16\'", main_type_ui ); //mask=\'0xffff00ff\'
				if( FAILED( hr ) )
				{
					break; 
				}

				pListElement->items.event_icon->SetBkImage( ( LPCWSTR )szBuf ); 
				hr = StringCbPrintfW( szBuf, sizeof( szBuf ), L"%u", main_type ); //mask=\'0xffff00ff\'
				if( FAILED( hr ) )
				{
					break; 
				}
				
				pListElement->items.event_icon->SetUserData( szBuf ); 

				/*****************************************************************************
				NOTICE:
				需要像PROCMON支持换行的TOOLTIP。
				*****************************************************************************/
#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
				pListElement->items.event_icon->SetToolTip( main_type_desc ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP	
			}
			else
			{
				ASSERT( FALSE ); 
				//break; 
			}
		}while( FALSE );

		file_dos_path[ ARRAYSIZE( file_dos_path ) - 1 ] = L'\0'; 

		pListElement->items.proc_icon = static_cast< CListSubElementExUI* >( manager->FindSubControlByName( pListElement, ProcIconLabelName ) ); 

		//if( NULL == wcsstr( ctx->proc_name, L".exe" ) )
		//{
		//	dbg_print( MSG_IMPORTANT, "process name is error %ws\n", ctx->proc_name ); 
		//	//DebugBreak(); 
		//}
		
		if( pListElement->items.proc_icon != NULL)
		{
			HRESULT hr; 

			ctx->proc_name[ ARRAYSIZE( ctx->proc_name ) - 1 ] = L'\0'; 

			_ret = convert_native_path_to_dos( ctx->proc_name, 
				ctx->proc_name_len, 
				file_dos_path, 
				ARRAYSIZE( file_dos_path ), 
				&ccb_ret_len ); 

			if( _ret != ERROR_SUCCESS )
			{
				*file_dos_path = L'\0'; 
				hr = StringCbPrintfW( szBuf, sizeof( szBuf ), L"filepath=\'%s\' source=\'0,0,32,32\' mask=\'0xffff00ff\'", ctx->proc_name ); 
#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
				pListElement->items.proc_icon->SetToolTip( ctx->proc_name ); 
#endif // SHOW_EVENT_INFO_FROM_TOOLTIP
			}
			else
			{
				hr = StringCbPrintfW( szBuf, sizeof( szBuf ), L"filepath=\'%s\' source=\'0,0,32,32\' mask=\'0xffff00ff\'", file_dos_path ); 
								
#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
				pListElement->items.proc_icon->SetToolTip( file_dos_path ); 
#else
				pListElement->items.proc_icon->SetUserData( file_dos_path ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP
				//{
				//	proc_icon_label->set_x_locate_ctrl( list_header_items[ 0 ] ); 
				//}
			}

			if( SUCCEEDED( hr ) )
			{
				pListElement->items.proc_icon->SetBkImage( szBuf ); 
			}

		}
		else
		{
			ASSERT( FALSE ); 
			//break; 
		}

		//tString html_text; 
		//html_text = level_expand_image;

		//#if defined(UNDER_WINCE)
		//		_stprintf(szBuf, _T("<x %d>"), level_text_start_pos);
		//#else
		//		_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x %d>"), level_text_start_pos);
		//#endif

#if 0
		do 
		{
			LPWSTR action_desc; 
			UI_PARAM *ui_param; 

			ret = get_action_main_type_desc( action->type, 
				&action_desc, &ui_param ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ASSERT( ui_param != NULL ); 
			//ASSERT( action_desc != NULL ); 
		

		}while( FALSE );
#endif //0

		if( pListElement->items.event_name != NULL)
		{
			LPCWSTR event_name; 
			HRESULT hr; 

			event_name = get_event_name_ex( action ); 
			
			StringCbPrintfW( szBuf, 
				sizeof( szBuf ), 
				L"%s", 
				event_name ); 

			pListElement->items.event_name->SetText( szBuf ); 
			
#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.event_name->SetToolTip( szBuf ); 
#else 
			pListElement->items.event_name->SetUserData( szBuf ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP

			hr = StringCbPrintfW( szBuf, sizeof( szBuf ), L"%u", action->type ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			pListElement->items.event_name->SetUserData( szBuf ); 
		}
		else
		{
			ASSERT( FALSE ); 
		}

		pListElement->items.action_id = static_cast< CListSubElementExUI* >( manager->FindSubControlByName( pListElement, IdLableName ) ); 

		if( pListElement->items.action_id != NULL)
		{
			StringCbPrintfW( szBuf, 
				sizeof( szBuf ), 
				L"%u", 
				log_id ); 

			pListElement->items.action_id->SetText( szBuf ); 
			
#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.action_id->SetToolTip( szBuf ); 
#else
			pListElement->items.action_id->SetUserData( szBuf ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP
		}
		else
		{
			ASSERT( FALSE ); 
		}

		if( pListElement->items.result != NULL)
		{
			LPCWSTR result_desc; 
			result_desc = get_result_desc( ctx->last_result ); 

			if( *result_desc != L'\0' )
			{
				pListElement->items.result->SetText( result_desc ); 
				
#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
				pListElement->items.result->SetToolTip( result_desc ); 
#else
				pListElement->items.result->SetUserData( result_desc ); 
#endif // SHOW_EVENT_INFO_FROM_TOOLTIP
			}
			else
			{
				StringCbPrintfW( szBuf, 
					sizeof( szBuf ), 
					L"0x%0.8x", 
					ctx->last_result ); 

				pListElement->items.result->SetText( szBuf ); 
				
#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
				pListElement->items.result->SetToolTip( szBuf ); 
#else
				pListElement->items.result->SetUserData( szBuf ); 
#endif // SHOW_EVENT_INFO_FROM_TOOLTIP
			}
		}
		else
		{
			ASSERT( FALSE ); 
		}

		if( pListElement->items.time != NULL 
			&& ctx->time_text != NULL )
		{
			sprint_local_time( szBuf, 
				ITEM_TEXT_BUF_LEN, 
				ctx->time ); 

			//time_label->SetShowHtml(true);
			pListElement->items.time->SetText( szBuf ); 

#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.time->SetToolTip( szBuf ); 
#else
			pListElement->items.time->SetUserData( szBuf ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP

			//time_label->SetText( ctx->time_text ); 
			//time_label->SetToolTip( ctx->time_text ); 
			//{
			//	time_label->set_x_locate_ctrl( list_header_items[ 1 ] ); 
			//}
		}
		else
		{
			ASSERT( FALSE ); 
		}

		if( pListElement->items.proc_id != NULL )
		{
#if defined(UNDER_WINCE)
			_stprintf(szBuf, _T("<x 20><c #808080>%u</c>"), ctx->proc_id ); 
#else
			_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("%u"), ctx->proc_id );
#endif
			//proc_id_label->SetShowHtml( true );
			pListElement->items.proc_id->SetText( szBuf ); 

#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.proc_id->SetToolTip( szBuf ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP

			//{
			//	proc_id_label->set_x_locate_ctrl( list_header_items[ 2 ] ); 
			//}
		}

		pListElement->items.thread_id = static_cast<CListSubElementExUI*>(manager->FindSubControlByName(pListElement, ThreadIdLabelName ) );

		if( pListElement->items.thread_id != NULL )
		{
#if defined(UNDER_WINCE)
			_stprintf(szBuf, _T("<x 20><c #808080>%d</c>"), ctx->thread_id ); 
#else
			//注意，需要显示无效线程ID-1
			_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T( "%d" ), ctx->thread_id );
#endif
			//thread_id_label->SetShowHtml( true );
			pListElement->items.thread_id->SetText( szBuf ); 
			//pListElement->items.thread_id->SetToolTip( VERBOSE_INFORMATION_MODE_TIP ); 

#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.thread_id->SetToolTip( szBuf ); 
#else
			pListElement->items.thread_id->SetUserData( szBuf ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP

			//{
			//	thread_id_label->set_x_locate_ctrl( list_header_items[ 3 ] ); 
			//}
		}

		pListElement->items.exe_file = static_cast<CListSubElementExUI*>(manager->FindSubControlByName(pListElement, ExeFileNameLabelName ) );

		if( pListElement->items.exe_file != NULL )
		{
			ctx->proc_name[ ARRAYSIZE( ctx->proc_name ) - 1 ] = L'\0'; 
#if defined(UNDER_WINCE)
			_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), file_dos_path ); 
#else

			if( *file_dos_path == L'\0' )
			{
				_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("%s"), ctx->proc_name );
			}
			else
			{
				_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("%s"), file_dos_path );
			}
#endif
			//exe_file_label->SetShowHtml( true );
			pListElement->items.exe_file->SetText( szBuf ); 
			//pListElement->items.exe_file->SetToolTip( VERBOSE_INFORMATION_MODE_TIP ); 

#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.exe_file->SetToolTip( szBuf ); 
#else
			pListElement->items.exe_file->SetUserData( szBuf ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP

			//{
			//	exe_file_label->set_x_locate_ctrl( list_header_items[ 4 ] ); 
			//}
		}

		pListElement->items.event_obj = static_cast<CListSubElementExUI*>(manager->FindSubControlByName(pListElement, EventObjLableName ) );

		if( pListElement->items.event_obj != NULL )
		{
			LPCWSTR obj_path; 
			ULONG cc_ret_len; 

			_ret = get_object_path( action, 
				ctx, 
				szBuf, 
				ITEM_TEXT_BUF_LEN, 
				&obj_path, 
#ifdef _EVENT_CACHE_BY_MEMORY
				0, 
#else
				SAVE_INFO_BY_POINTER, 
#endif //_EVENT_CACHE_BY_MEMORY
				&cc_ret_len
				); 

			if( _ret != 0 )
			{
				//*szBuf = L'\0'; 
				ASSERT( obj_path[ 0 ] == L'\0' );  
			}

			//#if defined(UNDER_WINCE)
			//		_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), L"event_desc" ); 
			//#else
			//		_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x 20><c #808080>%s</c>"), L"event_desc" );
			//#endif
			//		event_desc_label->SetShowHtml( true );
			pListElement->items.event_obj->SetText( obj_path ); 
			//pListElement->items.event_obj->SetToolTip( VERBOSE_INFORMATION_MODE_TIP ); 

#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.event_obj->SetToolTip( obj_path ); 
#else
			pListElement->items.event_obj->SetUserData( obj_path ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP
			//{
			//	event_desc_label->set_x_locate_ctrl( list_header_items[ 5 ] ); 
			//}
		}

		if( pListElement->items.event_desc != NULL )
		{
			_ret = get_action_detail_ex( action, 
				ctx, 
				data, 
				data_len, 
				szBuf, 
				ITEM_TEXT_BUF_LEN, 
#ifdef _EVENT_CACHE_BY_MEMORY
				0
#else
				SAVE_INFO_BY_POINTER 
#endif //_EVENT_CACHE_BY_MEMORY
				); 

			if( _ret != ERROR_SUCCESS 
				&& _ret != ERROR_BUFFER_TOO_SMALL 
				&& _ret != ERROR_BUFFER_OVERFLOW )
			{
				*szBuf = L'\0'; 
			}

#ifdef _DEBUG
			if( _ret == ERROR_BUFFER_TOO_SMALL )
			{
				DBG_BP(); 
			}
#endif //_DEBUG

			//#if defined(UNDER_WINCE)
			//		_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), L"event_desc" ); 
			//#else
			//		_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x 20><c #808080>%s</c>"), L"event_desc" );
			//#endif
			//		event_desc_label->SetShowHtml( true );
			pListElement->items.event_desc->SetText( szBuf ); 
			//pListElement->items.event_desc->SetToolTip( VERBOSE_INFORMATION_MODE_TIP ); 

#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.event_desc->SetToolTip( szBuf ); 
#else
			pListElement->items.event_desc->SetUserData( szBuf ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP

			//{
			//	event_desc_label->set_x_locate_ctrl( list_header_items[ 5 ] ); 
			//}
		}

		if( pListElement->items.data != NULL )
		{
			//ASSERT( data != NULL ); 

			try
			{
				_ret = get_key_data_text( szBuf, ITEM_TEXT_BUF_LEN, data, data_len ); 

				if( _ret == ERROR_BUFFER_OVERFLOW )
				{
					szBuf[ ITEM_TEXT_BUF_LEN - 1 ] = L'\0'; 
				}
				else if( _ret != ERROR_SUCCESS )
				{
					szBuf[ 0 ] = L'\0'; 
				}
			}
			catch( ... )
			{
				szBuf[ 0 ] = L'\0'; 
			}

			//#if defined(UNDER_WINCE)
			//		_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), L"123456789" ); 
			//#else
			//		_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x 20><c #808080>%s</c>"), L"123456789" );
			//#endif
			//data_label->SetShowHtml( true );
			pListElement->items.data->SetText( szBuf ); 
			//pListElement->items.data->SetToolTip( VERBOSE_INFORMATION_MODE_TIP ); 

#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.data->SetToolTip( szBuf ); 
#else
			pListElement->items.data->SetUserData( szBuf ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP

			//{
			//	data_label->set_x_locate_ctrl( list_header_items[ 6 ] ); 
			//}
		}

		if( pListElement->items.call_stack != NULL )
		{
			//ULONG ccb_ret_len; 

			//_ret = get_stack_trace_dump_text( ctx->proc_id, 
			//	action_notify->action.stack_frame, 
			//	action_notify->action.frame_count, 
			//	L"S:\\symbols\\mysymbols", 
			//	szBuf, 
			//	ITEM_TEXT_BUF_LEN, 
			//	&ccb_ret_len ); 

			//unlock_cs( action_log_lock ); 

			//if( _ret != ERROR_SUCCESS )
			//{
			//	szBuf[ 0 ] = L'\0'; 
			//}
			//else
			//{
			//	szBuf[ ITEM_TEXT_BUF_LEN - 1 ] = L'\0'; 
			//}

//#if defined(UNDER_WINCE)
//			_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), stack_frame != NULL ? stack_frame: L"" ); 
//#else
//			_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T( "%s" )/*_T("<x 20><c #808080>%s</c>")*/, stack_frame != NULL ? stack_frame: L"" );
//#endif

			//call_stack_label->SetShowHtml( true );
			
			pListElement->items.call_stack->SetText( L"" ); //szBuf ); 
			//pListElement->items.call_stack->SetToolTip( VERBOSE_INFORMATION_MODE_TIP ); 

#ifdef SHOW_EVENT_INFO_FROM_TOOLTIP
			pListElement->items.call_stack->SetToolTip( szBuf ); 
#else
			pListElement->items.call_stack->SetUserData( szBuf ); 
#endif //SHOW_EVENT_INFO_FROM_TOOLTIP

			//{
			//	call_stack_label->set_x_locate_ctrl( list_header_items[ 7 ] ); 
			//}
		}

		if( ui_info != NULL )
		{
			switch( ui_info->cond )
			{
			case FLT_PATH: 
				pListElement->items.event_obj->SetBkColor( ui_info->bk_clr ); 
				break; 
			case FLT_PROCESS_NAME: 
				pListElement->items.exe_file->SetBkColor( ui_info->bk_clr ); 
				break; 
			case FLT_COMMAND: 
				break; 
			case FLT_PID: 
				pListElement->items.proc_id->SetBkColor( ui_info->bk_clr ); 
				break; 
			case FLT_PARENT_PID: 
				break; 
			case FLT_TID: 
				pListElement->items.thread_id->SetBkColor( ui_info->bk_clr ); 
				break; 
			case FLT_MAIN_TYPE: 
				break; 
			case FLT_ACTION_TYPE: 
				pListElement->items.event_name->SetBkColor( ui_info->bk_clr ); 	
				break; 
			case FLT_READ_WRITE: 
				break; 
			case FLT_DESCRITION: 
				break; 
			case FLT_DESC_DETAIL: 
				pListElement->items.event_desc->SetBkColor( ui_info->bk_clr ); 	
				break; 
			case FLT_ACTION_DATA: 
				pListElement->items.data->SetBkColor( ui_info->bk_clr ); 	
				break; 
			case FLT_ACTION_RESULT: 
				pListElement->items.result->SetBkColor( ui_info->bk_clr ); 	
				break; 
			case FLT_CALL_STACK: 
				//result_label->SetBkColor( ui_info->bk_clr ); 	

				break; 
			case FLT_DURATION: 
				pListElement->items.time->SetBkColor( ui_info->bk_clr ); 	
				break; 
			case FLT_DATE_TIME: 
				pListElement->items.time->SetBkColor( ui_info->bk_clr ); 	
				break; 
			case FLT_RELATIVE_TIME: 
				pListElement->items.time->SetBkColor( ui_info->bk_clr ); 	
				break; 
			case FLT_SESSION: 
				break; 
			case FLT_USER: 
				break; 
			case FLT_AUTH_ID: 
				break; 
			case FLT_ACTION_NO: 
				break; 
			case FLT_CORPORATION: 
				break; 
			case FLT_VERSION: 
				break; 
			case FLT_VIRTUAL_TECH: 
				break; 
			case FLT_CPU_ARCH: 
				break; 
			case FLT_TYPE_INVALID: 
				break; 
			}
		}

	}while( FALSE );

#ifdef _DEBUG
	end_time = GetTickCount(); 
	
	dbg_print( MSG_IMPORTANT, "%s elapse:%u\n", __FUNCTION__, end_time - begin_time ); 
#endif //_DEBUG

	return ret;
}

#include "performance_test.h"

NTSTATUS CALLBACK cache_event_log_to_ui( ULONG log_id, 
										sys_action_record *action, 
										action_context *ctx, 
										PVOID data, 
										ULONG data_len, 
										LPCWSTR stack_dump,
										//ULONG frame_count, 
										r3_action_notify_buf *action_notify, 
										PVOID work_context, 
										ULONG *state_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret; 
	ULONG ring_index; 
	ui_cache_context *ui_cache; 
	CControlUI **ui_items; 
	CPaintManagerUI *manager; 
	//ULONG state; 
	ULONG index; 
	filter_ui_info ui_info; 
	//ULONG RealIndexEnd; 
	//LONG NeedLoadItemCount; 
	//ULONG ItemRingIndex; 
	//ULONG i; 

	do 
	{
		if( state_out != NULL )
		{
			*state_out = 0; 
		}

#ifndef _EVENT_CACHE_BY_MEMORY
		ret = filter_action_log( action, 
			ctx, 
			data, 
			data_len ); 

		if( ret != ERROR_SUCCESS )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			log_trace( ( MSG_IMPORTANT, "action log is filtered\n" ) ); 
			break; 
		}

		perf_test_end( NULL, ( PVOID )log_id ); 
#else
		//不需进行再次过滤，这里的信息已经是过滤的，处理不同步问题。
		//ret = filter_log_for_ui( action, 
		//	ctx,  
		//	data, 
		//	data_len, 
		//	&ui_info ); 
		//
		//if( ret != ERROR_SUCCESS )
		//{
		//}
#endif //_EVENT_CACHE_BY_MEMORY

		if( work_context == NULL )
		{
			ASSERT( FALSE && "ui cache is null" ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		ui_cache = ( ui_cache_context* )work_context; 
		index = ui_cache->loaded_count; 

		if( action ==  NULL )
		{
			ASSERT( FALSE && "input null action" ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		if( ui_cache->manager == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		ui_items = ui_cache->ui_cache; 

		ring_index = GetCorrectRingItemIndex( ui_cache->ring_load_index + index, ui_cache->max_count ); 

		if( ring_index == INVALID_ITEM_INDEX )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		if( ui_items[ ring_index ] == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		//if( index > ui_cache->max_count )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER; 
		//	break; 
		//}

		manager = ui_cache->manager; 

#ifndef _DEBUG_SQLITE_PERFORMANCE
		ASSERT( log_id  >= 1 ); 
		ret = load_event_log_to_ui( ui_items[ ring_index ], 
			manager, 
			log_id, 
			ui_cache->filtered_index + ui_cache->loaded_count, 
			action, 
			ctx, 
			data, 
			data_len, 
			stack_dump, 
			action_notify, 
			&ui_info, 
			work_context ); 

		if( ret != ERROR_SUCCESS )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}
#endif //_DEBUG_SQLITE_PERFORMANCE

		//if( data == NULL )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER; 
		//	break; 
		//}

		//if( data_len == 0 )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER; 
		//	break; 
		//}

		//if( param == NULL )
		//{
		//	ASSERT( FALSE ); 
		//	ntstatus = STATUS_INVALID_PARAMETER; 
		//	break; 
		//}

		ui_cache->loaded_count ++; 

		if( ui_cache->loaded_count == ui_cache->load_count )
		{
			*state_out = LOG_IS_ALL_LOADED; 
			//ntstatus = STATUS_BREAKPOINT; 
			//ntstatus = ( NTSTATUS )SQLITE_ABORT; 
			break; 
		}
	}while( FALSE );

	return ntstatus; 
}

#endif //_UI_DEBUG

//#define EVENT_LOG_LIST_HEADER_ITEM_COUNT 10

ULONG log_list_def_column_width[] = { 
	DEFAULT_ICON_COLUMN_WIDTH, 
	DEFAULT_ACTION_NAME_COLUMN_WIDTH, 
	DEFAULT_TIME_COLUMN_WIDTH, 
	ACTION_ID_COLUMN_WIDTH, 
	DEFAULT_ICON_COLUMN_WIDTH, 
PROCESS_ID_COLUMN_WIDTH, 
THREAD_ID_COLUMN_WIDTH, 
EXE_FILE_PATH_COLUMN_WIDTH, 
OBJECT_COLUME_WIDTH, 
DESC_COLUMN_WIDTH, 
DATA_COLUMN_WIDTH, 
RESULT_COLUMN_WIDTH, 
CALL_STACK_COLUMN_WIDTH }; 

	LPCTSTR header_items_name[] = { _T( "header_action_icon" ), 
		_T( "header_action_name" ), 
		_T( "header_time" ), 
		_T( "header_id" ), 
		_T( "header_icon" ), 
		_T( "header_proc_id" ),  
		_T( "header_thread_id" ),  
		_T( "header_exe_file_path" ),  
		_T( "header_object" ), 
		_T( "header_desc" ), 
		_T( "header_data" ), 
		_T( "header_result" ), 
		_T( "header_call_stack" ) }; 

namespace DuiLib
{
const int EventInfoListItemNormalHeight = LIST_ITEM_NORMAL_HEIGHT;
const int EventInfoItemSelectedHeight = LIST_ITEM_SELECTED_HEIGHT;

CListLogUI::CListLogUI()
	: m_CurSelCacheIndex( -1 ) 
{
	SetItemShowHtml(true); 

	//SetDebugNames( UI_LIST_EX_CLASS_NAME, NULL ); 
	start_tooltip_control( &event_tooltip ); 
}

CListLogUI::~CListLogUI()
{
	stop_tooltip_control(); 
}

LPCTSTR CListLogUI::GetClass()
{
	return _T( "ListLogUI" ); 
}

#pragma region support draggable
//bool CListLogUI::AddItem(CControlUI* pControl)
//{
//	bool ret = false; 
//	do 
//	{
//		if (!pControl)
//		{
//			ASSERT( FALSE ); 
//			break; 
//		}
//
//		if(_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) == 0)
//		{
//			//break; 
//		}
//
//		if(_tcsicmp(pControl->GetClass(), _T("ListHeaderItemUI")) == 0)
//		{
//			CListHeaderItemUI *ListHeaderItem; 
//			ListHeaderItem = static_cast< CListHeaderItemUI* >( pControl ); 
//			ListHeaderItem->SetDragable( false ); 
//			ListHeaderItem->SetOwner( this ); 
//
//			//break; 
//		}
//		
//		ret = CListExUI::AddItem(pControl); 
//	}while( FALSE );
//
//	return ret;
//}
#pragma endregion support draggable

//BOOLEAN need_timer_scroll = FALSE; 

#pragma region fill_cache
//LRESULT WINAPI fill_sub_items_content( LONG ItemBeginIndex, LONG ItemCount, LONG *ItemLoaded )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//#ifndef _UI_DEBUG
//	ui_cache_context context; 
//#else
//	LONG ItemRingIndex; 
//#endif //_UI_DEBUG
//
//	LONG RealIndexEnd; 
//	LONG NeedLoadItemCount; 
//	LONG _ItemLoaded = 0; 
//	LONG i; 
//
//	do 
//	{
//
//		//if( m_pContainerCallback == NULL )
//		//{
//		//	ret = ERROR_NOT_READY; 
//		//	break; 
//		//}
//
//		//ret = m_pContainerCallback->LoadSubItemsContent( ItemBeginIndex, ItemCount ); 
//
//		if( ItemLoaded != NULL )
//		{
//			*ItemLoaded = 0; 
//		}
//
//		if( ItemBeginIndex < 0 )
//		{
//			DBG_BP(); 
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		if( ItemCount > MaxItemCacheCount )
//		{
//			ItemCount = MaxItemCacheCount; 
//			//ret = ERROR_BUFFER_OVERFLOW; 
//			//break; 
//		}
//
//		RealIndexEnd = ItemBeginIndex + ItemCount; 
//
//		if( RealIndexEnd < ItemBeginIndex )
//		{
//			DBG_BP(); 
//			__Trace( _T( "item index to loading is overflow %d:%d\n" ), ItemBeginIndex, ItemCount ); 
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//#ifdef _DEBUG
//		if( ( RealIndexEnd > ( ItemCacheBeginIndex + ItemCacheCount ) )
//
//			&& ( ItemBeginIndex < ItemCacheBeginIndex ) )
//		{
//			ASSERT( FALSE && "item to load too more" ); 
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//#endif //_DEBUG
//
//		{
//			LONG OverlapBegin; 
//			LONG OverlapEnd; 
//			
//
//			/****************************************************************************************
//			3      10      13      30
//
//			region1:3-10 region2:13-30 bigger left:13 smaller right:10 SR-BL=10-13=-3
//			region1:3-13 region2:10-30 bigger left:10 smaller right:13 SR-BL=13-10=3
//			region1:3-30 region2:10-13 bigger left:10 smaller right:13 SR-BL=10-13=3
//			****************************************************************************************/
//			if( ItemBeginIndex > ItemCacheBeginIndex )
//			{
//				OverlapBegin = ItemBeginIndex; 
//			}
//			else
//			{
//				OverlapBegin = ItemCacheBeginIndex; 
//			}
//
//			if( RealIndexEnd > ItemCacheBeginIndex + ItemCacheCount )
//			{
//				OverlapEnd = ItemCacheBeginIndex + ItemCacheCount; 
//			}
//			else
//			{
//				OverlapEnd = RealIndexEnd; 
//			}
//			
//			if( OverlapEnd > OverlapBegin )
//			{
//				_ItemLoaded += OverlapEnd - OverlapBegin; 
//			}
//#ifdef _DEBUG
//			else //if( OverlapEnd <= OverlapBegin )
//			{
//				__Trace( _T( "cached item region:%d:%d is not crossed with the loading region:%d:%d" ), 
//					ItemCacheBeginIndex, 
//					ItemCacheBeginIndex + ItemCacheCount, 
//					ItemBeginIndex, 
//					ItemBeginIndex + ItemCount ); 
//
//				//DBG_BP(); 
//				//_ItemLoaded += OverlapBegin - OverlapEnd; 
//			}
//#endif //_DEBUG
//		}
//
//		do 
//		{
//			if( ItemBeginIndex < ItemCacheBeginIndex )
//			{
//				NeedLoadItemCount = ( ItemCacheBeginIndex - ItemBeginIndex ); 
//
//				if( NeedLoadItemCount > MaxItemCacheCount )
//				{
//					NeedLoadItemCount = MaxItemCacheCount; 
//					ItemCacheCount = 0; 
//					ItemCacheBeginIndex = ItemBeginIndex; 
//					ItemRingBegin = 0; 
//				}
//
//#ifndef _UI_DEBUG
//				context.manager = m_pManager; 
//				context.loaded_count = 0; 
//				context.ring_load_index = ItemRingBegin - ( LONG )NeedLoadItemCount; 
//				context.max_count = MaxItemCacheCount; 
//				context.ui_cache = ControlUICache; 
//
//				ret = load_action_log( ItemBeginIndex, NeedLoadItemCount, &context ); 
//				if( context.loaded_count != NeedLoadItemCount )
//				{
//					log_trace( ( MSG_ERROR, "load the event log to ui is not completely successful" ) ); 
//				}
//
//#else
//				for( i = 0; i < NeedLoadItemCount; i ++ )
//				{
//					ItemRingIndex = GetRingItemIndex( ItemRingBegin - i - 1, MaxItemCacheCount ); 
//					ASSERT( ItemRingIndex != INVALID_ITEM_INDEX ); 
//
//					if( 0 > ( ItemBeginIndex + ( NeedLoadItemCount - i ) - 1 ) )
//					{
//						DBG_BP(): 
//					}
//
//					SetTempControlContent( ( ItemBeginIndex + ( NeedLoadItemCount - i ) - 1 ), ItemRingIndex ); 
//				}
//#endif //_UI_DEBUG
//
//#ifdef _UI_DEBUG
//				ItemCacheCount += NeedLoadItemCount; 
//#else
//				ItemCacheCount += context.loaded_count; 
//#endif //_UI_DEBUG
//				if( ItemCacheCount > MaxItemCacheCount )
//				{
//					ItemCacheCount = MaxItemCacheCount; 
//				}
//
//#ifdef _UI_DEBUG
//				ItemCacheBeginIndex -= NeedLoadItemCount; 
//#else
//				ItemCacheBeginIndex -= context.loaded_count; 
//#endif //_DEBUG
//
//				//#ifdef _DEBUG
//				if( ItemCacheBeginIndex < 0 )
//				{
//					DBG_BP(); 
//					ItemCacheBeginIndex = 0; 
//				}
//				//#endif //_DEBUG
//
//#ifdef _UI_DEBUG
//				ItemRingBegin -= NeedLoadItemCount; 
//#else
//				ItemRingBegin -= context.loaded_count;
//#endif //_UI_DEBUG
//
//				if( ItemRingBegin < 0 )
//				{
//					ItemRingBegin = ( MaxItemCacheCount + ItemRingBegin ); //ItemRingIndex
//				}
//
//#ifdef _UI_DEBUG
//				_ItemLoaded += NeedLoadItemCount; 
//#else
//				_ItemLoaded += context.loaded_count; 
//#endif //_UI_DEBUG
//			}
//		}while( FALSE );
//
//
//		do 
//		{
//			if( RealIndexEnd > ( ItemCacheBeginIndex + ItemCacheCount ) )
//			{
//				NeedLoadItemCount = RealIndexEnd - ( ItemCacheBeginIndex + ItemCacheCount ); 
//
//				if( NeedLoadItemCount == 0 )
//				{
//					//DBG_BP(); 
//					break; 
//				}
//
//				if( NeedLoadItemCount > MaxItemCacheCount )
//				{
//					NeedLoadItemCount = MaxItemCacheCount; 
//					ItemCacheCount = 0; 
//					ItemCacheBeginIndex = RealIndexEnd - MaxItemCacheCount; 
//					ItemRingBegin = 0; 
//				}
//#ifndef _UI_DEBUG
//				context.manager = m_pManager; 
//				context.loaded_count = 0; 
//				context.ring_load_index = ItemRingBegin + ItemCacheCount; 
//				context.max_count = MaxItemCacheCount; 
//				context.ui_cache = ControlUICache; 
//				//context.item_real_index = RealIndexEnd - NeedLoadItemCount; 
//
//				ret = load_action_log( RealIndexEnd - NeedLoadItemCount, NeedLoadItemCount, &context ); 
//
//				if( context.loaded_count != NeedLoadItemCount )
//				{
//					log_trace( ( MSG_ERROR, "load the event log to ui is not completely successful" ) ); 
//				}
//#else
//				//static LONG PrevItemIndex = 0; 
//				for( i = 0; i < NeedLoadItemCount; i ++ )
//				{
//					ItemRingIndex = GetRingItemIndex( ItemRingBegin + ItemCacheCount + i, MaxItemCacheCount ); 
//					ASSERT( ItemRingIndex != INVALID_ITEM_INDEX ); 
//					
//					//PrevItemIndex = ItemBeginIndex + ItemCacheCount + i; 
//
//					SetTempControlContent( ( RealIndexEnd - NeedLoadItemCount ) + i, ItemRingIndex ); 
//				}
//#endif //_UI_DEBUG
//
//#ifdef _UI_DEBUG
//				ItemCacheCount += NeedLoadItemCount;
//#else
//				ItemCacheCount += context.loaded_count; 
//#endif //_UI_DEBUG
//
//
//				if( ItemCacheCount > MaxItemCacheCount )
//				{
//					//DBG_BP(); 
//					ItemRingBegin = ( ( ItemRingBegin + ItemCacheCount ) % MaxItemCacheCount ); 
//					//ItemRingBegin += ( ItemCacheCount - MaxItemCacheCount ); 
//					ItemCacheBeginIndex += ( ItemCacheCount - MaxItemCacheCount ); 
//					ItemCacheCount = MaxItemCacheCount; 
//				}
//
//				//if( ( ItemRingBegin + ItemCacheCount ) > MaxItemCacheCount )
//				//{
//				//	DBG_BP(); 
//				//	
//				//}
//
//#ifdef _UI_DEBUG
//				_ItemLoaded += NeedLoadItemCount; 
//#else
//				_ItemLoaded += context.loaded_count; 
//#endif //_UI_DEBUG
//			}
//		}while( FALSE ); 
//
//		{
//			LONG PrevItemIndex; 
//			LONG ItemIndex; 
//			LONG ItemRingIndex; 
//			LONG PrevItemRingIndex; 
//			INT32 DebugRet; 
//
//			CListContainerElementUI *ListItem; 
//			CListContainerElementUI *PrevListItem; 
//
//			for( i = 1; i < ItemCacheCount; i++ )
//			{
//				ItemRingIndex = GetRingItemIndex( ItemRingBegin + i, MaxItemCacheCount ); 
//				PrevItemRingIndex = GetRingItemIndex( ItemRingBegin + i - 1, MaxItemCacheCount ); 
//
//				ListItem = ( ( CListContainerElementUI* )ControlUICache[ ItemRingIndex ] ); 
//				PrevListItem = ( CListContainerElementUI* )ControlUICache[ PrevItemRingIndex ]; 
//
//				ItemIndex = ListItem->GetIndex(); 
//				PrevItemIndex = PrevListItem->GetIndex(); 
//
//				//if( ItemCacheBeginIndex + i != ListItem->GetIndex() )
//				//{
//				//	DebugRet = ERROR_ERRORS_ENCOUNTERED;
//				//}
//
//				if( ItemIndex != ItemCacheBeginIndex + i )
//				{
//					DebugRet = ERROR_ERRORS_ENCOUNTERED;
//				}
//
//				if( PrevItemIndex + 1 != ItemIndex )
//				{
//					DebugRet = ERROR_ERRORS_ENCOUNTERED; 
//				}
//			}
//		}
//
//		if( ItemLoaded )
//		{
//			*ItemLoaded = _ItemLoaded; 
//		}
//
//	}while( FALSE ); 
//
//	return ret; 
//}
//
#pragma endregion fill_cache

CListBodyExUI::~CListBodyExUI()
{
	//if( ModalCtrl != NULL )
	//{
	//	delete ModalCtrl; 
	//	ModalCtrl = NULL; 
	//}
}

/****************************************************************************************
滚动范围计算:
1.必须要达到精确的结果，不可以定位至更长的范围。
2.上，下按键的工作机制:
	使用与搜索相同的机制：
	1.计算当前选中的条目。
	2.将当前选中的条目加1或减1。
	3.如果不在当前显示页面中，进行滚动定位处理。
	4.刷新被选中的条目显示。
3.鼠标滚动的工作机制:
	1.进行滚动定位处理，不改变选中条目状态。
4.PAGE键的工作机制:
	1.进行滚动定位处理，不改变选中条目状态。
5.HOME键的工作机制:
	定位至起始位置，不改变选中条目状态。
6.END键的工作机制:
	定位至结束位置，不改变选中条目状态。
****************************************************************************************/

//if( need_timer_scroll == TRUE 
//	&& event.Type == UIEVENT_TIMER && event.wParam == SCROLL_TIMERID)
//{
//	if (delay_left > 0)
//	{
//		--delay_left;
//		SIZE sz = GetScrollPos();
//		LONG lDeltaY =  (LONG)(CalculateDelay((double)delay_left / delay_number) * delay_delta_y);
//		if ((lDeltaY > 0 && sz.cy != 0)  || (lDeltaY < 0 && sz.cy != GetScrollRange().cy ))
//		{
//			sz.cy -= lDeltaY;
//			SetScrollPos(sz);
//			return;
//		}
//	}
//	delay_delta_y = 0;
//	delay_number = 0;
//	delay_left = 0;
//	m_pManager->KillTimer(this, SCROLL_TIMERID);
//	return;
//}

/**********************************************************
注意,解决以下问题
1.TOOLTIP窗体不可以自关闭
2.进程信息需要使用TOOLTIP工作方式
**********************************************************/

HWND get_process_dialog_window()
{
	HWND _window; 

	_window = process_dlg.GetHWND(); 

	return _window; 
}

LRESULT WINAPI close_info_dlgs()
{
	event_tooltip.Close(); 
	event_dlg.Close(); 
	process_dlg.Close(); 

	return ERROR_SUCCESS; 
}

ULONG test_enable = 0; 

LRESULT WINAPI test_window_hang()
{
	HWND captured_wnd; 
	BOOLEAN window_is_visible; 
	BOOLEAN window_is_enable; 
	
	captured_wnd = GetCapture(); 

	if( event_tooltip.GetHWND() == captured_wnd )
	{
		captured_wnd = GetCapture(); 
	}
	else if( event_dlg.GetHWND() == captured_wnd )
	{
		captured_wnd = GetCapture(); 
	}
	else if( process_dlg.GetHWND() == captured_wnd )
	{
		captured_wnd = GetCapture(); 
	}

	window_is_enable = IsWindowEnabled( process_dlg.GetHWND() ); 
	window_is_visible = IsWindowVisible( process_dlg.GetHWND() ); 


	window_is_enable = IsWindowEnabled( event_dlg.GetHWND() ); 
	window_is_visible = IsWindowVisible( event_dlg.GetHWND() ); 

	window_is_enable = IsWindowEnabled( event_tooltip.GetHWND() ); 
	window_is_visible = IsWindowVisible( event_tooltip.GetHWND() ); 

	if( test_enable & 1 )
	{
	EnableWindow( process_dlg.GetHWND(), TRUE ); 
	ShowWindow( process_dlg.GetHWND(), TRUE ); 
	}

	if( test_enable & 2)
	{
		EnableWindow( event_dlg.GetHWND(), TRUE ); 
		ShowWindow( event_dlg.GetHWND(), TRUE ); 
	}
	
	if( test_enable & 4 )
	{

		EnableWindow( event_tooltip.GetHWND(), TRUE ); 
		ShowWindow( event_tooltip.GetHWND(), TRUE ); 
	}
	return ERROR_SUCCESS; 
}

LRESULT hide_info_dlgs()
{
	//HWND captured_wnd; 
	
	event_tooltip.Hide(); 
	event_dlg.Hide(); 
	process_dlg.Hide(); 

	//captured_wnd = GetCapture(); 

	//if( NULL != captured_wnd )
	//{
	//	INT32 ret; 
	//	__Trace( _T( "captured windows is 0x%0.8x\n" ), captured_wnd ); 
	//
	//	ret = ReleaseCapture(); 
	//	if( ret == FALSE )
	//	{
	//		__Trace( _T( "release capture error %u\n", GetLastError() ) ); 
	//	}
	//}

	return ERROR_SUCCESS; 
}

LRESULT CListLogUI::OnMouseHover( TEventUI& event )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do
	{
		CStdString name; 

		name = event.pSender->GetName(); 

		if( /*0 == name.Compare( L"time" ) */
			//0 == name.Compare( L"event_icon" ) 
			//|| 0 == name.Compare( L"event_name" ) 
			 0 == name.Compare( L"action_id" ) 
			//|| 0 == name.Compare( L"proc_icon" ) 
			|| 0 == name.Compare( L"proc_id" ) 
			//|| 0 == name.Compare( L"thread_id" ) 
			//|| 0 == name.Compare( L"result" 
			//|| 0 == name.Compare( L"call_stack" )
			|| 0 == name.Compare( L"data" ) 
			//|| 0 == name.Compare( L"exe_file" )
			//|| 0 == name.Compare( L"event_obj" ) 
			//|| 0 == name.Compare( L"event_desc" ) 
			) 
		{
			CStdString text; 
			POINT pt = { event.ptMouse.x, event.ptMouse.y };
			::ClientToScreen( m_pManager->GetPaintWindow(), &pt );

			text = event.pSender->GetText(); 
			if( text.GetLength() == 0 )
			{
				text = GetUserData(); 
			}

			if( text.GetLength() > 0 )
			{
				text.Append( L"\r\n" ); 
			}

			text.Append( L"[" ); 
			text.Append( VERBOSE_INFORMATION_MODE_TIP ); 
			text.Append( L"]" ); 

			event_tooltip.Init( event.pSender, pt, text.GetData() ); 

			//dlg.set_event_notify( action ); 
			break; 
		}
		else if( 0 == name.Compare( L"call_stack" ) )
		{
			//最初的设计方案是只支持键盘用户，但对于平板用户也应该支持，同时没有必要做到只加载一次后缓存。
			BOOLEAN lock_held = FALSE; 
			ULONGLONG *stack_frames = NULL; 
			ULONG frame_count; 
			WCHAR *stack_dump = NULL; 
			ULONG cch_ret_len; 
			r3_action_notify_buf *action; 
			CControlUI *list_item; 
			CStdString text; 
			LRESULT __ret; 

			text = event.pSender->GetText(); 
			//if( text.GetLength() == 0 )
			//{
			//	text = GetUserData(); 
			//}

			if( text.GetLength() > 0 )
			{
				POINT pt = { event.ptMouse.x, event.ptMouse.y };
				::ClientToScreen( m_pManager->GetPaintWindow(), &pt );

				text.Append( L"\r\n" ); 
				text.Append( L"[" ); 
				text.Append( VERBOSE_INFORMATION_MODE_TIP ); 
				text.Append( L"]" ); 

				event_tooltip.Init( event.pSender, pt, text.GetData() ); 

				//dlg.set_event_notify( action ); 
				break; 
			}

			do 
			{
				list_item = ( CControlUI* )( PVOID )event.pSender->GetTag(); 
				ASSERT( 0 == wcscmp( list_item->GetName(), UI_EVENT_LIST_ELEMENT_CLASS_NAME ) ); 

				action = ( r3_action_notify_buf* )( PVOID )list_item->GetTag(); 

				w_lock_filtering_tip_lock(); 
				lock_held = TRUE; 

				action = ( r3_action_notify_buf* )( PVOID )list_item->GetTag(); 
				if( NULL == action )
				{
					break; 
				}

				ASSERT( action->action.frame_count <= ARRAYSIZE( action->action.stack_frame ) ); 

				stack_frames = ( ULONGLONG* )malloc( action->action.frame_count * sizeof( ULONGLONG ) ); 
				if( stack_frames == NULL )
				{
					break; 
				}

				memcpy( stack_frames, action->action.stack_frame, sizeof( ULONGLONG ) * action->action.frame_count ); 

				w_unlock_filtering_tip_lock(); 
				lock_held = FALSE; 

				stack_dump = ( LPWSTR ) malloc( MAX_STACK_DUMP_TEXT_LEN << 1 ); 
				if( stack_dump == NULL )
				{
					break; 
				}

				//#ifdef _DEBUG
				//						for( i = 0; i < MAX_STACK_DUMP_TEXT_LEN; i ++ )
				//						{
				//							stack_dump[ i ] = 0xcdcd; 
				//						}
				//#endif //_DEBUG

				stack_dump[ MAX_STACK_DUMP_TEXT_LEN - 1 ] = L'\0'; 

				frame_count = action->action.frame_count; 

				/*********************************************************************************
				直接使用DBGHELP提供的方法加载事件的堆栈信息的非常占用时间，每秒加载的事件只能达到百量级,
				必须对其结构有所完善，或者不加载堆栈信息。
				*********************************************************************************/
				__ret = _get_stack_trace_dump_text( action->action.action.ctx.proc_id, 
					&action->action.action.ctx.time, 
					stack_frames, 
					frame_count, 
					stack_dump, 
					MAX_STACK_DUMP_TEXT_LEN, 
					&cch_ret_len ); 

				if( __ret != ERROR_SUCCESS 
					&& __ret != ERROR_BUFFER_TOO_SMALL )
				{
					log_trace( ( MSG_ERROR, "print the stack dump text error %u\n", __ret ) ); 

					*stack_dump = L'\0'; 
				}

				ASSERT( stack_dump[ MAX_STACK_DUMP_TEXT_LEN - 1 ] == L'\0' ); 

				//#ifdef _DEBUG
				//					if( stack_dump[ 0 ] == 0xcdcd 
				//						&& stack_dump[ 1 ] == 0xcdcd )
				//					{
				//						__asm int 3; 
				//					}
				//#endif //_DEBUG

				if( stack_dump[ 0 ] == L'\0' )
				{
					break; 
				}

				text = stack_dump; 
				text.Append( L"\r\n[" ); 
				text.Append( VERBOSE_INFORMATION_MODE_TIP ); 
				text.Append( L"]" ); 

#define MAX_STACK_DUMP_COLUME_LENGTH 300
				if( cch_ret_len > MAX_STACK_DUMP_COLUME_LENGTH )
				{
					stack_dump[ MAX_STACK_DUMP_COLUME_LENGTH ] = L'\0'; 
				}

				event.pSender->SetText( stack_dump ); 
				POINT pt = { event.ptMouse.x, event.ptMouse.y };
				::ClientToScreen( m_pManager->GetPaintWindow(), &pt );

				event_tooltip.Init( event.pSender, pt, text.GetData() ); 

			} while ( FALSE );

			if( lock_held != FALSE )
			{
				w_unlock_filtering_tip_lock(); 
			}
			if( stack_dump != NULL )
			{
				free( stack_dump ); 
			}

			if( stack_frames != NULL )
			{
				free( stack_frames ); 
			}
		}
		else
		{
			//ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

/*******************************************************************************
方便，高效的显示详细的信息：
1.使用快捷键,Alt/Shift (不可以使用Ctrl，会与其它键冲突)
2.使用模式的方式，当使用Ctrl+V时，界面进行入详细信息模式，Ctrl+S进行简单浏览模式
	切换操作会感觉有些复杂，不是很方便。
*******************************************************************************/
LRESULT CListLogUI::OnMouseMove( TEventUI& event )
{
	LRESULT ret = ERROR_SUCCESS; 
	UINT key_state; 

	do
	{
		CStdString name; 

		if( event.pSender == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		name = event.pSender->GetName(); 

		{
			static ULONG old_scan_time = 0; 

			if( GetTickCount() - old_scan_time > 100 )
			{
				if( 0 == name.Compare( L"event_name" ) 
					|| 0 == name.Compare( L"time" ) 
					|| 0 == name.Compare( L"thread_id" ) )
				{

					hide_info_dlgs(); 
				}
				old_scan_time = GetTickCount(); 
			}
		}
		
		key_state = MapKeyState(); 
		if( 0 == ( ( MK_SHIFT | MK_ALT /*| MK_CONTROL*/ ) & key_state ) )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		if( 0 == name.Compare( L"event_icon" ) 
			|| 0 == name.Compare( L"action_id" ) )
		{
			CControlUI *list_item; 
			r3_action_notify_buf *action; 
			//CEventListItemUI *event_item; 
			POINT pt = { event.ptMouse.x, event.ptMouse.y };

			::ClientToScreen( m_pManager->GetPaintWindow(), &pt );

			list_item = ( CControlUI* )( PVOID )event.pSender->GetTag(); 
			ASSERT( wcscmp( list_item->GetClass(), UI_EVENT_LIST_ELEMENT_CLASS_NAME ) == 0 ); 

			action = ( r3_action_notify_buf* )( PVOID )list_item->GetTag(); 

			event_dlg.set_event( ( PVOID )action ); 

			hide_info_dlgs(); 
			event_dlg._Init( event.pSender, pt, ( PVOID )action ); 
			
			//HWND captured_wnd; 
			//captured_wnd = GetCapture(); 

			//if( NULL != captured_wnd )
			//{
			//	INT32 ret; 
			//	__Trace( _T( "captured windows is 0x%0.8x\n" ), captured_wnd ); 

			//	ret = ReleaseCapture(); 
			//	if( ret == FALSE )
			//	{
			//		__Trace( _T( "release capture error %u\n", GetLastError() ) ); 
			//	}
			//}
		}
		else if( 0 == name.Compare( L"proc_icon" ) 
			|| 0 == name.Compare( L"proc_id" ) 
			/*|| 0 == name.Compare( L"thread_id" )*/ )
		{
			CControlUI *list_item; 
			r3_action_notify_buf *action; 
			//CEventListItemUI *event_item; 
			POINT pt = { event.ptMouse.x, event.ptMouse.y };

			::ClientToScreen( m_pManager->GetPaintWindow(), &pt );

			list_item = ( CControlUI* )( PVOID )event.pSender->GetTag(); 
			ASSERT( wcscmp( list_item->GetClass(), UI_EVENT_LIST_ELEMENT_CLASS_NAME ) == 0 ); 

			action = ( r3_action_notify_buf* )( PVOID )list_item->GetTag(); 

			hide_info_dlgs(); 
			process_dlg._Init( event.pSender, pt, ( PVOID )action ); 

			//HWND captured_wnd; 
			//captured_wnd = GetCapture(); 

			//if( NULL != captured_wnd )
			//{
			//	INT32 ret; 
			//	__Trace( _T( "captured windows is 0x%0.8x\n" ), captured_wnd ); 

			//	ret = ReleaseCapture(); 
			//	if( ret == FALSE )
			//	{
			//		__Trace( _T( "release capture error %u\n", GetLastError() ) ); 
			//	}
			//}
		}
		else if( 0 == name.Compare( L"call_stack" ) ) 
			//|| 0 == name.Compare( L"result" 
		{
			BOOLEAN lock_held = FALSE; 
			ULONGLONG *stack_frames = NULL; 
			ULONG frame_count; 
			WCHAR *stack_dump = NULL; 
			ULONG cch_ret_len; 
			r3_action_notify_buf *action; 
			CControlUI *list_item; 
			LRESULT __ret; 

			do 
			{
				list_item = ( CControlUI* )( PVOID )event.pSender->GetTag(); 
				ASSERT( 0 == wcscmp( list_item->GetName(), UI_EVENT_LIST_ELEMENT_CLASS_NAME ) ); 

				action = ( r3_action_notify_buf* )( PVOID )list_item->GetTag(); 

				w_lock_filtering_tip_lock(); 
				lock_held = TRUE; 

				action = ( r3_action_notify_buf* )( PVOID )list_item->GetTag(); 
				if( NULL == action )
				{
					break; 
				}

				ASSERT( action->action.frame_count <= ARRAYSIZE( action->action.stack_frame ) ); 

				stack_frames = ( ULONGLONG* )malloc( action->action.frame_count * sizeof( ULONGLONG ) ); 
				if( stack_frames == NULL )
				{
					break; 
				}

				memcpy( stack_frames, action->action.stack_frame, sizeof( ULONGLONG ) * action->action.frame_count ); 

				w_unlock_filtering_tip_lock(); 
				lock_held = FALSE; 

				stack_dump = ( LPWSTR ) malloc( MAX_STACK_DUMP_TEXT_LEN << 1 ); 
				if( stack_dump == NULL )
				{
					break; 
				}

				//#ifdef _DEBUG
				//						for( i = 0; i < MAX_STACK_DUMP_TEXT_LEN; i ++ )
				//						{
				//							stack_dump[ i ] = 0xcdcd; 
				//						}
				//#endif //_DEBUG

				stack_dump[ MAX_STACK_DUMP_TEXT_LEN - 1 ] = L'\0'; 

				frame_count = action->action.frame_count; 

				/*********************************************************************************
				直接使用DBGHELP提供的方法加载事件的堆栈信息的非常占用时间，每秒加载的事件只能达到百量级,
				必须对其结构有所完善，或者不加载堆栈信息。
				*********************************************************************************/
				__ret = _get_stack_trace_dump_text( action->action.action.ctx.proc_id, 
					&action->action.action.ctx.time, 
					stack_frames, 
					frame_count, 
					stack_dump, 
					MAX_STACK_DUMP_TEXT_LEN, 
					&cch_ret_len ); 

				if( __ret != ERROR_SUCCESS 
					&& __ret != ERROR_BUFFER_TOO_SMALL )
				{
					log_trace( ( MSG_ERROR, "print the stack dump text error %u\n", __ret ) ); 

					*stack_dump = L'\0'; 
				}

				ASSERT( stack_dump[ MAX_STACK_DUMP_TEXT_LEN - 1 ] == L'\0' ); 

				//#ifdef _DEBUG
				//					if( stack_dump[ 0 ] == 0xcdcd 
				//						&& stack_dump[ 1 ] == 0xcdcd )
				//					{
				//						__asm int 3; 
				//					}
				//#endif //_DEBUG

				if( stack_dump[ 0 ] == L'\0' )
				{
					break; 
				}

#define MAX_STACK_DUMP_COLUME_LENGTH 300
				if( cch_ret_len > MAX_STACK_DUMP_COLUME_LENGTH )
				{
					WCHAR old_ch; 

					old_ch = stack_dump[ MAX_STACK_DUMP_COLUME_LENGTH ]; 
					stack_dump[ MAX_STACK_DUMP_COLUME_LENGTH ] = L'\0'; 
					event.pSender->SetText( stack_dump ); 

					stack_dump[ MAX_STACK_DUMP_COLUME_LENGTH ] = old_ch; 					
				}
				else
				{
					event.pSender->SetText( stack_dump ); 
				}

				POINT pt = { event.ptMouse.x, event.ptMouse.y };
				::ClientToScreen( m_pManager->GetPaintWindow(), &pt );

				hide_info_dlgs(); 
				event_tooltip.Init( event.pSender, pt ); 
				//HWND captured_wnd; 
				//captured_wnd = GetCapture(); 

				//if( NULL != captured_wnd )
				//{
				//	INT32 ret; 
				//	__Trace( _T( "captured windows is 0x%0.8x\n" ), captured_wnd ); 

				//	ret = ReleaseCapture(); 
				//	if( ret == FALSE )
				//	{
				//		__Trace( _T( "release capture error %u\n", GetLastError() ) ); 
				//	}
				//}

			} while ( FALSE );

			if( lock_held != FALSE )
			{
				w_unlock_filtering_tip_lock(); 
			}
			if( stack_dump != NULL )
			{
				free( stack_dump ); 
			}

			if( stack_frames != NULL )
			{
				free( stack_frames ); 
			}
			break; 
		}
		else if( 0 == name.Compare( L"data" ) 
			|| 0 == name.Compare( L"exe_file" )
			|| 0 == name.Compare( L"event_obj" ) 
			|| 0 == name.Compare( L"event_desc" ) ) 
		{
			POINT pt = { event.ptMouse.x, event.ptMouse.y };
			::ClientToScreen( m_pManager->GetPaintWindow(), &pt );

			hide_info_dlgs(); 
			event_tooltip.Init( event.pSender, pt ); 
			//HWND captured_wnd; 
			//captured_wnd = GetCapture(); 

			//if( NULL != captured_wnd )
			//{
			//	INT32 ret; 
			//	__Trace( _T( "captured windows is 0x%0.8x\n" ), captured_wnd ); 

			//	ret = ReleaseCapture(); 
			//	if( ret == FALSE )
			//	{
			//		__Trace( _T( "release capture error %u\n", GetLastError() ) ); 
			//	}
			//}
			//dlg.set_event_notify( action ); 
			break; 
		}
		else
		{
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

/***************************************************************************
通过TOOLTIP的方式显示信息带来很多使用方面的不方便。需要进行一步设计优化。
***************************************************************************/
/******************************************************************************
对提示信息显示方法的优化：
1.不再使用HOVER显示提示的方法。
2.缩短提示信息显示的等待时间

解决方法:
1.当移动至某一个控制中时，检查Ctrl, Atl, Shift功能皱起任一是否按下，如是进行
低间隔的显示。

******************************************************************************/

LRESULT WINAPI get_filter_menu_info( CEventListItemUI *list_item, FILTER_MENU_INFO *menu_info )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( list_item != NULL ); 
		ASSERT( menu_info != NULL ); 

		if( list_item->items.time != NULL )
		{
			menu_info->time = list_item->items.time->GetText(); 
		}
		else
		{
			ASSERT( FALSE ); 
		}
	}while( FALSE );

	return ret; 
}

//__NTSTATUS_FROM_WIN32
void CListLogUI::DoEvent( TEventUI& event ) 
{
	BOOLEAN handled = FALSE; 

	do 
	{
		if( !IsMouseEnabled() 
			&& event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
		{
			if( m_pParent != NULL )
			{
				m_pParent->DoEvent(event);
			}
			else
			{
				CListExUI::DoEvent(event);
			}

			break; 
		}

		switch( event.Type )
		{
		case UIEVENT_KEYDOWN: 
			{
				switch( event.chKey )
				{
				case VK_SHIFT: 
				case VK_LMENU: 
					{
						POINT pos; 
						CControlUI *ctrl; 
						CControlUI *old_ctrl; 
						
						pos = event.ptMouse; 

						ctrl = m_pManager->FindSubControlByPoint( this, pos ); 

						if( ctrl != NULL )
						{
							old_ctrl = event.pSender; 

							if( old_ctrl != ctrl )
							{
								event.pSender = ctrl; 

								OnMouseMove( event ); 

								event.pSender = old_ctrl; 
							}
							else
							{
								OnMouseMove( event ); 
							}

							//handled = TRUE; 
						}
					}
					break; 
				default:
					INT32 test = 0; 
					break; 
				}
			}
			break; 
		case UIEVENT_MOUSEMOVE:
			{
				OnMouseMove( event ); 
				break; 
			}
			break; 
		case UIEVENT_MOUSEHOVER:
			{
				OnMouseHover( event ); 
				break; 
			}
			break; 

		case UIEVENT_CONTEXTMENU: 
			{
				CStdString Name; 
				CControlUI *list_item; 
				CEventListItemUI *_list_item; 
				CControlUI *event_obj; 
				LRESULT ret; 

				do 
				{
					list_item = event.pSender->GetParent(); 

					if( list_item == NULL )
					{
						DBG_BP(); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					if( list_item->GetName().CompareNoCase( L"event_info_layout" ) != 0 )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					Name = event.pSender->GetName(); 
					event_obj = NULL; 

					if( Name.CompareNoCase( _T( "event_icon" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "event_name" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "time" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "action_id" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "proc_icon" ) ) == 0 )
					{
						event_obj = m_pManager->FindSubControlByName( list_item, L"proc_id" ); 
					}
					else if( Name.CompareNoCase( _T( "proc_id" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "thread_id" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "exe_file" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "event_obj" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "event_desc" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "data" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "result" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else if( Name.CompareNoCase( _T( "call_stack" ) ) == 0 )
					{
						event_obj = event.pSender; 
					}
					else
					{

					}

					if( event_obj != NULL )
					{
						LPCTSTR ui_conf_file; 
						//CStdString Text; 
						//CStdString obj_path; 
						CFilterMenu* pMenu = NULL; 

						do 
						{
							FILTER_MENU_INFO menu_info; 

							//if( LANG_ID_CN == get_cur_lang_id() )
							//{
							//	ui_conf_file = L"app_rule_menu.xml"; 
							//}
							//else
							{
								ui_conf_file = L"filtering_menu.xml"; 
							}

							//Text = event.pSender->GetText(); 

							//event_obj = m_pManager->FindSubControlByName( list_item, 
							//	L"event_obj" ); 
							//ASSERT( event_obj != NULL ); 

							//obj_path = event_obj->GetText(); 

							pMenu = new CFilterMenu(); 
							if( pMenu == NULL )
							{ 
								break; 
							}

							ret = pMenu->set_cfg_file( ui_conf_file ); 
							if( ret != ERROR_SUCCESS )
							{
								log_trace( ( MSG_ERROR, "set config menu file failed\n" ) ); 
							}

							POINT pt = {event.ptMouse.x, event.ptMouse.y};

							::ClientToScreen(m_pManager->GetPaintWindow(), &pt); 

							_list_item = ( CEventListItemUI* )( PVOID )event_obj->GetTag(); 
							
							ret = get_filter_menu_info( _list_item, &menu_info ); 
							if( ret != ERROR_SUCCESS )
							{

							}

							ret = pMenu->Init(event_obj, &menu_info, pt ); 

							if( ret != ERROR_SUCCESS )
							{
								ASSERT( FALSE ); 
								break; 
							}

							pMenu = NULL; 

						}while( FALSE );

						if( pMenu != NULL )
						{
							delete pMenu; 
						}
					}
				}while( FALSE ); 

				handled = TRUE; 
			}
			break; 
		case UIEVENT_SCROLLWHEEL: 
			{
				/****************************************************
				BUG:当前滚动不流畅

				处理滚动不流畅的问题的方法：
				1.只有滚动的长度达到一定的度之后才加载数据，不利之处是加载需要时间（100多条需3秒，会有停顿感）
				2.使用专有的加载工作线程进行加载，每次滚动时判断是否正在加载数据，如果是在
				加载，则跳过，不进行加载.
				1.或者，如果正在加载，中断当前加载过程，开始加载新的数据。
				不利之处是细节处理复杂。

				滚动加速处理包括以下几个方面:
				1.数据文件格式要有直接定位功能，通过ID直接算出偏移。
				但同时也支持SQL访问，需要改善SQLITE实现机制。
				2.对UI加载，不使用中间层缓存区，不然会引入缓存处理的
				延迟。
				3.在写UI时加锁，在读UI时试加锁，如果加锁失败，马上做其它工作，这样界面有最好的灵活度。
				4.在读UI时，如果读的不完全，记录下来。在UI写完成后，通知读线程，读线程读（绘）出其余UI。

				以上WORK方式，处理机制复杂，WORKER之间没有独立性，同时需要在WM_PAINT后手动PAINT。

				再次分析，把最花时间的工作（读取数据库）抽出，由WORKER来做，将从数据库读取出的所有的缓存区，一次性的打给界面。
				这种方式，独立性高，处理机制简单。

				加载事件信息的效率可以从3个方面入手处理：
				1.sqlite的加载机制的效率，分析SQLITE STEP函数的细节，从文件结构入手，初步考虑FLAT文件直接偏移计算法。
				2.加载时机，方式的速度，使用WORKER来进行加载，需要时给WORKER发一个任务。
				3.将信息加载到界面的效率，数据与输出接口或存储区直接定位输出，去掉所有的中间环节。
				4.处理显示过程的效率，如不使用FINDCTRL NEED_UPDATE来定位重绘控制，而是将其放在链表中，直接取出。
				****************************************************/
			}
			break; 
		default: 
			{
			}
			break; 
		}

		if( handled == TRUE )
		{
			break; 
		}

		CListExUI::DoEvent(event);

	}while( FALSE ); 
}

static bool OnCallStackButtonEvent( void* event )
{
    if( ( ( TEventUI* ) event )->Type == UIEVENT_BUTTONDOWN )
	{
        CControlUI* pButton = ( ( TEventUI* )event )->pSender; 

        if( pButton != NULL )
		{
            CListContainerElementUI* pListElement = ( CListContainerElementUI* )( pButton->GetTag() ); 

            if( pListElement != NULL ) 
			{
				pListElement->DoEvent( *( TEventUI* )event ); 
			}
		}
    }

    return true;
}

LRESULT CListLogUI::NotifyHeaderDragged(CControlUI* pList )
{
	LRESULT ret = ERROR_SUCCESS; 
	CStdString Name; 

	do 
	{
		//if( pList == NULL )
		//{
		//	ASSERT( FALSE ); 
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		m_bColumnWidthChanged = TRUE; 

		//Name = pList->GetName(); 
		//if( Name.Compare() )
		//{

		//}

	}while( FALSE ); 

	return ret; 
}

void CListLogUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	__super::DoPaint(hDC, rcPaint); 

	m_bColumnWidthChanged = FALSE; 
}

#define TEST_LIST_CACHE_CONTAINER_ITEM_COUNT 1024

LRESULT CListLogUI::TestPrepare()
{
	LRESULT ret = ERROR_SUCCESS; 
	RECT ListRect; 
	SIZE ScrollPos; 
	
	do 
	{
		ASSERT( m_pList != NULL ); 
		ListRect = m_pList->GetPos(); 
		ScrollPos = m_pList->GetScrollRange(); 

		//m_pList->ProcessScrollBar( ListRect, ScrollPos.cx, TEST_LIST_CACHE_CONTAINER_ITEM_COUNT * LIST_ITEM_NORMAL_HEIGHT ); 

	}while( FALSE );

	return ret; 
}


LRESULT CListLogUI::set_auto_scroll( BOOLEAN auto_scroll )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		m_pList->SetAutoScroll( auto_scroll ); 
	}while( FALSE );

	return ret; 
}

LRESULT CListLogUI::clear_events()
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG MaxItemCacheCount; 
	//LONG ItemLoaded; 

	do 
	{
		//ret = m_pList->InitItemCache(); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		MaxItemCacheCount = m_pList->GetMaxCachedItemCount(); 

		ASSERT( MaxItemCacheCount > 0 ); 

#ifdef _EVENT_CACHE_BY_MEMORY
		{
			PVOID item; 
			FindItemSelected( &item ); 
			m_iCurSel = -1; 
			ResetItemSelectUI( item ); 
		}
		m_CurSelCacheIndex = INVALID_ITEM_INDEX; 
		m_pList->ResetItemCache(); 
#else
		{
			PVOID item; 
			FindItemSelected( &item ); 
			m_iCurSel = -1; 
			ResetItemSelectUI( item ); 
		}

		m_CurSelCacheIndex = INVALID_ITEM_INDEX; 
		m_pList->ResetItemCache(); 
#endif //_EVENT_CACHE_BY_MEMORY

		//#ifdef FILL_CACHED_ITEM_SYNC
		//		ret = m_pList->FillSubItemsContent( 0, MaxItemCacheCount, &ItemLoaded ); 
		//#else		
		//		ret = m_pList->fill_cached_item_aio( 0, MaxItemCacheCount, &ItemLoaded ); 
		//#endif //FILL_CACHED_ITEM_SYNC

#ifdef _UI_DEBUG
		SetLogCount( TEST_LIST_ITEM_COUNT, TEST_LIST_ITEM_COUNT ); 
		TestPrepare(); 
#else
		{
			ULONG log_count; 

			w_lock_filtering_tip_lock(); 
			filtering_event = TRUE; 
			
			stop_event_trace(); 

			clear_all_events(); 

			//ret = filter_all_events(); 
			//if( ret != ERROR_SUCCESS )
			//{
			//	log_trace( ( MSG_ERROR, "filter all events error" ) ); 
			//}

			filtering_event = FALSE; 
			w_unlock_filtering_tip_lock(); 

			//ret = get_filtered_action_log_count( &log_count ); 
			//if( ret != ERROR_SUCCESS )
			//{
			//	ASSERT( FALSE ); 
			//	log_trace( ( MSG_ERROR, "get system action log count error" ) ); 

			//	fatal_error( ret ); 
			//	//break; 
			//}

			log_count = 0; 

			SetLogCount( log_count, get_store_event_count(), 0, 0 ); 

			//#ifdef _DEBUG
			//			LONG TestLoadIndex[ 10 ] = { 2000, 1000, 900, 800, 1001, 2330, 1003, 289, 100, 0 }; 
			//			ULONG TestLoadCount[ 10 ] = { 100, 103, 203, 102, 302, 302, 101, 19, 19 }; 
			//
			//			m_pList->TestLogLoading( TestLoadIndex, TestLoadCount, 10 ); 
			//
			//#endif //_DEBUG	
		}
#endif //_UI_DEBUG

		RefreshEventListUI(); 
	}while( FALSE ); 

	return ret; 
}

LRESULT CListLogUI::hilight_by_key_word()
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG MaxItemCacheCount; 

	do 
	{
		MaxItemCacheCount = m_pList->GetMaxCachedItemCount(); 

		ASSERT( MaxItemCacheCount > 0 ); 

		//m_pList->ResetItemCache(); 
		//m_pList->ItemDisplayOnReset = 0; 

		{
			SIZE init_pos = { 0 }; 
			SetScrollPos( init_pos ); 
		}

	}while( FALSE );

	return ret; 
}

/*********************************************************************************
搜索内容需求分析:
1.用户有时只确定一个关键字符串，需要找到相关的相应信息。

通常需要找的对象，细节2个字段
其它的字段包括：数据，堆栈等

搜索功能需要对数据内容进行处理，而不只是对界面进行处理。
搜索功能的具体处理过程如下：
1.找出之后日志数据中的下一条匹配日志。
2.将日志界面定位到相应的日志条目上。

高亮功能可以在UI显示时进行处理。

1.第1版本先做只针对对象的日志搜索功能，进行性能相关的情况的分析，如果各方面的情况
可以接受，再加入其它的条目搜索支持。
*********************************************************************************/
typedef enum _event_sub_item_index
{
	SUB_ITEM_EVENT_ICON, 
	SUB_ITEM_EVENT_NAME, 
	SUB_ITEM_TIME, 
	SUB_ITEM_ACTION_ID, 
	SUB_ITEM_PROC_ICON, 
	SUB_ITEM_PROC_ID, 
	SUB_ITEM_THREAD_ID, 
	SUB_ITEM_EXE_FILE, 
	SUB_ITEM_EVENT_OBJ, 
	SUB_ITEM_EVENT_DESC, 
	SUB_ITEM_DATA, 
	SUB_ITEM_RESULT, 
	SUB_ITEM_CALL_STACK, 
	SUB_ITEM_INVALID, 
}event_sub_item_index, *pevent_sub_item_index; 

wchar_t * __cdecl __wcsistr (
						  const wchar_t * wcs1,
						  const wchar_t * wcs2
						  )
{
	wchar_t *cp = (wchar_t *) wcs1;
	wchar_t *s1, *s2;

	if ( !*wcs2)
		return (wchar_t *)wcs1;

	while (*cp)
	{
		s1 = cp;
		s2 = (wchar_t *) wcs2;

		while ( *s1 && *s2 && 
			( (*s1==*s2) 
			|| ( ( ( *s1 >= 'a' && *s1 <='z' ) 
			? ( (*s1 + ( 'A' - 'a' )-*s2) == 0 )
			: ( ( *s1 - ( 'A' - 'a' )-*s2) == 0 ) ) ) ) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);
}

/******************************************************************************************************
分析功能优化方法:
1.高亮
2.内容查找方法多样化,通配，正则表达式。 
3.多条记录组合。
4.多条行为组合含义分析：1级：猜测 2级：描述
5.复杂行为组合含义分析：多进程，多线程行为互联含义分析。
6.数据流解码分析（网络协议，解密，解压）
7.内存操作行为分析(技术难度最高，虚拟机技术？)
8.
******************************************************************************************************/

//搜索的算法需要进行优化，比如加入数字字符的判断，进行特殊过程处理，加整性能。
event_sub_item_index WINAPI search_event_by_keyword( r3_action_notify_buf *action, LPCWSTR key_words )
{
	event_sub_item_index sub_item_index = SUB_ITEM_INVALID; 
	LRESULT _ret; 
	HRESULT hr; 
	LPCWSTR text; 
	ULONG cc_ret_len; 
	WCHAR text_buf[ ITEM_TEXT_BUF_LEN ]; 

	do 
	{
		text = action->action.action.ctx.proc_name; 
		//cc_ret_len = action->action.action.ctx.proc_name_len; 

		if( NULL != __wcsistr( text, key_words ) )
		{
			sub_item_index = SUB_ITEM_EXE_FILE; 
			break; 
		}

		//得到用于显示的字符串，最好定制更专用的方法处理对象路径
		_ret = get_object_path( &action->action.action.action, 
			&action->action.action.ctx, 
			text_buf, 
			ITEM_TEXT_BUF_LEN, 
			&text, 
			0, 
			&cc_ret_len ); 
	
		if( _ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( NULL != __wcsistr( text, key_words ) )
		{
			sub_item_index = SUB_ITEM_EVENT_OBJ; 
			break; 
		}

		{
			size_t retmain_size; 
			hr = StringCbPrintfExW( text_buf, sizeof( text_buf ), NULL, &retmain_size, 0, L"%u", action->action.action.ctx.proc_id ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			//cc_ret_len = ( ( sizeof( text_buf ) - retmain_size ) >> 1 ); 

			if( NULL != wcsstr( text_buf, key_words ) )
			{
				sub_item_index = SUB_ITEM_PROC_ID; 
				break; 
			}
		}

		{
			size_t retmain_size; 
			hr = StringCbPrintfExW( text_buf, sizeof( text_buf ), NULL, &retmain_size, 0, L"%u", action->action.action.ctx.thread_id ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			//cc_ret_len = ( ( sizeof( text_buf ) - retmain_size ) >> 1 ); 

			if( NULL != wcsstr( text_buf, key_words ) )
			{
				sub_item_index = SUB_ITEM_THREAD_ID; 
				break; 
			}
		}

	}while( FALSE );

	return sub_item_index; 
}

LRESULT CListLogUI::search_by_key_word( LPCWSTR key_words, BOOLEAN up_direction )
{
	LRESULT ret = ERROR_SUCCESS; 
	//INT32 _ret; 
	event_sub_item_index sub_item_found; 
	//CListLogUI *List; 
	LONG all_item_count; 
	//std::vector<r3_action_notify_buf*>::iterator event_iterator; 
	INT32 i; 

	do 
	{
		if( NULL == key_words )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		i = GetCurSel(); 

		if( i == INVALID_ITEM_INDEX )
		{
			i = 0; 
		}

		if( i > ( LONG )all_filtered_actions.size() - 1 )
		{
			ASSERT( FALSE ); 
			i = ( LONG )all_filtered_actions.size() - 1; 
		}

		if( FALSE == up_direction )
		{
			i += 1; 

			all_item_count = ( LONG )all_filtered_actions.size(); 

			//event_iterator = all_filtered_actions.begin() + i
			for( ; ( size_t )i < all_filtered_actions.size(); i ++ )
			{
				r_lock_event_filter_store(); 
				//r_lock_event_store(); 

				sub_item_found = search_event_by_keyword( all_filtered_actions[ i ], key_words ); 
				
				//r_unlock_event_store(); 
				r_unlock_event_filter_store(); 

				if( sub_item_found != SUB_ITEM_INVALID )
				{
					break; 
				}
			}

			if( i >= all_item_count )
			{
				ret = ERROR_NOT_FOUND; 
				break; 
			}
		}
		else
		{
			i -= 1; 

			//需要加锁

			for( ; i >= 0 ; i -- )
			{
				r_lock_event_filter_store(); 
				//r_lock_event_store(); 

				sub_item_found = search_event_by_keyword( all_filtered_actions[ i ], key_words ); 
				
				//r_unlock_event_store(); 
				r_unlock_event_filter_store(); 

				if( sub_item_found != SUB_ITEM_INVALID )
				{
					break; 
				}
			}

			if( i < 0 )
			{
				ret = ERROR_NOT_FOUND; 
				break; 
			}
		}

		{ 
			//LRESULT _ret; 
			LONG DisplayItemIndex; 
			SIZE init_pos = { 0 }; 
			SIZE cur_pos; 

			DisplayItemIndex = i - ( m_pList->GetItemShowCount() / 2 ); 
			if( DisplayItemIndex < 0 )
			{
				DisplayItemIndex = 0; 
			}

			cur_pos = GetScrollPos(); 

			init_pos.cy = DisplayItemIndex * LIST_ITEM_NORMAL_HEIGHT; 

			init_pos.cx = cur_pos.cx; 

			//_ret = m_pList->InvalidateListItem( 0, ItemDisplayCount ); 

			SetScrollPos( init_pos ); 

			if( DisplayItemIndex < m_pList->ItemCacheBeginIndex 
				|| DisplayItemIndex >= m_pList->ItemCacheBeginIndex + m_pList->MaxItemCacheCount )
			{
				DisplayItemIndex = DisplayItemIndex; 
			}

			//if( cur_pos.cy == 0 )
			//{
			//	ULONG Height; 
			//	Height = GetScrollRange().cy + GetFixedHeight(); 

			//	m_pList->OnScrollPosChanged( init_pos.cx, init_pos.cy, Height ); 
			//}

			m_pList->SetDisplayIndexFromRealIndex( DisplayItemIndex ); 

			SelectListItem( i ); 
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT CListLogUI::FilterEvents()
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG MaxItemCacheCount; 
	//LONG ItemLoaded; 

	do 
	{
		//ret = m_pList->InitItemCache(); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		MaxItemCacheCount = m_pList->GetMaxCachedItemCount(); 

		ASSERT( MaxItemCacheCount > 0 ); 

#ifdef _EVENT_CACHE_BY_MEMORY
		m_pList->ItemDisplayOnReset = 0; 
		{
			PVOID item; 
			FindItemSelected( &item ); 
			m_iCurSel = -1; 
			ResetItemSelectUI( item ); 
		}
		m_CurSelCacheIndex = INVALID_ITEM_INDEX; 
		m_pList->ResetItemCache(); 

#else
		{
			PVOID item; 
			FindItemSelected( &item ); 
			m_iCurSel = -1; 
			ResetItemSelectUI( item ); 
		}
		m_CurSelCacheIndex = INVALID_ITEM_INDEX; 
		m_pList->ResetItemCache(); 
#endif //_EVENT_CACHE_BY_MEMORY

//#ifdef FILL_CACHED_ITEM_SYNC
//		ret = m_pList->FillSubItemsContent( 0, MaxItemCacheCount, &ItemLoaded ); 
//#else		
//		ret = m_pList->fill_cached_item_aio( 0, MaxItemCacheCount, &ItemLoaded ); 
//#endif //FILL_CACHED_ITEM_SYNC

#ifdef _UI_DEBUG
		SetLogCount( TEST_LIST_ITEM_COUNT, TEST_LIST_ITEM_COUNT ); 
		TestPrepare(); 
#else
		{
			ULONG log_count; 

			w_lock_filtering_tip_lock(); 
			filtering_event = TRUE; 

			stop_event_trace(); 
			_clear_filtered_events(); 

			ret = filter_all_events(); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "filter all events error" ) ); 
			}

			filtering_event = FALSE; 
			w_unlock_filtering_tip_lock(); 

			ret = get_filtered_action_log_count( &log_count ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				log_trace( ( MSG_ERROR, "get system action log count error" ) ); 

				fatal_error( ret ); 
				//break; 
			}

			SetLogCount( log_count, get_store_event_count(), 0, 0 ); 

//#ifdef _DEBUG
//			LONG TestLoadIndex[ 10 ] = { 2000, 1000, 900, 800, 1001, 2330, 1003, 289, 100, 0 }; 
//			ULONG TestLoadCount[ 10 ] = { 100, 103, 203, 102, 302, 302, 101, 19, 19 }; 
//
//			m_pList->TestLogLoading( TestLoadIndex, TestLoadCount, 10 ); 
//
//#endif //_DEBUG	
#endif //_UI_DEBUG

			RefreshEventListUI(); 
		}

	}while( FALSE );

	return ret; 
}

LRESULT CListLogUI::RefreshEventListUI()
{
	LRESULT _ret; 
	SIZE init_pos = { 0 }; 
	SIZE cur_pos; 
	LONG ItemDisplayCount; 

	ItemDisplayCount = m_pList->GetDisplayItemCount(); 

	//if( ItemDisplayCount > log_count )
	//{
	//	m_pList->OnItemAdded( log_count ); 
	//}
	//else
	//{
	//	m_pList->OnItemAdded( ItemDisplayCount ); 
	//}

	_ret = m_pList->InvalidateListItem( 0, ItemDisplayCount ); 

	cur_pos = GetScrollPos(); 

	SetScrollPos( init_pos ); 

	if( cur_pos.cy == 0 )
	{
		ULONG Height; 
		Height = GetScrollRange().cy + GetFixedHeight(); 

		m_pList->OnScrollPosChanged( init_pos.cx, init_pos.cy, Height ); 
	}

	return ERROR_SUCCESS; 
}

LRESULT CListLogUI::_SetLogCount( ULONG filtered_count, ULONG all_count, ULONG notify_count, ULONG time )
{
	LRESULT ret = ERROR_SUCCESS; 
#define MAX_STATUS_TEXT_LEN 64
	CLabelUI *status_label; 
	WCHAR LogStatus[ MAX_STATUS_TEXT_LEN ]; 
	HRESULT hr; 

	do 
	{
		status_label = ( CLabelUI* )m_pManager->FindControl( _T( "tracing_status" ) ); 
		if( status_label == NULL )			
		{
			ASSERT( FALSE ); 
			break; 
		}

		//if( filtered_count < 30 )
		//{
		//	DebugBreak(); 
		//}

		//if( filtered_count == 1 )
		//{
		//	DebugBreak(); 
		//}

#ifdef LANG_EN
#define EVENT_STATISITIC_STATUS_FORMAT_STRING L"Output:%u/All:%u(%u%%) %u/%ums"
#else
#define EVENT_STATISITIC_STATUS_FORMAT_STRING L"显示事件数%u:事件总数%u(%u%%) %u/%u毫秒"
#endif //LANG_EN
		hr = StringCbPrintfW( LogStatus, sizeof( LogStatus ), 
			EVENT_STATISITIC_STATUS_FORMAT_STRING, 
			filtered_count, 
			all_count, 
			all_count == 0 ? 0 : ( filtered_count * 100 / all_count ), 
			notify_count, 
			time ); 

		if( FAILED( hr ) )
		{
			ASSERT( FALSE ); 
		}

		__Trace( L"%s", LogStatus ); 

		status_label->SetText( LogStatus ); 

	}while( FALSE ); 

	return ret; 
}

LRESULT CListLogUI::SetLogCount( ULONG filtered_count, ULONG all_count, ULONG notify_count, ULONG time )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{

		ASSERT( m_pList != NULL ); 
		
		m_pList->SetItemRealCount( filtered_count ); 

		ret = _SetLogCount( filtered_count, all_count, notify_count, time ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT CListLogUI::_Init()
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG MaxItemCacheCount; 
	LONG ItemLoaded; 
	CListHeaderItemUI *ListHeaderItem; 
	INT32 i; 

	do 
	{

		m_pList->m_pOwner = this; 
		m_bMouseEnabled = TRUE; 
		SetListHeaderCallback( this ); 

		ASSERT( ARRAYSIZE( header_items_name ) == ARRAYSIZE( log_list_def_column_width ) ); 
		for( i = 0; i < ARRAYSIZE( log_list_def_column_width ); i ++ )
		{
			ListHeaderItem = ( CListHeaderItemUI* )m_pManager->FindSubControlByName( m_pHeader, header_items_name[ i ] ); 
			if( ListHeaderItem == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			ASSERT( TRUE == ( ListHeaderItem->GetName() == header_items_name[ i ] ) ); 

			//ListHeaderItem->SetOwner( this ); 
			ListHeaderItem->SetFixedWidth( log_list_def_column_width[ i ] ); 
		}
	}while( FALSE ); 

	do 
	{
		//m_pList->SetOwner( this ); 

		ret = m_pList->InitItemCache(); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		//if( m_iIndex % 2 == 0 ) 
		//{
		//	iBackColor = pInfo->dwBkColor;
		//}

		//if( (m_uButtonState & UISTATE_HOT) != 0 ) 
		//{
		//	iBackColor = pInfo->dwHotBkColor;
		//}

		//if( IsSelected() ) 
		//{
		//	iBackColor = pInfo->dwSelectedBkColor;
		//}

		//if( !IsEnabled() ) 
		//{
		//	iBackColor = pInfo->dwDisabledBkColor;
		//}

		MaxItemCacheCount = m_pList->GetMaxCachedItemCount(); 

		ASSERT( MaxItemCacheCount > 0 ); 

		{
			ULONG LoadItemCount; 
			
			if( all_filtered_actions.size() > MaxItemCacheCount )
			{
				LoadItemCount = MaxItemCacheCount; 
			}
			else
			{
				LoadItemCount = ( ULONG )all_filtered_actions.size(); 
			}

#ifdef FILL_CACHED_ITEM_SYNC
			ret = m_pList->FillSubItemsContent( 0, LoadItemCount, &ItemLoaded ); 
#else		
			ret = m_pList->fill_cached_item_aio( 0, MaxItemCacheCount, &ItemLoaded ); 
#endif //FILL_CACHED_ITEM_SYNC
		}
#ifdef _UI_DEBUG
		SetLogCount( TEST_LIST_ITEM_COUNT, TEST_LIST_ITEM_COUNT ); 
		TestPrepare(); 
#else
		{
			ULONG log_count; 

			ret = get_filtered_action_log_count( &log_count ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				log_trace( ( MSG_ERROR, "get system action log count error" ) ); 

				fatal_error( ret ); 
				//break; 
			}

			SetLogCount( log_count, 
				( ULONG )get_store_event_count(), 
				0, 
				0 ); 

//#ifdef _DEBUG
//			LONG TestLoadIndex[ 10 ] = { 2000, 1000, 900, 800, 1001, 2330, 1003, 289, 100, 0 }; 
//			ULONG TestLoadCount[ 10 ] = { 100, 103, 203, 102, 302, 302, 101, 19, 19 }; 
//
//			m_pList->TestLogLoading( TestLoadIndex, TestLoadCount, 10 ); 
//
//#endif //_DEBUG	
		}
#endif //_UI_DEBUG

		//m_pList->SetPos( m_pList->GetPos() ); 

		//m_pList->NeedUpdate(); 

		//if( m_pManager != NULL )
		//{
		//	m_pManager->NeedUpdate(); 
		//}

		//ULONG event_count; 

		//ret = get_action_log_count( &event_count ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	log_trace( ( MSG_ERROR, "get event count error" ) ); 
		//}

		do 
		{
			//CControlUI *ctrl; 
			//CScrollBarUI *scroll_bar; 
			//LONG list_width; 

//#ifdef _UI_DEBUG
//			ctrl = m_pManager->FindControl( _T( "friends" ) ); 
//#else
//			ctrl = m_pManager->FindControl( _T( "event_list_layout" ) ); 
//#endif //_UI_DEBUG
//
//			ASSERT( ctrl != NULL ); 

			//list_width = ctrl->GetPos(); 

			//if( list_width < 770 )
			//{
			//	list_width = 770; 
			//}

			//scroll_bar = GetHorizontalScrollBar(); 
			//if( scroll_bar != NULL )
			//{
			//	//scroll_bar->SetScrollRange( LIST_ROW_WIDTH - list_width ); 
			//	scroll_bar->SetBorderColor( 0xffbbbbbb ); 
			//}

			//scroll_bar = GetVerticalScrollBar(); 
			//if( scroll_bar != NULL )
			//{
			//	scroll_bar->SetBorderColor( 0xffbbbbbb ); 
			//}

		}while( FALSE );
	
	}while( FALSE );
	return ret; 
}

bool CListBodyExUI::IsUpdateNeeded() const
{
	return m_bUpdateNeeded;
}

ULONG CListBodyExUI::GetRingIndexFromDisplayIndex( ULONG ItemIndex )
{
	ULONG CachedItemIndex;  
	
	CachedItemIndex = GetRingItemIndex( ItemDisplayBegin + ItemIndex, MaxItemCacheCount ); 
	return CachedItemIndex; 
}

LRESULT CListBodyExUI::SetItemRealCount( LONG ItemCount )
{
	ULONG OldCount; 

	OldCount = ItemRealCount; 
	ItemRealCount = ItemCount; 

#ifdef _DEBUG
	if( ItemRealCount != all_filtered_actions.size() )
	{
		DBG_BP(); 
	}
#endif //_DEBUG

	if( ItemRealCount > ItemDisplayCount 
		&& m_pVerticalScrollBar != NULL )
	{
		INT32 YRange;  
		YRange = ItemRealCount * ItemDisplayHeight; 

		m_pVerticalScrollBar->SetScrollRange( YRange ); 
	}

	return ItemRealCount; 
}

LRESULT CListBodyExUI::GetItemDisplaySize( SIZE *Size )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		//Size->cx = ItemDisplayWidth; 
		//Size->cy = ItemDisplayHeight; 

		ret = CCachedItemManager::GetItemDisplaySize( Size ); 
	}while( FALSE );

	return ret; 
}

LRESULT CListBodyExUI::SetItemDisplaySize( LONG Width, LONG Height )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = CCachedItemManager::SetItemDisplaySize( Width, Height ); 
	} while ( FALSE ); 

	return ret; 
}

LRESULT CListBodyExUI::DisplayRegionFilled( LONG DisplayItem )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG RingRightPartCount; 
	LONG RingLeftPartCount; 

	do 
	{
		//if( ItemCacheCount < MaxItemCacheCount )
		
		//{
		//	RingLeftPartCount = ItemRingBegin; 
		//	RingRightPartCount = ItemCacheCount - ItemRingBegin; 
		//}

		if( DisplayItem < ItemRingBegin )
		{
			RingLeftPartCount = ItemRingBegin - DisplayItem; 
			RingRightPartCount = 0; 
		}
		else //if( DisplayItem >= ItemRingBegin )
		{
			RingLeftPartCount = ItemRingBegin; 
			RingRightPartCount = ItemCacheCount - DisplayItem; 
		}

		if( RingLeftPartCount + RingRightPartCount < ItemDisplayCount )
		{
			//DBG_BP(); 
			ret = ERROR_NOT_ENOUGH_QUOTA; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT CListBodyExUI::SetDisplayIndexFromRealIndex( LONG RealIndex )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG DisplayItemIndex; 

	DisplayItemIndex = RealIndex - ItemCacheBeginIndex; 

	ItemDisplayBegin = GetRingItemIndex( DisplayItemIndex + ItemRingBegin, MaxItemCacheCount ); //from 0

#if 1
	{
		CControlUI *pControl; 
		LRESULT __ret; 
		LONG index_diff; 

		__ret = GetDisplayItemAt( 0, &pControl ); 
		if( __ret == ERROR_SUCCESS )
		{
			if( ( ( CListContainerElementExUI* )pControl )->GetIndex() != RealIndex )
			{
				index_diff = ( ( CListContainerElementExUI* )pControl )->GetIndex() - RealIndex;  
			}
		}
	}
#endif //0

	return ret; 
}

LRESULT CListBodyExUI::OnScrollPosChanged( LONG PosX, LONG PosY, LONG Height )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG ItemIndex; 
	LONG ItemLoadCount; 
	//NOTICE:****item count can't greater than 2G.
	//LONG EndItemIndex; 
	LONG ItemLoaded; 
	//ULONG DisplayCount; 
//#ifdef _DEBUG
	//ULONG BeginTime; 
	//ULONG EndTime; 
//#endif //_DEBUG

	do 
	{
		if( PosY > Height )
		{
			//DBG_BP(): 
			PosY = Height; 

			//ret = ERROR_INVALID_PARAMETER; 
			//break; 
		}
		else if( PosY < 0 )
		{
			PosY = 0; 
		}

		/******************************************************************************
		界面显示的开始日志条目索引号计算方法：
		1.通过移动后的位置计算出起始索引号，但如果需要计算出目标位置对应的日志条目的物理
		编号，需要消耗计算时间。
		2.记录最后的日志显示结束编号，当进行移动时，从最后记录的结束编号开始取日志。但使
		用这种方法，需要移动是完全连续了，如果间隔超过的一个完整窗体显示日志总条目，会发
		生日志结束编号低于开始位置编号的问题。
		
		当前还是采用第一种与第二种方法结合的定位方法。
		******************************************************************************/
		//ItemIndex = ( ULONG )( ( ( DOUBLE )PosY / ( DOUBLE )Height ) * ItemRealCount );

		ItemIndex = ( ULONG )( PosY / ItemDisplayHeight ); 

		//EndItemIndex = ItemCacheBeginIndex + ItemCacheCount; 

		//BeginTime = GetTickCount(); 

		if( ItemIndex >= ItemRealCount )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ItemIndex + ItemDisplayCount > ItemRealCount )
		{
			ItemLoadCount = ItemRealCount - ItemIndex; 
		}
		else
		{
			ItemLoadCount = ItemDisplayCount; 
		}

#ifdef FILL_CACHED_ITEM_SYNC
		ret = FillSubItemsContent( ItemIndex, ItemLoadCount, &ItemLoaded ); 
#else
		ret = fill_cached_item_aio( ItemIndex, ItemDisplayCount, &ItemLoaded ); 
#endif //FILL_CACHED_ITEM_SYNC

		if( ret != ERROR_SUCCESS )
		{
			__Trace( _T( "fill the item content when scroll\n" ) ); 
			//break; 
		}

		//EndTime = GetTickCount(); 

		//__Trace( _T( "fill the ui content of %u controls elapse %u\n" ), ItemDisplayCount, EndTime - BeginTime ); 

#ifdef FILL_CACHED_ITEM_SYNC
		if( ItemIndex >= ItemCacheBeginIndex 
			&& ItemIndex < ItemCacheBeginIndex + ItemCacheCount )
		{
#ifdef _DEBUG
			CControlUI *Item; 
			LRESULT _ret;  
#endif //_DEBUG	
			LONG DisplayItemIndex; 
			
			DisplayItemIndex = ItemIndex - ItemCacheBeginIndex; 

			ItemDisplayBegin = GetRingItemIndex( DisplayItemIndex + ItemRingBegin, MaxItemCacheCount ); //from 0
			
#ifdef _DEBUG
			_ret = GetDisplayItemAt( 0, &Item ); 
			if( _ret == ERROR_SUCCESS )
			{
				DisplayItemIndex = ( ( CEventListItemUI *)Item )->get_filtered_index(); 
				
				if( DisplayItemIndex != ItemIndex )
				{
					DisplayItemIndex = DisplayItemIndex; 
				}
			}
#endif //_DEBUG

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

		//ListItemUpdateCount ++; 

#endif //FILL_CACHED_ITEM_SYNC

	}while( FALSE ); 

	return ret; 
}

LRESULT CListBodyExUI::GetDisplayItemAt( LONG Index, CControlUI **UIItem )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ItemIndex; 
	//ULONG ItemCacheCount; 
	//ULONG ItemDisplayBegin; 
	CControlUI *CachedItem; 

	do 
	{

		if( Index == INVALID_ITEM_INDEX )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 			
		}

		if( UIItem == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 			
		}

		*UIItem = NULL; 

		if( Index >= ItemCacheCount )
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}

		//ItemDisplayBegin = 0; 

		ItemIndex = GetRingIndexFromDisplayIndex( Index ); 
		if( ItemIndex == INVALID_ITEM_INDEX )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		CachedItem = CachedControlUI[ ItemIndex ]; 

		if( CachedItem == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			DBG_BP(); 
			break; 
		}

		//ret = LockDisplayCacheItem( CachedItem ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		*UIItem = CachedItem; 
	}while( FALSE );

	return ret; 
}

/*************************************************************************************
此DIRECTUI库的绘图方式不精确，存在性能损失。需要完善。
*************************************************************************************/
void CListBodyExUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = { 0 };
	//ULONG RealItemCount; 
	ULONG DisplayItemCount; 
	LRESULT ret; 
	SIZE ItemSize; 
	RECT ItemRc; 
	ULONG it; 
	RECT rc; 
	CRenderClip clip; 
	//CListExUI *Owner; 

	CCachedContainerUI::DoPaint( hDC, rcPaint ); 

	do 
	{
		if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) 
		{
			break; 
		}

		CRenderClip::GenerateClip( hDC, rcTemp, clip );

		TRACE( _T( "enter %s" ), __TFUNCTION__ ); 

		//dump_contro_ui_info( this ); 

		//TRACE( _T( "begin paint container %s" ), __TFUNCTION__ ); 

		//CControlUI::DoPaint(hDC, rcPaint);

		do 
		{
			//if( ListItemUpdateCount == 0 )
			//{
			//	break; 
			//}

			DisplayItemCount = GetItemShowCount(); 

			if( INVALID_ITEM_COUNT == DisplayItemCount )
			{
				//CCachedContainerUI::DoPaint( hDC, rcPaint ); 
				break; 
			}

			if( DisplayItemCount == 0 )
			{
				break; 
			}

			ret = CListBodyExUI::GetItemDisplaySize( &ItemSize ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( ItemSize.cx == 0 
				|| ItemSize.cy == 0 )
			{
				break; 
			}

			rc = m_rcItem;
			rc.left += m_rcInset.left;
			rc.top += m_rcInset.top;
			rc.right -= m_rcInset.right;
			rc.bottom -= m_rcInset.bottom; 

			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
			{
				rc.right -= m_pVerticalScrollBar->GetFixedWidth();
			}

			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			{
				rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
			}

			//if item rect is not included in the container rectangle then don't check the sub item.
			if( !::IntersectRect(&rcTemp, &rcPaint, &rc) ) 
			{
#if 0
				for( it = 0; it < DisplayItemCount; it++ ) 
				{
					CControlUI* pControl = NULL; 

					ret = GetDisplayItemAt( it, &pControl ); 
					if( ret == ERROR_SUCCESS )
					{
						//#ifdef _DEBUG
						//					DebugControlPaint( pControl ); 
						//#endif //_DEBUG
						ASSERT( pControl != NULL ); 

						if( !pControl->IsVisible() ) 
						{
							continue;
						}

						ItemRc.left = this->GetPadding().left; 
						ItemRc.right = ItemRc.left + ItemSize.cx; 
						ItemRc.top = it * ItemSize.cy; 
						ItemRc.bottom = ItemRc.top + ItemSize.cy; 

						if( !::IntersectRect(&rcTemp, &rcPaint, &ItemRc ) ) 
						{
							continue;
						}

						if( pControl ->IsFloat() )
						{
							if( !::IntersectRect(&rcTemp, &m_rcItem, &ItemRc ) ) 
							{
								continue;
							}

							pControl->DoPaint(hDC, rcPaint);
						}
					}
				}
#endif //0
			}
			else 
			{
				CRenderClip childClip;
				CRenderClip::GenerateClip(hDC, rcTemp, childClip);

				for( it = 0; it < DisplayItemCount; it++ ) 
				{
					CControlUI* pControl = NULL; 

					ret = GetDisplayItemAt( it, &pControl ); 
					if( ret != ERROR_SUCCESS )
					{
						//DBG_BP(); 
						continue; 
					}

					//#ifdef _DEBUG
					//					DebugControlPaint( pControl ); 
					//#endif //_DEBUG

					ASSERT( pControl != NULL ); 

					if( !pControl->IsVisible() ) 
					{
						continue;
					}

					//ItemSize.cy = 12;
					ItemRc.left = this->GetPadding().left + rc.left; 
					ItemRc.right = ItemRc.left + ItemSize.cx; 
					ItemRc.top = it * ItemSize.cy + rc.top; 
					ItemRc.bottom = ItemRc.top + ItemSize.cy; 

					if( !::IntersectRect(&rcTemp, &rcPaint, &ItemRc ) ) 
					{
						continue;
					}

					//if( pControl ->IsFloat() ) 
					//{
					//	if( !::IntersectRect(&rcTemp, &m_rcItem, &ItemRc ) ) 
					//	{
					//		continue;
					//	}

					//	CRenderClip::UseOldClipBegin(hDC, childClip);

					//	pControl->DoPaint(hDC, rcPaint);

					//	CRenderClip::UseOldClipEnd(hDC, childClip);
					//}
					//else 
					{
						//if( !::IntersectRect(&rcTemp, &rc, &ItemRc ) ) 
						//{
						//	continue;
						//}

						//pControl->SetManager( NULL, NULL, false ); 

						//pControl->SetPos( rcTemp ); 

						//pControl->SetPosNoLayout( rcTemp ); 

#ifndef _UI_DEBUG
						__Trace( _T( "painting the sub control,its position result:\n" ) ); 

						( ( CContainerUI* )pControl )->DumpSubItemPos( _T( "time" ) ); 
						( ( CContainerUI* )pControl )->DumpSubItemPos( _T( "event_desc" ) ); 
#else
						( ( CContainerUI* )pControl )->DumpSubItemPos( _T( "description" ) ); 

						__Trace( _T( "item real index is %d\n" ), ( ( CListContainerElementExUI* )pControl )->GetIndex() ); 
#endif //_UI_DEBUG

						//( ( CListContainerElementUI* )pControl )->SetIndex( it ); 

						pControl->DoPaint(hDC, rcPaint/*ItemRc*/); 
					}
				}

				//if( it == ItemDisplayCount )
				//{
				//	if( ListItemUpdateCount > 0 )
				//	{
				//		ListItemUpdateCount --; 
				//	}
				//}
			}
		}while( FALSE ); 

		if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() ) 
		{
			if( ::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) 
			{
				m_pVerticalScrollBar->DoPaint(hDC, rcPaint);
			}
		}

		if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() ) 
		{
			if( ::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) 
			{
				m_pHorizontalScrollBar->DoPaint(hDC, rcPaint);
			}
		}

	}while( FALSE ); 
}

void CListContainerElementExUI::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if( m_pParent != NULL ) m_pParent->DoEvent(event);
		else CControlUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_BUTTONDOWN ) //item can be selected by r button. 
	{
		if( IsEnabled() )
		{
			//m_pManager->SendNotify(this, _T("itemclick"));
			if( false == IsSelected() )
			{
				Select( true ); 
			}

			Invalidate();
		}

		if( m_pParent != NULL ) 
		{
			m_pParent->DoEvent(event);
		}

		return; 
	}
	else if( event.Type == UIEVENT_BUTTONUP 
		|| event.Type == UIEVENT_MOUSEMOVE )
	{
		if( m_pParent != NULL ) 
		{
			m_pParent->DoEvent(event);
		}
	}
	else if( event.Type == UIEVENT_RBUTTONDOWN )
	{
		return;
	}

	__super::DoEvent( event ); 
}

LRESULT __fastcall CListBodyExUI::AllocOneItemCache( CDialogBuilder *dlgBuilder, CControlUI** item_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	CEventListItemUI* pListElement = NULL;

	do 
	{
		event_list_item_builder list_item_builder; 

		if( dlgBuilder == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( item_out == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; ; 
			break; 
		}

		if( !dlgBuilder->GetMarkup()->IsValid() ) {
#ifndef _UI_DEBUG
#define ACTION_LIST_ITEM_XML_FILE _T( "action_list_item.xml" ) 
#else
#define ACTION_LIST_ITEM_XML_FILE _T( "friend_list_item.xml" ) 
#endif //_UI_DEBUG
			pListElement = static_cast<CEventListItemUI*>(dlgBuilder->Create(ACTION_LIST_ITEM_XML_FILE, (UINT)0, &list_item_builder, m_pManager));
		}
		else 
		{
			pListElement = static_cast<CEventListItemUI*>(dlgBuilder->Create( &list_item_builder, m_pManager));
		}

		if( pListElement == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		pListElement->SetOwner( m_pOwner ); 

		pListElement->SetManager( m_pManager, this, false ); 

		pListElement->SetVisible( true ); 

		pListElement->init_sub_item_holder(); 

		//CRect rcPadding = text_padding; 

		//pListElement->SetPadding( rcPadding );

		//pListElement->SetMaxWidth( LIST_ROW_WIDTH ); 
		pListElement->SetFixedWidth( LIST_ROW_WIDTH ); 
		
		CContainerUI* main_layout = static_cast< CContainerUI* >( m_pManager->FindSubControlByName( pListElement, EVENT_INFO_LAYOUT_CONTAINER_NAME ) ); 

		if( main_layout != NULL )
		{
//#ifdef _UI_DEBUG
			//main_layout->SetMaxWidth( LIST_ROW_WIDTH ); 
			main_layout->SetFixedWidth( LIST_ROW_WIDTH ); 
//#endif //_UI_DEBUG
			main_layout->SetVisible( true );
		}

		pListElement->SetFixedHeight( ItemDisplayHeight ); 

		*item_out = ( CControlUI* )pListElement; 

	}while( FALSE );

	return ret; 
}

#ifdef _UI_DEBUG
LRESULT CListBodyExUI::SetTempControlContent( LONG ItemRealIndex, LONG ItemSlotIndex )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 
	CControlUI *text_ctrl; 
	CStdString text; 

	do 
	{
		if( ItemSlotIndex > MaxItemCacheCount )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ctrl = CachedControlUI[ ItemSlotIndex ]; 


#define	TEXT_CTRL_WIDTH 120

		ASSERT( ctrl != NULL ); 

		text_ctrl = m_pManager->FindSubControlByName( ctrl, _T( "description" ) ); 

		text.Format( _T( "text:%u" ), ItemRealIndex ); 

		text_ctrl->SetFixedWidth( TEXT_CTRL_WIDTH ); 

		text_ctrl->SetText( text ); 

		text_ctrl = m_pManager->FindSubControlByName( ctrl, _T( "icon" ) ); 

		text_ctrl->SetBkImage( _T( "man.png" ) ); 

		( ( CListContainerElementUI* )ctrl )->SetIndex( ItemRealIndex ); 
		
		//ctrl->SetIndex( )

		//text_ctrl = m_pManager->FindSubControlByName( ctrl, _T( "nickname" ) ); 

		//text.Format( _T( "text2:%u" ), ItemIndex ); 

		//text_ctrl->SetText( text ); 

	} while ( FALSE );

	return ret; 
}
#endif //_UI_DEBUG

/**********************************************************************************************************
1.使用OFFSET时，第1次加载只需要72MS，但使用ID第1次加载需要至少100MS，OFFSET是否等于ROWID? 为什么OFFSET会
开始时会更快，但随着位置的加大，延时也越长。

这时总日志的个数等于过滤后的日志数，而不是所有日志的个数。

使用结构化的数据文件 + 对文件进行内存映射 进行日志的读取和处理，速度比较快，PROCMON也许使用了这种方法。
**********************************************************************************************************/

/*********************************************************************************************************
找出目标位置对应的日志条目编号的过程：
1.明确查找起始点的过滤后编号，真实编号
2.从真实编号开始查找过滤后编号，每找到对起始点过滤后编号一次就加（减1），直到找到目标编号为止。

定位UI控件编号与过滤后编号，真实编号。
每一个控制都加入一个CONTEXT结构，里面包括两种编号。
控制类事加入两种编号的成员。
*********************************************************************************************************/

LRESULT CListBodyExUI::TestLogLoading( LONG Indexes[], ULONG Counts[], ULONG InputsCount )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG ItemLoaded; 
	INT32 i; 
	INT32 j; 

	do 
	{
		ASSERT( InputsCount > 0 ); 
		ASSERT( Indexes != NULL ); 
		ASSERT( Counts != NULL ); 

		for( i = 0; ( ULONG )i < InputsCount; i ++ )
		{
			LONG ItemFilteredIndex; 
			LONG ItemRealIndex; 

			CEventListItemUI *ListItem; 
			//CListContainerElementExUI *PrevListItem; 

			dbg_print( MSG_IMPORTANT, "begin loading logs index(%u-%u)\n", Indexes[ i ], Indexes[ i ] + Counts[ i ] ); 

			for( j = 1; j < ItemCacheCount; j++ )
			{
				ListItem = ( ( CEventListItemUI* )CachedControlUI[ j ] ); 

				if( ListItem != NULL )
				{
					ItemFilteredIndex = ListItem->get_filtered_index(); 
					ItemRealIndex = ListItem->get_real_index(); 

					dbg_print( MSG_IMPORTANT, "item(%u) index:%u->%u\n", j, ItemFilteredIndex, ItemRealIndex ); 
				}
			}

			ret = FillSubItemsContent( Indexes[ i ], Counts[ i ], &ItemLoaded ); 
			if( ret != ERROR_SUCCESS )
			{

			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT CListBodyExUI::FilteredIndex2RealIndex( LONG FilteredIndex, LONG *RealIndex )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG CachedItemBeginFilterIndex; 
	LONG CachedItemEndFilterIndex; 
	LONG CachedItemBeginRealIndex; 
	LONG CachedItemEndRealIndex; 

	CEventListItemUI *ListItem; 

	do 
	{
		if( ItemCacheCount > 0 )
		{
			CachedItemBeginFilterIndex = GetRingItemIndex( ItemRingBegin + 0, MaxItemCacheCount ); 

			ListItem = ( ( CEventListItemUI* )CachedControlUI[ CachedItemBeginFilterIndex ] ); 

			CachedItemBeginFilterIndex = ListItem->get_filtered_index(); 
			CachedItemBeginRealIndex = ListItem->get_real_index(); 

			ASSERT( ItemCacheBeginIndex == CachedItemBeginFilterIndex ); 

			CachedItemEndFilterIndex = GetRingItemIndex( ItemRingBegin + ItemCacheCount - 1, MaxItemCacheCount ); 

			ListItem = ( ( CEventListItemUI* )CachedControlUI[ CachedItemEndFilterIndex ] ); 

			CachedItemEndFilterIndex = ListItem->get_filtered_index(); 
			CachedItemEndRealIndex = ListItem->get_real_index(); 

#ifdef _DEBUG
			if( CachedItemEndFilterIndex < CachedItemBeginFilterIndex ) 
			{
				INT32 i; 
				LONG ItemCacheIndex; 
				LONG ItemFilteredIndex; 
				LONG ItemRealIndex; 

				
				for( i = 0; i < ItemCacheCount; i ++ )
				{
					ItemCacheIndex = GetRingItemIndex( i, MaxItemCacheCount ); 
					ListItem = ( ( CEventListItemUI* )CachedControlUI[ ItemCacheIndex ] ); 

					ItemFilteredIndex = ListItem->get_filtered_index(); 
					ItemRealIndex = ListItem->get_real_index(); 

					dbg_print( MSG_IMPORTANT, "item(%u[no ring]) index:%u->%u\n", i, ItemFilteredIndex, ItemRealIndex ); 
				}

				dbg_print( MSG_IMPORTANT, "ring begin index:%u\n", ItemRingBegin ); 
			}
#endif //_DEBUG
		}
		else
		{
			CachedItemBeginFilterIndex = 0; 
			CachedItemEndFilterIndex = 0; 
			CachedItemBeginRealIndex = 0; 
			CachedItemEndRealIndex = 0; 
		}

		if( FilteredIndex == CachedItemEndFilterIndex )
		{
			*RealIndex = ItemCacheBeginIndex + ItemCacheCount; 
		}
		else if( FilteredIndex > CachedItemEndFilterIndex )
		{
			ret = _filtered_index_2_real_index( FilteredIndex, 
				CachedItemEndFilterIndex, 
				 //ItemFilteredBeginIndex + ItemCacheCount > 0 ? 
				 //ItemFilteredBeginIndex + ItemCacheCount - 1 
				 CachedItemEndRealIndex, 
				 RealIndex, 
				 0 ); 
		}
		else if( FilteredIndex == CachedItemBeginFilterIndex )
		{
			*RealIndex = ItemCacheBeginIndex; 
		}
		else
		{
			ret = _filtered_index_2_real_index( FilteredIndex, 
				CachedItemBeginFilterIndex, 
				CachedItemBeginRealIndex, 
				RealIndex, 
				LOG_FILTER_DECENDING ); 
		}

	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		//__asm int 3; 
	}

	return ret; 
}

LRESULT CListLogUI::get_select_event_info()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT CListBodyExUI::ResetItemCache()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	do 
	{
		for( i = 0; i < MaxItemCacheCount; i ++ )
		{
			ASSERT( CachedControlUI[ i ] != NULL ); 
			{
				CachedControlUI[ i ]->SetTag( 0 ); 
			}
		}

		ItemCacheBeginIndex = 0; 
		ItemCacheCount = 0; 
		ItemRingBegin = 0; 
		ItemDisplayBegin = 0; 
		ItemRealCount = 0; 
		//ListItemUpdateCount ++; 
		//ULONG ItemDisplayOffset; 
		//LONG ItemDisplayCount; 
		//SIZE ItemDisplaySize; 
		//LONG ItemDisplayWidth; 
		//LONG ItemDisplayHeight; 
	}while( FALSE );

	return ret; 
}

LRESULT CListBodyExUI::FillSubItemsContent( LONG ItemBeginIndex, LONG ItemCount, LONG *ItemLoaded )
{
	LRESULT ret = ERROR_SUCCESS; 
#ifndef _UI_DEBUG
	ui_cache_context context; 
#endif //_UI_DEBUG

	LONG RealIndexEnd; 
	LONG NeedLoadItemCount; 
	//LONG ItemRingIndex; 
	LONG _ItemLoaded = 0; 
	//LONG i; 

	LONG RealBeginIndex; 
	LONG FilteredBeginIndex; 

#ifdef _DEBUG
	ULONG begin_time; 
	ULONG end_time; 
#endif //_DEBUG

	do 
	{

#ifdef _DEBUG
		{
			begin_time = GetTickCount(); 

			__Trace( _T( "enter %s begin index %d, index count %d, %u\n" ), 
				__TFUNCTION__, 
				ItemBeginIndex, 
				ItemCount, 
				begin_time ); 
		}
#endif //_DEBUG

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
			ASSERT( FALSE && "item to load too more" ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( RealIndexEnd <= ItemRealCount ); 
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

				//if( NeedLoadItemCount > MaxItemCacheCount )
				//{
				//	NeedLoadItemCount = MaxItemCacheCount; 
				//}

#ifndef _EVENT_CACHE_BY_MEMORY
				{
					ULONG tick_begin; 
					ULONG tick_end; 

					tick_begin = GetTickCount(); 

					ret = FilteredIndex2RealIndex( ItemBeginIndex, &RealBeginIndex ); 

					tick_end = GetTickCount(); 

					log_trace( ( MSG_IMPORTANT, "filtering the logs elapse %u\n", tick_end - tick_begin ) ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}
				}

				ASSERT( RealBeginIndex >= ItemBeginIndex ); 
#endif //_EVENT_CACHE_BY_MEMORY

				if( NeedLoadItemCount > MaxItemCacheCount )
				{
					NeedLoadItemCount = MaxItemCacheCount; 
					ItemCacheCount = 0; 
					FilteredBeginIndex = ItemBeginIndex + MaxItemCacheCount; 
				}
				else
				{
					FilteredBeginIndex = ItemCacheBeginIndex; 
				}

#ifndef _UI_DEBUG
				context.manager = m_pManager; 
				context.loaded_count = 0; 
				context.filtered_index = ItemBeginIndex; 
				context.ring_load_index = ItemRingBegin - ( LONG )NeedLoadItemCount; 
				context.max_count = MaxItemCacheCount; 
				context.ui_cache = CachedControlUI; 
				context.load_count = NeedLoadItemCount; 
				//context.flags = LOG_FILTER_DECENDING; 

				RealBeginIndex = 0; 

				ret = load_action_log( RealBeginIndex, MAX_LOG_LOAD_COUNT, &context ); 
				if( context.loaded_count != NeedLoadItemCount )
				{
					log_trace( ( MSG_ERROR, "load the event log to ui is not completely successful" ) ); 
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

#ifdef _UI_DEBUG
				ItemCacheCount += NeedLoadItemCount; 
#else
				ItemCacheCount += context.loaded_count; 
#endif //_UI_DEBUG
				if( ItemCacheCount > MaxItemCacheCount )
				{
					ItemCacheCount = MaxItemCacheCount; 
				}

				if( ItemCacheCount == 0 )
				{
					ItemCacheBeginIndex = 0; 
				}
				else
				{
#ifdef _UI_DEBUG
					ItemCacheBeginIndex = FilteredBeginIndex - NeedLoadItemCount; 
#else
					ItemCacheBeginIndex = FilteredBeginIndex - context.loaded_count; 
#endif //_DEBUG

					//#ifdef _DEBUG
					if( ItemCacheBeginIndex < 0 )
					{
						DBG_BP(); 
						ItemCacheBeginIndex = 0; 
					}
					//#endif //_DEBUG
				}

#ifdef _UI_DEBUG
				ItemRingBegin -= NeedLoadItemCount; 
#else
				ItemRingBegin -= context.loaded_count;
#endif //_UI_DEBUG

				if( ItemRingBegin < 0 )
				{
					ItemRingBegin = ( MaxItemCacheCount + ItemRingBegin ); 
				}

				ASSERT( ItemRingBegin >= 0 ); 

#ifdef _UI_DEBUG
				_ItemLoaded += NeedLoadItemCount; 
#else
				_ItemLoaded += context.loaded_count; 
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

#ifndef _EVENT_CACHE_BY_MEMORY
				{
					ULONG tick_begin; 
					ULONG tick_end; 

					tick_begin = GetTickCount(); 
					if( NeedLoadItemCount > MaxItemCacheCount )
					{
						ret = FilteredIndex2RealIndex( RealIndexEnd - MaxItemCacheCount, &RealBeginIndex ); 

						if( ret != ERROR_SUCCESS )
						{
							break; 
						}
					}
					else
					{
						ret = FilteredIndex2RealIndex( RealIndexEnd - NeedLoadItemCount, &RealBeginIndex ); 

						if( ret != ERROR_SUCCESS )
						{
							break; 
						}
					}

					tick_end = GetTickCount(); 

					log_trace( ( MSG_IMPORTANT, "filtering the logs elapse %u\n", tick_end - tick_begin ) ); 

				}
#endif //_EVENT_CACHE_BY_MEMORY

				if( NeedLoadItemCount > MaxItemCacheCount )
				{
					NeedLoadItemCount = MaxItemCacheCount; 
					ItemCacheCount = 0; 
					FilteredBeginIndex = RealIndexEnd - MaxItemCacheCount; 
				
					/**********************************************************************
					NOTICE:
					不一定第1个加载成功的记录就是第1条记录，如果这样看，
					ItemCacheBeginIndex = RealIndexEnd - context.loaded_count 可能更正确
					
					但在正常情况下，也不可以保证第1个记录可以加载成功，所以这种错误是与
					ItemCacheBeginIndex的具体值无关的。
					**********************************************************************/
				}
				else
				{
					FilteredBeginIndex = ItemCacheBeginIndex; 
				}

				//else if( ( NeedLoadItemCount + ItemCacheCount ) > MaxItemCacheCount )
				//{
				//	ItemCacheBeginIndex = RealIndexEnd - MaxItemCacheCount; 
				//	ItemCacheCount = MaxItemCacheCount - NeedLoadItemCount; 
				//}

#ifndef _UI_DEBUG
				context.manager = m_pManager; 
				context.loaded_count = 0; 
				context.filtered_index = RealIndexEnd - NeedLoadItemCount; 
				context.ring_load_index = ItemRingBegin + ItemCacheCount; 
				//context.flags = 
				
				if( context.ring_load_index < 0 )
				{
					context.ring_load_index = MaxItemCacheCount - context.ring_load_index; 
				}
				else if( (LONG)context.ring_load_index > MaxItemCacheCount )
				{
					context.ring_load_index = context.ring_load_index - MaxItemCacheCount; 
				}

				context.max_count = MaxItemCacheCount; 
				context.ui_cache = CachedControlUI; 
				context.load_count = NeedLoadItemCount; 
				//context.item_real_index = RealIndexEnd - NeedLoadItemCount; 

				RealBeginIndex = 0; 
				ret = load_action_log( RealBeginIndex, MAX_LOG_LOAD_COUNT, &context ); 

				if( context.loaded_count != NeedLoadItemCount )
				{
					log_trace( ( MSG_ERROR, "load the event log to ui is not completely successful\n" ) ); 
				}
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

#ifdef _UI_DEBUG
				ItemCacheCount += NeedLoadItemCount;
#else
				ItemCacheCount += context.loaded_count; 
#endif //_UI_DEBUG

				if( ItemRingBegin + ItemCacheCount > MaxItemCacheCount )
				{
					ItemRingBegin = ( ( ItemRingBegin + ItemCacheCount ) % MaxItemCacheCount ); 
				}

				if( ItemCacheCount > MaxItemCacheCount )
				{
					//DBG_BP(); 
					//ItemRingBegin += ( ItemCacheCount - MaxItemCacheCount ); 
					
					/******************************************************************
					如果加载记录出现错误情况的话，这种计算方法是不正确的。
					******************************************************************/
					ItemCacheBeginIndex = FilteredBeginIndex + ( ItemCacheCount - MaxItemCacheCount ); 
					ItemCacheCount = MaxItemCacheCount; 
				}
				else if( ItemCacheCount == 0 )
				{
					ItemCacheBeginIndex = 0; 
				}
				else
				{
					ItemCacheBeginIndex = FilteredBeginIndex; 
				}

				//if( ( ItemRingBegin + ItemCacheCount ) > MaxItemCacheCount )
				//{
				//	DBG_BP(); 
				//	
				//}

#ifdef _UI_DEBUG
				_ItemLoaded += NeedLoadItemCount; 
#else
				_ItemLoaded += context.loaded_count; 
#endif //_UI_DEBUG
			}
		}while( FALSE ); 

#ifdef _DEBUG
		{
			LONG PrevItemIndex; 
			LONG ItemIndex; 
			LONG ItemRingIndex; 
			LONG PrevItemRingIndex; 
			LONG ItemFilteredIndex; 
			LONG i; 
			
			LONG ItemRealIndex; 
			LONG PrevItemRealIndex; 
			INT32 DebugRet = ERROR_SUCCESS; 

			CEventListItemUI *ListItem; 
			CEventListItemUI *PrevListItem; 

			//for( i = 1; i < ItemCacheCount; i++ )
			//{
			//	ListItem = ( ( CListContainerElementUI* )ControlUICache[ i ] ); 

			//	if( ListItem != NULL )
			//	{
			//		ItemIndex = ListItem->GetIndex(); 

			//		dbg_print( MSG_IMPORTANT, "item(%u) index:%u\n", i, ItemIndex ); 
			//	}
			//}

			for( i = 1; i < ItemCacheCount; i++ )
			{
				/************************************************************
				如果有加载失败的情况，会发现事件ID号与INDEX不一致的问题，
				但使用事件ID号来进行判断的前提是没有进行FILTER，
				使用这种事件ID号来标识顺序的方法不正确。
				*************************************************************/

				ItemRingIndex = GetRingItemIndex( ItemRingBegin + i, MaxItemCacheCount ); 
				PrevItemRingIndex = GetRingItemIndex( ItemRingBegin + i - 1, MaxItemCacheCount ); 

				ListItem = ( ( CEventListItemUI* )CachedControlUI[ ItemRingIndex ] ); 
				PrevListItem = ( CEventListItemUI* )CachedControlUI[ PrevItemRingIndex ]; 

				ItemIndex = ListItem->get_filtered_index(); 
				PrevItemIndex = PrevListItem->get_filtered_index(); 

				ItemRealIndex = ListItem->get_real_index(); 
				PrevItemRealIndex = PrevListItem->get_real_index(); 

				//if( ItemCacheBeginIndex + i != ListItem->GetIndex() )
				//{
				//	DebugRet = ERROR_ERRORS_ENCOUNTERED;
				//}

				if( ItemIndex != ItemCacheBeginIndex + i )
				{
					DebugRet = ERROR_ERRORS_ENCOUNTERED; 
					//DBG_BP(); 
				}

				if( PrevItemIndex + 1 != ItemIndex )
				{
					DebugRet = ERROR_ERRORS_ENCOUNTERED; 
					//DBG_BP(); 
				}

				if( ItemRealIndex <= PrevItemRealIndex )
				{
					DebugRet = ERROR_ERRORS_ENCOUNTERED; 
				}
			}

			//if( DebugRet == ERROR_ERRORS_ENCOUNTERED )
			{
				dbg_print( MSG_IMPORTANT, "items loaded result(error):\n" ); 

				for( i = 1; i < ItemCacheCount; i++ )
				{
					ListItem = ( ( CEventListItemUI* )CachedControlUI[ i ] ); 

					if( ListItem != NULL )
					{
						ItemFilteredIndex = ListItem->get_filtered_index(); 
						ItemRealIndex = ListItem->get_real_index(); 

						dbg_print( MSG_IMPORTANT, "item(%u) index:%u->%u\n", i, ItemFilteredIndex, ItemRealIndex ); 
					}
				}

				//DBG_BP(); 
			}
		}
#endif //_DEBUG

		if( ItemLoaded )
		{
			*ItemLoaded = _ItemLoaded; 
		}

	}while( FALSE ); 

#ifdef _DEBUG
		{
			ULONG elapse; 

			end_time = GetTickCount(); 

			elapse = end_time - begin_time; 

#define BASE_EVENTS_LOAD_SPEED ( ( DOUBLE )1000.0 / ( DOUBLE )1000000.0 )
#ifdef _DEBUG_PERFORMANCE
			if( elapse != 0.0 
				&& ( DOUBLE )ItemCount / ( ( DOUBLE )( elapse ) * 1000.0 ) > BASE_EVENTS_LOAD_SPEED )
			{
				__Trace( _T( "!!! TOO LONG" ) ); 

				//__asm int 3; 
			}
#endif //_DEBUG_PERFORMANCE

			__Trace( _T( "leave %s begin index:%u count:%u (elapse:%u)\n" ), __TFUNCTION__, ItemBeginIndex, ItemCount, elapse ); 
		}
#endif //_DEBUG

	return ret; 
}

CListBodyUI* CListLogUI::GetList() const
{
	DBG_BP(); 
	return NULL; 
}

//long CListBodyExUI::LoadItemCache( ULONG ItemBegin )
//{
//	return 0; 
//}

CControlUI* CListBodyExUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *pResult = NULL;
	// Check if this guy is valid
	if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) 
	{
		return NULL;
	}

	if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) 
	{
		return NULL;
	}

	if( (uFlags & UIFIND_HITTEST) != 0 ) 
	{
		if( !::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData))) ) 
		{
			return NULL;
		}

		if( !m_bMouseChildEnabled )
		{
			CControlUI* pResult = NULL;
			if( m_pVerticalScrollBar != NULL ) 
			{
				pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
			}
			
			if( pResult == NULL && m_pHorizontalScrollBar != NULL ) 
			{
				pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
			}
			
			if( pResult == NULL ) 
			{
				pResult = CControlUI::FindControl(Proc, pData, uFlags);
			}
			
			return pResult;
		}
	}

	pResult = NULL;
	if( m_pVerticalScrollBar != NULL ) 
	{
		pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
	}
	
	if( pResult == NULL && m_pHorizontalScrollBar != NULL ) 
	{
		pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
	}
	
	if( pResult != NULL ) 
	{
		return pResult;
	}
	
	if( (uFlags & UIFIND_ME_FIRST) != 0 ) 
	{
		CControlUI* pControl = CControlUI::FindControl(Proc, pData, uFlags);
		if( pControl != NULL ) 
		{
			return pControl;
		}
	}

	RECT rc = m_rcItem;
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;

	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
	{
		rc.right -= m_pVerticalScrollBar->GetFixedWidth();
	}

	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
	{
		rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
	}

	register INT32 ItemCount; 
	register CControlUI *pSubContainer; 

	ItemCount = GetItemShowCount(); 

	if( (uFlags & UIFIND_TOP_FIRST) != 0 ) {
		for( int it = ItemCount - 1; it >= 0; it-- ) 
		{
			ret = GetDisplayItemAt( it, &pSubContainer ); 
			if( ret != ERROR_SUCCESS )
			{
				__Trace( _T("why sub control %u is null??" ), it ); 
				continue; 
			}

			ASSERT( pSubContainer != NULL ); 

			CControlUI* pControl = pSubContainer->FindControl(Proc, pData, uFlags);
			if( pControl != NULL )
			{
				if( (uFlags & UIFIND_HITTEST) != 0 && !pControl->IsFloat() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))) )
				{
					continue;
				}
				else 
				{
					return pControl;
				}
			}            
		}
	}
	else {
		for( int it = 0; it < ItemCount; it++ ) 
		{
			ret = GetDisplayItemAt( it, &pSubContainer ); 
			if( ret != ERROR_SUCCESS )
			{
				__Trace( _T("why sub control %u is null??" ), it ); 
				continue; 
			}

			ASSERT( pSubContainer != NULL ); 

			CControlUI* pControl = pSubContainer->FindControl(Proc, pData, uFlags);
			if( pControl != NULL ) {
				if( (uFlags & UIFIND_HITTEST) != 0 && !pControl->IsFloat() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))) )
					continue;
				else 
					return pControl;
			} 
		}
	}

	if( pResult == NULL ) 
	{
		pResult = CControlUI::FindControl(Proc, pData, uFlags);
	}

	return pResult;
}

/***********************************************************************************
这种方式的效率不是最高的:
因为使用通知已经加入的事件条数的方法会产生:
1.两次事件读取，一次从内核，一次数据存储
2.两次事件过滤，刚从内核中读取时，收到条数通知时。

通过实际测试，发现时间差很大。

***********************************************************************************/

/***************************************************************************************
精确重绘的实现方法:
1.精确调用InvalidateRect WINDOWS API函数
2.控制具有NeedPaint属性,精确对其进行设置

具体配置重绘点:
1.初始化界面
2.重置界面
3.滚动

绘制的具体过程:
1.区域对应的数据发生变化
2.区域被挡后恢复

1.第1种情况绘制整体区域位图
2.第2种情况将整体位图绘制至窗体DC
***************************************************************************************/
LRESULT CListBodyExUI::InvalidateListItem( LONG BeginIndex, LONG EndIndex )
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG _BeginIndex; 
	ULONG CachedItemIndex; 
	LONG i; 

	do 
	{
		ASSERT( BeginIndex <= EndIndex ); 

		if( BeginIndex == EndIndex )
		{
			break; 
		}

		if( EndIndex < ItemDisplayBegin 
			|| BeginIndex > ItemDisplayBegin + ItemDisplayCount )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( BeginIndex < ItemDisplayBegin )
		{
			_BeginIndex = ItemDisplayBegin; 
		} 
		else
		{
			_BeginIndex = BeginIndex; 
		}

		for( i = _BeginIndex; i < EndIndex; i ++ )
		{
			CachedItemIndex = GetRingItemIndex( i, MaxItemCacheCount ); 

			if( NULL != CachedControlUI[ CachedItemIndex ] )
			{
				CachedControlUI[ CachedItemIndex ]->NeedUpdate(); 
			}
			else
			{
				ASSERT( FALSE ); 
			}
		}

		NeedUpdate(); 
	}while( FALSE );

	return ret; 
}

LRESULT CListBodyExUI::OnItemAdded( LONG log_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ItemAddedIndex; 
	//SIZE ScrollRange; 
	LONG ItemLoaded; 
	//LONG OriginalItemCount; 
	/***************************************************
	1.if item for display is not enough, then fill this item.
	2.set scroll size.
	***************************************************/

	do 
	{
		//OriginalItemCount = ItemRealCount; 

		//i can add the remain items for display.
		//this index is the index number of the item
		ItemAddedIndex = ( ItemCacheBeginIndex + ItemCacheCount ); 

		if( log_count == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ItemRealCount += log_count; 
//#ifdef _DEBUG
//		if( ItemRealCount != all_filtered_actions.size() )
//		{
//			__asm int 3; 
//		}
//#endif //_DEBUG

		ItemRealCount = ( LONG )all_filtered_actions.size(); 

		if( ItemCacheCount >= MaxItemCacheCount )
		{
			//ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ItemDisplayHeight == 0 )
		{
			ASSERT( FALSE && "display height is 0"); 
			__Trace( _T( "load the log to ui when item height is not set\n" ) ); 
			ret = ERROR_NOT_READY; 
			break; 
		}

		ASSERT( ( ItemCacheBeginIndex + ItemCacheCount + log_count ) <= ItemRealCount ); 

		ret = FillSubItemsContent( ItemAddedIndex, log_count, &ItemLoaded ); 
		if( ret != ERROR_SUCCESS || ItemLoaded < 1 )
		{
			__Trace( _T( "load the log (index:%u) that's added recently error\n" ), ItemAddedIndex ); 
		}

	}while( FALSE );


	//ScrollRange = GetScrollRange(); 
	//ScrollRange.cy += ItemDisplayHeight; 

	//SetScrollRange( ScrollRange ); 

	if( ItemRealCount > ItemDisplayCount 
		&& m_pVerticalScrollBar != NULL )
	{
		INT32 OldYRange; 
		INT32 YRange;  
		YRange = ItemRealCount * ItemDisplayHeight; 

		OldYRange = m_pVerticalScrollBar->GetScrollRange(); 
		m_pVerticalScrollBar->SetScrollRange( YRange ); 

		////if( abs( OldYRange - YRange ) > LIST_ITEM_NORMAL_HEIGHT * ItemDisplayCount )
		if( m_pVerticalScrollBar->IsUpdateNeeded() == FALSE )
		{
			m_pVerticalScrollBar->NeedUpdate(); 
		}

		//NeedUpdate(); 

		//if( m_pManager != NULL )
		//{
		//	m_pManager->NeedUpdate(); 
		//}
	}

	do 
	{
		if( log_count == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = InvalidateListItem( ItemAddedIndex, ItemAddedIndex + log_count ); 
	}while( FALSE );

	//if( ItemDisplayOnReset != ALL_DISPLAY_ITEM_PAINTED )
	//{
	//	if( ItemRealCount > 0 )
	//	{
	//		NeedUpdate(); 

	//		if( m_pManager != NULL )
	//		{
	//			m_pManager->NeedUpdate(); 
	//		}
	//	}

	//	if( ItemDisplayOnReset == ALL_DISPLAY_ITEM_PAINTING )
	//	{
	//		 ItemDisplayOnReset = ALL_DISPLAY_ITEM_PAINTED; 
	//	}

	//	if( ItemRealCount >= ItemDisplayCount )
	//	{
	//		ItemDisplayOnReset = ALL_DISPLAY_ITEM_PAINTING; 
	//	}
	//}

	do 
	{
		if( m_bAutoScroll == TRUE )
		{
			LONG DisplayCount; 
			SIZE ItemSize; 
			SIZE sz = GetScrollPos(); 

			if( log_count == 0 )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			ret = GetItemDisplaySize( &ItemSize ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			DisplayCount = GetItemShowCount(); 
			sz.cy = GetScrollRange().cy; 

			//auto scroll: must adjust the display index.if position is end, display index is not changed.that's a bug.
#define AUTO_SCROLL_ITEM_COUNT ( 10 )
			sz.cy -= ( DisplayCount * ItemSize.cy ); //( LIST_ITEM_NORMAL_HEIGHT * DisplayCount ); 

			//sz.cy = ( all_filtered_actions.size() - DisplayCount ) * ItemSize.cy; 

			SetScrollPos( sz ); 
		}
	}while( FALSE ); 
	//CCachedItemManager::OnItemAdded(); 

	return 0; 
}

//LRESULT CListBodyExUI::GetCacheItemLock( LONG CacheItemIndex, ITEM_CACHE_LOCK **LockOut )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	ULONG LockIndex; 
//
//	do 
//	{
//		ASSERT( CacheItemIndex >= 0 ); 
//		ASSERT( LockOut != NULL ); 
//		ASSERT( CacheItemIndex < MaxItemCacheCount ); 
//
//		LockIndex = ( CacheItemIndex % DEFAULT_CACHE_LOCK_COUNT ); 
//
//		*LockOut = &CacheLocks[ LockIndex ]; 
//
//	}while( FALSE );
//
//	return ret; 
//}

//LRESULT CListBodyExUI::InitCacheLocks()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	INT32 i; 
//
//	do 
//	{
//		if( CacheLockInited == TRUE )
//		{
//			ret = ERROR_ALREADY_EXISTS; 
//			break; 
//		}
//
//		CacheLocks = ( ITEM_CACHE_LOCK* )malloc( DEFAULT_CACHE_LOCK_COUNT * sizeof( CRITICAL_SECTION ) ); 
//		if( CacheLocks == NULL )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			break; 
//		}
//
//		for( i = 0; i < DEFAULT_CACHE_LOCK_COUNT; i ++ )
//		{
//			init_cs_lock( CacheLocks[ i ] ); 
//		}
//
//		CacheLockInited = TRUE; 
//	}while( FALSE );
//
//	return ret; 
//}

//LRESULT CListBodyExUI::UninitCacheLocks()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	INT32 i; 
//
//	do 
//	{
//		if( CacheLockInited == FALSE )
//		{
//			ret = ERROR_NOT_READY; 
//			log_trace( ( MSG_FATAL_ERROR, "why destroy all cache locks when the lock is not initialized\n" ) ); 
//			break; 
//		}
//
//		for( i = 0; i < DEFAULT_CACHE_LOCK_COUNT; i ++ )
//		{
//			uninit_cs_lock( CacheLocks[ i ] ); 
//		}
//
//		CacheLockInited = TRUE; 
//
//	}while( FALSE );
//
//	return ret; 
//}

LRESULT CListBodyExUI::UninitItemCache()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = UninitItemCacheWorker(); 
	if( ret != ERROR_SUCCESS )
	{

	}

	ret = CCachedItemManager::UninitItemCache(); 
	//if( CacheLockInited == TRUE )
	//{
	//	UninitCacheLocks(); 
	//}

	return ret; 
}

CListBodyExUI::CListBodyExUI(CListExUI* pOwner) 
{
	//ModalCtrl = new CControlUI(); 

	//if( ModalCtrl != NULL )
	//{
	//	ModalCtrl->SetFixedHeight( LIST_ITEM_NORMAL_HEIGHT ); 
	//	ModalCtrl->SetVisible( false ); 
	//}

	m_pOwner = pOwner; 
	ItemDisplayOnReset = 0; 
	m_bAutoScroll = FALSE; 
	//ListItemUpdateCount = 0; 

	//CacheLockInited = FALSE; 
}

LRESULT CListBodyExUI::InitItemCache()
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 
	//ULONG list_height; 
	//ULONG item_height; 
	BOOLEAN CacheBufAlloced = FALSE; 
	BOOLEAN CacheItemsInited = FALSE; 
	BOOLEAN CacheWorkerInited = FALSE; 
	//BOOLEAN CacheLocksInited = FALSE; 
	LONG i; 

	do 
	{
		m_bMouseChildEnabled = true; 

		//SIZE Size; 

#ifdef _UI_DEBUG
		SetItemDisplaySize( 220, LIST_ITEM_NORMAL_HEIGHT ); 
#else
		SetItemDisplaySize( LIST_ROW_WIDTH, LIST_ITEM_NORMAL_HEIGHT ); 
#endif //_UI_DEBUG

		//ULONG list_height; 
		//ULONG item_height; 
//#ifdef _UI_DEBUG
//		ctrl = m_pManager->FindControl( _T( "friends" ) ); 
//#else
//		ctrl = m_pManager->FindControl( _T( "event_list_layout" ) ); 
//#endif //_UI_DEBUG
//
//		list_height = ctrl->GetFixedHeight(); 
//
//		if( list_height < 350 )
//		{
//			list_height = 350; 
//		}
//
//		item_height = LIST_ITEM_NORMAL_HEIGHT; 
//
//		ItemDisplayCount = list_height / item_height; 
//		if( ItemDisplayCount == 0 )
//		{
//			DBG_BP(); 
//			ret = ERROR_ERRORS_ENCOUNTERED; 
//			break; 
//		}
//
#define DEFAULT_ITEM_ONE_PAGE ( LONG )( 200 / 16 )
		ItemCacheCount = 0; 
		MaxItemCacheCount = CACHE_ITEM_PAGE_COUNT * DEFAULT_ITEM_ONE_PAGE; 

		//ret = InitCacheLocks(); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		//CacheLocksInited = TRUE; 

		CachedControlUI = ( CControlUI** )malloc( sizeof( CEventListItemUI* ) * MaxItemCacheCount ); 
		if( CachedControlUI == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		CacheBufAlloced = TRUE; 

#ifdef _DEBUG
		memset( CachedControlUI, 0, sizeof( *CachedControlUI ) * MaxItemCacheCount ); 
#endif //_DEBUG

		for( i = 0; i < MaxItemCacheCount; i ++ )
		{
			ret = AllocOneItemCache( &DlgBuilder, &ctrl ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ASSERT( ctrl != NULL ); 

			//ret = GetCacheItemLock( i, &ControlUICache[ i ].lock ); 
			//if( ret != ERROR_SUCCESS )
			//{
			//	break; 
			//}

			CachedControlUI[ i ] = ctrl; 
		}

		if( i < MaxItemCacheCount )
		{
			ASSERT( ret != ERROR_SUCCESS ); 

			for( ; ( INT32 )i >= 0; i -- )
			{
				ASSERT( CachedControlUI[ i ] != NULL ); 
				delete CachedControlUI[ i ]; 
			}

			MaxItemCacheCount = 0; 

			//free( ControlUICache ); 
			//ControlUICache = NULL; 

			break; 
		}

		CacheItemsInited = TRUE; 

		ret = InitItemCacheWorker(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_FATAL_ERROR, "initialize cache worker error %u\n" ) ); 
			break; 
		}

		CacheWorkerInited = TRUE; 

	}while( FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		if( CacheItemsInited == TRUE )
		{
			ASSERT( CacheBufAlloced != FALSE ); 
			ASSERT( MaxItemCacheCount > 0 ); 

			for( i = 0; i < MaxItemCacheCount; i ++ )
			{
				if( NULL != CachedControlUI[ i ] )
				{
					delete CachedControlUI[ i ]; 
					CachedControlUI[ i ] = NULL ;
				}
			}
		}

		if( CacheBufAlloced == TRUE )
		{
			free( CachedControlUI ); 
		}

		if( CacheWorkerInited == TRUE )
		{
			UninitItemCacheWorker(); 
		}

		//if( CacheLocksInited == TRUE )
		//{
		//	UninitCacheLocks(); 
		//}
	}
	return ret; 
}

#pragma region add_node
//LRESULT CListLogUI::AddNode( sys_action_info_notify *action, ACTION_INFO_CONTEXT *InfoConext )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	LRESULT _ret; 
//	TCHAR szBuf[ ITEM_TEXT_BUF_LEN ]; // = {0}; 
//	WCHAR file_dos_path[ MAX_PATH ]; 
//	ULONG ccb_ret_len; 
//
//	CListContainerElementExUI* pListElement = NULL;
//
//	do 
//	{
//		if( !m_dlgBuilder.GetMarkup()->IsValid() ) {
//#define ACTION_LIST_ITEM_XML_FILE _T( "action_list_item.xml" ) 
//
//			pListElement = static_cast<CListContainerElementExUI*>(m_dlgBuilder.Create(ACTION_LIST_ITEM_XML_FILE, (UINT)0, this, m_pManager));
//		}
//		else {
//			pListElement = static_cast<CListContainerElementExUI*>(m_dlgBuilder.Create( this, m_pManager));
//		}
//
//		if( pListElement == NULL )
//		{
//			ret = ERROR_ERRORS_ENCOUNTERED; 
//			break; 
//		}
//
//		pListElement->SetVisible( true ); 
//
//		//CRect rcPadding = text_padding; 
//
//		//pListElement->SetPadding( rcPadding );
//
//		CContainerUI* main_layout = static_cast< CContainerUI* >( m_pManager->FindSubControlByName( pListElement, EventInfoLayoutContainerName ) ); 
//
//		if( main_layout != NULL )
//		{
//			main_layout->SetMaxWidth( LIST_ROW_WIDTH ); 
//			main_layout->SetFixedWidth( LIST_ROW_WIDTH ); 
//			main_layout->SetVisible( true );
//		}
//
//		file_dos_path[ ARRAYSIZE( file_dos_path ) - 1 ] = L'\0'; 
//
//		CListSubElementExUI *proc_icon_label = static_cast< CListSubElementExUI* >( m_pManager->FindSubControlByName( pListElement, ProcIconLabelName ) ); 
//
//		if( proc_icon_label != NULL)
//		{
//			HRESULT hr; 
//
//			action->action.ctx.proc_name[ ARRAYSIZE( action->action.ctx.proc_name ) - 1 ] = L'\0'; 
//
//			_ret = convert_native_path_to_dos( action->action.ctx.proc_name, 
//				action->action.ctx.proc_name_len, 
//				file_dos_path, 
//				ARRAYSIZE( file_dos_path ), 
//				&ccb_ret_len ); 
//
//			if( _ret != ERROR_SUCCESS )
//			{
//				*file_dos_path = L'\0'; 
//			}
//			else
//			{
//				hr = StringCbPrintfW( szBuf, sizeof( szBuf ), L"filepath=\'%s\' source=\'0,0,32,32\' mask=\'0xffff00ff\'", file_dos_path ); 
//
//				if( SUCCEEDED( hr ) )
//				{
//					proc_icon_label->SetBkImage( szBuf ); 
//					proc_icon_label->SetTag( ( UINT_PTR )pListElement );
//					proc_icon_label->OnEvent += MakeDelegate( &OnEventInfoLabelEvent ); 
//					proc_icon_label->SetToolTip( szBuf ); 
//				}
//				//{
//				//	proc_icon_label->set_x_locate_ctrl( list_header_items[ 0 ] ); 
//				//}
//			}
//		}
//		else
//		{
//			ASSERT( FALSE ); 
//			//break; 
//		}
//
//		//tString html_text; 
//		//html_text = level_expand_image;
//
//		//#if defined(UNDER_WINCE)
//		//		_stprintf(szBuf, _T("<x %d>"), level_text_start_pos);
//		//#else
//		//		_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x %d>"), level_text_start_pos);
//		//#endif
//
//		CListSubElementExUI* time_label = static_cast< CListSubElementExUI* >( m_pManager->FindSubControlByName( pListElement, TimeLableName ) ); 
//
//		if( time_label != NULL)
//		{
//			sprint_time( szBuf, 
//				ITEM_TEXT_BUF_LEN, 
//				action->action.ctx.time ); 
//
//			//time_label->SetShowHtml(true);
//			time_label->SetText( szBuf ); 
//			time_label->SetToolTip( szBuf ); 
//			//{
//			//	time_label->set_x_locate_ctrl( list_header_items[ 1 ] ); 
//			//}
//		}
//		else
//		{
//			ASSERT( FALSE ); 
//		}
//
//		CListSubElementExUI* proc_id_label = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName(pListElement, ProcIdLabelName ) );
//
//		if( proc_id_label != NULL )
//		{
//#if defined(UNDER_WINCE)
//			_stprintf(szBuf, _T("<x 20><c #808080>%u</c>"), action->action.ctx.proc_id ); 
//#else
//			_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x 20>%u"), action->action.ctx.proc_id );
//#endif
//			proc_id_label->SetShowHtml( true );
//			proc_id_label->SetText( szBuf ); 
//			proc_id_label->SetToolTip( szBuf + CONST_STR_LEN( "<x 20>" ) ); 
//
//			//{
//			//	proc_id_label->set_x_locate_ctrl( list_header_items[ 2 ] ); 
//			//}
//		}
//
//		CListSubElementExUI* thread_id_label = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName(pListElement, ThreadIdLabelName ) );
//
//		if( thread_id_label != NULL )
//		{
//#if defined(UNDER_WINCE)
//			_stprintf(szBuf, _T("<x 20><c #808080>%u</c>"), action->action.ctx.thread_id ); 
//#else
//			_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x 20>%u"), action->action.ctx.thread_id );
//#endif
//			thread_id_label->SetShowHtml( true );
//			thread_id_label->SetText( szBuf ); 
//			thread_id_label->SetToolTip( szBuf + CONST_STR_LEN( "<x 20>" ) ); 
//			//{
//			//	thread_id_label->set_x_locate_ctrl( list_header_items[ 3 ] ); 
//			//}
//		}
//
//		CListSubElementExUI* exe_file_label = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName(pListElement, ExeFileNameLabelName ) );
//
//		if( exe_file_label != NULL )
//		{
//			action->action.ctx.proc_name[ ARRAYSIZE( action->action.ctx.proc_name ) - 1 ] = L'\0'; 
//#if defined(UNDER_WINCE)
//			_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), file_dos_path ); 
//#else
//			_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x 20>%s"), file_dos_path );
//#endif
//			exe_file_label->SetShowHtml( true );
//			exe_file_label->SetText( szBuf ); 
//			exe_file_label->SetToolTip( szBuf + CONST_STR_LEN( "<x 20>" ) ); 
//			//{
//			//	exe_file_label->set_x_locate_ctrl( list_header_items[ 4 ] ); 
//			//}
//		}
//
//		CListSubElementExUI* event_desc_label = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName(pListElement, EventDescLableName ) );
//
//		if( event_desc_label != NULL )
//		{
//			_ret = get_action_tip( &action->action.action, 
//				&action->action.ctx, 
//				szBuf, 
//				ITEM_TEXT_BUF_LEN, 
//				0 ); 
//
//			if( _ret != 0 )
//			{
//				*szBuf = L'\0'; 
//			}
//
//			//#if defined(UNDER_WINCE)
//			//		_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), L"event_desc" ); 
//			//#else
//			//		_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x 20><c #808080>%s</c>"), L"event_desc" );
//			//#endif
//			//		event_desc_label->SetShowHtml( true );
//			event_desc_label->SetText( szBuf ); 
//			event_desc_label->SetToolTip( szBuf ); 
//			//{
//			//	event_desc_label->set_x_locate_ctrl( list_header_items[ 5 ] ); 
//			//}
//		}
//
//		CListSubElementExUI* data_label = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName(pListElement, DataLabelName ) );
//
//		if( data_label != NULL )
//		{
//			_ret = get_key_data_text( szBuf, ITEM_TEXT_BUF_LEN, &action->append, action->data_len ); 
//
//			if( _ret != ERROR_SUCCESS )
//			{
//				szBuf[ ITEM_TEXT_BUF_LEN - 1 ] = L'\0'; 
//			}
//
//			//#if defined(UNDER_WINCE)
//			//		_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), L"123456789" ); 
//			//#else
//			//		_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x 20><c #808080>%s</c>"), L"123456789" );
//			//#endif
//			//data_label->SetShowHtml( true );
//			data_label->SetText( szBuf ); 
//			data_label->SetToolTip( szBuf ); 
//
//			//{
//			//	data_label->set_x_locate_ctrl( list_header_items[ 6 ] ); 
//			//}
//		}
//
//		CListSubElementExUI* call_stack_label = static_cast<CListSubElementExUI*>(m_pManager->FindSubControlByName(pListElement, CallStackLabelName ) );
//
//		if( call_stack_label != NULL )
//		{
//			ULONG ccb_ret_len; 
//			_ret = get_stack_trace_dump_text( action->action.ctx.proc_id, 
//				action->stack_frame, 
//				action->frame_count, 
//				( LPSTR )InfoConext->sym_path, 
//				szBuf, 
//				ITEM_TEXT_BUF_LEN, 
//				&ccb_ret_len ); 
//
//			//unlock_cs( action_log_lock ); 
//
//			if( _ret != ERROR_SUCCESS )
//			{
//				szBuf[ 0 ] = L'\0'; 
//			}
//			else
//			{
//				szBuf[ ITEM_TEXT_BUF_LEN - 1 ] = L'\0'; 
//			}
//
//			//#if defined(UNDER_WINCE)
//			//		_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), L"call_stack_label" ); 
//			//#else
//			//		_stprintf_s(szBuf, ARRAYSIZE( szBuf ) - 1, _T("<x 20><c #808080>%s</c>"), L"call_stack_label" );
//			//#endif
//			//		call_stack_label->SetShowHtml( true );
//			call_stack_label->SetText( szBuf ); 
//			call_stack_label->SetToolTip( szBuf ); 
//
//			//{
//			//	call_stack_label->set_x_locate_ctrl( list_header_items[ 7 ] ); 
//			//}
//		}
//
//		pListElement->SetFixedHeight( EventInfoListItemNormalHeight );
//		//pListElement->SetTag( ( UINT_PTR )node ); 
//
//		if( !CListExUI::AddSubItem( pListElement ) )
//		{
//			delete pListElement; 
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//		}
//	}while( FALSE );
//
//	return ret;
//}

//void CListLogUI::RemoveNode(ListNode* node)
//{
//	if( !node ) return;
//
//	CListUI::Remove(node->data().list_element); 
//
//	delete node->data().list_element;
//	delete node;
//}

//void CListLogUI::SetChildVisible(ListNode* node, bool visible)
//{
//	if (!node || node == RootNode)
//		return;
//
//	if (node->data().child_visible == visible)
//		return;
//
//	node->data().child_visible = visible;
//
//	TCHAR szBuf[MAX_PATH] = {0};
//	tString html_text;
//	if (node->data().has_child)
//	{
//		if (node->data().child_visible)
//			html_text += level_expand_image;
//		else
//			html_text += level_collapse_image;
//
//#if defined(UNDER_WINCE)
//		_stprintf(szBuf, _T("<x %d>"), level_text_start_pos);
//#else
//		_stprintf_s(szBuf, MAX_PATH - 1, _T("<x %d>"), level_text_start_pos);
//#endif
//		html_text += szBuf;
//
//		html_text += node->data().text;
//
//		CLabelUI* nick_name = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(node->data().list_element, kNickNameControlName));
//		if (nick_name != NULL)
//		{
//			nick_name->SetShowHtml(true);
//			nick_name->SetText(html_text.c_str());
//		}
//	}
//
//	if (!node->data().list_element->IsVisible())
//		return;
//
//	if (!node->has_children())
//		return;
//
//	ListNode* begin = node->child(0);
//	ListNode* end = node->get_last_child();
//	for (int i = begin->data().list_element->GetIndex(); i <= end->data().list_element->GetIndex(); ++i)
//	{
//		CControlUI* control = GetItemAt(i);
//		if (_tcsicmp(control->GetClass(), _T("ListContainerElementUI")) == 0)
//		{
//			if (visible) 
//			{
//				ListNode* local_parent = ((ListNode*)control->GetTag())->parent();
//				if (local_parent->data().child_visible && local_parent->data().list_element->IsVisible())
//				{
//					control->SetVisible(true);
//				}
//			}
//			else
//			{
//				control->SetVisible(false);
//			}
//		}
//	}
//}
//
//bool CListLogUI::CanExpand(ListNode* node) const
//{
//	if (!node || node == RootNode)
//		return false;
//
//	return node->data().has_child;
//}

#pragma endregion add_node

CControlUI *CListLogUI::GetCurSelItem()
{
	CControlUI *pControl = NULL; 
	IListItemUI *pListItem; 

	do 
	{
		if( m_iCurSel < 0 )
		{
			break; 
		}
	
		ASSERT( m_CurSelCacheIndex >= 0 ); 

		if( m_CurSelCacheIndex >= m_pList->MaxItemCacheCount )
		{
			ASSERT( FALSE ); 
			break; 
		}

		pControl = m_pList->CachedControlUI[ m_CurSelCacheIndex ];

		if( pControl == NULL) 
		{
			break; 
		}

		pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
		if( pListItem != NULL ) 
		{
			if( pListItem->GetIndex() != m_iCurSel )
			{
				pListItem = NULL; 
				DBG_BP(); 
				break; 
			}
		}
		else
		{
			DBG_BP(); 
		}

	}while( FALSE ); 

	return pControl; 
}

//bool CListContainerElementExUI::SetSelect(bool bSelect)
//{
//	if( !IsEnabled() ) 
//	{
//		return false;
//	}
//
//	if( bSelect == m_bSelected ) 
//	{
//		return true;
//	}
//
//	m_bSelected = bSelect; 
//
//	return true; 
//}

//void CListSubElementExUI::DoEvent(TEventUI& event)
//{
//
//}

//bool CListContainerElementExUI::Select(bool bSelect)
//{
//	if( !IsEnabled() ) 
//	{
//		return false;
//	}
//
//	if( bSelect == m_bSelected ) 
//	{
//		return true;
//	}
//
//	m_bSelected = bSelect; 
//
//	if( bSelect && m_pOwner != NULL ) 
//	{
//		m_pOwner->SelectListItem( m_iIndex );
//	}
//
//	Invalidate();
//
//	return true;
//}

bool CListLogUI::GetScrollSelect()
{
	return m_bScrollSelect;
}

void CListLogUI::SetScrollSelect(bool bScrollSelect)
{
	m_bScrollSelect = bScrollSelect;
}

VOID CListLogUI::ResetItemSelectUI( PVOID Item )
{
	if( Item != NULL )
	{
		( ( CControlUI* )Item )->Invalidate(); 
	}
}

LRESULT CListLogUI::FindItemSelected( PVOID *ItemSelected )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *Control; 
	IListItemUI *ListItem; 
	BOOLEAN ItemFound = FALSE; 
	LONG i; 
	
	do 
	{
		ASSERT( ItemSelected != NULL ); 

		*ItemSelected = NULL; 

		do 
		{
			if( m_CurSelCacheIndex == INVALID_ITEM_INDEX )
			{
				ItemFound = TRUE; 
				break; 
			}

			if( m_CurSelCacheIndex >= m_pList->MaxItemCacheCount )
			{
				DBG_BP(); 
				break; 
			}

			Control = m_pList->CachedControlUI[ m_CurSelCacheIndex ];

			if( Control == NULL ) 
			{
				DBG_BP(); 
				break; 
			}

			ListItem = static_cast<IListItemUI*>( Control->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
			if( ListItem == NULL ) 
			{
				DBG_BP(); 
				break; 
			}

			if( ListItem->GetIndex() != m_iCurSel )
			{
				DBG_BP(); 
				break; 
				//多个线程同时处理日志队列，一定会发生日志队列先择项目记录不正确。
			}
			else
			{
				*ItemSelected = ( PVOID )Control; 
				ItemFound = TRUE; 
			}

		}while( FALSE ); 

		if( ItemFound == TRUE )
		{
			break; 
		}

		for( i = 0; i < m_pList->MaxItemCacheCount; i ++ )
		{
			do 
			{
				Control = m_pList->CachedControlUI[ i ]; 
				if( Control == NULL) 
				{
					DBG_BP(); 
					break; 
				}

				ListItem = static_cast<IListItemUI*>( Control->GetInterface( UI_LIST_ITEM_INTERFACE_NAME ) );
				if( ListItem == NULL ) 
				{
					DBG_BP(); 
					break; 
				}

				if( ListItem->GetIndex() != m_iCurSel )
				{
					//DBG_BP(); 
					break; 
					//多个线程同时处理日志队列，一定会发生日志队列先择项目记录不正确。
				}
				else
				{
					*ItemSelected = ( PVOID )Control; 
					ItemFound = TRUE; 
				}
			}while( FALSE );
		}

		if( ItemFound == FALSE )
		{
			ret = ERROR_NOT_FOUND; 
		}

	}while( FALSE );

	return ret; 
}

bool CListLogUI::SelectListItem(int iIndex)
{
	bool ret = true; 
	LRESULT _ret; 
	CControlUI* pControl; 
	IListItemUI* pListItem; 
	PVOID ItemSelected = NULL; 

	do 
	{
		if( iIndex == m_iCurSel )
		{
			FindItemSelected( &ItemSelected ); 
			m_iCurSel = -1; 
			ResetItemSelectUI( ItemSelected ); 

			break; 
		}

		int iOldSel = m_iCurSel;
		// We should first unselect the currently selected item

		if( m_iCurSel >= 0 )
		{
			FindItemSelected( &ItemSelected ); 
		}

		if( iIndex < 0 ) 
		{
			ret = false; 
			break; 
		}

		//CControlUI* pControl = GetSubItemAt( iIndex, UI_LIST_ELEMENT_CLASS_ID );
		//if( pControl == NULL ) 
		//{
		//	DBG_BP(); 
		//	ret = false; 
		//	break; 
		//}

		//if( ( ( CListContainerElementExUI* )pControl )->GetIndex() != iIndex )
		//{
		//	DBG_BP(); 
		//	return false; 
		//}
		do 
		{
			INT32 i; 
			LRESULT __ret; 

			_ret = ERROR_SUCCESS; 
			pControl = NULL; 

			for( i = 0; ( ULONG )i < m_pList->GetItemShowCount(); i ++ )
			{
				__ret = m_pList->GetDisplayItemAt( i, &pControl ); 
				if( __ret == ERROR_SUCCESS )
				{
					if( ( ( CListContainerElementExUI* )pControl )->GetIndex() == iIndex )
					{
						break; 
					}
				}
				else
				{
					if( __ret == ERROR_NOT_FOUND )
					{
						break; 
					}

					//ASSERT( FALSE ); 
				}
			}

			if( pControl == NULL )
			{
				_ret = ERROR_NOT_FOUND; 
				DBG_BP(); 
				break; 
			}

			if( ( ULONG )i == m_pList->GetItemShowCount() )
			{
				_ret = ERROR_NOT_FOUND; 
				DBG_BP(); 
				break; 
			}

			if( i >= m_pList->ItemCacheCount )
			{
				_ret = ERROR_NOT_FOUND; 
				DBG_BP(); 			
				break; 
			}

			//ItemDisplayBegin = 0; 

			m_CurSelCacheIndex = m_pList->GetRingIndexFromDisplayIndex( i ); 
			dbg_print( MSG_IMPORTANT, "the selected item index is %u\n", m_CurSelCacheIndex ); 

			if( m_CurSelCacheIndex == INVALID_ITEM_INDEX )
			{
				DBG_BP(); 
				_ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

		}while( FALSE );

		if( ERROR_SUCCESS != _ret )
		{
			DBG_BP(); 
			ret = false; 
			break; 
		}

		if( !pControl->IsVisible() ) 
		{
			ret = false; 
			break; 
		}

		if( !pControl->IsEnabled() ) 
		{
			ret = false;
			break; 
		}

		pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));

		if( pListItem == NULL ) 
		{
			DBG_BP(); 
			ret = false;
			break; 
		}

		if( pListItem->GetIndex() != iIndex )
		{
			DBG_BP(); 
			ret = false; 
			break; 
		}

		m_iCurSel = iIndex; 

		ResetItemSelectUI( ItemSelected ); 

		//if( !pListItem->Select(true) ) 
		//{
		//	m_iCurSel = -1; 
		//	m_CurSelCacheIndex = -1; 
		//	ret = false; 
		//	break; 
		//}

		//EnsureVisible( m_iCurSel ); 

		//pControl->SetFocus();
		//if( m_pManager != NULL ) 
		//{
		//	m_pManager->SendNotify(this, _T("itemselect"), m_iCurSel, iOldSel);
		//}

#ifdef ADJUST_LIST_ITEM_HEIGHT_ON_SELECTED
		//show stack layout if the event log item selected.

		//pControl = GetSubItemAt( m_iCurSel, UI_LIST_ELEMENT_CLASS_ID );
		ASSERT( pControl != NULL); 

		{
			CListContainerElementUI* EventInfoListItem = static_cast<CListContainerElementUI*>( pControl );

			//ListNode* node = (ListNode*)pControl->GetTag();
			if( EventInfoListItem != NULL )
			{
				EventInfoListItem->SetFixedHeight( EventInfoItemSelectedHeight );
				//CContainerUI* pOperatorPannel = static_cast<CContainerUI*>(m_pManager->FindSubControlByName(EventInfoListItem, AllStackLayoutContainerName));
				//if (pOperatorPannel != NULL)
				//{
				//	pOperatorPannel->SetVisible(true);
				//}
			}
		}
#endif //ADJUST_LIST_ITEM_HEIGHT_ON_SELECTED

	}while( FALSE ); 

	return ret;
}
//
//VOID CCachedContainerUI::SetRealItemsCount( ULONG Count )
//{
//	ItemCount = Count; 
//}
//
//VOID CCachedContainerUI::OnItemAdded()
//{
//	ItemCount += 1; 
//	NeedUpdate(); 
//
//	if( m_pManager != NULL )
//	{
//		m_pManager->NeedUpdate(); 
//	}
//}
//
//ULONG CCachedContainerUI::GetRealItemsCount()
//{
//	return ItemCount; 
//}
//
////typedef struct _LOAD_EVENT_LOG_PARAM
////{
////	CControlUI *control_cache[]; 
////};
//
//ULONG CCachedContainerUI::GetCachedItemIndex( ULONG Index )
//{
//	ULONG ItemIndex; 
//
//	do 
//	{
//		if( Index == INVALID_ITEM_INDEX )
//		{
//			ItemIndex = INVALID_ITEM_INDEX; 
//			break; 
//		}
//
//		ItemIndex = get_ring_item_index( Index, MaxItemCacheCount ); 
//	}while( FALSE );
//
//	return ItemIndex; 
//}
//

} // namespace DuiLib
