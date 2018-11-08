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

#include "stdafx.h"
#include "ui_ctrl.h"
#include "focus_proc_dlg.h"
#include "resource.h"

HWND located_main_wnd = NULL; 
BOOLEAN focus_proc_button_down = FALSE; 
LRESULT on_focus_proc_button_down( HWND main_wnd )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HCURSOR cursor = NULL; 
	HMODULE module_handle; 
	UINT_PTR timer_id; 

	//HICON show_icon; 

	do 
	{
		module_handle = GetModuleHandle( NULL ); 

		if( NULL == module_handle )
		{
			break; 
		}

		SetCapture( main_wnd ); 
		focus_proc_button_down = TRUE; 

		cursor = LoadCursor( module_handle, MAKEINTRESOURCE( IDC_FOCUS_WND_CURSOR ) ); 
		
		ASSERT( cursor != NULL ); 

		SetCursor( cursor );

		
#define FOCUS_TIME_ELAPSE 600
		timer_id = SetTimer( main_wnd, FOCUS_PROCESS_TIMER_ID, FOCUS_TIME_ELAPSE, NULL ); 

	}while( FALSE );

	return ret; 
}

LRESULT on_focues_proc_button_up( HWND main_wnd, ULONG *proc_id )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	POINT pt; 
	HWND grayHwnd; 
	RECT tempRc;
	BOOL bFind = FALSE; 
	ULONG iPID = 0;

	do 
	{
		ASSERT( main_wnd != NULL ); 
		ASSERT( proc_id != NULL ); 

		ReleaseCapture();
		focus_proc_button_down = FALSE; 

		KillTimer( main_wnd, FOCUS_PROCESS_TIMER_ID ); 


		::GetCursorPos( &pt );
		located_main_wnd = WindowFromPoint( pt ); 
		grayHwnd = ::GetWindow(located_main_wnd, GW_CHILD);
		while (grayHwnd)
		{
			::GetWindowRect(grayHwnd, &tempRc);
			if(::PtInRect(&tempRc,pt))
			{
				bFind = TRUE;
				break;
			}
			else
				grayHwnd = ::GetWindow(grayHwnd, GW_HWNDNEXT);

		} 

		if( bFind == TRUE )
		{
			bFind= FALSE;
			located_main_wnd = grayHwnd;
		}
		else
		{

		}

		GetWindowThreadProcessId( located_main_wnd, &iPID );

		*proc_id = ( ULONG )iPID; 


	}while( FALSE );

	return ret; 
}

LRESULT on_focus_proc_timer( UINT nIDEvent ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	INT32 old_rop2 = 0; 

	HWND desktop_wnd;    //取得桌面句柄
	HWND main_wnd_on_pos; 
	HWND wnd_on_pos; 

	RECT main_wnd_rect;
	RECT wnd_rect;

	HDC desktop_dc = NULL;     //取得桌面设备场景

	HPEN draw_pen = NULL; 
	HGDIOBJ old_pen = NULL; 

	POINT cur_pos;
	BOOL wnd_found = FALSE;

	do 
	{
		desktop_wnd = ::GetDesktopWindow(); 
		desktop_dc = ::GetWindowDC( desktop_wnd ); 

		old_rop2 = SetROP2( desktop_dc, R2_NOTXORPEN ); 

		GetCursorPos( &cur_pos );                
		
		main_wnd_on_pos = WindowFromPoint(cur_pos); 

		located_main_wnd = main_wnd_on_pos;
		wnd_on_pos = GetWindow( located_main_wnd, GW_CHILD ); 

		while( wnd_on_pos ) 
		{
			GetWindowRect( wnd_on_pos, &wnd_rect ); 

			_ret = PtInRect( &wnd_rect, cur_pos ); 

			if( TRUE == _ret ) 
			{
				wnd_found = TRUE;
				break;
			}
			else
			{
				wnd_on_pos = GetWindow( wnd_on_pos, GW_HWNDNEXT ); 
			}
		}

		if( wnd_found == TRUE)
		{
			wnd_found = FALSE; 
			located_main_wnd = wnd_on_pos; 
		}
		else
		{

		}

		_ret = GetWindowRect( located_main_wnd, &main_wnd_rect ); 
		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		if( main_wnd_rect.left < 0 ) 
		{
			main_wnd_rect.left = 0; 
		}

		if( main_wnd_rect.top < 0 ) 
		{
			main_wnd_rect.top = 0;
		}

#define	BORDER_DRAW_COLOR RGB( 125,0,125 ) 
#define BORDER_DRAW_WIDTH 3 
		draw_pen = CreatePen( 0, BORDER_DRAW_WIDTH, BORDER_DRAW_COLOR ); 
		if( NULL == draw_pen )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		old_pen = ::SelectObject( desktop_dc, draw_pen ); 
		
		_ret = Rectangle( desktop_dc, 
			main_wnd_rect.left, 
			main_wnd_rect.top, 
			main_wnd_rect.right, 
			main_wnd_rect.bottom );  

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
		
#define SPARK_TIME 400
		Sleep( SPARK_TIME );    
		
		_ret = Rectangle( desktop_dc, 
			main_wnd_rect.left, 
			main_wnd_rect.top, 
			main_wnd_rect.right, 
			main_wnd_rect.bottom ); 

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	if( old_rop2 != 0 )
	{
		SetROP2( desktop_dc, old_rop2 ); 
	}
	
	if( old_pen != NULL )
	{
		SelectObject( desktop_dc, old_pen); 
	}

	if( NULL != draw_pen )
	{
		DeleteObject(draw_pen); 
	}

	if( desktop_dc != NULL )
	{
		ReleaseDC( desktop_wnd, desktop_dc);
	}

	return ret; 
}

