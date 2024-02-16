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

#ifdef DRIVER
#include "common.h"
#else
#include "common_func.h"
#include <ntstatus.h>
#endif //DRIVER 

#include "buf_array.h"

NTSTATUS init_buf_array( buf_array *arr, PVOID buf, ULONG buf_size, ULONG cell_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	array_cell_head *head; 
	ULONG fixed_cell_size; 
	ULONG i; 
	BOOLEAN lock_inited = FALSE; 

	do 
	{
		ASSERT( arr != NULL ); 
		ASSERT( buf != NULL ); 
		ASSERT( buf_size > 0 ); 
		ASSERT( cell_size > 0 ); 

#ifndef DRIVER
		arr->lock = NULL; 
#endif //DRIVER

		if( cell_size == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define MAX_ARRAY_CELL_SIZE ( 1 << 14 )

		if( cell_size > MAX_ARRAY_CELL_SIZE )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		if( buf_size < cell_size + sizeof( array_cell_head ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		fixed_cell_size = cell_size + sizeof( array_cell_head ); 

		arr->buf = buf; 
		arr->real_buf_size = buf_size; 

		arr->buf_size = buf_size - ( buf_size % fixed_cell_size ); 
		arr->cell_size = fixed_cell_size; 
		arr->cell_count = buf_size / fixed_cell_size; 

		for( i = 0; i < arr->cell_count; i ++ )
		{
			head = ( array_cell_head* )( ( BYTE* )arr->buf + ( arr->cell_size * i ) ); 

			head->magic = ARRAY_CELL_MAGIC; 
			head->ref_count = 0; 
		}

#ifdef DRIVER
		ntstatus = init_res_lock( &arr->lock ); 
		if( ntstatus !=	STATUS_SUCCESS )
		{
			break; 
		}
#else
		arr->lock = CreateMutex( NULL, FALSE, NULL ); 
#endif //DRIVER

		lock_inited = TRUE; 
	}while( FALSE ); 

	if( ntstatus != STATUS_SUCCESS )
	{
		if( lock_inited == TRUE )
		{
#ifdef DRIVER
			uninit_res_lock( &arr->lock ); 
#else
			ASSERT( arr->lock != NULL ); 
			CloseHandle( arr->lock ); 
#endif //DRIVER
		}

		arr->buf = NULL; 
		arr->buf_size = 0; 
		arr->real_buf_size = 0; 
		arr->cell_count = 0; 
		arr->cell_size = 0; 
	}

	return ntstatus; 
}

NTSTATUS attach_buf_array( buf_array *arr, PVOID buf, ULONG buf_size, ULONG cell_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	//array_cell_head *head; 
	ULONG fixed_cell_size; 
	//ULONG i; 
	BOOLEAN lock_inited = FALSE; 

	do 
	{
		ASSERT( arr != NULL ); 
		ASSERT( buf != NULL ); 
		ASSERT( buf_size > 0 ); 
		ASSERT( cell_size > 0 ); 

#ifndef DRIVER
		arr->lock = NULL; 
#endif //DRIVER

		if( cell_size == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#define MAX_ARRAY_CELL_SIZE ( 1 << 14 )

		if( cell_size > MAX_ARRAY_CELL_SIZE )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		if( buf_size < cell_size + sizeof( array_cell_head ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		fixed_cell_size = cell_size + sizeof( array_cell_head ); 

		arr->buf = buf; 
		arr->real_buf_size = buf_size; 

		arr->buf_size = buf_size - ( buf_size % fixed_cell_size ); 
		arr->cell_size = fixed_cell_size; 
		arr->cell_count = buf_size / fixed_cell_size; 

#ifdef DRIVER
		ntstatus = init_res_lock( &arr->lock ); 
		if( ntstatus !=	STATUS_SUCCESS )
		{
			break; 
		}
#else
		arr->lock = CreateMutex( NULL, FALSE, NULL ); 
#endif //DRIVER

		lock_inited = TRUE; 
	}while( FALSE ); 

	if( ntstatus != STATUS_SUCCESS )
	{
		if( lock_inited == TRUE )
		{
#ifdef DRIVER
			uninit_res_lock( &arr->lock ); 
#else
			ASSERT( arr->lock != NULL ); 
			CloseHandle( arr->lock ); 
#endif //DRIVER
		}

		arr->buf = NULL; 
		arr->buf_size = 0; 
		arr->real_buf_size = 0; 
		arr->cell_count = 0; 
		arr->cell_size = 0; 
	}

	return ntstatus; 
}

NTSTATUS get_buf_from_array( buf_array *arr, PVOID *buf_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	register array_cell_head *head; 
	ULONG i; 

	ASSERT( arr != NULL ); 	

	do 
	{
		ASSERT( buf_out != NULL ); 
		ASSERT( TRUE == is_valid_buf_array( arr ) ); 

		*buf_out = NULL; 

#ifdef DRIVER
		hold_w_res_lock( arr->lock ); 
#endif //DRIVER

		for( i = 0; i < arr->cell_count; i ++ )
		{
			head = ( array_cell_head* )( ( BYTE* )arr->buf + ( arr->cell_size * i ) ); 
			
			if( head->magic != ARRAY_CELL_MAGIC )
			{
#ifdef DRIVER
				KeBugCheck( STATUS_DATA_OVERRUN ); 
#else
				ASSERT( FALSE ); 
#endif //DRIVER			
				break; 
			}
			if( 0 == head->ref_count )
			{
				head->ref_count = INIT_ARRAY_BUF_REF_COUNT; 
				*buf_out = head + 1; 
				break; 
			}
		}
	}while( FALSE );

#ifdef DRIVER
	release_res_lock( arr->lock ); 
#endif //DRIVER

	return ntstatus; 
}

NTSTATUS _release_buf_to_array( buf_array *arr, PVOID buf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	array_cell_head *head; 
	LONG old_ref_count; 

	do 
	{
		ASSERT( arr != NULL ); 
		ASSERT( buf != NULL ); 

#ifdef DRIVER
		hold_w_res_lock( arr->lock ); 
#endif //DRIVER

		if( ( BYTE* )buf - sizeof( array_cell_head ) < ( BYTE* )arr->buf || 
			( ( BYTE* )buf - sizeof( array_cell_head ) + arr->cell_size ) > ( BYTE* )arr->buf + arr->buf_size )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		head = ( array_cell_head* )( ( BYTE* )buf - sizeof( array_cell_head ) ); 

		if( head->magic != ARRAY_CELL_MAGIC )
		{
#ifdef DRIVER
			KeBugCheck( STATUS_DATA_OVERRUN ); 
#else
			ASSERT( FALSE ); 
#endif //DRIVER		
			break; 
		}

		old_ref_count = InterlockedExchangeAdd( &head->ref_count, -1 ); 

		ASSERT( old_ref_count >= 0 ); 
	}while( FALSE );

#ifdef DRIVER
	release_res_lock( arr->lock ); 
#endif //DRIVER

	return ntstatus; 
}