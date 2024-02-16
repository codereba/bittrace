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

#ifndef __EVENT_LOG_LOADER_H__
#define __EVENT_LOG_LOADER_H__

#define MAX_LOG_LOAD_COUNT 4096
struct event_log_notify
{
	ULONG log_id; 
	sys_action_record *action; 
	ULONG cached_item_index; 
	LPCWSTR stack_dump; 
	action_context *ctx; 
	PVOID data; 
	ULONG data_len; 
};

struct ui_cache_context
{
	ULONG ring_load_index; 
	ULONG loaded_count; 
	ULONG load_count; 
	CPaintManagerUI *manager; 
	ULONG max_count; 
	ULONG filtered_index; 
	CControlUI ** ui_cache; 
};

LRESULT WINAPI load_action_log( ULONG begin_index, ULONG count, PVOID context ); 
LRESULT WINAPI load_action_log_work( ULONG begin_index, ULONG count, PVOID context ); 

#endif //__EVENT_LOG_LOADER_H__