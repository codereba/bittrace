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
#include "resource.h"
#include "popup_tip_wnd.h"
#include <mmsystem.h>

#pragma comment( lib, "Winmm.lib" )

#define RECT_WIDTH( rect ) ( ULONG )( ( rect ).right - ( rect ).left )
#define RECT_HEIGHT( rect ) ( ULONG )( ( rect ).bottom - ( rect ).top )

ULONG popup_tip_wnd::max_popup_count = 0; 
ULONG popup_tip_wnd::popup_count = 0; 
#define SHOW_WINDOW_ELAPSE_NOTIFY_TIME 1000
ULONG popup_tip_wnd::msg_show_notify_time = SHOW_WINDOW_ELAPSE_NOTIFY_TIME; 
HANDLE popup_tip_wnd::popup_close_notify = NULL; 


#define POPUP_SPACE_USED 1
#define POPUP_SPACE_UNUSE 0
#define POPUP_SPACE_INVALID ( ULONG )( -1 )

typedef struct _popup_space
{
	INT32 state; 
} popup_space, *ppopup_space; 

#define MAX_POPUP_SPACE 15
CRITICAL_SECTION popup_space_lock; 
popup_space all_popup_space[ MAX_POPUP_SPACE ] = { 0 }; 

ULONG CALLBACK thread_timer( PVOID )
{
	LRESULT ret = ERROR_SUCCESS; 


	return ( ULONG )ret; 
}

LRESULT calc_max_popup_wnd_num( RECT *popup_rect, ULONG *max_popup )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	RECT destop_client_rect; 
	ULONG popup_height; 
	ULONG _max_popup; 

	ASSERT( max_popup != NULL ); 
	ASSERT( popup_rect != NULL ); 

	*max_popup = 0; 

	_ret = SystemParametersInfo( SPI_GETWORKAREA, 0, &destop_client_rect, 0 );
	if( FALSE == _ret )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	popup_height = popup_rect->bottom - popup_rect->top; 

	_max_popup = RECT_HEIGHT( destop_client_rect ) / ( RECT_HEIGHT( *popup_rect ) + POPUP_WND_SPAN ); 

	if( _max_popup > MAX_POPUP_SPACE )
	{
		_max_popup = MAX_POPUP_SPACE; 
	}

	*max_popup = _max_popup; 

_return:
	return ret; 
}

LRESULT init_popup_space( RECT *popup_rect, popup_mode mode )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	for( i = 0; i < MAX_POPUP_SPACE; i ++ )
	{
		all_popup_space[ i ].state = POPUP_SPACE_UNUSE; 
	}

	init_cs_lock( popup_space_lock ); 

	return ret; 
}

LRESULT uninit_popup_space()
{
	uninit_cs_lock( popup_space_lock ); 

	return ERROR_SUCCESS; 
}

LRESULT release_popup_space( ULONG index )
{
	LRESULT ret = ERROR_SUCCESS; 

	if( index > MAX_POPUP_SPACE )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	lock_cs( popup_space_lock ); 

	if( all_popup_space[ index ].state == POPUP_SPACE_USED )
	{

		all_popup_space[ index ].state = POPUP_SPACE_UNUSE; 
	}
	else
	{
		ASSERT( FALSE && "release not used space" ); 
	}

	unlock_cs( popup_space_lock ); 
_return:
	return ret; 
}

LRESULT _calc_popup_wnd_rect( ULONG space_index, popup_mode mode, RECT *wnd_rect, RECT *popup_rect )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	CRect destop_client_rect; 
	ULONG popup_height; 
	ULONG popup_width; 
	ULONG screen_height;  

	ASSERT( popup_rect != NULL ); 
	ASSERT( wnd_rect != NULL ); 
	ASSERT( space_index < MAX_POPUP_SPACE ); 

	if( space_index >= popup_tip_wnd::max_popup_count )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	_ret = SystemParametersInfo( SPI_GETWORKAREA, 0, &destop_client_rect, 0 );
	if( FALSE == _ret )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	screen_height = RECT_HEIGHT( destop_client_rect ); 

	popup_height = RECT_HEIGHT( *wnd_rect ); 
	popup_width = RECT_WIDTH( *wnd_rect ); 

	switch( mode )
	{
	case wnd_popup_left:
	case wnd_popup_top:
	case wnd_popup_bottom:
	case wnd_popup_right:
	default:
		popup_rect->left   = destop_client_rect.right - ( popup_width + WND_BORDER_SIZE ); 
		popup_rect->top = destop_client_rect.bottom - ( ( popup_height + POPUP_WND_SPAN + WND_BORDER_SIZE ) * ( space_index + 1 ) ); 
		popup_rect->right = popup_rect->left + popup_width + WND_BORDER_SIZE; 
		popup_rect->bottom  = popup_rect->top + popup_height + WND_BORDER_SIZE; 
		break; 
	}

_return:
	return ret; 
}

LRESULT calc_popup_wnd_rect( ULONG *space_index )
{
	LRESULT ret = ERROR_SUCCESS; 
	CRect destop_client_rect; 
	INT32 i; 
	ULONG index; 

	ASSERT( space_index != NULL ); 

	*space_index = 0xffffffff; 
	if( popup_tip_wnd::max_popup_count == 0 )
	{
		ASSERT( FALSE ); 
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	index = 0xffffffff; 
	lock_cs( popup_space_lock ); 

	for( i = 0; ( ULONG )i < popup_tip_wnd::max_popup_count; i ++ )
	{
		if( all_popup_space[ i ].state == POPUP_SPACE_UNUSE )
		{
			index = i; 
		
			all_popup_space[ i ].state = POPUP_SPACE_USED; 
			break; 
		}
	}
	
	unlock_cs( popup_space_lock ); 

	if( index == 0xffffffff )
	{
		ASSERT( FALSE ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	*space_index = index; 
_return:
	return ret; 
}

popup_tip_wnd::popup_tip_wnd(  ULONG msg_show_time,         
                      ULONG msg_popup_speed, 
					  ULONG popup_unit, 
					  HWND parent )
{
	m_parent = parent;

	this->popup_unit = popup_unit; 
	msg_show_time = msg_show_time;
	creation_delay = msg_popup_speed; 

	m_hCursor = NULL;
	popup_index = INVALID_POPUP_SPACE; 
}

popup_tip_wnd::~popup_tip_wnd()
{
}

LRESULT get_rest_popup_space( RECT *rc_wnd, ULONG *space_index, RECT *rc_out )
{

	LRESULT ret = ERROR_SUCCESS; 
	ULONG wait_ret; 
	ULONG index; 
	RECT popup_rect; 

	ASSERT( rc_wnd != NULL ); 
	ASSERT( rc_out != NULL ); 
	ASSERT( space_index != NULL ); 

	memset( rc_out, 0, sizeof( *rc_out ) ); 
	*space_index = INVALID_POPUP_SPACE; 

	ret = calc_max_popup_wnd_num( rc_wnd, &popup_tip_wnd::max_popup_count ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( popup_tip_wnd::max_popup_count == 0 )
	{
		ASSERT( FALSE ); 
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	for( ; ; )
	{
		if( popup_tip_wnd::popup_count >= popup_tip_wnd::max_popup_count )
		{
			if( popup_tip_wnd::popup_close_notify == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_NOT_READY; 
				goto _return; 
			}

			wait_ret = WaitForSingleObject( popup_tip_wnd::popup_close_notify, INFINITE ); 
			if( wait_ret == WAIT_OBJECT_0 
				|| wait_ret == WAIT_ABANDONED )
			{
				if( popup_tip_wnd::popup_count < popup_tip_wnd::max_popup_count )
				{
					DBG_BP(); 
				}
				else
				{
					DBG_BP(); 
				}

				continue; 
			}

			if( wait_ret == WAIT_FAILED )
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait process failed \n", GetLastError() ) );  

				SAFE_SET_ERROR_CODE( ret ); 
				goto _return; 
			}

		}
		else
		{
			break; 
		}
	}


	ret = calc_popup_wnd_rect( &index ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "can't got the popup space" ); 
		goto _return; 
	}

	ret = _calc_popup_wnd_rect( index, wnd_popup_bottom, rc_wnd, &popup_rect ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "can't got the popup space" ); 
		goto _return; 
	}

	*space_index = index; 
	*rc_out = popup_rect; 

_return:
	return ret; 
}

LRESULT popup_tip_wnd::popup_wnd_from_border( popup_mode mode )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	CRect desttop_client_area; 
	RECT rcWnd; 
	BOOL handled; 

	if( GetHWND() == NULL )
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	if( FALSE == IsWindow( GetHWND() ) )
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	popup_pos = 0; 

	_ret = GetWindowRect( *this, &rcWnd );
	if( _ret == FALSE )
	{
		SAFE_SET_ERROR_CODE( ret ); 
		goto _return; 
	}


	ret = get_rest_popup_space( &rcWnd, &popup_index, &popup_rect ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( popup_index == INVALID_POPUP_SPACE ); 
		goto _return; 
	}

	ASSERT( popup_index != INVALID_POPUP_SPACE ); 

	on_timer( WM_TIMER, IDT_POP_WINDOW_TIMER, wnd_popup_bottom, handled ); 

	_ret = ( INT32 )::SetTimer( GetHWND(), IDT_POP_WINDOW_TIMER, creation_delay, NULL );
	if( _ret == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	popup_count ++; 

	ShowModal(); 

_return: 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		Close(); 
	}

	return ret; 
}


LRESULT popup_tip_wnd::popup_wnd_from_right()
{
    return popup_wnd_from_border( wnd_popup_right ); 
}

LRESULT popup_tip_wnd::popup_wnd_from_top()
{
	return popup_wnd_from_border( wnd_popup_top ); 
}

LRESULT popup_tip_wnd::popup_wnd_from_left()
{
	return popup_wnd_from_border( wnd_popup_left );
}

LRESULT popup_tip_wnd::popup_wnd_from_bottom()
{
	return popup_wnd_from_border( wnd_popup_bottom );
}


LRESULT popup_tip_wnd::popup_msg()
{
	LRESULT ret = ERROR_SUCCESS; 

	INT32 _ret; 
	_ret = PlaySound( MAKEINTRESOURCE( IDR_WAVE_ALERT ), NULL, SND_RESOURCE | SND_ASYNC | SND_NODEFAULT  ); 
	if( _ret == FALSE )
	{
		log_trace( ( MSG_ERROR | DBG_MSG_AND_ERROR_OUT, "play sound error\n" ) ); 
	}

    if( TRUE == check_bottom_popup()) 
    {
        ret = popup_wnd_from_bottom(); 
    }
	else if( TRUE == check_top_popup())
	{
		ret = popup_wnd_from_top();
	}
	else if( TRUE == check_left_popup() )
	{
		ret = popup_wnd_from_left();
	}
	else
	{
		status_bar_pos = STP_RIGHT;
		ret = popup_wnd_from_right();
	}

	return ret; 
}

BOOL popup_tip_wnd::check_left_popup()
{
	UINT32 nAvailableScreenTop;
	UINT32 nAvailableScreenLeft;
	CRect desttop_client_area;  
	SystemParametersInfo( SPI_GETWORKAREA, 0, &desttop_client_area, 0 );
    nAvailableScreenLeft  = desttop_client_area.left;
    nAvailableScreenTop = desttop_client_area.top;

    if( ( nAvailableScreenLeft > 0 ) 
		&& ( nAvailableScreenTop == 0 ) )
    {
        status_bar_pos = STP_LEFT;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL popup_tip_wnd::check_right_popup()
{
    unsigned int nAvailableScreenWidth;
    unsigned int nAvailableScreenHeight;
    unsigned int nActualScreenWidth;
    unsigned int nActualScreenHeight;

    nActualScreenWidth  = ::GetSystemMetrics( SM_CXFULLSCREEN );
    nActualScreenHeight = ::GetSystemMetrics( SM_CYFULLSCREEN );

    CRect desttop_client_area;  
    
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &desttop_client_area, 0);
    nAvailableScreenWidth  = desttop_client_area.GetWidth();
    nAvailableScreenHeight = desttop_client_area.GetHeight();

    if ((nAvailableScreenWidth != nActualScreenWidth) &&
        (nAvailableScreenHeight == nActualScreenHeight))
    {
        status_bar_pos = STP_RIGHT;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL popup_tip_wnd::check_top_popup()
{
    unsigned int nAvailableScreenTop;
    unsigned int nAvailableScreenLeft;
    
    CRect desttop_client_area; 
    
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &desttop_client_area, 0);
    nAvailableScreenLeft  = desttop_client_area.left;
    nAvailableScreenTop = desttop_client_area.top;

    if ((nAvailableScreenLeft == 0) && (nAvailableScreenTop > 0))
    {
        status_bar_pos = STP_TOP;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
BOOL popup_tip_wnd::check_bottom_popup()
{
    UINT32 nAvailableScreenWidth;
    UINT32 nAvailableScreenBottom;
	UINT32 nActualScreenWidth;
    UINT32 nActualScreenBottom; 
	CRect desttop_client_area; 

    nActualScreenWidth  = ::GetSystemMetrics(SM_CXSCREEN);
    nActualScreenBottom = ::GetSystemMetrics(SM_CYSCREEN);

    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &desttop_client_area, 0);
    nAvailableScreenWidth  = desttop_client_area.GetWidth();
    nAvailableScreenBottom = desttop_client_area.bottom;

    if ((nAvailableScreenWidth == nActualScreenWidth) &&
        (nAvailableScreenBottom < nActualScreenBottom))
    {
        status_bar_pos = STP_BOTTOM;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

VOID CALLBACK popup_timer(
						__in  HWND hwnd,
						__in  UINT uMsg,
						__in  UINT_PTR idEvent,
						__in  DWORD dwTime
						)
{

}

#define SET_SHOW_WINDOW_TIMER() SetTimer( GetHWND(),  IDT_SHOW_WINDOW_TIMER, msg_show_time, NULL );  \
	SetTimer( GetHWND(),  IDT_SHOW_WINDOW_ELAPSE_TIMER, msg_show_notify_time, NULL ); \
	time_elapsed = 0; 

#define STOP_SHOW_WINDOW_TIMER() KillTimer( GetHWND(), IDT_SHOW_WINDOW_TIMER ); \
	KillTimer( GetHWND(), IDT_SHOW_WINDOW_ELAPSE_TIMER ); \
	time_elapsed = 0; 

LRESULT popup_tip_wnd::on_timer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT ret = ERROR_SUCCESS; 

	bHandled = TRUE; 

	switch( wParam )
	{
	case IDT_POP_WINDOW_TIMER: 
		{
			switch (status_bar_pos)
			{
			case STP_BOTTOM:
			case STP_RIGHT:
				{
					popup_pos += popup_unit;
					if( popup_pos > RECT_HEIGHT( popup_rect ) )
					{
						KillTimer(GetHWND(),  IDT_POP_WINDOW_TIMER);
						SET_SHOW_WINDOW_TIMER(); 
					}
					else
					{
						SetWindowPos( GetHWND(), 
							HWND_TOPMOST,
							popup_rect.left,
							popup_rect.bottom - popup_pos, 
							RECT_WIDTH( popup_rect ),
							popup_pos,
							SWP_SHOWWINDOW
							);
					}
				}
				break;

			case STP_TOP:
				{
					popup_pos += popup_unit;
					if( popup_pos > RECT_HEIGHT( popup_rect ) )
					{
						KillTimer( GetHWND(), IDT_POP_WINDOW_TIMER);
						
						SET_SHOW_WINDOW_TIMER(); 
					}
					else
					{
						SetWindowPos(GetHWND(), 
							HWND_TOPMOST,
							popup_rect.left,
							popup_rect.top, 
							RECT_WIDTH( popup_rect ),
							popup_pos,
							SWP_SHOWWINDOW
							);
					}
				}
				break;

			case STP_LEFT:
				{
					popup_pos += popup_unit;
					
					if( popup_pos > RECT_HEIGHT( popup_rect ) )
					{
						KillTimer( GetHWND(), IDT_POP_WINDOW_TIMER);
						SET_SHOW_WINDOW_TIMER(); 
					}
					else
					{
						SetWindowPos(
							GetHWND(), 
							HWND_TOPMOST,
							popup_rect.left,
							popup_rect.bottom - popup_pos, 
							RECT_WIDTH( popup_rect ),
							popup_pos,
							SWP_SHOWWINDOW
							);
					}
				}
				break;
			}
		}
		break;

	case IDT_SHOW_WINDOW_TIMER:
		{
			STOP_SHOW_WINDOW_TIMER(); 

			SetTimer( GetHWND(), IDT_COLLAPSE_WINDOW_TIMER, creation_delay, NULL );
		}
		break;
	case IDT_SHOW_WINDOW_ELAPSE_TIMER:
		{
			time_elapsed += 1000; 
			update_window_elapsed_time(); 
		}
		break; 
	case IDT_COLLAPSE_WINDOW_TIMER:
		{
			switch( status_bar_pos )
			{
			case STP_BOTTOM:
			case STP_RIGHT:
				{
					popup_pos -= popup_unit;
					if (popup_pos <= 0) 
					{
						KillTimer( GetHWND(), IDT_COLLAPSE_WINDOW_TIMER );
						popup_pos = 0;

						Close(); 
					}
					else
					{
						SetWindowPos(
							GetHWND(), 
							HWND_TOPMOST,
							popup_rect.left,
							popup_rect.bottom - popup_pos, 
							RECT_WIDTH( popup_rect ),
							popup_pos,
							SWP_SHOWWINDOW
							);      
					}
				}
				break;

			case STP_TOP:
				{
					popup_pos -= popup_unit;
					if (popup_pos <= 0)
					{
						KillTimer( GetHWND(), IDT_COLLAPSE_WINDOW_TIMER);
						popup_pos = 0;

						Close(); 
					}
					else
					{
						SetWindowPos(
							GetHWND(), 
							HWND_TOPMOST,
							popup_rect.left,
							popup_rect.top, 
							RECT_WIDTH( popup_rect ),
							popup_pos,
							SWP_SHOWWINDOW
							);
					}
				}
				break;

			case STP_LEFT:
				{
					popup_pos -= popup_unit;
					if (popup_pos <= 0)
					{
						KillTimer(GetHWND(), IDT_COLLAPSE_WINDOW_TIMER);
						popup_pos = 0;

						Close(); 
					}
					else
					{
						SetWindowPos(
							GetHWND(), 
							HWND_TOPMOST,
							popup_rect.left,
							popup_rect.bottom - popup_pos, 
							RECT_WIDTH( popup_rect ),
							popup_pos,
							SWP_SHOWWINDOW
							);
					}
				}
				break;
			}
		}
		break;
	}

	return ret; 
}

LRESULT popup_tip_wnd::on_mouse_hover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    return 0;
}

