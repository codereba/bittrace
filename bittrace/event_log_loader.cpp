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