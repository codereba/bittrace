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
#include <winsvc.h>
#include <process.h>
#include "service.h"
#include "bittrace_daemon.h"

ULONG work_proc_count = 0;
proc_work_param* work_proc_info = 0;

CRITICAL_SECTION service_lock;
HANDLE manage_event; 
ULONG manage_delay_time = 5000; 
INT32 stop_manage = FALSE; 
HANDLE manage_thread = NULL; 

SERVICE_STATUS          service_status; 
SERVICE_STATUS_HANDLE   service_status_handle; 

VOID WINAPI service_main( DWORD argc, LPTSTR *argv);
VOID WINAPI service_handler( DWORD ctrl_code ); 

SERVICE_TABLE_ENTRYA service_dispatch_table[] = 
{ 
	{ SERVICE_NAME, service_main }, 
	{ NULL, NULL } 
};

LRESULT start_proc_manage(); 
LRESULT init_proc_work_params( ULONG size )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 locked = FALSE;
	if( size == 0 )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( manage_thread != NULL )
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	lock_cs( service_lock );
	locked = TRUE; 
	work_proc_count = size; 

	work_proc_info = ( proc_work_param* )malloc( sizeof( proc_work_param ) * work_proc_count );

	if( work_proc_info == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	memset( work_proc_info, 0, sizeof( proc_work_param ) * work_proc_count ); 

_return:
	if( locked == TRUE )
	{
		unlock_cs( service_lock ); 
	}
	return 0; 
}

LRESULT set_proc_work_param( ULONG index, LPCSTR cmd_line, LPCSTR work_dir, ULONG flags, ULONG start_delay )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 locked = FALSE; 

	if( index >= work_proc_count )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	lock_cs( service_lock ); 
	locked = TRUE; 

	strncpy( work_proc_info[ index ].cmd_line, cmd_line, MAX_PATH ); 
	strncpy( work_proc_info[ index ].work_dir, work_dir, MAX_PATH ); 
	work_proc_info[ index ].flags = flags; 
	work_proc_info[ index ].start_delay = start_delay; 
	work_proc_info[ index ].proc_info.dwProcessId = 0; 
	work_proc_info[ index ].proc_info.dwThreadId = 0; 
	work_proc_info[ index ].proc_info.hProcess = NULL; 
	work_proc_info[ index ].proc_info.hThread = NULL; 

_return:
	if( locked == TRUE )
	{
		unlock_cs( service_lock ); 
	}

	return ret; 
}

LRESULT uninit_proc_work_params()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 locked = FALSE; 
	DWORD wait_ret; 

	if( manage_thread != NULL )
	{
		wait_ret = WaitForSingleObject( manage_thread, 1000 ); 
		if( wait_ret != WAIT_OBJECT_0 
			|| wait_ret != WAIT_ABANDONED )
		{
			if( wait_ret == WAIT_TIMEOUT )
			{
				ret = ERROR_NOT_READY; 
				goto _return; 
			}
			else if( wait_ret == WAIT_FAILED )
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait process end failed \n" ) ); 
			}
		}
	}

	manage_thread = NULL; 

	lock_cs( service_lock ); 
	locked = TRUE; 

	work_proc_count = 0; 
	free( work_proc_info ); 
	work_proc_info = NULL; 
	
	if( locked == TRUE )
	{
		unlock_cs( service_lock ); 
	}

	uninit_cs_lock( service_lock ); 

	CloseHandle( manage_event ); 

_return:

	return ret; 
}

LRESULT stop_proc_manage()
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD wait_ret; 

	if( manage_thread == NULL )
	{
		ret = ERROR_NOT_READY; 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "start manage thread failed \n" ) ); 
		goto _return; 
	}

	stop_manage = TRUE; 
	SetEvent( manage_event ); 
	wait_ret = WaitForSingleObject( manage_thread, 1000 ); 
	if( wait_ret == WAIT_OBJECT_0 
		|| wait_ret == WAIT_ABANDONED )
	{
		goto _return; 
	}

	if( wait_ret == WAIT_FAILED )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait process end failed \n" ) ); 
	}

	TerminateThread( manage_thread, 0 ); 

_return:
	return ret; 
}

LRESULT init_proc_manage_context( ULONG size )
{
	LRESULT ret = ERROR_SUCCESS; 

	stop_manage = FALSE; 
	manage_event = CreateEvent( NULL, FALSE, FALSE, NULL ); 
	if( manage_event == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	init_cs_lock( service_lock ); 

	ret = init_proc_work_params( size ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT uninit_proc_manage_context()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = stop_proc_manage(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!stop process maange failed \n" ) ); 
	}

	if( manage_event != NULL )
	{
		CloseHandle( manage_event ); 
	}

	uninit_cs_lock( service_lock ); 

	ret = uninit_proc_work_params(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!uninitialize the process manage context failed \n" ) ); 
	}

_return:
	return ret; 
}
BOOL is_inited_work_param( proc_work_param *work_param )
{
	BOOL ret = FALSE ; 
	ASSERT( work_param != NULL ); 

	if( *work_param->cmd_line == '\0' )
	{
		goto _return; 
	}

	ret = TRUE; 
_return:
	return ret; 
}

BOOL proc_started( proc_work_param *work_param )
{
	BOOL ret = FALSE; 
	BOOL _ret; 
	DWORD exit_code; 
	DWORD error_code; 

	if( work_param->proc_info.hProcess == NULL )
	{
		goto _return; 
	}
	
	log_trace( ( MSG_INFO, "GetExitCodeProcess handle 0x%0.8x\n", work_param->proc_info.hProcess ) ); 

	_ret = GetExitCodeProcess( work_param->proc_info.hProcess, &exit_code ); 
	if( _ret == FALSE )
	{
		error_code = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!GetExitCodeProcess failed\n" ) ); 
		if( error_code == ERROR_INVALID_HANDLE )
		{
			work_param->proc_info.hProcess = NULL; 
			goto _return; 
		}
	}
	else
	{
		if( exit_code == STILL_ACTIVE )
		{
			ret = TRUE; 
			goto _return; 
		}
	}

	_ret = CloseHandle( work_param->proc_info.hProcess ); 
	if( _ret == FALSE )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!GetExitCodeProcess failed\n" ) ); 
	}

	work_param->proc_info.hProcess = NULL; 

_return:
	return ret; 
}

LRESULT start_process( ULONG index ) 
{ 
	BOOL have_ui; 
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	INT32 locked = FALSE; 
	proc_work_param *work_param; 

	STARTUPINFOA proc_start_info = { sizeof( STARTUPINFO ), NULL, "", 
									NULL, 0, 0, 0, 
									0, 0, 0, 0, 
									STARTF_USESHOWWINDOW, 0, 0, NULL, 
									0, 0, 0 }; 
	
	log_trace( ( MSG_INFO, "enter %s process id %d\n", __FUNCTION__, index ) ); 
	
	lock_cs( service_lock ); 
	locked = TRUE; 

	if( index >= work_proc_count )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	work_param = &work_proc_info[ index ]; 
	if( is_inited_work_param( work_param ) == FALSE )
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	if( proc_started( work_param ) == TRUE )
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	have_ui = !!( work_param->flags & UI_PROCESS ); 

	log_trace( ( MSG_INFO, "process configure have ui %d, command line %s, working dir %s \n", have_ui, work_param->cmd_line, work_param->work_dir ) ); 

	if( have_ui )
	{
		proc_start_info.wShowWindow = SW_SHOW;
		proc_start_info.lpDesktop = NULL;
	}
	else
	{
		proc_start_info.wShowWindow = SW_HIDE;
		proc_start_info.lpDesktop = "";
	}

	_ret = CreateProcessA( 
		NULL,
		work_param->cmd_line, 
		NULL, 
		NULL, 
		TRUE,NORMAL_PRIORITY_CLASS, 
		NULL, 
		strlen( work_param->work_dir ) == 0 ? NULL : work_param->work_dir, 
		&proc_start_info, 
		&work_proc_info[ index ].proc_info ); 
	
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, 
			"start command '%s' failed\n", work_param->cmd_line ) ); 

		goto _return; 
	}

	Sleep( work_param->start_delay ); 

_return:
	if( locked == TRUE )
	{
		unlock_cs( service_lock ); 
	}
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT end_process( ULONG index ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	proc_work_param *work_param; 
	INT32 locked = FALSE; 

	log_trace( ( MSG_INFO, "enter %s prcoess id \n", __FUNCTION__, index ) ); 

	lock_cs( service_lock ); 
	locked = TRUE; 
	
	if( index >= work_proc_count )
	{
		ret = ERROR_INVALID_PARAMETER;
		goto _return; 
	}

	work_param = &work_proc_info[ index ]; 

	if( proc_started( work_param ) == FALSE )
	{
		goto _return; 
	}
	
	if( work_param->exist_delay > 0 )
	{
		DWORD wait_ret; 
		PostThreadMessage( work_proc_info[ index ].proc_info.dwThreadId, WM_QUIT, 0, 0 ); 
		wait_ret = WaitForSingleObject( work_proc_info[ index ].proc_info.hProcess, work_param->exist_delay ); 
		if( wait_ret == WAIT_OBJECT_0 
			|| wait_ret == WAIT_ABANDONED )
		{
			goto _return; 
		}

		if( wait_ret == WAIT_FAILED )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait process end failed \n" ) ); 
		}
	}
	
	_ret = TerminateProcess( work_proc_info[ index ].proc_info.hProcess, 0 ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	if( locked == TRUE )
	{
		unlock_cs( service_lock ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT ctrl_process( LPCSTR name, ULONG index ) 
{ 
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	SC_HANDLE svc_manage = NULL; 
	SC_HANDLE service = NULL; 
	SERVICE_STATUS status;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	svc_manage = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if( svc_manage == NULL ) 
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenSCManager failed\n" ) ); 
		goto _return; 
	}

	service = OpenServiceA( svc_manage, name, SERVICE_ALL_ACCESS);
	if( service == NULL ) 
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed\n" ) ); 
		goto _return; 
	}
		
	if( ( index & 0xffffff80 ) == 0 )
	{
		ret = ERROR_INVALID_PARAMETER; 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "Invalid argument to control process\n" ) ); 
		ASSERT( FALSE ); 
		goto _return; 
	}

	_ret = ControlService( service,( index | SERVICE_ACCEPT_SESSIONCHANGE ), &status ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "ControlService failed\n" ) ); 
		goto _return; 
	}

_return:
	if( service != NULL )
	{
		CloseServiceHandle( service ); 
	}

	if( svc_manage != NULL )
	{
		CloseServiceHandle( svc_manage ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x \n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT stop_service( LPCSTR name ) 
{ 
	LRESULT ret = ERROR_SUCCESS; 
	SC_HANDLE svc_manage = NULL; 
	SC_HANDLE service = NULL; 
	SERVICE_STATUS status;
	BOOL _ret; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	svc_manage = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 

	if( svc_manage == NULL ) 
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenSCManager failed\n" ) );
		goto _return; 
	}

	service = OpenServiceA( svc_manage, name, SERVICE_ALL_ACCESS ); 
	if( service == NULL ) 
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed\n" ) ); 
		goto _return; 
	}

	_ret = ControlService( service, SERVICE_CONTROL_STOP, &status ); 

	if( _ret == FALSE )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "ControlService failed\n" ) ); 
	}

_return:
	if( service != NULL )
	{
		CloseServiceHandle( service ); 
	}

	if( svc_manage != NULL )
	{
		CloseServiceHandle( svc_manage ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT check_service_exist( LPCSTR name )
{

	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	SC_HANDLE svc_manage = NULL; 
	SC_HANDLE service = NULL; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	svc_manage = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ); 

	if( svc_manage == NULL ) 
	{
		ret = GetLastError();
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenSCManager failed \n" ) ); 
		goto _return; 
	}

	service = OpenServiceA( svc_manage, name, SERVICE_QUERY_STATUS);
	if( service == NULL ) 
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed \n" ) );
		goto _return; 
	}

_return:
	if( svc_manage != NULL )
	{
		CloseServiceHandle( svc_manage ); 
	}

	if( service != NULL )
	{
		CloseServiceHandle( service ); 
	}

	return ret; 
}

LRESULT start_service( LPCSTR name, INT32 argv, LPCSTR *argc ) 
{ 
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	SC_HANDLE svc_manage = NULL; 
	SC_HANDLE service = NULL; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	svc_manage = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ); 

	if( svc_manage == NULL ) 
	{
		ret = GetLastError();
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenSCManager failed \n" ) ); 
		goto _return; 
	}

	service = OpenServiceA( svc_manage, name, SERVICE_ALL_ACCESS);
	if( service == NULL ) 
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed \n" ) );
		goto _return; 
	}

	_ret = StartServiceA( service, argv, argc ); 

	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		if( ret == ERROR_SERVICE_ALREADY_RUNNING )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "service aleary started \n" ) ); 
			ret = ERROR_SUCCESS; 
			goto _return; 
		}

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "start service failed \n" ) );
		goto _return; 
	}

	log_trace( ( MSG_INFO, "StartService successfully\n" ) ); 

_return:

	if( svc_manage != NULL )
	{
		CloseServiceHandle( svc_manage ); 
	}

	if( service != NULL )
	{
		CloseServiceHandle( service ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret;
}

#ifndef SERVICE_FUNC_REFERENCE
#define SERVICE_SPECIFIC_ERROR_CODE ( ULONG )( -1 )
VOID WINAPI service_main( DWORD argc, LPSTR *argv )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	INT32 i; 
	DWORD status = 0; 
    DWORD spec_error_code = SERVICE_SPECIFIC_ERROR_CODE; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	service_status.dwServiceType = SERVICE_WIN32; 
	service_status.dwCurrentState = SERVICE_START_PENDING; 
    service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP 
		| SERVICE_ACCEPT_SHUTDOWN 
		| SERVICE_ACCEPT_PAUSE_CONTINUE; 
    

	service_status.dwWin32ExitCode = 0; 
    service_status.dwServiceSpecificExitCode = 0; 
    service_status.dwCheckPoint = 0; 
    service_status.dwWaitHint = 0; 


	service_status_handle = RegisterServiceCtrlHandlerA( SERVICE_NAME, service_handler ); 
	if( service_status_handle == NULL ) 
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "RegisterServiceCtrlHandler failed \n" ) ); 

		if( ret != NO_ERROR ) 
		{ 
			service_status.dwCurrentState       = SERVICE_STOPPED; 
			service_status.dwCheckPoint         = 0; 
			service_status.dwWaitHint           = 0; 
			service_status.dwWin32ExitCode      = ret; 
			service_status.dwServiceSpecificExitCode = spec_error_code; 

			log_trace( ( DBG_MSG_AND_ERROR_OUT, "set service to stopped status \n" ) ); 
			_ret = SetServiceStatus( service_status_handle, &service_status ); 
			if( _ret == FALSE )
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "set service to stopped status failed \n" ) ); 
			}
		}

		goto _return; 
	}

	service_status.dwCurrentState = SERVICE_RUNNING; 
	service_status.dwCheckPoint = 0; 
	service_status.dwWaitHint= 0; 

	log_trace( ( MSG_INFO, "set service to running\n" ) ); 
	_ret = SetServiceStatus( service_status_handle, &service_status ); 
	if( _ret == FALSE ) 
	{ 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "set service to running status failed\n" ) ); 
	} 

	ret = init_service_context_async(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "initialize seven fw context failed 0x%0.8x\n", ret ) ); 
		goto _return; 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return; 
}
#else
VOID WINAPI service_main( DWORD argc, LPSTR *argv )
{
}
#endif //SERVICE_FUNC_REFERENCE

#ifndef SERVICE_FUNC_REFERENCE
VOID WINAPI service_handler( DWORD ctrl_code )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	INT32 i; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	switch( ctrl_code ) 
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:

		service_status.dwWin32ExitCode = 0; 
		service_status.dwCurrentState  = SERVICE_STOPPED; 
		service_status.dwCheckPoint    = 0; 
		service_status.dwWaitHint      = 0;

		for( i = work_proc_count - 1; i >= 0; i-- )
		{
			ret = end_process( i );
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "end process %d failed \n", i ) ); 
			}
		}

		ret = stop_proc_manage(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "stop process manage failed \n" ) ); 
		}

		ret = uninit_proc_manage_context(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "release process manage failed \n" ) ); 
		}

		ret = uninit_service_context(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "uninitialize servcie context failed\n" ) ); 
		}
		break; 
	case SERVICE_CONTROL_PAUSE:
		service_status.dwCurrentState = SERVICE_PAUSED; 

		break;
	case SERVICE_CONTROL_CONTINUE:
		service_status.dwCurrentState = SERVICE_RUNNING; 
		break;

	case SERVICE_CONTROL_INTERROGATE:
		break;

	default: 
		break; 
	}

	_ret = SetServiceStatus( service_status_handle,  &service_status ); 
    if( _ret == FALSE ) 
	{ 
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "SetServiceStatus failed \n" ) );
    }

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return; 
}
#endif //SERVICE_FUNC_REFERENCE

LRESULT uninstall_service( CHAR* name )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	SC_HANDLE svc_manage;// = NULL; 
	SC_HANDLE service = NULL; 

	log_trace( ( MSG_INFO, "enter %s service name %s\n", __FUNCTION__, name ) ); 
	svc_manage = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if( svc_manage == NULL ) 
	{
		ret = GetLastError(); 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenSCManager failed \n" ) ); 
		goto _return; 
	}

	service = OpenServiceA( svc_manage, name, SERVICE_ALL_ACCESS);
	if( service == NULL ) 
	{
		ret = GetLastError(); 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed\n" ) ); 
		goto _return; 
	}

	_ret = DeleteService( service ); 
	if( _ret == FALSE ) 
	{
		ret = GetLastError(); 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "Failed to delete service %s\n", name ) ); 
		goto _return; 
	}

	
	log_trace( ( DBG_MSG_AND_ERROR_OUT, "Service %s removed\n", name ) ); 


_return:
	if( service != NULL )
	{
		CloseServiceHandle( service ); 
	}

	if( svc_manage != NULL )
	{
		CloseServiceHandle( svc_manage ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT install_service( LPCSTR path, LPCSTR name ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	SC_HANDLE svc_manage = NULL; 
	SC_HANDLE service = NULL; 

	log_trace( ( MSG_INFO, "enter %s service path %s, servcie name %s\n", __FUNCTION__, path, name ) ); 

	svc_manage = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE); 

	if( svc_manage == NULL ) 
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenSCManager failed \n" ) ); 
		goto _return; 
	}

	service = CreateServiceA( svc_manage,	/* SCManager database */ 
		name,			/* name of service */ 
		name,			/* service name to display */  
		SERVICE_ALL_ACCESS,        /* desired access */  
		SERVICE_WIN32_SHARE_PROCESS, //SERVICE_WIN32_OWN_PROCESS  | SERVICE_INTERACTIVE_PROCESS /* service type */  
		/*SERVICE_SYSTEM_START,*/ 
		SERVICE_AUTO_START,      /* start type              */ 
		SERVICE_ERROR_NORMAL,      /* error control type      */ 
		path,			/* service's binary        */ 
		NULL,                      /* no load ordering group  */ 
		NULL,                      /* no tag identifier       */ 
		NULL,                      /* no dependencies         */ 
		NULL,                      /* LocalSystem account     */ 
		NULL );                     /* no password             */ 
	if( service == NULL ) 
	{
		ret = GetLastError(); 
		if( ret == ERROR_SERVICE_EXISTS) 
		{
			log_trace( ( MSG_WARNING, "service already install\n" ) ); 
			ret = ERROR_SUCCESS; 
			goto _return; 
		}
		else if( ret == ERROR_SERVICE_MARKED_FOR_DELETE ) 
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "Previous instance of the service is not fully deleted. Try again...\n") );
			goto _return; 
		}

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "Failed to create service %s \n", name ) ); 
		goto _return; 
	}

	log_trace( ( DBG_MSG_AND_ERROR_OUT, "Service %s install succfully\n", name ) ); 

_return:

	if( service != NULL )
	{
		CloseServiceHandle( service ); 
	}

	if( svc_manage != NULL )
	{
		CloseServiceHandle( svc_manage );
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

DWORD CALLBACK proc_manage_thread( PVOID pParam )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD wait_ret; 
	DWORD exit_code;
	BOOL _ret; 
	INT32 i; 
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	for( ; ; )
	{
		log_trace( ( MSG_INFO, "%s check loop \n", __FUNCTION__ ) ); 
		if( stop_manage == TRUE )
		{
			break; 
		}

		if( work_proc_count <= 0 )
		{
			goto _continue; 
		}

		for( i = 0; ( ULONG )i < work_proc_count; i++ )
		{
			ret = ERROR_SUCCESS; 
			exit_code = 0; 
			lock_cs( service_lock ); 
			if( is_inited_work_param( &work_proc_info[ i ]) == TRUE 
				&& proc_started( &work_proc_info[ i ]) == TRUE )
			{

				log_trace( ( MSG_INFO, "GetExitCodeProcess handle 0x%0.8x\n", work_proc_info[ i ].proc_info.hProcess ) ); 

				_ret = GetExitCodeProcess( work_proc_info[ i ].proc_info.hProcess, &exit_code ); 
				if( _ret == FALSE )
				{
					ret = GetLastError(); 
					log_trace( ( DBG_MSG_AND_ERROR_OUT, "GetExitCodeProcess failed\n" ) ); 
				}
			}
			unlock_cs( service_lock ); 

			if( ret != ERROR_SUCCESS )
			{
				goto _continue; 
			}

			log_trace( ( MSG_INFO, "process exit status is 0x%0.8x \n", exit_code ) ); 

			if( exit_code == STILL_ACTIVE )
			{
				
				goto _continue; 
			}

			ret = start_process( i ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "!!!restart process %d failed\n", i ) ); 
				goto _continue; 
			}

			log_trace( ( MSG_INFO, "process %d restarted \n", i ) );
		}
_continue:
		wait_ret = wait_event( manage_event, manage_delay_time ); 
		if( wait_ret != WAIT_TIMEOUT 
			&& wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait syncronous event failed will exit\n" ) ); 
			break; 
		}
	}

_return: 
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ret; 
}

LRESULT start_proc_manage()
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	manage_thread = CreateThread( NULL, 0, proc_manage_thread, NULL, 0, NULL ); 
	if( manage_thread == NULL )
	{
		ret = GetLastError(); 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "start manage thread failed \n" ) ); 
		goto _return; 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

#ifdef SERVICE_EXE
#define MAX_CML_LINE_LEN 512
void main(int argc, char *argv[] )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	CHAR exe_file_name[ _MAX_PATH ];
	CHAR cmd_line[ MAX_CML_LINE_LEN ]; 
	DWORD file_name_len; 

	log_trace( ( MSG_INFO, "enter %s argc is %d \n", __FUNCTION__, argc ) ); 

	file_name_len = GetModuleFileName( NULL, exe_file_name, _MAX_PATH ); 
	if( file_name_len == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	strcpy( cmd_line, "\"" ); 
	strncat( cmd_line, exe_file_name, MAX_CML_LINE_LEN - strlen( cmd_line ) ); 
	strncat( cmd_line, "\"", MAX_CML_LINE_LEN - strlen( cmd_line ) ); 
	strncat( cmd_line, " /s", MAX_CML_LINE_LEN - strlen( cmd_line ) ); 

	if( argc < 2 )
	{
		goto _return; 
	}

	if( _stricmp( argv[ 1 ], "/t" ) == 0 )
	{
		stop_service( SERVICE_NAME ); 
		uninstall_service( SERVICE_NAME ); 
		install_service( cmd_line, SERVICE_NAME ); 
		start_service( SERVICE_NAME, 0, NULL ); 
	}
	else if( _stricmp( argv[ 1 ], "/i" ) == 0 )
	{
		stop_service( SERVICE_NAME );
		uninstall_service( SERVICE_NAME );
		install_service( exe_file_name, SERVICE_NAME );
		start_service( SERVICE_NAME, 0, NULL ); 
	}
	else if( _stricmp( argv[ 1 ], "/d" ) == 0 )
	{
		stop_service( SERVICE_NAME );
		uninstall_service( SERVICE_NAME );
	}
	else if( _stricmp( argv[ 1 ], "/s" ) == 0 )
	{
		ret = init_proc_manage_context( 1 ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "init proc manage context failed \n" ) ); 
			goto _return; 
		}

		ret = set_proc_work_param( 0, "\"D:\\AntiArp\\ui\\SevenFw\\SevenFwUI\\duilib v1.1\\bin\\UIDesigner_d.exe\"", "", UI_PROCESS, 1 ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "set process work parameters failed \n" ) ); 
			goto _return; 
		}

		log_trace( ( MSG_INFO, "start service dispatch \n" ) ); 
		_ret = StartServiceCtrlDispatcher( service_dispatch_table ); 
		if( ret == FALSE )
		{
			ret = GetLastError();
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "StartServiceCtrlDispatcher failed \n" ) ); 
			goto _return; 
		}

		uninit_proc_manage_context(); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
}

#endif //SERVICE_EXE

class CAutoServiceHandle
{
public:
	CAutoServiceHandle() : handle( NULL ) {}; 
	~CAutoServiceHandle() { CleanUp();  }; 

	CAutoServiceHandle( SC_HANDLE h )
	{
		handle = h;
	}

	SC_HANDLE operator=( SC_HANDLE h ) 
	{ 
		CleanUp(); 
		handle = h;
		return ( *this );  
	}

	operator SC_HANDLE()
	{
		return handle;
	}

	SC_HANDLE operator->()                 // for using as smart pointer
	{
		return handle;
	}

	BOOL IsValid()
	{
		return handle != NULL;
	}

protected:
	void CleanUp()
	{
		if ( handle != NULL )
		{
			CloseServiceHandle( handle ); 
			handle = NULL;
		}
	}

protected:
	SC_HANDLE handle; 
};

//#define DPRINT printf

#define DPRINT 

struct T_WindowsInfo
{
	OSVERSIONINFOEX osvi;
	INT nWindowsVersion;
	INT nSP;

	BOOL Init();

protected:
	BOOL GetWindowsVersionInfo( OSVERSIONINFOEX &osvi );
	BOOL FixVersionInfo( OSVERSIONINFOEX &osvi );
	int _ParseWinVer( OSVERSIONINFOEX &osvi );
};

BOOL GetWinVerInfo(INT &nWindowsVersion, INT &nSP )
{
	T_WindowsInfo winInfo;
	if( !winInfo.Init() )
		return FALSE;
	nWindowsVersion = winInfo.nWindowsVersion;
	nSP = winInfo.nSP;
	return TRUE;
}

BOOL T_WindowsInfo::Init()
{
	nWindowsVersion = WINUNKNOWN;
	nSP = 0;

	ZeroMemory(&osvi, sizeof(osvi));
	if(GetWindowsVersionInfo(osvi))
	{
		FixVersionInfo(osvi);
		nWindowsVersion = _ParseWinVer( osvi );
		nSP = osvi.wServicePackMajor;
		return TRUE;
	}
	return FALSE;
}

BOOL T_WindowsInfo::GetWindowsVersionInfo( OSVERSIONINFOEX &osvi )
{
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( GetVersionEx((OSVERSIONINFO *) &osvi) )
		return TRUE;
	osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	return GetVersionEx ( (OSVERSIONINFO *) &osvi);
}

BOOL T_WindowsInfo::FixVersionInfo( OSVERSIONINFOEX &osvi )
{
	return FALSE;
}

int T_WindowsInfo::_ParseWinVer( OSVERSIONINFOEX &osvi )
{
	int wVersion = WINUNKNOWN;
	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		wVersion = WIN32S;
		break;

	case VER_PLATFORM_WIN32_WINDOWS:
		if ( osvi.dwMinorVersion == 0 )
			wVersion = WIN95;	
		else if ( osvi.dwMinorVersion == 10)
			wVersion = WIN98;
		else if ( osvi.dwMinorVersion == 90)
			wVersion = WINME;
		break;

	case VER_PLATFORM_WIN32_NT:
		if ( osvi.dwMajorVersion <= 4 )
			wVersion = WINNT;
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
			wVersion = WIN2K;
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			wVersion = WINXP;
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
			wVersion = WIN2003;
		else if ( osvi.dwMajorVersion == 6 )
		{
			if(osvi.dwMinorVersion==0)
			{
				wVersion = osvi.wProductType == VER_NT_WORKSTATION ? WINVISTA : WIN2008;
			}
			else if(osvi.dwMinorVersion==1)
			{
				wVersion = osvi.wProductType == VER_NT_WORKSTATION ? WIN7 : WIN2008R2;
			}
		}
		break;

	default:
		break;
	}
	return wVersion;
}

int CompareVersion(DWORD dwMajorVer, DWORD dwMinorVer)
{
	T_WindowsInfo wininfo;
	wininfo.Init();

	if (wininfo.osvi.dwMajorVersion > dwMajorVer)
	{
		return 1;
	}
	else if (wininfo.osvi.dwMajorVersion < dwMajorVer)
	{
		return -1;
	}
	return wininfo.osvi.dwMinorVersion - dwMinorVer;
}

BOOL IsVistaOrLater()
{
	return 0 <= CompareVersion(em_OS_MajorVer_Vista, 0);
}

HRESULT IsServiceExists( LPCTSTR pszSrvName, BOOL &bExists )
{
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
		return E_POINTER;

	bExists = FALSE;
	CAutoServiceHandle schService = OpenService( m_schSCManager, pszSrvName, SERVICE_ALL_ACCESS);
	if (( SC_HANDLE )NULL == schService)
	{
		return ERROR_SERVICE_DOES_NOT_EXIST==GetLastError() ? S_OK : E_FAIL;
	}
	bExists = TRUE;
	return S_OK;
}

HRESULT IsServiceEnabled( LPCTSTR szSvcName, BOOL &bEnabled )
{
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
		return E_POINTER;

	LPQUERY_SERVICE_CONFIG lpqscBuf;
	DWORD dwBytesNeeded; 

	CAutoServiceHandle schService = OpenService( m_schSCManager, szSvcName, SERVICE_QUERY_CONFIG);
	if (schService == NULL)
	{ 
		DPRINT("OpenService failed (%d)", GetLastError());
		return E_FAIL;
	}

	lpqscBuf = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, 4096); 
	if (lpqscBuf == NULL) 
	{
		return E_FAIL;
	}

	if (! QueryServiceConfig( schService, lpqscBuf, 4096, &dwBytesNeeded) ) 
	{
		DPRINT("QueryServiceConfig failed (%d)", GetLastError());
	}

	bEnabled = SERVICE_DISABLED != lpqscBuf->dwStartType;
	LocalFree(lpqscBuf);
	return S_OK;
}

HRESULT GetServiceStatus( LPCTSTR pszSrvName, SERVICE_STATUS &status )
{
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
		return E_POINTER;

	CAutoServiceHandle schService = OpenService( m_schSCManager, pszSrvName, SERVICE_ALL_ACCESS);
	if (( SC_HANDLE )NULL == schService)
		return E_FAIL;
	BOOL bRet = QueryServiceStatus(schService, &status);
	return bRet ? S_OK : E_FAIL;
}

BOOL dump_service_info( LPCTSTR szSvcName )
{
	CAutoServiceHandle schService = NULL;
	LPQUERY_SERVICE_CONFIG lpqscBuf = NULL; 
	LPSERVICE_DESCRIPTION lpqscBuf2 = NULL;
	DWORD dwBytesNeeded; 
	BOOL bSuccess=TRUE;
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	if( m_schSCManager == NULL )
		return E_POINTER;

	schService = OpenService( 
		m_schSCManager,         // SCManager database 
		szSvcName,				// name of service 
		SERVICE_QUERY_CONFIG);  // need QUERY access 
	if (schService == NULL)
	{ 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed (%d)\n", GetLastError() ) );
		return FALSE;
	}
	lpqscBuf = (LPQUERY_SERVICE_CONFIG) LocalAlloc( 
		LPTR, 4096); 
	if (lpqscBuf == NULL) 
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "allocate service configure query buffer failed \n" ) );
		return FALSE;
	}

	lpqscBuf2 = (LPSERVICE_DESCRIPTION) LocalAlloc( 
		LPTR, 4096); 
	if (lpqscBuf2 == NULL) 
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "allocate service description query buffer failed \n" ) );
		return FALSE;
	}

	if (! QueryServiceConfig( 
		schService, 
		lpqscBuf, 
		4096, 
		&dwBytesNeeded) ) 
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "QueryServiceConfig failed (%d)\n", GetLastError() ) );
		bSuccess = FALSE; 
	}

	if (! QueryServiceConfig2( 
		schService, 
		SERVICE_CONFIG_DESCRIPTION,
		(LPBYTE)lpqscBuf2, 
		4096, 
		&dwBytesNeeded) ) 
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "QueryServiceConfig2 failed (%d)\n", GetLastError() ) );
		bSuccess = FALSE;
	}

	dbg_print( MSG_INFO, "\\secu_mon configuration: \n");
	dbg_print( MSG_INFO, " Type: 0x%x\n", lpqscBuf->dwServiceType);
	dbg_print( MSG_INFO, " Start Type: 0x%x\n", lpqscBuf->dwStartType);
	dbg_print( MSG_INFO, " Error Control: 0x%x\n", lpqscBuf->dwErrorControl);
	dbg_print( MSG_INFO, " Binary path: %ws\n", lpqscBuf->lpBinaryPathName);

	if (lpqscBuf->lpLoadOrderGroup != NULL)
		dbg_print( MSG_INFO, " Load order group: %ws\n", lpqscBuf->lpLoadOrderGroup);
	if (lpqscBuf->dwTagId != 0)
		dbg_print( MSG_INFO, " Tag ID: %d\n", lpqscBuf->dwTagId);
	if (lpqscBuf->lpDependencies != NULL)
		dbg_print( MSG_INFO, " Dependencies: %ws\n", lpqscBuf->lpDependencies);
	if (lpqscBuf->lpServiceStartName != NULL)
		dbg_print( MSG_INFO, " Start Name: %ws\n", lpqscBuf->lpServiceStartName);
	if (lpqscBuf2->lpDescription != NULL)
		dbg_print( MSG_INFO, " Description: %ws\n", lpqscBuf2->lpDescription);

_return:

	if( lpqscBuf != NULL )
	{
		LocalFree( lpqscBuf ); 
	}

	if( lpqscBuf2 != NULL )
	{
		LocalFree(lpqscBuf2); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, bSuccess ) ); 
	return bSuccess;
}

HRESULT StartSvc( LPCTSTR lpServiceName )
{
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
		return E_POINTER;

	CAutoServiceHandle schService;
	schService = OpenService(m_schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
	if (schService == NULL) {
		return E_FAIL;
	}
	BOOL bRet = StartService(schService,0,NULL);
	return bRet ? S_OK : E_FAIL;
}

HRESULT StopSvc( LPCTSTR lpServiceName )
{
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
		return E_POINTER;

	CAutoServiceHandle schService;
	schService = OpenService(m_schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
	if (schService == NULL) {
		return E_FAIL;
	}

	SERVICE_STATUS ss = {0};
	BOOL bRet = ControlService(schService,SERVICE_CONTROL_STOP,&ss);
	return bRet ? S_OK : E_FAIL;
}

HRESULT EnableService( LPCTSTR szSvcName, BOOL fDisable )
{
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
		return E_POINTER;

	CAutoServiceHandle schService=NULL;
	SC_LOCK sclLock; 
	LPQUERY_SERVICE_LOCK_STATUS lpqslsBuf; 
	DWORD dwBytesNeeded, dwStartType; 
	BOOL bSuccess=TRUE;

	sclLock = LockServiceDatabase(m_schSCManager); 

	if (sclLock == NULL) 
	{ 
		if (GetLastError() != ERROR_SERVICE_DATABASE_LOCKED) 
		{
			DPRINT("Database lock failed (%d)\n", GetLastError()); 
			return E_FAIL;
		}

		lpqslsBuf = (LPQUERY_SERVICE_LOCK_STATUS) LocalAlloc( 
			LPTR, sizeof(QUERY_SERVICE_LOCK_STATUS)+256); 
		if (lpqslsBuf == NULL) 
		{
			DPRINT("LocalAlloc failed (%d)\n", GetLastError()); 
			return E_FAIL;
		}

		if (!QueryServiceLockStatus( 
			m_schSCManager, 
			lpqslsBuf, 
			sizeof(QUERY_SERVICE_LOCK_STATUS)+256, 
			&dwBytesNeeded) ) 
		{
			DPRINT("Query lock status failed (%d)", GetLastError()); 
			return E_FAIL;
		}

		if (lpqslsBuf->fIsLocked) 
			DPRINT("Locked by: %s, duration: %d seconds\n", 
			lpqslsBuf->lpLockOwner, 
			lpqslsBuf->dwLockDuration); 
		else 
			DPRINT("No longer locked\n"); 

		LocalFree(lpqslsBuf); 
	} 

	schService = OpenService( 
		m_schSCManager,           // SCManager database 
		szSvcName,				// name of service 
		SERVICE_CHANGE_CONFIG); // need CHANGE access 
	if (schService == NULL) 
	{
		DPRINT("OpenService failed (%d)\n", GetLastError());
		return E_FAIL;
	}

	dwStartType = (fDisable) ? SERVICE_DISABLED : SERVICE_DEMAND_START; 


	if (! ChangeServiceConfig( 
		schService,        // handle of service 
		SERVICE_NO_CHANGE, // service type: no change 
		dwStartType,       // change service start type 
		SERVICE_NO_CHANGE, // error control: no change 
		NULL,              // binary path: no change 
		NULL,              // load order group: no change 
		NULL,              // tag ID: no change 
		NULL,              // dependencies: no change 
		NULL,              // account name: no change 
		NULL,              // password: no change 
		NULL) )            // display name: no change
	{
		DPRINT("ChangeServiceConfig failed (%d)\n", GetLastError()); 
		bSuccess = FALSE;
	}
	else 
		DPRINT("ChangeServiceConfig succeeded.\n"); 
	UnlockServiceDatabase(sclLock); 
	return bSuccess ? S_OK : E_FAIL;
}

LRESULT change_service_path( LPCSTR service_name, LPCSTR binary_path )
{
	CAutoServiceHandle schService=NULL;
	SC_LOCK sclLock = NULL; 
	LPQUERY_SERVICE_LOCK_STATUSA lpqslsBuf = NULL; 
	DWORD dwBytesNeeded, dwStartType; 
	BOOL bSuccess=TRUE;
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	
	ASSERT( service_name != NULL ); 
	ASSERT( binary_path != NULL ); 

	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if( m_schSCManager == NULL )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "open service manager failed \n" ) );  

		goto _return;
	}

	// Need to acquire database lock before reconfiguring. 
	sclLock = LockServiceDatabase(m_schSCManager); 

	// If the database cannot be locked, report the details. 
	if (sclLock == NULL) 
	{ 
		// Exit if the database is not locked by another process. 

		if( GetLastError() != ERROR_SERVICE_DATABASE_LOCKED ) 
		{
			ret = GetLastError(); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "Database lock failed \n" ) ); 
			goto _return; 
		}

		// Allocate a buffer to get details about the lock. 
		lpqslsBuf = ( LPQUERY_SERVICE_LOCK_STATUSA )LocalAlloc( 
			LPTR, sizeof( QUERY_SERVICE_LOCK_STATUSA ) + 256 ); 
		
		if( lpqslsBuf == NULL ) 
		{
			ret = GetLastError(); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "LocalAlloc failed \n" ) ); 
			goto _return; 
		}

		_ret = QueryServiceLockStatusA( 
			m_schSCManager, 
			lpqslsBuf, 
			sizeof(QUERY_SERVICE_LOCK_STATUSA)+256, 
			&dwBytesNeeded ); 

		// Get and print the lock status information. 
		if( _ret == FALSE ) 
		{
			ret = GetLastError(); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "Query lock status failed\n" ) ); 
			goto _return;
		}

		if( lpqslsBuf->fIsLocked == TRUE ) 
		{
			DPRINT("Locked by: %s, duration: %d seconds\n", 
			lpqslsBuf->lpLockOwner, 
			lpqslsBuf->dwLockDuration); 
		}
		else
		{
			DPRINT("No longer locked\n"); 
		}
	} 

	schService = OpenServiceA( 
		m_schSCManager,           // SCManager database 
		service_name,				// name of service 
		SERVICE_CHANGE_CONFIG); // need CHANGE access 
	if (schService == NULL) 
	{
		DPRINT("OpenService failed (%d)\n", GetLastError());
		return E_FAIL;
	}

	_ret = ChangeServiceConfigA( 
		schService,        // handle of service 
		SERVICE_NO_CHANGE, // service type: no change 
		SERVICE_NO_CHANGE,       // change service start type 
		SERVICE_NO_CHANGE, // error control: no change 
		binary_path,              // binary path: no change 
		NULL,              // load order group: no change 
		NULL,              // tag ID: no change 
		NULL,              // dependencies: no change 
		NULL,              // account name: no change 
		NULL,              // password: no change 
		NULL );  
	
	if( _ret == FALSE )            // display name: no change
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "ChangeServiceConfig failed \n" ) ); 
		goto _return; 
	}
	else 
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "ChangeServiceConfig succeeded.\n" ) ); 
	}

_return:

	if( sclLock != NULL )
	{
		UnlockServiceDatabase( sclLock ); 
	}

	if( lpqslsBuf != NULL )
	{
		LocalFree( lpqslsBuf ); 
	}

	return ret; 
}

LRESULT get_service_exe_name( LPCSTR szSvcName, LPSTR binary_path, ULONG name_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	LPQUERY_SERVICE_CONFIGA lpqscBuf = NULL;
	DWORD dwBytesNeeded; 
	CAutoServiceHandle schService; 
	CAutoServiceHandle m_schSCManager; 

	ASSERT( szSvcName != NULL ); 
	ASSERT( binary_path != NULL && name_len > 0 ); 

	*binary_path = '\0'; 

	m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
	{
		ret = GetLastError(); 
		goto _return; 
	}

	schService = OpenServiceA( m_schSCManager, szSvcName, SERVICE_QUERY_CONFIG);
	if (schService == NULL)
	{ 
		ret = GetLastError(); 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed \n" ) );
		goto _return; 
	}

	lpqscBuf = (LPQUERY_SERVICE_CONFIGA) LocalAlloc(LPTR, 4096); 
	if (lpqscBuf == NULL) 
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_ret = QueryServiceConfigA( schService, lpqscBuf, 4096, &dwBytesNeeded); 
	if( _ret == FALSE ) 
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "QueryServiceConfig failed\n" ) );
		goto _return; 
	}

	if( strlen( lpqscBuf->lpBinaryPathName ) >= name_len )
	{
		ret = ERROR_BUFFER_TOO_SMALL; 
		goto _return; 
	}

	strcpy( binary_path, lpqscBuf->lpBinaryPathName ); 

_return:
	if( lpqscBuf != NULL )
	{
		LocalFree( lpqscBuf );
	}

	return ret ;
}

LRESULT auto_update_service_binary_path( LPCSTR service_name, LPCSTR binary_path )
{
	LRESULT ret = ERROR_SUCCESS; 
	CHAR _binary_path[ MAX_PATH ]; 

	ASSERT( service_name != NULL ); 
	ASSERT( binary_path != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	ret = get_service_exe_name( service_name, _binary_path, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "get service binary path failed \n" ) ); 
		goto _return; 
	}

	if( stricmp( binary_path, _binary_path ) == 0 )
	{
		log_trace( ( MSG_ERROR, "current service path is %s, original service path is %s\n", 
			binary_path, 
			_binary_path ) ); 

		goto _return; 
	}

	ret = change_service_path( service_name, binary_path ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "change the service binary path failed \n" ) ); 
		goto _return; 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

HRESULT UpdateSvcDesc( LPCTSTR lpServiceName, LPCTSTR szDesc )
{
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
		return E_POINTER;

	CAutoServiceHandle schService;
	SERVICE_DESCRIPTION sd = {0};
	schService = OpenService( m_schSCManager, lpServiceName, SERVICE_CHANGE_CONFIG);
	if (schService == NULL)
	{ 
		DPRINT("OpenService failed (%d)\n", GetLastError()); 
		return E_FAIL;
	}

	sd.lpDescription = (LPTSTR)szDesc;
	BOOL bRet = ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sd);

	if( bRet ) 
		DPRINT("Service description updated successfully.\n");
	else
		DPRINT("ChangeServiceConfig2 failed\n");
	return bRet ? S_OK : E_FAIL;
}

HRESULT InstallSvc( LPCTSTR lpServiceName, LPCTSTR lpDisplayName, DWORD dwServiceType, LPCTSTR lpBinaryPathName, LPCTSTR lpServiceStartName, LPCTSTR lpDependencies )
{
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
		return E_POINTER;

	HRESULT hr = S_OK;
	CAutoServiceHandle hService = CreateService(m_schSCManager, lpServiceName, lpDisplayName,
		SERVICE_ALL_ACCESS,
		dwServiceType,
		SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		lpBinaryPathName, NULL, NULL, lpDependencies,
		lpServiceStartName, NULL);

	if (hService == NULL){
		hService = OpenService(m_schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
		if (hService == NULL) {
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT RemoveSvc( LPCTSTR lpServiceName )
{
	CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_schSCManager==NULL)
		return E_POINTER;

	CAutoServiceHandle hService = OpenService(m_schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
	if (hService == NULL) {
		return E_FAIL;
	}
	BOOL bRet = DeleteService(hService);
	return bRet ? S_OK : E_FAIL;
}