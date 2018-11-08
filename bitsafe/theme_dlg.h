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

#include <vector>
#include "antiarp_api.h"
#include "ui_ctrl.h"
#include "ui_custom.h"

#define WM_THEME_NOTIFY ( WM_USER + 1001 )
LRESULT CALLBACK on_themes_found( LPCTSTR root_path, WIN32_FIND_DATA *find_data, PVOID context ); 


class themes_dlg : public CWindowWnd, public INotifyUI, public action_ui_notify
{
public:
	themes_dlg()
	{ };

		LRESULT get_themes_path( CStdString &theme_path )
		{
			LRESULT ret = ERROR_SUCCESS; 

			theme_path = CPaintManagerUI::GetInstancePath(); 

			if( TRUE == theme_path.IsEmpty() )
			{
				ret = ERROR_NOT_READY; 
			}

			theme_path.Append( _T( "themes" ) ); 

			return ret; 
		}

	LRESULT load_all_themes()
	{
		LRESULT ret = ERROR_SUCCESS; 

		CStdString theme_path; 

		ret = get_themes_path( theme_path ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = find_whole_path( theme_path.GetData(), on_themes_found, ( PVOID )this ); 

_return:
		return ret; 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("theme_dlg"); 
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
        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
  //      m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
  //      ASSERT( m_pMaxBtn != NULL ); 
		//m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
  //      ASSERT( m_pRestoreBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		arp_infos = static_cast< CTreeUI* >( m_pm.FindControl( _T( "arp_infos" ) ) ); 
		ASSERT( arp_infos != NULL ); 

		set_ctrl_text( &m_pm, _T( "anti_arp_btn" ), TEXT_ANTI_ARP_OPEN ); 
		set_ctrl_text( &m_pm, _T("anti_arp_notice_txt"), TEXT_ANTI_ARP_NOTICE ); 


		if( arp_infos != NULL)
		{
			if( arp_infos ->GetCount() > 0 )
			{
				arp_infos->RemoveAll();
			}
		}

		ret = ui_init(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

_return:
		if( ret != ERROR_SUCCESS )
		{
			Close(); 
		}

		return;	
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

	LRESULT on_ok_bn()
	{
		// TODO: 在此添加控件通知处理程序代码

		INT32 ret = 0; 
		CControlUI *ok_btn; 
		LPCTSTR bn_title = NULL; 
		ok_btn = m_pm.FindControl( _T( "anti_arp_btn" ) ); 
		ASSERT( ok_btn != NULL ); 

		load_all_themes(); 

		//add_theme_thumb_to_ui( ); 
		return ret; 
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
                SendMessage( WM_SYSCOMMAND, SC_MINIMIZE, 0 );
                return; 
            }
            //else if( msg.pSender == m_pMaxBtn ) 
            //{ 
            //    //SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
            //}
            //else if( msg.pSender == m_pRestoreBtn ) 
            //{ 
            //    SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
            //}
			else if( msg.pSender->GetName() == _T( "anti_arp_btn" ) )
			{
				on_ok_bn(); 
			}
        }

        else if( msg.sType == _T( "menu" ) ) 
        {
            //CMenuWnd* pMenu = new CMenuWnd();
            //if( pMenu == NULL ) { return; }
            //POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
            //::ClientToScreen(*this, &pt);
            //pMenu->Init(msg.pSender, pt);
        }
    }

	LRESULT ui_init()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *ok_btn; 

		//ok_btn = m_pm.FindControl( _T( "" ) ); 
		//ASSERT( ok_btn != NULL ); 

		return ret; 
	}

	LRESULT before_arp_info_output()
	{
		ASSERT( arp_infos != NULL ); 

		if( arp_infos->GetCount() > 0 )
		{
			arp_infos->RemoveAll(); 
		}

		return ERROR_SUCCESS; 
	}

	LRESULT add_theme_thumb_to_ui( 
		LPCTSTR thumb_file, 
		PVOID context, 
		LONG flags, 
		PVOID *param_out )
	{
		LRESULT ret = ERROR_SUCCESS;
		Node* node_added; 
		tree_list_item_info item;

		ASSERT( thumb_file != NULL ); 

		item.folder = false; 
		item.empty = false; 
		item.description = _T( "" ); 
		item.icon = thumb_file; 

		node_added = arp_infos->AddNode( item, NULL ); 

		if( param_out != NULL )
		{
			*param_out = NULL; 
		}

		if( node_added == NULL )
		{
			ASSERT( FALSE ); 
			*param_out = NULL; 
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}
		else
		{
			if( param_out != NULL )
			{
				*param_out = ( PVOID )node_added; 
			}
		}

		return ret; 
	}

	LRESULT action_notify( action_ui_notify_type type, PVOID param )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *control; 
		CStdString tip; 
		notify_data *data_notify; 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		data_notify = ( notify_data* )param; 

		//DBG_BP(); 

_return:
		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
		return ret; 
	}

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create( _T( "theme_dlg.xml" ), ( UINT )0, NULL, &m_pm ); 
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
		LRESULT ret; 

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

	LRESULT get_theme_define( LPCTSTR theme_path, theme_define *theme_out )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CStdString conf_file; 

		ASSERT( theme_path != NULL ); 
		ASSERT( theme_out != NULL ); 

		log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 
		log_trace( ( MSG_INFO, "theme path is %ws\n", theme_path ) ); 

		_tcsncpy( theme_out->bk_file, theme_path, MAX_PATH ); 
		if( theme_out->bk_file[ MAX_PATH - 1 ] != _T( '\0' ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		_tcsncat( theme_out->bk_file, _T( "\\bk.jpg" ), MAX_PATH - _tcslen( theme_out->bk_file ) ); 

		if( theme_out->bk_file[ MAX_PATH - 1 ] != _T( '\0' ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		_tcsncpy( theme_out->thumb_file, theme_path, MAX_PATH ); 
		if( theme_out->thumb_file[ MAX_PATH - 1 ] != _T( '\0' ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		_tcsncat( theme_out->thumb_file, _T( "\\thumb.jpg" ), MAX_PATH - _tcslen( theme_out->thumb_file ) ); 

		if( theme_out->thumb_file[ MAX_PATH - 1 ] != _T( '\0' ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}


		conf_file = theme_path; 
		conf_file.Append( _T( "\\config.xml" ) ); 

		ret = load_theme_conf( conf_file.GetData(), &theme_out->setting ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

_return:
		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
		
		return ret; 
	}

	LRESULT on_theme_notify( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		LRESULT ret = ERROR_SUCCESS; 
		theme_define *theme_found = NULL; 
		CStdString theme_path; 
		CStdString bk_image; 
		CStdString thumb_image; 

		log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
		
		all_themes.resize( all_themes.size() + 1 ); 

		theme_found = &all_themes[ all_themes.size() - 1 ]; 
		
		theme_path = ( LPCTSTR )wParam; 
		theme_path.Append( _T( "\\" ) ); 
		theme_path.Append( ( LPCTSTR )lParam ); 

		ret = get_theme_define( theme_path.GetData(), theme_found ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "load the theme path %ws error 0x%0.8x\n", theme_path, ret ) ); 
			goto _return; 
		}

		thumb_image = _T( "filepath=\'" ); 
		thumb_image += theme_path; 
		bk_image = theme_path; 

		bk_image .Append( _T( "\\bk.jpg" ) ); 
		thumb_image.Append( _T( "\\thumb.jpg\'" ) ); 

		ret = add_theme_thumb_to_ui( thumb_image.GetData(), NULL, 0, NULL ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

_return:
		if( ret != ERROR_SUCCESS )
		{
			all_themes.erase( all_themes.begin() + all_themes.size() - 1 ); 
		}

		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
		return ret; 
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
        switch( uMsg )
		{
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
			case WM_THEME_NOTIFY:  lRes = on_theme_notify(uMsg, wParam, lParam, bHandled); break; 
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
    CButtonUI* m_pMinBtn;
	CTreeUI *arp_infos; 
	std::vector< theme_define > all_themes; 
};
