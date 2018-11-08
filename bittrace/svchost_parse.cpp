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

#ifdef _DEBUG_MEM_LEAKS
#undef _DEBUG_MEM_LEAKS
#endif //_DEBUG_MEM_LEAKS

#include "common_func.h"


#define SERVICES_REGIGRY_PATH L"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services"
#include "svchost_parse.h"

LRESULT WINAPI get_all_process_shared_service( vector< PROCESS_SHARED_SERVICE_INFORMATION* > *service_infos )
{
	LRESULT ret = ERROR_SUCCESS; 
	LSTATUS _ret = ERROR_SUCCESS; 
	ULONG __ret; 
	ULONG service_type; 
	ULONG i; 
	HRESULT hr; 
	DWORD SubKeyCount = 0; 
	DWORD MaxSubKeyNameLen; 
	HKEY ServicesKey = NULL; 
	HKEY param_key = NULL; 
	HKEY SubServiceKey = NULL;  

	WCHAR SubKeyName[ MAX_PATH ] ={ 0 }; 
	DWORD ccSubKeyNameLen;
	TCHAR SubKeyPath[ MAX_PATH ]={ 0 }; 
	DWORD ValueSize; 
	ULONG ValueType; 
	LPWSTR service_dll = NULL; 
	PROCESS_SHARED_SERVICE_INFORMATION *service_info = NULL; 

#pragma  region 

	do 
	{
		ASSERT( NULL != service_infos ); 

		_ret = RegOpenKeyExW( HKEY_LOCAL_MACHINE, 
			L"SYSTEM\\CurrentControlSet\\Services", 
			0, 
			KEY_ALL_ACCESS, 
			&ServicesKey ); 

		if( _ret != ERROR_SUCCESS ) 
		{ 
			ret = _ret; 
			log_trace( ( MSG_FATAL_ERROR,  "open the key represent the services root error 0x%0.8x", _ret ) ); 

			break; 
		}

		_ret = RegQueryInfoKeyW(
			ServicesKey,
			NULL, 
			NULL,  
			NULL,
			&SubKeyCount,
			&MaxSubKeyNameLen, 
			NULL,
			NULL, 
			NULL, 
			NULL, 
			NULL,
			NULL ); 

		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
			log_trace( ( MSG_FATAL_ERROR,  "query the information of the key represent the services root error 0x%0.8x", _ret ) ); 

			break; 
		}

		if( SubKeyCount == 0 )
		{
			ret = _ret; 
			break; 
		}

		service_dll = ( LPWSTR )malloc( ( MAX_PATH << 1 ) ); 

		if( NULL == service_dll )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		for( i = 0; i < SubKeyCount; i++ ) 
		{ 
			ccSubKeyNameLen = ARRAYSIZE( SubKeyName );

			do 
			{
				_ret = RegEnumKeyExW( ServicesKey, 
					i,
					SubKeyName, 
					&ccSubKeyNameLen, 
					NULL, 
					NULL, 
					NULL, 
					NULL); 

				if( _ret == ERROR_NO_MORE_ITEMS )
				{
					break;
				}

				if( _ret != ERROR_SUCCESS ) 
				{
					log_trace( ( MSG_FATAL_ERROR,  "enumerate the key name of the service (no:%u) error 0x%0.8x", i, _ret ) ); 
					break; 
				}

				ASSERT( wcslen( SubKeyName ) == ccSubKeyNameLen ); 

				hr = StringCbPrintfW( SubKeyPath, sizeof( SubKeyPath ), L"SYSTEM\\CurrentControlSet\\Services\\%s", SubKeyName ); 
				if( FAILED( hr ) )
				{
					break; 
				}

				_ret = RegOpenKeyExW( HKEY_LOCAL_MACHINE, 
					SubKeyPath, 
					0, 
					KEY_ALL_ACCESS, 
					&SubServiceKey ); 

				if( _ret != ERROR_SUCCESS ) 
				{
					log_trace( ( MSG_FATAL_ERROR,  "open the key of the service %s error 0x%0.8x", SubKeyName, _ret ) ); 

					break; 
				}

#define SERVICE_IMAGE_PATH_VALUE_NAME L"ImagePath"
#define SERVICE_TYPE_VALUE_NAME L"Type"

				ValueSize = sizeof( service_type ); 

				_ret = RegQueryValueExW( SubServiceKey, 
					SERVICE_TYPE_VALUE_NAME, 
					NULL, 
					&ValueType, 
					( LPBYTE )&service_type, 
					&ValueSize ); 

				if( _ret != ERROR_SUCCESS )
				{
					if( _ret == ERROR_MORE_DATA )
					{
						log_trace( ( MSG_FATAL_ERROR,  "the type of the service %s too long (bigger than %u)", SubKeyName, sizeof( service_type ) ) ); 
					}
					else
					{						
						log_trace( ( MSG_FATAL_ERROR,  "receive the type of the service %s error", SubKeyName ) ); 
					}
					break; 
				}

				if( 0 == ( SERVICE_WIN32_SHARE_PROCESS & service_type ) )
				{
					ret = _ret; 
					break; 
				}

				service_info = ( PROCESS_SHARED_SERVICE_INFORMATION* )malloc( sizeof( PROCESS_SHARED_SERVICE_INFORMATION ) ); 
				if( service_info == NULL )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}

				*service_info->image_path = L'\0'; 
				*service_info->name = L'\0'; 
				*service_info->service_dll = L'\0'; 

				ValueSize = MAX_PATH * sizeof( WCHAR ); 

				_ret = RegQueryValueExW( SubServiceKey, 
					SERVICE_IMAGE_PATH_VALUE_NAME , 
					NULL, 
					&ValueType, 
					( LPBYTE )service_dll, 
					&ValueSize ); 

				if( _ret != ERROR_SUCCESS )
				{
					if( _ret == ERROR_MORE_DATA )
					{
						log_trace( ( MSG_FATAL_ERROR,  "the path of the image file of the service %s too long (bigger than %u)", SubKeyName, ARRAYSIZE( service_info->image_path ) ) ); 
					}
					else
					{						
						log_trace( ( MSG_FATAL_ERROR,  "receive the path of the image file of the service %s error", SubKeyName ) ); 
					}
					break; 
				}

				__ret = ExpandEnvironmentStringsW( ( LPCWSTR )service_dll, 
					service_info->image_path, 
					ARRAYSIZE( service_info->image_path ) ); 

				if( 0 == __ret 
					|| __ret >= ARRAYSIZE( service_info->image_path ) )
				{
					ret = GetLastError(); 
					break; 
				}

				service_info->image_path[ ARRAYSIZE( service_info->image_path ) - 1 ] = L'\0'; 

				do
				{
#define SERVICE_DLL_VALUE_NAME L"ServiceDll"

					_ret = RegOpenKeyExW( SubServiceKey, 
						L"Parameters", 
						0, 
						KEY_ALL_ACCESS, 
						&param_key ); 

					if( _ret != ERROR_SUCCESS ) 
					{
						log_trace( ( MSG_FATAL_ERROR,  "open the key of the service %s error 0x%0.8x", SubKeyName, _ret ) ); 

						break; 
					}

					ValueSize = MAX_PATH * sizeof( WCHAR ); 

					_ret = RegQueryValueExW( param_key, 
						SERVICE_DLL_VALUE_NAME , 
						NULL, 
						&ValueType, 
						( LPBYTE )service_dll, 
						&ValueSize ); 

					if( _ret != ERROR_SUCCESS )
					{
						if( _ret == ERROR_MORE_DATA )
						{
							log_trace( ( MSG_FATAL_ERROR,  "the path of the image file of the service %s too long (bigger than %u)", SubKeyName, ARRAYSIZE( service_info->image_path ) ) ); 
						}
						else
						{						
							log_trace( ( MSG_FATAL_ERROR,  "receive the path of the image file of the service %s error", SubKeyName ) ); 
						}
						break; 
					}
				}while( FALSE ); 

				if( NULL != param_key )
				{
					RegCloseKey( param_key ); 
					param_key = NULL; 
				}

				__ret = ExpandEnvironmentStringsW( ( LPCWSTR )service_dll, 
					service_info->service_dll, 
					ARRAYSIZE( service_info->service_dll ) ); 

				if( 0 == __ret 
					|| __ret >= ARRAYSIZE( service_info->service_dll ) )
				{
					ret = GetLastError(); 
					break; 
				}

				hr = StringCchCopyW( service_info->name, 
					ARRAYSIZE( service_info->name ), 
					SubKeyName ); 
				if( FAILED( hr ) )
				{

				}

				service_infos->push_back( service_info ); 
				service_info = NULL; 
			}while( FALSE ); 

			if( NULL != service_info )
			{
				free( service_info ); 
			}

			if( SubServiceKey != NULL )
			{
				RegCloseKey( SubServiceKey ); 
				SubServiceKey = NULL; 
			}
		}
	}while( FALSE );
#pragma endregion 

	if( NULL != service_dll )
	{
		free( service_dll ); 
	}

	if( SubServiceKey != NULL )
	{
		RegCloseKey( SubServiceKey ); 
	}

	if( ServicesKey != NULL )
	{
		RegCloseKey( ServicesKey ); 
	}

	return ret; 
}

LRESULT WINAPI release_process_shared_service( vector< PROCESS_SHARED_SERVICE_INFORMATION* > *service_infos )
{
	LRESULT ret = ERROR_SUCCESS; 
	vector< PROCESS_SHARED_SERVICE_INFORMATION* >::iterator it; 

	do 
	{
		for( it = service_infos->begin(); it != service_infos->end(); it ++ )
		{
			ASSERT( *( it ) ); 
			free( *( it ) ); 
		}

		service_infos->clear(); 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI find_process_shared_service( LPCWSTR service_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}
