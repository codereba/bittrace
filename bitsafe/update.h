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
typedef struct _updating_info
{
	PVOID context; 
	ULONG update_status; 
}updating_info, *pupdating_info; 

typedef LRESULT ( CALLBACK *update_status_notify )( updating_info *info, PVOID param ); 

typedef struct _file_download_tip
{
	LPCSTR file_name; 
	ULONG file_downloaded; 
	ULONG file_size; 
	ULONG download_time; 
} file_download_tip, *pfile_download_tip; 

typedef struct _update_tip_info
{
	ULONG count; 
	PVOID context; 
	LPCSTR version; 
	LPCSTR msg; 
	LPCSTR url; 
	LPCWSTR err_msg; 
} update_tip_info, *pupdate_tip_info; 

typedef struct _update_info
{
	CHAR url[ _MAX_URL_LEN ]; 
	CHAR target[ _MAX_PATH ]; 
	PVOID param; 
	update_status_notify notify_func; 
} update_info, *pupdate_info; 

#define UPDATE_PREPARE 1
#define UPDATE_RUNNING 6
#define UPDATE_SUCCESSFUL 2
#define UPDATE_FAILED 3
#define UPDATE_FILE_STARGING 4
#define UPDATE_FILE_END 5
#define UPDATE_NO_NEED 7
#define UPDATE_FILE_DOWNLOAD_TIP 8

typedef LRESULT ( CALLBACK *file_download_callback )( LPCSTR file_name, ULONG max_len, ULONG downloaded, ULONG download_time, PVOID context ); 

LRESULT InternetGetFile( LPCSTR url, LPCSTR file_name, file_download_callback download_tip_func, PVOID context ); 
LRESULT InternetGetInfo( LPCSTR url, LPSTR info, ULONG info_len, file_download_callback download_tip_func, PVOID context ); 

#define VERSION_LEN 32
INT32 is_digit_str( LPCSTR str ); 
BOOL TestVersion( LPCSTR local_ver, LPCSTR remote_ver ); 
INT32 GetFileFullPath( LPSTR file_path, ULONG buf_len ); 
PVOID start_auto_update( LPCSTR url, LPCSTR target_path, update_status_notify notify_func, PVOID param ); 

LRESULT wait_update_end( PVOID update_handle ); 

