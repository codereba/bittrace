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

#ifndef __MINI_PORT_DEV_H__
#define __MINI_PORT_DEV_H__


#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus

HRESULT switch_mini_port_func( HANDLE mini_port, ULONG sw_id, BYTE state ); 


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__MINI_PORT_DEV_H__