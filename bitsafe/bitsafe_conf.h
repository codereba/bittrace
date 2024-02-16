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

#pragma once

#define BITSAFE_CONF_FILE_NAME _T( "data5.dat" )

extern PBYTE conf_file_data; 
extern ULONG conf_file_len; 

typedef LRESULT ( CALLBACK *output_file_in_dir_callback )( LPCTSTR relative_path, BYTE *data, ULONG data_len, PVOID context ); 

LRESULT _output_dir_from_zip( LPCTSTR zip_file_name, STRINGorID zip_res_type, STRINGorID zip_res_name, STRINGorID file_name, output_file_in_dir_callback output_files, PVOID context ); 
LRESULT output_dir_from_zip( LPCTSTR dest_path, INT32 encoding, output_file_in_dir_callback output_files, PVOID context ); 

LRESULT write_krnl_hook_flag( BYTE flags ); 

LRESULT LoadFromFile(LPCTSTR pstrFilename, int encoding, PBYTE *data, PULONG data_len ); 

INLINE LRESULT load_bitsafe_conf( PBYTE *data, PULONG data_len )
{
	return LoadFromFile( BITSAFE_CONF_FILE_NAME, XMLFILE_ENCODING_UTF8, data, data_len ); 
}

LRESULT load_db_files_from_res(); 

INLINE LRESULT init_bitsafe_conf()
{
	return load_bitsafe_conf( &conf_file_data, &conf_file_len ); 
}

INLINE VOID uninit_bitsafe_conf()
{
	if( conf_file_data != NULL )
	{
		free( conf_file_data ); 
		conf_file_data = NULL; 
	}

	conf_file_len = 0; 
}

LRESULT CALLBACK output_file_in_dir( LPCTSTR relative_path, BYTE *data, ULONG data_len, PVOID context ); 

INLINE LRESULT create_dir_from_res_zip( LPCTSTR file_name, ULONG to_sys_dir )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = output_dir_from_zip( file_name, XMLFILE_ENCODING_UTF8, output_file_in_dir, NULL ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}
	
_return:
	return ret; 
}

INLINE LRESULT _create_file_from_res_zip( LPCTSTR file_name, LPCTSTR output_file_name, LPCTSTR output_path, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	BYTE *data = NULL; 
	ULONG data_len = 0; 
	WCHAR file_path[ MAX_PATH ]; 

	ASSERT( file_name != NULL ); 

	ret = LoadFromFile( file_name, XMLFILE_ENCODING_UTF8, &data, &data_len ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "load the theme file from resource error" ); 
		goto _return; 
	}

	if( output_file_name == NULL )
	{
		output_file_name = file_name; 
	}

	if( flags == FILE_IS_WHOLE_PATH )
	{
		ASSERT( output_path != NULL ); 

		wcsncpy( file_path, output_path, MAX_PATH ); 
		if( file_path[ MAX_PATH - 1 ] != L'\0' )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			goto _return; 
		}
	}
	else if( flags == FILE_TARGET_DIR )
	{
		ASSERT( output_path != NULL ); 

		ret = construct_file_path( file_path, MAX_PATH, output_path, output_file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE &&"construct the destination file name error\n" ); 

			log_trace( ( MSG_INFO, "construct the destination file name error\n" ) ); 
			goto _return;  
		}
	}
	else if( flags == FILE_SYS_DIR )
	{
		ret = add_app_path_to_file_name( file_path, MAX_PATH, output_file_name, FALSE ); 
	}
	else if( flags == FILE_APP_DIR )
	{
		ret = add_sys_drv_path_to_file_name( file_path, MAX_PATH, output_file_name, FALSE ); 
	}
	else
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = write_file( 0, data, data_len, file_path ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	if( data != NULL )
	{
		free( data ); 
	}

	return ret; 
}

INLINE LRESULT create_file_from_res_zip( LPCTSTR file_name, ULONG to_sys_dir )
{
	return _create_file_from_res_zip( file_name, NULL, NULL, to_sys_dir == TRUE ? FILE_SYS_DIR : FILE_APP_DIR ); 
}

INLINE LRESULT safe_create_file_from_res( LPCTSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = file_exists( file_name, FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		ret = create_file_from_res_zip( file_name, FALSE ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

_return:
	return ret; 
}

INLINE LRESULT safe_create_dir_fram_res( LPCTSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = file_exists( file_name, FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		ret = create_dir_from_res_zip( file_name, FALSE ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

_return:
	return ret; 
}

LRESULT get_update_path( LPSTR path, ULONG buf_len ); 
LRESULT get_run_time_msg_path( LPSTR path, ULONG buf_len ); 
LRESULT regx_format_string( LPCTSTR str_input, LPTSTR str_output, ULONG buf_size ); 
LRESULT add_auto_run_entry( LPCTSTR value_name, LPCTSTR file_name, BOOLEAN is_add ); 
#define BITSAFE_AUTORUN_NAME L"bitsafe" 
#define OLD_BITSAFE_AUTORUN_NAME L"sevenfw"

#define ADD_SELF_AUTO_RUN 1

LRESULT set_self_autorun( ULONG flags ); 
LRESULT del_prev_verions_auto_run(); 

LRESULT ctrl_dbg_mode( BOOLEAN open_dbg ); 
LRESULT get_dbg_mode( BOOLEAN *is_dbg );  

