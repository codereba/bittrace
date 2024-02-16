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

#ifndef __BTISAFE_ACTION_SVC_H__
#define __BTISAFE_ACTION_SVC_H__

typedef struct _GLOBAL_CENTER_DATA_
{
	SERVICE_STATUS_HANDLE ServiceStatusHandle;
	SERVICE_STATUS        ServiceStatus;
	BOOL                  ServiceShouldExit;
} GLOBAL_CENTER_DATA,*PGLOBAL_CENTER_DATA; 

#define _IF_TEST_

#define WAIT_SERVICE_STOP_TIME 1000
#define WAIT_CONTEXT_START_PENDING_TIME 100
#define ACTION_RESPONSE_SERVICE_NAME L"BITSAFE_RESPONSOR"

#define GOLBAL_MUTEX_FOR_ACTION_RESPONSOR ( L"Global\\F6371BEA-8246-4F80-B18D-CC3226D788FD" ) //( L"Global\\BITSAFE_ACTION_RESPONSOR" )

#define MAX_WORK_THREAD_COUNT 16 

LRESULT init_r3_action_handler(); 

#define _IF_TEST_

#define GOLBAL_MUTEX_FOR_ACTION_RESPONSOR ( L"Global\\F6371BEA-8246-4F80-B18D-CC3226D788FD" ) //( L"Global\\BITSAFE_ACTION_RESPONSOR" )

#define MAX_WORK_THREAD_COUNT 16 
typedef struct _resp_thread_param
{
	PVOID context; 
	PVOID param; 
} resp_thread_param, *presp_thread_param; 

typedef struct _sys_action_response
{
	HANDLE work_port; 
	HANDLE completion; 
	ULONG thread_count; 
	ULONG max_thread_count; 
	BOOLEAN stop; 
	HANDLE threads[ MAX_WORK_THREAD_COUNT ]; 
	resp_thread_param thread_params[ MAX_WORK_THREAD_COUNT ]; 
} sys_action_response, *psys_action_response; 

//extern sys_action_response response_info; 

typedef struct _service_manage
{
	SERVICE_STATUS status; 
	SERVICE_STATUS_HANDLE status_handle; 
	BOOLEAN stopping; 
	//HANDLE stop_notify; 
}service_manage, *pservice_manage; 

LRESULT init_action_response_context( PVOID context, PVOID param ); 
LRESULT uninit_action_response_context(); 

VOID report_svc_status( IN DWORD dwCurrentState,IN DWORD dwWin32ExitCode,IN DWORD dwWaitHint ); 

VOID WINAPI ServiceCtrlHandler (IN DWORD dwCtrl); 

VOID WINAPI ServiceMain (IN DWORD dwArgc,IN LPTSTR* lpszArgv); 

DWORD sys_action_response_thread( IN PVOID context ); 


//LRESULT r3_check_sys_action( sys_action_info_notify *action, event_action_response *resp ); 

//LRESULT r3_check_sys_action( sys_action_desc *action, event_action_response *resp ); 
LRESULT load_acl_to_ring3(); 
LRESULT release_acl_from_ring3(); 

#include "action_log_db.h"
#include "action_display.h"

FORCEINLINE LRESULT analyze_sys_action( r3_action_notify *sys_action, event_action_response *response )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
#ifdef DEBUG
		if( ( sys_action->data_size > ( ( sys_action->size - ACTION_RECORD_OFFEST ) - sys_action->action.action.size ) )
			|| ( sys_action->data_size > 0 
			&& FALSE == IsBadReadPtr( sys_action->append, sys_action->data_size ) ) )
		{
			ASSERT( FALSE ); 
			sys_action->data_size = 0; 
		}
#else
		if( sys_action->data_size > ( ( sys_action->size - ACTION_RECORD_OFFEST ) - sys_action->action.action.size ) )
		{
			ASSERT( FALSE ); 
			sys_action->data_size = 0; 
		}
#endif //DEBUG

		response->action = ACTION_ALLOW; 
		response->id = 0; 
		response->need_record = RECORD_APP_ACTION; 

		{
			WCHAR action_tip[ MAX_ACTION_TIP_LEN ]; 

			//DBG_BP(); 

			do
			{

				ret = get_action_tip( &sys_action->action.action, 
					&sys_action->action.ctx, 
					action_tip, 
					ARRAY_SIZE( action_tip ), 
					0 ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				dbg_print( MSG_IMPORTANT, "%ws\n", action_tip ); 
			}while( FALSE ); 
		}

#define DATA_TEXT_SIZE 1024 

#ifdef R3_REMAPPED_VM_DEFINED
		if( sys_action->data_buf.mdl != NULL 
			&& sys_action->data_buf.r3_addr!= NULL 
			&& sys_action->data_buf.vm_size > 0 )		
		{
			dump_mem( sys_action->data_buf.r3_addr, 
				sys_action->data_buf.vm_size ); 
		
			do 
			{
				CHAR data_text[ DATA_TEXT_SIZE ]; 

				ret = dump_mem_in_buf( data_text,
					sizeof( data_text ), 
					sys_action->data_buf.r3_addr, 
					sys_action->data_buf.vm_size ); 

			}while( FALSE );
		}
#else
		if( sys_action->data_size > 0 )		
		{
			PVOID action_data; 

			action_data = get_action_output_data( sys_action ); 
			
			dump_mem( action_data, 
				sys_action->data_size ); 

			do 
			{
				CHAR data_text[ DATA_TEXT_SIZE ]; 

				ret = dump_mem_in_buf( data_text,
					sizeof( data_text ), 
					action_data, 
					sys_action->data_size ); 

			}while( FALSE );
		}
#endif //R3_REMAPPED_VM_DEFINED

		//ret = add_log_ex( sys_action ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	log_trace( ( MSG_ERROR, "logging action error %u\n", ret ) ); 
		//	//break; 
		//}

		//ret = r3_check_sys_action( sys_action, response ); 

	}while( FALSE ); 

	return ret; 
}

INLINE LRESULT init_bitsafe_action_handler( PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		//init_bitsafe_policy_nfa(); 

		ret = load_acl_to_ring3(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load all acl to ring3 error 0x%0.8x\n", ret ) ); 

			ASSERT( FALSE && "load all acl to ring3\n" ); 
			break; 
		}

		ret = init_action_response_context( context, param ); 
	} while ( FALSE );

	return ret; 
}

INLINE LRESULT uninit_bitsafe_action_handler()
{
	LRESULT ret = ERROR_SUCCESS; 

	uninit_action_response_context(); 
	release_acl_from_ring3(); 

	return ret; 
}

#endif //__BTISAFE_ACTION_SVC_H__