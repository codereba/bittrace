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

#ifndef __UI_DEFINE_H__
#define __UI_DEFINE_H__

#define INVALID_THEME_NO ( ( ULONG )255 )
#define DEFAULT_THEME_NO ( ( ULONG )( 0 ) )
typedef struct _theme_setting
{
	ULONG bk_clr; 
	ULONG bar_clr; 
	WCHAR theme_desc[ MAX_PATH ]; 
} theme_setting, *ptheme_setting; 

typedef struct _theme_define
{
	WCHAR bk_file[ MAX_PATH ]; 
	WCHAR thumb_file[ MAX_PATH ]; 
	theme_setting setting; 
} theme_define, *ptheme_define; 

VOID set_theme_setting( theme_setting *theme ); 
VOID get_theme_setting( theme_setting *theme_out ); 
LONG get_tranparent(); 
VOID set_transparent( LONG transparent ); 
VOID set_theme_no( ULONG no ); 
ULONG get_theme_no(); 
VOID set_slogan_show( INT32 show ); 
INT32 get_slogan_show(); 

#define NO_ALPHA 0x00000001
LRESULT set_ui_style( PVOID ui_manager, ULONG flags = 0 ); 

#endif //__UI_DEFINE_H__