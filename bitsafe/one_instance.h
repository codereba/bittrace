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
#pragma once

#define BITSAFE_INST_PROP_NAME _T( "BITSAFE_INST_PROP" )
#define BITSAFE_UPDATE_INST_PROP_NAME _T( "BITSAFE_UPDATE_INST_PROP" )
#define BITSAFE_INST_TIP _T( "BITSAFE_INST_TIP" )
#define BITSAFE_UPDATE_INST_TIP _T( "BITSAFE_UPDATE_INST_TIP" )
#define BITSAFE_DAEMON_INST_TIP _T( "BITSAFE_DAEMON_INST_TIP" )

#define UPDATE_INSTANCE 0x00000001
#define DAEMON_INSTANCE 0x00000002

LRESULT check_app_instance( ULONG flags ); 
LRESULT set_app_flag( HWND wnd, INT32 is_update ); 
LRESULT remove_app_flag( HWND wnd, INT32 is_update ); 
LRESULT kill_app_instance( INT32 is_update ); 