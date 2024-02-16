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

#include "_StdAfx.h"
#include "ui_config.h"

ULONG ui_transparent = 222; 
ULONG theme_no = DEFAULT_THEME_NO; 
theme_setting theme_set; 
INT32 show_slogan = TRUE; 

VOID set_theme_setting( theme_setting *theme )
{
	ASSERT( theme != NULL ); 

	theme_set.bar_clr = theme->bar_clr; 
	theme_set.bk_clr = theme->bk_clr; 
}

VOID get_theme_setting( theme_setting *theme_out )
{
	ASSERT( theme_out != NULL ); 

	memcpy( theme_out, &theme_set, sizeof( theme_set ) ); 
}

LONG get_tranparent()
{
	return ui_transparent; 
}

VOID set_transparent( LONG transparent )
{
	ui_transparent = transparent; 
}

VOID set_theme_no( ULONG no )
{
	if( no != INVALID_THEME_NO )
	{
		theme_no = no; 
	}
}

VOID set_slogan_show( INT32 show )
{
	show_slogan = show; 
}

INT32 get_slogan_show()
{
	return show_slogan; 
}

ULONG get_theme_no( )
{
	return theme_no; 
}

LRESULT set_ui_style( PVOID ui_manager, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *form_layout; 

	CPaintManagerUI *pm; 

	ASSERT( ui_manager != NULL ); 

	pm = ( CPaintManagerUI* )ui_manager; 

	if( 0 == ( flags & NO_ALPHA ) )
	{
		pm->SetTransparent( ui_transparent ); 
	}

	form_layout = pm->FindControl( _T( "form_layout" ) ); 
	if( form_layout != NULL )
	{
		form_layout->SetBkColor( 0xff357913 ); 
	}

	return ret; 
}
