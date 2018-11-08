/*
 * Copyright 2010 JiJie Shi
 *
 * This file is part of NetMonitor.
 *
 * NetMonitor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NetMonitor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NetMonitor.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  2007.6.10	Ji Jie Shi modified it for this ui lib. 
 */

 
 #include "common_func.h"
#include "ui_ctrl.h"
#include "drv_load.h"
#include "inst_drv.h"
#include "netmon_.h"
#include "net_mon_ctrl.h"
#include "trace_log_api.h" 

#define NETMON_DRV_FILE_NAME "netmon.sys"
#define NETMON_DRV_SERVICE_NAME "BCNetMonitor"
#define NETMON_DEVICE_NAME "\\\\.\\BCTdiFilter"

HANDLE net_mon_dev = NULL;
HWND hMainDlg;
HANDLE hThreadTraffic = NULL;
INT32 bStopShowTraffic = FALSE; 
DWORD dwThreadTrafficId; 

DWORD CALLBACK ThreadUpdateTrafficInfo( LPVOID param )
{
	INT32 ret;
	DWORD dwProcessIoInfoLen;
	DWORD dwAllTrafficLen;
	DWORD dwOutputLength;
	PUI_CTRL ui_ctrl; 
	PPROCESS_IO_INFO_OUTPUT pProcessIo;
	LARGE_INTEGER all_traffic[ 2 ];
	CHAR szOutputMsg[ 512 ];
	INT32 i;

	dwProcessIoInfoLen = sizeof( PROCESS_IO_INFO_OUTPUT ) * 100;
	pProcessIo = ( PPROCESS_IO_INFO_OUTPUT )malloc( dwProcessIoInfoLen );

	ret = 0; 

	ui_ctrl = ( PUI_CTRL )param; 

	if( NULL == pProcessIo )
	{
		return FALSE;
	}

	for( ; ; )
	{
		if( TRUE == bStopShowTraffic )
		{
			break;
		}

		ret = DeviceIoControl( net_mon_dev, 
			IOCTL_TDI_GET_ALL_PROCESS_IO_INFO, 
			NULL, 
			0, 
			( PVOID )pProcessIo, 
			dwProcessIoInfoLen, 
			&dwOutputLength, 
			NULL );

		if( ret == FALSE )
		{
			continue;
		}

		ui_ctrl->ui_output_func( NET_MON_OUTPUT_PROC_TRAFFIC, ret, ( PBYTE )pProcessIo, dwOutputLength, ui_ctrl->param ); 

		//sprintf( szOutputMsg, "Process number %d\n", dwItemIndex );

		//ui_ctrl->ui_output_func( NET_MON_OUTPUT_TRAFFIC_COUNT, ret, pProcessIo, dwOutputLength, ui_ctrl->param ); 

		Sleep( 800 );
	}

	free( pProcessIo ); 

	return ret;
}

INT32 create_proc_traffic_thread( PVOID param )
{
	INT32 ret;

	hThreadTraffic = CreateThread( NULL, 0, ThreadUpdateTrafficInfo, param, 0, &dwThreadTrafficId );
	if( NULL == hThreadTraffic )
	{
		return FALSE;
	}
	
	return TRUE;
}

INT32 init_net_mon_work_cont( PVOID param )
{
	INT32 ret; 

	dbg_print( 0, DBG_DBG_BUF_OUT, "check netmon sys file existing \n" ); 
	if( FALSE == file_exists( NETMON_DRV_FILE_NAME, TRUE ) )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_NET_MON_FILE ), "DAT_FILE", NETMON_DRV_FILE_NAME, TRUE ); 
		if( ret < 0 )
		{
			dbg_print( 0, DBG_DBG_BUF_OUT, "load the net mon sys file from resource failed \n" ); 
			return ret; 
		}
	}

	net_mon_dev = load_drv_dev( NETMON_DEVICE_NAME, 
		NETMON_DRV_FILE_NAME, 
		NETMON_DRV_SERVICE_NAME, 
		DRIVER_IN_SYSTEM_DIR );

	if( NULL == net_mon_dev )
	{
		error_handle( "Open netmon device error, make sure netmon is staying in application directory.", MODE_MESSAGE_BOX, MB_YESNO ); 
			return -1;
	}

	ret = create_proc_traffic_thread( param ); 
	if( ret < 0 )
	{
		return ret; 
	}

	ret = unload_drv_dev( NETMON_DEVICE_NAME, 
		NETMON_DRV_FILE_NAME, 
		NETMON_DRV_SERVICE_NAME, 
		DRIVER_IN_SYSTEM_DIR );

	return ret; 
}

INT32 uninit_net_mon_work_cont()
{
	DWORD dwRet;
	bStopShowTraffic = TRUE;
	dwRet = WaitForSingleObject( hThreadTraffic, 200 );

	if( WAIT_OBJECT_0 != dwRet && 
		WAIT_ABANDONED != dwRet )
	{
		TerminateThread( hThreadTraffic, 0 );
	}

	//CloseHandle( hThreadEnumProc );

	//dwRet = WaitForSingleObject( hThreadTraffic, 200 );

	//if( WAIT_OBJECT_0 != dwRet && 
	//	WAIT_ABANDONED != dwRet )
	//{
	//	TerminateThread( hThreadTraffic, 0 );
	//}

	CloseHandle( hThreadTraffic );
	ASSERT( NULL != net_mon_dev );
	CloseHandle( net_mon_dev );

	return 0; 
}

INT32 set_proc_traffic( PPROCESS_INFORMATION_RECORD proc_traffic_ctrl, ULONG length )
{
	INT32 ret; 
	ULONG ret_length; 

	if( length != sizeof( PROCESS_INFORMATION_RECORD ) )
	{
		ASSERT( FALSE ); 
		return -1; 
	}

	ret = DeviceIoControl( net_mon_dev, 
		IOCTL_TDI_START_UPDATE_PROCESS_IO_INFO, 
		proc_traffic_ctrl,
		sizeof( PROCESS_INFORMATION_RECORD ), 
		NULL, 
		0, 
		&ret_length, 
		NULL );

	if( FALSE == ret )
	{
		ret = -1; 
		goto _return;
	}

	ret = 0; 
_return:
	return ret; 
}

INT32 get_prcos_net_traffic( HANDLE hDevice, PPROCESS_INFORMATION_RECORD pProcessInfo, DWORD *pSize )
{
	INT32 ret; 
	ret = DeviceIoControl( hDevice, 
		IOCTL_TDI_GET_ALL_PROCESS_INFO, 
		NULL, 
		0, 
		pProcessInfo, 
		*pSize, 
		pSize, 
		NULL );
	if( ret == FALSE )
	{
		ret = -1; 
	}
	else
	{
		ret = 0; 
	}

	return ret; 
}

INT32 trace_proc_traffic( PFLT_SETTINGS flt_setting, ULONG input_len )
{
	INT32 ret; 
	ULONG ret_len; 

	ASSERT( input_len >= sizeof( FLT_SETTINGS ) ); 
	if( net_mon_dev == NULL )
	{
		ret = -1; 
		goto _return; 
	}

	ret = DeviceIoControl( net_mon_dev, 
		IOCTL_TDI_TRACE_DATA_FLOW, 
		flt_setting, 
		sizeof( *flt_setting ), 
		NULL,
		0, 
		&ret_len, 
		NULL );

	if( ret == FALSE )
	{
		ret = -1; 
	}
	else
	{
		ret = 0; 
	}

_return:
	return ret; 
}

INT32 do_net_mon_ui_work( ULONG ui_action_id, PBYTE input_buf, ULONG input_length, PBYTE output_buf, ULONG *output_length, PVOID param )
{
	INT32 ret; 

	ret = 0; 
	switch( ui_action_id )
	{
	case NET_MON_INIT:
		ret = init_net_mon_work_cont( param ); 
		break; 

	case NET_MON_TRACE_PROC_TRAFFIC:
		ret = trace_proc_traffic( ( PFLT_SETTINGS )input_buf, input_length ); 
		break; 

	case NET_MON_SET_PROC_TRAFFIC:
		ret = set_proc_traffic( ( PPROCESS_INFORMATION_RECORD )input_buf, input_length ); 
		break;

	case NET_MON_UNINIT:
		ret = uninit_net_mon_work_cont(); 
		break; 
	default:
		break; 
	}

	return ret; 
}

