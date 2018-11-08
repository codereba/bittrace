#include "stdafx.h"
#include <exdisp.h>
#include <shellapi.h>
#include "bitsafe_common.h"
#include "bitsafe.h"
#include "proc_list_dlg.h"
#include "log_dlg.h"
#include "about_dlg.h"
#include "resource.h"
#include "ui_ctrl_mgr.h"
#include "anti_arp_dlg.h"
#include "netmon_dlg.h"
#include "msg_box.h"
#include "login_dlg.h"
#include "trust_block_dlg.h"
#include "fw_event_dlg.h"
#include "defense_event_dlg.h"
#include "setting_dlg.h"
#include "chg_mode_dlg.h"
#include "chg_pwd_dlg.h"
#include "conf_file.h"
#include "one_instance.h"
#include "update.h"
#include "UIUtil.h"
#include "bitsafe_rule_conf.h"
#include "bitsafe_conf.h"
#include "update_dlg.h"
#include "service.h"
#include "bitsafe_daemon.h"
#include "tray_icon.h"
#include "icon_menu.h"
#include "msg_tip.h"
#include "netcfg_dlg.h"
#include "crack360_dlg.h"
#include "remote_conf_dlg.h"
#include "icmp_dlg.h"
#include "lang_dlg.h"
#include "suggestion_dlg.h"
#include "donate_dlg.h"
#include "ui_custom.h"
#include "dbg_panel.h"
//#include "slogan_box.h"
#include "window_effective.h"
#include "option_dlg.h"
#include "runtime_msg_box.h"
#include "install_conf.h"
#include "volume_name_map.h"
#include "action_log_db.h"
#include "action_logger.h"

#include <dbt.h>

#ifdef _DEBUG
#include "ui_ctrl.h"
#endif //_DEBUG

//#ifdef _DEBUG
//#include "erc_setting.h"
//#endif //_DEBUG

//#ifdef _DEBUG
//#define JUST_DEBUG_CLIENT
#define NO_CRASH_REPORT
//#endif //_DEBUG

#define HAVE_LOG_MONITOR
#define WM_TRAY_ICON_NOTIFY ( WM_USER + 101 )
#define WM_RESPONSE_EVENT_NOTIFY ( WM_USER + 102 )
#define WM_SLOGAN_NOTIFY ( WM_USER + 103 )
#define TRAY_ICON_ID 102
#define DRIVER_TEST
#define INIT_DRIVER_THREAD

//#define UI_FILE_NAME _T( "SevenFw.zip" )

//LRESULT shape_window()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HANDLE handle = NULL; 
//
//	GenerateBitmap(); 
//
//	return ret; 
//}

LRESULT CALLBACK update_ui( PVOID context ); 

LRESULT check_run_service()
{
	LRESULT ret = ERROR_SUCCESS; 
	CHAR exe_file_name[ MAX_PATH ]; 
	CHAR svc_cmd_line[ MAX_CMD_LINE_LEN ]; 
	ULONG file_name_len; 

	//#ifdef _DEBUG
	//	stop_service( SERVICE_NAME ); 
	//	uninstall_service( SERVICE_NAME ); 
	//#endif //_DEBUG

#ifdef _DEBUG
	dump_service_info( _T( SERVICE_NAME ) ); 
#endif //_DEBUG

	file_name_len = GetModuleFileNameA( NULL, exe_file_name, _MAX_PATH ); 

	if( file_name_len == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	strcpy( svc_cmd_line, "\"" ); 
	strncat( svc_cmd_line, exe_file_name, MAX_CML_LINE_LEN - strlen( svc_cmd_line ) ); 
	strncat( svc_cmd_line, "\"", MAX_CML_LINE_LEN - strlen( svc_cmd_line ) ); 
	strncat( svc_cmd_line, " /s", MAX_CML_LINE_LEN - strlen( svc_cmd_line ) ); 

	ret = check_service_exist( SERVICE_NAME ); 
	if( ret == ERROR_SERVICE_DOES_NOT_EXIST )
	{
		ret = install_service( svc_cmd_line, SERVICE_NAME );
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = start_service( SERVICE_NAME, 0, NULL ); 
	} 
	else if( ret == ERROR_SUCCESS )
	{

#ifndef JUST_DEBUG_CLIENT
		ret = auto_update_service_binary_path( SERVICE_NAME, svc_cmd_line ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return;
		}
#endif //JUST_DEBUG_CLIENT

		ret = start_service( SERVICE_NAME, 0, NULL ); 
	}
	else
	{
		ASSERT( FALSE ); 
	}

_return:
	return ret; 
}; 

DWORD CALLBACK thread_test_rich_edit( PVOID context )
{
	log_dlg *sys_log_dlg; 
	ULONG all_writed; 
	//INT32 i; 

	//DBG_BP(); 

	sys_log_dlg = ( log_dlg* )context; 

#define TEST_RICH_EDIT_TEXT _T( "testtesttesttest" )
	all_writed = 0; 
	for( ; ; )
	{
		sys_log_dlg->SendMessage( WM_SYS_LOG_OUTPUT, MODIFY_FILE, ( LPARAM )TEST_RICH_EDIT_TEXT ); 
		//sys_log_dlg->output_log( TEST_RICH_EDIT_TEXT, ); 
		all_writed += sizeof( TEST_RICH_EDIT_TEXT ) - sizeof( WCHAR ); 
		log_trace( ( MSG_INFO, "rich edit inputed length is %u\n", all_writed ) ); 

		Sleep( 2 ); 
	}

	return 0; 
}


//class action_log_notify
//{
//public:
//	virtual LRESULT log_notify( sys_action_desc *action ) = 0; 
//}; 

class CBitSafeFrameWnd : public CWindowWnd, public INotifyUI, public action_logger, public action_ui_notify, public action_log_notify
{
public:
	CBitSafeFrameWnd() : download_x( 0 ), 
		upload_x( 0 ), 
		download_right_x( 0 ), 
		upload_right_x( 0 ), 
		logo_left( 0 ), 
		timer_id( 0 ), 
		icon_show_time( 0 ), 
		icon_showed_time( 0 )
		//logo_width( 0 )
	{ };
	
	LPCTSTR GetWindowClassName() const { return _T("UIMainFrame"); };
	UINT GetClassStyle() const { return CS_DBLCLKS; };
	void OnFinalMessage(HWND /*hWnd*/) { delete this; };

	
	INLINE LRESULT output_sys_log_ex( r3_action_notify *action )
	{
		LRESULT ret; // = ERROR_SUCCESS; 
		//#ifndef _UNICODE
		//		TCHAR unicode[ MAX_MSG_LEN ]; 
		//		ULONG need_len; 
		//#endif
		//WCHAR log_msg[ MAX_MSG_LEN ]; 

		ASSERT( action != NULL ); 

		do 
		{
			ret = add_log_ex( action ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE && "add sys action log failed" ); 
				log_trace( ( MSG_INFO, "add sys action log failed 0x%0.8x\n", ret ) ); 
			}

//			ret = get_event_msg_ex( action, log_msg, MAX_MSG_LEN, LOG_MSG ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				ASSERT( FALSE && "invalid sys action log" ); 
//				log_trace( ( MSG_INFO, "invalid sys action log\n" ) ); 
//				break; 
//			}
//
//			log_trace( ( MSG_INFO, "get message info %ws \n", log_msg ) ); 
//
//#ifdef HAVE_LOG_MONITOR
//			if( sys_log.GetHWND() != NULL && IsWindow( sys_log.GetHWND() ) )
//			{
//				//ret = get_event_msg( &msgs_cap->trace_logs[ i ], log_msg, MAX_MSG_LEN, LOG_MSG ); 
//				//if( ret != ERROR_SUCCESS )
//				//{
//				//	ASSERT( FALSE && "invalid sys action log" ); 
//				//	log_trace( ( MSG_INFO, "invalid sys action log\n" ) ); ; 
//				//}
//
//				//log_trace( ( MSG_INFO, "get message info %ws \n", log_msg ) ); 
//
//				sys_log.SendMessage( WM_SYS_LOG_OUTPUT, ( WPARAM )action->action.type, ( LPARAM )( PVOID )log_msg ); 
//				//sys_log.output_log( log_msg, msgs_cap->trace_logs[ i ].type ); 
//			}
//#endif //HAVE_LOG_MONITOR
		}while( FALSE );


		return ret; 
	}

	inline LRESULT output_sys_log( sys_action_output *action )
	{
		LRESULT ret; // = ERROR_SUCCESS; 
//#ifndef _UNICODE
//		TCHAR unicode[ MAX_MSG_LEN ]; 
//		ULONG need_len; 
//#endif
		WCHAR log_msg[ MAX_MSG_LEN + MAX_DATA_DUMP_LEN ]; 
		//WCHAR *data_dump = NULL; 
		
		ASSERT( action != NULL ); 

		do 
		{
			ret = add_log( &action->action ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE && "add sys action log failed" ); 
				log_trace( ( MSG_INFO, "add sys action log failed 0x%0.8x\n", ret ) ); 
			}

			//data_dump = ( WCHAR* )malloc( MAX_DATA_DUMP_LEN * sizeof( WCHAR ) ); 
			//if( data_dump == null )
			//{
			//	ret = ERROR_NOT_ENOUGH_MEMORY; 
			//	break; 
			//}

			ret = get_whole_event_msg_ex( action,  
				log_msg, 
				ARRAY_SIZE( log_msg ), 
				LOG_MSG ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			//ret = get_event_msg_ex( action, log_msg, MAX_MSG_LEN, LOG_MSG ); 
			//if( ret != ERROR_SUCCESS )
			//{
			//	ASSERT( FALSE && "invalid sys action log" ); 
			//	log_trace( ( MSG_INFO, "invalid sys action log\n" ) ); 
			//	break; 
			//}

			log_trace( ( MSG_INFO, "get message info %ws\n", 
				log_msg ) ); 

#ifdef HAVE_LOG_MONITOR
			if( sys_log.GetHWND() != NULL && IsWindow( sys_log.GetHWND() ) )
			{
				//ret = get_event_msg( &msgs_cap->trace_logs[ i ], log_msg, MAX_MSG_LEN, LOG_MSG ); 
				//if( ret != ERROR_SUCCESS )
				//{
				//	ASSERT( FALSE && "invalid sys action log" ); 
				//	log_trace( ( MSG_INFO, "invalid sys action log\n" ) ); ; 
				//}

				//log_trace( ( MSG_INFO, "get message info %ws \n", log_msg ) ); 

				sys_log.SendMessage( WM_SYS_LOG_OUTPUT, ( WPARAM )action->action.type, ( LPARAM )( PVOID )log_msg ); 
				//sys_log.output_log( log_msg, msgs_cap->trace_logs[ i ].type ); 
			}
#endif //HAVE_LOG_MONITOR
		}while( FALSE );


		return ret; 
	}

	LRESULT log_notify( sys_action_output *action )
	{
		LRESULT ret; 

		ASSERT( action != NULL ); 

		ret = output_sys_log( action ); 
	
		return ret; 
	}

	LRESULT log_notify_ex( r3_action_notify *action )
	{
		LRESULT ret; 

		ASSERT( action != NULL ); 

		ret = output_sys_log_ex( action ); 

		return ret; 
	}

	LRESULT _user_response_sys_event( sys_action_desc* sys_action, PVOID data, ULONG data_len, ULONG flags )
	{
		LRESULT ret = ERROR_SUCCESS; 

		//ret = SendMessage( WM_RESPONSE_EVENT_NOTIFY, NULL, ( LPARAM )( PVOID )action_event ); 

		WCHAR *action_msg; 
		WCHAR *data_dump = NULL; 
		//ACTION_RECORD_TYPE need_record; 
		//event_action_response event_respon; 
		//dlg_ret_state ret_status; 
		LPCTSTR tmp_text; 

		ASSERT( sys_action != NULL ); 
		ASSERT( is_valid_action_type( sys_action->type ) ); ; 

		action_msg = ( WCHAR* )malloc( MAX_MSG_LEN * sizeof( WCHAR ) ); 

		if( action_msg == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		data_dump = ( WCHAR* )malloc( MAX_DATA_DUMP_LEN * sizeof( WCHAR ) ); 

		if( data_dump == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		ret = _get_event_msg_ex( sys_action, 
			( BYTE* )data, 
			data_len, 
			action_msg, 
			MAX_MSG_LEN, 
			data_dump, 
			MAX_DATA_DUMP_LEN, 
			0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		tmp_text = _get_string_by_id( TEXT_SYS_ACTION_TIP_SPECIOUS_ACTION, _T( "被怀疑的系统行为" ) ); 

		ret = _show_event_tip( GetHWND()/*GetDesktopWindow()*/, 
			action_msg, 
			data_dump, 
			sys_action, 
			data, 
			data_len, 
			tmp_text, 
			0, 
			( action_log_notify* )this, 
			flags ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		//if( ret_status == OK_STATE )
		//{
		//	event_respon.action = ACTION_ALLOW; 
		//}
		//else
		//{
		//	event_respon.action = ACTION_BLOCK; 
		//}

		//ASSERT( TRUE == is_valid_action_type( need_record ) ); 
		//event_respon.need_record = ( BYTE )need_record; 
		//event_respon.id = action_event->id; 

		//ret = do_ui_ctrl( BITSAFE_UI_ID, SYS_MON_EVENT_RESPONSE, ( PBYTE )&event_respon, sizeof( event_respon ), NULL, 0, NULL, NULL ); 

		//if( ret == ERROR_SUCCESS )
		//{
		//	if( need_record == RECORD_APP_ACTION )
		//	{
		//		input_rule_from_action( action_event, 0 ); 
		//	}
		//	else if( need_record == RECORD_APP_ACTION_TYPE )
		//	{
		//		input_rule_from_action( action_event, APP_DEFINE_RULE ); 
		//	}
		//}
_return:
		if( data_dump != NULL )
		{
			free( data_dump ); 
		}

		if( action_msg != NULL )
		{
			free( action_msg ); 
		}

		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
		return ret; 
	}

	LRESULT user_response_sys_event( sys_action_output* action_event, ULONG length, ULONG flags )
	{
		LRESULT ret = ERROR_SUCCESS; 

		//ret = SendMessage( WM_RESPONSE_EVENT_NOTIFY, NULL, ( LPARAM )( PVOID )action_event ); 

		WCHAR *action_msg; 
		WCHAR * data_dump = NULL; 
		//ACTION_RECORD_TYPE need_record; 
		//event_action_response event_respon; 
		//dlg_ret_state ret_status; 
		LPCTSTR tmp_text; 

		ASSERT( action_event != NULL ); 
		ASSERT( is_valid_action_type( action_event->action.type ) ); ; 

		action_msg = ( WCHAR* )malloc( MAX_MSG_LEN * sizeof( WCHAR ) ); 

		if( action_msg == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		data_dump = ( WCHAR* )malloc( MAX_DATA_DUMP_LEN * sizeof( WCHAR ) ); 
		if( data_dump == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return;
		}

		ret = get_event_msg_ex( action_event, 
			action_msg, 
			MAX_MSG_LEN, 
			data_dump, 
			MAX_DATA_DUMP_LEN, 
			0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		tmp_text = _get_string_by_id( TEXT_SYS_ACTION_TIP_SPECIOUS_ACTION, _T( "被怀疑的系统行为" ) ); 

		ret = show_event_tip( GetHWND()/*GetDesktopWindow()*/, 
			action_msg, 
			data_dump, 
			action_event, 
			tmp_text, 
			0, 
			( action_log_notify* )this, 
			flags ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		//if( ret_status == OK_STATE )
		//{
		//	event_respon.action = ACTION_ALLOW; 
		//}
		//else
		//{
		//	event_respon.action = ACTION_BLOCK; 
		//}

		//ASSERT( TRUE == is_valid_action_type( need_record ) ); 
		//event_respon.need_record = ( BYTE )need_record; 
		//event_respon.id = action_event->id; 

		//ret = do_ui_ctrl( BITSAFE_UI_ID, SYS_MON_EVENT_RESPONSE, ( PBYTE )&event_respon, sizeof( event_respon ), NULL, 0, NULL, NULL ); 

		//if( ret == ERROR_SUCCESS )
		//{
		//	if( need_record == RECORD_APP_ACTION )
		//	{
		//		input_rule_from_action( action_event, 0 ); 
		//	}
		//	else if( need_record == RECORD_APP_ACTION_TYPE )
		//	{
		//		input_rule_from_action( action_event, APP_DEFINE_RULE ); 
		//	}
		//}
_return:
		if( data_dump != NULL )
		{
			free( data_dump ); 
		}

		if( action_msg != NULL )
		{
			free( action_msg ); 
		}

		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
		return ret; 
	}

	LRESULT __user_response_sys_event( sys_action_info* action_event, PVOID data_buf, ULONG data_len, ULONG flags )
	{
		LRESULT ret = ERROR_SUCCESS; 

		//ret = SendMessage( WM_RESPONSE_EVENT_NOTIFY, NULL, ( LPARAM )( PVOID )action_event ); 

		WCHAR *action_msg; 
		WCHAR * data_dump = NULL; 
		//ACTION_RECORD_TYPE need_record; 
		//event_action_response event_respon; 
		//dlg_ret_state ret_status; 
		LPCTSTR tmp_text; 

		ASSERT( action_event != NULL ); 
		ASSERT( is_valid_action_type( action_event->action.type ) ); 

		action_msg = ( WCHAR* )malloc( MAX_MSG_LEN * sizeof( WCHAR ) ); 

		if( action_msg == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		data_dump = ( WCHAR* )malloc( MAX_DATA_DUMP_LEN * sizeof( WCHAR ) ); 
		if( data_dump == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return;
		}

		//ret = get_event_msg_ex( action_event, 
		//	action_msg, 
		//	MAX_MSG_LEN, 
		//	data_dump, 
		//	MAX_DATA_DUMP_LEN, 
		//	0 ); 

		ret = get_action_msg_ex( action_event, 
			data_buf, 
			data_len, 
			ACTION_ALLOW, 
			action_msg, 
			MAX_MSG_LEN, 
			data_dump, 
			MAX_DATA_DUMP_LEN, 
			0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		tmp_text = _get_string_by_id( TEXT_SYS_ACTION_TIP_SPECIOUS_ACTION, _T( "被怀疑的系统行为" ) ); 

		ret = __show_event_tip( GetHWND()/*GetDesktopWindow()*/, 
			action_msg, 
			data_dump, 
			action_event, 
			tmp_text, 
			0, 
			( action_log_notify* )this, 
			flags ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		//if( ret_status == OK_STATE )
		//{
		//	event_respon.action = ACTION_ALLOW; 
		//}
		//else
		//{
		//	event_respon.action = ACTION_BLOCK; 
		//}

		//ASSERT( TRUE == is_valid_action_type( need_record ) ); 
		//event_respon.need_record = ( BYTE )need_record; 
		//event_respon.id = action_event->id; 

		//ret = do_ui_ctrl( BITSAFE_UI_ID, SYS_MON_EVENT_RESPONSE, ( PBYTE )&event_respon, sizeof( event_respon ), NULL, 0, NULL, NULL ); 

		//if( ret == ERROR_SUCCESS )
		//{
		//	if( need_record == RECORD_APP_ACTION )
		//	{
		//		input_rule_from_action( action_event, 0 ); 
		//	}
		//	else if( need_record == RECORD_APP_ACTION_TYPE )
		//	{
		//		input_rule_from_action( action_event, APP_DEFINE_RULE ); 
		//	}
		//}
_return:
		if( data_dump != NULL )
		{
			free( data_dump ); 
		}

		if( action_msg != NULL )
		{
			free( action_msg ); 
		}

		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
		return ret; 
	}

#define ALERT_ICON_TIMER 0x1001
#define ALERT_ICON_ELAPSE 200
	INLINE LRESULT start_alert_icon()
	{
		LRESULT ret = ERROR_SUCCESS; 
		UINT_PTR _ret; 
		_ret = SetTimer( GetHWND(), ALERT_ICON_TIMER, ALERT_ICON_ELAPSE, NULL ); 
		if( _ret == 0 )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			goto _return; 
		}

		timer_id = ALERT_ICON_TIMER; 
#define ALERT_ICON_SHOW_TIME 20
		icon_show_time = ALERT_ICON_SHOW_TIME; 
		icon_showed_time = 0; 

_return:
		return ret; 
	}

	LRESULT on_timer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		LRESULT ret = ERROR_SUCCESS; 
		BOOL _ret; 

		switch( wParam )
		{
		case ALERT_ICON_TIMER:

			if( icon_showed_time >= icon_show_time )
			{
				icon_show_time = 0; 
				icon_showed_time = 0; 
				_ret = KillTimer( GetHWND(), ALERT_ICON_TIMER ); 
				
				if( FALSE == _ret )
				{
					ret = GetLastError(); 
					log_trace( ( DBG_MSG_AND_ERROR_OUT, "kill alert timer error" ) ); 
				}

				icon_tray.restore_main_icon(); 
				break; 
			}

			icon_tray.show_next_icon(); 
			icon_showed_time ++; 

			break; 
		default: 
			ASSERT( FALSE && "unknown timer" ); 
			break; 
		}
_return:
		return ret; 
	}

	LRESULT action_notify( action_ui_notify_type type, PVOID param )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *control; 
		CStdString tip; 
		notify_data *data_notify; 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		data_notify = ( notify_data* )param; 

		//DBG_BRK(); 

		switch( type )
		{
		case BLOCK_COUNT_NOTIFY:
			{
				all_block_count *block_count; 

				ASSERT( param != NULL ); 

				block_count = ( all_block_count* )data_notify->data; 
				if( data_notify->data_len != sizeof( all_block_count ) )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}; 

				control = m_pm.FindControl( _T( "fw_blk_tip" ) ); 
				ASSERT( control != NULL ); 

				tip.Format( _T( "%d" ), block_count->fw_block_ocunt ); 

				control->SetText( tip.GetData() ); 

				control = m_pm.FindControl( _T( "defense_blk_tip" ) ); 
				ASSERT( control != NULL ); 

				tip.Format( _T( "%d" ), block_count->defense_block_count ); 

				control->SetText( tip.GetData() ); 
			}
			break; 
		case ALL_NET_DATA_COUNT:
			{

				LARGE_INTEGER *all_traffic; 
				if( data_notify->data_len != sizeof( LARGE_INTEGER ) * 2 )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				all_traffic = ( LARGE_INTEGER* )data_notify->data; 

				log_trace( ( MSG_INFO, "set all traffic %I64u, %I64u \n", 
					all_traffic[ 0 ].QuadPart, 
					all_traffic[ 0 ].QuadPart ) ); 

				control = m_pm.FindControl( _T( "fw_upload" ) ); 
				ASSERT( control != NULL ); 

				tip.Format( _T( "%I64u" ), all_traffic[ 1 ].QuadPart ); 

				control->SetText( tip.GetData() ); 

				control = m_pm.FindControl( _T( "fw_download" ) ); 
				ASSERT( control != NULL ); 

				tip.Format( _T( "%I64u" ), all_traffic[ 0 ].QuadPart ); 

				control->SetText( tip.GetData() ); 
			}
			break; 
		case SYS_EVENT_NOTIFY: 
			{
				sys_action_output *sys_action; 

				sys_action = ( sys_action_output* )data_notify->data; 

				if( data_notify->data_len < sizeof( sys_action_output ) )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				ret = start_alert_icon(); 
				ret = user_response_sys_event( sys_action, data_notify->data_len, 0 ); 
			}
			break; 
		case SYS_EVENT_R3_NOTIFY: 
			{
				sys_action_and_data_output *sys_action; 
				//PVOID data_buf; 
				//sys_action_info_notify *sys_action; 
				//DBG_BP(); 
				sys_action = ( sys_action_and_data_output* )data_notify->data; 

				if( data_notify->data_len != sizeof( sys_action_and_data_output ) )
				//if( data_notify->data_len >= sizeof( sys_action_info_notify ) )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				ret = start_alert_icon(); 

				//data_buf = get_action_output_data( sys_action ); 

				ret = __user_response_sys_event( sys_action->action, 
					sys_action->data_buf, 
					sys_action->data_size, 
					R3_SYS_EVENT_RESPONSE ); 
			}
			break; 
		case SYS_LOG_NOTIFY:
			{
				sys_log_output *log_out; 
				//sys_action_output *log_out; 

				log_out = ( sys_log_output* )data_notify->data; 

				log_trace( ( MSG_INFO, "output sys log buffer length is %d, recorded message number is %d need length is %d\n", 
					data_notify->data_len, 
					log_out->size, 
					log_out->size * sizeof( sys_log_unit ) + sizeof( ULONG ) ) ); 

				//log_trace( ( MSG_INFO, "output sys log buffer length is %d, recorded length is %d\n", 
				//	data_notify->data_len, 
				//	log_out->size ) ); 

				if( data_notify->data_len < log_out->size * sizeof( sys_log_unit ) + sizeof( ULONG ) )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				ret = output_logs( log_out, data_notify->data_len ); 
			}
			break; 
		case WORK_MODE_NOTIFY:
			{
				work_mode mode; 
				CControlUI *ctrl; 
				LPCTSTR tmp_text; 

				//DBG_BRK(); 
				if( data_notify->data_len != sizeof( ULONG ) )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				mode = ( work_mode )*( PULONG )data_notify->data; 

				if( is_valid_work_mode( mode ) == FALSE )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

#define SAGE_STATE_IMAGE_FILE _T( "file='safe_state.png' dest='0,0,59,76' source='0,0,59,76' mask='0xffff00ff'" )
#define SAFE_STATE_DESC _T( "多功能保护已经开启,系统在受保护状态" )

#define UNSAGE_STATE_IMAGE_FILE _T( "file='unsafe_state.png' dest='0,0,59,76' source='0,0,59,76' mask='0xffff00ff'" )
#define UNSAFE_STATE_DESC _T( "多功能保护还没有开启,系统在不安全状态" )

				if( mode == WORK_FREE_MODE )
				{
					ctrl = m_pm.FindControl( _T( "work_status" ) ); 
					ASSERT( ctrl != NULL ); 

					ctrl->SetBkImage( UNSAGE_STATE_IMAGE_FILE ); 

					ctrl = m_pm.FindControl( _T( "state_desc_txt" ) ); 
					ASSERT( ctrl != NULL ); 

					tmp_text = _get_string_by_id( TEXT_SECURITY_LOGO_CLOSED, UNSAFE_STATE_DESC ); 
					ctrl->SetText( tmp_text ); 

					ctrl = m_pm.FindControl( _T( "ctrl_btn" ) ); 
					ASSERT( ctrl != NULL ); 

					ctrl->SetEnabled( true ); 
					ctrl->SetVisible( true ); 

				}
				else
				{
					ctrl = m_pm.FindControl( _T( "work_status" ) ); 
					ASSERT( ctrl != NULL ); 

					ctrl->SetBkImage( SAGE_STATE_IMAGE_FILE );

					ctrl = m_pm.FindControl( _T( "state_desc_txt" ) ); 
					ASSERT( ctrl != NULL ); 

					tmp_text = _get_string_by_id( TEXT_SECURITY_LOGO_OPENED, SAFE_STATE_DESC ); 
					ctrl->SetText( tmp_text ); 

					ctrl = m_pm.FindControl( _T( "ctrl_btn" ) ); 
					ASSERT( ctrl != NULL ); 

					ctrl->SetEnabled( false ); 
					ctrl->SetVisible( false ); 
				}
			}
			break; 
		case UI_LANG_NOTIFY:
			{
				//DBG_BRK(); 
				if( data_notify->data_len != sizeof( LANG_ID ) )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

#ifdef _DEBUG
				{
					LANG_ID cur_lang; 
					cur_lang = *( LANG_ID *)data_notify->data; 
					log_trace( ( MSG_INFO, "language changed to %ws\n", get_lang_desc( cur_lang ) ) );
				}
#endif //_DEBUG

				chg_main_wnd_lang( FALSE ); 
			}
			break; 
		case ERC_CMD_EXECUTED:
			{
				LPTSTR msg; 
				ULONG msg_len; 

				DBG_BRK(); 

				if( data_notify->data_len == 0 )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				if( data_notify->data == NULL )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				msg = ( LPTSTR )data_notify->data; 
				msg_len = data_notify->data_len / sizeof( TCHAR ); 

				if( msg[ msg_len - 1 ] != _T( '\0' ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 				
				}

				show_msg( NULL, msg ); 
			}
			break; 
		case SLOGAN_NOTIFY:
			{

				LPTSTR msg; 
				ULONG msg_len; 

				DBG_BRK(); 

				if( data_notify->data_len == 0 )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				if( data_notify->data == NULL )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				msg = ( LPTSTR )data_notify->data; 
				msg_len = data_notify->data_len / sizeof( TCHAR ); 

				if( msg[ msg_len - 1 ] != _T( '\0' ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					ASSERT( FALSE && "input the slogan that's have not null terminated" ); 
					goto _return; 				
				}

				show_slogan( NULL, msg );
			}
			break; 
		case REQUEST_UI_INTERACT_NOTIFY:
			{
				LPTSTR msg; 
				ULONG msg_len; 
				dlg_ret_state ret_state; 

				DBG_BRK(); 

				if( data_notify->data_len == 0 )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				if( data_notify->data == NULL )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				msg = ( LPTSTR )data_notify->data; 
				msg_len = data_notify->data_len / sizeof( TCHAR ); 

				if( msg[ msg_len - 1 ] != _T( '\0' ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					ASSERT( FALSE && "input the slogan that's have not null terminated" ); 
					goto _return; 				
				}

				ret = show_msg( GetHWND(), msg, &ret_state ); 

				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				if( ret_state == OK_STATE )
				{
					ret = ERROR_SUCCESS; 
				}
				else
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
				}
			}
			break; 
		case ALL_DRIVER_LOADED: 
			{
				//check_run_service(); 
			}
			break; 
		default:
			{
				ASSERT( FALSE && "invalid action type" ); 
			}
			break; 
		}

_return:
		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
		return ret; 
	}

	LRESULT chg_main_wnd_lang( INT32 is_init )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *ctrl; 
		COptionUI *option; 
		//CTabLayoutUI *tab_ctrl; 
		CControlUI *frame_title; 
		LPCTSTR tmp_text; 
		SIZE prev_xy; 
		UINT text_style; 

		ASSERT( TRUE == IsWindow( sys_log.GetHWND() ) ); 

		if( sys_log.GetHWND() != NULL )
		{
			sys_log.set_lang_text(); 
		}

		set_ctrl_text( &m_pm, _T( "ctrl_btn" ), TEXT_SECURITY_OPEN ); 		
		set_ctrl_text( &m_pm, _T( "state_desc_txt" ), TEXT_SECURITY_LOGO_CLOSED ); 
		set_ctrl_text( &m_pm, _T( "fw_event_btn" ), TEXT_FIREWALL_PANEL_LOG ); 
		set_ctrl_text( &m_pm, _T( "fw_trust_btn" ), TEXT_FIREWALL_PANEL_TRUST_FILE ); 
		set_ctrl_text( &m_pm, _T( "fw_block_btn" ), TEXT_FIREWALL_PANEL_DISTRUST_FILE ); 
		set_ctrl_text( &m_pm, _T( "fw_rule_btn" ), TEXT_FIREWALL_PANEL_SETTING ); 
		set_ctrl_text( &m_pm, _T( "fw_setting_btn" ), TEXT_FIREWALL_PANEL_SETTING ); 
		set_ctrl_text( &m_pm, _T( "fw_secu_btn" ), TEXT_FIREWALL_PANEL_SETTING ); 
		set_ctrl_text( &m_pm, _T( "icmp_btn" ), TEXT_FIREWALL_PANEL_ICMP_CTRL ); 
		set_ctrl_text( &m_pm, _T( "fw_login" ), TEXT_FIREWALL_PANEL_USER_ACCOUNT ); 
		set_ctrl_text( &m_pm, _T( "all_conn_btn" ), TEXT_FIREWALL_PANEL_ALL_CONNECTIONS ); 
		set_ctrl_text( &m_pm, _T( "work_mode_btn" ), TEXT_FIREWALL_PANEL_WORK_MODE ); 
		set_ctrl_text( &m_pm, _T( "anti_arp_btn" ), TEXT_FIREWALL_PANEL_ANTI_ARP ); 

		set_ctrl_text( &m_pm, _T( "defense_trust_btn" ), TEXT_DEFENSE_PANEL_TRUST_FILE ); 
		set_ctrl_text( &m_pm, _T( "defense_event_btn" ), TEXT_DEFENSE_PANEL_DEFENSE_LOG ); 
		set_ctrl_text( &m_pm, _T( "defense_block_btn" ), TEXT_DEFENSE_PANEL_DISTRUST_FILE ); 
		set_ctrl_text( &m_pm, _T( "defense_secu_btn" ), TEXT_DEFENSE_PANEL_DEFENSE_SETTING ); 
		set_ctrl_text( &m_pm, _T( "all_proc_btn" ), TEXT_DEFENSE_PANEL_ALL_PROCESS ); 
		//set_ctrl_text( &m_pm, _T( "replace_360" ), TEXT_DEFENSE_PANEL_DEFENSE_POWER ); 
		set_ctrl_text( &m_pm, _T( "net_cfg_btn" ), TEXT_DEFENSE_PANEL_NETWORK_PANEL ); 

		set_ctrl_text( &m_pm, _T( "pref_btn" ), TEXT_HELP_PANEL_ABOUT ); 
		set_ctrl_text( &m_pm, _T( "about_info_btn" ), TEXT_HELP_PANEL_ABOUT ); 
		set_ctrl_text( &m_pm, _T( "diag_btn" ), TEXT_HELP_PANEL_ABOUT ); 
		set_ctrl_text( &m_pm, _T( "forum_btn" ), TEXT_HELP_PANEL_FORUM ); 
		set_ctrl_text( &m_pm, _T( "help_btn" ), TEXT_HELP_PANEL_HELP ); 
		set_ctrl_text( &m_pm, _T( "update_btn" ), TEXT_HELP_PANEL_MANUAL_UPDATE ); 
		set_ctrl_text( &m_pm, _T( "suggest_btn" ), TEXT_HELP_PANEL_MANUAL_SUGGESTION ); 
		set_ctrl_text( &m_pm, _T( "conf_btn" ), TEXT_HELP_PANEL_ABOUT ); 
		set_ctrl_text( &m_pm, _T( "uninstall_btn" ), TEXT_HELP_PANEL_UNINSTALL ); 
		set_ctrl_text( &m_pm, _T( "option_btn" ), TEXT_HELP_PANEL_OPTION ); 

		//set_ctrl_text( &m_pm, _T( "sel_lang_btn" ), TEXT_LANG_BTN_TEXT ); 
		set_ctrl_text( &m_pm, _T( "donate_btn" ), TEXT_HELP_TITLE ); 
		set_ctrl_text( &m_pm, _T( "relearn_btn" ), TEXT_HELP_PANEL_RELEARN_RULE ); 
		set_ctrl_text( &m_pm, _T( "erc_btn" ), TEXT_HELP_PANEL_ERC ); 

		//SIZE szXY = {rcPos.left >= 0 ? rcPos.left : rcPos.right, rcPos.top >= 0 ? rcPos.top : rcPos.bottom};
		//SetFixedXY(szXY);
		//SetFixedWidth(rcPos.right - rcPos.left);
		//SetFixedHeight(rcPos.bottom - rcPos.top);
		ctrl = m_pm.FindControl( _T( "fw_title" ) ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_SUMMARY_PANEL_FIREWALL_TITLE ); 
		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )ctrl )->SetFixedWidth( 95 ); 
		}

		set_ctrl_text( &m_pm, _T( "defense_title" ), TEXT_SUMMARY_PANEL_DEFENSE_TITLE ); 

		ctrl = m_pm.FindControl( _T( "fw_upload_left" ) ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_SUMMARY_PANEL_ALL_UPLOADED_LEFT ); 

		text_style = ( ( CLabelUI* )ctrl )->GetTextStyle(); 
		text_style &= ~( DT_CENTER | DT_RIGHT ); 
		text_style |= DT_LEFT; 

		( ( CLabelUI* )ctrl )->SetTextStyle( text_style );  

		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )ctrl )->SetFixedWidth( 85 ); 
		}

		ctrl = m_pm.FindControl( _T( "fw_download_left" ) ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_SUMMARY_PANEL_ALL_DOWNLOADED_LEFT ); 
		
		text_style = ( ( CLabelUI* )ctrl )->GetTextStyle(); 
		text_style &= ~( DT_CENTER | DT_RIGHT ); 
		text_style |= DT_LEFT; 

		( ( CLabelUI* )ctrl )->SetTextStyle( text_style );  

		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )ctrl )->SetFixedWidth( 80 ); 
		}

#define DOWNLOAD_UPLOAD_ADD_LEN 50
		ctrl = m_pm.FindControl( _T( "fw_download" ) ); 
		ASSERT( ctrl != NULL ); 
		
		prev_xy = ctrl->GetFixedXY(); 
		if( is_init == TRUE )
		{
			download_x = prev_xy.cx; 
		}

		if( download_x == 0 )
		{
			ASSERT( FALSE && "why have not init x value." ); 
			download_x = 221; 
		}

		if( get_cur_lang_id() == LANG_ID_EN )
		{
			prev_xy.cx = download_x + DOWNLOAD_UPLOAD_ADD_LEN; 
		}
		else
		{
			prev_xy.cx = download_x; 
		}

		( ( CLabelUI* )ctrl )->SetFixedXY( prev_xy ); 

		ctrl = m_pm.FindControl( _T( "fw_upload" ) ); 
		ASSERT( ctrl != NULL ); 
		
		prev_xy = ctrl->GetFixedXY(); 
		if( TRUE == is_init )
		{
			upload_x = prev_xy.cx; 
		}


		if( upload_x == 0 )
		{
			ASSERT( FALSE && "why have not init x value." ); 
			upload_x = 221; 
		}

		if( get_cur_lang_id() == LANG_ID_EN )
		{
			prev_xy.cx = upload_x + DOWNLOAD_UPLOAD_ADD_LEN; 
		}
		else
		{
			prev_xy.cx = upload_x; 
		}

		( ( CLabelUI* )ctrl )->SetFixedXY( prev_xy ); 

		ctrl = m_pm.FindControl( _T( "fw_blk_tip_left" ) ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_SUMMARY_PANEL_FIREWALL_BLOCK_COUNT_LEFT ); 
		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )ctrl )->SetFixedWidth( 95 ); 
		}

		set_ctrl_text( &m_pm, _T( "fw_blk_tip_right" ), TEXT_SUMMARY_PANEL_FIREWALL_BLOCK_COUNT_RIGHT ); 

		ctrl = m_pm.FindControl( _T( "fw_upload_right" ) ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_SUMMARY_PANEL_ALL_UPLOADED_RIGHT ); 

		prev_xy = ctrl->GetFixedXY(); 

		if( TRUE == is_init )
		{
			upload_right_x = prev_xy.cx; 
		}

		if( upload_right_x == 0 )
		{
			ASSERT( FALSE && "why have not init x value." ); 
			upload_right_x = 277; 
		}
		
		if( get_cur_lang_id() == LANG_ID_EN )
		{
			prev_xy.cx = upload_right_x + DOWNLOAD_UPLOAD_ADD_LEN ;  
		}
		else
		{
			prev_xy.cx = upload_right_x; 
		}

		( ( CLabelUI* )ctrl )->SetFixedXY( prev_xy );

		ctrl = m_pm.FindControl( _T( "defense_blk_tip" ) ); 
		ASSERT( ctrl != NULL ); 

		if( get_cur_lang_id() == LANG_ID_EN )
		{
			prev_xy = ctrl->GetFixedXY(); 
			prev_xy.cx += 5; 
			( ( CLabelUI* )ctrl )->SetFixedXY( prev_xy ); 
		}

		ctrl = m_pm.FindControl( _T( "fw_blk_tip" ) ); 
		ASSERT( ctrl != NULL ); 

		if( get_cur_lang_id() == LANG_ID_EN )
		{
			prev_xy = ctrl->GetFixedXY(); 
			prev_xy.cx += 5; 
			( ( CLabelUI* )ctrl )->SetFixedXY( prev_xy ); 
		}

		ctrl = m_pm.FindControl( _T( "fw_download_right" ) ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_SUMMARY_PANEL_ALL_DOWNLOADED_RIGHT ); 

		prev_xy = ctrl->GetFixedXY(); 

		if( TRUE == is_init )
		{
			download_right_x = prev_xy.cx; 
		}

		if( download_right_x == 0 )
		{
			ASSERT( FALSE && "why have not init x value." ); 
			download_right_x = 277; 
		}

		if( get_cur_lang_id() == LANG_ID_EN )
		{
			prev_xy.cx = download_right_x + DOWNLOAD_UPLOAD_ADD_LEN ;  
		}
		else
		{
			prev_xy.cx = download_right_x; 
		}

		( ( CLabelUI* )ctrl )->SetFixedXY( prev_xy );

		set_ctrl_text( &m_pm, _T( "defense_blk_tip_left" ), TEXT_SUMMARY_PANEL_DEFENSE_BLOCK_COUNT_LEFT ); 
		set_ctrl_text( &m_pm, _T( "defense_blk_tip_right" ), TEXT_SUMMARY_PANEL_DEFENSE_BLOCK_COUNT_RIGHT ); 

		ctrl = m_pm.FindControl( _T( "static_logo" ) ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_LOGO_7LAYER_FW ); 
		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )ctrl )->SetFont( 6 ); 
		}
		else if( get_cur_lang_id() == LANG_ID_CN )
		{
			( ( CLabelUI* )ctrl )->SetFont( 4 ); 
		}

		if( TRUE == is_init )
		{
			//logo_width = ctrl->GetFixedWidth(); 
			logo_left = ctrl->GetFixedXY().cx; 

			if( logo_left == 0 )
			{
				ASSERT( FALSE && "why have not init x value." ); 
				//logo_width = 136; 
				logo_left = 30; //43; 
			}
		}
		else
		{
			SIZE ctrl_pos; 

			if( /*logo_width == 0 ||*/ logo_left == 0 )
			{
				ASSERT( FALSE && "why have not init x value." ); 
				//logo_width = 136; 
			}
			else
			{
#define BITSAFE_LOGO_ADD_WIDTH 22
				if( get_cur_lang_id() == LANG_ID_CN )
				{
					ctrl_pos = ctrl->GetFixedXY(); 

					ctrl_pos.cx = logo_left - BITSAFE_LOGO_ADD_WIDTH; 

					ctrl->SetFixedXY( ctrl_pos ); 

					//ctrl->SetFixedWidth( logo_width ); 
				}
				else
				{
					ctrl_pos = ctrl->GetFixedXY(); 
					ctrl_pos.cx = logo_left; 

					ctrl->SetFixedXY( ctrl_pos ); 

					//ctrl->SetFixedWidth( logo_width ); 
				}
			}
		}

		frame_title = m_pm.FindControl( _T( "frame_title" ) ); 
		ASSERT( frame_title != NULL ); 

		option = ( COptionUI* )m_pm.FindControl( _T("summary_btn") ); 
		ASSERT( option != NULL ); 
		if( option->IsSelected() )
		{
			tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_SUMMARY, SUMMARY_TITLE ); 
			frame_title->SetText( tmp_text ); 

			//option->Activate(); 
		}

		text_style = option->GetTextStyle(); 
		text_style &= ~( DT_CENTER | DT_VCENTER | DT_RIGHT );
		option->SetTextStyle( text_style ); 

		_set_ctrl_text( option, TEXT_MAIN_BUTTON_SUMMARY ); 
		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )option )->SetFont( 5 ); 
		}
		else if( get_cur_lang_id() == LANG_ID_CN )
		{
			( ( CLabelUI* )option )->SetFont( 3 ); 
		}

		ctrl = m_pm.FindControl( _T( "sys_mon_btn" ) ); 
		ASSERT( ctrl != NULL ); 

		tmp_text = get_string_by_id( TEXT_SUMMARY_PANEL_SYS_MONITOR_TIP ); 
		ASSERT( tmp_text != NULL );
		ctrl->SetToolTip( tmp_text ); 

		option = ( COptionUI* )m_pm.FindControl( _T("net_manage_btn") ); 
		ASSERT( option != NULL ); 
		if( option->IsSelected() )
		{
			tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_NETWORK, NET_MANAGE_TITLE ); 
			frame_title->SetText( tmp_text ); 

			//option->Activate(); 
		}

		text_style = option->GetTextStyle(); 
		text_style &= ~( DT_CENTER | DT_VCENTER | DT_RIGHT );
		option->SetTextStyle( text_style ); 

		_set_ctrl_text( option, TEXT_MAIN_BUTTON_NETWORK ); 
		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )option )->SetFont( 5 ); 
		}
		else if( get_cur_lang_id() == LANG_ID_CN )
		{
			( ( CLabelUI* )option )->SetFont( 3 ); 
		}

		option = ( COptionUI* )m_pm.FindControl( _T("sys_manage_btn") ); 
		ASSERT( option != NULL ); 
		if( option->IsSelected() )
		{
			tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_SYSTEM, SYS_MANAGE_TITLE ); 
			frame_title->SetText( tmp_text ); 

			//option->Activate(); 
		}

		text_style = option->GetTextStyle(); 
		text_style &= ~( DT_CENTER | DT_VCENTER | DT_RIGHT );
		option->SetTextStyle( text_style ); 

		_set_ctrl_text( option, TEXT_MAIN_BUTTON_SYSTEM ); 
		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )option )->SetFont( 5 ); 
		}
		else if( get_cur_lang_id() == LANG_ID_CN )
		{
			( ( CLabelUI* )option )->SetFont( 3 ); 
		}

		option = ( COptionUI* )m_pm.FindControl( _T("about_btn") ); 
		ASSERT( option != NULL ); 
		if( option->IsSelected() )
		{
			tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_HELP, MORE_TITLE ); 
			frame_title->SetText( tmp_text ); 

			//option->Activate(); 
		}

		text_style = option->GetTextStyle(); 
		text_style &= ~( DT_CENTER | DT_VCENTER | DT_RIGHT );
		option->SetTextStyle( text_style ); 

		_set_ctrl_text( option, TEXT_MAIN_BUTTON_HELP ); 
		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )option )->SetFont( 5 ); 
		}
		else if( get_cur_lang_id() == LANG_ID_CN )
		{
			( ( CLabelUI* )option )->SetFont( 3 ); 
		}

_return:
		return ret; 
	}

	void Init() 
	{
		LRESULT ret = ERROR_SUCCESS; 
		//CButtonUI *control; 
		COptionUI *option; 
		LPCWSTR tmp_text; 

		//CTabLayoutUI* tab_ctrl; 

		//frame_title = m_pm.FindControl(_T("frame_title") ); 
		//ASSERT( frame_title != NULL ); 
		//frame_title->SetText( SUMMARY_TITLE ); 

		/***************************************************************
		notice:
		the restore button image is not perfect.
		***************************************************************/
		m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
		m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn"))); 

		ret = init_popup_wnd_context(); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "init popup window context error!\n" ); 
			log_trace( ( MSG_INFO, "init popup window context error!\n" ) ); 
		}

		//control = static_cast<CButtonUI*>(m_pm.FindControl(_T( "all_proc_btn" ) ) ); 

		//control->SetEnabled( false ); 
		//control->SetVisible( false ); 

		ret = load_common_conf( update_ui, &m_pm ); 

#ifdef _DEBUG
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load ui configuration error 0x%0.8x\n", ret ) ); 
		}
#endif //_DEBUG

		change_original_pwd( GetHWND() ); ; 

		//ret = init_service_context_async(); 

#ifdef _DEBUG
		{
			INT32 _ret; 
			ULONG session_id; 
			ULONG proc_id; 

			proc_id = GetCurrentProcessId(); 
			_ret = ProcessIdToSessionId( proc_id, &session_id ); 
			session_id = WTSGetActiveConsoleSessionId(); 
			if( _ret == TRUE )
			{

			}
		}
#endif //_DEBUG

		ret = init_ui_context_async( ( action_ui_notify* )this ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "!!!initialize ui work context failed 0x%0.8x, will exit", ret ) );  
		}

		//tab_ctrl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T( "frame" ) ) ); 

		//ASSERT( tab_ctrl != NULL ); 

		//tab_ctrl->SelectItem( 0 ); 
		//ASSERT( tab_ctrl != NULL ); 

		option = static_cast<COptionUI*>(m_pm.FindControl( _T( "summary_btn" ) ) ); 
		ASSERT( option != NULL ); 

		//tab_ctrl->SelectItem( 0 ); 
		option->Activate(); 

		ASSERT( FALSE == ::IsWindow( sys_log.GetHWND() ) ); 

		tmp_text = _get_string_by_id( TEXT_RUN_TIME_LOG_TITLE, _T( "实时日志" ) ); 
		sys_log.Create( NULL, tmp_text, UI_WNDSTYLE_FRAME, 0L, 0, 0, 1024, 738 ); 
		if( sys_log.GetHWND() != NULL )
		{
			sys_log.SetVisible( false ); 
			sys_log.CenterWindow();
			sys_log.SetIcon( IDI_MAIN_ICON ); 
		}

//#define TEST_RICH_EDIT
#ifdef TEST_RICH_EDIT

		//thread_test_rich_edit( ( PVOID )&sys_log ); 

		CreateThread( NULL, 0, thread_test_rich_edit, ( PVOID )&sys_log, 0, NULL ); 

#endif //TEST_RICH_EDIT

		chg_main_wnd_lang( TRUE ); 

		ret = start_slogan_display( ( action_ui_notify* )this ); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
		}

		//TestVersion( "22323.12341234.3838823.123412341", "22323.12341234.3838823.123412342" ); 
	}

	LRESULT on_more_info_tab_selected()
	{
		LRESULT ret = ERROR_SUCCESS; 


_return:
		return ret; 
	}

	LRESULT on_net_manage_tab_selected()
	{
		LRESULT ret = ERROR_SUCCESS; 

_return:
		return ret; 
	}

	LRESULT on_sys_manage_tab_selected() 
	{
		LRESULT ret = ERROR_SUCCESS; 
		CButtonUI *net_cfg_btn; 

		if( is_vista_or_later() == TRUE )
		{
			net_cfg_btn = static_cast< CButtonUI* >( m_pm.FindControl( _T( "net_cfg_btn" ) ) ); 
			//DBG_BP();

			if( net_cfg_btn == NULL )
			{
				ASSERT( FALSE ); 

				goto _return; 
			}

			net_cfg_btn->SetEnabled( false ); 
			net_cfg_btn->SetVisible( false ); 
		}

_return:
		return ret; 
	}

	LRESULT on_summary_tab_selected()
	{
		LRESULT ret = ERROR_SUCCESS; 

_return:
		return ret; 
	}

#ifdef DIRECT_SHOW_UI
	void OnPrepareAnimation()
	{
		COLORREF clrBack = RGB( 10, 10, 10 );
		RECT rcCtrl = m_pm.FindControl(_T("summary_btn"))->GetPos();
		m_pm.AddAnimJob(CAnimJobUI(UIANIMTYPE_FLAT, 0, 350, clrBack, clrBack, CRect(rcCtrl.left, rcCtrl.top, rcCtrl.left + 50, rcCtrl.top + 50), 40, 0, 4, 255, 0.3f));
		rcCtrl = m_pm.FindControl(_T("net_manage_btn"))->GetPos();
		m_pm.AddAnimJob(CAnimJobUI(UIANIMTYPE_FLAT, 200, 350, clrBack, clrBack, CRect(rcCtrl.left, rcCtrl.top, rcCtrl.left + 50, rcCtrl.top + 50), 40, 0, 4, 255, 0.3f));
		rcCtrl = m_pm.FindControl(_T("sys_manage_btn"))->GetPos();
		m_pm.AddAnimJob(CAnimJobUI(UIANIMTYPE_FLAT, 100, 350, clrBack, clrBack, CRect(rcCtrl.left, rcCtrl.top, rcCtrl.left + 50, rcCtrl.top + 50), 40, 0, 4, 255, 0.3f));
		rcCtrl = m_pm.FindControl(_T("about_btn"))->GetPos();
		m_pm.AddAnimJob(CAnimJobUI(UIANIMTYPE_FLAT, 250, 350, clrBack, clrBack, CRect(rcCtrl.left, rcCtrl.top, rcCtrl.left + 50, rcCtrl.top + 50), 40, 0, 4, 255, 0.3f));
	}
#endif //DIRECT_SHOW_UI

	void OnPrepare() 
	{
#ifdef DIRECT_SHOW_UI

		OnPrepareAnimation(); 
		//SendMessage( WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM( 0xff, 0x36 ) ); 
#endif //DIRECT_SHOW_UI

	}

	LRESULT on_change_work_mode()
	{
		LRESULT ret = ERROR_SUCCESS; 

		chg_mode_dlg dlg; 
		ret = check_admin_access( GetHWND() ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		dlg.Create( GetHWND(), _T("设置工作模式"), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
		dlg.SetIcon( IDI_MAIN_ICON ); 
		dlg.CenterWindow();
		dlg.ShowModal(); 

_return:
		return ret; 
	}

	LRESULT on_login()
	{
		LRESULT ret = ERROR_SUCCESS; 

		login_dlg dlg; 

		dlg.Create( GetHWND(), _T("用户登陆"), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
		dlg.SetIcon( IDI_MAIN_ICON ); 
		dlg.CenterWindow();
		dlg.ShowModal(); 

		return ret; 
	}

	LRESULT on_chg_pwd()
	{
		LRESULT ret = ERROR_SUCCESS; 
		chg_pwd_dlg dlg; 
		
		ret = check_admin_access( GetHWND() ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		dlg.Create( GetHWND(), _T("设置管理密码"), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
		dlg.SetIcon( IDI_MAIN_ICON ); 
		dlg.CenterWindow();
		dlg.ShowModal(); 

_return:
		return ret; 
	}

	LRESULT on_close()
	{
		LRESULT ret = ERROR_SUCCESS; 
		chg_pwd_dlg dlg; 

		ret = check_admin_access( GetHWND() ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		Close(); 

_return:
		return ret; 
	}

	void Notify(TNotifyUI& msg)
	{
		LRESULT ret = ERROR_SUCCESS; 
		LPCTSTR tmp_text; 

		if( msg.sType == _T("windowinit") ) 
		{
			OnPrepare();
		}
		else if( msg.sType == _T("click") ) 
		{
			if( msg.pSender == m_pCloseBtn ) 
			{
				short_window_effective(); 
				//ShowWindow( false, false ); 
				//PostQuitMessage(0);
				return; 
			}
			else if( msg.pSender == m_pMinBtn ) 
			{ 
				SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); return; 
			}
			//else if( msg.pSender == m_pMaxBtn ) 
			//{ 
			//	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
			//}
			else if( msg.pSender == m_pRestoreBtn ) 
			{ 
				SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
			}
			else
			{
				CStdString name = msg.pSender->GetName(); 

				if( name == _T( "all_proc_btn") )
				{
					//ret = check_admin_access( GetHWND() ); 

					//if( ret != ERROR_SUCCESS )
					//{
					//	goto _return; 
					//}

					if( !::IsWindow( proc_list.GetHWND() ) )
					{
						tmp_text = _get_string_by_id( TEXT_DEFENSE_PANEL_ALL_PROCESS, 
							_T("进程列表") ); 

						proc_list.Create(NULL, tmp_text, UI_WNDSTYLE_FRAME, 0L, 0, 0, 600, 400 );
						proc_list.CenterWindow();
						proc_list.SetIcon( IDI_MAIN_ICON ); 
					}
				}
				else if( name == _T( "about_info_btn" ) )
				{
					if( !::IsWindow( about_box.GetHWND() ) )
					{
						about_box.Create(NULL, _T("关于..."), UI_WNDSTYLE_FRAME, 0L, 0, 0, 490,280);
						about_box.CenterWindow();
						about_box.SetIcon( IDI_MAIN_ICON );
					}

					//CRect popup_rect; 
					//popup_rect.left = 300; 
					//popup_rect.top = 300; 
					//popup_rect.right = 800; 
					//popup_rect.bottom = 600; 

					//popup_tip_wnd *popup_dlg; 
					//
					//popup_dlg = popup_dlg->create_popup( _T( "hello" ), 
					//	50, 
					//	50, 
					//	popup_rect, 
					//	GetHWND() ); 

					//popup_dlg->popup_msg(); 
				}
				//else if( name == _T( "fw_rule_btn" ) )
				//{
				//	if( !::IsWindow( url_rule.GetHWND() ) )
				//	{
				//		url_rule.Create(NULL, _T("URL规则设置"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 1024, 738);
				//		url_rule.CenterWindow();
				//		url_rule.SetIcon( IDI_MAIN_ICON );	
				//	}
				//}
				else if( name == _T( "anti_arp_btn" ) )
				{

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					if( !::IsWindow( arp_infos.GetHWND() ) )
					{
						arp_infos.Create(NULL, _T("Anti-Arp"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 600, 400 );
						arp_infos.CenterWindow();
						arp_infos.SetIcon( IDI_MAIN_ICON );
					}
				}
				else if( name == _T( "all_conn_btn" ) )
				{
					//ret = check_admin_access( GetHWND() ); 

					//if( ret != ERROR_SUCCESS )
					//{
					//	goto _return; 
					//}

					if( !::IsWindow( netmon.GetHWND() ) )
					{
						tmp_text = _get_string_by_id( TEXT_FIREWALL_PANEL_ALL_CONNECTIONS, 
							_T("网络流量管理") ); 

						netmon.Create( GetHWND(), tmp_text, UI_WNDSTYLE_FRAME, 0L, 0, 0, 600, 400);
						netmon.CenterWindow();
						netmon.SetIcon( IDI_MAIN_ICON ); 
					}
				}
				//else if( name == _T( "help_btn" ) )
				//{
				//	msg_box msg_tip; 
				//	CControlUI *msg_txt; 

				//	msg_tip.Create( GetHWND(), _T("系统消息"), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
				//	msg_tip.SetIcon( IDI_MAIN_ICON ); 
				//	msg_txt = msg_tip.m_pm.FindControl( _T( "msg_txt" ) ); 
				//	
				//	ASSERT( msg_txt != NULL ); 
				//	msg_txt->SetText( _T( "Test Msg box" ) ); 

				//	msg_tip.CenterWindow();
				//	msg_tip.ShowModal(); 
				//} 
				else if( name == _T( "fw_login" ) )
				{
					on_chg_pwd(); 
				}
				else if( name == _T( "work_mode_btn" ) )
				{
					on_change_work_mode(); 
				}
				else if( name == _T( "fw_trust_btn" ) )
				{
					trust_block_dlg add_trust( ADD_FW_TRUST_FILE ); 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_FIREWALL_PANEL_TRUST_FILE, 
						_T( "添加被信任的程序" ) ); 

					add_trust.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					add_trust.SetIcon( IDI_MAIN_ICON ); 
					add_trust.CenterWindow();
					add_trust.ShowModal(); 
				}
				else if( name == _T( "fw_block_btn" ) )
				{
					trust_block_dlg add_block( ADD_FW_BLOCK_FILE ); 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_FIREWALL_PANEL_DISTRUST_FILE, 
						_T( "添加被阻止的程序" ) ); 

					add_block.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					add_block.SetIcon( IDI_MAIN_ICON ); 
					add_block.CenterWindow();
					add_block.ShowModal(); 
				}
				else if( name == _T( "defense_trust_btn" ) )
				{
					trust_block_dlg add_trust( ADD_DEFENSE_TRUST_FILE ); 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_DEFENSE_PANEL_TRUST_FILE, 
						_T("添加被信任的程序") ); 

					add_trust.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					add_trust.SetIcon( IDI_MAIN_ICON ); 
					add_trust.CenterWindow();
					add_trust.ShowModal(); 
				}
				else if( name == _T( "defense_block_btn" ) )
				{
					trust_block_dlg add_block( ADD_DEFENSE_BLOCK_FILE ); 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_DEFENSE_PANEL_DISTRUST_FILE, 
						_T("添加被阻止的程序") ); 

					add_block.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					add_block.SetIcon( IDI_MAIN_ICON ); 
					add_block.CenterWindow();
					add_block.ShowModal(); 
				}
				else if( name == _T( "fw_event_btn" ) )
				{
					fw_event_dlg fw_events; 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_FIREWALL_PANEL_LOG, 
						_T("防火墙日志") ); 

					fw_events.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					fw_events.SetIcon( IDI_MAIN_ICON ); 
					fw_events.CenterWindow();
					fw_events.ShowModal(); 
				}
				else if( name == _T( "defense_event_btn" ) )
				{
					defense_event_dlg defense_event; 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_DEFENSE_PANEL_DEFENSE_LOG, 
						_T("主动防御日志") ); 
					defense_event.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					defense_event.SetIcon( IDI_MAIN_ICON ); 
					defense_event.CenterWindow();
					defense_event.ShowModal(); 
				}
				else if( name == _T( "fw_secu_btn" ) )
				{
					setting_dlg fw_settings( FW_SETTING ); 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_FIREWALL_PANEL_SETTING, 
						_T("防火墙设置") ); 

					fw_settings.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					fw_settings.SetIcon( IDI_MAIN_ICON ); 
					fw_settings.CenterWindow();
					fw_settings.ShowModal(); 
				}
				else if( name == _T( "defense_secu_btn" ) )
				{
					setting_dlg defense_settings( DEFENSE_SETTING ); 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_DEFENSE_PANEL_DEFENSE_SETTING, 
						_T("主动防御设置") ); 
					defense_settings.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					defense_settings.SetIcon( IDI_MAIN_ICON ); 
					defense_settings.CenterWindow();
					defense_settings.ShowModal(); 
				}
#define SEVEN_FW_FORUM_URL _T( "http://www.codereba.com/bbs/" )
				else if( name == _T( "forum_btn" ) )
				{
					::ShellExecute( NULL, _T( "open" ), SEVEN_FW_FORUM_URL, NULL, NULL, SW_SHOWNORMAL ); 
				}
				else if( name == _T( "help_btn" ) )
				{
#define SEVEN_FW_HELP_URL _T( "http://www.codereba.com/" )
					::ShellExecute( NULL, _T( "open" ), SEVEN_FW_HELP_URL, NULL, NULL, SW_SHOWNORMAL ); 
				}
				else if( name == _T( "update_btn" ) )
				{
					ret = start_clone_process(); 
					if( ret != ERROR_SUCCESS )
					{
						log_trace( ( MSG_ERROR, "!!!start update process failed \n" ) );
					} 
				}
				else if( name == _T( "ctrl_btn" ) )
				{
					//ret = check_admin_access( GetHWND() ); 

					//if( ret != ERROR_SUCCESS )
					//{
					//	goto _return; 
					//}

					ret = set_cur_work_mode( WORK_ACL_MODE, ( action_ui_notify* )this ); 
				}
				else if( name == _T( "uninstall_btn" ) )
				{
					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					ret = unistall_bitsafe(); 
					if( ret != ERROR_SUCCESS )
					{
						if( ret != ERROR_FILE_NOT_FOUND )
						{
							tmp_text = _get_string_by_id( TEXT_UPDATE_OLD_VERSION_HAVE_NOT_UNINSTALL, 
								_T( "旧版本比特安全没有完成全部卸载,重新运行程序(特别是新版本)时,要先执行卸载(如出现安装提示,先不要执行安装),在保证卸载全部完成后,再重新安装并运行程序。" )  ); 

							show_msg( GetHWND(), tmp_text ); 
						}
						else
						{
							tmp_text = _get_string_by_id( TEXT_UNISNTALL_FAILED, 
								_T( "卸载比特安全失败,是否已经卸载" ) ); 

							show_msg( GetHWND(), tmp_text ); 
						}
					}
					else
					{
						dlg_ret_state ret_state; 

						tmp_text = _get_string_by_id( TEXT_UNINSTALL_NEED_RESTART, 
							_T( "卸载比特安全需要重启系统." ) ); 

						ret = show_msg( GetHWND(), tmp_text, &ret_state ); 
						if( ret != ERROR_SUCCESS )
						{
							ASSERT( FALSE ); 
						}

						//if( ret_state == OK_STATE )
						//{
						//	windows_shutdown( EWX_REBOOT | EWX_FORCE );  
						//}
					}
				}
				else if( name == _T( "suggest_btn" ) )
				{
					suggest_dlg dlg; 
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_SUGGESTION_TITLE, _T( "提出建议" ) ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
				else if( name == _T( "donate_btn" ) )
				{
					donate_dlg dlg; 
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_HELP_TITLE, 
						_T( "提出建议" ) ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 

				}
				else if( name == _T( "option_btn" ) ) //_T( "sel_lang_btn" ) )
				{

					option_dlg dlg( this ); 
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_OPTION_DIALOG_TITLE, _T( "选项" ) ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 

#if 0
					chg_lang_dlg dlg( this ); 
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_LANG_TITLE, _T( "选择语言" ) ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
#endif //0
					save_lang_conf(); 
				}
				else if( name == _T( "erc_btn" ) )
				{
					remote_conf_dlg dlg( ( action_ui_notify* )this ); 
					LPCTSTR tmp_text; 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_ERC_TITLE, _T( "远程控制设置" ) ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
				else if( name == _T( "net_cfg_btn" ) )
				{
					netcfg_dlg dlg; 
					LPCTSTR tmp_text; 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_NETWORK_CTRL_PANEL_TITLE, _T( "网络控制面板管理" ) ); 
					
					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
#ifdef COMPETITE_WITH_360
				else if( name == _T( "replace_360" ) )
				{
					crack360_dlg dlg; 
					LPCTSTR tmp_text; 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_GET_DEFENSE_POWER_FROM_OTHER, _T( "接管主动防御功能" ) ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
#endif //COMPETITE_WITH_360	
				else if( name == _T( "relearn_btn" ) )
				{
					LPCTSTR tmp_text; 

					ret = check_admin_access( GetHWND() ); 

					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_RELEARN, NULL, 0, NULL, 0, NULL, NULL ); 
					if( ret != ERROR_SUCCESS )
					{
						ASSERT( FALSE ); 
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_RELEARN_TIP_MSG, _T( "设置重新学习规则成功" ) ); 
					
					show_msg( GetHWND(), tmp_text ); 
				}
				else if( name == _T( "icmp_btn" ) )
				{
					icmp_dlg dlg; 
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_ICMP_TITLE, _T( "ICMP通信管理" ) ); 
					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
				else if( name == _T( "ui_btn" ) )
				{ 
					ui_custom_dlg
					/*ColorSkinWindow */
					dlg( &m_pm );  
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_UI_CUSTOM_TITLE, _T( "UI界面设置" ) ); 
					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
				else if( name == _T( "sys_mon_btn" ) )
				{
					if( sys_log.GetHWND() != NULL )
					{
						sys_log.set_lang_text(); 
						sys_log.SetVisible( true ); 
					}
				}
			}
		}
		else if( msg.sType == _T( "menu_exit" ))
		{
			on_close(); 
		}
		else if(msg.sType == _T( "menu_login" ) )
		{
			on_login(); 
		}
		else if( msg.sType == _T( "menu_work_mode" ) )
		{
			on_change_work_mode(); 
		}
		else if( msg.sType == _T( "menu_open" ) )
		{
			ShowWindow(); 
		}
		else if(msg.sType==_T("selectchanged"))
		{
			LPCTSTR tmp_text; 
			CStdString name = msg.pSender->GetName();
			CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("frame")));
			CControlUI *frame_title = m_pm.FindControl(_T("frame_title")); 

			ASSERT( pControl != NULL ); 
			ASSERT( frame_title != NULL ); 

			__Trace( _T( "the set focus target control is %s \n" ), name ); 
			
			if(name==_T("summary_btn"))
			{
				tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_SUMMARY, SUMMARY_TITLE ); 

				frame_title->SetText( tmp_text ); 
				pControl->SelectItem( 0 );
				on_summary_tab_selected(); 
			}
			else if(name==_T("net_manage_btn"))
			{
				tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_NETWORK, NET_MANAGE_TITLE ); 

				frame_title->SetText( tmp_text ); 
				pControl->SelectItem( 1 );
				on_net_manage_tab_selected(); 
			}
			else if(name==_T("sys_manage_btn"))
			{
				tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_SYSTEM, SYS_MANAGE_TITLE ); 

				frame_title->SetText( tmp_text ); 
				pControl->SelectItem( 2 );
				on_sys_manage_tab_selected(); 
			}
			else if(name==_T("about_btn"))
			{
				tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_HELP, MORE_TITLE ); 

				frame_title->SetText( tmp_text ); 
#ifdef SIMPLE_MODE 
				pControl->SelectItem( 2 );
#else
				pControl->SelectItem( 3 );
#endif //SIMPLE_MODE 
				on_more_info_tab_selected(); 
			}
		}

_return:
		return; 
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 
		HMENU sys_menu; 
		LPCTSTR tmp_text; 

		sys_menu = GetSystemMenu( GetHWND(), FALSE ); 
		ASSERT( sys_menu != NULL ); 
		
		tmp_text = _get_string_by_id( TEXT_MAIN_SYS_MENU_HIDE, _T( "隐藏(&H)" ) ); 

		ModifyMenu( sys_menu, SC_CLOSE, MF_BYCOMMAND, SC_CLOSE, tmp_text ); 

		//CloseHandle( sys_menu ); 
		
		//RemoveMenu(SC_CLOSE,MF_BYCOMMMAND); 
		//RemoveMenu(0,MF_BYPOSITION);       

		set_app_flag( GetHWND(), FALSE ); 

		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);

		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		CDialogBuilder builder;
		tabs_builder cb;
		CControlUI* pRoot = builder.Create(_T("bitsafe.xml"), (UINT)0,  &cb, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);
		set_ui_style( &m_pm ); 

		tmp_text = _get_string_by_id( TEXT_MAIN_BITSAFE_STARTED_BALLOON_TIP, 
			_T( "比特安全已经启动" ) ); 

		ret = icon_tray.balloon_icon( GetHWND(), 
			WM_TRAY_ICON_NOTIFY, 
			tmp_text, 
			_T( "" ), 
			_T( "" ), 
			GetModuleHandle( NULL ), 
			IDI_MAIN_ICON, 
			IDI_ALERT_ICON, 
			TRAY_ICON_ID, 
			MAKEINTRESOURCE( IDB_ALERT_ICONS ) ); 
		
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			Close(); 
		}

		Init();
		return 0;
	}

	LRESULT short_window_effective()
	{
		LRESULT ret = ERROR_SUCCESS; 

#define EFFECTIVE_TRANPARENT 255 
		m_pm.SetTransparent( EFFECTIVE_TRANPARENT ); 
		ret = short_window_close( GetHWND(), 0, 0 ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "shorting window close effective error 0x%0.8x\n", ret ) ); 
		}

		//m_pm.SetTransparent( get_tranparent() ); 
		//CenterWindow(); 
		
		return ret; 
	}

	LRESULT stop_action_response_dlg()
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
		}while( FALSE );
	
		return ret; 
	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 
		bHandled = FALSE;

		ret = stop_slogan_display(); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			log_trace( ( MSG_ERROR, "stop slogan show error 0x%0.8x\n", ret ) ); 
		}

		ret = stop_action_response_dlg(); 
		if( ret != ERROR_SUCCESS )
		{

		}

		if( ::IsWindow( sys_log.GetHWND() ) )
		{
			DestroyWindow( sys_log.GetHWND() ); 
			//sys_log.Close(); 
		}

		ret = uninit_ui_context(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "uninitialize the ui context failed \n" ) ); 
		}

		ret = icon_tray.DeleteIcon(); 
		if( ret !=	ERROR_SUCCESS )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!delete the tray icon failed \n" ) ); 
		}

		ret = close_bitsafe_conf(); 
		if( ret < 0 )
		{
			log_trace( ( MSG_ERROR, "close seven fw config file failed" ) ); 
		}

		release_all_rules(); 

		if( ::IsWindow( arp_infos.GetHWND() ) )
		{
			arp_infos.Close(); 
		}

		//if( ::IsWindow( url_rule.GetHWND() ) )
		//{
		//	url_rule.Close(); 
		//}

		if( ::IsWindow( netmon.GetHWND() ) )
		{
			netmon.Close(); 
		}

		if( ::IsWindow( proc_list.GetHWND() ) )
		{
			proc_list.Close(); 
		}; 

		if( ::IsWindow( about_box.GetHWND() ) )
		{
			about_box.Close(); 
		}

		ret = release_ui_ctrls(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "uninitialize ui control model in service failed \n" ) ); 
		}

		if( timer_id != 0 )
		{
			KillTimer( GetHWND(),timer_id ); 
		}

		ret = uninit_popup_wnd_context(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "uninit popup window context error!\n" ) ); 
		}

		uninit_bitsafe_function_state(); 

		if( TRUE == IsWindowVisible( GetHWND() ) )
		{
			short_window_effective(); 
		}

		PostQuitMessage( 0 ); 

		return 0;
	}

	LRESULT tray_icon_notify( ULONG msg, WPARAM wID, LPARAM lEvent, BOOL &bHandled )
	{
		LRESULT ret = ERROR_SUCCESS; 
		
		bHandled = TRUE; 

		if( wID != TRAY_ICON_ID )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		if( lEvent == WM_RBUTTONUP )
		{
			POINT pt; 
			//DWORD msg_pt; 
			CControlUI *main_layout = m_pm.FindControl( _T( "form_layout" ) ); 

			ASSERT( main_layout != NULL ); 

			//msg_pt = GetMessagePos(); 

			GetCursorPos( &pt ); 

			//pt.x = GET_X_LPARAM( msg_pt ); 
			//pt.y = GET_Y_LPARAM( msg_pt ); 

			//ClientToScreen( GetHWND(), &pt );

			tray_icon_menu.Init( main_layout, pt ); 
		}
		else if( lEvent == WM_LBUTTONDBLCLK ) 
		{ 
			ShowWindow(); 
			SetForegroundWindow( m_hWnd ); 
			//Close(); 
		} 

_return:
		return ret; 
	} 

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//::PostQuitMessage(0L);

		remove_app_flag( GetHWND(), FALSE ); 
		uninit_bitsafe_conf(); 

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        if( ::IsIconic( *this ) ) bHandled = FALSE;
        return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; 
		
		pt.x = GET_X_LPARAM(lParam); 
		pt.y = GET_Y_LPARAM(lParam);
		
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

 		if( !::IsZoomed(*this) ) {
 			RECT rcSizeBox = m_pm.GetSizeBox();
 			if( pt.y < rcClient.top + rcSizeBox.top ) {
 				if( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
 				if( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
 				return HTTOP;
 			}
 			else if( pt.y > rcClient.bottom - rcSizeBox.bottom ) {
 				if( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
 				if( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
 				return HTBOTTOM;
 			}
 			if( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
 			if( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
 		}

		RECT rcCaption = m_pm.GetCaptionRect();
		if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
				if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
					_tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
					_tcscmp(pControl->GetClass(), _T("TextUI")) != 0 )
					return HTCAPTION;
		}

		return HTCLIENT;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        SIZE szRoundCorner = m_pm.GetRoundCorner();
        if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
            CRect rcWnd;
            ::GetWindowRect(*this, &rcWnd);
            rcWnd.Offset(-rcWnd.left, -rcWnd.top);
            rcWnd.right++; rcWnd.bottom++;
            HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
            ::SetWindowRgn(*this, hRgn, TRUE);
            ::DeleteObject(hRgn);
        }

#ifdef DIRECT_SHOW_UI
		if( m_anim.IsAnimating() ) m_anim.CancelJobs();
#endif //DIRECT_SHOW_UI

        bHandled = FALSE;
        return 0;
	}

	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CRect rcWork = oMonitor.rcWork;
		rcWork.Offset(-rcWork.left, -rcWork.top);

		LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
		lpMMI->ptMaxPosition.x	= rcWork.left;
		lpMMI->ptMaxPosition.y	= rcWork.top;
		lpMMI->ptMaxSize.x		= rcWork.right;
		lpMMI->ptMaxSize.y		= rcWork.bottom;

		bHandled = FALSE;
		return 0;
	}

	LRESULT on_char(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 
		UINT32 key_state; 

		bHandled = FALSE; 
		ASSERT( uMsg == WM_KEYDOWN ); 

		if( wParam == ( WPARAM )'d' 
			|| wParam == ( WPARAM )'D')
		{		
			key_state = MapKeyState(); 
			if( MK_SHIFT == ( MK_SHIFT & key_state ) 
				&& MK_CONTROL == ( MK_CONTROL & key_state ) )
			{
				dbg_panel dlg( DRIVER_MANAGE ); 

				dlg.Create( GetHWND(), _T( "debugging" ), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
				dlg.SetIcon( IDI_MAIN_ICON ); 
				dlg.CenterWindow();
				dlg.ShowModal(); 
			}
		}

		if( wParam == ( WPARAM )'p' 
			|| wParam == ( WPARAM )'P')
		{		
			key_state = MapKeyState(); 
			if( MK_SHIFT == ( MK_SHIFT & key_state ) 
				&& MK_CONTROL == ( MK_CONTROL & key_state ) )
			{
				if( get_dbg_log_level() == DBG_LVL_NONE_PRINT )
				{
					set_dbg_log_level( 0 ); 
				}
				else
				{
					set_dbg_log_level( DBG_LVL_NONE_PRINT ); 
				}
			}
		}
		
_return:
		return ret; 
	}

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
		if( wParam == SC_CLOSE ) {
			//::PostQuitMessage(0L);

			//short_window_effective(); 

			//ShowWindow( false, false ); 

			bHandled = TRUE;
			return 0;
		}

		BOOL bZoomed = ::IsZoomed(*this);
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		if( ::IsZoomed(*this) != bZoomed ) {
			if( !bZoomed ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("max_btn")));
				if( pControl ) pControl->SetVisible(false);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("nor_btn")));
				if( pControl ) pControl->SetVisible(true);
			}
			else {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("max_btn")));
				if( pControl ) pControl->SetVisible(true);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("nor_btn")));
				if( pControl ) pControl->SetVisible(false);
			}
		}
		bHandled = TRUE; 
		return lRes;
	}

	LRESULT OnNcDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{	
		bHandled = FALSE; 

		return 0; 
	}

#define MENU_ITEM_EXIT 1001
#define MENU_ITEM_LOGIN 1002
#define MENU_ITEM_CHANGE_PWD 1003

	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = 0; 
		// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
		return ret;
	}

	LRESULT on_slogan_tip( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0; 
	}

	LRESULT on_response_event(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret; 
		WCHAR *action_msg; 
		WCHAR *data_dump = NULL; 
		//ACTION_RECORD_TYPE need_record; 
		//event_action_response event_respon; 
		//dlg_ret_state ret_status; 
		LPCTSTR tmp_text; 
		sys_action_output *action_event; 

		log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

		ASSERT( lParam != NULL ); 
		
		action_event = ( sys_action_output* )lParam; 
		ASSERT( is_valid_action_type( action_event->action.type ) ); ; 

		action_msg = ( WCHAR* )malloc( MAX_MSG_LEN * sizeof( WCHAR ) ); 

		if( action_msg == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		data_dump = ( WCHAR* )malloc( MAX_DATA_DUMP_LEN * sizeof( WCHAR ) ); 
		if( data_dump == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return;
		}

		ret = get_event_msg_ex( action_event, 
			action_msg, 
			MAX_MSG_LEN, 
			data_dump, 
			MAX_DATA_DUMP_LEN, 
			0 ); 

		//ret = get_event_msg_ex( action_event, action_msg, MAX_MSG_LEN, 0 ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		tmp_text = _get_string_by_id( TEXT_SYS_ACTION_TIP_SPECIOUS_ACTION, _T( "被怀疑的系统行为" ) ); 

#ifdef _DEBUG
		{
			//INT32 i; 
			//for( i = 0; i = 10; i ++ )
			{
				ret = show_event_tip( GetHWND()/*GetDesktopWindow()*/, 
					action_msg, 
					data_dump, 
					action_event, 
					tmp_text, 
					0, 
					( action_log_notify* )this  ); 

				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}
			}
		}	
#else
		ret = show_event_tip( GetHWND()/*GetDesktopWindow()*/, 
			action_msg, 
			data_dump, 
			action_event, 
			tmp_text, 
			0, 
			( action_log_notify* )this ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
#endif //_DEBUG
		//if( ret_status == OK_STATE )
		//{
		//	event_respon.action = ACTION_ALLOW; 
		//}
		//else
		//{
		//	event_respon.action = ACTION_BLOCK; 
		//}

		//ASSERT( TRUE == is_valid_action_type( need_record ) ); 
		//event_respon.need_record = ( BYTE )need_record; 
		//event_respon.id = action_event->id; 

		//ret = do_ui_ctrl( SEVENFW_UI_ID, SYS_MON_EVENT_RESPONSE, ( PBYTE )&event_respon, sizeof( event_respon ), NULL, 0, NULL, NULL ); 

		//if( ret == ERROR_SUCCESS )
		//{
		//	if( need_record == RECORD_APP_ACTION )
		//	{
		//		input_rule_from_action( action_event, 0 ); 
		//	}
		//	else if( need_record == RECORD_APP_ACTION_TYPE )
		//	{
		//		input_rule_from_action( action_event, APP_DEFINE_RULE ); 
		//	}
		//}
_return:
		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n" __FUNCTION__, ret ) ); 

		if( data_dump != NULL )
		{
			free( data_dump ); 
		}

		if( action_msg != NULL )
		{
			free( action_msg ); 
		}

		return ret; 
	}


	LRESULT on_usb_dev_change( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		LRESULT ret = TRUE; //ERROR_SUCCESS; 
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		//ULONG vol_disk_index; 

		//DBG_BP(); 

		switch( wParam )
		{
		case DBT_DEVICEARRIVAL:
			// Check whether a CD or DVD was inserted into a drive.
			if( lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME )
			{
				PDEV_BROADCAST_VOLUME lpdbv = ( PDEV_BROADCAST_VOLUME )lpdb;

				if( lpdbv->dbcv_flags & DBTF_MEDIA )
				{
					//vol_disk_index = first_drive_index_from_mask( lpdbv->dbcv_unitmask ); 

					//if( vol_disk_index != INVALID_DRIVE_INDEX )
					//{
					//	break; 
					//}

					//if( ( vol_disk_sign < L'a' || vol_disk_sign > L'z' ) 
					//	&& ( vol_disk_sign < L'a' || vol_disk_sign > L'z' ) )
					//{

					//}
					update_volume_name_map( ALL_DRIVE_INDEX/*lpdbv->dbcv_unitmask*/, 0 ); 
				}
			}
			//BROADCAST_QUERY_DENY 
			break;

		case DBT_DEVICEREMOVECOMPLETE:
			// Check whether a CD or DVD was removed from a drive.
			if( lpdb-> dbch_devicetype == DBT_DEVTYP_VOLUME )
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;

				if( lpdbv->dbcv_flags & DBTF_MEDIA )
				{
					; 
				}
			}
			break;

		default:
			/*
			Process other WM_DEVICECHANGE notifications for other 
			devices or reasons.
			*/ 
			;
		}
		return ret; 
	}
	
#ifdef DIRECT_SHOW_UI
	LRESULT on_paint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		LRESULT ret = ERROR_SUCCESS; 
		BOOL _ret; 

		bHandled = FALSE; 
		//
		// Render screen
		//
		if( m_anim.IsAnimating() )
		{
			// 3D animation in progress
			m_anim.Render();
			// Do a minimum paint loop
			// Keep the client area invalid so we generate lots of
			// WM_PAINT messages. Cross fingers that Windows doesn't
			// batch these somehow in the future.
			PAINTSTRUCT ps = { 0 };
			::BeginPaint(GetHWND(), &ps);
			::EndPaint(GetHWND(), &ps);
			::InvalidateRect(GetHWND(), NULL, FALSE);
		}
		else if( m_anim.IsJobScheduled() ) {
			// Animation system needs to be initialized
			m_anim.Init(GetHWND());
			// A 3D animation was scheduled; allow the render engine to
			// capture the window content and repaint some other time
			if( !m_anim.PrepareAnimation(GetHWND()) ) 
			{
				m_anim.CancelJobs();
			}

			::InvalidateRect(GetHWND(), NULL, TRUE);
		}
_return:
		return ret; 
	}
#endif //DIRECT_SHOW_UI

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE; 

		do 
		{
			//log_trace( ( MSG_INFO, "%s msg is %u[0x%0.8x], wparam is 0x%0.8x, lparam is 0x%0.8x\n", __FUNCTION__, uMsg, uMsg, wParam, lParam ) ); 

/********************
first click:
CBitSafeFrameWnd::HandleMessage msg is 70[0x00000046], wparam is 0x00000000, lparam is 0x0012f1b4
CBitSafeFrameWnd::HandleMessage msg is 71[0x00000047], wparam is 0x00000000, lparam is 0x0012f1b4
CBitSafeFrameWnd::HandleMessage msg is 28[0x0000001c], wparam is 0x00000001, lparam is 0x0000029c
CBitSafeFrameWnd::HandleMessage msg is 134[0x00000086], wparam is 0x00000001, lparam is 0x00000000
CBitSafeFrameWnd::HandleMessage msg is 6[0x00000006], wparam is 0x00000001, lparam is 0x00000000
CBitSafeFrameWnd::HandleMessage msg is 641[0x00000281], wparam is 0x00000001, lparam is 0xc000000f
CBitSafeFrameWnd::HandleMessage msg is 642[0x00000282], wparam is 0x00000002, lparam is 0x00000000
CBitSafeFrameWnd::HandleMessage msg is 7[0x00000007], wparam is 0x00000000, lparam is 0x00000000

second click:
CBitSafeFrameWnd::HandleMessage msg is 134[0x00000086], wparam is 0x00000000, lparam is 0x00000000
CBitSafeFrameWnd::HandleMessage msg is 6[0x00000006], wparam is 0x00000000, lparam is 0x00000000
CBitSafeFrameWnd::HandleMessage msg is 28[0x0000001c], wparam is 0x00000000, lparam is 0x0000029c
CBitSafeFrameWnd::HandleMessage msg is 8[0x00000008], wparam is 0x00000000, lparam is 0x00000000
CBitSafeFrameWnd::HandleMessage msg is 641[0x00000281], wparam is 0x00000000, lparam is 0xc000000f
CBitSafeFrameWnd::HandleMessage msg is 642[0x00000282], wparam is 0x00000001, lparam is 0x00000000


#define WM_IME_SETCONTEXT               0x0281
#define WM_IME_NOTIFY                   0x0282
#define WM_CTLCOLORLISTBOX              0x0134
#define WM_ACTIVATE                     0x0006
#define WM_NCACTIVATE                   0x0086

//WM_ACTIVATE state values
#define     WA_INACTIVE     0
#define     WA_ACTIVE       1
#define     WA_CLICKACTIVE  2

#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_ACTIVATEAPP                  0x001C

#define WM_WINDOWPOSCHANGING            0x0046

*******************************************/
			//if( uMsg == WM_MOUSEACTIVATE  )
			//{
			//	DBG_BP(); 
			//}

			//if( uMsg == WM_WINDOWPOSCHANGING )
			//{
			//	WINDOWPOS *wnd_pos; 

			//	DBG_BP(); 

			//	wnd_pos = ( WINDOWPOS* )lParam; 
			//
			//	wnd_pos = NULL; 
			//
			//}
			
			switch( uMsg )
			{
			case WM_TRAY_ICON_NOTIFY: lRes = tray_icon_notify(uMsg, wParam, lParam, bHandled); break;
			case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
			case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
			case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
			case WM_NCDESTROY:		lRes = OnNcDestroy( uMsg, wParam, lParam, bHandled ); break; 
			case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
			case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
			case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
			case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
			case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
			case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
			case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break; 
#ifdef DIRECT_SHOW_UI
			case WM_PAINT: 
				lRes = on_paint(uMsg, wParam, lParam, bHandled); 
				break; 
#endif //DIRECT_SHOW_UI
			case WM_KEYDOWN:
				lRes = on_char( uMsg, wParam, lParam, bHandled ); 
				break; 
			case WM_COMMAND: lRes = OnCommand(uMsg, wParam, lParam, bHandled); break; 
			case WM_TIMER: 
				lRes = on_timer( uMsg, wParam, lParam, bHandled ); 
				break; 
			case WM_RESPONSE_EVENT_NOTIFY:
				ASSERT( FALSE ); 
				//lRes = on_response_event( uMsg, wParam, lParam, bHandled ); 
				break;  
#ifdef DIRECT_SHOW_UI
			case WM_LBUTTONDOWN:
				// No need to burden user with 3D animations
					//m_anim.CancelJobs();
			break; 
#endif //DIRECT_SHOW_UI

			case WM_DEVICECHANGE:
				lRes = on_usb_dev_change( uMsg, wParam, lParam, bHandled ); 
				break; 
			default:
				bHandled = FALSE;
			}

			if( bHandled ) 
			{
				break; 
			}

			if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) 
			{
				break; 
			}
			
			lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam); 
		}while( FALSE );

		return lRes; 
	}

public:
	CPaintManagerUI m_pm;

private:
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
	
	proc_list_dlg proc_list; 
	log_dlg sys_log; 
	about_dlg about_box; 
	//url_rule_dlg url_rule; 
	anti_arp_dlg arp_infos; 
	netmon_dlg netmon; 
	tray_icon icon_tray; 
	HICON main_icon; 
	icon_menu tray_icon_menu; 

	LONG upload_x; 
	LONG download_x; 

	LONG upload_right_x; 
	LONG download_right_x; 
	//LONG logo_width; 
	LONG logo_left; 
	ULONG timer_id; 
	ULONG icon_show_time; 
	ULONG icon_showed_time; 
};

#include "datetime.h"
#define foreach_entry_list( list, entry ) for( entry = ( list )->Flink; entry != list; entry = ( entry )->Flink )

VOID test_foreach_macro()
{
	INT32 i; 
	LIST_ENTRY test_list; 
	PLIST_ENTRY entry_alloc; 
	LIST_ENTRY test_list2; 
	PLIST_ENTRY entry_alloc2; 

	InitializeListHead( &test_list ); 
	InitializeListHead( &test_list2 ); 

	for( i = 0; i < 1000; i ++ )
	{
		entry_alloc = ( PLIST_ENTRY )malloc( sizeof( LIST_ENTRY ) ); 

		if( entry_alloc == NULL )
		{
			goto _return; 
		}

		log_trace( ( MSG_INFO, "insert the entry 0x%0.8x to list1\n" ) ); 

		InsertHeadList( &test_list, entry_alloc ); 

		entry_alloc2 = ( PLIST_ENTRY )malloc( sizeof( LIST_ENTRY ) ); 

		if( entry_alloc2 == NULL )
		{
			goto _return; 
		}

		log_trace( ( MSG_INFO, "insert the entry 0x%0.8x to list2\n" ) ); 

		InsertHeadList( &test_list2, entry_alloc2 ); 
	}

	foreach_entry_list( &test_list2, entry_alloc2 )
	{
		log_trace( ( MSG_INFO, "foreach traverse the list1, entry is 0x%0.8x\n", entry_alloc2 ) ); 

	}

	entry_alloc2 = test_list2.Flink; 
	for( ; ; )
	{
		if( entry_alloc2 == &test_list2 )
		{
			break; 
		}

		log_trace( ( MSG_INFO, "traverse the list 2 entry is 0x%0.8x \n", entry_alloc2 ) ); 
		entry_alloc2 = entry_alloc2->Flink; 
	}

_return:
	return; 
}

#include "file_enc.h"

#define TEST_PWD _T( "hello" )
LRESULT test_file_enc()
{
	LRESULT ret = ERROR_SUCCESS; 
	StringList all_end_flle; 
	TCHAR test_pwd[] = TEST_PWD; 
	TCHAR test_pwd2[] = TEST_PWD; 

	init_crypt_context(); 
	all_end_flle.push_back( _T( "c:\\netcfg.rar" ) ); 
	encrypt_files( &all_end_flle, test_pwd, _tcslen( TEST_PWD ) ); 

	all_end_flle.clear(); 
	all_end_flle.push_back( _T( "c:\\netcfg.rar.aes" ) ); 
	decrypt_files( &all_end_flle, test_pwd2, _tcslen( TEST_PWD ) ); 
	uninit_crypt_context(); 
_return:
	return ret; 
}


#ifdef _DEBUG
#include "acl_cache.h"
#include <wininet.h>

LRESULT test_acl_cache()
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 

	ntstatus = cache_test(); 

	return ret; 
}

#include "mem_region_list.h"
#include "install_conf.h"

#endif //_DEBUG
#include "crash_rpt_supp.h"

#define NEED_SHOW_MSG 0x00000001
#define NEED_UNINSTALL_DRIVER 0x00000002

LRESULT uninstall_old_version()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_UNINSTALL_ALL_DRIVERS, NULL, 0, NULL, 0, NULL, NULL ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unistall all drivers error\n" ) );  
	}

	//ret = uninstall_bitsafe_by_conf_file( NULL ); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	DBG_BP(); 
	//	log_trace( ( MSG_ERROR, "uninstall bitsafe by configuration file error 0x%0.8x\n", ret ) ); 
	//}

	return ret; 
}

LRESULT update_other_components( ULONG flags )
{
	LRESULT ret; 
	HANDLE update_event; 
	LPCTSTR tmp_text; 
	dlg_ret_state ret_state; 

	//DBG_BP(); 
	ret = init_all_ui_ctrls(); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( flags & NEED_UNINSTALL_DRIVER )
	{
		ret = uninstall_old_version(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "uninstall all drivers when updating error 0x%0.8x\n", ret ) ); 
		}
	}

	ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_UPDATE_ALL_DRIVERS, NULL, 0, NULL, 0, NULL, NULL ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 

		log_trace( ( MSG_INFO, "update all drivers when updating error 0x%0.8x\n", ret ) ); 
	}

	if( flags & NEED_SHOW_MSG )
	{
		tmp_text = _get_string_by_id( TEXT_UNINSTALL_NEED_RESTART, 
			_T( "运行比特安全需要重启系统." ) ); 

		ret = show_msg( NULL, tmp_text, &ret_state ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
		}

		//if( ret_state == OK_STATE )
		//{
		//	windows_shutdown( EWX_REBOOT | EWX_FORCE );  
		//}

		//tmp_text = _get_string_by_id( TEXT_UPDATE_OLD_VERSION_HAVE_NOT_UNINSTALL, 
		//	_T( "由于系统环境原因,卸载其它组件失败,重新运行程序(新版本)时,要先执行卸载(如出现安装提示,先不要执行安装),在保证卸载全部完成后,再重新安装并运行程序。" ) ); 

		//show_msg( NULL, tmp_text ); 
	}

	do 
	{
		BOOL _ret; 
		update_event = OpenEvent( EVENT_MODIFY_STATE, FALSE, UPDATE_DRV_EVENT_NAME ); 
		if( update_event == NULL )
		{
			ret = GetLastError(); 
			break; 
		}

		_ret = SetEvent( update_event ); 
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			break; 
		}
	}while ( FALSE ); 

	if( update_event != NULL )
	{
		CloseHandle( update_event ); 
	}

_return:
	return ret; 
}

LRESULT check_os_compatible()
{
	LRESULT ret; 
	INT32 os_bit; 

	ret = get_sys_bits( &os_bit, NULL ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( os_bit != 32 )
	{
		if( os_bit == 64 )
		{
			show_msg( NULL, _T( "WINDOWS7 64位系统需要对此程序支持库进行验证,为保证比特安全正确加载,请安装:" ) ); 
		}
		ret = ERROR_NOT_SUPPORTED; 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT CALLBACK update_ui( PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	CPaintManagerUI *main_wnd_pm; 

	main_wnd_pm = ( CPaintManagerUI* )context; 

	main_wnd_pm->SetTransparent( get_tranparent() ); 
	
	if( get_theme_no() == INVALID_THEME_NO )
	{
		goto _return; 
	}

	ret = load_theme_setting( main_wnd_pm ); 

_return:
	return ret; 
}

#ifdef _DEBUG
LRESULT install_sevenfw_ex(); 
LRESULT install_sevenfw(); 
VOID unload_sevenfw_driver(); 
#define unload_sevenfw_ex_driver() unload_sevenfw_driver() 
LRESULT delete_sevenfw_driver(); 
LRESULT delete_sevenfw_ex_driver(); 
LRESULT uninstall_sevenfw(); 
LRESULT uninstall_sevenfw_ex(); 

LRESULT test_sevenfw_driver_install()
{
	LRESULT ret = ERROR_SUCCESS; 
	
	ret = install_sevenfw_ex(); 
	
	DBG_BP(); 
	ret = uninstall_sevenfw_ex(); 

	return ret; 
}

#endif //_DEBUG

//#define TEST_SEVENFW_WIN7_INSTALL

INT32 win7_dbg_bp()
{
	INT32 *bug_ptr = NULL; 
	*bug_ptr = 0; 

	return *bug_ptr; 
}

#define TEST_IP_ADDR "192.168.19.32" 
LRESULT test_network_byte_order()
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ip_addr_test; 

	ip_addr_test = inet_addr( TEST_IP_ADDR ); 

	log_trace( ( MSG_INFO, "the ip: %s, %u 0x%0.8x network byte order dump:\n", TEST_IP_ADDR, ip_addr_test, ip_addr_test ) ); 
	DUMP_MEM( &ip_addr_test, sizeof( ip_addr_test ) ); 

	ip_addr_test = ntohl( ip_addr_test ); 
	log_trace( ( MSG_INFO, "the ip: %s, %u 0x%0.8x network byte order dump:\n", TEST_IP_ADDR, ip_addr_test, ip_addr_test ) ); 
	DUMP_MEM( &ip_addr_test, sizeof( ip_addr_test ) ); 

	return ret; 
}

//#define TEST_CRT_MEM_LEAN 1

#ifdef TEST_CRT_MEM_LEAN
#include "test_crt_mem_leak.H"
#endif //TEST_CRT_MEM_LEAN

//#define TEST_AUTO_RUN
//why i connect to this site 220.181.26.195, upx do it?

INT32 APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR cmd_line; 
	INT32 argc; 
	INT32 com_inited = FALSE; 
	LPWSTR *argv; 
	WCHAR *_update_path = NULL; 
	WCHAR *_app_file_path = NULL; 
	BOOL _ret; 
	CHAR exe_file_name[ _MAX_PATH ];
	DWORD file_name_len; 
	CBitSafeFrameWnd* pFrame = NULL; 
#ifdef _DEBUG
	INT32 dbg_ctr_flag;
#endif //_DEBUG

	//__asm int 3; 

	//DBG_BP(); 
	//test_generate_report(); 

	//win7_dbg_bp(); 

	ret = init_std_lib_mbc_local_lang(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_FATAL_ERROR, "set std lib locale language error\n" ) ); 
	}

#ifdef TEST_CRT_MEM_LEAN
	return ( INT32 )test_crt_mem_leak_dbg(); 
	
#endif //TEST_CRT_MEM_LEAN

#ifdef TEST_AUTO_RUN
	ret = set_self_autorun( 0 ); 
	ret = set_self_autorun( 0 ); 
	ret = set_self_autorun( ADD_SELF_AUTO_RUN ); 
#endif //TEST_AUTO_RUN
	
#ifdef TEST_SEVENFW_WIN7_INSTALL
	DBG_BP(); 
	test_sevenfw_driver_install(); 
	return ret; 
#endif //TEST_SEVENFW_WIN7_INSTALL

	//DBG_BRK(); 

#ifdef _DEBUG
 	CPaintManagerUI::SetInstance( hInstance ); 

	//test_network_byte_order(); 
	//test_acl_cache(); 
	//test_offset_list(); 
	{
		DWORD err_code; 
		CHAR msg[ 1024 ]; 
		ULONG msg_len = 1024; 

		_ret = InternetGetLastResponseInfoA( &err_code, msg, &msg_len );
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "get last internet error code message failed\n", ret ) ); 		
		}
	}

#endif //_DEBUG
//#define UPDATE_URL "http://codereba.com/update/update.php?action=manual"

#ifdef _DEBUG
	//test_file_enc();
#endif //_DEBUG
	//ret = file_exists( UI_FILE_NAME, FALSE ); 
	//if( ERROR_SUCCESS != ret )
	//{
	//	ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_UI_ZIP_FILE ), DAT_FILE_RES_TYPE, UI_FILE_NAME, FALSE );
	//	
	//	if( ret != ERROR_SUCCESS )
	//	{
	//		log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw file from resource failed \n" ) ); 
	//		goto _return; 
	//	}
	//}

#ifdef _DEBUG

//#define new DEBUG_CLIENTBLOCK
	dbg_ctr_flag = _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif //_DEBUG

	CPaintManagerUI::SetInstance( hInstance ); 
	CPaintManagerUI::SetResourcePath( CPaintManagerUI::GetInstancePath() );
	//CPaintManagerUI::SetResourceZip( UI_FILE_NAME );
	CPaintManagerUI::SetResourceZipResType( DAT_FILE_RES_TYPE, IDR_UI_ZIP_FILE ); 

	//init_service_context_async(); 

	ret = init_bitsafe_conf(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load bitsafe config failed \n" ) ); 

		ASSERT( FALSE && "load bitsafe config failed \n" ); 

		goto _return; 
	}

	HRESULT Hr = ::CoInitialize(NULL);

	if( FAILED(Hr) ) 
	{
		ASSERT( FALSE && "initialize the com context failed\n" ); 

		goto _return; 
	}

	com_inited = TRUE; 

	cmd_line = GetCommandLineW(); 
	argv = CommandLineToArgvW( cmd_line, &argc ); 

	if( argc == NULL )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!load command line of the process failed\n" ) ); 
	}

	//ret = clone_start_self(); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	log_trace( ( MSG_WARNING, "clone start update self failed \n" ) ); 
	//
	//}

	// test param:  /u "D:\AntiArp\ui\SevenFw\SevenFwUI\duilib v1.1\bin\bitsafev2.exe"
	if( argv != NULL && argc > 1 )
	{
		if( argc == 3 
			&& _wcsicmp( argv[ 1 ], L"/u" ) == 0 
			&& *argv[ 2 ] != L'\0' )

		{
			CHAR update_path[ _MAX_URL_LEN ]; 
			//PVOID update_handle; 
			LPCTSTR tmp_text; 

			ret = check_os_compatible(); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}


			//ret = init_crash_rpt(); 
			//if( ret != ERROR_SUCCESS )
			//{
			//	log_trace( ( MSG_ERROR, "initialize the crash report failed" ) ); 

			//	goto _return; 
			//}

			load_lang_conf(); 

			_app_file_path = argv[ 2 ]; 
			log_trace( ( MSG_INFO, "update target path is %ws \n", _app_file_path ) ); 

			if( *_app_file_path == L'\0' )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				goto _return;; 
			}

			ret = check_app_instance( UPDATE_INSTANCE ); ; 
			if( ret == ERROR_SUCCESS )
			{
				goto _return; 
			}

			ret = get_update_path( update_path, _MAX_URL_LEN ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			update_dlg* _update_dlg = new update_dlg( );
			if( _update_dlg == NULL ) 
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				goto _return; 
			}

			_update_path = StringConvertor::AnsiToWide( update_path ); 
			if( _update_path == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				goto _return; 
			}

			_update_dlg->set_update_url( _update_path ); 
			_update_dlg->set_target_path( _app_file_path ); 

			tmp_text = _get_string_by_id( TEXT_UPDATE_TITLE, _T("更新比特安全") ); 

			_update_dlg->Create(NULL, tmp_text, UI_WNDSTYLE_FRAME, 0L, 0, 0, 800, 572);
			_update_dlg->CenterWindow();
			_update_dlg->SetIcon( IDI_MAIN_ICON ); 
			::ShowWindow( *_update_dlg, SW_HIDE ); 
			CPaintManagerUI::MessageLoop(); 

			delete _update_dlg; 
			goto _return; 
		}
		else if( argc == 2 )
		{
			CHAR svc_cmd_line[ MAX_CML_LINE_LEN ]; 

			file_name_len = GetModuleFileNameA( NULL, exe_file_name, _MAX_PATH ); 

			if( file_name_len == 0 )
			{
				ret = GetLastError(); 
				goto _return; 
			}

			if( 0 == _wcsicmp( argv[ 1 ], L"/dbg" ) )
			{
				ret = init_all_ui_ctrls(); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				ret = ctrl_dbg_mode( TRUE ); 
				if( ret != ERROR_SUCCESS )
				{
					DBG_BP(); 
				}

				set_dbg_log_level( 0 ); 

				ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_ENTER_DBG_MODE, NULL, 0, NULL, 0, NULL, NULL ); 
				
				goto _normal_start; 
			}
			else if( _wcsicmp( argv[ 1 ], L"/i" ) == 0 )
			{
				strcpy( svc_cmd_line, "\"" ); 
				strncat( svc_cmd_line, exe_file_name, MAX_CML_LINE_LEN - strlen( svc_cmd_line ) ); 
				strncat( svc_cmd_line, "\"", MAX_CML_LINE_LEN - strlen( svc_cmd_line ) ); 
				strncat( svc_cmd_line, " /s", MAX_CML_LINE_LEN - strlen( svc_cmd_line ) ); 

				//stop_service( SERVICE_NAME );
				//uninstall_service( SERVICE_NAME );
				install_service( svc_cmd_line, SERVICE_NAME );
				start_service( SERVICE_NAME, 0, NULL ); 
				goto _return; 
			}
			else if( _wcsicmp( argv[ 1 ], L"/d" ) == 0 )
			{
				stop_service( SERVICE_NAME );
				uninstall_service( SERVICE_NAME );
				goto _return; 
			}
			else if( _wcsicmp( argv[ 1 ], L"/s" ) == 0 )
			{
				//load_lang_conf(); 


				//__asm int 3; 

				ret = check_os_compatible(); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				file_name_len = GetModuleFileNameA( NULL, exe_file_name, _MAX_PATH ); 

				if( file_name_len == 0 )
				{
					ret = GetLastError(); 
					goto _return; 
				}

				strcpy( svc_cmd_line, "\"" ); 
				strncat( svc_cmd_line, exe_file_name, MAX_CML_LINE_LEN - strlen( svc_cmd_line ) ); 
				strncat( svc_cmd_line, "\"", MAX_CML_LINE_LEN - strlen( svc_cmd_line ) ); 
				strncat( svc_cmd_line, " /daemon", MAX_CML_LINE_LEN - strlen( svc_cmd_line ) ); 

				log_trace( ( MSG_INFO, "service command line is %s \n", svc_cmd_line ) ); 

				ret = init_proc_manage_context( 1 ); 
				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( DBG_MSG_AND_ERROR_OUT, "init proc manage context failed \n" ) ); 
					goto _return; 
				}

				ret = set_proc_work_param( 0, svc_cmd_line, "", UI_PROCESS, 0 ); 
				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( DBG_MSG_AND_ERROR_OUT, "set process work parameters failed \n" ) ); 
					goto _return; 
				}

				log_trace( ( MSG_INFO, "start service dispatch \n" ) ); 
				_ret = StartServiceCtrlDispatcherA( service_dispatch_table ); 
				if( _ret == FALSE )
				{
					ret = GetLastError();
					log_trace( ( DBG_MSG_AND_ERROR_OUT, "StartServiceCtrlDispatcher failed \n" ) ); 
					goto _return; 
				}

				//uninit_proc_manage_context(); 
				goto _return; 
			}
			else if( _wcsicmp( argv[ 1 ], L"/daemon" ) == 0 )
			{
				log_trace( ( MSG_INFO, "enter daemon\n" ) ); 


				load_lang_conf(); 

				ret = check_app_instance( DAEMON_INSTANCE ); ; 
				if( ret == ERROR_SUCCESS )
				{
					goto _return; 
				}

				ret = init_all_ui_ctrls(); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				ret = pipe_daemon(); 
				goto _return; 
			}
			else if( _wcsicmp( argv[ 1 ], L"/updated" ) == 0 )
			{
				ret = update_other_components( 0 ); 
			}
			else if( _wcsicmp( argv[ 1 ], L"/uninstall" ) == 0 )
			{
				ret = init_all_ui_ctrls(); 
				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( MSG_ERROR, "initialize the all ui controls error 0x%0.8x\n", ret ) ); 
					goto _return; 
				}
				ret = uninstall_old_version(); 
			}
			else if( _wcsicmp( argv[ 1 ], L"/installed" ) == 0 )
			{
				ret = update_other_components( 0 ); 
			}
			else if( _wcsicmp( argv[ 1 ], L"/restart" ) == 0 )
			{
				DBG_BP(); 
				ret = del_must_restart_mark(); 
			}

			goto _return; 
		}
	}
	else
	{
		ret = ctrl_dbg_mode( FALSE ); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
		}

_normal_start:
		log_trace( ( MSG_INFO, "start normal\n" ) );

		//ret = del_must_restart_mark(); 

#ifdef TEST_DBG_MODE
		do{
			BOOLEAN dbg_mode; 
			ret = get_dbg_mode( &dbg_mode ); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BP(); 
				break; 
			}

			if( dbg_mode == TRUE )
			{
				ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_ENTER_DBG_MODE, NULL, 0, NULL, 0, NULL, NULL ); 
			}
		}while( FALSE ); 
#endif //TEST_DBG_MODE

		ret = check_must_restart_mark(); 
		if( ret == ERROR_SUCCESS )
		{
			DBG_BP(); 
			show_msg( NULL, _get_string_by_id( TEXT_COMMON_MUST_RESTART, _T( "在系统未重启前,比特安全的功能无法正常加载." ) ) ); 

			goto _return; 
		}

		ret = check_os_compatible(); 
		if( ret != ERROR_SUCCESS )
		{

			show_msg( NULL, _get_string_by_id( TEXT_COMMON_NOT_SUPPORT_64BIT_OS, _T( "当前7层防火墙支持32位WINDOWS系统,暂时不支持64位WINDOWS系统。" ) ) ); 

			goto _return; 
		}

		ret = check_app_instance( 0 ); 
		if( ret == ERROR_SUCCESS )
		{
			ASSERT( FALSE && "bitsafe aleady running \n" ); 
			goto _return; 
		}

//#ifdef _DEBUG
//		ret = safe_output_themes(); 
//#endif //_DEBUG

#ifndef NO_CRASH_REPORT
		ret = init_crash_rpt(); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "initialize the crush report error\n" ); 
			//goto _return; 
		}
#endif  //NO_CRASH_REPORT

		pFrame = new CBitSafeFrameWnd();
		if( pFrame == NULL ) 
		{
			ASSERT( FALSE && "allocate new bitsafe main window error\n" ); 
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		{
			BOOLEAN have_conf_file = FALSE; 
			chg_original_lang( NULL, &have_conf_file ); 
			if( FALSE == have_conf_file )
			{
				ret = config_bitsafe(); 
				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( MSG_ERROR, "configure the bitsafe error 0x%0.8x\n", ret ) ); 
				}
			}
		}

		
#ifdef DEBUG_CRASH_RPT
		argv = NULL; 
		*argv = 0; 
#endif //DEBUG_CRASH_RPT

#ifndef _SIMULATE_TEST
		//ret = ERROR_SUCCESS; 
		ret = check_run_service(); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "start system manage service error\n" ); 
			log_trace( ( MSG_ERROR, "start system manage service error\n" ) ); 
			//goto _return; 
		}
#endif //_SIMULATE_TEST
		//goto _return; 

		ret = start_clone_process(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_WARNING, "clone start update self error \n" ) ); 
		}

//#define SQLITE_EXT_STACK_CRRUPT
#ifdef SQLITE_EXT_STACK_CRRUPT
		DBG_BP(); 
		test_sqlite3_ext_stack_overflow(); 
#endif //SQLITE_EXT_STACK_CRRUPT

		ret = init_all_ui_ctrls(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		//#ifdef _DEBUG
		//	test_foreach_macro(); 
		//#endif
		
		ret = open_function_conf(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = init_bitsafe_function_state(); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "load driver work state error" ); 
			goto _return; 
		}

		ret = safe_open_bitsafe_conf(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = safe_output_themes(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "output bitsafe themes error 0x%0.8x\n", ret ) ); 
		}

		//time_t time_test; 
		//time_t time_test2; 

		//time( &time_test2 ); 
		//str2time( _T( "2011-09-13 10:43:32" ), &time_test ); 

		//test_conf_function( TEST_COMMON_DRIVER_RULE ); 

		//input_rule_from_conf_file( _T( "black_list.txt" ) ); 
#ifdef _SIMULATE_TEST
		input_rule_from_conf_file( _T( "black_list.txt" ) ); 
		////add_test_url_rule(); 
		check_access_rule_size_limit(); 
		test_conf_function( TEST_COMMON_DRIVER_RULE ); 
		//init_service_context_async(); 
#endif //_DEBUG


		//ret = stop_net_cfg(); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	ASSERT( FALSE ); 
		//	log_trace( ( MSG_ERROR, "***start net configure management failed \n" ) ); 
		//}

		//!! need move some context initialize code to here, like driver load, access rule load etc.

		LPCTSTR tmp_text; 

		tmp_text = _get_string_by_id( TEXT_MAIN_BITSAFE_TITLE, _T("比特安全") ); 

		pFrame->Create(NULL, tmp_text, ( UI_WNDSTYLE_DIALOG /*& ( ~WS_DLGFRAME )*/ ), 0, 0, 0, 800, 572);
		pFrame->CenterWindow();
		pFrame->SetIcon( IDI_MAIN_ICON ); 
		::ShowWindow( *pFrame, SW_SHOW );


#ifdef _SIMULATE_TEST
		CPaintManagerUI::MessageLoop();
		stop_test_conf_function(); 

		pFrame = NULL; 
		goto _return; 
#endif 
	}

	CPaintManagerUI::MessageLoop(); 
	pFrame = NULL; 

_return:
	_CrtMemDumpAllObjectsSince( NULL );
	_CrtDumpMemoryLeaks(); 

	if( com_inited == TRUE )
	{
		::CoUninitialize();
	}

	if( _update_path != NULL )
	{
		StringConvertor::StringFree( _update_path ); 
	} 

	if( pFrame != NULL )
	{
		delete pFrame; 
	}

	return ( INT32 )ret;
}
