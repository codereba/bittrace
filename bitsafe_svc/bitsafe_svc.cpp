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
 
 #include "StdAfx.h"
#include "common_func.h"
#include "trace_log_api.h"
#include "seven_fw_api.h"
#include "sys_mng_api.h"
#include "antiarp_api.h"
#include "fs_mng_api.h"
#include "tdifw_api.h"
#include "inst_drv.h"
#include "net_svc_inst.h"
#include "drv_load.h"
#include "inst_im.h"
#include "ui_ctrl.h"
#include "resource.h"
#ifdef BITSAFE_TEST_TOOL
#include "../SevenFw/_resource.h"
#endif //BITSAFE_TEST_TOOL
#include "local_strings.h"
#include "mini_port_dev.h"
#include "mem_region_list.h"
#include "conf_file.h"
#include "bitsafe_daemon.h"
#include "bitsafe_svc.h"

#pragma pack( pop )

//#ifdef NTDDI_VERSION
//#undef NTDDI_VERSION
#define NTDDI_VERSION ( NTDDI_WINXP | NTDDI_WINXPSP2 )
//#include <DriverSpecs.h>
////#endif //NTDDI_VERSION
//__user_code
#define __struct_bcount( _x_ )

#include <fltUser.h>

#pragma comment( lib, "fltLib.lib" ) 

#pragma pack( push )
#pragma pack( 1 )

//#define FILTER_MESSAGE_HEADER_SIZE 16
//#if sizeof( FILTER_MESSAGE_HEADER ) == FILTER_MESSAGE_HEADER_SIZE 
typedef struct _RESPONCE_MESSAGE_
{
	FILTER_MESSAGE_HEADER msg_hdr;
	r3_action_notify action_info; 
	BYTE data_buf[ DEFAULT_PARAMETERS_VARIABLE_SIZE + DEFAULT_OUTPUT_DATA_REGION_SIZE ]; 
	//sys_action_desc action; 
	OVERLAPPED ovlp;
} RESPONCE_MESSAGE, *PRESPONCE_MESSAGE;

typedef struct _REPLY_MESSAGE_
{
	FILTER_REPLY_HEADER reply_hdr;

	event_action_response response; 
} REPLY_MESSAGE, *PREPLY_MESSAGE;

//#else 
//
//typedef struct _RESPONCE_MESSAGE_
//{
//	FILTER_MESSAGE_HEADER msg_hdr;
//	ULONG unknown_pad; 
//	sys_action_desc action; 
//	OVERLAPPED ovlp;
//} 
//RESPONCE_MESSAGE, *PRESPONCE_MESSAGE;
//
//typedef struct _REPLY_MESSAGE_
//{
//	FILTER_REPLY_HEADER reply_hdr;
//	ULONG unknown_pad; 
//	event_action_response response; 
//} REPLY_MESSAGE, *PREPLY_MESSAGE;
//
//#endif //
#pragma pack( pop )

sys_action_response response_info = { 0 }; 
service_manage service_management = { 0 }; 

VOID report_svc_status( IN DWORD dwCurrentState,IN DWORD dwWin32ExitCode,IN DWORD dwWaitHint )
{
	INT32 _ret; 
	ULONG ret_code; 
    static DWORD dwCheckPoint = 1;

	service_management.status.dwCurrentState = dwCurrentState;
	service_management.status.dwWin32ExitCode = dwWin32ExitCode;
	service_management.status.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
	{
		service_management.status.dwControlsAccepted = 0;
	}
    else
	{
		service_management.status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED) )
	{
        service_management.status.dwCheckPoint = 0;
	}
    else 
	{
		service_management.status.dwCheckPoint = dwCheckPoint++;
	}

	_ret = SetServiceStatus( service_management.status_handle, &service_management.status );

	if( _ret == FALSE )
	{
		ret_code = GetLastError(); 
		dbg_print( MSG_ERROR, "set service status to %u error %u\n", 
			service_management.status, 
			ret_code ); 
	}
}


VOID WINAPI ServiceCtrlHandler (IN DWORD dwCtrl)
{
   switch (dwCtrl) 
   {       
   case SERVICE_CONTROL_STOP: 	  
	   {
		   report_svc_status( SERVICE_STOP_PENDING,NO_ERROR,0 ); 
		   service_management.stopping = TRUE;
		   return;
	   }
      
   case SERVICE_CONTROL_INTERROGATE: 		 
	   {        
		   break;  
	   }
   default:  
	   {
		   break;
	   } 
   } 

   report_svc_status( service_management.status.dwCurrentState, NO_ERROR, 0 );
}

VOID WINAPI ServiceMain (IN DWORD dwArgc,IN LPTSTR* lpszArgv)
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		service_management.stopping = FALSE; 

		CPaintManagerUI::SetInstance( GetModuleHandle( NULL ) ); 

		CPaintManagerUI::SetResourcePath( CPaintManagerUI::GetInstancePath() );
		//CPaintManagerUI::SetResourceZip( UI_FILE_NAME );
		CPaintManagerUI::SetResourceZipResType( DAT_FILE_RES_TYPE, IDR_UI_ZIP_FILE ); 

		ret = safe_open_bitsafe_conf(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load bitsafe config failed \n" ) ); 

			ASSERT( FALSE && "load bitsafe config failed \n" ); 
			break; 
		}

		//{
		//	NTSTATUS ntstatus; 
		//	ntstatus = init_bitsafe_policy_nfa(); 
		//	if( ntstatus != STATUS_SUCCESS )
		//	{
		//		log_trace( ( MSG_ERROR, "load bitsafe nfa policy failed \n" ) ); 

		//		ASSERT( FALSE && "load bitsafe nfa policy failed \n" ); 
		//		break; 
		//	}
		//}

		ret = load_acl_to_ring3(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load all acl to ring3 error 0x%0.8x\n", ret ) ); 

			ASSERT( FALSE && "load all acl to ring3\n" ); 
			break; 
		}

		ret = init_action_response_context( NULL, NULL ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "init action response context error\n" ); 
			break; 
		}

		{
			Sleep( WAIT_CONTEXT_START_PENDING_TIME );
		}

		service_management.status_handle = RegisterServiceCtrlHandlerW( ACTION_RESPONSE_SERVICE_NAME,ServiceCtrlHandler );
		if( service_management.status_handle == NULL )
		{ 
			log_trace( ( MSG_ERROR, "CR %d",GetLastError () ) );
			break; 
		} 

		service_management.status.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
		service_management.status.dwServiceSpecificExitCode = 0;    

		report_svc_status( SERVICE_START_PENDING, NO_ERROR, 3000 );
		report_svc_status( SERVICE_RUNNING, NO_ERROR, 0 );

		while( TRUE )
		{
			if( service_management.stopping == TRUE )
			{
				report_svc_status( SERVICE_STOPPED, 
					NO_ERROR, 
					0 ); 

				break;
			}

			Sleep( WAIT_SERVICE_STOP_TIME );
		}

	}while( FALSE );

	uninit_action_response_context(); 
	release_acl_from_ring3(); 

}

DWORD sys_action_response_thread( IN PVOID context )
{ 
	LRESULT ret = 0; 
	HRESULT _ret; 
	LPOVERLAPPED      pOvlp;
	DWORD             Size;
	ULONG_PTR         Key; 
	RESPONCE_MESSAGE cur_action; 
	REPLY_MESSAGE response; 
	RESPONCE_MESSAGE *_cur_action; 
	UI_CTRL *ui_ctrl = NULL; 
	r3_action_notify *cur_sys_action; 
	//sys_action_desc *cur_action_desc = NULL; 
	sys_action_and_data_output sys_action_info; 
	resp_thread_param *thread_param = NULL; 
	PVOID _param = NULL; 

	//if( response_info.thread_count > response_info.max_thread_count )
	//{
	//	log_trace( ( MSG_ERROR, "work thread is too more." ) ); 
	//}

	//response_info.thread_count ++;

	do 
	{
		ASSERT( R3_NOTIFY_BUF_SIZE <= FIELD_OFFSET( RESPONCE_MESSAGE, ovlp ) ); 

		if( NULL != context ) 
		{
			thread_param = ( resp_thread_param* )context; 
			_param = thread_param->param; 
			ui_ctrl = ( UI_CTRL* )thread_param->context; 
		}

		//cur_action_desc = ( sys_action_desc* )malloc( sizeof( sys_action_desc ) ); 
		//if( cur_action_desc == NULL )
		//{
		//	ret = ERROR_NOT_ENOUGH_MEMORY; 
		//	break; 
		//}

		memset( &cur_action, 0, sizeof( cur_action ) ); 

		while (TRUE)
		{
			if( response_info.stop == TRUE )
			{
				break;
			}

			log_trace( ( MSG_INFO, __FUNCTION__ "sizeof( RESPONCE_MESSAGE ) is %u, sizeof( sys_action_desc ) is %u\n", 
				sizeof( RESPONCE_MESSAGE ), 
				sizeof( sys_action_info ) ) ); 

			memset( &cur_action.ovlp, 0, sizeof( OVERLAPPED ) ); 

			_ret = FilterGetMessage( response_info.work_port, 
				&cur_action.msg_hdr, 
				FIELD_OFFSET( RESPONCE_MESSAGE, ovlp ), 
				&cur_action.ovlp ); 

			if( _ret != HRESULT_FROM_WIN32( ERROR_IO_PENDING ) ) 
			{
				switch( _ret )
				{
				case ERROR_INSUFFICIENT_BUFFER:
					ASSERT( FALSE && "resize of the buffer for receive message\n" ); 

					//ret = resize_r3_output_buffer( output_buffer );
					log_trace( ( DBG_MSG_AND_ERROR_OUT, "the minifilter message header size if %u\n", sizeof( FILTER_MESSAGE_HEADER ) ) ); 
					break; 
				default:
					break; 
				}

				DBG_BP(); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				log_trace( ( MSG_ERROR, "get the filter message not return ERROR_IO_PENDING 0x%0.8x\n", _ret ) ); 
				break;
			}

			if( GetQueuedCompletionStatus( response_info.completion, &Size, &Key, &pOvlp, INFINITE ) == FALSE )
			{
				DBG_BP(); 
				ret = GetLastError(); 
				ASSERT( FALSE ); 

				log_trace( ( DBG_MSG_AND_ERROR_OUT, "get the message from completion port error 0x%0.8x\n", ret ) ); 
				
				switch( ret )
				{
				case ERROR_INVALID_HANDLE:
					break; 
#ifdef _DEBUG
				case ERROR_INSUFFICIENT_BUFFER:
					ASSERT( FALSE && "resize of the buffer for receive message\n" ); 

					//ret = resize_r3_output_buffer( output_buffer );
					log_trace( ( DBG_MSG_AND_ERROR_OUT, "the minifilter message header size if %u\n", sizeof( FILTER_MESSAGE_HEADER ) ) ); 
					break; 
#endif //_DEBUG	
				default:
					break; 
				}

				break;
			}

			DBG_BP(); 

			//__asm int 3; 

			if( pOvlp == NULL )
			{
				DBG_BP(); 
				log_trace( ( MSG_ERROR, "return null buffer from completion \n" ) ); 
				break;
			}

			_cur_action = CONTAINING_RECORD( pOvlp, RESPONCE_MESSAGE, ovlp );

			ASSERT( _cur_action == &cur_action ); 

			cur_sys_action = &_cur_action->action_info; 

#ifdef DEBUG
			if( _cur_action->action_info.action.action.type == SOCKET_CONNECT )
			{
				DBG_BP(); 
			}
#endif //DEBUG

			//cur_action_desc = NULL; 

			//Policy and reply.

			dbg_print( MSG_IMPORTANT, "receive system action type: %u:%ws\n", 
				_cur_action->action_info.action.action.type, 
				get_sys_action_desc( _cur_action->action_info.action.action.type ) ); 

			ret = analyze_sys_action( &_cur_action->action_info, 
				&response.response ); 

			ASSERT( TRUE == is_valid_response_type( response.response.action ) ); 

			if( ui_ctrl != NULL )
			{
				if( response.response.action == ACTION_LEARN )
				{
					sys_action_info.action = &_cur_action->action_info.action; 
					sys_action_info.data_buf = ( PBYTE )get_action_output_data( &_cur_action->action_info ); 
					sys_action_info.data_size = _cur_action->action_info.data_size; 

					ui_ctrl->ui_output_func( SYS_MON_R3_ACTION_REQUEST, ret, ( PBYTE )&sys_action_info, sizeof( sys_action_info ), _param ); 
				}
			}

			//if( cur_action_desc != NULL )
			//{
			//	free( cur_action_desc ); 
			//	cur_action_desc = NULL; 
			//}

			response.reply_hdr.Status = 0;
			response.reply_hdr.MessageId = _cur_action->msg_hdr.MessageId; 

			_ret = FilterReplyMessage( response_info.work_port,( PFILTER_REPLY_HEADER )&response, sizeof( REPLY_MESSAGE ) );
			if( !SUCCEEDED( _ret ) ) 
			{
				log_trace( ( MSG_ERROR, "response the sys action from ring3 error 0x%0.8x\n", _ret ) ); 
			}

			Sleep( 10 );
		}

		if( ret != ERROR_SUCCESS ) 
		{
			if( ret == ERROR_INVALID_HANDLE ) 
			{
				//port disconnected.
				log_trace( ( MSG_ERROR, "ring3 port disconnected\n" ) );
			} 
			else
			{

			}
		}

	}while( FALSE );

	//if( cur_action_desc != NULL )
	//{
	//	free( cur_action_desc ); 
	//}

	response_info.thread_count --;
	if( response_info.thread_count == 0)
	{
		CloseHandle( response_info.work_port );
		CloseHandle( response_info.completion );
	}

	return ( DWORD )ret;
}

#include "ring0_2_ring3.h"
#include "action_type.h"
#include "action_check.h"
//#include "action_source.h"
//#include "static_policy.h"
//#include "action_group_policy.h"
//#include "action_policy_common.h"
//#include "action_learn.h"

#define RING3_ACTION_RESPONSE_THREAD_COUNT 1
LRESULT init_action_response_context( PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 
	HRESULT _ret; 
	HANDLE hPort = NULL; 
	HANDLE hCompletion = NULL;
	HANDLE Handle;
	DWORD i;

	//test
	memset( &response_info, 0, sizeof( response_info ) ); 

	response_info.max_thread_count = MAX_WORK_THREAD_COUNT; 
	response_info.thread_count = RING3_ACTION_RESPONSE_THREAD_COUNT; 

	DBG_BP(); 

	do 
	{
		ntstatus = init_action_check_context(); 
		if( ntstatus != STATUS_SUCCESS )
		{
			ASSERT( FALSE ); 
			log_trace( ( MSG_FATAL_ERROR, "prepare action analyzation nfa error\n" ) ); 
			break; 
		}

		_ret = FilterConnectCommunicationPort( RING3_INTERFACE_PORT_NAME, 0, NULL, 0, NULL, &hPort );
		if( IS_ERROR( _ret ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hCompletion = CreateIoCompletionPort( hPort, NULL, 0, response_info.thread_count );
		if( hCompletion == NULL ) 
		{
			ret = GetLastError(); 
			break; 
		}

		response_info.work_port = hPort;
		response_info.completion = hCompletion;

		for( i=0; i < response_info.thread_count; i++ )
		{
			response_info.thread_params[ i ].param = param; 
			response_info.thread_params[ i ].context = context; 

			Handle = CreateThread( NULL, 0, 
				( LPTHREAD_START_ROUTINE )sys_action_response_thread, 
				&response_info.thread_params[ i ], 
				0, 
				NULL ); 

			if( Handle == NULL )
			{
				INT32 j;  

				response_info.stop = TRUE;
				
				CloseHandle( hPort ); 
				hPort = NULL; 

				CloseHandle( hCompletion ); 
				hCompletion = NULL;

				ret = ERROR_ERRORS_ENCOUNTERED; 

				for( j = i - 1; j >= 0; j -- )
				{
					ASSERT( response_info.threads[ j ] != NULL ); 
				}

				WaitForMultipleObjectsEx( i - 1, response_info.threads, TRUE, INFINITE, FALSE ); 
				
				for( j = i - 1; j >= 0; j -- )
				{
					CloseHandle( response_info.threads[ j ] ); 
				}

				response_info.thread_count = 0; 
				response_info.completion = 0; 
				response_info.work_port = 0; 
				break; 
			}
			else
			{
				response_info.threads[ i ] = Handle; 
			}
		}
	} while ( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		if( hPort != NULL )
		{
			CloseHandle( hPort );
		}

		if( hCompletion != NULL )
		{
			CloseHandle( hCompletion );
		}
	}

	return ret; 
}

LRESULT uninit_action_response_context()
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD _ret; 
	INT32 i;  

	response_info.stop = TRUE;

	CloseHandle( response_info.work_port ); 
	response_info.work_port = NULL; 

	CloseHandle( response_info.completion ); 
	response_info.completion = NULL;

	for( i = 0; ( ULONG )i < response_info.thread_count; i ++ )
	{
		ASSERT( response_info.threads[ i ] != NULL ); 
	}

	_ret = WaitForMultipleObjectsEx( response_info.thread_count, response_info.threads, TRUE, INFINITE, FALSE ); 

	if( _ret != WAIT_OBJECT_0 )
	{
		ret = GetLastError(); 
	}

	for( i = 0; ( ULONG )i < response_info.thread_count; i ++ )
	{
		CloseHandle( response_info.threads[ i ] ); 
	}

	{
		NTSTATUS ntstatus; 
		ntstatus = uninit_action_check_context(); 

		ASSERT( ntstatus == STATUS_SUCCESS ); 
	}
	return ret; 
}

#ifdef BITSAFE_SERVICE_EXE
#include <assert.h>
#define ASSERT( x ) assert( x )

void main (IN INT Argc,IN PCHAR *Argv)
{

	__asm int 3; 

	/*HANDLE hMutex = NULL;

	hMutex = OpenMutexW (NULL,FALSE,GOLBAL_MUTEX_NAME_FOR_SERVER_XDHY);
	if (hMutex == NULL)
	{
		if (GetLastError () == ERROR_FILE_NOT_FOUND)
		{
			hMutex = CreateMutexW (NULL,FALSE,GOLBAL_MUTEX_NAME_FOR_SERVER_XDHY);
			if (hMutex == NULL)
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		CloseHandle (hMutex);
		return;
	}*/
	ASSERT( sizeof( FILTER_MESSAGE_HEADER ) == 16 ); 
	ASSERT( sizeof( FILTER_REPLY_HEADER ) == 16 ); 

#ifdef _IF_TEST_
	return ServiceMain (0,NULL);
#endif 

	SERVICE_TABLE_ENTRYW DispatchTable[] = 
    { 
		{ ACTION_RESPONSE_SERVICE_NAME, ( LPSERVICE_MAIN_FUNCTION )ServiceMain }, 
        { NULL, NULL } 
    }; 

	if( StartServiceCtrlDispatcher( DispatchTable ) == FALSE ) 
    { 
        return;
    } 
}

#endif //BITSAFE_SERVICE_EXE