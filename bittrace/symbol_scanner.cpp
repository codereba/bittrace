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

#include "common_func.h"

#include <strsafe.h>
#include "ui_ctrl.h"
#include "acl_define.h"
#include "action_result.h"

/*****************************************************************************************
����ɨ�����Ĺ�������:
1.����ɨ�����������ٷ��ŵ�ʶ����̣�ʹ�������»���:
	1.uqfģ�����ں�/��ͬ���̵����ַ�ռ���б��棬��������Ҫ����ʶ��.
	2.�Զ��Ʒ����ļ����淽ʽ�������ҷ��ŵ��ٶ����Ż�
	3.���������ļ������ݣ�������������Ϣ���ٶ����Ż�

******************************************************************************************/

typedef struct _SYMBOL_CACHE
{
	
} SYMBOL_CACHE, *PSYMBOL_CACHE; 

typedef struct _PROCESS_SYMBOL_INFORMATION
{
	HANDLE proc_id; 
} PROCESS_SYMBOL_INFORMATION, *PPROCESS_SYMBOL_INFORMATION; 

LRESULT WINAPI get_symbol( LPCWSTR file_name, ULONG_PTR rva, LPWSTR symbol_out, ULONG cc_buf_len, ULONG * cc_ret_len ); 