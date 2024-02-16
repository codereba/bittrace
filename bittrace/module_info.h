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
�õ�ϵͳ�¼������еĽ��̺�ģ��ľ�����Ϣ,����ʹ�����·���;
1.��ÿһ�����յ��¼�ʱ��������/����(��ϵͳ)һ�ν���/ģ��ľ�����Ϣ
2.���ٿ�ʼʱ����ʼ��¼,����ϵͳ�н���ģ������б仯����ÿ�ν���/ģ��仯ʱ������
��Ӧ�ĸ��£��õ�ÿ���¼���˲�����,ģ��״̬��
	����/ģ�����Ϣ��¼�����Ľ���/ģ����Ϣ�б��С�

��2�ַ������и��õ����ܣ���1�����2 �ַ������в�ͬ��������Ժ�ԭ��
�����趨ʹ�õ�2�ַ���ʵ�ֹ��ܡ�
********************************************************************************/

/*******************************************************************************
�����ܷ��ŷ���ϵͳ:
��һ����ͨ��������DBGHELP�Ż���Ĵ��������/ģ����Ϣ�б�һ���齨�ɸ����ܵĵ���
��Ϣ����ϵͳ��
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