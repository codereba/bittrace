#include "common_func.h"
#include "StdAfx.h"
#include "bitsafe_common.h"
#include "ui_ctrl.h"
#include "conf_file.h"
#include "bitsafe_rule_conf.h"
#include "bitsafe_daemon.h"
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

	//sys_act_info->TraceMsgTip = CreateEvent( NULL, FALSE, FALSE, NULL ); 

	//if( sys_act_info->TraceMsgTip == NULL )
	//{
	//	ret = GetLastError(); 
	//	goto _return; 
	//}

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

	//ret = init_url_rules_buf(); 

	//if( ret != ERROR_SUCCESS )
	//{
	//	log_trace( ( MSG_INFO, "init url rule buffer failed 0x%0.8x\n", ret ) ); 
	//}

	//DBG_BP(); 
	ret = get_rule( URL_RULE_TYPE, read_rule_func, NULL, NULL ); 

	if( ret == ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "commit url rules failed 0x%0.8x\n", ret ) ); 
	}

	//release_url_rules_buf(); 
	log_trace( ( MSG_INFO, "leave %s return 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

#if 0
LRESULT load_url_rule_to_service()
{
	LRESULT ret; 
	ret = load_all_url_conf( load_url_rule_config ); 
	return ret; 
}

LRESULT load_url_rule_to_ui()
{
	LRESULT ret; 
	ret = load_all_url_conf( load_url_rule_config_to_buffer ); 
	return ret; 
}
#endif //0

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

		//ret = add_app_rule( ACTION_ALLOW, file_name + 1, FALSE ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	//DBG_BP(); 
		//	goto _return; 
		//}

		//ret = add_app_rule( ACTION_ALLOW, _T( "BITSAFEUPDATE.EXE" ), FALSE ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	//DBG_BP(); 
		//	goto _return; 
		//}
	}

	for( i = 0; i < MAX_ACTION_RULE_TYPE; i ++ )
	{
		//if( i == URL_RULE_TYPE )
		//{
		//	continue; 
		//}

		ret = input_default_rules( ( access_rule_type )i, is_service ); 
		if( ret != ERROR_SUCCESS )
		{
			//DBG_BP(); 
			log_trace( ( MSG_ERROR, "***load the default rule %ws failed\n", get_rule_type_desc( ( access_rule_type )i ) ) ); 
		}

		ret = get_rule( ( access_rule_type )i, read_rule_func, NULL, NULL ); 

		if( ret != ERROR_SUCCESS )
		{
			//DBG_BP(); 
			log_trace( ( MSG_INFO, "load %ws rules failed 0x%0.8x\n", get_rule_type_desc( ( access_rule_type )i ), ret ) ); 
		}
	}

_return:
	log_trace( ( MSG_INFO, "leave %s return 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT response_client_cmd( BYTE* data, ULONG data_len, BYTE* ret_data, ULONG *ret_data_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	CHAR term_ch; 
	ULONG _ret_data_len; 
	pipe_cmd *cmd; 
	ctrl_rule_cmd *rule_cmd; 
	ctrl_rule_resp *resp; 
	INT32 i; 

	ASSERT( ret_data_len != NULL ); 

#ifdef _DEBUG
	*ret_data_len = 0; 

	log_trace( ( MSG_INFO, "client command is:\n" ) ); 

	DUMP_MEM( data, data_len ); 
#endif //_DEBUG

	//for( i = 0; ( ULONG )i < data_len; i ++ )
	//{
	//	ret_data[ i ] = data[ data_len - i - 1 ]; 
	//}

	if( data_len < sizeof( pipe_cmd ) )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	cmd = ( pipe_cmd* )data; 

	switch( cmd->cmd )
	{
	case CMD_ADD_RULE: 
		if( data_len < sizeof( *rule_cmd ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		{
			rule_cmd = ( ctrl_rule_cmd* )data; 

			ret = manage_rule( ( access_rule_type )rule_cmd->rule_type, 
				rule_cmd->offset, 
				FALSE ); 
		}
		break; 
	case CMD_DEL_RULE:
		if( data_len < sizeof( *rule_cmd ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		{
			rule_cmd = ( ctrl_rule_cmd* )data; 

			ret = manage_rule( ( access_rule_type )rule_cmd->rule_type, 
				rule_cmd->offset, 
				TRUE ); 
		}
		break; 
	case CMD_RULE_LOAD_STATUS:
		{
			ULONG wait_ret; 
			ASSERT( init_thread != NULL ); 

			wait_ret = WaitForSingleObject( init_thread, 0 ); 

			ASSERT( wait_ret != WAIT_FAILED ); 

			if( wait_ret != WAIT_OBJECT_0 )
			{
				ret = ERROR_NOT_READY; 
			}
#ifdef _DEBUG
			else
			{
				ULONG exit_code; 
				GetExitCodeThread( init_thread, &exit_code ); 
				if( exit_code != ERROR_SUCCESS )
				{
					log_trace( ( MSG_ERROR, "!!!load the rules failed in service" ) ); 
				}
			}
#endif //_DEBUG
		}
		break; 
	default:
		ret = ERROR_INVALID_PARAMETER; 
		ASSERT( FALSE && "unknown pipe command" ); 
		break; 
	}

	resp = ( ctrl_rule_resp* )ret_data; 

	resp->cmd = cmd->cmd; 
	resp->param = ret; 

	*ret_data_len = sizeof( *resp ); 
	//ret_data

_return:
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

	//for( i = 0; ( ULONG )i < data_len; i ++ )
	//{
	//	if( ret_data[ i ] != data[ data_len - i - 1 ] )
	//	{
	//		ASSERT( FALSE ); 
	//	}
	//}

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

LRESULT notify_rule_modify( action_rule_buffered *rule_buf, ULONG rule_cmd )
{
	LRESULT ret = ERROR_SUCCESS; 
	ctrl_rule_cmd *cmd; 
	BYTE cmd_buf[ 100 ]; 
	ULONG cmd_len;
	BYTE *resp; 
	ULONG resp_len; 

	ASSERT( rule_buf != NULL ); 
	ASSERT( TRUE == is_valid_rule_cmd( rule_cmd ) ); 
	ASSERT( TRUE == is_valid_access_rule_type( rule_buf->rule.type ) ); 

	cmd = ( ctrl_rule_cmd* )cmd_buf; 

	cmd->cmd = rule_cmd; 
	cmd->rule_type = rule_buf->rule.type; 
	cmd->offset = ( ULONG )( ( BYTE* )rule_buf - safe_region_list_base( &all_rules_buf[ cmd->rule_type ] ) ); 

	ASSERT( cmd->offset < safe_region_list_size( &all_rules_buf[ cmd->rule_type ] ) ); 

	ret = exec_cmd_on_pipe( BITSAFE_SERVICE_PIPE, 
		( BYTE* )cmd, 
		sizeof( *cmd ), 
		&resp, 
		&resp_len ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "execute command on pipe line failed" ); 
		goto _return; 
	}

	ret = accept_server_response( resp, 
		resp_len, 
		cmd_buf, 
		&cmd_len ); 

_return:
	return ret; 
}

LRESULT check_rules_loaded()
{
	LRESULT ret = ERROR_SUCCESS; 
	//BOOL _ret; 
	pipe_cmd *cmd; 
	BYTE cmd_buf[ 100 ]; 
	ULONG cmd_len;
	BYTE *resp; 
	ULONG resp_len; 

	cmd = ( pipe_cmd* )cmd_buf; 

	cmd->cmd = CMD_RULE_LOAD_STATUS; 
	*( ULONG* )cmd->param = 0; 

	ret = exec_cmd_on_pipe( BITSAFE_SERVICE_PIPE, 
		( BYTE* )cmd, 
		sizeof( *cmd ), 
		&resp, 
		&resp_len ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "execute command on pipe line failed" ); 
		goto _return; 
	}

	ret = accept_server_response( resp, 
		resp_len, 
		cmd_buf, 
		&cmd_len ); 

	//if( ret != ERROR_SUCCESS )
	//{

	//}

_return:
	return ret; 
}

LRESULT load_all_confs_to_ui( )
{
	LRESULT ret = ERROR_SUCCESS; 

	for( ; ; )
	{
		ret = check_rules_loaded(); 
		if( ret != ERROR_SUCCESS )
		{
			Sleep( 500 ); 
		}
		else
		{
			break; 
		}
	}

	load_rules_from_mem( load_rule_buf ); 
	//ret = load_all_confs( load_rule_to_buffer, FALSE ); 
	return ret; 
}

FORCEINLINE LRESULT load_all_confs_to_service( )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = load_all_confs( load_rule_config, TRUE ); 
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

		//DBG_BP(); 
		if( type == SYS_LOG_NOTIFY )
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
			//goto _return; 
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
	//ret = safe_create_dir_fram_res( _T( "themes" ) ); 
	
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

//#ifndef _DEBUG
	ret = load_db_files_from_res(); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}
//#endif //_DEBUG

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
	INT32 i; 

	//DBG_BP(); 

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

//#define DRIVER_INSTALL_TRY_TIME 300
#ifdef DRIVER_TEST
	//for( ; ; )
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
			//ASSERT( FALSE ); 
			//Sleep( 5000 ); 
			//continue; 
			log_trace( ( MSG_ERROR, "!!!!bitsafe file is currupted or system ( and some software ) configuration have not compatible with bitsafe." ) ); 
			DBG_BP(); 
			goto _return; 
		}
		//else
		//{
		//	break; 
		//}
	}

#endif //DRIVER_TEST

	//if( i == DRIVER_INSTALL_TRY_TIME )
	//{
	//	ASSERT( ret != ERROR_SUCCESS ); 
	//	goto _return; 
	//}

	ASSERT( ret == ERROR_SUCCESS ); 

//#ifdef DRIVER_TEST
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

	//ret = load_url_rule_to_service(); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	ASSERT( FALSE ); 
	//	goto _return; 
	//}

//#endif //DRIVER_TEST

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
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

			//if( ret_state == OK_STATE )
			//{
			//	windows_shutdown( EWX_REBOOT | EWX_FORCE );  
			//}
		}

		goto _return; 
	}

//#ifndef _SIMULATE_TEST
//	//ret = ERROR_SUCCESS; 
//	ret = check_run_service(); 
//	if( ret != ERROR_SUCCESS )
//	{
//		log_trace( ( MSG_ERROR, "start system manage service failed\n" ) ); 
//		goto _return; 
//	}
//#endif //_SIMULATE_TEST
//	//goto _return; 
//
#ifdef _DEBUG
	test_work_timer();
#endif //_DEBUG

	ASSERT( param != NULL ); 

//#ifdef HAVE_KRNL_HOOK
//	ret = enable_sys_manage_func( param ); 
//	if( ret != ERROR_SUCCESS )
//	{
//		goto _return;
//	}
//#endif //HAVE_KENL_HOOK

	//( ( action_ui_notify* )param )->action_notify( ALL_DRIVER_LOADED, NULL ); 
	ret = get_cur_work_mode( &cur_mode, ( action_ui_notify* )param ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "get current work mode failed \n" ) ); 
	}

	ret = load_all_confs_to_ui(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load all configure to ui failed \n" ) ); 
	}

	//ret = load_url_rule_to_ui(); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	log_trace( ( MSG_ERROR, "get all url configure to ui failed \n" ) ); 
	//}

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

	//DBG_BP(); 
	for( ; ; )
	{
		ret = init_rules_ipc_buf( FALSE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "!!!initialize the ipc buffer for rule failed\n" ) ); 
			ASSERT( FALSE ); 
			
			retry_time ++; 
			if( retry_time == 6 )
			{
				break; 
			}

			Sleep( 1000 ); 
		}
		else
		{
			break; 
		}
	}

	if( ret != ERROR_SUCCESS )
	{
		if( is_vista_or_later() )
		{
			tmp_text = _get_string_by_id( TEXT_INSTALL_RETRY_EXEC_APP, 
				_T( "比特安全加载失败，请重新运行程序。如果仍无法加载，请将信息发送至( email:" ) BITSAFE_EMAIL __t( " bbs:codereba.com/bbs ),或者提出建议,谢谢." ) ); 

			show_msg( NULL, tmp_text, NULL ); 				
		}
		else
		{
			tmp_text = _get_string_by_id( TEXT_INSTALL_REINSTALL, 
				_T( "比特安全加载失败，请卸载后重新安装再试。如果仍无法加载，请将信息发送至( email:" ) BITSAFE_EMAIL __t( " bbs:codereba.com/bbs ),或者提出建议,谢谢." ) ); 

			show_msg( NULL, tmp_text, NULL ); 
		}
	}

	ret = init_erc_env( param, TRUE ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load the erc ocnfiguration failed \n" ) ); 
	}

	ret = init_dns_mod(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load dns function module failed\n" ) ); 
	}

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

	release_dns_mod(); 

	uninit_erc_env(); 

	uninit_rules_ipc_buf(); 

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

	release_dns_mod(); 

	uninit_erc_env(); 

	uninit_bitsafe_function_state(); 

	uninit_rules_ipc_buf(); 

_return:
	return ret; 
}

sys_action_mng_daemon sys_action_daemon; 
LRESULT init_service_context_async()
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ret = init_rules_ipc_buf( TRUE ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!initialize the ipc buffer for rule failed\n" ) ); 
		goto _return; 
	}

	init_erc_env( NULL, FALSE ); 

	ret = init_dns_mod(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "initialize the dns function module failed \n" ) ); 
	}

//#define _SIMULATE_TEST

	//ret = safe_open_bitsafe_conf(); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	log_trace( ( MSG_ERROR, "initialize the bitsafe configure db failed \n" ) ); 
	//	goto _return; 
	//}

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
	//INT32 i; 
	HANDLE service_pipe = NULL; 
	pipe_ipc_point point = { 0 }; 
	ULONG data_len; 
	BYTE *data = NULL; 
	BYTE *ret_data = NULL; 
	ULONG ret_data_len; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) );

	//ret = create_name_pipe( BITSAFE_SERVICE_PIPE, &service_pipe ); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	goto _return; 
	//}

	ret = init_pipe_point( &point, BITSAFE_SERVICE_PIPE, SERVER_PIPE, 5 ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	data = ( BYTE* )malloc( MAX_PIPE_DATA_LEN ); 
	if( data == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ret_data = ( BYTE* )malloc( MAX_PIPE_DATA_LEN ); 
	if( ret_data == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}


	//for( i = 0; i < 1000; i ++ )
	//{
	//	log_trace( ( MSG_INFO, "%s test output %d\n", __FUNCTION__, i ) ); 
	//	Sleep( 100 ); 
	//}

	//start_netcfg_manage(); 

	for( ; ; )
	{
		ret = read_pipe_sync( &point, ( BYTE* )&ret_data_len, sizeof( ret_data_len ) ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		if( ret_data_len == 0 || ret_data_len > MAX_PIPE_DATA_LEN )
		{
			ret = ERROR_INVALID_DATA; 
			goto _return; 
		}

		//realloc_if_need( &data, )
		ret = read_pipe_sync( &point, ret_data, ret_data_len ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = response_client_cmd( ret_data, ret_data_len, data, &data_len ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = write_pipe_sync( &point, ( BYTE* )&data_len, sizeof( ULONG ) ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = write_pipe_sync( &point, data, data_len ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}


		//ret = do_ui_ctrl( BITSAFE_UI_ID, 
		//	BITSAFE_INSTALL_ALL_DRIVERS, 
		//	NULL, 
		//	0, 
		//	NULL, 
		//	0, 
		//	NULL, 
		//	NULL ); 

		//if( ret != ERROR_SUCCESS )
		//{
		//	log_trace( ( MSG_ERROR, "load all sevefw drivers failed \n" ) ); 
		//}
		//else
		//{
		//	log_trace( ( MSG_INFO, "load all sevefw drivers successfully \n" ) ); 
		//}

		Sleep( 0 ); 
	}

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

	//DBG_BP(); 

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