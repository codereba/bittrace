/*
 *
 * Copyright 2010 JiJie Shi
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
 
 #ifdef BITSAFE_TEST_TOOL
#include "common_func.h"
#else
#include "StdAfx.h"
#endif //BITSAFE_TEST_TOOL

#include "strtbl.h"
#include "General.h"
#include "string_mng.h"
#include "memory_mng.h"
#include "erc_setting.h"
#include "remote_conf_dlg.h"
#ifdef BITSAFE_TEST_TOOL
typedef ULONG dlg_ret_state; 

INLINE LRESULT show_msg( HWND parent, LPCTSTR msg, dlg_ret_state *ret_state = NULL, LPCTSTR title = NULL, ULONG mode = 0 )
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}
#else
#include "msg_box.h"
#endif //BITSAFE_TEST_TOOL

//#include "email_setting_dlg.h"
//#include "pop_setting_dlg.h"
#include "ui_ctrl.h"

email_box email_box_info = { 0 }; 
CRITICAL_SECTION mail_box_lock; 

/* Define */
#define SEND_SIZE				4096			// 憲怣帪偺暘妱僒僀僘
#define SMTP_ERRORSTATUS		400				// SMTP僄儔乕儗僗億儞僗
#define MIME_VERSION			"1.0"			// MIME僶乕僕儑儞

#define AUTH_CRAM_MD5			0				// SMTP-AUTH 偺擣徹僞僀僾
#define AUTH_LOGIN				1
#define AUTH_PLAIN				2
#define AUTH_NON				99

#define CMD_HELO				"HELO"
#define CMD_EHLO				"EHLO"
#define CMD_STARTTLS			"STARTTLS"
#define CMD_AUTH				"AUTH"
#define CMD_AUTH_CRAM_MD5		"CRAM-MD5"
#define CMD_AUTH_LOGIN			"LOGIN"
#define CMD_AUTH_PLAIN			"PLAIN"
#define CMD_RSET				"RSET"
#define CMD_MAIL_FROM			"MAIL FROM"
#define CMD_RCPT_TO				"RCPT TO"
#define CMD_DATA				"DATA"
#define CMD_DATA_END			"\r\n.\r\n"
#define CMD_QUIT				"QUIT"

LRESULT set_mail_box( email_box *mail_box )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = check_email_conf_valid( mail_box ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	lock_cs( mail_box_lock ); 
	memcpy( &email_box_info, mail_box, sizeof( email_box ) ); 

	unlock_cs( mail_box_lock ); 
_return:
	return ret; 
}

LRESULT get_mail_box( email_box *mail_box )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( mail_box != NULL ); 

	lock_cs( mail_box_lock ); 
	ret = check_email_conf_valid( &email_box_info ); 
	unlock_cs( mail_box_lock ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	lock_cs( mail_box_lock ); 
	memcpy( mail_box, &email_box_info, sizeof( email_box ) ); 
	unlock_cs( mail_box_lock ); 

_return:
	return ret; 
}

//#define REMOTE_BLOCK_CMD "blockall" 
//#define REMOTE_ALLOW_CMD "allowall"
//
//LRESULT smtp_send_cmd( )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//#define SMTP_ERR_MSG_LEN 256
//	TCHAR err_msg[ SMTP_ERR_MSG_LEN ]; 
//	ULONG smtp_ip; 
//	SSL_INFO ssl_info = { 0 }; 
//	SOCKET smtp_socket; 
//
//	if( *email_box_info.server == '\0' )
//	{
//		ret = ERROR_NOT_READY; 
//		goto _return; 
//	}
//	
//	smtp_ip = get_host_by_name( NULL, email_box_info.server, err_msg ); 
//
//	if( smtp_ip == 0 )
//	{
//		log_trace( ( DBG_MSG_AND_ERROR_OUT, "%ws", err_msg ) ); 
//
//		ret = ERROR_ERRORS_ENCOUNTERED; 
//		goto _return; 
//	}
//
//	smtp_socket = connect_server( NULL, 
//		smtp_ip, 
//		email_box_info.port, 
//		FALSE, 
//		&ssl_info, 
//		err_msg ); 
//
//	if( smtp_socket == -1 )
//	{
//		ret = ERROR_ERRORS_ENCOUNTERED; 
//		goto _return; 
//	}
//}

#define REMOTE_CTRL_SUBJECT "ERC CONTROL"
#define MAIL_HEADER_SUBJECT "Subject:"
#define MAIL_HEADER_BODY "Body"
#define REMOTE_CTRL_BODY_BEGIN "REMOTE COMMAND:"

#if 0
#define REMOTE_CTRL_SUBJECT TEXT( "ERC CONTROL" )
#define MAIL_HEADER_SUBJECT TEXT( "Subject:" )
#define MAIL_HEADER_BODY TEXT( "Body" )
#define REMOTE_CTRL_BODY_BEGIN TEXT( "REMOTE COMMAND:" )
#endif //0

#define BLOCK_ALL_COMMAND "BLOCK ALL" 
#define ALLOW_ALL_COMMAND "ALLOW ALL" 
#define ACL_CTRL_COMMAND "ACL CTRL"

typedef enum _work_mode
{
	WORK_FREE_MODE, //自由
	WORK_BLOCK_MODE, //禁止
	WORK_ACL_MODE,  //绿色
	MAX_WORK_MODE
} work_mode;

typedef struct _remote_cmd_info
{
	LARGE_INTEGER time; 
	ULONG work_mode;
} remote_cmd_info, *premote_cmd_info; 
LRESULT check_remote_command_by_email( email_handler* handler, LPCSTR data, ULONG data_len, PVOID *context )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	CHAR *email_body; 
	CHAR *remote_command; 
	//TCHAR *erc_tip = NULL; 
	ULONG work_mode; 
	CHAR *work_time_str; 
	LARGE_INTEGER work_time; 
	//TCHAR tip_msg[ MAX_PATH ]; 
	remote_cmd_info *work_info; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( data != NULL ); 
	ASSERT( context != NULL ); 

	*context = NULL; 

	_ret = item_filter_check_content( ( CHAR* )data, _T( MAIL_HEADER_SUBJECT ), _T( REMOTE_CTRL_SUBJECT ) ); 
	if( _ret == FALSE ) 
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	//ret = item_filter_check_content( ( CHAR* )data, MAIL_HEADER_BODY, REMOTE_CTRL_BODY_BEGIN ); 
	//if( ret == FALSE ) 
	//{
	//	goto _return; 
	//}

	email_body = GetBodyPointa( ( CHAR* )data ); 

	if( email_body == NULL )
	{
		//command_begin = StrStrIA( email_body, REMOTE_CTRL_BODY_BEGIN ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	if( strncmp( email_body, 
		REMOTE_CTRL_BODY_BEGIN, 
		CONST_STR_LEN( REMOTE_CTRL_BODY_BEGIN ) ) != 0 )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	remote_command = email_body + CONST_STR_LEN( REMOTE_CTRL_BODY_BEGIN ); 

	if( strncmp( remote_command, ALLOW_ALL_COMMAND, CONST_STR_LEN( ALLOW_ALL_COMMAND ) ) == 0 )
	{
		//erc_tip = _T( "接收到允喧琦有通信远程脕E丒 ); 
		work_mode = WORK_FREE_MODE; 

		work_time_str = remote_command + CONST_STR_LEN( ALLOW_ALL_COMMAND ) + 1; 
		log_trace( ( MSG_INFO, "work time is %s\n", work_time_str ) ); 

		work_time.QuadPart = _strtoui64( work_time_str, &work_time_str, 10 ); 
	}
	else if( strncmp( remote_command, BLOCK_ALL_COMMAND, CONST_STR_LEN( BLOCK_ALL_COMMAND ) ) == 0 )
	{
		//erc_tip = _T( "接收到阻止所有通信远程脕E丒 ); 
		work_mode = WORK_BLOCK_MODE; 	

		work_time_str = remote_command + CONST_STR_LEN( BLOCK_ALL_COMMAND ) + 1; 
		log_trace( ( MSG_INFO, "work time is %s\n", work_time_str ) ); 

		work_time.QuadPart = _strtoui64( work_time_str, &work_time_str, 10 ); 
	}
	else if( strncmp( remote_command, ACL_CTRL_COMMAND, CONST_STR_LEN( ACL_CTRL_COMMAND ) ) == 0 )
	{
		//erc_tip = _T( "接收到访问控制方式通信远程脕E丒 ); 

		work_mode = WORK_ACL_MODE; 

		work_time_str = remote_command + CONST_STR_LEN( ACL_CTRL_COMMAND ) + 1; 
		log_trace( ( MSG_INFO, "work time is %s\n", work_time_str ) ); 

		work_time.QuadPart = _strtoui64( work_time_str, &work_time_str, 10 ); 
	}
	else
	{
		log_trace( ( MSG_ERROR, "unknown remote command %s\n", remote_command ) ); 

		ASSERT( FALSE && "unknown remote command \n" ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	//DBG_BP(); 

	work_info = ( remote_cmd_info* )malloc( sizeof( *work_info ) ); 
	if( work_info == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	work_info->time.QuadPart = work_time.QuadPart; 
	work_info->work_mode = work_mode; 

	*context = ( PVOID )work_info; 

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT execute_remote_command_by_email( email_handler* handler, PVOID context )
{
	LRESULT ret;// = ERROR_SUCCESS; 
	LPCTSTR erc_tip = NULL; 
	remote_cmd_info *cmd_info; 
	TCHAR tip_msg[ MAX_PATH ]; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( context != NULL ); 

	cmd_info = ( remote_cmd_info* )context; 

	if( cmd_info->work_mode == WORK_FREE_MODE )
	{
		erc_tip = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_RECEIVE_ALLOW_COMMAND, 
			_T( "接收到允喧琦有通信远程脕E丒 ) ); 
	}
	else if( cmd_info->work_mode == WORK_BLOCK_MODE )
	{
		erc_tip = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_RECEIVE_BLOCK_COMMAND, 
			_T( "接收到阻止所有通信远程脕E丒 ) ); 
	}
	else if( cmd_info->work_mode == WORK_ACL_MODE )
	{
		erc_tip = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_RECEIVE_ACL_COMMAND, 
			_T( "接收到访问控制方式通信远程脕E丒 ) ); 
	}
	else
	{
		ASSERT( FALSE && "unknow work mode " ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		BITSAFE_SET_WORK_TIME, 
		( PBYTE )&cmd_info->time, 
		sizeof( LARGE_INTEGER ), 
		NULL, 
		0, 
		NULL, 
		NULL ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		BITSAFE_SET_WORK_MODE, 
		( PBYTE )&cmd_info->work_mode, 
		sizeof( ULONG ), 
		NULL, 
		0, 
		NULL, 
		NULL ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( erc_tip != NULL )
	{
		LPCTSTR tmp_text; 
		action_ui_notify *notifier; 

		if( handler->notifier != NULL )
		{
			notify_data data_notify; 

			notifier = ( action_ui_notify* )handler->notifier; 

			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_COMMAND_HOLD_TIME, 
				_T( "%s, 保持时间为:%I64u脕En" ) ); 
			_sntprintf( tip_msg, MAX_PATH, tmp_text, erc_tip, ( cmd_info->time.QuadPart / 10000000 ) ); 
		
			data_notify.data = ( PVOID )tmp_text; 
			data_notify.data_len = ( wcslen( tmp_text ) + 1 ) * sizeof( TCHAR ); 

			notifier->action_notify( ERC_CMD_EXECUTED, &data_notify ); 
		}
	}

_return:
	free( cmd_info ); 

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}


HANDLE exec_remote_thread = NULL; 
BOOLEAN stop_erc_ctrl = FALSE; 
HANDLE erc_ctrl_notify = NULL; 
ULONG CALLBACK thread_exec_remote_control( PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG pop_ip; 
	TCHAR err_msg[ MAX_PATH ]; 
	email_handler *handler; 
	INT32 try_time; 
	ULONG wait_ret; 

	email_handler _handler = { 0 }; 
	email_box cur_email; 
	MAILITEM email_item; 
	SOCKET sock = INVALID_SOCKET; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCDNAME__ ) ); 

	ASSERT( check_email_conf_valid( &email_box_info ) == ERROR_SUCCESS ); 

	//cur_email = &email_box_info; 

	email_item.Attach = NULL; 
	email_item.Bcc = NULL; 
	email_item.Body = ( CHAR* )""; 
	email_item.BodyCharset = _T( "gb2312" ); //_T( "UNICODE" ); 
	email_item.BodyEncoding = 2; 
	email_item.Cc = NULL; //_T( "balancesoft@sina.com" ); 
	email_item.ContentType = NULL; 
	email_item.Date = NULL; //_T( "2011-12-06" ); 
	email_item.Download = TRUE; 
	email_item.Encoding = NULL; 
	email_item.HeadCharset = _T( "gb2312" ); 
	email_item.HeadEncoding = 2; 
	email_item.Indent = 0; 
	email_item.hEditWnd = NULL; 
	email_item.hProcess = NULL; 
	email_item.InReplyTo = NULL; 
	email_item.MailBox = cur_email.email; //_T( "balancesoft@sohu.com" ); 
	email_item.MailStatus = 0; 
	email_item.MessageID = NULL; 
	email_item.Multipart = TRUE; 
	email_item.New = FALSE; 
	email_item.NextNo = 0; 
	email_item.No = 0; 
	email_item.PrevNo = 0; 
	email_item.References = NULL; 
	email_item.ReplyTo = NULL; 
	email_item.Size = NULL; 
	email_item.Status = 0; 
	email_item.Subject = _T( "" ); 
	email_item.To = cur_email.email; //_T( "balancesoft@sohu.com" ); 
	email_item.UIDL = NULL; 

	//init_test_email_box( &cur_email ); 

	_handler.send_msg_id = TRUE; 
	_handler.auth_flag = TRUE; 
	_handler.command_check = check_remote_command_by_email; 
	_handler.command_handler = execute_remote_command_by_email; 
	_handler.notifier = context; 

	handler = &_handler; 

	//ASSERT( context != NULL ); 

	//handler = ( email_handler* )context; 

	//ASSERT( handler->cur_email != NULL ); 

	//cur_email = handler->cur_email; 
	
	for( ; ; )
	{
		ret = get_mail_box( &cur_email ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		_handler.cur_email = &cur_email; 

		if( stop_erc_ctrl == TRUE )
		{
			goto _return; 
		}

		ret = get_host_by_name( handler, cur_email.pop3_server, &pop_ip, err_msg ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( pop_ip != SOCKET_ERROR ); 

		ret = connect_server( handler,
			pop_ip, 
			( USHORT )cur_email.pop3_port,
			cur_email.pop_use_ssl == 0 
			|| ( cur_email.pop_ssl_info.Type == 4 ) ? 
			-1 : cur_email.pop_ssl_info.Type,
			&cur_email.pop_ssl_info,
			&sock, 
			err_msg ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ASSERT( sock != INVALID_SOCKET ); 

		_handler.command_status = POP_START; 

		try_time = 0; 

		for( ; ; )
		{
			if( stop_erc_ctrl == TRUE )
			{
				goto _return; 
			}

			Sleep( RECVTIME ); 

			ret = recv_select( handler, sock, pop3_list_proc ); 
			switch( ret ) 
			{
			case ERROR_NOT_ENOUGH_MEMORY:
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_MEMALLOC ) );
				goto _return; 
				break;

			case ERROR_SOCKET_ERROR:
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_SOCK_SELECT ) ); 
				goto _return; 
				break;

			case ERROR_SOCKET_CLOSED:
				if( handler->command_status != POP_QUIT ) 
				{
					log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", "socket is connected when the action have not done"	) );
				} 
				else 
				{
					socket_close( handler, sock ); 
					sock = INVALID_SOCKET; 
					ret = ERROR_SUCCESS; 
				}

				goto _return; 
				break;

			case ERROR_SUCCESS:
				{
				}
				break;

			case ERROR_SOCKET_NO_RECVED:
				{
					try_time += 1; 

					if( try_time == RETRY_TIME )
					{
						ret = ERROR_TIMEOUT; 
						goto _return; 
					}

					break; 
				}
			default:
				log_trace( ( MSG_ERROR, "select receive state of the seocket failed 0x%0.8x\n", ret ) ); 
				goto _return; 
				break; 
			}
		}

_return:
		if( sock != INVALID_SOCKET )
		{
			socket_close( handler, sock ); 
		}

		ASSERT( erc_ctrl_notify != NULL ); 

		wait_ret = WaitForSingleObject( erc_ctrl_notify, ERC_REST_TIME ); 
		if( wait_ret == WAIT_OBJECT_0 )
		{
			continue; 
		}
		else
		{
			ASSERT( wait_ret == WAIT_TIMEOUT ); 
		}
	}

	log_trace( ( MSG_INFO, "leave %s\n", __FUNCDNAME__, ret ) ); 

	return ( ULONG )ret; 
}

LRESULT wait_thread_end( HANDLE thread_handle, ULONG wait_time ); 
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	DWORD wait_ret; 
//
//	ASSERT( thread_handle != NULL ); 
//
//	wait_ret = WaitForSingleObject( thread_handle, wait_time );
//
//	if( WAIT_OBJECT_0 != wait_ret && 
//		WAIT_ABANDONED != wait_ret )
//	{
//		if( wait_ret == WAIT_FAILED )
//		{
//			ret = GetLastError(); 
//		}
//
//		TerminateThread( thread_handle, 0 );
//	}
//
//	CloseHandle( thread_handle );
//
//	return ret; 
//}

LRESULT stop_pop3_remote_control()
{
	LRESULT ret = ERROR_SUCCESS; 
	stop_erc_ctrl = TRUE; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCDNAME__) ); 

	if( exec_remote_thread != NULL )
	{
		stop_erc_ctrl = TRUE; 

		ASSERT( erc_ctrl_notify != NULL ); 
		
		SetEvent( erc_ctrl_notify ); 

		wait_thread_end( exec_remote_thread, 3000 ); 
		exec_remote_thread = NULL; 

		CloseHandle( erc_ctrl_notify ); 
		erc_ctrl_notify = NULL; 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCDNAME__, ret ) ); 
	return ret; 
}

LRESULT pop3_exec_remote_control( PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCDNAME__ ) ); 

	if( erc_ctrl_notify != NULL )
	{
		//ASSERT( FALSE ); 
		goto _return; 
	}

	if( exec_remote_thread != NULL )
	{
		//ASSERT( FALSE ); 
		goto _return; 
	}

	//erc_ctrl_notify = NULL; 
	//exec_remote_thread = NULL; 

	lock_cs( mail_box_lock ); 
	ret = check_email_conf_valid( &email_box_info ); 
	unlock_cs( mail_box_lock ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "email account for remote control invalid\n" ) ); 
		goto _return; 
	}

	erc_ctrl_notify = CreateEvent( NULL, FALSE, FALSE, NULL ); 
	if( erc_ctrl_notify == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	stop_erc_ctrl = FALSE; 

	exec_remote_thread = CreateThread( NULL, 0, thread_exec_remote_control, context, 0, NULL ); 
	if( exec_remote_thread == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( exec_remote_thread != NULL )
		{
			stop_erc_ctrl = TRUE; 

			if( erc_ctrl_notify != NULL )
			{
				SetEvent( erc_ctrl_notify ); 
			}

			wait_thread_end( exec_remote_thread, 3000 ); 
		}

		if( erc_ctrl_notify != NULL )
		{
			CloseHandle( erc_ctrl_notify ); 
		}
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCDNAME__, ret ) ); 
	return ret; 
}

LRESULT smtp_remote_control( ULONG mode, ULONG hour, ULONG minite, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR err_msg[ MAX_PATH ]; 
	email_handler handler = { 0 }; 
	email_box cur_email; 
	MAILITEM email_item; 
#define MAX_ERC_CMD_LEN 260 
	CHAR ctrl_cmd[ MAX_ERC_CMD_LEN ]; 
	SOCKET sock = INVALID_SOCKET; 
	LARGE_INTEGER ctrl_time; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCDNAME__ ) ); 

	ASSERT( hour <= 24 ); 

	lock_cs( mail_box_lock ); 
	ret = check_email_conf_valid( &email_box_info ); 
	unlock_cs( mail_box_lock ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "email account for remote control invalid\n" ) ); 
		goto _return; 
	}

	//cur_email = &email_box_info; 

	email_item.Attach = NULL; 
	email_item.Bcc = NULL; 

	ctrl_time.QuadPart = ( ULONGLONG )( ( ( ( ULONGLONG )hour * 3600 ) + ( ( ULONGLONG )minite * 60 ) ) * 10000000 ); 

	if( mode == REMOTE_CONTROL_BLOCK )
	{
		_snprintf( ctrl_cmd, 
			MAX_ERC_CMD_LEN, 
			"%s%s %I64u", 
			REMOTE_CTRL_BODY_BEGIN, 
			BLOCK_ALL_COMMAND, 
			ctrl_time.QuadPart ); 
	}
	else if( mode == REMOTE_CONTROL_ALLOW )
	{
		_snprintf( ctrl_cmd, 
			MAX_ERC_CMD_LEN, 
			"%s%s %I64u", 
			REMOTE_CTRL_BODY_BEGIN, 
			ALLOW_ALL_COMMAND, 
			ctrl_time.QuadPart ); 
	}
	else if( mode == REMOTE_CONTROL_ACL )
	{
		_snprintf( ctrl_cmd, 
			MAX_ERC_CMD_LEN, 
			"%s%s %I64u", 
			REMOTE_CTRL_BODY_BEGIN, 
			ACL_CTRL_COMMAND, 
			ctrl_time.QuadPart ); 
	}
	else
	{
		ASSERT( FALSE && "invalid remote control mode" ); 
	}

	ret = get_mail_box( &cur_email ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	email_item.Body = ( CHAR* )ctrl_cmd; 
	email_item.BodyCharset = _T( "gb2312" ); //_T( "UNICODE" ); 
	email_item.BodyEncoding = 2; 
	email_item.Cc = NULL; //_T( "balancesoft@sina.com" ); 
	email_item.ContentType = NULL; 
	email_item.Date = NULL; //_T( "2011-12-06" ); 
	email_item.Download = TRUE; 
	email_item.Encoding = NULL; 
	email_item.HeadCharset = _T( "gb2312" ); 
	email_item.HeadEncoding = 2; 
	email_item.Indent = 0; 
	email_item.hEditWnd = NULL; 
	email_item.hProcess = NULL; 
	email_item.InReplyTo = NULL; 
	email_item.MailBox = cur_email.email; //_T( "balancesoft@sohu.com" ); 
	email_item.MailStatus = 0; 
	email_item.MessageID = NULL; 
	email_item.Multipart = TRUE; 
	email_item.New = FALSE; 
	email_item.NextNo = 0; 
	email_item.No = 0; 
	email_item.PrevNo = 0; 
	email_item.References = NULL; 
	email_item.ReplyTo = NULL; 
	email_item.Size = NULL; 
	email_item.Status = 0; 
	email_item.Subject = _T( REMOTE_CTRL_SUBJECT ); 
	email_item.To = cur_email.email; //_T( "balancesoft@sohu.com" ); 
	email_item.UIDL = NULL; 

	//init_test_email_box( &cur_email ); 

	handler.send_msg_id = TRUE; 
	//handler.cur_email = cur_email; 
	handler.auth_flag = TRUE; 

	ret = smtp_send_mail( &handler, &cur_email, &email_item, SMTP_SENDEND, err_msg ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	//if( mode == REMOTE_CONTROL_BLOCK )
	//{
	//	sock = smtp_send_mail( &handler, &cur_email, &email_item, TRUE, err_msg ); 
	//	if( sock == INVALID_SOCKET )
	//	{
	//		goto _return; 
	//	}

	//	//smtp_proc( &handler, sock, )
	//}
	//else if( mode == REMOTE_CONTROL_ALLOW )
	//{

	//}
	//else
	//{
	//	ASSERT( FALSE && "invalid remote control mode" ); 
	//}

_return:
	return ret; 
}

typedef LRESULT ( CALLBACK* smtp_erc_ret_callback )( LRESULT ret_val, PVOID param ); 
LRESULT CALLBACK smtp_erc_return_notify( LRESULT ret_val, PVOID param )
{
	HWND parent_wnd; 
	LPCTSTR tmp_text; 

	//parent_wnd = ( HWND )param; 
	
	if( ret_val == ERROR_SUCCESS )
	{
		tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_SEND_COMMAND_SUCCESSFULLY, 
			_T( "发送远程控制脕E晒? ) ); 
		show_msg( NULL, tmp_text ); 
	}
	else 
	{
		tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_SEND_COMMAND_FAILED, 
			_T( "发送远程控制脕EО? ) ); 
		show_msg( NULL, tmp_text ); 
	}

	return ERROR_SUCCESS; 
}

typedef struct _smtp_erc_info
{
	PVOID param; 
	smtp_erc_ret_callback erc_return_notify; 
	ULONG mode; 
	ULONG hour; 
	ULONG minite; 
	PVOID context; 
} smtp_erc_info, *psmtp_erc_info;

HANDLE smtp_erc_thread = NULL; 
HANDLE smtp_erc_event = NULL; 
INT32 stop_smtp_erc_cmd = FALSE; 
smtp_erc_info erc_info = { 0 };  

ULONG CALLBACK thread_smtp_erc( PVOID param )
{
	smtp_erc_info *erc_info; 
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR err_msg[ MAX_PATH ]; 
	email_handler handler = { 0 }; 
	email_box cur_email; 
	MAILITEM email_item; 
#define MAX_ERC_CMD_LEN 260 
	CHAR ctrl_cmd[ MAX_ERC_CMD_LEN ]; 
	SOCKET sock = INVALID_SOCKET; 
	LARGE_INTEGER ctrl_time; 
	SOCKET soc = INVALID_SOCKET;
	ULONG smtp_ip; 
	ULONG try_time; 
	ULONG wait_ret; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCDNAME__ ) ); 

	ASSERT( param != NULL ); 

	erc_info = ( smtp_erc_info* )param; 
	ASSERT( erc_info->hour <= 24 ); 

	//cur_email = &email_box_info; 

	email_item.Attach = NULL; 
	email_item.Bcc = NULL; 

	ctrl_time.QuadPart = ( ULONGLONG )( ( ( ( ULONGLONG )erc_info->hour * 3600 ) + ( ( ULONGLONG )erc_info->minite * 60 ) ) * 10000000 ); 

	if( erc_info->mode == REMOTE_CONTROL_BLOCK )
	{
		_snprintf( ctrl_cmd, 
			MAX_ERC_CMD_LEN, 
			"%s%s %I64u", 
			REMOTE_CTRL_BODY_BEGIN, 
			BLOCK_ALL_COMMAND, 
			ctrl_time.QuadPart ); 
	}
	else if( erc_info->mode == REMOTE_CONTROL_ALLOW )
	{
		_snprintf( ctrl_cmd, 
			MAX_ERC_CMD_LEN, 
			"%s%s %I64u", 
			REMOTE_CTRL_BODY_BEGIN, 
			ALLOW_ALL_COMMAND, 
			ctrl_time.QuadPart ); 
	}
	else if( erc_info->mode == REMOTE_CONTROL_ACL )
	{
		_snprintf( ctrl_cmd, 
			MAX_ERC_CMD_LEN, 
			"%s%s %I64u", 
			REMOTE_CTRL_BODY_BEGIN, 
			ACL_CTRL_COMMAND, 
			ctrl_time.QuadPart ); 
	}
	else
	{
		ASSERT( FALSE && "invalid remote control mode" ); 
	}

	email_item.Body = ( CHAR* )ctrl_cmd; 
	email_item.BodyCharset = _T( "gb2312" ); //_T( "UNICODE" ); 
	email_item.BodyEncoding = 2; 
	email_item.Cc = NULL; //_T( "balancesoft@sina.com" ); 
	email_item.ContentType = NULL; 
	email_item.Date = NULL; //_T( "2011-12-06" ); 
	email_item.Download = TRUE; 
	email_item.Encoding = NULL; 
	email_item.HeadCharset = _T( "gb2312" ); 
	email_item.HeadEncoding = 2; 
	email_item.Indent = 0; 
	email_item.hEditWnd = NULL; 
	email_item.hProcess = NULL; 
	email_item.InReplyTo = NULL; 
	email_item.MailBox = cur_email.email; //_T( "balancesoft@sohu.com" ); 
	email_item.MailStatus = 0; 
	email_item.MessageID = NULL; 
	email_item.Multipart = TRUE; 
	email_item.New = FALSE; 
	email_item.NextNo = 0; 
	email_item.No = 0; 
	email_item.PrevNo = 0; 
	email_item.References = NULL; 
	email_item.ReplyTo = NULL; 
	email_item.Size = NULL; 
	email_item.Status = 0; 
	email_item.Subject = _T( REMOTE_CTRL_SUBJECT ); 
	email_item.To = cur_email.email; //_T( "balancesoft@sohu.com" ); 
	email_item.UIDL = NULL; 

	//init_test_email_box( &cur_email ); 

	//handler.send_msg_id = TRUE; 
	//handler.cur_email = cur_email; 
	//handler.auth_flag = TRUE; 

	ret = get_mail_box( &cur_email ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	handler.cur_email = &cur_email;
	handler.send_mail_item = &email_item;
	handler.auth_flag = FALSE;
	handler.starttls_flag = FALSE;
	handler.send_end_cmd = SMTP_SENDEND;

	if( ( email_item.To == NULL || *email_item.To == TEXT('\0' ) ) &&
		( email_item.Cc == NULL || *email_item.Cc == TEXT('\0' ) ) &&
		( email_item.Bcc == NULL || *email_item.Bcc == TEXT( '\0' ) ) ) 
	{
		lstrcpy( err_msg, STR_ERR_SOCK_NOTO );
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = get_host_by_name( &handler, cur_email.server, &smtp_ip, err_msg );

	if( ret != ERROR_SUCCESS ) 
	{
		goto _return; 
	}

	ASSERT( smtp_ip != SOCKET_ERROR ); 

	if( stop_smtp_erc_cmd == TRUE )
	{
		ret = ERROR_STOP_THREAD; 
		goto _return; 
	}

	ret = connect_server( &handler,
		smtp_ip, 
		( USHORT )cur_email.port,
		( cur_email.use_ssl == FALSE || cur_email.ssl_info.Type == 4 ) ? 
		-1 : cur_email.ssl_info.Type,
		&cur_email.ssl_info,
		&soc, 
		err_msg );

	if( ret != ERROR_SUCCESS ) 
	{
		goto _return; 
	}

	ASSERT( smtp_ip != SOCKET_ERROR ); 

	try_time = 0; 
	for( ; ; )
	{
		wait_ret = WaitForSingleObject( smtp_erc_event, RECVTIME ); 
		if( wait_ret == WAIT_OBJECT_0 )
		{
			continue; 
		}
		else
		{
			ASSERT( wait_ret == WAIT_TIMEOUT ); 
		}

		if( stop_smtp_erc_cmd == TRUE )
		{
			ret = ERROR_STOP_THREAD; 
			goto _return; 
		}

		ret = recv_select( &handler, soc, smtp_proc ); 

		switch( ret ) 
		{
		case ERROR_NOT_ENOUGH_MEMORY:
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_MEMALLOC ) );
			goto _return; 
			break;

		case ERROR_SOCKET_ERROR:
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_SOCK_SELECT ) ); 
			goto _return; 
			break;

		case ERROR_SOCKET_CLOSED:
			if( handler.command_status != POP_QUIT ) 
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", "socket is connected when the action have not done"	) );
			} 
			else 
			{
				socket_close( &handler, soc ); 
				soc = INVALID_SOCKET; 
				ret = ERROR_SUCCESS; 
			}

			goto _return; 
			break;

		case ERROR_SUCCESS:
			{
				//CHAR *buf; 
				//ULONG buf_len; 

				//get_old_recv_buf( &buf, &buf_len ); 
				//dump_mem( buf, buf_len );

				//get_recv_buf( &buf, &buf_len ); 
				//dump_mem( buf, buf_len ); 

				//ret = smtp_proc( handler, soc, buf, buf_len, err_msg, TRUE ); 

				//if( ret != ERROR_SUCCESS )
				//{
				//	goto _return; 
				//}
			}
			break;

		case ERROR_SOCKET_NO_RECVED:
			{
				try_time += 1; 

				if( try_time == SMTP_RETRY_TIME )
				{
					ret = ERROR_TIMEOUT; 
					goto _return; 
				}

				break; 
			}
		default:
			log_trace( ( MSG_ERROR, "select receive state of the seocket failed 0x%0.8x\n", ret ) ); 
			goto _return; 
			break; 
		}
	}

_return:

	if( ret != ERROR_STOP_THREAD )
	{
		if( erc_info->erc_return_notify != NULL )
		{
			erc_info->erc_return_notify( ret, erc_info->param ); ; 
		}
	}

	if( soc != INVALID_SOCKET )
	{
		socket_close( &handler, soc ); 
		//soc = INVALID_SOCKET;
	}
	return ( ULONG )ret; 
}

LRESULT smtp_remote_control_async( ULONG mode, ULONG hour, ULONG minite, PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG wait_ret; 
	LPCTSTR tmp_text; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	//lock_cs( mail_box_lock ); 
	ret = _check_email_conf_valid( &email_box_info ); 
	//unlock_cs( mail_box_lock ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "email account for remote control invalid\n" ) ); 
		goto _return; 
	}

	//erc_info = malloc( sizeof( *erc_info ) ); 
	//if( erc_info == NULL )
	//{
	//	ret = ERROR_NOT_ENOUGH_MEMORY; 
	//	goto _return; 
	//}

	erc_info.context = context; 
	erc_info.mode = mode; 
	erc_info.hour = hour; 
	erc_info.minite = minite; 
	erc_info.param = param; 
	erc_info.erc_return_notify = smtp_erc_return_notify; 

	if( smtp_erc_thread != NULL )
	{
		wait_ret = WaitForSingleObject( smtp_erc_thread, 0 ); 
		if( wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_FAILED )
		{
			ASSERT( wait_ret == WAIT_TIMEOUT ); 

			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_SEND_COMMAND_NOT_IN_ORDER, 
				_T( "上次的远程脕E诜⑺椭?脕E荒芩承蚍⑺?" ) ); 
			show_msg( NULL, tmp_text ); 
			
			ret = ERROR_THREAD_NOT_READY; 
			goto _return; 
		}
		
		CloseHandle( smtp_erc_thread ); 
		smtp_erc_thread = NULL; 
	}

	if( smtp_erc_event == NULL )
	{
		smtp_erc_event = CreateEvent( NULL, FALSE, FALSE, NULL ); 
		if( smtp_erc_event == NULL )
		{
			ret = GetLastError(); 
			goto _return; 
		}
	}

	stop_smtp_erc_cmd = FALSE; 

	smtp_erc_thread = CreateThread( NULL, 
		0, 
		thread_smtp_erc, 
		&erc_info, 
		0, 
		NULL ); 

	if( smtp_erc_thread == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( ret != ERROR_THREAD_NOT_READY )
		{
			if( smtp_erc_thread != NULL )
			{
				stop_smtp_erc_cmd = TRUE; 

				if( smtp_erc_event != NULL )
				{
					SetEvent( smtp_erc_event ); 
				}

				wait_thread_end( smtp_erc_thread, 3000 ); 
				smtp_erc_thread = NULL; 
			}

			if( smtp_erc_event != NULL )
			{
				CloseHandle( smtp_erc_event ); 
				smtp_erc_event = NULL; 
			}
		}
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT stop_smtp_erc_ctrl()
{
	LRESULT ret = ERROR_SUCCESS; 

	if( smtp_erc_thread != NULL )
	{
		stop_smtp_erc_cmd = TRUE; 

		if( smtp_erc_event != NULL )
		{
			SetEvent( smtp_erc_event ); 
		}

		ret = wait_thread_end( smtp_erc_thread, 3000 ); 
		smtp_erc_thread = NULL; 
	}

	if( smtp_erc_event != NULL )
	{
		CloseHandle( smtp_erc_event ); 
		smtp_erc_event = NULL; 
	}

	return ret; 
}

LRESULT test_erc_conf( email_box *mail_box )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG pop_ip; 
	TCHAR err_msg[ MAX_PATH ]; 
	email_handler *handler; 
	INT32 try_time; 
	//ULONG wait_ret; 

	email_handler _handler = { 0 }; 
	email_box *cur_email; 
	MAILITEM email_item; 
	SOCKET sock = INVALID_SOCKET; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCDNAME__ ) ); 

	ASSERT( mail_box != NULL ); 
	ASSERT( check_email_conf_valid( mail_box ) == ERROR_SUCCESS ); 

	cur_email = mail_box; 

	email_item.Attach = NULL; 
	email_item.Bcc = NULL; 
	email_item.Body = ( CHAR* )""; 
	email_item.BodyCharset = _T( "gb2312" ); //_T( "UNICODE" ); 
	email_item.BodyEncoding = 2; 
	email_item.Cc = NULL; //_T( "balancesoft@sina.com" ); 
	email_item.ContentType = NULL; 
	email_item.Date = NULL; //_T( "2011-12-06" ); 
	email_item.Download = TRUE; 
	email_item.Encoding = NULL; 
	email_item.HeadCharset = _T( "gb2312" ); 
	email_item.HeadEncoding = 2; 
	email_item.Indent = 0; 
	email_item.hEditWnd = NULL; 
	email_item.hProcess = NULL; 
	email_item.InReplyTo = NULL; 
	email_item.MailBox = cur_email->email; //_T( "balancesoft@sohu.com" ); 
	email_item.MailStatus = 0; 
	email_item.MessageID = NULL; 
	email_item.Multipart = TRUE; 
	email_item.New = FALSE; 
	email_item.NextNo = 0; 
	email_item.No = 0; 
	email_item.PrevNo = 0; 
	email_item.References = NULL; 
	email_item.ReplyTo = NULL; 
	email_item.Size = NULL; 
	email_item.Status = 0; 
	email_item.Subject = _T( "" ); 
	email_item.To = cur_email->email; //_T( "balancesoft@sohu.com" ); 
	email_item.UIDL = NULL; 

	//init_test_email_box( &cur_email ); 

	_handler.send_msg_id = TRUE; 
	_handler.cur_email = cur_email; 
	_handler.auth_flag = TRUE; 
	_handler.command_handler = NULL; //execute_remote_command_by_email; 
	_handler.command_check = NULL; 

	handler = &_handler; 

	ret = smtp_test_login( handler, cur_email, &email_item, SMTP_SENDEND, err_msg ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = get_host_by_name( handler, cur_email->pop3_server, &pop_ip, err_msg ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ASSERT( pop_ip != SOCKET_ERROR ); 

	ret = connect_server( handler,
		pop_ip, 
		( USHORT )cur_email->pop3_port,
		cur_email->pop_use_ssl == 0 
		|| ( cur_email->pop_ssl_info.Type == 4 ) ? 
		-1 : cur_email->pop_ssl_info.Type,
		&cur_email->pop_ssl_info,
		&sock, 
		err_msg ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ASSERT( sock != INVALID_SOCKET ); 

	_handler.command_status = POP_START; 

	try_time = 0; 

	for( ; ; )
	{
		Sleep( RECVTIME ); 

		ret = recv_select( handler, sock, pop3_test_login_proc ); 
		switch( ret ) 
		{
		case ERROR_NOT_ENOUGH_MEMORY:
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_MEMALLOC ) );
			goto _return; 
			break;

		case ERROR_SOCKET_ERROR:
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_SOCK_SELECT ) ); 
			goto _return; 
			break;

		case ERROR_SOCKET_CLOSED:
			if( handler->command_status != POP_QUIT ) 
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", "socket is connected when the action have not done"	) );
			} 
			else 
			{
				socket_close( handler, sock ); 
				sock = INVALID_SOCKET; 
				ret = ERROR_SUCCESS; 
			}

			goto _return; 
			break;

		case ERROR_SUCCESS:
			{
			}
			break;

		case ERROR_SOCKET_NO_RECVED:
			{
				try_time += 1; 

				if( try_time == RETRY_TIME )
				{
					ret = ERROR_TIMEOUT; 
					goto _return; 
				}

				break; 
			}
		default:
			log_trace( ( MSG_ERROR, "select receive state of the seocket failed 0x%0.8x\n", ret ) ); 
			goto _return; 
			break; 
		}
	}

_return:
	return ret; 
}

LRESULT check_email_conf_valid( email_box *mail_box )
{
	LRESULT ret = ERROR_ERRORS_ENCOUNTERED; 

	ASSERT( mail_box != NULL ); 

	if( mail_box->erc_on != FALSE 
		&& mail_box->erc_on != TRUE )
	{
		log_trace( ( MSG_INFO, "erc on is binary option how can set it to no 0 or 1?" ) ); 
		goto _return; 
	}

	if( *mail_box->email == _T( '\0' ) )
	{
		log_trace( ( MSG_INFO, "EMAIL帐号必衼E柚? ) ); 
		goto _return; 
	}

	if( wcsnlen( mail_box->email, MAX_EMAIL_NAME_LEN - 1 ) >= MAX_EMAIL_NAME_LEN - 1 )
	{
		log_trace( ( MSG_INFO, "EMAIL帐号长度不得大于260" ) ); 
		goto _return; 
	}

	if( _tcschr( mail_box->email, _T( '@' ) ) == NULL )
	{
		log_trace( ( MSG_INFO, "EMAIL帐号格式不正确" ":%s", mail_box->email ) );
		goto _return; 
	}

	//if( *mail_box->name == _T( '\0' ) )
	//{
	//	log_trace( ( MSG_INFO, "用户名必衼E柚? ) ); 
	//	goto _return; 
	//}

	if( wcsnlen( mail_box->name, MAX_EMAIL_NAME_LEN - 1 ) >= MAX_EMAIL_NAME_LEN - 1 )
	{
		log_trace( ( MSG_INFO, "用户名长度不得大于260" ) ); 
		goto _return; 
	}

	if( *mail_box->pop3_server == _T( '\0' ) )
	{
		log_trace( ( MSG_INFO, "SMTP邮件服务器名必衼E柚? ) ); 
		goto _return; 
	}

	if( wcsnlen( mail_box->server, MAX_EMAIL_NAME_LEN - 1 ) >= MAX_EMAIL_NAME_LEN - 1 )
	{
		log_trace( ( MSG_INFO, "SMTP邮件服务器名长度不得大于260" ) ); 
		goto _return; 
	}

	if( *mail_box->pop3_server == _T( '\0' ) )
	{
		log_trace( ( MSG_INFO, "POP3邮件服务器名必衼E柚? ) ); 
		goto _return; 
	}

	if( wcsnlen( mail_box->pop3_server, MAX_EMAIL_NAME_LEN - 1 ) >= MAX_EMAIL_NAME_LEN - 1 )
	{
		log_trace( ( MSG_INFO, "POP3邮件服务器名长度不得大于260" ) ); 
		goto _return; 
	}

	if( *mail_box->pwd == _T( '\0' ) )
	{
		log_trace( ( MSG_INFO, "密聛E匦丒柚? ) ); 
		goto _return; 
	}

	if( wcsnlen( mail_box->pwd, MAX_EMAIL_NAME_LEN - 1 ) >= MAX_EMAIL_NAME_LEN - 1 )
	{
		log_trace( ( MSG_INFO, "密聛Eざ炔坏么笥?60" ) ); 
		goto _return; 
	}

	if( mail_box->port == 0 )
	{
		log_trace( ( MSG_INFO, "SMTP端口必衼E柚? ) ); 
		goto _return; 
	}

	if( mail_box->pop3_port == 0 )
	{
		log_trace( ( MSG_INFO, "POP3端口必衼E柚? ) ); 
	}

	ret = ERROR_SUCCESS; 

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCDNAME__, ret ) ); 

	return ret; 
}

LRESULT _check_email_conf_valid( email_box *mail_box )
{
	LRESULT ret; 

	lock_cs( mail_box_lock ); 
	ret = check_email_conf_valid( mail_box ); 
	unlock_cs( mail_box_lock ); 

	return ret; 
}

#include "tinyxml.h"
#define BITSAFE_ERC_CONF_FILE _T( "data6.dat" )

#define ERC_CONF "ERC_config"

#define EMAIL_CONF "email_account"
#define EMAIL_BOX_NAME "name"
#define EMAIL_BOX_EMAIL "email"
#define EMAIL_BOX_SMTP_SERVER "smtp_svr"
#define EMAIL_BOX_SMTP_PORT "smtp_port"
#define EMAIL_BOX_POP3_SERVER "pop3_svr"
#define EMAIL_BOX_POP3_PORT "pop3_port"
#define EMAIL_BOX_PWD "pwd"
#define EMAIL_BOX_ERC_ON "erc_on"

LRESULT get_erc_conf_file_name( WCHAR *conf_file_path, ULONG name_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	ASSERT( conf_file_path != NULL ); 
	ASSERT( name_len > 0 ); 

	_ret = GetSystemDirectory( conf_file_path, name_len ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	wcsncat( conf_file_path, L"\\", name_len - wcslen( conf_file_path ) );

	//if( conf_file_path[ name_len - 1 ] != L'\0' )
	//{
	//	//ASSERT( FALSE ); 
	//	ret = ERROR_BUFFER_TOO_SMALL; 
	//	goto _return; 
	//}

	wcsncat( conf_file_path, BITSAFE_ERC_CONF_FILE, name_len - wcslen( conf_file_path ) );

	if( conf_file_path[ name_len - 1 ] != L'\0' )
	{
		conf_file_path[ name_len - 1 ] = L'\0'; 
		//ASSERT( FALSE ); 
		//ret = ERROR_BUFFER_TOO_SMALL; 
		//goto _return; 
	}

	log_trace( ( MSG_INFO, "user configure file name is %ws\n", conf_file_path ) ); 

_return:
	return ret; 
}

LRESULT load_erc_conf()
{
	LRESULT ret = ERROR_SUCCESS; 

	TiXmlDocument xml_doc; 
	TiXmlElement *root = NULL; 
	TiXmlNode *node = NULL; 
	TiXmlElement *email_conf = NULL; 
	TCHAR *tmp_str = NULL; 
	TCHAR erc_file_name[ MAX_PATH ]; 
	CHAR *file_name = NULL; 
	LPCSTR attr_val; 
	bool _ret; 

	lock_cs( mail_box_lock ); 
	ret = get_erc_conf_file_name( erc_file_name, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_file_exist( erc_file_name ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( ret == ERROR_FILE_NOT_FOUND ); 
		goto _return; 
	}

	file_name = alloc_tchar_to_char( erc_file_name ); 
	if( file_name == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_ret = xml_doc.LoadFile( file_name ); 
	if( _ret == false )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	root = xml_doc.RootElement(); 
	if( root == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	node = root->FirstChild( EMAIL_CONF ); 
	if( node == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	email_conf = node->ToElement(); 
	
	tmp_str = erc_file_name; 

	attr_val = email_conf->Attribute( EMAIL_BOX_NAME ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	char_to_tchar( attr_val, tmp_str, MAX_PATH ); 
	_tcsncpy( email_box_info.name, tmp_str, MAX_PATH ); 

	attr_val = email_conf->Attribute( EMAIL_BOX_EMAIL ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	char_to_tchar( attr_val, tmp_str, MAX_PATH ); 
	_tcsncpy( email_box_info.email, tmp_str, MAX_PATH ); 

	attr_val = email_conf->Attribute( EMAIL_BOX_PWD ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	char_to_tchar( attr_val, tmp_str, MAX_PATH ); 
	_tcsncpy( email_box_info.pwd, tmp_str, MAX_PATH ); 

	attr_val = email_conf->Attribute( EMAIL_BOX_SMTP_SERVER ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	char_to_tchar( attr_val, tmp_str, MAX_PATH ); 
	_tcsncpy( email_box_info.server, tmp_str, MAX_PATH ); 

	INT32 port_val; 
	attr_val = email_conf->Attribute( EMAIL_BOX_SMTP_PORT, &port_val ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	email_box_info.port = port_val; 

	attr_val = email_conf->Attribute( EMAIL_BOX_POP3_SERVER ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}
	
	char_to_tchar( attr_val, tmp_str, MAX_PATH ); 
	_tcsncpy( email_box_info.pop3_server, tmp_str, MAX_PATH ); 

	attr_val = email_conf->Attribute( EMAIL_BOX_POP3_PORT, &port_val ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	email_box_info.pop3_port = port_val; 

	attr_val = email_conf->Attribute( EMAIL_BOX_ERC_ON, &port_val ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	email_box_info.erc_on = port_val; 

	xml_doc.Print(); 

_return:
	unlock_cs( mail_box_lock ); 

	if( file_name != NULL )
	{
		mem_free( ( VOID** )&file_name ); 
	}

	if( ret != ERROR_SUCCESS )
	{
		memset( &email_box_info, 0, sizeof( email_box_info ) ); 
	}
	return ret; 
}

LRESULT save_erc_conf()
{
	TiXmlDocument xml_doc; 
	TiXmlDeclaration Declaration( "1.0","gb2312", "yes" ); 
	TiXmlElement *root = NULL; 
	TiXmlNode *node = NULL; 
	TiXmlElement *email_conf = NULL; 
	CHAR* tmp_str = NULL; 

	LRESULT ret = ERROR_SUCCESS; 
	TCHAR erc_file_name[ MAX_PATH ]; 
	CHAR *file_name = NULL; 
	//bool _ret; 

	lock_cs( mail_box_lock ); 

	ret = check_email_conf_valid( &email_box_info ); 
	if( ret != ERROR_SUCCESS ) 
	{
		ASSERT( FALSE && "how can send the invalid email box to here" ); 
		goto _return; 
	}

	//_ret = xml_doc.LoadFile( file_name ); 
	//if( _ret == false )
	//{
	//	ret = ERROR_ERRORS_ENCOUNTERED; 
	//	goto _return; 
	//}

	xml_doc.InsertEndChild( Declaration ); 

	//root = xml_doc.RootElement(); 
	//if( root == NULL )
	//{
		root = new TiXmlElement( ERC_CONF ); 
		if( root == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		node = xml_doc.InsertEndChild( *root ); 
		
		ASSERT( node != NULL ); 

		log_trace( ( MSG_INFO, "root is 0x%0.8x, root of node is 0x%0.8x\n", root, node->ToElement() ) ); 
		
		delete root; 

		root = node->ToElement(); 

		email_conf = new TiXmlElement( EMAIL_CONF ); 

		if( email_conf == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		node = root->InsertEndChild( *email_conf ); 
		if( node == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; ; 
		}

		delete email_conf; 

		email_conf = node->ToElement(); 
	//}
	//else
	//{
	//	node = root->FirstChild( EMAIL_CONF ); 
	//	if( node == NULL )
	//	{
	//		root->Clear(); 
	//	}

	//	email_conf = new TiXmlElement( EMAIL_CONF ); 

	//	if( email_conf == NULL )
	//	{
	//		ret = ERROR_NOT_ENOUGH_MEMORY; 
	//		goto _return; 
	//	}

	//	node = root->InsertEndChild( *email_conf ); 

	//	email_conf = node->ToElement(); 
	//}

	tmp_str = ( CHAR* )erc_file_name; 

	tchar_to_char( email_box_info.name, tmp_str, MAX_PATH ); 
	email_conf->SetAttribute( EMAIL_BOX_NAME, tmp_str ); 

	tchar_to_char( email_box_info.email, tmp_str, MAX_PATH ); 
	email_conf->SetAttribute( EMAIL_BOX_EMAIL, tmp_str ); 

	tchar_to_char( email_box_info.pwd, tmp_str, MAX_PATH ); 
	email_conf->SetAttribute( EMAIL_BOX_PWD, tmp_str ); 

	tchar_to_char( email_box_info.server, tmp_str, MAX_PATH ); 
	email_conf->SetAttribute( EMAIL_BOX_SMTP_SERVER, tmp_str ); 

	email_conf->SetAttribute( EMAIL_BOX_SMTP_PORT, email_box_info.port ); 

	tchar_to_char( email_box_info.pop3_server, tmp_str, MAX_PATH ); 
	email_conf->SetAttribute( EMAIL_BOX_POP3_SERVER, tmp_str ); 

	email_conf->SetAttribute( EMAIL_BOX_POP3_PORT, email_box_info.pop3_port ); 

	email_conf->SetAttribute( EMAIL_BOX_ERC_ON, email_box_info.erc_on ); 

	ret = get_erc_conf_file_name( erc_file_name, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	file_name = alloc_tchar_to_char( erc_file_name ); 
	if( file_name == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	xml_doc.Print(); 
	xml_doc.SaveFile( file_name ); 

_return:
	unlock_cs( mail_box_lock ); 

	if( file_name != NULL )
	{
		mem_free( ( void** )&file_name ); 
	}

	return ret; 
}

#define ERROR_TIP_ERC_CONFIG_CRUPPTED _T( "加载邮件远程控制配置失败,可能是配置数据破坏,莵E匦陆信渲?" ) 

LRESULT init_erc_env( PVOID context, INT32 have_ui )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR tmp_text; 
	dlg_ret_state ret_status; 
	//remote_conf_dlg dlg( context ); 

	init_cs_lock( mail_box_lock ); 
	memset( &email_box_info, 0, sizeof( email_box_info ) ); 

	ret = init_socket_env(); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = load_erc_conf(); 
	if( ret != ERROR_SUCCESS )
	{
		if( ret != ERROR_FILE_NOT_FOUND )
		{
			if( have_ui == TRUE )
			{
				tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_LOAD_ERC_CONFIG_FAILED, 
					ERROR_TIP_ERC_CONFIG_CRUPPTED ); 
				ret = show_msg( NULL, tmp_text, &ret_status ); 

				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				if( ret_status == OK_STATE )
				{
					//dlg.Create( NULL, _T(""), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					//dlg.SetIcon( IDI_MAIN_ICON ); 
					//dlg.CenterWindow();
					//dlg.ShowModal(); 
					set_erc_config_now(); 
				}
			}
		}
		goto _return; 
	}

	ret = check_email_conf_valid( &email_box_info ); 
	if( ret != ERROR_SUCCESS )
	{
		if( have_ui == TRUE )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_LOAD_ERC_CONFIG_FAILED, 
				ERROR_TIP_ERC_CONFIG_CRUPPTED ); 

			ret = show_msg( NULL, tmp_text, &ret_status ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			if( ret_status == OK_STATE )
			{
				//dlg.Create( NULL, _T(""), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
				//dlg.SetIcon( IDI_MAIN_ICON ); 
				//dlg.CenterWindow();
				//dlg.ShowModal(); 

				set_erc_config_now(); 
			}
		}
		goto _return; 
	}

	if( email_box_info.erc_on == TRUE )
	{
		LRESULT _ret; 
		_ret = pop3_exec_remote_control( NULL ); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "start the pop erc function failed\n" ) ); 
		}
	}

	//for( ; ; )
	//{
	//	thread_exec_remote_control( NULL ); 
	//	Sleep( 1000 ); 
	//}

	//smtp_remote_control( REMOTE_CONTROL_ALLOW, NULL ); 

_return:
	return ret; 
}

VOID uninit_erc_env()
{
	uninit_socket_env(); 

#ifdef _DEBUG
	{
		LRESULT ret; 
		lock_cs( mail_box_lock ); 
		ret = check_email_conf_valid( &email_box_info ); 
		unlock_cs( mail_box_lock ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "mail box not setting\n" ) );
		}
	}
#endif //_DEBUG

	stop_pop3_remote_control(); 

	stop_smtp_erc_ctrl(); 

	uninit_cs_lock( mail_box_lock ); 
}
