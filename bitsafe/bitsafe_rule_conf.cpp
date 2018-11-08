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
 
 #include "StdAfx.h"
#include "common_func.h"
#include "bitsafe_common.h"
#include "seven_fw_api.h"
#include "ui_ctrl.h"
#include "conf_file.h"
#include "UIUtil.h"
#include "bitsafe_rule_conf.h"
#include "name_rule_edit_dlg.h"
#include "ip_rule_edit_dlg.h"
#include "proc_shared_data.h"
#include "mem_region_list.h"
#include "bitsafe_daemon.h"

#define DRIVER_TEST
#define DELETE_RULE_NOW

#ifndef BITSAFE_SERVICE_EXE
access_rule_queue all_rules[ MAX_ACTION_RULE_TYPE ]; 
#endif //BITSAFE_SERVICE_EXE

#define MAX_URL_RULE_NUM 2000
#define MAX_SOCKET_RULE_NUM 300 
#define MAX_FILE_RULE_NUM 350
#define MAX_REG_RULE_NUM 350 
#define MAX_COM_RULE_NUM 150
#define MAX_COMMON_RULE_NUM 100

#define MAX_RULES_NUM ( ULONG )( MAX_URL_RULE_NUM + MAX_SOCKET_RULE_NUM + MAX_FILE_RULE_NUM + MAX_REG_RULE_NUM + MAX_COM_RULE_NUM + MAX_COMMON_RULE_NUM ) 
#define MAX_RULES_BUF_SIZE ( ( MAX_RULES_NUM * sizeof( action_rule_buffered ) ) + ( MAX_ACTION_RULE_TYPE * sizeof( mem_region_list ) ) )

#define BITSAFE_RULES_POOL_NAME L"Global\\BITSAFE_RULES_POOL"


FILTER_URLS_INPUT *all_url_rules = NULL; 
ULONG all_url_rules_len = 0; 
ULONG url_rules_buf_used = 0;
#define BUF_ALLOC_INC_LEN ( 4 * 1024 )

CProcessSharedData all_rules_pool; 
safe_region_list all_rules_buf[ MAX_ACTION_RULE_TYPE ] = { 0 }; 

LRESULT check_mem_region_inited( safe_region_list *region_list )
{
	LRESULT ret = ERROR_SUCCESS; 

	if( region_list->region_list == NULL )
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT init_rules_ipc_buf( BOOLEAN is_service )
{
	LRESULT ret = ERROR_SUCCESS; 

	ULONG rule_num[ MAX_ACTION_RULE_TYPE ] = { MAX_SOCKET_RULE_NUM, 
		MAX_URL_RULE_NUM, 
		MAX_FILE_RULE_NUM, 
		MAX_REG_RULE_NUM, 
		MAX_COM_RULE_NUM, 
		MAX_COMMON_RULE_NUM }; 

//#ifdef BITSAFE_SERVICE_EXE
	if( is_service == TRUE )
	{
		BOOL _ret; 
		_ret = all_rules_pool.Create( BITSAFE_RULES_POOL_NAME, MAX_RULES_BUF_SIZE ); 
		if( _ret == FALSE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		ret = depart_rule_region( all_rules_buf, 
			rule_num, 
			MAX_ACTION_RULE_TYPE, 
			all_rules_pool.GetData(), 
			all_rules_pool.GetSize(),  
			sizeof( action_rule_buffered ), 
			all_rules_pool.GetLockHandle(), 
			0 
			); 
	}
//#else
	else
	{
		BOOL _ret; 
		_ret = all_rules_pool.Open( BITSAFE_RULES_POOL_NAME, MAX_RULES_BUF_SIZE ); 
		if( _ret == FALSE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		ret = depart_rule_region( all_rules_buf, 
			rule_num, 
			MAX_ACTION_RULE_TYPE, 
			all_rules_pool.GetData(), 
			all_rules_pool.GetSize(),  
			sizeof( action_rule_buffered ), 
			all_rules_pool.GetLockHandle(), 
			NO_INIT_MEM_REGION
			); 
	}
//#endif //IN_BITSAFE_SERVICE

_return:
	return ret; 
}

LRESULT load_rules_from_mem( read_list_entry read_func )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	OFFSET_LIST_ENTRY *entry; 
	action_rule_buffered *rule_buf; 

	ASSERT( read_func != NULL ); 

	for( i = 0; i < MAX_ACTION_RULE_TYPE; i ++ )
	{
		ret = check_mem_region_inited( &all_rules_buf[ 0 ] ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "not initialize the memory gion for rules" ); 
			goto _return; 
		}

		ret = read_all_allocated_unit( &all_rules_buf[ i ], read_func ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load rules ( type: %d )from ipc shared memory failed\n" ) ); 
		}
	}

_return:
	return ret; 
}

LRESULT uninit_rules_ipc_buf()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	for( i = 0; i < MAX_ACTION_RULE_TYPE; i ++ )
	{
		all_rules_buf[ i ].lock = NULL; 
		all_rules_buf[ i ].release_func = NULL; 
		all_rules_buf[ i ].region_list = NULL; 
	}

	all_rules_pool.Close(); 

	return ret; 
}

LRESULT release_all_rules()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	for( i = 0; i < MAX_ACTION_RULE_TYPE; i ++ )
	{
		all_rules[ i ].clear(); 
	}

	all_rules_pool.Close(); 

	return ret; 
}

#ifdef USE_SECOND_ACL
LRESULT init_url_rules_buf( )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( all_url_rules == NULL ); 

	all_url_rules = ( FILTER_URLS_INPUT* )malloc( BUF_ALLOC_INC_LEN ); 
	if( all_url_rules == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	all_url_rules_len = BUF_ALLOC_INC_LEN; 

	all_url_rules->flag = 0;
	all_url_rules->size = FIELD_OFFSET( FILTER_URLS_INPUT, urls ); 
	url_rules_buf_used = FIELD_OFFSET( FILTER_URLS_INPUT, urls ); 
	memset( all_url_rules, 0, all_url_rules_len ); 

_return:
	return ret; 
}

LRESULT release_url_rules_buf( )
{
	free( all_url_rules ); 
	all_url_rules = NULL; 
	all_url_rules_len = 0; 
	url_rules_buf_used = 0; 

	return ERROR_SUCCESS; 
}

#define OPERA_ADD_RULE 0
#define OPERA_DEL_RULE 1
#define OPERA_DEL_ALL 2

LRESULT add_url_rule_to_buf( LPCWSTR input )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 name_len;
	//ULONG max_size;
	//DWORD ret_len;
	FILTER_URL_INPUT *url_input;
	CHAR *_url = NULL; 

	ASSERT( input != NULL ); 
	ASSERT( NULL != all_url_rules ); 

	name_len = wcslen( input ) + sizeof( CHAR );

	ret = realloc_if_need( ( BYTE** )&all_url_rules, 
		url_rules_buf_used, 
		&all_url_rules_len, 
		name_len + FIELD_OFFSET( FILTER_URL_INPUT, url ) ); 

	if( ERROR_SUCCESS != ret )
	{
		ASSERT( FALSE ); 
		goto _return;
	}

	url_input = ( FILTER_URL_INPUT* )( ( CHAR* )all_url_rules + url_rules_buf_used ); 
	url_input->length = name_len; 

	_url = StringConvertor::WideToAnsi( input ); 
	if( _url == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	strcpy( url_input->url, _url ); 

	log_trace( ( MSG_INFO, "url config length is %d, url seted length is %d\n", url_input->length, strlen( url_input->url ) ) ); 

	ASSERT( strlen( url_input->url ) == url_input->length - sizeof( CHAR ) ); 

	url_rules_buf_used += FIELD_OFFSET( FILTER_URL_INPUT, url ) + url_input->length;

_return:
	if( _url != NULL )
	{
		StringConvertor::StringFree( _url ); 
	}

	return ret;
}

LRESULT commit_url_rules( DWORD opera_mode )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( all_url_rules != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	if( url_rules_buf_used == 0 )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	all_url_rules->size = url_rules_buf_used; 
	if( OPERA_ADD_RULE == opera_mode )
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			HTTP_URL_FLT_ADD_URLS, 
			( PBYTE )all_url_rules, 
			all_url_rules->size, 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}
	else if( OPERA_DEL_ALL == opera_mode )
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			HTTP_URL_FLT_DEL_URLS, 
			( PBYTE )all_url_rules, 
			all_url_rules->size, 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}
	else if( OPERA_DEL_RULE == opera_mode )
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			HTTP_URL_FLT_DEL_URLS, 
			( PBYTE )all_url_rules, 
			all_url_rules->size, 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT config_url_rule( access_rule_desc *rule_input, INT32 mode )
{
	LRESULT ret = ERROR_SUCCESS;
	PFILTER_URL_INPUT filt_rule = NULL; 
	DWORD ret_len; 
	WCHAR *url; 
	CHAR *_url = NULL; 

	ASSERT( rule_input != NULL ); 
	//ASSERT( rule_input->type == URL_RULE_TYPE ); 

#ifndef DRIVER_TEST
	return ret; 
#endif //DRIVER_TEST

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	url = rule_input->desc.params[ NAME_PARAM_INDEX ]->common.name; 

	filt_rule = ( PFILTER_URL_INPUT )malloc( sizeof( FILTER_URL_INPUT ) + wcslen( url ) + sizeof( CHAR ) ); 

	if( NULL == filt_rule )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	url = rule_input->desc.params[ NAME_PARAM_INDEX ]->url.url; 

	filt_rule->length = wcslen( url ) + sizeof( CHAR ); 

	_url = StringConvertor::WideToAnsi( url ); 
	if( _url == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	strcpy( filt_rule->url, _url ); 

	//filt_rule->type = text_type_from_desc( text ); 
	//if( filt_rule->type < 0 )
	//{
	//	ret = FALSE; 
	//	goto _return; 
	//}

	if( mode == TRUE )
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			HTTP_URL_FLT_ADD_URL, 
			( PBYTE )filt_rule, 
			sizeof( FILTER_URL_INPUT ) + filt_rule->length, 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}
	else
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			HTTP_URL_FLT_DEL_URL, 
			( PBYTE )filt_rule, 
			sizeof( FILTER_URL_INPUT ) + filt_rule->length, 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}

_return:
	if( _url != NULL )
	{
		StringConvertor::StringFree( _url ); 
	}

	log_trace( ( MSG_INFO, "leave %s return %d\n", __FUNCTION__, ret ) ); 
	return ret; 
}

#endif //USE_SECOND_ACL

//LRESULT check_params_meaning( access_rule_desc *rule_input )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	ASSERT( rule_input != NULL ); 
//
//	if( rule_input->type == SOCKET_RULE_TYPE )
//	{
//	}
//_return:
//	return ret; 
//}

LRESULT _config_rule( access_rule_desc *rule_input, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS;
	DWORD ret_len; 
	INT32 _ret; 

	ASSERT( rule_input != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

#ifdef DRIVER_TEST

	//_ret = check_params_meaning( rule_input ); 

	if( flags == APP_DEFINE_RULE )
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			BITSAFE_SET_APP_RULE, 
			( PBYTE )rule_input, 
			sizeof( access_rule_desc ), 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}
	else if( flags == APP_ACTION_TYPE_RULE )
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			BITSAFE_SET_APP_ACTION_RULE, 
			( PBYTE )rule_input, 
			sizeof( access_rule_desc ), 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}
	else
	{
		ASSERT( flags == APP_ACTION_RULE ); 

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			BITSAFE_SET_RULE, 
			( PBYTE )rule_input, 
			sizeof( access_rule_desc ), 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}
#endif //DRIVER_TEST

_return:

	log_trace( ( MSG_INFO, "leave %s return %d\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT _del_rule( access_rule_desc *rule_input, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS;
	DWORD ret_len; 

	ASSERT( rule_input != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	if( APP_DEFINE_RULE == flags )
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			BITSAFE_UNSET_APP_RULE, 
			( PBYTE )rule_input, 
			sizeof( access_rule_desc ), 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}
	else if( APP_ACTION_TYPE_RULE == flags )
	{
		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			BITSAFE_UNSET_APP_ACTION_RULE, 
			( PBYTE )rule_input, 
			sizeof( access_rule_desc ), 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}
	else
	{
		ASSERT( APP_ACTION_RULE == flags ); 

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			BITSAFE_UNSET_RULE, 
			( PBYTE )rule_input, 
			sizeof( access_rule_desc ), 
			NULL, 
			0, 
			NULL, 
			NULL ); 
	}

_return:

	log_trace( ( MSG_INFO, "leave %s return %d\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT _modify_rule( access_rule_modify_info *modify_info )
{
	LRESULT ret = ERROR_SUCCESS;
	DWORD ret_len; 

	ASSERT( modify_info != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		BITSAFE_MODIFY_RULE, 
		( PBYTE )modify_info, 
		sizeof( access_rule_modify_info ), 
		NULL, 
		0, 
		NULL, 
		NULL ); 

_return:

	log_trace( ( MSG_INFO, "leave %s return %d\n", __FUNCTION__, ret ) ); 
	return ret; 
}

#ifdef USE_SECOND_ACL
LRESULT _config_url_rule( access_rule_desc *rule_input )
{
	LRESULT ret = ERROR_SUCCESS;
	PFILTER_URL_INPUT filt_rule = NULL; 
	DWORD ret_len; 
	LPCWSTR url; 
	LPSTR _url = NULL; 

	ASSERT( rule_input != NULL ); 
	ASSERT( rule_input->type == URL_RULE_TYPE ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	url = rule_input->desc.params[ NAME_PARAM_INDEX ]->common.name; 

	filt_rule = ( PFILTER_URL_INPUT )malloc( sizeof( FILTER_URL_INPUT ) + wcslen( url ) + sizeof( CHAR ) ); 

	if( NULL == filt_rule )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	url = rule_input->desc.params[ NAME_PARAM_INDEX ]->url.url; 

	filt_rule->length = wcslen( url ) + sizeof( CHAR ); 

	_url = StringConvertor::WideToAnsi( url ); 

	if( _url == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	strcpy( filt_rule->url, _url );  

	//filt_rule->type = text_type_from_desc( text ); 
	//if( filt_rule->type < 0 )
	//{
	//	ret = FALSE; 
	//	goto _return; 
	//}

	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		HTTP_URL_FLT_ADD_URL, 
		( PBYTE )filt_rule, 
		sizeof( FILTER_URL_INPUT ) + filt_rule->length, 
		NULL, 
		0, 
		NULL, 
		NULL ); 

_return:

	if( _url != NULL )
	{
		StringConvertor::StringFree( _url ); 
	}

	log_trace( ( MSG_INFO, "leave %s return %d\n", __FUNCTION__, ret ) ); 
	return ret; 
}

#endif //USE_SECOND_ACL

LRESULT config_app_rule( access_rule_ptr_desc *rule_input, INT32 is_add, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	access_rule_desc _rule_input; 
	ULONG ctl_code; 

	ret = init_access_rule( rule_input->type, &_rule_input ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = copy_rule_input_param( &_rule_input, 
		rule_input ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( flags == APP_ACTION_RULE_OP )
	{
		if( is_add == TRUE )
		{
			ctl_code = BITSAFE_SET_APP_ACTION_RULE; 
		}
		else
		{
			ctl_code = BITSAFE_UNSET_APP_ACTION_RULE; 
		}
	}
	else if( flags == APP_DEFINE_RULE_OP )
	{
		if( is_add == TRUE )
		{
			ctl_code = BITSAFE_SET_APP_RULE; 
		}
		else
		{
			ctl_code = BITSAFE_UNSET_APP_RULE; 
		}
	}
	else
	{
		ASSERT( FALSE && "invalid application define rule operations" ); 
	}

	ret = do_ui_ctrl( BITSAFE_UI_ID, 
		ctl_code, 
		( PBYTE )&_rule_input, 
		sizeof( _rule_input ), 
		NULL, 
		0, 
		NULL, 
		NULL ); 

_return:
	return ret; 
}

LRESULT config_fw_app_rule( action_response_type resp, LPCWSTR app_name, INT32 need_record )
{
	LRESULT ret = ERROR_SUCCESS; 
	access_rule_ptr_desc rule_input; 

	ASSERT( is_valid_response_type( resp ) == TRUE ); 
	ASSERT( app_name != NULL ); 

	init_access_rule_ptr_desc( &rule_input ); 

	rule_input.resp = resp; 							

	rule_input.type = URL_RULE_TYPE; 
	rule_input.desc.params[ 0 ].type = APP_DEFINE; 
	rule_input.desc.params[ 1 ].type = URL_DEFINE; 

	rule_input.desc.params[ 0 ].app.app_name = app_name; 

	if( need_record == TRUE )
	{
		ret = input_rule( &rule_input, APP_ACTION_TYPE_RULE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "add the firewall allow action rule failed %ws \n", app_name ) ); 
		}
	}

	ret = config_app_rule( &rule_input, TRUE, APP_ACTION_RULE_OP ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "configure the application rule failed \n" ) );
	}

	rule_input.type = SOCKET_RULE_TYPE; 

	//rule_input.desc.socket.app.type = APP_DEFINE; 
	//rule_input.desc.socket.app.app.app_name = app_path.GetData(); 
	rule_input.desc.socket.dest_ip.type = IP_DEFINE; 
	rule_input.desc.socket.src_ip.type = IP_DEFINE; 
	rule_input.desc.socket.dest_port.type = PORT_DEFINE; 
	rule_input.desc.socket.src_port.type = PORT_DEFINE; 
	rule_input.desc.socket.src_port.port.type = ALL_PROT; 
	rule_input.desc.socket.dest_port.port.type = ALL_PROT; 

	if( need_record == TRUE )
	{
		ret = input_rule( &rule_input, APP_ACTION_TYPE_RULE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "add the firewall allow action rule failed %ws \n", app_name ) ); 
		}
	}

	ret = config_app_rule( &rule_input, TRUE, APP_ACTION_RULE_OP ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "configure the application rule failed \n" ) );
	}
_return:
	return ret; 
}

LRESULT config_defense_app_rule( action_response_type resp, LPCWSTR app_name, INT32 need_record )
{
	LRESULT	ret = ERROR_SUCCESS; 
	access_rule_ptr_desc rule_input; 

	ASSERT( is_valid_response_type( resp ) == TRUE ); 
	ASSERT( app_name != NULL ); 

	init_access_rule_ptr_desc( &rule_input ); 

	rule_input.type = FILE_RULE_TYPE; 
	rule_input.resp = resp; 
	rule_input.desc.params[ 0 ].type = APP_DEFINE; 
	rule_input.desc.params[ 0 ].app.app_name = app_name; 
	rule_input.desc.params[ 1 ].type = FILE_DEFINE; 

	if( need_record == TRUE )
	{
		ret = input_rule( &rule_input, APP_ACTION_TYPE_RULE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "add the firewall allow action rule failed %ws \n", app_name ) ); 
			goto _return; 
		}
	}

	ret = config_app_rule( &rule_input, TRUE, APP_ACTION_RULE_OP ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "configure the application rule failed \n" ) );
		goto _return; 
	}

	rule_input.type = REG_RULE_TYPE; 
	rule_input.desc.params[ 1 ].type = REG_DEFINE; 

	if( need_record == TRUE )
	{
		ret = input_rule( &rule_input, APP_ACTION_TYPE_RULE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "add the firewall allow action rule failed %ws \n", app_name ) ); 
			goto _return; 
		}
	}

	ret = config_app_rule( &rule_input, TRUE, APP_ACTION_RULE_OP ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "configure the application rule failed \n" ) );
		goto _return; 
	}

	rule_input.type = COM_RULE_TYPE; 
	rule_input.desc.params[ 1 ].type = COM_DEFINE; 

	if( need_record == TRUE )
	{
		ret = input_rule( &rule_input, APP_ACTION_TYPE_RULE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "add the firewall allow action rule failed %ws \n", app_name ) ); 
			goto _return; 
		}
	}

	ret = config_app_rule( &rule_input, TRUE, APP_ACTION_RULE_OP ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "configure the application rule failed \n" ) );
		goto _return; 
	}

	rule_input.type = COMMON_RULE_TYPE; 
	rule_input.desc.params[ 1 ].type = COMMON_DEFINE; 
	if( need_record == TRUE )
	{
		ret = input_rule( &rule_input, APP_ACTION_TYPE_RULE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "add the firewall allow action rule failed %ws \n", app_name ) ); 
			goto _return; 
		}
	}

	ret = config_app_rule( &rule_input, TRUE, APP_ACTION_RULE_OP ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "configure the application rule failed \n" ) );
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT add_app_rule( action_response_type resp, LPCWSTR app_name, INT32 need_record )
{
	LRESULT	ret = ERROR_SUCCESS; 
	access_rule_ptr_desc rule_input; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( is_valid_response_type( resp ) == TRUE ); 
	ASSERT( app_name != NULL ); 

	init_access_rule_ptr_desc( &rule_input ); 

	rule_input.type = FILE_RULE_TYPE; 
	rule_input.resp = resp; 
	rule_input.desc.params[ 0 ].type = APP_DEFINE; 
	rule_input.desc.params[ 0 ].app.app_name = app_name; 
	rule_input.desc.params[ 1 ].type = FILE_DEFINE; 

	ret = config_app_rule( &rule_input, TRUE, APP_DEFINE_RULE_OP ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "configure the application rule failed \n" ) );
		goto _return; 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__, ret ) ); 
	return ret; 
}

#if 0
LRESULT CALLBACK load_url_rule_config( access_rule_type type, action_rule_record *record, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	access_rule_desc rule_input; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	log_trace( ( MSG_INFO, "rule type is %ws \n", get_rule_type_desc( type ) ) ); 
	ASSERT( type == URL_RULE_TYPE ); 
	ASSERT( record->type == URL_RULE_TYPE ); 

	ret = load_rule_from_record( type, record, &rule_input ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = config_rule( &rule_input, NULL, ADD_RULE_DEFINE, APP_ACTION_RULE ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	//ret = add_url_rule_to_buf( rule_input.desc.url.url.url.url ); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	ASSERT( FALSE ); 
	//	log_trace( ( MSG_ERROR, "add url rule to buffer failed 0x%0.8x\n", ret ) ); 
	//	goto _return; 
	//}

	Sleep( 1 ); 

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

#ifdef STL_RULE_BUFFER
LRESULT CALLBACK load_url_rule_config_to_buffer( access_rule_type type, action_rule_record *record, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	//access_rule_desc rule_input; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	log_trace( ( MSG_INFO, "rule type is %ws \n", get_rule_type_desc( type ) ) ); 
	ASSERT( type == URL_RULE_TYPE ); 
	ASSERT( record->type == URL_RULE_TYPE ); 

	//ret = load_rule_from_record( type, record, &rule_input ); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	goto _return; 
	//}

	ret = input_rule_to_buffer( type, record, &all_rules[ URL_RULE_TYPE ], 0, NULL ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load url rule to buffer failed \n" ) ); 
		goto _return; 
	}

	Sleep( 1 ); 

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}
#endif //STL_RULE_BUFFER
#endif //0

LPCWSTR all_def_block_reg_path[ ] = {
										L"HKEY_LOCAL_MACHINE\\SYSTEM"
									};

INLINE LRESULT alloc_access_rule_buffer( safe_region_list *region_list, 
										action_rule_buffered **rule_buffer_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	//action_rule_buffered *rule_alloc; 
	OFFSET_LIST_ENTRY *entry; 

	ASSERT( region_list != NULL ); 
	ASSERT( rule_buffer_out != NULL ); 

	*rule_buffer_out = NULL; 

	ret = alloc_region_unit( region_list, &entry ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}


	//rule_buffer->resize( rule_buffer->size() + 1 ); 
	//rule_alloc = &rule_buffer->operator []( rule_buffer->size() - 1 ); 

	*rule_buffer_out = ( action_rule_buffered* )entry; 

_return:
	return ret; 
}

LRESULT input_rule_to_buffer( access_rule_type type, 
							 action_rule_record *record, 	
							 ULONG flags, 
							 access_rule_desc *loaded_rule )
{
	LRESULT ret = ERROR_SUCCESS; 
	action_rule_buffered *rule_alloc; 
	OFFSET_LIST_ENTRY *entry; 
	//ASSERT( rule_buffer != NULL ); 
	ASSERT( record != NULL ); 
	ASSERT( TRUE == is_valid_access_rule_type( type ) ); 

//#ifdef _DEBUG
//	if( rule_buffer == &all_rules[ SOCKET_RULE_TYPE ] )
//	{
//		//DBG_BP(); 
//	}
//#endif //_DEBUG

#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)     // ntsubauth

#ifdef STL_RULE_BUFFER
	rule_buffer->resize( rule_buffer->size() + 1 ); 
	rule_alloc = &rule_buffer->operator []( rule_buffer->size() - 1 ); 
#else
	ret = alloc_region_unit( &all_rules_buf[ type ], &entry ); 
	if( ret != ERROR_SUCCESS )
	{
		if( ret == STATUS_INSUFFICIENT_RESOURCES )
		{
			log_trace( ( MSG_ERROR, "the rule %d is buffer is overflow max rule number is %dj\n", 
				type, 
				all_rules_buf[ type ].region_list->size ) ); 
		}
		else
		{
			ASSERT( FALSE ); 
		}

		goto _return; 
	}

	rule_alloc = ( action_rule_buffered* )entry; 

#endif //STL_RULE_BUFFER

	if( wcslen( record->desc ) >= _MAX_DESC_LEN )
	{
		ASSERT( FALSE ); 
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	wcscpy( rule_alloc->desc, record->desc ); 

	rule_alloc->flags = flags; 

	if( loaded_rule != NULL )
	{
		memcpy( &rule_alloc->rule, loaded_rule, sizeof( *loaded_rule ) ); 
	}
	else
	{
		ret = load_rule_from_record( type, record, &rule_alloc->rule ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load the parameters of the rule failed type %d\n", type ) ); 
			goto _return; 
		}
	}

	//ret = set_action_desc_param_offset( rule_input->type, &( ( rule_buffer->operator []( rule_buffer->size() - 1 ).rule ).desc ) ); 

_return: 
	return ret; 
}

LRESULT input_default_rules( access_rule_type type, INT32 is_service )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	ULONG _ret; 
	action_rule_buffered *rule_buffer; 
	OFFSET_LIST_ENTRY *entry; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 
	ASSERT( is_valid_access_rule_type( type ) == TRUE ); 

	if( type == SOCKET_RULE_TYPE )
	{
		
	}
	else if ( type == URL_RULE_TYPE )
	{
#ifdef HAVE_DEF_URL_RULE
		ret = alloc_region_unit( &all_rules_buf[ type ], &entry ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		ASSERT( entry != NULL ); 

		rule_buffer = ( action_rule_buffered* )entry; 

		//alloc_access_rule_buffer( &all_rules[ type ], &rule_buffer ); 

		rule_buffer->flags = SUPER_RULE_MASK; 

		ret = init_access_rule( type, &rule_buffer->rule ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		rule_buffer->rule.type = URL_RULE_TYPE; 
		rule_buffer->rule.resp = ACTION_ALLOW; 
		wcscpy( rule_buffer->desc, L"默认URL保护规则" );  
		rule_buffer->rule.desc.params[ 0 ]->id = INVALID_ID_VALUE; 
		rule_buffer->rule.desc.params[ 0 ]->is_cls = FALSE; 
		rule_buffer->rule.desc.params[ 0 ]->type = APP_DEFINE; 
		*rule_buffer->rule.desc.params[ 0 ]->app.app_name = L'\0'; 

		rule_buffer->rule.desc.params[ 1 ]->id = INVALID_ID_VALUE; 
		rule_buffer->rule.desc.params[ 1 ]->is_cls = FALSE; 
		rule_buffer->rule.desc.params[ 1 ]->type = URL_DEFINE; 

		wcscpy( rule_buffer->rule.desc.params[ 1 ]->url.url, L"codereba.com" ); 

		//if( is_service == TRUE )
		//{
		//	ret = _config_rule( &rule_buffer->rule, 0 ); 
		//	if( ret != ERROR_SUCCESS )
		//	{
		//		log_trace( ( MSG_ERROR, "*** config the default file rule failed \n" ) ); 
		//		goto _return; 
		//	}

		//	all_rules[ type ].clear(); 
		//	//all_rules[ type ].erase( all_rules[ type ].begin() + all_rules[ type ].size() - 1 ); 
		//}
#endif //HAVE_DEF_URL_RULE

	}
	else if ( type == FILE_RULE_TYPE )
	{
		//alloc_access_rule_buffer( &all_rules[ type ], &rule_buffer ); 
		//rule_buffer->flags = SUPER_RULE_MASK; 

		ret = alloc_region_unit( &all_rules_buf[ type ], &entry ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		rule_buffer = ( action_rule_buffered* )entry; 

		ret = init_access_rule( type, &rule_buffer->rule ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		rule_buffer->rule.type = FILE_RULE_TYPE; 
		rule_buffer->rule.resp = ACTION_BLOCK; 
		wcscpy( rule_buffer->desc, L"默认文件保护规则" );  
		rule_buffer->rule.desc.params[ 0 ]->id = INVALID_ID_VALUE; 
		rule_buffer->rule.desc.params[ 0 ]->is_cls = FALSE; 
		rule_buffer->rule.desc.params[ 0 ]->type = APP_DEFINE; 
		*rule_buffer->rule.desc.params[ 0 ]->app.app_name = L'\0'; 

		rule_buffer->rule.desc.params[ 1 ]->id = INVALID_ID_VALUE; 
		rule_buffer->rule.desc.params[ 1 ]->is_cls = FALSE; 
		rule_buffer->rule.desc.params[ 1 ]->type = FILE_DEFINE; 

		_ret = GetModuleFileName( NULL, rule_buffer->rule.desc.params[ 1 ]->file.file_path, _MAX_FILE_NAME_LEN ); 
		; 
		if( _ret == 0 )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "set self protection failed\n" ) ); 
			SAFE_SET_ERROR_CODE( ret ); 
			goto _return; 
		}

		if( is_service == TRUE )
		{
			ret = _config_rule( &rule_buffer->rule, APP_ACTION_RULE ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "*** config the default file rule failed \n" ) ); 
				goto _return; 
			}

			//all_rules[ type ].clear(); 
			//all_rules[ type ].erase( all_rules[ type ].begin() + all_rules[ type ].size() - 1 ); 
		}

		//alloc_access_rule_buffer( &all_rules[ type ], &rule_buffer ); 
		//ret = alloc_region_unit( &all_rules_buf[ type ], &entry ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	ASSERT( FALSE ); 
		//	goto _return; 
		//}

		rule_buffer = ( action_rule_buffered* )entry; 

		rule_buffer->flags = SUPER_RULE_MASK; 

		ret = init_access_rule( type, &rule_buffer->rule ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		rule_buffer->rule.type = FILE_RULE_TYPE; 
		rule_buffer->rule.resp = ACTION_LEARN; 
		wcscpy( rule_buffer->desc, L"默认文件保护规则" );  
		rule_buffer->rule.desc.params[ 0 ]->id = INVALID_ID_VALUE; 
		rule_buffer->rule.desc.params[ 0 ]->is_cls = FALSE; 
		rule_buffer->rule.desc.params[ 0 ]->type = APP_DEFINE; 
		*rule_buffer->rule.desc.params[ 0 ]->app.app_name = L'\0'; 

		rule_buffer->rule.desc.params[ 1 ]->id = INVALID_ID_VALUE; 
		rule_buffer->rule.desc.params[ 1 ]->is_cls = FALSE; 
		rule_buffer->rule.desc.params[ 1 ]->type = FILE_DEFINE; 

		//ret = init_access_rule( type, &rule_buffer->rule ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	goto _return; 
		//}

		_ret = GetSystemDirectory( rule_buffer->rule.desc.params[ 1 ]->file.file_path, _MAX_FILE_NAME_LEN ); 
; 
		if( _ret == 0 )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			goto _return; 
		}

		if( is_service == TRUE )
		{
			ret = _config_rule( &rule_buffer->rule, APP_ACTION_RULE ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "*** config the default file rule failed \n" ) ); 
				goto _return; 
			}

			//all_rules[ type ].clear(); 
			//all_rules[ type ].erase( all_rules[ type ].begin() + all_rules[ type ].size() - 1 ); 
		}

		//rule_input.type &= SUPER_RULE_MASK;  
		//input_rule_to_buffer( type, &rule_input, &all_rules[ type ], SUPER_RULE_MASK ); 
	}
	else if ( type == REG_RULE_TYPE )
	{
		for( i = 0; i < ARRAY_SIZE( all_def_block_reg_path ); i ++ )
		{
			//alloc_access_rule_buffer( &all_rules[ type ], &rule_buffer ); 
			ret = alloc_region_unit( &all_rules_buf[ type ], &entry ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				goto _return; 
			}

			rule_buffer = ( action_rule_buffered* )entry; 

			rule_buffer->flags = SUPER_RULE_MASK; 

			ret = init_access_rule( type, &rule_buffer->rule ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			rule_buffer->rule.type = REG_RULE_TYPE; 
			rule_buffer->rule.resp = ACTION_LEARN; 
			wcscpy( rule_buffer->desc, L"默认注册表保护规则" );  
			rule_buffer->rule.desc.params[ 0 ]->id = INVALID_ID_VALUE; 
			rule_buffer->rule.desc.params[ 0 ]->is_cls = FALSE; 
			rule_buffer->rule.desc.params[ 0 ]->type = APP_DEFINE; 
			*rule_buffer->rule.desc.params[ 0 ]->app.app_name = L'\0'; 

			rule_buffer->rule.desc.params[ 1 ]->id = INVALID_ID_VALUE; 
			rule_buffer->rule.desc.params[ 1 ]->is_cls = FALSE; 
			rule_buffer->rule.desc.params[ 1 ]->type = REG_DEFINE; 

			wcsncpy( rule_buffer->rule.desc.params[ 1 ]->reg.reg_path, all_def_block_reg_path[ i ], _MAX_REG_PATH_LEN ); 
			if( is_service == TRUE )
			{
				ret = _config_rule( &rule_buffer->rule, APP_ACTION_RULE ); 
				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( MSG_ERROR, "*** config the default file rule failed \n" ) ); 
					goto _return; 
				}

				//all_rules[ type ].erase( all_rules[ type ].begin() + all_rules[ type ].size() - 1 ); 
				//all_rules[ type ].clear(); 
			}

			//rule_input.type &= SUPER_RULE_MASK; 
			//input_rule_to_buffer( type, &all_rules[ type ], &rule_buffer->rule, SUPER_RULE_MASK ); 
		}
	}
	else if ( type == COM_RULE_TYPE )
	{
	}
	else if ( type == COMMON_RULE_TYPE )
	{
	}

_return:
	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT CALLBACK load_rule_config( access_rule_type type, action_rule_record *record, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	access_rule_desc rule_input; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	log_trace( ( MSG_INFO, "rule type is %ws \n", get_rule_type_desc( type ) ) ); 
	//ASSERT( type != URL_RULE_TYPE ); 
	//ASSERT( record->type != URL_RULE_TYPE ); 
	ASSERT( is_valid_access_rule_type( type ) ); 

	ret = load_rule_from_record( type, record, &rule_input ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load the parameters of the rule failed type %d\n", type ) ); 
		goto _return; 
	}

	if( type == URL_RULE_TYPE )
	{
		ret = _config_rule( &rule_input, record->flag ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "config this rule failed type %d\n", type ) ); 
			goto _return; 
		}
	}
	else
	{
		ret = config_rule( &rule_input, NULL, ADD_RULE_DEFINE, record->flag ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

	ret = input_rule_to_buffer( type, record, 0, &rule_input ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "input this rule to buffer failed type %d\n", type ) ); 
		goto _return; 
	}

	Sleep( 1 ); 

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

#ifdef STL_RULE_BUFFER
LRESULT CALLBACK load_rule_to_buffer( access_rule_type type, action_rule_record *record, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	log_trace( ( MSG_INFO, "rule type is %ws \n", get_rule_type_desc( type ) ) ); 
	//ASSERT( type != URL_RULE_TYPE ); 
	//ASSERT( record->type != URL_RULE_TYPE ); 
	ASSERT( is_valid_access_rule_type( type ) ); 

#ifdef _DEBUG
	if( type == SOCKET_RULE_TYPE )
	{
		//DBG_BP(); 
	}
#endif //_DEBUG

	input_rule_to_buffer( type, record, &all_rules[ type ], 0, NULL ); 

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}
#endif //STL_RULE_BUFFER

LRESULT print_param_define( LPTSTR output, ULONG buf_len, param_all_desc *param_input ) 
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( output != NULL ); 
	ASSERT( param_input != NULL ); 
	ASSERT( is_valid_param_define_type( param_input->type ) ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	*output = L'\0'; 

	if( param_input->is_cls )
	{
		_sntprintf( output, buf_len, _T( "%s" ), param_input->cls.class_name ); 
	}
	else
	{
		switch( param_input->type )
		{
		case URL_DEFINE: 
			_sntprintf( output, buf_len, _T( "%s" ), param_input->url.url ); 
			break; 
		case IP_DEFINE: 
			{
				ULONG ip_begin; 
				ULONG ip_end; 

				UCHAR *ip_begin_bytes; 
				UCHAR *ip_end_bytes; 

				ip_begin = param_input->ip.ip_begin; 
				ip_end = param_input->ip.ip_end; 

				if( ip_end < ip_begin )
				{
					ip_end = ip_begin; 
					log_trace( ( MSG_INFO |  DBG_DBG_BUF_OUT, "the end ip %d is lower than the begin ip %d \n", ip_end, ip_begin ) ); 
				}

				ip_begin_bytes = ( UCHAR* )&ip_begin; 
				ip_end_bytes = ( UCHAR* )&ip_end; 

				_sntprintf( output, buf_len, _T( "%d.%d.%d.%d - %d.%d.%d.%d" ), 
					ip_begin_bytes[ 3 ], 
					ip_begin_bytes[ 2 ], 
					ip_begin_bytes[ 1 ], 
					ip_begin_bytes[ 0 ], 

					ip_end_bytes[ 3 ], 
					ip_end_bytes[ 2 ], 
					ip_end_bytes[ 1 ], 
					ip_end_bytes[ 0 ] 
					); ; 
			}
			break; 
		case PORT_DEFINE: 
			_sntprintf( output, buf_len, _T( "%d-%d" ), 
				param_input->port.port_begin, 
				param_input->port.port_end
				); ; 
			break; 
		case FILE_DEFINE: 
			_sntprintf( output, buf_len, _T( "%s" ), param_input->file.file_path ); 
			break; 
		case REG_DEFINE: 
			_sntprintf( output, buf_len, _T( "%s" ), param_input->reg.reg_path ); ; 
			break; 
		case COM_DEFINE: 
			_sntprintf( output, buf_len, _T( "%s" ), param_input->com.com_name ); 
			break; 
		case APP_DEFINE: 
			_sntprintf( output, buf_len, _T( "%s" ), param_input->app.app_name ); ; 
			break; 
		default:
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
		}
	}

_return:
	log_trace( ( MSG_INFO, "leave %s return 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT print_rule_param( LPTSTR output, ULONG buf_len, access_rule_desc *rule_input, ULONG param_index )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG param_count; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	*output = L'\0'; 
	param_count = get_access_rule_param_count( rule_input->type ); 

	if( param_index >= param_count )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = print_param_define( output, buf_len, rule_input->desc.params[ param_index ] ); 

_return:
	log_trace( ( MSG_INFO, "leave %s return 0x%0.8x \n", __FUNCTION__, ret ) ); 
	return ret; 
}

VOID get_prot_type_desc( LPTSTR text_out, ULONG length, prot_type type )
{
	LPCTSTR tmp_text; 
	if( type == PROT_TCP )
	{
		tmp_text = _get_string_by_id( TEXT_IP_EDIT_PROT_TCP, 
			_T( "TCP协议" ) ); 

		_tcsncpy( text_out, tmp_text, length ); 
	}
	else if( type == PROT_UDP )
	{
		tmp_text = _get_string_by_id( TEXT_IP_EDIT_PROT_UDP, 
			_T( "UDP协议" ) ); 

		_tcsncpy( text_out, tmp_text, length ); 
	}
	else if( type == ALL_PROT )
	{
		tmp_text = _get_string_by_id( TEXT_IP_EDIT_PROT_ALL, 
			_T( "所有协议" ) ); 
		_tcsncpy( text_out, tmp_text, length ); 
	}
	else 
	{
		ASSERT( FALSE ); 
	}
}

VOID get_action_type_desc( LPTSTR text_out, ULONG length, action_response_type action )
{
	LPCTSTR tmp_text; 

	ASSERT( text_out != NULL ); 
	
	if( action == ACTION_ALLOW )
	{
		tmp_text = _get_string_by_id( TEXT_RESPONSE_ALLOW, 
			_T( "允许" ) ); 
		_tcsncpy( text_out, tmp_text, length ); 
	}
	else if( action == ACTION_BLOCK )
	{
		tmp_text = _get_string_by_id( TEXT_RESPONSE_BLOCK, 
			_T( "拒绝" ) ); 
		_tcsncpy( text_out, tmp_text, length ); 
	}
	else if( action == ACTION_LEARN )
	{
		tmp_text = _get_string_by_id( TEXT_RESPONSE_LEARN, 
			_T( "询问" ) ); 
		_tcsncpy( text_out, tmp_text, length ); 
	}
	else 
	{
		ASSERT( FALSE ); 
	}
}

LRESULT process_rule_program_name( WCHAR *program_name, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( program_name != NULL ); 
	ASSERT( length > 0 ); 

	if( *program_name == L'\0' )
	{
		if( length > 2 )
		{
			*program_name = L'*'; 
			*program_name = L'\0'; 
		}
	}

_return:
	return ret; 
}

LRESULT get_rule_column_text( LPTSTR text_out, ULONG length, access_rule_desc *rule_input, LPWSTR desc, ULONG index )
{
	LRESULT ret = ERROR_SUCCESS; 
	PBYTE bytes; 
	param_all_desc *param_desc; 

	ASSERT( desc != NULL ); 

	if( is_name_rule_type( rule_input->type ) == TRUE )
	{
		if( index >= 4 )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
	}
	else if( is_socket_rule_type( rule_input->type ) == TRUE )
	{
		if( index >= 12 )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
	}
	else
	{
		ASSERT( FALSE ); 
	}

	if( index == 0 )
	{
		get_rule_param_from_index( rule_input->type, &rule_input->desc, &param_desc, 0 ); 

		_tcsncpy( text_out, param_desc->common.name, length ); 

		process_rule_program_name( text_out, length ); 
		goto _return; 
	}

	if( rule_input->type == SOCKET_RULE_TYPE )
	{
		ULONG ipv4_addr; 
		param_all_desc *param_desc; 
		
		switch( index )
		{
		case 1:
			ipv4_addr = rule_input->desc.socket.src_ip.ip.ip_begin; 
			bytes = ( PBYTE )&ipv4_addr; 

			_sntprintf( text_out, length, _T( "%d.%d.%d.%d" ), 
				bytes[ 3 ], 
				bytes[ 2 ], 
				bytes[ 1 ], 
				bytes[ 0 ] ); 
			break; 
		case 2:
			ipv4_addr = rule_input->desc.socket.src_ip.ip.ip_end; 
			bytes = ( PBYTE )&ipv4_addr; 

			_sntprintf( text_out, length, _T( "%d.%d.%d.%d" ), 
				bytes[ 3 ], 
				bytes[ 2 ], 
				bytes[ 1 ], 
				bytes[ 0 ] ); 
			break; 
		case 3:
			_sntprintf( text_out, length, _T( "%d" ), 
				rule_input->desc.socket.src_port.port.port_begin ); 			
			break; 
		case 4:
			_sntprintf( text_out, length, _T( "%d" ), 
				rule_input->desc.socket.src_port.port.port_end ); 			
			break; 
		case 5:
			ipv4_addr = rule_input->desc.socket.dest_ip.ip.ip_begin; 
			bytes = ( PBYTE )&ipv4_addr; 

			_sntprintf( text_out, length, _T( "%d.%d.%d.%d" ), 
				bytes[ 3 ], 
				bytes[ 2 ], 
				bytes[ 1 ], 
				bytes[ 0 ] ); 
			break; 
		case 6:
			ipv4_addr = rule_input->desc.socket.dest_ip.ip.ip_end; 
			bytes = ( PBYTE )&ipv4_addr; 

			_sntprintf( text_out, length, _T( "%d.%d.%d.%d" ), 
				bytes[ 3 ], 
				bytes[ 2 ], 
				bytes[ 1 ], 
				bytes[ 0 ] ); 
			break; 
		case 7:
			_sntprintf( text_out, length, _T( "%d" ), 
				rule_input->desc.socket.dest_port.port.port_begin ); 			
			break; 
		case 8:
			_sntprintf( text_out, length, _T( "%d" ), 
				rule_input->desc.socket.dest_port.port.port_end ); 			
			break; 
		case 9:
			get_prot_type_desc( text_out, length, rule_input->desc.socket.src_port.port.type ); 
			break; 
		case 10:
			get_action_type_desc( text_out, length, rule_input->resp ); 
			break; 
		case 11:
			_tcsncpy( text_out, desc, length ); 
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
	}
	else if( is_name_rule_type( rule_input->type ) == TRUE )
	{
		switch( index )
		{
		case 1:
			ret = get_rule_param_from_index( rule_input->type, &rule_input->desc, &param_desc, 1 ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			_tcsncpy( text_out, param_desc->common.name, length ); 
			break; 
		case 2:
			get_action_type_desc( text_out, length, rule_input->resp ); 
			break; 
		case 3:
			_tcsncpy( text_out, desc, length ); 
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
	}
	else
	{
		ASSERT( FALSE ); 
	}
	
	if( text_out[ length - 1 ] != L'\0' )
	{
		text_out[ length - 1 ] != L'\0'; 
	}

_return:
	return ret; 
}

typedef struct _fw_settings
{
	CRITICAL_SECTION locker; 
	vector< access_rule_desc* > url_rules; 
	vector< access_rule_desc* > ip_rules; 

	_fw_settings()
	{
		InitializeCriticalSection( &locker ); 
	}

	~_fw_settings()
	{
		INT32 i; 
		access_rule_desc *define; 

		for( i = 0; i < ip_rules.size(); i ++ )
		{
			define = ip_rules.back(); 
			ip_rules.pop_back(); 
			delete define; 
		}

		for( i = 0; i < url_rules.size(); i ++ )
		{
			define = url_rules.back(); 
			url_rules.pop_back(); 
			delete define; 
		}

		DeleteCriticalSection( &locker ); 
	}

	VOID lock()
	{
		EnterCriticalSection( &locker ); 
	}

	VOID unlock()
	{
		LeaveCriticalSection( &locker ); 
	}

} fw_settings, *pfw_settings; 

//INT32 CALLBACK alloc_url_rule( PVOID *rule, PVOID param )
//{
//	INT32 ret = 0; 
//	url_param_record *url; 
//	fw_settings *settings; 
//	access_rule_desc *rule_define; 
//
//	ASSERT( rule != NULL ); 
//	ASSERT( param != NULL ); 
//
//	settings = ( fw_settings* )param; 
//	rule_define = new access_rule_desc(); 
//
//	if( rule_define == NULL )
//	{
//		ret = -ENOMEM; 
//		goto _return; 
//	}
//
//	settings->lock(); 
//	settings->url_rules.push_back( rule_define ); 
//	settings->unlock(); 
//
//	*rule = ( PVOID )rule_define; 
//_return:
//	return ret; 
//}

LRESULT CALLBACK alloc_fw_rule( PVOID *rule, PVOID param )
{
	INT32 ret = 0; 
	url_param_record *url; 
	fw_settings *settings; 
	access_rule_desc *rule_desc; 

	ASSERT( rule != NULL ); 
	ASSERT( param != NULL ); 

	settings = ( fw_settings* )param; 
	rule_desc = new access_rule_desc(); 

	if( rule_desc == NULL )
	{
		ret = -ENOMEM; 
		goto _return; 
	}

	settings->lock(); 
	settings->ip_rules.push_back( rule_desc ); 
	settings->unlock(); 

	*rule = ( PVOID )rule_desc; 
_return:
	return ret; 
}

LRESULT CALLBACK load_fw_rule( access_rule_type type, action_rule_record *record, PVOID param )
{
	return 0; 
}

typedef struct _defense_settings
{
	CRITICAL_SECTION locker; 
	vector< access_rule_desc* > app_rules; 

	_defense_settings()
	{
		InitializeCriticalSection( &locker ); 
	}

	~_defense_settings()
	{
		INT32 i; 
		access_rule_desc *define; 

		for( i = 0; i < app_rules.size(); i ++ )
		{
			define = app_rules.back(); 
			app_rules.pop_back(); 
			delete define; 
		}

		DeleteCriticalSection( &locker ); 
	}

	VOID lock()
	{
		EnterCriticalSection( &locker ); 
	}

	VOID unlock()
	{
		LeaveCriticalSection( &locker ); 
	}

} defense_settings, *pdefense_settings; 

LRESULT CALLBACK alloc_defense_rule( PVOID *rule, PVOID param ); 

LRESULT CALLBACK load_defense_rule( access_rule_type type, action_rule_record *record, PVOID param ); 

LRESULT CALLBACK alloc_defense_rule( PVOID *rule, PVOID param )
{
	INT32 ret = 0; 
	//url_param_record *url; 
	defense_settings *settings; 
	access_rule_desc *rule_desc; 

	ASSERT( rule != NULL ); 
	ASSERT( param != NULL ); 

	settings = ( defense_settings* )param; 
	rule_desc = new access_rule_desc(); 

	if( rule_desc == NULL )
	{
		ret = -ENOMEM; 
		goto _return; 
	}

	settings->lock(); 
	settings->app_rules.push_back( rule_desc ); 
	settings->unlock(); 

	*rule = ( PVOID )rule_desc; 
_return:
	return ret; 
}

LRESULT CALLBACK load_defense_rule( access_rule_type type, action_rule_record *record, PVOID param )
{
	return 0; 
}

LRESULT config_rule( access_rule_desc *action_rule, access_rule_desc* org_action_rule, ULONG mode, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ret = check_access_rule_input_valid( action_rule, FALSE ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	if( mode == ADD_RULE_DEFINE )
	{
		//if( action_rule->type == URL_RULE_TYPE )
		//{
		//	ret = config_url_rule( action_rule, TRUE ); 
		//	if( ret != ERROR_SUCCESS )
		//	{
		//		goto _return; 
		//	}
		//}
		//else
		//{
		ASSERT( is_valid_common_rule_flag( flags ) == TRUE ); 

			ret = _config_rule( action_rule, flags ); 
		//}
	}
	else if( mode == DEL_RULE_DEFINE )
	{
		//if( action_rule->type == URL_RULE_TYPE )
		//{
		//	ret = config_url_rule( action_rule, FALSE ); 
		//	if( ret != ERROR_SUCCESS )
		//	{
		//		goto _return; 
		//	}
		//}
		//else
		//{
		ASSERT( is_valid_common_rule_flag( flags ) == TRUE ); 
			ret = _del_rule( action_rule, 0 ); 
		//}
	}
	else if( mode == MODIFY_RULE_DEFINE )
	{
		ret = check_access_rule_input_valid( org_action_rule, FALSE ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		ret = check_access_rule_input_valid( action_rule, FALSE ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}


		//if( action_rule->type == URL_RULE_TYPE )
		//{
		//	ret = config_url_rule( org_action_rule, FALSE ); 
		//	if( ret != ERROR_SUCCESS )
		//	{
		//		goto _return; 
		//	}

		//	ret = config_url_rule( action_rule, TRUE ); 
		//	if( ret != ERROR_SUCCESS )
		//	{
		//		goto _return; 
		//	}
		//}
		//else
		{

			ASSERT( flags == APP_ACTION_RULE ); 

			access_rule_modify_info modify_info; 
			modify_info.dest_rule = *org_action_rule; 
			modify_info.rule_setting = *action_rule; 

			ret = _modify_rule( &modify_info ); 
		}
	}
	//else if( action_rule->type == SOCKET_RULE_TYPE )
	//{

	//}
	//else if( action_rule->type == REG_RULE_TYPE )
	//{

	//}
	//else if( action_rule->type == COM_RULE_TYPE )
	//{

	//}
	//else if( action_rule->type == COMMON_RULE_TYPE )
	//{

	//}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

#include "dns_flush.h"

LRESULT add_rule_action( HWND wnd, access_rule_type cur_rule_type )
{
	LRESULT ret = ERROR_SUCCESS; 
	access_rule_queue *target_rules; 
	action_rule_buffered *rule_alloc; 
	LPCTSTR tmp_text; 
	//ASSERT( action_rule != NULL ); 

	ASSERT( is_valid_access_rule_type( cur_rule_type ) == TRUE );  

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	target_rules = &all_rules[ cur_rule_type ]; 
	
	ret = alloc_access_rule_buffer( &all_rules_buf[ cur_rule_type ], &rule_alloc ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	init_access_rule( cur_rule_type, &rule_alloc->rule ); 
	
	rule_alloc->flags = 0; 
	rule_alloc->desc[ 0 ] = L'\0'; 

	if( is_name_rule_type( cur_rule_type ) == TRUE )
	{
		name_rule_edit_dlg rule_edit( &rule_alloc->rule, cur_rule_type ); 
		rule_edit.Create( wnd, _T(""), UI_WNDSTYLE_FRAME, 0L, 0, 0, 600, 400);
		rule_edit.CenterWindow();
		rule_edit.SetIcon( IDI_MAIN_ICON ); 
		rule_edit.ShowModal(); 
		
		if( rule_edit.create_rule_success() == FALSE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
	}
	else if( is_socket_rule_type( cur_rule_type ) == TRUE )
	{
		ip_rule_edit_dlg rule_edit( &rule_alloc->rule, cur_rule_type ); 
		rule_edit.Create( wnd, _T(""), UI_WNDSTYLE_FRAME, 0L, 0, 0, 600, 400);
		rule_edit.CenterWindow();
		rule_edit.SetIcon( IDI_MAIN_ICON ); 
		rule_edit.ShowModal(); 

		if( rule_edit.create_rule_success() == FALSE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
	}

	ASSERT( check_access_rule_input_valid( &rule_alloc->rule, FALSE ) == ERROR_SUCCESS ); 

	//!!use the rule flags to descript the super rule type.
	ret = input_rule_to_config( &rule_alloc->rule, 0 ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "add the action rule failed \n") ); 
		target_rules->resize( target_rules->size() - 1 ); 
		dump_action_rule( &rule_alloc->rule ); 
	}

	ret = config_rule( &rule_alloc->rule, NULL, ADD_RULE_DEFINE, APP_ACTION_RULE ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = notify_rule_modify( rule_alloc, CMD_ADD_RULE ); 

	if( cur_rule_type == URL_RULE_TYPE )
	{
		flush_dns(); 
	}
	
	target_rules->push_back( rule_alloc ); 

	tmp_text = _get_string_by_id( TEXT_ACL_MANAGEMENT_ADD_SUCCESSULLY_TIP, 
		_T( "添加规则定义成功" ) ); 

	show_msg( wnd, tmp_text ); 

_return:
	if( ret != ERROR_SUCCESS )
	{
		ret = free_rule_buf( rule_alloc ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
		}
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT modify_rule_action( HWND wnd, ULONG sel_index, access_rule_desc *action_rule, access_rule_type cur_rule_type )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR tmp_text; 

	access_rule_queue *target_rules; 
	access_rule_desc *org_action_rule; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( sel_index >= 0 ); 

	ASSERT( is_valid_access_rule_type( cur_rule_type ) ); 

	//init_rule_input( action_rule ); 
	//action_rule->type = INVALID_RULE_TYPE; 

	target_rules = &all_rules[ cur_rule_type ]; 

	log_trace( ( MSG_INFO, "target rule flags is 0x%0.8x \n", target_rules->operator []( sel_index )->flags ) ); 

	if( is_super_rule( target_rules->operator []( sel_index )->flags ) == TRUE )
	{
		ret = ERROR_DEL_SUPER_RULE; 
		goto _return; 
	}

	*action_rule = target_rules->operator []( sel_index )->rule; 
	org_action_rule = &target_rules->operator []( sel_index )->rule; 

	init_access_desc_param_ptr( action_rule->type, &action_rule->desc ); 

	ASSERT( check_access_rule_input_valid( action_rule, FALSE ) == ERROR_SUCCESS ); 

	if( is_name_rule_type( cur_rule_type ) == TRUE )
	{
		name_rule_edit_dlg rule_edit( action_rule, cur_rule_type, TRUE ); 
		rule_edit.Create( wnd, _T(""), UI_WNDSTYLE_FRAME, 0L, 0, 0, 600, 400);
		rule_edit.CenterWindow();
		rule_edit.SetIcon( IDI_MAIN_ICON ); 
		rule_edit.ShowModal(); 
		if( rule_edit.create_rule_success() == FALSE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
	}
	else if( is_socket_rule_type( cur_rule_type ) == TRUE )
	{
		ip_rule_edit_dlg rule_edit( action_rule, cur_rule_type, TRUE ); 
		rule_edit.Create( wnd, _T(""), UI_WNDSTYLE_FRAME, 0L, 0, 0, 600, 400 );
		rule_edit.CenterWindow();
		rule_edit.SetIcon( IDI_MAIN_ICON ); 
		rule_edit.ShowModal(); 
		if( rule_edit.create_rule_success() == FALSE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
	}

	if( action_rule->type != cur_rule_type )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	init_access_desc_param_ptr( action_rule->type, &action_rule->desc ); 
	ASSERT( check_access_rule_input_valid( action_rule, FALSE ) == ERROR_SUCCESS ); 

	ret = del_rule_from_config( org_action_rule ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		log_trace( ( MSG_ERROR, "delete the action rule failed\n" ) ); 
		dump_action_rule( action_rule ); 
		goto _return; 
	}

	ret = input_rule_to_config( action_rule, 0 ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		log_trace( ( MSG_ERROR, "add the action rule failed \n" ) ); 
		goto _return; 
	}

#ifdef DELETE_RULE_NOW
	ret = config_rule( action_rule, org_action_rule, MODIFY_RULE_DEFINE, APP_ACTION_RULE ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}
#endif //DELETE_RULE_NOW

	target_rules->operator []( sel_index )->rule = *action_rule; 

	tmp_text = _get_string_by_id( TEXT_ACL_MANAGEMENT_EDIT_SUCCESSULLY_TIP, 
		_T( "编辑规则定义成功" ) ); 

	show_msg( wnd, tmp_text ); 

_return:

	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT free_rule_buf( action_rule_buffered *rule_buf )
{
	LRESULT ret = ERROR_SUCCESS; 
	safe_region_list *region_list; 

	ASSERT( rule_buf != NULL ); 
	ASSERT( TRUE == is_valid_access_rule_type( rule_buf->rule.type ) ); 

	region_list = &all_rules_buf[ rule_buf->rule.type ]; 

	ret = free_region_unit( region_list, &rule_buf->entry ); 

_return:
	return ret; 
}

NTSTATUS delete_rule_action( INT32 sel_index, access_rule_type cur_rule_type, ULONG *del_state )
{
	LRESULT ret = ERROR_SUCCESS; 
	//access_rule_desc *rule; 
	access_rule_queue *target_rules; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	target_rules = &all_rules[ cur_rule_type ]; 

	log_trace( ( MSG_INFO, "target rule flags is 0x%0.8x \n", target_rules->operator []( sel_index )->flags ) ); 
	if( del_state != NULL )
	{
		*del_state = 0; 
	}

	if( is_super_rule( target_rules->operator []( sel_index )->flags ) == TRUE )
	{
		ret = ERROR_DEL_SUPER_RULE; 
		goto _return; 
	}

	ret = del_rule_from_config( &target_rules->operator []( sel_index )->rule ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!delete rule from database failed\n" ) ); 
		goto _return; 
	}

	if( del_state != NULL )
	{
		*del_state |= DELETED_FROM_DB; 
	}

#ifdef DELETE_RULE_NOW
	ret = config_rule( &target_rules->operator []( sel_index )->rule, NULL, DEL_RULE_DEFINE, APP_ACTION_RULE ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "remove the config from db failed \n" ) );  
	}
#endif //DELETE_RULE_NOW

	if( del_state != NULL )
	{
		*del_state |= DELETED_FROM_ACL; 
	}

	ret = notify_rule_modify( target_rules->operator []( sel_index ), CMD_DEL_RULE ); 
	
	ret = free_rule_buf( target_rules->operator []( sel_index ) ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
	}

	target_rules->erase( target_rules->begin() + sel_index ); 

_return:
	log_trace( ( MSG_INFO, "leave %s 0z%0.8x\n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT load_rule_buf( OFFSET_LIST_ENTRY *entry )
{
	LRESULT ret = ERROR_SUCCESS; 
	action_rule_buffered *rule_buf; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	rule_buf = ( action_rule_buffered* )entry; 

	if( is_valid_access_rule_type( rule_buf->rule.type ) != TRUE )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	all_rules[ rule_buf->rule.type ].push_back( rule_buf ); 

_return:
	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__, ret ) ); 

	return ret; 
}

#pragma function( memcpy, strcpy )

LRESULT manage_rule( access_rule_type type, ULONG_PTR offset, BOOLEAN is_del )
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 
	//OFFSET_LIST_ENTRY entry; 
	action_rule_buffered *rule_buf; 
	safe_region_list *region_list; 

	//DBG_BP(); 
	ASSERT( TRUE == is_valid_access_rule_type( type ) ); 

	region_list = &all_rules_buf[ type ]; 

	if( offset > safe_region_list_size( region_list ) )
	{
		ASSERT( FALSE && "how to input the invalid offset in the memory region" ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	rule_buf = ( action_rule_buffered* )( OFFSET_LIST_ENTRY* )( ( PBYTE )safe_region_list_base( region_list ) + offset ); 

	//because have not still initialize the action rule some time.
	init_access_desc_param_ptr( rule_buf->rule.type, &rule_buf->rule.desc ); 

	ret = check_access_rule_input_valid( &rule_buf->rule, FALSE ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}
	
	if( TRUE != is_valid_access_rule_type( rule_buf->rule.type ) )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	//ntstatus = find_allocated_unit_lock_free( &all_rules_buf[ rule_buf->rule.type ], &rule_buf->entry ); 
	//if( !NT_SUCCESS( ntstatus ) )
	//{
	//	ret = ERROR_INVALID_PARAMETER; 
	//	goto _return; 
	//}

	if( is_del == TRUE )
	{
		log_trace( ( MSG_INFO, "deleted rule is %d\n", offset ) ); 
		//free_region_unit( region_list, &rule_buf->entry ); 
	}
	else
	{
		log_trace( ( MSG_INFO, "added rule is %d\n", offset ) ); 
	}

_return:
	return ret; 
}

