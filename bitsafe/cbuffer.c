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

#ifdef DRIVER
#include "common.h"
#else
#include "common_func.h"
#include "ring0_2_ring3.h"
#endif //DRIVER

#include "cbuffer.h"

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef INLINE 
#define INLINE __inline
#endif //INLINE 

#ifndef DRIVER
#include <ntstatus.h>
#endif //DRIVER

#ifndef DRIVER

#ifndef ASSERT
#ifdef DEBUG
#include <assert.h>
#define ASSERT( _exp_ ) assert( _exp_ )
#else
#define ASSERT( _exp_ ) 
#endif //DEBUG
#endif //ASSERT

#endif //DRIVER

#ifdef DRIVER
NTSTATUS uninit_cbuffer_lock( safe_cbuffer *cbuf )
{
	return STATUS_SUCCESS; 
}
#else
NTSTATUS uninit_cbuffer_lock( safe_cbuffer *cbuf )
{
	ASSERT( cbuf != NULL ); 

	CloseHandle( cbuf->lock ); 

	return STATUS_SUCCESS; 
}
#endif //DRIVER

INLINE ULONG buf_size_2_cell_count( cbuffer_t *buf, ULONG buf_size, ULONG *remain_size )
{
	ULONG cell_count; 
	register ULONG _remain_size; 

	//__asm int 3; 

	ASSERT( buf->cellsize > 0 ); 

	cell_count = buf_size / buf->cellsize; 

	_remain_size = ( buf_size % buf->cellsize ); 

	if( remain_size != NULL )
	{
		*remain_size = _remain_size; 	
	}
	
	if( 0 != _remain_size )
	{
		cell_count += 1; 
	}

	return cell_count; 
}

#ifdef DRIVER
NTSTATUS w_lock_cbuffer( safe_cbuffer *buf )
{
	NTSTATUS ntstatus; 
	ASSERT( buf != NULL ); 

	lock_fmutex( &buf->lock ); 

	//ntstatus = hold_w_res_lock( buf->lock ); 
	return STATUS_SUCCESS; ; 
}

NTSTATUS init_cbuffer_lock( safe_cbuffer *buf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	
	do
	{
		ASSERT( buf != NULL ); 

		ExInitializeFastMutex( &buf->lock ); 

	}while( FALSE ); 

	return ntstatus; 
}
#else

NTSTATUS init_cbuffer_lock( safe_cbuffer *buf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do
	{
		ASSERT( buf != NULL ); 

		buf->lock = CreateMutexW( NULL, FALSE, CBUFFER_LOCK_NAME ); 

		if( buf->lock == NULL )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

	}while( FALSE ); 

	return ntstatus; 
}

#endif //DRIVER

NTSTATUS cbuffer_init( cbuffer_t *buf, ULONG bitsize, ULONG cellsize, release_cbuffer_item release_item_func )
{
	ASSERT( buf != NULL ); 

	( ( cbuffer_t* )buf )->start = sizeof( cbuffer_t ); // (unsigned char *) buf + 
	( ( cbuffer_t* )buf )->w = 0;
	( ( cbuffer_t* )buf )->r = 0;
	( ( cbuffer_t* )buf )->mask = ( 2 << ( bitsize - 1 ) ) - 1; 
	( ( cbuffer_t* )buf )->cellsize = cellsize; 
	( ( cbuffer_t* )buf )->release_cbuffer_item_func = release_item_func; 

	return STATUS_SUCCESS; 
}

NTSTATUS cbuffer_create( PVOID cbuf_in, ULONG bitsize, ULONG cellsize, release_cbuffer_item release_item_func, cbuffer_t **buf_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	PVOID buf = NULL;

	ASSERT( buf_out != NULL ); 

	do
	{
		*buf_out = NULL; 

		if( bitsize > MAX_BIT_SIZE )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		if( cbuf_in == NULL )
		{
#ifdef DRIVER
			buf = ExAllocatePoolWithTag(NonPagedPool,sizeof(cbuffer_t) + cellsize * ( 2 << ( bitsize - 1 ) ), CBUFFER_POOL_TAG );
#else
			buf = malloc( sizeof(cbuffer_t) + cellsize * ( 2 << ( bitsize - 1 ) ) );
#endif //DRIVER

			if( buf == NULL )
			{
				ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
				break; 
			}

			cbuf_in = buf; 
		}

		ntstatus = cbuffer_init( ( cbuffer_t* )cbuf_in, bitsize, cellsize, release_item_func ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		*buf_out = ( cbuffer_t* )cbuf_in; 
	}while( FALSE ); 


	if( ntstatus != STATUS_SUCCESS )
	{
		ASSERT( *buf_out = NULL ); 

		if( buf != NULL )
		{
#ifdef DRIVER
			ExFreePoolWithTag( buf, CBUFFER_POOL_TAG ); 
#else
			free( buf ); 
#endif //DRIVER	
		}
	}
	return ntstatus; 
}

NTSTATUS safe_cbuffer_attach( safe_cbuffer *cbuf, PVOID buf_in, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	BOOLEAN lock_inited = FALSE; 

	do 
	{
		ASSERT( cbuf != NULL ); 
		ASSERT( buf_in != NULL ); 

		cbuf->cbuf = NULL; 

		if( ( ( cbuffer_t* )buf_in )->cellsize * ( ( ( cbuffer_t* )buf_in )->mask + 1 ) > buf_size )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( STATUS_SUCCESS != is_valid_cbuffer( ( cbuffer_t* )buf_in ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		ntstatus = init_cbuffer_lock( ( safe_cbuffer* )cbuf ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		lock_inited = TRUE; 

		cbuf->cbuf = ( cbuffer_t* )buf_in; 

	}while( FALSE );

	if( ntstatus != STATUS_SUCCESS )
	{
		if( lock_inited == FALSE )
		{
			uninit_cbuffer_lock( ( safe_cbuffer* )cbuf ); 
		}
	}

	return ntstatus; 
}

NTSTATUS safe_cbuffer_detach( safe_cbuffer *cbuf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( cbuf != NULL ); 
		ASSERT( cbuf->cbuf != NULL ); 

		if( STATUS_SUCCESS != is_valid_cbuffer( ( cbuffer_t* )cbuf->cbuf ) )
		{
			ASSERT( FALSE && "is not valid cbuffer" ); 

			ntstatus = STATUS_INVALID_PARAMETER_2; 
			//break; 
		}

		cbuf->cbuf = NULL; 

		uninit_cbuffer_lock( ( safe_cbuffer* )cbuf ); 

#ifndef DRIVER
		cbuf->lock = NULL; 
#endif //DRIVER

	}while( FALSE );


	return ntstatus; 
}

NTSTATUS create_safe_cbuffer( safe_cbuffer *cbuf, PVOID buf_in, ULONG bitsize, ULONG cellsize, release_cbuffer_item release_item_func )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	BOOLEAN lock_inited = FALSE; 

	do 
	{
		cbuf->cbuf = NULL; 

		init_cbuffer_lock( ( safe_cbuffer* )cbuf ); 

		lock_inited = TRUE; 

		ntstatus = cbuffer_create( buf_in, bitsize, cellsize, release_item_func, &cbuf->cbuf ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			ASSERT( cbuf->cbuf != NULL ); 
			break; 
		}
	}while( FALSE );

	if( ntstatus != STATUS_SUCCESS )
	{
		if( cbuf->cbuf != NULL 
			&& buf_in != cbuf->cbuf )
		{
			ASSERT( FALSE && "can't go here" ); 
			cbuffer_free( cbuf->cbuf ); 
		}

		if( lock_inited == FALSE )
		{
			uninit_cbuffer_lock( cbuf ); 
		}
	}

	return ntstatus; 
}

NTSTATUS destroy_safe_cbuffer( safe_cbuffer *cbuf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( cbuf != NULL ); 

	do 
	{
		if( cbuf->cbuf == NULL )
		{
			ASSERT( FALSE && "release not initialized circle buffer" ); 
		} 
		else
		{
			w_lock_cbuffer( cbuf ); 

			cbuffer_free( cbuf->cbuf ); 

			unlock_cbuffer( cbuf ); 
		}

		uninit_cbuffer_lock( cbuf ); 

		cbuf->cbuf = NULL; 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS uninit_safe_cbuffer( safe_cbuffer *cbuf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( cbuf != NULL ); 

	do 
	{
		if( cbuf->cbuf == NULL )
		{
			ASSERT( FALSE && "release not initialized circle buffer" ); 
		} 
		else
		{
			w_lock_cbuffer( cbuf ); 

			uninit_cbuffer( cbuf->cbuf ); 

			unlock_cbuffer( cbuf ); 
		}

		uninit_cbuffer_lock( cbuf ); 

		cbuf->cbuf = NULL; 

	}while( FALSE );

	return ntstatus;  
}


PVOID get_cbuffer_item( cbuffer_t *buf, ULONG index )
{
	PVOID data = NULL; 

	do
	{
		if( index > buf->mask )
		{
			break; 
		}

		data = ( ( CHAR* )buf + buf->start + ( buf->cellsize * index ) ); 
	}while( FALSE ); 

	return data; 
}

/****************************************
notice: hold circle buffer lock when 
multiple thread accessing.
****************************************/
NTSTATUS uninit_cbuffer( cbuffer_t *buf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	NTSTATUS _ntstatus; 

	ASSERT( buf != NULL ); 

	do 
	{
#ifdef DBG

		if( NULL == buf )
		{
			ASSERT( FALSE ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}
#endif //DBG

		if( NULL == buf->release_cbuffer_item_func )
		{
			break; 
		}

		{
			ULONG i; 
			PVOID data; 
			buf->w = 0; 
			buf->r = 0; 

			for( i = 0; i <= buf->mask; i ++ )
			{
				data = get_cbuffer_item( buf, i ); 
				ASSERT( data != NULL ); 

				_ntstatus = buf->release_cbuffer_item_func( data ); 

				if( _ntstatus != STATUS_SUCCESS )
				{
					ntstatus = _ntstatus; 
				}
			}
		}

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS cbuffer_free( cbuffer_t *buf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( buf != NULL ); 

	if( NULL != buf )
	{
		ntstatus = uninit_cbuffer( buf ); 

#ifdef DRIVER
		ExFreePoolWithTag( buf, CBUFFER_POOL_TAG );
#else
		free( buf ); 
#endif //DRIVER

	}
#ifdef DBG
	else
	{
		ASSERT( FALSE ); 
		ntstatus = STATUS_INVALID_PARAMETER; 
	}
#endif //DBG

	return ntstatus; 
}

#ifdef DRIVER
NTSTATUS r_lock_cbuffer( safe_cbuffer *buf )
{
	//NTSTATUS ntstatus = STATUS_SUCCESS; 
	ASSERT( buf != NULL ); 

	lock_fmutex( &buf->lock ); 

	//ntstatus = hold_r_res_lock( buf->lock ); 
	//return ntstatus; 

	return STATUS_SUCCESS; 
}

NTSTATUS unlock_cbuffer( safe_cbuffer *buf )
{
	ASSERT( buf != NULL ); 

	unlock_fmutex( &buf->lock ); 
	//release_res_lock( buf->lock ); 
	
	return STATUS_SUCCESS; 
}
#else

NTSTATUS w_lock_cbuffer( safe_cbuffer *buf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	DWORD wait_ret; 

	ASSERT( buf != NULL ); 

	do 
	{
		wait_ret = WaitForSingleObject( buf->lock, INFINITE ); 

		if( wait_ret == WAIT_OBJECT_0 || wait_ret == WAIT_ABANDONED )
		{
#ifdef DEBUG
			if( wait_ret == WAIT_ABANDONED )
			{
				DBG_BP(); 
			}
#endif //DEBUG

			break; 
		}

#ifdef DEBUG
		if( wait_ret == WAIT_FAILED )
		{
			ULONG err_code; 
			err_code = GetLastError(); 

			log_trace( ( MSG_ERROR, "wait lock error 0x%0.8x\n", err_code ) ); 
			break; 
		}
#endif //DEBUG

	}while( FALSE );

	return STATUS_SUCCESS; ; 
}

NTSTATUS r_lock_cbuffer( safe_cbuffer *buf )
{
	NTSTATUS ntstatus; 
	ASSERT( buf != NULL ); 

	ntstatus = w_lock_cbuffer( buf ); 
	return ntstatus; 
}

NTSTATUS unlock_cbuffer( safe_cbuffer *buf )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	BOOL ret; 
	ASSERT( buf != NULL ); 
	ASSERT( buf->lock != NULL ); 

	ret = ReleaseMutex( buf->lock ); 
	
	if( ret == FALSE )
	{
		ntstatus = GetLastError(); 
	}

	return STATUS_SUCCESS; 
}
#endif //DRIVER

NTSTATUS cbuffer_write_quick( cbuffer_t *buf, PVOID data_in, ULONG data_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	PVOID data; // = NULL; 
	ULONG cell_count; 
	ULONG next_cell_count; 

	ASSERT( buf != NULL ); 
	ASSERT( data_in != NULL ); 
	ASSERT( data_size > 0 ); 

	do
	{
		if( data_size == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		cell_count = buf_size_2_cell_count( buf,data_size, NULL ); 

		ASSERT( cell_count != 0 ); 

#ifdef DBG
		if( cell_count > buf->mask + 1 )
		{
			ASSERT( FALSE && "read cbuffer too long" ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}
#endif //DBG

		next_cell_count = ( ULONG )( buf->w + cell_count ) - buf->r; 

		if( next_cell_count > buf->mask + 1 )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		data = ( CHAR* )( ( CHAR* )buf + ( buf->start ) + ( ( buf->w & buf->mask ) * buf->cellsize ) ); 

		memcpy( data, data_in, data_size ); 

#ifdef IO_IN_CLEAN_DATA_BUF
		{
			ULONG remain_size; 

			remain_size = ( data_size % buf->cellsize ); 

			if( remain_size > 0 )
			{
				remain_size = buf->cellsize - remain_size; 

				if( remain_size != 0 )
				{
					memset( data + ( cell_count * buf->cellsize ) - remain_size, 0, remain_size ); 
				}
			}

		}
#endif //IO_IN_CLEAN_DATA_BUF

		buf->w += cell_count; 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS safe_cbuffer_write_quick( safe_cbuffer *buf, PVOID data_in, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	void *data = NULL; 

	ASSERT( buf != NULL ); 
	ASSERT( data != NULL ); 

	w_lock_cbuffer( buf ); 

	ntstatus = cbuffer_write_quick( buf->cbuf, data_in, buf_size ); 

	unlock_cbuffer( buf ); 

	return ntstatus; 
}

NTSTATUS cbuffer_read_quick( cbuffer_t *buf, PVOID data_out, ULONG data_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	CHAR* data = NULL; 
	ULONG cell_count; 
	ULONG all_cell_count; 
	ULONG remain_size; 

	ASSERT( buf != NULL ); 
	ASSERT( data_out != NULL ); 


	do
	{

		cell_count = buf_size_2_cell_count( buf, data_size, &remain_size ); 

		all_cell_count = ( ULONG )( buf->w - buf->r ); 

		if( all_cell_count < cell_count )
		{
			ntstatus = STATUS_NO_MORE_ENTRIES; 
			break; 
		}

		data = ( ( CHAR* )buf + ( buf->start ) + ( ( buf->r & buf->mask ) * buf->cellsize ) );

		memcpy( data_out, data, data_size ); 

#ifdef IO_OUT_CLEAN_DATA_BUF
		if( 0 < remain_size )
		{

			ULONG _remain_size; 

			_remain_size = buf->cellsize - remain_size; 

			memset( data_out + ( cell_count * buf->cellsize ) - _remain_size, 0, _remain_size ); 

//			remain_size = ( data_size % buf->cellsize ); 
//
//			if( remain_size > 0 )
//			{
//				remain_size = buf->cellsize - remain_size; 
//
//				if( remain_size != 0 )
//				{
//					memset( data_out + ( cell_count * buf->cellsize ) - remain_size, 0, remain_size ); 
//				}
//			}
		}
#endif //IO_OUT_CLEAN_DATA_BUF

		buf->r += cell_count; 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS safe_cbuffer_read_quick( safe_cbuffer *buf, PVOID data_out, ULONG buf_size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	w_lock_cbuffer( buf ); 

	ntstatus = cbuffer_read_quick( buf->cbuf, data_out, buf_size ); 

	unlock_cbuffer( buf ); 

	return ntstatus; 
}

NTSTATUS cbuffer_read_begin( cbuffer_t *buf, cbuffer_iter *cbuf_iter )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	CHAR* data; 

	ASSERT( buf != NULL ); 
	ASSERT( cbuf_iter != NULL ); 

	do
	{
		cbuf_iter->buf = NULL; 

		if( cbuf_iter->buf_size == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		cbuf_iter->cell_count = buf_size_2_cell_count( buf, cbuf_iter->buf_size, NULL ); 


#ifdef DBG
		if( cbuf_iter->cell_count > buf->mask + 1 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //DBG

		//if( buf->r == buf->w )
		if( ( ULONG )( buf->w - buf->r ) < cbuf_iter->cell_count )
		{
			ntstatus = STATUS_NO_MORE_ENTRIES; 
			break; 
		}

		data = ( CHAR* )( ( CHAR* )buf + ( buf->start ) + ( ( buf->r & buf->mask ) * buf->cellsize ) );

		cbuf_iter->buf = data; 

		if( ( ULONG )( buf->r + cbuf_iter->cell_count ) > buf->mask + 1 )
		{
			ULONG all_buf_size; 

			all_buf_size = cbuf_iter->buf_size; 

			cbuf_iter->buf_size = buf->mask - buf->r + 1; 
			cbuf_iter->part_size = all_buf_size - cbuf_iter->buf_size; 

#ifdef DBG
			{
				ULONG remain_size; 

				remain_size = ( cbuf_iter->buf_size % buf->cellsize ); 

				ASSERT( remain_size == 0 ); 
			}
#endif //DBG

			{
#ifdef DBG
				ULONG all_cell_count; 
				all_cell_count = cbuf_iter->cell_count; 
#endif //DBG
				cbuf_iter->cell_count = cbuf_iter->buf_size / buf->cellsize; 
				cbuf_iter->part_cell_count = buf_size_2_cell_count( buf, cbuf_iter->part_size, NULL ); 

#ifdef DBG
				ASSERT( all_cell_count == cbuf_iter->cell_count + cbuf_iter->part_cell_count ); 
#endif //DBG
			}
			break; 
		}
		else
		{
			cbuf_iter->part_size = 0; 
			cbuf_iter->part_cell_count = 0; 
		}

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS safe_cbuffer_read_begin( safe_cbuffer *buf, cbuffer_iter *cbuf_iter )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( buf != NULL ); 
		ASSERT( cbuf_iter != NULL ); 

		w_lock_cbuffer( buf ); 

		ntstatus = cbuffer_read_begin( buf->cbuf, cbuf_iter ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS cbuffer_write_begin( cbuffer_t *buf, cbuffer_iter *cbuf_iter )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	CHAR *data; 
	ULONG next_cell_count; 

	ASSERT( buf != NULL ); 
	ASSERT( cbuf_iter != NULL ); 

	do
	{

		cbuf_iter->buf = NULL; 

		if( cbuf_iter->buf_size == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		cbuf_iter->cell_count = buf_size_2_cell_count( buf, cbuf_iter->buf_size, NULL ); 

#ifdef DBG
		if( cbuf_iter->cell_count > buf->mask + 1 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //DBG

		next_cell_count = ( ULONG )( buf->w + cbuf_iter->cell_count ) - buf->w; 

		if( next_cell_count > buf->mask + 1 )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 

			break; 
		}

		ASSERT( ( buf->w + cbuf_iter->cell_count ) < buf->mask + 1 ); 

		data = ( CHAR* )( ( CHAR* )buf + ( buf->start ) + ( ( buf->w & buf->mask ) * buf->cellsize ) ); 

		cbuf_iter->buf = data; 

		if( ( ULONG )( buf->w + cbuf_iter->cell_count ) > buf->mask + 1 )
		{
			ULONG all_buf_size; 

			all_buf_size = cbuf_iter->buf_size; 

			cbuf_iter->buf_size = buf->mask - buf->w + 1; 
			cbuf_iter->part_size = all_buf_size - cbuf_iter->buf_size; 

#ifdef DBG
			{
				ULONG remain_size; 

				remain_size = ( cbuf_iter->buf_size % buf->cellsize ); 

				ASSERT( remain_size == 0 ); 
			}
#endif //DBG

			{
#ifdef DBG
				ULONG all_cell_count; 
				all_cell_count = cbuf_iter->cell_count; 
#endif //DBG
				cbuf_iter->cell_count = cbuf_iter->buf_size / buf->cellsize; 
				cbuf_iter->part_cell_count = buf_size_2_cell_count( buf, cbuf_iter->part_size, NULL ); 
			
#ifdef DBG
				ASSERT( all_cell_count == cbuf_iter->cell_count + cbuf_iter->part_cell_count ); 
#endif //DBG
			}
			break; 
		}
		else
		{
			cbuf_iter->part_size = 0; 
			cbuf_iter->part_cell_count = 0; 
		}
	}while( FALSE ); 

	return ntstatus;
}

NTSTATUS safe_cbuffer_write_begin( safe_cbuffer *buf, cbuffer_iter *cbuf_iter )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( buf != NULL ); 
		ASSERT( cbuf_iter != NULL ); 

		w_lock_cbuffer( buf ); 
		ntstatus = cbuffer_write_begin( buf->cbuf, cbuf_iter ); 
	}while( FALSE );

	return ntstatus; 
}

VOID cbuffer_read_done( cbuffer_t *buf, cbuffer_iter *cbuf_iter )
{
	do
	{ 
		ASSERT( buf != NULL ); 
		ASSERT( cbuf_iter != NULL ); 

		InterlockedExchangeAdd( ( LONG* )&buf->r, cbuf_iter->cell_count + cbuf_iter->part_cell_count ); 

	}while( FALSE ); 
}

VOID cbuffer_write_done( cbuffer_t *buf, cbuffer_iter *cbuf_iter ) 
{
	do
	{ 
		ASSERT( buf != NULL ); 
		ASSERT( cbuf_iter != NULL ); 

		/***********************************************************
		use atomic operation, because other cpu or thread can read it
		when it be modifying, no synchronize this, is data took mixed? 
		or is previous value?
		************************************************************/

		InterlockedExchangeAdd( ( LONG* )&buf->w, cbuf_iter->cell_count + cbuf_iter->part_cell_count ); 

	}while( FALSE ); 
}

VOID safe_cbuffer_write_done( safe_cbuffer *buf, cbuffer_iter *cbuf_iter )
{
	cbuffer_write_done( buf->cbuf, cbuf_iter ); 

	unlock_cbuffer( buf ); 
}

VOID safe_cbuffer_read_done( safe_cbuffer *buf, cbuffer_iter *cbuf_iter )
{
	cbuffer_read_done( buf->cbuf, cbuf_iter ); 

	unlock_cbuffer( buf ); 
}

VOID cbuffer_clear( cbuffer_t* buf )
{
	ASSERT( buf != NULL ); 

	buf->w = buf->r = 0; 
}

VOID safe_cbuffer_clear( safe_cbuffer* buf)
{
	ASSERT( buf != NULL ); 

	w_lock_cbuffer( buf ); 

	cbuffer_clear( buf->cbuf ); 
	
	unlock_cbuffer( buf ); 
}

BOOLEAN safe_cbuffer_is_locked( safe_cbuffer *buf )
{
	BOOLEAN lock_held = FALSE; 
	ASSERT( buf != NULL ); 

	do
	{
#ifdef DRIVER
		lock_held = ExTryToAcquireFastMutex( &buf->lock ); 
		if( lock_held == TRUE )
		{
			ExReleaseFastMutex( &buf->lock ); 
		}
#endif //DRIVER
	}while( FALSE ); 

	return lock_held == TRUE ? FALSE : TRUE; 
}

#ifndef DRIVER
typedef struct _test_buf_cell
{
	ULONG data; 
} test_buf_cell, *ptest_buf_cell; 


#define TEST_CBUFFER_CELL_BIT ( 11 )
#define TEST_CBUFFER_CELL_SIZE ( 1 )

#define TEST_CBUFFER_WRITE 0x00000001
#define TEST_CBUFFER_READ 0x00000002

cbuffer_t *cbuf = NULL; 
//safe_cbuffer safe_buf = { 0 }; 
#define TEST_READ_THREAD_COUNT 6
#define TEST_WRITE_THREAD_COUNT 6

#define TEST_READ_TIME 6
#define TEST_WRITE_TIME 6
NTSTATUS test_cbuffer_work( PVOID cbuf_buf, ULONG flags )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	//cbuffer_t *buf_test = NULL;  
	safe_cbuffer safe_buf; 
	cbuffer_iter cbuf_iter; 
	test_buf_cell cell_data = { 0 }; 
	test_buf_cell *cell_data_ptr; 
	ULONG test_buf_bit_size = TEST_CBUFFER_CELL_BIT; 
	ULONG test_time; 
	ULONG i; 

	do
	{
		if( cbuf_buf == NULL )
		{
			ASSERT( FALSE ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		ntstatus = create_safe_cbuffer( &safe_buf, cbuf_buf, test_buf_bit_size, sizeof( test_buf_cell ), NULL ); 

		//ntstatus = cbuffer_create( NULL, test_buf_bit_size, sizeof( test_buf_cell ), NULL, &buf_test ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		if( flags == TEST_CBUFFER_READ )
		{
			for( test_time = 0; test_time < TEST_READ_TIME; test_time ++ )
			{
				//ASSERT( buf_test != NULL ); 

				for( i = 0; i < ( ULONG )( i << ( test_buf_bit_size + 1 ) ); i ++ )
				{
					cell_data.data = i; 
					ntstatus = safe_cbuffer_write_quick(  &safe_buf, ( PVOID )&cell_data, sizeof( test_buf_cell ) );  

					if( ntstatus == STATUS_SUCCESS )
					{
						dbg_print( MSG_INFO, "set test buffer cell %u data %u successfully\n", i, cell_data.data ); 
					}
					else
					{
						dbg_print( MSG_INFO, "set test buffer cell %u data %u error\n", i, cell_data.data ); 
					}

				}

				init_cbuffer_iter( &cbuf_iter ); 

				for( i = 0; i < ( ULONG )( i << ( test_buf_bit_size + 1 ) ); i ++ )
				{
					cbuf_iter.buf_size = sizeof( test_buf_cell ); 

					ntstatus = safe_cbuffer_write_begin( &safe_buf, &cbuf_iter );  

					if( ntstatus == STATUS_SUCCESS )
					{
						dbg_print( MSG_INFO, "get test buffer cell %u for write successfully\n", i, cell_data_ptr->data ); 

						cell_data_ptr = ( test_buf_cell* )cbuf_iter.buf; 

#ifdef DRIVER
						ASSERT( TRUE == safe_cbuffer_is_locked( &cbuf_iter ) ); 
#endif //DRIVER
						cell_data_ptr->data = ( 1 << test_buf_bit_size ) - i ; 

						safe_cbuffer_write_done( &safe_buf, &cbuf_iter ); 
					}
					else
					{
						dbg_print( MSG_INFO, "get test buffer cell %u for write error\n", i ); 
#ifdef DRIVER
						ASSERT( FALSE == safe_cbuffer_is_locked( &safe_buf ) ); 
#endif //DRIVER
					}

				}
			}
		}
		else if( flags == TEST_CBUFFER_WRITE )
		{
			for( test_time = 0; test_time < TEST_WRITE_TIME; test_time ++ )
			{
				for( i = 0; i < ( ULONG )( i << ( test_buf_bit_size + 1 ) ); i ++ )
				{
					ntstatus = safe_cbuffer_read_quick( &safe_buf, ( PVOID )&cell_data, sizeof( test_buf_cell ) );  

					if( ntstatus == STATUS_SUCCESS )
					{
						dbg_print( MSG_INFO, "get test buffer cell %u data %u successfully\n", i, cell_data.data ); 
					}
					else
					{
						dbg_print( MSG_INFO, "get test buffer cell %u error\n", i ); 
					}
				}

				init_cbuffer_iter( &cbuf_iter ); 

				for( i = 0; i < ( ULONG )( i << ( test_buf_bit_size + 1 ) ); i ++ )
				{
					cbuf_iter.buf_size = sizeof( test_buf_cell ); 
					ntstatus = safe_cbuffer_read_begin( &safe_buf, &cbuf_iter ); 

					cell_data_ptr = ( test_buf_cell* )cbuf_iter.buf; 
					if( ntstatus == STATUS_SUCCESS )
					{
						dbg_print( MSG_INFO, "get test buffer cell %u for read successfully data %u \n", i, cell_data_ptr->data ); 

#ifdef DRIVER
						ASSERT( TRUE == safe_cbuffer_is_locked( &safe_buf ) ); 
#endif //DRIVER
						safe_cbuffer_read_done( &safe_buf, &cbuf_iter ); 
					}
					else
					{
#ifdef DRIVER
						ASSERT( FALSE == safe_cbuffer_is_locked( &safe_buf ) ); 
#endif //DRIVER
						dbg_print( MSG_INFO, "get test buffer cell %u for read error\n", i ); 
					}
				}
			}
		}
		safe_cbuffer_clear( &safe_buf ); 

	}while( FALSE ); 

	return ntstatus; 
}

ULONG CALLBACK test_thread( PVOID param )
{
	ULONG ret = 0; 
	
	ret = test_cbuffer_work( cbuf, ( ULONG )( ULONG_PTR )param ); 
	return ret; 
}

HANDLE test_read_threads[ TEST_READ_THREAD_COUNT ] = { 0 }; 
HANDLE test_write_threads[ TEST_WRITE_THREAD_COUNT ] = { 0 }; 
NTSTATUS test_cbuffer()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG i; 

	do 
	{
		cbuf = ( cbuffer_t* )malloc( ( 1 << TEST_CBUFFER_CELL_BIT ) * TEST_CBUFFER_CELL_SIZE ); 
		if( cbuf == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			break; 
		}

		for( i = 0; i < TEST_READ_THREAD_COUNT; i ++ )
		{
			test_read_threads[ i ] = CreateThread( NULL, 
				0, 
				test_thread, 
				( PVOID )TEST_CBUFFER_READ, 
				0, 
				NULL ); 

			if( test_read_threads[ i ] == NULL )
			{
				dbg_print( MSG_ERROR, "create test thread %u error 0x%0.8x\n", i, GetLastError() ); 
				ntstatus = STATUS_UNSUCCESSFUL; 
			}
		}

		for( i = 0; i < TEST_READ_THREAD_COUNT; i ++ )
		{
			test_write_threads[ i ] = CreateThread( NULL, 
				0, 
				test_thread, 
				( PVOID )TEST_CBUFFER_WRITE, 
				0, 
				NULL ); 

			if( test_write_threads[ i ] == NULL )
			{
				dbg_print( MSG_ERROR, "create test thread %u error 0x%0.8x\n", i, GetLastError() ); 
				ntstatus = STATUS_UNSUCCESSFUL; 
			}
		}

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS stop_test_cbuffer()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG i; 

	do 
	{
		BOOL _ret; 
		for( i = 0; i < TEST_READ_THREAD_COUNT; i ++ )
		{
			if( test_read_threads[ i ] != NULL )
			{
				_ret = TerminateThread( test_read_threads[ i ], 0 ); 
				if( _ret == FALSE )
				{
					dbg_print( MSG_ERROR, "terminate the test thread %u error 0x%0.8x\n", i, GetLastError() ); 
				}

				test_read_threads[ i ] = NULL; 
			}
		}

		for( i = 0; i < TEST_WRITE_THREAD_COUNT; i ++ )
		{
			if( test_write_threads[ i ] != NULL )
			{
				_ret = TerminateThread( test_write_threads[ i ], 0 ); 
				if( _ret == FALSE )
				{
					dbg_print( MSG_ERROR, "terminate the test thread %u error 0x%0.8x\n", i, GetLastError() ); 
				}

				test_write_threads[ i ] = NULL; 
			}
		}

		if( cbuf != NULL )
		{
			free( cbuf ); 
			cbuf = NULL; 
		}
	}while( FALSE );

	return ntstatus; 
}

#endif //DRIVER

NTSTATUS safe_cbuffer_read_to_buf( safe_cbuffer *cbuf, PVOID dest_buf, ULONG dest_buf_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	cbuffer_iter cbuf_iter; 

	do 
	{
		ASSERT( cbuf != NULL ); 
		ASSERT( dest_buf != NULL ); 
		ASSERT( dest_buf_len > 0 ); 

		cbuf_iter.buf_size = dest_buf_len; 
		ntstatus = safe_cbuffer_read_begin( cbuf, &cbuf_iter ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}
		if( cbuf_iter.part_size != 0 )
		{
			ASSERT( cbuf_iter.part_size == 0 ); 

			memcpy( dest_buf, cbuf_iter.buf, dest_buf_len ); 
		}
		else
		{
			ASSERT( cbuf_iter.buf_size > 0 ); 
			ASSERT( cbuf_iter.part_size > 0 ); 
			ASSERT( cbuf_iter.buf_size + cbuf_iter.part_size == dest_buf_len ); 

			memcpy( dest_buf, cbuf_iter.buf, cbuf_iter.buf_size ); 
			memcpy( ( BYTE* )dest_buf + cbuf_iter.buf_size, 
				cbuffer_start( cbuf->cbuf ), 
				dest_buf_len - cbuf_iter.buf_size ); 
		}

		safe_cbuffer_read_done( cbuf, &cbuf_iter ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS safe_cbuffer_write_from_datas( safe_cbuffer *cbuf, PVOID data1, ULONG data1_len, PVOID data2, ULONG data2_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	cbuffer_iter cbuf_iter; 


	do 
	{
		cbuf_iter.buf = NULL; 
		cbuf_iter.cell_count = 0; 
		cbuf_iter.buf_size = data1_len + data2_len; 

		ntstatus = safe_cbuffer_write_begin( cbuf, &cbuf_iter  ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}
; 
		if( cbuf_iter.part_size == 0 )
		{
			ASSERT( cbuf_iter.part_cell_count == 0 ); 

			memcpy( cbuf_iter.buf, data1, data1_len ); 
			memcpy( ( BYTE* )cbuf_iter.buf + data1_len, data2, data2_len ); 
		}
		else
		{
			ASSERT( cbuf_iter.buf_size > 0 ); 
			ASSERT( cbuf_iter.part_size > 0 ); 

			if( cbuf_iter.buf_size < data1_len )
			{
				ASSERT( cbuf_iter.buf_size  + cbuf_iter.part_size == data1_len + data2_len ); 

				memcpy( cbuf_iter.buf, data1, cbuf_iter.buf_size ); 
				memcpy( ( BYTE* )cbuffer_start( cbuf->cbuf ), 
					( BYTE* )data1 + cbuf_iter.buf_size, data1_len - cbuf_iter.buf_size ); 

				memcpy( ( BYTE* )cbuffer_start( cbuf->cbuf ) + ( data1_len - cbuf_iter.buf_size ), 
					data2, data2_len ); 
			}
			else
			{
				memcpy( cbuf_iter.buf, data1, data1_len ); 

				memcpy( ( BYTE* )cbuf_iter.buf + data1_len, data2, cbuf_iter.buf_size - data1_len ); 

				memcpy( ( BYTE* )cbuffer_start( cbuf->cbuf ), 
					( BYTE* )data2 + cbuf_iter.buf_size - data1_len, data2_len - ( cbuf_iter.buf_size - data1_len ) ); 
			}
		}

		safe_cbuffer_write_done( cbuf, &cbuf_iter ); 

	}while( FALSE );

	return ntstatus; 
}