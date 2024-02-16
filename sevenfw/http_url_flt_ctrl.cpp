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
 
#include "common_func.h"
#include "ui_ctrl.h"
#include "seven_fw_api.h"
#include "seven_fw_ioctl.h"
#include "inst_im.h"
#include "inst_drv.h"
#include "drv_load.h"
#include "net_svc_inst.h"
#include "http_url_flt_ctrl.h"

#define ADD_TEXT 1
#define DEL_TEXT 0

#define DEBUG_WITH_DEV 1

#define HTTP_URL_FLT_FILE_NAME "http_url_fw.sys"
#define HTTP_URL_FLT_SVC_NAME "HttpUrlFlt"

HANDLE http_url_flt_dev = NULL; 

#pragma comment( lib, "comctl32.lib" )
//#pragma comment( lib, "ws2_32.lib" )
//#pragma comment( lib, "NdisHook.lib" )
#define SEVEN_FW_SYS_FILE_NAME "sevenfw.sys"
#define SEVEN_FW_INF_FILE_NAME "sevenfw.inf"
#define SEVEN_FW_M_INF_FILE_NAME "sevenfw_m.inf"

CInstallPassthru Installer;
HANDLE seven_fw_dev = NULL;

INT32 uninstall_7fw()
{
	if( NULL != seven_fw_dev )
	{
		CloseHandle( seven_fw_dev ); 
		seven_fw_dev = NULL; 
	}

	Installer.UninstallPassthru();
	DelSysFile( _T( "sevenfw.sys" ) );
	return 0;
}

INT32 install_7fw()
{
	INT32 ret; 
	if( FALSE == file_exists( SEVEN_FW_SYS_FILE_NAME, TRUE ) )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVEN_FW_FILE ), "DAT_FILE", SEVEN_FW_SYS_FILE_NAME, TRUE ); 
		if( ret < 0 )
		{
			dbg_print( 0, DBG_DBG_BUF_OUT, "create the the seven fw file from resource failed \n" ); 
			goto _err_return; 
		}
	}

	if( FALSE == file_exists( SEVEN_FW_INF_FILE_NAME, TRUE ) )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVEN_FW_INF ), "DAT_FILE", SEVEN_FW_INF_FILE_NAME, TRUE ); 
		if( ret < 0 )
		{
			dbg_print( 0, DBG_DBG_BUF_OUT, "create the the seven fw inf file from resource failed \n" ); 
			goto _err_return; 
		}
	}

	if( FALSE == file_exists( SEVEN_FW_M_INF_FILE_NAME, TRUE ) )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVEN_FW_M_INF ), "DAT_FILE", SEVEN_FW_M_INF_FILE_NAME, TRUE ); 
		if( ret < 0 )
		{
			dbg_print( 0, DBG_DBG_BUF_OUT, "create the the seven fw m inf file from resource failed \n" ); 
			goto _err_return; 
		}
	}

	//CopySysFile( _T( SEVEN_FW_SYS_FILE_NAME ) );
	Installer.InstallPassthru();

_err_return:
	return 0;
}

INT32 load_seven_fw_dev(); 

INT32 install_http_url_flt_drv()
{
	INT32 ret; 

	dbg_print( 0, DBG_DBG_BUF_OUT, "enter %s \n", __FUNCTION__ ); 

	if( http_url_flt_dev != NULL )
	{
		return 0; 
	}

	if( FALSE == file_exists( HTTP_URL_FLT_FILE_NAME, TRUE ) )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_HTTP_URL_FW_FILE ), "DAT_FILE", HTTP_URL_FLT_FILE_NAME, TRUE ); 
		if( ret < 0 )
		{
			dbg_print( 0, DBG_DBG_BUF_OUT, "create the filter sys file from resource failed \n" ); 
			goto _err_return; 
		}
	}

	dbg_print( 0, DBG_DBG_BUF_OUT, "create the filter sys file from resource successfully \n" ); 

	http_url_flt_dev = load_drv_dev( HTTP_URL_FLT_DEV_FILE_NAME, 
		HTTP_URL_FLT_FILE_NAME, 
		HTTP_URL_FLT_SVC_NAME, 
		DRIVER_IN_SYSTEM_DIR ); 

	if( NULL == http_url_flt_dev )
	{
		ret = -1; 
		unload_drv_dev( HTTP_URL_FLT_DEV_FILE_NAME, 
		HTTP_URL_FLT_FILE_NAME, 
		HTTP_URL_FLT_SVC_NAME, 
		DRIVER_IN_SYSTEM_DIR ); 

		error_handle( "Initializing error, will exit.\n", MODE_DEBUG_OUTPUT, 0 ); 
		goto _err_return;  
	}

	dbg_print( 0, DBG_DBG_BUF_OUT, "load the http url filter driver successfully\n" ); 

	ret = 0; 
	dbg_print( 0, DBG_DBG_BUF_OUT, "leave %s filter device 0x%0.8x\n", __FUNCTION__, http_url_flt_dev ); 
	return ret; 

_err_return:

	if( NULL != http_url_flt_dev )
	{
		CloseHandle( http_url_flt_dev ); 
		http_url_flt_dev = NULL; 
	}

	dbg_print( 0, DBG_DBG_BUF_OUT, "leave %s \n", __FUNCTION__ ); 

	return ret; 
}

INT32 uninstall_http_url_flt_drv()
{
	if( NULL != http_url_flt_dev )
	{
		CloseHandle( http_url_flt_dev ); 
		http_url_flt_dev = NULL; 
	}

	unload_drv_dev( HTTP_URL_FLT_DEV_FILE_NAME, 
		HTTP_URL_FLT_FILE_NAME, 
		HTTP_URL_FLT_SVC_NAME, 
		0 );  

	return 0; 
}

INT32 unload_http_url_flt_drv()
{
	INT32 ret; 

	ret = uninstall_http_url_flt_drv(); 
	if( ret < 0 )
	{
		ASSERT( FALSE ); 
		return ret; 
	}

	ret = uninstall_7fw(); 
	if( ret < 0 )
	{
		ASSERT( FALSE ); 
		return ret; 
	}

	ret = 0; 
	return ret; 
}

INT32 init_http_url_flt()
{
	INT32 ret; 
	ret = load_seven_fw_dev(); 
	if( ret < 0 )
	{
		dbg_print( 0, DBG_DBG_BUF_OUT, "load_seven_fw_dev failed \n" ); 
		goto err_return; 
	}

	ret = install_http_url_flt_drv();
	if( ret < 0 )
	{
		dbg_print( 0, DBG_DBG_BUF_OUT, "load_http_txt_flt_drv failed \n" );
	}

err_return:
	return ret; 
}

#define ALL_FILTER_NAME_BUFF_LEN ( 1024 * 100 ) 
INT32 get_all_seted_filter_urls( PVOID param )
{
	INT32 ret; 
	ULONG ret_length; 
	PFILTER_NAMES_OUTPUT filter_text = NULL; 
	PUI_CTRL ui_ctrl; 

	ASSERT( NULL != seven_fw_dev ); 
	
	ui_ctrl = ( PUI_CTRL )param; 

	filter_text = ( PFILTER_NAMES_OUTPUT )malloc( ALL_FILTER_NAME_BUFF_LEN ); 
	if( NULL == filter_text )
	{
		ret = -1; 
		goto _return; 
	}

#ifdef DEBUG_WITH_DEV
	ret = DeviceIoControl( seven_fw_dev, 
		IOCTL_SEVEN_FW_GET_HTTP_FILTER_URLS, 
		NULL, 
		0, 
		filter_text, 
		ALL_FILTER_NAME_BUFF_LEN, 
		&ret_length, 
		NULL );
#else
	ret = TRUE; 
#endif 

	//MessageBox( NULL, "get the seted text \n", NULL, 0 ); 

	if( ret == TRUE )
	{
		//{
		//	CHAR output[ 512 ]; 
		//	sprintf( output, "get the seted text length is %d \n", ret_length ); 

		//	MessageBox( NULL, output, NULL, 0 ); 
		//}

		ret = ui_ctrl->ui_output_func( HTTP_URL_FLT_OUTPUT, ( INT32 )0, ( PBYTE )filter_text, ret_length, ui_ctrl->param ); 
	}
	else
	{
		ret = -1; 
	}

_return:
	if( filter_text != NULL )
	{
		free( filter_text ); 
	}

	return ret; 
}

INT32 uninit_http_url_flt()
{
	DWORD dwRet;
	if( NULL != seven_fw_dev )
	{
		CloseHandle( seven_fw_dev ); 
		seven_fw_dev = NULL; 
	}

	//uninstall_http_url_flt_drv(); 
	//if( hInstMutex != NULL )
	//{
	//	CloseHandle( hInstMutex ); 
	//	hInstMutex = NULL; 
	//}

	return 0; 
	//WSACleanup();
}

INT32 del_flt_url( PFILTER_URL_INPUT filter_text, ULONG length )
{
	INT32 ret; 
	ULONG ret_length; 

	ASSERT( length >= sizeof( FILTER_URL_INPUT ) + filter_text->length ); 

#ifdef DEBUG_WITH_DEV
	ret = DeviceIoControl( seven_fw_dev, 
		IOCTL_SEVEN_FW_UNSET_HTTP_FILTER_URL, 
		filter_text, 
		sizeof( FILTER_URL_INPUT ) + filter_text->length, 
		NULL, 
		0, 
		&ret_length, 
		NULL );

	if( ret == FALSE )
	{
		ret = -1; 
	}
	else
	{
		ret = 0; 
	}

#else
	ret = 0; 
#endif

	return ret; 
}

INT32 add_flt_url( PFILTER_URL_INPUT filter_text, ULONG length )
{
	INT32 ret; 
	ULONG ret_length; 
	ULONG add_ret; 

	ASSERT( length >= sizeof( FILTER_URL_INPUT ) + filter_text->length ); 

#ifdef DEBUG_WITH_DEV
	ret = DeviceIoControl( seven_fw_dev, 
		IOCTL_SEVEN_FW_SET_HTTP_FILTER_URL, 
		filter_text, 
		sizeof( FILTER_URL_INPUT ) + filter_text->length, 
		&add_ret, 
		sizeof( ULONG ), 
		&ret_length, 
		NULL ); 

	if( ret == FALSE )
	{
		ret = -1; 
	}
	else
	{
		if( ret_length == sizeof( ULONG ) && add_ret == ADDED_NEW_FILTER )
		{
			ret = ADDED_NEW_FILTER; 
		}
		else
		{
			ret = 0; 
		}
	}
	

#else
	ret = 0; 
#endif

	return ret; 
}

INT32 do_http_url_flt_ui_work( ULONG ui_action_id, PBYTE input_buf, ULONG input_length, PBYTE output_buf, ULONG *output_length, PVOID param )
{
	INT32 ret; 

	ret = 0; 
	switch( ui_action_id )
	{
	case HTTP_URL_FLT_DEL_URL:
		ret = del_flt_url( ( PFILTER_URL_INPUT )input_buf, input_length ); 
		break; 

	case HTTP_URL_FLT_ADD_URL:
		ret = add_flt_url( ( PFILTER_URL_INPUT )input_buf, input_length ); 
		break; 

	case HTTP_URL_FLT_INSTALL:
		ret = install_http_url_flt_drv(); 
		break;

	case HTTP_URL_FLT_UNINSTALL:
		ret = uninstall_http_url_flt_drv(); 
		break; 
	case HTTP_URL_FLT_INIT:
		ret = init_http_url_flt(); 
		break; 
	case HTTP_URL_FLT_UNINIT:
		ret = uninit_http_url_flt(); 
		break; 
	case HTTP_URL_FLT_INSTALL_7FW:
		ret = install_7fw(); 
		break; 
	case HTTP_URL_FLT_UNINSTALL_7FW:
		ret = uninstall_7fw(); 
		break; 

	case HTTP_URL_FLT_GET_ADDED_FLT_URL:
		ret = get_all_seted_filter_urls( param );
		break; 

	default:
		break; 
	}

	return ret; 
}

