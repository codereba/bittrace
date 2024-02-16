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

#ifndef __BUF_ARRAY_H__
#define __BUF_ARRAY_H__

#define ARRAY_CELL_MAGIC ( ULONG )'LECA'
#define INIT_ARRAY_BUF_REF_COUNT 1

typedef struct _array_cell_head
{
	ULONG magic; 
	LONG ref_count; 
} array_cell_head, *parray_cell_head; 

typedef struct _buf_array
{
#ifdef DRIVER
	ERESOURCE lock; 
#else 
	HANDLE lock; 
#endif //DRIVER

	PVOID buf; 
	ULONG buf_size; 
	ULONG real_buf_size; 
	ULONG cell_size; 
	ULONG cell_count; 
} buf_array, *pbuf_array; 

FORCEINLINE NTSTATUS is_valid_buf_array( buf_array *arr )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( arr != NULL ); 

		if( arr->buf == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( arr->buf_size == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
		}

		if( arr->cell_size == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_3; 
		}

		if( arr->cell_size == arr->buf_size )
		{
			ntstatus = STATUS_INVALID_PARAMETER_4; 
		}

		if( arr->cell_count == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_5; 
		}
	}while( FALSE );

	return ntstatus; 
}

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

NTSTATUS init_buf_array( buf_array *arr, PVOID buf, ULONG buf_size, ULONG cell_size ); 
NTSTATUS attach_buf_array( buf_array *arr, PVOID buf, ULONG buf_size, ULONG cell_size ); 

NTSTATUS get_buf_from_array( buf_array *arr, PVOID *buf_out ); 
NTSTATUS _release_buf_to_array( buf_array *arr, PVOID buf ); 

#ifdef __cplusplus
}
#endif //__cplusplus

INLINE NTSTATUS reference_buf_from_array( buf_array *arr, PVOID buf )
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

		old_ref_count = InterlockedExchangeAdd( &head->ref_count, 1 ); 

		ASSERT( old_ref_count >= 0 ); 

	}while( FALSE );

#ifdef DRIVER
	release_res_lock( arr->lock ); 
#endif //DRIVER

	return ntstatus; 
}

INLINE array_cell_head *get_array_cell_from_buf( PVOID buf )
{
	array_cell_head *head; 

	ASSERT( buf != NULL ); 

#ifdef DBG
#ifdef DRIVER
	if( FALSE == MmIsAddressValid( ( BYTE* )buf - sizeof( array_cell_head ) ) )
#else
	if( FALSE == IsBadReadPtr( ( BYTE* )buf - sizeof( array_cell_head ), sizeof( array_cell_head ) ) )
#endif //DRVIVER
	{
#ifdef DRIVER
		KeBugCheck( STATUS_DATA_ERROR ); 
#else
		ASSERT( FALSE ); 
#endif //DRIVER
	}
#endif //DBG

	head = ( array_cell_head* )( ( BYTE* )buf - sizeof( array_cell_head ) ); 

#ifdef DBG
	if( head->magic != ARRAY_CELL_MAGIC )
	{
#ifdef DRIVER
		KeBugCheck( STATUS_DATA_OVERRUN ); 
#else
		ASSERT( FALSE ); 
#endif //DRIVER		
	}
#endif //DBG

	return head; 
}

INLINE NTSTATUS release_buf_to_array( PVOID buf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	array_cell_head *head; 
	LONG old_ref_count; 

	do 
	{
		ASSERT( buf != NULL ); 
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

	return ntstatus; 
}

INLINE NTSTATUS detach_buf_array( buf_array *arr )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( arr != NULL ); 
		arr->buf = NULL; 
		arr->buf_size = 0; 
		arr->cell_count = 0; 
		arr->cell_size = 0; 

#ifdef DRIVER
		uninit_res_lock( &arr->lock ); 
#else
		CloseHandle( arr->lock ); 
#endif //DRIVER

	}while( FALSE );

	return ntstatus; 
}

INLINE NTSTATUS uninit_buf_array( buf_array *arr )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	array_cell_head *head; 
	ULONG i; 

	do 
	{
		ASSERT( arr != NULL ); 

#ifdef DRIVER
		hold_w_res_lock( arr->lock ); 
#endif //DRIVER

		for( i = 0; i < arr->cell_count; i ++ )
		{
			head = ( array_cell_head* )( ( BYTE* )arr->buf + ( arr->cell_size * i ) ); 

			if( head->magic != ARRAY_CELL_MAGIC )
			{
#ifdef DRIVER
				KeBugCheck( STATUS_DATA_ERROR ); 
#else
				ASSERT( FALSE ); 
#endif //DRIVER			
				break; 
			}

			if( 0 > head->ref_count )
			{
#ifdef DRIVER
#ifndef DBG
				KeBugCheck( STATUS_DATA_ERROR ); 
#endif //DBG
#else
				ASSERT( FALSE ); 
#endif //DRIVER	
				break; 
			}

			if( 0 != head->ref_count )
			{
				ASSERT( FALSE ); 
				//break; 
			}
		}
	}while( FALSE );

#ifdef DRIVER
	release_res_lock( arr->lock ); 
#endif //DRIVER

	detach_buf_array( arr ); 

	return ntstatus; 
}

INLINE NTSTATUS reinit_buf_array( buf_array *arr )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	array_cell_head *head; 
	INT32 i; 

	ASSERT( arr != NULL ); 

	do 
	{
		if( arr->buf == NULL 
			|| arr->real_buf_size == 0 
			|| arr->buf_size == 0 
			|| arr->cell_size == 0 
			|| arr->cell_count == 0 )
		{
			ASSERT( FALSE && "reinitialize the buffer array that have not initialized." ); 
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

#ifdef DRIVER
		hold_w_res_lock( arr->lock ); 
#endif //DRIVER

		for( i = 0; ( ULONG )i < arr->cell_count; i ++ )
		{
			head = ( array_cell_head* )( ( BYTE* )arr->buf + ( arr->cell_size * i ) ); 

			head->magic = ARRAY_CELL_MAGIC; 
			head->ref_count = 0; 
		}

#ifdef DRIVER
		release_res_lock( arr->lock ); 
#endif //DRIVER

	}while( FALSE ); 

	return ntstatus; 
}
#endif //__BUF_ARRAY_H__