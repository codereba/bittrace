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
