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

#include "StdAfx.h"
#include "common_func.h"
#include "action_type.h"
#include "trace_log_api.h"
#include "bitsafe_common.h"
#include "list_ex_common.h"
#include "UIListSubElementEx.h"
#include "list_log.h"
#include "win_impl_base.hpp"
#include "trace_frame.h"
#include "event_memory_store.h"


LRESULT WINAPI trace_frame::add_action_log( ULONG event_count, ULONG filtered_event_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	CListLogUI *log_list; 

	do 
	{
		log_list = static_cast<CListLogUI*>( m_pManager->FindControl( _T( "action_log_list" ) ) );

		CListBodyExUI *ListEx = log_list->GetListEx(); 
		
		if( ListEx != NULL )
		{
			//ListEx->GetRealItemsCount() + log_count
			log_list->_SetLogCount( ( ULONG )get_store_filtered_events_count(), 
				( ULONG )get_store_event_count(), event_count, DEFAULT_TRACE_NOTIFY_TIME ); 

			ListEx->OnItemAdded( filtered_event_count ); 
		}
		else
		{
			ASSERT( FALSE ); 
		}

	}while( FALSE );

	return ret; 
}

LRESULT CALLBACK alloc_sys_action_log( PVOID *log, PVOID param )

{
	ASSERT( log != NULL ); 
	
	*log = NULL; 
	return 0; 
}

INT32 CALLBACK load_sys_action_log( sys_action_log *log, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	sys_action_log *_log; 
	vector< sys_action_log > *logs; 

	ASSERT( log != NULL ); 
	ASSERT( param != NULL ); 

	_log = ( sys_action_log* )log; 

	logs = ( vector< sys_action_log >* )param; 

	logs->push_back( *_log ); 


	return ( INT32 )ret; 
}


