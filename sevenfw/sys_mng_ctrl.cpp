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
#include "sys_mng_api.h"
#include "trace_log_api.h"
#include "sys_mng_ctrl.h"
#include "drv_load.h"
#include "inst_drv.h"

#define MGR_DRV_FILE_NAME "KrnlMgr.sys"
#define MGR_DRV_SERVICE_NAME "KrnlMgr"
#define MGR_DEVICE_NAME "\\\\.\\KrnlMgr"

#define TRACE_LOG_SVC_NAME "TraceLog"
#define TRACE_LOG_FILE_NAME "tracelog.sys"

#define MAX_HOOK_INFO_SIZE ( sizeof( HOOK_FILTER_INFO ) + sizeof( ULONG ) * NtApiFilterEnd )
#define MAX_TRACE_INFO_LEN ( ULONG )( 1024 * 10 )

BOOL get_sys_act_info( PTRACE_INFO_OUTPUT msgs_cap, ULONG Size ); 

BOOL bStopSysManage = FALSE;
HANDLE SysMgrDevice = NULL;
HANDLE trace_log_dev = NULL; 
HANDLE hThreadSysMgr = NULL;
DWORD dwSysMgrThreadId = 0;
HANDLE thread_sys_mng_handle = NULL; 
ULONG sys_mng_thread_id = 0; 
HANDLE InstMutex = NULL; 

BOOL StartSysMgr()
{
	BOOL ret;
	DWORD ret_length;

	ret = DeviceIoControl( SysMgrDevice, 
		IOCTL_START_HOOK_FILTER_MSG, 
		NULL,
		0, 
		( PVOID )NULL, 
		0,  
		&ret_length, 
		NULL );
	return ret;
}

BOOL get_sys_act_info( PTRACE_INFO_OUTPUT msgs_cap, ULONG Size )
{
	INT32 ret;
	DWORD ret_length;
	ULONG logger_name; 

	//DBG_BP(); 

	logger_name = TRACE_LOGGER_TRACE; 
	ret = DeviceIoControl( trace_log_dev, 
		IOCTL_GET_TRACE_LOG,  
		&logger_name,
		sizeof( logger_name ), 
		msgs_cap,
		Size, 
		&ret_length, 
		NULL );

	//free( msgs_cap ); 

	return ret;
}

BOOL get_sys_event( PSYS_EVENT sys_event, ULONG Size )
{
	INT32 ret;
	DWORD ret_length;

	ASSERT( Size > sizeof( SYS_EVENT ) ); 

	ret = DeviceIoControl( SysMgrDevice, 
		IOCTL_GET_FILTER_SYS_EVENT,  
		NULL, 
		0, 
		sys_event,
		Size, 
		&ret_length, 
		NULL );

	//free( msgs_cap ); 

	return ret;
}

BOOL response_sys_event( PSYS_EVENT_RESPON sys_event_respon, ULONG Size )
{
	INT32 ret;
	DWORD ret_length;

	//__asm int 3; 
	ASSERT( sys_event_respon != NULL ); 
	ASSERT( Size == sizeof( SYS_EVENT_RESPON ) ); 
	ASSERT( sys_event_respon->action == TRUE 
		|| sys_event_respon->action == FALSE ); 

	ret = DeviceIoControl( SysMgrDevice, 
		IOCTL_RESPONSE_FILTER_SYS_EVENT,  
		sys_event_respon, 
		sizeof( *sys_event_respon ), 
		NULL,
		0, 
		&ret_length, 
		NULL );

	//free( msgs_cap ); 

	return ret;
}

//BOOL get_sys_event( PSYS_EVENT_RESPON sys_event_respon, ULONG size )
//{
//	INT32 ret;
//	DWORD ret_length;
//	
//	ASSERT( size > sizeof( SYS_EVENT ) ); 
//
//	ret = DeviceIoControl( SysMgrDevice, 
//		IOCTL_GET_HOOK_FILTER_MSG,  
//		ssy_event_respon, 
//		sizeof( SYS_EVENT_RESPON ),  
//		NULL,
//		0, 
//		&ret_length, 
//		NULL );
//
//	//free( msgs_cap ); 
//
//	return ret;
//}

BOOL EndManageFunc()
{
	BOOL ret;
	DWORD ret_length;

	ret = DeviceIoControl( SysMgrDevice, 
		IOCTL_RESET_HOOK_FILTER, 
		NULL,
		0, 
		( PVOID )NULL, 
		0, 
		&ret_length, 
		NULL );
	return ret;
}

BOOL EndSysMgr()
{
	BOOL ret;
	DWORD ret_length;

	ret = DeviceIoControl( SysMgrDevice, 
		IOCTL_END_HOOK_FILTER_MSG, 
		NULL,
		0, 
		( PVOID )NULL, 
		0, 
		&ret_length, 
		NULL );
	return ret;
}

//编辑并继续 : error 1007 : 未定义符号: long `unsigned long __stdcall thread_sys_mon(void *)'::`2'::__LINE__Var(由 d:\antiarp\ui\sevenfw\sevenfw\debug\sys_mng_ctrl.obj 引用)
ULONG CALLBACK thread_sys_mon( LPVOID lpParam )
{
	BOOL ret;
	DWORD dwItemIndex;
	DWORD dwWaitRet;
	PUI_CTRL ui_ctrl; 
	PVOID param; 
	PTRACE_INFO_OUTPUT msgs_cap;

	ASSERT( NULL != lpParam );

	ui_ctrl = ( PUI_CTRL )lpParam;
	param = ui_ctrl->param; 

	msgs_cap = ( PTRACE_INFO_OUTPUT )malloc( MAX_TRACE_INFO_LEN + sizeof( TRACE_INFO_OUTPUT ) );
	if( NULL == msgs_cap )
	{
		ret = FALSE;
		goto _return;
	}

	for( ; ; )
	{
		//dwWaitRet = WaitForSingleObject( sys_act_info->TraceMsgTip, INFINITE );
		//if( WAIT_OBJECT_0 != dwWaitRet && 
		//	WAIT_ABANDONED != dwWaitRet )
		//{
		//	ASSERT( FALSE );
		//}

		Sleep( 600 );

		//MessageBox( NULL, "Got msg tip \n", NULL, 0 );
		msgs_cap->Size = MAX_TRACE_INFO_LEN;
		memset( msgs_cap->Msgs, 0, sizeof( TRACE_INFO_OUTPUT ) + sizeof( CHAR ) );

		//__asm int 3; 
		ret = get_sys_act_info( msgs_cap, MAX_TRACE_INFO_LEN + sizeof( TRACE_INFO_OUTPUT ) );
		if( ret == TRUE )
		{
			ui_ctrl->ui_output_func( SYS_MON_MSGS_OUTPUT, ret, ( PBYTE )msgs_cap, MAX_TRACE_INFO_LEN + sizeof( TRACE_INFO_OUTPUT ), param ); 
		}
		else
		{
			//MessageBox( NULL, "Waited msg failed", NULL, 0 );
		}
	}

_return:
	if( NULL != msgs_cap )
	{
		free( msgs_cap ); 
	}

	return ( ULONG )ret; 
}

ULONG CALLBACK thread_sys_mng( LPVOID lpParam )
{
	BOOL ret;
	DWORD dwItemIndex;
	DWORD dwWaitRet;
	PUI_CTRL ui_ctrl; 
	PSYS_EVENT cur_sys_event; ;
	SYS_EVENT_RESPON sys_event_respon; 

	ASSERT( NULL != lpParam );

	ui_ctrl = ( PUI_CTRL )lpParam;

	cur_sys_event = ( PSYS_EVENT )malloc( MAX_TRACE_INFO_LEN + sizeof( SYS_EVENT ) );
	if( NULL == cur_sys_event )
	{
		ret = FALSE;
		goto _return;
	}

	for( ; ; )
	{
		//dwWaitRet = WaitForSingleObject( sys_act_info->TraceMsgTip, INFINITE );
		//if( WAIT_OBJECT_0 != dwWaitRet && 
		//	WAIT_ABANDONED != dwWaitRet )
		//{
		//	ASSERT( FALSE );
		//}

		Sleep( 600 ); 

		//MessageBox( NULL, "Got msg tip \n", NULL, 0 );
		cur_sys_event->Size = MAX_TRACE_INFO_LEN;
		memset( cur_sys_event->Msgs, 0, sizeof( SYS_EVENT ) + sizeof( CHAR ) );

		ret = get_sys_event( cur_sys_event, MAX_TRACE_INFO_LEN + sizeof( SYS_EVENT ) );
		if( ret == TRUE )
		{
			//__asm int 3; 
			ui_ctrl->ui_output_func( SYS_MON_EVENT_HAPPEND, ret, ( PBYTE )cur_sys_event, MAX_TRACE_INFO_LEN + sizeof( SYS_EVENT ), ui_ctrl->param ); 
		}
		else
		{
			//MessageBox( NULL, "Waited msg failed", NULL, 0 );
		}
	}

_return:
	if( NULL != cur_sys_event )
	{
		free( cur_sys_event ); 
	}

	return ( ULONG )ret; 
}

INT32 StartSysManage( PVOID param )
{
	hThreadSysMgr = CreateThread( NULL, 0, thread_sys_mon, param, 0, &dwSysMgrThreadId );

	thread_sys_mng_handle = CreateThread( NULL, 0, thread_sys_mng, param, 0, &sys_mng_thread_id );
	return TRUE;
}

INT32 init_sys_mon_work_cont( PVOID param )
{
	INT32 ret; 

	//DBG_BP(); 

	if( FALSE == file_exists( TRACE_LOG_FILE_NAME, TRUE ) )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_TRACE_LOG_FILE ), "DAT_FILE", TRACE_LOG_FILE_NAME, TRUE ); 
		if( ret < 0 )
		{
			dbg_print( 0, DBG_DBG_BUF_OUT, "load the trace sys file from resource failed \n" ); 
			return ret; 
		}
	}

	if( FALSE == file_exists( MGR_DRV_FILE_NAME, TRUE ) )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_KRNL_MGR_FILE ), "DAT_FILE", MGR_DRV_FILE_NAME, TRUE ); 
		if( ret < 0 )
		{
			dbg_print( 0, DBG_DBG_BUF_OUT, "load the sys mng sys file from resource failed \n" ); 
			return ret; 
		}
	}

	for( ; ; )
	{
		trace_log_dev = load_drv_dev( TRACE_LOG_DEV_NAME_APP,
			TRACE_LOG_FILE_NAME, 
			TRACE_LOG_SVC_NAME, 
			DRIVER_IN_SYSTEM_DIR | DRIVER_BOOT_START );

		if( trace_log_dev == NULL )
		{
			error_handle( "Open load sysmgr driver error, make sure krnlmgr.sys is staying in application directory.\n Try Again?", MODE_MESSAGE_BOX, MB_OK );; 
			return -1; 
		}

		SysMgrDevice = load_drv_dev( MGR_DEVICE_NAME,
			MGR_DRV_FILE_NAME, 
			MGR_DRV_SERVICE_NAME, 
			DRIVER_IN_SYSTEM_DIR | DRIVER_BOOT_START );

		if( SysMgrDevice == NULL )
		{
			error_handle( "Open load sysmgr driver error, make sure krnlmgr.sys is staying in application directory.\n Try Again?", MODE_MESSAGE_BOX, MB_OK );; 
			return -1; 
		}

		break;
	}

	//ret = unload_drv_dev( MGR_DEVICE_NAME,
	//	MGR_DRV_FILE_NAME, 
	//	MGR_DRV_SERVICE_NAME, 
	//	DRIVER_IN_SYSTEM_DIR );

	ret = StartSysMgr();
	if( FALSE == ret )
	{
		ret = -1; 
		goto err_return; 
	}

	ret = StartSysManage( param );
	if( FALSE == ret )
	{
		ret = -1; 
		goto err_return; 
	}

err_return:
	return ret; 
}

INT32 uninit_sys_mon_work_cont()
{
	INT32 ret = 0; 
	DWORD dwRet;
	bStopSysManage = TRUE;

	//DBG_BP(); 

	if( NULL != SysMgrDevice )
	{
		EndSysMgr(); 
	}

	if( hThreadSysMgr != NULL )
	{
		dwRet = WaitForSingleObject( hThreadSysMgr, 200 );

		if( WAIT_OBJECT_0 != dwRet && 
			WAIT_ABANDONED != dwRet )
		{
			TerminateThread( hThreadSysMgr, 0 );
		}

		CloseHandle( hThreadSysMgr );
		hThreadSysMgr = NULL; 
	}

	if( InstMutex != NULL )
	{
		CloseHandle( InstMutex ); 
		InstMutex = NULL ;
	}

	if( NULL != SysMgrDevice )
	{
		CloseHandle( SysMgrDevice );
		SysMgrDevice = NULL; 
	}

	//ASSERT( NULL != RichEditMod );
	//FreeLibrary( RichEditMod );

	return ret; 
}

INT32 uninstall_sys_mon_work_cont()
{
	INT32 ret; 

	ret = uninit_sys_mon_work_cont(); 
	if( ret < 0 )
	{
		goto _return; 
	}

	ret = unload_drv_dev( MGR_DEVICE_NAME,
		MGR_DRV_FILE_NAME, 
		MGR_DRV_SERVICE_NAME, 
		DRIVER_IN_SYSTEM_DIR );
	if( ret < 0 )
	{
		dbg_print( 0, DBG_CONSOLE_OUT, "unload the sys mgr failed\n" ); 
	}

	ret = unload_drv_dev( TRACE_LOG_DEV_NAME_APP,
		TRACE_LOG_FILE_NAME, 
		TRACE_LOG_SVC_NAME, 
		DRIVER_IN_SYSTEM_DIR );

_return:
	return ret; 
}

INT32 set_sys_mon_flt( PHOOK_FILTER_INFO flt_info, ULONG length )
{
	INT32 ret; 
	DWORD ret_length; 

	ret = DeviceIoControl( SysMgrDevice, 
		IOCTL_SET_HOOK_FILTER, 
		flt_info,
		MAX_HOOK_INFO_SIZE, 
		( PVOID )NULL, 
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

	return ret; 
}

INLINE INT32 sys_mon_event_response( PSYS_EVENT_RESPON event_respon, ULONG length )
{
	return response_sys_event( event_respon, length ); 
}

INT32 do_sys_mon_ui_work( ULONG ui_action_id, PBYTE input_buf, ULONG input_length, PBYTE output_buf, ULONG *output_length, PVOID param )
{
	INT32 ret; 

	ret = 0; 
	switch( ui_action_id )
	{
	case SYS_MON_INIT:
		ret = init_sys_mon_work_cont( param ); 
		break;  
	case SYS_MON_START:
		ret = set_sys_mon_flt( ( PHOOK_FILTER_INFO )input_buf, input_length ); 
		break; 
	case SYS_MON_STOP:
		ret = EndSysMgr(); 
		break; 
	case SYS_MON_SET_FLT:
		ret = set_sys_mon_flt( ( PHOOK_FILTER_INFO )input_buf, input_length );
		break; 
	case SYS_MON_EVENT_RESPONSE:
		ret = sys_mon_event_response( ( PSYS_EVENT_RESPON )input_buf, input_length ); 
		if( ret == TRUE )
		{
			ret = 0; 
		}
		else
		{
			ret = -1; 
		}
		break; 
	case SYS_MON_UNINIT:
		ret = uninit_sys_mon_work_cont(); 
		break; 

	case SYS_MON_UNISTALL:
		ret = uninstall_sys_mon_work_cont(); 

	default:
		break; 
	}

	return ret; 
}


