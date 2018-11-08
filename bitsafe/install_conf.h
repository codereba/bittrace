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

#ifndef __INSTALL_CONF_H__
#define __INSTALL_CONF_H__

typedef enum _file_install_action
{
	FILE_INSTALL_DELETE = 0x01, 
	FILE_INSTALL_REPLACE = 0x02, 
	FILE_INSTALL_UNINSTALL = 0x04, 
} file_install_action, *pfile_install_action; 
#define FILE_INSTALL_ACTION_MASK ( FILE_INSTALL_DELETE | FILE_INSTALL_REPLACE | FILE_INSTALL_UNINSTALL )

typedef enum _prepare_work_action
{
	PREPARE_NONE, 
	PREPARE_BY_RESTART, 
	PREPARE_BEGIN = PREPARE_NONE, 
	PREPARE_END = PREPARE_BY_RESTART, 
} prepare_work_action, *pprepare_work_action; 

#define is_valid_prepare_action( action ) ( action >= PREPARE_BEGIN && action <= PREPARE_END )
#define is_valid_install_action( action ) ( 0 == ( action & ~FILE_INSTALL_ACTION_MASK ) )

#define COMPARE_EQUAL 1
#define COMPARE_GREATER 2
#define COMPARE_LOWER 4

#define is_valid_compare_mode( mode ) ( mode == COMPARE_EQUAL || mode == COMPARE_GREATER || mode == COMPARE_LOWER )

#define SERVICE_NAME_LEN 100
#define MAX_FILE_VERSION_LEN 64
#define MAX_MD5_STR_LEN 32

typedef struct _file_install_conf
{
	file_install_action true_install_action; 
	prepare_work_action true_prepare_action; 
	file_install_action false_install_action; 
	prepare_work_action false_prepare_action; 
	WCHAR file_name[ MAX_PATH ]; 
	WCHAR update_name[ MAX_PATH ]; 
	WCHAR service_name[ SERVICE_NAME_LEN ]; 
	WCHAR file_version[ MAX_FILE_VERSION_LEN ]; 
	CHAR md5[ MAX_MD5_STR_LEN ]; 
	ULONG compare_type; 
} file_install_conf, *pfile_install_conf; 


#define MAX_RES_TYPE_LEN 64
typedef struct _install_conf_context
{
	BOOLEAN prepare_restart_count; 
	BOOLEAN prepare_none_count; 
	WCHAR dest_dir[ MAX_PATH ]; 
	WCHAR src_dir[ MAX_PATH ]; 
	WCHAR res_type[ MAX_RES_TYPE_LEN ]; 
}install_conf_context, *pinstall_conf_context; 

LRESULT get_file_version( LPCWSTR file_name, ULARGE_INTEGER *file_version ); 
LRESULT _check_file_version( LPCWSTR file_name, ULARGE_INTEGER file_version, ULONG flags, BOOLEAN *result ); 
LRESULT check_file_version( LPCWSTR file_name, LPCWSTR file_version, ULONG flags, BOOLEAN *result ); 
LRESULT do_def_action( LPCWSTR file_name ); 
LRESULT uninstall_service_w( LPCWSTR service_name ); 
LRESULT install_file_by_conf( file_install_conf *conf, LPCWSTR src_dir, LPCWSTR dest_dir, prepare_work_action *prepare_action ); 
LRESULT set_must_restart_mark(); 
LRESULT check_must_restart_mark(); 
LRESULT del_must_restart_mark(); 
LRESULT must_restart(); 
LRESULT on_file_install( file_install_conf *conf, LPCWSTR src_dir, LPCWSTR dest_dir, BOOLEAN result ); 
LRESULT check_file_sign( LPCWSTR file_name ); 

typedef LRESULT ( CALLBACK* on_install_conf_loaded )( file_install_conf *conf, PVOID context ); 


LRESULT CALLBACK on_bitsafe_install_conf_loaded( file_install_conf *conf, PVOID context ); 
LRESULT load_install_conf( on_install_conf_loaded conf_loaded_callback, PVOID context ); 
LRESULT prepare_installation( install_conf_context *context ); 
LRESULT uninstall_bitsafe_by_conf_file( ULONG *prepare_work ); 
LRESULT do_install_prepare_work(); 
LRESULT init_uninstall_context( LPCWSTR dest_dir, LPCWSTR src_dir, LPCWSTR res_type ); 

#endif //__INSTALL_CONF_H__