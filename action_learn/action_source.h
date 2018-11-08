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
 
 #ifndef __ACTION_SOURCE_H__
#define __ACTION_SOURCE_H__

#include "ref_node.h"

#define MAX_POLICY_COUNT 32

typedef struct _policy_state
{
	//ULONG id; 
	ULONG policy_count; 
	policy_holder *hitted_polices[ MAX_POLICY_COUNT ]; 
} policy_state, *ppolicy_state; 

typedef struct _action_source
{
	ULONG max_action_count; 
	sys_action_record *recorded_actions; 

	action_source_type type; 
	policy_state cur_state; 
} action_source, *paction_source; 

typedef struct _action_source_ctx
{
	policy_state active_state; 
} action_source_ctx, *paction_source_ctx; 

typedef struct action_source_data
{
	union
	{ 
		PVOID job; 
		PVOID proc; 
	}; 

	PVOID thrd; 	
} action_source_data, action_source_data; 

//typedef struct _action_policy_ex
//{
//	action_policy policy; 
//} action_policy_ex, *paction_policy_ex; 

//typedef enum _ACTION_STATE_TYPE
//{
//	ASSOCIATED_STATE, 
//	MAX_ACTION_STATE, 
//}ACTION_STATE_TYPE, *PACTION_STATE_TYPE; 
//
//#define is_valid_action_state( __state ) ( __state >= ASSOCIATED_STATE && __state < MAX_ACTION_STATE ) 
//
//typedef struct _ACTIVE_STATE_HEADER
//{
//	ACTION_STATE_TYPE type; 
//} ACTIVE_STATE_HEADER, *PACTIVE_STATE_HEADER; 

//typedef struct _ACTION_SOURCE
//{
//	ACTIVE_STATE_HEADER active_state; 
//	PVOID object; 
//} ACTION_SOURCE, *PACTION_SOURCE; 

//typedef struct _ASSOCIATION_ACTIVE_STATE
//{
//	ACTIVE_STATE_HEADER header; 
//	ULONG state_count; 
//	ULONG action_states[1]; 
//} ASSOCIATION_ACTIVE_STATE, *PASSOCIATION_ACTIVE_STATE; 

typedef struct _action_source_info
{
	ref_obj obj; 

	action_source_data source; 
	//volatile DWORD owner;
	action_source_ctx ctx; 
} action_source_info, *paction_source_info; 

NTSTATUS add_action_source( action_source_data *source ); 
NTSTATUS del_action_source( action_source_data *source ); 

NTSTATUS find_action_source( action_source_data *source, 
							action_source_info **info_out ); 

NTSTATUS init_action_sources(); 
NTSTATUS uninit_action_sources(); 

#endif //__ACTION_SOURCE_H__