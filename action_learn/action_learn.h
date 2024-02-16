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
 
#ifndef __ACTION_LEARN_H__
#define __ACTION_LEARN_H__

/********************************************************************
system action state machine.
********************************************************************/

extern group_policy_nfa global_nfa; 

#define INLINE __inline 

ULONG CALLBACK calc_proc_id_hash_code( PVOID param1, 
							 PVOID param2, 
							 ULONG size ); 

/******
notice: must is already removed from the list table
******/

NTSTATUS CALLBACK del_proc_in_table( PLIST_ENTRY element ); 

NTSTATUS CALLBACK compare_proc_id_in_table( PLIST_ENTRY element, PVOID param1, PVOID param2 ); 

NTSTATUS CALLBACK lock_mutex_func( PVOID lock ); 

NTSTATUS CALLBACK unlock_mutex_func( PVOID mutex ); 

LRESULT check_path_policy_ex( sys_action_link *actions ); 

LRESULT action_is_match_learing_target( sys_action_record *action ); 

LRESULT study_this_action( sys_action_record *action ); 

LRESULT action_study( sys_action_record *action ); 

NTSTATUS init_bitsafe_policy_nfa(); 

LRESULT check_policy_holder_ok( policy_holder *pol_holder ); 

INLINE LRESULT w_lock_policy_holder( policy_holder *pol_holder )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( pol_holder != NULL ); 
		ASSERT( pol_holder->lock != NULL ); 

		lock_mutex( pol_holder->lock ); 
	} while ( FALSE );

	return ret; 
}

#define r_lock_policy_holder w_lock_policy_holder

LRESULT release_policy_holder( policy_holder *pol_holder ); 

NTSTATUS find_sub_policy( group_policy_nfa *nfa, 
						 policy_holder *pol_locator, 
						 PVOID policy ); 

NTSTATUS _find_sub_policy( group_policy_nfa *nfa, 
						  policy_holder *pol_locator, 
						  policy_holder *policy ); 

NTSTATUS _compound_action_policy( policy_holder *src_policy, 
								 PVOID dest_policy ); 

NTSTATUS compound_action_policy( policy_holder *src_policy, 
								policy_holder *dest_policy ); 

LRESULT CALLBACK compare_number_policy( policy_holder *src_pol, 
									   policy_holder *dst_pol ); 

LRESULT CALLBACK check_number_policy( PVOID context, 
	policy_holder *pol_holder, 
	PVOID param ); 

NTSTATUS check_sys_action_record( action_source_ctx *ctx, 
								 sys_action_record *action, 
								 data_trace_option *trace_option, 
								 action_response_type *response ); 

NTSTATUS check_action_record( action_source_info *proc_sub, 
							 action_context *ctx, 
							 sys_action_record *action, 
							 data_trace_option *trace_option, 
							 action_response_type *response ); 

NTSTATUS check_sys_action_desc( action_source_ctx *ctx, 
							   sys_action_desc *action, 
							   data_trace_option *trace_option, 
							   action_response_type *response ); 

#endif //__ACTION_LEARN_H__