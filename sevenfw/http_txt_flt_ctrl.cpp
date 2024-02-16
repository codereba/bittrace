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
#include "http_txt_flt_ctrl.h"
#include "seven_fw_api.h"
#include "seven_fw_ioctl.h"
#include "inst_im.h"
#include "net_svc_inst.h"
#include "inst_drv.h"
#include "drv_load.h"

ULONG text_type_from_desc( CHAR *desc ); 
CHAR *get_text_type_desc( ULONG type ); 

#define HTTP_TXT_FLT_FILE_NAME "http_txt_fw.sys"
#define HTTP_TXT_FLT_SVC_NAME "HttpTxtFlt"

extern HANDLE seven_fw_dev; 
HANDLE http_txt_flt_dev = NULL; 

INT32 load_http_txt_flt_drv()
{
	INT32 ret; 

	::MessageBox( NULL, "enter install_http_txt_flt_drv\n", NULL, 0 ); 

	//__asm int 3; 
	ret = 0; 
	if( http_txt_flt_dev != NULL )
	{
		return ret; 
	}

	dbg_print( 0, DBG_DBG_BUF_OUT, "check http txt filter file existing \n" ); 
	if( FALSE == file_exists( HTTP_TXT_FLT_FILE_NAME, FALSE ) )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_HTTP_TXT_FW_FILE ), "DAT_FILE", HTTP_TXT_FLT_FILE_NAME, FALSE ); 
		if( ret < 0 )
		{
			dbg_print( 0, DBG_DBG_BUF_OUT, "load the http txt filter file from resource failed \n" ); 
			return ret; 
		}
	}

	dbg_print( 0, DBG_DBG_BUF_OUT, "loading the http txt filter driver\n" ); 

	http_txt_flt_dev = load_drv_dev( HTTP_TXT_FLT_DEV_FILE_NAME, 
		HTTP_TXT_FLT_FILE_NAME, 
		HTTP_TXT_FLT_SVC_NAME, 
		0 ); 

	dbg_print( 0, DBG_DBG_BUF_OUT, "loaded http txt device 0x%0.8x \n", http_txt_flt_dev ); 

	if( NULL == http_txt_flt_dev )
	{
		ret = -1; 
		unload_drv_dev( HTTP_TXT_FLT_DEV_FILE_NAME, 
		HTTP_TXT_FLT_FILE_NAME, 
		HTTP_TXT_FLT_SVC_NAME, 
		0 ); 
		error_handle( "Initializing error, will exit.\n", MODE_DEBUG_OUTPUT, 0 ); 
		goto _err_return;  
	}

	dbg_print( 0, DBG_DBG_BUF_OUT, "load the http txt filter driver successfully\n" ); 
	ret = 0; 
	return ret; 

_err_return:

	if( NULL != http_txt_flt_dev )
	{
		CloseHandle( http_txt_flt_dev ); 
		http_txt_flt_dev = NULL; 
	}

	return ret; 
}

INT32 unload_http_txt_flt_drv()
{
	INT32 ret; 

	if( NULL != http_txt_flt_dev )
	{
		CloseHandle( http_txt_flt_dev ); 
		http_txt_flt_dev = NULL; 
	}

	unload_drv_dev( HTTP_TXT_FLT_DEV_FILE_NAME, 
		HTTP_TXT_FLT_FILE_NAME, 
		HTTP_TXT_FLT_SVC_NAME, 
		0 );  

	ret = 0; 
	return ret; 
}

INT32 load_seven_fw_dev()
{
	INT32 ret; 

	ret = 0; 
	if( seven_fw_dev != NULL )
	{
		goto _return; 
	}

	seven_fw_dev = CreateFile( "\\\\.\\SevenFW", 
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if( INVALID_HANDLE_VALUE == seven_fw_dev )
	{
		if( IDYES == MessageBox( NULL, "7�����ǽ��û�а�װ,�Ƿ����ڰ�װ?", "SevenFw", MB_YESNO ) )
		{
			//uninstall_7fw(); 
			install_7fw();  
		}
		else
		{ 
			return -1; 
		}

		seven_fw_dev = CreateFile( "\\\\.\\SevenFW", 
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);

		if( INVALID_HANDLE_VALUE == seven_fw_dev )
		{
			CHAR output[ 512 ]; 
			sprintf( output, "����7�����ǽʧ��,������֮������\n" ); 
			MessageBox( NULL, output, NULL, 0 ); 

			seven_fw_dev = NULL; 

			return -1; 
		}
	}

_return:
	return ret; 
}

INT32 init_http_txt_flt()
{
	INT32 ret; 
	ret = load_seven_fw_dev(); 
	if( ret < 0 )
	{
		dbg_print( 0, DBG_DBG_BUF_OUT, "load_seven_fw_dev failed \n" ); 
		goto err_return; 
	}

	ret = load_http_txt_flt_drv();
	if( ret < 0 )
	{
		dbg_print( 0, DBG_DBG_BUF_OUT, "load_http_txt_flt_drv failed \n" ); 
	}
err_return:

	return ret; 
}

#define ALL_FILTER_NAME_BUFF_LEN ( 1024 * 100 ) 
INT32 get_all_seted_filter_txts( PVOID param )
{
	INT32 ret; 
	ULONG ret_length; 
	PFILTER_NAMES_OUTPUT filter_text; 
	PUI_CTRL ui_ctrl; 

	ASSERT( NULL != seven_fw_dev ); 
	
	ui_ctrl = ( PUI_CTRL )param; 

	filter_text = ( PFILTER_NAMES_OUTPUT )malloc( ALL_FILTER_NAME_BUFF_LEN ); 
	if( NULL == filter_text )
	{
		return -1; 
	}

#ifdef DEBUG_WITH_DEV
	ret = DeviceIoControl( seven_fw_dev, 
		IOCTL_SEVEN_FW_GET_HTTP_FILTER_NAMES, 
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

		ui_ctrl->ui_output_func( HTTP_TXT_FLT_OUTPUT, ( INT32 )0, ( PBYTE )filter_text, ret_length, ui_ctrl->param ); 
	}

	free( filter_text ); 
	return ret; 
}

INT32 uninit_http_txt_flt()
{
	//INT32 ret; 
	//DWORD dwRet;
	if( NULL != seven_fw_dev )
	{
		CloseHandle( seven_fw_dev ); 
		seven_fw_dev = NULL; 
	}
	if( NULL != http_txt_flt_dev )
	{
		CloseHandle( http_txt_flt_dev ); 
		http_txt_flt_dev = NULL; 
	}

	//ret = 0; 
	//unload_http_txt_flt_drv(); 
	//if( hInstMutex != NULL )
	//{
	//	CloseHandle( hInstMutex ); 
	//	hInstMutex = NULL; 
	//}

	return 0; 
	//WSACleanup();
}

INT32 del_flt_txt( PFILTER_NAME_INPUT filter_text, ULONG length )
{
	INT32 ret; 
	ULONG ret_length; 

	ASSERT( length >= sizeof( FILTER_NAME_INPUT ) + filter_text->length ); 

#ifdef DEBUG_WITH_DEV
	ret = DeviceIoControl( seven_fw_dev, 
		IOCTL_SEVEN_FW_UNSET_HTTP_FILTER_NAME, 
		filter_text, 
		sizeof( FILTER_NAME_INPUT ) + filter_text->length, 
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

INT32 add_flt_txt( PFILTER_NAME_INPUT filter_text, ULONG length )
{
	INT32 ret; 
	ULONG ret_length; 
	ULONG add_ret; 

	ASSERT( length >= sizeof( FILTER_NAME_INPUT ) + filter_text->length ); 

#ifdef DEBUG_WITH_DEV
	ret = DeviceIoControl( seven_fw_dev, 
		IOCTL_SEVEN_FW_SET_HTTP_FILTER_NAME, 
		filter_text, 
		sizeof( FILTER_NAME_INPUT ) + filter_text->length, 
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

INT32 uninstall_http_txt_flt_drv()
{
	INT32 ret; 
	ret = uninstall_7fw(); 
	if( ret < 0 )
	{
		ASSERT( FALSE ); 
		return ret; 
	}
}

INT32 install_http_txt_flt_drv()
{
	INT32 ret; 
	ret = install_7fw(); 
	if( ret < 0 )
	{
		ret = -1; 
		goto _err_return; 
	}

_err_return:
	return ret; 
}

INT32 do_http_txt_flt_ui_work( ULONG ui_action_id, PBYTE input_buf, ULONG input_length, PBYTE output_buf, ULONG *output_length, PVOID param )
{
	INT32 ret; 

	ret = 0; 
	switch( ui_action_id )
	{
	case HTTP_TXT_FLT_DEL_URL:
		ret = del_flt_txt( ( PFILTER_NAME_INPUT )input_buf, input_length ); 
		break; 

	case HTTP_TXT_FLT_ADD_URL:
		ret = add_flt_txt( ( PFILTER_NAME_INPUT )input_buf, input_length );
		break; 

	case HTTP_TXT_FLT_INSTALL:
		ret = install_http_txt_flt_drv(); 
		break;

	case HTTP_TXT_FLT_UNINSTALL:
		ret = uninstall_http_txt_flt_drv(); 
		break; 
	case HTTP_TXT_FLT_INIT:
		ret = init_http_txt_flt(); 
		break; 
	case HTTP_TXT_FLT_UNINIT:
		ret = uninit_http_txt_flt(); 
		break; 

	case HTTP_TXT_FLT_GET_ADDED_FLT_TXT:
		ret = get_all_seted_filter_txts( param );
		break; 

	default:
		break; 
	}

	return ret; 
}
