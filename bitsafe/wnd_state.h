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

typedef enum _dlg_ret_state
{
	CANCEL_STATE, 
	OK_STATE
} dlg_ret_state; 

class wnd_state
{
public:
	wnd_state(void) : ret_status( CANCEL_STATE )
	{

	}

	~wnd_state(void)
	{

	}

	dlg_ret_state get_dlg_ret_statue()
	{
		return ret_status; 
	}

	void set_dlg_ret_status( dlg_ret_state state )
	{
		ret_status = state; 
	}

protected:
	dlg_ret_state ret_status; 
};
