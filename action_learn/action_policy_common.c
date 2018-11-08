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
 
 #include "common_func.h"
#include "action_policy_common.h"

action_check_dispatch_set action_check_dispatchs; 

NTSTATUS init_action_check_dispatchs()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 i; 

	do 
	{
		action_check_dispatchs.count = 0; 

		for( i = 0; i < ARRAY_SIZE( action_check_dispatchs.dispatchs ); i ++ )
		{
			action_check_dispatchs.dispatchs[ i ] = NULL; 
		}

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS register_action_check_dispatch( action_check_dispatch *dispatch )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		if( dispatch == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		ntstatus = is_valid_mem_region( dispatch, sizeof( *dispatch ) ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		if( action_check_dispatchs.count >= ARRAY_SIZE( action_check_dispatchs.dispatchs ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER_3; 
			break; 
		}

		action_check_dispatchs.dispatchs[ action_check_dispatchs.count ] = dispatch; 
		action_check_dispatchs.count ++; 

	}while( FALSE );

	return ntstatus; 
}


NTSTATUS unregister_action_check_dispatch( action_check_dispatch *dispatch )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 i; 

	do 
	{
		if( dispatch == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		ntstatus = is_valid_mem_region( dispatch, sizeof( *dispatch ) ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		//this function only can called when initialize error.
		for( i = action_check_dispatchs.count - 1; i >= 0; i -- )
		{
			if( action_check_dispatchs.dispatchs[ i ] == dispatch )
			{
				if( i < action_check_dispatchs.count - 1 )
				{
					memmove( &action_check_dispatchs.dispatchs[ i ], 
						&action_check_dispatchs.dispatchs[ i + 1 ], 
						( action_check_dispatchs.count - 1 - i ) ); 
				}
				else
				{
					action_check_dispatchs.dispatchs[ i ] = NULL; 
				}

				action_check_dispatchs.count --; 
			}
		}

	}while( FALSE );

	return ntstatus; 
}
