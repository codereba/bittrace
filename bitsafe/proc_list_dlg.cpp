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
 
 #include "StdAfx.h"
#include "common_func.h"
#include "seven_fw_api.h"
#include "ui_config.h"
#include "proc_list_dlg.h"
#include "seven_fw_ioctl.h"

ULONG CALLBACK thread_list_proc( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	proc_list_dlg *dlg; 
	DWORD wait_ret;
	//ULONG ret_len; 
	thread_manage *thread_param; 
	

	ASSERT( NULL != param );

	thread_param = ( thread_manage* )param; 

	ASSERT( thread_param->param != NULL ); 

	dlg = ( proc_list_dlg* )thread_param->param; 

	for( ; ; )
	{
		log_trace( ( MSG_INFO, "%s get process information loop\n", __FUNCTION__ ) ); 

		lock_cs( dlg->proc_info_lock ); 

		ret = get_all_proc_list( dlg->all_proc_infos, dlg->max_proc_count, &dlg->proc_count ); 

		unlock_cs( dlg->proc_info_lock ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _continue; 
		}

		dlg->on_proc_info_got(); 

_continue:
		wait_ret = wait_event( thread_param->notify, 1500 ); 
		if( wait_ret != WAIT_TIMEOUT 
			&& wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait syncronous event failed will exit\n" ) ); 
			break; 
		}
	}

_return:

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return ( ULONG )ret; 
}