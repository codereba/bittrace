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
