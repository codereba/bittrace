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