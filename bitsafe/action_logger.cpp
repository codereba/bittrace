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

#include "common_func.h"
#include "ring0_2_ring3.h"
#include "action_logger.h"
#include "volume_name_map.h"
#include "mem_region_list.h"
#include "action_display.h"
#include "ui_ctrl.h"

#ifndef NO_NEED_LOGGER_INTERFACE
#include "conf_file.h"
#endif //NO_NEED_LOGGER_INTERFACE

LPCWSTR get_port_desc( USHORT port_num )
{
	LPCWSTR desc = L""; 

#define POP3_PORT_DESC L"[POP3�ʼ�����]"
#define HTTP_PORT_DESC L"[WEB����]"
#define FTP_PORT_DESC L"[�ļ��������]"
#define SAMBA_PORT_DESC L"[WINDOWS��������]"
#define HTTP_AGANT_PORT_DESC L"[WEB��������]"
	switch( port_num )
	{
	case 110:
		desc = POP3_PORT_DESC; 
		break; 
	case 21:
		desc = FTP_PORT_DESC; 
		break; 
	case 80:
		desc = HTTP_PORT_DESC; 
		break; 
	case 8080:
		desc = HTTP_AGANT_PORT_DESC; 
		break; 
	case 139:
		desc = SAMBA_PORT_DESC; 
		break; 
	default:
		break; 
	}

	return desc; 
}

LRESULT search_param_meaning( param_all_desc *param, LPWSTR meaning, ULONG ccb_len, ULONG *ret_ccb_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( param != NULL ); 
	ASSERT( meaning != NULL ); 

	*meaning = L'\0'; 

	if( ret_ccb_len != NULL )
	{
		*ret_ccb_len = 0; 
	}

	return ret; 
}

LRESULT action_desc_param_process( sys_action_desc *action )
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}

#pragma comment( lib, "ntdll.lib" )

typedef ULONG ( *ntstatus_to_dos_error_func )( NTSTATUS Status );
LRESULT load_ntdll_func()
{
	LRESULT ret = ERROR_SUCCESS; 
	HMODULE handle; // = NULL; 
	ntstatus_to_dos_error_func *ntstatus_to_dos_error = NULL; 

	do 
	{
		handle = GetModuleHandle( _T( "ntdll.dll" ) ); 
		if( handle == NULL )
		{
			break; 
		}

		ntstatus_to_dos_error = ( ntstatus_to_dos_error_func* )GetProcAddress( handle, "RtlNtStatusToDosError" ); 

	} while ( FALSE );

	return ret; 
}

extern "C" ULONG NTAPI RtlNtStatusToDosError( NTSTATUS Status	);

#define IPV4_ADDR_LEN 32
#define SOCKET_PARAM_NUM 8 
#define FILE_ACTION_PARAM_NUM 1

#ifdef SUPPORT_MDL_REMAPPING
LRESULT calc_event_msg_len( sys_action_desc *action, ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG need_len = MAX_MSG_LEN; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( ret_len != NULL ); 

		if( action->data.data == NULL && action->data.data_len != 0 )
		{
			ASSERT( FALSE && "the data infomation of the action is invalid\n" ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
		else if( action->data.data != NULL && action->data.data_len == 0 )
		{
			ASSERT( FALSE && "the data infomation of the action is invalid\n" ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
		else if( action->data.data == NULL && action->data.data_len == 0 )
		{
			break; 
		}

		need_len += action->data.data_len; 

	}while( FALSE );

	*ret_len = need_len; 

	return ret; 
}
#else
LRESULT calc_event_msg_len( sys_action_output *action, ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG need_len = MAX_MSG_LEN; 

	need_len += get_action_output_data_size( action ); 

	*ret_len = need_len; 

	return ret; 
}

#endif //SUPPORT_MDL_REMAPPING

extern "C" void *          __cdecl _alloca(size_t);

LRESULT get_event_msg( sys_action_desc *action, LPWSTR msg, ULONG buf_ccb_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR fmt = NULL; 

#define MAX_ACTION_DESC_SIZE ( ULONG )( 64 )

#ifndef _DEBUG_MEM_LEAKS
	TCHAR action_desc[ MAX_ACTION_DESC_SIZE ]; 
#else
	LPTSTR action_desc; 
#endif //_DEBUG_MEM_LEAKS

	LPCTSTR tmp_text; 
	LPCTSTR tmp_fmt; 
#define MAX_PARAMS_NUM ( ULONG )( 16 )
	TCHAR *app_name = NULL; 
	TCHAR *meanings[ MAX_PARAMS_NUM ] = { 0 }; 
	INT32 i; 
	ULONG app_name_len; 
	ULONG _app_name_len; 

	ASSERT( action != NULL ); 
	ASSERT( msg != NULL ); 
	ASSERT( buf_ccb_len > 0 ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

	*msg = L'\0'; 

	app_name = ( TCHAR* )malloc( sizeof ( TCHAR ) * MAX_PATH ); 

	if( app_name == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_app_name_len = wcslen( action->desc.common.app.app.app_name ); 

	ret = convert_native_path_to_dos( action->desc.common.app.app.app_name, 
		_app_name_len, 
		app_name, 
		MAX_PATH, 
		&app_name_len ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->desc.common.app.app.app_name, ret ) ); 

		if( _app_name_len > MAX_PATH - 1 )
		{
			_app_name_len = MAX_PATH - 1; 
		}

		memcpy( app_name, 
			action->desc.common.app.app.app_name, 
			_app_name_len << 1 ); 

		app_name[ _app_name_len ] = L'\0'; 
	}

#define EVENT_REQUEST_TIP _T( " �Ƿ�����?" )

	tmp_text = _get_string_by_id( TEXT_SYS_ACTION_ALLOW_TIP, 
		EVENT_REQUEST_TIP ); 

	if( flags == LOG_MSG )
	{
#ifdef _DEBUG_MEM_LEAKS
		action_desc = ( LPTSTR )_alloca( MAX_ACTION_DESC_SIZE * sizeof( TCHAR ) ); 
		if( action_desc == NULL )
		{
			goto _return; 
		}
#endif //_DEBUG_MEM_LEAKS

		_sntprintf( action_desc, MAX_ACTION_DESC_SIZE, _T( " %s" ), action->resp == ACTION_BLOCK ? _T( "��ֹ" ) : _T( "����" ) ); 

	}
	else
	{
		log_trace( ( MSG_ERROR, "invalid system action type %d \n", action->type ) ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( fmt != NULL )
	{
		ULONG ccb_len; 

		_snwprintf( msg, buf_ccb_len, fmt, 
			app_name, 
			action->desc.common.param0.common.name ); 

		search_param_meaning( ( param_all_desc* )&action->desc.common.app, meanings[ 0 ], _MAX_DESC_LEN, &ccb_len ); 
		search_param_meaning( ( param_all_desc* )&action->desc.common.param0, meanings[ 1 ], _MAX_DESC_LEN, &ccb_len ); 
	}

	if( flags == LOG_MSG )
	{
		_tcsncat( msg, action_desc, buf_ccb_len - wcslen( msg ) ); 
	}
	else
	{
		_tcsncat( msg, tmp_text, buf_ccb_len - wcslen( msg ) ); 
	}

	if( msg[ buf_ccb_len - 1 ] != L'\0' )
	{
		msg[ buf_ccb_len - 1 ] = L'\0'; 
	}

_return:
	for( i = 0; i < ARRAYSIZE( meanings ); i ++ )
	{
		if( meanings[ i ] != NULL )
		{
			free( meanings[ i ] ) ; 
		}
	}

	if( app_name != NULL )
	{
		free( app_name ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
	return ret; 
}

LRESULT get_action_notify_tip( sys_action_record *action, action_context *ctx, action_response_type resp, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR fmt = NULL; 

#define MAX_ACTION_DESC_SIZE ( ULONG )( 64 )

#ifndef _DEBUG_MEM_LEAKS
	TCHAR action_desc[ MAX_ACTION_DESC_SIZE ]; 
#else
	LPTSTR action_desc; 
#endif //_DEBUG_MEM_LEAKS

	LPCTSTR tmp_text; 

	ASSERT( action != NULL ); 
	ASSERT( tip != NULL ); 
	ASSERT( ccb_buf_len > 0 ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

	*tip = L'\0'; 
	tip[ ccb_buf_len - 1 ] = L'\0';

#define EVENT_REQUEST_TIP _T( " �Ƿ�����?" )

	tmp_text = _get_string_by_id( TEXT_SYS_ACTION_ALLOW_TIP, 
		EVENT_REQUEST_TIP ); 

	if( flags == LOG_MSG )
	{
#ifdef _DEBUG_MEM_LEAKS
		action_desc = ( LPTSTR )_alloca( MAX_ACTION_DESC_SIZE * sizeof( TCHAR ) ); 
		if( action_desc == NULL )
		{
			goto _return; 
		}
#endif //_DEBUG_MEM_LEAKS

		_sntprintf( action_desc, MAX_ACTION_DESC_SIZE, _T( " %s" ), resp == ACTION_BLOCK ? _T( "��ֹ" ) : _T( "����" ) ); 

	}

	ret = get_action_tip( action, ctx, tip, ccb_buf_len, flags ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( flags == LOG_MSG )
	{
		_tcsncat( tip, action_desc, ccb_buf_len - wcslen( tip ) ); 
	}
	else
	{
		_tcsncat( tip, tmp_text, ccb_buf_len - wcslen( tip ) ); 
	}

	if( tip[ ccb_buf_len - 1 ] != L'\0' )
	{
		tip[ ccb_buf_len - 1 ] = L'\0'; 

		ret = ERROR_BUFFER_OVERFLOW; 
		
		goto _return; 
	}

_return: 
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
	return ret; 
}

LRESULT get_action_msg_ex( sys_action_info *action, 
						  PVOID data_buf, 
						  ULONG data_len, 
						  action_response_type resp, 
						  LPWSTR msg, 
						  ULONG buf_ccb_len, 
						  LPWSTR data_dump, 
						  ULONG ccb_data_buf_len, 
						  ULONG flags ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG outputed_len; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != msg ); 
		ASSERT( 0 < buf_ccb_len ); 

		ret = get_action_notify_tip( &action->action, &action->ctx, resp, msg, buf_ccb_len, flags ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		outputed_len = wcslen( msg ); 

		if( data_len == 0 )
		{
			*data_dump = L'\0'; 
			break; 
		}

		ASSERT( data_buf != NULL ); 

		ret = get_data_desc( data_len, msg + outputed_len, buf_ccb_len - outputed_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		outputed_len = wcslen( msg ); 

		ret = _dump_event_data( data_buf, 
			data_len, 
			DEF_DUMP_LINE_BYTE_COUNT, 
			data_dump, 
			ccb_data_buf_len, 
			flags ); 

#ifdef DEBUG
		if( ret == ERROR_BUFFER_OVERFLOW )
		{
			dbg_print( MSG_ERROR, "dumping the data of the action is overflow %u\n", data_len ); 
			ASSERT( msg[ buf_ccb_len - 1 ] == L'\0' ); 
		}
#endif //DEBUG 
	}while( FALSE );

	return ret; 
}

#ifdef SUPPORT_MDL_REMAPPING
LRESULT get_event_msg_ex( sys_action_desc *action, LPWSTR msg, ULONG buf_ccb_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 
	ULONG outputed_len; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != msg ); 
		ASSERT( 0 < buf_ccb_len ); 

		ret = get_event_msg( action, msg, buf_ccb_len, flags ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		//ntstatus = RtlStringCbCatNW( msg, 
		//	buf_ccb_len << 1, 
		//	L"\nthe data of action ", 
		//	( buf_ccb_len - wcslen( msg ) ) << 1 ); 

		//if( ntstatus != STATUS_SUCCESS )
		//{
		//	ret = RtlNtStatusToDosError( ntstatus ); 
		//	//ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		outputed_len = wcslen( msg ); 

		ret = get_data_desc( data_len, msg + outputed_len, buf_ccb_len - outputed_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		outputed_len = wcslen( msg ); 
		ret = dump_event_data( action, msg + outputed_len, buf_ccb_len - outputed_len, flags ); 

#ifdef DEBUG
		if( ret == ERROR_BUFFER_OVERFLOW )
		{
			dbg_print( MSG_ERROR, "dumping the data of the action is overflow %u\n", action->data.data_len ); 
			ASSERT( msg[ buf_ccb_len - 1 ] == L'\0' ); 
		}
#endif //DEBUG
	}while( FALSE );

	return ret; 
}
#else
LRESULT get_event_msg_ex( sys_action_output *action, 
						 LPWSTR msg, 
						 ULONG buf_ccb_len, 
						 LPWSTR data_dump, 
						 ULONG data_buf_ccb_len, 
						 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG data_len; 
	ULONG outputed_len; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != msg ); 
		ASSERT( 0 < buf_ccb_len ); 

		ret = get_event_msg( &action->action, msg, buf_ccb_len, flags ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		outputed_len = wcslen( msg ); 

		data_len = get_action_output_data_size( action ); 

		ret = get_data_desc( data_len, 
			msg + outputed_len, 
			buf_ccb_len - outputed_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( data_dump != NULL 
			&& data_buf_ccb_len > 0 )
		{
			ret = dump_event_data( action, data_dump/*msg + outputed_len*/, data_buf_ccb_len/*buf_ccb_len - outputed_len*/, flags ); 
		}
		else
		{
			outputed_len = wcslen( msg ); 

			ASSERT( outputed_len <= buf_ccb_len ); 

			if( buf_ccb_len > outputed_len + 1 )
			{
				ret = dump_event_data( action, msg + outputed_len, buf_ccb_len - outputed_len, flags ); 
			}
		}

#ifdef DEBUG
		if( ret == ERROR_BUFFER_OVERFLOW )
		{
			dbg_print( MSG_ERROR, "dumping the data of the action is overflow %u\n", data_len ); 
			ASSERT( msg[ buf_ccb_len - 1 ] == L'\0' ); 
		}
#endif //DEBUG
	}while( FALSE );

	return ret; 
}

LRESULT _get_event_msg_ex( sys_action_desc *action, 
						  BYTE *data, 
						  ULONG data_len, 
						  LPWSTR msg, 
						  ULONG buf_ccb_len, 
						  LPWSTR data_dump, 
						  ULONG data_buf_ccb_len, 
						  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG msg_len; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != msg ); 
		ASSERT( 0 < buf_ccb_len ); 

		ret = get_event_msg( action, msg, buf_ccb_len, flags ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		msg_len = wcslen( msg ); 

		ret = get_data_desc( data_len, msg + msg_len, buf_ccb_len - msg_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( data_dump != NULL 
			&& data_buf_ccb_len > 0 )
		{
			ret = _dump_event_data( data, data_len, DEF_DUMP_LINE_BYTE_COUNT, data_dump, data_buf_ccb_len, flags ); 
		}
		else
		{
			ret = _dump_event_data( data, data_len, DEF_DUMP_LINE_BYTE_COUNT, msg + wcslen( msg ), buf_ccb_len - wcslen( msg ), flags ); 
		}

#ifdef _DEBUG
		if( ret == ERROR_BUFFER_OVERFLOW )
		{
			dbg_print( MSG_ERROR, "dumping the data of the action is overflow %u\n", data_len ); 
			ASSERT( msg[ buf_ccb_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG
	}while( FALSE );

	return ret; 
}

#endif //SUPPORT_MDL_REMAPPING

#ifndef NO_NEED_LOGGER_INTERFACE
LRESULT action_logger::output_logs( sys_log_output* msgs_cap, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 

#ifndef _UNICODE
	TCHAR unicode[ MAX_MSG_LEN ]; 
	ULONG need_len; 
#endif
	WCHAR log_msg[ MAX_MSG_LEN ]; 
	INT32 i; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

	for( i = 0; ( ULONG )i < msgs_cap->size; i ++ )
	{
		if( msgs_cap->trace_logs[ i ].action_log.magic != SYS_ACTION_OUTPUT_MAGIC )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			continue; 
		}
		else if( msgs_cap->trace_logs[ i ].action_log.size < sizeof( sys_action_output ) 
			|| msgs_cap->trace_logs[ i ].action_log.size > sizeof( sys_log_unit ) )
		{
			ASSERT( FALSE ); 

			ret = ERROR_INVALID_PARAMETER; 
			continue; 
		}
#ifdef _DEBUG
		{
			WCHAR *data_dump = NULL; 
			do
			{
				data_dump = ( WCHAR* )malloc( MAX_DATA_DUMP_LEN * sizeof( WCHAR ) ); 
				if( data_dump == NULL )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}

				ret = get_event_msg_ex( &msgs_cap->trace_logs[ i ].action_log, 
					log_msg, 
					MAX_MSG_LEN, 
					data_dump, 
					MAX_DATA_DUMP_LEN, 
					LOG_MSG ); 

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE && "invalid sys action log" ); 
					log_trace( ( MSG_INFO, "invalid sys action log\n" ) ); ; 
				}

				log_trace( ( MSG_INFO, "get message info %ws data %ws\n", 
					log_msg, 
					data_dump ) ); 
			}while( FALSE ); 

			if( data_dump != NULL )
			{
				free( data_dump ); 
			}
		}
#endif //_DEBUG

		ret = add_log( &msgs_cap->trace_logs[ i ].action_log.action ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "add sys action log failed" ); 
			log_trace( ( MSG_INFO, "add sys action log failed 0x%0.8x\n", ret ) ); ; 
		}
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
	return 0; 
}

#endif //NO_NEED_LOGGER_INTERFACE