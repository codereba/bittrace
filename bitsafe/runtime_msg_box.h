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

#include "ui_config.h"
#include "wnd_state.h"
#include "resource.h"
#include "window_effective.h"
#include "popup_tip_wnd.h"
#include "common_config.h"
class runtime_msg_box : public CWindowWnd, public wnd_state, public INotifyUI
{
public:
	runtime_msg_box() : tranparent_val( 0 )
	{
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("SloganBox"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

	LRESULT hide_not_show_tip()
	{
		COptionUI *tip_btn; 
		tip_btn = ( COptionUI* )m_pm.FindControl( _T( "tip_setting_btn" ) ); 

		ASSERT( tip_btn != NULL ); 

		tip_btn->SetEnabled( false ); 
		tip_btn->SetVisible( false ); 

		return ERROR_SUCCESS; 
	}

    void Init() 
    {
		ret_status = CANCEL_STATE; 

		set_ctrl_text( &m_pm, _T( "tip_setting_btn" ), TEXT_SLOGAN_NOT_TIP_AGAIN ); 
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
			}
        }
		else if( msg.sType == _T( "selectchanged" ) )
		{
		}
        else if(msg.sType==_T("setfocus"))
        {
        }
    }


    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		LRESULT ret = ERROR_SUCCESS; 

        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong( *this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN ); 

		styleValue = ::GetWindowLong(*this, GWL_EXSTYLE);
		::SetWindowLong( *this, GWL_STYLE, styleValue | WS_EX_TOOLWINDOW ); 
		
        m_pm.Init(m_hWnd);

        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("slogan_box.xml"), (UINT)0, NULL, &m_pm);
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

		{
			COptionUI *tip_btn; 
			tip_btn = ( COptionUI* )m_pm.FindControl( _T( "tip_setting_btn" ) ); 
			ASSERT( tip_btn != NULL ); 

			if( TRUE == tip_btn->IsSelected() )
			{
				if( get_slogan_show() == TRUE )
				{
					set_slogan_show( FALSE ); 
					save_common_conf(); 
				}
			}
#if 0
			else
			{
				if( get_slogan_show() == FALSE )
				{
					set_slogan_show( TRUE ); 
					save_ui_conf(); 
				}
			}
#endif //0	
		}

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
        // ��ʱ�����յ�WM_NCDESTROY���յ�wParamΪSC_CLOSE��WM_SYSCOMMAND
        if( wParam == SC_CLOSE ) {
            DestroyWindow( GetHWND() ); 
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

	LRESULT on_timer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 
		do 
		{
#define TRANSPARENT_VAL_INC 10
			if( tranparent_val + TRANSPARENT_VAL_INC < 255 /*get_tranparent()*/ )
			{
				tranparent_val += TRANSPARENT_VAL_INC; 
				m_pm.SetTransparent( tranparent_val ); 
			}
			else
			{
				m_pm.SetTransparent( 255 ); 

				KillTimer( GetHWND(), wParam ); 
			}

		}while( FALSE );

		return ret; 
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
			case WM_TIMER:		 lRes = on_timer(uMsg, wParam, lParam, bHandled); break;
            default:
                bHandled = FALSE;
        }
        if( bHandled ) return lRes;
        if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

public:
    CPaintManagerUI m_pm; 
	ULONG tranparent_val; 

private: 
};

INLINE LRESULT show_slogan( HWND parent, 
						   LPCTSTR msg, 
						   dlg_ret_state *ret_state = NULL, 
						   LPCTSTR title = NULL, 
						   ULONG mode = 0 )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	runtime_msg_box msg_tip; 
	CControlUI *msg_txt; 
	RECT show_rc; 
	RECT rcWnd; 
	
	if( ret_state != NULL )
	{
		*ret_state = CANCEL_STATE; 
	}

	if( parent )
	{
		_ret = SetForegroundWindow( parent ); 
		if( _ret == FALSE )
		{
			ASSERT( FALSE ); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "set main window to foreground failed\n") ); 
		}
	}

	if( NULL == msg_tip.Create( parent, title, UI_WNDSTYLE_DIALOG | WS_EX_TOPMOST, 0L, 0, 0, 0, 0) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	if( title == NULL )
	{
		title = _T( "" ); 
	}

	msg_tip.SetIcon( IDI_MAIN_ICON ); 
	msg_txt = msg_tip.m_pm.FindControl( _T( "msg_txt" ) ); 

	if( get_slogan_show() == FALSE )
	{
		msg_tip.hide_not_show_tip(); 
	}

	ASSERT( msg_txt != NULL ); 
	msg_txt->SetText( msg ); 

	if( title != NULL )
	{
		msg_txt = msg_tip.m_pm.FindControl( _T( "title_txt" ) ); 

		ASSERT( msg_txt != NULL ); 
		msg_txt->SetText( title ); 
	}

	do 
	{
		_ret = GetWindowRect( msg_tip.GetHWND(), &rcWnd ); 
		if( FALSE == _ret )
		{
			break; 
		}

		ret = _calc_popup_wnd_rect( 0, wnd_popup_bottom, &rcWnd, &show_rc ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		_ret = SetWindowPos( msg_tip.GetHWND(), HWND_TOPMOST,
			show_rc.left,
			show_rc.top, 
			RECT_WIDTH( show_rc ),
			RECT_HEIGHT( show_rc ), 
			SWP_SHOWWINDOW ); 

		if( _ret == FALSE )
		{
			DBG_BP(); 
		}
	}while( FALSE ); 


	msg_tip.m_pm.SetTransparent( 0 ); 

#define TRANSPARENT_TIMER_ELAPSE 13
#define BLEND_ANIMATE_TIMER 1001
	_ret = ( INT32 )SetTimer( msg_tip.GetHWND(), BLEND_ANIMATE_TIMER, TRANSPARENT_TIMER_ELAPSE, NULL ); 
	if( FALSE == _ret )
	{
		DBG_BP(); 
	}

	msg_tip.ShowModal(); 

	if( ret_state != NULL )
	{
		*ret_state = msg_tip.get_dlg_ret_statue(); 
	}

_return:
	return ret; 
}

LRESULT start_slogan_display( action_ui_notify *ui_notifer ); 
LRESULT stop_slogan_display(); 