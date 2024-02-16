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
 
#include "StdAfx.h"
#include "common_func.h"
#include "bitsafe_common.h"
#include "user_manage.h"
#include "chg_pwd_dlg.h"
#include "login_dlg.h"

//#define JUST_DEBUG
#ifdef JUST_DEBUG
user_group cur_user_group = ADMIN; 
#else
user_group cur_user_group = USER; 
#endif //JUST_DEBUG

user_group get_cur_user_group()
{
	return cur_user_group; 
}

void set_cur_user_group( user_group group )
{
	cur_user_group = group; 
}

LRESULT get_user_conf_file_name( WCHAR *conf_file_path, ULONG name_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	ASSERT( conf_file_path != NULL ); 
	ASSERT( name_len > 0 ); 

	_ret = GetSystemDirectory( conf_file_path, name_len ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	wcsncat( conf_file_path, L"\\", name_len - wcslen( conf_file_path ) );

	//if( conf_file_path[ name_len - 1 ] != L'\0' )
	//{
	//	//ASSERT( FALSE ); 
	//	ret = ERROR_BUFFER_TOO_SMALL; 
	//	goto _return; 
	//}

	wcsncat( conf_file_path, BITSAFE_USER_CONF_FILE, name_len - wcslen( conf_file_path ) );

	if( conf_file_path[ name_len - 1 ] != L'\0' )
	{
		conf_file_path[ name_len - 1 ] = L'\0'; 
		//ASSERT( FALSE ); 
		//ret = ERROR_BUFFER_TOO_SMALL; 
		//goto _return; 
	}

	log_trace( ( MSG_INFO, "user configure file name is %ws\n", conf_file_path ) ); 

_return:
	return ret; 
}

LRESULT get_cur_pwd( PBYTE cur_pwd, ULONG buf_len )
{
	BOOL _ret; 
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR conf_file_path[ MAX_PATH ]; 

	ret = read_file( 0, cur_pwd, ARRAY_SIZE( cur_pwd ), conf_file_path ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	return ret; 
}

#define PWD_SAVE_TIME 5
#define PWD_SAVE_POS_SPAN 10
LRESULT write_pwd( PBYTE pwd, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR conf_file_path[ MAX_PATH ]; 
	INT32 i = 0; 

	ASSERT( buf_len == PWD_DATA_LEN ); 

	ret = get_user_conf_file_name( conf_file_path, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	for( i = 0; i < PWD_SAVE_TIME; i ++ )
	{
		ret = write_file( i *( PWD_SAVE_POS_SPAN + buf_len ), pwd, buf_len, conf_file_path ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

_return:
	return ret; 
}

LRESULT check_pwd_file_consist( BYTE pwd_out[ PWD_DATA_LEN ], ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR conf_file_path[ MAX_PATH ]; 
	INT32 i = 0; 
	BYTE first_pwd[ PWD_DATA_LEN ]; 
	BYTE pwd[ PWD_DATA_LEN ]; 

	memset( pwd_out, 0, PWD_DATA_LEN ); 

	ret = get_user_conf_file_name( conf_file_path, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = read_file( 0, first_pwd, PWD_DATA_LEN, conf_file_path ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	for( i = 1; i < PWD_SAVE_TIME; i ++ )
	{
		ret = read_file( i * ( PWD_SAVE_POS_SPAN + PWD_DATA_LEN ), pwd, PWD_DATA_LEN, conf_file_path ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		if( memcmp( pwd, first_pwd, sizeof( pwd ) ) != 0 )
		{
			ret = ERROR_INVALID_PASSWORD; 
			goto _return; 
		}
	}

	memcpy( pwd_out, first_pwd, PWD_DATA_LEN ); 

_return:
	return ret; 
}

BYTE none_pwd_def[] = { 0x8B, 0x40, 0x14, 0x3D, 
						0x20, 0x05, 0x93, 0x19, 
						0x0E, 0x3D, 0x22, 0x05, 
						0x93, 0x19, 0x74, 0x07, 
						0xBE, 0xFE, 0xFF, 0xFF, 
						0x33, 0xC0, 0xC2, 0x04, 
						0x2C, 0x82, 0x40, 0x00, 
						0x33, 0xC0, 0xC3, 0xCC, 
						0xB8, 0x60, 0xA0, 0x40, 
						0x00, 0xBF, 0x60, 0xA0, 
						0x8B, 0x06, 0x85, 0xC0, 
						0x74, 0x02, 0xFF, 0xD0, 
						0x5E, 0xC3, 0x56, 0x57, 
						0xB8, 0x68, 0xA0, 0x40, 
						0x8B, 0xF0, 0x73, 0x0F, 
						0x8B, 0x06, 0x85, 0xC0, 
						0xF7, 0x72, 0xF1, 0x5F, 
						0x5E, 0xC3, 0xFF, 0x25 }; 

LRESULT clear_password()
{
	BOOL _ret; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR conf_file_path[ MAX_PATH ]; 
	LARGE_INTEGER offset; 
	LARGE_INTEGER prev_offset; 
	INT32 i = 0; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ret = get_user_conf_file_name( conf_file_path, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

//#if 0
	file_handle = CreateFile( conf_file_path, 
		GENERIC_ALL, 
		0, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_HIDDEN, 
		NULL ); 

	if( file_handle == INVALID_HANDLE_VALUE )
	{
		SAFE_SET_ERROR_CODE( ret ); 
		goto _return; 
	}
	
	offset.QuadPart = 0; 

	_ret = SetFilePointerEx( file_handle, offset, &prev_offset, SEEK_SET ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "set file pointer failed \n" ) ); 
		goto _return; 
	}

	_ret = SetEndOfFile( file_handle ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "set file end failed \n" ) ); 
		goto _return; 
	}
//#endif 

	CloseHandle( file_handle ); 
	file_handle = INVALID_HANDLE_VALUE; 

	ret = write_file( 0, none_pwd_def, sizeof( none_pwd_def ), conf_file_path ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	if( file_handle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( file_handle ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT check_none_pwd( INT32 *is_none )
{
	BOOL _ret; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR conf_file_path[ MAX_PATH ]; 
	BYTE file_data[ sizeof( none_pwd_def ) ]; 

	LARGE_INTEGER file_size; 
	INT32 i = 0; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( is_none != NULL ); 

	*is_none = FALSE; 

	ret = get_user_conf_file_name( conf_file_path, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "get the config file name failed\n" ) );  
		goto _return; 
	}

	file_handle = CreateFile( conf_file_path, 
		GENERIC_ALL, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_HIDDEN, 
		NULL ); 

	if( file_handle == INVALID_HANDLE_VALUE )
	{
		SAFE_SET_ERROR_CODE( ret ); 
		goto _return; 
	}

	_ret = GetFileSizeEx( file_handle, &file_size ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "set file pointer failed \n" ) ); 
		goto _return; 
	}

	CloseHandle( file_handle ); 
	file_handle = INVALID_HANDLE_VALUE; 

	if( file_size.QuadPart != ( ULONGLONG )sizeof( none_pwd_def ) )
	{
		goto _return; 
	}

	ret = read_file( 0, file_data, sizeof( none_pwd_def ), conf_file_path ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( memcmp( file_data, none_pwd_def, sizeof( none_pwd_def ) ) == 0 )
	{
		*is_none = TRUE; 
		goto _return; 
	}

_return:
	if( file_handle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( file_handle ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT change_original_pwd( HWND parent_wnd )
{
	LRESULT ret = ERROR_SUCCESS; 
	BYTE cur_pwd[ PWD_DATA_LEN ]; 

	ASSERT( parent_wnd != NULL ); 
	ASSERT( IsWindow( parent_wnd ) == TRUE ); 

	ret = check_pwd_file_consist( cur_pwd, sizeof( cur_pwd ) ); 

	if( ret != ERROR_SUCCESS 
		|| *cur_pwd == '\0' )
	{
		chg_pwd_dlg dlg;
		dlg_ret_state ret_state; 
		LRESULT _ret; 
		LPCTSTR tmp_text; 
		INT32 is_none_pwd; 

		if( ret != ERROR_FILE_NOT_FOUND )
		{
			ret = check_none_pwd( &is_none_pwd ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			if( is_none_pwd == TRUE )
			{
				set_cur_user_group( ADMIN ); 
				goto _return; 
			}

			tmp_text = _get_string_by_id( TEXT_CHANGE_PASSWORD_CONF_DATA_CURRUPT_TIP, 
				_T( "ϵͳ�ؼ����ݱ��ƻ�,�û���Ϣ��Ҫ��������!" ) ); 

			show_msg( parent_wnd, tmp_text, NULL, _T( "" ), 0 ); 

			ret = clear_password(); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE && "clear password failed" ); 
				log_trace( ( MSG_ERROR, "clear the corrupted password failed\n" ) ); 
				goto _return; 
			}

			//PostQuitMessage( 0 ); 
		}

		tmp_text = _get_string_by_id( TEXT_CHANGE_PASSWORD_HAVE_NOT_SETTING_PWD_TIP, 
			_T( "���ڻ�û����������,�����Ҫ����ѧ������,������һ����������,��ֻ֤������Ը�������,���򽫲�ʹ������." ) ); 

		_ret = show_msg( parent_wnd, tmp_text, &ret_state, 0 ); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create message box failed to indicate null password failed\n" ) ); 
			//goto _return; 
		}

		if( ret_state == OK_STATE )
		{
			tmp_text = _get_string_by_id( TEXT_CHANGE_PASSWORD_SET_PWD_TITLE, 
				_T("������������") ); 

			dlg.Create( parent_wnd, tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
			dlg.SetIcon( IDI_MAIN_ICON ); 
			dlg.CenterWindow();
			dlg.ShowModal(); 
		}
		else
		{
			clear_password(); 
		}

		ret = check_none_pwd( &is_none_pwd ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		if( is_none_pwd == TRUE )
		{
			set_cur_user_group( ADMIN ); 
		}

		goto _return; 
	}

_return:
	return ret; 
}

LRESULT get_cur_work_mode( work_mode *mode_got, action_ui_notify *ui_notify )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG mode; 
	ULONG ret_len; 
	notify_data data_notify; 

	ASSERT( mode_got != NULL ); 
	
	*mode_got = WORK_FREE_MODE; 

	ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_GET_WORK_MODE, NULL, 0, ( PBYTE )&mode, sizeof( ULONG ), &ret_len, NULL ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( ret_len != sizeof( ULONG ) )
	{
		ASSERT( FALSE ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	*mode_got = ( work_mode )mode; 

	if( ui_notify != NULL )
	{
		data_notify.data_len = sizeof( ULONG ); 
		data_notify.data = &mode; 

		ret = ui_notify->action_notify( WORK_MODE_NOTIFY, &data_notify ); 

	}

_return:
	return ret; 
}

LRESULT set_cur_work_mode( work_mode mode_out, action_ui_notify *ui_notify )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG mode; 
	notify_data data_notify; 

	mode = mode_out; 

	ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_SET_WORK_MODE, ( PBYTE )&mode, sizeof( ULONG ), NULL, 0, NULL, NULL ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( ui_notify != NULL )
	{
		data_notify.data_len = sizeof( ULONG ); 
		data_notify.data = &mode; 

		ret = ui_notify->action_notify( WORK_MODE_NOTIFY, &data_notify ); 

	}

_return:
	return ret; 
}

LRESULT check_user_access( user_group check_group, HWND parent_wnd, dlg_ret_state *ret_state )
{
	LRESULT ret = ERROR_SUCCESS; 
	login_dlg login; 
	dlg_ret_state _ret_state; 
	LPCTSTR tmp_text; 

	if( ret_state != NULL )
	{
		*ret_state = CANCEL_STATE; 
	}

	if( get_cur_user_group() < check_group )
	{
		tmp_text = _get_string_by_id( TEXT_COMMON_DONT_HAVE_PRIVILEGE, 
			tmp_text = _T( "��ǰ�û�Ȩ�޲���ִ�д˲���,���½������." ) ); 

		show_msg( parent_wnd, tmp_text, &_ret_state, NULL, 0 ); 
		
		if( ret_state != NULL )
		{
			*ret_state = _ret_state; 
		}

		if( _ret_state == CANCEL_STATE )
		{
			ret = ERROR_ACCESS_DENIED;  
			goto _return; 
		}

		tmp_text = _get_string_by_id( TEXT_ACCOUNT_MANAGEMENT_TITLE, _T("�ҳ�����") ); 
		
		login.Create( parent_wnd, tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
		login.SetIcon( IDI_MAIN_ICON ); 
		login.CenterWindow();
		login.ShowModal(); 

		if( get_cur_user_group() < check_group )
		{
			ret = ERROR_ACCESS_DENIED;  
		}

		_ret_state = login.get_dlg_ret_statue(); 

		if( ret_state != NULL )
		{
			*ret_state = _ret_state; 
		}
	}

_return:
	return ret; 
}