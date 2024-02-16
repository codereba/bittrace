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
#include "md5.h"
#include "msg_box.h"
#include "user_manage.h"

#define USER_PWD_LEN_LIMIT 1

INLINE LRESULT get_text_from_name( CPaintManagerUI *pm, LPCTSTR name, CStdString &text )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 

	ASSERT( pm != NULL ); 

	ctrl = pm->FindControl( name ); 

	if( ctrl == NULL )
	{
		ret = ERROR_NOT_FOUND; 
		goto _return; 
	}

	text = ctrl->GetText(); 

_return:
	return ret; 
}

INLINE LRESULT set_text_by_name( CPaintManagerUI *pm, LPCTSTR name, LPCTSTR text )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 

	ASSERT( pm != NULL ); 

	ctrl = pm->FindControl( name ); 

	if( ctrl == NULL )
	{
		ret = ERROR_NOT_FOUND; 
		goto _return; 
	}

	ctrl->SetText( text ); 

_return:
	return ret; 
}

class chg_pwd_dlg : public CWindowWnd, public INotifyUI
{
public:
    chg_pwd_dlg()
	{
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("TChangePwdDlg"); 
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

		set_ctrl_text( &m_pm, _T( "title_label" ), TEXT_CHANGE_PASSWORD_TITLE ); 
		set_ctrl_text( &m_pm, _T( "ok_btn" ), TEXT_COMMON_BUTTON_CONFIRM ); 
		set_ctrl_text( &m_pm, _T( "cancel_btn" ), TEXT_COMMON_BUTTON_CANCEL ); 
		set_ctrl_text( &m_pm, _T( "old_pwd_label" ), TEXT_CHANGE_PASSWORD_OLD_LABEL ); 
		set_ctrl_text( &m_pm, _T( "new_pwd_label" ), TEXT_CHANGE_PASSWORD_NEW_LABEL ); 
		set_ctrl_text( &m_pm, _T( "confirm_new_label" ), TEXT_CHANGE_PASSWORD_NEW_AGAIN_LABEL ); 
		set_ctrl_text( &m_pm, _T( "tip_txt" ), TEXT_CHANGE_PASSWORD_ORIGINAL_TIP ); 
		set_ctrl_text( &m_pm, _T( "mng_tip_txt" ), TEXT_CHANGE_PASSWORD_USER_MANAGE_TIP ); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

	LRESULT on_apply()
	{
		LRESULT ret; 
		dlg_ret_state ret_state; 
		CStdString old_pwd; 
		CStdString new_pwd; 
		CStdString confirm_pwd; 
		BYTE pwd_md5[ PWD_DATA_LEN ];
		BYTE old_pwd_md5[ PWD_DATA_LEN ]; 
		LPCTSTR tmp_text; 

		ret = get_text_from_name( &m_pm, _T( "org_pwd_edit" ), old_pwd ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = get_text_from_name( &m_pm, _T( "new_pwd_edit" ), new_pwd ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = get_text_from_name( &m_pm, _T( "confirm_pwd_edit" ), confirm_pwd ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		if( confirm_pwd != new_pwd )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			set_text_by_name( &m_pm, _T( "new_pwd_edit" ), _T( "" ) ); 
			set_text_by_name( &m_pm, _T( "confirm_pwd_edit" ), _T( "" ) ); 

			tmp_text = _get_string_by_id( TEXT_CHANGE_PASSWORD_NEW_NOT_SAME_ERROR, 
				_T( "������������벻һ��" ) ); 

			show_msg( GetHWND(), tmp_text, NULL, NULL, 0 ); 
			goto _return; 
		}

		ret = check_pwd_file_consist( pwd_md5, sizeof( pwd_md5 ) ); 

		if( ret != ERROR_SUCCESS )
		{
			INT32 is_none_pwd; 
			if( ret != ERROR_FILE_NOT_FOUND )
			{
				ret = check_none_pwd( &is_none_pwd ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				if( is_none_pwd != TRUE )
				{
					goto _return; 
				}
			}

			if( old_pwd != _T( "" ) )
			{
				set_text_by_name( &m_pm, _T( "org_pwd_edit" ), _T( "" ) ); 
				
				tmp_text = _get_string_by_id( TEXT_CHANGE_PASSWORD_OLD_INCORRECT_ERROR, _T( "�����벻��ȷ" ) ); 
				
				show_msg( GetHWND(), tmp_text, NULL, 0 ); 
				goto _return; 
			}
		}
		else
		{
			MD5( ( PBYTE )old_pwd.GetData(), old_pwd.GetLength(), old_pwd_md5 ); 

			if( memcmp( pwd_md5, old_pwd_md5, MD5_DIGEST_LENGTH ) != 0 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				tmp_text = _get_string_by_id( TEXT_CHANGE_PASSWORD_OLD_INCORRECT_ERROR, _T( "�����벻��ȷ" ) ); 

				set_text_by_name( &m_pm, _T( "org_pwd_edit" ), _T( "" ) ); 
				show_msg( GetHWND(), tmp_text, NULL, 0 );
				goto _return; 
			}
		}

		if( new_pwd.GetLength() == 0 )
		{
			dlg_ret_state ret_state; 
			//ret = ERROR_ERRORS_ENCOUNTERED; 

			tmp_text = _get_string_by_id( TEXT_CHANGE_PASSWORD_NEW_LENGTH_ERROR, _T( "���뽫������Ϊ��,��ȷ��?" ) ); 

			ret = show_msg( GetHWND(), tmp_text, &ret_state, 0 ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		
			if( ret_state == OK_STATE )
			{
				ret = clear_password(); 
			}
			else
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
		}
		else if( new_pwd.GetLength() > 0 )
		{
			MD5( ( PBYTE )new_pwd.GetData(), new_pwd.GetLength(), pwd_md5 ); 

			//WritePrivateProfile( "BITSAFE", "USER_PWD", pwd_md5, conf_file_path.GetData() ); 

			ret = write_pwd( pwd_md5, sizeof( pwd_md5 ) ); 
		}
		else
		{

		}
_return:
		if( ret == ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_CHANGE_PASSWORD_SUCCESSFUL_TIP, _T( "�޸�����ɹ�" ) ); 
			
			show_msg( GetHWND(), tmp_text, NULL, 0 ); 
			Close(); 
		}

		return ret; 
	}

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T( "windowinit" ) ) 
		{
			OnPrepare( msg );
		}
		else if( msg.sType == _T( "return" ) )
		{
			on_apply(); 
		}
		else if( msg.sType == _T( "click" ) ) 
        {
            if( msg.pSender == m_pCloseBtn ) 
            {
				Close(); 
                return; 
            }
			else
			{
				CStdString name = msg.pSender->GetName(); 

				if( name == _T( "ok_btn" ) )
				{
					on_apply(); 
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
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("chg_pwd.xml"), (UINT)0, NULL, &m_pm);
        ASSERT(pRoot && "Failed to parse XML");
        m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this); 
		set_ui_style( &m_pm ); 

        Init();
        return 0;
    }

	LRESULT on_key_up(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE; 
		m_pm.defaut_key_handle( ( ULONG )wParam, ( ULONG )lParam, _T( "ok_btn" ), _T( "cancel_btn" ) ); 
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
			case WM_KEYUP:			lRes = on_key_up( uMsg, wParam, lParam, bHandled ); break; 
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
};