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

#ifndef __WINDOW_EFFICTIVE_H__
#define __WINDOW_EFFICTIVE_H__

#define RECT_HEIGHT( _rc_ ) _abs( ( _rc_ ).bottom - ( _rc_ ).top )
#define RECT_WIDTH( _rc_ ) _abs( ( _rc_ ).right - ( _rc_ ).left )

LRESULT short_window_close( CPaintManagerUI *pm, HWND wnd, ULONG delay, ULONG step ); 
LRESULT transparent_animate( HWND wnd, ULONG time ); 

#endif //__WINDOW_EFFICTIVE_H__