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

#ifndef __PERIPHERAL_LIST_H__
#define __PERIPHERAL_LIST_H__

typedef enum _PERIPHERAL_TYPE
{
	PERIPHERAL_BUS, 
	PERIPHERAL_DEVICE, 
	PERIPHERAL_UNKNOWN, 
} PERIPHERAL_TYPE, *PPERIPHERAL_TYPE; 

typedef struct _PERIPHERAL_INFORMATION
{
#define MAX_DEVICE_INTERFACE_SYMBOLIC_PATH 520
	PERIPHERAL_TYPE type; 
	WCHAR name[ MAX_PATH ]; 
	WCHAR symbolic_path[ MAX_DEVICE_INTERFACE_SYMBOLIC_PATH ]; 
	BOOLEAN filtering; 
} PERIPHERAL_INFORMATION, *PPERIPHERAL_INFORMATION; 

typedef struct _DEVICE_ENUM_CONTEXT
{
	PVOID enum_context; 
}DEVICE_ENUM_CONTEXT, *PDEVICE_ENUM_CONTEXT; 

LRESULT WINAPI init_enum_context( DEVICE_ENUM_CONTEXT *enum_context ); 
LRESULT WINAPI uninit_enum_rontext( DEVICE_ENUM_CONTEXT *enum_context ); 

LRESULT WINAPI enum_bus_device( DEVICE_ENUM_CONTEXT *context, ULONG dev_index );  
LRESULT WINAPI enum_usb_hubs(); 
LRESULT WINAPI _enum_usb_ports( LPCWSTR symbolic_name ); 
LRESULT WINAPI enum_usb_ports( DEVICE_ENUM_CONTEXT *context );  

LRESULT WINAPI get_device_name_from_symbolic_name( LPCWSTR symbolic_name, 
												  LPWSTR target_name, 
												  ULONG cc_buf_len, 
												  ULONG *cc_ret_len ); 

LRESULT WINAPI enumerate_all_peripherals( PERIPHERAL_INFORMATION *peripherals, ULONG max_count, ULONG *ret_count ); 

#endif //__PERIPHERAL_LIST_H__

