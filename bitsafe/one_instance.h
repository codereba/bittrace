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