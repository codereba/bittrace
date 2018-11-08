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

#include "StdAfx.h"
#include "common_func.h"
#include "tooltip.h"

static BOOLEAN stop_control = FALSE; 

ULONG WINAPI tooltip_control_thread( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 Ret; 
	CTooltipWindow *tooltip; 
	RECT WindowRect; 
	POINT cursor_pos; 
	BOOLEAN cursor_in_control; 
	ULONG sleep_time; 

	do 
	{
		ASSERT( NULL != param ); 

		tooltip = ( CTooltipWindow* )param; 
		sleep_time = 0; 

		for( ; ; )
		{
#define TOOLTIP_DISPLAY_CHECK_TIME 100
			Sleep( TOOLTIP_DISPLAY_CHECK_TIME ); 

			if( stop_control == TRUE )
			{
				break; 
			}

			do 
			{
				cursor_in_control = FALSE; 

				if( NULL == tooltip->GetHWND() 
					|| FALSE == ::IsWindow( tooltip->GetHWND() ) ) 
				{
					cursor_in_control = TRUE; 
					break; 
				}

				Ret = GetWindowRect( tooltip->GetHWND(), &WindowRect ); 

				if( Ret == FALSE )
				{
					break; 
				}

				Ret = GetCursorPos( &cursor_pos ); 
				if( Ret == FALSE )
				{
					break;
				}

				if( TRUE == PtInRect( &WindowRect, cursor_pos ) )
				{
					cursor_in_control = TRUE; 
				}
			}while( FALSE );

			if( cursor_in_control == FALSE )
			{
				sleep_time += 1; 

				if( sleep_time >= DISAPPEAR_DELAY_TIME )
				{
					tooltip->Close(); 
					sleep_time = 0; 
				}
			}
			else
			{
				sleep_time = 0; 
			}
		}
	}while( FALSE );

	return ret; 
}

HANDLE thread_tooltip_control = NULL; 
LRESULT WINAPI start_tooltip_control( CTooltipWindow *tooltip )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		stop_control = FALSE; 

		thread_tooltip_control = CreateThread( NULL, 0, tooltip_control_thread, ( PVOID )tooltip, 0, NULL ); 
		if( thread_tooltip_control == NULL )
		{
			ret = GetLastError(); 
			break; 
		}

	}while( FALSE );

	return ret; 
}
LRESULT WINAPI stop_tooltip_control() 
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG wait_ret; 

	do 
	{
		stop_control = TRUE; 

		if( NULL != thread_tooltip_control )
		{
#define WAIT_TOOLTIP_CONTROL_THREAD_TIME 10

			do 
			{
				wait_ret = WaitForSingleObject( thread_tooltip_control, WAIT_TOOLTIP_CONTROL_THREAD_TIME ); 

				if( wait_ret == WAIT_OBJECT_0 )
				{
					break; 
				}
				else if( wait_ret == WAIT_ABANDONED )
				{
					ASSERT( FALSE ); 
				}
				else if( wait_ret == WAIT_FAILED )
				{
					ret = GetLastError(); 
					break; 
				}

				ret = TerminateThread( thread_tooltip_control, 0 ); 
				if( FALSE == ret )
				{
					ret = GetLastError(); 
					break; 
				}
			}while( FALSE );

			CloseHandle( thread_tooltip_control ); 
		}
	}while( FALSE );

	return ret; 
}
