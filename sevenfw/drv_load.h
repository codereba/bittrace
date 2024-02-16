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

#ifndef __DRV_LOAD_H__
#define __DRV_LOAD_H__

#ifdef __cplusplus
extern "C"
{
#endif 

	LRESULT test_load_device( LPCTSTR dev_name ); 

	LRESULT load_drv_dev( HANDLE *drv_load, LPCTSTR dev_file_name, LPCTSTR driver_name, DWORD flags ); 
	LRESULT install_driver( LPCTSTR dev_file_name, LPCTSTR file_name, LPCTSTR driver_name, ULONG driver_file_res, DWORD flags ); 
	LRESULT safe_install_driver( LPCTSTR dev_file_name, 
		LPCTSTR file_name, 
		LPCTSTR driver_name, 
		ULONG driver_file_res, 
		DWORD flags ); 

	LRESULT WINAPI install_fs_mini_filter_driver( LPCTSTR dev_file_name, 
		LPCTSTR file_name, 
		LPCTSTR driver_name, 
		ULONG driver_file_res, 
		DWORD flags ); 

	LRESULT _uninstall_driver( LPCTSTR driver_name, DWORD flags ); 

#ifdef __cplusplus
}
#endif

#endif //__DRV_LOAD_H__