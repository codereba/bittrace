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
#include "conf_file.h"
#include "bitsafe_rule_conf.h"
#include "msg_box.h"

typedef enum _truct_block_app_op
{
	ADD_DEFENSE_BLOCK_FILE = 0, 
	ADD_DEFENSE_TRUST_FILE = 1, 
	ADD_FW_BLOCK_FILE = 2, 
	ADD_FW_TRUST_FILE = 3, 
	BEGIN_APP_OP = ADD_DEFENSE_BLOCK_FILE , 
	END_APP_OP = ADD_FW_TRUST_FILE
} truct_block_app_op, *ptruct_block_app_op; 

#define is_valid_app_work_type( work_mode ) ( work_mode >= BEGIN_APP_OP && work_mode <= END_APP_OP )
class trust_block_dlg : public CWindowWnd, public INotifyUI
{
public:
	//trust_block_dlg() : work_mode( ADD_DEFENSE_BLOCK_FILE  )
	//{
 //   };

	trust_block_dlg( ULONG mode ) 
	{
		if( is_valid_app_work_type( mode ) == FALSE )
		{
			work_mode = ADD_DEFENSE_BLOCK_FILE ; 
		}
		else
		{
			work_mode = mode; 
		}
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("AddTrustAppDlg"); 
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
		CControlUI *ctrl; 
		LPCTSTR tmp_text; 

        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 

		ctrl = m_pm.FindControl( _T( "title_txt" ) ); 

		if( work_mode == ADD_DEFENSE_BLOCK_FILE 
			|| work_mode == ADD_FW_BLOCK_FILE )
		{
			tmp_text = _get_string_by_id( TEXT_DISTRUST_APP_ADD, 
				_T( "添加被阻止的程序" ) ); 
			ctrl->SetText( tmp_text ); 
		
		}
		else if( work_mode ==  ADD_DEFENSE_TRUST_FILE 
				|| work_mode == ADD_FW_TRUST_FILE )
		{
			tmp_text = _get_string_by_id( TEXT_TRUST_APP_ADD, 
				_T( "添加被信任的程序") ); 
			ctrl->SetText( tmp_text ); 
		}

		set_ctrl_text( &m_pm, _T( "path_lbl" ), TEXT_TRUST_APP_MODULE_FILE_NAME ); 
		set_ctrl_text( &m_pm, _T( "sel_btn" ), TEXT_TRUST_APP_BROWSER_FILE ); 
		set_ctrl_text( &m_pm, _T( "apply_btn" ), TEXT_COMMON_BUTTON_CONFIRM ); 
		set_ctrl_text( &m_pm, _T( "cancel_btn" ), TEXT_COMMON_BUTTON_CANCEL ); 

		//m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
  //      ASSERT( m_pRestoreBtn != NULL ); 
		//m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		//ASSERT( m_pMinBtn != NULL ); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

    void Notify(TNotifyUI& msg)
    {
		INT32 ret; 
		TCHAR file_name[ MAX_PATH ]; 
		CEditUI *file_path_edit; 
		CStdString name = msg.pSender->GetName(); 
		CStdString app_path; 
		action_response_type resp; 
		INT32 file_name_index; 
		LPCTSTR tmp_text; 

        if( msg.sType == _T("windowinit") ) 
            OnPrepare(msg);
        else if( msg.sType == _T("click") ) 
        {
            if( msg.pSender == m_pCloseBtn ) 
            {
				Close(); 
                return; 
            }
            //else if( msg.pSender == m_pMinBtn ) 
            //{ 
            //    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
            //    return; 
            //}
            //else if( msg.pSender == m_pMaxBtn ) 
            //{ 
            //    //SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
            //}
            //else if( msg.pSender == m_pRestoreBtn ) 
            //{ 
            //    SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
            //}
			else
			{
				*file_name = _T( '\0' ); 
				if( name == _T( "sel_btn" ) )
				{
					ret = open_file_dlg( TRUE, NULL, file_name, NULL ); 
					if( ret < 0 )
					{
						goto _return; 
					}

					//name = file_name; 

					file_path_edit = ( CEditUI* )m_pm.FindControl( _T( "add_path_edit" ) ); 
					ASSERT( file_path_edit != NULL ); 

					file_path_edit->SetText( file_name ); 
				}
				else if( name == _T( "apply_btn" ) )
				{
					file_path_edit = ( CEditUI* )m_pm.FindControl( _T( "add_path_edit" ) ); 
					ASSERT( file_path_edit != NULL ); 

					//DBG_BP(); 
					app_path = file_path_edit->GetText(); 

					file_name_index = app_path.ReverseFind( _T( '\\' ) ) + 1; 
					app_path = app_path.Right( app_path.GetLength() - file_name_index ); 

					//*rule_input.addition = L'\0'; 
					if( work_mode == ADD_FW_TRUST_FILE
						|| work_mode == ADD_FW_BLOCK_FILE )
					{
						if( ADD_FW_TRUST_FILE == work_mode )
						{
							resp = ACTION_ALLOW; 							
						}
						else
						{
							resp = ACTION_BLOCK; 
						}

						ret = config_fw_app_rule( resp, app_path.GetData(), TRUE ); 
						if( ret != ERROR_SUCCESS )
						{
							goto _return; 
						}
					}
					else if( work_mode ==  ADD_DEFENSE_TRUST_FILE
						|| work_mode == ADD_DEFENSE_BLOCK_FILE )
					{
						if( ADD_DEFENSE_TRUST_FILE == work_mode )
						{
							resp = ACTION_ALLOW; 							
						}
						else
						{
							resp = ACTION_BLOCK; 
						}

						ret = config_defense_app_rule( resp, app_path.GetData(), TRUE ); 
						if( ret != ERROR_SUCCESS )
						{
							goto _return; 
						}
					}
					else
					{
						ASSERT( FALSE ); 
					} 

_return:
					if( ret == ERROR_SUCCESS )
					{
						tmp_text = _get_string_by_id( TEXT_TRUST_APP_ADD_SUCCESSFULLY_TIP, 
							_T( "添加应用程序规则成功." ) ); 
						show_msg( GetHWND(), tmp_text ); 
					}
					else
					{
						tmp_text = _get_string_by_id( TEXT_TRUST_APP_ADD_ERROR_TIP, 
							_T( "添加应用程序规则失败,是否已经卸载?" ) ); 
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
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("trust_block_dlg.xml"), (UINT)0, NULL, &m_pm);
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
    //CButtonUI* m_pMaxBtn;
    //CButtonUI* m_pRestoreBtn;
    //CButtonUI* m_pMinBtn;
	ULONG work_mode; 
};