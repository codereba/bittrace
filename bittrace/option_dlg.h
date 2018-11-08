/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
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

#include "tab_builder.h"
#include "msg_box.h"
#include "common_api.h"
#include "common_config.h"

#ifdef LANG_EN
#define UNINSTALL_NEED_RESTART_TEXT L"Bitsafe is uninstalled, delete the directory(bittrace32.exe in it) to clean entirely."
#define MAIN_BITSAFE_STARTED_BALLOON_TIP_TEXT L"Bittrace aleady started" 
#define RUN_BITTRACE_NEED_RESTART L"Run bittrace need restart windows." 
#define BITTRACE_SUPPORT_OS_TIP_TEXT L"BITTRACE only support 32bit,64bit WINDOWS XP,WINDOWS 2003,WINDOWS VISTA,WINDOWS 7,WINDOWS 2008,WINDOWS2012, have not support other version of windows now."
#define CONFIG_SYMBOL_CACHE_PATH_TIP_TEXT L"Please input correct path for symbol cache"
#define SAVE_COMMON_CONFIG_ERROR_MESSAGE_TEXT L"Save common configuration error"
#define SAVE_COMMON_CONFIG_ERROR_MESSAGE_TEXT L"Save common configuration successfully"
#else
#define UNINSTALL_NEED_RESTART_TEXT L"比特跟踪卸载完成,关闭程序后删除程序目录完成全面卸载"
#define MAIN_BITSAFE_STARTED_BALLOON_TIP_TEXT L"比特跟踪已经启动" 
#define RUN_BITTRACE_NEED_RESTART L"运行比特安全需要重启系统." 
#define BITTRACE_SUPPORT_OS_TIP_TEXT L"当前BITTRACE支持32位,64位的WINDOWS XP,WINDOWS 2003,WINDOWS VISTA,WINDOWS 7,WINDOWS 2008,WINDOWS2012系统,暂时不支持其它系统。"
#define CONFIG_SYMBOL_CACHE_PATH_TIP_TEXT L"请输入正确的符号保存目录路径"
#define SAVE_COMMON_CONFIG_ERROR_MESSAGE_TEXT L"保存辅助设置失败"
#define SAVE_COMMON_CONFIG_ERROR_MESSAGE_TEXT L"保存辅助设置成功"
#endif //LANG_EN

typedef enum _option_type
{
	PROTECT_OPTION, 
	LANG_OPTION, 
	GENERAL_OPTION
} option_type; 

class option_dlg : public CWindowWnd, public INotifyUI
{
public:
	option_dlg( action_ui_notify *notifier = NULL ) 
	{
		ui_notify = notifier; 
	};

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("OptionDlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    }

	LRESULT change_to_local()
	{
#ifdef LANG_EN
		set_ctrl_text_from_name( &m_pm, L"help_btn", L"Analyzing" ); 
		set_ctrl_text_and_tooltip_from_name_ex( &m_pm, L"symbol_ok_btn", L"OK", L"Set symbol path" ); 
		set_ctrl_text_and_tooltip_from_name_ex( &m_pm, 
			L"symbol_label", 
			L"Symbol path:", 
			L"Debug symbol:The string and id for data structure and code in .dll,.sys,.exe, help to understand (code<=>data),if already have, input it or input one directory anywhere( select auto download from microsoft) that will be cache of the symbols downloaded from microsoft." ); 
		set_ctrl_tooltip_from_name( &m_pm, L"symbol_path", L"Symbol path" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"sym_browser", L"Browser..." ); 
		set_ctrl_text_from_name( &m_pm, L"txt_debug_symbol", 
			L"Debug symbol:The string and id for data structure and code in .dll,.sys,.exe, help to understand (code<=>data),if already have, input it or input one directory anywhere( select Auto download from microsoft) that will be cache of the symbols downloaded from microsoft." ); 
		set_ctrl_text_from_name( &m_pm, L"auto_download", L"Auto download from microsoft" ); 
		set_ctrl_text_and_tooltip_from_name_ex( &m_pm, L"txt_dbg_help", 
			L"DBGHELP.DLL path:", 
			L"dbghelp.dll can parse the debug information(symbol etc.) in .dll,.sys,.exe files, dbghelp is published with windows,but version maybe too old, so need to set dbghelp.dll path manually." ); 
		set_ctrl_tooltip_from_name( &m_pm, L"dbg_help_path", L"DBGHELP.DLL path" ); 
		set_ctrl_tooltip_from_name( &m_pm, L"btn_browser", L"Browser..." ); 
		set_ctrl_text_from_name( &m_pm, L"txt_desc", L"dbghelp.dll can parse the debug information(symbol etc.) in .dll,.sys,.exe files, dbghelp is published with windows,but version maybe too old, so need to set dbghelp.dll path manually." ); 
#endif //LANG_EN

		return ERROR_SUCCESS; 
	}


	void Init() 
    {
		INT32 ret = ERROR_SUCCESS; 
		COptionUI *option; 

        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		option = static_cast<COptionUI*>(m_pm.FindControl( _T( "help_btn" ) ) ); 
		ASSERT( option != NULL ); 

		option->Activate(); 

		change_to_local(); 

		on_analyze_tab_selected(); 

		if( ret != ERROR_SUCCESS )
		{
			Close(); 
		}

		return; 
	}

    void OnPrepare(TNotifyUI& msg) 
    {
    }

	LRESULT on_analyze_tab_selected()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CEditUI *edit; 

		edit = ( CEditUI * )m_pm.FindControl( _T( "symbol_path" ) ); 
		ASSERT( edit != NULL ); 

		edit->SetText( get_analyze_help_conf()->symbol_path ); 

		return ret; 
	}

	void Notify(TNotifyUI& msg)
	{
		CStdString name; 
		LRESULT ret = ERROR_SUCCESS; 
		LPCTSTR tmp_text; 

		if( msg.sType == _T( "windowinit" ) ) 
			OnPrepare(msg);
		else if( msg.sType == _T( "click" ) ) 
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

			name = msg.pSender->GetName(); 

			if( name == _T( "symbol_ok_btn" ) )
			{
				LRESULT ret; 
				CEditUI *edit; 
				COptionUI *option; 
				CStdString symbol_path; 
				ULONG flags; 

				do 
				{
					edit = ( CEditUI* )m_pm.FindControl( _T( "symbol_path" ) ); 
					ASSERT( edit != NULL ); 

					option = ( COptionUI* )m_pm.FindControl( _T( "auto_download" ) ); 
					ASSERT( option != NULL ); 

					symbol_path = edit->GetText(); 

					if( symbol_path.GetLength() == 0 )
					{
						tmp_text = CONFIG_SYMBOL_CACHE_PATH_TIP_TEXT; //L"请输入正确的符号保存目录路径"; 

						show_msg( GetHWND(), tmp_text ); 
						log_trace( ( MSG_ERROR, "!!!set current work mode failed \n" ) ); 
						break; 
					}

					ret = check_dir_exist( symbol_path ); 

					if( option->IsSelected() == true || ret != ERROR_SUCCESS )
					{
						flags = NO_TIP_DEFAULT_SYMBOL_CHECK | FORMAT_LOCAL_SYMBOL_PATH; 
					}
					else
					{
						flags = NO_TIP_DEFAULT_SYMBOL_CHECK; 
					}

					ret = set_analyze_help_conf( symbol_path.GetData(), NULL, flags ); 
					if( ret != ERROR_SUCCESS )
					{

					}

					ret = save_common_conf(); 

					if( ret != ERROR_SUCCESS )
					{
						tmp_text = SAVE_COMMON_CONFIG_ERROR_MESSAGE_TEXT; 

						show_msg( GetHWND(), tmp_text ); 
						log_trace( ( MSG_ERROR, "!!!set current work mode failed \n" ) ); 
					}
					else
					{
						tmp_text = SAVE_COMMON_CONFIG_ERROR_MESSAGE_TEXT; 

						show_msg( GetHWND(), tmp_text );  
					}
				}while( FALSE );

			}
			else if( name == _T( "sym_browser" ) )
			{
				CEditUI *edit; 
				CStdString symbol_path; 
				WCHAR file_name[ MAX_PATH ]; 

				do 
				{
					*file_name = _T( '\0' ); 

					edit = ( CEditUI* )m_pm.FindControl( _T( "symbol_path" ) ); 
					ASSERT( edit != NULL ); 

					ret = open_folder_dlg( ( LPWSTR )file_name, ARRAYSIZE( file_name ) ); 
					if( ret < 0 )
					{
						break; 
					}

					if( *file_name != L'\0' )
					{
						break; 
					}

					edit->SetText( file_name ); 
				}while( FALSE );
			}
		}

		else if( msg.sType == _T( "selectchanged" ) )
		{
			CTabLayoutUI* tab_container = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("frame")));
			name = msg.pSender->GetName(); 
			//LPCTSTR tmp_text; 

			ASSERT( tab_ctrl != NULL ); 

			__Trace( _T( "the set focus target control is %s \n" ), name ); 
			
			if( name == _T( "trans_btn" ) )
			{
				tab_container->SelectItem( 0 ); 
				on_analyze_tab_selected(); 
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
		}
		else if(msg.sType == _T("menu")) 
		{
		}

		return; 
	}

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		
		hide_sys_menu( GetHWND() ); 

		m_pm.Init(m_hWnd);
        CDialogBuilder builder;
		tabs_builder cb; 
        CControlUI* pRoot = builder.Create(_T("option_dlg.xml"), (UINT)0, &cb, &m_pm);
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
	CButtonUI* m_pMinBtn; 

	CTabLayoutUI *tab_ctrl; 

	action_ui_notify *ui_notify; 
};