#ifndef __INST_DRIVER_H__ 
#define __INST_DRIVER_H__

#define DRIVER_FUNC_INSTALL 0
#define DRIVER_FUNC_REMOVE 1

#define DRIVER_IN_SYSTEM_DIR 0x01
#define DRIVER_BOOT_START 0x02
#define DRIVER_AUTO_START 0x04
#define DO_NOT_AUTO_UNINSTALL_DRIVER 0x08 
#define DRIVER_UNINSTALL_DO_NOT_STOP 0x10  
#define DRIVER_INSTALL_DO_NOT_START 0x40
#define DRIVER_MANUAL_START 0x20 

#ifdef __cplusplus
extern "C" { 
#endif 

LRESULT install_drv(
					IN SC_HANDLE  SchSCManager,
					IN LPCTSTR    driver_name,
					IN LPCTSTR    service_exe, 
					IN ULONG flags
					);

LRESULT remove_drv(
				   IN SC_HANDLE  SchSCManager,
				   IN LPCTSTR    driver_name
				   );

LRESULT start_drv(
				  IN SC_HANDLE  SchSCManager,
				  IN LPCTSTR    driver_name
				  );

LRESULT stop_driver(
					IN SC_HANDLE  SchSCManager,
					IN LPCTSTR    driver_name
					);

LRESULT manage_drv(
				   IN LPCTSTR  driver_name,
				   IN LPCTSTR  file_name,
				   IN USHORT   Function, 
				   IN ULONG flags
				   );

INLINE LRESULT _start_drv(
				   IN LPCTSTR  service_name 
				   )
{

	LRESULT ret = ERROR_SUCCESS;
	SC_HANDLE   schSCManager;

	//
	// Insure (somewhat) that the driver and service names are valid.
	//

	ASSERT( service_name != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	//
	// Connect to the Service Control Manager and open the Services database.
	//

	schSCManager = OpenSCManager( NULL,                   // local machine
		NULL,                   // local database
		SC_MANAGER_ALL_ACCESS   // access required
		); 

	if( schSCManager == NULL ) 
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "Open SC Manager failed! \n" ) ); 
		goto _return; 
	}

	//
	// Do the requested function.
	//

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = start_drv( schSCManager, service_name ); 

_return:
	if( schSCManager = NULL ) 
	{
		CloseServiceHandle( schSCManager ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x \n", __FUNCTION__, ret ) ); 
	return ret;
}   // manage_drv

LRESULT delete_drv_file( LPCTSTR pszSysFile, INT32 is_sys_path ); 

#define MAX_SETUP_CMD_LEN ( MAX_PATH + 100 )
#define DEF_INSTALL_SECTION_NAME L"DefaultInstall"
#define DEF_UNINSTALL_SECTION_NAME L"DefaultUninstall"

LRESULT setup_drv_from_inf( LPCWSTR file_name, LPCWSTR section_name ); 

typedef struct _FS_MINI_FILTER_CONFIG_PARAM
{
	LPCWSTR altitude; 
	ULONG flags; 
} FS_MINI_FILTER_CONFIG_PARAM, *PFS_MINI_FILTER_CONFIG_PARAM; 

INT32 WINAPI config_fs_mini_filter_driver( LPCWSTR service_name, 
										  FS_MINI_FILTER_CONFIG_PARAM *config_param ); 

typedef struct _MINI_FILTER_PARAM
{
	LPCWSTR altitude; 
	ULONG start_type; 
} MINI_FILTER_PARAM, *PMINI_FILTER_PARAM; 

#define SET_MINI_FILTER_PARAM 0x00000001
#define CREATE_DRIVER_SERVICE 0x00000002
LRESULT install_mini_filter_driver_ex( LPCWSTR driver_sys_path, 
									  LPCWSTR driver_name, 
									  LPCWSTR service_name, 
									  MINI_FILTER_PARAM *update_param, 
									  ULONG flags ); 
#ifdef __cplusplus
}
#endif 


#endif