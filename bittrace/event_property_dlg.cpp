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

#include "StdAfx.h"
#include "common_func.h"
#include "acl_define.h"
#include "event_store.h"
#include "event_property_dlg.h"
#include "action_display.h"
#include "action_ui_support.h"

static CDialogBuilder dlg_builder; 

LRESULT event_properies_dlg::set_event( PVOID event )
{
	this->event = event; 

	return ERROR_SUCCESS; 
}

/**********************************************************************
<HorizontalLayout >
<List name="image_list" bkcolor="#FFFFFFFF" inset="0,0,0,0" itemshowhtml="true" vscrollbar="true" hscrollbar="true" headerbkimage="file='list_header_bg.png'" itemalign="left" itemtextpadding="5,0,0,0" itembkcolor="#FFE2DDDF" itemaltbk="true" menu="true">
<ListHeader height="24" menu="true">
<ListHeaderItem text="模块"  font="10" width="100" hotimage="file='list_header_hot.png'" pushedimage="file='list_header_pushed.png'" sepimage="file='list_header_sep.png'" sepwidth="1"/>
<ListHeaderItem text="基地址" font="10" width="240" hotimage="file='list_header_hot.png'" pushedimage="file='list_header_pushed.png'" sepimage="file='list_header_sep.png'" sepwidth="1"/>
<ListHeaderItem text="大小" font="10" width="240" hotimage="file='list_header_hot.png'" pushedimage="file='list_header_pushed.png'" sepimage="file='list_header_sep.png'" sepwidth="1"/>
<ListHeaderItem text="路径" font="10" width="240" hotimage="file='list_header_hot.png'" pushedimage="file='list_header_pushed.png'" sepimage="file='list_header_sep.png'" sepwidth="1"/>
</ListHeader>
</List>
</HorizontalLayout>
**********************************************************************/
#define ITEM_TEXT_BUF_LEN 2048
LRESULT event_properies_dlg::load_event_information() 
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	CControlUI *ctrl; 
	WCHAR text[ ITEM_TEXT_BUF_LEN ]; 

	r3_action_notify_buf *action; 

	try
	{
	do 
	{
		if( event == NULL )
		{
			break; 
		}

		action = ( r3_action_notify_buf* )event; 

		sprint_local_time( text, ARRAYSIZE( text ), action->action.action.ctx.time ); 

		ctrl = m_pm.FindControl( L"time" ); 
		ASSERT( ctrl != NULL ); 

		ctrl->SetText( text ); 
		ctrl->SetToolTip( text ); 

		ctrl = m_pm.FindControl( L"thread" ); 
		ASSERT( ctrl != NULL ); 

		StringCbPrintfW( text, sizeof( text ), L"%u", action->action.action.ctx.thread_id ); 
		ctrl->SetText( text ); 

		ctrl = m_pm.FindControl( L"event_type" ); 
		ASSERT( ctrl != NULL ); 

		{
			LPWSTR _text; 
			UI_PARAM ui_param; 

			action_main_type main_type; 
			_ret = _get_action_main_type( action->action.action.action.type, &main_type ); 
			if( _ret != ERROR_SUCCESS )
			{
				DBG_BP(); 
				break; 
			}

			get_action_main_type_desc( main_type, &_text, &ui_param ); 
			ctrl->SetText( _text ); 
		}

		ctrl = m_pm.FindControl( L"event_name" ); 
		ASSERT( ctrl != NULL ); 

		ctrl->SetText( get_event_name_ex( &action->action.action.action ) ); 
		ctrl->SetToolTip( get_event_name_ex( &action->action.action.action ) ); 

		ctrl = m_pm.FindControl( L"event_source" ); 
		ASSERT( ctrl != NULL ); 

		StringCbCopyW( text, sizeof( text ), action->action.action.ctx.proc_name ); 
		ctrl->SetText( text ); 
		ctrl->SetToolTip( text ); 

		ctrl = m_pm.FindControl( L"event_obj" ); 
		ASSERT( ctrl != NULL ); 

		{
			LPCWSTR _obj_path; 
			ULONG obj_path_len; 

			_ret = get_object_path( &action->action.action.action, 
				&action->action.action.ctx, 
				text, 
				ARRAYSIZE( text ), 
				&_obj_path, 
				0, 
				&obj_path_len ); 

			if( _ret != ERROR_SUCCESS )
			{
				break; 
			}
			ctrl->SetText( _obj_path ); 
			ctrl->SetToolTip( _obj_path ); 
		}

		ctrl = m_pm.FindControl( L"duration" ); 
		ASSERT( ctrl != NULL ); 

		//计算事件的执行总时间
		ctrl->SetText( L"" ); 

		ctrl = m_pm.FindControl( L"event_properties" ); 
		ASSERT( ctrl != NULL ); 

		_ret = get_action_detail( &action->action.action.action, 
			&action->action.action.ctx, 
			text, 
			ARRAYSIZE( text ), 
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
			*text = L'\0'; 
		}

#ifdef _DEBUG
		if( _ret == ERROR_BUFFER_TOO_SMALL )
		{
			DBG_BP(); 
		}
#endif //_DEBUG

		ctrl->SetText( text ); 
		ctrl->SetToolTip( text ); 		
	}while( FALSE ); 
	}
	catch( ... )
	{
		ULONG test = 0; 
	}

	return ret; 
}