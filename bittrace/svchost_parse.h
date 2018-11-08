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
#ifndef __SVCHOST_PARSE_H__
#define __SVCHOST_PARSE_H__

#include <vector>
using namespace std; 

#define _MAX_SERVICE_NAME_LEN        256

typedef struct _PROCESS_SHARED_SERVICE_INFORMATION
{
	WCHAR service_dll[ MAX_PATH ]; 
	WCHAR image_path[ MAX_PATH ]; 
	WCHAR name[ _MAX_SERVICE_NAME_LEN ]; 
} PROCESS_SHARED_SERVICE_INFORMATION, *PPROCESS_SHARED_SERVICE_INFORMATION; 

LRESULT WINAPI get_all_process_shared_service( vector< PROCESS_SHARED_SERVICE_INFORMATION* > *service_infos ); 

LRESULT WINAPI release_process_shared_service( vector< PROCESS_SHARED_SERVICE_INFORMATION* > *service_infos ); 

LRESULT WINAPI find_process_shared_service( LPCWSTR service_name ); 

#endif //__SVCHOST_PARSE_H__