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
 
#ifndef __SQL_STMT_DEF_H__
#define __SQL_STMT_DEF_H__

#define SQL_CHECK_TABLE_EXIST _T( "select count(*) from sqlite_master where type='table' and name='%s'" )

#define PROC_NAME_LEN _T( "1024" )
#define MAX_FILE_PATH_LEN _T( "260" )
#define CLASS_NAME_LEN _T( "260" )
#define DESC_LEN _T( "512" )
#define FW_LOG_TABLE _T( "FW_LOG" )
#define DEFENSE_LOG_TABLE _T( "DEFENSE_LOG" )

#define FW_URL_DEFINE_TABLE _T( "FW_URL_DEFINE" )
#define FW_IP_DEFINE_TABLE _T( "FW_IP_DEFINE" )
#define FW_PORT_DEFINE_TABLE _T( "FW_PORT_DEFINE" )
#define FW_ICMP_ACTION_DEFINE_TABLE _T( "FW_ICMP_ACTION_DEFINE" )

#define COMMON_DEFINE_TABLE _T( "COMMON_DEFINE" )

#define DEFENSE_REG_PROTECT_TABLE _T( "DEFENSE_PROTECTED_REG" )
#define DEFENSE_FILE_PROTECT_TABLE _T( "DEFENSE_PROTECTED_FILE" )
#define DEFENSE_COM_PROTECT_TABLE _T( "DEFENSE_PROTECTED_COM" )

#define FW_RULE_TABLE _T( "FW_RULES" )
#define DEFENSE_RULE_TABLE _T( "DEFENSE_RULES" )

#define APP_DEFINE_TABLE _T( "APP_DEFINES" )

#define DRIVER_LOAD_CONF_TABLE _T( "DRIVER_LOAD_CONF" )
#define GENERAL_CONF_TABLE _T( "GENERAL_CONF" )

#define MAX_URL_LEN _T( "1024" )
#define MAX_REG_PATH_LEN _T( "2048" )

#define CONF_NAME_LEN _T( "256" )

#define CONF_VAL_NAME_LEN _T( "64" )
#define _CONF_VAL_NAME_LEN 64
#define CONF_VAL_DATA_LEN _T( "256" )
#define _CONF_VAL_DATA_LEN 256

/***************************************************************
notice:
use index enhance performance.
use limit and offset to support information paging.
use transaction support atmic operation.
***************************************************************/
//#define SELECT_ROWS_BY_OFFSET "select * from table limit 100 offset 1"
//#define CREATE_ROW_INDEX "create index on ( ) table"

//PRAGMA encoding;
//PRAGMA encoding = "UTF-8";
//PRAGMA encoding = "UTF-16";
//PRAGMA encoding = "UTF-16le";
//PRAGMA encoding = "UTF-16be";

//CREATE TABLE [TBL_TANK_ALARM] ( 
//  [ALARM_ID] NUMBER(10), 
//  [TANK_NO] VARCHAR2(50) NOT NULL CONSTRAINT [ALARM_TANKNO] REFERENCES [TBL_TANK]([TANK_NO]) MATCH SIMPLE, 
//  [ALARM] VARCHAR2(255), 
//  [UPLOAD_TIME] DATETIME DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime')), 
//  [IS_PROCESS] CHAR(1) DEFAULT ('0'), 
//  [PROCESS_USER] VARCHAR2(20), 
//  [PROCESS_TIME] DATE, 
//  [PROCESS_REMARK] VARCHAR2(255), 
//  [ALARM_LEVEL] VARCHAR2(20), 
//  [ALARM_VALUE] NUMBER(10,4)); 

#define SQL_CREATE_FW_URL_DEFINE_TABLE _T( "CREATE TABLE " ) FW_URL_DEFINE_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, url NVARCHAR(" ) MAX_URL_LEN _T( "), class NVARCHAR(" ) CLASS_NAME_LEN _T( ") DEFAULT (''), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ), flag INTEGER DEFAULT 0, UNIQUE ( url ) )" ) 

#define SQL_CREATE_FW_IP_DEFINE_TABLE _T( "CREATE TABLE " ) FW_IP_DEFINE_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, ip_begin LARGEINT, ip_end LARGEINT DEFAUT ( 0 ), class NVARCHAR(" ) CLASS_NAME_LEN _T( ") DEFAULT (''), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ),flag INTEGER DEFAULT 0, UNIQUE ( ip_begin, ip_end ) )" ) 

#define SQL_CREATE_FW_PORT_DEFINE_TABLE _T( "CREATE TABLE " ) FW_PORT_DEFINE_TABLE _T( " ( id INTEGER  PRIMARY KEY AUTOINCREMENT, port_begin INTEGER, port_end INTEGER DEFAULT ( 0 ), class NVARCHAR(" ) CLASS_NAME_LEN _T( ") DEFAULT(''), type INTEGER DEFAULT ( 0 ), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ), flag INTEGER DEFAULT( 0 ), UNIQUE ( port_begin, port_end, type ) )" ) 

#define SQL_CREATE_APP_DEFINE_TABLE _T( "CREATE TABLE " ) APP_DEFINE_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, name NVARCHAR(" ) PROC_NAME_LEN  _T( " ), class NVARCHAR(" ) CLASS_NAME_LEN _T( ") DEFAULT (''), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ), flag INTEGER DEFAULT (0), UNIQUE( name ) )" ) 

#define SQL_CREATE_COMMON_DEFINE_TABLE _T( "CREATE TABLE " ) COMMON_DEFINE_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, name NVARCHAR(" ) MAX_FILE_PATH_LEN  _T( " ), class NVARCHAR(" ) CLASS_NAME_LEN _T( ") DEFAULT (''), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ), flag INTEGER DEFAULT (0), UNIQUE( name ) )" ) 

#define SQL_CREATE_FW_RULE_TABLE _T( "CREATE TABLE " ) FW_RULE_TABLE _T( " ( type INTEGER DEFAULT( 0 ), class_name NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT (''), id INTEGER DEFAULT ( 0 ), class_name2 NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT ('' ), id2 INTEGER DEFAULT ( 0 ), class_name3 NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT ('' ), id3 INTEGER DEFAULT ( 0 ), class_name4 NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT ('' ), id4 INTEGER DEFAULT ( 0 ), class_name5 NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT (''), id5 INTEGER DEFAULT ( -1 ), action INTEGER DEFAULT( 0 ), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ), flag INTEGER DEFAULT (0 ), PRIMARY KEY ( class_name, id, class_name2, id2, class_name3, id3, class_name4, id4, class_name5, id5, type ) )" ) 

#define SQL_CREATE_PROTECT_REG_TABLE _T( "CREATE TABLE " ) DEFENSE_REG_PROTECT_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, reg_path NVARCHAR(" ) MAX_REG_PATH_LEN _T( ") UNIQUE, class_name NVARCHAR(" ) CLASS_NAME_LEN _T( ") DEFAULT ( '' ), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ), flag INTEGER DEFAULT ( 0 ) )" ) 

#define SQL_CREATE_PROTECT_FILE_TABLE _T( "CREATE TABLE " ) DEFENSE_FILE_PROTECT_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, file_path NVARCHAR(" ) MAX_FILE_PATH_LEN _T( ") UNIQUE, class_name NVARCHAR(" ) CLASS_NAME_LEN _T( ") DEFAULT ( '' ), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ), flag INTEGER DEFAULT( 0 ) )" ) 

#define SQL_CREATE_PROTECT_COM_TABLE _T( "CREATE TABLE " ) DEFENSE_COM_PROTECT_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, com_name NVARCHAR(" ) MAX_FILE_PATH_LEN _T( ") UNIQUE, class_name NVARCHAR(" ) CLASS_NAME_LEN _T( ") DEFAULT( '' ), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ), flag INTEGER DEFAULT ( 0 ) )" ) 

#define SQL_CREATE_DEFENSE_RULE_TABLE _T( "CREATE TABLE " ) DEFENSE_RULE_TABLE _T( " ( type INTEGER DEFAULT ( 0 ), class_name NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT (''), id INTEGER DEFAULT ( -1 ), class_name2 NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT (''), id2 INTEGER DEFAULT ( -1 ), class_name3 NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT (''), id3 INTEGER DEFAULT ( -1 ), class_name4 NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT (''), id4 INTEGER DEFAULT ( -1 ), class_name5 NVARCHAR(" ) CONF_NAME_LEN _T( ") DEFAULT (''), id5 INTEGER DEFAULT ( -1 ), action INTEGER DEFAULT ( 0 ), desc NVARCHAR (" )  DESC_LEN _T( " ) DEFAULT ('' ), flag INTEGER DEFAULT (0), PRIMARY KEY ( class_name, id, class_name2, id2, class_name3, id3, class_name4, id4, class_name5, id5, type, flag ) )" ) 

#define SQL_CREATE_FW_LOG_TABLE _T( "CREATE TABLE " ) FW_LOG_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, proc_name NVARCHAR(" ) PROC_NAME_LEN _T( "), action TINYINT, type TINYINT DEFFAULT (0 ), prot TINYINT, src_ip INTEGER,  src_port SMALLINT, dest_ip INTEGER, dest_port SMALLINT, desc NVARCHAR( " ) DESC_LEN _T( "), time TIMESTAMP DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime') ) )" ) 

#define SQL_CREATE_DEFNESE_LOG_TABLE _T( "CREATE TABLE " ) DEFENSE_LOG_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, proc_info NVARCHAR(") PROC_NAME_LEN _T( "), flag TINYINT, action TINYINT, target_info NVARCHAR(" ) PROC_NAME_LEN _T( "), time TIMESTAMP DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime') ) )" )


#define SQL_CREATE_DRIVER_LOAD_CONF_TABLE _T( "CREATE TABLE " ) DRIVER_LOAD_CONF_TABLE _T( " ( drv_id INTEGER PRIMARY KEY, drv_state TINYINT )" )
#define SQL_CREATE_GENERAL_CONF_TABLE _T( "CREATE TABLE " ) GENERAL_CONF_TABLE _T( " ( id INTEGER PRIMARY KEY AUTOINCREMENT, val_name NVARCHAR(" ) CONF_VAL_NAME_LEN _T( "), val_data NVARCHAR( " ) CONF_VAL_DATA_LEN _T( ") )" )


#define SQL_INSERT_REG_DEFINE _T( "INSERT INTO " ) DEFENSE_REG_PROTECT_TABLE _T( " ( reg_path, class_name, desc, flag ) VALUES ( '%s', '%s', '%s', %d ) " )

#define SQL_INSERT_PORT_DEFINE _T( "INSERT INTO " ) FW_PORT_DEFINE_TABLE _T( " ( port_begin, port_end, type, class, desc, flag ) VALUES ( %d, %d, %d, '%s', '%s', %d )" )

#define SQL_INSERT_APP_DEFINE _T( "INSERT INTO " ) APP_DEFINE_TABLE _T( " ( name, class, desc, flag ) VALUES ( '%s', '%s', '%s', %d )" ) 

#define SQL_INSERT_FW_RULE _T( "INSERT INTO " ) FW_RULE_TABLE _T( " ( class_name, id, class_name2, id2, class_name3, id3, class_name4, id4, class_name5, id5, action, type, desc, flag ) VALUES ( '%s', %d, '%s', %d, '%s', %d, '%s', %d, '%s', %d, %d, %d, '%s', %d ) " )

#define SQL_INSERT_DEFENSE_RULE _T( "INSERT INTO " ) DEFENSE_RULE_TABLE _T( " ( class_name, id, class_name2, id2, class_name3, id3, class_name4, id4, class_name5, id5, action, type, desc, flag ) VALUES ( '%s', %d, '%s', %d, '%s', %d, '%s', %d, '%s', %d, %d, %d, '%s', %d ) " )

#define SQL_INSERT_FW_LOG _T( "INSERT INTO " ) FW_LOG_TABLE _T( "( proc_name, action, type, prot, src_ip, src_port, dest_ip, dest_port, desc, time ) VALUES ( '%s', %d, %d, %d, %d, %d, %d, %d, '%s', datetime( %d, 'unixepoch') )" )

#define SQL_INSERT_FILE_DEFINE _T( "INSERT INTO " ) DEFENSE_FILE_PROTECT_TABLE _T( " ( file_path, class_name, desc, flag ) VALUES ( '%s', '%s', '%s', %d ) " )

#define SQL_INSERT_COMMON_DEFINE _T( "INSERT INTO " ) COMMON_DEFINE_TABLE _T( " ( name, class, desc, flag ) VALUES ( '%s', '%s', '%s', %d ) " )

#define SQL_INSERT_COM_DEFINE _T( "INSERT INTO " ) DEFENSE_COM_PROTECT_TABLE _T( " ( com_name, class_name, desc, flag ) VALUES ( '%s', '%s', '%s', %d ) " ) 

#define SQL_INSERT_DEFENSE_LOG _T( "INSERT INTO " ) DEFENSE_LOG_TABLE _T( "( proc_info, flag, action, target_info, time ) values ( '%s', %d, %d, '%s', datetime( %d, 'unixepoch' ) )" )

#define SQL_INSERT_URL_DEFINE _T( "INSERT INTO " ) FW_URL_DEFINE_TABLE _T( " ( url, class, desc, flag ) VALUES ( '%s', '%s', '%s', %d ) " ) 

#define SQL_INSERT_IP_DEFINE _T( "INSERT INTO ") FW_IP_DEFINE_TABLE _T( " ( ip_begin, ip_end, class, desc, flag ) VALUES ( %I64u, %I64u, '%s', '%s', %d ) " )

#define SQL_INSERT_IP_DEFINE _T( "INSERT INTO ") FW_IP_DEFINE_TABLE _T( " ( ip_begin, ip_end, class, desc, flag ) VALUES ( %I64u, %I64u, '%s', '%s', %d ) " )

#define SQL_INSERT_DRIVER_LOAD_CONF _T( "INSERT INTO ") DRIVER_LOAD_CONF_TABLE _T( " ( drv_id, drv_state ) VALUES ( %u, %u ) " )

#define SQL_INSERT_GENERAL_CONF _T( "INSERT INTO ") GENERAL_CONF_TABLE _T( " ( val_name, val_data ) VALUES ( %s, %s ) " )

#define SQL_UPDATE_DRIVER_LOAD_CONF _T( "UPDATE ") DRIVER_LOAD_CONF_TABLE _T( " SET drv_state=%u WHERE drv_id=%u" )

//delete sql
//#define SQL_DELETE_REG_DEFINE _T( "DELETE FROM " ) DEFENSE_REG_PROTECT_TABLE 
//#define SQL_DELETE_PORT_DEFINE _T( "DELETE FROM " ) FW_PORT_DEFINE_TABLE 
//#define SQL_DELETE_APP_DEFINE _T( "DELETE FROM " ) APP_DEFINE_TABLE 

//#define SQL_DELETE_FW_LOG _T( "DELETE FROM " ) FW_LOG_TABLE 
//#define SQL_DELETE_FILE_DEFINE _T( "DELETE FROM " ) DEFENSE_FILE_PROTECT_TABLE 
//#define SQL_DELETE_COM_DEFINE _T( "DELETE FROM " ) DEFENSE_COM_PROTECT_TABLE 
//#define SQL_DELETE_DEFENSE_LOG _T( "DELETE FROM " ) DEFENSE_LOG_TABLE 
//#define SQL_DELETE_URL_DEFINE _T( "DELETE FROM " ) FW_URL_DEFINE_TABLE 
//#define SQL_DELETE_IP_DEFINE _T( "DELETE FROM ") FW_IP_DEFINE_TABLE 
//#define SQL_DELETE_FW_RULE _T( "DELETE FROM " ) FW_RULE_TABLE 
//#define SQL_DELETE_DEFENSE_RULE _T( "DELETE FROM " ) DEFENSE_RULE_TABLE 

#endif //__SQL_STMT_DEF_H__