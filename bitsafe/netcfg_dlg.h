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
#include "netcfg_mng.h"
class netcfg_dlg : public CWindowWnd, public INotifyUI, public IListCallbackUI
{
public:
    netcfg_dlg() {
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("AboutDlg"); 
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

		set_ctrl_text( &m_pm, _T( "title_txt" ), TEXT_NETWORK_CTRL_PANEL_TITLE ); 
		set_ctrl_text( &m_pm, _T( "open_btn" ), TEXT_NETWORK_CTRL_PANEL_OPEN ); 
		set_ctrl_text( &m_pm, _T( "stop_btn" ), TEXT_NETWORK_CTRL_PANEL_CLOSE ); 
		set_ctrl_text( &m_pm, _T( "tip_txt" ), TEXT_NETWORK_CTRL_PANEL_NOTICE ); 

		//m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
  //      ASSERT( m_pRestoreBtn != NULL ); 
		//m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		//ASSERT( m_pMinBtn != NULL ); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

    void OnSearch()
    {
        CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("domainlist")));
        pList->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID );
        DWORD dwThreadID = 0;
        pList->SetTextCallback(this);//[1]

        //HANDLE hThread = CreateThread(NULL,0,&netcfg_dlg::Search, (LPVOID)NULL,  0,&dwThreadID);
    }
    /*
    * 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
    */
    LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
    {
        TCHAR szBuf[MAX_PATH] = {0};
        switch (iSubItem)
        {
        case 0:
            _stprintf(szBuf, _T("%d"), iIndex);
            break;
        case 1:
            {
#ifdef _UNICODE		
            //int iLen = arp_info_list[iIndex].length();
            //LPWSTR lpText = new WCHAR[iLen + 1];
            //::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
            //::MultiByteToWideChar(CP_ACP, 0, arp_info_list[iIndex].c_str(), -1, (LPWSTR)lpText, iLen) ;
            //_stprintf(szBuf, lpText);
            //delete[] lpText;
#else
            _stprintf(szBuf, arp_info_list[iIndex].c_str());
#endif
            }
            break;
        case 2:
            {
#ifdef _UNICODE		
				
				//INT32 iLen = 100; 
    //        LPWSTR lpText = new WCHAR[iLen + 1];
    //        ::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
    //        ::MultiByteToWideChar(CP_ACP, 0, desc[iIndex].c_str(), -1, (LPWSTR)lpText, iLen) ;
    //        _stprintf(szBuf, lpText);
    //        delete[] lpText;
#else
            _stprintf(szBuf, desc[iIndex].c_str());
#endif
            }
            break;
        }
        pControl->SetUserData(szBuf);
        return pControl->GetUserData();
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
				LRESULT ret = ERROR_SUCCESS; 
				CStdString name; 
				LPCTSTR tmp_text; 

				name = msg.pSender->GetName(); 

				if( name == _T( "open_btn" ) )
				{
#define REPLACE_360_HOOK 0x01

					ret = start_net_cfg(); 
					if( ret != ERROR_SUCCESS )
					{
						tmp_text = _get_string_by_id( TEXT_NETWORK_CTRL_PANEL_OPEN_FAILED, 
							_T( "开启网络控制面板失败,可能是不兼容当前系统环境软件，请稍后重试." ) ); 

						show_msg( GetHWND(), tmp_text ); 
					}
					else
					{
						tmp_text = _get_string_by_id( TEXT_NETWORK_CTRL_PANEL_OPEN_SUCCESSFULLY, 
							_T( "开启网络控制面板成功." ) ); 
						
						show_msg( GetHWND(), tmp_text ); 
					}
					Close(); 
				}
				else if( name == _T( "stop_btn" ) )
				{
					ret = stop_net_cfg(); 	
					if( ret != ERROR_SUCCESS )
					{
						tmp_text = _get_string_by_id( TEXT_NETWORK_CTRL_PANEL_CLOSE_FAILED, 
							_T( "停止网络控制面板失败,可能是不兼容当前系统环境软件，请稍后重试." ) ); 

						show_msg( GetHWND(), tmp_text ); 
					}
					else
					{
						tmp_text = _get_string_by_id( TEXT_NETWORK_CTRL_PANEL_CLOSE_SUCCESSFULLY, 
							_T( "停止网络控制面板成功." ) ); 
						show_msg( GetHWND(), tmp_text ); 
					}
					Close(); 
				}
			}
            //else if( msg.pSend._SYSCOMMAND, SC_MINIMIZE, 0);6
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
			//else
			//{
			//	CStdString name; 
			//	name = msg.pSender->GetName(); 
			//}
        }
        else if(msg.sType==_T("setfocus"))
        {
        }
        else if( msg.sType == _T("itemclick") ) 
        {
        }
        else if( msg.sType == _T("itemactivate") ) 
        {
//            int iIndex = msg.pSender->GetTag();
//            CStdString sMessage = _T("Click: ");;
//#ifdef _UNICODE		
//            int iLen = domain[iIndex].length();
//            LPWSTR lpText = new WCHAR[iLen + 1];
//            ::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
//            ::MultiByteToWideChar(CP_ACP, 0, domain[iIndex].c_str(), -1, (LPWSTR)lpText, iLen) ;
//            sMessage += lpText;
//            delete[] lpText;
//#else
//            sMessage += domain[iIndex].c_str();
//
//#endif
//            ::MessageBox(NULL, sMessage.GetData(), _T("提示"), MB_OK);
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
        //else if( msg.sType == _T("menu_Delete") ) {
        //    //CListUI* pList = static_cast<CListUI*>(msg.pSender);
        //    //int nSel = pList->GetCurSel();
        //    //if( nSel < 0 ) return;
        //    //pList->RemoveAt(nSel);
        //    //domain.erase(domain.begin() + nSel);
        //    //desc.erase(desc.begin() + nSel);   
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
        CControlUI* pRoot = builder.Create(_T("netcfg.xml"), (UINT)0, NULL, &m_pm);
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
		m_pm.RemoveNotifier(this);
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
        POINT pt; 
		pt.x = GET_X_LPARAM(lParam); 
		pt.y = GET_Y_LPARAM(lParam);
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

    //...
};