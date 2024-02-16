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

#ifndef __LANG_EN_H__
#define __LANG_EN_H__

#include "string_common.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern pcchar_t all_strings_en[]; 

#ifdef _DEBUG
extern ULONG all_strings_en_count; 
#define LAST_EN_STRING __t( "last en string" )
#endif //_DEBUG

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__LANG_EN_H__