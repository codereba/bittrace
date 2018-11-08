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
#include "ui_ctrl.h"
#include "msg_box.h"

LRESULT WINAPI notify_error_msg( LPCWSTR msg )
{
	LRESULT ret = ERROR_ERRORS_ENCOUNTERED; 

	do 
	{

#ifdef _DEBUG
		LRESULT _ret; 
		ULONG session_id; 

		_ret = ( LRESULT )ProcessIdToSessionId( GetCurrentProcessId(), &session_id ); 

		if( ( BOOL )_ret == FALSE )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "get the session id of current process failed \n" ) ); 
		}
		else
		{
			log_trace( ( MSG_INFO, "current session id is %d \n", session_id ) ); 
			if( session_id == 0 )
			{}
		}
#endif //_DEBUG
		ret = show_msg( NULL, msg, NULL ); 

	} while ( FALSE );

	return ret; 
}

LRESULT request_user_action( PVOID param )
{
	LRESULT ret = ERROR_ERRORS_ENCOUNTERED; 
	LPCTSTR tmp_text; 
	LRESULT _ret; 

#ifdef _DEBUG
	ULONG session_id; 

	_ret = ( LRESULT )ProcessIdToSessionId( GetCurrentProcessId(), &session_id ); 

	if( ( BOOL )_ret == FALSE )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "get the session id of current process failed \n" ) ); 
	}
	else
	{
		log_trace( ( MSG_INFO, "current session id is %d \n", session_id ) ); 
		if( session_id == 0 )
		{}
	}
#endif //_DEBUG

	if( param != NULL )
	{
		action_ui_notify *notifier; 
		notify_data data; 

		tmp_text = _get_string_by_id( TEXT_BK_WORK_BITSAFE_NEED_INSTALL_TIP, 
			_T( "7层防火墙还没有安装,是否现在安装?" ) ); 

		notifier = ( action_ui_notify* )param; 

		data.data = ( PVOID )tmp_text; 
		data.data_len = ( ULONG )( ( wcslen( tmp_text ) + 1 ) << 1 ); 

		ret = notifier->action_notify( REQUEST_UI_INTERACT_NOTIFY, &data ); 
	}
	else
	{
		ASSERT( FALSE && "request user action error\n" ); 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT dbg_show_msg( HWND parent, LPCTSTR msg, LPCTSTR title, ULONG mode )
{
	dlg_ret_state ret_state; 

	return show_msg( parent, msg, &ret_state, title, mode ); 
}