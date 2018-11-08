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

#include "StdAfx.h"
#ifdef _DEBUG_MEM_LEAKS
#undef _DEBUG_MEM_LEAKS
#include "common_func.h"
#endif //_DEBUG_MEM_LEAKS

#include "action_type.h"
#include "trace_log_api.h"
#include "trace_filter.h"
#include "action_type_parse.h"
#include "sqlite3.h"
#include "filter_manage.h"

typedef struct _filter_cond
{
	ULONG param_count; 
	action_context ctx; 
	param_info params[ 1 ]; 
} filter_cond, *pfilter_cond; 

typedef struct _param_info_ex
{
#define MAX_PARAM_NAME_LEN 100
	WCHAR param_name[ MAX_PARAM_NAME_LEN ]; 
	//param_info
	param_type type; 
	param_data data; 
} param_info_ex, *pparam_info_ex; 

typedef struct _filter_cond_ex
{
	ULONG max_param_count; 
	ULONG param_count; 
	param_info_ex params[ 1 ]; 
} filter_cond_ex, *pfilter_cond_ex; 

typedef struct _load_filter_context
{
	ULONG flags; 
	action_filter_infos *filters; 
} load_filter_context, *pload_filter_context; 

NTSTATUS CALLBACK add_action_filter_cond( ULONG action_cond_id, 
										 action_filter_info *filter_info, 
										 LPCWSTR time, 
										 PVOID work_context, 
										 ULONG *state_out ); 
LRESULT save_filter_cond()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	} while ( FALSE );

	return ret; 
}

LRESULT init_filter_conds()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		//safe_open_bitsafe_conf(); 
	} while ( FALSE );

	return ret; 
}

#define TRACE_FILTER_TABLE L"TRACE_FILTER"

INT32 open_trace_filter_db( LPCTSTR file_path, sqlite3 **handle )
{
	INT32 ret = 0; 
	return ret; 
}

LRESULT uninit_filter_conds()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	} while ( FALSE );

	return ret; 
}

#include "filter_cond_db.h"
NTSTATUS CALLBACK add_action_filter_cond( ULONG action_cond_id, 
										  action_filter_info *filter_info, 
										  LPCWSTR time, 
										  PVOID work_context, 
										  ULONG *state_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret = ERROR_SUCCESS;  
	load_filter_context *load_context; 
	action_filter_infos *filters; 
	ULONG flags; 

	do 
	{
		if( work_context == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		load_context = ( load_filter_context* )work_context; 

		flags = load_context->flags; 

		filters = load_context->filters; 

		if( filters != NULL )
		{
			if( filter_info->value.text.text_is_ptr == TRUE )
			{
				LPCWSTR flt_value; 
				HRESULT hr; 

				flt_value = filter_info->value.text.text_mode_value_ptr; 

				hr = StringCbCopyW( filter_info->value.text.text_mode_value, 
					sizeof( filter_info->value.text.text_mode_value ), 
					flt_value ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				filter_info->value.text.text_is_ptr = FALSE; 
			}

			{
				action_filter_info *_filter_info; 

				_filter_info = ( action_filter_info* )malloc( sizeof( action_filter_info ) ); 
				if( _filter_info == NULL )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}

				memcpy( _filter_info, filter_info, sizeof( *_filter_info ) ); 

				filters->filter_infos.push_back( _filter_info ); 
			}
		}

	}while( FALSE );

	return ntstatus; 
}

LRESULT load_filter_conds()
{
	LRESULT ret = ERROR_SUCCESS; 
	load_filter_context context; 
	
	do 
	{
		context.flags = DATA_BY_POINTER; 
		context.filters = &filter_infos; 

		ret = output_action_cond_from_db( 0, 0, &context, add_action_filter_cond ); 

	} while ( FALSE );

	return ret; 
}

LRESULT release_filter_conds()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	do 
	{
		for( i = 0; ( ULONG )i < filter_infos.filter_infos.size(); i ++ )
		{
			free( filter_infos.filter_infos[ i ] ); 
		}

		filter_infos.filter_infos.clear(); 

	} while ( FALSE );

	return ret; 
}

LRESULT set_filter_cond( filter_cond_ex *cond, 
							 LPCWSTR column_name, 
							 param_type type, 
							 param_data data )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( cond != NULL ); 
		ASSERT( column_name != NULL ); 
		ASSERT( TRUE == is_valid_param_type( type ) ); 

		if( cond->param_count + 1 > cond->max_param_count )
		{
			filter_cond_ex *cond_ext; 
			ULONG max_param_count; 
			
			if( cond->param_count < 100 )
			{
				max_param_count = cond->param_count * 2; 
			}
			else
			{
				max_param_count = cond->param_count + 30; 
			}

			cond_ext = ( filter_cond_ex* )malloc( sizeof( filter_cond_ex ) + max_param_count * sizeof( param_info ) ); 
			if( cond_ext == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			cond_ext->max_param_count = cond->param_count * 2; 
			cond_ext->param_count = cond->param_count; 
			memcpy( cond_ext->params, cond->params, sizeof( param_info ) * cond->param_count ); 
		}


	}while( FALSE );

	return ret; 
}


