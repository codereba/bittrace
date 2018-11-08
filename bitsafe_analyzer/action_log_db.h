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

#ifndef __ACTION_LOG_DB_H__
#define __ACTION_LOG_DB_H__

#include "action_type_parse.h"
#include "filter_manage.h"
#include "event_store.h"

#define LIMIT_INFINITE ( ( ULONG )0xffffffff )

#define RESULT_IGNORE 0xfffffff0

typedef enum _param_sel_mode
{
	SELECT_OR = 0x00000000, 
	SELECT_AND, 
	SELECT_NOT, 
} param_sel_mode, *pparam_sel_mode; 

#define PARAM_SEL_MODE_COUNT 3

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif //PAGE_SIZE

#define ACTION_FILTER_COND_DB_NAME L"data17.dat"

#define ACTION_LOG_DB_NAME L"data15.dat"
#define ACTION_LOG_BACKUP_DB_NAME _T( "data16.dat" )

#define ACTION_LOG_BLOB_COLUME_NAME "data"

#define ACTION_COUNT_IS_REACH_MAX 0x00000001
#define LOG_IS_ALL_LOADED 0x00000002

#define INVALID_ROW_INDEX ( INT64 )( UINT64 )( -1 )
#define MAX_ACTION_DATA_SIZE ( PAGE_SIZE * 4 )


#define INVALID_ITEM_REAL_INDEX ( LONG )( -1 )
typedef struct _calc_filtered_index_work
{
	LONG search_filtered_index; 
	LONG search_real_index; 
	LONG target_filtered_index; 
	LONG target_real_index; 
	ULONG flags; 
} calc_filtered_index_work, *pcalc_filtered_index_work; 

extern WCHAR log_db_file_name[ MAX_PATH ]; 
extern ULONG log_db_file_name_len; 
extern ULONG log_db_dir_name_len; 
extern HANDLE bittrace_proc_id; 

LRESULT WINAPI prepare_action_to_load( r3_action_notify *action ); 

NTSTATUS WINAPI restore_action_log_db_backup( LPCWSTR file_path ); 

NTSTATUS WINAPI open_action_log_db( LPCWSTR file_path, PVOID *handle ); 

NTSTATUS WINAPI close_action_log_db( PVOID handle ); 

extern LPCWSTR param_sel_mode_string[ PARAM_SEL_MODE_COUNT ]; 

INLINE BOOLEAN WINAPI invalid_sel_mode( param_sel_mode mode )
{
	return ( mode == SELECT_OR 
		|| mode == SELECT_AND 
		|| mode == SELECT_NOT ); 
}

NTSTATUS CALLBACK _filter_event_log( ULONG log_id, 
									sys_action_record *action, 
									action_context *ctx, 
									PVOID data, 
									ULONG data_len, 
									LPCWSTR stack_dump,
									//ULONG frame_count, 
									PVOID work_context, 
									ULONG *state_out ); 

typedef struct _select_action_cond
{
	sys_action_type type; 
	ULONG cond_count; 
	param_sel_mode conds[ MAX_PARAMS_COUNT ]; 
} select_action_cond, *pselect_action_cond; 


typedef NTSTATUS ( CALLBACK * action_log_load_callback )( ULONG log_id, 
														 sys_action_record *action, 
														 action_context *ctx, 
														 PVOID data, 
														 ULONG data_len, 
														 LPCWSTR stack_dump, 
														 r3_action_notify_buf *action_notify, 
														 PVOID work_context, 
														 ULONG *state_out ); 

typedef NTSTATUS ( CALLBACK * _action_log_load_callback )( r3_action_notify *action, 
										   LPCWSTR stack_dump,
										   PVOID work_context, 
										   ULONG *state_out ); 


NTSTATUS WINAPI create_param_fmt_string( sys_action_record *action, 
								 LPWSTR fmt_string, 
								 ULONG ccb_buf_len, 
								 ULONG *ccb_ret_len ); 

LPCWSTR WINAPI get_table_name_from_action_type( sys_action_type type ); 


NTSTATUS WINAPI get_sql_param_string( param_info *param, 
						  LPWSTR param_text, 
						  ULONG ccb_buf_len, 
						  ULONG *ccb_ret_len ); 

LPCWSTR WINAPI get_param_sel_cond_string( param_sel_mode sel_mode ); 

NTSTATUS WINAPI construct_select_sql( LPWSTR sql_out, 
							  ULONG ccb_buf_len, 
							  action_context *ctx, 
							  sys_action_type type, 
							  param_info all_params[], 
							  param_sel_mode all_conds[], 
							  ULONG count, 
							  ULONG offset, 
							  ULONG limit ); 

LRESULT WINAPI get_action_log_count( ULONG *log_count ); 

LRESULT WINAPI get_filtered_action_log_count( ULONG *log_count ); 

#define LOG_FILTER_DECENDING 0x00000001

LRESULT WINAPI _filtered_index_2_real_index( LONG filtered_index, 
											LONG search_filtered_index, 
											LONG search_real_index, 
											LONG *real_index, 
											ULONG flags ); 

NTSTATUS WINAPI construct_delete_sql( LPWSTR sql_out, 
							  ULONG ccb_buf_len, 
							  action_context *ctx, 
							  sys_action_type type, 
							  param_info all_params[], 
							  param_sel_mode all_conds[], 
							  ULONG count ); 


NTSTATUS WINAPI update_blob_size( ); 

VOID WINAPI free_blob_buf( PVOID buf ); 

NTSTATUS WINAPI write_action_log_data( INT64 row_index, 
							   PBYTE buf, 
							   ULONG buf_len, 
							   ULONG *ret_len ); 

NTSTATUS WINAPI read_action_log_data( INT64 row_index, 
							  PBYTE buf, 
							  ULONG buf_len, 
							  ULONG *ret_len ); 

typedef PVOID ( CALLBACK* alloc_log_buffer )( PVOID param ); 

typedef struct _param_colume_info
{
	WCHAR *colume_name; 
} param_colume_info, *pparam_colume_info; 

NTSTATUS WINAPI construct_insert_sql( LPWSTR sql_out, 
							  ULONG ccb_buf_len, 
							  action_context *ctx, 
							  sys_action_type type, 
							  param_info all_params[], 
							  param_colume_info all_columes[], 
							  param_sel_mode all_conds[], 
							  ULONG count ); 

NTSTATUS WINAPI input_action_log_to_db( action_context *ctx, 
									   sys_action_record *action,
									   ULONGLONG call_frame[], 
										ULONG frame_count, 
										PVOID data, 
										ULONG data_len ); 

NTSTATUS WINAPI input_action_log_to_db_ex( PVOID db_handle, 
										  action_context *ctx, 
										  sys_action_record *action, 
										  ULONGLONG call_frame[], 
										  ULONG frame_count, 
										  PVOID data, 
										  ULONG data_len ); 


NTSTATUS WINAPI input_action_log_to_db_cached( action_context *ctx, 
								sys_action_record *action, 
								PVOID data, 
								ULONG data_len ); 

NTSTATUS WINAPI output_action_log_from_db( action_context *ctx, 
								   sys_action_type type, 
								   param_sel_mode all_cond[], 
								   param_info all_param[], 
								   ULONG param_count, 
								   ULONG offset, 
								   ULONG limit, 
								   PVOID context, 
								   action_log_load_callback filter_log_func ); 

NTSTATUS WINAPI output_action_log_from_db_ex( PVOID db_handle, 
											 action_context *ctx, 
											 sys_action_type type, 
											 param_sel_mode all_cond[], 
											 param_info all_param[], 
											 ULONG param_count, 
											 ULONG offset, 
											 ULONG limit, 
											 PVOID context, 
											 PVOID alloc_func, 
											 PVOID filter_log_func ); 

NTSTATUS WINAPI commit_common_log_transaction(); 

NTSTATUS WINAPI begin_common_log_transaction(); 

#define FACILITY_BITTRACE 0x8010000
#define ERROR_CODE_BASE_WORK_EVENT 0x0000801
#define ERROR_BASE_LOG_WORK ( 0xE0000000 | FACILITY_BITTRACE | ERROR_CODE_BASE_WORK_EVENT )

#define DEFAULT_TRACE_NOTIFY_TIME 100

#define SQLITE_DB_JOURNAL_FILE_POSTFIX L"-journal" 

INLINE LRESULT WINAPI filter_trace_base_work( r3_action_notify *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( action->action.ctx.proc_id != ( ULONG )( ULONG_PTR )bittrace_proc_id 
			&& action->action.ctx.proc_id != ( ULONG )4 )
		{
			break; 
		} 

		if( action->action.action.do_file_write.path_len != log_db_dir_name_len + log_db_file_name_len 
			&& action->action.action.do_file_write.path_len != log_db_dir_name_len + log_db_file_name_len + CONST_STR_LEN( SQLITE_DB_JOURNAL_FILE_POSTFIX ) )
		{
			break; 
		}

		switch( action->action.action.type )
		{
		case FILE_write:
		case FILE_read:
			if( 0 == wcsnicmp( log_db_file_name, 
				action->action.action.do_file_write.path_name + log_db_dir_name_len, 
				log_db_file_name_len ) )
			{
				ret = ERROR_BASE_LOG_WORK; 
				break; 
			}
			break; 
		default:
			break; 
		}
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI add_log_to_db_ex( r3_action_notify *action )
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 
	PVOID action_data; 

	do 
	{
		ASSERT( action != NULL ); 

		if( action->action.action.type == FILE_write )
		{
			DBG_BP(); 
		}

#ifdef _DEBUG
		ret = filter_trace_base_work( action ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}
#endif //_DEBUG

		action_data = get_action_output_data( action ); 
		
		/***************************************************************************
		stack trace is meaningful for professional users.
		who is the target user which i focusing?
		is not for basic user.
		for normal and professional users.

		***************************************************************************/

		ntstatus = input_action_log_to_db( &action->action.ctx, 
			&action->action.action, 
			action->stack_frame, 
			action->frame_count, 
			action_data, 
			action->data_size ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	return ret; 
}

#endif //__ACTION_LOG_DB_H__