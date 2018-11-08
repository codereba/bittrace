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

#include "stdafx.h"
#include "common_func.h"
#define INITGUID
#include <Wmistr.h>
#include <evntrace.h>
#include <strsafe.h>
#include <conio.h>
#include "tracer.h"

/************************************************************************
跟踪信息类型定义:
1.所有之前BITSAFE进行安全保护的信息类型。
2.加入WINDOWS XP和WINDOWS 7系统内置信息类型的支持。
3.充分利用PERFERMENCE NONITOR COUNTER。
4.
************************************************************************/


//#define LOGFILE_PATH L"c:\\tracelog.etl"
#define LOGSESSION_NAME L"TraceLogSession"

// GUID that identifies your trace session.
// Remember to create your own session GUID.

// {AE44CB98-BD11-4069-8093-770EC9258A12}
static const GUID SessionGuid = 
{ 0xae44cb98, 0xbd11, 0x4069, { 0x80, 0x93, 0x77, 0xe, 0xc9, 0x25, 0x8a, 0x12 } };

// GUID that identifies the provider that you want
// to enable to your session.

// {D8909C24-5BE9-4502-98CA-AB7BDC24899D}
//static const GUID ProviderGuid = 
//{ 0xd8909c24, 0x5be9, 0x4502, {0x98, 0xca, 0xab, 0x7b, 0xdc, 0x24, 0x89, 0x9d } };

#define WPP_GUID_TEXT(l,w1,w2,w3,ll) #l "-" #w1 "-" #w2 "-" #w3 "-" #ll
//#define WPP_GUID_WTEXT(l,w1,w2,w3,ll) _WPPW(#l) L"-" _WPPW(#w1) L"-" _WPPW(#w2) L"-" _WPPW(#w3) L"-" _WPPW(#ll)
#define WPP_EXTRACT_BYTE(val,n) (((ULONGLONG)(0x ## val) >> (8 * n)) & 0xFF)
#define WPP_GUID_STRUCT(l,w1,w2,w3,ll) {0x ## l, 0x ## w1, 0x ## w2,\
{WPP_EXTRACT_BYTE(w3, 1), WPP_EXTRACT_BYTE(w3, 0),\
	WPP_EXTRACT_BYTE(ll, 5), WPP_EXTRACT_BYTE(ll, 4),\
	WPP_EXTRACT_BYTE(ll, 3), WPP_EXTRACT_BYTE(ll, 2),\
	WPP_EXTRACT_BYTE(ll, 1), WPP_EXTRACT_BYTE(ll, 0)} }

static const GUID bitsafe_trace_guid = WPP_GUID_STRUCT(d58c128e, b309, 11d1, 969e, 0000f875a5bc); 
//etw_trace_guid = {0xb5a0bda9, 0x50fe, 0x4d0e, {0xa8, 0x3d, 0xba, 0xe3, 0xf5, 0x8c, 0x94, 0xd6}};

static const GUID ProviderGuid = bitsafe_trace_guid; 


//VOID TestEventTrace( void ); 

// Prototypes for the callback functions.
// Use this prototype if you use the trace data helper (TDH) functions
// to consume manifest-, MOF-, or TMF-defined events.
//void WINAPI ProcessEvents(PEVENT_RECORD pEvent);

// Use this prototype if you consume MOF-defined events.
//void WINAPI ProcessEvents(PEVENT_TRACE pEvent);
//
//DEFINE_GUID ( /* 90cbdc39-4a3e-11d1-84f4-0000f80464e3 */
//			 FileIoGuid,
//			 0x90cbdc39,
//			 0x4a3e,
//			 0x11d1,
//			 0x84, 0xf4, 0x00, 0x00, 0xf8, 0x04, 0x64, 0xe3
//			 );
//
//DEFINE_GUID ( /* 3d6fa8d4-fe05-11d0-9dda-00c04fd7ba7c */
//			 DiskIoGuid,
//			 0x3d6fa8d4,
//			 0xfe05,
//			 0x11d0,
//			 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
//			 );

//Prototype for the EventClassCallback functions.

void WINAPI ProcessBuffers(PEVENT_TRACE_LOGFILE pBuffer)
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return; 
}

void WINAPI ProcessDiskEvents(PEVENT_TRACE pEvent)
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return; 
}

void WINAPI ProcessFileEvents(PEVENT_TRACE pEvent)
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return; 
}

typedef struct _BIT_TRACE_CONTEXT
{
	TRACEHANDLE trace_handle; 
} BIT_TRACE_CONTEXT, *PBIT_TRACE_CONTEXT; 

/***************************************************************************************
etw trace command:
wevtutil im evntdrv.xml
Tracelog -start TestEventdrv -guid #b5a0bda9-50fe-4d0e-a83d-bae3f58c94d6 -f Eventdrv.etl
tracelog -stop TestEventdrv
tracerpt Eventdrv.etl
wevtutil um evntdrv.xml

wpp trace command:
tracepdb -f <path>\tracedrv.pdb
tracelog -start TestTracedrv -guid tracedrv.ctl -f tracedrv.etl -flag 1
tracelog -stop TestTracedrv
tracefmt tracedrv.etl -p <Path to TMF file> -o Tracedrv.out

*****************************************************************************************/

void WINAPI ProcessDiskEvents(PEVENT_TRACE pEvent);
void WINAPI ProcessFileEvents(PEVENT_TRACE pEvent);
void WINAPI ProcessHeaderEvent(PEVENT_TRACE pEvent);
typedef struct FILE_STANDARD_INFORMATION {
	LARGE_INTEGER  AllocationSize;
	LARGE_INTEGER  EndOfFile;
	ULONG  NumberOfLinks;
	BOOLEAN  DeletePending;
	BOOLEAN  Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

struct DiskIo_TypeGroup1
{
	UINT32	DiskNumber;
	UINT32	IrpFlags;
	UINT32	TransferSize;
	UINT32	QueueDepth;
	INT64	ByteOffset;
	UINT32	FileObject;
};

VOID WINAPI EventCallback(PEVENT_TRACE pEvent)
{
}

LRESULT start_log_trace(); 

#define MAX_STOP_STARTED_SESSION_TIME 3

#include "stack_dump.h"

LRESULT WINAPI test_dbghelp()
{
	LRESULT ret = ERROR_SUCCESS; 
	LPWSTR stack_dump = NULL; 
	ULONG cch_ret_len; 
	ULONGLONG test_addresses[ ] = { 0x5bfa0213ULL, 0x5bfa03ABULL }; 

	do 
	{
		stack_dump = ( LPWSTR ) malloc( MAX_STACK_DUMP_TEXT_LEN << 1 ); 
		if( stack_dump == NULL )
		{
			break; 
		}

		stack_dump[ MAX_STACK_DUMP_TEXT_LEN - 1 ] = L'\0'; 

		/*********************************************************************************
		直接使用DBGHELP提供的方法加载事件的堆栈信息的非常占用时间，每秒加载的事件只能达到百量级,
		必须对其结构有所完善，或者不加载堆栈信息。
		*********************************************************************************/
		ret = _get_stack_trace_dump_text( GetCurrentProcessId(), 
			0, 
			test_addresses, 
			ARRAYSIZE( test_addresses ), 
			stack_dump, 
			MAX_STACK_DUMP_TEXT_LEN, 
			&cch_ret_len );

	}while( FALSE );

	if( NULL != stack_dump )
	{
		free( stack_dump ); 
	}

	return ret; 
}

LRESULT prepare_log_trace( trace_session *session )  
{
	LRESULT ret = ERROR_SUCCESS; 
	EVENT_TRACE_PROPERTIES* pProperties = NULL;
	ULONG BufferSize = 0;
	ULONG rc = 0;
	TRACEHANDLE SessionHandle = 0; 
	ULONG uRet; 
	BOOLEAN session_started = FALSE; 
	ULONG stop_old_session_time = 0; 
	BOOLEAN sym_context_inited = FALSE; 

	do 
	{
		ret = init_sym_context(); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		sym_context_inited = TRUE; 

		if( session == NULL )
		{
			break; 
		}

		session->properties = NULL; 
		session->session_handle = 0; 

		BufferSize = sizeof( EVENT_TRACE_PROPERTIES ) + sizeof( LOGSESSION_NAME );
		pProperties = ( EVENT_TRACE_PROPERTIES* )malloc( BufferSize );	
		if( NULL == pProperties )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			log_trace( ( MSG_ERROR, 
				"Unable to allocate %d bytes for properties structure.\n", 
				BufferSize ) );
			
			break; 
		}

		for( ; ; )
		{
			ZeroMemory( pProperties, BufferSize ); 

			pProperties->Wnode.BufferSize = BufferSize;
			pProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
			pProperties->Wnode.ClientContext = 1; //QPC clock resolution
			pProperties->Wnode.Guid = SessionGuid; 

#define EVENT_TRACE_FLAG_SYSTEM_ACTION 0x000000001
#define EVENT_TRACE_FLAG_SYSTEM_ACTION2 0x000000002

			pProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL | EVENT_TRACE_REAL_TIME_MODE;
			pProperties->EnableFlags = EVENT_TRACE_FLAG_SYSTEM_ACTION | EVENT_TRACE_FLAG_SYSTEM_ACTION2;                  // trace enable flags
			pProperties->FlushTimer = 1;
			pProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

			rc = StartTrace( ( PTRACEHANDLE )&SessionHandle, 
				LOGSESSION_NAME, 
				pProperties ); 

			if( ERROR_SUCCESS != rc )
			{
				ASSERT( GetLastError() == rc ); 

				if( rc != ERROR_ALREADY_EXISTS )
				{
					ret = rc; 

					log_trace( ( MSG_ERROR, 
						"StartTrace() failed, %d\n", rc ) ); 
					break; 
				}

				stop_old_session_time ++; 

				if( stop_old_session_time > MAX_STOP_STARTED_SESSION_TIME )
				{
					ret = rc; 

					log_trace( ( MSG_ERROR, 
						"StartTrace() failed, %d\n", rc ) ); 
					break; 
				}

				rc = StopTraceW( NULL, LOGSESSION_NAME, pProperties ); 
				if( rc != ERROR_SUCCESS )
				{
					ret = rc; 
					log_trace( ( MSG_ERROR, 
						"StopTraceW() failed, %d\n", rc ) ); 

					break; 
				}

				continue; 
			}
			else
			{
				break; 
			}
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		session_started = TRUE; 

		uRet = GetTraceEnableFlags( SessionHandle );

		rc = EnableTrace( TRUE, 
			uRet, 
			TRACE_LEVEL_VERBOSE, 
			&ProviderGuid, 
			SessionHandle );

		if( ERROR_SUCCESS != rc )
		{
			ret = rc; 
			log_trace( ( MSG_ERROR, 
				"EnableTrace() failed, %d\n", rc ) ); 
			break; 
		}

		session->properties = pProperties; 
		session->session_handle = SessionHandle; 

	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		if( SessionHandle != 0 )
		{
			if( session_started == TRUE )
			{
				ControlTrace(SessionHandle,LOGSESSION_NAME,pProperties,EVENT_TRACE_CONTROL_STOP);
			}

			CloseTrace(SessionHandle);
		}
		else
		{
			ASSERT( session_started == FALSE ); 
		}

		if( pProperties )
		{
			free( pProperties );
		}

		if( sym_context_inited == TRUE )
		{
			uninit_sym_context(); 
		}
	}

	return ret; 
}

static const GUID WPP_LOCAL_TraceGuids[] = { {0x4cd91029,0x4746,0x0691,{0x66,0xbc,0xef,0x01,0x5e,0x2f,0x6c,0x6c}}, };

void WINAPI process_log(PEVENT_TRACE pEvent); 

/*****************************************************************************
if is working in windows 7, need start etw function. 
*****************************************************************************/
LRESULT start_log_trace()
{
	LRESULT ret = ERROR_SUCCESS; 
	EVENT_TRACE_LOGFILE trace;
	ULONG ulSize = 0;
	ULONG rc = ERROR_SUCCESS;
	TRACEHANDLE handles[1] = { ( TRACEHANDLE )INVALID_HANDLE_VALUE };

	do 
	{
		ZeroMemory(&trace, sizeof(EVENT_TRACE_LOGFILE));
		trace.LoggerName = (LPWSTR)LOGSESSION_NAME; 
		trace.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
		trace.EventCallback = process_log; 

		handles[0] = OpenTrace( &trace ); 
		if( ( TRACEHANDLE )INVALID_HANDLE_VALUE == handles[ 0 ] )
		{
			SAFE_SET_ERROR_CODE( ret ); 

			//Handle error as appropriate for your application.
			log_trace( ( MSG_ERROR, "OpenTrace failed, %d\n", 
				ret ) );
			break; 
		}

		rc = ProcessTrace(handles, 1, 0, 0); 

		if( rc != ERROR_SUCCESS 
			&& rc != ERROR_CANCELLED )
		{
			ret = rc; 
			log_trace( ( MSG_ERROR, "ProcessTrace failed, %d", rc ) ); 
			break; 
		}

		//_getch(); 
	}while( FALSE ); 

	if( handles[0] != ( TRACEHANDLE )INVALID_HANDLE_VALUE )
	{
		rc = CloseTrace( handles[0] );
	}

	return ret; 
}

LRESULT _stop_log_trace( trace_session *session )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		if( session == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( session->session_handle == 0 )
		{
			ASSERT( session->properties != NULL ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = ( LRESULT )ControlTrace( session->session_handle, 
			LOGSESSION_NAME, 
			session->properties, 
			EVENT_TRACE_CONTROL_STOP ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "stop the log trace error %u\n", ret ) ); 
		}

	}while( FALSE );

	return ret; 
}

LRESULT stop_log_trace( trace_session *session )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		_ret = _stop_log_trace( session ); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "stop the log trace session error %u\n", _ret ) ); 
			ret = _ret; 
		}

		_ret = uninit_sym_context(); 

		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "uninitialize the symbolic context error %u\n", _ret ) ); 
			ret = _ret; 
		}
	} while ( FALSE ); 

	return ret; 
}


