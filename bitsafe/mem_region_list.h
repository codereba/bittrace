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
 
 #ifndef __MEM_REGION_LIST_H__
#define __MEM_REGION_LIST_H__

#pragma pack( push )
#pragma pack( 4 )

typedef struct _OFFSET_LIST_ENTRY
{
	ULONG_PTR Flink; 
	ULONG_PTR Blink; 
} OFFSET_LIST_ENTRY, *POFFSET_LIST_ENTRY; 


/*++

LINK list:

    Definitions for a double link list.

--*/

#ifndef InitializeOffsetListHead

//#define InsertOffsetHeadList( ListHead, Entry, Base ) {\
//    POFFSET_LIST_ENTRY _EX_Flink;\
//    POFFSET_LIST_ENTRY _EX_ListHead;\
//    _EX_ListHead = ( ListHead );\
//    _EX_Flink = _EX_ListHead->Flink + ( Base );\
//    ( Entry )->Flink = _EX_Flink - ( Base );\
//    ( Entry )->Blink = _EX_ListHead - ( Base );\
//    _EX_Flink->Blink = ( Entry ) - ( Base );\
//    _EX_ListHead->Flink = ( Entry ) - ( Base );\
//    }

INLINE VOID InsertOffsetHeadList( POFFSET_LIST_ENTRY ListHead, POFFSET_LIST_ENTRY Entry, PVOID Base ) 
{
	POFFSET_LIST_ENTRY _EX_Flink; 
	POFFSET_LIST_ENTRY _EX_ListHead; 

	ASSERT( ( PVOID )ListHead >= Base ); 
	ASSERT( ( PVOID )Entry >= Base ); 

	_EX_ListHead = ( ListHead ); 
	_EX_Flink = ( POFFSET_LIST_ENTRY )( PVOID )( _EX_ListHead->Flink + ( ULONG_PTR )( PBYTE )( Base ) ); 

	( Entry )->Flink = ( ULONG_PTR )( ( PBYTE )_EX_Flink - ( ULONG_PTR )( PBYTE )( Base ) ); 
	( Entry )->Blink = ( ULONG_PTR )( ( PBYTE )_EX_ListHead - ( ULONG_PTR )( PBYTE )( Base ) ); 

	_EX_Flink->Blink = ( ULONG_PTR )( ( PBYTE )( Entry ) - ( ULONG_PTR )( PBYTE )( Base ) ); 
	_EX_ListHead->Flink = ( ULONG_PTR )( ( PBYTE )( Entry ) - ( ULONG_PTR )( PBYTE )( Base ) ); 
}

//#define RemoveOffsetEntryList( Entry, Base ) {\
//	POFFSET_LIST_ENTRY _EX_Blink;\
//	POFFSET_LIST_ENTRY _EX_Flink;\
//	_EX_Flink = ( POFFSET_LIST_ENTRY )( ( PBYTE )( Entry )->Flink + ( PBYTE )Base );\
//	_EX_Blink = ( POFFSET_LIST_ENTRY )( ( PBYTE )( Entry )->Blink + ( PBYTE )Base );;\
//	_EX_Blink->Flink = _EX_Flink - Base;\
//	_EX_Flink->Blink = _EX_Blink - Base;\
//}

INLINE VOID RemoveOffsetEntryList( POFFSET_LIST_ENTRY Entry, PVOID Base )
{
	POFFSET_LIST_ENTRY _EX_Blink; 
	POFFSET_LIST_ENTRY _EX_Flink; 

	ASSERT( ( PVOID )Entry >= Base ); 

	_EX_Flink = ( POFFSET_LIST_ENTRY )( ( Entry )->Flink + ( PBYTE )Base ); 
	_EX_Blink = ( POFFSET_LIST_ENTRY )( ( Entry )->Blink + ( PBYTE )Base ); 

	_EX_Blink->Flink = ( ULONG_PTR )( ( PBYTE )_EX_Flink - ( ULONG_PTR )( PBYTE )( Base ) ); 
	_EX_Flink->Blink = ( ULONG_PTR )( ( PBYTE )_EX_Blink - ( ULONG_PTR )( PBYTE )( Base ) ); 
}

//#define InitializeOffsetListHead( ListHead, Base ) (\
//    ( ListHead )->Flink = ( ListHead )->Blink = ( ( ListHead ) - ( Base ) ) )

INLINE VOID InitializeOffsetListHead( POFFSET_LIST_ENTRY ListHead, PVOID Base )
{
	ASSERT( ( PVOID )ListHead >= Base ); 

	( ListHead )->Flink = ( ListHead )->Blink = ( ULONG_PTR )( ( PBYTE )( ListHead ) - ( ULONG_PTR )( PBYTE )( Base ) ); 
}

//#define IsOffsetListEmpty( ListHead, Base ) \
//    ( ( ListHead )->Flink == ( ( ListHead ) - ( Base ) ) )

INLINE BOOLEAN IsOffsetListEmpty( POFFSET_LIST_ENTRY ListHead, PVOID Base ) 
{
	ASSERT( ( PVOID )ListHead >= Base ); 

	return ( ListHead )->Flink == ( ULONG_PTR )( ( PBYTE )( ListHead ) - ( ULONG_PTR )( PBYTE )( Base ) ); 
}

//#define RemoveOffsetHeadList( ListHead, Base ) \
//    ( ListHead )->Flink;\
//    { RemoveOffsetEntryList( ( ListHead )->Flink, Base ) }


INLINE POFFSET_LIST_ENTRY RemoveOffsetHeadList( POFFSET_LIST_ENTRY ListHead, PVOID Base ) 
{
	POFFSET_LIST_ENTRY entry; 

	ASSERT( ( PVOID )ListHead >= Base ); 

	entry = ( POFFSET_LIST_ENTRY )( ( PBYTE )( ListHead )->Flink + ( ULONG_PTR )( PBYTE )Base ); 

    RemoveOffsetEntryList( entry, Base ); 
	
	return entry; 
}

//#define RemoveOffsetTailList( ListHead, Base ) \
//    ( ListHead )->Blink;\
//    { RemoveEntryList( ( ListHead )->Blink, Base ) }

INLINE POFFSET_LIST_ENTRY RemoveOffsetTailList( POFFSET_LIST_ENTRY ListHead, PVOID Base ) 
{
	POFFSET_LIST_ENTRY entry; 
	
	ASSERT( ListHead != Base ); 

	entry = ( POFFSET_LIST_ENTRY )( ( ListHead )->Blink + ( PBYTE )( Base ) ); 
	RemoveOffsetEntryList( entry, Base ); 

	return entry; 
}

//#define InsertOffsetTailList( ListHead, Entry, Base ) {\
//    POFFSET_LIST_ENTRY _EX_Blink;\
//    POFFSET_LIST_ENTRY _EX_ListHead;\
//    _EX_ListHead = ( ListHead );\
//    _EX_Blink = ( _EX_ListHead->Blink + ( Base );\
//    ( Entry )->Flink = _EX_ListHead - ( Base );\
//    ( Entry )->Blink = _EX_Blink - ( Base );\
//    _EX_Blink->Flink = ( Entry ) - ( Base );\
//    _EX_ListHead->Blink = ( Entry ) - ( Base );\
//    }

INLINE VOID InsertOffsetTailList( POFFSET_LIST_ENTRY ListHead, POFFSET_LIST_ENTRY Entry, PVOID Base ) 
{ 
	POFFSET_LIST_ENTRY _EX_Blink; 
	POFFSET_LIST_ENTRY _EX_ListHead; 

	ASSERT( ( PVOID )ListHead >= Base ); 
	ASSERT( ( PVOID )Entry >= Base ); 

	_EX_ListHead = ( ListHead ); 
	_EX_Blink = ( POFFSET_LIST_ENTRY )( ( _EX_ListHead->Blink ) + ( ULONG_PTR )( PBYTE )( Base ) ); 
	( Entry )->Flink = ( ULONG_PTR )( ( PBYTE )_EX_ListHead - ( ULONG_PTR )( PBYTE )( Base ) ); 
	( Entry )->Blink = ( ULONG_PTR )( ( PBYTE )_EX_Blink - ( ULONG_PTR )( PBYTE )( Base ) ); 
	_EX_Blink->Flink = ( ULONG_PTR )( PBYTE )( Entry ) - ( ULONG_PTR )( PBYTE )( Base ); 
	_EX_ListHead->Blink = ( ULONG_PTR )( ( PBYTE )( Entry ) - ( ULONG_PTR )( PBYTE )( Base ) ); 
}

#endif //InitializeOffsetListHead

#pragma pack( push )

typedef struct _mem_region_list
{
	LONG_PTR base_off; 
	ULONG count; 
	ULONG unit_size; 
	ULONG size; 
	ULONG used_count; 

	OFFSET_LIST_ENTRY allocated_units; 
	OFFSET_LIST_ENTRY free_units; 

	BYTE all_units[ 1 ]; 
} mem_region_list, *pmem_region_list; 

typedef NTSTATUS ( *release_list_entry )( POFFSET_LIST_ENTRY entry ); 
typedef NTSTATUS ( *read_list_entry )( POFFSET_LIST_ENTRY entry ); 

typedef struct _safe_region_list
{
	HANDLE lock; 
	release_list_entry release_func; 
	mem_region_list *region_list; 
} safe_region_list, *psafe_region_list; 

#pragma pack( pop )

#define safe_region_list_base( _region_list ) ( ( PBYTE )( _region_list )->region_list + ( _region_list )->region_list->base_off )
#define lock_region_list( region_list ) lock_mutex( ( region_list )->lock ) 
#define unlock_region_list( region_list ) unlock_mutex( ( region_list )->lock ) 
#define safe_region_list_size( _region_list ) ( ULONG_PTR )( ( _region_list )->region_list->size )

#define NO_INIT_MEM_REGION 0x0000001

#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus


	NTSTATUS test_offset_list(); 
	NTSTATUS init_region_list( safe_region_list *region_list, 
		PVOID buf, 
		ULONG buf_size, 
		ULONG unit_size, 
		PVOID lock, 
		ULONG flags
		//,release_list_entry release_func 
		); 
	LRESULT depart_rule_region( safe_region_list region_lists[], 
		ULONG all_size[], 
		ULONG region_num, 
		PVOID buf, 
		ULONG buf_size, 
		ULONG unit_size, 
		PVOID lock, 
		ULONG flags 
		//,release_list_entry release_func 
		); 
	NTSTATUS uninit_region_list( safe_region_list *region_list ); 
	NTSTATUS alloc_region_unit( safe_region_list *region_list, OFFSET_LIST_ENTRY **unit_out ); 
	NTSTATUS free_region_unit( safe_region_list *region_list, OFFSET_LIST_ENTRY *unit ); 
	NTSTATUS read_all_allocated_unit_lock_free( safe_region_list *region_list, read_list_entry read_func ); 
	NTSTATUS read_all_allocated_unit( safe_region_list *region_list, read_list_entry read_func ); 
	NTSTATUS find_region_unit_lock_free( OFFSET_LIST_ENTRY *region_list, OFFSET_LIST_ENTRY *unit, PVOID base ); 
	NTSTATUS find_free_unit_lock_free( safe_region_list *region_list, OFFSET_LIST_ENTRY *unit ); 
	NTSTATUS find_allocated_unit_lock_free( safe_region_list *region_list, OFFSET_LIST_ENTRY *unit ); 

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__MEM_REGION_LIST_H__