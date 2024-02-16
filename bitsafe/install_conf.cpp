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
#include "install_conf.h"
#include "tinyxml.h"
#include "msg_box.h"
#include "bitsafe_conf.h"
#include "safe_install.h"
#include "ui_ctrl.h"

install_conf_context install_conf = { 0, 0, { 0 }, { 0 } }; 

LRESULT get_file_version( LPCWSTR file_name, ULARGE_INTEGER *file_version )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( file_version != NULL ); 

	do 
	{
		file_version->QuadPart = 0; 

	} while ( FALSE ); 

	return ret; 
}

LRESULT _check_file_version( LPCWSTR file_name, ULARGE_INTEGER file_version, ULONG flags, BOOLEAN *result )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULARGE_INTEGER dest_file_version; 
	BOOLEAN _result = FALSE; 
	
	do 
	{
		ASSERT( result != NULL ); 

		ret = get_file_version( file_name, &dest_file_version ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		switch( flags )
		{
		case COMPARE_EQUAL: 
			if( dest_file_version.QuadPart == file_version.QuadPart )
			{
				_result = TRUE; 
			}
			else
			{
				_result = FALSE; 
			}
			break; 
		case COMPARE_GREATER: 
			if( dest_file_version.QuadPart < file_version.QuadPart )
			{
				_result = TRUE; 
			}
			else
			{
				_result = FALSE; 
			}
			break; 
		case COMPARE_LOWER:
			if( dest_file_version.QuadPart > file_version.QuadPart )
			{
				_result = TRUE; 
			}
			else
			{
				_result = FALSE; 
			}
			break; 
		default: 
			ASSERT( FALSE && "invalid compare type" ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT check_file_version( LPCWSTR file_name, LPCWSTR file_version, ULONG flags, BOOLEAN *result )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	BOOLEAN condition_ok = TRUE; 

	CFileVersionInfo version_info; 
	
	do 
	{
		ASSERT( file_version != NULL ); 

		ret = check_file_exist( file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		_ret = ( INT32 )version_info.Create( file_name ); 
		if( _ret == FALSE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ret = version_info.CompareFileVersion( file_version, &_ret ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		switch( flags )
		{
		case COMPARE_EQUAL:
			if( _ret != 0 )
			{
				condition_ok = FALSE; 
			}
			break; 
		case COMPARE_GREATER: 
			if( _ret <= 0 )
			{
				condition_ok = FALSE; 
			}
			break; 
		case COMPARE_LOWER:
			if( _ret >= 0 )
			{
				condition_ok = FALSE; 
			}
			break; 
		case COMPARE_LOWER | COMPARE_EQUAL:
			if( _ret > 0 )
			{
				condition_ok = FALSE; 
			}
			break; 
		case COMPARE_GREATER | COMPARE_EQUAL:
			if( _ret > 0 )
			{
				condition_ok = FALSE; 
			}
			break; 
		default:
			break; 
		}

	}while( FALSE );

	*result = condition_ok; 
	return ret; 
}

LRESULT do_def_action( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );
	
	return ret; 
}

LRESULT check_file_sign( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}

LRESULT check_file_integrety( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( file_name != NULL ); 

	return ret; 
}

FORCEINLINE LRESULT delete_dest_file( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	do 
	{
		_ret = DeleteFile( file_name ); 
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			if( ret != ERROR_FILE_NOT_FOUND )
			{
				break; 
			}

			ret = ERROR_SUCCESS; 
		}
	}while( FALSE );

	return ret; 
}


#define LOAD_FROM_RES_ZIP 0
#define LOAD_FROM_RES 1
#define LOAD_FROM_FILE 2

#define INVALID_RES_ID ( UINT32 )( -1 ) 

FORCEINLINE LRESULT _replace_dest_file( LPCWSTR dest_file, LPCWSTR src_name, LPCWSTR res_type, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	do 
	{
		if( *src_name == L'\0' )
		{
			log_trace( ( MSG_INFO, "have not set src file name for file %ws\n", dest_file ) ); 
			break; 
		}

		_ret = DeleteFile( dest_file ); 
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			log_trace( ( MSG_ERROR, "delete existing file which replaced error 0x%0.8x\n", ret ) ); 

			ret = GetLastError(); 
			if( ret != ERROR_FILE_NOT_FOUND )
			{
				break; 
			}

			ret = ERROR_SUCCESS; 
		}

		if( flags == LOAD_FROM_RES_ZIP )
		{
			ASSERT( src_name[ 0 ] != L'\0' && src_name[ 0 ] != L'#' ); 
			ASSERT( src_name[ 1 ] != L':' ); 

			ret = _create_file_from_res_zip( src_name, NULL, dest_file, FILE_IS_WHOLE_PATH ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "create param define db failed \n" ) );
				break; 
			}
		}
		else if( flags == LOAD_FROM_RES )
		{
			UINT32 file_res_id; 
			LPWSTR tmp_str; 

			if( res_type == NULL 
				|| *res_type == L'\0' )
			{
				log_trace( ( MSG_ERROR, "install this file %ws, but have not set the resource type,why?", src_name ) ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( src_name[ 0 ] == L'#' )
			{
				file_res_id = wcstol( src_name + 1, &tmp_str, 10 ); 
			}
			else
			{
				log_trace( ( MSG_ERROR, "install this file %ws, but have not the correct format,why?", src_name ) ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
#if 0
			else
			{
				file_res_id = get_file_res_id( src_name ); 
			}
#endif //0

			if( file_res_id == INVALID_RES_ID )
			{
				ret = ERROR_INVALID_PARAMETER; 
				log_trace( ( MSG_ERROR, "install this file %ws, but this file have not its id,why?", src_name ) ); 
				break; 
			}
			
			ret = output_file_from_res( NULL, 
				MAKEINTRESOURCE( file_res_id ), 
				res_type, 
				dest_file ); 
		}
		else if( flags == LOAD_FROM_FILE )
		{
			ASSERT( src_name[ 1 ] != L':' ); 

			_ret = MoveFileEx( src_name, dest_file, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED ); 
			if( _ret == FALSE )
			{
				ret = GetLastError(); 
			}
		}
		else
		{
			ASSERT( FALSE && "invalid load file method when installation" ); 
		}
	}while( FALSE );
	return ret; 
}

#define INVALID_LOAD_TYPE ( ULONG )( -1 )
FORCEINLINE LRESULT replace_dest_file( LPCWSTR dest_file, LPCWSTR src_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG flags = INVALID_LOAD_TYPE; 
	LPWSTR tmp_str; 

	do 
	{
		if( *src_name == L'\0' )
		{
			log_trace( ( MSG_INFO, "have not set src file name for file %ws\n", dest_file ) ); 
			break; 
		}
		else if( *src_name == L'#' )
		{
			flags = LOAD_FROM_RES; 
			if( install_conf.res_type[ 0 ] == L'\0' )
			{
				DBG_BP(); 
				ret = ERROR_NOT_READY; 
				break; 
			}

		}
		else 
		{
			tmp_str = ( LPWSTR )wcsstr( src_name, L":\\" ); 

			if( NULL == tmp_str /*src_name[ 1 ] == L':'*/ )
			{
				flags = LOAD_FROM_FILE; 
			}
			else
			{
				flags = LOAD_FROM_RES_ZIP; 
			}
		}

		ret = _replace_dest_file( dest_file, src_name, install_conf.res_type, flags ); 
	}while( FALSE );
	return ret; 
}

FORCEINLINE LRESULT uninstall_service_w( LPCWSTR service_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	SC_HANDLE svc_manage = NULL;
	SC_HANDLE service = NULL; 

	log_trace( ( MSG_INFO, "enter %s service name %ws\n", __FUNCTION__, service_name ) ); 

	do 
	{
		if( service_name == NULL )\
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( L'\0' == service_name[ 0 ] )
		{
			log_trace( ( MSG_WARNING, "uninstall action but have no service name, destination file\n" ) ); 
		}
		else
		{	
			svc_manage = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
			if( svc_manage == NULL ) 
			{
				ret = GetLastError(); 

				log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenSCManager failed \n" ) ); 
				break; 
			}

			service = OpenService( svc_manage, service_name, SERVICE_ALL_ACCESS);
			if( service == NULL ) 
			{
				ret = GetLastError(); 
				if( ret = ERROR_SERVICE_DOES_NOT_EXIST )
				{
					ret = ERROR_SUCCESS; 
				}

				log_trace( ( DBG_MSG_AND_ERROR_OUT, "OpenService failed\n" ) ); 
				break; 
			}

			_ret = DeleteService( service ); 
			if( _ret == FALSE ) 
			{
				ret = GetLastError(); 

				log_trace( ( DBG_MSG_AND_ERROR_OUT, "Failed to delete service %s\n", service_name ) ); 
				break; 
			} 
		}

	}while( FALSE );

	if( service != NULL )
	{
		CloseServiceHandle( service ); 
	}

	if( svc_manage != NULL )
	{
		CloseServiceHandle( svc_manage ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

INLINE LRESULT do_install_action( file_install_action action, file_install_conf *conf, LPCWSTR dest_file_path, LPCWSTR src_file_path )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( conf != NULL ); 
		ASSERT( dest_file_path != NULL ); 
		ASSERT( src_file_path != NULL ); 

		if( action & FILE_INSTALL_DELETE )
		{
			ret = delete_dest_file( dest_file_path ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

		if( action & FILE_INSTALL_REPLACE )
		{
			ret = replace_dest_file( dest_file_path, src_file_path ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

		if( action & FILE_INSTALL_UNINSTALL )
		{
			ret = uninstall_service_w( conf->service_name ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
	} while ( FALSE ); 

	return ret; 
}

#define CHECK_NAME_NORMAL 0
#define CHECK_NAME_UPDATED 1 
LRESULT _install_file_by_conf( file_install_conf *conf, LPCWSTR src_dir, LPCWSTR dest_dir, prepare_work_action *prepare_action, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR dest_file_path[ MAX_PATH ]; 
	WCHAR src_file_path[ MAX_PATH ]; 
	BOOLEAN confition_ok; 
	BOOLEAN handled = FALSE; 
	LPCWSTR check_name; 

	do 
	{
		if( flags == CHECK_NAME_NORMAL )
		{
			check_name = conf->file_name; 
		}
		else if( flags == CHECK_NAME_UPDATED )
		{
			if( *conf->update_name == L'\0' )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			check_name = conf->update_name; 
		}
		else
		{
			DBG_BP(); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( *check_name == L'\0' )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( prepare_action != NULL ); 
		*prepare_action = PREPARE_BY_RESTART; 

		ret = construct_file_path_w( dest_file_path, MAX_PATH, dest_dir, check_name ); 	
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( *src_dir != L'\0' )
		{
			ret = construct_file_path_w( src_file_path, MAX_PATH, src_dir, check_name ); 	
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else
		{
			*src_file_path = L'\0'; 
		}

		ret = check_file_version( dest_file_path, conf->file_version, conf->compare_type, &confition_ok ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( TRUE == confition_ok )
		{
			ret = do_install_action( conf->true_install_action, conf, dest_file_path, src_file_path ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			*prepare_action = conf->true_prepare_action; 
		}
		else
		{
			ret = do_install_action( conf->false_install_action, conf, dest_file_path, src_file_path ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			*prepare_action = conf->false_prepare_action; 

		}

		if( handled == FALSE )
		{
			ret = do_def_action( dest_file_path ); 
		}

	}while( FALSE );

	return ret; 
}

LRESULT install_file_by_conf( file_install_conf *conf, LPCWSTR src_dir, LPCWSTR dest_dir, prepare_work_action *prepare_action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = _install_file_by_conf( conf, src_dir, dest_dir, prepare_action, CHECK_NAME_NORMAL ); 
		if( ret != ERROR_SUCCESS )
		{
			ret = _install_file_by_conf( conf, src_dir, dest_dir, prepare_action, CHECK_NAME_UPDATED ); 
		}
	}while( FALSE ); 

	return ret; 
}

#define BITSAFE_CONF_REG_PATH L"SOFTWARE\\codereba\\must_restart"
LRESULT set_must_restart_mark()
{
	LRESULT ret = ERROR_SUCCESS; 
	HKEY restart_tip_key = NULL; 
	ULONG create_disp = 0; 

	do 
	{
		ret = RegCreateKeyEx( HKEY_LOCAL_MACHINE, BITSAFE_CONF_REG_PATH, 0, NULL, REG_OPTION_VOLATILE, /*KEY_ALL_ACCESS*/KEY_READ, NULL, &restart_tip_key, &create_disp ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( restart_tip_key == NULL ); 
			log_trace( ( MSG_ERROR, "create restart tip key error 0x%0.8x\n", ret ) ); 
			break; 
		}

		ASSERT( restart_tip_key != NULL ); 
		RegCloseKey( restart_tip_key ); 
	}while( FALSE );

	return ret; 
}

LRESULT check_must_restart_mark()
{
	LRESULT ret = ERROR_ERRORS_ENCOUNTERED; 
	HKEY restart_tip_key = NULL; 

	do 
	{
		ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, BITSAFE_CONF_REG_PATH, 0, /*KEY_ALL_ACCESS*/KEY_READ, &restart_tip_key ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( restart_tip_key == NULL ); 
			log_trace( ( MSG_ERROR, "create restart tip key error 0x%0.8x\n", ret ) ); 
			break; 
		}

		ASSERT( restart_tip_key != NULL ); 
		RegCloseKey( restart_tip_key ); 
	}while( FALSE );

	return ret; 
}

LRESULT del_must_restart_mark()
{
	LRESULT ret = ERROR_ERRORS_ENCOUNTERED; 

	do 
	{
#ifdef _VISTA
		ret = RegDeleteKeyEx( HKEY_LOCAL_MACHINE, BITSAFE_CONF_REG_PATH, KEY_READ, 0 ); 
#else
		ret = RegDeleteKey( HKEY_LOCAL_MACHINE, BITSAFE_CONF_REG_PATH ); 
#endif //_VISTA

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create restart tip key error 0x%0.8x\n", ret ) ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT must_restart()
{
	LRESULT ret = ERROR_SUCCESS; 

	do
	{
		dlg_ret_state ret_state; 
		LPCTSTR tmp_text; 

		//���ذ�ȫ�����ط��������ضܣ����ر��������ع��ˣ����ذ�ȫ���ˣ����ط���

		tmp_text = _get_string_by_id( TEXT_INSTALL_MUST_RESTART, _T( "���ذ�ȫ��Ҫ����ϵͳ����ɰ�װ,�Ƿ���������?" ) ); 

		ret = show_msg( NULL, tmp_text, &ret_state, NULL, 0, NULL ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
		}

		ret = set_must_restart_mark(); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
		}

		if( ret_state == OK_STATE )
		{
			windows_shutdown( EWX_REBOOT | EWX_FORCE );  
		}

	}while( FALSE ); 

	return ret; 
}

typedef LRESULT ( CALLBACK* on_install_conf_loaded )( file_install_conf *conf, PVOID context ); 

#define INSTALL_CONF_FILE_NAME L"uninstall_conf.dat"
#define INSTALL_CONF "installation"
#define INSTALL_FILE "file"
#define INSTALL_FILE_NAME "name"
#define TRUE_INSTALL_ACTION "true_install_action"
#define FALSE_INSTALL_ACTION "false_install_action"
#define TRUE_PREPARE_ACTION "true_prepare_action"
#define FALSE_PREPARE_ACTION "false_prepare_action"
#define SERVICE_CONF_NAME "service"
#define COND_VERSION "version"
#define COMPARE_MODE "compare"
#define FILE_HASH_MD5 "md5"
#define FILE_UPDATE_NAME "update_name"

LRESULT get_bitsafe_intall_path( LPWSTR install_path, ULONG cch_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( install_path != NULL ); 
		ASSERT( cch_len > 0 ); 


	} while ( FALSE );
	
	return ret; 
}

LRESULT CALLBACK on_bitsafe_install_conf_loaded( file_install_conf *conf, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	install_conf_context *_conf; 
	prepare_work_action prepare_action; 

	do 
	{
		ASSERT( conf != NULL ); 
		ASSERT( context != NULL ); 
	
		_conf = ( install_conf_context* )context; 

		ret = install_file_by_conf( conf, _conf->src_dir, _conf->dest_dir, &prepare_action ); 
		if( ret != ERROR_SUCCESS )
		{
			_conf->prepare_restart_count ++; 
			break; 
		}
		else
		{
			if( prepare_action == PREPARE_BY_RESTART )
			{
				_conf->prepare_restart_count ++; 
			}
			else if( prepare_action == PREPARE_NONE )
			{
				_conf->prepare_none_count ++; 
			}
		}
	} while ( FALSE );
	
	return ret;  
}

LRESULT load_install_conf( on_install_conf_loaded conf_loaded_callback, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	TiXmlDocument xml_doc; 
	TiXmlElement *root = NULL; 
	TiXmlNode *node = NULL; 
	TiXmlElement *install_conf = NULL; 
	CHAR *tmp_str = NULL; 
	LPCSTR attr_val; 
	bool _ret; 
	file_install_conf file_conf; 
	ULONG ret_len; 
	PBYTE uninstall_conf_data = NULL; 
	ULONG uninstall_conf_len = 0; 

	PBYTE uninstall_conf_data_copy = NULL; 


	if( conf_loaded_callback == NULL )
	{
		DBG_BP(); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = LoadFromFile( INSTALL_CONF_FILE_NAME, 
		XMLFILE_ENCODING_UTF8, 
		&uninstall_conf_data, 
		&uninstall_conf_len ); 

	if( ret != ERROR_SUCCESS )
	{
		DBG_BP(); 
		goto _return; 
	}

#define TRAILING_ZERO_SIZE 30 
	uninstall_conf_data_copy = ( BYTE* )malloc( uninstall_conf_len + TRAILING_ZERO_SIZE ); 
	memcpy( uninstall_conf_data_copy, uninstall_conf_data, uninstall_conf_len ); 


	_ret = xml_doc.LoadMem( ( CHAR* )uninstall_conf_data_copy, uninstall_conf_len ); 
	if( _ret == false )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	root = xml_doc.RootElement(); 
	if( root == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	node = root->FirstChild( INSTALL_FILE ); 
	if( node == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	for( ; ; )
	{
		if( node == NULL )
		{
			goto _return; 
		}

		install_conf = node->ToElement(); 

		attr_val = install_conf->Attribute( INSTALL_FILE_NAME ); 
		if( attr_val == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		ret = mcbs_to_unicode( attr_val, file_conf.file_name, 
			ARRAYSIZE( file_conf.file_name ), 
			&ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			goto _return; 
		}

		attr_val = install_conf->Attribute( FILE_UPDATE_NAME ); 
		if( attr_val == NULL )
		{
			*file_conf.update_name = L'\0'; 
		}
		else
		{
			ret = mcbs_to_unicode( attr_val, file_conf.update_name, 
				ARRAYSIZE( file_conf.update_name ), 
				&ret_len ); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BP(); 
				goto _return; 
			}
		}

		attr_val = install_conf->Attribute( SERVICE_CONF_NAME ); 
		if( attr_val == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		ret = mcbs_to_unicode( attr_val, file_conf.service_name, 
			ARRAYSIZE( file_conf.service_name ), 
			&ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			goto _return; 
		}

		attr_val = install_conf->Attribute( COND_VERSION ); 
		if( attr_val == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		ret = mcbs_to_unicode( attr_val, file_conf.file_version, 
			ARRAYSIZE( file_conf.file_version ), 
			&ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			goto _return; 
		}

		attr_val = install_conf->Attribute( COMPARE_MODE ); 
		if( attr_val == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
		
		file_conf.compare_type = strtol( attr_val, &tmp_str, 10 ); 
		if( FALSE == is_valid_compare_mode( file_conf.compare_type ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		attr_val = install_conf->Attribute( TRUE_INSTALL_ACTION ); 
		if( attr_val == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		file_conf.true_install_action = ( file_install_action )strtol( attr_val, &tmp_str, 10 ); 
		if( FALSE == is_valid_install_action( file_conf.true_install_action ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		attr_val = install_conf->Attribute( FALSE_INSTALL_ACTION ); 
		if( attr_val == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		file_conf.false_install_action = ( file_install_action )strtol( attr_val, &tmp_str, 10 ); 
		if( FALSE == is_valid_install_action( file_conf.false_install_action  ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		attr_val = install_conf->Attribute( TRUE_PREPARE_ACTION ); 
		if( attr_val == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		file_conf.true_prepare_action = ( prepare_work_action )strtol( attr_val, &tmp_str, 10 ); 
		if( FALSE == is_valid_prepare_action( file_conf.true_prepare_action ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		attr_val = install_conf->Attribute( FALSE_PREPARE_ACTION ); 
		if( attr_val == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		file_conf.false_prepare_action = ( prepare_work_action )strtol( attr_val, &tmp_str, 10 ); 
		if( FALSE == is_valid_prepare_action( file_conf.false_prepare_action  ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		attr_val = install_conf->Attribute( FILE_HASH_MD5 ); 
		if( attr_val == NULL )
		{
			file_conf.md5[ 0 ] = L'\0'; 			
		}
		else
		{
			strncpy( file_conf.md5, attr_val, MAX_MD5_STR_LEN ); 
		}

		if( conf_loaded_callback != NULL )
		{
			LRESULT _ret; 
			_ret = conf_loaded_callback( &file_conf, context ); 

#ifdef _DEBUG
			if( _ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "update ui configuration on callback function error\n" ) ); 
			}
#endif //_DEBUG

		}

		node = install_conf->NextSibling( INSTALL_FILE_NAME ); 
	}

#ifdef _DEBUG
	xml_doc.Print(); 
#endif //_DEBUG

_return:
	if( uninstall_conf_data != NULL )
	{
		free( uninstall_conf_data ); 
		//mem_free( ( VOID** )&file_name ); 
	}

	if( uninstall_conf_data_copy != NULL )
	{
		free( uninstall_conf_data_copy ); 
	}

	return ret; 
}

LRESULT prepare_installation( install_conf_context *context )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( context != NULL ); 

		if( ( context->prepare_none_count == 0 
			&& context->prepare_restart_count == 0 ) 
			|| context->prepare_restart_count > 0 )
		{
			must_restart(); 
			break; 
		}
		
		ASSERT( context->prepare_restart_count == 0 && context->prepare_none_count > 0 ); 

	} while ( FALSE );
	
	return ret; 
}

LRESULT init_uninstall_context( LPCWSTR dest_dir, LPCWSTR src_dir, LPCWSTR res_type )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( dest_dir == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		install_conf.prepare_restart_count= 0; 
		install_conf.prepare_none_count = 0; 

		if( wcslen( dest_dir ) >= MAX_PATH )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		wcscpy( install_conf.dest_dir, dest_dir ); 

		if( src_dir == NULL )
		{
			*install_conf.src_dir = L'\0';  
		}
		else
		{
			if( wcslen( src_dir ) >= MAX_PATH )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			wcscpy( install_conf.src_dir, src_dir ); 
		}

		if( src_dir == NULL )
		{
			*install_conf.res_type = L'\0';  
		}
		else
		{
			if( wcslen( res_type ) >= MAX_RES_TYPE_LEN )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			wcscpy( install_conf.res_type, res_type ); 
		}
	}while( FALSE );
	return ret; 
}

LRESULT uninstall_bitsafe_by_conf_file( ULONG *prepare_work )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = load_install_conf( on_bitsafe_install_conf_loaded, &install_conf );
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "uninstall bitsafe by configuration file error 0x%0.8x\n", ret ) ); 
		}

	} while ( FALSE ); 

	if( prepare_work != NULL )
	{
		if( ( install_conf.prepare_none_count == 0 
			&& install_conf.prepare_restart_count == 0 ) 
			|| install_conf.prepare_restart_count > 0 )
		{
			*prepare_work = PREPARE_BY_RESTART; 
		}
		else
		{
			*prepare_work = PREPARE_NONE; 
		}
	}

	return ret; 
}

LRESULT do_install_prepare_work()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = prepare_installation( &install_conf ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "prepare uninstall bitsafe error 0x%0.8x\n", ret ) ); 
	}

	return ret; 
}