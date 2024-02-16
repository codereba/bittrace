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
#include "update.h"
#include "tinyxml.h"
#include <Wininet.h>
#include <Mmsystem.h>
#include <UIUtil.h>

#pragma comment( lib, "wininet.lib" )

#define BITSAFE_VERSION "0.9.3.6"

#ifdef LANG_EN
#define UPDATE_ERROR_TEXT L"Updating error" 
#define BITTRACE_BIN_FILE_CORRUPT_TEXT L"BitSafe configuration file is corrupted, check or delete the dat files"
#define ERROR_TIP_CANT_DOWNLOAD_UPDATE_FILE L"Get updating information error, please check network,if that's correct, you download manually from:\"http://www.simpai.net/bittrace_en.zip\" wait server have maintained."
#define ERROR_TIP_SERVICE_MAINTAIN L"Updating server is maintained, can't be updating now."
#define ERROR_TIP_UPDATE_FILE_ERROR __t( "Update service configure error, please report to: ( email:" ) BITSAFE_EMAIL __t( " bbs:codereba.com/bbs ),thanks very much." )
#define DONT_NEED_UPDATE_TEXT L"You are using newest version, don't need to update."
#else
#define ERROR_TIP_CANT_DOWNLOAD_UPDATE_FILE L"��ȡ������Ϣʧ��,������������.�����������û������,���ֶ�������\"http://www.simpai.net/bittrace.zip\"���ػ�ȴ�������ά�����."
#define BITTRACE_BIN_FILE_CORRUPT_TEXT L"���ذ�ȫ�����ܵ��ƻ�,���鲢��֤�ļ�����ȷ��."
#define UPDATE_ERROR_TEXT L"����ʧ��" 
#define ERROR_TIP_SERVICE_MAINTAIN L"���·���������ά��,�����޷����и���."
#define ERROR_TIP_UPDATE_FILE_ERROR L"���·�������������,�������Ա��������( email:" BITSAFE_EMAIL L" bbs:codereba.com/bbs ),лл."
#define DONT_NEED_UPDATE_TEXT L"������ʹ�����°�ı��ذ�ȫ,����Ҫ����."
#endif //LANG_EN

LRESULT InternetGetFile( LPCSTR url, LPCSTR file_name, file_download_callback download_tip_func, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
    DWORD flags;

	HINTERNET hOpen = NULL;	
	HINTERNET  hConnect = NULL; 
	HANDLE file = INVALID_HANDLE_VALUE; 

	CHAR agent[64]; 
	DWORD dwSize;
	CHAR   szHead[] = "Accept: */*\r\n\r\n"; 
#define HTTP_READ_TMP_BUF_LEN 16384
	VOID* szTemp[ HTTP_READ_TMP_BUF_LEN ];
	DWORD dwByteToRead = 0;
	DWORD bytes_readed; 
	DWORD dwSizeOfRq = 4;
	DWORD dwBytes = 0;
	DWORD writed = 0; 
	LARGE_INTEGER offset = { 0, 0 }; 
	LARGE_INTEGER now_offset; 
	ULONG try_time; 

	memset( agent, 0, sizeof( agent ) ); 
	sprintf( agent, "agent %ld", timeGetTime() ); 
	_ret = InternetGetConnectedState( &flags, 0 ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "get internet connection state failed\n" ) ); 

		if( ret == ERROR_SUCCESS )
		{
			//ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}

		goto _return; 
	}

	if( ( flags & INTERNET_CONNECTION_PROXY ) != 0 )
		hOpen = InternetOpenA( agent, INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, NULL, NULL, 0 );
	else
		hOpen = InternetOpenA( agent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	
	if( hOpen == NULL )
	{
		ret = GetLastError();  
		log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "internet open failed 0x%0.8x \n", GetLastError() ) ); 
		goto _return; 
	}

	hConnect = InternetOpenUrlA ( hOpen, url, szHead,
		lstrlenA( szHead ), INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0 ); 

	if ( hConnect == NULL )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "open url failed 0x%0.8x \n", GetLastError() ) ); 
		goto _return; 
	}

	_ret = HttpQueryInfo( hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
		( LPVOID )&dwByteToRead, &dwSizeOfRq, NULL ); 

    if( _ret == FALSE )
	{
		ret = GetLastError(); 

		log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "query http info failed 0x%0.8x \n", GetLastError() ) ); 

		dwByteToRead = 0;
	}

	file = CreateFileA( file_name, FILE_ALL_ACCESS, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL ); 

	if( file == INVALID_HANDLE_VALUE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "create the destination file failed\n" ) ); 
		goto _return; 
	}

	_ret = SetFilePointerEx( file, offset, &now_offset, SEEK_SET ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "set file pointer failed \n" ) ); 
		goto _return; 
	}

	_ret = SetEndOfFile( file ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "set file end failed \n" ) ); 
		goto _return; 
	}

	bytes_readed = 0; 
	try_time = 0; 
#define MAX_TRY_TIME 5

	if( download_tip_func != NULL )
	{
		download_tip_func( file_name, dwByteToRead, bytes_readed, 0, context ); 
	}

	do
	{
		_ret = InternetReadFile( hConnect, szTemp, HTTP_READ_TMP_BUF_LEN, &dwSize ); 

		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			
			log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "read data from internet failed \n" ) ); 

			if( ret == ERROR_INTERNET_TIMEOUT 
				|| ret == ERROR_INTERNET_CONNECTION_ABORTED 
				|| ret == ERROR_INTERNET_CONNECTION_RESET )
			{
				DWORD bytes_to_read; 
				DWORD move_to_pos; 

				if( try_time == MAX_TRY_TIME )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					goto _return; 
				}

				InternetCloseHandle( hConnect ); 
				hConnect = InternetOpenUrlA ( hOpen, url, szHead,
					lstrlenA( szHead ), INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0 ); 

				if ( hConnect == NULL )
				{
					ret = GetLastError(); 
					log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "open url failed 0x%0.8x \n", GetLastError() ) ); 
					goto _return; 
				}
				
				dwSizeOfRq = 4; 
				_ret = HttpQueryInfo( hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
					( LPVOID )&bytes_to_read, &dwSizeOfRq, NULL ); 

				if( _ret == FALSE )
				{
					ret = GetLastError(); 

					log_trace( ( DBG_INET_MSG_AND_ERROR_OUT, "query http info failed 0x%0.8x \n", GetLastError() ) ); 

					bytes_to_read = 0;
				}

				if( bytes_to_read != dwByteToRead )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					log_trace( ( MSG_ERROR, "file content length is not same as previous!\n" ) ); 
					goto _return; 
				}

				move_to_pos = InternetSetFilePointer( hConnect, bytes_readed, NULL, FILE_BEGIN, NULL ); 

				if( move_to_pos == ( DWORD )( -1 ) )
				{
					SAFE_SET_ERROR_CODE( ret ); 
					goto _return; 
				}

				if( move_to_pos == bytes_readed )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					goto _return; 
				}

				try_time ++; 
				continue; 
			}
			goto _return; 
		}

		if( dwSize == 0 )
		{
			if( bytes_readed < dwByteToRead )
			{
				ASSERT( FALSE && "data is not all readed " ); 
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "read data data length is smaller than the size requested\n" ) ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
			break;
		}
		else
		{

			//ERROR_HTTP_HEADER_NOT_FOUND
			_ret = WriteFile( file, szTemp, dwSize, &writed, NULL ); 
			if( _ret == FALSE )
			{
				ret = GetLastError(); 
				goto _return; 
			}
		
			if( writed < dwSize )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}
		}

		bytes_readed += dwSize; 

		if( download_tip_func != NULL )
		{
			download_tip_func( file_name, dwByteToRead, bytes_readed, 1, context ); 
		}

	}while (TRUE);

_return:
	
	if( hConnect != NULL )
	{
		InternetCloseHandle( hConnect ); 
	}

	if( hOpen != NULL )
	{
		InternetCloseHandle( hOpen ); 
	}

	if( file != INVALID_HANDLE_VALUE )
	{
		CloseHandle( file ); 
	}
	return ret;
}

LRESULT InternetGetInfo( LPCSTR url, LPSTR info, ULONG info_len, file_download_callback download_tip_func, PVOID context )
{
#define MAX_INTERNET_CONTENT_LEN ( 1024 * 512 ) 
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
    DWORD flags;
	//CHAR content = NULL; 
	CHAR strAgent[64]; 
	HINTERNET hOpen = NULL;
	HINTERNET hConnect = NULL;
	DWORD dwSize;
	CHAR  szHead[] = "Accept: */*\r\n\r\n";
	CHAR szTemp[HTTP_READ_TMP_BUF_LEN];
	DWORD dwByteToRead = 0;
	DWORD dwSizeOfRq = 4;
	DWORD dwBytes = 0;
	CHAR InfoTemp; 
	ULONG readed; 
	
	memset( strAgent, 0, sizeof( strAgent ) ); 
	sprintf( strAgent, "Agent%ld", timeGetTime() ); 

	_ret = InternetGetConnectedState( &flags, 0 ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	if( ( flags & INTERNET_CONNECTION_PROXY ) == NULL )
	{
		hOpen = InternetOpenA( strAgent, INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, NULL, NULL, 0 ); 
	}
	else
	{
		hOpen = InternetOpenA( strAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 ); 
	}
	
	if( hOpen == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	hConnect = InternetOpenUrlA ( hOpen, url, szHead, 
		lstrlenA( szHead ), INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0 ); 

	if( hConnect == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	_ret = HttpQueryInfo( hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
		( LPVOID )&dwByteToRead, &dwSizeOfRq, NULL ); 
    if(  ret == FALSE )
	{
		dwByteToRead = 0; 
		log_trace( ( MSG_WARNING, "http query info failed 0x%0.8x\n", GetLastError() ) ); 
	}

	memset( szTemp, 0, sizeof( szTemp ) ); 
	
	readed = 0; 

	if( download_tip_func != NULL )
	{
		download_tip_func( url, dwByteToRead, readed, 0, context ); 
	}

	do
	{
		_ret = InternetReadFile( hConnect, szTemp, HTTP_READ_TMP_BUF_LEN,  &dwSize ); 
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		if( dwSize == 0 )
		{
			info[ readed ] = '\0'; 
			break;
		}
		else
		{
			ASSERT( info_len - readed >= 0 ); 

			memcpy( info + readed, szTemp, ( info_len - readed ) >= dwSize ? dwSize : ( info_len - readed ) ); 

			if( ( info_len - readed ) < dwSize )
			{
				ret = ERROR_BUFFER_OVERFLOW; 
				goto _return; 
			}

			readed += ( info_len - readed ) >= dwSize ? dwSize : ( info_len - readed ); 
			
			if( download_tip_func != NULL )
			{
				download_tip_func( url, dwByteToRead, readed, 1, context ); 
			}
		}
		
	}while (TRUE);

_return:
	if( hConnect != NULL )
	{
		InternetCloseHandle( hConnect );
	}

	if( hOpen != NULL )
	{
		InternetCloseHandle( hOpen );
	}

	return ret;
}

#define VERSION_LEN 32
INT32 is_digit_str( LPCSTR str )
{
	INT32 index = 0; 
	INT32 ret = TRUE; 

	for( ; ; )
	{
		if( str[ index ] == '\0' )
		{
			break; 
		}

		if( isdigit( str[ index ] ) == FALSE )
		{
			ret = FALSE; 
			goto _return; 
		}

		index ++; 
	}

_return:
	return ret; 
}

BOOL TestVersion( LPCSTR old_ver, LPCSTR new_ver )
{
	BOOL ret = FALSE; 
    INT32 iOld,iNew; 
	LPCSTR old_sub_ver_begin; 
	LPCSTR new_sub_ver_begin; 
	LPCSTR old_sub_ver_end; 
	LPCSTR new_sub_ver_end; 
	CHAR strOldVer[ VERSION_LEN ]; 
	CHAR strNewVer[ VERSION_LEN ]; 
	INT32 found_end = FALSE; 

	ASSERT( old_ver != NULL ); 
	ASSERT( new_ver != NULL ); 

	old_sub_ver_begin = old_ver; 
	new_sub_ver_begin = new_ver; 

	for( ; ; )
	{
		old_sub_ver_end = strchr( old_sub_ver_begin, '.' ); 
		new_sub_ver_end = strchr( new_sub_ver_begin, '.' ); 

		if( old_sub_ver_end == NULL )
		{
			old_sub_ver_end = old_ver + strlen( old_ver ); 
			found_end = TRUE; 
		}
		
		if( new_sub_ver_end == NULL )
		{
			new_sub_ver_end = new_ver + strlen( new_ver ); 
			found_end = TRUE; 
		}
		
		memcpy( strOldVer, old_sub_ver_begin, old_sub_ver_end - old_sub_ver_begin ); 
		strOldVer[ old_sub_ver_end - old_sub_ver_begin ] = '\0'; 

		memcpy( strNewVer, new_sub_ver_begin, new_sub_ver_end - new_sub_ver_begin ); 
		strNewVer[ new_sub_ver_end - new_sub_ver_begin ] = '\0'; 

		if( is_digit_str( strOldVer ) == FALSE 
			|| is_digit_str( strNewVer ) == FALSE )
		{
			goto _return; 
		}

		iOld = atoi( strOldVer );
		iNew = atoi( strNewVer );

		if( iNew > iOld )
		{
			ret = TRUE; 
			break; 
		}
		else if( iNew == iOld )
		{
			new_sub_ver_begin = new_sub_ver_end + 1; 
			old_sub_ver_begin = old_sub_ver_end + 1; 
			if( found_end == TRUE )
			{
				break; 
			}
			continue; 
		}
		else
		{
			break; 
		}
	}

_return:
	return ret;
}

INT32 GetFileFullPath( LPSTR file_path, ULONG buf_len )
{
    INT32 ret; 
    
	if( GetModuleFileNameA( NULL, file_path, MAX_PATH ) == 0 )
	{
		ret = FALSE; 
	}
	else
	{
		ret = TRUE; 
	}

	return ret; 
}

typedef struct _file_download_info
{
	update_info *info; 
	update_tip_info *tip_info; 
} file_download_info, *pfile_download_info; 

LRESULT CALLBACK tip_file_download_info( LPCSTR file_name, ULONG max_len, ULONG downloaded, ULONG download_time, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	file_download_info *download_info; 
	updating_info report_info; 
	file_download_tip download_tip; 

	if( context == NULL )
	{
		ASSERT( FALSE && "must give download info context" ); 
	}

	download_info = ( file_download_info* )context; 

	ASSERT( download_info->info != NULL ); 
	ASSERT( download_info->tip_info != NULL ); 
	ASSERT( downloaded <= max_len ); 

	if( download_info->info->notify_func != NULL )
	{
		download_tip.download_time = download_time; 
		download_tip.file_size = max_len; 
		download_tip.file_downloaded = downloaded; 
		download_tip.file_name = file_name; 
		
		download_info->tip_info->context = &download_tip; 

		report_info.context = ( PVOID )download_info->tip_info; 
		report_info.update_status = UPDATE_FILE_DOWNLOAD_TIP; 

		download_info->info->notify_func( &report_info, download_info->info->param ); 
	}

	return ret; 
}

DWORD WINAPI thread_update( LPVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	TCHAR file_path[ MAX_PATH ]; 
	CHAR *_file_path = NULL; 
	LPCSTR version; 
	CHAR update_file_url[ _MAX_URL_LEN ]; 
	CHAR target_file[ MAX_PATH ]; 
	CHAR tmp_file[ MAX_PATH ]; 
	CHAR backup_file[ MAX_PATH ]; 
	CHAR *_target_file = NULL; 
	CHAR *target_dir; 
	CHAR *info; 
	ULONG info_len; 
	CHAR *ver_begin; 
	CHAR *ver_end; 
	update_info *update; 
	updating_info report; 
	TiXmlDocument update_conf; 
	TiXmlElement* confs; 
	TiXmlNode* conf; 
	ULONG file_count; 
	bool ret_val; 
	LPCSTR conf_file_name; 
	//LPWSTR _conf_file_name = NULL; 
	LPCSTR conf_server_path; 
	update_tip_info tip_info; 
	file_download_info download_info; 
	LPCTSTR tmp_text; 

#define UPDATE_TEMP_FILE _T( "data6.dat" )

	ASSERT( param != NULL ); 

	tmp_text = UPDATE_ERROR_TEXT; //_get_string_by_id( TEXT_UPDATE_UPDATE_FAILED, ); 

	tip_info.count = 0; 
	tip_info.msg = ""; 
	tip_info.url = ""; 
	tip_info.version = ""; 
	tip_info.err_msg = tmp_text; 
	tip_info.context = NULL; 
	
	info = ( CHAR* )malloc( MAX_INTERNET_CONTENT_LEN ); 
	if( info == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	update = ( update_info* )param; 
	ASSERT( update->url != NULL ); 
	if( *update->url == '\0' )
	{
		ASSERT( FALSE && "invalid update url configure \n" ); 

		tmp_text = BITTRACE_BIN_FILE_CORRUPT_TEXT; //_get_string_by_id( TEXT_UPDATE_CONFIGURATION_FILE_CORRUPTED,  ); 

		tip_info.err_msg = tmp_text; 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	tip_info.url = update->url; 

	ret = InternetGetInfo( update->url, info, MAX_INTERNET_CONTENT_LEN, NULL, NULL );

	if( ret != ERROR_SUCCESS )
	{
		tmp_text = _get_string_by_id( TEXT_UPDATE_GET_UPDATE_INFO_FAILED, ERROR_TIP_CANT_DOWNLOAD_UPDATE_FILE ); 

		tip_info.err_msg = tmp_text; 
		goto _return; 
	}

	if( update->notify_func != NULL )
	{
		tip_info.context = info; 
		report.context = ( PVOID )&tip_info; 
		report.update_status = UPDATE_PREPARE; 
		update->notify_func( &report, update->param ); 
	}

	//construct_file_path( target_file, MAX_PATH, update->target, conf_file_name );
	ret = add_app_path_to_file_name( file_path, MAX_PATH, UPDATE_TEMP_FILE, FALSE ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( *info == '\0' 
		|| strcmp( info, "Error" ) == 0 )
	{
		tmp_text = _get_string_by_id( TEXT_UPDATE_UPDATE_SERVER_MAINTAINING, ERROR_TIP_SERVICE_MAINTAIN ); 

		tip_info.err_msg = tmp_text; 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	_file_path = StringConvertor::WideToAnsi( file_path ); 
	if( _file_path == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ret = InternetGetFile( info, _file_path, NULL, NULL ); 
	if( ret != ERROR_SUCCESS )
	{
		tip_info.err_msg = ERROR_TIP_SERVICE_MAINTAIN ; 
		goto _return; 
	}

	ret_val = update_conf.LoadFile( _file_path ); 
	if( ret_val == false )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	confs = update_conf.RootElement(); 

	ASSERT( confs != NULL ); 

	if( strcmp( confs->Value(), "files" ) != 0 )
	{
		tmp_text = _get_string_by_id( TEXT_UPDATE_UPDATE_SERVER_CONFIG_ERROR, ERROR_TIP_UPDATE_FILE_ERROR ); 

		tip_info.err_msg = tmp_text; 

		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	version = confs->ToElement()->Attribute( "version");	
	if( version == NULL )
	{
		tip_info.err_msg = ERROR_TIP_UPDATE_FILE_ERROR ; 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	tip_info.version = version; 

	tip_info.msg = confs->ToElement()->Attribute( "message" ); 
	if( tip_info.msg == NULL )
	{
		tip_info.err_msg = ERROR_TIP_UPDATE_FILE_ERROR ; 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	_ret = TestVersion( BITSAFE_VERSION, version ); 
	if( _ret == FALSE )
	{
		tmp_text = _get_string_by_id( TEXT_UPDATE_ALREADY_NEWEST_VERSION, DONT_NEED_UPDATE_TEXT ); 

		tip_info.err_msg = tmp_text; 
		if( update->notify_func != NULL )
		{
			report.context = ( PVOID )&tip_info; 
			report.update_status = UPDATE_NO_NEED; 
			update->notify_func( &report, update->param ); 
		}

		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	file_count = confs->child_count(); 

	tip_info.count = file_count; 

	if( update->notify_func != NULL )
	{
		report.context = ( PVOID )&tip_info; 
		report.update_status = UPDATE_RUNNING; 
		ret = update->notify_func( &report, update->param ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

	conf = confs->FirstChild();

	download_info.tip_info = &tip_info; 
	download_info.info = update; 

	while( conf )
	{
		if( conf->Type() != TiXmlNode::ELEMENT )
		{
			continue; 
		}

		if( strcmp( conf->Value(), "file" ) != 0 )
		{
			continue; 
		}

		conf_file_name = conf->ToElement()->Attribute("name");
		conf_server_path = conf->ToElement()->Attribute( "server" ); 
		if( conf_file_name == NULL || conf_server_path == NULL )
		{
			tip_info.err_msg = ERROR_TIP_UPDATE_FILE_ERROR ; 
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}

#define HTTP_PROT_PREFIX "http://"
		if( strncmp( conf_server_path, HTTP_PROT_PREFIX, CONST_STR_LEN( HTTP_PROT_PREFIX ) ) !=  0 )
		{
			strcpy( update_file_url, "http://" ); 
		}
		else
		{
			*update_file_url = '\0'; 
		}

		strncat( update_file_url, conf_server_path, _MAX_URL_LEN - strlen( update_file_url ) ); 
		if( update_file_url[ strlen( update_file_url ) - 1 ] != '/' )
		{
			strncat( update_file_url, "/", _MAX_URL_LEN - strlen( update_file_url ) ); 
		}

		strncat( update_file_url, conf_file_name, _MAX_URL_LEN - strlen( update_file_url ) ); 

		_ret = construct_file_path_a( target_file, MAX_PATH, update->target, conf_file_name ); 
		if( _ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		if( update->notify_func != NULL )
		{
			tip_info.context = ( PVOID )target_file; 
			report.context = ( PVOID )&tip_info; 
			report.update_status = UPDATE_FILE_STARGING; 
			update->notify_func( &report, update->param ); 
		}

		_ret = GetTempPathA( _MAX_PATH, tmp_file ); 
		if( _ret == 0 )
		{
			ret = GetLastError(); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "get tmp file name failed \n" ) ); 

			goto _return;
		}

		_ret = GetTempFileNameA( tmp_file, "tmp", 0, tmp_file ); 

		if( _ret == 0 )
		{
			ret = GetLastError(); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "get tmp file name failed \n" ) ); 
			goto _return; 
		}

		log_trace( ( MSG_INFO, "get file from %s to %s \n", update_file_url, tmp_file ) ); 

		ret = InternetGetFile( update_file_url, tmp_file, tip_file_download_info, &download_info ); 
		if( ret != ERROR_SUCCESS )
		{
			tip_info.err_msg = ERROR_TIP_CANT_DOWNLOAD_UPDATE_FILE; 
			goto _return; 
		}

		strcpy( backup_file, target_file ); 
		strncat( backup_file, ".backup", MAX_PATH - strlen( backup_file ) ); 
		_ret = MoveFileA( target_file, backup_file ); 
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		_ret = MoveFileA( tmp_file, target_file ); 
		if( _ret == FALSE )
		{

			log_trace( ( DBG_MSG_AND_ERROR_OUT, "move the new version file failed \n") ); 
			_ret = DeleteFileA( tmp_file ); 
			if( _ret == FALSE )
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "delete the temp file %s failed \n", tmp_file ) ); 
			}
			
			_ret = DeleteFileA( target_file ); 
			if( _ret == FALSE )
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "delete the incorrect target file %s failed\n", target_file ) ); 
			}
			
			_ret = MoveFileA( backup_file, target_file ); 
			if( _ret == FALSE )
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "restore the bakcup file failed \n") ); 
			}
			ret = GetLastError(); 
			goto _return; 
		}

		_ret = DeleteFileA( backup_file ); 
		if( _ret == FALSE )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "delete the backup file failed \n") ); 
		}
			
		if( update->notify_func != NULL )
		{
			tip_info.context = ( PVOID )target_file; 
			report.context = ( PVOID )&tip_info; 
			report.update_status = UPDATE_FILE_END; 

			ret = update->notify_func( &report, update->param ); 
			if( ret != ERROR_SUCCESS )
			{
				//tip_info.err_msg = L"�û�ȡ������"; 
				goto _return; 
			}
		}

		conf = conf->NextSibling(); 
	}

_return:
	if( info != NULL )
	{
		free( info ); 
	}
	
	if( ret != ERROR_SUCCESS )
	{
		if( update->notify_func != NULL )
		{
			report.context = ( PVOID )&tip_info; 
			report.update_status = UPDATE_FAILED; 
			update->notify_func( &report, update->param ); 
		}
	}
	else
	{
		if( update->notify_func != NULL )
		{
			report.context = ( PVOID )&tip_info; 
			report.update_status = UPDATE_SUCCESSFUL; 
			update->notify_func( &report, update->param ); 
		}
	}

	if( _file_path != NULL )
	{
		StringConvertor::StringFree( _file_path ); 
	}

	ExitThread( 0 );

	return 0;
}

PVOID start_auto_update( LPCSTR url, LPCSTR target_path, update_status_notify notify_func, PVOID param )
{
	PVOID update_handle; 

	static update_info info = { 0 }; 

	ASSERT( url != NULL ); 

	strncpy( info.url, url, _MAX_URL_LEN ); 
	strncpy( info.target, target_path, MAX_PATH ); 

	info.notify_func = notify_func; 
	info.param = param; 

	update_handle = ( PVOID )CreateThread( NULL, 0, thread_update, ( PVOID )&info, 0, NULL ); 
	
	return update_handle; 
}

LRESULT wait_update_end( PVOID update_handle )
{
	DWORD wait_ret; 
	LRESULT ret = ERROR_SUCCESS; 
	ASSERT( update_handle != NULL ); 

	wait_ret = WaitForSingleObject( ( HANDLE )update_handle, INFINITE ); 

	if( wait_ret == WAIT_FAILED )
	{
		log_trace( ( MSG_ERROR, "wait the update thread end failed \n" ) ); 
		ret = GetLastError(); 
	}

	GetExitCodeThread( ( HANDLE )update_handle, &wait_ret ); 

	ret = wait_ret; 

_return:
	CloseHandle( update_handle ); 
	return ret; 
}

