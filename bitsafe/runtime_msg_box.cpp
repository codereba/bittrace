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

#include "stdafx.h"
#include "ui_ctrl.h"
#include "runtime_msg_box.h"
#include "bitsafe_conf.h"
#include "string_mng.h"
#include "memory_mng.h"

#define SHOW_SLOGAN_MSG

#define BEGIN_SLOGAN_WAIT_TIME 3000
#define CHECK_SLOGAN_WAIT_TIME_INC 1000
#define MAX_SLOGAN_WAIT_TIME 8000

ULONG slogan_wait_time = BEGIN_SLOGAN_WAIT_TIME; 
static ULONG msg_index = 0; 
#define LOCAL_SLOGAN_COUNT 5
#define BITSAFE_SLOGAN_URL_FMT "http://www.simpai.net/slogan/slogan.php?slogan=%u"

typedef struct _builtin_slogans
{
	LPCWSTR file_name; 
} builtin_slogans, *pbuiltin_slogans; 

builtin_slogans all_builtin_slogans[] = { { L"slogan1" }, { L"slogan2" } }; 
#ifndef NO_UILIB_SUPP
INLINE LRESULT get_slogan_path( CStdString &slogan_path )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		slogan_path = CPaintManagerUI::GetInstancePath(); 

		if( TRUE == slogan_path.IsEmpty() )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		slogan_path.Append( _T( "slogans" ) ); 
	}while( FALSE );

	return ret; 
}
#endif //NO_UILIB_SUPP

LRESULT safe_output_slogans()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	CStdString slogan_root_path; 
	CStdString slogan_file_name;

	ret = get_slogan_path( slogan_root_path ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = safe_create_dir( CPaintManagerUI::GetInstancePath().GetData(), slogan_root_path.GetData() ); 
	if( ret != ERROR_SUCCESS )
	{
		if( ret != ERROR_ALREADY_EXISTS )
		{
			ASSERT( FALSE && "create themes root directory error" ); 
			goto _return; 
		}
	}

	slogan_root_path.Append( _T( "\\" ) ); 
	//ret = safe_create_dir_fram_res( _T( "themes" ) ); 

	for( i = 0; i < ARRAYSIZE( all_builtin_slogans ); i ++ )
	{
		slogan_file_name = slogan_root_path; 
		slogan_file_name.Append( all_builtin_slogans[ i ].file_name ); 

		ret = _create_file_from_res_zip( all_builtin_slogans[ i ].file_name, NULL, slogan_file_name.GetData(), FILE_IS_WHOLE_PATH ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "output the builtin slogan %ws file error 0x%0.8x\n", slogan_file_name.GetData(), ret ) ); 
			ASSERT( FALSE && "output the builtin slogan file error" ); 
			//goto _return; 
		}
	}

_return:
	return ret; 
}

#define BITSAFE_SLOGAN_TITLE "*bitsafeslogan*:"
#define BITSAFE_SLOGAN_MUST_SHOW_MARK ":*mustshow*"

typedef struct _const_str_info 
{
	ULONG str_len; 
	CHAR *str; 
}const_str_info, *pconst_str_info; 

const_str_info all_slogan_mark[] = { { CONST_STR_LEN( BITSAFE_SLOGAN_TITLE ), BITSAFE_SLOGAN_TITLE }, 
							{ CONST_STR_LEN( BITSAFE_SLOGAN_MUST_SHOW_MARK ), BITSAFE_SLOGAN_MUST_SHOW_MARK } }; 

LRESULT remove_bitsafe_slogan_sign( LPSTR msg_buf, ULONG buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPSTR mark_ptr; 
	ULONG remain_len; 

	ASSERT( msg_buf != NULL ); 
	do 
	{
		if( msg_buf[ buf_len -1 ] != 0 )
		{
			msg_buf[ buf_len -1 ] = 0; 
		}

		if( flags >= ARRAY_SIZE( all_slogan_mark ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		mark_ptr = strstr( msg_buf, all_slogan_mark[ flags ].str ); 

		if( mark_ptr == NULL )
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}

		remain_len = strlen( mark_ptr ); 

		memmove( mark_ptr, mark_ptr + all_slogan_mark[ flags ].str_len, remain_len + 1 ); 

	} while ( FALSE ); 

	return ret; 
}

LRESULT recv_run_time_msg( WCHAR *msg_buf, ULONG buf_len, ULONG *ret_len, HANDLE notifier, BOOLEAN *is_must_show )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	CHAR slogan_url[ MAX_PATH ]; 
	ULONG data_readed; 
	LPCWSTR builting_slogan; 
	ULONG builting_slogan_id; 
	ULONG wait_ret; 
	LPWSTR unicode_str = NULL; 
	BOOLEAN must_show = FALSE; 

	ASSERT( ret_len != NULL ); 
	ASSERT( msg_buf != NULL ); 
	ASSERT( buf_len > 0 ); 
	ASSERT( notifier != NULL ); 

	*ret_len = 0; 

	do 
	{
		if( msg_index < LOCAL_SLOGAN_COUNT )
		{

			switch( msg_index )
			{
			case 0:
				builting_slogan_id = TEXT_SLOGAN_SECURITY_SOFT_COMPATIBLE; 
				break; 
			case 1:
				builting_slogan_id = TEXT_BITSAFE_SLOGAN; 
				break; 
			default: 
				ret = ERROR_NOT_READY; 
				//builting_slogan_id = 0; 
				break; 
			}

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			builting_slogan = get_string_by_id( ( ALL_LANG_STRINGS_ID )builting_slogan_id ); 
			ASSERT( NULL != builting_slogan ); 
		
			if( wcslen( builting_slogan ) > ( buf_len >> 1 ) - 1 )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
			
				break; 
			}
			
			wcsncpy( msg_buf, builting_slogan, ( buf_len >> 1 ) ); 
			*ret_len  = ( ULONG )( wcslen( builting_slogan ) + 1 )<< 1; 

			ASSERT( msg_buf[ *ret_len ] == 0 ); 
		}
		else
		{
			_ret = _snprintf( slogan_url, MAX_PATH, BITSAFE_SLOGAN_URL_FMT, msg_index ); 
			if( _ret < 0 )
			{
				DBG_BP(); 
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			for( ; ; )
			{
				ret = recv_data_from_internet( slogan_url, msg_buf, buf_len, &data_readed, NULL, NULL ); 
				if( ERROR_SUCCESS == ret )
				{
					ULONG unicode_len; 
					//Sleep( CHECK_SLOGAN_WAIT_TIME ); 

					if( msg_buf[ data_readed - 1 ] != 0 )
					{
						msg_buf[ data_readed - 1 ] = 0; 
					}

					ret = remove_bitsafe_slogan_sign( ( LPSTR )msg_buf, data_readed, 0 ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					_ret = remove_bitsafe_slogan_sign( ( LPSTR )msg_buf, data_readed, 1 ); 

					if( _ret != ERROR_SUCCESS )
					{
						must_show = FALSE; 
					}
					else
					{
						must_show = TRUE; 
					}

					if( FALSE == get_slogan_show() )
					{
						if( must_show == FALSE )
						{
							ret = ERROR_NOT_FOUND; 
							break; 
						}
					}

					{

						unicode_str = alloc_char_to_tchar( ( CHAR* )msg_buf ); 
						if( NULL == unicode_str )
						{
							ret = ERROR_NOT_ENOUGH_MEMORY; 
							DBG_BP(); 
							break; 
						}

						unicode_len = wcslen( unicode_str ); 
						data_readed = ( unicode_len + 1 )<< 1; 
						
						if( data_readed > buf_len )
						{
							memcpy( msg_buf, unicode_str, buf_len ); 
							msg_buf[ ( buf_len / 2 ) - 1 ] = 0; 
						}
						else
						{
							memcpy( msg_buf, unicode_str, data_readed ); 
							msg_buf[ ( data_readed / 2 ) - 1 ] = 0; 
						}

						if( unicode_str != NULL )
						{
							mem_free( ( VOID** )&unicode_str ); 
							unicode_str = NULL; 
						}
					}


					break; 
				}


				wait_ret = wait_event( notifier, slogan_wait_time ); 
				if( wait_ret != WAIT_TIMEOUT 
					&& wait_ret != WAIT_OBJECT_0 
					&& wait_ret != WAIT_ABANDONED )
				{
					log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait syncronous event failed will exit\n" ) ); 
					break; 
				}
			}

			//ASSERT( ERROR_SUCCESS == ret ); 

			if( ERROR_SUCCESS == ret )
			{
				if( data_readed > buf_len )
				{
					ASSERT( FALSE && "how can readed the data to the buffer that size is overflowed\n" ); 
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				*ret_len = data_readed; 
				break; 
			}
		}
	} while ( FALSE ); 

	if( ret == ERROR_SUCCESS 
		|| 	msg_index < LOCAL_SLOGAN_COUNT )
	{
		msg_index ++; 
	}

	if( is_must_show != NULL )
	{
		*is_must_show = must_show; 
	}

	if( unicode_str != NULL )
	{
		mem_free( ( VOID** )&unicode_str ); 
		unicode_str = NULL; 
	}

	return ret; 
}

#define MAX_SLOGAN_MSG_LEN 2048
ULONG CALLBACK thread_show_slogan( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	action_ui_notify *ui_notify; 
	DWORD wait_ret; 
	WCHAR *slogan_msg; // = NULL; 
	ULONG data_readed; 
	ULONG end_ch_index; 
	thread_manage *thread_param; 
	BOOLEAN must_show; 

	ASSERT( param != NULL ); 

	thread_param = ( thread_manage* )param; 

	ASSERT( thread_param->param != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ui_notify = ( action_ui_notify* )thread_param->param; 

	slogan_msg = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_SLOGAN_MSG_LEN ); 
	if( slogan_msg == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	slogan_wait_time = BEGIN_SLOGAN_WAIT_TIME;  
	for( ; ; )
	{
		wait_ret = wait_event( thread_param->notify, slogan_wait_time ); 
		if( wait_ret != WAIT_TIMEOUT 
			&& wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait syncronous event failed will exit\n" ) ); 
			break; 
		}

		if( thread_param->stop_run == TRUE )
		{
			break; 
		}

		ret = recv_run_time_msg( slogan_msg, MAX_SLOGAN_MSG_LEN * sizeof( WCHAR ), &data_readed, thread_param->notify, &must_show ); 
		if( ret != ERROR_SUCCESS )
		{
			if( slogan_wait_time < MAX_SLOGAN_WAIT_TIME )
			{
				slogan_wait_time += CHECK_SLOGAN_WAIT_TIME_INC; 
			}
			goto _continue; 
		}

		end_ch_index = data_readed / sizeof( WCHAR ); 

		if( slogan_msg[ data_readed / sizeof( WCHAR ) ] != L'\0' )
		{
			if( data_readed / sizeof( WCHAR ) + 1 < MAX_SLOGAN_MSG_LEN )
			{
				slogan_msg[ data_readed / sizeof( WCHAR ) ] = L'\0'; 
			}
			else
			{
				slogan_msg[ data_readed / sizeof( WCHAR ) - 1 ] = L'\0'; 
			}
		}

		if( ret == ERROR_SUCCESS )
		{
			notify_data data_notify; 

			data_notify.data = ( PVOID )slogan_msg; 
			data_notify.data_len = data_readed; 
			ui_notify->action_notify( SLOGAN_NOTIFY, &data_notify ); 
		}

_continue:
		log_trace( ( MSG_INFO, "%s get blcok count loop\n", __FUNCTION__ ) ); 
	}

_return:
	if( slogan_msg != NULL )
	{
		free( slogan_msg ); 
	}

	return ( ULONG )ret; 
}

typedef enum _msg_thread_type
{
	OUTPUT_SLOGAN_THREAD, 
	MAX_MSG_THREAD_TYPE
} work_thread_type; 

thread_manage msg_thread_manage[ 1 ] = { 0 }; 

LRESULT start_slogan_display( action_ui_notify *ui_notifer )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( FALSE == get_slogan_show() )
		{ 
			log_trace( ( MSG_INFO, "slogan is not shwo by ui setting\n" ) ); 
			msg_index = LOCAL_SLOGAN_COUNT; 
		}

		if( NULL == ui_notifer )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

#ifdef SHOW_SLOGAN_MSG
		ret = create_work_thread( &msg_thread_manage[ OUTPUT_SLOGAN_THREAD ], thread_show_slogan, NULL, ui_notifer ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create block count thread failed \n" ) ); 
		}
#endif //SHOW_SLOGAN_MSG
	}while( FALSE );

	return ret; 
}

LRESULT stop_slogan_display()
{
	LRESULT ret = ERROR_SUCCESS; 
#ifdef SHOW_SLOGAN_MSG
	ret = stop_work_thread( &msg_thread_manage[ OUTPUT_SLOGAN_THREAD ] ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "create block count thread failed \n" ) ); 
	}
#endif //SHOW_SLOGAN_MSG
	return ret; 
}