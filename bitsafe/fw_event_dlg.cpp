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
 
 #include "StdAfx.h"
#include "common_func.h"
#include "trace_log_api.h"
#include "bitsafe_common.h"
#include "fw_event_dlg.h"

vector< fw_log > fw_logs; 

LRESULT CALLBACK alloc_fw_log( PVOID *log, PVOID param )

{
	ASSERT( log != NULL ); 
	
	*log = NULL; 
	return 0; 
}

INT32 CALLBACK load_fw_log( fw_log *log, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	fw_log *_log; 
	vector< fw_log > *logs; 

	ASSERT( log != NULL ); 
	ASSERT( param != NULL ); 

	_log = ( fw_log* )log; 

	logs = ( vector< fw_log >* )param; 

	logs->push_back( *_log ); 


	return ret; 
}
