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

#ifdef _DEBUG_MEM_LEAKS
#undef _DEBUG_MEM_LEAKS
#endif //_DEBUG_MEM_LEAKS
#include "common_func.h"
#include "module_info.h"

#include <strsafe.h>
#include "ui_ctrl.h"
#include "acl_define.h"
#include "action_result.h"

using namespace std; 

#define DEBUG_MODULE_INFO 1
#define MAX_PROCESS_COUNT 260
#define LOAD_CURRENT_PROCESS_MODULE_INFO 1

typedef vector< PROCESS_MODULE_INFO_TABLE* > PROCESS_MODULE_INFO_TABLES; 
typedef PROCESS_MODULE_INFO_TABLES::iterator PROCESS_MODULE_INFO_TABLE_ITERATOR; 

PROCESS_MODULE_INFO_TABLES proc_infos; 

CRITICAL_SECTION proc_modules_lock; 

MODULE_INFO_TABLE kernel_modules = { 0 }; 

#define MAX_RING3_MODULE_COUNT 200
#define MAX_RING0_MODULE_COUNT MAX_RING3_MODULE_COUNT 

thread_manage module_info_work[ 1 ] = { 0 }; 

LRESULT WINAPI init_symbol_mapping(); 

LRESULT WINAPI get_file_time_stamp( LPCWSTR file_name, 
								   ULONG *time_stamp ); 

LRESULT WINAPI get_process_addition_info( ULONG proc_id, 
										 LPCWSTR file_path, 
										 PROCESS_MODULE_INFO_TABLE *info ); 

LRESULT WINAPI process_proc_module_info( r3_action_notify *action ); 
ULONG CALLBACK thread_module_info_maintain( LPVOID param ); 
ULONG CALLBACK thread_trace_event( LPVOID param ); 

MODULE_INFO_TABLE *get_kernel_modules()
{
	return &kernel_modules; 
}

FORCEINLINE BOOLEAN WINAPI is_kernel_process( ULONG proc_id )
{
	BOOLEAN ret = TRUE; 

	do 
	{
		if( proc_id == 0 
			|| proc_id == 4 )
		{
			break; 	
		}

		ret = FALSE; 
	}while( FALSE );

	return ret; 
}

FORCEINLINE LRESULT region_overlapped( ULONG_PTR src_begin, 
									  ULONG_PTR src_size, 
									  ULONG_PTR dest_begin, 
									  ULONG_PTR dest_size )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( src_size > 0 ); 
		ASSERT( dest_size > 0 ); 

		if( src_size == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( dest_size == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( src_begin >= dest_begin )
		{
			if( src_begin < dest_begin + dest_size )
			{
				break; 
			}
		}
		else
		{
			if( src_begin + src_size >= dest_begin )
			{
				break; 
			}
		}

		ret = ERROR_ERRORS_ENCOUNTERED; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI init_module_info_table( MODULE_INFO_TABLE *module_table )
{
	LRESULT ret = ERROR_SUCCESS; 
	ASSERT( module_table != NULL ); 

	do 
	{
		init_cs_lock( module_table->lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI uninit_module_info_table( MODULE_INFO_TABLE *module_table )
{
	LRESULT ret = ERROR_SUCCESS; 
	MODULE_INFO_ITERATOR it; 
	ASSERT( module_table != NULL ); 

	do 
	{
		lock_cs( module_table->lock ); 

		for( it = module_table->modules.begin(); it != module_table->modules.end(); it ++ )
		{
			free( *it ); 
		}

		module_table->modules.clear(); 
		unlock_cs( module_table->lock ); 

		uninit_cs_lock( module_table->lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI init_module_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	BOOLEAN work_thread_started = FALSE; 

	do 
	{
		init_cs_lock( proc_modules_lock ); 

		_ret = init_module_info_table( &kernel_modules ); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "\n" ) ); 
			//ret = _ret; 
		}
		
		_ret = init_symbol_mapping(); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "\n" ) ); 
			//ret = _ret; 
		}

		_ret = load_current_module_infos(); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "\n" ) ); 
			//ret = _ret; 
		}
	}while( FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		if( work_thread_started == TRUE )
		{
			_ret = stop_work_thread( &module_info_work[ 0 ] ); 
			if( _ret != ERROR_SUCCESS )
			{
				ASSERT( _ret == ERROR_NOT_READY ); 
				log_trace( ( MSG_ERROR, "stop work thread failed\n" ) ); 
				//ret = _ret; 
			}
		}
	}

	return ret; 
}

LRESULT WINAPI uninit_module_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret;  
	PROCESS_MODULE_INFO_TABLE_ITERATOR it; 

	do 
	{
		for( it = proc_infos.begin(); it != proc_infos.end(); it ++ )
		{
			uninit_module_info_table( &( *it )->modules ); 

			delete ( *it ); 
		}

		uninit_module_info_table( &kernel_modules ); 

		_ret = stop_work_thread( &module_info_work[ 0 ] ); 
		if( _ret != ERROR_SUCCESS )
		{
			ASSERT( _ret == ERROR_NOT_READY ); 
			log_trace( ( MSG_ERROR, "stop work thread failed\n" ) ); 
			ret = _ret; 
		}

	}while( FALSE );

	return ret; 
}

//must hold lock
MODULE_INFO_TABLE *get_proc_modules_lock_free( ULONG proc_id )
{
	MODULE_INFO_TABLE *proc_modules = NULL; 
	PROCESS_MODULE_INFO_TABLE_ITERATOR it; 
	
	do 
	{
		if( TRUE == is_kernel_process( proc_id ) )
		{
			ASSERT( FALSE && "do not need get module informations for system process" ); 
			break; 
		}

		for( it = proc_infos.begin(); it < proc_infos.end(); it ++ )
		{
			if( ( *it )->proc_id == proc_id )
			{
				proc_modules = &( *it )->modules; 
				break; 
			}
		}

		if( proc_modules == NULL )
		{
			break; 
		}
	}while( FALSE );

	return proc_modules; 
}

MODULE_INFO_TABLE *get_proc_modules_on_time_lock_free( ULONG proc_id, LARGE_INTEGER *time )
{
	MODULE_INFO_TABLE *proc_modules = NULL; 
	PROCESS_MODULE_INFO_TABLE_ITERATOR it; 

	do 
	{
		if( TRUE == is_kernel_process( proc_id ) )
		{
			ASSERT( FALSE && "do not need get module informations for system process" ); 
			break; 
		}

		if( time == NULL )
		{
			break; 
		}

		for( it = proc_infos.begin(); it < proc_infos.end(); it ++ )
		{
			if( ( *it )->proc_id == proc_id 
				&& ( *it )->duration.start_time.QuadPart <= time->QuadPart 
				&& ( ( *it )->duration.end_time.QuadPart == 0 
				|| ( *it )->duration.end_time.QuadPart >= time->QuadPart ) )
			{
				proc_modules = &( *it )->modules; 
				break; 
			}
		}

		if( proc_modules == NULL )
		{
			break; 
		}
	}while( FALSE );

	return proc_modules; 
}

LRESULT WINAPI set_current_time( LARGE_INTEGER *time )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	SYSTEMTIME sys_time; 
	FILETIME file_time; 

	ASSERT( time != NULL ); 

	do 
	{
		GetSystemTime( &sys_time ); 
		_ret = SystemTimeToFileTime( &sys_time, &file_time ); 
		if( FALSE == _ret )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			ASSERT( FALSE && "get the time when module loaded error" ); 
			break; 
		}

		time->HighPart = file_time.dwHighDateTime; 
		time->LowPart = file_time.dwLowDateTime; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI on_process_change( ULONG proc_id, 
								 LPCWSTR file_path, 
						  BOOLEAN is_remove )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	MODULE_INFO_ITERATOR it; 
	MODULE_INFO_TABLE *proc_modules; 
	BOOLEAN proc_manage_lock_held = FALSE; 
	BOOLEAN proc_lock_held = FALSE; 

	do 
	{
		//ASSERT( file_path != NULL ); 

		if( is_remove == TRUE )
		{
			if( TRUE == is_kernel_process( proc_id ) )
			{
				ASSERT( FALSE && "system process is exit?" ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
				//proc_modules = &kernel_modules; 
			}
			else
			{
				lock_cs( proc_modules_lock ); 
				proc_manage_lock_held = TRUE; 

				proc_modules = get_proc_modules_lock_free( proc_id ); 

				if( proc_modules == NULL )
				{
					ret = ERROR_NOT_FOUND; 
					ASSERT( FALSE && "can not get process module informations" ); 
					break; 
				}

				lock_cs( proc_modules->lock ); 
				proc_lock_held = TRUE; 
			}

			ret = set_current_time( &proc_modules->duration.end_time ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			for( it = proc_modules->modules.begin(); it != proc_modules->modules.end(); it ++ )
			{
				( *it )->duration.end_time.QuadPart = proc_modules->duration.end_time.QuadPart; 
			}
		}
		else
		{
			if( TRUE == is_kernel_process( proc_id ) )
			{
				//ASSERT( FALSE && "system process is already created." ); 
				//ret = ERROR_INVALID_PARAMETER; 
				//break; 
				proc_modules = &kernel_modules; 
			
				lock_cs( proc_modules->lock ); 
				proc_lock_held = TRUE; 
			}
			else
			{
				lock_cs( proc_modules_lock ); 
				proc_manage_lock_held = TRUE; 

				proc_modules = get_proc_modules_lock_free( proc_id ); 

				if( proc_modules != NULL )
				{
					ASSERT( FALSE && "first receive process created notify." ); 

					ASSERT( proc_modules->modules.size() > 0 ); 

					ASSERT( proc_modules->duration.end_time.QuadPart == 0 ); 
				}
				else
				{
					PROCESS_MODULE_INFO_TABLE *proc_info = NULL; 

					do
					{
						proc_info = new PROCESS_MODULE_INFO_TABLE(); 
						if( proc_info == NULL )
						{
							ret = ERROR_NOT_ENOUGH_MEMORY; 
							break; 
						}

						proc_info->proc_id = proc_id; 
						proc_info->duration.end_time.QuadPart = 0; 

						_ret = get_process_addition_info( proc_id, file_path, proc_info ); 
						if( _ret != ERROR_SUCCESS )
						{
							dbg_print( MSG_FATAL_ERROR, "get the process (%u) additional information error\n", proc_id ); 
							//break; 
						}

						proc_modules = &proc_info->modules; 
						init_module_info_table( proc_modules ); 

						proc_infos.push_back( proc_info ); 

						proc_info = NULL; 
					}while( FALSE ); 

					if( proc_info != NULL )
					{
						delete proc_info; 
					}

					if( ret != ERROR_SUCCESS )
					{
						ASSERT( proc_modules == NULL ); 
						break; 
					}

					lock_cs( proc_modules->lock ); 
					proc_lock_held = TRUE; 
				}
			}

			ret = set_current_time( &proc_modules->duration.start_time ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			for( it = proc_modules->modules.begin(); it != proc_modules->modules.end(); it ++ )
			{
				( *it )->duration.start_time.QuadPart = proc_modules->duration.start_time.QuadPart; 
			}
		}
	}while( FALSE );

	if( proc_lock_held == TRUE )
	{
		unlock_cs( proc_modules->lock ); 
	}

	if( proc_manage_lock_held == TRUE )
	{
		unlock_cs( proc_modules_lock ); 
	}

	return ret; 
}

LRESULT WINAPI format_file_name()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI on_module_change( ULONG proc_id, 
						 MODULE_INFO_PTR *module, 
						 BOOLEAN is_remove )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	BOOLEAN module_set = FALSE; 
	MODULE_INFO_TABLE *proc_modules; 
	BOOLEAN proc_manage_lock_held = FALSE; 
	BOOLEAN proc_lock_held = FALSE; 

	do 
	{
		ASSERT( module != NULL ); 

		if( module->name_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( TRUE == is_kernel_process( proc_id ) )
		{
			proc_modules = &kernel_modules; 
		}
		else
		{
			lock_cs( proc_modules_lock ); 
			proc_manage_lock_held = TRUE; 

			proc_modules = get_proc_modules_lock_free( proc_id ); 
		
			if( proc_modules == NULL )
			{
				ret = ERROR_NOT_FOUND; 
				ASSERT( FALSE && "can not get process module informations" ); 
				break; 
			}
		}

		lock_cs( proc_modules->lock ); 
		proc_lock_held = TRUE; 

		do 
		{
			MODULE_INFO_ITERATOR it; 

			if( is_remove == TRUE )
			{
				for( it = proc_modules->modules.begin(); it != proc_modules->modules.end(); it ++ )
				{
					do 
					{
						if( wcscmp( ( *it )->file_name, 
							module->file_name ) == 0 )
						{
							if( ( *it )->base_addr != module->base_addr )
							{
								ASSERT( FALSE ); 
							}

							if( ( *it )->size != module->size )
							{
								ASSERT( FALSE ); 
							}

							if( ( *it )->duration.end_time.QuadPart != 0 )
							{
								break; 
							}

							ASSERT( FALSE == module_set ); 

							{
								SYSTEMTIME sys_time; 
								FILETIME file_time; 
								
								GetSystemTime( &sys_time ); 
								_ret = SystemTimeToFileTime( &sys_time, &file_time ); 
								if( FALSE == _ret )
								{
									ASSERT( FALSE && "get the time when module loaded error" ); 
									break; 
								}

								( *it )->duration.end_time.HighPart = file_time.dwHighDateTime; 
								( *it )->duration.end_time.LowPart = file_time.dwLowDateTime; 
							}

							module_set = TRUE; 

							break; 
						}

					}while( FALSE ); 
				}

				break; 
			}

#if DEBUG_MODULE_INFO 
			for( it = proc_modules->modules.begin(); it != proc_modules->modules.end(); it ++ )
			{				
				if( wcscmp( ( *it )->file_name, 
					module->file_name ) == 0 )
				{
					if( ( *it )->duration.end_time.QuadPart == 0 )
					{
						break; 
					}
				}

			}
#endif //DEBUG_MODULE_INFO 
			{
				SYSTEMTIME sys_time; 
				FILETIME file_time; 
				MODULE_INFO *_module = NULL; 

				do 
				{
					_module = ( MODULE_INFO* )malloc( sizeof( MODULE_INFO ) + ( module->name_len << 1 ) ); 

					if( NULL == _module )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					GetSystemTime( &sys_time ); 
					_ret = SystemTimeToFileTime( &sys_time, &file_time ); 
					if( FALSE == _ret )
					{
						ASSERT( FALSE && "get the time when module loaded error" ); 
						break; 
					}

					_module->duration.start_time.HighPart = file_time.dwHighDateTime; 
					_module->duration.start_time.LowPart = file_time.dwLowDateTime; 

					_module->base_addr = module->base_addr; 
					_module->sym_base_addr = 0; 
					_module->size = module->size; 

					memcpy( _module->file_name, module->file_name, ( module->name_len << 1 ) ); 

					_module->file_name[ module->name_len ] = L'\0'; 
					_module->name_len = module->name_len; 

					_module->final_component = wcsrchr( _module->file_name, L'\\' );
					if( _module->final_component == NULL )
					{
						_module->final_component = _module->file_name;
					}
					else
					{
						_module->final_component ++; 
					}

					_module->duration.end_time.QuadPart = 0; 

					proc_modules->modules.push_back( _module );  
					_module = NULL; 
				}while( FALSE ); 

				if( NULL != _module )
				{
					free( _module ); 
				}
			}

			//if( module_set == TRUE )
			//{
			//	break; 
			//}
		}while( FALSE );
	}while( FALSE );

	if( proc_lock_held == TRUE )
	{
		ASSERT( proc_modules != NULL ); 
		unlock_cs( proc_modules->lock ); 
	}

	if( proc_manage_lock_held == TRUE )
	{
		unlock_cs( proc_modules_lock ); 
	}

	return ret; 
}

/********************************************************************************************************
保证进程/模块的信息永远不会删除，所以直接返回指针使用。
********************************************************************************************************/
LRESULT WINAPI get_proc_module_info( ULONG proc_id, LARGE_INTEGER *time, MODULE_INFO_TABLE **modules )
{
	LRESULT ret = ERROR_SUCCESS; 
	MODULE_INFO_TABLE *_modules = NULL; 
	BOOLEAN proc_manage_lock_held = FALSE; 

	do 
	{
		ASSERT( modules != NULL ); 
		*modules = NULL; 

		lock_cs( proc_modules_lock ); 
		proc_manage_lock_held = TRUE; 

		_modules = get_proc_modules_on_time_lock_free( proc_id, time ); 

		if( _modules == NULL )
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}

		lock_cs( _modules->lock ); 
		//proc_lock_held = TRUE; 
		*modules = _modules; 

	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		//if( TRUE == proc_lock_held )
		//{
		//	ASSERT( FALSE ); 
		//	unlock_cs( _modules->lock ); 
		//}

		if( TRUE == proc_manage_lock_held ) 
		{
			unlock_cs( proc_modules_lock ); 
		}
	}

	return ret; 
}

PROCESS_MODULE_INFO_TABLE system_proc_info; 
LRESULT WINAPI set_common_system_proc_info()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI _get_proc_module_info( ULONG proc_id, LARGE_INTEGER *time, PROCESS_MODULE_INFO_TABLE **proc_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	PROCESS_MODULE_INFO_TABLE *proc_modules = NULL; 
	PROCESS_MODULE_INFO_TABLE_ITERATOR it; 
	BOOLEAN proc_manage_lock_held = FALSE; 
	BOOLEAN proc_lock_held = FALSE; 

	do 
	{
		lock_cs( proc_modules_lock ); 
		proc_manage_lock_held = TRUE; 

		if( TRUE == is_kernel_process( proc_id ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE && "do not need get module informations for system process" ); 
			break; 
		}

		if( time == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		for( it = proc_infos.begin(); it < proc_infos.end(); it ++ )
		{
			if( ( *it )->proc_id == proc_id 
				&& ( *it )->duration.start_time.QuadPart <= time->QuadPart 
				&& ( ( *it )->duration.end_time.QuadPart == 0 
				|| ( *it )->duration.end_time.QuadPart >= time->QuadPart ) )
			{
				proc_modules = ( *it ); 
				break; 
			}
		}

		if( proc_modules == NULL )
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}

		lock_cs( proc_modules->modules.lock ); 
		proc_lock_held = TRUE; 

	}while( FALSE );

	*proc_info = proc_modules; 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( proc_modules == NULL ); 

		if( TRUE == proc_lock_held )
		{
			ASSERT( FALSE ); 
			unlock_cs( proc_modules->modules.lock ); ; 
		}

		if( TRUE == proc_manage_lock_held )
		{
			unlock_cs( proc_modules_lock ); 
		}
	}

	return ret; 
}

LRESULT WINAPI release_proc_module_info( MODULE_INFO_TABLE *modules )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( modules != NULL ); 
		unlock_cs( modules->lock ); 
		unlock_cs( proc_modules_lock ); 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI _release_proc_module_info( PROCESS_MODULE_INFO_TABLE *proc_info )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( proc_info != NULL ); 
		unlock_cs( proc_info->modules.lock ); 
		unlock_cs( proc_modules_lock ); 

	}while( FALSE );

	return ret; 
}


LRESULT WINAPI recv_event_aio( PVOID input_buf, 
					   ULONG input_size, 
					   PVOID output_buf, 
					   ULONG output_size, 
					   ULONG *ret_len ); 

ULONG CALLBACK thread_module_info_maintain( LPVOID param )
{
	LRESULT ret = ERROR_SUCCESS;
	LRESULT _ret; 
	ULONG ret_len; 
	thread_manage *thread_param; 
	r3_action_notify *action = NULL; 
	event_action_response resp = { 0 }; 

	ASSERT( NULL != param );


	thread_param = ( thread_manage* )param; 

	log_trace( ( MSG_INFO, "%s begin receive module information \n", __FUNCTION__  ) ); 

	do 
	{
		action = ( r3_action_notify* )malloc( R3_NOTIFY_BUF_SIZE ); //sizeof( sys_action_info_notify ) ); 
		if( action == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		for( ; ; )
		{
			//log_trace( ( MSG_INFO, "%s get sys event loop\n", __FUNCTION__ ) ); 

			if( thread_param->stop_run == TRUE )
			{
				break; 
			}

			do 
			{
				ret = recv_event_aio( &resp, 
					sizeof( resp ), 
					action, 
					R3_NOTIFY_BUF_SIZE, 
					&ret_len ); 

				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( MSG_FATAL_ERROR, "receive event aio error %u", ret ) ); 
					Sleep( 100 ); 
					break; 
				}

				_ret = process_proc_module_info( action ); 
				if( _ret != ERROR_SUCCESS )
				{
					
				}

				ret = process_event( action ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}
			} while ( FALSE ); 
		}

	}while( FALSE );

	if( action != NULL )
	{
		free( action ); 
	}

	log_trace( ( MSG_INFO, 
		"%s end receive module information 0x%0.8x \n", 
		__FUNCTION__, 
		ret ) ); 

	//log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ( ULONG )ret; 
}

ULONG CALLBACK thread_trace_event( LPVOID param )
{
	LRESULT ret = ERROR_SUCCESS;
	ULONG ret_len; 
	thread_manage *thread_param; 
	r3_action_notify *action = NULL; 
	event_action_response resp = { 0 }; 

	ASSERT( NULL != param );

	thread_param = ( thread_manage* )param; 

	log_trace( ( MSG_INFO, "%s begin receive module information \n", __FUNCTION__  ) ); 

	do 
	{
		action = ( r3_action_notify* )malloc( R3_NOTIFY_BUF_SIZE ); //sizeof( sys_action_info_notify ) ); 
		if( action == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		for( ; ; )
		{
			if( thread_param->stop_run == TRUE )
			{
				break; 
			}

			do 
			{
				ret = recv_event_aio( &resp, 
					sizeof( resp ), 
					action, 
					R3_NOTIFY_BUF_SIZE, 
					&ret_len ); 

				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( MSG_FATAL_ERROR, "receive event aio error %u", ret ) ); 
					Sleep( 100 ); 
					break; 
				}

				ret = process_event( action ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}
			} while ( FALSE ); 
		}

	}while( FALSE );

	if( action != NULL )
	{
		free( action ); 
	}

	log_trace( ( MSG_INFO, 
		"%s end receive module information 0x%0.8x \n", 
		__FUNCTION__, 
		ret ) ); 

	return ( ULONG )ret; 
}

LRESULT WINAPI get_process_addition_info( ULONG proc_id, LPCWSTR file_path, PROCESS_MODULE_INFO_TABLE *info )
{
	LRESULT ret = ERROR_SUCCESS; 
	CString _text; 

	try
	{
		do 
		{
			ASSERT( proc_id != ( ULONG )INVALID_PROCESS_ID ); 

			ret = info->info.SetProcessId( proc_id ); 
			
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( file_path == NULL )
			{
				info->info.GetMainModuleFullName(); 
			}
			else
			{
				info->info.SetProcessFullName( file_path ); 
			}

			ret = info->info.GetAllInfo(); 
			if( ret != ERROR_SUCCESS )
			{}


		}while( FALSE ); 
	}
	catch( ... )
	{
		ULONG test = 0; 
	}

	return ret; 
}

//使用特殊的消息通道来处理模块/进程的加载，卸载活动。保证信息的正确，与时间,空间一致。

LRESULT WINAPI process_proc_module_info( r3_action_notify *action )
{
	LRESULT ret = ERROR_SUCCESS; 
	MODULE_INFO_PTR module_info; 
	MODULE_INFO_TABLE proc_info; 

	do 
	{
		log_trace( ( MSG_INFO, "action type is %u\n", action->action.action.type ) ) ; 

		switch( action->action.action.type )
		{
		case SYS_load_mod: 
		case EXEC_module_load:
			module_info.base_addr = ( PVOID )( ULONG_PTR )action->action.action.do_exec_module_load.base; 
			module_info.size = ( ULONG )action->action.action.do_exec_module_load.size; 
			set_current_time( &module_info.duration.start_time ); 

			module_info.duration.end_time.QuadPart = 0; 
			module_info.file_name = action->action.action.do_exec_module_load.path_name; 
			module_info.name_len = action->action.action.do_exec_module_load.path_len; 
			module_info.time_stamp = action->action.action.do_exec_module_load.time_stamp; 

			ret = on_module_change( action->action.ctx.proc_id, 
				&module_info, 
				FALSE ); 

			break; 
		case SYS_unload_mod: 
			ASSERT( action->action.action.do_sys_unload_mod.path_name[ action->action.action.do_sys_unload_mod.path_len - 1 ] == L'\0' ); 

			module_info.base_addr = ( PVOID )( ULONG_PTR )action->action.action.do_exec_module_load.base; 
			module_info.size = ( ULONG )action->action.action.do_exec_module_load.size; 
			set_current_time( &module_info.duration.end_time ); 
			module_info.file_name = action->action.action.do_exec_module_load.path_name; 
			module_info.name_len = action->action.action.do_exec_module_load.path_len; 

			ret = on_module_change( action->action.ctx.proc_id, 
				&module_info, 
				TRUE ); 
			break; 

		case PROC_exec: 
			set_current_time( &proc_info.duration.start_time ); 
			proc_info.duration.end_time.QuadPart = 0; 

			ret = on_process_change( action->action.action.do_proc_exec.target_pid, 
				action->action.action.do_proc_exec.path_name, 
				FALSE ); 
			break; 
		case EXEC_destroy:
			set_current_time( &proc_info.duration.end_time ); 
			proc_info.duration.end_time.QuadPart = 0; 

			//因为使用进程创建回调，进程退出时
			ret = on_process_change( action->action.action.do_exec_destroy.pid, 
				action->action.action.do_exec_destroy.path_name, 
				TRUE ); 

			break; 
		default: 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

//加载内核层模块的符号信息

#pragma pack( push )
#pragma pack( 1 )

typedef struct _SYSTEM_MODULE_INFORMATION
{
	PVOID Reserved[2];
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT Index;
	USHORT Unknown;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct __SYSTEM_MODULE_LIST
{
	ULONG_PTR ModuleNum; 
	SYSTEM_MODULE_INFORMATION Modules[ 1 ];
} SYSTEM_MODULE_LIST, *PSYSTEM_MODULE_LIST;

#pragma pack( pop )

typedef enum __SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemNextEventIdInformation,
	SystemEventIdsInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation,
	SystemPowersInformation,
	SystemProcessorSpeedInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

/* extra system functions declaration */
typedef NTSTATUS ( __stdcall * ZW_QUERY_SYSTEM_INFORMATION_FUNC )( 
	SYSTEM_INFORMATION_CLASS SystemInformationClass, 
	PVOID SystemInformation, 
	ULONG SystemInformationLength, 
	PULONG ReturnLength
	);

#define SYSTEM_IDLE_PROCESS_ID 0
#define SYSTEM_SYSTEM_PROCESS_ID 4

#define PAGE_SIZE 0x1000
#define PAGE_ALIGN_MASK 0xFFFFF000
#define PAGE_MASK ( ULONG )( ~PAGE_SIZE )

//#pragma comment( lib, "ntdll.lib" ) 
LRESULT WINAPI load_system_module_info()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	NTSTATUS ntstatus; 
	MODULE_INFO_PTR module; 
	ULONG SysModuleInfoLen = PAGE_SIZE;
	PSYSTEM_MODULE_LIST SysModuleList = NULL;
	PSYSTEM_MODULE_INFORMATION SysModuleInfo;
	ULONG _SysModuleInfoLen;
	ULONG AllModuleNum;
	CHAR *_ModuleName;
	WCHAR module_file_name[ MAX_PATH ]; 
	ULONG name_len; 
	ULONG _name_len; 
	INT32 i; 
	HMODULE nt_dll = NULL; 
	ZW_QUERY_SYSTEM_INFORMATION_FUNC QuerySystemInformationFunc = NULL; 

	do 
	{ 
		nt_dll = LoadLibraryW( L"ntdll.dll" ); 
		if( NULL == nt_dll )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		QuerySystemInformationFunc = ( ZW_QUERY_SYSTEM_INFORMATION_FUNC )GetProcAddress( nt_dll, "ZwQuerySystemInformation" ); 
		if( NULL == QuerySystemInformationFunc )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ntstatus = QuerySystemInformationFunc( SystemModuleInformation, 
			NULL, 
			0, 
			&_SysModuleInfoLen );

		if( !NT_SUCCESS( ntstatus ) )
		{
			if( STATUS_INFO_LENGTH_MISMATCH != ntstatus )
			{
				ret = ERROR_ERRORS_ENCOUNTERED;
				break; 
			}
			else
			{
				SysModuleInfoLen = _SysModuleInfoLen; 
				SysModuleList =( PSYSTEM_MODULE_LIST )malloc( SysModuleInfoLen ); 

				if( NULL == SysModuleList )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}
			}
		}
		else
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break;
		}

		ntstatus = QuerySystemInformationFunc( SystemModuleInformation, 
			SysModuleList, 
			SysModuleInfoLen, 
			&_SysModuleInfoLen );

		if( !NT_SUCCESS( ntstatus ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		AllModuleNum = SysModuleList->ModuleNum;
		if( 0 == AllModuleNum )
		{
			ASSERT( FALSE && "can not get the module list for system" ); 
			ret = ERROR_ERRORS_ENCOUNTERED;
			break; 
		}

		SysModuleInfo = SysModuleList->Modules;

		for( i = 0; ( ULONG )i < AllModuleNum; i ++ )
		{
			do 
			{
				_ModuleName = SysModuleInfo[ i ].ImageName;

				if( *_ModuleName == L'\0' )
				{
					break; 
				}

#define ROOT_SYMBOL_PATH "\\??\\"
				if( 0 == memcmp( _ModuleName, ROOT_SYMBOL_PATH, sizeof( ROOT_SYMBOL_PATH ) - sizeof( WCHAR ) ) ) 
				{
					_ModuleName += ( ARRAYSIZE( ROOT_SYMBOL_PATH ) - 1 ); 
				}

				module.base_addr = SysModuleInfo[ i ].Base; 
				module.size = SysModuleInfo[ i ].Size; 

				_ret = mcbs_to_unicode( ( LPCSTR )_ModuleName, 
					module_file_name, 
					ARRAYSIZE( module_file_name ), 
					&name_len ); 

				if( _ret != ERROR_SUCCESS )
				{
					ret = _ret; 
					log_trace( ( MSG_FATAL_ERROR, "convert the file name of system module error.\n" ) ); 
					break; 
				}

				_ret = convert_symbolic_file_path( module_file_name, 
					name_len, 
					module_file_name, 
					ARRAYSIZE( module_file_name ), 
					&_name_len ); 

				if( ERROR_SUCCESS != _ret )
				{
					log_trace( ( MSG_FATAL_ERROR, "convert the symbol name of the file name of system module error.\n" ) ); 
				}
				else
				{
					name_len = _name_len; 
				}

				_ret = set_current_time( &module.duration.start_time ); 
				if( _ret != ERROR_SUCCESS )
				{
					ret = _ret; 
					log_trace( ( MSG_FATAL_ERROR, "get the start time for system module.\n" ) ); 
				}
				else
				{
					module.name_len = name_len; 
					module.file_name = module_file_name; 

					module.duration.end_time.QuadPart = 0; 

					_ret = get_file_time_stamp( module.file_name, &module.time_stamp ); 

					_ret = on_module_change( SYSTEM_SYSTEM_PROCESS_ID, &module, FALSE ); 
					if( _ret != ERROR_SUCCESS )
					{
						ret = _ret; 
						log_trace( ( MSG_FATAL_ERROR, "notify module status error.\n" ) ); 
					}
				}
			}while( FALSE );
		}
	}while( FALSE );

	if( NULL != SysModuleList )
	{
		free( SysModuleList );
	}

	if( NULL != nt_dll )
	{
		FreeLibrary( nt_dll ); 
	}

	return ret; 
}

#define QUERY_PROCESS_INFORMATION_ACCESS ( PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ )  

LRESULT WINAPI get_file_time_stamp( LPCWSTR file_name, 
								   ULONG *time_stamp )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE                           hFile = INVALID_HANDLE_VALUE;

#define IMAGE_NT_HEADER_PART_SIZE FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) 

	PVOID                            Buffer = NULL;
	DWORD                            BufferSize = sizeof( IMAGE_DOS_HEADER ) + sizeof( IMAGE_NT_HEADER_PART_SIZE ) + 512;

	do 
	{
		ASSERT( file_name != NULL ); 
		ASSERT( time_stamp != NULL ); 

		*time_stamp = 0; 

#if 0
		hFile = CreateFileW(file_name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		if( GetFileSizeEx(hFile,&file_size ) == FALSE)
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		Buffer = malloc (BufferSize);
		if (Buffer == NULL)
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		memset (Buffer,0,BufferSize);

		if (ReadFile (hFile,Buffer,BufferSize,&ReadLength,NULL) == FALSE)
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		if( BufferSize > ReadLength )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		dos_header = ( IMAGE_DOS_HEADER* )Buffer; 

		if( IMAGE_DOS_SIGNATURE != dos_header->e_magic )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ReadLength < ( ULONG )dos_header->e_lfanew + IMAGE_NT_HEADER_PART_SIZE )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			break; 
		}

		nt_headers = ( IMAGE_NT_HEADERS* )( ( PBYTE )dos_header + dos_header->e_lfanew ); 

		if( IMAGE_NT_SIGNATURE != nt_headers->Signature )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*time_stamp = nt_headers->FileHeader.TimeDateStamp; 

#endif //0 

	}while( FALSE );

#if 0
	if( NULL != Buffer )
	{
		free (Buffer); 
	}
	
	if( INVALID_HANDLE_VALUE != hFile )
	{
		CloseHandle( hFile ); 
	}
#endif //0

	return ret; 
}

LRESULT WINAPI load_proc_modules( ULONG proc_id )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	//HRESULT hr; 
	HANDLE hSnapModules = INVALID_HANDLE_VALUE; 
	HANDLE proc_handle = NULL; 
	MODULEENTRY32 stmeModuleEntry32 = { 0 };
	MODULE_INFO_PTR module; 
	ULONG name_len; 

	do 
	{
		if( proc_id == SYSTEM_SYSTEM_PROCESS_ID )
		{
			ret = ERROR_NOT_SUPPORTED; 
			break; 
		}
		else if( proc_id == SYSTEM_IDLE_PROCESS_ID )
		{
			ret = ERROR_NOT_SUPPORTED; 
			break; 
		}

		proc_handle = OpenProcess( QUERY_PROCESS_INFORMATION_ACCESS, FALSE, proc_id ); 
		if(  NULL == proc_handle )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		hSnapModules = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, proc_id );
		if( hSnapModules == INVALID_HANDLE_VALUE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		stmeModuleEntry32.dwSize = sizeof( stmeModuleEntry32 );

		// First entry
		if( !Module32First( hSnapModules, &stmeModuleEntry32 ) ) 
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		// Loop through and add items
		do 
		{
			// Set module entry for module
			
			name_len = ( ULONG )wcslen( stmeModuleEntry32.szExePath ); 

			module.base_addr = stmeModuleEntry32.modBaseAddr; 
			module.size = stmeModuleEntry32.modBaseSize; 

			_ret = set_current_time( &module.duration.start_time ); 
			if( _ret != ERROR_SUCCESS )
			{
				break; 
			}

			module.duration.end_time.QuadPart = 0; 
			module.name_len= name_len; 
			module.file_name = stmeModuleEntry32.szExePath; 

			_ret = get_file_time_stamp( module.file_name, 
				&module.time_stamp ); 

			if( _ret != ERROR_SUCCESS )
			{
			}

			_ret = on_module_change( proc_id, &module, FALSE ); 
			if( _ret != ERROR_SUCCESS )
			{
				break; 
			}

		}while( Module32Next( hSnapModules, &stmeModuleEntry32 ) ); // Get next module

	}while( FALSE );

	if( NULL != hSnapModules )
	{
		CloseHandle( hSnapModules ); 
	}

	if( NULL != proc_handle )
	{
		CloseHandle( proc_handle ); 
	}

	return ret;
} 


LRESULT WINAPI load_current_processes_module_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	PROCESSENTRY32 *proc_entries = NULL; 
	ULONG proc_count; 
	INT32 i; 

	do 
	{
		proc_entries = ( PROCESSENTRY32 *)malloc( sizeof( PROCESSENTRY32 ) * MAX_PROCESS_COUNT ); 
		if( proc_entries == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		ret = get_all_proc_list( proc_entries, MAX_PROCESS_COUNT, &proc_count ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
		
		/***************************************************************************************
		注意:
		
		这里的进程EXE是文件名，不是路径。
		可以在内核中制做PROCESS + MODULE信息映像。但内核不合适实现PROCESS+MODULE的历史记录。
		本身WINDOWS就在内核中包括了PROCESS+MODULE记录，它的性能如何?
		
		要与WINDOWS的内部进行精确，深度整合。尽量不要进行多余的复制型功能实现。
		***************************************************************************************/
		for( i = 0; ( ULONG )i < proc_count; i ++ )
		{
			do 
			{
				dbg_print( MSG_IMPORTANT, "PROCESS ID IS: %u %ws \n", 
					proc_entries[ i ].th32ProcessID, 
					proc_entries[ i ].szExeFile ); 

#ifdef LOAD_CURRENT_PROCESS_MODULE_INFO
				_ret = on_process_change( proc_entries[ i ].th32ProcessID, 
					NULL, 
					FALSE ); 

				if( ERROR_SUCCESS != _ret )
				{
					log_trace( ( MSG_IMPORTANT, "add process [%u] information error\n", proc_entries[ i ].th32ProcessID ) ); 
					break; 
				}

				_ret = load_proc_modules( proc_entries[ i ].th32ProcessID ); 
				if( ERROR_SUCCESS != ret )
				{
					log_trace( ( MSG_IMPORTANT, "add the modules information for the process [%u] error\n", proc_entries[ i ].th32ProcessID ) ); 
					break; 
				}
#endif //LOAD_CURRENT_PROCESS_MODULE_INFO

			}while( FALSE );
		}
	}while( FALSE );

	if( proc_entries != NULL )
	{
		free( proc_entries ); 
	}

	return ret; 
}

LRESULT WINAPI load_current_module_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		_ret = load_system_module_info(); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_FATAL_ERROR, "load system modules information of system now error" ) ); 
			ret = _ret; 
		}

#if 1
		_ret = load_current_processes_module_infos(); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_FATAL_ERROR, "load the modules information of all process now error" ) ); 
			ret = _ret; 
		}
#endif //0

	}while( FALSE );

	return ret; 
}
