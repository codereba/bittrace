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

#ifndef __PROCESS_PROPERTY_DLG_H__
#define __PROCESS_PROPERTY_DLG_H__

#include "ui_config.h"
class process_properies_dlg : public CWindowWnd, public INotifyUI
{
public:
	process_properies_dlg() 
	{
		event = NULL; 
		mouse_move_on = FALSE; 
		m_pOwner = NULL;
	}

	~process_properies_dlg()
	{
		if( m_hWnd != NULL )
		{
			Close(); 
		}
	}

	LPCTSTR GetWindowClassName() const 
	{ 
		return _T("PROCESS_PROPERIES_DLG"); 
	};

	UINT GetClassStyle() const
	{ 
		return CS_DBLCLKS; 
	};

	void OnFinalMessage(HWND /*hWnd*/) 
	{ 
	} 

	void Init()
	{
	}

	void AdjustPostion()
	{
		CRect rcWnd;
		GetWindowRect(m_hWnd, &rcWnd);
		int nWidth = rcWnd.GetWidth();
		int nHeight = rcWnd.GetHeight();
		
		rcWnd.left = m_ptPos.x - 30;
		rcWnd.top = m_ptPos.y - 30;

		rcWnd.right = rcWnd.left + nWidth;
		rcWnd.bottom = rcWnd.top + nHeight;
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CRect rcWork = oMonitor.rcWork;

		if( rcWnd.bottom > rcWork.bottom ) {
			if( nHeight >= rcWork.GetHeight() ) {
				rcWnd.top = 0;
				rcWnd.bottom = nHeight;
			}
			else {
				rcWnd.bottom = rcWork.bottom;
				rcWnd.top = rcWnd.bottom - nHeight;
			}
		}
		if( rcWnd.right > rcWork.right ) {
			if( nWidth >= rcWork.GetWidth() ) {
				rcWnd.left = 0;
				rcWnd.right = nWidth;
			}
			else {
				rcWnd.right = rcWork.right;
				rcWnd.left = rcWnd.right - nWidth;
			}
		}
		::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	LRESULT init_ui()
	{
		LRESULT ret; 
		ret = load_process_information(); 
	
		AdjustPostion(); 
		return ret; 
	}

	void OnPrepare(TNotifyUI& msg) 
	{

	}

	void Notify(TNotifyUI& msg)
	{
		if( msg.sType == _T("windowinit") ) 
		{
			OnPrepare( msg ); 
		}
		else if( msg.sType == _T("click") ) 
		{
			//         if( msg.pSender == m_pCloseBtn ) 
			//         {
			//	ret_status = CANCEL_STATE; 
			//	Hide(); 
			//             return; 
			//         }
			//else
			{
				CStdString name = msg.pSender->GetName(); 

				if( name == _T( "close_btn" ) )
				{
					Hide(); 
				}
				else if( name == _T( "no_btn" ) )
				{
					Hide(); 
				}
			}
		}
		else if(msg.sType==_T("setfocus"))
		{
		}
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_pm.Init(m_hWnd);
		//m_pm.SetTransparent(100);
		CDialogBuilder builder;

		CControlUI* pRoot = builder.Create( _T( "process_properties_dlg.xml" ), ( UINT )0, NULL, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

		set_ui_style( &m_pm, NO_ALPHA ); 
		Init(); 
		return 0;
	}

	void Hide()
	{
		if( TRUE == IsWindowVisible( GetHWND() ) )
		{
			::ShowWindow( m_hWnd, SW_HIDE );
		}

		if( TRUE == IsWindowEnabled( GetHWND() ) )
		{
			::EnableWindow( GetHWND(), FALSE ); 
		}

		event = NULL; 
		mouse_move_on = FALSE; 
		dbg_print( MSG_IMPORTANT, "hide process properties\n" ); 

	}

	LRESULT OnMouseLeave( UINT32 uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE; 

		//if( mouse_move_on == TRUE )
		//{
		//	//if( GetHWND() == GetCapture() )
		//	//{
		//	//	ReleaseCapture(); 
		//	//}

		//	if( TRUE == IsWindowVisible( GetHWND() ) )
		//	{
		//		::ShowWindow( m_hWnd, SW_HIDE );
		//	}

		//	if( TRUE == IsWindowEnabled( GetHWND() ) )
		//	{
		//		::EnableWindow( GetHWND(), FALSE ); 
		//	}

		//	//bHandled = TRUE; 
		//	event = NULL; 
		//	dbg_print( MSG_IMPORTANT, "hide process properties\n" ); 

		//	mouse_move_on = FALSE; 
		//}

		return 0; 
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 Ret; 
		POINT Pos; 
		RECT WindowRect; 
		RECT DeskWindowRect; 
		//BOOLEAN CursorOnTooltip = FALSE; 

		do 
		{
			bHandled = FALSE; 

			Pos.x = GET_X_LPARAM( lParam ); 
			Pos.y = GET_Y_LPARAM( lParam ); 

			Ret = ClientToScreen( GetHWND(), &Pos ); 
			if( Ret == FALSE )
			{
				mouse_move_on = FALSE; 
				break; 
			}

			do 
			{
				HWND DesktopWindow; 

				DesktopWindow = GetDesktopWindow(); 

				if( DesktopWindow == NULL )
				{
					break; 
				}

				Ret = GetWindowRect( DesktopWindow, &DeskWindowRect ); 

				if( Ret == FALSE )
				{
					mouse_move_on = FALSE; 
					break; 
				}

#define STOP_CAPTURE_SPACE_MARGIN 8
				if( ( Pos.x >= DeskWindowRect.right - STOP_CAPTURE_SPACE_MARGIN ) 
					|| ( Pos.x <= DeskWindowRect.left + STOP_CAPTURE_SPACE_MARGIN ) 
					|| ( Pos.y <= DeskWindowRect.top + STOP_CAPTURE_SPACE_MARGIN ) 
					|| ( Pos.y >= DeskWindowRect.bottom - STOP_CAPTURE_SPACE_MARGIN ) )
				{
					mouse_move_on = FALSE; 
					break; 
				}

			}while( FALSE ); 

			Ret = GetWindowRect( GetHWND(), &WindowRect ); 

			if( Ret == FALSE )
			{
				mouse_move_on = FALSE; 
				break; 
			}

			if( PtInRect( &WindowRect, Pos ) == TRUE )
			{
				if( FALSE == mouse_move_on )
				{
					mouse_move_on = TRUE; 

					// Register the tracking of Mouse entering and leaveing the window
					// for WM_MOUSEHOVER and WM_MOUSELEAVE
					TRACKMOUSEEVENT t_MouseEvent;
					t_MouseEvent.cbSize      = sizeof(TRACKMOUSEEVENT);
					t_MouseEvent.dwFlags     = TME_LEAVE | TME_HOVER;
					t_MouseEvent.hwndTrack   = m_hWnd;
					t_MouseEvent.dwHoverTime = 1;

					Ret = _TrackMouseEvent( &t_MouseEvent ); 
					if( FALSE == Ret )
					{

					}

					//ReleaseCapture(); 
				}

				//CursorOnTooltip = TRUE; 
			}
			else
			{
				mouse_move_on = FALSE; 
			}
		}while( FALSE ); 

		if( mouse_move_on == FALSE )
		{
			//if( GetHWND() == GetCapture() )
			//{
			//	ReleaseCapture(); 
			//}

			if( TRUE == IsWindowVisible( GetHWND() ) )
			{
				::ShowWindow( m_hWnd, SW_HIDE );
			}

			if( TRUE == IsWindowEnabled( GetHWND() ) )
			{
				::EnableWindow( GetHWND(), FALSE ); 
			}

			event = NULL; 
			//bHandled = TRUE; 
			dbg_print( MSG_IMPORTANT, "hide process properties\n" ); 
		}

		return 0; 
	}

	LRESULT _Init( CControlUI* pOwner, POINT pt, PVOID context )
	{
		LRESULT ret = ERROR_SUCCESS; 
		HWND paint_wnd; 

		do 
		{
			if( pOwner == NULL ) 
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			set_event( context ); 

			m_pOwner = pOwner;
			m_ptPos = pt;

			if( m_hWnd == NULL )
			{
				HWND hwnd; 
				paint_wnd = pOwner->GetManager()->GetPaintWindow(); 
				
				hwnd = Create( paint_wnd, NULL, WS_POPUP, WS_EX_TOOLWINDOW, pt.x, pt.y, 400, 500, NULL ); 
				if( NULL == hwnd )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			}

			if( FALSE == IsWindowVisible( GetHWND() ) )
			{
				::ShowWindow( GetHWND(), SW_SHOW );
			}

			if( FALSE == IsWindowEnabled( GetHWND() ) )
			{
				::EnableWindow( GetHWND(), TRUE ); 
			}

			init_ui(); 

			HWND hWndParent = m_hWnd; 
			while( ::GetParent( hWndParent ) != NULL ) 
			{
				hWndParent = ::GetParent( hWndParent );
			}

			//CenterWindow(); 

			//::SendMessage( hWndParent, WM_NCACTIVATE, TRUE, 0L ); 
			//SetCapture( GetHWND() ); 
			//SetFocus( m_hWnd ); 

			dbg_print( MSG_IMPORTANT, "show process properties\n" ); 

		} while ( FALSE ); 

		return ret; 

	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if( ::IsIconic(*this) ) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		if( !::IsZoomed(*this) ) {
			RECT rcSizeBox = m_pm.GetSizeBox();
			if( pt.y < rcClient.top + rcSizeBox.top ) {
				if( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
				if( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
				return HTTOP;
			}
			else if( pt.y > rcClient.bottom - rcSizeBox.bottom ) {
				if( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
				if( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
				return HTBOTTOM;
			}
			if( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
			if( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
		}

		RECT rcCaption = m_pm.GetCaptionRect();
		if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
				if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
					_tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
					_tcscmp(pControl->GetClass(), _T("TextUI")) != 0 )
					return HTCAPTION;
		}

		return HTCLIENT;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
			CRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			RECT rc = { rcWnd.left, rcWnd.top + szRoundCorner.cx, rcWnd.right, rcWnd.bottom };
			HRGN hRgn1 = ::CreateRectRgnIndirect( &rc );
			HRGN hRgn2 = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom - szRoundCorner.cx, szRoundCorner.cx, szRoundCorner.cy);
			::CombineRgn( hRgn1, hRgn1, hRgn2, RGN_OR );
			::SetWindowRgn(*this, hRgn1, TRUE);
			::DeleteObject(hRgn1);
			::DeleteObject(hRgn2);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CRect rcWork = oMonitor.rcWork;
		rcWork.Offset(-rcWork.left, -rcWork.top);

		LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
		lpMMI->ptMaxPosition.x	= rcWork.left;
		lpMMI->ptMaxPosition.y	= rcWork.top;
		lpMMI->ptMaxSize.x		= rcWork.right;
		lpMMI->ptMaxSize.y		= rcWork.bottom;

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
		if( wParam == SC_CLOSE ) {
			::PostQuitMessage(0L);
			bHandled = TRUE;
			return 0;
		}
		BOOL bZoomed = ::IsZoomed(*this);
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		if( ::IsZoomed(*this) != bZoomed ) {
			if( !bZoomed ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("max_btn")));
				if( pControl ) pControl->SetVisible(false);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("nor_btn")));
				if( pControl ) pControl->SetVisible(true);
			}
			else {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("max_btn")));
				if( pControl ) pControl->SetVisible(true);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("nor_btn")));
				if( pControl ) pControl->SetVisible(false);
			}
		}
		return lRes;
	}

	//void AdjustPostion()
	//{
	//	CRect rcWnd;
	//	GetWindowRect(m_hWnd, &rcWnd);
	//	int nWidth = rcWnd.GetWidth();
	//	int nHeight = rcWnd.GetHeight();
	//	rcWnd.left = m_ptPos.x;
	//	rcWnd.top = m_ptPos.y;
	//	rcWnd.right = rcWnd.left + nWidth;
	//	rcWnd.bottom = rcWnd.top + nHeight;
	//	MONITORINFO oMonitor = {};
	//	oMonitor.cbSize = sizeof(oMonitor);
	//	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	//	CRect rcWork = oMonitor.rcWork;

	//	if( rcWnd.bottom > rcWork.bottom ) {
	//		if( nHeight >= rcWork.GetHeight() ) {
	//			rcWnd.top = 0;
	//			rcWnd.bottom = nHeight;
	//		}
	//		else {
	//			rcWnd.bottom = rcWork.bottom;
	//			rcWnd.top = rcWnd.bottom - nHeight;
	//		}
	//	}
	//	if( rcWnd.right > rcWork.right ) {
	//		if( nWidth >= rcWork.GetWidth() ) {
	//			rcWnd.left = 0;
	//			rcWnd.right = nWidth;
	//		}
	//		else {
	//			rcWnd.right = rcWork.right;
	//			rcWnd.left = rcWnd.right - nWidth;
	//		}
	//	}
	//	::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, wnd_size.cx, wnd_size.cy/*rcWnd.GetHeight()*/, SWP_NOZORDER | SWP_NOACTIVATE);
	//}

	LRESULT set_event( PVOID event ); 
	LRESULT load_process_information(); 

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch( uMsg ) {
case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
case WM_MOUSEMOVE:		lRes = OnMouseMove( uMsg, wParam, lParam, bHandled ); break; 
case WM_MOUSELEAVE:     lRes = OnMouseLeave( uMsg, wParam, lParam, bHandled ); break; 
default:
	bHandled = FALSE;
		}
		if( bHandled ) return lRes;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}
public:
	CPaintManagerUI m_pm; 

protected: 
	CControlUI* m_pOwner;
	POINT m_ptPos; 
	BOOLEAN mouse_move_on; 
	PVOID event; 
};

#endif //__PROCESS_PROPERTY_DLG_H__