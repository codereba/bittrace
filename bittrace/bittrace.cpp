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
#include "bitsafe_common.h"
#include "bittrace.h"
#include "ui_ctrl.h"
#include "about_dlg.h"
#include "ui_ctrl_mgr.h"
#include "msg_box.h"
#include "acl_define.h"
#include "one_instance.h"
#include "UIUtil.h"
#include "bitsafe_conf.h"
#include "update.h"
#include "update_dlg.h"
#include "bittrace_daemon.h"
#include "tray_icon.h"
#include "icon_menu.h"
#include "msg_tip.h"
#include "lang_dlg.h"
#include "suggestion_dlg.h"
#include "donate_dlg.h"
#include "common_config.h"
#include "dbg_panel.h"
#include "tab_builder.h"
#include "runtime_msg_box.h"
#include "volume_name_map.h"
#include "action_log_db.h"
#include "action_logger.h"
#include <dbt.h>
#define INITGUID
#include <Wmistr.h>
#include <evntrace.h>
#include "tracer.h"
#include "_resource.h"
#include "bittrace_svc.h"
#include "stack_dump.h"
#include "list_ex_common.h"
#include "UIListSubElementEx.h"
#include "win_impl_base.hpp"
#include "filter_cond_db.h"
#include "list_log.h"
#include "trace_frame.h"
#include "filter_dlg.h"
#include "action_result.h"
#include "trace_filter.h"
#include "event_memory_store.h"
#include "focus_proc_dlg.h"
#include "search_dlg.h"
#include "load_log_dlg.h"
#include "backup_log_dlg.h"
#include "option_dlg.h"
#include "peripheral_list_dlg.h"
#include <windowsx.h>
#include <olectl.h>

#define NO_SUPPORT_32BIT_PROCCESS 1

#ifndef LANG_EN
#define GIVE_SUGESTION_TITLE_TEXT L"�������"
#define OPTION_DLG_TITLE_TEXT L"ѡ��" 
#define UPDATE_DLG_TITLE_TEXT L"���±��ذ�ȫ"
#define MAIN_BITTRACE_TITLE_TEXT L"���ظ���"
#define UI_CUSTOM_DLG_TITLE_TEXT L"UI��������"
#define INSTALL_CODEREBA_CERTIFICATE_TIP_TEXT L"WINDOWS7 64λϵͳ��Ҫ�Լ��ص�����ǩ����֤,��Ҫ��װCODEREBA˽��֤�������Ϳ��ŷ�������(BITTRACE����ȫ��ѵ�����,�����ʹ����������,��֧��1Ԫ����һ��֤��(http://www.simpai.net/support.php)),ʹBITTRACE��ȷ����,CODEREBA��֤BITTRACE��ֻ�и��ٺ���־��������,�ҳ����º��û�����֮��û��BIT���κ�ͨ��,����ϵͳ������Ϊ������ȫ���,����?"
#else
#define GIVE_SUGESTION_TITLE_TEXT L"Suggestion"
#define OPTION_DLG_TITLE_TEXT L"Options" 
#define UPDATE_DLG_TITLE_TEXT L"Update bittrace"
#define MAIN_BITTRACE_TITLE_TEXT L"BITTRACE"
#define UI_CUSTOM_DLG_TITLE_TEXT L"UI customize"
#define INSTALL_CODEREBA_CERTIFICATE_TIP_TEXT L"WINDOWS7 64 will check driver signing, run bittrace need install CODEREBA private ceritificate to root ceritifcate store(BITTRACE is free,if you like it,please donate 1 dollar to buy one certificate.(http://www.simpai.net/support_en.php)),CODEREBA promises BITTRACE only have trace and log analyze functions,and have no any bit communication except updating and user operation, every action of system captured will be all outputed,continue?"
#endif //LANG_EN

extern BOOLEAN process_log_sw; 

LRESULT WINAPI get_filter_info_from_event_ui( CControlUI *ctrl, 
											 LPCWSTR filter_mode_name, 
											 action_filter_info *filter_info ); 

INT32 WINAPI is_same_filter_info( action_filter_info *src_flt_info, action_filter_info *dst_flt_info ); 

LRESULT WINAPI delete_filter_condition( action_filter_info *filter_condition ); 

ULONGLONG log_mem_threshold = 0xffffffffffffffff; 

LRESULT WINAPI check_and_set_log_mem_threshold()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI copy_text_to_clipboard( 	HWND wnd_handle, 
									  LPCWSTR text, 
									  ULONG cc_text_len ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	LPWSTR lptstrCopy; 
	HGLOBAL hglbCopy; 
	BOOLEAN clipboard_opened = FALSE; 

	do 
	{
		if( NULL == text )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( 0 == cc_text_len )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( wnd_handle == NULL ) 
		{
			ret = ERROR_NOT_READY; 
			break; 
		}


		if( !OpenClipboard( wnd_handle ) ) 
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		clipboard_opened = TRUE; 

		EmptyClipboard(); 

		hglbCopy = GlobalAlloc( GMEM_MOVEABLE, 
			( cc_text_len + 1 ) * sizeof( WCHAR ) ); 
		
		if( hglbCopy == NULL ) 
		{ 
			break; 
		}

		lptstrCopy = ( LPWSTR )GlobalLock( hglbCopy ); 
		
		memcpy( lptstrCopy, 
			text, 
			cc_text_len * sizeof( WCHAR ) ); 

		lptstrCopy[ cc_text_len ] = ( WCHAR )0; 
		
		GlobalUnlock( hglbCopy ); 

		if( NULL == SetClipboardData( CF_UNICODETEXT, hglbCopy ) )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	if( clipboard_opened == TRUE )
	{
		CloseClipboard(); 
	}

	return ret; 
}

LRESULT WINAPI reg_jump( LPCTSTR KeyPath, LPCTSTR ValueName )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG _ret; 
	INT32 __ret; 
	DWORD   dwPid = 0;
	HANDLE  hProcess = NULL;
	HWND    hRegeditMainHwnd;

	do 
	{
		if( NULL == KeyPath )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hRegeditMainHwnd = ::FindWindow(_T("RegEdit_RegEdit"), NULL);
		if (hRegeditMainHwnd == NULL)
		{
			SHELLEXECUTEINFO sei = { 0 };
			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			sei.lpVerb = _T("open");
			sei.lpFile = _T("regedit.exe");
			sei.hwnd = NULL;
			sei.nShow = SW_SHOWNORMAL;
			sei.cbSize = sizeof(SHELLEXECUTEINFO);

			__ret = ShellExecuteEx(&sei);
			if( FALSE == __ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
			}
			else
			{
				WaitForInputIdle(sei.hProcess, 500); 
			}

			hRegeditMainHwnd = ::FindWindow(_T("RegEdit_RegEdit"), NULL);
			if( hRegeditMainHwnd == NULL )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			hProcess = sei.hProcess; 
		}
		else
		{
			_ret = GetWindowThreadProcessId( hRegeditMainHwnd, &dwPid );
			if( _ret == 0 )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "get the thread and the process that created the windows error" ); 
				break; 
			}

			hProcess = OpenProcess( SYNCHRONIZE, FALSE, dwPid );
			if( hProcess == NULL )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "get the process handle of the regedit error" ); 
				break; 
			}
		}

		::ShowWindow(hRegeditMainHwnd, SW_SHOWNORMAL); 
		HWND hRegeditKeyTree = ::FindWindowEx(hRegeditMainHwnd, NULL, _T("SysTreeView32"), NULL);
		if (hRegeditKeyTree)
		{
			::SetForegroundWindow( hRegeditKeyTree ); 

			for ( int i = 0; i < 30; i++ )
			{
				::SendMessage(hRegeditKeyTree, WM_KEYDOWN, VK_LEFT, 0);
			}

			_ret = WaitForInputIdle( hProcess,100 );
			if( _ret != 0 )
			{
				dbg_print( MSG_FATAL_ERROR, "wait the window of the regedit to input idle failed. %u\n", _ret ); 
			}

			::SendMessage(hRegeditKeyTree, WM_KEYDOWN, VK_RIGHT, 0);
			_ret = WaitForInputIdle( hProcess,100 );
			if( _ret != 0 )
			{
				dbg_print( MSG_FATAL_ERROR, "wait the window of the regedit to input idle failed. %u\n", _ret ); 
			}

			for (int i = 0; KeyPath[i] != _T('\0'); i++)
			{
				if (KeyPath[i] == _T('\\'))
				{
					::SendMessage(hRegeditKeyTree, WM_KEYDOWN, VK_RIGHT, 0);
					_ret = WaitForInputIdle( hProcess,100 );
					if( _ret != 0 )
					{
						dbg_print( MSG_FATAL_ERROR, "wait the window of the regedit to input idle failed. %u\n", _ret ); 
					}
				}
				else
					::SendMessage(hRegeditKeyTree, WM_CHAR, KeyPath[i], 0);
			}
		}

		if (NULL != ValueName && ValueName[0] != _T('\0'))
		{
			HWND hWnd = ::FindWindowEx(hRegeditMainHwnd, NULL, _T("SysListView32"), NULL);
			if (hWnd)
			{
				::SetForegroundWindow(hWnd);
				_ret = WaitForInputIdle( hProcess,100 );
				if( _ret != 0 )
				{
					dbg_print( MSG_FATAL_ERROR, "wait the window of the regedit to input idle failed. %u\n", _ret ); 
				}

				for (int i = 0; ValueName[i] != _T('\0'); i++)
				{
					::SendMessage(hWnd, WM_CHAR, ValueName[i], 0);
				}
			}
		}

	}while( FALSE );

	if( hProcess != NULL )
	{
		CloseHandle( hProcess ); 
	}

	return ret; 
}

LRESULT jump_to_periphral_device( LPCWSTR obj_path )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		SHELLEXECUTEINFO sei = { 0 };
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpVerb = _T("open");
		sei.lpFile = _T("hdwwiz.cpl");
		sei.hwnd = NULL;
		sei.nShow = SW_SHOWNORMAL;
		sei.cbSize = sizeof(SHELLEXECUTEINFO);

		_ret = ShellExecuteEx(&sei);
		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI jump_to_reg_object( LPCWSTR obj_path )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR reg_path[ _MAX_REG_PATH_LEN ]; 
	ULONG key_name_len; 
	WCHAR *value_name; 
	LPCWSTR _obj_path; 

	do 
	{
		if( obj_path == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		value_name = wcschr( ( LPWSTR )obj_path, L',' ); 

		if( value_name != NULL )
		{
			value_name += 1; 

			key_name_len = ( value_name - obj_path ); 

			if( ARRAYSIZE( reg_path ) < key_name_len )
			{
				ret = ERROR_BUFFER_OVERFLOW; 
				break; 
			}

			memcpy( reg_path, obj_path, ( key_name_len << 1 ) ); 
			reg_path[ key_name_len - 1 ] = L'\0'; 

			_obj_path = reg_path; 
		}
		else
		{
			_obj_path = obj_path; 
			//value_name = NULL; 
		}

		/***************************************************************************
		'Launches Registry Editor with the chosen branch open automatically

		'Author  : Ramesh Srinivasan

		'Website: http://windowsxp.mvps.org



		Set WshShell = CreateObject("WScript.Shell")

		Dim MyKey

		MyKey = Inputbox("Type the Registry path")

		MyKey = "My Computer\" & MyKey

		WshShell.RegWrite "HKCU\Software\Microsoft\Windows\CurrentVersion\Applets\Regedit\Lastkey",MyKey,"REG_SZ"

		WshShell.Run "regedit", 1,True

		Set WshShell = Nothing
		***************************************************************************/

		ret = reg_jump( _obj_path, value_name ); 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI jump_to_file_object( LPCWSTR obj_path )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	WCHAR cmd_line[ MAX_CMD_LINE_LEN ]; 

	do 
	{
		if( obj_path == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		/***********************************************************************
		Explorer [/n] [/e] [(,)/root,<object>] [/select,<object>] 
		***********************************************************************/

		hr = StringCbCopyW( cmd_line, sizeof( cmd_line ), L"explorer /select," ); 

		if( FAILED( hr ) )
		{
			break; 
		}

		hr = StringCbCatW( cmd_line, sizeof( cmd_line ), obj_path ); 

		if( FAILED( hr ) )
		{
			break; 
		}

		ret = run_proc( cmd_line ); 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI jump_to_object( LPCWSTR obj_path, action_main_type type )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( obj_path != NULL ); 

		if( obj_path == NULL )
		{
			break; 
		}

		switch( type )
		{
		case MT_execmon: 
		case MT_filemon: 
			ret = jump_to_file_object( obj_path ); 
			break; 
		case MT_regmon: 
			ret = jump_to_reg_object( obj_path ); 
			break; 
		case MT_procmon: 
		case MT_common: 
		case MT_sysmon: 
		case MT_w32mon: 
		case MT_netmon: 
		case MT_behavior: 
		case MT_unknown: 

			ULONG path_len; 

			path_len = wcslen( obj_path ); 

			if( path_len < 2 )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( obj_path[ 1 ] == L':' )
			{
				ret = jump_to_file_object( obj_path ); 
			}
			else if( obj_path[ 0 ] == L'H'  
				&& obj_path[ 1 ] == L'K' )
			{
				ret = jump_to_reg_object( obj_path ); 				
			}
			else
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 

		case MT_peripheral:
			ret = jump_to_periphral_device( obj_path ); 
			break; 
		default:
			break; 
		}

	}while( FALSE ); 

	return ret; 
}

#define NO_CRASH_REPORT


#define HAVE_LOG_MONITOR
#define WM_TRAY_ICON_NOTIFY ( WM_USER + 101 )
#define WM_RESPONSE_EVENT_NOTIFY ( WM_USER + 102 )
#define WM_SLOGAN_NOTIFY ( WM_USER + 103 )

#define TRAY_ICON_ID 102
#define DRIVER_TEST

void WINAPI process_log(PEVENT_TRACE pEvent); 
LRESULT CALLBACK update_ui( PVOID context ); 

#pragma pack(1)
typedef struct _ACTION_TRACE_HEADER
{
	ULONG seq_num; 
	LARGE_INTEGER time_stamp; 
	ULONG proc_id; 
	ULONG thread_id; 
}ACTION_TRACE_HEADER, *PACTION_TRACE_HEADER; 

#pragma pack()

thread_manage log_work_threads[ 1 ] = { 0 }; 
trace_session session = { 0 }; 

LRESULT WINAPI check_base_files()
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR file_name[ MAX_PATH ]; 

	do 
	{
		//Ӧ��һ��ʼ���DBGHELP���ļ��Ƿ���ȷ���ڣ�����ȷ����ʱ����ʾ��Ӧ����Ϣ��
		ret = add_app_path_to_file_name( file_name, 
			ARRAY_SIZE( file_name), 
#ifdef _WIN64
			L"dbghelp64.dll", 
#else
			L"dbghelp32.dll", 
#endif //_WIN64	

			FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = check_file_exist( file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			show_msg( NULL, 
#ifdef _WIN64
				L"dbghelp64.dll" L"�ļ���ʧ,BITTRACE������������,�������°�װ,�ָ��ļ�."
#else
				L"dbghelp32.dll" L"�ļ���ʧ,BITTRACE������������,�������°�װ,�ָ��ļ�."
#endif //_WIN64	
				); 

			break; 
		}

#if 0 
#ifdef _WIN64
		ret = add_app_path_to_file_name( file_name, 
			ARRAY_SIZE( file_name), 
			L"codereba_cert.cer", 
			FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = check_file_exist( file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			show_msg( NULL, L"%s�ļ���ʧ,BITTRACE������֤ǩ��,�������°�װ,�ָ��ļ�.", 
				L"codereba_cert.cer" ); 

			break; 
		}

		ret = add_app_path_to_file_name( file_name, 
			ARRAY_SIZE( file_name), 
			L"codereba_sign.bat", 
			FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = check_file_exist( file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			show_msg( NULL, L"%s�ļ���ʧ,BITTRACE������֤ǩ��,�������°�װ,�ָ��ļ�.", 
				L"codereba_sign.bat" ); 

			break; 
		}
#endif //_WIN64
#endif //0
	}while( FALSE );

	return ret; 
}

ULONG CALLBACK thread_log_trace( LPVOID param )
{
	LRESULT ret = ERROR_SUCCESS;
	thread_manage *thread_param; 
	BOOLEAN log_trace_prepared = FALSE; 

	ASSERT( NULL != param );

	thread_param = ( thread_manage* )param; 

	do 
	{
		log_trace( ( MSG_INFO, "%s get sys event loop\n", __FUNCTION__ ) ); 

		if( thread_param->stop_run == TRUE )
		{
			break; 
		}

		log_trace( ( MSG_INFO, "%s begin trace logging\n", __FUNCTION__  ) ); 

		ret = prepare_log_trace( &session ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "prepare the work for log trace error 0x%0.8x\n", ret ); 
			break; 
		}

		log_trace_prepared = TRUE; 

		ret = start_log_trace(); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE );

	if( log_trace_prepared == TRUE )
	{
		stop_log_trace( &session ); 

		CloseTrace( session.session_handle );

		if( session.properties != NULL )
		{
			free( session.properties );  
		}
	}

	log_trace( ( MSG_INFO, 
		"%s end trace logging 0x%0.8x \n", 
		__FUNCTION__, 
		ret ) ); 

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ( ULONG )ret; 
}

LRESULT start_log_trace_work( PVOID param, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = create_work_thread( &log_work_threads[ 0 ], thread_log_trace, context, param ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create trace logging thread failed \n" ) ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT stop_log_trace_work()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		if( session.session_handle != 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			//break; 
		}

		_ret = _stop_log_trace( &session ); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
			log_trace( ( MSG_ERROR, "stop log session failed %d \n", ret ) ); 
		}

		_ret = commit_common_log_transaction(); 
		if( _ret != 0 ) //SQLITE_OK
		{

		}

		_ret = stop_work_thread( &log_work_threads[ 0 ] ); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "stop work thread failed\n" ) ); 
			ret = _ret; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI format_sql_string_value( LPWSTR string_value, ULONG cc_string_len, ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	INT32 replaced_count; 
	CStdString string_tool; 

	do 
	{
		string_tool = string_value; 
		replaced_count = string_tool.Replace( L"\'", L"\'\'" ); 

		if( replaced_count > 0 )
		{
			hr = StringCchCopyW( string_value, cc_string_len, string_tool.GetData() ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		*cc_ret_len = string_tool.GetLength(); 

	}while( FALSE );

	return ret; 
}


class CBitSafeFrameWnd : public CWindowWnd, public INotifyUI, public action_logger, public action_ui_notify
{
public:
	CBitSafeFrameWnd() : download_x( 0 ), 
		upload_x( 0 ), 
		download_right_x( 0 ), 
		upload_right_x( 0 ), 
		logo_left( 0 ), 
		timer_id( 0 ), 
		icon_show_time( 0 ), 
		icon_showed_time( 0 ), 
		_trace_frame( NULL ), 
		_search_dlg( NULL )
		//logo_width( 0 )
	{
	}; 

	~CBitSafeFrameWnd()
	{
		if( _search_dlg != NULL )
		{
			if( NULL != _search_dlg->GetHWND() )
			{
				_search_dlg->Close(); 
			}

			delete _search_dlg; 
		}
	}
	
	LPCTSTR GetWindowClassName() const { return _T("UIMainFrame"); };
	UINT GetClassStyle() const { return CS_DBLCLKS; };
	void OnFinalMessage(HWND /*hWnd*/) { delete this; };

	
	INLINE LRESULT output_sys_log_ex( r3_action_notify *action )
	{
		LRESULT ret; 

		ASSERT( action != NULL ); 

		do 
		{
#ifdef _EVENT_CACHE_BY_MEMORY
			ULONG state; 
			ret = add_log_to_store( action, &state ); 
#else
			ret = add_log_to_db_ex( action ); 
#endif //#ifdef _EVENT_CACHE_BY_MEMORY

			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE && "add sys action log failed" ); 
				log_trace( ( MSG_INFO, "add sys action log failed 0x%0.8x\n", ret ) ); 
			}

		}while( FALSE );

		return ret; 
	}

	inline LRESULT output_sys_log( sys_action_output *action )
	{
		LRESULT ret;

		WCHAR log_msg[ MAX_MSG_LEN + MAX_DATA_DUMP_LEN ]; 
		
		ASSERT( action != NULL ); 

		do 
		{
			ret = add_log( &action->action ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE && "add sys action log failed" ); 
				log_trace( ( MSG_INFO, "add sys action log failed 0x%0.8x\n", ret ) ); 
			}

			ret = get_whole_event_msg_ex( action,  
				log_msg, 
				ARRAY_SIZE( log_msg ), 
				LOG_MSG ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			log_trace( ( MSG_INFO, "get message info %ws\n", 
				log_msg ) ); 

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

	LRESULT user_response_sys_event( sys_action_output* action_event, ULONG length, ULONG flags )
	{
		LRESULT ret = ERROR_SUCCESS; 
		WCHAR *action_msg; 
		WCHAR * data_dump = NULL; 
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

		tmp_text = _get_string_by_id( TEXT_SYS_ACTION_TIP_SPECIOUS_ACTION, _T( "�����ɵ�ϵͳ��Ϊ" ) ); 

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
		WCHAR *action_msg; 
		WCHAR * data_dump = NULL; 
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

		tmp_text = _get_string_by_id( TEXT_SYS_ACTION_TIP_SPECIOUS_ACTION, _T( "�����ɵ�ϵͳ��Ϊ" ) ); 

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
		case FOCUS_PROCESS_TIMER_ID:
			ret = on_focus_proc_timer( wParam ); 

			break; 
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
			bHandled = FALSE; 
			break; 
		}
		return ret; 
	}

	LRESULT action_notify( action_ui_notify_type type, PVOID param )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CStdString tip; 
		notify_data *data_notify; 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		data_notify = ( notify_data* )param; 
 

		switch( type )
		{
		case BLOCK_COUNT_NOTIFY:
			{
			}
			break; 
		case ALL_NET_DATA_COUNT:
			{
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
				sys_action = ( sys_action_and_data_output* )data_notify->data; 

				if( data_notify->data_len != sizeof( sys_action_and_data_output ) )
				{
					ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				ret = start_alert_icon(); 

				ret = __user_response_sys_event( sys_action->action, 
					sys_action->data_buf, 
					sys_action->data_size, 
					R3_SYS_EVENT_RESPONSE ); 
			}
			break; 
		case SYS_LOG_NOTIFY:
			{
				sys_log_output *log_out; 

				log_out = ( sys_log_output* )data_notify->data; 

				log_trace( ( MSG_INFO, "output sys log buffer length is %d, recorded message number is %d need length is %d\n", 
					data_notify->data_len, 
					log_out->size, 
					log_out->size * sizeof( sys_log_unit ) + sizeof( ULONG ) ) ); 

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
		UINT text_style; 


		set_ctrl_text( &m_pm, _T( "ctrl_btn" ), TEXT_SECURITY_OPEN ); 		
		set_ctrl_text( &m_pm, _T( "state_desc_txt" ), TEXT_SECURITY_LOGO_CLOSED ); 
		change_to_local(); 

#define DOWNLOAD_UPLOAD_ADD_LEN 50
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
			logo_left = ctrl->GetFixedXY().cx; 

			if( logo_left == 0 )
			{
				ASSERT( FALSE && "why have not init x value." ); 
				logo_left = 30; //43; 
			}
		}
		else
		{
			SIZE ctrl_pos; 

			if(  logo_left == 0 )
			{
				ASSERT( FALSE && "why have not init x value." );
			}
			else
			{
#define BITSAFE_LOGO_ADD_WIDTH 22
				if( get_cur_lang_id() == LANG_ID_CN )
				{
					ctrl_pos = ctrl->GetFixedXY(); 

					ctrl_pos.cx = logo_left - BITSAFE_LOGO_ADD_WIDTH; 

					ctrl->SetFixedXY( ctrl_pos ); 
				}
				else
				{
					ctrl_pos = ctrl->GetFixedXY(); 
					ctrl_pos.cx = logo_left; 

					ctrl->SetFixedXY( ctrl_pos ); 
				}
			}
		}

		option = ( COptionUI* )m_pm.FindControl( _T("trace_btn") ); 
		ASSERT( option != NULL ); 
		if( option->IsSelected() )
		{
		}

		text_style = option->GetTextStyle(); 
		text_style &= ~( DT_CENTER | DT_VCENTER | DT_RIGHT );
		option->SetTextStyle( text_style ); 

		_set_ctrl_text( option, TEXT_MAIN_BUTTON_TOOLS ); 
		if( get_cur_lang_id() == LANG_ID_EN )
		{
			( ( CLabelUI* )option )->SetFont( 5 ); 
		}
		else if( get_cur_lang_id() == LANG_ID_CN )
		{
			( ( CLabelUI* )option )->SetFont( 3 ); 
		}
		option = ( COptionUI* )m_pm.FindControl( _T("trigger_btn") ); 
		ASSERT( option != NULL ); 
		if( option->IsSelected() )
		{
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

		return ret; 
	}

	LRESULT init_toolbar_status()
	{
		LRESULT ret = ERROR_SUCCESS; 
		vector< action_filter_info* >::iterator it; 
		BOOLEAN filter_found = FALSE; 
		ULONG filter_value; 
		COptionUI *option; 

		do 
		{
			filter_value = 0; 

			for( it = filter_infos.filter_infos.begin(); it != filter_infos.filter_infos.end(); it ++ )
			{
				if( FLT_MAIN_TYPE == ( *it )->cond )
				{	
					if( ( *it )->filter_mode != FLT_INCLUDE )
					{
						break; 
					}

					if( filter_found == FALSE )
					{
						filter_found = TRUE; 
					}

					filter_value |= ( *it )->value.data.ulong_value; 
				}
			}

			if( FALSE != filter_found )
			{
				break; 
			}

			filter_value = 	MT_execmon | MT_filemon | MT_regmon | MT_procmon | MT_common | MT_sysmon | MT_w32mon | MT_netmon | MT_behavior | MT_peripheral; 
			for( it = filter_infos.filter_infos.begin(); it != filter_infos.filter_infos.end(); it ++ )
			{

				do 
				{
					if( FLT_MAIN_TYPE == ( *it )->cond )
					{	
						break; 
					}

					if( ( *it )->filter_mode != FLT_EXCLUDE )
					{
						break; 
					}

					filter_value &= ~( ( *it )->value.data.ulong_value ); 

				}while( FALSE ); 
			}
		}while( FALSE ); 

		if( filter_value & MT_filemon )
		{
			option = ( COptionUI* )m_pm.FindControl( L"file_fliter_btn" );
			ASSERT( option != NULL ); 

			option->Selected( true ); 
		}

		if( filter_value & MT_regmon )
		{
			option = ( COptionUI* )m_pm.FindControl( L"reg_fliter_btn" );
			ASSERT( option != NULL ); 

			option->Selected( true ); 
		}

		if( filter_value & MT_netmon )
		{
			option = ( COptionUI* )m_pm.FindControl( L"net_fliter_btn" );
			ASSERT( option != NULL ); 

			option->Selected( true ); 
		}					

		if( filter_value & ( MT_w32mon | MT_execmon | MT_sysmon | MT_behavior | MT_common | MT_behavior ) )
		{
			option = ( COptionUI* )m_pm.FindControl( L"other_filter_btn" );
			ASSERT( option != NULL ); 

			option->Selected( true ); 
		}

		if( filter_value & ( MT_procmon ) )
		{
			option = ( COptionUI* )m_pm.FindControl( L"profile_filter_btn" );
			ASSERT( option != NULL ); 

			option->Selected( true ); 
		}
	
		return ret; 
	}

	LRESULT change_to_local()
	{
#ifdef LANG_EN
		set_ctrl_text_from_name( &m_pm, L"static_logo", L"BITTRACE" ); 

		set_ctrl_tooltip_from_name( &m_pm, L"open_btn", L"Open" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"save_btn", L"Save" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"switch_btn", L"Trace On/Off" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"auto_scroll_btn", L"Auto scroll" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"erase_btn", L"Remove all" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"filter_btn", L"Filters..." ); 
		set_ctrl_tooltip_from_name( &m_pm, L"target_btn", L"Target" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"search_btn", L"Search..." ); 
		set_ctrl_tooltip_from_name( &m_pm, L"jump_to_btn", L"Jump to" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"reg_fliter_btn", L"Registry event" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"file_fliter_btn", L"File event" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"net_fliter_btn", L"Network event" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"profile_filter_btn", L"Profile event" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"other_filter_btn", L"Other event" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"io_filter_btn", L"Peripheral event" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"about_btn", L"About..." ); 
		set_ctrl_tooltip_from_name( &m_pm, L"close_btn", L"Close" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"nor_btn", L"Normalize" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"max_btn", L"Maximize" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"min_btn", L"Minimize" ); 
		
		set_ctrl_text_from_name( &m_pm, L"header_action_name", L"Event type" ); 
		set_ctrl_text_from_name( &m_pm, L"header_time", L"Time" ); 
		set_ctrl_text_from_name( &m_pm, L"header_id", L"ID" ); 
		set_ctrl_text_from_name( &m_pm, L"header_proc_id", L"Process" ); 
		set_ctrl_text_from_name( &m_pm, L"header_thread_id", L"Thread" ); 
		set_ctrl_text_from_name( &m_pm, L"header_exe_file_path", L"Subject" ); 

		set_ctrl_text_from_name( &m_pm, L"header_object", L"Object" ); 
		set_ctrl_text_from_name( &m_pm, L"header_desc", L"Detail" ); 
		set_ctrl_text_from_name( &m_pm, L"header_data", L"Data" ); 
		set_ctrl_text_from_name( &m_pm, L"header_result", L"Result" ); 
		set_ctrl_text_from_name( &m_pm, L"header_call_stack", L"Call stack" ); 

		set_ctrl_text_from_name( &m_pm, L"pref_btn", L"Option" ); 
		set_ctrl_text_from_name( &m_pm, L"about_info_btn", L"About..." ); 
		set_ctrl_text_from_name( &m_pm, L"diag_btn", L"Diagnostic" ); 
		set_ctrl_text_from_name( &m_pm, L"forum_btn", L"Forum" ); 
		set_ctrl_text_from_name( &m_pm, L"help_btn", L"Help" ); 
		set_ctrl_text_from_name( &m_pm, L"update_btn", L"Update" ); 
		set_ctrl_text_from_name( &m_pm, L"conf_btn", L"Configure" ); 
		set_ctrl_text_from_name( &m_pm, L"option_btn", L"Option" ); 
		set_ctrl_text_from_name( &m_pm, L"uninstall_btn", L"Uninstall" ); 
		set_ctrl_text_from_name( &m_pm, L"donate_btn", L"Donate" ); 
		set_ctrl_text_from_name( &m_pm, L"suggest_btn", L"Suggestion" ); 

#endif //LANG_EN

		return ERROR_SUCCESS; 
	}

	void _Init() 
	{
		LRESULT ret = ERROR_SUCCESS; 
		COptionUI *option;  

		change_to_local(); 
#ifdef PAINT_DEBUG
		m_pm.SetShowUpdateRect( TRUE ); 
#endif //PAINT_DEBUG

		m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
		m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
		ASSERT( m_pMaxBtn != NULL ); 
		m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
		ASSERT( m_pRestoreBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		ret = init_popup_wnd_context(); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "init popup window context error!\n" ); 
			log_trace( ( MSG_INFO, "init popup window context error!\n" ) ); 
		}

		ret = load_common_conf( update_ui, &m_pm ); 
		ret = load_common_conf_callback(); 

#ifdef _DEBUG
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load ui configuration error 0x%0.8x\n", ret ) ); 
		}
#endif //_DEBUG

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

		ret = init_ui_context_thread( ( PVOID )this ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "!!!initialize ui work context failed 0x%0.8x, will exit", ret ) );  
		}

		CTabLayoutUI* tabs = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("frame")));
		ASSERT( tabs != NULL ); 

		_trace_frame = static_cast<trace_frame*>(tabs->GetItemAt( 0 ) ); 
		ASSERT( _trace_frame != NULL ); 
		_trace_frame->_Init(); 

		_search_dlg = new search_dlg( _trace_frame ); 

		ret = init_event_result_receiver(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "start event trace error\n" ) ); 
		}

		option = static_cast<COptionUI*>(m_pm.FindControl( _T( "auto_scroll_btn" ) ) ); 
		ASSERT( option != NULL ); 

		option->Activate(); 

		tabs->SelectItem( 0 ); 

//#define TEST_RICH_EDIT
#ifdef TEST_RICH_EDIT

		CreateThread( NULL, 0, thread_test_rich_edit, ( PVOID )&sys_log, 0, NULL ); 

#endif //TEST_RICH_EDIT

#ifdef SHOW_SLOGAN
		ret = start_slogan_display( ( action_ui_notify* )this ); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
		}
#endif //SHOW_SLOGAN

		ret = init_toolbar_status(); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
		}
	}

	LRESULT on_more_info_tab_selected()
	{
		LRESULT ret = ERROR_SUCCESS; 
		return ret; 
	}

	LRESULT on_net_manage_tab_selected()
	{
		LRESULT ret = ERROR_SUCCESS; 
		return ret; 
	}

	LRESULT have_manual_filter_condition()
	{
		LRESULT ret = ERROR_SUCCESS; 
		ULONG _ret; 

		do 
		{
			if( filter_infos.filter_infos.size() == 1 )
			{
				WCHAR exe_file_name[ MAX_PATH ]; 

				_ret = GetModuleFileName( NULL, exe_file_name, MAX_PATH ); 

				if( _ret <= 0 
					|| _ret >= MAX_PATH )
				{
					break; 
				}

				if( filter_infos.filter_infos[ 0 ]->filter_mode == FLT_EXCLUDE 
					&& filter_infos.filter_infos[ 0 ]->value.type == STRING_VALUE_TYPE 
					&& 0 == wcsicmp( filter_infos.filter_infos[ 0 ]->value.text.string_value, exe_file_name ) ) 
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}
				else
				{
					break; 
				}
			}
			else if( filter_infos.filter_infos.size() < 1 ) 
			{
				ret = ERROR_INVALID_PARAMETER; 
			}

		}while( FALSE );
	
		return ret; 
	}

	LRESULT on_sys_manage_tab_selected() 
	{
		LRESULT ret = ERROR_SUCCESS; 
		CButtonUI *net_cfg_btn; 

		if( is_vista_or_later() == TRUE )
		{
			net_cfg_btn = static_cast< CButtonUI* >( m_pm.FindControl( _T( "net_cfg_btn" ) ) ); 

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
		return ret; 
	}

#ifdef DIRECT_SHOW_UI
	void OnPrepareAnimation()
	{
		COLORREF clrBack = RGB( 10, 10, 10 );
		RECT rcCtrl; 
		//rcCtrl = m_pm.FindControl(_T("summary_btn"))->GetPos();
		//m_pm.AddAnimJob(CAnimJobUI(UIANIMTYPE_FLAT, 0, 350, clrBack, clrBack, CRect(rcCtrl.left, rcCtrl.top, rcCtrl.left + 50, rcCtrl.top + 50), 40, 0, 4, 255, 0.3f));
		//rcCtrl = m_pm.FindControl(_T("net_manage_btn"))->GetPos();
		//m_pm.AddAnimJob(CAnimJobUI(UIANIMTYPE_FLAT, 200, 350, clrBack, clrBack, CRect(rcCtrl.left, rcCtrl.top, rcCtrl.left + 50, rcCtrl.top + 50), 40, 0, 4, 255, 0.3f));
		//rcCtrl = m_pm.FindControl(_T("sys_manage_btn"))->GetPos();
		//m_pm.AddAnimJob(CAnimJobUI(UIANIMTYPE_FLAT, 100, 350, clrBack, clrBack, CRect(rcCtrl.left, rcCtrl.top, rcCtrl.left + 50, rcCtrl.top + 50), 40, 0, 4, 255, 0.3f));
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

		if( 0 == have_manual_filter_condition() )
		{
			show_filter_dlg( this, SHOW_WINDOW_CENTER );  
		}
	}

	LRESULT on_close()
	{
		LRESULT ret = ERROR_SUCCESS; 
		Close(); 

		return ret; 
	}

	LRESULT WINAPI print_main_filter_type( action_filter_info *filter )
	{
		LRESULT ret = ERROR_SUCCESS; 
		ULONG main_type; 

		do 
		{
			ASSERT( filter != NULL ); 
			ASSERT( filter->cond == FLT_MAIN_TYPE ); 

			main_type = filter->value.data.ulong_value; 
			
			dbg_print( MSG_IMPORTANT, "the main type for filtering is 0x%0.8x :\n", main_type ); 

			if( ( main_type & MT_execmon ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_execmon\n" ); 
			}
			
			if( ( main_type & MT_filemon ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_filemon\n" ); 
			}
			
			if( ( main_type & MT_regmon ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_regmon\n" ); 
			}

			if( ( main_type & MT_procmon ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_procmon\n" ); 
			}

			if( ( main_type & MT_common ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_common\n" ); 
			}

			if( ( main_type & MT_sysmon ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_sysmon\n" ); 
			}

			if( ( main_type & MT_w32mon ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_w32mon\n" ); 
			}

			if( ( main_type & MT_netmon ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_netmon\n" ); 
			}

			if( ( main_type & MT_behavior ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_behavior\n" ); 
			}

			if( ( main_type & MT_unknown ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_unknown\n" ); 
			}

			if( ( main_type & MT_peripheral ) != 0 )
			{
				dbg_print( MSG_IMPORTANT, "MT_peripheral\n" ); 
			}

		}while( FALSE );
	
		return ret; 
	}

	LRESULT WINAPI filter_main_type( ULONG type )
	{
		LRESULT ret = ERROR_SUCCESS; 
		HRESULT hr; 
		action_filter_info filter_info; 
		BOOLEAN is_found = FALSE; 

		do 
		{
			if( type & MT_unknown )
			{
				ASSERT( FALSE ); 
				break; 
			}

			//can set the region more preciously
			w_lock_filter_infos(); 

			do
			{
				ULONG _main_type; 
				action_filter_info old_filter_info; 
				std::vector<action_filter_info*>::iterator it; 

				for( it = filter_infos.filter_infos.begin(); it != filter_infos.filter_infos.end(); it ++ )
				{
					if( (*it)->cond == FLT_MAIN_TYPE 
						&& ( (*it)->filter_mode == FLT_INCLUDE ) )
						//|| (*it)->filter_mode == FLT_CONTAINS 
					{
						is_found = TRUE; 

						_main_type = (*it)->value.data.ulong_value; 

						if( _main_type == type )
						{
							break; 
						}

						memcpy( &old_filter_info, (*it), sizeof( old_filter_info ) ); 

						if( (*it)->compare_mode != FLT_CONTAINS )
						{
							(*it)->compare_mode = FLT_CONTAINS; 
						}

						_main_type = type; 

						hr = StringCbPrintfW( (*it)->value.text.text_mode_value, 
							sizeof( (*it)->value.text.text_mode_value ), 
							L"%u", 
							_main_type ); 

						if( FAILED( hr ) )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						(*it)->value.text.cc_string_len = ( ULONG )wcslen( (*it)->value.text.text_mode_value ); 

						(*it)->value.type = STRING_VALUE_TYPE; 
						(*it)->value.text.text_is_ptr = FALSE; 

						ret = adjust_filter_cond_value( (*it) ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						update_action_cond_to_db( (*it), &old_filter_info ); 

#ifdef _DEBUG
						print_main_filter_type( ( *it ) ); 
#endif //_DEBUG	
						break; 
					}
				}
			}while( FALSE ); 

			w_unlock_filter_infos(); 

			do 
			{
				if( TRUE == is_found )
				{
					break; 
				}

				init_filter_ui_info( &filter_info.ui_info ); 

				filter_info.cond = FLT_MAIN_TYPE; 

				filter_info.filter_mode = FLT_INCLUDE; 

				filter_info.compare_mode = FLT_CONTAINS; 

				hr = StringCbPrintfW( filter_info.value.text.text_mode_value, 
					sizeof( filter_info.value.text.text_mode_value ), 
					L"%u", 
					type ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				filter_info.value.text.cc_string_len = ( ULONG )wcslen( filter_info.value.text.text_mode_value ); 

				filter_info.value.type = STRING_VALUE_TYPE; 
				filter_info.value.text.text_is_ptr = FALSE; 

				ret = adjust_filter_cond_value( &filter_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = check_action_filter_valid( &filter_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = input_action_cond_to_db( &filter_info ); 
				if( ret != ERROR_SUCCESS )
				{
					//break; 
				}

				{
					action_filter_info *_filter_info; 

					_filter_info = ( action_filter_info* )malloc( sizeof( action_filter_info ) ); 
					if( _filter_info == NULL )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					memcpy( _filter_info, &filter_info, sizeof( *_filter_info ) ); 

					w_lock_filter_infos(); 
					filter_infos.filter_infos.push_back( _filter_info ); 
					w_unlock_filter_infos(); 
				}

			}while( FALSE );

			_trace_frame->FilterEventsUI(); 

		}while(FALSE );
		return ret; 
	}

	LRESULT show_statistic()
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
		}while( FALSE ); 
	
		return ret; 
	}

	/*******************************************************************
	������������ȷ��ʹ�ò��㣺û���Զ�FOCUS����ENTER����Ч
	*******************************************************************/
	LRESULT search_events()
	{
		LRESULT ret = ERROR_SUCCESS; 
		INT32 _ret; 

		do
		{
			POINT cursor_pos; 
			BOOLEAN pos_got = FALSE; 

			_ret = GetCursorPos( &cursor_pos ); 
			if( TRUE == _ret )
			{
				pos_got = TRUE; 
			}

			if( NULL == _search_dlg->GetHWND() )
			{
				_search_dlg->Create( GetHWND(), _T( "search" ), ( UI_WNDSTYLE_FRAME/*UI_WNDSTYLE_DIALOG *//*& ( ~WS_DLGFRAME )*/ ), WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES, 0, 0, 500, 100);
				_search_dlg->SetIcon( IDI_MAIN_ICON ); 
				ASSERT( NULL != _search_dlg->GetHWND() ); 
			}
			else
			{

			}

			if( TRUE == pos_got )
			{
				SetWindowPos( _search_dlg->GetHWND(), NULL, cursor_pos.x, cursor_pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER ); 
			}
			else
			{
				_search_dlg->CenterWindow(); 
			}

			_search_dlg->ShowWindow(); 
		}while( FALSE ); 

		return ret; 
	}

	LRESULT select_trace_tab()
	{
		COptionUI *about_btn; 
		CTabLayoutUI* frame = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("frame")));

		about_btn = static_cast< COptionUI* >( m_pm.FindControl( _T( "about_btn" ) ) ); 
	 	ASSERT( frame != NULL ); 
		ASSERT( about_btn != NULL ); 
		
		about_btn->Selected( false ); 

		frame->SelectItem( 0 ); 
		on_more_info_tab_selected(); 
	
		return ERROR_SUCCESS; 
	}

	void Notify(TNotifyUI& msg)
	{
		LRESULT ret = ERROR_SUCCESS; 
		LPCTSTR tmp_text; 
		CStdString text; 
		CStdString name; 

		if( msg.sType == _T("windowinit") ) 
		{
			OnPrepare();
		}
		else if( msg.sType == _T("click") ) 
		{
			if( msg.pSender == m_pCloseBtn ) 
			{
				ShowWindow( false, false ); 
				PostQuitMessage(0);
				return; 
			}
			else if( msg.pSender == m_pMinBtn ) 
			{ 
				short_window_effective(); 
				swap_max_restore( FALSE ); 

				return; 
			}
			else if( msg.pSender == m_pMaxBtn ) 
			{ 
				SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
			}
			else if( msg.pSender == m_pRestoreBtn ) 
			{ 
				SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
			}
			else
			{
				name = msg.pSender->GetName(); 

				if( name == _T( "filter_btn" ) )
				{
					select_trace_tab(); 

					show_filter_dlg( this, 0 ); 
				}
				else if( name == _T( "all_proc_btn") )
				{
				}
				else if( name == _T( "about_info_btn" ) )
				{
					if( !::IsWindow( about_box.GetHWND() ) )
					{
						about_box.Create(NULL, _T("����..."), UI_WNDSTYLE_FRAME, 0L, 0, 0, 490,280);
						about_box.CenterWindow();
						about_box.SetIcon( IDI_MAIN_ICON );
					}
				}

				else if( name == _T( "anti_arp_btn" ) )
				{
				}
				else if( name == _T( "all_conn_btn" ) )
				{
				}
			
#define SEVEN_FW_FORUM_URL _T( "http://www.simpai.net/bbs/" )
				else if( name == _T( "forum_btn" ) )
				{
					::ShellExecute( NULL, _T( "open" ), SEVEN_FW_FORUM_URL, NULL, NULL, SW_SHOWNORMAL ); 
				}
				else if( name == _T( "help_btn" ) )
				{
#define SEVEN_FW_HELP_URL _T( "http://www.simpai.net/" )
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
				}
				else if( name == _T( "uninstall_btn" ) )
				{

					ret = unistall_bitsafe(); 

					{
						dlg_ret_state ret_state; 

						tmp_text = UNINSTALL_NEED_RESTART_TEXT; 

						ret = show_msg( GetHWND(), tmp_text, &ret_state ); 
						if( ret != ERROR_SUCCESS )
						{
							ASSERT( FALSE ); 
						}

					}
				}
				else if( name == _T( "suggest_btn" ) )
				{
					suggest_dlg dlg; 
					LPCTSTR tmp_text; 

					tmp_text = GIVE_SUGESTION_TITLE_TEXT; //_get_string_by_id( TEXT_SUGGESTION_TITLE, _T(  ) ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
				else if( name == _T( "donate_btn" ) )
				{

					/**********************************************************************************
					�û�ϣ����ʲô��ʽ��֧������:
					1.donate money
					2.change they home page of ie.( is not good for wealth people.
					and users of BITTRACE are little wealth. 
					goto donation page directly.
					**********************************************************************************/

#define SEVEN_FW_DONATE_URL _T( "http://www.simpai.net/support_en.php" )
					::ShellExecute( NULL, _T( "open" ), SEVEN_FW_DONATE_URL, NULL, NULL, SW_SHOWNORMAL ); 

				}
				else if( name == _T( "option_btn" ) ) //_T( "sel_lang_btn" ) )
				{
					option_dlg dlg( this ); 
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_OPTION_DIALOG_TITLE, OPTION_DLG_TITLE_TEXT ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 

#if 0
					chg_lang_dlg dlg( this ); 
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_LANG_TITLE, _T( "ѡ������" ) ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
#endif //0
					save_lang_conf(); 
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

					tmp_text = _get_string_by_id( TEXT_GET_DEFENSE_POWER_FROM_OTHER, _T( "�ӹ�������������" ) ); 

					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
#endif //COMPETITE_WITH_360	
				else if( name == _T( "relearn_btn" ) )
				{
					LPCTSTR tmp_text; 

					ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_RELEARN, NULL, 0, NULL, 0, NULL, NULL ); 
					if( ret != ERROR_SUCCESS )
					{
						ASSERT( FALSE ); 
						goto _return; 
					}

					tmp_text = _get_string_by_id( TEXT_RELEARN_TIP_MSG, L"��������ѧϰ����ɹ�" ); 
					
					show_msg( GetHWND(), tmp_text ); 
				}
				
				else if( name == _T( "ui_btn" ) )
				{ 
					ui_custom_dlg
					dlg( &m_pm );  
					LPCTSTR tmp_text; 

					tmp_text = UI_CUSTOM_DLG_TITLE_TEXT; //_get_string_by_id( TEXT_UI_CUSTOM_TITLE, L"UI��������" ); 
					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
				else if( name == _T( "trans_btn" ) )
				{ 
					ui_custom_dlg
					dlg( &m_pm );  
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_UI_CUSTOM_TITLE, _T( "UI��������" ) ); 
					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
				else if( name == _T( "switch_btn" ) )
				{
					select_trace_tab(); 

					//BUT:ֹͣʱ�䳤�������޷�����
					if( ( ( COptionUI* )msg.pSender )->IsSelected() )
					{
						process_log_sw = TRUE; 
					}
					else
					{
						process_log_sw = FALSE; 
					}
				}
				else if( name == _T( "auto_scroll_btn" ) )
				{
					select_trace_tab(); 

					if( ( ( COptionUI* )msg.pSender )->IsSelected() )
					{
						_trace_frame->set_auto_scroll( FALSE ); 
					}
					else
					{
						_trace_frame->set_auto_scroll( TRUE ); 
					}
				}
				else if( name == _T( "erase_btn" ) )
				{
					select_trace_tab(); 

					_trace_frame->clear_events(); 
				}
				else if( name == _T( "font_btn" ) )
				{
					select_trace_tab(); 

					_trace_frame->change_font(); 
				}
				else if( name == _T( "statistic_btn" ) )
				{
					select_trace_tab(); 

					show_statistic(); 
				}
				else if( name == _T( "search_btn" ) )
				{
					select_trace_tab(); 

					search_events(); 
				}
				else if( name == _T( "jump_to_btn" ) )
				{
					CStdString obj_path; 

					do 
					{
						select_trace_tab(); 

						obj_path = _trace_frame->get_cur_sel_obj_path(); 

						if( NULL == obj_path )
						{
							break; 
						}

						ret = jump_to_object( obj_path.GetData(), MT_unknown ); 
					}while( FALSE );  
				}
				else if( name == _T( "about_btn" ) ) 
				{
					COptionUI *option; 
					CTabLayoutUI* tabs = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("frame")));
					ASSERT( tabs != NULL ); 

					option = ( COptionUI* )msg.pSender;
					ASSERT( option != NULL ); 

					if( option->IsSelected() )
					{
						tabs->SelectItem( 0 ); 
					}
					else
					{
						tabs->SelectItem( 1 ); 
					}
					on_more_info_tab_selected(); 
				}
				else if( name == _T( "reg_fliter_btn" ) 
					|| name == _T( "file_fliter_btn" ) 
					|| name == _T( "net_fliter_btn" ) 
					|| name == _T( "other_filter_btn" ) 
					|| name == _T( "profile_filter_btn" ) 
					|| name == _T( "io_filter_btn" ) )
				{
					LRESULT ret = ERROR_SUCCESS; 
					ULONG filter_value = 0; 
					COptionUI *option; 

					do 
					{
						select_trace_tab(); 

						option = ( COptionUI* )m_pm.FindControl( L"reg_fliter_btn" );
						ASSERT( option != NULL ); 

						if( ( option->IsSelected() 
							&& option != msg.pSender ) 
							|| ( false == option->IsSelected() 
							&& option == msg.pSender )  )
						{
							filter_value |= MT_regmon; 
						}

						option = ( COptionUI* )m_pm.FindControl( L"file_fliter_btn" );
						ASSERT( option != NULL ); 

						if( ( option->IsSelected() 
							&& option != msg.pSender ) 
							|| ( false == option->IsSelected() 
							&& option == msg.pSender )  )
						{
							filter_value |= MT_filemon; 
						}

						option = ( COptionUI* )m_pm.FindControl( L"net_fliter_btn" );
						ASSERT( option != NULL ); 

						if( ( option->IsSelected() 
							&& option != msg.pSender ) 
							|| ( false == option->IsSelected() 
							&& option == msg.pSender )  )
						{
							filter_value |= MT_netmon;  
						}

						option = ( COptionUI* )m_pm.FindControl( L"other_filter_btn" );
						ASSERT( option != NULL ); 

						if( ( option->IsSelected() 
							&& option != msg.pSender ) 
							|| ( false == option->IsSelected() 
							&& option == msg.pSender ) )
						{
							filter_value |= ( MT_w32mon | MT_execmon | MT_sysmon | MT_behavior | MT_common | MT_behavior ); 
						}

						option = ( COptionUI* )m_pm.FindControl( L"profile_filter_btn" );
						ASSERT( option != NULL ); 

						if( ( option->IsSelected() 
							&& option != msg.pSender ) 
							|| ( false == option->IsSelected() 
							&& option == msg.pSender ) )
						{
							filter_value |= ( MT_procmon ); 
						}

						option = ( COptionUI* )m_pm.FindControl( L"io_filter_btn" );
						ASSERT( option != NULL ); 

						if( ( option->IsSelected() 
							&& option != msg.pSender ) 
							|| ( false == option->IsSelected() 
							&& option == msg.pSender ) )
						{
							filter_value |= ( MT_peripheral ); 
						}
						else
						{
							filter_value |= ( MT_peripheral ); 
						}

					}while( FALSE ); 
				
					ret = filter_main_type( filter_value ); 
					if( ret != ERROR_SUCCESS )
					{
					}
				}
				else if( name == _T( "save_btn" ) )
				{
					/***************************************************************************
					���ڸ������ݵ����ַ�ʽ��
					���°�ť���б���

					����ʱ��̬����(Ӱ����������)
					***************************************************************************/
					LRESULT _ret; 
					save_log_dlg dlg; 
					POINT cursor_pos; 
					BOOLEAN pos_got = FALSE; 

					{
						_ret = GetCursorPos( &cursor_pos ); 
						if( TRUE == _ret )
						{
							pos_got = TRUE; 
						}
					}

					dlg.Create( GetHWND(), _T( "backup" ), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					dlg.SetIcon( IDI_MAIN_ICON ); 

					if( TRUE == pos_got )
					{
						SetWindowPos( dlg.GetHWND(), NULL, cursor_pos.x, cursor_pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER ); 
					}
					else
					{
						dlg.CenterWindow(); 
					}

					dlg.ShowModal(); 
				}
				else if( name == _T( "open_btn" ) )
				{
					LRESULT _ret; 
					load_log_dlg dlg; 
					POINT cursor_pos; 
					COptionUI *option; 
					BOOLEAN pos_got = FALSE; 

					{
						_ret = GetCursorPos( &cursor_pos ); 
						if( TRUE == _ret )
						{
							pos_got = TRUE; 
						}
					}

					option = ( COptionUI* )m_pm.FindControl( _T("switch_btn") ); 
					ASSERT( option != NULL ); 
					if( false == option->IsSelected() )
					{
						option->Selected( true ); 
					}

					process_log_sw = FALSE; 
					_trace_frame->clear_events(); 

					dlg.Create( GetHWND(), _T( "loading" ), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
					dlg.SetIcon( IDI_MAIN_ICON ); 

					if( TRUE == pos_got )
					{
						SetWindowPos( dlg.GetHWND(), NULL, cursor_pos.x, cursor_pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER ); 
					}
					else
					{
						dlg.CenterWindow(); 
					}

					dlg.ShowModal(); 

				}
				else if( name == _T( "trace_config_btn" ) )
				{
				
				}
			}
		}
		else if( msg.sType == _T( "menu_exit" ))
		{
			on_close(); 
		}
		else if( msg.sType == _T( "menu_open" ) )
		{
			ShowWindow(); 
		}
		else if( msg.sType == _T( "menu_properties" ) )
		{
		}
		else if( msg.sType == _T( "menu_stack" ) )
		{
		}
		else if( msg.sType == _T( "menu_jump_to" ) )
		{
			CControlUI *list_item; 
			CControlUI *event_obj; 
			CStdString name; 
			name = msg.pSender->GetName(); 

			if( name == L"exe_file" )
			{
				text = msg.pSender->GetUserData(); 

				if( text.GetLength() == 0 ) 
				{
					text = msg.pSender->GetText(); 
				}
			}
			else if( name == L"event_obj" )
			{
				text = msg.pSender->GetUserData(); 

				if( text.GetLength() == 0 ) 
				{
					text = msg.pSender->GetText(); 
				}
			}
			else
			{
				list_item = ( CControlUI* )( PVOID )msg.pSender->GetTag(); 
				ASSERT( 0 == wcscmp( list_item->GetName(), UI_EVENT_LIST_ELEMENT_CLASS_NAME ) ); 
				event_obj = m_pm.FindSubControlByName( list_item, L"event_obj" ); 

				ASSERT( event_obj != NULL ); 

				text = event_obj->GetUserData(); 

				if( text.GetLength() == 0 ) 
				{
					text = event_obj->GetText(); 
				}
			}

			ret = jump_to_object( text.GetData(), MT_unknown ); 
		}
		else if( msg.sType == _T( "slider_data_size" ) )
		{
			BOOLEAN lock_held = FALSE; 
			r3_action_notify_buf *action; 
			CControlUI *list_item; 
			CSliderUI *slider; 
			ULONG proc_id; 
			ULONG data_size; 

			do 
			{
				list_item = ( CControlUI* )( PVOID )msg.pSender->GetTag(); 
				ASSERT( 0 == wcscmp( list_item->GetName(), UI_EVENT_LIST_ELEMENT_CLASS_NAME ) ); 

				w_lock_filtering_tip_lock(); 
				lock_held = TRUE; 

				action = ( r3_action_notify_buf* )( PVOID )list_item->GetTag(); 
				if( NULL == action )
				{
					break; 
				}

				proc_id = action->action.action.ctx.proc_id; 

				w_unlock_filtering_tip_lock(); 
				lock_held = FALSE; 

				slider = ( CSliderUI* )msg.pSender; 
				ASSERT( slider != NULL ); 

				data_size = ( ULONG )msg.lParam; 

				ret = config_proc_trace_data_size( proc_id, data_size ); 
			} while ( FALSE );

			if( lock_held != FALSE )
			{
				w_unlock_filtering_tip_lock(); 
			}
		}
		else if( msg.sType == _T( "menu_search" ) )
		{
			CStdString search_url; 
			CStdString item_text; 
#define SEACH_ENGIN_URL_FORMAT_STRING _T( "http://cn.bing.com/search?q=%s" ) 

			item_text = msg.pSender->GetText(); 
			
			search_url.Format( SEACH_ENGIN_URL_FORMAT_STRING, item_text.GetData() ); 

			::ShellExecute( NULL, _T( "open" ), search_url.GetData(), NULL, NULL, SW_SHOWNORMAL ); 
		}
		else if( msg.sType == _T( "menu_include_it" ) )
		{
			action_filter_info flt_info; 

			do 
			{
				ret = get_filter_info_from_event_ui( msg.pSender, msg.sType.GetData(), &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = check_action_filter_valid( &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				flt_info.filter_mode = FLT_INCLUDE;  

				ret = input_filter_condition( &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					dbg_print( MSG_FATAL_ERROR, "input the filter condition error %u\n", ret ); 
				}

				_trace_frame->FilterEventsUI(); 

			}while( FALSE );
		}
		else if( msg.sType == _T( "menu_exclude_it" ) )
		{
			action_filter_info flt_info; 
			do 
			{
				ret = get_filter_info_from_event_ui( msg.pSender, msg.sType.GetData(), &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = check_action_filter_valid( &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				flt_info.filter_mode = FLT_INCLUDE; 

				ret = delete_filter_condition( &flt_info ); 
				if( ret == ERROR_SUCCESS )
				{

				}

				flt_info.filter_mode = FLT_EXCLUDE; 

				ret = input_filter_condition( &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					dbg_print( MSG_FATAL_ERROR, "input the filter condition error %u\n", ret ); 
				}

				_trace_frame->FilterEventsUI(); 

			}while( FALSE ); 
		}
		else if( msg.sType == _T( "menu_begin_time" ) )
		{
			action_filter_info flt_info; 

			do 
			{
				ret = get_filter_info_from_event_ui( msg.pSender, msg.sType.GetData(), &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = check_action_filter_valid( &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				flt_info.filter_mode = FLT_EXCLUDE; 

				ret = delete_filter_condition( &flt_info ); 
				if( ret == ERROR_SUCCESS )
				{

				}

				flt_info.filter_mode = FLT_INCLUDE; 

				ret = input_filter_condition( &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					dbg_print( MSG_FATAL_ERROR, "input the filter condition error %u\n", ret ); 
				}

				_trace_frame->FilterEventsUI(); 

			}while( FALSE ); 
		}
		else if( msg.sType == _T( "menu_end_time" ) )
		{
			action_filter_info flt_info; 

			do 
			{
				ret = get_filter_info_from_event_ui( msg.pSender, msg.sType.GetData(), &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = check_action_filter_valid( &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				flt_info.filter_mode = FLT_EXCLUDE; 

				ret = delete_filter_condition( &flt_info ); 
				if( ret == ERROR_SUCCESS )
				{

				}

				flt_info.filter_mode = FLT_INCLUDE; 

				ret = input_filter_condition( &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					dbg_print( MSG_FATAL_ERROR, "input the filter condition error %u\n", ret ); 
				}

				_trace_frame->FilterEventsUI(); 

			}while( FALSE ); 
		}
		else if( msg.sType == _T( "menu_hilight_it" ) )
		{
			action_filter_info flt_info;  

			do 
			{
				ret = get_filter_info_from_event_ui( msg.pSender, msg.sType.GetData(), &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = check_action_filter_valid( &flt_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = _trace_frame->hilight_by_key_word(); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}
			}while( FALSE ); 
		}
		else if( msg.sType == _T( "menu_copy" ) )
		{
			CStdString text; 
			text = msg.pSender->GetText(); 

			ret = copy_text_to_clipboard( GetHWND(), text.GetData(), text.GetLength() ); 
			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_FATAL_ERROR, "copy the text to clipboard error\n" ); 
			}
		}
		else if( msg.sType == _T( "menu_include" ) )
		{
		}
		else if( msg.sType == _T( "menu_exclude" ) )
		{
		}
		else if( msg.sType == _T( "menu_hilight" ) )
		{
		}
		
		else if( msg.sType == _T( "menu_trace_data_size" ) )
		{
		}
		else if( msg.sType == _T("menu") )
		{
			CStdString ctrl_name; 

			ctrl_name = msg.pSender->GetName(); 

			if( 0 == ctrl_name.Compare( L"io_filter_btn") )
			{
				periperal_list_dlg dlg; 

				dlg.Create( GetHWND(), _T("�����б�"), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
				dlg.SetIcon( IDI_MAIN_ICON ); 
				dlg.CenterWindow();
				dlg.ShowModal(); 
			}
		}
		else if(msg.sType==_T("selectchanged"))
		{
			__Trace( _T( "the set focus target control is %s \n" ), name ); 
		}

_return:
		return; 
	}

	void FullScreen( bool bFull )
	{

	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 
		HMENU sys_menu; 
		LPCTSTR tmp_text; 

		sys_menu = GetSystemMenu( GetHWND(), FALSE ); 
		ASSERT( sys_menu != NULL ); 

		set_app_flag( GetHWND(), FALSE ); 

		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);

		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		CDialogBuilder builder;
		tabs_builder cb; 

#define BITTRACE_UI_LAYOUT_FILE_NAME _T("bittrace.xml")
		CControlUI* pRoot = builder.Create(BITTRACE_UI_LAYOUT_FILE_NAME, (UINT)0,  &cb, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);
		set_ui_style( &m_pm ); 

		tmp_text = MAIN_BITSAFE_STARTED_BALLOON_TIP_TEXT; 

		ret = icon_tray.balloon_icon( GetHWND(), 
			WM_TRAY_ICON_NOTIFY, 
			tmp_text, 
			_T( "" ), 
			_T( "" ), 
			GetModuleHandle( NULL ), 
			IDI_MAIN_ICON, 
			IDI_ALERT_ICON,  
			TRAY_ICON_ID, 
			NULL ); 
		
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			Close(); 
		}

		_Init();
		return 0;
	}

	LRESULT short_window_effective()
	{
		LRESULT ret = ERROR_SUCCESS; 

#define EFFECTIVE_TRANPARENT 130 
		m_pm.SetCloseStretchPaint( true ); 

		m_pm.SetTransparent( EFFECTIVE_TRANPARENT ); 
		ret = short_window_close( &m_pm, GetHWND(), 0, 0 ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "shorting window close effective error 0x%0.8x\n", ret ) ); 
		}

		m_pm.SetCloseStretchPaint( false ); 
		m_pm.SetTransparent( get_tranparent() ); 
		CenterWindow(); 
		
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
		bHandled = FALSE; 
		return uninit_bittrace_main_wnd(); 
	}

	LRESULT uninit_bittrace_main_wnd()
	{
		LRESULT ret = ERROR_SUCCESS; 

		ret = uninit_ui_context(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "uninitialize the ui context failed \n" ) ); 
		}

		ret = stop_slogan_display(); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			log_trace( ( MSG_ERROR, "stop slogan show error 0x%0.8x\n", ret ) ); 
		}

		ret = uninit_event_result_receiver(); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			log_trace( ( MSG_ERROR, "stop log tracing error 0x%0.8x\n", ret ) ); 
		}

		ret = stop_action_response_dlg(); 
		if( ret != ERROR_SUCCESS )
		{

		}

		ret = icon_tray.DeleteIcon(); 
		if( ret !=	ERROR_SUCCESS )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!delete the tray icon failed \n" ) ); 
		}

		ret = close_bitsafe_conf(); 
		if( ret < 0 )
		{
			log_trace( ( MSG_ERROR, "close policy configuration db failed" ) ); 
		}

		ret = close_function_conf(); 
		if( ret < 0 )
		{
			log_trace( ( MSG_ERROR, "close basic fucntion config db failed" ) ); 
		}

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

		close_info_dlgs(); 

		uninit_bitsafe_function_state(); 

		if( TRUE == IsWindowVisible( GetHWND() ) )
		{
			short_window_effective(); 
		}

		release_filter_conds(); 

		ret = uninit_filter_infos(); 

		ret = uninit_event_store(); 

		ret = uninit_peripheral_devices_info(); 
		
		PostQuitMessage( 0 ); 

		return 0;
	}


	LRESULT on_receive_event_log_done( ULONG msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		LRESULT ret = ERROR_SUCCESS; 

		bHandled = TRUE; 

		do 
		{
			ASSERT( wParam != NULL );

			_trace_frame->send_action_log_done( ( PVOID )wParam ); 
		}while( FALSE ); 

		return ret; 
	}

	LRESULT on_receive_event_log( ULONG msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		LRESULT ret = ERROR_SUCCESS; 
		event_log_notify *event_log; 

		bHandled = TRUE; 

		do 
		{
			ASSERT( lParam != NULL ); 
			ASSERT( wParam != NULL ); 

			event_log = ( event_log_notify* )lParam;

			_trace_frame->send_action_log( event_log, ( PVOID )wParam ); 

		}while( FALSE );
	
		return ret; 
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
			CControlUI *main_layout = m_pm.FindControl( _T( "form_layout" ) ); 

			ASSERT( main_layout != NULL ); 

			GetCursorPos( &pt ); 
			tray_icon_menu.Init( main_layout, pt ); 
		}
		else if( lEvent == WM_LBUTTONDBLCLK ) 
		{ 
			ShowWindow(); 
			SetForegroundWindow( m_hWnd ); 
		} 

_return:
		return ret; 
	} 

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
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
		LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;

		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTONEAREST), &oMonitor);
		CRect rcWork = oMonitor.rcWork;
		CRect rcMonitor = oMonitor.rcMonitor;
		
		rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

		// �������ʱ����ȷ��ԭ������
		lpMMI->ptMaxPosition.x	= rcWork.left;
		lpMMI->ptMaxPosition.y	= rcWork.top;

		lpMMI->ptMaxTrackSize.x = rcWork.GetWidth();
		lpMMI->ptMaxTrackSize.y = rcWork.GetHeight();

		lpMMI->ptMinTrackSize.x = m_pm.GetMinInfo().cx;
		lpMMI->ptMinTrackSize.y = m_pm.GetMinInfo().cy;
	
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
		else if( wParam == ( WPARAM )'f' 
			|| wParam == ( WPARAM )'F')
		{		
			key_state = MapKeyState(); 
			if( MK_CONTROL == ( MK_CONTROL & key_state ) ) 
			{
				ret = search_events(); 
				bHandled = TRUE; 
			}
		}
		else if( wParam == ( WPARAM )'m' 
			|| wParam == ( WPARAM )'M')
		{		
			if( BROWSER_VERBOSE_MODE == browser_mode )
			{
 				browser_mode = BROWSER_SIMPLE_MODE; 
			}
			else
			{
				browser_mode = BROWSER_VERBOSE_MODE;
			}
		}
		else if( wParam == ( WPARAM )VK_F3 )
		{	
			LRESULT _ret; 
			_ret = search_events(); 
		}		
		return ret; 
	}

	LRESULT swap_max_restore( BOOLEAN is_max )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI* pControl; 

		pControl  = static_cast<CControlUI*>(m_pm.FindControl(_T("max_btn")));
		if( pControl ) 
		{
			pControl->SetVisible( is_max == FALSE ? true : false );
		}
		else
		{
			ASSERT( FALSE ); 
			ret = ERROR_NOT_READY; 
		}

		pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("nor_btn")));

		if( pControl ) 
		{
			pControl->SetVisible( is_max == FALSE ? false : true ); 
		}
		else
		{
			ASSERT( FALSE ); 
			ret = ERROR_NOT_READY; 
		}

		return ret; 
	}

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// ��ʱ�����յ�WM_NCDESTROY���յ�wParamΪSC_CLOSE��WM_SYSCOMMAND
		if( wParam == SC_CLOSE )
		{
			short_window_effective(); 

			::PostQuitMessage(0L);

			//ShowWindow( false, false ); 

			bHandled = TRUE;
			return 0;
		}

		BOOL bZoomed = ::IsZoomed(*this);
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		if( ::IsZoomed(*this) != bZoomed ) {
			if( !bZoomed ) 
			{
				swap_max_restore( TRUE ); 
			}
			else {
				swap_max_restore( FALSE ); 
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
		// ��ʱ�����յ�WM_NCDESTROY���յ�wParamΪSC_CLOSE��WM_SYSCOMMAND
		bHandled = false; 
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

		tmp_text = _get_string_by_id( TEXT_SYS_ACTION_TIP_SPECIOUS_ACTION, _T( "�����ɵ�ϵͳ��Ϊ" ) ); 

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

		switch( wParam )
		{
		case DBT_DEVICEARRIVAL:
			// Check whether a CD or DVD was inserted into a drive.
			if( lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME )
			{
				PDEV_BROADCAST_VOLUME lpdbv = ( PDEV_BROADCAST_VOLUME )lpdb;

				if( lpdbv->dbcv_flags & DBTF_MEDIA )
				{
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

	//void OnDropFiles( ULONG msg, WPARAM hwnd, LPARAM hDropInfo, BOOL &bHandled )
	void OnDropFiles( HWND hwnd, HDROP hDropInfo )
	{
		UINT  nFileCount = ::DragQueryFile( ( HDROP )hDropInfo, (UINT)-1, NULL, 0);
		::DragFinish(hDropInfo);
	}

	void AdaptWindowSize( UINT cxScreen )
	{
		int iX = 968, iY = 600;
		int iWidthList = 225, iWidthSearchEdit = 193;
		SIZE szFixSearchBtn = {201, 0};

		if(cxScreen <= 1024)      // 800*600  1024*768  
		{
			iX = 775;
			iY = 470;
		} 
		else if(cxScreen <= 1280) // 1152*864  1280*800  1280*960  1280*1024
		{
			iX = 968;
			iY = 600;
		}
		else if(cxScreen <= 1366) // 1360*768 1366*768
		{
			iX = 1058;
			iY = 656;
			iWidthList        += 21;
			iWidthSearchEdit  += 21;
			szFixSearchBtn.cx += 21;
		}
		else                      // 1440*900
		{
			iX = 1224;
			iY = 760;
			iWidthList        += 66;
			iWidthSearchEdit  += 66;
			szFixSearchBtn.cx += 66;
		}

		CControlUI *tab_frame = m_pm.FindControl(_T("frame_mid"));

		if( NULL != tab_frame )
		{
			tab_frame->SetFixedWidth( iWidthList ); 
		}

		::SetWindowPos(m_pm.GetPaintWindow(), NULL, 0, 0, iX, iY, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE);
		CenterWindow();
	}

	LRESULT hilight_process( ULONG proc_id )
	{
		LRESULT ret = ERROR_SUCCESS; 
		action_filter_info filter_info; 
		BOOLEAN is_found = FALSE; 
		BOOLEAN removing = FALSE; 
		HRESULT hr; 

		do 
		{
			if( proc_id == ( ULONG )INVALID_PROCESS_ID )
			{
				break; 
			}

			//can set the region more preciously
			w_lock_filter_infos(); 

			{
				action_filter_info old_filter_info; 
				std::vector<action_filter_info*>::iterator it; 

				for( it = filter_infos.filter_infos.begin(); it != filter_infos.filter_infos.end(); it ++ )
				{
					if( (*it)->cond == FLT_PID 
						&& ( (*it)->filter_mode == FLT_INCLUDE ) 
						&& (*it)->value.data.ulong_value == proc_id )
					{
						is_found = TRUE; 

						memcpy( &old_filter_info, (*it), sizeof( old_filter_info ) ); 
						(*it)->ui_info.bk_clr = DEFAULT_HILIGHT_COLOR; 
						update_action_cond_to_db( (*it), &old_filter_info ); 
					}
				}
			}

			w_unlock_filter_infos(); 

			do 
			{
				if( TRUE == is_found )
				{
					break; 
				}

				init_filter_ui_info( &filter_info.ui_info ); 

				filter_info.cond = FLT_PID; 

				filter_info.filter_mode = FLT_INCLUDE; 

				filter_info.compare_mode = FLT_CONTAINS; 

				hr = StringCbPrintfW( filter_info.value.text.text_mode_value, 
					sizeof( filter_info.value.text.text_mode_value ), 
					L"%u", 
					proc_id ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				filter_info.value.text.cc_string_len = ( ULONG )wcslen( filter_info.value.text.text_mode_value ); 

				filter_info.value.type = STRING_VALUE_TYPE; 
				filter_info.value.text.text_is_ptr = FALSE; 
				filter_info.ui_info.bk_clr = DEFAULT_HILIGHT_COLOR; 

				ret = adjust_filter_cond_value( &filter_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = check_action_filter_valid( &filter_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = input_action_cond_to_db( &filter_info ); 
				if( ret != ERROR_SUCCESS )
				{
					//break; 
				}

				{
					action_filter_info *_filter_info; 

					_filter_info = ( action_filter_info* )malloc( sizeof( action_filter_info ) ); 
					if( _filter_info == NULL )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					memcpy( _filter_info, &filter_info, sizeof( *_filter_info ) ); 

					w_lock_filter_infos(); 
					filter_infos.filter_infos.push_back( _filter_info ); 
					w_unlock_filter_infos(); 
				}

			}while( FALSE );

			ret = _trace_frame->hilight_by_key_word(); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

		}while(FALSE );
		return ret; 
	}

	LRESULT include_filter_process( ULONG proc_id )
	{
		LRESULT ret = ERROR_SUCCESS; 
		action_filter_info filter_info; 
		BOOLEAN is_found = FALSE; 
		BOOLEAN removing = FALSE; 
		HRESULT hr; 

		do 
		{
			if( proc_id == ( ULONG )INVALID_PROCESS_ID )
			{
				break; 
			}

			//can set the region more preciously
			w_lock_filter_infos(); 

			{
				action_filter_info old_filter_info; 
				std::vector<action_filter_info*>::iterator it; 

				for( it = filter_infos.filter_infos.begin(); it != filter_infos.filter_infos.end(); it ++ )
				{
					if( (*it)->cond == FLT_PID 
						&& ( (*it)->filter_mode == FLT_INCLUDE ) 
						&& ( (*it)->compare_mode == FLT_EQUALS ) )
					{
						is_found = TRUE; 

						memcpy( &old_filter_info, (*it), sizeof( old_filter_info ) ); 

						if( (*it)->value.data.ulong_value != proc_id )
						{
							(*it)->value.data.ulong_value = proc_id; 
						}

						(*it)->ui_info.bk_clr = DEFAULT_HILIGHT_COLOR; 
						update_action_cond_to_db( (*it), &old_filter_info ); 
					}
				}
			}

			w_unlock_filter_infos(); 

			do 
			{
				if( TRUE == is_found )
				{
					break; 
				}

				init_filter_ui_info( &filter_info.ui_info ); 

				filter_info.cond = FLT_PID; 

				filter_info.filter_mode = FLT_INCLUDE; 

				filter_info.compare_mode = FLT_EQUALS; 

				hr = StringCbPrintfW( filter_info.value.text.text_mode_value, 
					sizeof( filter_info.value.text.text_mode_value ), 
					L"%u", 
					proc_id ); 

				if( FAILED( hr ) )
				{
					dbg_print( MSG_FATAL_ERROR, "%s:%u print the text for filter condition error %u\n", 
						__FUNCTION__, 
						__LINE__, 
						hr ); 

					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				filter_info.value.text.cc_string_len = ( ULONG )wcslen( filter_info.value.text.text_mode_value ); 

				filter_info.value.type = STRING_VALUE_TYPE; 
				filter_info.value.text.text_is_ptr = FALSE; 
				filter_info.ui_info.bk_clr = DEFAULT_HILIGHT_COLOR; 

				ret = adjust_filter_cond_value( &filter_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = check_action_filter_valid( &filter_info ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = input_action_cond_to_db( &filter_info ); 
				if( ret != ERROR_SUCCESS )
				{
				}

				{
					action_filter_info *_filter_info; 

					_filter_info = ( action_filter_info* )malloc( sizeof( action_filter_info ) ); 
					if( _filter_info == NULL )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					memcpy( _filter_info, &filter_info, sizeof( *_filter_info ) ); 

					w_lock_filter_infos(); 
					filter_infos.filter_infos.push_back( _filter_info ); 
					w_unlock_filter_infos(); 
				}

			}while( FALSE );

			ret = _trace_frame->FilterEventsUI(); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

		}while(FALSE );
		return ret; 
	}

	void OnDisplayChange( HWND hwnd, UINT bitsPerPixel, UINT cxScreen, UINT cyScreen )
	{
		AdaptWindowSize(cxScreen);
	}

#ifdef _INC_SHELLAPI
	/* void Cls_OnDropFiles(HWND hwnd, HDROP hdrop) */
#define HANDLE_WM_DROPFILES(hwnd, wParam, lParam, fn) \
	((fn)((hwnd), (HDROP)(wParam)), 0L)
#define FORWARD_WM_DROPFILES(hwnd, hdrop, fn) \
	(void)(fn)((hwnd), WM_DROPFILES, (WPARAM)(HDROP)(hdrop), 0L)
#endif  /* _INC_SHELLAPI */

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE; 

		do 
		{
		
			switch( uMsg )
			{
			case WM_EVENT_LOG_RECEIVE: 
				{
					lRes = on_receive_event_log( uMsg, wParam, lParam, bHandled ); 
					break; 
				}
				break; 
			case WM_EVENT_LOG_RECEIVE_DONE:
				{
					lRes = on_receive_event_log_done( uMsg, wParam, lParam, bHandled ); 
					break; 
				}
				break; 

				HANDLE_MSG (*this, WM_DROPFILES, OnDropFiles);
				HANDLE_MSG (*this, WM_DISPLAYCHANGE, OnDisplayChange);

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
			case WM_COMMAND: 
				{
					lRes = OnCommand(uMsg, wParam, lParam, bHandled); 
					break; 
				}
			case WM_NOTIFY:
				{
					LPNMHDR notify_hdr; 

					notify_hdr = ( LPNMHDR )lParam; 

					switch( notify_hdr->code )
					{
					case TTN_GETDISPINFO:
						{
							LPNMTTDISPINFO pInfo = ( LPNMTTDISPINFO )lParam;

							::SendMessage( pInfo->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, MAX_TOOLTIP_WIDTH ); 
	
							break;
						}
					}
					break;
				}
			case WM_LBUTTONDOWN: 
				{
					LRESULT ret;

				
					POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
					CControlUI* pControl = m_pm.FindControl(pt);

					bHandled = FALSE; 

					if( pControl == NULL ) 
					{
						break;
					}

					if( pControl->GetManager() != &m_pm ) 
					{
						break;
					}
					
					{
						CStdString name; 
						name = pControl->GetName(); 
						
						if( name.CompareNoCase( L"target_btn" ) == 0 )
						{
							ret = on_focus_proc_button_down( GetHWND() ); 
						
							bHandled = TRUE; 
							break; 
						}
					}
				}
				break; 
			case WM_LBUTTONUP:
				{
					LRESULT ret;
					bHandled = FALSE; 

					if( focus_proc_button_down == FALSE )
					{
						break; 
					}

					{
						ULONG proc_id; 
	

						ret = on_focues_proc_button_up( GetHWND(), &proc_id ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						ret = include_filter_process( proc_id ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}
					}

					bHandled = TRUE; 
				}
				break; 
		
			case WM_TIMER: 
				lRes = on_timer( uMsg, wParam, lParam, bHandled ); 
				break; 
			case WM_RESPONSE_EVENT_NOTIFY:
				ASSERT( FALSE ); 
				break;  
			case WM_ACTION_EVENT_LOG:
				{
					if( _trace_frame != NULL )
					{
						lRes = _trace_frame->on_action_event_log(uMsg, wParam, lParam, bHandled); 
					}
					break;
				}
			case WM_ACTION_EVENT_FILTERING: 	
				{
					/****************************************************
					��ʾ����ƿ����
					****************************************************/
					if( _trace_frame )
					{
						_trace_frame->FilterEventsUI(); 
					}
				}
				break; 

			case WM_DEVICECHANGE:
				lRes = on_usb_dev_change( uMsg, wParam, lParam, bHandled ); 
				break; 
			case WM_RBUTTONDOWN:
				bHandled = FALSE; 
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

	about_dlg about_box; 

	tray_icon icon_tray; 
	HICON main_icon; 
	icon_menu tray_icon_menu; 

	LONG upload_x; 
	LONG download_x; 

	LONG upload_right_x; 
	LONG download_right_x; 
	LONG logo_left; 
	ULONG timer_id; 
	ULONG icon_show_time; 
	ULONG icon_showed_time; 
	trace_frame *_trace_frame; 
	search_dlg *_search_dlg; 
};

CBitSafeFrameWnd* main_frame = NULL; 

LRESULT WINAPI notify_action_event_to_ui( ULONG filtered_event_count, ULONG event_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		if( main_frame->GetHWND() == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		main_frame->PostMessage( WM_ACTION_EVENT_LOG, ( WCHAR )filtered_event_count, ( LPARAM )event_count ); 

	}while( FALSE );

	return ret; 
}

#ifdef _DEBUG
ULONG test_proc_id = ( ULONG )INVALID_PROCESS_ID; 
#endif //_DEBUG

VOID WINAPI process_log(PEVENT_TRACE pEvent)
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{

		if( process_log_sw == FALSE )
		{
			break; 
		}

#define MAX_EVENT_LOG_COUNT_INTEGER_NUMBER ( LONG )( 0x7fffffff )
		if( all_actions.size() > MAX_EVENT_LOG_COUNT_INTEGER_NUMBER )
		{
			break; 
		}

		switch( pEvent->Header.Class.Type )
		{
		case SYSTEM_ACTION_MESSAGE_ID:
			break; 
		case MODULE_ACTIVITY_MESSAGE_ID:
			break; 
		default:
			break; 
		}

		if( pEvent->MofData == NULL )
		{
			dbg_print( MSG_FATAL_ERROR, "etw can not out the data of log now" ); 
			break; 
		}

		if( pEvent->MofLength == 0 )
		{
			DBG_BP(); 
			break; 
		}

		{
			r3_action_notify *action; 

		
			{
				action = ( r3_action_notify* )( ( BYTE* )pEvent->MofData ); 
			}


			ASSERT( pEvent->MofLength == action->size ); 

			ret = process_event( action ); 
			if( ret != ERROR_SUCCESS )
			{
				//break; 
			}
		}

	}while( FALSE );

	return; 
}

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

#ifdef _DEBUG
#include <wininet.h>

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
		tmp_text = RUN_BITTRACE_NEED_RESTART; ; 

		ret = show_msg( NULL, tmp_text, &ret_state ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
		}

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

INT32 os_bit = 32; 
INT32 os_version = OSINVALID; 
INT32 is_wow64 = FALSE; 

#include "cert_manage.h"
LRESULT WINAPI check_os_compatible()
{
	LRESULT ret; 

	do 
	{
		ret = get_sys_bits( &os_bit, &is_wow64 ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		os_version = get_windows_os_version(); 
		if( os_version == OSOTHER 
			|| os_version == OSINVALID 
			|| os_version == OSWIN98 
			|| os_version == OSWIN2000 
			|| ( os_version == OSWINXP && os_bit == 64 ) )
		{
			log_trace( ( MSG_FATAL_ERROR, "bittrace do not support this windows version %u\n", os_version ) ); 
			break; 
		}

		if( os_bit != 32 )
		{
			if( os_bit == 64 )
			{
				WCHAR file_name[ MAX_PATH ]; 
				dlg_ret_state ret_state; 

				if( 32 == get_proc_bits() )
				{
#ifndef _DEBUG
#define BIITRACE_64BIT_FILE_NAME L"BITTRACE64.EXE" 
#else
#define BIITRACE_64BIT_FILE_NAME L"BITTRACE_ud64.EXE" 
#endif //_DEBUG
					ret = add_app_path_to_file_name( file_name, ARRAYSIZE( file_name ), BIITRACE_64BIT_FILE_NAME, TRUE ); 
					if( ERROR_NOT_FOUND == ret )
					{

						dbg_print( MSG_FATAL_ERROR, "bittrace64 file is lost.\n" ); 
					}

#if NO_SUPPORT_32BIT_PROCCESS
					ret = run_proc( file_name ); 
					if( ERROR_SUCCESS == ret )
					{
						exit( 0 ); 
					}
#endif //NO_SUPPORT_32BIT_PROCCESS
				}

				ret = check_cedereba_cert_installed(); 

				if( ret == ERROR_SUCCESS )
				{
					break; 
				}

				ret = show_msg( NULL, INSTALL_CODEREBA_CERTIFICATE_TIP_TEXT, &ret_state ); 
				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					break; 
				}

				if( ret_state != OK_STATE )
				{
					exit( ERROR_NOT_READY ); 
					break; 
				}

				ret = install_cedereba_cert(); 
				if( ret != ERROR_SUCCESS )
				{
					
				}

				ret = add_app_path_to_file_name( file_name, ARRAYSIZE( file_name ), L"codereba_sign.bat", TRUE ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = run_proc_sync( file_name ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}
			}
			
		}

	}while( FALSE );

	return ret; 
}

LRESULT CALLBACK update_ui( PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	CPaintManagerUI *main_wnd_pm; 

	main_wnd_pm = ( CPaintManagerUI* )context; 

	main_wnd_pm->SetTransparent( get_tranparent() ); 
	
	if( ( ULONG )get_theme_no() == INVALID_THEME_NO )
	{
		ASSERT( FALSE ); 
		set_theme_no( DEFAULT_THEME_NO ); 
		//goto _return; 
	}

	ret = load_theme_setting( main_wnd_pm ); 

//_return:
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
#include <winsock2.h>
#pragma comment( lib, "ws2_32.lib")

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

#ifdef TEST_CRT_MEM_LEAN
#include "test_crt_mem_leak.h"
#endif //TEST_CRT_MEM_LEAN

void CheckOnExit()
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
	assert( _CrtDumpMemoryLeaks() == 0);
}

LRESULT test()
{
	LRESULT ret; 
#define MAX_TIME_STRING_LENGTH 32
	WCHAR time_string[ MAX_TIME_STRING_LENGTH ] = L"2014/12/11 10:10:13"; 
	LARGE_INTEGER time_val; 
	LARGE_INTEGER time_val2; 
	std::string test_std_string; 

	do 
	{
#define TEST_REG_PATH1 L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\FontCache3.0.0.0" 

		//ret = reg_jump( TEST_REG_PATH1, NULL ); 
		//break; 

		ret = string_to_time( time_string, &time_val ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = sprint_local_time( time_string, ARRAYSIZE( time_string ), time_val ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = string_to_time( time_string, &time_val2 ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT install_event_mon_drv(); 

#include "svchost_parse.h"

INT32 APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR cmd_line; 
	INT32 argc; 
	INT32 com_inited = FALSE; 
	LPWSTR *argv; 
	WCHAR *_update_path = NULL; 
	WCHAR *_app_file_path = NULL; 
	CHAR exe_file_name[ _MAX_PATH ];
	DWORD file_name_len; 
	BOOLEAN filter_db_not_exist = FALSE; 

#ifdef _DEBUG
	INT32 dbg_ctr_flag;
#endif //_DEBUG


	{
		vector<PROCESS_SHARED_SERVICE_INFORMATION*> infos; 
		{

			get_all_process_shared_service( &infos ); 
		}
	}

	ret = init_std_lib_mbc_local_lang(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_FATAL_ERROR, "set std lib locale language error\n" ) ); 
	}


#ifdef _DEBUG
	dbg_ctr_flag = _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif //_DEBUG

	CPaintManagerUI::SetInstance( hInstance ); 
	CPaintManagerUI::SetResourcePath( CPaintManagerUI::GetInstancePath() );
	CPaintManagerUI::SetResourceZipResType( DAT_FILE_RES_TYPE, IDR_UI_ZIP_FILE_BITTRACE ); 

	{
		LRESULT __ret; 
		CStdString conf_file_path; 

		conf_file_path = CPaintManagerUI::GetInstancePath(); 
		ASSERT( conf_file_path != _T( "" ) ); 

		conf_file_path += ACTION_FILTER_COND_DB_NAME; 

		__ret = check_file_exist( conf_file_path.GetData() ); 
		if( __ret != 0 )
		{
			filter_db_not_exist = TRUE; 
		}
	}

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

	ret = clone_start_self(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_WARNING, "clone start update self failed \n" ) ); 
	
	}

	if( argv != NULL && argc > 1 )
	{
		if( argc == 3 
			&& _wcsicmp( argv[ 1 ], L"/u" ) == 0 
			&& *argv[ 2 ] != L'\0' )

		{
			CHAR update_path[ _MAX_URL_LEN ];  
			LPCTSTR tmp_text; 


			ret = check_os_compatible(); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

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

			update_dlg* _update_dlg = new update_dlg();
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

			tmp_text = UPDATE_DLG_TITLE_TEXT;

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
		}

_normal_start:
		log_trace( ( MSG_INFO, "start normal\n" ) );

		load_lang_conf(); 

		ret = check_must_restart_mark(); 
		if( ret == ERROR_SUCCESS )
		{
			DBG_BP(); 
			show_msg( NULL, _get_string_by_id( TEXT_COMMON_MUST_RESTART, _T( "��ϵͳδ����ǰ,���ذ�ȫ�Ĺ����޷���������." ) ) ); 

			goto _return; 
		}

		ret = check_app_instance( 0 ); 
		if( ret == ERROR_SUCCESS )
		{
			ASSERT( FALSE && "bitsafe aleady running \n" ); 
			goto _return; 
		}

		ret = check_base_files(); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "base work file is currupt!\n" ); 
			dbg_print( MSG_FATAL_ERROR, "base work file is currupt!\n" ); 
		}

		ret = check_os_compatible(); 
		if( ret != ERROR_SUCCESS )
		{

			show_msg( NULL, BITTRACE_SUPPORT_OS_TIP_TEXT ); 

			goto _return; 
		}

		main_frame = new CBitSafeFrameWnd();
		if( main_frame == NULL ) 
		{
			ASSERT( FALSE && "allocate new bitsafe main window error\n" ); 
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		ret = init_all_ui_ctrls(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		adjust_proc_token(); 
		
		ret = open_function_conf(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = init_peripheral_devices_info(); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "intialize peripheral list error\n" ); 
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

		if( TRUE == filter_db_not_exist )
		{
			input_default_filter_info( 0 ); 
		} 

		ret = load_filter_conds(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = init_filter_infos(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = init_event_store(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = safe_output_themes(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "output bitsafe themes error 0x%0.8x\n", ret ) ); 
		}
	
		LPCTSTR tmp_text; 

		tmp_text = MAIN_BITTRACE_TITLE_TEXT; 

		main_frame->Create(NULL, tmp_text, ( UI_WNDSTYLE_FRAME ), WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES, 0, 0, 800, 572);
		main_frame->CenterWindow();
		main_frame->SetIcon( IDI_MAIN_ICON ); 
		::ShowWindow( *main_frame, SW_SHOW );

		/********************************************************
		ע�⣺
		��Ϣѭ���У�������յ�WM_QUIT��Ϣ������Ҫ����WPARAM������
		********************************************************/
	}

	CPaintManagerUI::MessageLoop(); 
	ASSERT(::IsWindow(main_frame->GetHWND()));
	if( ::IsWindow(main_frame->GetHWND()) ) 
	{
		main_frame->SendMessage( WM_CLOSE ); 
	}

	main_frame = NULL; 

_return:
	if( com_inited == TRUE )
	{
		::CoUninitialize();
	}

	if( _update_path != NULL )
	{
		StringConvertor::StringFree( _update_path ); 
	} 

	if( main_frame != NULL )
	{
		delete main_frame; 
	}

	return ( INT32 )ret;
}
