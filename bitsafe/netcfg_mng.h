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
 
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

LRESULT switch_netcfg( INT32 is_on ); 
DWORD CALLBACK thread_netcfg_switch( PVOID param ); 
LRESULT CALLBACK com_reg_switch( LPCTSTR dll_name, INT32 is_reg ); 
LRESULT start_netcfg_manage(); 
LRESULT start_net_cfg(); 
LRESULT stop_net_cfg(); 

#ifdef __cplusplus
}
#endif
