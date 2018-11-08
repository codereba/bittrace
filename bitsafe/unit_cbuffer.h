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


#ifndef __UNIT_CBUFFER_H__
#define __UNIT_CBUFFER_H__

#include "acl_define.h"


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

NTSTATUS output_sys_action_desc( safe_cbuffer *cbuf, sys_action_desc *action_output, PVOID data, ULONG data_len ); 
NTSTATUS read_sys_action_desc_begin( safe_cbuffer *cbuf, cbuffer_iter *cbuf_iter_out ); 
NTSTATUS read_sys_action_desc_end( safe_cbuffer *cbuf, cbuffer_iter *cbuf_iter ); 
NTSTATUS cbuffer_copy_sys_action_desc( safe_cbuffer *cbuf, sys_action_output *output, ULONG buf_len ); 

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__UNIT_CBUFFER_H__