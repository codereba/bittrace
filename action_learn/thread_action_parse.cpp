#include "common_func.h"
#include "action_type.h"
#include "thread_action_parse.h"

NTSTATUS parse_thread_remote_param( thrd_remote *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_REMOTE_PARAM_COUNT 9
		*count_out = PROC_THREAD_REMOTE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_REMOTE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->access );  
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->suspended ); 
		set_param_info( params[ 4 ], PTR_TYPE, ptr_val, action->start_vaddr ); 
		set_param_info( params[ 5 ], PTR_TYPE, ptr_val, action->thread_param ); 
		set_param_info( params[ 6 ], UINT32_TYPE, uint32_val, action->result );  
		set_param_info( params[ 7 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 8 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_thread_set_context_param( thrd_setctxt *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_SET_CONTEXT_PARAM_COUNT 5
		*count_out = PROC_THREAD_SET_CONTEXT_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_SET_CONTEXT_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result );  
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name );

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_thread_suspend_param( thrd_suspend *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_SUSPEND_PARAM_COUNT 5
		*count_out = PROC_THREAD_SUSPEND_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_SUSPEND_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result );  
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name );

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_thread_kill_param( thrd_kill *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_KILL_PARAM_COUNT 6
		*count_out = PROC_THREAD_KILL_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_KILL_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->exitcode );  
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->path_name_len );
		set_param_info( params[ 5 ], WSTRING_TYPE, wstring_val, action->path_name );

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_thread_queue_apc_param( thrd_queue_apc *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_QUEUE_APC_PARAM_COUNT 5
		*count_out = PROC_THREAD_QUEUE_APC_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_QUEUE_APC_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_name_len );
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name );

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_open_phys_mm_param( sys_open_physmm *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_OPEN_PHYSICAL_MM_PARAM_COUNT 2
		*count_out = SYS_OPEN_PHYSICAL_MM_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_OPEN_PHYSICAL_MM_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->access ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_read_phys_mm_param( sys_read_physmm *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_READ_PHYSICAL_MM_PARAM_COUNT 3
		*count_out = SYS_READ_PHYSICAL_MM_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_READ_PHYSICAL_MM_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->offset ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->count ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_load_kmod_param( sys_load_kmod *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_LOAD_KMOD_PARAM_COUNT 3
		*count_out = SYS_LOAD_KMOD_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_LOAD_KMOD_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_reg_srv_param( sys_regsrv *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_REG_SRV_PARAM_COUNT 8
		*count_out = SYS_REG_SRV_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_REG_SRV_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->access ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->type ); 
		set_param_info( params[ 2 ], PTR_TYPE, uint32_val, action->starttype ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->srv_name_len ); 
		set_param_info( params[ 6 ], WSTRING_TYPE, wstring_val, action->path_name ); 
		set_param_info( params[ 7 ], WSTRING_TYPE, wstring_val, action->path_name 
			+ action->path_name_len ); 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_open_dev_param( sys_opendev *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_OPEN_DEV_PARAM_COUNT 6
		*count_out = SYS_OPEN_DEV_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_OPEN_DEV_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->devtype ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->access ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->share ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 5 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_w32_msg_hook( w32_msghook *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_W32_MSG_HOOK_PARAM_COUNT 7
		*count_out = SYS_W32_MSG_HOOK_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_W32_MSG_HOOK_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], PTR_TYPE, ptr_val, action->modbase ); 
		set_param_info( params[ 2 ], PTR_TYPE, ptr_val, action->hookfunc ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->hooktype ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->lib_path_len ); 
		set_param_info( params[ 6 ], WSTRING_TYPE, wstring_val, action->lib_path ); 


	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_w32_lib_inject_hook( w32_lib_inject *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_W32_LIB_INJECT_PARAM_COUNT 5
		*count_out = SYS_W32_LIB_INJECT_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_W32_LIB_INJECT_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name 
			+ action->path_name_len ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_net_connect( net_connect *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define NET_CONNECT_PARAM_COUNT 4
		*count_out = NET_CONNECT_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < NET_CONNECT_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->protocal ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->ip_addr ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->port ); 

	}while( FALSE ); 

	return ntstatus; 
}

#define parse_net_create parse_net_connect
#define parse_net_listen parse_net_connect

NTSTATUS parse_net_send( net_send *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define NET_SEND_PARAM_COUNT 5
		*count_out = NET_SEND_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < NET_SEND_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->protocal ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->datalen ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->ip_addr ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->port ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_net_http( net_http *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define NET_HTTP_PARAM_COUNT 6
		*count_out = NET_HTTP_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < NET_HTTP_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->protocal ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->cmd ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->datalen ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 5 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_extract_hidden( ba_extract_hidden *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_EXTRACT_HIDDEN_PARAM_COUNT 2
		*count_out = BA_EXTRACT_HIDDEN_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_EXTRACT_HIDDEN_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_extract_pe( ba_extract_pe *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_EXTRACT_PE_PARAM_COUNT 2
		*count_out = BA_EXTRACT_PE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_EXTRACT_PE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_self_copy( ba_self_copy *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_SELF_COPY_PARAM_COUNT 2
		*count_out = BA_SELF_COPY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_SELF_COPY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_self_delete( ba_self_delete *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_SELF_DELETE_PARAM_COUNT 2
		*count_out = BA_SELF_DELETE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_SELF_DELETE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_invade_process( ba_invade_process *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_INVADE_PROCESS_PARAM_COUNT 3
		*count_out = BA_INVADE_PROCESS_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_INVADE_PROCESS_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_infect_pe( ba_infect_pe *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_INFRECT_PE_PARAM_COUNT 3
		*count_out = BA_INFRECT_PE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_INFRECT_PE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_overwrite_pe( ba_overwrite_pe *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_OVERWRITE_PE_PARAM_COUNT 3
		*count_out = BA_OVERWRITE_PE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_OVERWRITE_PE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_register_autorun( ba_register_autorun *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_OVERWRITE_PE_PARAM_COUNT 3
		*count_out = BA_OVERWRITE_PE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_OVERWRITE_PE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->type ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_name_len ); 
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

