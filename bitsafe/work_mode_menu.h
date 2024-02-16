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

#pragma once

class work_mode_menu : public CWindowWnd, public INotifyUI
{
public:
	work_mode_menu() : m_pOwner(NULL), can_close( FALSE )
	{

	}; 

	~work_mode_menu()
	{
		if( IsWindow( m_hWnd ) == TRUE )
		{
			Close(); 
		}
	}; 

	void Init(CControlUI* pOwner, POINT pt) {
		if( pOwner == NULL ) 
		{
			ASSERT( FALSE ); 
			goto _return;
		}

		if( IsWindow( m_hWnd ) == TRUE )
		{
			SendMessage( WM_CLOSE ); 
			//m_hWnd = NULL; 
			//ShowWindow(); 
			//SetFocus( m_hWnd ); 
			//Close(); 
			//goto _return; 
		}

		m_pOwner = pOwner;
		m_ptPos = pt; 
		hWndParent = pOwner->GetManager()->GetPaintWindow(); 
		Create(GetDesktopWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, 100, 100, 300, 300 ); 
		hWndParent = m_hWnd;
		while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent( hWndParent ); 
		BOOL _ret = SetForegroundWindow( m_hWnd ); 
		if( _ret == FALSE )
		{
			ASSERT( FALSE ); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "set main window to foreground failed\n") ); 
		}
		::ShowWindow(m_hWnd, SW_SHOW);
		::SendMessage(m_hWnd, WM_NCACTIVATE, TRUE, 0L); 
		//SetFocus( m_hWnd ); 
_return:
		return; 
	}

	void AdjustPostion() {
		CRect rcWnd;
		//GetWindowRect(m_hWnd, &rcWnd);
		int nWidth = m_pm.GetClientSize().cx; //rcWnd.GetWidth();
		int nHeight = m_pm.GetClientSize().cy;  //rcWnd.GetHeight();
		rcWnd.left = m_ptPos.x;
		rcWnd.top = m_ptPos.y;
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
		::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(), SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	LPCTSTR GetWindowClassName() const { return _T("TTaryIconMenu"); };
	void OnFinalMessage(HWND /*hWnd*/) 
	{
		m_pm.uninit(); 
		//m_pm.RemoveNotifier( this ); 
	};

	void Notify( TNotifyUI& msg )
	{
		if( msg.sType == _T( "itemselect") )
		{
			can_close = TRUE; 	
			Close(); 
		}
		else if( msg.sType == _T("itemclick") ) 
		{
			//WPARAM param; 
			//if( msg.pSender->GetName() == _T( "menu_login" ) )
			//{
			//	param = MENU_ITEM_LOGIN; 
			//}
			//else if( msg.pSender->GetName() == _T( "menu_exit" ) )
			//{
			//	param = MENU_ITEM_EXIT; 
			//}
			//else if( msg.pSender->GetName() == _T( "menu_work_mode" ) )
			//{
			//	param = MENU_ITEM_CHANGE_PWD; 
			//}

			can_close = FALSE; 
			ShowWindow( false, false ); 

			//::SendMessage( m_pOwner->GetManager()->GetPaintWindow(), WM_COMMAND, param, ( LPARAM )0 ); 
			if( m_pOwner ) m_pOwner->GetManager()->SendNotify(m_pOwner, msg.pSender->GetName(), 0, 0, true);
		}
		else if( msg.sType == _T( "timer" ) )
		{
			if( msg.pSender->GetName() == _T( "work_mode_menu" ) )
			{
				if( IsWindow( m_hWnd ) == TRUE )
				{
					HWND tmp_wnd; 
					tmp_wnd = SetActiveWindow( m_hWnd ); 
					//log_trace( ( MSG_INFO, "prev activated window is 0x%0.8x\n", tmp_wnd ) ); 
					if( tmp_wnd == NULL )
					{
						log_trace( ( MSG_INFO, "set active window failed return 0x%0.8x\n", GetLastError() ) ); 
					}

					if( tmp_wnd != m_hWnd )
					{
						//DBG_BP(); 
						log_trace( ( MSG_INFO, "prev other active windows is 0x%0.8x\n", tmp_wnd ) ); 
					}

					tmp_wnd = SetFocus( m_hWnd ); 
					//log_trace( ( MSG_INFO, "prev focused window is 0x%0.8x\n", tmp_wnd ) ); 
					if( tmp_wnd == NULL )
					{
						log_trace( ( MSG_INFO, "set focus window failed return 0x%0.8x\n", GetLastError() ) ); 
					}
					if( tmp_wnd != m_hWnd )
					{
						//DBG_BP(); 
						log_trace( ( MSG_INFO, "prev other active windows is 0x%0.8x\n", tmp_wnd ) ); 
					}
				}
			}
		}
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pm.Init(m_hWnd);
		m_pm.init(); 

		can_close = TRUE; 
		CDialogBuilder builder;
		CControlUI* pRoot = builder.Create(_T("work_mode_menu.xml"), (UINT)0, NULL, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");

		m_pm.AttachDialog( pRoot );
		m_pm.AddNotifier( this );
		m_pm.SetRoundCorner( 3, 3 );

		m_pm.SetTimer( pRoot, 101, 100 ); 

		set_ui_style( &m_pm, NO_ALPHA ); 

		AdjustPostion();
		return 0;
	}

	
	LRESULT on_active(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		log_trace( ( MSG_INFO, "active state is 0x%0.8x, minimized %d, windows handle is 0x%0.8x, host window is 0x%0.8x, main windows is 0x%0.8x \n", LOWORD( wParam ), HIWORD( wParam ), lParam, m_hWnd, hWndParent ) ); 

		if( m_hWnd == (HWND) lParam ) 
		{
			ASSERT( HIWORD( wParam ) != 0 ); 

			if( LOWORD( wParam ) == WA_INACTIVE )
			{

				//ShowWindow( false, false ); 
				if( can_close == TRUE )
				{
					Close(); 
				} 
			}
		}

		log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

		return 0;
	}


	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		log_trace( ( MSG_INFO, "focus windows handle is 0x%0.8x, host window is 0x%0.8x, main windows is 0x%0.8x \n", LOWORD( wParam ), HIWORD( wParam ), lParam, m_hWnd, hWndParent ) ); 

		if( m_hWnd != (HWND) wParam ) 
		{
			//ShowWindow( false, false ); 
			if( can_close == TRUE )
			{
				Close(); 
			}; 
		}

		log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

		return 0;
	}

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if( wParam == VK_ESCAPE ) 
		{
			Close(); 
		}
		return 0;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
			CRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}

		bHandled = FALSE; 
		goto _return; 

_return:
		return 0;
	}

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		//if( uMsg == WM_CLOSE )
		//{
		//	DBG_BP(); 
		//}

		switch( uMsg ) {
case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
case WM_KILLFOCUS:     lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
case WM_ACTIVATE:	lRes = on_active(uMsg, wParam, lParam, bHandled); break;	
case WM_KEYDOWN:       lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
case WM_MOUSEWHEEL:    break;
case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
//case WM_TIMER:			lRes = OnTimer( uMsg, wParam, lParam, bHandled ) break; 
default:
	bHandled = FALSE;
		}
		if( bHandled ) return lRes;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

public:
	CPaintManagerUI m_pm;
	CControlUI* m_pOwner;
	POINT m_ptPos; 
	INT32 can_close; 
	HWND hWndParent; 
};