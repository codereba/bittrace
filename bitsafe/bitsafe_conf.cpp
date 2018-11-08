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

#include "StdAfx.h"
#include "common_func.h"
#include "resource.h"
#include "bitsafe_common.h"
#include "bitsafe_conf.h"
#include "ui_ctrl.h"
#include "conf_file.h"
#include "tinyxml.h"
#include "inst_drv.h"
#include "..\DuiLib\XUnzip.h"
#include <winreg.h>

PBYTE conf_file_data = NULL; 
ULONG conf_file_len = 0; 

LRESULT LoadFromFile(LPCTSTR pstrFilename, int encoding, PBYTE *data, PULONG data_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	CStdString sFile = CPaintManagerUI::GetResourcePath();
	HANDLE hFile = INVALID_HANDLE_VALUE; 
	DWORD dwRead = 0;
	BYTE* pByte = NULL; 
	DWORD dwSize; 
	STRINGorID res_type; 
	STRINGorID res_name; 
	CStdString zip_file_name; 

	ASSERT( data != NULL ); 
	ASSERT( data_len != NULL ); 

	*data = NULL; 
	*data_len = 0; 

	if( CPaintManagerUI::is_res_zipped() == FALSE ) 
	{
		sFile += pstrFilename;
		hFile = ::CreateFile(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile == INVALID_HANDLE_VALUE ) 
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "Error opening file" ) ); 
			ret = GetLastError(); 
			goto _return; 
		}

		dwSize = ::GetFileSize(hFile, NULL);
		if( dwSize == 0 )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "File is empty" ) ); 
			ret = GetLastError(); 
			goto _return; 
		}

#ifndef ERROR_FILE_TOO_LARGE
#define ERROR_FILE_TOO_LARGE 223L
#endif //ERROR_FILE_TOO_LARGE

		if ( dwSize > 4096*1024 ) 
		{
			log_trace( ( MSG_INFO, "File too large" ) ); 
			ret = ERROR_FILE_TOO_LARGE; 
			goto _return; 
		}

		pByte = ( PBYTE )malloc( dwSize ); 
		if( pByte == NULL )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "allocate the data buffer failed \n" ) ); 
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		::ReadFile( hFile, pByte, dwSize, &dwRead, NULL );

		if( dwRead != dwSize ) 
		{
			ret = ERROR_FILE_CORRUPT; 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "Could not read file" ) ); 
			goto _return; ; 
		}

		goto _return; 
	}
	else 
	{
		CPaintManagerUI::GetResourceZipResType( res_type, res_name ); 
		ret = read_data_from_zip( CPaintManagerUI::GetResourceZip(), res_type, res_name, pstrFilename, &pByte, &dwSize ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "Could not read zip" ));
		}
	}

	*data = pByte; 
	*data_len = dwSize; 

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( pByte != NULL )
		{
			free( pByte ); 
		}
	}

	if( hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( hFile );
	}

	return ret; 
}

LRESULT load_db_files_from_res()
{
	LRESULT ret; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) );

	ret = safe_create_file_from_res( PARAM_DEFINE_DB_NAME ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "create param define db failed \n" ) );
 		goto _return; 
	}

	ret = safe_create_file_from_res( FW_LOG_DB_NAME ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "create fw log db failed \n" ) );
		goto _return; 
	}

	ret = safe_create_file_from_res( DEFENSE_LOG_DB_NAME ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "create defense log db failed \n" ) );
		goto _return; 
	}

	ret = safe_create_file_from_res( ACTION_RULE_DB_NAME ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "create access rule db failed \n" ) );
		goto _return; 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) );

	return ret; 
}


INLINE LRESULT get_config_path( LPCSTR config_name, LPSTR path, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	TiXmlDocument all_conf; 
	TiXmlElement* confs; 
	TiXmlNode* conf; 
	LPCSTR conf_file_name; 
	LPCSTR conf_server_path; 
	CHAR update_path[ _MAX_URL_LEN ]; 
	bool _ret; 

	ASSERT( config_name != NULL ); 
	ASSERT( path != NULL ); 

	*path = '\0'; 

	if( conf_file_data == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED;
		goto _return; 
	}

	_ret = all_conf.LoadMem( ( PCHAR )conf_file_data, conf_file_len ); 
	if( _ret == false )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	confs = all_conf.RootElement(); 
	ASSERT( confs != NULL ); 

	if( strcmp( confs->Value(), "configures" ) != 0 )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	conf = confs->FirstChild();

	ret = ERROR_NOT_FOUND; 
	while( conf )
	{
		if( conf->Type() == TiXmlNode::ELEMENT )
		{
			if( strcmp( conf->Value(), config_name ) != 0 )
			{
				continue; 
			}

			conf_file_name = conf->ToElement()->Attribute("name");
			conf_server_path = conf->ToElement()->Attribute( "server" ); 

#define HTTP_PROT_PREFIX "http://"
			if( strncmp( conf_server_path, HTTP_PROT_PREFIX, CONST_STR_LEN( HTTP_PROT_PREFIX ) ) !=  0 )
			{
				strcpy( update_path, HTTP_PROT_PREFIX ); 
			}
			else
			{
				*update_path = '\0'; 
			}

			strncat( update_path, conf_server_path, _MAX_URL_LEN - strlen( update_path ) ); 
			if( update_path[ strlen( update_path ) - 1 ] != '/' )
			{
				strncat( update_path, "/", _MAX_URL_LEN - strlen( update_path ) ); 
			}

			strncat( update_path, conf_file_name, _MAX_URL_LEN - strlen( update_path ) ); 

			ret = ERROR_SUCCESS; 
			break; 
		}
	}

	if( buf_len < strlen( update_path ) + sizeof( CHAR ) )
	{
		ret= ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	strcpy( path, update_path ); 

_return:
	return ret; 
}

LRESULT get_update_path( LPSTR path, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	ASSERT( path != NULL ); 
	ASSERT( buf_len > 0 ); 

	ret = get_config_path( "update_path", path, buf_len ); 

	return ret; 
}

LRESULT get_run_time_msg_path( LPSTR path, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( path != NULL ); 
	ASSERT( buf_len > 0 ); 

	ret = get_config_path( "msg_path", path, buf_len ); 

	return ret; 
}

#ifndef NO_NEED_SYS_MNG_DRIVER_CONF
#include "sys_mng_api.h"
#include "drv_load.h"

LRESULT write_krnl_hook_flag( BYTE flags )
{
#define MNG_DRV_FILE_NAME _T( "krnlmgr.sys" )
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR driver_file_name[ MAX_PATH ]; 
	IMAGE_DOS_HEADER dos_hdr; 

	if( flags != 0 
		&& flags != REPLACE_360_HOOK )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = add_sys_drv_path_to_file_name( driver_file_name, MAX_PATH, MNG_DRV_FILE_NAME, TRUE ); 
	if( ret != ERROR_SUCCESS ) 
	{
		log_trace( ( MSG_ERROR, "check the driver file in system dir failed \n" ) ); 
		goto _return; 
	}

	log_trace( ( MSG_INFO, "check %ws existing \n", driver_file_name ) ); 
	ret = file_exists( driver_file_name, DRIVER_IN_SYSTEM_DIR ); 
	if( ERROR_SUCCESS != ret )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_KRNL_MGR_FILE ), DAT_FILE_RES_TYPE, driver_file_name, !!( flags & DRIVER_IN_SYSTEM_DIR ) ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create the %ws from resource failed \n", driver_file_name ) ); 
			goto _return; 
		}
	}

	ret = read_file( 0, &dos_hdr, sizeof( dos_hdr ), driver_file_name ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "read dos header from sys manager driver file %ws failed\n", driver_file_name ) ); 
		goto _return; 
	}

	if( dos_hdr.e_lfanew <= FLAGS_1BYTE_ABS_OFFSET )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = write_file( FLAGS_1BYTE_ABS_OFFSET, &flags, sizeof( flags ), driver_file_name ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "write hook mode flags to sys manager driver file %ws, failed\n", driver_file_name ) ); 
		goto _return; 
	}

_return:
	return ret; 
}

#endif //NO_NEED_SYS_MNG_DRIVER_CONF

LRESULT _output_dir_from_zip( LPCTSTR zip_file_name, STRINGorID zip_res_type, STRINGorID zip_res_name, STRINGorID file_name, output_file_in_dir_callback output_files, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG dwSize = 0; 
	HZIP hz = NULL; 
	BYTE *pData = NULL; 
	ZIPENTRY ze; 
	INT32 _ret; 
	HRSRC hRes;
	HINSTANCE res_mod; 
	HGLOBAL hgResMem;
	LPVOID pRes;
	DWORD dwResSize; 
	INT32 i; 
	CStdString sFile;

	ASSERT( ( zip_file_name != NULL && *zip_file_name != _T( '\0' ) ) || zip_res_type.m_lpstr != NULL ); 

	if( zip_res_type.m_lpstr != NULL )
	{
		ASSERT( zip_res_name.m_lpstr != NULL ); 

		res_mod = CPaintManagerUI::GetResourceDll(); 

		hRes = FindResource( res_mod, zip_res_name.m_lpstr, zip_res_type.m_lpstr );
		if ( hRes == NULL )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		hgResMem = LoadResource( res_mod, hRes );
		if ( hgResMem == NULL )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		pRes = LockResource( hgResMem );
		if ( pRes == NULL )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		dwResSize = SizeofResource( res_mod, hRes );
		if ( dwResSize == 0 )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		hz = OpenZip( pRes, dwResSize, ZIP_MEMORY );
	}
	else
	{
		sFile = CPaintManagerUI::GetResourcePath();
		sFile += zip_file_name; 
		hz = OpenZip((void*)sFile.GetData(), 0, ZIP_FILENAME);
	}

	if( FindZipItem(hz, file_name.m_lpstr, true, &i, &ze) != 0 ) 
	{
		ret = ERROR_NOT_FOUND; 
		goto _return; 
	}

	dwSize = ze.unc_size;
	if( dwSize == 0 ) 
	{
		ASSERT( FALSE ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	pData = new BYTE[ dwSize ];
	if( pData == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_ret = UnzipItem(hz, i, pData, dwSize, 3);

	if( _ret != 0x00000000 && _ret != 0x00000600) 
	{	
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

_return:

	if( ret != ERROR_SUCCESS )
	{
		if( pData != NULL )
		{
			delete []pData; 
		}
	}

	if( hz != NULL )
	{
		CloseZip(hz);
	}

	return ret; 
}

LRESULT output_dir_from_zip( LPCTSTR dest_path, INT32 encoding, output_file_in_dir_callback output_files, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	CStdString sFile = CPaintManagerUI::GetResourcePath();
	HANDLE hFile = INVALID_HANDLE_VALUE; 
	DWORD dwRead = 0;
	BYTE* pByte = NULL; 
	STRINGorID res_type; 
	STRINGorID res_name; 
	CStdString zip_file_name; 

	ASSERT( output_files != NULL ); 

	DBG_BP(); 

	if( CPaintManagerUI::is_res_zipped() == FALSE ) 
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}
	else 
	{
		CPaintManagerUI::GetResourceZipResType( res_type, res_name ); 
		ret = _output_dir_from_zip( CPaintManagerUI::GetResourceZip(), res_type, res_name, dest_path, output_files, context ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "Could not read zip" ));
		}
	}

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( pByte != NULL )
		{
			free( pByte ); 
		}
	}

	if( hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( hFile );
	}

	return ret; 
}

LRESULT CALLBACK output_file_in_dir( LPCTSTR relative_path, BYTE *data, ULONG data_len, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}

#define WINDOWS_LOCAL_MACHINE_AUTO_RUN L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"

LRESULT regx_format_string( LPCTSTR str_input, LPTSTR str_output, ULONG buf_size )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR ch; 
	ULONG outputed; 

	outputed = 0; 
	ch = str_input; 

	for( ; ; )
	{
		if( *ch != L'\\' )
		{
			if( outputed + 1 >= buf_size )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				goto _return; 
			}

			str_output[ outputed ] = *ch; 
			outputed += 1; 

			if( *ch == L'\0' )
			{
				break; 
			}
		}
		else
		{
			if( outputed + 2 >= buf_size )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				goto _return; 
			}

			str_output[ outputed ] = L'\\'; 
			str_output[ outputed + 1 ] = L'\\'; 
			outputed += 2; 
		}

		ch ++; 
	}
_return:
	return ret; 
}

LRESULT add_auto_run_entry( LPCTSTR value_name, LPCTSTR file_name, BOOLEAN is_add )
{
	LRESULT ret = ERROR_SUCCESS; 
	HKEY main_key = NULL; 
	TCHAR _file_name[ MAX_PATH ]; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ret = RegOpenKey( HKEY_LOCAL_MACHINE, /*HKEY_CURRENT_USER*/
		WINDOWS_LOCAL_MACHINE_AUTO_RUN, 
		&main_key ); 
	
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( main_key == NULL ); 
		goto _return; 
	}

	if( TRUE == is_add )
	{
		if( FALSE == is_vista_or_later_ex() )
		{
			ret = regx_format_string( file_name, _file_name, MAX_PATH ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			log_trace( ( MSG_INFO, "set the auto run file %ws\n", _file_name ) ); 

			_tcsnccat( _file_name, _T( " restart" ), MAX_PATH - _tcslen( _file_name ) ); 
			if( _file_name[ _MAX_PATH - 1 ] != _T( '\0' ) )
			{
				_file_name[ _MAX_PATH - 1 ] = _T( '\0' ); 
			}

			ret = RegSetValueEx( main_key, 
				value_name, 
				0, 
				REG_SZ, 
				( BYTE* ) _file_name, 
				( _tcslen( _file_name ) + 1 ) * sizeof( TCHAR ) ); 
		}
		else
		{

			log_trace( ( MSG_INFO, "set the auto run file %ws\n", file_name ) ); 

			_tcsncpy( _file_name, file_name, MAX_PATH ); 
			if( _file_name[ MAX_PATH - 1 ] != _T( '\0' ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			_tcsnccat( _file_name, _T( " restart" ), MAX_PATH - _tcslen( _file_name ) ); 
			if( _file_name[ _MAX_PATH - 1 ] != _T( '\0' ) )
			{
				_file_name[ _MAX_PATH - 1 ] = _T( '\0' ); 
			}

			ret = RegSetValueEx( main_key, 
				value_name, 
				0, 
				REG_SZ, 
				( BYTE* ) _file_name, 
				( _tcslen( _file_name ) + 1 ) * sizeof( TCHAR ) ); 
		}
	}
	else
	{
		ret = RegDeleteValue( main_key, value_name ); 
	}
	
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	if( main_key != NULL )
	{
		RegCloseKey( main_key ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT set_self_autorun( ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD _ret; 
	TCHAR file_name[ MAX_PATH ]; 
	BOOLEAN is_add; 

	is_add = !!( flags & ADD_SELF_AUTO_RUN ); 
	
	if( is_add == TRUE )
	{
		_ret = GetModuleFileName( NULL, file_name, MAX_PATH ); 
		ASSERT( _ret <= MAX_PATH ); 

		if( _ret == 0 )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			goto _return; 
		}
		else if( _ret == MAX_PATH )
		{
			ASSERT( file_name[ MAX_PATH - 1 ] == _T( '\0' ) ); 

			ret = ERROR_BUFFER_TOO_SMALL; 
			goto _return; 
		}

		ret = add_auto_run_entry( BITSAFE_AUTORUN_NAME, file_name, is_add ); 
	}
	else
	{
		ret = add_auto_run_entry( BITSAFE_AUTORUN_NAME, NULL, is_add ); 
	}

_return:
	return ret; 
}

LRESULT del_prev_verions_auto_run()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = add_auto_run_entry( OLD_BITSAFE_AUTORUN_NAME, NULL, 0 ); 

	return ret; 
}

#define CODEREBA_REG_PATH L"SOFTWARE\\codereba\\bitsafe"
#define BITSAFE_DBG_NAME L"dbg"

LRESULT ctrl_dbg_mode( BOOLEAN open_dbg )
{	
	LRESULT ret = ERROR_SUCCESS; 
	HKEY main_key = NULL; 
	ULONG reg_disp; 
	ULONG dword_val; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ret = RegCreateKeyEx( HKEY_LOCAL_MACHINE, 
		CODEREBA_REG_PATH, 
		0, 
		NULL, 
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS, 
		NULL, 
		&main_key, 
		&reg_disp ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( main_key == NULL ); 
		goto _return; 
	}

	if( TRUE == open_dbg )
	{

		dword_val = 1; 
		ret = RegSetValueEx( main_key, 
			BITSAFE_DBG_NAME,
			0, 
			REG_DWORD, 
			( CONST BYTE* )&dword_val, 
			sizeof( dword_val ) ); 
	}
	else
	{
		ret = RegDeleteValue( main_key, BITSAFE_DBG_NAME ); 
	}
	
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	if( main_key != NULL )
	{
		RegCloseKey( main_key ); 
	}
	return ret; 
}

LRESULT get_dbg_mode( BOOLEAN *is_dbg )
{
	LRESULT ret = ERROR_SUCCESS; 
	HKEY main_key = NULL; 	
	ULONG open_dbg; 
	ULONG val_type = REG_DWORD; 
	ULONG ret_len; 

	ASSERT( is_dbg != NULL ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	*is_dbg = FALSE; 

	ret_len = sizeof( open_dbg ); 

	do 
	{
		ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
			CODEREBA_REG_PATH,
			REG_OPTION_NON_VOLATILE, 
			KEY_ALL_ACCESS, 
			&main_key ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( main_key == NULL ); 
			break; 
		}

		ret = RegQueryValueEx( main_key, 
			BITSAFE_DBG_NAME, 
			0, 
			&val_type, 
			( BYTE* )&open_dbg, 
			&ret_len ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		*is_dbg = ( BOOLEAN )open_dbg; 

	}while( FALSE );

	if( main_key != NULL )
	{
		RegCloseKey( main_key ); 
	}
	return ret; 
}