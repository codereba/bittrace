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
