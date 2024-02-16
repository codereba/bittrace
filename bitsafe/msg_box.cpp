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
			_T( "7�����ǽ��û�а�װ,�Ƿ����ڰ�װ?" ) ); 

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