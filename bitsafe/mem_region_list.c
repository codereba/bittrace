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
 
#ifdef TEST_IN_RING3
#include "common_func.h"
#include "ring0_2_ring3.h"
#include "mem_region_list.h"
#endif //TEST_IN_RING3

NTSTATUS init_region_list( safe_region_list *region_list, 
						  PVOID buf, 
						  ULONG buf_size, 
						  ULONG unit_size, 
						  PVOID lock, 
						  ULONG flags
						  //,release_list_entry release_func 
						  )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( buf != NULL ); 
	ASSERT(	buf_size > 0 ); 
	ASSERT( unit_size > 0 ); 
	ASSERT( lock != NULL ); 

	if( unit_size < sizeof( OFFSET_LIST_ENTRY ) )
	{
		ASSERT( FALSE && "unit size must greater than offset list" ); 
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	if( unit_size + sizeof( mem_region_list ) > buf_size ) 
	{
		ASSERT( FALSE ); 
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	region_list->region_list = ( mem_region_list* )( PBYTE )buf; 
	region_list->lock = lock; 
	//region_list->release_func = release_func; 

	if( flags & NO_INIT_MEM_REGION )
	{
		goto _return; 
	}

	region_list->region_list->base_off = 0; //FIELD_OFFSET( mem_region_list, all_units ); 
	region_list->region_list->count = ( buf_size - sizeof( mem_region_list ) ) / unit_size; 
	region_list->region_list->unit_size = unit_size; 
	region_list->region_list->size = buf_size; 
	region_list->region_list->used_count = 0; 

	InitializeOffsetListHead( &region_list->region_list->free_units, buf ); 
	InitializeOffsetListHead( &region_list->region_list->allocated_units, buf ); 

_return:
	return ntstatus; 
}

NTSTATUS uninit_region_list( safe_region_list *region_list )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

//_return:
	return ntstatus; 
}

LRESULT depart_rule_region( safe_region_list region_lists[], 
						   ULONG all_size[], 
						   ULONG region_num, 
						   PVOID buf, 
						   ULONG buf_size, 
						   ULONG unit_size, 
						   PVOID lock, 
						   ULONG flags
						   //,release_list_entry release_func 
						   )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG offset; 
	ULONG region_size; 
	INT32 i; 

	offset = 0; 

	ASSERT( buf != NULL ); 
	ASSERT(	buf_size > 0 ); 
	ASSERT( unit_size > 0 ); 
	ASSERT( lock != NULL ); 

	if( unit_size < sizeof( OFFSET_LIST_ENTRY ) )
	{
		ASSERT( FALSE && "unit size must greater than offset list" ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( ( unit_size + sizeof( mem_region_list ) ) * region_num > buf_size ) 
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	for( i = 0; i < region_num; i ++ )
	{
		if( all_size[ i ] == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		region_size = sizeof( mem_region_list ) + unit_size * all_size[ i ]; 
		if( offset + region_size > buf_size )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		ret = init_region_list( &region_lists[ i ], 
			( BYTE* )buf + offset, 
			region_size, 
			unit_size, 
			lock, 
			flags ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		offset += region_size; 
	}

_return:
	return ret; 
}

NTSTATUS alloc_region_unit( safe_region_list *region_list, OFFSET_LIST_ENTRY **unit_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	OFFSET_LIST_ENTRY *unit_alloc; 
	PBYTE unit_buf; 

	ASSERT( region_list != NULL ); 
	ASSERT( region_list->region_list != NULL ); 
	ASSERT( unit_out != NULL ); 

	*unit_out = NULL; 

	lock_region_list( region_list ); 

	if( FALSE == IsOffsetListEmpty( &region_list->region_list->free_units, 
		safe_region_list_base( region_list ) ) )
	{
		unit_alloc = RemoveOffsetHeadList( &region_list->region_list->free_units, 
			safe_region_list_base( region_list ) ); 

		ASSERT( ( PVOID )unit_alloc != region_list->region_list->free_units.Flink 
			+ safe_region_list_base( region_list ) ); 
	}
	else
	{
		if( region_list->region_list->used_count >= region_list->region_list->count )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			goto _return; 
		}

		unit_buf = region_list->region_list->all_units 
			+ ( region_list->region_list->used_count * region_list->region_list->unit_size ); 

		unit_alloc = ( OFFSET_LIST_ENTRY* )unit_buf; 

		region_list->region_list->used_count += 1; 
	}

	InsertOffsetTailList( &region_list->region_list->allocated_units, 
		unit_alloc, 
		safe_region_list_base( region_list ) ); 

	*unit_out = unit_alloc; 

_return:
	unlock_region_list( region_list ); 

	return ntstatus; 
}

NTSTATUS free_region_unit( safe_region_list *region_list, OFFSET_LIST_ENTRY *unit )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	ASSERT( region_list != NULL ); 
	ASSERT( region_list->region_list != NULL ); 
	ASSERT( unit != NULL ); 

	lock_region_list( region_list ); 
	
	ASSERT( ( PBYTE )unit > safe_region_list_base( region_list ) ); 
	ASSERT( unit->Flink < safe_region_list_size( region_list ) ); 
	ASSERT( unit->Blink < safe_region_list_size( region_list ) ); 

	ASSERT( region_list->region_list->used_count <= region_list->region_list->count ); 

	if( TRUE == IsOffsetListEmpty( unit, safe_region_list_base( region_list ) ) )
	{
		log_trace( ( MSG_WARNING, "free one aleary freed offset list entry 0x%0.8x( %d, %d )\n", 
			unit, 
			unit->Flink, 
			unit->Blink ) ); 
		ASSERT( FALSE ); 
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	ntstatus = find_allocated_unit_lock_free( region_list, unit ); 
	if( !NT_SUCCESS( ntstatus ) )
	{
		ASSERT( FALSE && "free the have not allocated unit" ); 
		goto _return; 
	}

	RemoveOffsetEntryList( unit, 
			safe_region_list_base( region_list ) ); 
	
	InsertOffsetTailList( &region_list->region_list->free_units, 
		unit, 
		safe_region_list_base( region_list ) ); 

_return:
	unlock_region_list( region_list ); 

	return ntstatus; 
}

NTSTATUS read_all_allocated_unit_lock_free( safe_region_list *region_list, read_list_entry read_func )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	OFFSET_LIST_ENTRY *list_head; 
	OFFSET_LIST_ENTRY *entry; 
	ULONG_PTR offset; 

	ASSERT( region_list != NULL ); 
	ASSERT( read_func != NULL ); 

	list_head = &region_list->region_list->allocated_units; 

	offset = list_head->Flink; 

	for( ; ; )
	{
		NTSTATUS _ntstatus; 

		entry = ( POFFSET_LIST_ENTRY )( offset 
			+ ( PBYTE )safe_region_list_base( region_list ) ); ; 

		if( entry == list_head )
		{
			break; 
		}

		_ntstatus = read_func( entry ); 
		if( !NT_SUCCESS( _ntstatus ) )
		{
			log_trace( ( MSG_ERROR, "read the offset list entry 0x%0.8x data failed\n", entry ) ); 
		}

		offset = entry->Flink; 
	}

//_return:
	return ntstatus; 
}

NTSTATUS read_all_allocated_unit( safe_region_list *region_list, read_list_entry read_func )
{
	NTSTATUS ntstatus; 
	
	lock_region_list( region_list ); 

	ntstatus = read_all_allocated_unit_lock_free( region_list, read_func ); 

	unlock_region_list( region_list ); 

	return ntstatus; 
}

NTSTATUS find_region_unit_lock_free( OFFSET_LIST_ENTRY *region_list, OFFSET_LIST_ENTRY *unit, PVOID base )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG_PTR offset; 
	OFFSET_LIST_ENTRY *entry; 

	ASSERT( ( PVOID )region_list != NULL ); 
	ASSERT( ( PVOID )unit >= base ); 
	ASSERT( ( PVOID )region_list >= base ); 

	offset = region_list->Flink; 

	for( ; ; )
	{
		entry = ( POFFSET_LIST_ENTRY )( offset + ( PBYTE )base ); 

		if( entry == region_list )
		{
			ntstatus = STATUS_NOT_FOUND; 
			//entry = NULL; 
			break; 
		}

		if( entry == unit )
		{
			break; 
		}

		offset = entry->Flink; 
	}

	return ntstatus; 
}

NTSTATUS find_free_unit_lock_free( safe_region_list *region_list, OFFSET_LIST_ENTRY *unit )
{
	ASSERT( region_list != NULL ); 
	ASSERT( region_list->region_list != NULL ); 
	ASSERT( unit != NULL ); 

	ASSERT( ( PBYTE )unit > safe_region_list_base( region_list ) ); 
	ASSERT( unit->Flink < safe_region_list_size( region_list ) ); 
	ASSERT( unit->Blink < safe_region_list_size( region_list ) ); 

	ASSERT( region_list->region_list->used_count <= region_list->region_list->count ); 

	return find_region_unit_lock_free( &region_list->region_list->free_units, 
		unit, 
		safe_region_list_base( region_list ) ); 
}

NTSTATUS find_allocated_unit_lock_free( safe_region_list *region_list, OFFSET_LIST_ENTRY *unit )
{
	ASSERT( region_list != NULL ); 
	ASSERT( region_list->region_list != NULL ); 
	ASSERT( unit != NULL ); 

	ASSERT( ( PBYTE )unit > safe_region_list_base( region_list ) ); 
	ASSERT( unit->Flink < safe_region_list_size( region_list ) ); 
	ASSERT( unit->Blink < safe_region_list_size( region_list ) ); 

	ASSERT( region_list->region_list->used_count <= region_list->region_list->count ); 

	return find_region_unit_lock_free( &region_list->region_list->allocated_units, 
		unit, 
		safe_region_list_base( region_list ) ); 
}

NTSTATUS test_offset_list()
{
	typedef struct _offset_list_test
	{
		ULONG param; 
		OFFSET_LIST_ENTRY entry; 
	} offset_list_test, *poffset_list_test; 

#define TEST_UNIT_NUM 800
#define TEST_MEM_UNIT_SIZE ( sizeof( offset_list_test ) )
#define MEM_REGION_SIZE ( TEST_MEM_UNIT_SIZE * TEST_UNIT_NUM + sizeof( OFFSET_LIST_ENTRY ) * 2 )
	
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	PVOID mem_region = NULL; 
	offset_list_test *all_units; 
	POFFSET_LIST_ENTRY allocated_units; 
	POFFSET_LIST_ENTRY free_units; 
	INT32 i; 
	ULONG_PTR offset; 
	ULONG_PTR offset_next; 
	POFFSET_LIST_ENTRY entry; 
	offset_list_test *test_unit; 

	mem_region = ALLOC_TAG_POOL( MEM_REGION_SIZE ); 

	if( mem_region == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	allocated_units = ( POFFSET_LIST_ENTRY )mem_region; 
	InitializeOffsetListHead( allocated_units, mem_region ); 
	
	free_units = ( POFFSET_LIST_ENTRY )( ( PBYTE )mem_region + sizeof( OFFSET_LIST_ENTRY ) ); 
	InitializeOffsetListHead( free_units, mem_region ); 
	
	all_units = ( offset_list_test* )( ( PBYTE )mem_region + sizeof( OFFSET_LIST_ENTRY ) * 2 ); 

	for( i = 0; i < TEST_UNIT_NUM; i ++ )
	{
		all_units[ i ].param = i; 
		InsertOffsetTailList( allocated_units, &all_units[ i ].entry, mem_region ); 
	}

	i = 0; 

	offset = allocated_units->Flink; 
	
	for( ; ; )
	{
		entry = ( POFFSET_LIST_ENTRY )( offset + ( PBYTE )mem_region ); 

		if( entry == allocated_units )
		{
			if( i < TEST_UNIT_NUM )
			{
				ASSERT( FALSE ); 
			}

			break; 
		}

		test_unit = ( offset_list_test* )CONTAINING_RECORD( entry, offset_list_test, entry ); 

		log_trace( ( MSG_INFO, "the parameter of the allocated unit is %d\n", test_unit->param ) ); 

		offset = entry->Flink; 
		i += 1; 
	}

	i = 0; 

	offset = allocated_units->Flink; 

	for( ; ; )
	{
		entry = ( POFFSET_LIST_ENTRY )( offset + ( PBYTE )mem_region ); 

		if( entry == allocated_units )
		{
			if( i < TEST_UNIT_NUM )
			{
				ASSERT( FALSE ); 
			}

			break; 
		}

		test_unit = ( offset_list_test* )CONTAINING_RECORD( entry, offset_list_test, entry ); 

		offset_next = entry->Flink; 

		if( ( i % 3 ) == 0 )
		{
			log_trace( ( MSG_INFO, "remove the unit %d its parameter is %d\n", offset, test_unit->param ) ); 

			RemoveOffsetEntryList( entry, mem_region ); 
			InsertOffsetTailList( free_units, entry, mem_region ); 
		}

		log_trace( ( MSG_INFO, "the parameter of the unit is %d\n", test_unit->param ) ); 

		offset = offset_next; 
		i += 1; 
	}

	i = 0; 

	offset = allocated_units->Flink; 

	for( ; ; )
	{
		entry = ( POFFSET_LIST_ENTRY )( offset + ( PBYTE )mem_region ); 

		if( entry == allocated_units )
		{
			log_trace( ( MSG_INFO, "all remain units number is %d\n", i ) ); 
			break; 
		}

		test_unit = ( offset_list_test* )CONTAINING_RECORD( entry, offset_list_test, entry ); 

		log_trace( ( MSG_INFO, "the parameter of the allocated unit is %d\n", test_unit->param ) ); 

		offset = entry->Flink; 
		i += 1; 
	}

	i = 0; 

	offset = free_units->Flink; 

	for( ; ; )
	{
		entry = ( POFFSET_LIST_ENTRY )( offset + ( PBYTE )mem_region ); 

		if( entry == free_units )
		{
			log_trace( ( MSG_INFO, "all remain units number is %d\n", i ) ); 
			break; 
		}

		test_unit = ( offset_list_test* )CONTAINING_RECORD( entry, offset_list_test, entry ); 

		log_trace( ( MSG_INFO, "the parameter of the free unit is %d\n", test_unit->param ) ); 

		offset = entry->Flink; 
		i += 1; 
	}

_return:
	if( mem_region != NULL )
	{
		free( mem_region ); 
	}

	return ntstatus; 
}