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

#ifndef __ACTION_LOGGER_H__
#define __ACTION_LOGGER_H__

#include "trace_log_api.h"
#include "local_strings.h"

#define MAX_MSG_LEN 512
#define MAX_DATA_DUMP_LEN ( 1024 + 512 )

#define LOG_MSG 0x0000001

LPCWSTR get_port_desc( USHORT port_num ); 

#ifdef SUPPORT_MDL_REMAPPING
LRESULT get_event_msg_ex( sys_action_desc *action, LPWSTR msg, ULONG buf_len, ULONG flags ); 

LRESULT calc_event_msg_len( sys_action_desc *action, ULONG *ret_len ); 
#else

LRESULT get_event_msg_ex( sys_action_output *action, 
						 LPWSTR msg, 
						 ULONG buf_len, 
						 WCHAR *data_dump, 
						 ULONG data_buf_ccb_len, 
						 ULONG flags ); 

LRESULT _get_event_msg_ex( sys_action_desc *action, 
						  BYTE *data, 
						  ULONG data_len, 
						  LPWSTR msg, 
						  ULONG buf_ccb_len, 
						  LPWSTR data_dump, 
						  ULONG data_buf_ccb_len, 
						  ULONG flags ); 

INLINE LRESULT get_whole_event_msg_ex( sys_action_output *action, 
									  LPWSTR msg, 
									  ULONG buf_ccb_len, 
									  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	do
	{

		ret = get_event_msg_ex( action, 
			msg,
			buf_ccb_len, 
			NULL, 
			0, 
			flags ); 

	}while( FALSE ); 


	return ret; 
}


LRESULT calc_event_msg_len( sys_action_output *action, ULONG *ret_len ); 
#endif //SUPPORT_MDL_REMAPPING

LRESULT get_action_notify_tip( sys_action_record *action, action_context *ctx, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT get_action_msg_ex( sys_action_info *action, 
						  PVOID data_buf, 
						  ULONG data_len, 
						  action_response_type resp, 
						  LPWSTR msg, 
						  ULONG buf_ccb_len, 
						  LPWSTR data_dump, 
						  ULONG ccb_data_buf_len, 
						  ULONG flags ); 

LRESULT get_event_msg( sys_action_desc *action, LPWSTR msg, ULONG buf_len, ULONG flags ); 

#define action_info_data_valid( action_info ) ( ( action_info )->data.data != NULL && ( action_info )->data.data_size > 0 ) 

#ifdef SUPPORT_MDL_REMAPPING
INLINE LRESULT dump_event_data( sys_action_desc *action, LPWSTR data_str, ULONG buf_ccb_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 printed_len; 
	LPCWSTR tmp_text; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( data_str != NULL ); 
		ASSERT( buf_ccb_len > 0 ); 

		if( FALSE == action_info_data_valid( action ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			*data_str = L'\0'; 
			break; 
		}

		if( action->data.data == NULL )
		{
			if( action->data.data_len == 0 )
			{
				//ASSERT( FALSE && "why output the data have null buffer and data length is 0" ); 
				ret = ERROR_INVALID_PARAMETER; 
				*data_str = L'\0'; 
				break; 
			}

			tmp_text = _get_string_by_id( TEXT_ACTION_INFO_DATA, L"���ݳ���Ϊ:%u,����:\n" ); 

			if( tmp_text != NULL )
			{
				printed_len = _snwprintf( data_str, 
					buf_ccb_len, 
					tmp_text, 
					action->data.data_len ); 

				if( printed_len < 0 || printed_len + 1 == buf_ccb_len )
				{
					ret = ERROR_BUFFER_OVERFLOW; 
					ASSERT( data_str[ buf_ccb_len - 1 ] != L'\0' ); 
					data_str[ buf_ccb_len - 1 ] = L'\0'; 
					//ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			}
		}
		else
		{
			if( action->data.data_len == 0 )
			{
				ASSERT( FALSE && "why output the data which length is 0" ); 

				ret = ERROR_INVALID_PARAMETER; 
				*data_str = L'\0'; 
				break; 
			}

			tmp_text = _get_string_by_id( TEXT_ACTION_INFO_DATA, L"���ݳ���Ϊ:%u,����:\n" ); 

			if( tmp_text != NULL )
			{
				printed_len = _snwprintf( data_str, 
					buf_ccb_len, 
					tmp_text, 
					action->data.data_len ); 

				if( printed_len < 0 || printed_len + 1 == buf_ccb_len )
				{
					ret = ERROR_BUFFER_OVERFLOW; 
					ASSERT( data_str[ buf_ccb_len - 1 ] != L'\0' ); 
					data_str[ buf_ccb_len - 1 ] = L'\0'; 
					//ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			}

			ASSERT( printed_len <= buf_ccb_len ); 

			ret = dump_mem_in_buf_w( 
				data_str + printed_len, 
				buf_ccb_len - printed_len, 
				( PVOID )action->data.data, 
				action->data.data_len, 
				0 ); 

			if( ret != ERROR_SUCCESS )
			{
				ASSERT( data_str[ buf_ccb_len - 1 ] == L'\0' ); 
				break; 
			}
		}

		if( data_str[ buf_ccb_len - 1 ] != L'\0' )
		{
			data_str[ buf_ccb_len - 1 ] = L'\0'; 

			ret = ERROR_BUFFER_OVERFLOW; 

			break; 
		}

	}while( FALSE );

	return ret; 
}
#else

INLINE LRESULT get_data_desc( ULONG data_len, LPWSTR msg_out, ULONG buf_ccb_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 printed_len; 
	LPCWSTR tmp_text; 

	do
	{
		if( data_len == 0 )
		{
			tmp_text = _get_string_by_id( TEXT_ACTION_INFO_DATA, L" ���ݳ���Ϊ:%u,����:\n" ); 

			if( tmp_text != NULL )
			{
				printed_len = _snwprintf( msg_out, 
					buf_ccb_len, 
					tmp_text, 
					data_len ); 

				if( printed_len < 0 || printed_len + 1 == buf_ccb_len )
				{
					ret = ERROR_BUFFER_OVERFLOW; 
					ASSERT( msg_out[ buf_ccb_len - 1 ] != L'\0' ); 
					msg_out[ buf_ccb_len - 1 ] = L'\0'; 
					break; 
				}
			}
		}
		else
		{
			tmp_text = _get_string_by_id( TEXT_ACTION_INFO_DATA, L"���ݳ���Ϊ:%u,����:\n" ); 

			if( tmp_text != NULL )
			{
				printed_len = _snwprintf( msg_out, 
					buf_ccb_len, 
					tmp_text, 
					data_len ); 

				if( printed_len < 0 || printed_len + 1 == buf_ccb_len )
				{
					ret = ERROR_BUFFER_OVERFLOW; 
					ASSERT( msg_out[ buf_ccb_len - 1 ] != L'\0' ); 
					msg_out[ buf_ccb_len - 1 ] = L'\0'; 
					break; 
				}
			}

			ASSERT( printed_len <= ( INT32 )buf_ccb_len ); 
		}
	}while( FALSE ); 

	return ret; 
}

INLINE LRESULT _dump_event_data( PVOID data, ULONG data_len, USHORT ln_byte, LPWSTR data_str, ULONG buf_ccb_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( data != NULL ); 
		ASSERT( data_str != NULL ); 
		ASSERT( buf_ccb_len > 0 ); 

		*data_str = L'\0'; 
		data_str[ buf_ccb_len - 1 ] = L'\0'; 

		if( data_len > 0 )
		{
			ret = _dump_mem_in_buf_w( data_str, 
				buf_ccb_len, 
				ln_byte, 
				data, 
				data_len, 
				flags ); 

			if( ret != ERROR_SUCCESS )
			{
				ASSERT( data_str[ buf_ccb_len - 1 ] == L'\0' ); 
				break; 
			}
		}

		if( data_str[ buf_ccb_len - 1 ] != L'\0' )
		{
			data_str[ buf_ccb_len - 1 ] = L'\0'; 

			ret = ERROR_BUFFER_OVERFLOW; 

			break; 
		}

	}while( FALSE );

	return ret; 
}

INLINE LRESULT dump_event_data( sys_action_output *action, LPWSTR data_str, ULONG buf_ccb_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG data_len; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( data_str != NULL ); 
		ASSERT( buf_ccb_len > 0 ); 

		*data_str = L'\0'; 

		ret = is_valid_action_output( action ); 

		if( ret != ERROR_SUCCESS )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		data_len = get_action_output_data_size( action ); 

		ret = _dump_event_data( action->data, data_len, DEF_DUMP_LINE_BYTE_COUNT, data_str, buf_ccb_len, flags ); 
	}while( FALSE );

	return ret; 
}
#endif //SUPPORT_MDL_REMAPPING

#ifndef NO_NEED_LOGGER_INTERFACE
class action_logger
{
public:
	action_logger()
	{}

	LRESULT output_logs( sys_log_output* msgs_cap, ULONG length ); 

}; 
#endif //NO_NEED_LOGGER_INTERFACE

#endif //__ACTION_LOGGER_H__