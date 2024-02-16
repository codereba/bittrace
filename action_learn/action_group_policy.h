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
 
 #ifndef __ACTION_GROUP_POLICY_H__
#define __ACTION_GROUP_POLICY_H__

typedef struct _group_policy_nfa
{
	ref_node_tbl policy_groups; 
	ref_node_tbl policies; 
	check_policy_callback check_policy_func; 
	find_policy_callback find_policy_func; 
} group_policy_nfa, *pgroup_policy_nfa; 

#define MAX_POLICY_GROUP_DESC 1024
typedef struct _policy_group_desc
{
	action_reply reply; 
	ULONG id; 
	ULONG level; 
	ULONG desc_len; 
	WCHAR desc[ 1 ]; 
} policy_group_desc, *ppolicy_group_desc; 

typedef struct _policy_group
{
	ref_obj obj; 
	LIST_ENTRY sub_polices; 

	HANDLE lock; 
	policy_group_desc desc; 
} policy_group, *ppolicy_group; 

typedef struct _group_policy_holder
{
	LIST_ENTRY entry; 
	struct _policy_holder *policy; 
} group_policy_holder, *pgroup_policy_holder; 

NTSTATUS init_group_policy_dispatchs(); 
NTSTATUS uninit_group_policy_dispatchs(); 

#define DEFAULT_POLICY_GROUP_LENGTH ( sizeof( policy_group_desc ) + ( MAX_POLICY_GROUP_DESC << 1 ) )

NTSTATUS nfa_action_group_analyze( group_policy_nfa *nfa, 
								  action_source_info *source, 
								  r3_action_notify *action, 
								  policy_group_desc **desc ); 

NTSTATUS group_nfa_check_policy( group_policy_nfa *nfa, 
								action_source_info *source, 
								PVOID action ); 

NTSTATUS init_group_policy_nfa( group_policy_nfa *nfa ); 

NTSTATUS uninit_group_policy_nfa( group_policy_nfa *nfa ); 

NTSTATUS dump_nfa_policys( group_policy_nfa *nfa ); 

typedef NTSTATUS ( CALLBACK *group_hitting_callback )( group_policy_nfa *nfa, 
													  action_source_info *source, 
													  policy_group *group ); 

NTSTATUS check_group_hitted( group_policy_nfa *nfa, 
							action_source_info *source, 
							group_hitting_callback callback_func ); 

INLINE NTSTATUS add_policy_group( group_policy_nfa *nfa, 
								 policy_group_desc *desc, 
								 policy_group **group )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	//LRESULT ret; 
	ref_obj *obj = NULL; 

	do 
	{
		ASSERT( nfa != NULL ); 
		ASSERT( group != NULL ); 

		*group = NULL; 

		ntstatus = add_ref_obj( &nfa->policy_groups, 
			desc, 
			&obj ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			ASSERT( FALSE ); 
			log_trace( ( MSG_FATAL_ERROR, "add policy group error 0x%0.8x\n", ntstatus ) ); 
			break; 
		}

		*group = CONTAINING_RECORD( obj, policy_group, obj ); 
	}while( FALSE );

	return ntstatus; 
}

INLINE NTSTATUS add_policy_to_group( group_policy_nfa *nfa, 
									group_action_policy *policy, 
									policy_group *group )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret; 
	policy_holder *holder = NULL; 
	group_policy_holder *group_holder = NULL; 
	ref_obj *obj = NULL; 

	do 
	{
		ASSERT( nfa != NULL ); 
		ASSERT( group != NULL ); 

		do 
		{
			ntstatus = reference_obj( &nfa->policies, 
				policy, 
				&obj, 
				NULL ); 

			if( ntstatus != STATUS_SUCCESS )
			{
				if( ntstatus == STATUS_NOT_FOUND )
				{
					ntstatus = add_ref_obj( &nfa->policies, 
						policy, 
						&obj ); 
				}

				if( ntstatus != STATUS_SUCCESS )
				{
					break; 
				}
			}

			holder = CONTAINING_RECORD( obj, policy_holder, obj ); 

		}while( FALSE );

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		ASSERT( holder != NULL ); 

		do 
		{
			group_holder = ( group_policy_holder* )ALLOC_TAG_POOL( sizeof( group_policy_holder ) ); 
			if( group_holder == NULL )
			{
				ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
				break; 
			}

			InitializeListHead( &group_holder->entry ); 

			group_holder->policy = holder; 

			ret = lock_mutex( group->lock ); 
			if( ret != ERROR_SUCCESS )
			{
				ntstatus = STATUS_UNSUCCESSFUL; 
				break; 
			}

			InsertTailList( &group->sub_polices, 
				&group_holder->entry ); 

			ret = unlock_mutex( group->lock ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_FATAL_ERROR, "unlock mutex error %u\n", ret ) ); 
			}
		}while( FALSE ); 

	}while( FALSE ); 

	if( ntstatus != STATUS_SUCCESS )
	{
		if( obj != NULL )
		{
			NTSTATUS _ntstatus; 
			_ntstatus = del_ref_obj( &nfa->policies, policy, NULL ); 
			ASSERT( _ntstatus == STATUS_SUCCESS ); 
		}
	}

	return ntstatus; 
}


#endif //__ACTION_GROUP_POLICY_H__