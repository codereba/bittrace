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

#ifndef __ACTION_FILTER_COND_SQL_H__
#define __ACTION_FILTER_COND_SQL_H__

//define the actions of the all type of an action in one common table.
//order all action log by time,then get all them using select sentence.
//analyze all output actions, display the analyze result by grams.
//use common table for all action type will be simplest programming.

#define _MAX_FILTER_COND_VALUE_LEN _T( "1024" )

//����UNIQUEԼ������֤����������Ψһ��
//DEFAULT (0)
//id INTEGER PRIMARY KEY AUTOINCREMENT, 
#define ACTION_FILTER_COND_TABLE_NAMEA "action_filter_cond_table"
#define ACTION_FILTER_COND_TABLE_NAME _T( "action_filter_cond_table" )
#define ACTION_FILTER_COND_TABLE_DEFINE _T( "cond INTEGER, mode INTEGER, value_type INTEGER, " ) \
	_T( "value NVARCHAR(" ) _MAX_FILTER_COND_VALUE_LEN _T( ") COLLATE NOCASE" ) \
	_T( ", result INTEGER, disabled INTEGER, bk_color INTEGER, time TIMESTAMP DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime') ), " ) \
	_T( "PRIMARY KEY (cond, mode, value_type, value, result ) ON CONFLICT REPLACE" )

#define SQL_CREATE_ACTION_FILTER_COND_TABLE _T( "CREATE TABLE " ) ACTION_FILTER_COND_TABLE_NAME _T( "( " ) ACTION_FILTER_COND_TABLE_DEFINE _T( ")" )

#define SQL_INSERT_ACTION_COND _T( "INSERT INTO " ) \
	ACTION_FILTER_COND_TABLE_NAME \
	_T( " (cond, mode, value_type, value, result, bk_color, disabled)" ) \
	_T( "VALUES (%u, %u, %u, '%s', %u, %u, 0)" ) 

#define SQL_UPDATE_ACTION_COND _T( "UPDATE " ) \
	ACTION_FILTER_COND_TABLE_NAME \
	_T( " SET cond=%u, mode=%u, value_type=%u, value='%s', result=%u, bk_color=%u, disabled=0 where cond=%u " \
	_T( "and mode=%u and value_type=%u and value='%s' " \
	_T( "and result=%u and bk_color=%u " ) ) ) 

#define SQL_DELETE_ACTION_COND _T( "DELETE FROM " ) \
	ACTION_FILTER_COND_TABLE_NAME \
	_T( " WHERE cond=%u AND mode=%u AND value_type=%u AND value='%s' AND result=%u" ) 

#define SQL_SELECT_ACTION_COND _T( "SELECT " ) \
	_T( "cond, mode, value, result, bk_color, time FROM " ) \
	ACTION_FILTER_COND_TABLE_NAME

#define SQL_DELETE_ALL_ACTION_COND _T( "DELETE FROM " ) \
	ACTION_FILTER_COND_TABLE_NAME 

#define ACTION_FILTER_COND_COLUME_INDEX 0
#define ACTION_FILTER_MODE_COLUME_INDEX 1
#define ACTION_FILTER_VALUE_COLUME_INDEX 2
#define ACTION_FILTER_RESULT_COLUME_INDEX 3
#define ACTION_FILTER_BACKGROUD_COLOR_COLUME_INDEX 4
#define ACTION_FILTER_COND_CREATE_TIME_INDEX 5

#define ORG_ACTION_FILTER_COND_COLUME_INDEX 0
#define ORG_ACTION_FILTER_MODE_COLUME_INDEX 1
#define ORG_ACTION_FILTER_VALUE_TYPE_COLUME_INDEX 2
#define ORG_ACTION_FILTER_VALUE_COLUME_INDEX 3
#define ORG_ACTION_FILTER_RESULT_COLUME_INDEX 4
#define ORG_ACTION_FILTER_DISABLED_COLUME_INDEX 5
#define ORG_ACTION_FILTER_BACKGROUD_COLOR_COLUME_INDEX 6
#define ORG_ACTION_FILTER_COND_CREATE_TIME_INDEX 7

#endif //__ACTION_FILTER_COND_SQL_H__