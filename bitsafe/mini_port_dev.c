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

#include "common_func.h"
#include "wdk_macros.h"
#include "fs_mng_api.h"
#include "common_api.h"
#include "mini_port_dev.h"

#include <fltUser.h>

HRESULT switch_mini_port_func( HANDLE mini_port, ULONG sw_id, BYTE state )
{
	HRESULT ret = S_OK; 
	DWORD ret_len = 0;
	ULONG result; 
	ULONG cmd_len; 
	FS_CMD *cmd_buf = NULL; 
	sw_ctrl *_ctrl; 

	ASSERT( mini_port != NULL ); 

#pragma warning(push)
#pragma warning(disable:4127) 

	do 
	{
#pragma warning(pop)

		cmd_len = sizeof( FS_CMD ) + sizeof( sw_ctrl ); 

		cmd_buf = ( FS_CMD* )malloc( cmd_len ); 
		if( cmd_buf == NULL )
		{
			ret = S_FALSE; 
			break; 
		}

		//
		//  add file name to white list.
		//

		cmd_buf->cmd = SwitchFsMngFunc; 
		_ctrl = ( sw_ctrl* )cmd_buf->data; 
			
		_ctrl->id = sw_id; 
		_ctrl->state = state; 

		ret = FilterSendMessage( mini_port,
			( FS_CMD* )cmd_buf,
			cmd_len,
			&result,
			sizeof( result ),
			&ret_len );

		if( IS_ERROR( ret ) ) 
		{

			if( HRESULT_FROM_WIN32( ERROR_INVALID_HANDLE ) == ret )
			{

				log_trace( ( MSG_INFO, "The kernel component of task manage has unloaded. Exiting\n" ) );
				ret = S_FALSE; 
				break; 
			} 
			else 
			{
				log_trace( ( MSG_INFO, "UNEXPECTED ERROR received: %x\n", ret ) ); 
			}
		}
	} while ( FALSE );

	if( cmd_buf != NULL )
	{
		free( cmd_buf ); 
	}

	return ret; 
}