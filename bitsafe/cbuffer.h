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

#ifndef _CIRCULAR_BUFFER
#define _CIRCULAR_BUFFER

#define CBUFFER_LOCK_NAME L"Global\\SAFE_CBUFFER_{2013}"
#define CBUFFER_POOL_TAG ( ULONG )'cric'
#define MAX_BIT_SIZE 33

#define DEBUG_CBUFFER_LOCK 1

typedef NTSTATUS ( *release_cbuffer_item )( PVOID item_data ); 


typedef struct _cbuffer_t {
	//	KSPIN_LOCK guard; 
#ifdef DEBUG_CBUFFER_LOCK
	BOOLEAN cbuf_w_locked; 
	BOOLEAN cbuf_r_locked; 
#endif //DEBUG_CBUFFER_LOCK


	ULONG start;                  /* the start address of buffer */
	ULONG w;                        /* the position of the write pointer */
	ULONG r;                        /* the position of the read pointer */
	ULONG mask;
	ULONG cellsize; 
	release_cbuffer_item release_cbuffer_item_func; 
} cbuffer_t, *pcbuffer_t; 

typedef struct _safe_cbuffer
{
#ifdef DRIVER
	FAST_MUTEX lock; 
#else
	HANDLE lock; 
#endif //DRIVER

	cbuffer_t *cbuf; 
} safe_cbuffer, *psafe_cbuffer; 

typedef struct _cbuffer_iter
{
	PVOID buf; 
	ULONG buf_size; 
	ULONG cell_count; 
	ULONG part_size; 
	ULONG part_cell_count; 
}cbuffer_iter, *pcbuffer_iter; 

//#endif //CBUFFER_DEF

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif //MIN

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

	NTSTATUS cbuffer_init( cbuffer_t *buf, ULONG bitsize, ULONG cellsize, release_cbuffer_item release_item_func ); 
	NTSTATUS cbuffer_create( PVOID cbuf_in, ULONG bitsize, ULONG cellsize, release_cbuffer_item release_item_func, cbuffer_t **buf_out ); 

	NTSTATUS safe_cbuffer_attach( safe_cbuffer *cbuf, PVOID buf_in, ULONG buf_size ); 
	NTSTATUS safe_cbuffer_detach( safe_cbuffer *cbuf ); 
	
	NTSTATUS create_safe_cbuffer( safe_cbuffer *cbuf, PVOID buf_in, ULONG bitsize, ULONG cellsize, release_cbuffer_item release_item_func ); 
	NTSTATUS destroy_safe_cbuffer( safe_cbuffer *cbuf ); 
	NTSTATUS uninit_safe_cbuffer( safe_cbuffer *cbuf ); 

	NTSTATUS uninit_cbuffer( cbuffer_t *buf ); 
	NTSTATUS cbuffer_free( cbuffer_t *buf ); 

	FORCEINLINE PVOID cbuffer_start( cbuffer_t *buf )
	{
		ASSERT( buf != NULL ); 

		return ( CHAR* )buf + buf->start; 
	}

	FORCEINLINE ULONG cbuffer_count( cbuffer_t *buf ) 
	{
		ASSERT( buf != NULL ); 
		return ( ( ULONG )buf->w - ( ULONG )buf->r ); 
	}

	INLINE NTSTATUS init_cbuffer_iter( cbuffer_iter *cbuf_iter )
	{
		ASSERT( cbuf_iter != NULL ); 
		cbuf_iter->buf = NULL; 
		cbuf_iter->buf_size = 0; 
		cbuf_iter->cell_count = 0; 
		cbuf_iter->part_size = 0; 
		cbuf_iter->part_cell_count = 0; 

		return STATUS_SUCCESS; 
	}


	NTSTATUS w_lock_cbuffer( safe_cbuffer *buf ); 
	NTSTATUS r_lock_cbuffer( safe_cbuffer *buf ); 
	NTSTATUS unlock_cbuffer( safe_cbuffer *buf ); 
	NTSTATUS init_cbuffer_lock( safe_cbuffer *buf ); 
	NTSTATUS uninit_cbuffer_lock( safe_cbuffer *cbuf ); 

#ifdef DRIVER

	FORCEINLINE NTSTATUS lock_fmutex( FAST_MUTEX *lock )
	{
		ASSERT( lock != NULL ); 

		ExAcquireFastMutex( lock ); 

		return STATUS_SUCCESS; 
	}

	FORCEINLINE NTSTATUS unlock_fmutex( FAST_MUTEX *lock )
	{
		ASSERT( lock != NULL ); 

		ExReleaseFastMutex( lock ); 

		return STATUS_SUCCESS; 
	}
#endif //DRIVER

	NTSTATUS cbuffer_write_quick( cbuffer_t *buf, PVOID data_in, ULONG buf_size ); 
	NTSTATUS cbuffer_read_quick(cbuffer_t *buf, PVOID data_out, ULONG buf_size ); 
	NTSTATUS cbuffer_read_begin( cbuffer_t *buf, cbuffer_iter *cbuf_iter ); 
	NTSTATUS cbuffer_write_begin( cbuffer_t *buf, cbuffer_iter *cbuf_iter ); 
	VOID cbuffer_read_done( cbuffer_t *buf, cbuffer_iter *cbuf_iter ); 
	VOID cbuffer_write_done( cbuffer_t *buf, cbuffer_iter *cbuf_iter ) ; 

	NTSTATUS safe_cbuffer_write_quick( safe_cbuffer *buf, PVOID data_in, ULONG buf_size ); 
	NTSTATUS safe_cbuffer_read_quick( safe_cbuffer *buf, PVOID data_out, ULONG buf_size ); 
	
	NTSTATUS safe_cbuffer_read_begin( safe_cbuffer *buf, cbuffer_iter *cbuf_iter ); 
	NTSTATUS safe_cbuffer_write_begin( safe_cbuffer *buf, cbuffer_iter *cbuf_iter ); 

	VOID safe_cbuffer_write_done( safe_cbuffer *buf, cbuffer_iter *cbuf_iter ); 
	VOID safe_cbuffer_read_done( safe_cbuffer *buf, cbuffer_iter *cbuf_iter ); 

	void cbuffer_clear( cbuffer_t* buf ); 
	void safe_cbuffer_clear( safe_cbuffer* buf); 
	BOOLEAN safe_cbuffer_is_locked( safe_cbuffer *buf ); 

	PVOID get_cbuffer_item( cbuffer_t *buf, ULONG index ); 

	INLINE BOOLEAN check_cbuffer_write_depart( cbuffer_t *cbuf, 
		cbuffer_iter *cbuf_iter, 
		ULONG *cbuf_first_size, 
		ULONG *cbuf_last_size )
	{
		BOOLEAN ret = TRUE; 

		do 
		{
			ASSERT( cbuf != NULL ); 
			ASSERT( cbuf_iter != NULL ); 

			if( ( ULONG )( cbuf->w + cbuf_iter->cell_count ) > cbuf->mask + 1 )
			{
				*cbuf_first_size = cbuf->mask - cbuf->w + 1; 
				*cbuf_last_size = cbuf_iter->cell_count - *cbuf_first_size; 
				
				break; 
			}
			else
			{
				*cbuf_first_size = 0; 
				*cbuf_last_size = 0; 

				ret = FALSE; 
			}

		}while( FALSE );
	
		return ret; 
	}


#ifndef DRIVER

	NTSTATUS test_cbuffer(); 
	NTSTATUS stop_test_cbuffer(); 

#endif //DRIVER

	INLINE NTSTATUS is_valid_cbuffer( cbuffer_t *buf )
	{
		NTSTATUS ntstatus = STATUS_SUCCESS; 

		ASSERT( buf != NULL ); 

		do
		{
			if( buf->cellsize == 0 )
			{
				ntstatus = STATUS_INVALID_PARAMETER_1; 
				break; 
			}

			if( buf->mask == 0 && buf->mask == 0xffffffff )
			{
				ntstatus = STATUS_INVALID_PARAMETER_2; 
				break; 
			}

			if( buf->start != sizeof( cbuffer_t ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER_3; 
				break; 
			}
		}while( FALSE ); 

		return ntstatus; 
	}

	INLINE VOID dump_cbuffer( cbuffer_t *buf )
	{
		ASSERT( buf != NULL ); 

		dbg_print( MSG_ERROR, "the circle buffer:\n cell size %u\n mask 0x%0.8x\n read position %u\n write position %u\n release function 0x%0.8x\n start position %u\n", 
			buf->cellsize, 
			buf->mask, 
			buf->r, 
			buf->w, 
			buf->release_cbuffer_item_func, 
			buf->start ); 

		return; 
	}

	NTSTATUS safe_cbuffer_read_to_buf( safe_cbuffer *cbuf, PVOID dest_buf, ULONG dest_buf_len ); 
	NTSTATUS safe_cbuffer_write_from_datas( safe_cbuffer *cbuf, PVOID data1, ULONG data1_len, PVOID data2, ULONG data2_len ); 

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_CIRCULAR_BUFFER
