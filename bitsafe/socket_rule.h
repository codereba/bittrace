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

#ifndef __SOCKET_RULE_H__
#define __SOCKET_RULE_H__

typedef struct _rb_tree
{
	PVOID lock; 
	hold_lock_callback hold_lock; 
	hold_lock_callback hold_r_lock; 
	release_lock_callback release_lock; 
	rb_root root; 
} rb_tree, *prb_tree; 

typedef action_rule_item socket_rule_item; 
typedef access_rule_desc socket_rule_desc; 

typedef NTSTATUS ( CALLBACK* release_rb_node_callback )( rb_node *node ); 

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	extern rb_tree socket_rule_rbt; 

	NTSTATUS CALLBACK release_socket_rule( rb_node *node ); 
	NTSTATUS release_rbt( rb_tree *root, release_rb_node_callback release_func ); 

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	//NTSTATUS rb_check_socket_action( rb_tree *tree, LPCWSTR app, ipv4_socket_action *socket_action, data_trace_option *trace_option, action_response_type *resp ); 
#endif //COMPATIBLE_OLD_ACTION_DEFINE

	INT32 compare_socket_info( socket_rule_define *socket_define, socket_rule_desc *socket, INT32 *is_greater ); 
	NTSTATUS rb_search_socket_rule_lock_free( 
		rb_tree *tree, 
		socket_rule_desc *socket_rule, 
		socket_rule_item** rule_found ); 

	INLINE VOID hold_rbt_lock( rb_tree *tree )
	{
		ASSERT( tree != NULL ); 
		log_trace( ( MSG_INFO, "before hold tree lock 0x%0.8x, current irql is %d\n", 
			tree->lock, 
			KeGetCurrentIrql() ) ); 

		tree->hold_lock( tree->lock ); 
	}

	INLINE VOID release_rbt_lock( rb_tree *tree )
	{
		ASSERT( tree != NULL ); 
		tree->release_lock( tree->lock ); 

		log_trace( ( MSG_INFO, "after release tree lock 0x%0.8x, current irql is %d\n", 
			tree->lock, 
			KeGetCurrentIrql() ) ); 
	}

	INLINE NTSTATUS rb_search_socket_rule( 
		rb_tree *tree, 
		socket_rule_desc *socket_rule, 
		socket_rule_item** rule_found )
	{
		NTSTATUS ntstatus = STATUS_SUCCESS; 

		ASSERT( tree != NULL ); 

		hold_rbt_lock( tree ); 
		ntstatus = rb_search_socket_rule_lock_free( 
			tree, 
			socket_rule, 
			rule_found ); 

		release_rbt_lock( tree ); 

		return ntstatus; 
	}

	NTSTATUS rb_insert_socket_rule( socket_rule_desc *socket_rule, rb_node **node_out ); 
	NTSTATUS __rb_insert_socket_rule( rb_tree *tree, socket_rule_desc *socket_rule, 
	struct rb_node** node_found, socket_rule_item** rule_alloc ); 

	INLINE VOID init_rbtee( rb_tree *tree,  
		PVOID lock, 
		hold_lock_callback r_lock_func, 
		hold_lock_callback w_lock_func, 
		release_lock_callback unlock_func )
	{
		ASSERT( tree != NULL ); 
		ASSERT( w_lock_func != NULL ); 
		ASSERT( unlock_func != NULL ); 

		tree->lock = lock; 
		tree->hold_r_lock = r_lock_func; 
		tree->hold_lock = w_lock_func; 
		tree->release_lock = unlock_func; 

		tree->root.rb_node = NULL; 
	}

	INLINE NTSTATUS init_rbtree_spin_lock( rb_tree *tree )
	{
		NTSTATUS ntstatus = STATUS_SUCCESS; 
		spin_lock *sp_lock; 

		ASSERT( tree != NULL ); 

		sp_lock = ( spin_lock* )ALLOC_TAG_POOL( sizeof( spin_lock ) ); 
		if( sp_lock == NULL )
		{
			ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
			goto _return; 
		}

		KeInitializeSpinLock( &sp_lock->lock ); 

		init_rbtee( tree, 
			sp_lock, 
			hold_spin_lock, 
			hold_spin_lock, 
			release_spin_lock ); 

_return:
		return ntstatus; 
	}

	INLINE NTSTATUS release_rbtree_spin_lock( rb_tree *tree, release_rb_node_callback release_func )
	{
		NTSTATUS ntstatus = STATUS_SUCCESS; 
		ASSERT( tree != NULL ); 

		release_rbt( tree, release_func ); 
		FREE_TAG_POOL( tree->lock ); 
		return ntstatus; 
	}

	INLINE NTSTATUS release_socket_rule_rbt()
	{
		return release_rbtree_spin_lock( &socket_rule_rbt, release_socket_rule ); 
	}

	INLINE NTSTATUS init_socket_rule_rbt()
	{
		return init_rbtree_spin_lock( &socket_rule_rbt ); 
	}

	INLINE VOID rb_remove_socket_rule_lock_free( rb_node *node )
	{
		rb_erase( node, &socket_rule_rbt.root ); 
	}

	INLINE VOID rb_replace_socket_rule_lock_free( rb_node *old_node, rb_node *new_node )
	{
		rb_replace_node( old_node, new_node, &socket_rule_rbt.root ); 
	}

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__SOCKET_RULE_H__