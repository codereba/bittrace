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
#include <ntstatus.h>
#endif //DRIVER

#include "cbuffer.h"
#include "unit_cbuffer.h"


NTSTATUS output_sys_action_desc( safe_cbuffer *cbuf, sys_action_desc *action_output, PVOID data, ULONG data_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG action_output_size; 
	cbuffer_iter cbuf_iter; 
	sys_action_output *output; 
	BOOLEAN cbuf_fill_partial = FALSE; 

	do 
	{
		ASSERT( cbuf !=	NULL ); 
		ASSERT( action_output != NULL ); 

#ifdef DBG
		if( data != NULL )
		{
			ASSERT( data_len == 0 ); 
		}
		else
		{
			ASSERT( data_len != 0 ); 
		}
#endif //DBG

		action_output_size = calc_action_output_size( data_len ); 

		for( ; ; )
		{
			cbuf_iter.buf = NULL; 
			cbuf_iter.cell_count = 0; 
			cbuf_iter.buf_size = action_output_size; 

			ntstatus = safe_cbuffer_write_begin( cbuf, &cbuf_iter ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}

			ASSERT( cbuf_iter.buf != NULL ); 

			if( 0 == cbuf_iter.part_size )
			{
				ASSERT( cbuf_iter.part_cell_count == 0 ); 

				output = ( sys_action_output* )cbuf_iter.buf; 
				output->magic = SYS_ACTION_OUTPUT_MAGIC; 
				output->size = action_output_size; 
				memcpy( &output->action, action_output, sizeof( sys_action_desc ) ); 

				if( data != NULL )
				{
					memcpy( output->data, data, data_len ); 			
				}

				//this ending
				output->data[ data_len ] = 0; 

				safe_cbuffer_write_done( cbuf, &cbuf_iter ); 
				break; 
			}
			else
			{
				if( cbuf_fill_partial == TRUE )
				{
					ASSERT( FALSE && "partial buffer in the buffer is filled, but still need depart? bug." ); 
				}

				ASSERT( cbuf_iter.part_cell_count != 0 ); 
				ASSERT( cbuf_iter.buf_size + cbuf_iter.part_size == action_output_size ); 

#ifdef DBG
				if( cbuf_iter.buf_size >= FIELD_OFFSET( sys_action_output, action ) )
				{
					output = ( sys_action_output* )cbuf_iter.buf; 
					output->magic = SYS_ACTION_PLACEHOLDER_MAGIC; 
					output->size = cbuf_iter.buf_size; 
				}			
#endif //DBG

				cbuf_iter.part_cell_count = 0; 
				cbuf_iter.part_size = 0; 
				safe_cbuffer_write_done( cbuf, &cbuf_iter ); 

				cbuf_fill_partial = TRUE; 
			}
		}		

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS read_sys_action_desc_begin( safe_cbuffer *cbuf, cbuffer_iter *cbuf_iter_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG action_output_size; 
	cbuffer_iter cbuf_iter; 
	sys_action_output *output; 
	BOOLEAN cbuf_skip_partial = FALSE; 

	do 
	{
		ASSERT( cbuf !=	NULL ); 
		ASSERT( cbuf_iter_out != NULL ); 

		init_cbuffer_iter( cbuf_iter_out ); 

		action_output_size = sizeof( sys_action_output ); 
		cbuf_iter.buf = NULL; 
		cbuf_iter.cell_count = 0; 
		cbuf_iter.buf_size = action_output_size; 

		for( ; ; )
		{

			ntstatus = safe_cbuffer_read_begin( cbuf, &cbuf_iter ); 
			if( ntstatus != STATUS_SUCCESS )
			{
				break; 
			}

			ASSERT( cbuf_iter.buf != NULL ); 

			output = ( sys_action_output* )cbuf_iter.buf; 

			if( 0 == cbuf_iter.part_size )
			{
				ASSERT( cbuf_iter.part_cell_count == 0 ); 

				if( output->size == 0 || output->size > MAX_SYS_ACTION_UNIT_SIZE )
				{
					ntstatus = STATUS_UNSUCCESSFUL; 

					ASSERT( FALSE && "cbuffer cruppted" ); 
#define CBUFFER_CRUPPTED ((ULONG)0x800000C5L)
#ifdef DRIVER
					KeBugCheckEx( CBUFFER_CRUPPTED, __LINE__, ( LONG_PTR )output, ( LONG_PTR )action_output_size, ( LONG_PTR )cbuf->cbuf ); 
#endif //DRIVER			
					break; 
				}

				if( action_output_size == output->size )
				{
					*cbuf_iter_out = cbuf_iter; 
					break; 
				}
				else if( action_output_size < output->size )
				{
					if( action_output_size > sizeof( sys_action_output ) )
					{
						ntstatus = STATUS_UNSUCCESSFUL; 

						ASSERT( FALSE && "cbuffer cruppted" ); 

#ifdef DRIVER
						KeBugCheckEx( CBUFFER_CRUPPTED, __LINE__, ( LONG_PTR )output, ( LONG_PTR )action_output_size, ( LONG_PTR )cbuf->cbuf ); 
#endif //DRIVER
						break; 	
					}

					action_output_size = output->size; 
					cbuf_iter.buf = NULL; 
					cbuf_iter.cell_count = 0; 				
					cbuf_iter.buf_size = action_output_size; 
					continue; 
				}
				else
				{
					ntstatus = STATUS_UNSUCCESSFUL; 

					ASSERT( FALSE && "cbuffer cruppted" ); 
#ifdef DRIVER
					KeBugCheckEx( CBUFFER_CRUPPTED, __LINE__, ( LONG_PTR )output, ( LONG_PTR )action_output_size, ( LONG_PTR )cbuf->cbuf ); 
#endif //DRIVER			
					break; 					
				}
			}
			else
			{
				if( cbuf_iter.buf_size >= FIELD_OFFSET( sys_action_output, action ) )
				{
					if( output->magic != SYS_ACTION_PLACEHOLDER_MAGIC )
					{
						ASSERT( FALSE ); 
					}

					if( output->size != cbuf_iter.buf_size )
					{
						ASSERT( FALSE ); 
					}
				}

				cbuf_iter.part_size = 0; 
				cbuf_iter.part_cell_count = 0; 

				safe_cbuffer_read_done( cbuf, &cbuf_iter ); 
				continue; 
			}
		}

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS read_sys_action_desc_end( safe_cbuffer *cbuf, cbuffer_iter *cbuf_iter )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	do 
	{
		ASSERT( cbuf != NULL ); 
		ASSERT( cbuf_iter != NULL ); 

		safe_cbuffer_read_done( cbuf, cbuf_iter ); 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS cbuffer_copy_sys_action_desc( safe_cbuffer *cbuf, sys_action_output *output, ULONG buf_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	NTSTATUS _ntstatus; 
	cbuffer_iter cbuf_iter; 

	do 
	{
		ASSERT( cbuf != NULL ); 
		ASSERT( output != NULL ); 
		ASSERT( buf_len >= sizeof( sys_action_output ) ); 

		_ntstatus = read_sys_action_desc_begin( cbuf, &cbuf_iter ); 

		if( _ntstatus != STATUS_SUCCESS )
		{
			ntstatus = _ntstatus; 

			break; 
		}

		do 
		{
			ASSERT( cbuf_iter.part_size == 0 && cbuf_iter.part_cell_count == 0 ); 

			if( cbuf_iter.buf_size > buf_len )
			{
				ntstatus = STATUS_BUFFER_TOO_SMALL; 
				break; 
			}

			memcpy( output, cbuf_iter.buf, cbuf_iter.buf_size ); 

		}while( FALSE );

		_ntstatus = read_sys_action_desc_end( cbuf, &cbuf_iter ); 
		ASSERT( _ntstatus == STATUS_SUCCESS ); 
	}while( FALSE );

	return ntstatus; 
}