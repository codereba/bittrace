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

