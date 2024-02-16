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

#include "acl_define.h"
#include "wnd_state.h"
#include "md5.h"

#define BITSAFE_USER_CONF_FILE _T( "data.dat" )
#define PWD_DATA_LEN MD5_DIGEST_LENGTH

typedef enum _user_group
{
	USER, 
	ADMIN
} user_group; 

LRESULT change_original_pwd( HWND parent_wnd ); 
LRESULT check_user_access( user_group check_group, HWND parent_wnd, dlg_ret_state *ret_state = NULL ); 
user_group get_cur_user_group(); 
void set_cur_user_group( user_group group ); 
LRESULT get_cur_pwd( PBYTE cur_pwd, ULONG buf_len ); 
LRESULT write_pwd( PBYTE pwd, ULONG buf_len ); 
LRESULT check_pwd_file_consist( BYTE pwd_out[ PWD_DATA_LEN ], ULONG buf_len ); 

INLINE LRESULT check_admin_access( HWND parent_wnd )
{
	return check_user_access( ADMIN, parent_wnd ); 
}

LRESULT clear_password(); 
LRESULT check_none_pwd( INT32 *is_none ); 
LRESULT get_cur_work_mode( work_mode *mode_got, action_ui_notify *ui_notify ); 
LRESULT set_cur_work_mode( work_mode mode, action_ui_notify *ui_notify ); 
