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

//可以再加一个参数指定哪些参数进行比较
NTSTATUS update_action_cond_to_db( action_filter_info *filter_info, 
								  action_filter_info *old_filter_info ); 


NTSTATUS remove_action_cond_from_db( action_filter_info *filter_info ); 

NTSTATUS delete_action_cond_from_db( action_filter_info *filter_info ); 

LRESULT WINAPI adjust_filter_cond_value( action_filter_info *filter_info ); 


#endif //__FILTER_COND_DB_H__
