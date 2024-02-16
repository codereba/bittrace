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

#ifndef _LOAD_LOG_DLG_H__
#define _LOAD_LOG_DLG_H__

#include "event_memory_store.h"
#include "trace_frame.h"
#include "msg_box.h"

class load_log_dlg : public CWindowWnd, public INotifyUI, public IDialogBuilderCallback
{
public:
	load_log_dlg()
	{
		_trace_frame = NULL; 
	}

	//load_log_dlg( CPaintManagerUI* pManager); 
	
	~load_log_dlg()
	{
	}; 

	CControlUI* CreateControl(LPCTSTR pstrClass, CPaintManagerUI *pManager )
	{
		return NULL; 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T( "LOAD_LOG_DLG" ); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    }

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    }
    
	LRESULT init_search_text_history(); 

    void OnPrepare(TNotifyUI& msg) 
    {
    }

	LRESULT on_load_log()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *ctrl; 
		CStdString file_path; 

		ctrl = m_pm.FindControl( L"file_path_edit" ); 
		ASSERT( NULL != ctrl ); 

		file_path = ctrl->GetText(); 

		ret = load_log_from_db( file_path ); 
		if( ret == ERROR_SUCCESS )
		{
			dlg_ret_state ret_state; 
			LPCWSTR tmp_text = L"Event loaded"; 

			ret = show_msg( NULL, tmp_text, &ret_state ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
			}
		}

		return ret; 
	}

    void Notify(TNotifyUI& msg)
    {
		LRESULT ret; 
		CStdString name; 
        
		name = msg.pSender->GetName(); 

		if( msg.sType == _T("windowinit") ) 
		{
			OnPrepare(msg);
		}
		else if( msg.sType == _T( "return" ) )
		{
			on_load_log(); 
		}
		else if( msg.sType == _T("click") ) 
        {
            if( name == _T( "close_btn" ) )
            {
				Close(); 
				return; 
			}
			else if( name == _T( "sel_btn" ) )
			{
				WCHAR file_name[ MAX_PATH ]; 
				CEditUI *file_path_edit; 
				WCHAR app_path[ MAX_PATH ]; 
				ULONG cc_ret_len; 

				do 
				{
					ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					app_path[ cc_ret_len - 1 ] = L'\0'; 

					*file_name = _T( '\0' ); 

					//_ret = SetCurrentDirectoryW( app_path ); 
					//if( FALSE == _ret )
					//{

					//}

					ret = open_file_dlg( TRUE, NULL, file_name, NULL, app_path ); 
					if( ret < 0 )
					{
						break; 
					}

					file_path_edit = ( CEditUI* )m_pm.FindControl( _T( "file_path_edit" ) ); 
					ASSERT( file_path_edit != NULL ); 

					file_path_edit->SetText( file_name ); 
				}while( FALSE );

			}
    //        else if( msg.pSender == m_pMinBtn ) 
    //        { 
    //            SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
    //            return; 
    //        }
    //        else if( msg.pSender == m_pMaxBtn ) 
    //        { 
    //            SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
    //        }
    //        else 
				//if( msg.pSender == m_pRestoreBtn ) 
    //        { 
    //            SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
    //        }
			//else 
			if( name == _T( "load_btn" ) )
			{
				on_load_log(); 
				Close(); 
			}
			else if( name == _T( "cancel_btn" ) )
			{
				Close(); 
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
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		LRESULT ret = ERROR_SUCCESS; 
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		CDialogBuilder builder;
        CControlUI* pRoot; 

		do 
		{
			styleValue &= ~WS_CAPTION;
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			m_pm.Init(m_hWnd);

			//m_pm.SetTransparent(100);

			pRoot = builder.Create( _T("load_log_dlg.xml"), 
				(UINT)0, 
				this, 
				&m_pm ); 

			ASSERT(pRoot && "Failed to parse XML");

			m_pm.AttachDialog(pRoot);
			m_pm.AddNotifier(this);

			set_ui_style( &m_pm ); 
		}while( FALSE );

		return ret;
    }

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 
		
		bHandled = TRUE;
		
		do 
		{
			switch( wParam )
			{
			case VK_ESCAPE:
				Close(); 
				break; 
			default: 
				bHandled = FALSE; 
				break; 
			}
		}while( FALSE );
		
		return ret; 
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

		//log_trace( ( MSG_INFO, "main window %s msg is %u[0x%0.8x], wparam is 0x%0.8x, lparam is 0x%0.8x\n", __FUNCTION__, uMsg, uMsg, wParam, lParam ) ); 

		//if( uMsg == WM_NOTIFY 
		//	|| uMsg == WM_COMMAND )
		//{
		//	DebugBreak(); 
		//}

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
			case WM_CHAR:         
				lRes = OnChar( uMsg, wParam, lParam, bHandled ); 
				break; 
				//case WM_ACTION_EVENT_LOG:
			//	{
			//		lRes = on_action_event_log(uMsg, wParam, lParam, bHandled); 
			//		break;
			//	}
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
	trace_frame *_trace_frame; 
};

#endif //_LOAD_LOG_DLG_H__