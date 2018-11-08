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