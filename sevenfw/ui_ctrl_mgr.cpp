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

#include "common_func.h"
#include "ui_ctrl.h"
#include "ui_ctrl_mgr.h"

LRESULT init_all_ui_ctrls()
{
	LRESULT ret = ERROR_SUCCESS; 
	ret = init_ui_ctrls(); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = add_ui_ctrl( BITSAFE_UI_ID, 
		do_bitsafe_ui_work, 
		sevefw_ui_output ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

_return:
	return ret; 
}
