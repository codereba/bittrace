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
 
#ifndef __FILTER_MANAGE_H__
#define __FILTER_MANAGE_H__

#ifdef new
#undef new
#include <vector>
#else
#include <vector>
#endif //new

#include "filter_info.h"

typedef std::vector<action_filter_info*> action_filter_info_array; 

typedef struct _action_filter_infos
{
	action_filter_info_array filter_infos; 
	//LONG next_info_id; 
} action_filter_infos, *paction_filter_infos; 

//extern CRITICAL_SECTION filter_infos; 
extern action_filter_infos filter_infos; 
//action_filter_info hilight_filter; 

LRESULT WINAPI r_lock_filter_infos(); 
LRESULT WINAPI r_unlock_filter_infos(); 
LRESULT WINAPI w_lock_filter_infos(); 
LRESULT WINAPI w_unlock_filter_infos(); 
LRESULT WINAPI w_try_lock_filter_infos(); 

//BOOL ClearBlobData(); 
//
//BOOL LoadBlobData(__in IN LPTSTR lpstTableName,__in LPTSTR lpstLastModified,__out LPVOID *ppvData,__out ULONG* pulDataLen,__in int role,__in int domain); 
//
//BOOL SaveBlobData(__in LPTSTR lpstTableName,__in LPTSTR lpstLastModified,__in LPVOID pvData,__in ULONG ulDataLen,__in int role,__in int domain); 

LRESULT WINAPI init_action_filter_sql( WCHAR *sql, 
									  ULONG cc_buf_len, 
									  ULONG *cc_ret_len ); 

//LRESULT get_filter_values( action_filter_info *filter_cond, 
//						  WCHAR *filter_values, 
//						  ULONG cc_buf_len, 
//						  ULONG *cc_ret_len ); 
//
//LRESULT get_compare_op( action_filter_info *filter_cond, 
//					   WCHAR *compare_op, 
//					   ULONG cc_buf_len, 
//					   ULONG *cc_ret_len ); 
//
//LRESULT get_colume_name( action_filter_info *filter_cond, 
//						WCHAR *compare_op, 
//						ULONG cc_buf_len, 
//						ULONG *cc_ret_len ); 

LRESULT insert_condition_sql( WCHAR *colume_name, 
							 WCHAR *compare_op, 
							 WCHAR *filter_values  ); 

//NTSTATUS CALLBACK load_action_filter_cond( ULONG action_cond_id, 
//										  action_filter_info *filter_cond, 
//										  LPCWSTR time, 
//										  PVOID work_context, 
//										  ULONG *state_out ); 

LRESULT WINAPI end_action_filter_sql( WCHAR *sql, ULONG cc_buf_len, ULONG *cc_ret_len ); 
LRESULT WINAPI do_filtered_events_cache(); 

//LRESULT WINAPI lock_hilight_filter(); 
//LRESULT WINAPI unlock_hilight_filter(); 
//LRESULT WINAPI try_lock_hilight_filter(); 

/*****************************************************************************
will make more database for caching events which is filtered.so need a function 
set to manage the data structure for caching.
caching database id:
1.timestamp
2.base database information.
3.filtered target database and table.
4.filtered sql

is when remove the temporary database for filtering? 
1.change the filtered condition.( accumulated filtering is best )
*****************************************************************************/

//extern ULONG filter_conds_count; 
//extern action_filter_info *filters; 

//LRESULT WINAPI set_hilight_filter( action_filter_info *filter ); 

LRESULT WINAPI filter_action( action_filter_info *filter_info, 
							 sys_action_record *action, 
							 action_context *ctx, 
							 PVOID data, 
							 ULONG data_size ); 

typedef struct _filter_ui_info
{
	ULONG bk_clr; 
	action_filter_cond cond; 
} filter_ui_info, *pfilter_ui_info; 

INLINE VOID WINAPI init_filter_ui_info( event_filter_ui_info *ui_info )
{
	ASSERT( ui_info != NULL ); 
	ui_info->bk_clr = INVALID_UI_FILTERED_BK_COLOR; 
}

LRESULT WINAPI filter_log_for_ui( sys_action_record *action, 
								 action_context *ctx,  
								 PVOID data, 
								 ULONG data_size, 
								 filter_ui_info *ui_info ); 

LRESULT WINAPI filter_action_log( sys_action_record *action, 
								 action_context *ctx, 
								 PVOID data, 
								 ULONG data_size ); 

LRESULT WINAPI init_filter_infos(); 
LRESULT WINAPI uninit_filter_infos(); 

//LRESULT WINAPI filter_action_log( r3_action_notify *action ); 
//LRESULT WINAPI do_event_filter( WCHAR *sql ); 

typedef struct _filter_sql_context
{
	WCHAR *sql; 
	ULONG cc_buf_len; 
	ULONG *cc_ret_len; 
} filter_sql_context, *pfilter_sql_context;

typedef NTSTATUS ( CALLBACK * action_filter_cond_callback )( ULONG action_cond_id, 
															action_filter_info *filter_cond, 
															LPCWSTR time, 
															PVOID work_context, 
															ULONG *state_out ); 

NTSTATUS output_action_cond_from_conf_file( LPCWSTR file_name, 
										   ULONG offset, 
										   ULONG limit, 
										   PVOID context, 
										   action_filter_cond_callback filter_cond_func ); 

#endif //__FILTER_MANAGE_H__