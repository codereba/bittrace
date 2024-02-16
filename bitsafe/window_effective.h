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

#ifndef __WINDOW_EFFICTIVE_H__
#define __WINDOW_EFFICTIVE_H__

#define RECT_HEIGHT( _rc_ ) _abs( ( _rc_ ).bottom - ( _rc_ ).top )
#define RECT_WIDTH( _rc_ ) _abs( ( _rc_ ).right - ( _rc_ ).left )

LRESULT short_window_close( CPaintManagerUI *pm, HWND wnd, ULONG delay, ULONG step ); 
LRESULT transparent_animate( HWND wnd, ULONG time ); 

#endif //__WINDOW_EFFICTIVE_H__