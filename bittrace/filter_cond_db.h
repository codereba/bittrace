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

#ifndef __FILTER_COND_DB_H__
#define __FILTER_COND_DB_H__

#include "filter_info.h"

NTSTATUS open_action_filter_cond_db( LPCWSTR file_path, PVOID *handle ); 

typedef NTSTATUS ( CALLBACK * action_filter_cond_callback )( ULONG action_cond_id, 
															action_filter_info *filter_cond, 
															LPCWSTR time, 
															PVOID work_context, 
															ULONG *state_out ); 

NTSTATUS output_action_cond_from_db( ULONG offset, 
								   ULONG limit, 
								   PVOID context, 
								   action_filter_cond_callback filter_cond_func ); 

LRESULT get_action_filter_cond_count( ULONG *cond_count ); 

NTSTATUS input_action_cond_to_db( action_filter_info *filter_info ); 

//�����ټ�һ������ָ����Щ�������бȽ�
NTSTATUS update_action_cond_to_db( action_filter_info *filter_info, 
								  action_filter_info *old_filter_info ); 


NTSTATUS remove_action_cond_from_db( action_filter_info *filter_info ); 

NTSTATUS delete_action_cond_from_db( action_filter_info *filter_info ); 

LRESULT WINAPI adjust_filter_cond_value( action_filter_info *filter_info ); 


#endif //__FILTER_COND_DB_H__
