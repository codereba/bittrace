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
#include "inst_drv.h"
#include "drv_load.h"

LRESULT WINAPI install_fs_mini_filter_driver( LPCTSTR dev_file_name, 
											 LPCTSTR file_name, 
											 LPCTSTR driver_name, 
											 ULONG driver_file_res, 
											 DWORD flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		_ret = install_driver( dev_file_name, 
			file_name, 
			driver_name, 
			driver_file_res, 
			flags ); 

		if( _ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "install event monitor driver error 0x%0.8x\n", ret ); 
			ret = _ret; 
			//break; 
		}

		{
			FS_MINI_FILTER_CONFIG_PARAM config_param; 

#define BITTRACE_FS_MINI_FILTER_ALTITUDE_START L"363981" 

			config_param.altitude = BITTRACE_FS_MINI_FILTER_ALTITUDE_START; 
			config_param.flags = 0; 

			_ret = config_fs_mini_filter_driver( driver_name, &config_param ); 
			if( _ret != ERROR_SUCCESS )
			{
				ret = _ret; 
				break; 
			}
		}

	}while( FALSE );

	return ret; 
}

LRESULT install_driver( LPCTSTR dev_file_name, 
					   LPCTSTR file_name, 
					   LPCTSTR driver_name, 
					   ULONG driver_file_res, 
					   DWORD flags )
{	
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR driver_file_name[ MAX_PATH ];
	PVOID old_wow_value = NULL; 
	BOOLEAN wow64_disabled = FALSE; 


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
	ASSERT( NULL != file_name && 
		NULL != driver_name );

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	log_trace( ( MSG_INFO, "check %ws existing \n", driver_file_name ) ); 
	ret = file_exists( file_name, !!( flags & DRIVER_IN_SYSTEM_DIR ) ); 
	if( ERROR_SUCCESS != ret )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( driver_file_res ), DAT_FILE_RES_TYPE, file_name, !!( flags & DRIVER_IN_SYSTEM_DIR ) ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create the %ws from resource failed \n", driver_file_name ) ); 
			goto _return; 
		}
	}

	if( DRIVER_IN_SYSTEM_DIR & flags )
	{
		ret = add_sys_drv_path_to_file_name( driver_file_name, MAX_PATH, file_name, TRUE ); 
		if( ret != ERROR_SUCCESS ) 
		{
			log_trace( ( MSG_ERROR, "check the driver file in system dir failed \n" ) ); 
			goto _return; 
		}
	}
	else
	{
		ret = add_app_path_to_file_name( driver_file_name, MAX_PATH, file_name, TRUE ); 

		if( ret != ERROR_SUCCESS ) 
		{
			log_trace( ( MSG_INFO, "check the driver file in app dir failed" ) );
			goto _return; 
		}
	}

	log_trace( ( MSG_INFO, "the driver file path is %ws \n", driver_file_name ) ); 

	ret = manage_drv( driver_name,
		driver_file_name,
		DRIVER_FUNC_INSTALL, 
		flags
		); 

	log_trace( ( MSG_INFO, "install driver return 0x%0.8x \n", ret ) ); 

	if( ret != ERROR_SUCCESS ) 
	{
		log_trace( ( MSG_INFO, "install driver %ws in %ws failed\n", driver_name, driver_file_name ) ); 
		if( 0 == ( flags & DO_NOT_AUTO_UNINSTALL_DRIVER ) )
		{
			manage_drv( driver_name,
				driver_file_name,
				DRIVER_FUNC_REMOVE, 
				flags
				);
		}
		goto _return; 
	}

_return:

#ifdef _WIN32
	if( wow64_disabled == TRUE )
	{
		revert_wow_64( old_wow_value ); 
	}
#endif //_WIN32

	log_trace( ( MSG_INFO, "leave %s return 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT safe_install_driver( LPCTSTR dev_file_name, 
							LPCTSTR file_name, 
							LPCTSTR driver_name, 
							ULONG driver_file_res, 
							DWORD flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = install_driver( dev_file_name, 
			file_name, 
			driver_name, 
			driver_file_res, 
			flags ); 

		if( ret != ERROR_SUCCESS )
		{
			LRESULT _ret; 

			_ret = delete_drv_file( file_name, TRUE ); 
			if( _ret != ERROR_SUCCESS )
			{
				log_trace( ( DBG_DBG_BUF_OUT, "delete the driver file %ws error 0x%0.8x\n", file_name, ret ) ); 
				ASSERT( FALSE && "delete the fs manage driver files error " ); 
			}

			ret = install_driver( dev_file_name, 
				file_name, 
				driver_name, 
				driver_file_res, 
				flags ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT test_load_device( LPCTSTR dev_name )
{
	HANDLE dev_handle; 
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	dev_handle = CreateFile(
		dev_name,             // pointer to name of the file
		0,                      // access (read-write) mode
		0,                      // share mode
		NULL,                   // pointer to security attributes
		OPEN_EXISTING,          // how to create
		FILE_ATTRIBUTE_NORMAL,  // file attributes
		NULL                    // handle to file with attributes to 
		); 

	if( dev_handle != INVALID_HANDLE_VALUE ) 
	{
		CloseHandle( dev_handle ); 
		goto _return; 
	}

	ret = GetLastError(); 

	log_trace( ( DBG_MSG_AND_ERROR_OUT, "test device exist error is not file no exist: 0x%0.8x\n", ret ) ); 

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT load_drv_dev( HANDLE *dev_load, LPCTSTR dev_file_name, LPCTSTR driver_name, DWORD flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE hDevice;              // handle to a device, file, or directory 

	ASSERT( NULL != dev_file_name && 
		NULL != driver_name );
	ASSERT( dev_load != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	*dev_load = NULL; 

	hDevice = CreateFile(
		dev_file_name,             // pointer to name of the file
		0x10000000,                      // access (read-write) mode
		0,                      // share mode
		NULL,                   // pointer to security attributes
		OPEN_EXISTING,          // how to create
		FILE_ATTRIBUTE_NORMAL,  // file attributes
		NULL                    // handle to file with attributes to 
		); 

	if( hDevice != INVALID_HANDLE_VALUE ) 
	{
		*dev_load = hDevice; 
		goto _return; 
	}

	log_trace( ( DBG_MSG_AND_ERROR_OUT, "load driver %ws failed \n", dev_file_name ) ); 
	SAFE_SET_ERROR_CODE( ret ); 

_return:
	log_trace( ( MSG_INFO, "leave %s return 0x%0.8x \n", __FUNCTION__, ret ) ); 
	return ret;
}

LRESULT _uninstall_driver( LPCTSTR driver_name, DWORD flags )
{
	LRESULT ret = ERROR_SUCCESS;

	ASSERT( NULL != driver_name );

	ret = manage_drv( driver_name,
		NULL,
		DRIVER_FUNC_REMOVE, 
		flags
		);

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret;
}
