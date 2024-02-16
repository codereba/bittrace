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
#include "antiarp_api.h"
#include "ui_ctrl.h"
#include "inst_drv.h"
#include "drv_load.h"

HANDLE anti_arp_dev = NULL; 
INT32 stop_arp_analyze = FALSE;

#define ANTI_ARP_DEVICE_NAME "\\\\.\\AntiArp0208"
#define ANTI_ARP_FILE_NAME "antiarp.sys"
#define ANTI_ARP_SVC_NAME "AntiARP"

#define ARP_HOST_MAC_LISTS_BUFF_LEN ( ULONG )( 1024 * 300 )

INT32 antiarp_get_arp_info( PVOID input_buf, ULONG input_length, PVOID output_buf, ULONG output_length, ULONG *ret_length )
{
	INT32 ret;
	PARP_HOST_MAC_LISTS host_arp_mac_lists; 

	host_arp_mac_lists = ( PARP_HOST_MAC_LISTS )output_buf; 

	ret =  do_safe_dev_io_ctl( 
		anti_arp_dev, 
		IOCTL_ANTI_ARP_OUTPUT_MAC_LIST, 
		input_buf, 
		input_length, 
		output_buf, 
		output_length, 
		ret_length ); 
	
	return ret; 
}

DWORD CALLBACK thread_analyze_arp_info( PVOID param )
{
	ULONG ret_length; 
	BOOL ret;
	PBYTE buf;
	ARP_HOST_MAC_LISTS *arp_info_output; 
	PUI_CTRL callback_param; 

	callback_param = ( PUI_CTRL )param; 
	ASSERT( NULL != callback_param ); 

	buf = ( PBYTE )malloc( ARP_HOST_MAC_LISTS_BUFF_LEN ); 
	if( buf == NULL )
	{
		return 0; 
	}

	arp_info_output = ( PARP_HOST_MAC_LISTS )buf; 
	for( ; ; )
	{
		if( stop_arp_analyze == TRUE )
		{
			break; 
		}

		memset( arp_info_output, 0, ARP_HOST_MAC_LISTS_BUFF_LEN ); 

		ret = antiarp_get_arp_info( NULL, 
			0, 
			arp_info_output, 
			ARP_HOST_MAC_LISTS_BUFF_LEN, 
			&ret_length ); 

		if( ret == 0 )
		{
			//::MessageBox( NULL, "antiarp_get_arp_info return 0 \n", NULL, 0 ); 

			ret = callback_param->ui_output_func( 
				ANTI_ARP_GET_ARP_INFO, 
				ret, 
				( PBYTE )arp_info_output, 
				ARP_HOST_MAC_LISTS_BUFF_LEN, 
				callback_param->param ); 

			if( ret < 0 )
			{
				error_handle( "do ui work failed \n", MODE_DEBUG_OUTPUT, 0 ); 
			}
		}

		Sleep( 3000 );
	}

	free( buf ); 

	return 0; 
}

INT32 antiarp_arp_mod_inited()
{
	INT32 ret; 
	ULONG ret_length;

	ret = DeviceIoControl( anti_arp_dev, 
		IOCTL_ANTI_ARP_INITIALIZED, 
		NULL, 
		0, 
		NULL, 
		0, 
		&ret_length, 
		NULL ); 

	if( TRUE == ret )
	{
		return 0; 
	}

	return -1; 
}

HANDLE inst_mutex = NULL; 
HANDLE thread_analyze = NULL; 

INLINE INT32 set_anti_arp_work_mode( ULONG work_mode )
{
	INT32 ret; 
	ULONG ret_length;

	ret = DeviceIoControl( anti_arp_dev, 
		IOCTL_ANTI_ARP_SET_WORK_MODE, 
		&work_mode, 
		sizeof( ULONG ), 
		NULL, 
		0, 
		&ret_length, 
		NULL ); 

	if( TRUE == ret )
	{
		return 0; 
	}

	return -1; 
}

INLINE INT32 start_arp_filter()
{
	//::MessageBox( NULL, __FUNCTION__, NULL, 0 );
	return set_anti_arp_work_mode( ANTI_ARP_WORK ); 
}

INLINE INT32 stop_arp_filter()
{
	//::MessageBox( NULL, __FUNCTION__, NULL, 0 ); 
	return set_anti_arp_work_mode( ANTI_ARP_NO_WORK ); 
}

INLINE INT32 init_anti_arp( PVOID param )
{
	INT32 ret; 

	if( FALSE == file_exists( ANTI_ARP_FILE_NAME, TRUE ) )
	{
		create_file_from_res( NULL, MAKEINTRESOURCE( IDR_ANTI_ARP_FILE ), "DAT_FILE", ANTI_ARP_FILE_NAME, TRUE ); 
	}

	anti_arp_dev = load_drv_dev( ANTI_ARP_DEVICE_NAME, 
		ANTI_ARP_FILE_NAME, 
		ANTI_ARP_SVC_NAME, 
		DRIVER_IN_SYSTEM_DIR ); 

	if( NULL == anti_arp_dev )
	{
		ret = -1; 
		unload_drv_dev( ANTI_ARP_DEVICE_NAME, 
		ANTI_ARP_FILE_NAME, 
		ANTI_ARP_SVC_NAME, 
		DRIVER_IN_SYSTEM_DIR ); 
		error_handle( "Initializing error, will exit.\n", MODE_DEBUG_OUTPUT, 0 ); 
		goto _err_return;  
	}

	//ret = antiarp_arp_mod_inited(); 

	//if( ret < 0 )
	//{
	//	ret = -1; 
	//	if( IDYES == error_handle( "Anti-Arp need system restart.\n", MODE_DEBUG_OUTPUT, 0 ) )
	//	{
	//		windows_shutdown(EWX_REBOOT | EWX_FORCE); //(EWX_SHUTDOWN | EWX_FORCE | EWX_POWEROFF);
	//	}

	//	goto _err_return; 
	//}

	thread_analyze  = ::CreateThread( NULL, 
		0, 
		thread_analyze_arp_info, 
		param, 
		0, 
		NULL ); 

	if( NULL == thread_analyze  )
	{
		ret = -1; 
		
		error_handle( "Initializing error, will exit.\n", MODE_DEBUG_OUTPUT, 0 ); 
		goto _err_return;  
	}

	ret = 0; 
	return ret; 

_err_return:
	if( NULL != inst_mutex )
	{
		CloseHandle( inst_mutex ); 
	}

	if( NULL != anti_arp_dev )
	{
		CloseHandle( anti_arp_dev ); 
	}

	return ret; 
}

INT32 do_anti_arp_ui_work( ULONG ui_action_id, PBYTE input_buf, ULONG input_length, PBYTE output_buf, ULONG *output_length, PVOID param )
{
	INT32 ret; 
	ULONG _output_length; 

	_output_length = 0; 

	switch( ui_action_id )
	{
	case ANTI_ARP_INIT:
		ret = init_anti_arp( param );  
		break; 
	case ANTI_ARP_STOP:
		ret = stop_arp_filter(); 
		break; 
	case ANTI_ARP_START:
		ret = start_arp_filter(); 
		break; 
	default:
		break; 
	}

	*output_length = _output_length; 
	return ret; 
}

