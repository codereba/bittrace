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