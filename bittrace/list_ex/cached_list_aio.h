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

#ifndef __CACHED_LIST_AIO_H__
#define __CACHED_LIST_AIO_H__

typedef struct _work_item
{
	LIST_ENTRY entry; 
	LONG begin_index; 
	LONG ring_begin_index; 
	LONG count; 
	LONG flags; 
	LONG max_count; 
	LONG loaded_count; 
} work_item, *pwork_item; 


typedef struct _cache_work_help
{
	HWND notify_wnd; 
	CListBodyExUI *list; 
	work_item *work; 
	BOOLEAN break_work; 
} cache_work_help, *pcache_work_help; 

typedef struct _cache_work_context
{
	thread_manage worker; 
	CRITICAL_SECTION work_lock; 
	cache_work_help context; 
	//vector< >
	LIST_ENTRY all_work_item; 

}cache_work_context, *pcache_work_context; 


#endif //__CACHED_LIST_AIO_H__