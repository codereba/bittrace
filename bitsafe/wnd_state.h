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
