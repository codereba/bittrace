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

#pragma once 

#include "ui_ctrl.h"
#include "action_log_db.h"
#include "action_display.h"
#include "stack_dump.h"
#include "event_log_loader.h"

#define WM_ACTION_EVENT_LOG ( WM_USER + 1001 ) 

typedef struct _stack_frame_info
{
	WCHAR module_name[ MAX_PATH ]; 
	ULONG_PTR offset; 
} stack_frame_info, *pstack_frame_info; 

#define _MAX_PROC_ID_LEN 64
#define _MAX_THREAD_ID_LEN 64
#define _MAX_ACTION_NAME_LEN 64
#define _MAX_PARAM_TEXT_LEN 512
typedef struct _sys_action_log_text
{
	WCHAR proc_id[ _MAX_PROC_ID_LEN ]; 
	WCHAR proc_name[ _MAX_FILE_NAME_LEN ]; 
	WCHAR thread_id[ _MAX_THREAD_ID_LEN ]; 
	WCHAR action_name[ _MAX_ACTION_NAME_LEN ]; 
	WCHAR param1[ _MAX_PARAM_TEXT_LEN ]; 
	WCHAR param2[ _MAX_PARAM_TEXT_LEN ]; 
	WCHAR param3[ _MAX_PARAM_TEXT_LEN ]; 
	WCHAR param4[ _MAX_PARAM_TEXT_LEN ]; 
	WCHAR param5[ _MAX_PARAM_TEXT_LEN ]; 
	WCHAR param6[ _MAX_PARAM_TEXT_LEN ]; 
} sys_action_log_text, *psys_action_log_text; 


typedef r3_action_notify sys_action_log; 

#define FW_EVENT_APP _T( "应用程序" )

class WindowImplBase; 

/************************************************************
next functions:
1.window can do sizing,default size is bigger.
2.inner control can sizing by column header.
3.dumping logging speed fastest.

fastest log loading implementation:
1.level 1:data base
2.level 2:log memory cache
3.level 3:ui control property

next task:
1.hung when move slip bar.
2.drag column support
3.logging data precise.

************************************************************/
class control_builder : public IDialogBuilderCallback
{
public:
	control_builder()
	{ 
	}

public:
	CControlUI* CreateControl(LPCTSTR pstrClass, CPaintManagerUI *pManager ) 
	{
		if( _tcscmp( pstrClass, _T( "ListEx" ) ) == 0 ) 
		{
			return new CListLogUI();
		}

		return NULL; 
	} 
}; 

class trace_frame : public CHorizontalLayoutUI
{

public:
	trace_frame( CPaintManagerUI* pManager)
	{
		control_builder cb; 
		CDialogBuilder builder;
		CContainerUI* trace_ui = static_cast<CContainerUI*>(builder.Create(_T("trace_tab.xml"), NULL, &cb, pManager ));
	
		ASSERT( pManager != NULL ); 

		if( trace_ui != NULL ) 
		{
			this->AddItem(trace_ui);
		}
		else 
		{
			this->RemoveAllItem();
			return;
		}

		this->SetName( _T( "trace_tab" ) ); 

		init_cs_lock( action_log_lock ); 

	}

	~trace_frame()
	{
		uninit_cs_lock( action_log_lock ); 
	}; 

	LRESULT set_auto_scroll( BOOLEAN auto_scroll )
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
			CListLogUI *List; 

			do 
			{
				List = ( CListLogUI * )m_pManager->FindControl( L"action_log_list" ); 
				ASSERT( List != NULL ); 

				ret = List->set_auto_scroll( auto_scroll ); 
			}while( FALSE );
		}while( FALSE );
		
		return ret; 
	}

	LRESULT clear_events()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CListLogUI *List; 

		do 
		{
			List = ( CListLogUI * )m_pManager->FindControl( L"action_log_list" ); 
			ASSERT( List != NULL ); 

			ret = List->clear_events(); 
		}while( FALSE );
	
		return ret; 
	}

	LRESULT change_font()
	{
		LRESULT ret = ERROR_SUCCESS; 
		bool _ret; 
		INT32 font_size = 10; 

		do 
		{
			CListLogUI *List; 
			TFontInfo *FontInfo; 

#define EVENT_LIST_ITEM_EVENT_FONT_INDEX 12 
			FontInfo = m_pManager->GetFontInfo( EVENT_LIST_ITEM_EVENT_FONT_INDEX ); 
			_ret = m_pManager->ChangeFont( EVENT_LIST_ITEM_EVENT_FONT_INDEX, FontInfo->sFontName, 
				font_size, 
				FontInfo->bBold, 
				FontInfo->bUnderline, 
				FontInfo->bItalic ); 

			List = ( CListLogUI * )m_pManager->FindControl( L"action_log_list" ); 
			ASSERT( List != NULL ); 

			List->Invalidate(); 

		}while( FALSE );

		return ret; 
	}

	CStdString get_cur_sel_obj_path()
	{
		CStdString obj_path; 
		CListLogUI *List; 

		List = ( CListLogUI * )m_pManager->FindControl( L"action_log_list" ); 
		ASSERT( List != NULL ); 

		obj_path = List->get_cur_sel_obj_path(); 
		return obj_path; 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T( "EventTraceTab" ); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

	LRESULT WINAPI add_action_log( ULONG event_count, ULONG filtered_event_count ); 
    
	LRESULT search_text( LPCWSTR text, BOOLEAN direction )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CListLogUI *List; 

		do 
		{
			ASSERT( NULL != text ); 

			List = ( CListLogUI * )m_pManager->FindControl( L"action_log_list" ); 
			ASSERT( List != NULL ); 

			ret = List->search_by_key_word( text, direction ); 
			
		}while( FALSE ); 
	
		return ret; 
	}

	LRESULT init_action_log_list()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CListLogUI *List; 
		CControlUI *Ctrl; 

		Ctrl = ( CControlUI * )m_pManager->FindControl( L"form_layout" ); 
		ASSERT( Ctrl != NULL ); 

		Ctrl->SetFixedWidth( 0 ); 

		Ctrl = ( CControlUI * )m_pManager->FindControl( L"event_list_layout" ); 
		ASSERT( Ctrl != NULL ); 

		Ctrl->SetFixedWidth( 0 ); 

		List = ( CListLogUI * )m_pManager->FindControl( L"action_log_list" ); 
		ASSERT( List != NULL ); 
		
		ret = List->_Init(); 

		return ret; 
	}

	LRESULT hilight_by_key_word()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CListLogUI *List; 

		do 
		{
			List = ( CListLogUI * )m_pManager->FindControl( L"action_log_list" ); 
			ASSERT( List != NULL ); 

			ret = List->hilight_by_key_word(); 
		}while( FALSE );
	
		return ret; 
	}

	LRESULT FilterEventsUI() 
	{
		LRESULT ret; 
		CListLogUI *List; 

		do 
		{
			List = ( CListLogUI * )m_pManager->FindControl( L"action_log_list" ); 
			ASSERT( List != NULL ); 

			ret = List->FilterEvents(); 

		}while( FALSE ); 

		return ret; 
	}

	void _Init() 
    {
		LRESULT ret; 

		do 
		{
			ret = init_action_log_list(); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}while( FALSE );
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }


	/***********************************
	系统的模块在内核中进行维护，当有事件发生时，使用如下的结构记录调用堆栈相关的信息：
	1.模块的名称:相对偏移量
	
	模块名称集在内核中维护，并且将其直接映射至RING3层。使用加锁的方式，对其进行保护性的读写。

	打印至缓存区后，将字符串保留下来，不再进行重复的打印。

	打印事件信息方法：
	1.事件信息为一个结构体，所有事件信息长度都统一进行分配：
	 缺点：内核消耗大
	 定位,读取事件的信息参数相对非常简单

	2.使用结构解析方法对不同长度的各个事件信息参数进行定位，同时设置相应的字符串输出。
	3.使用一种统一的方法将事件的信息集中输出为一条消息。
	***********************************/
	/*
    * 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
    */
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        return 0;
    }

	LRESULT on_action_event_log(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
			ret = add_action_log( ( ULONG )wParam, ( ULONG )lParam ); 
			bHandled = TRUE; 

		}while( FALSE );
	
		return ret; 
	}

	LRESULT send_action_log_done( PVOID context )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CListLogUI *log_list; 

		do 
		{
			if( context == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			log_list = static_cast<CListLogUI*>( m_pManager->FindControl( _T( "action_log_list" ) ) );

			CListBodyExUI *ListEx = log_list->GetListEx(); 

			if( ListEx != NULL )
			{
				ret = ListEx->on_cache_item_loaded_done( context ); 
				if( ret != ERROR_SUCCESS )
				{

				}
			}
			else
			{
				ASSERT( FALSE ); 
			}
		}while( FALSE );

		return ret; 
	}

	LRESULT send_action_log( event_log_notify *event_log, PVOID work_context )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CListLogUI *log_list; 

		do 
		{
			if( event_log == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			log_list = static_cast<CListLogUI*>( m_pManager->FindControl( _T( "action_log_list" ) ) );

			CListBodyExUI *ListEx = log_list->GetListEx(); 

			if( ListEx != NULL )
			{
				ret = ListEx->on_cache_item_loaded( event_log, work_context ); 
				if( ret != ERROR_SUCCESS )
				{

				}
			}
			else
			{
				ASSERT( FALSE ); 
			}
		}while( FALSE );

		return ret; 
	}


private:

	CListLogUI *log_list; 

	CHAR sym_path[ MAX_SYMBOL_PATH ]; 

	CRITICAL_SECTION action_log_lock; 
};