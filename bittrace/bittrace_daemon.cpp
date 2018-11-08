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

#include "common_func.h"
#include "StdAfx.h"
#include "bitsafe_common.h"
#include "ui_ctrl.h"
#include "conf_file.h"
#include "bitsafe_rule_conf.h"
#include "bittrace_daemon.h"
#include "sys_mng_api.h"
#include "bitsafe_conf.h"
#include "user_manage.h"
#include "trace_log_api.h"
#include "ui_ctrl_mgr.h"
#include "msg_box.h"
#include "service.h"
#include "General.h"
#include "erc_setting.h"
#include "dns_flush.h"
#include "pipe_line.h"
#include "lang_dlg.h"
#include "action_logger.h"
#include "volume_name_map.h"

#pragma intrinsic( memcpy, strcpy )

HANDLE init_thread = NULL; 
HANDLE daemon_thread = NULL; 
BOOLEAN stop_pipe_daemon = FALSE; 

LRESULT delete_all_conf_file()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = del_lang_conf(); 

	return ret; 
}

LRESULT unistall_bitsafe()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	_ret = set_self_autorun( 0 ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "stop seven fw error 0x%0.8x\n", ret ) ); 
		ASSERT( FALSE ); 
	}

	_ret = stop_service( SERVICE_NAME ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "stop seven fw service failed 0x%0.8x\n", ret ) ); 
		ASSERT( FALSE ); 
		//goto _return; 
	}

	_ret = uninstall_service( SERVICE_NAME ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "uninstall seven fw service failed 0x%0.8x\n", ret ) ); 
		ASSERT( FALSE ); 
		//goto _return; 
	}

	_ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_UNINSTALL_ALL_DRIVERS, NULL, 0, NULL, 0, NULL, NULL ); 
	if( _ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		//goto _return; 
	}

	_ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_DELETE_ALL_DRIVERS, NULL, 0, NULL, 0, NULL, NULL ); 
	if( _ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		ret = _ret; 
	}

	ret = delete_all_conf_file( ); 
	if( _ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "delete the all config file failed" ); 
		ret = _ret; 
	}

	_ret = set_self_autorun( 0 ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "stop seven fw error 0x%0.8x\n", ret ) ); 

		if( _ret != ERROR_FILE_NOT_FOUND )
		{
			ASSERT( FALSE ); 
		}
	}

_return:
	return ret; 
}

LRESULT enable_sys_manage_func( PVOID param )
{
	INT32 ret;
	DWORD ret_length; 
	ULONG Index;
	PHOOK_FILTER_INFO sys_act_info = NULL; 

	sys_act_info = ( PHOOK_FILTER_INFO )malloc( MAX_HOOK_INFO_SIZE );

	if( NULL == sys_act_info )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return;
	}

	sys_act_info->Size = 0;
	sys_act_info->TraceMsgTip = NULL; 

	ULONG RegMngFilterIndex[] = {
		NtCreateKeyFilter, 
			NtQueryValueKeyFilter, 
			NtDeleteKeyFilter, 
			NtDeleteValueKeyFilter, 
			NtSetValueKeyFilter, 
			NtRestoreKeyFilter, 
			NtOpenKeyFilter
	};

	ULONG ProcessMngFilterIndex[] = {
		NtCreateProcessFilter, 
			NtCreateProcessExFilter, 
			NtCreateUserProcessFilter, 
			NtCreateThreadOrNtCreateThreadExFilter, 
			NtOpenThreadFilter, 
			NtTerminateProcessFilter, 
	};

	ULONG FileMngFilterIndex[] = {
		NtDeleteFileFilter, 
			NtOpenFileFilter, 
			NtCreateFileFilter
	};

	ULONG DrvMngFilterIndex[] = {
		NtLoadDriverFilter 
	};

	FILTER_INDEX_TABLE AllFilterIndexes[] = {
		{ DrvMngFilterIndex, ARRAY_SIZE( DrvMngFilterIndex ) }, 
		{ FileMngFilterIndex, ARRAY_SIZE( FileMngFilterIndex ) }, 
		{ RegMngFilterIndex, ARRAY_SIZE( RegMngFilterIndex ) }, 
		{ ProcessMngFilterIndex, ARRAY_SIZE( ProcessMngFilterIndex ) } 
	};

#define FILTER_CHECK_BOX_ID_BEGIN 0
#define FILTER_CHECK_BOX_ID_END 3

	ASSERT( FILTER_CHECK_BOX_ID_END - FILTER_CHECK_BOX_ID_BEGIN <= NtApiFilterEnd );

	sys_act_info->Size = 0;
	for( Index = FILTER_CHECK_BOX_ID_BEGIN; Index <= FILTER_CHECK_BOX_ID_END; Index ++ )
	{
		INT32 i; 
		CHAR Output[ 1024 ];
		for( i = 0; ( ULONG )i < AllFilterIndexes[ Index - FILTER_CHECK_BOX_ID_BEGIN ].IndexNumber; i ++ )
		{
			sprintf( Output, "Add filter index: table %d, index %d, value %d \n", Index - FILTER_CHECK_BOX_ID_BEGIN, 
				i, 
				AllFilterIndexes[ Index - FILTER_CHECK_BOX_ID_BEGIN ].FilterIndexes[ i ] ); 
			OutputDebugStringA( Output ); 

			sys_act_info->FilterIndexes[ sys_act_info->Size ] = AllFilterIndexes[ Index - FILTER_CHECK_BOX_ID_BEGIN ].FilterIndexes[ i ];

			sys_act_info->Size += 1;
		}
	}

	ret = do_ui_ctrl( BITSAFE_UI_ID, SYS_MON_START, ( PBYTE )sys_act_info, MAX_HOOK_INFO_SIZE, NULL, 0, NULL, param ); 

_return:
	if( sys_act_info != NULL )
	{
		free( sys_act_info ); 
	}

	return ret;
}

LRESULT load_all_url_conf( read_action_rule_callback read_rule_func )
{	
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s load url rule\n", __FUNCTION__ ) ); 

	ASSERT( read_rule_func != NULL ); 

	ret = get_rule( URL_RULE_TYPE, read_rule_func, NULL, NULL ); 

	if( ret == ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "commit url rules failed 0x%0.8x\n", ret ) ); 
	}
 
	log_trace( ( MSG_INFO, "leave %s return 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}


LRESULT load_all_confs( read_action_rule_callback read_rule_func, INT32 is_service )
{	
	INT32 i; 
	LRESULT ret = ERROR_SUCCESS;  

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ASSERT( read_rule_func != NULL ); 

	if( is_service == FALSE )
	{
		BOOL _ret; 
		TCHAR sevefw_file_name[ MAX_PATH ]; 
		TCHAR *file_name; 
		access_rule_desc rule_input; 

		_ret = GetModuleFileName( NULL, sevefw_file_name, MAX_PATH ); 
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			goto _return; 
		}
		
		file_name = _tcsrchr( sevefw_file_name, _T( '\\' ) ); 
		ASSERT( file_name != NULL ); 

	}
_return:
	log_trace( ( MSG_INFO, "leave %s return 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}


LRESULT accept_server_response( BYTE* data, ULONG data_len, BYTE* ret_data, ULONG *ret_data_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	//BYTE term_ch; 
	ULONG _ret_data_len; 
	pipe_cmd *cmd; 
	//INT32 i; 

	ASSERT( ret_data_len != NULL ); 

#ifdef _DEBUG
	*ret_data_len = 0; 

	log_trace( ( MSG_INFO, "client command is:\n" ) ); 

	DUMP_MEM( data, data_len ); 
#endif //_DEBUG

	if( data_len < sizeof( pipe_cmd ) )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	cmd = ( pipe_cmd* )data; 

	switch( cmd->cmd )
	{
	case CMD_ADD_RULE: 
		if( data_len < FIELD_OFFSET( pipe_cmd, param ) + sizeof( ULONG_PTR ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
		{
			ret = *( ULONG* )cmd->param; 

			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "add rule failed return 0x%0.8x\n", 
					ret ) ); 
			}
			else
			{
				log_trace( ( MSG_ERROR, "add rule successfully return 0x%0.8x\n", ret ) ); 
			}
		}
		break; 
	case CMD_DEL_RULE:
		if( data_len < FIELD_OFFSET( pipe_cmd, param ) + sizeof( ULONG_PTR ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		{
			ret = *( ULONG* )cmd->param; 

			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "delete rule failed return 0x%0.8x\n", 
					ret ) ); 
			}
			else
			{
				log_trace( ( MSG_ERROR, "delete rule successfully return 0x%0.8x\n", ret ) ); 
			}
		}
		break; 
	case CMD_RULE_LOAD_STATUS: 
		{
			if( data_len < sizeof( sizeof( ULONG ) + sizeof( ULONG ) ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				goto _return; 
			}
			
			ret = *( ULONG* )cmd->param; 
		}
		break; 
	default:
		if( data_len < sizeof( sizeof( ULONG ) + sizeof( ULONG ) ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		ret = *( ULONG* )cmd->param; 
		if( ret != ERROR_INVALID_PARAMETER )
		{
			ASSERT( FALSE && "send unknown pipe command should receive unknown command response" ); 
		}
		break; 
	}

	*ret_data_len = 0; 
	//ret_data

_return:
	return ret; 
}


LRESULT load_all_confs_to_ui( )
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}

FORCEINLINE LRESULT load_all_confs_to_service( )
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}

class sys_action_mng_daemon : public action_logger, public action_ui_notify
{
public:
	sys_action_mng_daemon()
	{

	}

	LRESULT action_notify( action_ui_notify_type type, PVOID param )
	{
		LRESULT ret = ERROR_SUCCESS;
		notify_data *data_notify; 

		log_trace( ( MSG_INFO, "enter %s action type is %d\n", __FUNCTION__, type ) ); 

		data_notify = ( notify_data* )param; 

		if( type == SYS_LOG_NOTIFY )
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
		else
		{
			ASSERT( FALSE && "!!!sevice don't support this action \n" ); 
		}

_return:
		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
		return ret; 
	}
}; 

LPCTSTR all_builtin_themes[] = { 
	_T( "green_bamboo" ), 
	_T( "forest_bamboo" ) }; 

LPCTSTR all_themes_file_name[] = {
		_T( "bk.jpg" ), 
			_T( "thumb.jpg" ), 
			_T( "config.xml" ) }; 

#define THEMES_ROOT_DIR_NAME _T( "themes" )

LRESULT output_theme_files( LPCTSTR theme_path, LPCTSTR prefix_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	CStdString file_name; 
	CStdString file_path; 

	ASSERT( theme_path != NULL ); 
	ASSERT( prefix_name != NULL ); 

	file_path = theme_path; 
	file_path.Append( _T( "\\" ) ); 

	for( i = 0; i < ARRAYSIZE( all_themes_file_name ); i ++ )
	{
		file_name = prefix_name; 
		file_name.Append( _T( "_" ) ); 
		file_name.Append( all_themes_file_name[ i ] ); 

		log_trace( ( MSG_INFO, "output the theme file %ws\n", file_name.GetData() ) ); 

		ret = _create_file_from_res_zip( file_name, all_themes_file_name[ i ], file_path.GetData(), FILE_TARGET_DIR ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "output the builtin theme %ws file error 0x%0.8x\n", file_name.GetData(), ret ) ); 
			ASSERT( FALSE && "output the builtin theme file error" ); 
		}
	}

_return:
	return ret; 
}

LRESULT safe_output_themes()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	CStdString theme_root_path; 
	CStdString theme_path; 
	
	ret = get_themes_path( theme_root_path ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = safe_create_dir( CPaintManagerUI::GetInstancePath().GetData(), theme_root_path.GetData() ); 
	if( ret != ERROR_SUCCESS )
	{
		if( ret != ERROR_ALREADY_EXISTS )
		{
			ASSERT( FALSE && "create themes root directory error" ); 
			goto _return; 
		}
	}

	theme_root_path.Append( _T( "\\" ) );  
	
	for( i = 0; i < ARRAYSIZE( all_builtin_themes ); i ++ )
	{
		theme_path = theme_root_path; 
		theme_path.Append( all_builtin_themes[ i ] ); 

		ret = safe_create_dir( CPaintManagerUI::GetInstancePath().GetData(), theme_path.GetData() ); 
		if( ret != ERROR_SUCCESS )
		{
			if( ret != ERROR_ALREADY_EXISTS )
			{
				ASSERT( FALSE && "create themes root directory error" ); 
				goto _return; 
			}
		}

		ret = output_theme_files( theme_path.GetData(), all_builtin_themes[ i ] ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "output the themes files error\n" ) ); 
			goto _return; 
		}
	}

_return:
	return ret; 
}

LRESULT safe_open_bitsafe_conf()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = load_db_files_from_res(); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = open_bitsafe_conf(); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT config_bitsafe()
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR tmp_text; 
	dlg_ret_state ret_state; 

	tmp_text = _get_string_by_id( TEXT_BITSAFE_SETTING_AUTORUN, _T( "比特安全是否在系统启动时自动加载?" ) ); 
	ret = show_msg( NULL, tmp_text, &ret_state ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( ret_state == OK_STATE )
	{
		ret = set_self_autorun( ADD_SELF_AUTO_RUN ); 
	}
	else
	{
		ret = set_self_autorun( 0 ); 
	}

_return:
	return ret; 
}

DWORD CALLBACK init_service_context_thread( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ret = init_all_ui_ctrls(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "initialize ui control model in service failed \n" ) ); 
		goto _return; 
	}

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
			ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_ENTER_BACK_PROC_DBG_MODE, NULL, 0, NULL, 0, NULL, NULL ); 
		}
	}while( FALSE ); 

	ret = safe_open_bitsafe_conf(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load configure database failed\n" ) ); 
		goto _return; 
	}

	ret = open_function_conf(); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = init_bitsafe_function_state(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load driver loading state error 0x%0.8x\n", ret ) ); 
		goto _return; 
	}


#ifdef DRIVER_TEST
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			BITSAFE_LOAD_SERVICE_CONTEXT, 
			NULL, 
			0, 
			NULL, 
			0, 
			NULL, 
			param ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "!!!!bitsafe file is currupted or system ( and some software ) configuration have not compatible with bitsafe." ) ); 
			DBG_BP(); 
			goto _return; 
		}
	}

#endif //DRIVER_TEST


	ASSERT( ret == ERROR_SUCCESS ); 

	ret = load_all_confs_to_service(); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

#ifdef HAVE_KRNL_HOOK
	ret = enable_sys_manage_func( param ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return;
	}
#endif //HAVE_KENL_HOOK

	ret = init_volumes_name_map(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load volumes mapping error 0x%0.8x\n", ret ) ); 

		ASSERT( FALSE && "load volumes mapping error\n" ); 

		goto _return; 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ( DWORD )ret; 
}

VOID test_work_timer()
{
	LRESULT ret; 
#ifdef _DEBUG
	ULONGLONG test_work_times[ 6 ] = { ( ULONGLONG )3 * 60 * 10000000, 
		( ULONGLONG )5 * 60 * 10000000, 
		( ULONGLONG )30 * 10000000, 
		( ULONGLONG )5 * 60 * 10000000, 
		( ULONGLONG )6 * 60 * 10000000, 
		( ULONGLONG )30 * 10000000 }; 

	INT32 i; 
	LARGE_INTEGER test_time; 

	for( i = 0; i < 5; i ++ )
	{
		test_time.QuadPart = test_work_times[ i ]; 

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			BITSAFE_SET_WORK_TIME, 
			( PBYTE )&test_time, 
			sizeof( LARGE_INTEGER ), 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}

	test_time.QuadPart = test_work_times[ i ]; 
	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		BITSAFE_SET_WORK_TIME, 
		( PBYTE )&test_time, 
		sizeof( LARGE_INTEGER ), 
		NULL, 
		0, 
		NULL, 
		NULL ); 
#endif //_DEBUG
}

LRESULT check_bitsafe_setup_pack()
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}

ULONG CALLBACK init_ui_context_thread( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	work_mode cur_mode; 

#ifdef DRIVER_TEST
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		BITSAFE_LOAD_UI_CONTEXT, 
		NULL, 
		0, 
		NULL, 
		0, 
		NULL, 
		param ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 

		if( ret != ERROR_USER_CANCEL_INSTALL )
		{
			dlg_ret_state ret_state; 
			LPCTSTR tmp_text; 

			ret = check_bitsafe_setup_pack(); 
			if( ret == ERROR_SUCCESS )
			{
				tmp_text = _get_string_by_id( TEXT_INSTALL_NEED_RESTART, _T( "系统或其它安全软件环境与比特安全不兼容,或比特安全文件被破坏,请重启后再次运行.仍有问题,请使用原始安装包重新安装.如果仍无法加载,请将信息发送至( email:" ) BITSAFE_EMAIL __t( " bbs:codereba.com/bbs ),或者提出建议,谢谢." ) ); 
			}
			else
			{

				tmp_text = _get_string_by_id( TEXT_INSTALL_NEED_RESTART, _T( "比特安全文件被破坏,请使用原始安装包重新安装.如果仍无法加载,请将信息发送至( email:" ) BITSAFE_EMAIL __t( " bbs:codereba.com/bbs ),或者提出建议,谢谢." ) ); 
			}

			ret = show_msg( NULL, tmp_text, &ret_state ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
			}
		}

		goto _return; 
	}

	ASSERT( param != NULL ); 

	ret = load_all_confs_to_ui(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load all configure to ui failed \n" ) ); 
	}

	ret = init_volumes_name_map(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load volumes mapping error 0x%0.8x\n", ret ) ); 

		ASSERT( FALSE && "load volumes mapping error\n" ); 

		goto _return; 
	}

#endif //DRIVER_TEST

_return:
	return ret; 
}

LRESULT init_ui_context_async( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG retry_time; 
	LPCTSTR tmp_text; 

	retry_time = 0; 

#ifdef INIT_DRIVER_THREAD
	init_thread = CreateThread( NULL, 0, init_ui_context_thread, param, 0, NULL ); 

	if( init_thread == NULL )
	{
		ASSERT( FALSE ); 
		ret = GetLastError(); 
		goto _return; 
	}
#else

	//ret = load_url_rule_to_ui(); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	//goto _return; 
	//}

	ret = load_all_confs_to_ui(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "load all rule to ui failed\n" ) ); 
	}

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		BITSAFE_LOAD_UI_CONTEXT, 
		NULL, 
		0, 
		NULL, 
		0, 
		NULL, 
		param ); 

#endif //INIT_DRIVER_THREAD

_return:
	return ret; 
}

LRESULT uninit_ui_context()
{
	LRESULT ret = ERROR_SUCCESS; 

	if( init_thread != NULL )
	{
		TerminateThread( init_thread, 0 ); 
		CloseHandle( init_thread ); 
		init_thread = NULL; 
	}

	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		BITSAFE_UNLOAD_UI_CONTEXT, 
		NULL, 
		0, 
		NULL, 
		0, 
		NULL, 
		NULL ); 

_return:
	return ret; 
}

LRESULT uninit_service_context()
{
	LRESULT ret = ERROR_SUCCESS; 

	if( daemon_thread != NULL )
	{
		stop_pipe_daemon = TRUE; 

		TerminateThread( daemon_thread, 0 ); 
		CloseHandle( daemon_thread ); 
		daemon_thread = NULL; 
	}

	if( init_thread != NULL )
	{
		TerminateThread( init_thread, 0 ); 
		CloseHandle( init_thread );
		init_thread = NULL; 
	}

	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		BITSAFE_UNLOAD_SERVICE_CONTEXT, 
		NULL, 
		0, 
		NULL, 
		0, 
		NULL, 
		NULL ); 

	ret = release_ui_ctrls(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "uninitialize ui control model in service failed \n" ) ); 
		goto _return; 
	}

	ret = close_bitsafe_conf(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "unload configure database failed\n" ) ); 
		goto _return; 
	}

_return:
	return ret; 
}

sys_action_mng_daemon sys_action_daemon; 
LRESULT init_service_context_async()
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

#ifdef _SIMULATE_TEST
	init_service_context_thread( ( action_ui_notify* )&sys_action_daemon ); 

#else
	init_thread = CreateThread( NULL, 0, init_service_context_thread, ( action_ui_notify* )&sys_action_daemon, 0, NULL ); 

	if( init_thread == NULL )
	{
		ASSERT( FALSE ); 
		ret = GetLastError(); 
		goto _return; 
	}
#endif

	daemon_thread = CreateThread( NULL, 0, thread_pipe_daemon, ( action_ui_notify* )&sys_action_daemon, 0, NULL ); 

	if( daemon_thread == NULL )
	{
		ASSERT( FALSE ); 
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT pipe_server()
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE service_pipe = NULL; 
	pipe_ipc_point point = { 0 }; 
	ULONG data_len; 
	BYTE *data = NULL; 
	BYTE *ret_data = NULL; 
	ULONG ret_data_len; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) );

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

_return:
	if( data != NULL )
	{
		free( data ); 
	}

	if( ret_data != NULL )
	{
		free( ret_data ); 
	}

	if( point.pipe != NULL )
	{
		uninit_pipe_point( &point ); 
	}
	return ret; 
}

LRESULT pipe_daemon()
{
	LRESULT ret;  

	for( ; ; )
	{
		if( stop_pipe_daemon == TRUE )
		{
			break; 
		}

		ret = pipe_server(); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "response pipe command failed\n" ) ); 

		}

		Sleep( 0 ); 
	}

	return ret; 
}

ULONG CALLBACK thread_pipe_daemon( PVOID param )
{
	return pipe_daemon(); 
}