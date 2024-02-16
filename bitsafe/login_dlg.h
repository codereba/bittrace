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
//#include "menu_ui.h"
#include "md5.h"
#include "msg_box.h"
#include "wnd_state.h"
#include "chg_pwd_dlg.h"

class login_dlg : public CWindowWnd, public wnd_state, public INotifyUI
{
public:
    login_dlg() 
	{
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("LoginDlg"); 
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
        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 

		set_ctrl_text( &m_pm, _T( "title_label" ), TEXT_USER_LOGIN_TITLE ); 
		set_ctrl_text( &m_pm, _T( "pwd_label" ), TEXT_USER_LOGIN_PASSWORD ); 
		set_ctrl_text( &m_pm, _T( "ok_btn" ), TEXT_USER_LOGIN_LOGIN_BUTTON ); 
		set_ctrl_text( &m_pm, _T( "chg_pwd_btn" ), TEXT_USER_LOGIN_CHANGE_PASSWORD_BUTTON ); 
		set_ctrl_text( &m_pm, _T( "cancel_btn" ), TEXT_COMMON_BUTTON_CANCEL ); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

	}

	LRESULT apply_pwd()
	{
		INT32 ret = 0; 
		CStdString pwd; 
		CStdString cur_pwd; 
		BYTE pwd_md5[ PWD_DATA_LEN ];
		BYTE cur_pwd_md5[ PWD_DATA_LEN ]; 
		LPCTSTR tmp_text; 

		change_original_pwd( GetHWND() ); 

		ret = get_text_from_name( &m_pm, _T( "pwd_edit" ), pwd ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		MD5( ( PBYTE )pwd.GetData(), pwd.GetLength(), pwd_md5 ); 

		tmp_text = _get_string_by_id( TEXT_USER_LOGIN_INCORRECT_PASSWORK_TIP, 
			_T( "���벻��ȷ" ) ); 

		ret = check_pwd_file_consist( cur_pwd_md5, sizeof( cur_pwd_md5 ) ); 
		if( ret != ERROR_SUCCESS )
		{
			if( ret != ERROR_FILE_NOT_FOUND )
			{
				goto _return; 
			}

			if( pwd != _T( "" ) )
			{
				show_msg( GetHWND(), tmp_text, NULL, 0 ); 
				goto _return; 
			}
		}
		else
		{
			if( memcmp( pwd_md5, cur_pwd_md5, MD5_DIGEST_LENGTH ) != 0 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				set_text_by_name( &m_pm, _T( "pwd_edit" ), _T( "" ) ); 

				show_msg( GetHWND(), tmp_text, NULL, 0 );
				goto _return; 
			}
		}

_return:
		if( ret == ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_USER_LOGIN_SUCCESSFUL_TIP, 
				_T( "�ɹ���½,֮�����еĲ������Թ���Ա����ִ��." ) ); 

			set_cur_user_group( ADMIN ); 
			show_msg( GetHWND(), tmp_text, NULL, 0 ); 
			ret_status = OK_STATE; 
			Close(); 
		}

		return ret; 
	}

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("windowinit") ) 
            OnPrepare(msg);
		else if( msg.sType == _T( "return" ) )
		{
			apply_pwd(); 
		}
        else if( msg.sType == _T("click") ) 
        {
            if( msg.pSender == m_pCloseBtn ) 
            {
				ret_status = CANCEL_STATE; 
				Close(); 
                return; 
            }
			else
			{
				CStdString name = msg.pSender->GetName(); 

				if( name == _T( "ok_btn" ) )
				{
					apply_pwd(); 
				}
				else if( name == _T( "chg_pwd_btn" ) )
				{
					chg_pwd_dlg dlg; 
					LPCTSTR tmp_text; 

					tmp_text = _get_string_by_id( TEXT_CHANGE_PASSWORD_SET_PWD_TITLE, 
						_T("������������") ); 
					dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
					dlg.SetIcon( IDI_MAIN_ICON ); 
					dlg.CenterWindow();
					dlg.ShowModal(); 
				}
				else if( name == _T( "cancel_btn" ) )
				{
					ret_status = CANCEL_STATE; 
					Close(); 
				}
			}
        }
        else if(msg.sType==_T("setfocus"))
        {
        }
        //else if(msg.sType == _T("menu")) 
        //{
        //    if( msg.pSender->GetName() != _T("domainlist") ) return;
        //    menu_ui* pMenu = new menu_ui();
        //    if( pMenu == NULL ) { return; }
        //    POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
        //    ::ClientToScreen(*this, &pt);
        //    pMenu->Init(msg.pSender, pt);
        //}
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("login.xml"), (UINT)0, NULL, &m_pm);
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

	LRESULT on_key_up(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pm.defaut_key_handle( ( ULONG )wParam, ( ULONG )lParam, _T( "ok_btn" ), _T( "cancel_btn" ) ); 
		bHandled = TRUE; 
		return 0; 
	}

    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        // ��ʱ�����յ�WM_NCDESTROY���յ�wParamΪSC_CLOSE��WM_SYSCOMMAND
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
			case WM_KEYUP: 
				lRes = on_key_up(uMsg, wParam, lParam, bHandled ); 
				break; 
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

    //...
};