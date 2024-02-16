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
 
#ifndef __CRT_MEM_LEAK_DBG_H__
#define __CRT_MEM_LEAK_DBG_H__

#ifdef _DEBUG_MEM_LEAKS

#ifndef _DEBUG_MEM_LEAKS_DEFINED
#define _DEBUG_MEM_LEAKS_DEFINED
#define CRTDBG_MAP_ALLOC 
#define _CRTDBG_MAP_ALLOC 

#include <stdlib.h> 
#include <crtdbg.h> 

#ifndef DBG_NORMAL_BLOCK
#define DBG_NORMAL_BLOCK new ( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif //DBG_NORMAL_BLOCK

#ifndef DEBUG_CLIENT_BLOCK 
#define DEBUG_CLIENT_BLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif //DEBUG_CLIENT_BLOCK 

#define new DBG_NORMAL_BLOCK
#endif //_DEBUG_MEM_LEAKS_DEFINED
#endif //_DEBUG_MEM_LEAKS

#endif //__CRT_MEM_LEAK_DBG_H__