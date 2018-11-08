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
