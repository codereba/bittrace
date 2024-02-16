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
#if !defined(UNDER_CE)
#include <shellapi.h>
#endif //UNDER_CE

#include <strsafe.h>
#include "stack_dump.h"
#include "analyze_config.h"
#include "bitsafe_common.h"
#include "msg_box.h"
#include "common_config.h"

BOOLEAN config_tip = FALSE; 
analyze_setting event_analyze_help = { 0 }; 

analyze_setting *WINAPI get_analyze_help_conf()
{
	return &event_analyze_help; 
}

ULONG WINAPI get_analyze_ui_flags()
{
	return NO_TIP_DEFAULT_SYMBOL_CHECK; 
}

LRESULT WINAPI load_common_conf_callback()
{
	if( config_tip == TRUE )
	{
		save_common_conf(); 
	}

	return ERROR_SUCCESS; 
}

LRESULT WINAPI set_default_symbol_path()// LPWSTR symbol_path, ULONG cc_buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	WCHAR default_symbol_path[ MAX_PATH ]; 

	do 
	{
#define DEF_STORE_SYMBOL_DIRECTORY_NAME L"symbols"

		ret = add_app_path_to_file_name( default_symbol_path, 
			ARRAYSIZE( default_symbol_path ), 
			DEF_STORE_SYMBOL_DIRECTORY_NAME, 
			FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCbPrintfW( event_analyze_help.symbol_path, 
			sizeof( event_analyze_help.symbol_path ), 
			SYMBOL_PATH_TEMPLATE, 
			default_symbol_path, 
			default_symbol_path ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI set_analyze_help_conf( LPCWSTR symbol_path, LPCWSTR dbg_help_path, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	do 
	{
		if( symbol_path != NULL )
		{
			if( FORMAT_LOCAL_SYMBOL_PATH & flags 
				&& 0 != wcsicmp( symbol_path, L"srv*" ) )
			{
				hr = StringCbPrintfW( event_analyze_help.symbol_path, 
					sizeof( event_analyze_help.symbol_path ), 
					SYMBOL_PATH_TEMPLATE, 
					symbol_path, 
					symbol_path ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			}
			else
			{
				hr = StringCbCopyW( event_analyze_help.symbol_path, sizeof( event_analyze_help.symbol_path ), symbol_path ); 
				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			}
		}
		else
		{
			*event_analyze_help.symbol_path = L'\0'; 
		}

		if( 0 == ( flags & NO_TIP_DEFAULT_SYMBOL_CHECK ) 
			&& *event_analyze_help.symbol_path == L'\0' )
		{
			dlg_ret_state ret_state = CANCEL_STATE; 

			ret = show_msg( NULL, _T( "��ǰû������WINDOWS������Ϣ����,ͨ�����İ���,��������ʾ��ȫ���ϵͳ�¼�ϸ��,�Ƿ��������ڳ�������Ŀ¼�µ�\"SYMBOLS\"��Ŀ¼��? �´�����ѡ������н�������." ), &ret_state ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
			}

			if( ret_state == OK_STATE )
			{
				set_default_symbol_path(); 
			}

			config_tip = TRUE; 
		}

		if( dbg_help_path != NULL )
		{
			hr = StringCbCopyW( event_analyze_help.dbg_help_path, sizeof( event_analyze_help.dbg_help_path ), dbg_help_path ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

	}while( FALSE ); 

	return ret; 
}
