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
