/*
 *
 * Copyright 2010 JiJie Shi
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
 
 #ifndef __SERVICE_H__
#define __SERVICE_H__

#define UI_PROCESS 0x00000001
#define MAX_CMD_LINE_LEN 512
#define SERVICE_NAME "secu_mon"

//extern CHAR service_name[ _MAX_PATH ]; 
extern SERVICE_TABLE_ENTRYA service_dispatch_table[]; 

typedef struct _proc_work_param
{
	CHAR cmd_line[ _MAX_PATH ]; 
	CHAR work_dir[ _MAX_PATH ]; 
	ULONG flags; 
	ULONG start_delay; 
	ULONG exist_delay; 
	PROCESS_INFORMATION proc_info; 
} proc_work_param, *pproc_work_param; 

VOID WINAPI service_main( DWORD argc, LPSTR *argv);
VOID WINAPI service_handler( DWORD ctrl_code ); 
LRESULT init_proc_work_params( ULONG size ); 
LRESULT set_proc_work_param( ULONG index, LPCSTR cmd_line, LPCSTR work_dir, ULONG flags, ULONG start_delay ); 
LRESULT uninit_proc_work_params(); 
LRESULT stop_proc_manage(); 
LRESULT init_proc_manage_context( ULONG size ); 
LRESULT uninit_proc_manage_context(); 
BOOL is_inited_work_param( proc_work_param *work_param ); 
BOOL proc_started( proc_work_param *work_param ); 
LRESULT start_process( ULONG index ); 
LRESULT end_process( ULONG index ); 
LRESULT ctrl_process( LPCSTR name, ULONG index ); 
LRESULT stop_service( LPCSTR name ); 
LRESULT start_service( LPCSTR name, INT32 argv, LPCSTR *argc ); 
#define SERVICE_SPECIFIC_ERROR_CODE ( ULONG )( -1 )
VOID WINAPI service_main( DWORD argc, LPTSTR *argv ); 
VOID WINAPI service_handler( DWORD ctrl_code ); 
LRESULT uninstall_service( CHAR* name ); 
LRESULT install_service( LPCSTR path, LPCSTR name ); 
LRESULT start_proc_manage(); 
LRESULT check_service_exist( LPCSTR name ); 
BOOL dump_service_info( LPCTSTR szSvcName ); 
LRESULT auto_update_service_binary_path( LPCSTR service_name, LPCSTR binary_path ); 

#endif //__SERVICE_H__