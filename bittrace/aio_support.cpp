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

#include "stdafx.h"
#include "aio_support.h"

/****************************************************
���������IRP������PENDING�ȴ��������и��õ���Ӧ
�ٶȡ�
****************************************************/
HANDLE aio_notifier = NULL; 

LRESULT WINAPI init_aio_work()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		aio_notifier = CreateEvent( NULL, FALSE, FALSE, NULL ); 
		if( aio_notifier  == NULL )
		{
			ret = GetLastError();
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI uninit_aio_work()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( NULL != aio_notifier )
		{
			CloseHandle( aio_notifier ); 
			aio_notifier = NULL; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI _dev_io_ctl_aio( HANDLE device, 
							  HANDLE notifier, 
							  ULONG ctl_code, 
							  PVOID input_data, 
							  ULONG input_len, 
							  PVOID data, 
							  ULONG buf_len, 
							  ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT __ret; 
	INT32 _ret; 
	//ULONG wait_ret; 
	ULONG _ret_len = 0; 
	OVERLAPPED ov; 

	do 
	{
		ASSERT( ctl_code != 0 ); 
		ASSERT( ret_len != NULL ); 

		/*********************************************************************
		1.device need to be freed after all work done, this's is a bug.
		fixed next time.
		*********************************************************************/
		if( device == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ret_len != NULL )
		{
			*ret_len = 0; 
		}

		ZeroMemory( &ov, sizeof(ov) );

		ov.hEvent = notifier; 

		_ret = DeviceIoControl( device, 
			ctl_code, 
			input_data, 
			input_len, 
			data, 
			buf_len, 
			&_ret_len, 
			&ov ); 
		
		if( ret_len != NULL )
		{
			*ret_len = _ret_len; 
		}

		if( _ret == FALSE )
		{
			__ret = GetLastError(); 
			if( __ret != ERROR_IO_PENDING )
			{
				ret = __ret; 
				ASSERT( ret != ERROR_SUCCESS ); 
				break; 
			}
		}
		else
		{
			break; 
		}

		_ret = ::GetOverlappedResult( device, 
			&ov, 
			ret_len, 
			TRUE ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
		}
		else
		{
			ret = ERROR_SUCCESS; 
		}

	}while( FALSE ); 

	return ret; 
}

VOID CALLBACK aio_completion(
	ULONG error_code,
	ULONG bytes_trans,
	LPOVERLAPPED ov )
{

	UNREFERENCED_PARAMETER(error_code);
	UNREFERENCED_PARAMETER(ov);

	//fprintf( stdout, "Thread %d read: %d bytes\n",
	//	GetCurrentThreadId(), bytes_trans );


	return;
}

LRESULT WINAPI _read_aio( HANDLE device, 
						HANDLE notifier, 
						PVOID data, 
						ULONG buf_len, 
						ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	OVERLAPPED ov = { 0 }; 

	do 
	{
		//ov.Offset = 0;
		//ov.OffsetHigh = 0; 
		ov.hEvent = notifier; 

		_ret = ReadFile( device, 
			( LPVOID )data, 
			buf_len,
			ret_len, 
			&ov ); 

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 

			if( ret != ERROR_IO_PENDING )
			{
				break; 
			}

			_ret = ::GetOverlappedResult( device, 
				&ov, 
				ret_len, 
				TRUE ); 

			if( _ret == FALSE )
			{
				SAFE_SET_ERROR_CODE( ret ); 
			}
			else
			{
				ret = ERROR_SUCCESS; 
			}
		}
	}while( FALSE );

	return ret; 
}