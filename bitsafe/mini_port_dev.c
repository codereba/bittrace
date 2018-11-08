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