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

#ifndef __FOCUS_PROC_DLG_H__
#define __FOCUS_PROC_DLG_H__

#define FOCUS_PROCESS_TIMER_ID 0x9001
extern HWND located_main_wnd; 
extern BOOLEAN focus_proc_button_down; 

LRESULT on_focus_proc_button_down( HWND main_wnd ); 
LRESULT on_focues_proc_button_up( HWND main_wnd, ULONG *proc_id ); 
LRESULT on_focus_proc_timer( UINT nIDEvent ); 

#endif //__FOCUS_PROC_DLG_H__