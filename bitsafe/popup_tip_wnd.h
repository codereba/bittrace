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

#ifndef __POPUP_TIP_WND_H__
#define __POPUP_TIP_WND_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "acl_define.h"


const INT32 IDT_POP_WINDOW_TIMER       = 100;
const INT32 IDT_COLLAPSE_WINDOW_TIMER  = 101;
const INT32 IDT_SHOW_WINDOW_TIMER      = 102;
const INT32 IDT_SHOW_WINDOW_ELAPSE_TIMER = 103; 

const INT32 STP_BOTTOM = 200;
const INT32 STP_TOP    = 201;
const INT32 STP_RIGHT  = 202;
const INT32 STP_LEFT   = 203;

typedef enum _popup_mode
{
	wnd_popup_left, 
	wnd_popup_right, 
	wnd_popup_top, 
	wnd_popup_bottom
} popup_mode; 

#define POPUP_WND_SPAN 1
#define WND_BORDER_SIZE 2

#define EVENT_POPUP_SHOW_TIME 15000 
#define POPUP_UPDATE_TIME 1
#define POPUP_UNIT 12

#define INVALID_POPUP_SPACE 0xffffffff

LRESULT _calc_popup_wnd_rect( ULONG space_index, popup_mode mode, RECT *wnd_rect, RECT *popup_rect ); 
LRESULT uninit_popup_space(); 
LRESULT init_popup_space( RECT *popup_rect, popup_mode mode ); 
LRESULT calc_max_popup_wnd_num( RECT *popup_rect, ULONG *max_popup ); 
LRESULT calc_popup_wnd_rect( ULONG popup_index, RECT *wnd_rect, RECT *popup_rect ); 
LRESULT release_popup_space( ULONG index ); 

class popup_tip_wnd : public CWindowWnd
{
public:

	popup_tip_wnd() 
	{
		msg_show_time = EVENT_POPUP_SHOW_TIME; 
		creation_delay = POPUP_UPDATE_TIME; 
		popup_unit = POPUP_UNIT; 
		popup_index = INVALID_POPUP_SPACE; 
	}

	~popup_tip_wnd(); 

	popup_tip_wnd(  ULONG MsgTimeOut,         
		ULONG MsgWndCreationDelay, 
		ULONG popup_unit, 
		HWND parent = NULL ); 

	LPCTSTR GetWindowClassName() const 
	{ 
		return _T("PopupTip"); 
	};

	UINT GetClassStyle() const
	{ 
		return CS_DBLCLKS; 
	};

	void OnFinalMessage(HWND /*hWnd*/) 
	{ 
	};

	void Init() 
	{
		time_elapsed = 0; 
	}

	void OnPrepare(TNotifyUI& msg)
	{

	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CloseHandle( m_hCursor ); 
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{

		return 0;
	}

	LRESULT on_set_cursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = TRUE; 

		SetCursor( m_hCursor );  // Set cursor to HAND type when mouse is over window

		return TRUE;
	}

	LRESULT on_mouse_move(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		TRACKMOUSEEVENT t_MouseEvent;
		t_MouseEvent.cbSize      = sizeof(TRACKMOUSEEVENT);
		t_MouseEvent.dwFlags     = TME_LEAVE | TME_HOVER;
		t_MouseEvent.hwndTrack   = m_hWnd;
		t_MouseEvent.dwHoverTime = 1;

		_TrackMouseEvent( &t_MouseEvent );
	
		return ERROR_SUCCESS; 
	}

	LRESULT on_mouse_leave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 

		return ret; 
	}

	//virtual 
	LRESULT on_timer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

public:

    LRESULT popup_msg();
    
protected:
    LRESULT on_mouse_hover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
 
public:
	ULONG creation_delay; 
	ULONG popup_unit; 
	ULONG popup_index; 

protected:

	CButtonUI *m_pCloseBtn; 

	BOOL check_bottom_popup();
	BOOL check_top_popup();  
	BOOL check_right_popup(); 
	BOOL check_left_popup(); 
	
	LRESULT popup_wnd_from_border( popup_mode mode ); 
	LRESULT popup_wnd_from_bottom();
	LRESULT popup_wnd_from_top(); 
	LRESULT popup_wnd_from_right(); 
	LRESULT popup_wnd_from_left(); 
	
	virtual LRESULT update_window_elapsed_time() = 0; 

	ULONG msg_show_time;

	RECT popup_rect; 
    
	ULONG popup_pos; 

    ULONG status_bar_pos; 
	ULONG time_elapsed; 

    HWND m_parent; 


    HCURSOR m_hCursor;

public:
	static ULONG popup_count; 
	static ULONG max_popup_count; 
	static ULONG msg_show_notify_time; 
	static HANDLE popup_close_notify; 
};

#endif // __POPUP_TIP_WND_H__
