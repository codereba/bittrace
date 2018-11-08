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
 
 #pragma once
//#include "menu_ui.h"
#include "md5.h"
#include "msg_box.h"
#include "user_manage.h"

class chg_mode_dlg : public CWindowWnd, public INotifyUI 
{
public:
    chg_mode_dlg( action_ui_notify *notifier = NULL ) 
	{
		ui_notify = notifier; 
    };

    LPCTSTR GetWindowClassName() const 
    { 
		return _T("TChangeWorkModeDlg"); 
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
		LRESULT ret = ERROR_SUCCESS; 
		INT32 cur_sel; 
		user_group cur_user_group; 
		work_mode cur_work_mode = WORK_ACL_MODE; 
		CComboUI *combo; 
		LPCTSTR tmp_text; 
		CControlUI *ctrl; 

        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
		
		ret = get_cur_work_mode( &cur_work_mode, NULL ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "!!get current work mode failed\n") ); 
		}

		set_ctrl_text( &m_pm, _T( "title_label" ), TEXT_WORK_MODE_TITLE ); 
		set_ctrl_text( &m_pm, _T( "ok_btn" ), TEXT_COMMON_BUTTON_CONFIRM ); 
		set_ctrl_text( &m_pm, _T( "cancel_btn" ), TEXT_COMMON_BUTTON_CANCEL ); 
		set_ctrl_text( &m_pm, _T( "mode_tip_label" ), TEXT_WORK_MODE_LABLE ); 

		combo = ( CComboUI* )m_pm.FindControl( _T( "mode_combo" ) ); 
		ASSERT( combo != NULL ); 

		tmp_text = get_string_by_id( TEXT_WORK_MODE_CHANGE_TOOLTIP ); 
		if( tmp_text != NULL )
		{
			combo->SetToolTip( tmp_text ); 
		}

		ctrl = combo->GetItemAt( 0 ); 
		ASSERT( ctrl != NULL ); 

		tmp_text = get_string_by_id( TEXT_WORK_MODE_FREE ); 
		if( tmp_text != NULL )
		{
			ctrl->SetText( tmp_text ); 
		}

		ctrl = combo->GetItemAt( 1 ); 
		ASSERT( ctrl != NULL ); 
		tmp_text = get_string_by_id( TEXT_WORK_MODE_ACL_CTRL ); 
		if( tmp_text != NULL )
		{
			ctrl->SetText( tmp_text ); 
		}


		ctrl = combo->GetItemAt( 2 ); 
		ASSERT( ctrl != NULL ); 
		tmp_text = get_string_by_id( TEXT_WORK_MODE_BLOCK_ALL ); 
		if( tmp_text != NULL )
		{
			ctrl->SetText( tmp_text ); 
		}

		if( cur_work_mode == WORK_FREE_MODE )
		{
			combo->SelectListItem( 0 ); 
		}
		else if( cur_work_mode == WORK_ACL_MODE )
		{
			combo->SelectListItem( 1 ); 			
		}
		else if( cur_work_mode == WORK_BLOCK_MODE )
		{
			combo->SelectListItem( 2 ); 
		}
		else
		{
			ASSERT( FALSE && "invalid work mode " ); 
		}

_return:
		return; 
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
            if( msg.pSender == m_pCloseBtn ) 
            {
				Close(); 
                return; 
            }
			else
			{
				LPCTSTR tmp_text; 
				work_mode cur_work_mode = WORK_ACL_MODE; 
				INT32 cur_sel; 
				LRESULT ret = ERROR_SUCCESS; 
				CStdString name = msg.pSender->GetName(); 
				if( name == _T( "ok_btn" ) )
				{
					CComboUI *combo; 

					combo = ( CComboUI* )m_pm.FindControl( _T( "mode_combo" ) ); 
					ASSERT( combo != NULL ); 

					cur_sel = combo->GetCurSel(); 
					if( cur_sel == 0 )
					{
						cur_work_mode = WORK_FREE_MODE; 
					}
					else if( cur_sel == 1 )
					{
						cur_work_mode = WORK_ACL_MODE; 
					}
					else if( cur_sel == 2 )
					{
						cur_work_mode = WORK_BLOCK_MODE; 
					}
					else
					{
						ASSERT( FALSE && "invalid work mode " ); 
					}

					ret = set_cur_work_mode( cur_work_mode, ui_notify ); 
					if( ret != ERROR_SUCCESS )
					{
						tmp_text = _get_string_by_id( TEXT_CHANGE_WORK_MODE_FAILED_TIP, 
							_T( "设置工作模式失败,是否已经执行卸载?" ) ); 

						show_msg( GetHWND(), tmp_text ); 
						log_trace( ( MSG_ERROR, "!!!set current work mode failed \n" ) ); 
					}
					else
					{
						tmp_text = _get_string_by_id( TEXT_CHANGE_WORK_MODE_SUCCESSFULLY_TIP, 
							_T( "设置工作模式成功" ) ); 

						show_msg( GetHWND(), tmp_text ); 
					}

					Close(); 
				}
				else if( name == _T( "cancel_btn" ) )
				{
					Close(); 
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

        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("chg_mode.xml"), (UINT)0, NULL, &m_pm);
        ASSERT(pRoot && "Failed to parse XML");
        m_pm.AttachDialog(pRoot);
        m_pm.AddNotifier(this); 
		set_ui_style( &m_pm ); 

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

private:
    CButtonUI* m_pCloseBtn;
	action_ui_notify *ui_notify; 
};