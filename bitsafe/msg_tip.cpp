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

#include "stdafx.h"
#include "resource.h"
#include "acl_define.h"
#include "ui_ctrl.h"
#include "msg_tip.h"

ULONG event_tip_dlg::show_time = EVENT_POPUP_SHOW_TIME; 
ULONG event_tip_dlg::popup_speed = POPUP_UPDATE_TIME; 
ULONG event_tip_dlg::one_popup_unit = POPUP_UNIT; 

#define MAX_INT32_VAL 0x7fffffff

#ifdef _DEBUG
extern BOOLEAN debugging_action; 
LRESULT init_debug_actions(); 
LRESULT set_socket_debug_info( LPCSTR ip, ULONG port ); 
LRESULT check_socket_debug_info( ULONG ip, ULONG port ); 
LRESULT debug_sys_action( sys_action_desc *action ); 
LRESULT _set_socket_debug_info( ULONG ip, ULONG port ); 

#endif //_DEBUG

LRESULT CALLBACK _on_popup_closed_r3( sys_action_output *action_event, PVOID context, PVOID context2 )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	dlg_ret_state ret_state;  
	ACTION_RECORD_TYPE need_record; 
	event_tip_dlg *msg_tip; 
	action_log_notify *logger; 

	ASSERT( context != NULL ); 
	ASSERT( context2 != NULL ); 

	DBG_BP(); 

	logger = ( action_log_notify* )context2; 

	msg_tip = ( event_tip_dlg* )context; 

	if( msg_tip->popup_index != INVALID_POPUP_SPACE )
	{
		release_popup_space( msg_tip->popup_index ); 
	}

	ret_state = msg_tip->get_dlg_ret_statue(); 

	need_record = msg_tip->is_need_record(); 

	ASSERT( TRUE == is_valid_action_type( need_record ) ); 

	if( msg_tip->popup_count <= 0 )
	{
		ASSERT( FALSE ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	msg_tip->popup_count --; 

	if( msg_tip->popup_close_notify == NULL )
	{
		ASSERT( FALSE ); 
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	_ret = SetEvent( msg_tip->popup_close_notify ); 
	if( FALSE == _ret )
	{
		ret = GetLastError(); 
	}

_return:
	return ret; 
}

LRESULT CALLBACK on_popup_closed_r3( sys_action_output *action_event, PVOID context, PVOID context2 )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	dlg_ret_state ret_state;  
	ACTION_RECORD_TYPE need_record; 
	event_tip_dlg *msg_tip; 
	action_log_notify *logger; 

	ASSERT( action_event != NULL ); 
	ASSERT( context != NULL ); 
	ASSERT( context2 != NULL ); 

	DBG_BP(); 

	logger = ( action_log_notify* )context2; 

	msg_tip = ( event_tip_dlg* )context; 

	if( msg_tip->popup_index != INVALID_POPUP_SPACE )
	{
		release_popup_space( msg_tip->popup_index ); 
	}

	ret_state = msg_tip->get_dlg_ret_statue(); 

	need_record = msg_tip->is_need_record(); 

	if( ret_state == OK_STATE )
	{
		action_event->action.resp = ACTION_ALLOW; 
	}
	else
	{
		action_event->action.resp = ACTION_BLOCK; 
	}

	if( logger != NULL )
	{
		logger->log_notify( action_event ); 
	}

#ifdef _DEBUG
	{
		if( debugging_action == TRUE )
		{
			_set_socket_debug_info( action_event->action.desc.socket.dest_ip.ip.ip_begin, 
				action_event->action.desc.socket.dest_port.port.port_begin ); 
		}
	} 

	debug_sys_action( &action_event->action ); 
#endif //_DEBUG

	ASSERT( TRUE == is_valid_action_type( need_record ) ); 

	switch( need_record )	
	{
	case RECORD_APP_ACTION:
		input_r3_rule_from_action( &action_event->action, 0 ); 
		break; 
	case RECORD_APP_ACTION_TYPE:
		input_r3_rule_from_action( &action_event->action, APP_ACTION_TYPE_RULE ); 
		break; 
	case RECORD_APP:
		input_r3_rule_from_action( &action_event->action, APP_DEFINE_RULE ); 
		break; 
	case RECORD_NONE:
		break; 
	default:
		ASSERT( FALSE && "why get the unknown reocrd type in " __FUNCTION__ ); 
		break; 
	}

	if( msg_tip->popup_count <= 0 )
	{
		ASSERT( FALSE ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	msg_tip->popup_count --; 

	if( msg_tip->popup_close_notify == NULL )
	{
		ASSERT( FALSE ); 
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	_ret = SetEvent( msg_tip->popup_close_notify ); 
	if( FALSE == _ret )
	{
		ret = GetLastError(); 
	}

_return:
	return ret; 
}

LRESULT CALLBACK on_popup_closed( sys_action_output *action_event, PVOID context, PVOID context2 )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	event_action_response event_respon; 
	dlg_ret_state ret_state;  
	ACTION_RECORD_TYPE need_record; 
	event_tip_dlg *msg_tip; 

	ASSERT( action_event != NULL ); 
	ASSERT( context != NULL ); 
	UNREFERENCED_PARAMETER( context2 ); 

	msg_tip = ( event_tip_dlg* )context; 

	if( msg_tip->popup_index != INVALID_POPUP_SPACE )
	{
		release_popup_space( msg_tip->popup_index ); 
	}

	ret_state = msg_tip->get_dlg_ret_statue(); 

	need_record = msg_tip->is_need_record(); 

	if( ret_state == OK_STATE )
	{
		event_respon.action = ACTION_ALLOW; 
	}
	else
	{
		event_respon.action = ACTION_BLOCK; 
	}

	ASSERT( TRUE == is_valid_action_type( need_record ) ); 
	event_respon.need_record = ( BYTE )need_record; 
	event_respon.id = action_event->action.id; 

	ret = do_ui_ctrl( BITSAFE_UI_ID, SYS_MON_EVENT_RESPONSE, ( PBYTE )&event_respon, sizeof( event_respon ), NULL, 0, NULL, NULL ); 

	if( ret == ERROR_SUCCESS )
	{
		if( need_record == RECORD_APP_ACTION )
		{
			input_rule_from_action( &action_event->action, 0 ); 
		}
		else if( need_record == RECORD_APP_ACTION_TYPE )
		{
			input_rule_from_action( &action_event->action, APP_ACTION_TYPE_RULE ); 
		}
		else if( need_record == RECORD_APP )
		{
			input_rule_from_action( &action_event->action, APP_DEFINE_RULE ); 
		}
		else
		{
			ASSERT( FALSE && "why get the unknown reocrd type in " __FUNCTION__ ); 
		}
	}

	if( msg_tip->popup_count <= 0 )
	{
		ASSERT( FALSE ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	msg_tip->popup_count --; 

	if( msg_tip->popup_close_notify == NULL )
	{
		ASSERT( FALSE ); 
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	_ret = SetEvent( msg_tip->popup_close_notify ); 
	if( FALSE == _ret )
	{
		ret = GetLastError(); 
	}

_return:
	return ret; 
}

LRESULT show_event_tip( HWND parent, 
							  LPCTSTR msg, 
							  LPCTSTR data_dump, 
							  sys_action_output* action_event, 
							  LPCTSTR title, 
							  ULONG mode, 
							  action_log_notify *log_notifier, 
							  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	event_tip_dlg *msg_tip; 
	CControlUI *msg_txt; 
	HWND wnd; 

	msg_tip = event_tip_dlg::create_popup( event_tip_dlg::show_time, event_tip_dlg::popup_speed, event_tip_dlg::one_popup_unit, parent ); 
	if( msg_tip == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ret = msg_tip->set_action_event( action_event ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "set current action in the message tip box error" ); 
		log_trace( ( MSG_ERROR, "set current action in the message tip box error 0x%0.8x\n", ret ) ); 
		goto _return; 
	}

	if( flags == R3_SYS_EVENT_RESPONSE )
	{
		msg_tip->set_popup_callback( on_popup_closed_r3 ); 
		msg_tip->set_log_notifier( log_notifier ); 
	}
	else
	{
		msg_tip->set_popup_callback( on_popup_closed ); 
	}

	wnd = msg_tip->Create( NULL, title, WS_VISIBLE | WS_OVERLAPPEDWINDOW, MAX_INT32_VAL, MAX_INT32_VAL, 0, 0, 0 ); 
	if( NULL == wnd )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	if( title == NULL )
	{
		title = _T( "" ); 
	}

	msg_tip->SetIcon( IDI_MAIN_ICON ); 
	msg_txt = msg_tip->m_pm.FindControl( _T( "msg_txt" ) ); 

	ASSERT( msg_txt != NULL ); 
	msg_txt->SetText( msg ); 

	if( data_dump != NULL 
		&&  *data_dump != L'\0' )
	{
		INT32 LineCount; 

		msg_txt = msg_tip->m_pm.FindControl( _T( "data_txt" ) ); 
		ASSERT( msg_txt != NULL ); 
		msg_txt->SetText( data_dump ); 

		LineCount = ( ( CRichEditUI* )msg_txt )->GetLineCount(); 
		ASSERT( LineCount >= 0 ); 
		( ( CRichEditUI* )msg_txt )->LineScroll( -LineCount, 0 ); 
	}

	if( title != NULL )
	{
		msg_txt = msg_tip->m_pm.FindControl( _T( "title_txt" ) ); 

		ASSERT( msg_txt != NULL ); 
		msg_txt->SetText( title ); 
	}

	ASSERT( wnd == msg_tip->GetHWND() ); 

	ret = msg_tip->popup_msg(); 
	
	if( ret != ERROR_SUCCESS )
	{
		msg_tip->Close(); 
	}

_return: 
	return ret; 
}

LRESULT __show_event_tip( HWND parent, 
					   LPCTSTR msg, 
					   LPCTSTR data_dump, 
					   sys_action_info* action_event, 
					   LPCTSTR title, 
					   ULONG mode, 
					   action_log_notify *log_notifier, 
					   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	event_tip_dlg *msg_tip; 
	CControlUI *msg_txt; 
	HWND wnd; 

	msg_tip = event_tip_dlg::create_popup( event_tip_dlg::show_time, event_tip_dlg::popup_speed, event_tip_dlg::one_popup_unit, parent ); 
	if( msg_tip == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	if( flags == R3_SYS_EVENT_RESPONSE )
	{
		msg_tip->set_popup_callback( _on_popup_closed_r3 ); 
		msg_tip->set_log_notifier( log_notifier ); 
	}
	else
	{
		goto _return; 
	}

	wnd = msg_tip->Create( NULL, title, WS_VISIBLE | WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0 );
	if( NULL == wnd )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	if( title == NULL )
	{
		title = _T( "" ); 
	}

	msg_tip->SetIcon( IDI_MAIN_ICON ); 
	msg_txt = msg_tip->m_pm.FindControl( _T( "msg_txt" ) ); 

	ASSERT( msg_txt != NULL ); 
	msg_txt->SetText( msg ); 

	if( data_dump != NULL 
		&&  *data_dump != L'\0' )
	{
		INT32 LineCount; 
		msg_txt = msg_tip->m_pm.FindControl( _T( "data_txt" ) ); 
		ASSERT( msg_txt != NULL ); 
		msg_txt->SetText( data_dump ); 

		LineCount = ( ( CRichEditUI* )msg_txt )->GetLineCount(); 
		ASSERT( LineCount >= 0 ); 
		( ( CRichEditUI* )msg_txt )->LineScroll( -LineCount, 0 ); 
	}

	if( title != NULL )
	{
		msg_txt = msg_tip->m_pm.FindControl( _T( "title_txt" ) ); 

		ASSERT( msg_txt != NULL ); 
		msg_txt->SetText( title ); 
	}

	ASSERT( wnd == msg_tip->GetHWND() ); 

	ret = msg_tip->popup_msg(); 

	if( ret != ERROR_SUCCESS )
	{
		msg_tip->Close(); 
	}

_return: 
	return ret; 
}

LRESULT _show_event_tip( HWND parent, 
					   LPCTSTR msg, 
					   LPCTSTR data_dump, 
					   sys_action_desc* sys_action, 
					   PVOID data, 
					   ULONG data_len, 
					   LPCTSTR title, 
					   ULONG mode, 
					   action_log_notify *log_notifier, 
					   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	event_tip_dlg *msg_tip; 
	CControlUI *msg_txt; 
	HWND wnd; 

	msg_tip = event_tip_dlg::create_popup( event_tip_dlg::show_time, event_tip_dlg::popup_speed, event_tip_dlg::one_popup_unit, parent ); 
	if( msg_tip == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ret = msg_tip->_set_action_event( sys_action, data, data_len ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "set current action in the message tip box error" ); 
		log_trace( ( MSG_ERROR, "set current action in the message tip box error 0x%0.8x\n", ret ) ); 
		goto _return; 
	}

	if( flags == R3_SYS_EVENT_RESPONSE )
	{
		msg_tip->set_popup_callback( on_popup_closed_r3 ); 
		msg_tip->set_log_notifier( log_notifier ); 
	}
	else
	{
		msg_tip->set_popup_callback( on_popup_closed ); 
	}

	{

		wnd = msg_tip->Create( NULL, title, WS_VISIBLE | WS_OVERLAPPEDWINDOW, MAX_INT32_VAL, MAX_INT32_VAL, 0, 0, 0 );
	}

	if( NULL == wnd )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	if( title == NULL )
	{
		title = _T( "" ); 
	}

	msg_tip->SetIcon( IDI_MAIN_ICON ); 
	msg_txt = msg_tip->m_pm.FindControl( _T( "msg_txt" ) ); 

	ASSERT( msg_txt != NULL ); 
	msg_txt->SetText( msg ); 

	if( data_dump != NULL 
		&&  *data_dump != L'\0' )
	{
		msg_txt = msg_tip->m_pm.FindControl( _T( "data_txt" ) ); 
		ASSERT( msg_txt != NULL ); 
		msg_txt->SetText( data_dump ); 
	}

	if( title != NULL )
	{
		msg_txt = msg_tip->m_pm.FindControl( _T( "title_txt" ) ); 

		ASSERT( msg_txt != NULL ); 
		msg_txt->SetText( title ); 
	}

	ASSERT( wnd == msg_tip->GetHWND() ); 

	ret = msg_tip->popup_msg(); 

	if( ret != ERROR_SUCCESS )
	{
		msg_tip->Close(); 
	}

_return: 
	return ret; 
}

LRESULT init_popup_wnd_context()
{
	LRESULT ret = ERROR_SUCCESS; 
	event_tip_dlg msg_tip; 
	RECT msg_tip_rect; 


	msg_tip_rect.left = 0; 
	msg_tip_rect.top = 0; 
	msg_tip_rect.bottom = 393; 
	msg_tip_rect.right = 318; 

	ret = init_popup_space( &msg_tip_rect, wnd_popup_bottom ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = calc_max_popup_wnd_num( &msg_tip_rect, &popup_tip_wnd::max_popup_count ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	popup_tip_wnd::popup_count = 0; 
	popup_tip_wnd::popup_close_notify = CreateEvent( NULL, FALSE, FALSE, NULL ); 

	if( NULL == popup_tip_wnd::popup_close_notify )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	if( ret != ERROR_SUCCESS )
	{
		popup_tip_wnd::max_popup_count = 1; 
	}

	return ret; 
}

LRESULT uninit_popup_wnd_context()
{
	LRESULT ret = ERROR_SUCCESS; 

	if( NULL != popup_tip_wnd::popup_close_notify )
	{
		CloseHandle( popup_tip_wnd::popup_close_notify ); 
	}

	uninit_popup_space(); 

	return ret; 
}