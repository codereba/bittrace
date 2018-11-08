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

 
 #ifndef __INSTALL_DLG_H__
#define __INSTALL_DLG_H__

#include "ui_config.h"
#include "wnd_state.h"

class install_dlg : public CWindowWnd, public wnd_state, public INotifyUI
{
public:
	install_dlg() 
	{
	};

	LPCTSTR GetWindowClassName() const 
	{ 
		return _T("TMsgBox"); 
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
		//      m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		//ASSERT( m_pCloseBtn != NULL ); 
		ret_status = CANCEL_STATE; 

		set_ctrl_text( &m_pm, _T( "yes_btn" ), TEXT_COMMON_BUTTON_YES ); 
		set_ctrl_text( &m_pm, _T( "no_btn" ), TEXT_COMMON_BUTTON_NO ); 
	}

	void OnPrepare(TNotifyUI& msg) 
	{

	}

	void Notify(TNotifyUI& msg)
	{
		if( msg.sType == _T("windowinit") ) 
			OnPrepare(msg);
		else if( msg.sType == _T("click") ) 
		{
			//         if( msg.pSender == m_pCloseBtn ) 
			//         {
			//	ret_status = CANCEL_STATE; 
			//	Close(); 
			//             return; 
			//         }
			//else
			{
				CStdString name = msg.pSender->GetName(); 

				if( name == _T( "yes_btn" ) )
				{
					ret_status = OK_STATE; 
					Close(); 
				}
				else if( name == _T( "no_btn" ) )
				{
					ret_status = CANCEL_STATE; 
					Close(); 
				}
				else if( name == _T( "browser_btn" ) )
				{
					open_file_dlg( TRUE ); 
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
		CControlUI* pRoot = builder.Create(_T("install_dlg.xml"), (UINT)0, NULL, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

		set_ui_style( &m_pm, NO_ALPHA ); 
		Init();
		return 0;
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
default:
	bHandled = FALSE;
		}
		if( bHandled ) return lRes;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}
public:
	CPaintManagerUI m_pm;

};

INLINE LRESULT show_install_dlg( HWND parent, LPCTSTR title, UINT32 main_icon = NULL )
{
	LRESULT ret = ERROR_SUCCESS; 
	install_dlg dialog; 
	//CControlUI *msg_txt; 

	//if( ret_state != NULL )
	//{
	//	*ret_state = CANCEL_STATE; 
	//}

	if( title == NULL )
	{
		title = _T( "" ); 
	}

	if( NULL == dialog.Create( parent, title, UI_WNDSTYLE_DIALOG | WS_EX_TOPMOST, 0L, 0, 0, 1024, 738) )
	{
		goto _return; 
	}

	if( main_icon != NULL )
	{
		dialog.SetIcon( main_icon ); 
	}

	//msg_txt = msg_tip.m_pm.FindControl( _T( "msg_txt" ) ); 

	//ASSERT( msg_txt != NULL ); 
	//msg_txt->SetText( msg ); 

	dialog.CenterWindow();
	BringWindowToTop( dialog.GetHWND() ); 

	//SwitchDesktop
	dialog.ShowModal(); 

	//if( ret_state != NULL )
	//{
	//	*ret_state = dialog.get_dlg_ret_statue(); 
	//}

_return:
	return ret; 
} 

#endif //__INSTALL_DLG_H__