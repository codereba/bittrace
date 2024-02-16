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
