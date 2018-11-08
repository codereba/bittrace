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