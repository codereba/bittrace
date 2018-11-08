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

#include "common_func.h"
#include "wdk_macros.h"
#include "trace_log_api.h"
#include "seven_fw_api.h"
#include "sys_mng_api.h"
#include "antiarp_api.h"
#include "fs_mng_api.h"
#include "tdifw_api.h"
#include "inst_drv.h"
#include "net_svc_inst.h"
#include "drv_load.h"
#include "inst_im.h"
#include "ui_ctrl.h"
#include "resource.h"
#include "cbuffer.h"
#include "unit_cbuffer.h"
#include "buf_array.h"
#include "local_strings.h"
#include "mini_port_dev.h"
#include "action_log_db.h"
#include "event_memory_store.h"
#include "bitsafe_svc.h"

#define REMOVE_TRACE_FROM_NDIS 1
#define MAX_ACTION_LOG_NUM_ONCE 30
#define MAX_ACTION_LOG_BUF_LEN ( sizeof( sys_log_output ) + MAX_ACTION_LOG_NUM_ONCE * sizeof( sys_log_unit ) )

#include "fltUser.h"

#pragma comment( lib, "fltLib.lib" ) 

#define UNKNOWN_RESULT ( BOOLEAN )( -1 )

ULONG all_bitsafe_function_flags[ MAX_FUNCTION_TYPE ] = { 0 }; 

LRESULT load_driver_work_sets( ULONG flags, PVOID ui_context, PVOID work_context ); 

LRESULT WINAPI get_driver_version( ULARGE_INTEGER *driver_version ); 

/******************************************************************************************
驱动的兼容性检查的方案:
1.通过IO CTL码来得到DRIVER的版本号
2.进行版本号检查，必须与应用层相应的版本号一致才能运行,否则通知用户机器重启后才能运行,进行卸载。
3.驱动的兼容版本号只有在与应用层的通信方式或内容发生变化时，才改变。
******************************************************************************************/

ULARGE_INTEGER compatible_driver_version = { ( ( 0 ) << 16 | 8906 ), 0 }; 

#define FACILITY_BITTRACE 0x8010000
#define ERROR_CODE_DRIVER_VERION_ERROR 0x0000901
#define ERROR_DRIVER_VERSION_ERROR ( 0xE0000000 | FACILITY_BITTRACE | ERROR_CODE_DRIVER_VERION_ERROR )

LRESULT WINAPI check_driver_version( )
{
	LRESULT ret = ERROR_SUCCESS;  
	ULARGE_INTEGER driver_version; 

	do 
	{
		ret = get_driver_version( &driver_version ); 
		if( ret != ERROR_SUCCESS )
		{
			ret = ERROR_SUCCESS; 
			break; 
		}

		if( driver_version.HighPart != compatible_driver_version.HighPart 
			&& driver_version.LowPart != compatible_driver_version.LowPart )
		{
			ret = ERROR_DRIVER_VERSION_ERROR; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT ctrl_all_drv_load( ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	for( i = 0; i < ARRAY_SIZE( all_bitsafe_function_flags ); i ++ )
	{
		all_bitsafe_function_flags[ i ] = flags; 
	}

	return ret; 
}

LRESULT unload_driver_work_sets( BOOLEAN ui_action ); 

FORCEINLINE LRESULT stop_all_function_driver_load()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	for( i = 0; i < ARRAY_SIZE( all_bitsafe_function_flags ); i ++ )
	{
		all_bitsafe_function_flags[ i ] = BITSAFE_FUNCTION_DISABLED; 
	}

	all_bitsafe_function_flags[ TRACE_LOG_FUNCTION ] = BITSAFE_FUNCTION_OPENED; 

	return ret; 
}

FORCEINLINE LRESULT stop_all_driver_load()
{
	return ctrl_all_drv_load( BITSAFE_FUNCTION_DISABLED ); 
}

FORCEINLINE LRESULT start_all_driver_load()
{
	return ctrl_all_drv_load( BITSAFE_FUNCTION_OPENED ); 
}

LRESULT dbg_show_msg( HWND parent, LPCTSTR msg, LPCTSTR title = NULL, ULONG mode = 0 ); 
LRESULT request_user_action( PVOID param ); 
LRESULT set_action_notify_event( notify_events_set *all_events, ULONG input_len ); 
LRESULT ctrl_driver_work_state( bitsafe_function_type type, bitsafe_function_state load_state ); 
LRESULT load_driver_work_state( bitsafe_function_type type ); 
LRESULT ctrl_drivers_work_state(); 

#define HTTP_TEXT_FLT_OUTPUT_WND 0x00020001
#define HTTP_TEXT_FLT_SET_FLT_TEXT 0x00020001
#define HTTP_TEXT_FLT_GET_FLT_TEXT 0x00020002

#define HTTP_URL_FLT_OUTPUT_WND 0x00030001
#define HTTP_URL_FLT_SET_FLT_URL 0x00030001
#define HTTP_URL_FLT_GET_FLT_URL 0x00030002

#define NETMON_OUTPUT_WND 0x00010001
#define NETMON_SET_BLOCK_PROCESS 0x00010001
#define NETMON_GET_ALL_PROCESS_INFO 0x00010002

#define SYS_ACT_MNG_OUTPUT_WND 0x00040001
#define SYS_ACT_MNG_GET_SYS_ACTION 0x00040001

#define ADD_TEXT 1
#define DEL_TEXT 0

#define DEBUG_WITH_DEV 1

ULONG text_type_from_desc( CHAR *desc ); 
CHAR *get_text_type_desc( ULONG type ); 

#define HTTP_URL_FLT_FILE_NAME _T( "http_url_fw.sys" )
#define HTTP_URL_FLT_SVC_NAME _T( "HttpUrlFlt" )

#define SEVENFW_DEV_NAME _T( "\\\\.\\SevenFW" )
#define HTTP_TXT_FLT_FILE_NAME _T( "http_txt_fw.sys" )
#define HTTP_TXT_FLT_SVC_NAME _T( "HttpTxtFlt" )

#define ANTI_ARP_DEVICE_NAME _T( "\\\\.\\AntiArp0208" )
#define ANTI_ARP_FILE_NAME _T( "antiarp.sys" )
#define ANTI_ARP_SVC_NAME _T( "BCAntiARP" )

#define ARP_HOST_MAC_LISTS_BUFF_LEN ( ULONG )( 1024 * 300 )

#define SEVEN_FW_SYS_FILE_NAME _T( "sevenfw.sys" )
#define SEVEN_FW_INF_FILE_NAME _T( "sevenfw.inf" )
#define SEVEN_FW_M_INF_FILE_NAME _T( "sevenfw_m.inf" )

#define SEVEN_FW_EX_INF_FILE_NAME SEVEN_FW_INF_FILE_NAME 
#define SEVEN_FW_EX_SYS_FILE_NAME SEVEN_FW_SYS_FILE_NAME 

#define SEVEN_FW_EX_SERVICE_NAME _T( "Sevenfw" ) 
#define SEVEN_FW_EX_DEVICE_NAME _T( "\\\\.\\Sevenfw" )

#define NETMON_DEVICE_NAME _T( "\\\\.\\BCTdiFW" )

#define MNG_DEVICE_NAME _T( "\\\\.\\KrnlMgr" )

#define FS_MNG_DEVICE_NAME _T( "\\\\.\\fs_mng" )

#define NET_FW_DEVICE_NAME _T( "\\\\.\\NetFw" )

#define REG_MNG_DEVICE_NAME _T( "\\\\.\\RegMng" )

#define EVENT_MON_INF_FILE_NAME _T( "eventmon.inf" )
#define EVENT_MON_SVC_NAME _T( "EventMon" )
#define EVENT_MON_SYS_FILE_NAME _T( "eventmon.sys" )

#define BITSAFE_UI_TRACE_LOG_MARK L"Global\\BITSAFE_UI_LOG"

#define BITSAFE_TRACE_LOG_EVENT L"Global\\BITSAFE_LOG"
#define ALL_FILTER_NAME_BUFF_LEN ( 1024 * 100 ) 

#define MAX_HOOK_INFO_SIZE ( sizeof( HOOK_FILTER_INFO ) + sizeof( ULONG ) * NtApiFilterEnd )
#define MAX_TRACE_INFO_LEN ( ULONG )( 1024 * 10 )

HANDLE anti_arp_dev = NULL; 

HANDLE reg_mng_dev = NULL; 
HANDLE trace_log_dev = NULL; 
HANDLE seven_fw_dev = NULL; 
HANDLE net_mon_dev = NULL; 
HANDLE fs_mng_dev = NULL; 
HANDLE http_txt_flt_dev = NULL; 
HANDLE http_url_flt_dev = NULL; 

HANDLE log_notifier = NULL; 
HANDLE ui_log_exist = NULL; 

CRITICAL_SECTION sys_event_lock; 


ndis_drv_installer Installer;

LIST_ENTRY all_ui_ctrls; 
CRITICAL_SECTION ui_ctrls_lock; 

#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus

LRESULT init_bitsafe_function_state(); 
LRESULT uninit_bitsafe_function_state(); 
LRESULT check_bitsafe_function_state( bitsafe_function_type type, bitsafe_function_state *load_state ); 

#ifdef __cplusplus
}
#endif //__cplusplus

LRESULT install_sevenfw()
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	DBG_BP(); 

	ret = file_exists( SEVEN_FW_SYS_FILE_NAME, TRUE ); 

	if( ret != ERROR_SUCCESS )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVEN_FW_FILE ), DAT_FILE_RES_TYPE, SEVEN_FW_SYS_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw file from resource failed \n" ) ); 
			goto _return; 
		}
	}

	ret = file_exists( SEVEN_FW_INF_FILE_NAME, TRUE ); 
	if( ERROR_SUCCESS != ret )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVEN_FW_INF ), DAT_FILE_RES_TYPE, SEVEN_FW_INF_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw inf file from resource failed \n" ) ); 
			goto _return; 
		}
	}

	ret = file_exists( SEVEN_FW_M_INF_FILE_NAME, TRUE ); 
	if( ERROR_SUCCESS != ret )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVEN_FW_M_INF ), DAT_FILE_RES_TYPE, SEVEN_FW_M_INF_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw m inf file from resource failed \n" ) ); 
			goto _return; 
		}
	}

	Installer.install_ndis_drv( 0 ); 

_return: 
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret;
}

LRESULT delete_event_mon_driver_file()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		_ret = delete_drv_file( EVENT_MON_INF_FILE_NAME, TRUE ); 
		if( ERROR_SUCCESS != _ret )
		{
			ret = _ret; 
			log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw inf file from resource failed \n" ) ); 
			//break; 
		}

		_ret = delete_drv_file( EVENT_MON_SYS_FILE_NAME, TRUE ); 
		if( ERROR_SUCCESS != _ret )
		{
			ret = _ret; 
			log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw inf file from resource failed \n" ) ); 
			//break; 
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT install_driver_from_inf( LPCWSTR inf_file_name, 
								LPCWSTR sys_file_name, 
								LPCWSTR service_name, 
								ULONG inf_file_res, 
								ULONG sys_file_res )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR file_name[ MAX_PATH ]; 
	BOOLEAN wow64_disabled = FALSE; 
	PVOID old_wow_value = NULL; 

#ifdef _WIN32
	{
		LRESULT _ret; 
		_ret = safe_disable_wow64( &old_wow_value ); 
		if( _ret == ERROR_SUCCESS )
		{
			wow64_disabled = TRUE; 
		}
	}
#endif //_WIN32


#define INVALID_RES_ID ( ULONG )( -1 )

	do 
	{
		ret = file_exists( inf_file_name, TRUE ); 
		if( ERROR_SUCCESS != ret )
		{
			ret = create_file_from_res( NULL, 
				MAKEINTRESOURCE( inf_file_res ), 
				DAT_FILE_RES_TYPE, 
				inf_file_name, TRUE ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw inf file from resource failed \n" ) ); 
				break; 
			}
		}

		ret = file_exists( sys_file_name, TRUE ); 
		if( ERROR_SUCCESS != ret )
		{
			ret = create_file_from_res( NULL, 
				MAKEINTRESOURCE( sys_file_res ), 
				DAT_FILE_RES_TYPE, 
				sys_file_name, 
				TRUE ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw m inf file from resource failed \n" ) ); 
				break; 
			}
		}

		ret = add_sys_drv_path_to_file_name( file_name, MAX_PATH, inf_file_name, FALSE ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ret = setup_drv_from_inf( file_name, NULL ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ret = _start_drv( service_name ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

	}while( FALSE ); 

#ifdef _WIN32
	if( wow64_disabled == TRUE )
	{
		revert_wow_64( old_wow_value ); 
	}
#endif //_WIN32

	return ret; 
}


LRESULT uninstall_driver_from_inf( LPCWSTR inf_file_name, 
								  ULONG inf_file_res )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR file_name[ MAX_PATH ]; 

	BOOLEAN wow64_disabled = FALSE; 
	PVOID old_wow_value = NULL; 

	do 
	{
		ASSERT( inf_file_name != NULL ); 

#ifndef _WIN64
		{
			LRESULT _ret; 
			_ret = safe_disable_wow64( &old_wow_value ); 
			if( _ret == ERROR_SUCCESS )
			{
				wow64_disabled = TRUE; 
			}
		}
#endif //_WIN64

		ret = file_exists( inf_file_name, TRUE ); 
		if( ERROR_SUCCESS != ret )
		{
			ret = create_file_from_res( NULL, 
				MAKEINTRESOURCE( inf_file_res ), 
				DAT_FILE_RES_TYPE, 
				inf_file_name, 
				TRUE ); 

			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw inf file from resource failed \n" ) ); 
				break; 
			}
		}

		ret = add_sys_drv_path_to_file_name( file_name, MAX_PATH, inf_file_name, FALSE ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ret = setup_drv_from_inf( file_name, DEF_UNINSTALL_SECTION_NAME ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}
	}while( FALSE ); 

#ifndef _WIN64
	if( wow64_disabled == TRUE )
	{
		revert_wow_64( old_wow_value ); 
	}
#endif //_WIN64

	return ret; 
}

extern INT32 os_bit; 
extern INT32 os_version; 

ULONG WINAPI get_sys_file_resource_id( INT32 local_os_bit, INT32 local_os_version )
{
	ULONG file_res_id = 0; 
	
	switch( local_os_version )
	{
	case OSWIN98:
	case OSWIN2000:
		log_trace( ( MSG_FATAL_ERROR, "bittrace do not support windows lower than windows xp %u\n", local_os_version ) ); 
		break; 
	case OSWINXP: 
		file_res_id = ( os_bit == 32 ) ? IDR_EVENT_MON_WINXP_X86_FILE : 0; //IDR_EVENT_MON_WINXP_X64_FILE; 
		break; 
	case OSWIN2003:
	case OSWIN2003R2: 
		file_res_id = ( os_bit == 32 ) ? IDR_EVENT_MON_WIN2K3_X86_FILE : IDR_EVENT_MON_WIN2K3_X86_FILE; //IDR_EVENT_MON_WINXP_X64_FILE; 
		break; 

	case OSWINVISTA: 
	case OSWIN2008: 
		file_res_id = ( os_bit == 32 ) ? IDR_EVENT_MON_WIN2K8_X86_FILE : IDR_EVENT_MON_WIN2K8_X64_FILE; 
		break; 

	case OSWIN2008R2: 
	case OSWIN7: 
	case OSWIN8: 
	case OSWIN2012: 
		file_res_id = ( os_bit == 32 ) ? IDR_EVENT_MON_WIN7_X86_FILE : IDR_EVENT_MON_WIN7_X64_FILE; 
		break; 

	case OSOTHER:
	case OSINVALID:
		log_trace( ( MSG_FATAL_ERROR, "unknown windows version %u\n", local_os_version ) ); 
		break; 
	}

	return file_res_id; 
}

LRESULT WINAPI check_version_compatible()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret;  
}

LRESULT install_event_mon_drv()
{
	ULONG res_file_id; 
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		res_file_id = get_sys_file_resource_id( os_bit, os_version ); 

		if( res_file_id == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = install_fs_mini_filter_driver( NULL, 
			EVENT_MON_SYS_FILE_NAME, 
			EVENT_MON_SVC_NAME, 
			res_file_id, 
			DRIVER_INSTALL_DO_NOT_START | DRIVER_MANUAL_START | DRIVER_IN_SYSTEM_DIR | DO_NOT_AUTO_UNINSTALL_DRIVER ); 

	}while( FALSE );

	return ret; 
}

LRESULT uninstall_event_mon_drv_from_inf()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = uninstall_driver_from_inf( EVENT_MON_INF_FILE_NAME, IDR_FS_MNG_INF_FILE ); 

	return ret; 
}

LRESULT uninstall_event_mon_drv()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = _uninstall_driver( EVENT_MON_SVC_NAME, DRIVER_UNINSTALL_DO_NOT_STOP ); 

	return ret; 
}

LRESULT safe_install_event_mon_drv_from_inf()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = install_event_mon_drv(); 
		if( ret != ERROR_SUCCESS )
		{
			LRESULT _ret; 

			_ret = delete_event_mon_driver_file(); 

			if( _ret != ERROR_SUCCESS )
			{

				log_trace( ( DBG_DBG_BUF_OUT, "delete the fs manage driver files error 0x%0.8x\n", ret ) ); 
				ASSERT( FALSE && "delete the fs manage driver files error " ); 
			}

			ret = install_event_mon_drv(); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT install_sevenfw_ex()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	BOOLEAN wow64_disabled = FALSE; 
	PVOID old_wow_value = NULL; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 


#ifdef _WIN32
	{
		LRESULT _ret; 
		_ret = safe_disable_wow64( &old_wow_value ); 
		if( _ret == ERROR_SUCCESS )
		{
			wow64_disabled = TRUE; 
		}
	}
#endif //_WIN32

#ifdef DELAY_WIN7_SUPPORT
	ret = install_sevenfw(); 
#else

	ret = file_exists( SEVEN_FW_EX_SYS_FILE_NAME, TRUE ); 

	if( ret != ERROR_SUCCESS )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVENFW_EX_FILE ), DAT_FILE_RES_TYPE, SEVEN_FW_EX_SYS_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw file from resource failed \n" ) ); 
			goto _return; 
		}
	}

	ret = file_exists( SEVEN_FW_EX_INF_FILE_NAME, TRUE ); 
	if( ERROR_SUCCESS != ret )
	{
		ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVENFW_EX_INF_FILE ), DAT_FILE_RES_TYPE, SEVEN_FW_EX_INF_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw inf file from resource failed \n" ) ); 
			goto _return; 
		}
	}

	ret = Installer.install_ndis_drv( INSTALL_SEVENFW_WIN7_DRV );
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "install sevenfwex driver error 0x%0.8x\n", ret ) ); 
		//goto _return; 
	}

	_ret = delete_drv_file( SEVEN_FW_EX_INF_FILE_NAME, TRUE ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!delete the driver file failed \n" ) ); 
	}
#endif //DELAY_WIN7_SUPPORT

_return: 

#ifdef _WIN32
	if( wow64_disabled == TRUE )
	{
		revert_wow_64( old_wow_value ); 
	}
#endif //_WIN32

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

INLINE VOID unload_sevenfw_driver()
{
	BOOL _ret; 
	if( seven_fw_dev != NULL )
	{
		_ret = CloseHandle( seven_fw_dev ); 
		if( _ret == FALSE )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "close the sevenfw device failed\n" ) ); 
		}

		seven_fw_dev = NULL; 
	}

	return; 
}

#define unload_sevenfw_ex_driver() unload_sevenfw_driver() 

LRESULT delete_sevenfw_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	_ret = delete_drv_file( SEVEN_FW_SYS_FILE_NAME, TRUE ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!delete the driver file failed \n" ) ); 
	}

	_ret = delete_drv_file( SEVEN_FW_INF_FILE_NAME, TRUE ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!delete the driver file failed \n" ) ); 
	}

	_ret = delete_drv_file( SEVEN_FW_M_INF_FILE_NAME, TRUE ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!delete the driver file failed \n" ) ); 
	}

	return ret; 
}

LRESULT delete_sevenfw_ex_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

#ifdef DELAY_WIN7_SUPPORT
	ret = delete_sevenfw_driver(); 
	goto _return; 
#else
	_ret = delete_drv_file( SEVEN_FW_SYS_FILE_NAME, TRUE ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!delete the driver file failed \n" ) ); 
	}

	_ret = delete_drv_file( SEVEN_FW_INF_FILE_NAME, TRUE ); 
	ASSERT( _ret != ERROR_SUCCESS ? _ret == ERROR_FILE_NOT_FOUND : TRUE ); 

	if( _ret != ERROR_FILE_NOT_FOUND )
	{
		ASSERT( FALSE && "why have the ndisim inf file on windows 7" ); 
		log_trace( ( MSG_ERROR, "!!!delete the driver file failed \n" ) ); 
	}

	_ret = delete_drv_file( SEVEN_FW_M_INF_FILE_NAME, TRUE ); 
	ASSERT( _ret != ERROR_SUCCESS ? _ret == ERROR_FILE_NOT_FOUND : TRUE ); 

	if( _ret != ERROR_FILE_NOT_FOUND )
	{
		ASSERT( FALSE && "why have the ndisim inf file on windows 7" ); 	
		log_trace( ( MSG_ERROR, "!!!delete the driver file failed \n" ) ); 
	}

	_ret = delete_drv_file( SEVEN_FW_EX_INF_FILE_NAME, TRUE ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!delete the driver file failed \n" ) ); 
	}

	_ret = delete_drv_file( SEVEN_FW_EX_SYS_FILE_NAME, TRUE ); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!delete the driver file failed \n" ) ); 
	}
#endif //DELAY_WIN7_SUPPORT


	return ret; 
}

LRESULT safe_install_sevenfw_driver()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = install_sevenfw(); 

	if( ret != ERROR_SUCCESS )
	{
		LRESULT _ret; 
		_ret = delete_sevenfw_driver(); 
		if( _ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
		}

		ret = install_sevenfw(); 
	}

	return ret; 
}

LRESULT uninstall_sevenfw()
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN wow64_disabled = FALSE; 
	PVOID old_wow_value = NULL; 

#ifdef _WIN32
	{
		LRESULT _ret; 
		_ret = safe_disable_wow64( &old_wow_value ); 
		if( _ret == ERROR_SUCCESS )
		{
			wow64_disabled = TRUE; 
		}
	}
#endif //_WIN32

	unload_sevenfw_driver(); 

	ret = Installer.uninstall_ndis_drv( 0 ); 
	if( FAILED( ret ) )
	{
		log_trace( ( MSG_ERROR, "!!!uninstall ndis driver failed \n" ) ); 
	}

	delete_sevenfw_driver(); 

#ifdef _WIN32
	if( wow64_disabled == TRUE )
	{
		revert_wow_64( old_wow_value ); 
	}
#endif //_WIN32

	return ret;
}

LRESULT uninstall_sevenfw_ex()
{
	LRESULT ret = ERROR_SUCCESS; 

	BOOLEAN wow64_disabled = FALSE; 
	PVOID old_wow_value = NULL; 

#ifdef _WIN32
	{
		LRESULT _ret; 
		_ret = safe_disable_wow64( &old_wow_value ); 
		if( _ret == ERROR_SUCCESS )
		{
			wow64_disabled = TRUE; 
		}
	}
#endif //_WIN32

#ifdef DELAY_WIN7_SUPPORT
	ret = uninstall_sevenfw(); 
	goto _return; 
#else
	unload_sevenfw_ex_driver(); 

	ret = Installer.uninstall_ndis_drv( INSTALL_SEVENFW_WIN7_DRV ); 
	if( FAILED( ret ) )
	{
		log_trace( ( MSG_ERROR, "!!!uninstall ndis driver failed \n" ) ); 
	}

	delete_sevenfw_ex_driver(); 
#endif //DELAY_WIN7_SUPPORT


#ifdef _WIN32
	if( wow64_disabled == TRUE )
	{
		revert_wow_64( old_wow_value ); 
	}
#endif //_WIN32

	return ret;
}

LRESULT safe_install_sevenfw_ex()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = install_sevenfw_ex(); 
		if( ret != ERROR_SUCCESS )
		{
			LRESULT _ret; 

			_ret = delete_sevenfw_ex_driver(); 

			if( _ret != ERROR_SUCCESS )
			{

				log_trace( ( DBG_DBG_BUF_OUT, "delete the fs manage driver files error 0x%0.8x\n", ret ) ); 
				ASSERT( FALSE && "delete the fs manage driver files error " ); 
			}

			ret = install_sevenfw_ex(); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT load_device( HANDLE *dev_handle, LPTSTR dev_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE device; 

	ASSERT( dev_handle != NULL ); 
	ASSERT( dev_name != NULL ); 

	if( *dev_handle != NULL )
	{
		goto _return; 
	}

	device = CreateFile( dev_name, 
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		); 

	if( INVALID_HANDLE_VALUE != device )
	{
		*dev_handle = device; 
		goto _return; 
	}

	ret = GetLastError(); 
	log_trace( ( DBG_MSG_AND_ERROR_OUT, "load the device( name: %ws ) failed \n", dev_name ) ); 

_return:
	return ret; 
}

LRESULT load_sevenfw_device()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = load_device( &seven_fw_dev, SEVENFW_DEV_NAME ); 

	return ret; 
}

LRESULT load_sevenfw_ex_device()
{
	LRESULT ret = ERROR_SUCCESS; 

#ifdef DELAY_WIN7_SUPPORT
	ret = load_sevenfw_device(); 
#else
	ret = load_device( &seven_fw_dev, SEVEN_FW_EX_DEVICE_NAME ); 
#endif //DELAY_WIN7_SUPPORT
	return ret; 
}


#ifdef DELAY_WIN7_SUPPORT
#define load_netfw_device() load_netmon_driver() 
#else
INLINE LRESULT load_netfw_device()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = load_device( &net_mon_dev, NET_FW_DEVICE_NAME ); 

	return ret; 
}
#endif //DELAY_WIN7_SUPPORT

INLINE LRESULT load_driver( HANDLE *dev_handle, 
					LPCTSTR driver_name, 
					LPCTSTR device_name, 
					ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( dev_handle != NULL ); 
	ASSERT( driver_name != NULL ); 
	ASSERT( device_name != NULL ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	if( *dev_handle != NULL )
	{
		log_trace( ( MSG_INFO, "open device %ws when have not closed it?\n", device_name ) ); 
		goto _return; 
	}

	ret = load_drv_dev( dev_handle, 
		device_name, 
		driver_name, 
		flags ); 

	log_trace( ( MSG_INFO, "load the %ws device 0x%0.8x\n", device_name, ret ) ); 

	if( ret != ERROR_SUCCESS )
	{
		ret = _start_drv( driver_name ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "start mini port driver failed \n" ) ); 
		}
		else
		{
			ret = load_drv_dev( dev_handle, 
				device_name, 
				driver_name, 
				flags ); 

			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_INFO, "load mini port driver failed \n" ) ); 
			}
		}
	}

_return:
	ASSERT( ret != ERROR_SUCCESS ? *dev_handle == NULL : *dev_handle != NULL ); 

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x \n", __FUNCTION__, ret ) ); 
	return ret; 
}

INLINE LRESULT unload_driver( HANDLE *dev_handle )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	ASSERT( dev_handle != NULL ); 
	
	if( *dev_handle == NULL )
	{
		log_trace( ( MSG_ERROR, "!!!close device when it have not opened\n" ) ); 
		goto _return; 
	}
	
	_ret = CloseHandle( *dev_handle ); 
	if( _ret == FALSE )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "close device failed\n" ) ); 
		ASSERT( FALSE && "close device handle failed" ); 
		ret = GetLastError(); 
	}

	*dev_handle = NULL; 

_return:
	return ret; 
}

INLINE LRESULT uninstall_driver( LPCTSTR driver_name, ULONG flags )
{
	LRESULT ret; 
	ret = _uninstall_driver( driver_name, flags );  
	return ret; 
}

INLINE LRESULT output_sevenfw_driver()
{
	LRESULT ret; 

	ret = file_exists( SEVEN_FW_SYS_FILE_NAME, TRUE ); 
	if( ret == ERROR_SUCCESS )
	{
		ret = delete_drv_file( SEVEN_FW_SYS_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "can't delete file %ws", 
				SEVEN_FW_SYS_FILE_NAME ) ); 
		}
	}
	
	ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVEN_FW_FILE ), DAT_FILE_RES_TYPE, SEVEN_FW_SYS_FILE_NAME, TRUE ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw file from resource failed \n" ) ); 
		goto _return; 
	}

	ret = file_exists( SEVEN_FW_INF_FILE_NAME, TRUE ); 
	if( ret == ERROR_SUCCESS )
	{
		ret = delete_drv_file( SEVEN_FW_INF_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "can't delete file %ws", 
				SEVEN_FW_INF_FILE_NAME ) ); 
		}
	}

	ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVEN_FW_INF ), DAT_FILE_RES_TYPE, SEVEN_FW_INF_FILE_NAME, TRUE ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw inf file from resource failed \n" ) ); 
		goto _return; 
	}

	ret = file_exists( SEVEN_FW_M_INF_FILE_NAME, TRUE ); 
	if( ret == ERROR_SUCCESS )
	{
		ret = delete_drv_file( SEVEN_FW_M_INF_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "can't delete file %ws", 
				SEVEN_FW_M_INF_FILE_NAME ) ); 
		}
	}

	ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVEN_FW_M_INF ), DAT_FILE_RES_TYPE, SEVEN_FW_M_INF_FILE_NAME, TRUE ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw m inf file from resource failed \n" ) ); 
		goto _return; 
	}

_return:
	return ret; 
}

INLINE LRESULT output_sevenfw_ex_driver()
{
	LRESULT ret; 

#ifdef DELAY_WIN7_SUPPORT
	ret = output_sevenfw_driver(); 
	goto _return; 
#else
	ret = file_exists( SEVEN_FW_EX_SYS_FILE_NAME, TRUE ); 
	if( ret == ERROR_SUCCESS )
	{
		ret = delete_drv_file( SEVEN_FW_EX_SYS_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "can't delete file %ws", 
				SEVEN_FW_EX_SYS_FILE_NAME ) ); 
		}
	}

	ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVENFW_EX_FILE ), DAT_FILE_RES_TYPE, SEVEN_FW_EX_SYS_FILE_NAME, TRUE ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw ex file from resource failed \n" ) ); 
		goto _return; 
	}

	ret = file_exists( SEVEN_FW_EX_INF_FILE_NAME, TRUE ); 
	if( ret == ERROR_SUCCESS )
	{
		ret = delete_drv_file( SEVEN_FW_EX_INF_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "can't delete file %ws", 
				SEVEN_FW_INF_FILE_NAME ) ); 
		}
	}

	ret = create_file_from_res( NULL, MAKEINTRESOURCE( IDR_SEVENFW_EX_INF_FILE ), DAT_FILE_RES_TYPE, SEVEN_FW_EX_INF_FILE_NAME, TRUE ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO | DBG_DBG_BUF_OUT, "create the the seven fw ex inf file from resource failed \n" ) ); 
		goto _return; 
	}
#endif //DELAY_WIN7_SUPPORT

_return:
	return ret; 
}

INLINE LRESULT output_drv_file( LPCTSTR file_name, ULONG file_res, ULONG flags )
{
	LRESULT ret; 
	BOOLEAN wow64_disabled = FALSE; 
	PVOID old_wow_value = NULL; 

	ASSERT( file_name != NULL ); 
	ASSERT( file_res != 0 ); 

#ifdef _WIN32
	{
		LRESULT _ret; 
		_ret = safe_disable_wow64( &old_wow_value ); 
		if( _ret == ERROR_SUCCESS )
		{
			wow64_disabled = TRUE; 
		}
	}
#endif //_WIN32

	ret = file_exists( file_name, !!( flags & DRIVER_IN_SYSTEM_DIR ) ); 
	if( ERROR_SUCCESS == ret )
	{
		ret = delete_drv_file( file_name, !!( flags & DRIVER_IN_SYSTEM_DIR ) );  
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "can't delete file %ws", 
				SEVEN_FW_M_INF_FILE_NAME ) ); 
		}
	}

	ret = create_file_from_res( NULL, 
		MAKEINTRESOURCE( file_res ), 
		DAT_FILE_RES_TYPE, 
		file_name, 
		!!( flags & DRIVER_IN_SYSTEM_DIR ) ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "create the %ws from resource failed \n", file_name ) ); 
		goto _return; 
	}

_return:
#ifdef _WIN32
	if( wow64_disabled == TRUE )
	{
		revert_wow_64( old_wow_value ); 
	}
#endif //_WIN32

	return ret; 
}

LRESULT load_mini_port_driver( HANDLE *dev_handle, LPCWSTR mini_port_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT _ret; 
	HANDLE port_handle; 

	ASSERT( dev_handle != NULL ); 
	ASSERT( mini_port_name != NULL ); 

	*dev_handle = NULL; INVALID_HANDLE_VALUE; 

	_ret = FilterConnectCommunicationPort( mini_port_name,
		0,
		NULL,
		0,
		NULL,
		&port_handle );

	if( IS_ERROR( _ret ) )
	{
		ASSERT( port_handle == INVALID_HANDLE_VALUE ); 

#define E_FILE_NOT_FOUND                   _HRESULT_TYPEDEF_(0x800700A1)
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "connect to filter port %ws error 0x%08x\n", 
			mini_port_name, 
			_ret ) ); 

		if( _ret == E_FILE_NOT_FOUND )
		{
			ret = ERROR_FILE_NOT_FOUND; 
		}
		else
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}
		goto _return; 
	}

	ASSERT( port_handle != INVALID_HANDLE_VALUE ); 

	*dev_handle = port_handle; 

_return:
	return ret; 
}

INLINE LRESULT unload_mini_port_driver( HANDLE *dev_handle/*LPCTSTR mini_port_name */)
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = unload_driver( dev_handle ); 
	return ret; 
}

LRESULT mini_port_dev_get_data_async( HANDLE dev_handle, PVOID output_buf, ULONG output_len, OVERLAPPED *overlap )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT _ret; 

	if( dev_handle == INVALID_HANDLE_VALUE )
	{
		ASSERT( FALSE && "invalid mini port handle" ); 
	}

	
	if( output_len < sizeof( FILTER_MESSAGE_HEADER ) )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	_ret = FilterGetMessage( dev_handle,
		( PFILTER_MESSAGE_HEADER )output_buf, 
		output_len,
		overlap );

	if( _ret == HRESULT_FROM_WIN32( ERROR_IO_PENDING ) )
	{
		ret = ERROR_IO_PENDING; 

		goto _return; 
	}
	else if( IS_ERROR( _ret ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}
	else
	{
		ret = ERROR_SUCCESS; 
	}

_return:
	return ret; 
}

INLINE LRESULT mini_port_dev_io_ctl( HANDLE dev_handle, PVOID input_buf, ULONG input_len, PVOID output_buf, ULONG output_len, ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT _ret; 
	ULONG _ret_len; 

	if( dev_handle == INVALID_HANDLE_VALUE )
	{
		ret = ERROR_INVALID_HANDLE; 
		ASSERT( FALSE && "invalid mini port handle" ); 
		goto _return; 
	}

	if( ret_len != NULL )
	{
		*ret_len = 0; 
	}

	_ret = FilterSendMessage( dev_handle, 
		input_buf,
		input_len,
		output_buf,
		output_len,
		&_ret_len ); 

	if( IS_ERROR( _ret ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}


	if( ret_len != NULL )
	{
		*ret_len = _ret_len; 
	}

_return:
	return ret; 
}

LRESULT WINAPI load_mini_port_device( HANDLE *dev_handle, 
									 LPCWSTR port_name, 
									 LPCWSTR service_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( dev_handle != NULL ); 
	ASSERT( port_name != NULL ); 
	ASSERT( service_name != NULL ); 

	do 
	{
		ret = load_mini_port_driver( dev_handle, port_name ); 

		if( ret == ERROR_SUCCESS )
		{
			break; 
		}

		ret = _start_drv( service_name ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "start mini port driver failed \n" ) ); 
			break; 
		}

		ret = load_mini_port_driver( dev_handle, port_name ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "load mini port driver failed \n" ) ); 
		}

	}while( FALSE );

	return ret; 
}

LRESULT ctrl_bitsafe_driver( bitsafe_function_type type, driver_action action )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE *dev_handle = NULL; 
	ULONG file_res = 0; 
	LPCTSTR driver_file_name = NULL; 
	LPCTSTR driver_name = NULL; 
	LPCTSTR _device_name = NULL; 
	ULONG flags; 

	ASSERT( is_valid_driver_type( type ) ); 
	ASSERT( is_valid_driver_operation( action ) ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	log_trace( ( MSG_INFO, "device type is %ws, action is %ws\n", 
		get_driver_type_desc( type ), 
		get_operation_type_desc( action ) ) ); 

	switch( type )
	{
	case NET_PACKET_FLT_FUNCTION:
		{
			if( action == INSTALL_DRIVER )
			{
				if( is_vista_or_later_ex() == TRUE )
				{
					ret = safe_install_sevenfw_ex();
				}
				else
				{
					ret = safe_install_sevenfw_driver(); 
				}

				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( MSG_INFO, "load sevenfw driver failed \n" ) ); 
				}

				goto _return; 
			}
			else if( action == LOAD_DRIVER_DEVICE )
			{
				if( TRUE == is_vista_or_later_ex() )
				{
					ret = load_sevenfw_ex_device(); 
				}
				else
				{
					ret = load_sevenfw_device(); 
				}

				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( MSG_INFO, "load sevenfwex driver failed \n" ) ); 
				}

				goto _return; 
			}
			else if( action == UNLOAD_DRIVER )
			{
				if( TRUE == is_vista_or_later_ex() )
				{
					unload_sevenfw_ex_driver(); 
				}
				else
				{
					unload_sevenfw_driver(); 
				}
				goto _return; 
			}
			else if( action == UNINSTALL_DRIVER )
			{
				if( TRUE == is_vista_or_later_ex() )
				{
					ret = uninstall_sevenfw_ex(); 
				}
				else
				{
					ret = uninstall_sevenfw(); 
				}
				goto _return; 
			}
			else if( action == DELETE_DRIVER )
			{
				if( TRUE == is_vista_or_later_ex() )
				{
					ret = delete_sevenfw_ex_driver(); 
				}
				else
				{
					ret = delete_sevenfw_driver(); 
				}

				goto _return; 
			}
			else if( action == OUTPUT_DRIVER )
			{
				if( TRUE == is_vista_or_later_ex() )
				{
					ret = output_sevenfw_ex_driver(); 
				}
				else
				{
					ret = output_sevenfw_driver(); 
				}

				goto _return; 
			}

			goto _return; 
		}
		break; 
	case HTTP_TEXT_FLT_FUNCTION: 
		{
			dev_handle = &http_txt_flt_dev; 
			flags = DRIVER_IN_SYSTEM_DIR; 
			driver_name = HTTP_TXT_FLT_SVC_NAME; 
			driver_file_name = HTTP_TXT_FLT_FILE_NAME;  
			_device_name = HTTP_TXT_FLT_DEV_FILE_NAME; 
			file_res = IDR_HTTP_TXT_FW_FILE; 
		}
		break; 

	case HTTP_URL_FLT_FUNCTION: 
		{
			dev_handle = &http_url_flt_dev; 
			flags = DRIVER_IN_SYSTEM_DIR; 
			driver_name = HTTP_URL_FLT_SVC_NAME; 
			driver_file_name = HTTP_URL_FLT_FILE_NAME; 
			_device_name = HTTP_URL_FLT_DEV_FILE_NAME; 
			file_res = IDR_HTTP_URL_FW_FILE; 
		}
		break; 
	case ANTI_ARP_ATTACK_FUNCTION: 
		{
			dev_handle = &anti_arp_dev; 
			flags = DRIVER_IN_SYSTEM_DIR; 
			file_res = IDR_ANTI_ARP_FILE; 
			driver_file_name = ANTI_ARP_FILE_NAME; 
			driver_name = ANTI_ARP_SVC_NAME; 
			_device_name = ANTI_ARP_DEVICE_NAME; 
		}
		break; 
	case TRACE_LOG_FUNCTION: 
		{
			switch( action )
			{
			case INSTALL_DRIVER: 
				ret = safe_install_event_mon_drv_from_inf(); 
				goto _return; 
				break; 

			case UNINSTALL_DRIVER: 
				ret = uninstall_event_mon_drv(); 
				goto _return; 
				break; 
			case DELETE_DRIVER: 
				ret = delete_event_mon_driver_file(); 
				goto _return; 
				break; 
			case OUTPUT_DRIVER: 
				{
					ULONG res_file_id; 

					res_file_id = get_sys_file_resource_id( os_bit, os_version ); 

					if( res_file_id == 0 )
					{
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					ret = output_drv_file( EVENT_MON_SYS_FILE_NAME, 
						res_file_id, 
						DRIVER_IN_SYSTEM_DIR ); 

					if( ret != ERROR_SUCCESS )
					{
						log_trace( ( MSG_FATAL_ERROR, "output event mon driver file error %u\n", ret ) ); 
					}

					ret = output_drv_file( EVENT_MON_INF_FILE_NAME, IDR_EVENT_MON_INF_FILE, DRIVER_IN_SYSTEM_DIR ); 
					if( ret != ERROR_SUCCESS )
					{
						log_trace( ( MSG_FATAL_ERROR, "output event mon driver file error %u\n", ret ) ); 
					}
				}
				goto _return; 
				break; 
			default: 
				dev_handle = &trace_log_dev; 
				flags = DRIVER_IN_SYSTEM_DIR | DRIVER_BOOT_START; 
				driver_name = EVENT_MON_SVC_NAME; 
				_device_name = TRACE_LOG_DEV_NAME_APP; 

				break; 
			}
		}
		break; 
	case NET_COMM_MANAGE_FUNCTION:
		{
			{
				switch( action )
				{
				case INSTALL_DRIVER: 
				case UNINSTALL_DRIVER: 
				case DELETE_DRIVER: 
				case OUTPUT_DRIVER: 
					ret = ERROR_NOT_SUPPORTED; 
					goto _return; 
					break; 
				}

				dev_handle = &net_mon_dev; 
				flags = DRIVER_IN_SYSTEM_DIR; 
				file_res = IDR_NET_MON_FILE; 
				driver_name = EVENT_MON_SVC_NAME; 
				_device_name = NETMON_DEVICE_NAME; 
			}
		}
		break; 
	case FILE_MANAGE_FUNCTION: 
		{
			dev_handle = &fs_mng_dev; 

			switch( action )
			{
			case INSTALL_DRIVER: 
			case UNINSTALL_DRIVER: 
			case DELETE_DRIVER: 
				ret = ERROR_NOT_SUPPORTED; 
				goto _return; 
				break; 
			case LOAD_DRIVER_MINI_PORT:
				{
					load_mini_port_device( dev_handle, FS_MNG_PORT_NAME, EVENT_MON_SVC_NAME ); 
					goto _return; 
				}
				break; 
			case UNLOAD_DRIVER: 
				{
					ret = unload_mini_port_driver( dev_handle ); 
					goto _return; 
				}
				break; 
			}
			goto _return; 
		}
		break; 
	case REG_MANAGE_FUNCTION: 
		{
			dev_handle = &reg_mng_dev; 
			flags = DRIVER_IN_SYSTEM_DIR; 
			file_res = IDR_REG_MNG_FILE; 
			driver_name = EVENT_MON_SVC_NAME; 
			_device_name = REG_MNG_DEVICE_NAME; 	
			flags = DRIVER_IN_SYSTEM_DIR | DRIVER_BOOT_START; 
			driver_name = EVENT_MON_SVC_NAME; 
		}
		break; 
	default:
		{
			ASSERT( FALSE && "invalid driver type" ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
		break; 
	}

	switch( action )
	{
	case INSTALL_DRIVER: 
		{
			ret = test_load_device( _device_name ); 
			if( ret == ERROR_SUCCESS )
			{
				goto _return; 
			}

			ret = safe_install_driver( _device_name, 
				driver_file_name, 
				driver_name, 
				file_res, 
				flags ); 
		}
		break; 
	case LOAD_DRIVER_DEVICE:
		{
			ret = load_driver( dev_handle, 
				driver_name, 
				_device_name, 
				flags ); 
		}
		break; 
	case LOAD_DRIVER_MINI_PORT:
		{
			ret = load_mini_port_driver( dev_handle, 
				_device_name ); 
		}
		break; 
	case UNLOAD_DRIVER:
		{
			ret = unload_driver( dev_handle ); 
		}
		break; 
	case UNINSTALL_DRIVER: 
		{
			ret = uninstall_driver( driver_name, flags ); 
		}
		break; 
	case DELETE_DRIVER: 
		{
			if( driver_file_name == NULL )
			{
				break; 
			}

			ret = delete_drv_file( driver_file_name, !!( flags & DRIVER_IN_SYSTEM_DIR ) ); 
		}
		break; 
	case OUTPUT_DRIVER: 
		{
			ret = output_drv_file( driver_file_name, file_res, flags ); 
		}
		break; 
	default: 
		{
			ASSERT( FALSE && "invalid operation type"); 
		}
		break; 
	}

_return:
	return ret; 
}

INLINE LRESULT load_net_filter_driver()
{
	LRESULT ret; 
	ret = ctrl_bitsafe_driver( HTTP_URL_FLT_FUNCTION, LOAD_DRIVER_DEVICE ); 
	return ret; 
}

INLINE LRESULT unload_net_filter_driver()
{
	LRESULT ret; 
	ret = ctrl_bitsafe_driver( HTTP_URL_FLT_FUNCTION, UNLOAD_DRIVER ); 
	return ret; 
}

INLINE LRESULT delete_net_filter_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( HTTP_URL_FLT_FUNCTION, DELETE_DRIVER ); 
	return ret; 
}

INLINE LRESULT uninstall_net_filter_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( HTTP_URL_FLT_FUNCTION, UNINSTALL_DRIVER ); 
	return ret; 
}

INLINE LRESULT install_net_filter_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( HTTP_URL_FLT_FUNCTION, INSTALL_DRIVER ); 
	return ret; 
}

INLINE LRESULT load_netmon_driver()
{
	LRESULT ret; 
	ret = ctrl_bitsafe_driver( NET_COMM_MANAGE_FUNCTION, LOAD_DRIVER_DEVICE ); 

	return ret; 
}

INLINE LRESULT unload_netmon_driver()
{
	LRESULT ret; 

	ret = ctrl_bitsafe_driver( NET_COMM_MANAGE_FUNCTION, UNLOAD_DRIVER ); 

	return ret; 
}


LRESULT load_reg_flt_driver()
{
	LRESULT ret; 

	ret = ctrl_bitsafe_driver( REG_MANAGE_FUNCTION, LOAD_DRIVER_DEVICE ); 

	return ret; 
}

INLINE LRESULT unload_reg_flt_driver()
{
	LRESULT ret; 
	
	ret = ctrl_bitsafe_driver( REG_MANAGE_FUNCTION, UNLOAD_DRIVER ); 

	return ret; 
}


INLINE LRESULT load_fs_mng_driver()
{
	LRESULT ret; 
	ret = ctrl_bitsafe_driver( FILE_MANAGE_FUNCTION, LOAD_DRIVER_DEVICE ); 
	return ret; 
}

INLINE LRESULT unload_fs_mng_driver()
{
	LRESULT ret; 
	ret = ctrl_bitsafe_driver( FILE_MANAGE_FUNCTION, UNLOAD_DRIVER ); 
	return ret; 
}

INLINE LRESULT load_tracelog_driver()
{
	LRESULT ret; 
	ret = ctrl_bitsafe_driver( TRACE_LOG_FUNCTION, LOAD_DRIVER_DEVICE ); 

	return ret; 
}

INLINE LRESULT unload_tracelog_driver()
{
	LRESULT ret; 

	ret = ctrl_bitsafe_driver( TRACE_LOG_FUNCTION, UNLOAD_DRIVER ); 

	return ret; 
}


INLINE LRESULT load_anti_arp_driver()
{
	LRESULT ret; 

	ret = ctrl_bitsafe_driver( ANTI_ARP_ATTACK_FUNCTION, LOAD_DRIVER_DEVICE ); 
	return ret; 
}

INLINE LRESULT unload_anti_arp_driver()
{
	LRESULT ret; 
	ret = ctrl_bitsafe_driver( ANTI_ARP_ATTACK_FUNCTION, UNLOAD_DRIVER ); 
	return ret; 
}

INLINE LRESULT delete_anti_arp_driver()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = ctrl_bitsafe_driver( ANTI_ARP_ATTACK_FUNCTION, DELETE_DRIVER ); 
	return ret; 
}

INLINE LRESULT uninstall_anti_arp_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( ANTI_ARP_ATTACK_FUNCTION, UNINSTALL_DRIVER ); 
	return ret; 
}

INLINE LRESULT install_anti_arp_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( ANTI_ARP_ATTACK_FUNCTION, INSTALL_DRIVER ); 
	return ret; 
}

INLINE LRESULT load_http_txt_filter_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( HTTP_TEXT_FLT_FUNCTION, UNLOAD_DRIVER ); 
	return ret;  
}

INLINE LRESULT unload_http_txt_filter_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( HTTP_TEXT_FLT_FUNCTION, UNLOAD_DRIVER ); 
	return ret; 
}

INLINE LRESULT delete_http_txt_filter_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( HTTP_TEXT_FLT_FUNCTION, DELETE_DRIVER ); 
	return ret; 
}

INLINE LRESULT uninstall_http_txt_filter_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( HTTP_TEXT_FLT_FUNCTION, UNINSTALL_DRIVER ); 
	return ret; 
}

INLINE LRESULT install_http_txt_filter_driver()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = ctrl_bitsafe_driver( HTTP_TEXT_FLT_FUNCTION, INSTALL_DRIVER ); 
	return ret; 
}

LRESULT wait_thread_end( HANDLE thread_handle, ULONG wait_time )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD wait_ret; 

	ASSERT( thread_handle != NULL ); 

	wait_ret = WaitForSingleObject( thread_handle, wait_time );

#ifdef _DEBUG
	if( wait_ret == WAIT_FAILED )
	{
		ULONG err_code; 

		err_code = GetLastError(); 

		ASSERT( err_code != ERROR_INVALID_HANDLE ); 
	}
#endif //_DEBUG

	if( WAIT_OBJECT_0 != wait_ret && 
		WAIT_ABANDONED != wait_ret )
	{
		//ret = GetLastError(); 

		if( wait_ret == WAIT_FAILED )
		{
			ret = GetLastError(); 
		}

		TerminateThread( thread_handle, 0 );
	}

	CloseHandle( thread_handle );

	return ret; 
}

LRESULT work_thread_inited( thread_manage *work_thread )
{
	LRESULT ret = ERROR_SUCCESS; 
	ASSERT( work_thread != NULL ); 
	
	if( work_thread->thread_handle == NULL )
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT stop_work_thread( thread_manage *work_thread )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( work_thread != NULL ); 

	if( work_thread->thread_handle == NULL )
	{
		ret = ERROR_NOT_READY; 
		if( work_thread->notify != NULL )
		{
			if( work_thread->self_notify == TRUE )
			{
				CloseHandle( work_thread->notify ); 
			}
		}

		goto _return; 
	}

	ASSERT( work_thread->notify != NULL ); 

	work_thread->stop_run = TRUE; 
	SetEvent( work_thread->notify ); 
	
	ret = wait_thread_end( work_thread->thread_handle, 200 ); 

	if( work_thread->self_notify == TRUE )
	{
		CloseHandle( work_thread->notify ); 
	}

_return:
	work_thread->notify = NULL; 
	work_thread->thread_handle = NULL; 
	work_thread->stop_run = FALSE; 
	work_thread->param = NULL; 
	work_thread->id = 0; 
	return ret; 
}

LRESULT _create_work_thread( thread_manage *work_thread, 
						   PTHREAD_START_ROUTINE thread_routine, 
						   HANDLE notifier, 
						   PVOID context, 
						   PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	work_thread->notify = NULL; 
	work_thread->id = 0; 
	work_thread->stop_run = FALSE; 
	work_thread->thread_handle = NULL; 
	work_thread->context = context; 
	work_thread->param = param; 
	work_thread->self_notify = FALSE; 

	if( notifier != NULL )
	{
		work_thread->notify = notifier; 
		ASSERT( WaitForSingleObject( notifier, 0 ) != WAIT_FAILED ); 
		work_thread->self_notify = FALSE; 
	}
	else
	{
		work_thread->notify = CreateEvent( NULL, FALSE, FALSE, NULL ); 
		if( work_thread->notify == NULL )
		{
			ret = GetLastError(); 
			goto _return; 
		}
		work_thread->self_notify = TRUE; 
	}

	work_thread->thread_handle = CreateThread( NULL, 0, thread_routine, ( PVOID )work_thread, 0, &work_thread->id ); 
	if( work_thread->thread_handle == NULL )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "create thread failed 0x%0.8x \n", ret ) ); 
		goto _return; 
	}

_return:
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( work_thread->thread_handle == NULL ); 

		stop_work_thread( work_thread ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT init_ui_ctrls()
{
	LRESULT ret = ERROR_SUCCESS; 
	InitializeListHead( &all_ui_ctrls );
	init_cs_lock( ui_ctrls_lock ); 
	return ret; 
}

INLINE PUI_CTRL find_ui_ctrl( ULONG ui_id )
{
	PUI_CTRL ui_ctrl; 
	PLIST_ENTRY list_entry; 
	
	lock_cs( ui_ctrls_lock ); 
	list_entry = all_ui_ctrls.Flink; 

	for( ; ; )
	{
		if( list_entry == &all_ui_ctrls )
		{
			ui_ctrl = NULL; 
			break; 
		}

		ui_ctrl = ( PUI_CTRL )list_entry; 

		if( ui_ctrl->ui_id == ui_id )
		{
			break; 
		}

		list_entry = list_entry->Flink; 
		
	}

	unlock_cs( ui_ctrls_lock ); 

	return ui_ctrl; 
}

LRESULT __add_ui_ctrl( PUI_CTRL ui_ctrl_add )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( NULL != ui_ctrl_add ); 

	lock_cs( ui_ctrls_lock ); 
	
	InsertHeadList( &all_ui_ctrls, &ui_ctrl_add->ui_list ); 

	unlock_cs( ui_ctrls_lock ); 
	return ret; 
}

LRESULT add_ui_ctrl( ULONG ui_id, do_ui_work ui_work_func, do_ui_output ui_output_func )
{
	LRESULT ret = ERROR_SUCCESS; 
	PUI_CTRL ui_ctrl; 
	ui_ctrl = find_ui_ctrl( ui_id ); 
	if( NULL != ui_ctrl )
	{
		ret = ERROR_UI_CTRL_AREADY_EXIST; 
		goto _return; 
	}

	ui_ctrl = ( PUI_CTRL )malloc( sizeof( UI_CTRL ) ); 
	if( ui_ctrl == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ASSERT( ui_work_func != NULL ); 

	ui_ctrl->ui_id = ui_id; 
	ui_ctrl->ui_work_func = ui_work_func; 
	ui_ctrl->ui_output_func = ui_output_func; 

	__add_ui_ctrl( ui_ctrl ); 

_return:
	return ret; 
}

LRESULT delete_ui_ctrl( ULONG ui_id )
{
	LRESULT ret = ERROR_SUCCESS; 
	PUI_CTRL ui_ctrl; 

	ui_ctrl = find_ui_ctrl( ui_id ); 
	if( NULL == ui_ctrl )
	{
		ret = ERROR_UI_CTRL_CANT_FIND; 
		goto _return; 
	}

	lock_cs( ui_ctrls_lock ); 
	
	RemoveEntryList( &ui_ctrl->ui_list ); 

	unlock_cs( ui_ctrls_lock ); 

	free( ui_ctrl ); 

_return:
	return ret; 
}

LRESULT release_ui_ctrls()
{
	LRESULT ret = ERROR_SUCCESS; 
	PUI_CTRL ui_ctrl; 
	PLIST_ENTRY list_entry; 
	PLIST_ENTRY next_entry; 
	
	lock_cs( ui_ctrls_lock ); 
	list_entry = all_ui_ctrls.Flink; 

	for( ; ; )
	{
		next_entry = list_entry->Flink; 

		if( list_entry == &all_ui_ctrls )
		{
			break; 
		}

		ui_ctrl = ( PUI_CTRL )list_entry; 
		RemoveEntryList( list_entry ); 
		free( ui_ctrl ); 
		
		list_entry = next_entry; 
	}

	unlock_cs( ui_ctrls_lock ); 
	uninit_cs_lock( ui_ctrls_lock ); 

	return ret; 
}

LRESULT do_ui_ctrl( ULONG ui_id, ULONG ui_action_id, PBYTE input_buf, ULONG input_length, PBYTE output_buf, ULONG output_length, ULONG *ret_len, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	PUI_CTRL ui_ctrl; 
	
	ui_ctrl = find_ui_ctrl( ui_id ); 
	if( NULL == ui_ctrl )
	{
		ret = ERROR_NOT_FOUND; 
		goto _return; 
	}

	ASSERT( NULL != ui_ctrl->ui_work_func ); 
	ret = ui_ctrl->ui_work_func( ui_action_id, input_buf, input_length, output_buf, output_length, ret_len, ui_ctrl, param ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	return ret; 
}

INLINE LRESULT do_dev_io_ctrl( HANDLE dev_handle, 
							  ULONG ctl_code, 
							  PVOID input_buf, 
							  ULONG input_len, 
							  PVOID output_buf, 
							  ULONG output_len, 
							  PULONG ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret;

	ASSERT( ctl_code != 0 ); 
	ASSERT( ret_len != NULL ); 

	if( dev_handle == NULL )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	_ret = DeviceIoControl( dev_handle, 
		ctl_code,  
		input_buf, 
		input_len, 
		output_buf,
		output_len, 
		ret_len, 
		NULL );

	if( _ret != TRUE )
	{
		ret = GetLastError(); 
	}

_return:
	return ret;
}


LRESULT WINAPI get_driver_version( ULARGE_INTEGER *driver_version )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	do 
	{
		if( driver_version == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = do_dev_io_ctrl( trace_log_dev, 
			IOCTL_GET_EVENTMON_DRIVER_VERSION, 
			NULL, 
			0,  
			driver_version, 
			sizeof( *driver_version ), 
			&ret_len ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( ret_len != sizeof( *driver_version ) )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT set_work_time( LARGE_INTEGER *work_time )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	if( work_time->QuadPart > MAX_WORK_TIME )
	{
		work_time->QuadPart = MAX_WORK_TIME; 
	}

	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_SEVEN_FW_SET_WORK_TIME, 
		work_time, 
		sizeof( *work_time ), 
		NULL, 
		0, 
		&ret_len ); 

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

RESULT get_sys_event( sys_action_desc* sys_event, ULONG size, PULONG ret_len )
{
	HRESULT ret = ERROR_SUCCESS; 

	ASSERT( size >= sizeof( sys_action_desc ) ); 
	ASSERT( trace_log_dev != NULL );  

	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_GET_ACTION_EVENT,  
		NULL, 
		0, 
		sys_event,
		size, 
		ret_len );

	return ret;
}

LRESULT set_action_notify_event( notify_events_set *all_events, ULONG input_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD ret_len; 
	INT32 i; 

	//DBG_BP(); 
	ASSERT( all_events != NULL ); 
	if( all_events->event_num * sizeof( event_to_notify ) + FIELD_OFFSET( notify_events_set, events ) > input_len )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	for( i = 0; ( ULONG )i < all_events->event_num; i ++ )
	{
		if( all_events->events[ i ].event_handle == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
		
		if( is_valid_notify_event_type( all_events->events[ i ].type ) == FALSE )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}
	}

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_SEVEN_FW_SET_NOTIFY_EVENTS, 
		all_events, 
		input_len,  
		NULL, 
		0, 
		&ret_len );

#endif //DEBUG_WITH_DEV

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
_return:
	return ret; 
}

#include "aio_support.h"
//FORCEINLINE 
LRESULT WINAPI recv_event_aio( PVOID input_buf, 
								   ULONG input_size, 
								   PVOID output_buf, 
								   ULONG output_size, 
								   ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	//DWORD ret_len; 
	//INT32 i; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

#ifdef DEBUG_WITH_DEV
	ret = dev_io_ctl_aio( trace_log_dev, 
		IOCTL_ASYNC_GET_ACTION_EVENT, 
		input_buf,
		input_size, 
		output_buf, 
		output_size, 
		ret_len );

#endif //DEBUG_WITH_DEV

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

HRESULT get_block_counts( all_block_count *block_count )
{
	HRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	ASSERT( block_count != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_GET_BLOCK_COUNT, 
		( PVOID )NULL, 
		0,  
		block_count,
		sizeof( *block_count ), 
		&ret_len );

	//_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__,ret ) ); 
	return ret; 
}

HRESULT get_sys_act_info( sys_log_output* msgs_cap, ULONG size, PULONG ret_len )
{
	HRESULT ret = ERROR_SUCCESS; 
	ULONG logger_name; 

	logger_name = TRACE_LOGGER_SYSTEM; 

	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_GET_TRACE_LOG,  
		&logger_name,
		sizeof( logger_name ), 
		msgs_cap,
		size, 
		ret_len );

	return ret;
}

ULONG CALLBACK thread_action_manage( LPVOID param )
{
	HRESULT ret = ERROR_SUCCESS;
	DWORD wait_ret;
	ULONG ret_len; 
	PUI_CTRL ui_ctrl; 
	sys_action_desc* cur_sys_event; 
	thread_manage *thread_param; 
	PVOID _param; 

	ASSERT( NULL != param );

	thread_param = ( thread_manage* )param; 

	ASSERT( thread_param->param != NULL ); 

	ui_ctrl = ( PUI_CTRL )thread_param->context; 
	_param = thread_param->param; 

	cur_sys_event = ( sys_action_desc* )malloc( sizeof( sys_action_desc ) );
	if( NULL == cur_sys_event )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY;
		goto _return;
	}

	for( ; ; )
	{
		log_trace( ( MSG_INFO, "%s get sys event loop\n", __FUNCTION__ ) ); 

		if( thread_param->stop_run == TRUE )
		{
			break; 
		}

		log_trace( ( MSG_INFO, "%s begin get sys event \n", __FUNCTION__  ) ); 

		ret = get_sys_event( cur_sys_event, 
			sizeof( sys_action_desc ), 
			&ret_len ); 

		log_trace( ( MSG_INFO, "%s end get sys event 0x%0.8x \n", __FUNCTION__, ret ) ); 

		if( ret != ERROR_SUCCESS || ret_len == 0 )
		{
			goto _continue; 
		}

		ui_ctrl->ui_output_func( SYS_MON_ACTION_REQUEST, ret, ( PBYTE )cur_sys_event, ret_len, _param ); 

_continue:
		wait_ret = wait_event( thread_param->notify, 5000 ); 
		if( wait_ret != WAIT_TIMEOUT 
			&& wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait syncronous event failed will exit\n" ) ); 
			break; 
		}
	}

_return:
	if( NULL != cur_sys_event )
	{
		free( cur_sys_event ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ( ULONG )ret; 
}

LRESULT check_ui_log_thread_exist( INT32 *ui_exist )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HANDLE sem_handle = NULL; 

	ASSERT( ui_exist != NULL ); 

	*ui_exist = FALSE; 

	for( ; ; )
	{
		ret = check_global_name_exist( BITSAFE_UI_TRACE_LOG_MARK, &sem_handle ); 
		if( ret == ERROR_SUCCESS )
		{
			ULONG wait_ret; 

			if( *ui_exist == FALSE )
			{
				*ui_exist = TRUE; 
			}

			ASSERT( sem_handle != NULL ); 

			wait_ret = WaitForSingleObject( sem_handle, INFINITE ); 
#ifdef _DEBUG
			if( wait_ret != WAIT_OBJECT_0 )
			{
				log_trace( ( MSG_ERROR, "wait the global name end error %u\n", wait_ret ) ); 
				if( wait_ret != WAIT_ABANDONED 
					&& wait_ret != WAIT_TIMEOUT )
				{}
			}
#endif //_DEBUG

			_ret = ReleaseSemaphore( sem_handle, 1, NULL ); 
			if( FALSE == _ret )
			{
				ret = GetLastError(); 

				log_trace( ( MSG_ERROR, "release the ui log tip error" ) ); 
			}

			CloseHandle( sem_handle ); 
		}
		else
		{
			break; 
		}
	}

	return ret; 
}


ULONG CALLBACK thread_service_action_log( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS;
	DWORD wait_ret;
	PUI_CTRL ui_ctrl; 
	ULONG ret_len; 
	thread_manage *thread_param; 
	PVOID _param; 
	sys_log_output *action_log; 
	ULONG count = 0; 

	ASSERT( NULL != param ); 

	thread_param = ( thread_manage* )param; 
	ASSERT( thread_param->param != NULL ); 
	ui_ctrl = ( PUI_CTRL )thread_param->context;
	_param = thread_param->param; 

	action_log = ( sys_log_output* )malloc( MAX_ACTION_LOG_BUF_LEN );
	if( NULL == action_log )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return;
	}
#define CHECK_UI_LOG_EXIST_TIME_SPAN 1
	for( ; ; )
	{
		count ++; 

		if( ( count % CHECK_UI_LOG_EXIST_TIME_SPAN ) == 0 )
		{
			INT32 ui_exist; 
			check_ui_log_thread_exist( &ui_exist ); 
			if( ui_exist == TRUE )
			{
				log_trace( ( MSG_INFO, "the ui trace log have opened\n" ) ); 
			}
		}

		wait_ret = wait_event( thread_param->notify, INFINITE ); 
		if( wait_ret != WAIT_TIMEOUT 
			&& wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait syncronous event failed will exit\n" ) ); 
			break; 
		}

		if( thread_param->stop_run == TRUE )
		{
			break; 
		}

		log_trace( ( MSG_INFO, "%s begin get sys log \n", __FUNCTION__  ) ); 

#ifdef TRACE_LOGS_BY_MEM_COPY
		ret = output_trace_logs( TRACE_LOGGER_SYSTEM, 
			action_log, MAX_ACTION_LOG_BUF_LEN ); 
#else
		ret = get_sys_act_info( action_log, MAX_ACTION_LOG_BUF_LEN, &ret_len ); 
#endif //TRACE_LOGS_BY_MEM_COPY

		log_trace( ( MSG_INFO, "%s end get sys log %d \n", __FUNCTION__, ret ) ); 

		if( !NT_SUCCESS( ret ) )
		{
			goto _continue; 
		}
		if( ret != ERROR_SUCCESS 
			|| ret_len < sizeof( sys_log_output ) + sizeof( sys_log_unit ) )
		{
			goto _continue; 
		}

		ui_ctrl->ui_output_func( BITSAFE_LOG_OUTPUT, ret, ( PBYTE )action_log, ret_len, _param ); 

_continue:
		continue; 
	}

_return:
	if( NULL != action_log )
	{
		free( action_log ); 
	}

	return ( ULONG )ret; 
}

ULONG CALLBACK thread_ui_action_log( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS;
	DWORD wait_ret;
	PUI_CTRL ui_ctrl; 
	ULONG ret_len; 
	thread_manage *thread_param; 
	PVOID _param; 
	sys_log_output *action_log; 

	ASSERT( NULL != param ); 

	thread_param = ( thread_manage* )param; 
	ASSERT( thread_param->param != NULL ); 
	ui_ctrl = ( PUI_CTRL )thread_param->context;
	_param = thread_param->param; 

	action_log = ( sys_log_output* )malloc( MAX_ACTION_LOG_BUF_LEN );
	if( NULL == action_log )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return;
	}

	for( ; ; )
	{
		//count ++; 

		wait_ret = wait_event( thread_param->notify, INFINITE ); 
		if( wait_ret != WAIT_TIMEOUT 
			&& wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait syncronous event failed will exit\n" ) ); 
			break; 
		}

		if( thread_param->stop_run == TRUE )
		{
			break; 
		}

		log_trace( ( MSG_INFO, "%s begin get sys log \n", __FUNCTION__  ) ); 

#ifdef TRACE_LOGS_BY_MEM_COPY
		ret = output_trace_logs( TRACE_LOGGER_SYSTEM, 
			action_log, MAX_ACTION_LOG_BUF_LEN ); 
#else
		ret = get_sys_act_info( action_log, MAX_ACTION_LOG_BUF_LEN, &ret_len ); 
#endif //TRACE_LOGS_BY_MEM_COPY

		log_trace( ( MSG_INFO, "%s end get sys log %d \n", __FUNCTION__, ret ) ); 

		if( !NT_SUCCESS( ret ) )
		{
			goto _continue; 
		}

		if( ret != ERROR_SUCCESS 
			|| ret_len < sizeof( sys_log_output ) + sizeof( sys_log_unit ) )
		{
			goto _continue; 
		}

		ui_ctrl->ui_output_func( BITSAFE_LOG_OUTPUT, ret, ( PBYTE )action_log, ret_len, _param ); 

_continue: 
		continue; 
	}

_return:
	if( NULL != action_log )
	{
		free( action_log ); 
	}

	return ( ULONG )ret; 
}

LRESULT get_all_process_traffic( PROCESS_TRAFFIC_OUTPUT* process_infos, ULONG input_len, PULONG ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( net_mon_dev != NULL ); 
	ASSERT( ret_len != NULL ); 

	*ret_len = 0; 

	ret = do_dev_io_ctrl( net_mon_dev, 
		IOCTL_GET_ALL_PROCESS_TRAFFIC, 
		NULL, 
		0, 
		( PVOID )process_infos, 
		input_len, 
		ret_len ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "get all process traffic failed 0x%0.8x \n", ret ) ); 
	}

	return ret; 
}

LRESULT get_all_listen_info( PBYTE listen_info, ULONG output_len, ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( net_mon_dev != NULL ); 
	ASSERT( ret_len != NULL ); 

	ret = do_dev_io_ctrl( net_mon_dev, 
		IOCTL_CMD_ENUM_LISTEN, 
		NULL, 
		0, 
		( PVOID )listen_info, 
		output_len, 
		ret_len ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "get all listen infomation failed 0x%0.8x \n", ret ) ); 
	}

	return ret; 
}

LRESULT get_all_tcp_conn_info( BYTE *conn_info_out, ULONG output_len, ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( net_mon_dev != NULL ); 
	ASSERT( ret_len != NULL ); 

	ret = do_dev_io_ctrl( net_mon_dev, 
		IOCTL_CMD_ENUM_TCP_CONN, 
		NULL, 
		0, 
		( PVOID )conn_info_out, 
		output_len, 
		ret_len ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "get all infomation of conntection of tcp failed 0x%0.8x \n", ret ) ); 
	}

	return ret; 
}

LRESULT get_all_traffic( LARGE_INTEGER all_traffic[ 2 ], ULONG output_len, PULONG ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( net_mon_dev != NULL ); 
	ASSERT( ret_len != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

	log_trace( ( MSG_INFO, "netmon device is 0x%0.8x \n", net_mon_dev ) ); 

	ret = do_dev_io_ctrl( net_mon_dev, 
		IOCTL_GET_ALL_TRAFFIC, 
		NULL, 
		0, 
		( PVOID )all_traffic, 
		output_len, 
		ret_len ); 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "get all traffic failed 0x%0.8x \n", ret ) ); 
	}

	log_trace( ( MSG_ERROR, "get all traffic length is %d \n", *ret_len ) ); 

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
	return ret; 
}

DWORD CALLBACK net_mon_all_traffic_thread( LPVOID param )
{
	LRESULT ret = ERROR_SUCCESS;
	DWORD dwOutputLength;
	PUI_CTRL ui_ctrl; 
	DWORD wait_ret; 
	PVOID _param; 
	thread_manage *thread_param; 
	LARGE_INTEGER all_traffic[ 2 ];
	//INT32 i;

	thread_param = ( thread_manage* )param; 
	ui_ctrl = ( PUI_CTRL )thread_param->context; 
	_param = thread_param->param; 


	for( ; ; )
	{
		log_trace( ( MSG_INFO, "%s get all process traffic loop\n", __FUNCTION__ ) ); 

		wait_ret = wait_event( thread_param->notify, 5000 ); 
		if( wait_ret != WAIT_TIMEOUT 
			&& wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!wait synchronous event failed will exit\n" ) ); 
			ASSERT( FALSE ); 
			break; 
		}

		if( TRUE == thread_param->stop_run )
		{
			break;
		}

		ret = get_all_traffic( all_traffic, sizeof( all_traffic ), &dwOutputLength ); 

		if( ret == ERROR_SUCCESS )
		{
			ui_ctrl->ui_output_func( NET_MON_OUTPUT_TRAFFIC_COUNT, ret, ( PBYTE )all_traffic, dwOutputLength, _param ); 
		}

		continue; 
	}

	return ( DWORD )ret;
}

LRESULT set_action_rule( access_rule_desc *rule_input, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( rule_input != NULL ); 
	ASSERT( length >= sizeof( access_rule_desc ) ); 
	ret = check_access_rule_input_valid( rule_input, FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		//ASSERT( FALSE && "input invlaid access rule" ); 
		goto _return; 
	}

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_ADD_RULE_DEFINE, 
		rule_input, 
		length, 
		NULL, 
		0, 
		&ret_len );

#endif //DEBUG_WITH_DEV

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "add rule define failed \n" ) ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT unset_action_rule( access_rule_desc *rule_input, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( rule_input != NULL ); 
	ASSERT( length >= sizeof( access_rule_desc ) ); 

	ret = check_access_rule_input_valid( rule_input, FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "input invlaid access rule" ); 
		goto _return; 
	}

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_DEL_RULE_DEFINE, 
		rule_input, 
		length, 
		NULL, 
		0, 
		&ret_len );

#endif 

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "add rule define failed \n" ) ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT _modify_action_rule( access_rule_modify_info *modify_info, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( modify_info != NULL ); 
	ASSERT( length >= sizeof( access_rule_modify_info ) ); 

	#ifdef DEBUG_WITH_DEV
		ret = do_dev_io_ctrl( trace_log_dev, 
			IOCTL_MODIFY_RULE_DEFINE, 
			modify_info, 
			length, 
			NULL, 
			0, 
			&ret_len );
	
	#endif

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "add rule define failed \n" ) ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT set_app_rule( access_rule_desc *rule_input, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( rule_input != NULL ); 
	ASSERT( length >= sizeof( access_rule_desc ) ); 
	ret = check_access_rule_input_valid( rule_input, FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "input invlaid access rule" ); 
		goto _return; 
	}

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_ADD_APP_RULE_DEFINE, 
		rule_input, 
		length, 
		NULL, 
		0, 
		&ret_len );

#endif

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "add rule define failed \n" ) ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT unset_app_rule( access_rule_desc *rule_input, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( rule_input != NULL ); 
	ASSERT( length >= sizeof( access_rule_desc ) ); 

	ret = check_access_rule_input_valid( rule_input, FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "input invlaid access rule" ); 
		goto _return; 
	}

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_DEL_APP_RULE_DEFINE, 
		rule_input, 
		length, 
		NULL, 
		0, 
		&ret_len ); 
#endif

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "add rule define failed \n" ) ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT set_app_action_rule( access_rule_desc *rule_input, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( rule_input != NULL ); 
	ASSERT( length >= sizeof( access_rule_desc ) ); 
	ret = check_access_rule_input_valid( rule_input, FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "input invlaid access rule" ); 
		goto _return; 
	}

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_ADD_APP_ACTION_TYPE_RULE, 
		rule_input, 
		length, 
		NULL, 
		0, 
		&ret_len );

#endif

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "add rule define failed \n" ) ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT unset_app_action_rule( access_rule_desc *rule_input, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( rule_input != NULL ); 
	ASSERT( length >= sizeof( access_rule_desc ) ); 

	ret = check_access_rule_input_valid( rule_input, FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "input invlaid access rule" ); 
		goto _return; 
	}

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_DEL_APP_ACTION_TYPE_RULE, 
		rule_input, 
		length, 
		NULL, 
		0, 
		&ret_len );

#endif

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "add rule define failed \n" ) ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT del_flt_url( PFILTER_URL_INPUT filt_rule, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	ASSERT( length >= sizeof( FILTER_URL_INPUT ) + filt_rule->length ); 

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_SEVEN_FW_UNSET_HTTP_FILTER_URL, 
		filt_rule, 
		sizeof( FILTER_URL_INPUT ) + filt_rule->length, 
		NULL, 
		0, 
		&ret_len );

#endif

	return ret; 
}

LRESULT del_flt_urls( PFILTER_URLS_INPUT filt_rules, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	ASSERT( length >= filt_rules->size ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_SEVEN_FW_UNSET_HTTP_FILTER_URLS, 
		filt_rules, 
		filt_rules->size, 
		NULL, 
		0, 
		&ret_len );

#endif

	log_trace( ( MSG_INFO, "leave %s 0x%0.8\n", __FUNCTION__, ret ) ); 
	return ret; 
}

LRESULT add_flt_url( PFILTER_URL_INPUT filt_rule, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 
	ULONG add_ret; 

	ASSERT( length >= sizeof( FILTER_URL_INPUT ) + filt_rule->length ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	//ASSERT( FALSE ); 
#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_SEVEN_FW_SET_HTTP_FILTER_URL, 
		filt_rule, 
		sizeof( FILTER_URL_INPUT ) + filt_rule->length, 
		&add_ret, 
		sizeof( ULONG ), 
		&ret_len ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( ret_len != sizeof( ULONG ) 
		|| add_ret != ADDED_NEW_FILTER )
	{
		ret = ERROR_LIST_ITEM_ALREADY_EXIST; 
	}
#endif

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT add_flt_urls( PFILTER_URLS_INPUT filt_rules, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	ASSERT( length >= filt_rules->size ); 

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_SEVEN_FW_SET_HTTP_FILTER_URLS, 
		filt_rules, 
		filt_rules->size, 
		NULL, 
		0, 
		&ret_len ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

#endif

_return:
	return ret; 
}

LRESULT get_flt_url( PFILTER_NAMES_OUTPUT filt_rule, ULONG length, PULONG ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_SEVEN_FW_GET_HTTP_FILTER_URLS, 
		NULL, 
		0, 
		filt_rule, 
		length, 
		ret_len );
#endif 

	return ret; 
}

#define ALL_FILTER_NAME_BUFF_LEN ( 1024 * 100 ) 
LRESULT get_all_url_rule( PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 
	PFILTER_NAMES_OUTPUT filt_rule = NULL; 
	PUI_CTRL ui_ctrl; 

	ASSERT( NULL != trace_log_dev ); 
	ASSERT( NULL != param ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	ui_ctrl = ( PUI_CTRL )context; 

	filt_rule = ( PFILTER_NAMES_OUTPUT )malloc( ALL_FILTER_NAME_BUFF_LEN ); 
	if( NULL == filt_rule )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ret = get_flt_url( filt_rule, ALL_FILTER_NAME_BUFF_LEN, &ret_len ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = ui_ctrl->ui_output_func( HTTP_URL_FLT_OUTPUT, ( INT32 )0, ( PBYTE )filt_rule, ret_len, param ); 

_return:
	if( filt_rule != NULL )
	{
		free( filt_rule ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

LRESULT del_flt_txt( PFILTER_NAME_INPUT filt_rule, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	ASSERT( length >= sizeof( FILTER_NAME_INPUT ) + filt_rule->length ); 

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( seven_fw_dev, 
		IOCTL_SEVEN_FW_UNSET_HTTP_FILTER_NAME, 
		filt_rule, 
		sizeof( FILTER_NAME_INPUT ) + filt_rule->length, 
		NULL, 
		0, 
		&ret_len );

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "device io control IOCTL_SEVEN_FW_UNSET_HTTP_FILTER_NAME failed error code 0x%0.8x\n", ret ) ); 
	}
#endif

	return ret; 
}

#ifdef HAVE_KRNL_HOOK
LRESULT set_bp_match_name( LPCWSTR match_name, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	ASSERT( buf_len > sizeof( WCHAR ) * 2 ); 

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( sys_manage_dev, 
		IOCTL_BITSAFE_SET_BP_MATCH_NAME, 
		( PVOID )match_name, 
		buf_len, 
		NULL, 
		0, 
		&ret_len ); 

	if( ret == ERROR_SUCCESS )
	{
		goto _return; 
	}

	log_trace( ( MSG_INFO, "device io control IOCTL_BITSAFE_SET_BP_MATCH_NAME failed error code 0x%0.8x\n", ret ) ); 
#endif //DEBUG_WITH_DEV

_return:
	return ret; 
}
#endif //HAVE_KRNL_HOOK

LRESULT add_flt_txt( PFILTER_NAME_INPUT filt_rule, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 
	ULONG add_ret; 

	ASSERT( length >= sizeof( FILTER_NAME_INPUT ) + filt_rule->length ); 

#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( seven_fw_dev, 
		IOCTL_SEVEN_FW_SET_HTTP_FILTER_NAME, 
		filt_rule, 
		length, 
		&add_ret, 
		sizeof( ULONG ), 
		&ret_len ); 

	if( ret == ERROR_SUCCESS )
	{
		if( ret_len == sizeof( ULONG ) 
			&& add_ret == ADDED_NEW_FILTER ) 
		{
			ret = ERROR_LIST_ITEM_ALREADY_EXIST; 
		}

		goto _return; 
	}

	log_trace( ( MSG_INFO, "device io control IOCTL_SEVEN_FW_SET_HTTP_FILTER_NAME failed error code 0x%0.8x\n", ret ) ); 
#endif

_return:
	return ret; 
}

LRESULT get_filter_names( PFILTER_NAMES_OUTPUT flt_rule, ULONG size, PULONG ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
#ifdef DEBUG_WITH_DEV
	ret = do_dev_io_ctrl( seven_fw_dev, 
		IOCTL_SEVEN_FW_GET_HTTP_FILTER_NAMES, 
		NULL, 
		0, 
		flt_rule, 
		size, 
		ret_len ); 
#endif 

	return ret; 
}

LRESULT get_all_seted_filter_txts( PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 
	PFILTER_NAMES_OUTPUT filt_rule; 
	PUI_CTRL ui_ctrl; 

	ASSERT( NULL != seven_fw_dev ); 

	ui_ctrl = ( PUI_CTRL )context; 

	filt_rule = ( PFILTER_NAMES_OUTPUT )malloc( ALL_FILTER_NAME_BUFF_LEN ); 
	if( NULL == filt_rule )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ret = get_filter_names( filt_rule, ALL_FILTER_NAME_BUFF_LEN, &ret_len ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	log_trace( ( MSG_INFO, "got the text rule length is %d \n", ret_len ) ); 

	ui_ctrl->ui_output_func( HTTP_TXT_FLT_OUTPUT, ( INT32 )0, ( PBYTE )filt_rule, ret_len, param ); 

_return:
	if( filt_rule != NULL )
	{
		free( filt_rule ); 
	}

	return ret; 
}

LRESULT init_net_mon_work_cont( PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "%s check netmon sys file existing \n", __FUNCTION__ ) ); 

	ret = load_netmon_driver(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "Open netmon device error, make sure netmon is staying in application directory." ) ); 
		goto _return; 
	}

_return: 
	return ret; 
}

LRESULT uninit_net_mon_work_cont()
{
	LRESULT ret = ERROR_SUCCESS; 
	ASSERT( NULL != net_mon_dev );

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return 0; 
}

LRESULT set_proc_traffic( PROCESS_TRAFFIC_CTRL* proc_traffic_ctrl, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	if( length != sizeof( PROCESS_TRAFFIC_CTRL ) )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = do_dev_io_ctrl( net_mon_dev, 
		IOCTL_CTRL_PROCESS_TRAFFIC, 
		proc_traffic_ctrl,
		sizeof( PROCESS_TRAFFIC_CTRL ), 
		NULL, 
		0, 
		&ret_len );

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "start monitor process traffic failed \n" ) ); 
	}

_return:
	return ret; 
}

INLINE LRESULT get_prcos_net_traffic( PROCESS_TRAFFIC_CTRL *pProcessInfo, DWORD *size )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = do_dev_io_ctrl( net_mon_dev, 
		IOCTL_GET_ALL_PROCESS_TRAFFIC, 
		NULL, 
		0, 
		pProcessInfo, 
		*size, 
		size );

	return ret; 
}

LRESULT trace_proc_traffic( PFLT_SETTINGS flt_setting, ULONG input_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	ASSERT( input_len >= sizeof( FLT_SETTINGS ) ); 

	ret = do_dev_io_ctrl( net_mon_dev, 
		IOCTL_TRACE_DATA_FLOW, 
		flt_setting, 
		sizeof( *flt_setting ), 
		NULL,
		0, 
		&ret_len ); 

	return ret; 
}

#ifdef HAVE_KRNL_HOOK
LRESULT start_sys_manage()
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD ret_len;

	ret = do_dev_io_ctrl( sys_manage_dev, 
		IOCTL_START_HOOK_FILTER_MSG, 
		NULL,
		0, 
		( PVOID )NULL, 
		0,  
		&ret_len );

	return ret;
}
#endif //HAVE_KRNL_HOOK

INLINE LRESULT response_sys_event( event_action_response* sys_event_respon, ULONG Size )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD ret_len;

	ASSERT( sys_event_respon != NULL ); 
	ASSERT( Size == sizeof( event_action_response ) ); 
	ASSERT( sys_event_respon->action == ACTION_ALLOW 
		|| sys_event_respon->action == ACTION_BLOCK ); 
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ret = do_dev_io_ctrl( trace_log_dev, 
		IOCTL_RESPONSE_ACTION_EVENT,  
		sys_event_respon, 
		sizeof( *sys_event_respon ), 
		NULL,
		0, 
		&ret_len );

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret;
}

#ifdef HAVE_KRNL_HOOK

LRESULT end_sys_manage()
{
	LRESULT ret = ERROR_SUCCESS;
	DWORD ret_len;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ret = do_dev_io_ctrl( sys_manage_dev, 
		IOCTL_RESET_HOOK_FILTER, 
		NULL,
		0, 
		( PVOID )NULL, 
		0, 
		&ret_len );

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret;
}

LRESULT stop_sys_manage()
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD ret_len;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	ret = do_dev_io_ctrl( sys_manage_dev, 
		IOCTL_END_HOOK_FILTER_MSG, 
		NULL,
		0, 
		( PVOID )NULL, 
		0, 
		&ret_len ); 

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret;
}

LRESULT uninit_sys_mon_work_cont()
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	if( NULL != sys_manage_dev )
	{
		stop_sys_manage(); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

	return ret; 
}

LRESULT uninstall_sys_mon_work_cont()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	_ret = uninit_sys_mon_work_cont(); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "end sys action monitor failed\n" ) ); 
		ret = _ret; 
	}

	_ret = unload_sys_mon_driver(); 

	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload the sys mgr failed\n" ) ); 
		ret = _ret; 
	}

	_ret = unload_tracelog_driver(); 
	
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload the sys mgr failed\n" ) ); 
		ret = _ret; 
	}

_return:
	return ret; 
}

LRESULT set_sys_mon_flt( PHOOK_FILTER_INFO flt_info, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	ret = do_dev_io_ctrl( sys_manage_dev, 
		IOCTL_SET_HOOK_FILTER, 
		flt_info,
		MAX_HOOK_INFO_SIZE, 
		( PVOID )NULL, 
		0, 
		&ret_len );


	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ret; 
}

#endif //HAVE_KRNL_HOOK

LRESULT antiarp_get_arp_info( PVOID input_buf, ULONG input_length, PVOID output_buf, ULONG output_length, ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ret = do_safe_dev_io_ctl( 
		anti_arp_dev, 
		IOCTL_ANTI_ARP_OUTPUT_MAC_LIST, 
		input_buf, 
		input_length, 
		output_buf, 
		output_length, 
		ret_len ); 

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x \n", __FUNCTION__, ret ) ); 
	return ret; 
}


LRESULT config_proc_trace_data_size( ULONG proc_id, ULONG data_size )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len; 
	event_trace_config trace_config; 

	do
	{
		ASSERT( proc_id != ( ULONG )INVALID_PROCESS_ID ); 

		if( data_size > MAX_TRACE_DATA_SIZE )
		{
			data_size = MAX_TRACE_DATA_SIZE; 
		}

		if( data_size < MIN_TRACE_DATA_SIZE )
		{
			data_size = MIN_TRACE_DATA_SIZE; 
		}

		trace_config.proc_id = proc_id; 
		trace_config.trace_data_size = data_size; 

		ret = do_dev_io_ctrl( trace_log_dev, 
			IOCTL_EVENT_TRACE_CONFIG, 
			&trace_config, 
			sizeof( trace_config ), 
			NULL, 
			0, 
			&ret_len ); 

	}while( FALSE ); 

	return ret; 
}

INLINE LRESULT antiarp_arp_mod_inited()
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len;

	ret = do_dev_io_ctrl( anti_arp_dev, 
		IOCTL_ANTI_ARP_INITIALIZED, 
		NULL, 
		0, 
		NULL, 
		0, 
		&ret_len ); 

	return ret; 
}

INLINE LRESULT set_anti_arp_work_mode( ULONG work_mode )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len;

	ret = do_dev_io_ctrl( anti_arp_dev, 
		IOCTL_ANTI_ARP_SET_WORK_MODE, 
		&work_mode, 
		sizeof( ULONG ), 
		NULL, 
		0, 
		&ret_len ); 

	return ret; 
}

#define MAX_EVENT_ITEM_COUNT 256 //1048576
#define MAX_EVENT_ITEM_SIZE ( sizeof( sys_action_output ) + DEFAULT_OUTPUT_DATA_REGION_SIZE ) 

#define MAX_R3_NOTIFY_ITEM_COUNT 256 //( 1 << 20 ) //1048576
#define MAX_R3_NOTIFY_ITEM_SIZE R3_NOTIFY_BUF_SIZE

ULONG r3_array_buf_size[ MAX_R3_ARRAY_TYPE ] = { 
	MAX_EVENT_ITEM_COUNT * ( MAX_EVENT_ITEM_SIZE + sizeof( array_cell_head ) ), 
	MAX_R3_NOTIFY_ITEM_COUNT * ( MAX_R3_NOTIFY_ITEM_SIZE + sizeof( array_cell_head ) ) }; 

	ULONG r3_array_buf_cell_size[ MAX_R3_ARRAY_TYPE ] = { 
		MAX_EVENT_ITEM_SIZE, 
		MAX_R3_NOTIFY_ITEM_SIZE }; 

LRESULT ctrl_event_trace_config( event_trace_config *config )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len;

	do 
	{
		ASSERT( config != NULL ); 

		ret = do_dev_io_ctrl( trace_log_dev, 
			IOCTL_EVENT_TRACE_CONFIG, 
			config, 
			sizeof( *config ), 
			NULL, 
			NULL, 
			&ret_len ); 

	}while( FALSE );

	return ret; 
}

INLINE LRESULT ctrl_tracelog_interact( BOOLEAN is_start )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len;

	if( is_start == TRUE )
	{
		ret = do_dev_io_ctrl( trace_log_dev, 
			IOCTL_START_TRACE_LOG_UI_INTERACT, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			&ret_len ); 
	}
	else
	{
		ret = do_dev_io_ctrl( trace_log_dev, 
			IOCTL_STOP_TRACE_LOG_UI_INTERACT, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			&ret_len ); 
	}

	return ret; 
}

INLINE LRESULT relearn_rule()
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len;
	
	ASSERT( trace_log_dev != NULL ); 

	{
		ret = do_dev_io_ctrl( trace_log_dev, 
			IOCTL_RESET_LEARNED_RULE, 
			NULL, 
			0, 
			NULL, 
			0, 
			&ret_len ); 
	}

	return ret; 
}

INLINE LRESULT ctrl_icmp_ping( BOOLEAN is_block )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG ret_len;
	ULONG block_op; 

	block_op = is_block; 
	{
		ret = do_dev_io_ctrl( trace_log_dev, 
			IOCTL_BLOCK_PING, 
			&block_op, 
			sizeof( block_op ), 
			NULL, 
			0, 
			&ret_len ); 
	}
	return ret; 
}

INLINE LRESULT start_arp_filter()
{
	return set_anti_arp_work_mode( ANTI_ARP_WORK ); 
}

INLINE LRESULT stop_arp_filter()
{
	return set_anti_arp_work_mode( ANTI_ARP_NO_WORK ); 
}

INLINE LRESULT uninit_anti_arp( PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	_ret = unload_anti_arp_driver(); 
	if( _ret != ERROR_SUCCESS )
	{
		ret = _ret; 
		log_trace( ( MSG_ERROR, "unload the arp driver failed \n" ) ); 
	}

	return ret; 
}

INLINE LRESULT init_anti_arp( PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR tmp_text; 

	ret = load_anti_arp_driver(); 
	if( ret != ERROR_SUCCESS )
	{
		ret = install_anti_arp_driver(); 
		if( ret != ERROR_SUCCESS )
		{
#define ANTI_ARP_NEED_RESTART_TIP _T( "由于系统环境原因,反ARP功能无法正常加载,需要系统重启,是否立即重启?" )
			
			tmp_text = _get_string_by_id( TEXT_ANTI_ARP_NEED_REBOOT_TIP, ANTI_ARP_NEED_RESTART_TIP ); 

			if( IDYES == MessageBox( NULL, tmp_text, _T( "BitSafe" ), MB_YESNO ) )
			{
				windows_shutdown(EWX_REBOOT | EWX_FORCE); 
			}
			else
			{
				goto _return; 
			}
		}

		ret = load_anti_arp_driver(); 
		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ANTI_ARP_NEED_REBOOT_TIP, ANTI_ARP_NEED_RESTART_TIP ); 

			if( IDYES == MessageBox( NULL, tmp_text, _T( "BitSafe" ), MB_YESNO ) )
			{
				windows_shutdown(EWX_REBOOT | EWX_FORCE); 
			}
			else
			{
				goto _return; 
			}
		}
	}

_return:
	if( ret != ERROR_SUCCESS )
	{
		unload_anti_arp_driver(); 
	}

	return ret; 
}

LRESULT _unload_main_drivers()
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ret = unload_netmon_driver();
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload_netmon_driver failed \n" ) ); 
	}

	ret = unload_net_filter_driver();
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload_net_filter_driver failed \n" ) ); 
	}

	ret = unload_tracelog_driver(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload_tracelog_driver driver failed \n" ) ); 
	}

	unload_sevenfw_driver(); 

	ret = unload_anti_arp_driver();
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!uninstall net filter driver failed \n" ) ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT unload_service_context()
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ret = unload_driver_work_sets( FALSE ); 
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT unload_ui_context()
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 


	ret = unload_driver_work_sets( TRUE ); 
	
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}


LRESULT install_main_drivers()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = install_event_mon_drv(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "load trace log driver failed \n" ) ); 
	}

	return ret; 
}

#define SERVICE_CONTEXT_WORK 1
#define UI_CONTEXT_WORK 2

/*****************************************************************************************************************
NEEDED MODULE LIST:
EVENTMON THE DRIVER CONTAIN 3 FUNCTION.
SEVENFW NDIS DRIVER, CAN GIVE SOME LOWEST INFORMATION ABOUT NETWORK ACTIVITIES. REMOVE IT IN VERSION 1.
AND OTHER DRIVERS BASED ON SEVENFW WILL BE REMOVED IN VERSION 1.
*****************************************************************************************************************/

LRESULT load_sevenfw_work_set( BOOLEAN request_user, PVOID user_context, PVOID work_context, BOOLEAN *can_install )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _can_install;  

	ASSERT( can_install != NULL ); 

	_can_install = *can_install; 

	if( TRUE == is_vista_or_later_ex() )
	{
		ret = load_sevenfw_ex_device(); 
	}
	else
	{
		ret = load_sevenfw_device(); 
	}

	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "load_sevenfw_device failed \n" ) ); 
		
#ifdef DRIVER_MUST_INSTALL_BY_USER
		if( request_user == FALSE )
		{
			DBG_BRK(); 
			goto _return; 
		}
#endif //DRIVER_MUST_INSTALL_BY_USER
		if( ret != ERROR_FILE_NOT_FOUND )
		{
			DBG_BRK(); 
			goto _return; 
		}

		if( _can_install == UNKNOWN_RESULT )
		{
			if( request_user_action( user_context ) != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				ret = ERROR_USER_CANCEL_INSTALL; 
				_can_install = FALSE; 			
				goto _return; 
			}

			_can_install = TRUE; 
		}
		else if( _can_install == FALSE )
		{
			goto _return; 
		}

		if( TRUE == is_vista_or_later_ex() )
		{
			ret = safe_install_sevenfw_ex(); 
		}
		else
		{
			ret = safe_install_sevenfw_driver(); 
		}

		if( ret != ERROR_SUCCESS )
		{
			DBG_BRK(); 
			goto _return; 
		}

		if( TRUE == is_vista_or_later_ex() )
		{
			ret = load_sevenfw_ex_device(); 
		}
		else
		{
			ret = load_sevenfw_device(); 
		}

		if( ret != ERROR_SUCCESS )
		{
			DBG_BRK(); 
			goto _return; 
		}
	}



_return:
	*can_install = _can_install; 
	return ret; 
}

LRESULT stop_sevenfw_work_set()
{
	unload_sevenfw_driver(); 

	return ERROR_SUCCESS; 
}

LRESULT load_trace_log_work_set( BOOLEAN request_user, PVOID user_context, PVOID work_context, BOOLEAN *can_install )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	BOOLEAN _can_install; 
	notify_events_set *all_events = NULL; 

	ASSERT( can_install != NULL ); 


	_can_install = *can_install; 

#ifdef UNLOAD_DRIVER_BY_UNINSTALL
	_ret = check_bitsafe_function_state( TRACE_LOG_FUNCTION, &load_state ); 
	if( _ret != ERROR_SUCCESS )
	{
		load_state = BITSAFE_FUNCTION_DISABLED; 
	}

	if( load_state == BITSAFE_FUNCTION_DISABLED )
#endif //UNLOAD_DRIVER_BY_UNINSTALL

	{	
		/*******************************************************************************
		maybe return STATUS_FLT_INSTANCE_ALTITUDE_COLLISION 
		solution:
		1.check the return code, change the altitude value, try again.
		2.manual check all altitude value in the system,select suitable value.
		3.?
		*******************************************************************************/

		ret = load_tracelog_driver(); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "load trace log driver failed \n" ) ); 

#ifdef DRIVER_MUST_INSTALL_BY_USER
			if( request_user == FALSE )
			{
				goto _return; 
			}
#endif //DRIVER_MUST_INSTALL_BY_USER

			if( ret != ERROR_FILE_NOT_FOUND )
			{
				log_trace( ( MSG_INFO, "load trace log failed, return 0x%0.8x\n", ret ) ); 
				//goto _return; 
			}

			if( _can_install == UNKNOWN_RESULT )
			{
				if( request_user_action( user_context ) != ERROR_SUCCESS )
				{
					DBG_BRK(); 
					ret = ERROR_USER_CANCEL_INSTALL; 
					_can_install = FALSE; 			
					goto _return; 
				}

				_can_install = TRUE; 
			}
			else if( _can_install == FALSE )
			{
				goto _return; 
			}

			{
				ret = install_event_mon_drv(); 
				if( ret != ERROR_SUCCESS )
				{
					ret = install_event_mon_drv(); 
					if( ret != ERROR_SUCCESS )
					{
						DBG_BRK(); 
						goto _return; 
					}
				}

				ret = load_tracelog_driver(); 
				if( ret != ERROR_SUCCESS )
				{
					ret = load_tracelog_driver(); 
					if( ret != ERROR_SUCCESS )
					{
						DBG_BRK(); 
						goto _return; 
					}
				}
			}
		}

		if( request_user == TRUE )
		{
			ret = ctrl_tracelog_interact( TRUE ); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				log_trace( ( MSG_ERROR, "set trace log device to interactive mode failed\n" ) ); 
			}
		}
	}

	if( request_user == FALSE )
	{
#ifdef UNLOAD_DRIVER_BY_UNINSTALL
		_ret = check_bitsafe_function_state( TRACE_LOG_FUNCTION, &load_state ); 
		if( _ret != ERROR_SUCCESS )
		{
			load_state = BITSAFE_FUNCTION_DISABLED; 
		}

		if( load_state == BITSAFE_FUNCTION_DISABLED )
#endif //UNLOAD_DRIVER_BY_UNINSTALL	
	}
	else
	{
		ASSERT( user_context != NULL ); 

		init_cs_lock( sys_event_lock ); 

#ifdef UNLOAD_DRIVER_BY_UNINSTALL
		_ret = check_bitsafe_function_state( TRACE_LOG_FUNCTION, &load_state ); 
		if( _ret != ERROR_SUCCESS )
		{
			load_state = BITSAFE_FUNCTION_DISABLED; 
		}

		if( load_state == BITSAFE_FUNCTION_DISABLED )
#endif //UNLOAD_DRIVER_BY_UNINSTALL

#ifdef  UNLOAD_DRIVER_BY_UNINSTALL
		_ret = check_bitsafe_function_state( TRACE_LOG_FUNCTION, &load_state ); 
		if( _ret != ERROR_SUCCESS )
		{
			load_state = BITSAFE_FUNCTION_DISABLED; 
		}

		if( load_state == BITSAFE_FUNCTION_DISABLED )
#endif //UNLOAD_DRIVER_BY_UNINSTALL

		{
			_ret = init_bitsafe_action_handler( work_context, user_context ); 
			if( _ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "load the ring3 system event handler error!\n" ) ); 
			}
		}
	}

_return:
	if( all_events != NULL )
	{
		free( all_events ); 
	}

	if( ret != ERROR_SUCCESS )
	{

	}

	*can_install = _can_install; 
	return ret; 
}

LRESULT stop_trace_log_work_set( BOOLEAN request_user )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
#ifdef UNLOAD_DRIVER_BY_UNINSTALL
	bitsafe_function_state load_state; 
#endif //UNLOAD_DRIVER_BY_UNINSTALL

	do 
	{
		_ret = uninit_bitsafe_action_handler(); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "unload the ring3 system event handler error!\n" ) ); 
		}
	}while( FALSE ); 

	ret = unload_tracelog_driver(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload_tracelog_driver driver failed \n" ) ); 
	}

	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 

	return ret; 
}

#ifdef HAVE_KRNL_HOOK

LRESULT load_sys_mng_work_set( BOOLEAN request_user, PVOID user_context, PVOID work_context, BOOLEAN *can_install )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	BOOLEAN _can_install; 
	bitsafe_function_state load_state; 

	ASSERT( can_install != NULL ); 

	_can_install = *can_install; 

#ifdef UNLOAD_DRIVER_BY_UNINSTALL
_ret = check_bitsafe_function_state( SYS_MANAGE_FUNCTION, &load_state ); 
if( _ret != ERROR_SUCCESS )
{
	load_state = BITSAFE_FUNCTION_DISABLED; 
}

if( load_state == BITSAFE_FUNCTION_DISABLED )
#endif //UNLOAD_DRIVER_BY_UNINSTALL
{
	ret = load_sys_mon_driver(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "load sys mon driver failed \n" ) ); 

#ifdef DRIVER_MUST_INSTALL_BY_USER
		if( request_user == FALSE )
		{
			goto _return; 
		}
#endif //DRIVER_MUST_INSTALL_BY_USER

		if( ret != ERROR_FILE_NOT_FOUND )
		{
			DBG_BRK(); 
			goto _return; 
		}

		if( _can_install == UNKNOWN_RESULT )
		{
			if( request_user_action( user_context ) != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				ret = ERROR_USER_CANCEL_INSTALL; 
				_can_install = FALSE; 			
				goto _return; 
			}

			_can_install = TRUE; 
		}
		else if( _can_install == FALSE )
		{
			goto _return; 
		}

		ret = install_sys_mon_driver(); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BRK(); 
			goto _return; 
		}

		ret = load_sys_mon_driver(); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BRK(); 
			goto _return; 
		}
	}
}

_return:
	*can_install = _can_install; 

	return ret; 
}

#endif //HAVE_KRNL_HOOK

LRESULT stop_sys_mon_work_set()
{
	LRESULT ret = ERROR_SUCCESS; 

#ifdef HAVE_KRNL_HOOK
	ret = unload_sys_mon_driver(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload_sys_mon_driver failed \n" ) ); 
	}
#endif //HAVE_KRNL_HOOK

	return ret; 
}

LRESULT load_anti_arp_work_set( BOOLEAN request_user, PVOID user_context, PVOID work_context, BOOLEAN *can_install )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN _can_install; 

	ASSERT( can_install != NULL ); 

	_can_install = *can_install; 

#ifdef UNLOAD_DRIVER_BY_UNINSTALL
	_ret = check_bitsafe_function_state( ANTI_ARP_ATTACK_FUNCTION, &load_state ); 
	if( _ret != ERROR_SUCCESS )
	{
		load_state = BITSAFE_FUNCTION_DISABLED; 
	}

	if( load_state == BITSAFE_FUNCTION_DISABLED )
#endif //UNLOAD_DRIVER_BY_UNINSTALL
	{
		ret = load_anti_arp_driver();
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "load net filter driver failed \n" ) ); 
			if( FALSE == request_user )
			{
				goto _return; 
			}

			if( ret != ERROR_FILE_NOT_FOUND )
			{
				DBG_BRK(); 
				goto _return; 
			}

			if( _can_install == UNKNOWN_RESULT )
			{
				if( request_user_action( user_context ) != ERROR_SUCCESS )
				{
					DBG_BRK(); 
					ret = ERROR_USER_CANCEL_INSTALL; 
					_can_install = FALSE; 			
					goto _return; 
				}

				_can_install = TRUE; 
			}
			else if( _can_install == FALSE )
			{
				goto _return; 
			}

			ret = install_anti_arp_driver(); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				goto _return; 
			}

			ret = load_anti_arp_driver(); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				goto _return; 
			}
		}
	}

_return:
	*can_install = _can_install; 
	return ret; 
}

LRESULT stop_anti_arp_work_set()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = unload_anti_arp_driver();
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload_net_filter_driver failed \n" ) ); 
	}

	return ret; 
}

LRESULT load_http_flt_work_set( BOOLEAN request_user, PVOID user_context, PVOID work_context, BOOLEAN *can_install )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN _can_install; 

	ASSERT( can_install != NULL ); 

	_can_install = *can_install; 

#ifdef UNLOAD_DRIVER_BY_UNINSTALL
	_ret = check_bitsafe_function_state( HTTP_URL_FLT_FUNCTION, &load_state ); 
	if( _ret != ERROR_SUCCESS )
	{
		load_state = BITSAFE_FUNCTION_DISABLED; 
	}

	if( load_state == BITSAFE_FUNCTION_DISABLED )
#endif //UNLOAD_DRIVER_BY_UNINSTALL
	{
		ret = load_net_filter_driver();
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "load net filter driver failed \n" ) ); 
			if( FALSE == request_user )
			{
				goto _return; 
			}

			if( ret != ERROR_FILE_NOT_FOUND )
			{
				DBG_BRK(); 
				goto _return; 
			}

			if( _can_install == UNKNOWN_RESULT )
			{
				if( request_user_action( user_context ) != ERROR_SUCCESS )
				{
					DBG_BRK(); 
					ret = ERROR_USER_CANCEL_INSTALL; 
					_can_install = FALSE; 			
					goto _return; 
				}

				_can_install = TRUE; 
			}
			else if( _can_install == FALSE )
			{
				goto _return; 
			}

			ret = install_net_filter_driver(); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				goto _return; 
			}

			ret = load_net_filter_driver(); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				goto _return; 
			}
		}
	}

_return:
	*can_install = _can_install; 
	return ret; 
}

LRESULT stop_http_flt_work_set()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = unload_net_filter_driver();
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload_net_filter_driver failed \n" ) ); 
	}

	return ret; 
}

LRESULT stop_other_work_set()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = unload_anti_arp_driver();
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!uninstall net filter driver failed \n" ) ); 
	}

	return ret; 
}

LRESULT load_reg_flt_work_set( BOOLEAN request_user, PVOID user_context, PVOID work_context, BOOLEAN *can_install )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN _can_install; 

	ASSERT( can_install != NULL ); 

	_can_install = *can_install; 

	ret = load_reg_flt_driver();
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "load net monitor driver failed \n" ) ); 

#ifdef DRIVER_MUST_INSTALL_BY_USER
		if( request_user == FALSE )
		{
			goto _return; 
		}
#endif //DRIVER_MUST_INSTALL_BY_USER

		if( ret != ERROR_FILE_NOT_FOUND )
		{
			DBG_BRK(); 
			goto _return; 
		}

		if( _can_install == UNKNOWN_RESULT )
		{
			if( request_user_action( user_context ) != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				ret = ERROR_USER_CANCEL_INSTALL; 
				_can_install = FALSE; 			
				goto _return; 
			}

			_can_install = TRUE; 
		}
		else if( _can_install == FALSE )
		{
			goto _return; 
		}

		ret = load_reg_flt_driver(); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BRK(); 
			goto _return; 
		}
	}

_return:
	*can_install = _can_install; 

	return ret; 
}

LRESULT stop_reg_flt_work_set()
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = unload_reg_flt_driver(); 

	return ret; 
}

LRESULT load_net_mon_work_set( BOOLEAN request_user, PVOID user_context, PVOID work_context, BOOLEAN *can_install )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN _can_install; 

	ASSERT( can_install != NULL ); 

	_can_install = *can_install; 

#ifdef UNLOAD_DRIVER_BY_UNINSTALL
	if( TRUE == is_vista_or_later_ex() )
	{
		_ret = check_bitsafe_function_state( NET_COMM_MANAGE_FUNCTION, &load_state ); 
	}
	else
	{
		_ret = check_bitsafe_function_state( NET_FW_DRIVER, &load_state ); 
	}

	if( _ret != ERROR_SUCCESS )
	{
		load_state = BITSAFE_FUNCTION_DISABLED; 
	}

	if( load_state == BITSAFE_FUNCTION_DISABLED )
#endif //UNLOAD_DRIVER_BY_UNINSTALL

	{
		ret = load_netmon_driver();
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_INFO, "load net monitor driver failed \n" ) ); 
#ifdef DRIVER_MUST_INSTALL_BY_USER
			if( request_user == FALSE )
			{
				goto _return; 
			}
#endif //DRIVER_MUST_INSTALL_BY_USER

			if( ret != ERROR_FILE_NOT_FOUND )
			{
				DBG_BRK(); 
				goto _return; 
			}

			if( _can_install == UNKNOWN_RESULT )
			{
				if( request_user_action( user_context ) != ERROR_SUCCESS )
				{
					DBG_BRK(); 
					ret = ERROR_USER_CANCEL_INSTALL; 
					_can_install = FALSE; 			
					goto _return; 
				}

				_can_install = TRUE; 
			}
			else if( _can_install == FALSE )
			{
				goto _return; 
			}

			ret = load_netmon_driver(); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				goto _return; 
			}
		}
	}

_return:
	*can_install = _can_install; 

	return ret; 
}

LRESULT stop_net_mon_work_set( )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = unload_netmon_driver();
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "unload_netmon_driver failed \n" ) ); 
	}

	return ret; 
}

LRESULT load_fs_mng_work_set( BOOLEAN request_user, PVOID user_context, PVOID work_context, BOOLEAN *can_install )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN _can_install; 

	ASSERT( can_install != NULL ); 

	_can_install = *can_install; 
	ret = load_fs_mng_driver();
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "load net monitor driver failed \n" ) ); 
#ifdef DRIVER_MUST_INSTALL_BY_USER
		if( request_user == FALSE )
		{
			goto _return; 
		}
#endif //DRIVER_MUST_INSTALL_BY_USER

		if( ret != ERROR_FILE_NOT_FOUND )
		{
			log_trace( ( MSG_ERROR, "load file manage mini port return error that's not \"not found file\" \n", ret ) ); 
		}

		if( _can_install == UNKNOWN_RESULT )
		{
			if( request_user_action( user_context ) != ERROR_SUCCESS )
			{
				DBG_BRK(); 
				ret = ERROR_USER_CANCEL_INSTALL; 
				_can_install = FALSE; 			
				goto _return; 
			}

			_can_install = TRUE; 
		}
		else if( _can_install == FALSE )
		{
			goto _return; 
		}

		ret = load_fs_mng_driver(); 
		if( ret != ERROR_SUCCESS )
		{
			DBG_BRK(); 
			goto _return; 
		}
	}

	*can_install = _can_install; 

_return:
	return ret; 
}

LRESULT stop_fs_mng_work_set( )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = unload_fs_mng_driver(); 

	return ret; 
}

LRESULT WINAPI notify_error_msg( LPCWSTR msg ); 
LRESULT WINAPI lite_work( ULONG flags, PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		do 
		{
			ret = load_tracelog_driver(); 
			if( ret == ERROR_SUCCESS )
			{
				log_trace( ( MSG_WARNING, "eventmon driver already running\n" ) ); 
				break; 
			}

			ret = load_driver_work_sets( flags, param, context ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_WARNING, "load eventmon driver error %u\n", ret ) ); 
			}
		}while( FALSE ); 

		if( ret == ERROR_SUCCESS )
		{
			ret = check_driver_version(); 
			if( ret != ERROR_SUCCESS )
			{
				_ret = notify_error_msg( L"BITTRACE其它版本已经运行,重启系统更新至当前版本." ); 
				exit( 0 ); 
				break; 
			}

#ifdef SUPPORT_LITE_WORK
			_ret = ctrl_drivers_work_state(); 
			if( _ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_WARNING, "control driver work state error %u\n", _ret ) ); 
			}
#endif //SUPPORT_LITE_WORK
		}
		
#ifdef SUPPORT_LITE_WORK
		//如果在这里进行卸载，那么如果程序退出后，就无法再次打开EVENTMON驱动
		_ret = delete_event_mon_driver_file(); 
		if( _ret != ERROR_SUCCESS )
		{
			//break; 
			log_trace( ( MSG_WARNING, "delete eventmon driver file error %u\n", _ret ) ); 
		}
#endif //SUPPORT_LITE_WORK

	}while( FALSE );

	return ret;  
}

LRESULT unload_driver_work_sets( BOOLEAN ui_action )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		_ret = stop_fs_mng_work_set(); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

#ifdef HAVE_KRNL_HOOK
		_ret = stop_sys_mon_work_set(); 
		if( _ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			log_trace( ( MSG_INFO, "load system monitor work set error 0x%0.8x\n", ret ) ); 

			ret = _ret; 
		}
#endif //HAVE_KRNL_HOOK

		_ret = stop_reg_flt_work_set(); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		_ret = stop_net_mon_work_set(); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		_ret = stop_other_work_set(); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		_ret = stop_http_flt_work_set(); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		_ret = stop_trace_log_work_set( ui_action ); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		_ret = stop_sevenfw_work_set(); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}
	}while( FALSE ); 
	
	return ret; ;
}

//whether direct install all drivers by service,when the service is already installed.
LRESULT load_driver_work_sets( ULONG flags, PVOID ui_context, PVOID work_context )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	BOOLEAN can_install = TRUE;

	BOOLEAN request_ui; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	do 
	{
		switch( flags )
		{
		case UI_CONTEXT_WORK:
			request_ui = TRUE; 
			break; 
		case SERVICE_CONTEXT_WORK: 
			request_ui = FALSE; 
			break; 
		default: 
			ASSERT( FALSE && "invalid driver work context" ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
			break; 
		}
	}while( FALSE ); 

	if( flags == UI_CONTEXT_WORK )
	{
#ifndef REMOVE_TRACE_FROM_NDIS
		if( all_bitsafe_function_flags[ NET_PACKET_FLT_FUNCTION ] == BITSAFE_FUNCTION_OPENED )
		{
			_ret = load_sevenfw_work_set( request_ui, ui_context, work_context, &can_install ); 
			if( _ret != ERROR_SUCCESS )
			{
				DBG_BP(); 
				log_trace( ( MSG_INFO, "load sevenfw work set error 0x%0.8x\n", ret ) ); 

				ret = _ret; 
			}
		}
#endif //REMOVE_TRACE_FROM_NDIS
	}

	{
		_ret = load_trace_log_work_set( request_ui, ui_context, work_context, &can_install ); 
		if( _ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			log_trace( ( MSG_INFO, "load trace log work set error 0x%0.8x\n", ret ) ); 
		
			ret = _ret; 
		}
	}


	if( flags == UI_CONTEXT_WORK )
	{

#ifndef REMOVE_TRACE_FROM_NDIS
		if( all_bitsafe_function_flags[ HTTP_URL_FLT_FUNCTION ] == BITSAFE_FUNCTION_OPENED )
		{
			_ret = load_http_flt_work_set( request_ui, ui_context, work_context, &can_install ); 
			if( _ret != ERROR_SUCCESS )
			{
				DBG_BP(); 
				log_trace( ( MSG_INFO, "load http filter work set error 0x%0.8x\n", ret ) ); 

				ret = _ret; 
			}
		}
#endif //REMOVE_TRACE_FROM_NDIS
	}

_return:
	log_trace( ( MSG_INFO, "leave %s, 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT load_service_context( PVOID context, PVOID param )
{

	LRESULT ret = ERROR_SUCCESS;  

	ASSERT( param != NULL ); 
	ASSERT( context != NULL ); 


	ret = lite_work( SERVICE_CONTEXT_WORK, context, param ); 

	return ret; 
}

LRESULT load_ui_context( PVOID context, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( param != NULL ); 
	ASSERT( context != NULL ); 
	
	ret = lite_work( UI_CONTEXT_WORK, context, param ); 

	return ret; 
}

LRESULT _uninstall_main_drivers()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	BOOLEAN is_vista; 

	is_vista = is_vista_or_later_ex(); 

#ifdef HAVE_KRNL_HOOK

	if( sys_manage_dev != NULL )
	{
		_ret = stop_sys_manage(); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "stop sys manage failed\n" ) ); 
		}
	}

	_ret = uninstall_sys_mon_driver(); 
	if( _ret != ERROR_SUCCESS )
	{
		ret = _ret; 
		log_trace( ( MSG_ERROR, "!!!uninstall sys mon driver failed \n" ) ); 
	}
#endif //HAVE_KRNL_HOOK

	_ret = uninstall_net_filter_driver();
	if( _ret != ERROR_SUCCESS )
	{
		ret = _ret; 

		log_trace( ( MSG_ERROR, "!!!uninstall net filter driver failed \n" ) ); 
	}

	_ret = uninstall_event_mon_drv(); 
	if( _ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "uninstall trace log driver failed \n" ) ); 
	}

	{
		_ret = uninstall_sevenfw_ex(); 
		if( _ret != ERROR_SUCCESS )
		{
			if( is_vista == TRUE )
			{
				ret = _ret; 
			}

			log_trace( ( MSG_ERROR, "!!!uninstall seven fw failed \n" ) ); 
		}
	}

	{
		_ret = uninstall_sevenfw(); 

		if( _ret != ERROR_SUCCESS )
		{
			if( is_vista == FALSE )
			{
				ret = _ret; 
			}

			log_trace( ( MSG_ERROR, "!!!unistall seven fw failed \n" ) ); 
		}
	}

	_ret = uninstall_anti_arp_driver();
	if( _ret != ERROR_SUCCESS )
	{
		ret = _ret; 
		log_trace( ( MSG_ERROR, "!!!uninstall net filter driver failed \n" ) ); 
	}

	return ret; 
}

driver_os_type is_vista_driver( bitsafe_function_type function_type )
{
	driver_os_type ret = ALL_COMP_DRIVER; 

	do 
	{
		switch( function_type )
		{
		case NET_PACKET_FLT_FUNCTION: 
			ret = WINXP_DRIVER; 
			break; 
		case HTTP_TEXT_FLT_FUNCTION: 
			ret = ALL_COMP_DRIVER; 
			break; 
		case HTTP_URL_FLT_FUNCTION: 
			ret = ALL_COMP_DRIVER; 
			break; 
		case ANTI_ARP_ATTACK_FUNCTION: 
			ret = ALL_COMP_DRIVER; 
			break;  
		case NET_COMM_MANAGE_FUNCTION: 
			ret = WINXP_DRIVER; 
			break; 
		case REG_MANAGE_FUNCTION:
			ret = WINXP_DRIVER; 
			break; 
		case FILE_MANAGE_FUNCTION: 
			break; 
		default:
			ASSERT( FALSE ); 
			ret = ALL_COMP_DRIVER; 
			break; 
		}
	
	}while( FALSE );
	
	return ret; 
}

LRESULT delete_main_drivers()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	BOOLEAN is_vista; 
	bitsafe_function_type i; 

	is_vista = is_vista_or_later_ex(); 

	for( i = FUNCTION_TYPE_BEGIN; i < MAX_FUNCTION_TYPE; i = ( bitsafe_function_type )( ( ULONG )i + 1 ) )
	{
		_ret = ctrl_bitsafe_driver( i, DELETE_DRIVER ); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "can't output all drivers\n" ) ); 
			ret = _ret; 
		}
	}
	return ret; 
}

typedef struct _updated_file_info
{
	ULONG file_ver; 
} updated_file_info, *pupdated_file_info; 

typedef LRESULT ( CALLBACK *update_file_callback )( updated_file_info *file_info, PVOID context ); 

LRESULT update_main_drivers()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	bitsafe_function_type i; 
	BOOLEAN is_vista; 

	is_vista = is_vista_or_later_ex(); 

	for( i = FUNCTION_TYPE_BEGIN; i < MAX_FUNCTION_TYPE; i = ( bitsafe_function_type )( ( ULONG )i + 1 )  )
	{

		_ret = ctrl_bitsafe_driver( i, OUTPUT_DRIVER ); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "can't output all drivers\n" ) ); 

			ret = _ret; 
		}
	}

	return ret; 
}

LRESULT uninstall_main_drivers( INT32 is_service )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = _uninstall_main_drivers(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "!!!uninstall main drivers failed \n" ) ); 
	}

	return ret; 
}

LRESULT sevefw_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
						 PBYTE data, 
						 ULONG length, 
						 PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 

	notify_data data_notify; 
	action_ui_notify *action_notify; 
	action_ui_notify_type notify_type = UNKNOWN_NOTIFY; ; 

	log_trace( ( MSG_INFO, "enter %s aciton id is %ws(0x%0.8x)\n", 
		__FUNCTION__, 
		get_output_work_desc( ui_action_id ), 
		ui_action_id ) ); 

	if( ret_val != ERROR_SUCCESS )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	data_notify.data = data; 
	data_notify.data_len = length; 

	action_notify = ( action_ui_notify* )param; 

	switch( ui_action_id )
	{
	case BITSAFE_LOG_OUTPUT:
		if( ret_val != ERROR_SUCCESS )
		{
			break; 
		}

		notify_type = SYS_LOG_NOTIFY; 
		break; 

	case SYS_MON_ACTION_REQUEST:
		if( ret_val != ERROR_SUCCESS )
		{
			break; 
		}

		notify_type = SYS_EVENT_NOTIFY; 
		break; 
	case SYS_MON_R3_ACTION_REQUEST:
		if( ret_val != ERROR_SUCCESS )
		{
			break; 
		}

		notify_type = SYS_EVENT_R3_NOTIFY; 
		break; 

	case HTTP_URL_FLT_OUTPUT:
		if( ret_val != ERROR_SUCCESS )
		{
			break; 
		}

		notify_type = CUR_URL_RULE_NOTIFY; 
		break; 

	case SYS_MON_BLOCK_COUNT_OUTPUT:
		log_trace( ( MSG_INFO, "output block count return value is 0x%0.8x\n", ret_val ) ); 

		notify_type = BLOCK_COUNT_NOTIFY; 
		break;

	case NET_MON_OUTPUT_PROC_TRAFFIC:
		if( ret_val != ERROR_SUCCESS )
		{
			break; 
		}
		
		log_trace( ( MSG_INFO, "ui output process traffic ui address is 0x%0.8x \n", action_notify ) ); 

		notify_type = PROCESS_NET_DATA_COUNT; 
		break; 
	case BITSAFE_WORK_MODE_OUTPUT:
		if( ret_val < 0 )
		{
			break; 
		}

		notify_type = WORK_MODE_NOTIFY; 
		break; 

	case NET_MON_OUTPUT_TRAFFIC_COUNT:

		if( ret_val < 0 )
		{
			break; 
		}
		
		notify_type = ALL_NET_DATA_COUNT; 
		break; 
	case ANTI_ARP_GET_ARP_INFO:
		notify_type = ARP_INFO_NOTIFY;  
		break; 

	case BITSAFE_SLOGAN_OUTPUT:
		if( ret_val < 0 )
		{
			ASSERT( FALSE && "how can go here when return error" ); 
			break; 
		}
		notify_type = SLOGAN_NOTIFY;  
		break; 

	default:
		ASSERT( FALSE ); 
		break; 
	}

	if( notify_type != UNKNOWN_NOTIFY )
	{
		ret = action_notify->action_notify( notify_type, &data_notify ); 
	}
	else
	{
		ret = ERROR_INVALID_PARAMETER; 
		log_trace( ( MSG_ERROR, "invalid notify type action id is %d\n", ui_action_id ) ); 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) );  
	return ret; 
}

LRESULT ctrl_driver_work_state( bitsafe_function_type type, bitsafe_function_state load_state )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HANDLE dev_handle = NULL; 
	sw_ctrl work_state_sw; 

	ULONG ret_len; 

	switch( type )
	{
	case NET_PACKET_FLT_FUNCTION:
		dev_handle = seven_fw_dev; 
		break; 
	case HTTP_TEXT_FLT_FUNCTION: 
		dev_handle = http_txt_flt_dev; 
		break; 
	case HTTP_URL_FLT_FUNCTION:
		dev_handle = http_url_flt_dev; 
		break; 
	case ANTI_ARP_ATTACK_FUNCTION:
		dev_handle = anti_arp_dev; 
		break; 
	case NET_COMM_MANAGE_FUNCTION:

		dev_handle = net_mon_dev; 
		break; 

	case TRACE_LOG_FUNCTION:
		dev_handle = trace_log_dev; 
		break; 

	case REG_MANAGE_FUNCTION:
		dev_handle = reg_mng_dev; 
		break; 

	case FILE_MANAGE_FUNCTION:
		{
			/***************notice the mini filter have special io interface*****************/

			if( fs_mng_dev == NULL || fs_mng_dev == INVALID_HANDLE_VALUE )
			{
				ret = ERROR_NOT_READY; 
				goto _return; 
			}

			dev_handle = fs_mng_dev; 
			_ret = switch_mini_port_func( fs_mng_dev, DRIVER_WORK_STATE_SW_ID, load_state ); 

			if( _ret != ERROR_SUCCESS )
			{
				DBG_BP(); 
				log_trace( ( MSG_ERROR, "ctrl the driver %uwork state error 0x%0.8\n", _ret ) );
				ret = _ret; 
			}
			goto _return; 
		}
		break; 
	default:
		ASSERT( FALSE && "ctrl invalid driver state" ); 
		break; 
	}

	if( dev_handle != NULL && dev_handle != INVALID_HANDLE_VALUE)
	{
		work_state_sw.id = DRIVER_WORK_STATE_SW_ID; 
		work_state_sw.state = ( ULONG )load_state; 

		//driver_state = load_state; 
		_ret = do_dev_io_ctrl( dev_handle, IOCTL_DRIVER_WORK_STATE, &work_state_sw, sizeof( work_state_sw ), NULL, 0, &ret_len ); 

		if( _ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			log_trace( ( MSG_ERROR, "ctrl the driver %uwork state error 0x%0.8\n", _ret ) );
			ret = _ret; 
		}
	}
	else
	{
		ret = ERROR_NOT_READY; 
		log_trace( ( MSG_ERROR, "control the driver %u which have not loaded\n", type ) ); 
	}

_return:
	return ret; 
}

LRESULT load_driver_work_state( bitsafe_function_type type )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	bitsafe_function_state load_state; 

	if( FALSE == is_valid_driver_type( type ) )
	{
		DBG_BP(); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	switch( type )
	{
	case NET_PACKET_FLT_FUNCTION:
		{
			ASSERT( type == NET_PACKET_FLT_FUNCTION ); 
			_ret = check_bitsafe_function_state( NET_PACKET_FLT_FUNCTION, &load_state ); 
		}

		break; 
	case NET_COMM_MANAGE_FUNCTION:

		{
			ASSERT( type == NET_COMM_MANAGE_FUNCTION ); 
			_ret = check_bitsafe_function_state( NET_COMM_MANAGE_FUNCTION, &load_state ); 
		}
		break; 

	case HTTP_TEXT_FLT_FUNCTION:
	case HTTP_URL_FLT_FUNCTION:
	case ANTI_ARP_ATTACK_FUNCTION:

	case TRACE_LOG_FUNCTION:
	case FILE_MANAGE_FUNCTION:

		_ret = check_bitsafe_function_state( type, &load_state ); 
		break; 
	default:
		ASSERT( FALSE && "invlaid driver type to get the driver state" ); 
		break; 
	}

#ifdef _DEBUG
	if( _ret != ERROR_SUCCESS )
	{
		if( load_state != BITSAFE_FUNCTION_DEFAULT )
		{
			DBG_BP(); 
		}
	}
#endif //_DEBUG

	ctrl_driver_work_state( type, load_state ); 

_return:
	return ret; 
}

LRESULT ctrl_drivers_work_state()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	{
		_ret = load_driver_work_state( NET_PACKET_FLT_FUNCTION ); 
		if( _ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			ret = _ret; 
		}

		_ret = load_driver_work_state( NET_COMM_MANAGE_FUNCTION ); 
		if( _ret != ERROR_SUCCESS )
		{
			DBG_BP(); 
			ret = _ret; 
		}
	}


	_ret = load_driver_work_state( HTTP_TEXT_FLT_FUNCTION ); 
	if( _ret != ERROR_SUCCESS )
	{
		DBG_BP(); 
		ret = _ret; 
	}

	_ret = load_driver_work_state( HTTP_URL_FLT_FUNCTION ); 
	if( _ret != ERROR_SUCCESS )
	{
		DBG_BP(); 
		ret = _ret; 
	}

	_ret = load_driver_work_state( ANTI_ARP_ATTACK_FUNCTION ); 
	if( _ret != ERROR_SUCCESS )
	{
		DBG_BP(); 
		ret = _ret; 
	}


	_ret = load_driver_work_state( TRACE_LOG_FUNCTION ); 
	if( _ret != ERROR_SUCCESS )
	{
		DBG_BP(); 
		ret = _ret; 
	}

	_ret = load_driver_work_state( FILE_MANAGE_FUNCTION ); 
	if( _ret != ERROR_SUCCESS )
	{
		DBG_BP(); 
		ret = _ret; 
	}

	return ret; 
}

FORCEINLINE LRESULT driver_work_enable( bitsafe_function_type type, BOOLEAN enable )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( FALSE == is_valid_driver_type( type ) )
		{
			DBG_BP(); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( enable == FALSE )
		{
			all_bitsafe_function_flags[ type ] |= BITSAFE_FUNCTION_OPENED; 
		}
		else
		{
			all_bitsafe_function_flags[ type ] &= ~( BITSAFE_FUNCTION_OPENED ); 
		}
	}while( FALSE );

	return ret; 
}

FORCEINLINE LRESULT set_driver_work_flags( bitsafe_function_type type, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( FALSE == is_valid_driver_type( type ) )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		all_bitsafe_function_flags[ type ] = flags; 

		switch( flags )
		{
		case BITSAFE_FUNCTION_DISABLED: 
			break; 
		case BITSAFE_FUNCTION_OPENED:
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT ctrl_driver_work_set( bitsafe_function_type type, PVOID ui_context, PVOID work_context, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN can_install = TRUE; 

	if( FALSE == is_valid_bitsafe_function_type( type ) )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( FALSE == is_valid_bitsafe_function_state( flags ) )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	all_bitsafe_function_flags[ type ] = flags; 

	switch( type )
	{
	case NET_PACKET_FLT_FUNCTION:
		{
			switch( flags )
			{
			case BITSAFE_FUNCTION_OPENED:
				ret = load_sevenfw_work_set( TRUE, ui_context, work_context, &can_install ); 
				break; 
			case BITSAFE_FUNCTION_DISABLED: 
				ret = stop_sevenfw_work_set(); 
				break; 
			default:
				ASSERT( FALSE ); 
				break; 
			}
		}
		break; 

	case HTTP_URL_FLT_FUNCTION:
		switch( flags )
		{
		case BITSAFE_FUNCTION_OPENED:
			ret = load_http_flt_work_set( TRUE, ui_context, work_context, &can_install ); 
			break; 
		case BITSAFE_FUNCTION_DISABLED: 
			ret = stop_http_flt_work_set(); 
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
		break; 

	case ANTI_ARP_ATTACK_FUNCTION:
		switch( flags )
		{
		case BITSAFE_FUNCTION_OPENED:
			ret = load_anti_arp_work_set( TRUE, ui_context, work_context, &can_install ); 
			break; 
		case BITSAFE_FUNCTION_DISABLED: 
			ret = stop_anti_arp_work_set(); 
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
		break; 

	case NET_COMM_MANAGE_FUNCTION:
		switch( flags )
		{
		case BITSAFE_FUNCTION_OPENED:
			ret = load_net_mon_work_set( TRUE, ui_context, work_context, &can_install ); 
			break; 
		case BITSAFE_FUNCTION_DISABLED: 		
			ret = stop_net_mon_work_set(); 
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
		break; 

	case TRACE_LOG_FUNCTION:
		switch( flags )
		{

		case BITSAFE_FUNCTION_OPENED:
			ret = load_trace_log_work_set( TRUE, ui_context, work_context, &can_install ); 
			break; 
		case BITSAFE_FUNCTION_DISABLED: 
			ret = stop_trace_log_work_set( TRUE ); 
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
		break; 

	case REG_MANAGE_FUNCTION:
		switch( flags )
		{
		case BITSAFE_FUNCTION_OPENED:
			ret = load_reg_flt_work_set( TRUE, ui_context, work_context, &can_install ); 
			break; 
		case BITSAFE_FUNCTION_DISABLED: 		
			ret = stop_reg_flt_work_set(); 
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
		break; 

	case FILE_MANAGE_FUNCTION:
		switch( flags )
		{
		case BITSAFE_FUNCTION_OPENED:
			ret = load_fs_mng_work_set( TRUE, ui_context, work_context, &can_install ); 
			break; 
		case BITSAFE_FUNCTION_DISABLED: 		
			ret = stop_fs_mng_work_set(); 
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
		break; 
	default:
		ASSERT( FALSE && "ctrl invalid driver state" ); 
		break; 
	}

_return:
	return ret; 
}

LRESULT do_bitsafe_ui_work( ULONG ui_action_id, PBYTE input_buf, ULONG input_length, PBYTE output_buf, ULONG output_length, ULONG *ret_len, PUI_CTRL ui_ctrl, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG _ret_len = 0; 

	log_trace( ( MSG_INFO, "enter %s %ws(0x%0.8x)\n", 
		__FUNCTION__, 
		get_bitsafe_ui_work_desc( ui_action_id ), 
		ui_action_id ) ); 

	if( ret_len != NULL )
	{
		*ret_len = 0; 
	}

	switch( ui_action_id )
	{
	case BITSAFE_LOAD_SERVICE_CONTEXT:
		ret = load_service_context( ( PVOID )ui_ctrl, param ); 
		break; 
	case BITSAFE_LOAD_UI_CONTEXT:
		ret = load_ui_context( ui_ctrl, param ); 
		break; 
	case BITSAFE_UNLOAD_SERVICE_CONTEXT:
		ret = unload_service_context(); 
		break; 
	case BITSAFE_UNLOAD_UI_CONTEXT:
		ret = unload_ui_context(); 
		break; 
	case BITSAFE_INSTALL_ALL_DRIVERS:
		ret = install_main_drivers(); 
		break; 
	case BITSAFE_UNINSTALL_ALL_DRIVERS: 
		ret = _uninstall_main_drivers(); 
		break; 
	case BITSAFE_DELETE_ALL_DRIVERS:
		ret = delete_main_drivers(); 
		break; 
	case BITSAFE_UPDATE_ALL_DRIVERS:
		ret = update_main_drivers(); 
		break; 
	case BITSAFE_SET_RULE: 
		ret = set_action_rule( ( access_rule_desc* )input_buf, input_length ); 
		break; 

	case BITSAFE_UNSET_RULE: 
		ret = unset_action_rule( ( access_rule_desc* )input_buf, input_length ); 
		break; 

	case BITSAFE_MODIFY_RULE:
		ret = _modify_action_rule( ( access_rule_modify_info* )input_buf, input_length ); 		
		break; 

	case BITSAFE_SET_APP_RULE:
		ret = set_app_rule( ( access_rule_desc* )input_buf, input_length ); 
		break; 

	case BITSAFE_UNSET_APP_RULE:
		ret = unset_app_rule( ( access_rule_desc* )input_buf, input_length ); 
		break; 

	case BITSAFE_SET_APP_ACTION_RULE: 
		ret = set_app_action_rule( ( access_rule_desc* )input_buf, input_length ); 		
		break; 

	case BITSAFE_UNSET_APP_ACTION_RULE:
		ret = unset_app_action_rule( ( access_rule_desc* )input_buf, input_length ); 
		break; 

	case BITSAFE_SET_WORK_TIME:
		{
			LARGE_INTEGER *work_time; 

			work_time = ( LARGE_INTEGER* )input_buf; 

			ret = set_work_time( work_time ); 
		}
		break; 

	case HTTP_URL_FLT_DEL_URL:
		ret = del_flt_url( ( PFILTER_URL_INPUT )input_buf, input_length ); 
		break; 

	case HTTP_URL_FLT_ADD_URL:
		ret = add_flt_url( ( PFILTER_URL_INPUT )input_buf, input_length ); 
		break;

#ifdef USE_SECOND_ACL
	case HTTP_URL_FLT_DEL_URLS:
		ret = del_flt_urls( ( PFILTER_URLS_INPUT )input_buf, input_length ); 
		break; 

	case HTTP_URL_FLT_ADD_URLS:
		ret = add_flt_urls( ( PFILTER_URLS_INPUT )input_buf, input_length ); 
		break; 
#endif //USE_SECOND_ACL
	case HTTP_URL_FLT_INSTALL_7FW:
		ret = install_sevenfw(); 
		break; 
	case HTTP_URL_FLT_UNINSTALL_7FW:
		ret = uninstall_sevenfw(); 
		break; 

	case HTTP_URL_FLT_GET_ADDED_FLT_URL:
		ret = get_all_url_rule( ( PVOID )ui_ctrl, param );
		break; 
	case NET_MON_INIT:
		ret = init_net_mon_work_cont( ( PVOID )ui_ctrl, param ); 
		break; 

	case NET_MON_TRACE_PROC_TRAFFIC:
		ret = trace_proc_traffic( ( PFLT_SETTINGS )input_buf, input_length ); 
		break; 

	case NET_MON_SET_PROC_TRAFFIC:
		ret = set_proc_traffic( ( PROCESS_TRAFFIC_CTRL* )input_buf, input_length ); 
		break;

	case NET_MON_UNINIT:
		ret = uninit_net_mon_work_cont(); 
		break; 
	case ANTI_ARP_INIT:
		ret = init_anti_arp( ( PVOID )ui_ctrl, param );  
		break; 
	case ANTI_ARP_UNINIT:
		ret = uninit_anti_arp( ( PVOID )ui_ctrl, param );  
		break; 
	case ANTI_ARP_STOP:
		ret = stop_arp_filter(); 
		break; 
	case ANTI_ARP_START:
		ret = start_arp_filter(); 
		break; 
	case BITSAFE_START_TRACE_INTERACT:
		ret = ctrl_tracelog_interact( TRUE ); 
		break; 
	case BITSAFE_STOP_TRACE_INTERACT:
		ret = ctrl_tracelog_interact( FALSE ); 
		break; 

		
	case BITSAFE_ALLOW_PING:
		ret = ctrl_icmp_ping( FALSE ); 
		break; 
	case BITSAFE_BLOCK_PING:
		ret = ctrl_icmp_ping( TRUE ); 
		break; 
	case BITSAFE_RELEARN:
		ret = relearn_rule(); 
		break; 

#ifdef HAVE_KRNL_HOOK
	case SYS_MON_START:
		ret = set_sys_mon_flt( ( PHOOK_FILTER_INFO )input_buf, input_length ); 
		break; 
	case SYS_MON_STOP:
		ret = stop_sys_manage(); 
		break; 
	case SYS_MON_SET_FLT:
		ret = set_sys_mon_flt( ( PHOOK_FILTER_INFO )input_buf, input_length );
		break; 
#endif //HAVE_KRNL_HOOK

	case SYS_MON_EVENT_RESPONSE:
		ret = response_sys_event( ( event_action_response* )input_buf, input_length ); 
		break; 

#ifdef HAVE_KRNL_HOOK
	case SYS_MON_UNINIT:
		ret = uninit_sys_mon_work_cont(); 
		break; 
	case SYS_MON_UNINSTALL:
		ret = uninstall_sys_mon_work_cont(); 
#endif //HAVE_KRNL_HOOK

	case HTTP_TXT_FLT_DEL_URL:
		ret = del_flt_txt( ( PFILTER_NAME_INPUT )input_buf, input_length ); 
		break; 
	case HTTP_TXT_FLT_ADD_URL:
		ret = add_flt_txt( ( PFILTER_NAME_INPUT )input_buf, input_length );
		break; 

#ifdef HAVE_KRNL_HOOK
	case BITSAFE_SET_MATCH_NAME_BP: 
		ASSERT( input_buf != NULL ); 
		ASSERT( input_length > sizeof( WCHAR ) * 2 ); 

		ret = set_bp_match_name( ( LPCWSTR )input_buf, input_length );
		break; 
#endif //HAVE_KRNL_HOOK

	case HTTP_TXT_FLT_UNINSTALL:
		ret = uninstall_http_txt_filter_driver(); 
		break; 
	case HTTP_TXT_FLT_GET_ADDED_FLT_TXT:
		ret = get_all_seted_filter_txts( ( PVOID )ui_ctrl, param );
		break; 
	case BITSAFE_INSTALL_DRIVER:
		if( input_length < sizeof( ULONG ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = ctrl_bitsafe_driver( ( bitsafe_function_type )*( ULONG* )input_buf, INSTALL_DRIVER ); 
		break; 
	case BITSAFE_UNINSTALL_DRIVER:
		if( input_length < sizeof( ULONG ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = ctrl_bitsafe_driver( ( bitsafe_function_type )*( ULONG* )input_buf, UNINSTALL_DRIVER ); 
		break;  
	case BITSAFE_LOAD_DRIVER:
		if( input_length < sizeof( ULONG ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = ctrl_driver_work_set( ( bitsafe_function_type )*( ULONG* )input_buf, param, ( PVOID )ui_ctrl, BITSAFE_FUNCTION_OPENED ); 
		//ret = ctrl_bitsafe_driver( ( driver_type )*( ULONG* )input_buf, LOAD_DRIVER ); 
		break; 
	case BITSAFE_UNLOAD_DRIVER:
		if( input_length < sizeof( ULONG ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		ret = ctrl_driver_work_set( ( bitsafe_function_type )*( ULONG* )input_buf, param, ( PVOID )ui_ctrl, BITSAFE_FUNCTION_DISABLED ); 
		//ret = ctrl_bitsafe_driver( ( driver_type )*( ULONG* )input_buf, UNLOAD_DRIVER ); 
		break; 
	case BITSAFE_SET_DRIVER_WORK_MODE: 
		{
			sw_ctrl *drv_state_ctrl; 

			if( input_length < sizeof( sw_ctrl ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			drv_state_ctrl = ( sw_ctrl* )input_buf; 

			ret = ctrl_driver_work_state( ( bitsafe_function_type )drv_state_ctrl->id, ( bitsafe_function_state )drv_state_ctrl->state ); 
		}
		break; 
	case BITSAFE_ENTER_DBG_MODE:
		{
			ret = stop_all_function_driver_load(); 
		}
		break; 
	case BITSAFE_ENTER_BACK_PROC_DBG_MODE:
		{
			ret = stop_all_function_driver_load(); 
		}
		break; 
	default:
		ASSERT( FALSE && "unknown action id" ); 
		break; 
	}

	if( ret_len != NULL )
	{
		*ret_len = _ret_len; 
	}

	return ret; 
}