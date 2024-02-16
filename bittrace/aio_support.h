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

#ifndef __AIO_READ_H__
#define __AIO_READ_H__

extern HANDLE aio_notifier; 

LRESULT WINAPI init_aio_work(); 

LRESULT WINAPI uninit_aio_work(); 

LRESULT WINAPI _dev_io_ctl_aio( HANDLE device, 
							  HANDLE notifier, 
							  ULONG ctl_code, 
							  PVOID input_data, 
							  ULONG input_len, 
							  PVOID data, 
							  ULONG buf_len, 
							  ULONG *ret_len ); 

INLINE LRESULT WINAPI dev_io_ctl_aio( HANDLE device, 
							  ULONG ctl_code, 
							  PVOID input_data, 
							  ULONG input_len, 
							  PVOID data, 
							  ULONG buf_len, 
							  ULONG *ret_len )
{
	return _dev_io_ctl_aio( device, 
		aio_notifier, 
		ctl_code, 
		input_data, 
		input_len, 
		data, 
		buf_len, 
		ret_len ); 
}

LRESULT WINAPI _read_aio( HANDLE device, 
						HANDLE notifier, 
						PVOID data, 
						ULONG buf_len, 
						ULONG *ret_len ); 

INLINE LRESULT WINAPI read_aio( HANDLE device, 
						PVOID data, 
						ULONG buf_len, 
						ULONG *ret_len )
{
	return _read_aio( device, aio_notifier, data, buf_len, ret_len ); 	
} 

VOID CALLBACK aio_completion(
							 ULONG error_code,
							 ULONG bytes_trans,
							 LPOVERLAPPED ov ); 

#endif //__AIO_READ_H__