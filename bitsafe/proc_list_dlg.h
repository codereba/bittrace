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

#include "ui_ctrl.h"
#include "mem_region_list.h"
#include "conf_file.h"
#include "bitsafe_rule_conf.h"
#include "menu_wnd.h"

ULONG CALLBACK thread_list_proc( PVOID param ); 

class proc_list_dlg : public CWindowWnd, public INotifyUI, public IListCallbackUI
{
public:
#define PROC_ENTRYS_BUF_INC 100
	proc_list_dlg() : proc_list( NULL )
	{
		init_cs_lock( proc_info_lock ); 

		memset( &proc_info_thread, 0, sizeof( proc_info_thread ) ); 
		all_proc_infos = NULL; 
		max_proc_count = 0; 
    };

	~proc_list_dlg()
	{
		uninit_cs_lock( proc_info_lock ); 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T( "ProcListDlg" ); 
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
		LRESULT ret; 

		proc_list = static_cast< CListUI* >( m_pm.FindControl( _T( "proc_list" ) ) ); 

		ASSERT( proc_list != NULL ); 

		proc_list->SetTextCallback( this ); 

        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
        m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
        ASSERT( m_pMaxBtn != NULL ); 
		m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
        ASSERT( m_pRestoreBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		all_proc_infos = ( PROCESSENTRY32* )malloc( sizeof( PROCESSENTRY32 ) * PROC_ENTRYS_BUF_INC ); 
		if( all_proc_infos == NULL )
		{
			log_trace( ( MSG_ERROR, "allocate proces information buffer failed\n" ) ); 
			Close(); 
		}

		max_proc_count = PROC_ENTRYS_BUF_INC; 
		proc_count = 0; 

		ret = create_work_thread( &proc_info_thread, thread_list_proc, NULL, this ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create the process information thread failed\n" ) ); 
			Close(); 
		}
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }
 
    /*
    * 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
    */
    LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
    {
        TCHAR szBuf[ MAX_PATH ] = { 0 }; 

		ASSERT( iIndex >= 0 ); 

		//if( proc_count == 0 )
		//{
		//	goto _return; 
		//}

		if( iIndex >= proc_count )
		{
			goto _return; 
		}

        switch( iSubItem )
        {
		case 0:
			{
				_stprintf( szBuf, _T( "%d" ), all_proc_infos[ iIndex ].th32ProcessID );
			}
			break;
		case 1:
			{
				_stprintf( szBuf, _T( "%s" ), all_proc_infos[ iIndex ].szExeFile );
			}
			break;
		default:
			break; 
		}

_return:
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
        }
        else if(msg.sType==_T("setfocus"))
        {
        }
		else if(msg.sType == _T("menu")) 
		{
			LRESULT ret; 
			LPCTSTR ui_conf_file; 

			log_trace( ( MSG_INFO, "the menu notify sender is 0x%0.8x \n", msg.pSender, proc_list ) ); 

			if( msg.pSender != proc_list ) 
			{
				return; 
			}

			CMenuWnd* pMenu = new CMenuWnd();
			if( pMenu == NULL )
			{ 
				return; 
			}

			if( LANG_ID_CN == get_cur_lang_id() )
			{
				ui_conf_file = L"app_rule_menu.xml"; 
			}
			else
			{
				ui_conf_file = L"app_rule_menu_en.xml"; 
			}
			
			ret = pMenu->set_cfg_file( ui_conf_file ); 
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "set config menu file failed\n" ) ); 
				Close(); 
			}

			POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
			::ClientToScreen(*this, &pt);
			pMenu->Init(msg.pSender, pt);
		}
		else if( msg.sType == _T( "menu_add_block" ) )
		{
			LRESULT ret; 
			INT32 sel_index; 
			sel_index = proc_list->GetCurSel(); 

			if( sel_index < 0 )
			{
				goto _return; 
			}

			if( ( ULONG )sel_index >= proc_count )
			{
				goto _return; 
			}

			lock_cs( proc_info_lock ); 
 			ret = add_app_rule( ACTION_BLOCK, all_proc_infos[ sel_index ].szExeFile, FALSE ); 
			unlock_cs( proc_info_lock ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}
		else if( msg.sType == _T( "menu_add_trust" ) )
		{
			LRESULT ret; 
			INT32 sel_index; 
			sel_index = proc_list->GetCurSel(); 

			if( sel_index < 0 )
			{
				goto _return; 
			}

			if( ( ULONG )sel_index >= proc_count )
			{
				goto _return; 
			}

			lock_cs( proc_info_lock ); 
			ret = add_app_rule( ACTION_ALLOW, all_proc_infos[ sel_index ].szExeFile, FALSE ); 
			unlock_cs( proc_info_lock ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}
		}
        else if( msg.sType == _T("itemactivate") ) 
        {
        }

_return:
		return; 
    }

	LRESULT on_proc_info_got()
	{
		LRESULT ret = ERROR_SUCCESS; 
		ULONG count; 
		INT32 i; 

		ASSERT( proc_list != NULL ); 

		//lock_cr( proc_info_lock ); 
		//unlock_cs( proc_info_lock ); 

		count = proc_count; 
		//proc_list->GetItemCount(); 

		for( i = proc_list->GetItemCount(); ( ULONG )i < count; i ++ )
		{
			add_list_item( proc_list, ( ULONG )i ); 
		}

		return ret; 
	}

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("proc_list_dlg.xml"), (UINT)0, NULL, &m_pm);
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

		stop_work_thread( &proc_info_thread ); 

		if( all_proc_infos != NULL )
		{
			free( all_proc_infos ); 
		}

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
    CButtonUI* m_pMaxBtn;
    CButtonUI* m_pRestoreBtn;
    CButtonUI* m_pMinBtn;
	CRITICAL_SECTION proc_info_lock; 
	PROCESSENTRY32 *all_proc_infos; 
	ULONG max_proc_count; 
	ULONG proc_count; 
	thread_manage proc_info_thread; 
	CListUI *proc_list; 

	friend ULONG CALLBACK thread_list_proc( PVOID param ); 

};