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