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

#ifndef _SEARCH_DLG_H__
#define _SEARCH_DLG_H__

#include "list_ex_common.h"
#include "list_log.h"
#include "trace_frame.h"
#include "msg_box.h"

NTSTATUS CALLBACK load_action_filter_cond( ULONG action_cond_id, 
										  action_filter_info *filter_cond, 
										  LPCWSTR time, 
										  PVOID work_context, 
										  ULONG *state_out ); 

#define WM_ACTION_EVENT_FILTERING ( WM_USER + 1101 ) 

class search_dlg : public CWindowWnd, public INotifyUI, public IDialogBuilderCallback
{
public:
	search_dlg( trace_frame *frame )
	{
		_trace_frame = frame; 
	}
	
	~search_dlg()
	{
	}; 

	CControlUI* CreateControl(LPCTSTR pstrClass, CPaintManagerUI *pManager )
	{
		return NULL; 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T( "SEARCH_DLG" ); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    }

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    }
    
	LRESULT init_search_text_history(); 

	LRESULT WINAPI _Init(); 

    void OnPrepare(TNotifyUI& msg) 
    {
		_Init(); 
    }


	/***********************************
	系统的模块在内核中进行维护，当有事件发生时，使用如下的结构记录调用堆栈相关的信息：
	1.模块的名称:相对偏移量
	
	模块名称集在内核中维护，并且将其直接映射至RING3层。使用加锁的方式，对其进行保护性的读写。

	打印至缓存区后，将字符串保留下来，不再进行重复的打印。

	打印事件信息方法：
	1.事件信息为一个结构体，所有事件信息长度都统一进行分配：
	 缺点：内核消耗大
	 定位,读取事件的信息参数相对非常简单

	2.使用结构解析方法对不同长度的各个事件信息参数进行定位，同时设置相应的字符串输出。
	3.使用一种统一的方法将事件的信息集中输出为一条消息。
	***********************************/
	/*
    * 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
    */

	LRESULT on_search()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *ctrl; 
		CStdString key_word; 
		BOOLEAN up_direction; 

		ctrl = m_pm.FindControl( L"key_word_edit" ); 
		ASSERT( NULL != ctrl ); 

		key_word = ctrl->GetText(); 

		ctrl = m_pm.FindControl( L"search_dir" ); 
		ASSERT( NULL != ctrl ); 

		up_direction = ( ( COptionUI* )ctrl )->IsSelected(); 

		ret = _trace_frame->search_text( key_word, up_direction ); 
		if( ret == ERROR_NOT_FOUND )
		{
			dlg_ret_state ret_state; 
			LPCWSTR tmp_text = L"Search done"; //L"搜索完毕"; 

			ret = show_msg( NULL, tmp_text, &ret_state ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
			}
		}
		return ret; 
	}

	LRESULT on_key_down(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 

		bHandled = FALSE; 
		ASSERT( uMsg == WM_KEYDOWN ); 

		if( wParam == ( WPARAM )VK_F3 )
		{		
			on_search(); 
		}

		return ret; 
	}

    void Notify(TNotifyUI& msg)
    {
		CStdString name; 
        
		name = msg.pSender->GetName(); 

		if( msg.sType == _T("windowinit") ) 
		{
			OnPrepare(msg);
		}
		else if( msg.sType == _T( "return" ) )
		{
			on_search(); 
		}
		else if( msg.sType == _T("click") ) 
        {
            if( name == _T( "close_btn" ) )
            {
				ShowWindow( false, false ); 
				//Close(); 
                return; 
            }
 
			if( name == _T( "search_btn" ) )
			{
				on_search(); 
			}
			else if( name == _T( "cancel_btn" ) )
			{
				ShowWindow( false, false ); 
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

			pRoot = builder.Create( _T("search_log_dlg.xml"), 
				(UINT)0, 
				this, 
				&m_pm ); 

			ASSERT(pRoot && "Failed to parse XML");

			m_pm.AttachDialog(pRoot);
			m_pm.AddNotifier(this);

			set_ui_style( &m_pm ); 

			_Init(); 
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
			case WM_CHAR:         
				lRes = OnChar( uMsg, wParam, lParam, bHandled ); 
				break; 

			case WM_KEYDOWN:
				{
					lRes = on_key_down(uMsg, wParam, lParam, bHandled); 
					break;
				}
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

#endif //_SEARCH_DLG_H__