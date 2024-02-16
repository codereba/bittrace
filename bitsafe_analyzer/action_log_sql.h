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

#ifndef __ACTION_LOG_SQL_H__
#define __ACTION_LOG_SQL_H__

//define the actions of the all type of an action in one common table.
//order all action log by time,then get all them using select sentence.
//analyze all output actions, display the analyze result by grams.
//use common table for all action type will be simplest programming.

#define MAX_PROC_NAME_LEN _T( "260" )
#define MAX_THREAD_ID_LEN _T( "64" )
#define MAX_PROC_ID_LEN _T( "64" )
#define MAX_PARAM1_LEN _T( "" )

#define COMMON_ACTION_TABLE_NAMEA "common_action_table"
#define COMMON_ACTION_TABLE_NAME _T( "common_action_table" ) 

#define COLUME_PATH_NAME _T( "path" ) 
#define COLUME_COMMAND_NAME _T( "command" )
#define COLUME_PID_NAME _T( "pid" ) 
#define COLUME_TID_NAME _T( "tid" ) 

#define COLUME_PARENT_PID_NAME _T( "parent_pid" ) 
#define COLUME_ID_NAME _T( "id" ) 
#define COLUME_TYPE_NAME _T( "type" )
#define COLUME_PID_NAME _T( "pid" ) 
#define COLUME_TIME_NAME _T( "time" ) 
#define COLUME_RESULT_NAME _T( "result" ) 
#define COLUME_DETAIL_NAME _T( "detail" ) 
#define COLUME_DESC_NAME _T( "desc" ) 
#define COLUME_SESSION_NAME _T( "session" ) 
#define COLUME_USER_NAME _T( "user" ) 
#define COLUME_CORP_NAME _T( "corp" ) 
#define COLUME_VERSION_NAME _T( "version" ) 
#define COLUME_VIRT_NAME _T( "virt" ) 
#define COLUME_ARCH_NAME _T( "arch" ) 

#define COMMON_ACTION_TABLE_DEFINE _T( "id INTEGER PRIMARY KEY, type INTEGER, proc_name NVARCHAR(" ) \
	MAX_PROC_NAME_LEN _T( ") COLLATE NOCASE, proc_id NVARCHAR(" ) \
	MAX_PROC_ID_LEN _T( "), thread_id NVARCHAR(" ) \
	MAX_THREAD_ID_LEN _T( "), param0 TEXT " ) \
	_T( ", param1 TEXT " ) \
	_T( ", param2 TEXT " ) \
	_T( ", param3 TEXT " ) \
	_T( ", param4 TEXT " ) \
	_T( ", param5 TEXT " ) \
	_T( ", param6 TEXT " ) \
	_T( ", param7 TEXT " ) \
	_T( ", stack TEXT " ) \
	_T( ", data BLOB " ) \
	_T( ", data_len INTEGER DEFAULT (0)" ) \
	_T( ", result INTEGER DEFAULT(0)" ) \
	_T( ", time TIMESTAMP DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime') ) " )

#define COMMON_ACTION_TABLE_ID_INDEX_NAME L"id_index"
#define COMMON_ACTION_TABLE_TIME_INDEX_NAME L"time_index"
#define COMMON_ACTION_TABLE_COMMON_INDEX_NAME L"common_index"

#define SQL_CREATE_COMMON_ACTION_TABLE _T( "CREATE TABLE " ) COMMON_ACTION_TABLE_NAME _T( "( " ) COMMON_ACTION_TABLE_DEFINE _T( ")" )

#define SQL_TEST_INSERT_ACTION_LOG _T( "INSERT INTO " ) \
	COMMON_ACTION_TABLE_NAME \
	_T( " ( proc_name, proc_id, thread_id, param1, param2, param3, param4, param5, param6, param7, data, data_len ) " ) \
	_T( "VALUES ( '%s', %u, %u, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', ?, %u ) " ) 

#define ACTION_LOG_ID_INDEX 0
#define ACTION_TYPE_COLUME_INDEX 1
#define PROC_NAME_COLUME_INDEX 2
#define PROC_ID_COLUME_INDEX 3
#define THREAD_ID_COLUME_INDEX 4
#define ACTION_PARAM_FIELD_INDEX 5

#define ACTION_PARAM_COUNT 8

#define ACTION_STACK_PARAM_FIELD_INDEX ( ACTION_PARAM_COUNT + ACTION_PARAM_FIELD_INDEX )

#define ACTION_DATA_PARAM_FIELD_INDEX ( ACTION_PARAM_COUNT + ACTION_PARAM_FIELD_INDEX + 1 )

#define ACTION_DATA_SIZE_PARAM_FIELD_INDEX ( ACTION_PARAM_COUNT + ACTION_PARAM_FIELD_INDEX + 2 )

#define ACTION_RESULT_PARAM_FIELD_INDEX ( ACTION_PARAM_COUNT + ACTION_PARAM_FIELD_INDEX + 3 )

#define ACTION_TIME_COLUME_INDEX ( ACTION_PARAM_COUNT + ACTION_PARAM_FIELD_INDEX + 4 )

#endif //__ACTION_LOG_SQL_H__