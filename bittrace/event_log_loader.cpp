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

#include "stdafx.h"
#include "common_func.h"
#include "ui_ctrl.h"
#include "acl_define.h"
#include "action_type.h"
#include "cached_list_aio.h"
#include "event_log_loader.h"

LRESULT WINAPI load_action_log_work( ULONG begin_index, ULONG count, PVOID context)
{
	LRESULT ret = ERROR_SUCCESS; 
	cache_work_help *help; 

	do 
	{}while(FALSE);

	return ret; 
}

LRESULT WINAPI load_action_log( ULONG begin_index, ULONG count, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}