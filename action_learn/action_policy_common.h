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
 
 #ifndef __ACTION_POLICY_COMMON_H__
#define __ACTION_POLICY_COMMON_H__

#include "acl_define.h"

//this second parameter will be a output union for multiple policy type.
typedef NTSTATUS ( CALLBACK* CHECK_ACTION_FUCTION )( r3_action_notify *action, action_reply *resp ); 

#define MAX_ACTION_CHECK_DISPATCH 6

#pragma pack( 4 )
typedef struct _action_check_dispatch
{
	CHECK_ACTION_FUCTION action_check_functions[ SYS_ACTION_END ]; 
} action_check_dispatch, *paction_check_dispatch; 
#pragma pack()

typedef struct _action_check_dispatch_set
{
	ULONG count; 
	action_check_dispatch *dispatchs[ MAX_ACTION_CHECK_DISPATCH ]; 
} action_check_dispatch_set, *paction_check_dispatch_set; 

extern action_check_dispatch_set action_check_dispatchs; 
NTSTATUS register_action_check_dispatch( action_check_dispatch *action_dispatch ); 
NTSTATUS unregister_action_check_dispatch( action_check_dispatch *dispatch ); 


#endif //__ACTION_POLICY_COMMON_H__
