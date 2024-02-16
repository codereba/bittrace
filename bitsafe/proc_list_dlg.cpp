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