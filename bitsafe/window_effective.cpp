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

#include "stdafx.h"
#include "window_effective.h"

#define SHORT_WINDOW_DELAY 1
#define SHORT_WINDOW_STEP_SIZE 5

LRESULT short_window_close( CPaintManagerUI *pm, HWND wnd, ULONG delay, ULONG step )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	RECT rect; 
	ULONG height; 
	INT32 i; 
	RECT cur_rect; 
	RECT paint_rc; 
	SIZE old_min_info; 

	do 
	{
		ASSERT( wnd != NULL ); 
		ASSERT( pm != NULL ); 

		if( step == 0 )
		{
			step = SHORT_WINDOW_STEP_SIZE; 
		}

		if( delay == 0 )
		{
			delay = SHORT_WINDOW_DELAY; 
		}

		if( FALSE == ::IsWindow( wnd ) )
		{
			ASSERT( FALSE && "input the invalid window to show effective" ); 
			break; 
		}

		_ret = GetWindowRect( wnd, &rect ); 
		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 

			break; 
		}

		height = _abs( rect.bottom - rect.top ); 

		cur_rect = rect; 

		old_min_info = pm->GetMinInfo(); 

		pm->SetMinInfo( 0, 0 ); 

		for( i = 0; ( ULONG )i < height / 2; i += step )
		{
			cur_rect.bottom = rect.bottom - i; 
			cur_rect.top = rect.top + i; 

			paint_rc = cur_rect; 

			_ret = SetWindowPos( wnd, NULL, cur_rect.left, cur_rect.top, RECT_WIDTH( cur_rect ), RECT_HEIGHT( cur_rect ), SWP_SHOWWINDOW | SWP_NOREDRAW ); 
			if( _ret == FALSE )
			{}
			{
				pm->PaintStretchShortEffective( wnd, paint_rc ); 
			}

			Sleep( delay ); 
		}

		SetWindowPos( wnd, NULL, cur_rect.left, cur_rect.top, RECT_WIDTH( rect ), RECT_HEIGHT( rect ), SWP_HIDEWINDOW ); 

		pm->SetMinInfo( old_min_info.cx, old_min_info.cy ); 

	} while ( FALSE ); 

	return ret; 
}

#define DEF_TRANSPARENT_ANIMATE_TIME 5000

LRESULT transparent_animate( HWND wnd, ULONG time )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	ASSERT( wnd != NULL ); 

	if( FALSE == IsWindow( wnd ) )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( time == 0 )
	{
		time = DEF_TRANSPARENT_ANIMATE_TIME; 
	}

#if(WINVER >= 0x0500)
	_ret = AnimateWindow( wnd, time, AW_ACTIVATE | AW_CENTER | AW_BLEND ); 

	if( _ret == FALSE )
	{
		ret = GetLastError(); 
	}
#endif	// WINVER >= 0x0500

_return:
	return ret; 

}