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

#ifndef __MODULE_INFO_H__
#define __MODULE_INFO_H__

#include <vector>
using namespace  std;

typedef struct _TIME_DURATION
{
	LARGE_INTEGER start_time; 
	LARGE_INTEGER end_time; 
} TIME_DURATION, *PTIME_DURATION; 

typedef struct _MODULE_INFO
{
	TIME_DURATION duration; 
	PVOID base_addr; 
	ULONG64 sym_base_addr; 
	ULONG size; 

	ULONG time_stamp; 
	ULONG name_len; 
	LPCWSTR final_component; 
	WCHAR file_name[ 1 ]; //MAX_PATH 
} MODULE_INFO, *PMODULE_INFO; 

typedef struct _MODULE_INFO_PTR
{
	TIME_DURATION duration; 
	PVOID base_addr; 
	ULONG size; 

	ULONG time_stamp; 
	ULONG name_len; 
	WCHAR *file_name; //MAX_PATH 
} MODULE_INFO_PTR, *PMODULE_INFO_PTR; 

typedef vector<MODULE_INFO*> MODULE_INFOS; 
typedef MODULE_INFOS::iterator MODULE_INFO_ITERATOR; 

typedef struct _MODULE_INFO_TABLE
{
	CRITICAL_SECTION lock; 
	TIME_DURATION duration; 
	MODULE_INFOS modules; 
}MODULE_INFO_TABLE, *PMODULE_INFO_TABLE; 

//#define MAX_CMD_LINE 1024 
//#define MAX_USER_NAME 128 
//#define MAX_AUTH_ID 64 
//#define MAX_VERSION 64 
//
//typedef struct _PROCESS_ADDITION_INFO
//{
//	WCHAR cmd_line[ MAX_CMD_LINE ]; 
//	ULONG virtual_tech; 
//	ULONG cpu_arch; 
//	ULONG session_id; 
//	WCHAR user_name[ MAX_USER_NAME ]; 
//	WCHAR auth_id[ MAX_AUTH_ID ]; 
//	ULONG integrety; 
//	WCHAR file_version[ MAX_VERSION ]; 
//}PROCESS_ADDITION_INFO, *PPROCESS_ADDITION_INFO; 

#include "process_info.h"

typedef struct _PROCESS_MODULE_INFO_TABLE
{
	ULONG proc_id; 
	TIME_DURATION duration; 
	Process  info; 
	MODULE_INFO_TABLE modules; 
} PROCESS_MODULE_INFO_TABLE, *PPROCESS_MODULE_INFO_TABLE; 

/********************************************************************************
得到系统事件过程中的进程和模块的具体信息,可以使用以下方法;
1.在每一个接收到事件时，都计算/接收(从系统)一次进程/模块的具体信息
2.跟踪开始时，开始记录,跟踪系统中进程模块的所有变化，在每次进程/模块变化时，进行
相应的更新，得到每个事件的瞬间进程,模块状态。
	进程/模块的信息记录单独的进程/模块信息列表中。

第2种方法具有更好的性能，第1种与第2 种方法具有不同的误差特性和原因。
初步设定使用第2种方法实现功能。
********************************************************************************/

/*******************************************************************************
高性能符号分析系统:
下一步将通过链接入DBGHELP优化后的代码与进程/模块信息列表一起组建成高性能的调试
信息分析系统。
*******************************************************************************/
#define INVALID_MODULE_INDEX ( ULONG )( -1 ) 

MODULE_INFO_TABLE *get_kernel_modules(); 

LRESULT WINAPI init_module_infos(); 

LRESULT WINAPI uninit_module_infos(); 

LRESULT WINAPI on_module_change( ULONG proc_id, 
						 MODULE_INFO_PTR *module, 
						 BOOLEAN is_remove ); 


LRESULT WINAPI on_process_change( ULONG proc_id, 
								 LPCWSTR file_path, 
								 BOOLEAN is_remove ); 

LRESULT WINAPI get_proc_module_info( ULONG proc_id, LARGE_INTEGER *time, MODULE_INFO_TABLE **modules ); 
LRESULT WINAPI release_proc_module_info( MODULE_INFO_TABLE *modules ); 

LRESULT WINAPI _get_proc_module_info( ULONG proc_id, LARGE_INTEGER *time, PROCESS_MODULE_INFO_TABLE **proc_info ); 
LRESULT WINAPI _release_proc_module_info( PROCESS_MODULE_INFO_TABLE *proc_info ); 

LRESULT WINAPI load_current_module_infos(); 

#endif //__MODULE_INFO_H__