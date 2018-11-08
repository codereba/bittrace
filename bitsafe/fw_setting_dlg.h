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

#include "menu_ui.h"
#include "tab_builder.h"
#include "conf_file.h"
#include "sevenfw_rule_conf.h"
#include "name_rule_edit_dlg.h"
#include "ip_rule_edit_dlg.h"

class fw_setting_dlg : public CWindowWnd, public INotifyUI, public IListCallbackUI
{
public:
	fw_setting_dlg() : ip_list( NULL ), 
		url_list( NULL ), 
		tab_ctrl( NULL ), 
		cur_rule_type( URL_RULE_TYPE )
	{
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("FwSettingDlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

	void add_arp_info()
	{
	}

    void Init() 
    {
		//INT32 ret;
		INT32 i; 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
        m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
        ASSERT( m_pMaxBtn != NULL ); 
		m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
        ASSERT( m_pRestoreBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		tab_ctrl = static_cast<CTabLayoutUI*>(m_pm.FindControl( _T( "frame" ) ) ); 

		url_list = static_cast<CListUI*>( m_pm.FindControl( _T( "url_rule_list" ) ) );
		ASSERT( url_list != NULL ); 
		url_list->RemoveAll();
		url_list->SetTextCallback( this ); 

		for( i = 0; i < all_url_rules.size(); i ++ )
		{
			add_list_item( url_list, ( ULONG )i ); 
		}

		log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	}

    void OnPrepare( TNotifyUI& msg ) 
    {

    }

    /*
    * 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
    */
    LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
    {
		HRESULT ret = ERROR_SUCCESS; 
        TCHAR szBuf[ ITEM_TEXT_BUF_LEN ] = {0};
		action_rule_input *define; 

		log_trace( ( MSG_INFO, "enter %s get text of firewall setting %d \n", __FUNCTION__, iIndex ) ); 

		if( tab_ctrl->GetCurSel() == 1 )
		{
			ASSERT( iIndex < all_ip_rules.size() ); 

			define = &all_ip_rules[ iIndex ]; 

			ret = print_rule_param( szBuf, ITEM_TEXT_BUF_LEN, define, iSubItem ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				log_trace( ( MSG_INFO, "print parameter of the rule failed return 0x%0.8x", ret ) ); 
			}
		}
		else if( tab_ctrl->GetCurSel() == 0 )
		{
			ASSERT( iIndex < all_url_rules.size() ); 

			define = &all_url_rules[ iIndex ]; 
			ret = print_rule_param( szBuf, ITEM_TEXT_BUF_LEN, define, iSubItem ); 
			if( ret != ERROR_SUCCESS )
			{
				//ASSERT( FALSE ); 
				log_trace( ( MSG_INFO, "print parameter of the rule failed return 0x%0.8x", ret ) ); 
			}
		}
		else
		{
			ASSERT( FALSE ); 
		}
			//default:
			//	ASSERT( "invalid ccolumn index\n" && FALSE ); 

        pControl->SetUserData( szBuf );

		log_trace( ( MSG_INFO, "leave %s load text %ws\n", __FUNCTION__, pControl->GetUserData() ) ); 

        return pControl->GetUserData();
    }

    void Notify(TNotifyUI& msg)
    {
		CStdString name; 

        if( msg.sType == _T("windowinit") ) 
            OnPrepare(msg);
        else if( msg.sType == _T("click") ) 
        {
            if( msg.pSender == m_pCloseBtn ) 
            {
				Close(); 
                return; 
            }
            else if( msg.pSender == m_pMinBtn ) 
            { 
                SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
                return; 
            }
            else if( msg.pSender == m_pMaxBtn ) 
            { 
                //SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
            }
            else if( msg.pSender == m_pRestoreBtn ) 
            { 
                SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
            }

			name = msg.pSender->GetName(); 
			if( name == _T( "add_btn" ) )
			{
				HRESULT ret; 
				action_rule_input action_rule; 
				ip_rule_edit_dlg ip_rule_edit( &action_rule, cur_rule_type ); 
				name_rule_edit_dlg rule_edit( &action_rule, cur_rule_type ); 

				init_rule_input( &action_rule ); 
				action_rule.type = INVALID_RULE_TYPE; 

				if( cur_rule_type == SOCKET_RULE_TYPE )
				{
					ip_rule_edit.Create( GetHWND(), _T(""), UI_WNDSTYLE_FRAME, 0L, 0, 0, 600, 400);
					ip_rule_edit.CenterWindow();
					ip_rule_edit.SetIcon( IDI_MAIN_ICON ); 
					ip_rule_edit.ShowModal(); 
				}
				else
				{
					rule_edit.Create( GetHWND(), _T(""), UI_WNDSTYLE_FRAME, 0L, 0, 0, 600, 400);
					rule_edit.CenterWindow();
					rule_edit.SetIcon( IDI_MAIN_ICON ); 
					rule_edit.ShowModal(); 
				}

				if( action_rule.type != URL_RULE_TYPE )
				{
					goto _return; 
				}

				if( *action_rule.desc.url.url.define.url.url == L'\0' )
				{
					goto _return; 
				}

				ret = input_rule_to_config( &action_rule ); 

				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( MSG_ERROR, "add the defense allow action rule failed %s \n", action_rule.desc.params[ 1 ].define.common.name ) ); 
				}

				ret = config_url_rule( &action_rule ); 
				if( ret != ERROR_SUCCESS )
				{
					goto _return; 
				}

				all_url_rules.push_back( action_rule ); 

				add_list_item( url_list, all_url_rules.size() - 1 ); 
			}
			else if( name == _T( "cancel_btn" ) )
			{
				Close(); 
			}
			else if( name == _T( "edit_btn" ) )
			{
			}
			else if( name == _T( "del_btn" ) )
			{
			}
        }
		else if(msg.sType==_T("selectchanged"))
		{

			name = msg.pSender->GetName();
			
			ASSERT( tab_ctrl != NULL ); 

			__Trace( _T( "the set focus target control is %s \n" ), name ); 

			if(name==_T("url_btn"))
			{
				tab_ctrl->SelectItem( 0 );
				cur_rule_type = URL_RULE_TYPE; 
			}
			else if( name == _T( "ip_btn" ) )
			{
				tab_ctrl->SelectItem( 1 ); 
				cur_rule_type = SOCKET_RULE_TYPE; 

				if( ip_list == NULL )
				{
					INT32 i; 

					ip_list = static_cast<CListUI*>(m_pm.FindControl( _T( "ip_rule_list" ) ) );
					ASSERT( ip_list != NULL ); 
					ip_list->RemoveAll();
					ip_list->SetTextCallback( this ); 

					for( i = 0; i < all_ip_rules.size(); i ++ )
					{
						add_list_item( ip_list, ( ULONG )i ); 
					}
				}
			}
		}
        else if(msg.sType==_T("setfocus"))
        {
        }
        else if( msg.sType == _T("itemclick") ) 
        {
        }
        else if( msg.sType == _T("itemactivate") ) 
        {
            int iIndex = msg.pSender->GetTag();
            CStdString sMessage = _T("Click: ");;
        }
        else if(msg.sType == _T("menu")) 
        {
            if( msg.pSender->GetName() != _T("domainlist") ) return;
            menu_ui* pMenu = new menu_ui();
            if( pMenu == NULL ) { return; }
            POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
            ::ClientToScreen(*this, &pt);
            pMenu->Init(msg.pSender, pt);
        }
        else if( msg.sType == _T("menu_Delete") ) {
            CListUI* pList = static_cast<CListUI*>(msg.pSender);
            int nSel = pList->GetCurSel();
            if( nSel < 0 ) return;
            pList->RemoveAt(nSel);
        }
_return:
		return; 
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
		tabs_builder cb; 
        CControlUI* pRoot = builder.Create(_T("fw_setting.xml"), (UINT)0, &cb, &m_pm);
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
			//case WM_ADDLISTITEM	lRes = OnAddListItem(uMsg, wParam, lParam, bHandled); break;
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
    CButtonUI* m_pMaxBtn;
    CButtonUI* m_pRestoreBtn;
    CButtonUI* m_pMinBtn;

	CListUI *url_list; 
	CListUI *ip_list; 
	CTabLayoutUI* tab_ctrl; 
	action_rule_type cur_rule_type; 
};