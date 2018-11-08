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

#ifndef __FOCUS_PROC_DLG_H__
#define __FOCUS_PROC_DLG_H__

#define FOCUS_PROCESS_TIMER_ID 0x9001
extern HWND located_main_wnd; 
extern BOOLEAN focus_proc_button_down; 

LRESULT on_focus_proc_button_down( HWND main_wnd ); 
LRESULT on_focues_proc_button_up( HWND main_wnd, ULONG *proc_id ); 
LRESULT on_focus_proc_timer( UINT nIDEvent ); 

#endif //__FOCUS_PROC_DLG_H__