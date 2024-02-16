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

#ifdef _DRIVER
#include "common.h"
#include "ring3_2_ring0.h"
#else
#include "common_func.h"
#endif //_DRIVER

#define PASSIVE_LEVEL 0
#include "volume_name_map.h"
#include "wdk_macros.h"
#ifndef _DRIVER
#define NTDDI_VERSION ( NTDDI_WINXP | ( NTDDI_WINXPSP2 << 1 ) )
#include "fltUser.h"
#include <winioctl.h>
#endif //_DRIVER

volume_name_map all_volumes_name_map[ MAX_VOLUME_COUNT ]; 

#ifdef _DRIVER
ERESOURCE all_vol_maps_lock; 
#else
HANDLE all_vol_maps_lock = NULL; 
#endif //_DRIVER

FORCEINLINE BOOLEAN is_valid_map_index( ULONG index )
{
	return ( /*index >= 0 &&*/ index < ( ULONG )MAX_VOLUME_COUNT ); 
}

#ifndef _DRIVER
LRESULT get_volume_dos_name(
						  __in PWCHAR vol_name, 
						  __out PWCHAR dos_name_out, 
						  __in ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD  CharCount = MAX_PATH + 1;
	PWCHAR Names     = NULL;
	PWCHAR NameIdx   = NULL;
	BOOL   Success   = FALSE;

	ASSERT( dos_name_out != NULL ); 
	ASSERT( buf_len > 0 ); 

	for( ; ; ) 
	{
		//
		//  Allocate a buffer to hold the paths.
		Names = ( PWCHAR )malloc( CharCount * sizeof( WCHAR ) ); 

		if( NULL == Names ) 
		{
			//
			//  If memory can't be allocated, return.
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		//
		//  Obtain all of the paths
		//  for this volume.
		Success = GetVolumePathNamesForVolumeNameW(
			vol_name, Names, CharCount, &CharCount );

		if( Success == TRUE ) 
		{
			break;
		}

		ret = GetLastError(); 

		ASSERT( ret != ERROR_SUCCESS ); 

		if( ret != ERROR_MORE_DATA ) 
		{
			break;
		}

		//
		//  Try again with the
		//  new suggested size.
		free( Names );
		Names = NULL;
	}

	if ( TRUE == Success )
	{
		ASSERT( ret == ERROR_MORE_DATA || ret == ERROR_SUCCESS ); 
		
		if( ret != ERROR_SUCCESS )
		{
			ret = ERROR_SUCCESS; 
		}

		wcsncpy( dos_name_out, Names, ( buf_len ) ); 

#ifdef _DEBUG
		//NOTICE: here maybe have more dos names for the same volume.
		//
		//  Display the various paths.
		for ( NameIdx = Names; 
			NameIdx[ 0 ] != L'\0'; 
			NameIdx += wcslen( NameIdx ) + 1 ) 
		{
			wprintf( L"  %s", NameIdx );
		}
		wprintf( L"\n" );
#endif //_DEBUG

	}
	else
	{
		ASSERT( ret != ERROR_MORE_DATA && ret != ERROR_SUCCESS ); 
	}

	if( Names != NULL ) 
	{
		free( Names );
		Names = NULL;
	}

	return ret;
}
#endif //_DRIVER


BOOLEAN only_update_no_set_vol_map = FALSE; 

#define ONLY_UPDATE_NO_SET_VOL_MAP 0x00000001

LRESULT input_volume_map_name( LPCWSTR dos_name, ULONG dos_name_len, LPCWSTR dev_name, ULONG dev_name_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG vol_map_index; 

	do 
	{
		ASSERT( dos_name != NULL ); 
		ASSERT( dev_name != NULL ); 
		ASSERT( dos_name_len > 0 ); 
		ASSERT( dev_name_len > 0 ); 

		vol_map_index = get_volume_map_index( dos_name ); 
		if( FALSE == is_valid_map_index( vol_map_index ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break;
		}

		if( TRUE == only_update_no_set_vol_map 
			|| ONLY_UPDATE_NO_SET_VOL_MAP == flags )
		{
			if( all_volumes_name_map[ vol_map_index ].dos_name_len > 0 
				&& all_volumes_name_map[ vol_map_index ].dev_name_len > 0 )
			{
				break; 
			}
		}

		wcsncpy( all_volumes_name_map[ vol_map_index ].dos_name, 
			dos_name, 
			ARRAYSIZE( all_volumes_name_map[ vol_map_index ].dos_name ) ); 

		if( L'\0' != all_volumes_name_map[ vol_map_index ].dos_name[ MAX_DOS_VOLUME_NAME_LEN - 1 ] )
		{
#ifdef _DEBUG
	
			all_volumes_name_map[ vol_map_index ].dos_name[ MAX_DOS_VOLUME_NAME_LEN - 1 ] = L'\0'; 

			ASSERT( FALSE && "dos name length more than 260???" ); 
			log_trace( ( MSG_ERROR, "special device name of the volume %ws\n", 
				all_volumes_name_map[ vol_map_index ].dos_name ) ); 
#endif //_DEBUG
			*all_volumes_name_map[ vol_map_index ].dos_name = L'\0'; 
			*all_volumes_name_map[ vol_map_index ].dev_name = L'\0'; 

			all_volumes_name_map[ vol_map_index ].dos_name_len = 0; 
			all_volumes_name_map[ vol_map_index ].dev_name_len = 0;
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		wcsncpy( all_volumes_name_map[ vol_map_index ].dev_name, 
			dev_name, 
			ARRAYSIZE( all_volumes_name_map[ vol_map_index ].dev_name ) ); 
		if( L'\0' != all_volumes_name_map[ vol_map_index ].dev_name[ MAX_NATIVE_VOLUME_NAME_LEN - 1 ] )
		{
#ifdef _DEBUG
			all_volumes_name_map[ vol_map_index ].dev_name[ MAX_NATIVE_VOLUME_NAME_LEN - 1 ] = L'\0'; 

			ASSERT( FALSE && "volume device name length more than 260???" ); 
			log_trace( ( MSG_ERROR, "special device name of the volume %ws\n", 
				all_volumes_name_map[ vol_map_index ].dev_name ) ); 
#endif //_DEBUG
			*all_volumes_name_map[ vol_map_index ].dos_name = L'\0'; 
			*all_volumes_name_map[ vol_map_index ].dev_name = L'\0'; 

			all_volumes_name_map[ vol_map_index ].dos_name_len = 0; 
			all_volumes_name_map[ vol_map_index ].dev_name_len = 0;
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		all_volumes_name_map[ vol_map_index ].dev_name_len = wcslen( all_volumes_name_map[ vol_map_index ].dev_name ); 
		all_volumes_name_map[ vol_map_index ].dos_name_len = wcslen( all_volumes_name_map[ vol_map_index ].dos_name ); 

		if( all_volumes_name_map[ vol_map_index ].dev_name[ all_volumes_name_map[ vol_map_index ].dev_name_len - 1 ] != L'\\' )
		{
			if( all_volumes_name_map[ vol_map_index ].dev_name_len + 2 > MAX_NATIVE_VOLUME_NAME_LEN )
			{
				//clear all name got
#ifdef _DEBUG
				all_volumes_name_map[ vol_map_index ].dev_name[ MAX_NATIVE_VOLUME_NAME_LEN - 1 ] = L'\0'; 

				ASSERT( FALSE && "volume device name length more than 260???" ); 
				log_trace( ( MSG_ERROR, "special device name of the volume %ws\n", 
					all_volumes_name_map[ vol_map_index ].dev_name ) ); 
#endif //_DEBUG

				*all_volumes_name_map[ vol_map_index ].dos_name = L'\0'; 
				*all_volumes_name_map[ vol_map_index ].dev_name = L'\0'; 

				all_volumes_name_map[ vol_map_index ].dos_name_len = 0; 
				all_volumes_name_map[ vol_map_index ].dev_name_len = 0; 

				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			all_volumes_name_map[ vol_map_index ].dev_name[ all_volumes_name_map[ vol_map_index ].dev_name_len ] = L'\\'; 
			all_volumes_name_map[ vol_map_index ].dev_name[ all_volumes_name_map[ vol_map_index ].dev_name_len + 1 ] = L'\0'; 
			all_volumes_name_map[ vol_map_index ].dev_name_len += 1; 
		}
	}while( FALSE );

	return ret; 
}

#ifndef _DRIVER
LRESULT update_volume_name_map( ULONG drv_mask, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	ULONG index; 
	HANDLE find_handle = INVALID_HANDLE_VALUE; 
	WCHAR dev_name[ MAX_PATH ] = L"";
	WCHAR dos_name[ MAX_PATH ]; 
	WCHAR vol_name[ MAX_PATH ] = L""; 
	ULONG ret_len; 
	ULONG ccb_len; 
	ULONG map_index; 

	do
	{
		find_handle = FindFirstVolumeW( vol_name, ARRAYSIZE( vol_name ) ); 

		if( find_handle == INVALID_HANDLE_VALUE )
		{
			ret = GetLastError();
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "find first volume( FindFirstVolumeW )failed with error code %u\n", ret ) );
			break; 
		}

		if( drv_mask == ALL_DRIVE_INDEX )
		{
			for( ; ; )
			{
				do
				{
					index = wcslen( vol_name ) - 1;

					if( vol_name[0]     != L'\\' ||
						vol_name[1]     != L'\\' ||
						vol_name[2]     != L'?'  ||
						vol_name[3]     != L'\\' ||
						vol_name[index] != L'\\') 
					{
						ret = ERROR_BAD_PATHNAME;
						log_trace( ( DBG_MSG_AND_ERROR_OUT, "FindNextVolumeW returned a bad path: %ws\n", vol_name ) );
						break; 
					}

					_ret = get_volume_dos_name( vol_name, dos_name, ARRAYSIZE( dos_name ) ); 
					if( _ret != 0 )
					{
						ret = _ret; 
						break;
					}

					if( *dos_name == L'\0' )
					{
						log_trace( ( DBG_MSG_AND_ERROR_OUT, "volume %ws is not mapped to dos name.\n", vol_name ) );						
						break; 
					}

					vol_name[ index ] = L'\0';

					ccb_len = QueryDosDeviceW( &vol_name[ 4 ], dev_name, ARRAYSIZE( dev_name ) ); 

					vol_name[ index ] = L'\\';

					if( ccb_len == 0 ) 
					{
						_ret = GetLastError(); 
						log_trace( ( DBG_MSG_AND_ERROR_OUT, "querying device dos name ( QueryDosDeviceW ) failed with error code %d\n", _ret ) );
						break; 
					}

#ifdef _DEBUG
					log_trace( ( MSG_INFO, "found a device:\n %ws", dev_name ) );
					log_trace( ( MSG_INFO, "Volume name: %ws", vol_name ) );
					//log_trace( ( MSG_INFO, L"\nPaths:" ) );
#endif //_DEBUG
					ret = input_volume_map_name( dos_name, wcslen( dos_name ), dev_name, ccb_len, flags ); 
				}while( FALSE ); 

				_ret = FindNextVolumeW(find_handle, vol_name, ARRAYSIZE(vol_name));

				if( FALSE == _ret ) 
				{
					_ret = GetLastError();

					if( _ret != ERROR_NO_MORE_FILES ) 
					{
						ret = _ret; 
						log_trace( ( DBG_MSG_AND_ERROR_OUT, "finding the next volume( FindNextVolumeW ) failed with error code %d\n", ret ) ); 
						break;
					}

					ret = ERROR_SUCCESS;
					break;
				}
			}
		}
		else
		{
			for( ; ; )
			{
				do
				{
					//
					//  Skip the \\?\ prefix and remove the trailing backslash.
					index = wcslen( vol_name ) - 1;

					if( vol_name[0]     != L'\\' ||
						vol_name[1]     != L'\\' ||
						vol_name[2]     != L'?'  ||
						vol_name[3]     != L'\\' ||
						vol_name[index] != L'\\') 
					{
						ret = ERROR_BAD_PATHNAME;
						log_trace( ( DBG_MSG_AND_ERROR_OUT, "FindNextVolumeW returned a bad path: %s\n", vol_name ) );
						break; 
					}

					_ret = get_volume_dos_name( vol_name, dos_name, ARRAYSIZE( dos_name ) ); 
					if( _ret != 0 )
					{
						ret = _ret; 
						break;
					}

					map_index = get_volume_map_index( dos_name ); 
					if( INVALID_INDEX == map_index )
					{
						ASSERT( FALSE && "can't get the mapped index of volume from its dos name" );; 

						log_trace( ( MSG_ERROR, "can't get the mapped index of volume from its dos name %ws\n", dos_name ) ); 
						
						continue; 
					}
					else
					{
						ASSERT( map_index < MAX_DRIVE_SIGN_NUM ); 

						if( ( ( 1 << map_index ) & drv_mask ) == 0 )
						{
							continue; 
						}
					}

					//
					//  QueryDosDeviceW does not allow a trailing backslash,
					//  so temporarily remove it.
					vol_name[ index ] = L'\0';

					ccb_len = QueryDosDeviceW( &vol_name[ 4 ], dev_name, ARRAYSIZE( dev_name ) ); 

					vol_name[ index ] = L'\\';

					if( ccb_len == 0 ) 
					{
						_ret = GetLastError(); 
						log_trace( ( DBG_MSG_AND_ERROR_OUT, "querying device dos name ( QueryDosDeviceW ) failed with error code %d\n", _ret ) );
						break; 
					}

#ifdef _DEBUG
					log_trace( ( MSG_INFO, "found a device:\n %ws", dev_name ) );
					log_trace( ( MSG_INFO, "Volume name: %ws", vol_name ) );
					//log_trace( ( MSG_INFO, L"\nPaths:" ) );
#endif //_DEBUG
					ret = input_volume_map_name( dos_name, wcslen( dos_name ), dev_name, ccb_len, flags ); 
				}while( FALSE ); 
				//
				//  Move on to the next volume.
				_ret = FindNextVolumeW(find_handle, vol_name, ARRAYSIZE(vol_name));

				if( FALSE == _ret ) 
				{
					_ret = GetLastError();

					if( _ret != ERROR_NO_MORE_FILES ) 
					{
						ret = _ret; 
						log_trace( ( DBG_MSG_AND_ERROR_OUT, "finding the next volume( FindNextVolumeW ) failed with error code %d\n", ret ) ); 
						break;
					}

					//
					//  Finished iterating
					//  through all the volumes.
					ret = ERROR_SUCCESS;
					break;
				}
			}
		}

	}while ( FALSE );

	if( find_handle != INVALID_HANDLE_VALUE )
	{
		FindVolumeClose( find_handle );
		find_handle = INVALID_HANDLE_VALUE; 
	}

	return ret; 
}
#endif //_DRIVER

#define MAX_VOLUME_LINKS_NAME_SIZE 1024

#ifndef _DRIVER
HRESULT init_volumes_name_map_ex()
{
    UCHAR buffer[1024];
    PFILTER_VOLUME_BASIC_INFORMATION volumeBuffer = ( PFILTER_VOLUME_BASIC_INFORMATION )buffer;
    HANDLE volumeIterator = INVALID_HANDLE_VALUE;
    ULONG volumeBytesReturned;
    HRESULT hResult = S_OK;
    __nullterminated WCHAR driveLetter[15] = { 0 };
    ULONG instanceCount; 
	ULONG vol_native_name_len; 
	INT32 i; 

    _try 
	{

		for( i = 0; i < ARRAYSIZE( all_volumes_name_map ); i ++ )
		{
			*all_volumes_name_map[ i ].dev_name = L'\0'; 
			all_volumes_name_map[ i ].dev_name_len = 0; 
			*all_volumes_name_map[ i ].dos_name = L'\0'; 
			all_volumes_name_map[ i ].dos_name_len = 0; 
		}

        //
        //  Find out size of buffer needed
        //

        hResult = FilterVolumeFindFirst( FilterVolumeBasicInformation,
                                         volumeBuffer,
                                         sizeof( buffer ) - sizeof( WCHAR ),   //save space to null terminate name
                                         &volumeBytesReturned,
                                         &volumeIterator );

        if( IS_ERROR( hResult ) )
		{
             _leave;
        }

        ASSERT( INVALID_HANDLE_VALUE != volumeIterator );

        //
        //  Loop through all of the filters, displaying instance information
        //

        do {

            ASSERT( ( FIELD_OFFSET( FILTER_VOLUME_BASIC_INFORMATION, FilterVolumeName ) + volumeBuffer->FilterVolumeNameLength ) <= ( sizeof( buffer ) - sizeof( WCHAR ) ) );
            __analysis_assume( ( FIELD_OFFSET( FILTER_VOLUME_BASIC_INFORMATION, FilterVolumeName ) + volumeBuffer->FilterVolumeNameLength ) <= ( sizeof( buffer ) - sizeof( WCHAR ) ) ); 

            volumeBuffer->FilterVolumeName[ volumeBuffer->FilterVolumeNameLength / sizeof( WCHAR ) ] = UNICODE_NULL; 

			hResult = FilterGetDosName(
				volumeBuffer->FilterVolumeName,
				driveLetter,
				sizeof( driveLetter ) / sizeof( WCHAR ) ); 

			if( !SUCCEEDED( hResult) )
			{
				continue; 
			}

			{
				INT32 dos_name_len; 
				dos_name_len = wcslen( driveLetter ); 
				if( driveLetter[ dos_name_len - 1 ] != L'\\' )
				{
					if( dos_name_len + 2 > ARRAYSIZE( driveLetter ) )
					{
						hResult = ERROR_ERRORS_ENCOUNTERED; 
						DBG_BP(); 
						break; 
					}

					driveLetter[ dos_name_len ] = L'\\'; 
					dos_name_len += 1; 
				}
			}

			vol_native_name_len = volumeBuffer->FilterVolumeNameLength / sizeof( WCHAR ); 
			if( volumeBuffer->FilterVolumeName[ vol_native_name_len - 1 ] == L'\\' )
			{
				vol_native_name_len -= 1; 
			}

			hResult = input_volume_map_name( driveLetter, wcslen( driveLetter ), volumeBuffer->FilterVolumeName, vol_native_name_len, 0 );  

        }while( SUCCEEDED( hResult = FilterVolumeFindNext( volumeIterator,
                                                                        FilterVolumeBasicInformation,
                                                                        volumeBuffer,
                                                                        sizeof( buffer )-sizeof( WCHAR ),    //save space to null terminate name
                                                                        &volumeBytesReturned ) ) ); 

        if( HRESULT_FROM_WIN32( ERROR_NO_MORE_ITEMS ) == hResult )
		{

            hResult = S_OK;
        }
    } 
	_finally 
	{

        if( INVALID_HANDLE_VALUE != volumeIterator )
		{
            FilterVolumeFindClose( volumeIterator );
        }

        if( IS_ERROR( hResult ) )
		{
            if( HRESULT_FROM_WIN32( ERROR_NO_MORE_ITEMS ) == hResult )
			{

                dbg_print( MSG_ERROR, "No volumes found.\n" );

            }
			else
			{
                dbg_print( MSG_ERROR, "volume traversing failed with error: 0x%08x\n", hResult );
            }
        }
    }

	return hResult; 
}

#endif //_DRIVER

#ifndef _DRIVER

LRESULT init_volumes_name_map()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HANDLE find_handle           = INVALID_HANDLE_VALUE;
	INT32 i; 

	//__asm int 3; 
	for( i = 0; i < ARRAYSIZE( all_volumes_name_map ); i ++ )
	{
		*all_volumes_name_map[ i ].dev_name = L'\0'; 
		all_volumes_name_map[ i ].dev_name_len = 0; 
		*all_volumes_name_map[ i ].dos_name = L'\0'; 
		all_volumes_name_map[ i ].dos_name_len = 0; 
	}

	ret = update_volume_name_map( ALL_DRIVE_INDEX, 0 ); 
	return ret; 
}
#endif //_DRIVER

LRESULT convert_native_path_to_dos( LPWSTR native_path, ULONG native_path_len, LPWSTR dos_path, ULONG ccb_buf_len, ULONG *ccb_ret_len )
{
	LRESULT ret = ERROR_NOT_FOUND; 
	ULONG dos_name_len; 
	INT32 i; 

	do 
	{
		if( ccb_ret_len != NULL )
		{
			*ccb_ret_len = 0; 
		}

		if( native_path[ 1 ] == L':' )
		{
			if( ccb_buf_len < native_path_len + 1 )
			{
				*ccb_ret_len = native_path_len + 1; 
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}

			memcpy( dos_path, native_path, ( native_path_len << 1 ) ); 
			

			dos_path[ native_path_len ] = L'\0'; 

			ret = ERROR_SUCCESS; 
			break; 
		}

		for( i = 0; i < ARRAYSIZE( all_volumes_name_map ); i ++ )
		{
			if( ( native_path_len >= all_volumes_name_map[ i ].dev_name_len ) 
				&& 0 == compare_str( native_path, 
				all_volumes_name_map[ i ].dev_name_len, 
				all_volumes_name_map[ i ].dev_name, 
				all_volumes_name_map[ i ].dev_name_len ) )
			{
				dos_name_len = all_volumes_name_map[ i ].dos_name_len + 
					( native_path_len - all_volumes_name_map[ i ].dev_name_len ); 

#ifdef _DEBUG
				if( all_volumes_name_map[ i ].dos_name[ all_volumes_name_map[ i ].dos_name_len - 1 ] != L'\\' )
				{
					DBG_BP(); 
				}
#endif //_DEBUG

				if( dos_path != NULL )
				{
					if( ccb_buf_len < dos_name_len + 1 )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					memcpy( dos_path, 
						all_volumes_name_map[ i ].dos_name, 
						all_volumes_name_map[ i ].dos_name_len * sizeof( WCHAR ) ); 

					memcpy( dos_path + all_volumes_name_map[ i ].dos_name_len, 
						native_path + all_volumes_name_map[ i ].dev_name_len, 
						( dos_name_len - all_volumes_name_map[ i ].dos_name_len ) * sizeof( WCHAR ) ); 

					dos_path[ dos_name_len ] = L'\0'; 
				}
				else
				{
					if( ccb_buf_len < dos_name_len + 1 )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					memmove( native_path + all_volumes_name_map[ i ].dos_name_len, 
						native_path + all_volumes_name_map[ i ].dev_name_len, 
						( dos_name_len - all_volumes_name_map[ i ].dos_name_len ) * sizeof( WCHAR ) ); 

					memcpy( native_path, 
						all_volumes_name_map[ i ].dos_name, 
						all_volumes_name_map[ i ].dos_name_len * sizeof( WCHAR ) ); 

					native_path[ dos_name_len ] = L'\0'; 
				}

				if( ccb_ret_len != NULL )
				{
					*ccb_ret_len = dos_name_len; 
				}

				ret = ERROR_SUCCESS; 
				break; 
			}
		}

		if( i == ARRAYSIZE( all_volumes_name_map ) )
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT input_vol_map_from_dev_name( LPCWSTR native_path, ULONG native_path_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}

LRESULT convert_dos_path_to_native( LPCWSTR dos_path, ULONG dos_path_len, LPWSTR native_path, ULONG ccb_buf_len, ULONG *ccb_ret_len )
{
	LRESULT ret = ERROR_NOT_FOUND; 
	ULONG dev_name_len; 
	INT32 i; 

	if( ccb_ret_len != NULL )
	{
		*ccb_ret_len = 0; 
	}

	for( i = 0; i < ARRAYSIZE( all_volumes_name_map ); i ++ )
	{
		if( ( dos_path_len >= all_volumes_name_map[ i ].dos_name_len ) 
			&& 0 == compare_str( dos_path, 
			all_volumes_name_map[ i ].dos_name_len, 
			all_volumes_name_map[ i ].dos_name, 
			all_volumes_name_map[ i ].dos_name_len ) )
		{
			dev_name_len = all_volumes_name_map[ i ].dev_name_len + 
				( dos_path_len - all_volumes_name_map[ i ].dos_name_len ) + 1; 

			if( ccb_buf_len < dev_name_len )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}

			memcpy( native_path, 
				all_volumes_name_map[ i ].dev_name, 
				all_volumes_name_map[ i ].dev_name_len * sizeof( WCHAR ) ); 

			memcpy( native_path + all_volumes_name_map[ i ].dev_name_len, 
				dos_path + all_volumes_name_map[ i ].dos_name_len, 
				( dev_name_len - all_volumes_name_map[ i ].dev_name_len ) * sizeof( WCHAR ) ); 


			if( ccb_ret_len != NULL )
			{
				*ccb_ret_len = dev_name_len - 1; 
			}

			ASSERT( native_path[ dev_name_len - 1 ] == L'\0'); 

			ret = ERROR_SUCCESS; 
			break; 
		}
	}

	return ret; 
}

LRESULT input_vol_map_from_dos_name( LPCWSTR dos_path, ULONG dos_path_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}
/*------------------------------------------------------------------
   FirstDriveFromMask (unitmask)

   Description
     Finds the first valid drive letter from a mask of drive letters.
     The mask must be in the format bit 0 = A, bit 1 = B, bit 3 = C, 
     etc. A valid drive letter is defined when the corresponding bit 
     is set to 1.

   Returns the first drive letter that was found.
--------------------------------------------------------------------*/

ULONG first_drive_index_from_mask( ULONG unitmask )
{
   ULONG i;

   for( i = 0; i < MAX_DRIVE_SIGN_NUM; ++i )
   {
      if( unitmask & 0x1 )
	  {
		  break;
	  }

      unitmask = unitmask >> 1;
   }

   if( i >= MAX_DRIVE_SIGN_NUM )
   {
	   i = INVALID_DRIVE_INDEX; 
   }

   return i; 
}