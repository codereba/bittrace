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
#define PASSIVE_LEVEL 0
#include "wdk_macros.h"
#include <usbioctl.h>
#include <setupapi.h>
#include <devguid.h>

#include "peripheral_list.h"

LRESULT WINAPI get_usb_port_detail( DEVICE_ENUM_CONTEXT  *context, 
								   LPCWSTR symbolic_name, 
								   PUSB_NODE_CONNECTION_INFORMATION_EX usb_node_info, 
								   UCHAR port_index ); 

LRESULT WINAPI uninit_enum_rontext( DEVICE_ENUM_CONTEXT *context )
{
	LRESULT ret; 

	do 
	{
		ASSERT( NULL != context ); 

		if( NULL == context->enum_context 
			|| INVALID_HANDLE_VALUE == context->enum_context ) 
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		if( FALSE == SetupDiDestroyDeviceInfoList( context->enum_context ) )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			context->enum_context = INVALID_HANDLE_VALUE; 
			break; 
		}

		context->enum_context = INVALID_HANDLE_VALUE; 
		ret = ERROR_SUCCESS; 
	} while ( FALSE );

	return ret; 
}

//#define INITGUID

#define _DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	EXTERN_C const GUID DECLSPEC_SELECTANY name \
	= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

_DEFINE_GUID(GUID_DEVINTERFACE_USB_HUB,    0xf18a0e88, 0xc30c, 0x11d0, 0x88, 0x15, 0x00, \
			0xa0, 0xc9, 0x06, 0xbe, 0xd8);

_DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, \
			0xC0, 0x4F, 0xB9, 0x51, 0xED);

/* 3ABF6F2D-71C4-462a-8A92-1E6861E6AF27 */
_DEFINE_GUID(GUID_DEVINTERFACE_USB_HOST_CONTROLLER, 0x3abf6f2d, 0x71c4, 0x462a, 0x8a, 0x92, 0x1e, \
			0x68, 0x61, 0xe6, 0xaf, 0x27);

LRESULT WINAPI init_enum_context( DEVICE_ENUM_CONTEXT *context )
{
	LRESULT ret = ERROR_SUCCESS; 
	HDEVINFO root_bus_info; 

	do 
	{
		ASSERT( NULL != context ); 

		context->enum_context = INVALID_HANDLE_VALUE; 

		root_bus_info = ::SetupDiGetClassDevs( &GUID_CLASS_USBHUB, 
			NULL, 
			NULL, 
			( DIGCF_DEVICEINTERFACE | DIGCF_PRESENT ) ); 

		if( INVALID_HANDLE_VALUE == root_bus_info )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "get usb hub information error\n" ); 
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		context->enum_context = root_bus_info; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_symbol_target_name( LPCWSTR symbolic_name, 
									  LPWSTR target_name, 
									  ULONG cc_buf_len, 
									  ULONG *cc_ret_len)
{
	LRESULT ret = ERROR_SUCCESS; 
	
	return ret; 
}
LRESULT WINAPI get_device_name_from_symbolic_name( LPCWSTR symbolic_name, 
												  LPWSTR target_name, 
												  ULONG cc_buf_len, 
												  ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
#define MAX_SYMBOLIC_NAME 512
	WCHAR _symbolic_name[ MAX_SYMBOLIC_NAME ];
	LPWSTR __symbolic_name;

	do 
	{
		ASSERT( NULL != symbolic_name ); 

		__symbolic_name = ( LPWSTR )symbolic_name;
		__symbolic_name += 4;

		RtlZeroMemory( _symbolic_name, sizeof( _symbolic_name ) );

#define RING3_DEVICE_NAME_PREFIX L"\\DosDevices\\Global\\"
		hr = StringCbCopyW( _symbolic_name, 
			sizeof(_symbolic_name), 
			RING3_DEVICE_NAME_PREFIX ); 

		if( FAILED( hr ) ) 
		{
			break; 
		}

		hr = ::StringCbCatW(_symbolic_name, sizeof( _symbolic_name ), symbolic_name ); 
		if(FAILED( hr ) )
		{
			break; 
		}

#if 0
		HANDLE events_pnp; 
		BOOL _ret; 
		ULONG __ret; 

		do 
		{
			//get the system symbolic information from kernel module support.but noew get them directly from ring3; 
			events_pnp = ::CreateFile(DKPORT_NAME_STR, GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL);
			if( events_pnp == INVALID_HANDLE_VALUE )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			_ret = ::DeviceIoControl( events_pnp, 
				IOCTL_DKSYSPORT_GET_TGTHUB, 
				_symbolic_name, 
				sizeof( _symbolic_name ), 
				target_name, 
				( cc_buf_len << 1 ), 
				&__ret, 
				NULL ); 

			if( FALSE == _ret ) 
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

		}while ( FALSE ); 

		if( INVALID_HANDLE_VALUE != events_pnp )
		{
			::CloseHandle( events_pnp ); 
		}
#endif //0p

		ret = get_symbol_target_name( _symbolic_name, target_name, cc_buf_len, cc_ret_len ); 
	} while ( FALSE );

	return ret;
}

LRESULT WINAPI enum_bus_device( DEVICE_ENUM_CONTEXT *context, ULONG dev_index, PERIPHERAL_INFORMATION *info )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret = FALSE;
	DWORD ret_size = 0;
	SP_DEVICE_INTERFACE_DATA device_interface_data = { 0 }; 
	PSP_DEVICE_INTERFACE_DETAIL_DATA device_interface_detail = NULL;

	do 
	{
		ASSERT( NULL != context ); 
		ASSERT( NULL != info ); 

		if (context->enum_context == INVALID_HANDLE_VALUE)
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		device_interface_data.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA ); 

		_ret = ::SetupDiEnumDeviceInterfaces( context->enum_context, 
			NULL, 
			&GUID_CLASS_USBHUB, 
			dev_index, 
			&device_interface_data ); 

		if( FALSE == _ret ) 
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "enum the device interfaces of the bus error\n"); 
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = ::SetupDiGetDeviceInterfaceDetail( context->enum_context, 
			&device_interface_data, 
			NULL, 
			0, 
			&ret_size, 
			NULL ); 

		if( ret_size <= 0 ) 
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		device_interface_detail = ( PSP_DEVICE_INTERFACE_DETAIL_DATA )::GlobalAlloc( GPTR, ret_size );  
		if( NULL == device_interface_detail )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		device_interface_detail->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA ); 

		_ret = ::SetupDiGetDeviceInterfaceDetail( context->enum_context, 
			&device_interface_data, 
			device_interface_detail, 
			ret_size, 
			&ret_size, 
			NULL ); 

		if( FALSE == _ret ) 
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "enum the device interfaces detail of the bus error\n"); 
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		ULONG cc_name_len; 
		ret = get_device_name_from_symbolic_name( device_interface_detail->DevicePath, info->name, ARRAYSIZE( info->name ), &cc_name_len ); 
		if( ERROR_SUCCESS != ret )
		{
			
		}

	}while( FALSE ); 

	if( NULL != device_interface_detail )
	{
		::GlobalFree( ( HGLOBAL )device_interface_detail ); 
	}
	return ret;
}

LRESULT WINAPI enum_usb_hubs( PERIPHERAL_INFORMATION *peripherals, 
							 ULONG max_count, 
							 ULONG *ret_count ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT __ret; 
	BOOL _ret; 
	HRESULT hr; 
	HDEVINFO devs_info;
	SP_DEVICE_INTERFACE_DATA device_interface_data = { 0 }; 
	SP_DEVINFO_DATA device_info_data = { 0 }; 
	PSP_DEVICE_INTERFACE_DETAIL_DATA dev_interface_detail = NULL;
	ULONG ret_size = 0; 
	INT32 index; 
	PVOID property_buffer = NULL; 
	ULONG property_type; 
	ULONG property_size; 
	ULONG info_count; 
	INT32 guid_index; 
	CONST GUID *query_guids[] = { &GUID_DEVCLASS_USB, 
		&GUID_CLASS_USB_DEVICE, 
		&GUID_CLASS_USB_HOST_CONTROLLER }; 

#define MAX_DEVICE_PROPERTY_DATA_SIZE 2048

	do 
	{
		ASSERT( NULL != ret_count ); 
		*ret_count = 0; 
		property_buffer = malloc( MAX_DEVICE_PROPERTY_DATA_SIZE ); 
		if( NULL == property_buffer ) 
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		info_count = 0; 
		for( guid_index = 0; guid_index < ARRAYSIZE( query_guids ); guid_index ++ )
		{
			do 
			{
				// 1. Get class devices
				devs_info = ::SetupDiGetClassDevs( 
					query_guids[ guid_index ], 
					NULL, 
					NULL,
					( DIGCF_DEVICEINTERFACE | DIGCF_PRESENT ) ); 

				if( devs_info == INVALID_HANDLE_VALUE )
				{
					SAFE_SET_ERROR_CODE( ret ); 
					dbg_print( DBG_MSG_AND_ERROR_OUT | MSG_FATAL_ERROR, "get devices information for class error\n" ); 
					break; 
				}

				index = 0; 
				// 2. Enumerate USB Root Hubs
				for( ; ; )
				{
					device_info_data.cbSize = sizeof( device_info_data ); 

					peripherals[ info_count ].type = PERIPHERAL_UNKNOWN; 
					*peripherals[ info_count ].symbolic_path = L'\0'; 
					*peripherals[ info_count ].name = L'\0'; 

					_ret = SetupDiEnumDeviceInfo( devs_info, index, &device_info_data ); 

					if( _ret == FALSE )
					{
						__ret = GetLastError(); 

						if( __ret == ERROR_NO_MORE_ITEMS )
						{
							break; 
						}
					}

					do 
					{
						dbg_print( MSG_IMPORTANT, "class guid for device is %x-%x-%x-%x%x%x%x%x%x%x%x\n", 
							device_info_data.ClassGuid.Data1, 
							device_info_data.ClassGuid.Data2, 
							device_info_data.ClassGuid.Data3, 
							device_info_data.ClassGuid.Data4[ 0 ], 
							device_info_data.ClassGuid.Data4[ 1 ], 
							device_info_data.ClassGuid.Data4[ 2 ], 
							device_info_data.ClassGuid.Data4[ 3 ], 
							device_info_data.ClassGuid.Data4[ 4 ], 
							device_info_data.ClassGuid.Data4[ 5 ], 
							device_info_data.ClassGuid.Data4[ 6 ], 
							device_info_data.ClassGuid.Data4[ 7 ] ); 

						if( ( ULONG )info_count >= max_count ) 
						{
							break; 
						}

						do 
						{
							_ret = SetupDiGetDeviceRegistryPropertyW( devs_info, 
								&device_info_data, 
								SPDRP_DEVICEDESC, 
								&property_type, 
								( PBYTE )peripherals[ info_count ].name, 
								sizeof( peripherals[ info_count ].name ), 
								&property_size ); 

							if( FALSE == _ret )
							{
								if( peripherals[ info_count ].name[ ARRAYSIZE( peripherals[ index ].name ) - 1 ] != L'\0' ) 
								{
									peripherals[ info_count ].name[ ARRAYSIZE( peripherals[ index ].name ) - 1 ] = L'\0'; 
								}

								//break; 
							}
							else
							{
								if( peripherals[ info_count ].name[ ARRAYSIZE( peripherals[ info_count ].name ) - 1 ] != L'\0' ) 
								{
									peripherals[ info_count ].name[ ARRAYSIZE( peripherals[ info_count ].name ) - 1 ] = L'\0'; 
								}
							}

							_ret = SetupDiGetDeviceRegistryPropertyW( devs_info, 
								&device_info_data, 
								SPDRP_LOCATION_PATHS, 
								&property_type, 
								( PBYTE )property_buffer, 
								MAX_DEVICE_PROPERTY_DATA_SIZE, 
								&property_size ); 

							if( FALSE == _ret )
							{
							}

							peripherals[ info_count ].type = PERIPHERAL_DEVICE; 

						}while( FALSE ); 

						do 
						{
							device_interface_data.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA ); 

							_ret = ::SetupDiEnumDeviceInterfaces( devs_info, 
								&device_info_data, 
								query_guids[ guid_index ], 
								0, 
								&device_interface_data ); 

							if( _ret == FALSE ) 
							{
								__ret = GetLastError(); 

								SAFE_SET_ERROR_CODE( ret ); 
								dbg_print( DBG_MSG_AND_ERROR_OUT | MSG_FATAL_ERROR, "get device interface information error\n" ); 
								break;
							}

							ret_size = 0;

							_ret = ::SetupDiGetInterfaceDeviceDetail( devs_info, 
								&device_interface_data, 
								NULL, 
								0, 
								&ret_size, 
								NULL ); 

							if( ret_size == 0 )
							{
								break; 
							}

							dev_interface_detail = ( PSP_DEVICE_INTERFACE_DETAIL_DATA ) ::GlobalAlloc(GPTR, ret_size);

							if( NULL == dev_interface_detail )
							{
								break; 
							}

							dev_interface_detail->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );

							_ret = ::SetupDiGetInterfaceDeviceDetail( devs_info, 
								&device_interface_data,
								dev_interface_detail, 
								ret_size, 
								&ret_size, 
								NULL ); 

							if( FALSE == _ret )
							{
								break; 
							}

							hr = StringCbCopyW( peripherals[ info_count ].symbolic_path, 
								ARRAYSIZE( peripherals[ info_count ].symbolic_path ), 
								dev_interface_detail->DevicePath ); 

							if( FAILED( hr ) )
							{
								dbg_print( MSG_FATAL_ERROR, "cpoy device interface path error %ws\n", dev_interface_detail->DevicePath ); 
							}

							// 3. Enumerate USB Hub Ports
							__ret = _enum_usb_ports( dev_interface_detail->DevicePath ); 
							if( ERROR_SUCCESS != __ret ) 
							{
							}

							info_count += 1; 

						}while( FALSE ); 

						if( NULL != dev_interface_detail )
						{
							::GlobalFree( ( HGLOBAL )dev_interface_detail ); 			
							dev_interface_detail = NULL; 
						}
					}while( FALSE ); 

					index ++;
				}

			}while( FALSE );

			if( NULL != dev_interface_detail )
			{
				::GlobalFree( ( HGLOBAL )dev_interface_detail ); 			
			}
		}

		*ret_count = info_count; 
	}while( FALSE ); 



	if( NULL != property_buffer ) 
	{
		free( property_buffer ); 
	}


	if( INVALID_HANDLE_VALUE != devs_info )
	{
		::SetupDiDestroyDeviceInfoList( devs_info ); 
	}
	
	return ret; 
}

LRESULT WINAPI _enum_usb_ports( LPCWSTR symbolic_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE usb_hub_handle = INVALID_HANDLE_VALUE; 
	BOOL					_ret = FALSE;
	USB_NODE_INFORMATION	usb_node_info;
	DWORD					ret_size = 0;

	UCHAR								hub_port_count = 0;
	PUSB_NODE_CONNECTION_INFORMATION_EX	usb_node_connect_info = NULL;
	DWORD								buf_size = 0;

	do 
	{
		ASSERT( NULL != symbolic_name ); 

		if( L'\0' == *symbolic_name ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		usb_hub_handle = ::CreateFile( symbolic_name, 
			GENERIC_ALL, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			NULL ); 

		if( INVALID_HANDLE_VALUE == usb_hub_handle )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			dbg_print( MSG_FATAL_ERROR | MSG_FATAL_ERROR, "open the usb hub %ws error\n", symbolic_name ); 
			break; 
		}

		_ret = ::DeviceIoControl( usb_hub_handle, 
			IOCTL_USB_GET_NODE_INFORMATION, 
			( LPVOID )&usb_node_info, 
			sizeof( USB_NODE_INFORMATION ), 
			( LPVOID )&usb_node_info, 
			sizeof( USB_NODE_INFORMATION ), 
			&ret_size, 
			NULL ); 

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			dbg_print( MSG_FATAL_ERROR | MSG_FATAL_ERROR, "get the usb hub node %ws error\n", symbolic_name ); 
			break; 
		}

		hub_port_count = usb_node_info.u.HubInformation.HubDescriptor.bNumberOfPorts;

#define MAX_USB_PIPE_COUNT 30

		buf_size = sizeof( USB_NODE_CONNECTION_INFORMATION_EX ) + ( sizeof( USB_PIPE_INFO ) * MAX_USB_PIPE_COUNT ); 

		usb_node_connect_info = ( PUSB_NODE_CONNECTION_INFORMATION_EX )::GlobalAlloc( GPTR, buf_size ); 

		if( NULL == usb_node_connect_info )
		{
			break; 
		}

		for( UCHAR index = 1; index <= hub_port_count; index++ )
		{
			do 
			{
				usb_node_connect_info->ConnectionIndex = index;

				_ret = ::DeviceIoControl( usb_hub_handle, 
					IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
					usb_node_connect_info, 
					buf_size, 
					usb_node_connect_info, 
					buf_size, 
					&ret_size, 
					NULL ); 

				if( FALSE == _ret )
				{
					break; 
				}

				if( usb_node_connect_info->ConnectionStatus != DeviceConnected ) 
				{
					break; 
				}

			} while ( FALSE );
		}

	}while( FALSE ); 

	if( NULL != usb_node_connect_info )
	{
		::GlobalFree((HGLOBAL) usb_node_connect_info); 
	}

	if( INVALID_HANDLE_VALUE != usb_hub_handle ) 
	{
		::CloseHandle( usb_hub_handle );
	}

	return ret; 
}

LRESULT WINAPI enum_usb_ports( DEVICE_ENUM_CONTEXT *context )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret = FALSE;

	do 
	{
		ASSERT( NULL != context ); 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_usb_port_detail( DEVICE_ENUM_CONTEXT  *context, 
								   LPCWSTR symbolic_name, 
								   PUSB_NODE_CONNECTION_INFORMATION_EX usb_node_info, 
								   UCHAR port_index, 
								   PERIPHERAL_INFORMATION *info )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	WCHAR speed_desc[ 16 ];
	WCHAR port_detail[ MAX_PATH ]; 

	do 
	{
		ASSERT( NULL != info ); 

		ULONG cc_name_len; 
		ret = get_device_name_from_symbolic_name( symbolic_name, info->name, ARRAYSIZE( info->name ), &cc_name_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		switch (usb_node_info->Speed){
		case UsbLowSpeed:
			::StringCbPrintf(speed_desc, sizeof(speed_desc), TEXT("Low Speed"));
			break;
		case UsbHighSpeed:
			::StringCbPrintf(speed_desc, sizeof(speed_desc), TEXT("High Speed"));
			break;
		case UsbFullSpeed:
			::StringCbPrintf(speed_desc, sizeof(speed_desc), TEXT("Full Speed"));
			break;
		default:
			::StringCbPrintf(speed_desc, sizeof(speed_desc), TEXT("Unknown Speed"));
			break;
		}

		hr = ::StringCbPrintf( port_detail, sizeof( port_detail ), 
			TEXT("USB device attached to Root Hub: %s, port: %d. Detail: Vendor Id = 0x%X, Product Id = 0x%X, Speed = %s, Is hub device = 0x%X, Device address = 0x%X"),
			L"", //context->bus_name, 
			port_index, 
			usb_node_info->DeviceDescriptor.idVendor, 
			usb_node_info->DeviceDescriptor.idProduct,
			speed_desc, 
			usb_node_info->DeviceIsHub, 
			usb_node_info->DeviceAddress ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			break; 
		}

	} while ( FALSE );

	return ret; 
}

DEVICE_ENUM_CONTEXT peripheral_enum = { INVALID_HANDLE_VALUE }; 
LRESULT WINAPI enumerate_all_peripherals( PERIPHERAL_INFORMATION *peripherals, 
										 ULONG max_count, 
										 ULONG *ret_count ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN enum_context_inited = FALSE; 

	do 
	{
		ASSERT( NULL != ret_count ); 
		*ret_count = 0; 

		ret = enum_usb_hubs( peripherals, 
			max_count, 
			ret_count ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	} while ( FALSE ); 

	if( TRUE == enum_context_inited ) 
	{
		uninit_enum_rontext( &peripheral_enum ); 
	}
	return ret; 
}
