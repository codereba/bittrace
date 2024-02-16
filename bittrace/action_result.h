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

#ifndef __ACTION_RESULT_H__
#define __ACTION_RESULT_H__

typedef struct _action_result_waiter
{
	ULONG action_id; 
	NTSTATUS result; 
} action_result_waiter, *paction_result_waiter; 

LPCWSTR WINAPI get_result_desc( NTSTATUS ntstatus ); 
LRESULT WINAPI process_event( r3_action_notify *action ); 


LRESULT WINAPI init_event_result_receiver(); 
LRESULT WINAPI uninit_event_result_receiver(); 

#endif //__ACTION_RESULT_H__