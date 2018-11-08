/*
 *
 * Copyright 2010 JiJie Shi
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
#include "ring0_2_ring3.h"
#include "action_type.h"
#include "action_check.h"
#include "action_source.h"

#define PROC_STATE_HASH_TABLE_SIZE 2048
ref_node_tbl action_sources; 
NTSTATUS get_action_source_param( ref_obj* obj, PVOID *param )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	action_source_info *source; 

	ASSERT( obj != NULL ); 
	ASSERT( param != NULL ); 

	source = ( action_source_info* )CONTAINING_RECORD( obj, action_source_info, obj ); 

	*param = ( PVOID )&source->source; 

	return ntstatus; 
}

INLINE BOOLEAN compare_action_source_data( action_source_data *src, 
										  action_source_data *dst )
{
	BOOLEAN ret = FALSE; 

	do 
	{
		if( src->job != dst->job )
		{
			break; 
		}

		if( src->thrd != dst->thrd )
		{
			break; 
		}

		ret = TRUE; 

	}while( FALSE );

	return ret; 
}

BOOLEAN compare_action_source( ref_obj *obj, PVOID param )
{
	BOOLEAN ret = FALSE; 
	HANDLE proc_id; 
	action_source_info *info; 
	register action_source_data *src_source; 

	//ASSERT( param != NULL ); 
	ASSERT( obj != NULL ); 

	src_source = ( action_source_data* )param; 

	info = ( action_source_info* )CONTAINING_RECORD( obj, action_source_info, obj ); 

	if( compare_action_source_data( src_source, &info->source ) == TRUE );
	{
		ret = TRUE; 
	}

	return ret; 
}

ULONG calc_action_source_hash( PVOID param, ULONG size )
{
	ULONG hash_code; 
	register action_source_data *data; 

	data = ( action_source_data* )param; 

	hash_code = ( ULONG )( ( ULONG_PTR )data->proc + ( ULONG_PTR )data->thrd ) % size; 

	return hash_code; 
}

NTSTATUS alloc_action_source( PVOID param, ref_obj **obj_alloc )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	action_source_info *info; 
	action_source_data *source; 

	ASSERT( obj_alloc != NULL ); 

	*obj_alloc = NULL; 

	info = ( action_source_info* )ALLOC_TAG_POOL( sizeof( action_source_info ) ); 
	if( info == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	init_action_source_ctx( &info->ctx ); 

	source = ( action_source_data* )param; 

	set_action_source_data( &info->source, source ); 

	*obj_alloc = &info->obj; 

_return:
	return ntstatus; 
}

INLINE NTSTATUS init_action_source_ctx( action_source_ctx *ctx )
{
	ASSERT( ctx != NULL ); 

	memset( ctx->active_state.hitted_polices, 0, sizeof( ctx->active_state.hitted_polices ) ); 
	ctx->active_state.policy_count = 0; 

	return STATUS_SUCCESS; 
}

INLINE NTSTATUS set_action_source_data( action_source_data *src, action_source_data *dst )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( src != NULL ); 
		ASSERT( dst != NULL ); 

		src->job = dst->job; 
		src->thrd = dst->thrd; 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS CALLBACK release_action_source( ref_obj *obj )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	action_source_info *info; 

	ASSERT( obj != NULL ); 

	info = ( action_source_info* )CONTAINING_RECORD( obj, action_source_info, obj ); 

	ASSERT( info->obj.ref_count == 0 ); 
	ASSERT( TRUE == IsListEmpty( &info->obj.entry ) ); 

	FREE_TAG_POOL( info ); 

	return ntstatus; 
}

INLINE NTSTATUS ref_action_source( action_source_info *source )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ntstatus = _reference_obj( &action_sources, 
			&source->obj ); 
	} while ( FALSE );

	return ntstatus; 
}

INLINE NTSTATUS deref_action_source( action_source_info *info )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ntstatus = deref_obj( &action_sources, &info->obj ); 
	
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS add_action_source( action_source_data *source )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG hash_code; 
	PLIST_ENTRY item_found; 
	ref_obj *_obj = NULL; 

	do 
	{
		if( source == NULL )
		{
			ASSERT( FALSE && "why add null object to action source?" ); 
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		//info = ( action_source_info* )malloc( sizeof( action_source_info ) ); 

		//if( info == NULL )
		//{
		//	ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		//	break; 
		//}

		//info->source = source; 

		//ntstatus = add_to_common_hash_table( &all_proc_state, 
		//	calc_proc_id_hash_code, 
		//	compare_proc_id_in_table, 
		//	&info->entry,  
		//	( PVOID )source,  
		//	NULL ); 

		ntstatus = add_ref_obj( &action_sources, 
			source, 
			&_obj ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			ASSERT( _obj == NULL ); 
			break; 
		}

		ASSERT( _obj != NULL ); 
	}while( FALSE );

	if( ntstatus != STATUS_SUCCESS )
	{
		ASSERT( _obj == NULL ); 

		//ASSERT( TRUE == IsListEmpty( &info->entry ) ); 

		//if( info != NULL )
		//{
		//	free( info ); 
		//}
	}

	return ntstatus; 
}

/*******************************************************
动态策略系统运作过程：
1.初始化系统的行为链策略。
2.对系统中某个行为源（JOB，进程，进程组，线程）加入跟踪列表中。
3.在行为源产生行为进行抓取，检查，保存检查状态，执行响应行为。
4.在行为源退出时，将其中跟踪列表中删除。
*******************************************************/


NTSTATUS del_action_source( action_source_data *source )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG ref_count; 

	do 
	{
		if( source == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		//ntstatus = del_from_common_hash_table( &all_proc_state, 
		//	calc_proc_id_hash_code, 
		//	compare_proc_id_in_table, 
		//	del_proc_in_table, 
		//	( PVOID )proc_id,  
		//	NULL ); 

		ntstatus = del_ref_obj( &action_sources, 
			source, 
			&ref_count ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ntstatus; 
}

ULONG CALLBACK calc_action_source_hash_code( PVOID param1, 
									  PVOID param2, 
									  ULONG size )
{
	ULONG hash_code; 

	//parameter 1 is the process id.
	hash_code = ( ULONG )( ( ULONG )( ULONG_PTR )param1 % size ); 

	return hash_code; 
}

/******
notice: must is already removed from the list table
******/

NTSTATUS CALLBACK del_action_source_in_table( PLIST_ENTRY element )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( TRUE == IsListEmpty( element ) ); 

		free( element ); 

	}while( FALSE );

	return ntstatus; 
}

INLINE BOOLEAN is_valid_source_data( action_source_data *source_data ) 
{
	return ( source_data->proc != INVALID_PROCESS_ID 
		&& source_data->thrd != INVALID_PROCESS_ID ); 
}

NTSTATUS find_action_source( action_source_data *source_data, 
							action_source_info **source )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	action_source_info *_source = NULL; 
	ref_obj *obj = NULL; 

	do 
	{
		ASSERT( source_data != NULL ); 
		ASSERT( source != NULL ); 

		*source = NULL; 
		if( FALSE == is_valid_source_data( source_data ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		ntstatus = reference_obj( &action_sources,  
			( PVOID )source_data, 
			&obj, 
			NULL ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			ASSERT( obj == NULL ); 
			//break; 
		
			ntstatus = add_ref_obj( &action_sources, 
				source_data, 
				&obj ); 

			if( ntstatus != STATUS_SUCCESS )
			{
				ASSERT( obj == NULL ); 
				break; 
			}
		}

		ASSERT( obj != NULL ); 

		_source = CONTAINING_RECORD( obj, action_source_info, obj ); 

		*source = _source; 

	}while( FALSE );

	return ntstatus; 
}

//NTSTATUS register_action_source( PVOID object, ACTION_SOURCE **source_output )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	ACTION_SOURCE *source = NULL; 
//	do 
//	{
//		if( object == NULL )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_1; 
//			break;
//		}
//
//		if( source_output == NULL )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_2; 
//			break; 
//		}
//
//		source = ( ACTION_SOURCE* )ALLOC_TAG_POOL( sizeof( ACTION_SOURCE ) ); 
//
//		if( source == NULL )
//		{
//			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
//			break; 
//		}
//
//		source->object = object; 
//		ntstatus = init_common_policy_nfa( &source->policies ); 
//
//		if( ntstatus != STATUS_SUCCESS )
//		{
//			break; 
//		}
//
//	}while ( FALSE );
//
//	return ntstatus; 
//}

NTSTATUS init_action_sources()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
#define ACTION_SOURCE_HASH_TABLE_SIZE 20

		ntstatus = init_ref_obj_tbl( &action_sources, 
			ACTION_SOURCE_HASH_TABLE_SIZE, 
			compare_action_source, 
			alloc_action_source, 
			release_action_source, 
			calc_action_source_hash, 
			get_action_source_param, 
			NULL, 
			NULL, 
			NULL, 
			NULL ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS uninit_action_sources()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
#define ACTION_SOURCE_HASH_TABLE_SIZE 20

		ntstatus = uninit_ref_obj_tbl( &action_sources ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

	}while( FALSE );

	return ntstatus; 
}
