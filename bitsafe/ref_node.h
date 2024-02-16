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

#ifndef __REF_NODE_H__
#define __REF_NODE_H__

#ifdef TEST_IN_RING3
typedef CRITICAL_SECTION REF_OBJECT_GROUP_LOCK; 
#else
typedef ERESOURCE REF_OBJECT_GROUP_LOCK; 
#endif //TEST_IN_RING3

#define REF_NODE_GROUP_LOCK 

typedef struct _thread_ref_count
{
	LIST_ENTRY entry; 
	ULONG thread_id; 
	HANDLE thread_handle; 
	ULONG ref_count; 
} thread_ref_count, *pthread_ref_count; 

typedef struct _ref_obj
{
	LIST_ENTRY entry; 
#ifdef DEBUG_REF_COUNT
	LIST_ENTRY all_thread_ref; 
#endif //DEBUG_REF_COUNT

	LONG ref_count; 
	//volatile DWORD owner; 

#ifdef TEST_IN_RING3
	REF_OBJECT_GROUP_LOCK *lock; 
#else
	union
	{
		struct 
		{
			KSPIN_LOCK lock; 
			KIRQL old_irql; 
		};

		ERESOURCE *res_lock; 
	};
#endif //TEST_IN_RING3
#ifdef DEBUG_REF_COUNT
#endif //DEBUG_REF_COUNT

} ref_obj, *pref_obj; 


//ֱ��ʹ�ýṹ�е�IRQL�����������Ƕ�ջ��û��ʵ�����ͻ��
INLINE NTSTATUS init_ref_obj_lock( ref_obj *obj )
{
	init_sp_lock( obj->lock ); 

	return STATUS_SUCCESS; 
}

INLINE NTSTATUS r_lock_ref_obj( ref_obj *obj )
{
	hold_sp_lock( obj->lock, obj->old_irql ); 
	return STATUS_SUCCESS; 
}

INLINE NTSTATUS w_lock_ref_obj( ref_obj *obj )
{
	hold_sp_lock( obj->lock, obj->old_irql ); 

	return STATUS_SUCCESS; 
}

INLINE NTSTATUS unlock_ref_obj( ref_obj *obj )
{
	release_sp_lock( obj->lock, obj->old_irql ); 

	return STATUS_SUCCESS; 
}

#define  w_unlock_ref_obj( obj ) unlock_ref_obj( obj )
#define  r_unlock_ref_obj( obj ) unlock_ref_obj( obj )

//typedef NTSTATUS ( CALLBACK *add_ref_node )( ref_obj_ex *obj ); 
typedef ULONG ( *calc_ref_obj_hash )( PVOID param, ULONG tbl_size ); 

typedef NTSTATUS ( *get_param_from_ref_obj )( ref_obj *obj, PVOID *param ); 
typedef NTSTATUS ( CALLBACK *release_ref_node )( ref_obj *obj ); 
typedef NTSTATUS ( CALLBACK *hold_ref_tbl_lock )( PVOID lock ); 
typedef NTSTATUS ( CALLBACK *release_ref_tbl_lock )( PVOID lock ); 
typedef NTSTATUS ( CALLBACK *hold_w_ref_tbl_lock )( PVOID lock ); 
typedef BOOLEAN ( *compare_ref_node )( ref_obj *obj, PVOID param ); 
typedef NTSTATUS ( *alloc_ref_node )( PVOID param, ref_obj **obj_alloc ); 

typedef struct _ref_node_tbl
{
	LIST_ENTRY *all_ref_node; 

#ifdef REF_NODE_GROUP_LOCK
	REF_OBJECT_GROUP_LOCK *all_ref_lock; 
#endif //REF_NODE_GROUP_LOCK

	ULONG list_num; 
	LONG ref_obj_num; 
	PVOID lock; 
	BOOLEAN lock_alloc; 
	//BOOLEAN inserting_obj; 
	
	compare_ref_node compare_func; 
	alloc_ref_node alloc_func; 
	release_ref_node release_func; 
	hold_ref_tbl_lock r_lock_func; 
	hold_w_ref_tbl_lock w_lock_func; 
	release_ref_tbl_lock unlock_func; 
	calc_ref_obj_hash hash_func; 
#ifdef DBG
	get_param_from_ref_obj get_param_func; 
#endif //DBG
} ref_node_tbl, *pref_node_tbl; 

INLINE BOOLEAN ref_obj_tbl_inited( ref_node_tbl *tbl )
{
	BOOLEAN ret = FALSE; 
	ASSERT( tbl != NULL ); 

	if( tbl->list_num > 0 )
	{
		ret = TRUE; 
	}

	return ret; 
}

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

NTSTATUS init_ref_obj_tbl( ref_node_tbl *tbl, 
					   ULONG size, 
					   compare_ref_node compare_func, 
					   alloc_ref_node alloc_func, 
					   release_ref_node release_func, 
					   calc_ref_obj_hash calc_hash_func, 
#ifdef DBG
					   get_param_from_ref_obj get_param_func, 
#endif //DBG
					   PVOID lock, 
					   hold_ref_tbl_lock r_lock_func, 
					   hold_w_ref_tbl_lock w_lock_func, 
					   release_ref_tbl_lock unlock_func ); 

NTSTATUS uninit_ref_obj_tbl( ref_node_tbl *tbl ); 
NTSTATUS get_ref_obj( ref_node_tbl *tbl, PVOID param, ref_obj** obj_out ); 
NTSTATUS def_release_obj( ref_obj *obj ); 
ULONG _del_ref_obj( ref_node_tbl *tbl, ref_obj *obj ); 
NTSTATUS del_ref_obj( ref_node_tbl *tbl, PVOID param, ULONG *ref_count ); 
ULONG _del_ref_obj_lock_free( ref_node_tbl *tbl, ref_obj *obj ); 
#ifdef DBG
NTSTATUS find_ref_obj( ref_node_tbl *tbl, ref_obj *obj ); 
#endif //DBG

NTSTATUS add_ref_obj( ref_node_tbl *tbl, PVOID param, ref_obj** obj_out ); 
NTSTATUS find_ref_obj_locked( ref_node_tbl *tbl, PVOID param, ref_obj **obj ); 
INLINE NTSTATUS unlock_ref_node_tbl( ref_node_tbl* tbl ) 
{ 
	ASSERT( tbl->unlock_func != NULL ); 
	return tbl->unlock_func( tbl->lock ); 
}

NTSTATUS reference_obj( ref_node_tbl *tbl, PVOID param, ref_obj** obj, LONG *ref_count ); 
NTSTATUS _reference_obj( ref_node_tbl *tbl, ref_obj *obj ); 
NTSTATUS deref_obj( ref_node_tbl *tbl, ref_obj *obj ); 
#ifdef DBG

ULONG CALLBACK del_obj_thread( PVOID param ); 
ULONG CALLBACK ref_obj_thread( PVOID param ); 
typedef struct _test_ref_obj
{
	ref_obj obj; 
	ULONG test_val; 
} test_ref_obj, *ptest_ref_obj; 

NTSTATUS get_test_ref_obj_param( ref_obj* obj, PVOID *param ); 
BOOLEAN compare_test_ref_obj( ref_obj *obj, PVOID param ); 
NTSTATUS CALLBACK release_test_ref_obj( ref_obj *obj ); 
NTSTATUS alloc_test_ref_obj( PVOID param, ref_obj **obj_alloc ); 
NTSTATUS ref_obj_test(); 
#endif //DBG

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__REF_NODE_H__