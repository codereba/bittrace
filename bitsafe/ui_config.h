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