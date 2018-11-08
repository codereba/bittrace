/*++
Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    install.c

Abstract:

    Win32 routines to dynamically load and unload a Windows NT kernel-mode
    driver using the Service Control Manager APIs.

Environment:

    User mode only

Author:

    Steve Dziok

Revision History:

    01-Jul-1996:    Created

--*/

#include "common_func.h"
#include "wdk_macros.h"
#include <windows.h>
#include <stdio.h>

#include <string.h>
#include <setupapi.h>
#include "inst_drv.h"

#include <atlbase.h>
INT32 WINAPI config_fs_mini_filter_driver( LPCWSTR service_name, 
									 FS_MINI_FILTER_CONFIG_PARAM *config_param )
{
	INT32 ret = 0; 
	ULONG error_code; 
	LSTATUS _ret; 
	WCHAR key_name[ MAX_PATH ] = { 0 }; 
	WCHAR value_name[ MAX_PATH ] = { 0 };;

	do 
	{
		CRegKey reg; 

		if( config_param == NULL )
		{
			log_trace( ( MSG_ERROR, "%s invalid paramenter", __FUNCTION__ ) );

			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if(  config_param->altitude == NULL )
		{
			log_trace( ( MSG_ERROR, "%s invalid parameter", __FUNCTION__ ) );

			ret = ERROR_INVALID_PARAMETER; 
			break;
		}

		if( service_name == NULL )
		{
			log_trace( ( MSG_ERROR, "%s invalid parameter", __FUNCTION__ ) );

			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
#if (NTDDI_VERSION == NTDDI_WIN2K)

		_ret = reg.Open(HKEY_LOCAL_MACHINE,_T("SYSTEM\\CurrentControlSet\\Services\\FltMgr"),KEY_READ); 

		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "%s check FltMgr driver status error %u", __FUNCTION__, _ret ) ); 

			ret = _ret; 
			break; 
		}

		reg.Close(); 
#endif //(NTDDI_VERSION == NTDDI_WIN2K)

		StringCbPrintfW(key_name,sizeof(key_name),L"SYSTEM\\CurrentControlSet\\Services\\%s",service_name );

		if(reg.Create(HKEY_LOCAL_MACHINE,key_name)!=ERROR_SUCCESS)
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_ERROR, "%s make registry key %ws error 0x%08x", __FUNCTION__, key_name, error_code ) );

			ret = _ret; 
			break; 
		}

		reg.Close();

		StringCbPrintfW(key_name,sizeof(key_name), L"SYSTEM\\CurrentControlSet\\Services\\%s\\Instances",service_name );

		if(reg.Create(HKEY_LOCAL_MACHINE,key_name)!=ERROR_SUCCESS)
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_ERROR, "%s make registry key %ws error 0x%08x", __FUNCTION__, key_name, error_code ) );

			ret = _ret; 
			break; 
		}

		StringCbPrintfW(value_name,sizeof(value_name),L"%s Instance", service_name);

		_ret = reg.SetStringValue(L"DefaultInstance",value_name);
		reg.Close();
		if( _ret != ERROR_SUCCESS )
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_ERROR, "%s make the default instance value %ws of the xdhy update driver error 0x%08x", __FUNCTION__, value_name, error_code ) );

			ret = _ret; 
			break; 
		}

		StringCbPrintfW(key_name,sizeof(key_name),_T("SYSTEM\\CurrentControlSet\\Services\\%s\\Instances\\%s Instance"), service_name, service_name ); 


		_ret = reg.Create(HKEY_LOCAL_MACHINE,key_name); 

		if( _ret != ERROR_SUCCESS )
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_ERROR, "%s make the default instance key %ws of the xdhy update driver error 0x%08x", __FUNCTION__, service_name, error_code ) );

			ret = _ret; 

			break; 
		}

		reg.Close();

		StringCbPrintfW(key_name,sizeof(key_name),L"SYSTEM\\CurrentControlSet\\Services\\%s\\Instances\\%s Instance",service_name,service_name );

		_ret = reg.Create(HKEY_LOCAL_MACHINE,key_name); 

		if( _ret != ERROR_SUCCESS)
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_ERROR, "%s make registry key %ws error 0x%08x", __FUNCTION__, key_name, error_code ) );

			ret = _ret; 
			break; 
		}

		do 
		{
			_ret = reg.SetStringValue(_T("Altitude"), config_param->altitude );
			if( _ret != ERROR_SUCCESS )
			{
				error_code = GetLastError(); 

				log_trace( ( MSG_ERROR, "%s make the configuration value of the default instance %ws of the xdhy update driver error 0x%08x", __FUNCTION__, L"Altitude", error_code ) );

				ret = _ret; 

				break; 
			}

			_ret = reg.SetDWORDValue( _T("Flags"), config_param->flags ); 
			if( _ret != ERROR_SUCCESS )
			{
				error_code = GetLastError(); 

				log_trace( ( MSG_ERROR, "%s make the configuration value of the default instance %ws of the xdhy update driver error 0x%08x", __FUNCTION__, L"Flags", error_code ) );

				ret = _ret; 

				break; 

			}
		}while( FALSE ); 

		reg.Close();

	}while( FALSE );

	return ret;
}

LRESULT setup_drv_from_inf( LPCWSTR file_name, LPCWSTR section_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR setup_cmd[ MAX_SETUP_CMD_LEN ]; 
	INT32 _ret; 

	do 
	{
		if( section_name == NULL )
		{
			section_name = DEF_INSTALL_SECTION_NAME; 
		}

		_ret = _snwprintf( setup_cmd, ARRAY_SIZE( setup_cmd ), L"%s 132 %s", section_name, file_name ); 
		if( _ret < 0 )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			break; 
		}

		{
			HRESULT hr; 
			STARTUPINFOW                     si = {0};
			PROCESS_INFORMATION              pi = {0};

			hr = StringCbPrintfW( setup_cmd, sizeof( setup_cmd ), L"RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection %s 132 %s", section_name, file_name ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			si.cb = sizeof (si);

			if (!CreateProcessW (NULL, setup_cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
			{
				log_trace( ( MSG_FATAL_ERROR, "execute rundll for install error" ) );
				SAFE_SET_ERROR_CODE( ret );
				break; 
			}

			DWORD dwRet = WaitForSingleObject(pi.hProcess,INFINITE);
			if (dwRet != WAIT_OBJECT_0)
			{
				SAFE_SET_ERROR_CODE( ret );
				break; 
			}
		}

	}while( FALSE );
	
	return ret; 
}

#define SET_MINI_FILTER_PARAM 0x00000001
#define CREATE_DRIVER_SERVICE 0x00000002
LRESULT install_mini_filter_driver_ex( LPCWSTR driver_sys_path, 
									  LPCWSTR driver_name, 
									  LPCWSTR service_name, 
									  MINI_FILTER_PARAM *update_param, 
									  ULONG flags )
{
	INT32 ret = 0; 
	ULONG error_code; 
	LSTATUS _ret; 
	SC_HANDLE scm_handle = NULL; 
	SC_HANDLE service_handle = NULL; 
	HRESULT hr;  
	WCHAR _service_name[ MAX_PATH ] = { 0 }; 
	ULONG dest_path_len; 
	HKEY reg_key = NULL; 

	do 
	{
		if( update_param == NULL )
		{
			log_trace( ( MSG_FATAL_ERROR, "%s invalid paramenter", __FUNCTION__ ) );

			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( 0 != ( ( CREATE_DRIVER_SERVICE | SET_MINI_FILTER_PARAM ) & flags ) )
		{
			if(  update_param->altitude == NULL )
			{
				log_trace( ( MSG_ERROR, "%s invalid parameter", __FUNCTION__ ) );

				ret = ERROR_INVALID_PARAMETER; 
				break;
			}

			if( update_param->start_type != SERVICE_BOOT_START 
				&& update_param->start_type != SERVICE_SYSTEM_START 
				&& update_param->start_type != SERVICE_AUTO_START
				&& update_param->start_type != SERVICE_DEMAND_START )
			{
				log_trace( ( MSG_FATAL_ERROR, "%s invalid paramenter", __FUNCTION__ ) );

				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}

		if( driver_sys_path == NULL )
		{
			log_trace( ( MSG_FATAL_ERROR, "%s invalid paramenter", __FUNCTION__ ) );

			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( driver_name == NULL )
		{
			log_trace( ( MSG_FATAL_ERROR, "%s invalid paramenter", __FUNCTION__ ) );

			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( service_name == NULL )
		{
			log_trace( ( MSG_FATAL_ERROR, "%s invalid paramenter", __FUNCTION__ ) );

			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_ret = RegOpenKeyExW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\FltMgr", 0, KEY_READ, &reg_key ); 

		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_FATAL_ERROR, "%s check FltMgr driver status error %u", __FUNCTION__, _ret ) ); 

			ret = _ret; 
			break; 
		}

		RegCloseKey( reg_key ); 
		reg_key = NULL; 


		scm_handle = OpenSCManager ( NULL, NULL, SC_MANAGER_ALL_ACCESS );

		if( scm_handle == NULL )
		{
			log_trace( ( MSG_FATAL_ERROR, "%s OpenSCManager for install file driver error return 0x%08x", __FUNCTION__, GetLastError() ) ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		{
			StringCbCopyW( _service_name, sizeof( service_name ), service_name ); 
		}

		if( CREATE_DRIVER_SERVICE & flags )
		{
			service_handle = CreateServiceW (
				scm_handle,                 // handle to SC manager
				_service_name,         // name of service
				_service_name,        // display name
				SERVICE_ALL_ACCESS,     // access mask
				SERVICE_KERNEL_DRIVER,  // service type
				update_param->start_type,   // start type
				SERVICE_ERROR_NORMAL,   // error control
				driver_sys_path,           // full path to driver
				L"FSFilter Encryption", // load ordering
				NULL,                   // tag id
				L"FltMgr",           // dependency
				NULL,                   // account name
				NULL                    // password
				);

			if( service_handle == NULL ) 
			{
				error_code = GetLastError(); 

				log_trace( ( MSG_FATAL_ERROR, "%s CreateService for install driver %ws error return 0x%08x", __FUNCTION__, driver_sys_path, error_code ) );

				switch( error_code )
				{
				case ERROR_SERVICE_EXISTS: 
					break; 
				default:
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				if( ret != 0 )
				{
					break; 
				}
			}
			else
			{
				CloseServiceHandle( service_handle ); 
				service_handle = NULL; 
			}
		}

		if( 0 == ( ( CREATE_DRIVER_SERVICE | SET_MINI_FILTER_PARAM ) & flags ) )
		{
			break; 
		}

		WCHAR KeyName[256] ={0};
		StringCbPrintfW(KeyName,sizeof(KeyName),L"SYSTEM\\CurrentControlSet\\Services\\%s",_service_name);

		_ret = RegCreateKeyW( HKEY_LOCAL_MACHINE,KeyName, &reg_key ); 
		if( _ret !=ERROR_SUCCESS )
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_FATAL_ERROR, "%s make registry key %ws error 0x%08x", __FUNCTION__, KeyName, error_code ) );

			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		RegCloseKey( reg_key ); 
		reg_key = NULL; 

		StringCbPrintfW(KeyName,sizeof(KeyName), L"SYSTEM\\CurrentControlSet\\Services\\%s\\Instances",_service_name);

		WCHAR KeyValue[256] ={0};
		_ret = RegCreateKeyW( HKEY_LOCAL_MACHINE, KeyName, &reg_key ); 

		if( _ret != ERROR_SUCCESS ) 
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_FATAL_ERROR, "%s make registry key %ws error 0x%08x", __FUNCTION__, KeyName, error_code ) );

			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		StringCbPrintfW( KeyValue,sizeof( KeyValue ), L"%s Instance", _service_name ); 

		_ret = RegSetValueW( reg_key, L"DefaultInstance", REG_SZ, KeyValue, ( ( wcslen( KeyValue ) + 1 ) << 1 ) );
		RegCloseKey( reg_key ); 
		reg_key = NULL; 
		if( _ret != ERROR_SUCCESS )
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_FATAL_ERROR, "%s make the default instance value %ws of the xdhy update driver error 0x%08x", __FUNCTION__, KeyValue, error_code ) );

			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		StringCbPrintfW( KeyName, sizeof( KeyName ), L"SYSTEM\\CurrentControlSet\\Services\\%s\\Instances\\%s Instance", _service_name, _service_name ); 

		_ret = RegCreateKeyW( HKEY_LOCAL_MACHINE, KeyName, &reg_key ); 

		if( _ret != ERROR_SUCCESS )
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_FATAL_ERROR, "%s make the default instance key %ws of the xdhy update driver error 0x%08x", __FUNCTION__, _service_name, error_code ) );

			ret = _ret; 

			break; 
		}

		RegCloseKey( reg_key ); 
		reg_key = NULL; 

		StringCbPrintfW( KeyName, sizeof( KeyName ), L"SYSTEM\\CurrentControlSet\\Services\\%s\\Instances\\%s Instance", _service_name, _service_name ); 

		_ret = RegCreateKeyW( HKEY_LOCAL_MACHINE, KeyName, &reg_key ); 

		if( _ret != ERROR_SUCCESS)
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_FATAL_ERROR,  "%s make registry key %ws error 0x%08x", __FUNCTION__, KeyName, error_code ) );

			ret = _ret; 
			break; 
		}

		do 
		{
			_ret = RegSetValueW( reg_key, L"Altitude", REG_SZ, update_param->altitude, ( ( wcslen( update_param->altitude ) + 1 ) << 1 ) );

			if( _ret != ERROR_SUCCESS )
			{
				error_code = GetLastError(); 

				log_trace( ( MSG_FATAL_ERROR, "%s make the configuration value of the default instance %ws of the xdhy update driver error 0x%08x", __FUNCTION__, L"Altitude", error_code ) );

				ret = _ret; 

				break; 
			}

			_ret = RegSetValueW( reg_key, L"Flags", REG_DWORD, 0, sizeof( DWORD ) ); 

			if( _ret != ERROR_SUCCESS )
			{
				error_code = GetLastError(); 

				log_trace( ( MSG_FATAL_ERROR, "%s make the configuration value of the default instance %ws of the xdhy update driver error 0x%08x", __FUNCTION__, L"Flags", error_code ) );

				ret = _ret; 

				break; 

			}
		}while( FALSE ); 

		RegCloseKey( reg_key ); 
		reg_key = NULL; 

		if( ret != 0 )
		{
			break; 
		}

		service_handle = OpenService( scm_handle, _service_name, SERVICE_ALL_ACCESS );
		if( service_handle == NULL ) 
		{
			error_code = GetLastError(); 

			log_trace( ( MSG_FATAL_ERROR, "%s open the created service %ws of driver error 0x%08x", __FUNCTION__, _service_name, error_code ) );

			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );


	if( NULL != service_handle )
	{
		CloseServiceHandle( service_handle ); 
	}

	if( NULL != scm_handle )
	{
		CloseServiceHandle( scm_handle ); 
	}

	return ret;
}

LRESULT install_drv(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    driver_name,
    IN LPCTSTR    service_exe, 
	IN ULONG Flags
    )
{
	LRESULT ret = ERROR_SUCCESS; 
    SC_HANDLE   schService;
	DWORD start_mode; 

    //
    // NOTE: This creates an entry for a standalone driver. If this
    //       is modified for use with a driver that requires a Tag,
    //       Group, and/or Dependencies, it may be necessary to
    //       query the registry for existing driver information
    //       (in order to determine a unique Tag, etc.).
    //

    //
    // Create a new a service object.
    //

	ASSERT( service_exe != NULL ); 
	ASSERT( driver_name != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	log_trace( ( MSG_INFO, "%s driver name %ws sys file path %ws\n", __FUNCTION__, driver_name, service_exe ) ); 


	if( Flags & DRIVER_BOOT_START )
	{
		start_mode = SERVICE_BOOT_START; 
	}
	else if( Flags & DRIVER_AUTO_START )
	{
		//DBG_BP(); 
		start_mode = SERVICE_AUTO_START; 
	}
	else if( Flags & DRIVER_MANUAL_START )
	{
		//DBG_BP(); 
		start_mode = SERVICE_DEMAND_START; 
	}
	else
	{
		start_mode = SERVICE_SYSTEM_START; 
	}

    schService = CreateService( SchSCManager,           // handle of service control manager database
                               driver_name,             // address of name of service to start
                               driver_name,             // address of display name
                               SERVICE_ALL_ACCESS,     // type of access to service
                               SERVICE_KERNEL_DRIVER,  // type of service
                               start_mode,   // when to start service
                               SERVICE_ERROR_NORMAL,   // severity if service fails to start
                               service_exe,             // address of name of binary file
                               NULL,                   // service does not belong to a group
                               NULL,                   // no tag requested
                               NULL,                   // no dependency names
                               NULL,                   // use LocalSystem account
                               NULL                    // no password for service account
                               );

    if( schService != NULL )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "Service %ws installed\n", driver_name ) ); 

		goto _return; 
	}
	
	ret = GetLastError();

	if( ret == ERROR_SERVICE_EXISTS ) 
	{
		//
		// Ignore this error.
		//
		log_trace( ( MSG_WARNING, "driver already install\n" ) ); 
		ret = ERROR_SUCCESS; 
		goto _return; 
	}
	else if( ret == ERROR_SERVICE_MARKED_FOR_DELETE ) 
	{
		//
		// Previous instance of the service is not fully deleted so sleep
		// and try again.
		//
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "Previous instance of the service is not fully deleted. Try again...\n") );
		goto _return; 
	}
	else 
	{

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "CreateService failed! \n" ) );

		//
		// Indicate an error.
		//

		goto _return; 
	}

_return:
	if( schService != NULL )
	{
        CloseServiceHandle( schService ); 
    }

    return ret;
}   // install_drv

LRESULT manage_drv(
    IN LPCTSTR  driver_name,
    IN LPCTSTR  file_name,
    IN USHORT   Function, 
	IN ULONG Flags
    )
{

	LRESULT ret = ERROR_SUCCESS;
    SC_HANDLE   schSCManager;

    //
    // Insure (somewhat) that the driver and service names are valid.
    //

    ASSERT( driver_name != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
    //
    // Connect to the Service Control Manager and open the Services database.
    //

    schSCManager = OpenSCManager( NULL,                   // local machine
                                 NULL,                   // local database
                                 SC_MANAGER_ALL_ACCESS   // access required
                                 ); 

    if( schSCManager == NULL ) 
	{
		ret = GetLastError(); 
        log_trace( ( DBG_MSG_AND_ERROR_OUT, "Open SC Manager failed! \n" ) ); 
		goto _return; 
    }

    //
    // Do the requested function.
    //

	switch( Function ) 
	{
	case DRIVER_FUNC_INSTALL: 
		ASSERT( file_name != NULL ); 

		ret = install_drv( schSCManager, 
					driver_name, 
					file_name, 
					Flags ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		if( 0 == ( DRIVER_INSTALL_DO_NOT_START & Flags ) )
		{
			ret = start_drv( schSCManager, driver_name ); 
		}
	break;

	case DRIVER_FUNC_REMOVE:


		log_trace( ( MSG_INFO, "uninstall the driver \n" ) ); 

		if( 0 == ( DRIVER_UNINSTALL_DO_NOT_STOP & Flags ) )
		{

			ret = stop_driver( schSCManager, 
				driver_name );

			log_trace( ( MSG_INFO, "stop driver return 0x%0.8x\n", ret ) ); 
		}

		ret = remove_drv( schSCManager, 
					driver_name ); 

		//
		// Ignore all errors.
		//

		log_trace( ( MSG_INFO, "remove driver return 0x%0.8x\n", ret ) ); 
		break;

	default:

		log_trace( ( MSG_ERROR, "!!!Unknown manage_drv() function. \n" ) ); 
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		break;
	}

_return:
	if( schSCManager = NULL ) 
	{
		CloseServiceHandle( schSCManager ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x \n", __FUNCTION__, ret ) ); 
	return ret;
}   // manage_drv


LRESULT remove_drv(
    IN SC_HANDLE    SchSCManager,
    IN LPCTSTR      driver_name
    )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
    SC_HANDLE   schService = NULL;

    //
    // Open the handle to the existing service.
    //

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

    schService = OpenService(SchSCManager,
                             driver_name,
                             SERVICE_ALL_ACCESS
                             );

    if( schService == NULL ) 
	{
		ret = GetLastError(); 

        log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed!\n" ) );
        return ret;
    }

	log_trace( ( MSG_INFO, "OpenService %ws ok\n", driver_name ) );

    //
    // Mark the service for deletion from the service control manager database.
    //

	_ret = DeleteService( schService ); 
    if( _ret == FALSE )
	{
		ret = GetLastError(); 
		if( ret == ERROR_SERVICE_MARKED_FOR_DELETE )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "DeleteService %ws already deleted\n", driver_name ) );

			ret = ERROR_SUCCESS; 
		}
		else
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "DeleteService %ws failed!\n", driver_name ) );
			goto _return; 
		}
	}

	log_trace( ( MSG_INFO, "DeleteService ok!\n" ) ); 

_return:
    if( schService != NULL ) 
	{
        CloseServiceHandle( schService ); 
    }

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
    return ret;
}   // remove_drv

LRESULT start_drv( IN SC_HANDLE SchSCManager, 
				  IN LPCTSTR driver_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
    SC_HANDLE schService; 

    schService = OpenService( SchSCManager, 
							driver_name,
							SERVICE_ALL_ACCESS );

    if( schService == NULL ) 
	{
		ret = GetLastError(); 

        log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed! \n" ) );
        
		goto _return; 
    }

    //
    // Start the execution of the service (i.e. start the driver).
    //

	_ret = StartService( schService,     // service identifier
						0,              // number of arguments
						NULL            // pointer to arguments
						); 
    if( _ret == FALSE ) 
	{
        ret = GetLastError();

        if( ret == ERROR_SERVICE_ALREADY_RUNNING )
		{
            ret = ERROR_SUCCESS; 
        } 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "StartService failure!\n" ) );

		//
		// Indicate failure.  Fall through to properly close the service handle.
		//

		goto _return; 
    }

_return:
    //
    // Close the service object.
    //

    if( schService != NULL ) 
	{
        CloseServiceHandle(schService);
    }

    return ret;
}   // start_drv



LRESULT stop_driver(
    IN SC_HANDLE    SchSCManager,
    IN LPCTSTR      driver_name
    )
{
    LRESULT         ret = ERROR_SUCCESS; 
	BOOL _ret; 
    SC_HANDLE       schService;
    SERVICE_STATUS  serviceStatus;

    //
    // Open the handle to the existing service.
    //

	ASSERT( driver_name != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

    schService = OpenService(SchSCManager,
                             driver_name,
                             SERVICE_ALL_ACCESS
                             );

    if( schService == NULL )
	{
		ret = GetLastError(); 
	
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService %ws failed! \n", driver_name ) );

        goto _return; 
    }

	log_trace( ( MSG_INFO, "OpenService %ws ok! \n", driver_name ) ); 
	; 
    //
    // Request that the service stop.
    //

	_ret = ControlService( schService, 
		SERVICE_CONTROL_STOP, 
		&serviceStatus ); 

    if( _ret == FALSE )
	{
		ret = GetLastError(); 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "stop service failed! \n" ) ); 
	} 

_return:
    if( schService != NULL )
	{
        CloseServiceHandle( schService ); 
    }

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

    return ret;
}   //  stop_driver

LRESULT delete_drv_file( LPCTSTR pszSysFile, INT32 is_sys_path )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	TCHAR szOutFullPath[ MAX_PATH * 2 ];
	BOOLEAN wow64_disabled = FALSE; 
	PVOID old_wow_value = NULL; 

#ifdef _WIN32
	{
		LRESULT _ret; 
		_ret = safe_disable_wow64( &old_wow_value ); 
		if( _ret == ERROR_SUCCESS )
		{
			wow64_disabled = TRUE; 
		}
	}
#endif //_WIN32


	ASSERT( pszSysFile != NULL ); 

	if( is_sys_path == FALSE )
	{
		ret = add_app_path_to_file_name( szOutFullPath, MAX_PATH, pszSysFile, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}
	else
	{
		ret = add_sys_drv_path_to_file_name( szOutFullPath, MAX_PATH, pszSysFile, TRUE ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}
	_ret = DeleteFile( szOutFullPath );
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
#ifdef _WIN32
	if( wow64_disabled == TRUE )
	{
		revert_wow_64( old_wow_value ); 
	}
#endif //_WIN32


	return ret;
}
