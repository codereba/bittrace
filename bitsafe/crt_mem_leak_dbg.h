/*
 *
 * Copyright 2010 JiJie Shi
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

//#define new DEBUG_CLIENTBLOCK 
#define new DBG_NORMAL_BLOCK

//#ifdef new
//#undef new
//#endif //new 

//#ifdef _DEBUG_MEM_LEAKS
//
//#ifdef DBG_NORMAL_BLOCK
//#define new DBG_NORMAL_BLOCK
//#endif //DBG_NORMAL_BLOCK
//
//#endif //_DEBUG_MEM_LEAKS
#endif //_DEBUG_MEM_LEAKS_DEFINED
#endif //_DEBUG_MEM_LEAKS

#endif //__CRT_MEM_LEAK_DBG_H__