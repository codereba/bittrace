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
符号扫描器的工作机制:
1.符号扫描器用来加速符号的识别过程，使用了以下机制:
	1.uqf模块在内核/不同进程的虚地址空间进行保存，这样不需要反复识别.
	2.自定制符号文件保存方式，将查找符号的速度最优化
	3.分析符号文件的内容，将分析符号信息的速度最优化

******************************************************************************************/

typedef struct _SYMBOL_CACHE
{
	
} SYMBOL_CACHE, *PSYMBOL_CACHE; 

typedef struct _PROCESS_SYMBOL_INFORMATION
{
	HANDLE proc_id; 
} PROCESS_SYMBOL_INFORMATION, *PPROCESS_SYMBOL_INFORMATION; 

LRESULT WINAPI get_symbol( LPCWSTR file_name, ULONG_PTR rva, LPWSTR symbol_out, ULONG cc_buf_len, ULONG * cc_ret_len ); 