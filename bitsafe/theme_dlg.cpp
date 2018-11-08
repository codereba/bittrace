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
 
 #include "StdAfx.h"
#include "common_func.h"
#include "ui_ctrl.h"
#include "ui_config.h"
#include "theme_dlg.h"

LRESULT CALLBACK on_themes_found( LPCTSTR root_path, WIN32_FIND_DATA *find_data, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	themes_dlg *dlg; 

	dlg = ( themes_dlg* )context; 

	ASSERT( find_data != NULL ); 

	if( 0 != ( find_data->dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY ) )
	{
		if( 0 != _tcscmp( find_data->cFileName, _T( ".." ) ) 
			&& 0 != _tcscmp( find_data->cFileName, _T( "." ) ) )
		{
			dlg->SendMessage( WM_THEME_NOTIFY, ( WPARAM )root_path, ( LPARAM )find_data->cFileName ); 
		}
	}
	//else
	//{
	//	ret = ERROR_INVALID_PARAMETER; 
	//}

	return ret; 
}