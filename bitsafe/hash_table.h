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

#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#pragma pack( push )
#pragma pack( 1 )

#define DEL_FILTER_TEXT 0
#define ADD_FILTER_TEXT 1

#define ERR_NOT_FOUND -11

#ifdef USER_PROC_TEST


#endif

typedef NTSTATUS ( CALLBACK *hold_lock_callback )( PVOID lock ); 
typedef NTSTATUS ( CALLBACK *release_lock_callback )( PVOID lock ); 
typedef struct _common_hash_table
{
	LIST_ENTRY *hash_table;
	//LIST_ENTRY all_tcpip_info; 
	PVOID lock; 
	hold_lock_callback hold_lock; 
	hold_lock_callback hold_r_lock; 
	release_lock_callback release_lock; 
	ULONG size; 
	ULONG count; 
} common_hash_table, *pcommon_hash_table;

typedef NTSTATUS ( CALLBACK *release_hash_element_func )( PLIST_ENTRY element ); 
typedef NTSTATUS ( CALLBACK *traverse_hash_element_func )( PLIST_ENTRY element, PBYTE output, ULONG length, PULONG need_length, PULONG output_length );  
typedef NTSTATUS ( CALLBACK *search_hash_element_func )( PLIST_ENTRY element, PVOID param1, PVOID param2 );  
typedef ULONG ( CALLBACK *calc_param_hash_func )( PVOID param1, PVOID param2, ULONG size ); 
#pragma pack( pop )

#define foreach_entry_list( list, entry ) for( entry = ( list )->Flink; entry != list; entry = ( entry )->Flink )
#define foreach_entry_list_safe( list, entry, next_backup ) for( entry = ( list )->Flink, next_backup = ( entry )->Flink; entry != list; entry = ( next_backup ), next_backup = ( entry ) )

#define TCPIP_HASH_TABLE_SIZE ( 2048 )

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	NTSTATUS CALLBACK release_hash_element_place_holder( PLIST_ENTRY element ); 

NTSTATUS init_common_hash_table( pcommon_hash_table table, 
							ULONG size, 
							PVOID lock, 
							hold_lock_callback r_lock_func, 
							hold_lock_callback w_lock_func, 
							release_lock_callback unlock_func ); 

NTSTATUS release_common_hash_table( pcommon_hash_table ip_info_hash, release_hash_element_func release_func ); 
VOID clear_common_hash_table( pcommon_hash_table table, release_hash_element_func release_func ); 
NTSTATUS traverse_common_hash_table( pcommon_hash_table ip_info_hash, traverse_hash_element_func release_func, PBYTE output, ULONG length, PULONG need_length, PULONG output_length ); 

NTSTATUS del_from_common_hash_table( pcommon_hash_table table, 
									calc_param_hash_func hash_func, 
									search_hash_element_func search_func, 
									release_hash_element_func del_func, 
									PVOID param1, 
									PVOID param2 ); 

NTSTATUS add_to_common_hash_table( pcommon_hash_table table, 
								  calc_param_hash_func hash_func, 
								  search_hash_element_func search_func, 
								  PLIST_ENTRY entry, 
								  PVOID param1, 
								  PVOID param2 ); 

NTSTATUS search_common_hash_table( pcommon_hash_table table, calc_param_hash_func hash_func, search_hash_element_func search_func, PVOID param1, PVOID param2 ); 

#define GET_HASH_CODE( value, size ) ( value & ( size - 1 ) )

INLINE ULONG calc_2str_hash_code( CHAR *name1, ULONG name1_len, CHAR *name2, ULONG name2_len, ULONG table_size )
{
	ULONG hash_code = 0; 
	CHAR *name1_hash_code; 
	CHAR *name2_hash_code; 

	ASSERT( NULL != name1 ); 

	log_trace( ( MSG_INFO, "enter %s name1 %s, name2 is %s \n", name1, name2 == NULL ? "NULL" : name2 ) ); 

	DUMP_MEM( name1, name1_len ); 
	DUMP_MEM( name2, name2_len ); 

	name1_hash_code = ( CHAR* )&hash_code; 
	name2_hash_code = name1_hash_code + 2; 

	if( name2 != NULL )
	{
		if( name1_len >= 2 )
		{
			memcpy( name1_hash_code, name1, 2 ); 
		}
		else
		{
			memcpy( name1_hash_code, name1, name1_len ); 
		}

		if( name2_len >= 2 )
		{
			memcpy( name2_hash_code, name2, 2 ); 
		}
		else
		{
			memcpy( name2_hash_code, name2, name2_len ); 
		}
	}
	else
	{
		if( name1_len >= 4 )
		{
			memcpy( name1_hash_code, name1, 4 ); 
		}
		else
		{
			memcpy( name1_hash_code, name1, name1_len ); 
		}
	}

	DBGPRINT( ( "leave %s hash code 0x%0.8x \n", __FUNCTION__, hash_code ) ); 
	return hash_code % table_size; 
}

INLINE ULONG copy_hash_buf( const UCHAR* data, ULONG data_len, ULONG table_size )
{
	ULONG hash_code = 0;

	RtlCopyMemory( &hash_code, data, data_len >= 4 ? 4 : data_len );

	return hash_code % table_size;
}

INLINE ULONG hash_buffer( PUCHAR data, ULONG data_len, ULONG table_size )
{
	ULONG hash_val = 0;
	INT32 i;

	for( i = 0; ( ULONG )i < data_len; i += sizeof( ULONG ) )
	{
		hash_val ^= *( ( PULONG )( data + i ) );
		hash_val = hash_val << 3;
	} 

	i -= sizeof( ULONG );

	for( ; ( ULONG )i < data_len; i ++ )
	{
		hash_val ^= ( ULONG )( *data );
		hash_val = hash_val << 3;
	}
	return hash_val % table_size;
}

ULONG unicode_fix_len_str_hash( LPCWSTR str, ULONG len, ULONG table_size ); 
ULONG unicode_str_hash( LPCWSTR str, ULONG table_size ); 

ULONG ansi_str_hash( LPCSTR str, ULONG table_size ); 
ULONG ansi_fix_len_str_hash( LPCSTR str, ULONG len, ULONG table_size ); 

INLINE ULONG long_hash( ULONG num, ULONG size )
{
	return ( num >> 2 ) % size; 
}

INLINE NTSTATUS hold_hash_table_lock( pcommon_hash_table table )
{
	NTSTATUS ntstatus; 
	ASSERT( table != NULL ); 
	ASSERT( table->hold_lock != NULL ); 

	ntstatus = table->hold_lock( table->lock ); 

	
	return ntstatus; 
}

INLINE NTSTATUS hold_hash_table_r_lock( pcommon_hash_table table )
{
	NTSTATUS ntstatus; 
	ASSERT( table != NULL ); 
	ASSERT( table->hold_r_lock != NULL ); 

	ntstatus = table->hold_r_lock( table->lock ); 


	return ntstatus; 
}

INLINE NTSTATUS release_hash_table_lock( pcommon_hash_table table )
{
	NTSTATUS ntstatus; 
	ASSERT( table != NULL ); 
	ASSERT( table->release_lock != NULL ); 

	ntstatus = table->release_lock( table->lock ); 

	return ntstatus; 
}

INLINE VOID insert_to_hash_table_lock_free( pcommon_hash_table table, ULONG hash_code, PLIST_ENTRY entry )
{
	InsertHeadList( &table->hash_table[ hash_code ], entry ); 

	table->count ++; 
}

INLINE VOID insert_to_hash_table( pcommon_hash_table table, ULONG hash_code, PLIST_ENTRY entry )
{
#ifdef DBG
	if( hash_code >= table->size )
	{
		ASSERT( FALSE && "how can input the invalid hash code for hash table index" ); 
		goto _return; 
	}
#endif //DBG

	hold_hash_table_lock( table ); 

	insert_to_hash_table_lock_free( table, hash_code, entry ); 

	release_hash_table_lock( table ); 

	goto _return; 

_return:
	return; 
}

NTSTATUS init_resource_locks( ULONG size, ERESOURCE **lock_alloc ); 
NTSTATUS uninit_resource_locks( ULONG size, ERESOURCE *all_lock ); 
NTSTATUS CALLBACK hold_w_resource_lock( PVOID lock ); 
NTSTATUS CALLBACK hold_r_resource_lock( PVOID lock ); 
NTSTATUS CALLBACK release_resource_lock( PVOID lock ); 

NTSTATUS init_sp_locks( ULONG size, spin_lock **lock_alloc ); 
NTSTATUS uninit_sp_locks( ULONG size, spin_lock *all_lock ); 
NTSTATUS CALLBACK hold_spin_lock( PVOID lock ); 
NTSTATUS CALLBACK release_spin_lock( PVOID lock ); 

INLINE NTSTATUS init_common_hash_table_def_lock( pcommon_hash_table table, 
								ULONG size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ERESOURCE *resource = NULL; 
	INT32 lock_inited = FALSE; 

	resource = ( ERESOURCE* )ALLOC_TAG_POOL( sizeof( ERESOURCE ) ); 
	if( resource == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	ntstatus = ExInitializeResourceLite( resource ); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

	lock_inited = TRUE; 

	ntstatus = init_common_hash_table( table, 
		size, 
		resource, 
		hold_w_resource_lock, 
		hold_r_resource_lock, 
		release_resource_lock ); 

_return:

	if( !NT_SUCCESS( ntstatus ) )
	{
		if( lock_inited == TRUE )
		{
			ExDeleteResourceLite( resource ); 
		}

		if( resource != NULL )
		{
			FREE_TAG_POOL( resource ); 
		}
	}

	return ntstatus; 
}

INLINE NTSTATUS release_common_hash_table_def_lock( pcommon_hash_table hash_table, release_hash_element_func release_func )
{
	NTSTATUS ntstatus; 
	release_common_hash_table( hash_table, release_func ); 
	ntstatus = ExDeleteResourceLite( ( ERESOURCE* )hash_table->lock ); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		log_trace( ( MSG_ERROR, "*** resource lock can't delete 0x%0.8x *** \n", ntstatus ) ); 
	}
	FREE_TAG_POOL( hash_table->lock ); 
	return ntstatus; 
}

INLINE NTSTATUS init_common_hash_table_spin_lock( pcommon_hash_table table, 
												ULONG size )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	spin_lock *sp_lock; 

	sp_lock = ( spin_lock* )ALLOC_TAG_POOL( sizeof( spin_lock ) ); 
	if( sp_lock == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	KeInitializeSpinLock( &sp_lock->lock ); 

	ntstatus = init_common_hash_table( table, 
		size, 
		sp_lock, 
		hold_spin_lock, 
		hold_spin_lock, 
		release_spin_lock ); 

_return:

	if( !NT_SUCCESS( ntstatus ) )
	{
		if( sp_lock != NULL )
		{
			FREE_TAG_POOL( sp_lock ); 
		}
	}
	return ntstatus; 
}

INLINE NTSTATUS release_common_hash_table_spin_lock( pcommon_hash_table hash_table, release_hash_element_func release_func )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	release_common_hash_table( hash_table, release_func ); 
	FREE_TAG_POOL( hash_table->lock ); 
	return ntstatus; 
}

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__HASH_TABLE_H__