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

#ifndef __STACK_DUMP_H__
#define __STACK_DUMP_H__

#include "analyze_config.h"

#define MODULE_NAME_LEN MAX_PATH
#define SYMBOL_NAME_LEN MAX_PATH
typedef struct _DUMP_SYMBOL_INFO
{
	ULONGLONG dwAddress;
	ULONGLONG dwOffset;
	WCHAR	szModule[MODULE_NAME_LEN];
	WCHAR	szSymbol[SYMBOL_NAME_LEN];
} DUMP_SYMBOL_INFO; 

NTSTATUS WINAPI dump_stack( ULONG proc_id, PVOID call_stack[], ULONG frame_count, LPCWSTR sym_path ); 

LRESULT WINAPI format_sql_string_value( LPWSTR string_value, ULONG cc_string_len, ULONG *cc_ret_len ); 

NTSTATUS WINAPI _dump_stack( ULONG proc_id, PVOID call_stack[], ULONG frame_count ); 

LRESULT WINAPI init_sym_context(); 
LRESULT WINAPI uninit_sym_context(); 

LRESULT WINAPI resolve_symbol( __in HANDLE hProcess, 
					   __in ULONGLONG dwAddress,
					   DUMP_SYMBOL_INFO &siSymbol ); 

/************************************************************************************
符号加载高速方案:
1.对需要的模块定位，解析出符号，将此符号缓存。而不是不同进程中的不同模块每次解析一次。
2.?
************************************************************************************/
LRESULT WINAPI get_stack_trace_dump_text_ex( ULONG proc_id, 
								  PLARGE_INTEGER time, 
								  ULONGLONG call_stack[], 
								  ULONG frame_count, 
								  LPWSTR sym_path, 
								  LPWSTR stack_dump, 
								  ULONG ccb_buf_len, 
								  ULONG *ccb_ret_len ); 

#define MAX_STACK_DUMP_TEXT_LEN 2048

LRESULT _get_stack_trace_dump_text( ULONG proc_id, 
								   PLARGE_INTEGER time, 
								   ULONGLONG call_stack[],  
								   ULONG frame_count, 
								   LPWSTR stack_dump, 
								   ULONG ccb_buf_len, 
								   ULONG *ccb_ret_len ); 

LRESULT WINAPI get_stack_trace_dump_text( ULONG proc_id, 
										 ULONGLONG call_stack[],  								  
										 ULONG frame_count, 
										 LPWSTR sym_path, 
										 LPWSTR stack_dump, 
										 ULONG ccb_buf_len, 
										 ULONG *ccb_ret_len ); 

#endif //__STACK_DUMP_H__