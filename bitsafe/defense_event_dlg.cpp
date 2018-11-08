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
#include "bitsafe_common.h"
#include "defense_event_dlg.h"

vector< defense_log > defense_logs; 

LRESULT CALLBACK alloc_defense_log( PVOID *log, PVOID param )

{
	ASSERT( log != NULL ); 

	*log = NULL; 
	return 0; 
}

INT32 CALLBACK load_defense_log( defense_log *log, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	defense_log *_log; 
	vector< defense_log > *logs; 

	ASSERT( log != NULL ); 
	ASSERT( param != NULL ); 

	_log = ( defense_log* )log; 

	logs = ( vector< defense_log >* )param; 

	logs->push_back( *_log ); 

	return ret; 
}
