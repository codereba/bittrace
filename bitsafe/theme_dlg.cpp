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