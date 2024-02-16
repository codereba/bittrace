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