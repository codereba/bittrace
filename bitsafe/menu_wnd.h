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
 
#ifndef __MENU_WND_H__
#define __MENU_WND_H__

class CMenuWnd : public CWindowWnd, public INotifyUI
{
public:
	CMenuWnd() : m_pOwner( NULL ) 
	{
		*cfg_file_name = L'\0'; 
	};
	
	LRESULT set_cfg_file( LPCTSTR cfg_file )
	{
		LRESULT ret = ERROR_SUCCESS; 
		ASSERT( cfg_file != NULL ); 

		_tcsncpy( cfg_file_name, cfg_file, MAX_PATH ); 
		if( cfg_file_name[ MAX_PATH - 1 ] != L'\0' )
		{
			//cfg_file_name[ MAX_PATH - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_TOO_SMALL; 
		}

		return ret; 
	}

	LRESULT init_ui()
	{
		LRESULT ret = ERROR_SUCCESS; 
		
		CListUI* pList = static_cast<CListUI*>(m_pOwner); 

		int nSel = pList->GetCurSel();

		if( nSel < 0 ) 
		{
			CControlUI* pControl = m_pm.FindControl(_T("menu_Delete"));
			
			if( pControl ) 
			{
				pControl->SetEnabled(false);
			}
		}

		AdjustPostion(); 

//_return:
		return ret; 
	}

	LRESULT Init(CControlUI* pOwner, POINT pt ) 
	{
		LRESULT ret = ERROR_SUCCESS; 
		HWND paint_wnd; 

		do 
		{
			if( pOwner == NULL ) 
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			m_pOwner = pOwner;
			m_ptPos = pt;

			if( *cfg_file_name == L'\0' )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			paint_wnd = pOwner->GetManager()->GetPaintWindow(); 
			Create( paint_wnd, NULL, WS_POPUP, WS_EX_TOOLWINDOW ); 

			init_ui(); 

			HWND hWndParent = m_hWnd;
			while( ::GetParent(hWndParent) != NULL ) 
			{
				hWndParent = ::GetParent( hWndParent );
			}

			::ShowWindow(m_hWnd, SW_SHOW);
			::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
		}while( FALSE );

		return ret; 
	}

	virtual void AdjustPostion() {
		CRect rcWnd;
		GetWindowRect(m_hWnd, &rcWnd);
		int nWidth = rcWnd.GetWidth();
		int nHeight = rcWnd.GetHeight();
		rcWnd.left = m_ptPos.x;
		rcWnd.top = m_ptPos.y;
		rcWnd.right = rcWnd.left + nWidth;
		rcWnd.bottom = rcWnd.top + nHeight;
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CRect rcWork = oMonitor.rcWork;

		if( rcWnd.bottom > rcWork.bottom ) {
			if( nHeight >= rcWork.GetHeight() ) {
				rcWnd.top = 0;
				rcWnd.bottom = nHeight;
			}
			else {
				rcWnd.bottom = rcWork.bottom;
				rcWnd.top = rcWnd.bottom - nHeight;
			}
		}
		if( rcWnd.right > rcWork.right ) {
			if( nWidth >= rcWork.GetWidth() ) {
				rcWnd.left = 0;
				rcWnd.right = nWidth;
			}
			else {
				rcWnd.right = rcWork.right;
				rcWnd.left = rcWnd.right - nWidth;
			}
		}
		::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(), SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	LPCTSTR GetWindowClassName() const 
	{ 
		return _T( "MenuWnd"); 
	}; 

	void OnFinalMessage(HWND /*hWnd*/) { delete this; };

	void Notify(TNotifyUI& msg)
	{
		if( msg.sType == _T("itemselect") ) 
		{
			Close();
		}
		else if( msg.sType == _T("itemclick") 
			|| msg.sType == _T("valuechanged") ) 
		{
			if( m_pOwner ) 
			{
				m_pOwner->GetManager()->SendNotify(m_pOwner, msg.pSender->GetName(), 0, 0, true);
			}
		}
	}

	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pm.Init(m_hWnd);
		CDialogBuilder builder;
		CControlUI* pRoot = builder.Create( cfg_file_name, ( UINT )0, NULL, &m_pm );
		ASSERT(pRoot && "Failed to parse XML"); 
		if( pRoot == NULL )
		{
			
		}

		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);
		m_pm.SetRoundCorner(3, 3);

		return 0;
	}

	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if( m_hWnd != (HWND) wParam ) 
		{
			//�������ʹ���첽�ķ�ʽ�����ܻἤ��2��WM_CLOSE��Ϣ��
			//PostMessage(WM_CLOSE);
			Close(); 
		}
		return 0;
	}

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if( wParam == VK_ESCAPE ) 
		{
			Close();
		}
		
		return 0;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
			CRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch( uMsg ) {
case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
case WM_KILLFOCUS:     lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
case WM_KEYDOWN:       lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
case WM_MOUSEWHEEL:    break;
case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
default:
	bHandled = FALSE;
		}
		if( bHandled ) return lRes;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

public:
	CPaintManagerUI m_pm; 
	CControlUI* m_pOwner; 
	POINT m_ptPos;
	TCHAR cfg_file_name[ MAX_PATH ]; 
};

#endif //__MENU_WND_H__