#ifndef __UI_CONTROL_H__
#define __UI_CONTROL_H__

#ifndef BITSAFE_FUNCTION_TYPE_DEFINED
#define BITSAFE_FUNCTION_TYPE_DEFINED

#pragma pack(push)
#pragma pack(1)

typedef struct _thread_manage
{
	INT32 stop_run; 
	HANDLE thread_handle;
	DWORD id;
	HANDLE notify; 
	BOOLEAN self_notify; 
	PVOID param;
	PVOID context; 
} thread_manage, *pthread_manage; 

typedef struct _all_block_count
{
	ULONGLONG fw_block_ocunt; 
	ULONGLONG defense_block_count; 
} all_block_count, *pall_block_count; 


typedef struct _notify_data
{
	PVOID data; 
	ULONG data_len;
} notify_data, *pnotify_data; 


#define MAX_HOOK_INFO_SIZE ( sizeof( HOOK_FILTER_INFO ) + sizeof( ULONG ) * NtApiFilterEnd )
#define MAX_TRACE_INFO_LEN ( ULONG )( 1024 * 10 )
typedef struct __FILTER_INDEX_TABLE
{
	ULONG *FilterIndexes;
	ULONG IndexNumber;
} FILTER_INDEX_TABLE, *PFILTER_INDEX_TABLE;
#pragma pack(pop)
typedef enum _bitsafe_function_type
{
	NET_PACKET_FLT_FUNCTION, 
	HTTP_TEXT_FLT_FUNCTION, 
	HTTP_URL_FLT_FUNCTION, 
	ANTI_ARP_ATTACK_FUNCTION, 
	NET_COMM_MANAGE_FUNCTION, 
	//SYS_MANAGE_FUNCTION, 
	TRACE_LOG_FUNCTION, 
	REG_MANAGE_FUNCTION,
	FILE_MANAGE_FUNCTION, 
	//EVENTS_MON_DRIVER, 
	FUNCTION_TYPE_BEGIN = NET_PACKET_FLT_FUNCTION, 
	FUNCTION_TYPE_END = FILE_MANAGE_FUNCTION, //FS_MNG_DRIVER, 
	MAX_FUNCTION_TYPE
} bitsafe_function_type; 

#endif //BITSAFE_FUNCTION_TYPE_DEFINED

typedef enum _driver_os_type
{
	UNKOWN_DRIVER, 
	WINXP_DRIVER, 
	VISTA_DRIVER, 
	ALL_COMP_DRIVER
} driver_os_type, *pdriver_os_type; 

typedef enum _bitsafe_function_state
{
	BITSAFE_FUNCTION_OPENED, 
	BITSAFE_FUNCTION_DISABLED,
	BITSAFE_FUNCTION_DEFAULT = BITSAFE_FUNCTION_OPENED, 
} bitsafe_function_state; 

#define is_valid_bitsafe_function_state( _function_state_ ) ( _function_state_ >= BITSAFE_FUNCTION_OPENED && _function_state_ <= BITSAFE_FUNCTION_DISABLED )

#define is_valid_bitsafe_function_type( _function_type_ ) ( _function_type_ >= NET_PACKET_FLT_FUNCTION && _function_type_ <= FUNCTION_TYPE_END )

//typedef struct _drv_open_state
//{
//	driver_type id; 
//	drv_state state; 	
//} drv_open_state, *pdrv_open_state; 

#define BITSAFE_UI_ID 0x000a0001
//#define ANTI_ARP_UI_ID 0x00000001
#define ANTI_ARP_GET_ARP_INFO 0x00000001
#define ANTI_ARP_INIT 0x00000002
#define ANTI_ARP_UNINIT 0x00000005
#define ANTI_ARP_START 0x00000003
#define ANTI_ARP_STOP 0x00000004

//#define NET_MON_UI_ID 0x00010001
#define NET_MON_INIT 0x00010001
#define NET_MON_TRACE_PROC_TRAFFIC 0x00010002
#define NET_MON_OUTPUT_PROC_TRAFFIC 0x00010003
#define NET_MON_SET_PROC_TRAFFIC 0x00010006
#define NET_MON_OUTPUT_TRAFFIC_COUNT 0x00010004
#define NET_MON_UNINIT 0x00010005

//#define SYS_MON_UI_ID 0x00000002
#define SYS_MON_INIT 0x00020001
#define SYS_MON_START 0x00020002
#define SYS_MON_STOP 0x00020003
#define SYS_MON_UNINIT 0x00020004
#define BITSAFE_LOG_OUTPUT 0x00020005
#define SYS_MON_SET_FLT 0x00020006
#define SYS_MON_EVENT_RESPONSE 0x00020007
#define SYS_MON_ACTION_REQUEST 0x00020008
#define SYS_MON_UNINSTALL 0x00020009
#define SYS_MON_BLOCK_COUNT_OUTPUT 0x0002000a
#define SYS_MON_R3_ACTION_REQUEST 0x0002000b

//#define HTTP_URL_FLT_UI_ID 0x00000003
#define HTTP_URL_FLT_DEL_URL 0x00030001
#define HTTP_URL_FLT_ADD_URL 0x00030002
//#define HTTP_URL_FLT_INSTALL 0x00030003
//#define HTTP_URL_FLT_UNINSTALL 0x00030004
//#define HTTP_URL_FLT_INIT 0x00030005
//#define HTTP_URL_FLT_UNINIT 0x00030006
#define HTTP_URL_FLT_OUTPUT 0x00030007
#define HTTP_URL_FLT_GET_ADDED_FLT_URL 0x00030008
#define HTTP_URL_FLT_INSTALL_7FW 0x00030009
#define HTTP_URL_FLT_UNINSTALL_7FW 0x0003000a
#define HTTP_URL_FLT_ADD_URLS 0x0003000c
#define HTTP_URL_FLT_DEL_URLS 0x0003000b

//#define HTTP_TXT_FLT_UI_ID 0x00000004
#define HTTP_TXT_FLT_DEL_URL 0x00040001
#define HTTP_TXT_FLT_ADD_URL 0x00040002
#define HTTP_TXT_FLT_INSTALL 0x00040003
#define HTTP_TXT_FLT_UNINSTALL 0x00040004
#define HTTP_TXT_FLT_INIT 0x00040005
#define HTTP_TXT_FLT_UNINIT 0x00040006
#define HTTP_TXT_FLT_OUTPUT 0x00040007
#define HTTP_TXT_FLT_GET_ADDED_FLT_TXT 0x00040008

#define BITSAFE_UNINSTALL_ALL_DRIVERS 0x00050001
#define BITSAFE_INSTALL_ALL_DRIVERS 0x00050002

#define BITSAFE_SET_RULE 0x00060003
#define BITSAFE_UNSET_RULE 0x00060004
#define BITSAFE_MODIFY_RULE 0x00060005

#define BITSAFE_WORK_MODE_OUTPUT 0x00060001
#define BITSAFE_SET_WORK_MODE 0x00060002
#define BITSAFE_GET_WORK_MODE 0x00060006
#define BITSAFE_SET_MATCH_NAME_BP 0x00060007

#define BITSAFE_LOAD_SERVICE_CONTEXT 0x00060009
#define BITSAFE_LOAD_UI_CONTEXT 0x0006000a
#define BITSAFE_UNLOAD_SERVICE_CONTEXT 0x0006000b
#define BITSAFE_UNLOAD_UI_CONTEXT 0x0006000c
#define BITSAFE_DELETE_ALL_DRIVERS 0x0006000d
#define BITSAFE_SET_APP_RULE 0x0006000e
#define BITSAFE_UNSET_APP_RULE 0x0006000f
#define BITSAFE_SET_WORK_TIME 0x00060010
#define BITSAFE_UPDATE_ALL_DRIVERS 0x00060011
#define BITSAFE_START_TRACE_INTERACT 0x00060012
#define BITSAFE_STOP_TRACE_INTERACT 0x00060013
#define BITSAFE_BLOCK_PING 0x00060014
#define BITSAFE_ALLOW_PING 0x00060015
#define BITSAFE_RELEARN 0x00060016
#define BITSAFE_SET_APP_ACTION_RULE 0x00060017
#define BITSAFE_UNSET_APP_ACTION_RULE 0x00060018

#define BITSAFE_INSTALL_DRIVER 0x00060019
#define BITSAFE_UNINSTALL_DRIVER 0x0006001a
#define BITSAFE_LOAD_DRIVER 0x0006001b
#define BITSAFE_UNLOAD_DRIVER 0x0006001c

#define BITSAFE_SET_LOG_MODE 0x0006001d
#define BITSAFE_GET_LOG_MODE 0x0006001e

#define BITSAFE_SLOGAN_OUTPUT 0x0006001f

#define BITSAFE_SET_DRIVER_WORK_MODE 0x00060022
#define BITSAFE_ENTER_DBG_MODE 0x00060023
#define BITSAFE_ENTER_BACK_PROC_DBG_MODE 0x00060024


//typedef struct _sw_ctrl
//{
//	ULONG id; 
//	ULONG state; 
//} sw_ctrl, *psw_ctrl; 

extern HANDLE trace_log_dev; 

#define is_valid_driver_type( type ) ( type >= FUNCTION_TYPE_BEGIN && type <= FUNCTION_TYPE_END )

INLINE LPCWSTR get_driver_type_desc( bitsafe_function_type type )
{
	LPCWSTR type_desc = L"UNKONW_FUNCTION_TYPE"; 

	switch( type )
	{
	case NET_PACKET_FLT_FUNCTION: 
		type_desc = L"NET_PACKET_FLT_FUNCTION"; 
		break; 
	case HTTP_TEXT_FLT_FUNCTION: 
		type_desc = L"HTTP_TEXT_FLT_FUNCTION"; 
		break; 
	case HTTP_URL_FLT_FUNCTION: 
		type_desc = L"HTTP_URL_FLT_FUNCTION"; 
		break; 
	case ANTI_ARP_ATTACK_FUNCTION: 
		type_desc = L"ANTI_ARP_ATTACK_FUNCTION"; 
		break; 
	//case SYS_MON_DRIVER: 
	//	type_desc = L"SYS_MON_DRIVER"; 
	//	break; 
	case TRACE_LOG_FUNCTION: 
		type_desc = L"TRACE_LOG_FUNCTION"; 
		break; 
	case NET_COMM_MANAGE_FUNCTION:
		type_desc = L"NET_COMM_MANAGE_FUNCTION"; 
		break; 

//#ifndef DELAY_WIN7_SUPPORT
//	case NET_FW_DRIVER:
//		type_desc = L"NET_FW_DRIVER"; 
//		break; 
//#endif //DELAY_WIN7_SUPPORT	
	
	//case SEVEN_FW_WIN7_DRIVER:
	//	type_desc = L"SEVEN_FW_WIN7_DRIVER"; 
	//	break; 
	}

	return type_desc; 
}

typedef enum _driver_action
{
	INSTALL_DRIVER, 
	LOAD_DRIVER_DEVICE, 
	LOAD_DRIVER_MINI_PORT, 
	UNLOAD_DRIVER, 
	UNINSTALL_DRIVER, 
	DELETE_DRIVER, 
	OUTPUT_DRIVER, 
	DRIVER_OPERATION_BEGIN = INSTALL_DRIVER, 
	DRIVER_OPERATION_END = OUTPUT_DRIVER
} driver_action; 

#define is_valid_driver_operation( operation ) ( operation >= DRIVER_OPERATION_BEGIN && operation <= DRIVER_OPERATION_END ) 

INLINE LPCWSTR get_operation_type_desc( driver_action type )
{
	LPCWSTR type_desc = L"UNKONW_OPRATION_TYPE"; ; 

	switch( type )
	{
	case INSTALL_DRIVER: 
		type_desc = L"INSTALL_DRIVER"; 
		break; 
	case LOAD_DRIVER_DEVICE: 
		type_desc = L"LOAD_DRIVER"; 
		break;
	case UNLOAD_DRIVER: 
		type_desc = L"UNLOAD_DRIVER"; 
		break; 
	case UNINSTALL_DRIVER: 
		type_desc = L"UNINSTALL_DRIVER"; 
		break; 
	case DELETE_DRIVER: 
		type_desc = L"DELETE_DRIVER"; 
		break; 
	}

	return type_desc; 
}

typedef enum _action_ui_notify_type
{
	BLOCK_COUNT_NOTIFY, 
	CUR_URL_RULE_NOTIFY, 
	PROCESS_NET_DATA_COUNT, 
	ALL_NET_DATA_COUNT, 
	WORK_MODE_NOTIFY, 
	UI_LANG_NOTIFY, 
	SYS_LOG_NOTIFY, 
	SYS_EVENT_NOTIFY, 
	SYS_EVENT_R3_NOTIFY, 
	ARP_INFO_NOTIFY, 
	ALL_DRIVER_LOADED, 
	ERC_CMD_EXECUTED, 
	SLOGAN_NOTIFY, 
	REQUEST_UI_INTERACT_NOTIFY, 
	MAX_NOTIFY_TYPE, 
	UNKNOWN_NOTIFY = -1
} action_ui_notify_type; 

class action_ui_notify
{
public:
	virtual LRESULT action_notify( action_ui_notify_type type, PVOID param ) = 0; 
};

struct UI_CTRL; 
typedef LRESULT ( *do_ui_work )( ULONG ui_action_id, 
							  PBYTE input_buf, 
							  ULONG input_length, 
							  PBYTE output_buf, 
							  ULONG output_len, 
							  ULONG *ret_len, 
							  struct UI_CTRL *ui_ctrl, 
							  PVOID param ); 

typedef LRESULT ( *do_ui_output )( ULONG ui_action_id, 
								INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param ); 

typedef struct UI_CTRL
{
	LIST_ENTRY ui_list; 
	ULONG ui_id; 
	do_ui_work ui_work_func; 
	do_ui_output ui_output_func; 
} UI_CTRL, *PUI_CTRL; 

LRESULT init_ui_ctrls(); 
LRESULT add_ui_ctrl( ULONG ui_id, do_ui_work ui_work_func, do_ui_output ui_output_func ); 
LRESULT delete_ui_ctrl( ULONG ui_id ); 
LRESULT release_ui_ctrls(); 
LRESULT do_ui_ctrl( ULONG ui_id, ULONG ui_action_id, PBYTE input_buf, ULONG input_length, PBYTE output_buf, ULONG output_length, PULONG ret_len, PVOID param );  

LRESULT config_proc_trace_data_size( ULONG proc_id, ULONG data_size ); 

INLINE LRESULT do_safe_dev_io_ctl( HANDLE dev, ULONG ioctl_code, PVOID input_buf, ULONG input_length, PVOID output_buf, ULONG output_length, ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	ASSERT( ret_len != NULL ); 

	if( output_buf == NULL 
		|| output_length == 0)
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	memset( output_buf, 0, output_length ); 
	_ret = DeviceIoControl( dev, 
		ioctl_code, 
		NULL, 
		0, 
		output_buf, 
		output_length, 
		ret_len, 
		NULL ); 

	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	return ret; 
}

INLINE LPCWSTR get_bitsafe_ui_work_desc( ULONG ui_action_id )
{
	LPCWSTR desc = L"UNKOWN_UI_WORK"; 
	switch( ui_action_id )
	{
	case HTTP_URL_FLT_DEL_URL:
		desc = L"HTTP_URL_FLT_DEL_URL"; 
		break; 

	case HTTP_URL_FLT_ADD_URL:
		desc = L"HTTP_URL_FLT_ADD_URL"; 
		break; 
	case HTTP_URL_FLT_ADD_URLS:
		desc = L"HTTP_URL_FLT_ADD_URLS"; 
		break; 
	
	case HTTP_URL_FLT_DEL_URLS:
		desc = L"HTTP_URL_FLT_DEL_URLS"; 
		break; 

	//case HTTP_URL_FLT_INSTALL:
	//	desc = L"HTTP_URL_FLT_INSTALL";  
	//	break;

	//case HTTP_URL_FLT_UNINSTALL:
	//	desc = L"HTTP_URL_FLT_UNINSTALL"; 
		//break; 
	//case HTTP_URL_FLT_INIT:
	//	desc = L"HTTP_URL_FLT_INIT";  
	//	break; 
	//case HTTP_URL_FLT_UNINIT:
	//	desc = L"HTTP_URL_FLT_UNINIT";  
	//	break; 
	case HTTP_URL_FLT_INSTALL_7FW:
		desc = L"HTTP_URL_FLT_INSTALL_7FW";  
		break; 
	case HTTP_URL_FLT_UNINSTALL_7FW:
		desc = L"HTTP_URL_FLT_UNINSTALL_7FW"; 
		break; 

	case HTTP_URL_FLT_GET_ADDED_FLT_URL:
		desc = L"HTTP_URL_FLT_GET_ADDED_FLT_URL"; 
		break; 
	case NET_MON_INIT:
		desc = L"NET_MON_INIT"; 
		break; 

	case NET_MON_TRACE_PROC_TRAFFIC:
		desc = L"NET_MON_TRACE_PROC_TRAFFIC"; 
		break; 

	case NET_MON_SET_PROC_TRAFFIC:
		desc = L"NET_MON_SET_PROC_TRAFFIC"; 
		break;

	case NET_MON_UNINIT:
		desc = L"NET_MON_UNINIT"; 
		break; 
	case ANTI_ARP_INIT:
		desc = L"ANTI_ARP_INIT"; 
		break; 
	case ANTI_ARP_STOP:
		desc = L"ANTI_ARP_STOP"; 
		break; 
	case ANTI_ARP_START:
		desc = L"ANTI_ARP_START"; 
		break; 
	case SYS_MON_INIT:
		desc = L"SYS_MON_INIT"; 
		break;  
	case SYS_MON_START:
		desc = L"SYS_MON_START"; 
		break; 
	case SYS_MON_STOP:
		desc = L"SYS_MON_STOP"; 
		break; 
	case SYS_MON_SET_FLT:
		desc = L"SYS_MON_SET_FLT"; 
		break; 
	case SYS_MON_EVENT_RESPONSE:
		desc = L"SYS_MON_EVENT_RESPONSE"; 
		break; 
	case SYS_MON_UNINIT:
		desc = L"SYS_MON_UNINIT"; 
		break; 
	case SYS_MON_UNINSTALL:
		desc = L"SYS_MON_UNINSTALL"; 
	case HTTP_TXT_FLT_DEL_URL:
		desc = L"HTTP_TXT_FLT_DEL_URL"; 
		break; 
	case HTTP_TXT_FLT_ADD_URL:
		desc = L"HTTP_TXT_FLT_ADD_URL"; 
		break; 
	case BITSAFE_SET_RULE: 
		desc = L"BITSAFE_SET_RULE"; 
		break; 

	case BITSAFE_UNSET_RULE:
		desc = L"BITSAFE_UNSET_RULE"; 
		break; 

	case BITSAFE_SET_APP_RULE:
		desc = L"BITSAFE_SET_APP_RULE"; 
		break; 

	case BITSAFE_UNSET_APP_RULE:
		desc = L"BITSAFE_UNSET_APP_RULE"; 
		break; 

	case BITSAFE_SET_APP_ACTION_RULE:
		desc = L"BITSAFE_SET_APP_ACTION_RULE"; 
		break; 

	case BITSAFE_UNSET_APP_ACTION_RULE:
		desc = L"BITSAFE_UNSET_APP_ACTION_RULE"; 
		break; 

	case BITSAFE_SET_WORK_TIME:
		desc = L"BITSAFE_SET_WORK_TIME"; 
		break; 

	case BITSAFE_START_TRACE_INTERACT:
		desc = L"BITSAFE_START_TRACE_INTERACT";
		break; 

	case BITSAFE_STOP_TRACE_INTERACT:
		desc = L"BITSAFE_STOP_TRACE_INTERACT"; 
		break; 

	case BITSAFE_BLOCK_PING: 
		desc = L"BITSAFE_BLOCK_PING"; 
		break; 
	case BITSAFE_ALLOW_PING:
		desc = L"BITSAFE_ALLOW_PING"; 
		break; 
	case BITSAFE_RELEARN:
		desc = L"BITSAFE_RELEARN";  
		break; 
	case HTTP_TXT_FLT_INSTALL:
		desc = L"HTTP_TXT_FLT_INSTALL"; 
		break;
	case HTTP_TXT_FLT_UNINSTALL:
		desc = L"HTTP_TXT_FLT_UNINSTALL"; 
		break; 
	case HTTP_TXT_FLT_INIT:
		desc = L"HTTP_TXT_FLT_INIT"; 
		break; 
	case HTTP_TXT_FLT_UNINIT:
		desc = L"HTTP_TXT_FLT_UNINIT"; 
		break; 
	case HTTP_TXT_FLT_GET_ADDED_FLT_TXT:
		desc = L"HTTP_TXT_FLT_GET_ADDED_FLT_TXT"; 
		break; 
	case BITSAFE_SET_WORK_MODE:
		desc = L"BITSAFE_SET_WORK_MODE"; 
		break; 
	case BITSAFE_GET_WORK_MODE:
		desc = L"BITSAFE_GET_WORK_MODE"; 			
		break; 
	default:
		break; 
	}

	return desc; 
}

INLINE LPCWSTR get_output_work_desc( ULONG ui_action_id )
{
	LPCWSTR desc = L"UNKNOWN_OUTOUT_WORK"; 

	switch( ui_action_id )
	{
	case BITSAFE_LOG_OUTPUT:
		desc = L"BITSAFE_LOG_OUTPUT"; 
		break; 
	case SYS_MON_ACTION_REQUEST:
		desc = L"SYS_MON_ACTION_REQUEST"; 
		break; 
	case SYS_MON_R3_ACTION_REQUEST:
		desc = L"SYS_MON_R3_ACTION_REQUEST"; 
		break; 
	case HTTP_URL_FLT_OUTPUT:
		desc = L"HTTP_URL_FLT_OUTPUT"; 
		break; 
	case SYS_MON_BLOCK_COUNT_OUTPUT:
		desc = L"SYS_MON_BLOCK_COUNT_OUTPUT"; 
		break;

		//case NET_MON_CTRL_PROC_TRAFFIC:
		//	break; 
	case NET_MON_OUTPUT_PROC_TRAFFIC:
		desc = L"NET_MON_OUTPUT_PROC_TRAFFIC"; 
		break; 
	case NET_MON_OUTPUT_TRAFFIC_COUNT:
		desc = L"NET_MON_OUTPUT_TRAFFIC_COUNT"; 
		break; 
	case ANTI_ARP_INIT:
		desc = L"ANTI_ARP_INIT"; 
		break; 
	case ANTI_ARP_GET_ARP_INFO:
		desc = L"ANTI_ARP_GET_ARP_INFO"; 
		break; 
	default:
		break; 
	}

	return desc; 
}

LRESULT do_bitsafe_ui_work( ULONG ui_action_id, PBYTE input_buf, ULONG input_length, PBYTE output_buf, ULONG output_len, ULONG *ret_len, PUI_CTRL ui_ctrl, PVOID param ); 

LRESULT sevefw_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
						 PBYTE data, 
						 ULONG length, 
						 PVOID param ); 

LRESULT _create_work_thread( thread_manage *work_thread, 
							PTHREAD_START_ROUTINE thread_routine, 
							HANDLE notifier, 
							PVOID context, 
							PVOID param ); 

FORCEINLINE LRESULT create_work_thread( thread_manage *work_thread, 
									   PTHREAD_START_ROUTINE thread_routine, 
									   PVOID context, 
									   PVOID param )
{
	return _create_work_thread( work_thread, thread_routine, NULL, context, param ); 
}

LRESULT stop_work_thread( thread_manage *work_thread ); 


//typedef struct _CHARRANGE
//{
//	LONG	cpMin;
//	LONG	cpMax;
//} CHARRANGE;

#define EM_EXSETSEL				(WM_USER + 55)

#endif //__UI_CONTROL_H__