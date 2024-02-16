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

#ifndef __COMMON_CONFIG_DB_SQL_H__
#define __COMMON_CONFIG_DB_SQL_H__

//define the actions of the all type of an action in one common table.
//order all action log by time,then get all them using select sentence.
//analyze all output actions, display the analyze result by grams.
//use common table for all action type will be simplest programming.

#define _MAX_FILTER_COND_VALUE_LEN _T( "1024" )

//DEFAULT (0)
#define COMMON_CONFIG_DB_NAMEA "common_config"
#define COMMON_CONFIG_DB_NAMEA _T( "common_config" )

#define SEARCH_HISTORY_TABLE_NAME _T( "search_history" )

#define SEARCH_HISTORY_TABLE_DEFINE _T( "id INTEGER PRIMARY KEY AUTOINCREMENT, " ) \
	_T( "search_text NVARCHAR(" ) _MAX_FILTER_COND_VALUE_LEN _T( ") COLLATE NOCASE" ) 

#define SQL_CREATE_SEARCH_HISTORY_TABLE _T( "CREATE TABLE " ) SEARCH_HISTORY_TABLE_NAME _T( "( " ) SEARCH_HISTORY_TABLE_DEFINE _T( ")" )

#define SQL_INSERT_SEARCH_HISTORY _T( "INSERT INTO " ) \
	SEARCH_HISTORY_TABLE_NAME \
	_T( " ( search_text )" ) \
	_T( "VALUES ('%s')" ) 

#define SQL_UPDATE_SEARCH_HISTORY _T( "UPDATE " ) \
	SEARCH_HISTORY_TABLE_NAME \
	_T( " SET search_text='%s'where search_text='%s' " ) 

#define SQL_DELETE_SEARCH_HISTORY _T( "DELETE FROM " ) \
	SEARCH_HISTORY_TABLE_NAME \
	_T( " WHERE seach_text='%s'" ) 

#define SQL_SELECT_SEARCH_HISTORY _T( "SELECT " ) \
	_T( "id, seach_text FROM " ) \
	ACTION_FILTER_COND_TABLE_NAME

#define SQL_DELETE_ALL_SEARCH_HISTORY _T( "DELETE FROM " ) \
	SEARCH_HISTORY_TABLE_NAME 

#define SEARCH_HISTORY_ID_COLUME_INDEX 0
#define SEARCH_HISTORY_SEACH_TEXT_COLUME_INDEX 1

#endif //__COMMON_CONFIG_DB_SQL_H__