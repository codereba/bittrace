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