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

#ifndef __UI_TOOLTIP_H__
#define __UI_TOOLTIP_H__

#define DISAPPEAR_DELAY_TIME 3
#define TOOLTIP_TEXT_X_BORDER_SPACE 10
#define TOOLTIP_TEXT_Y_BORDER_SPACE 10
#define MAX_TOOLTIP_WIDTH 420
#define MIN_TOOLTIP_WIDTH 160
#define MAX_TOOLTIP_HEIGHT 260
#define MIN_TOOLTIP_HEIGHT 20

ULONG WINAPI tooltip_control_thread( PVOID param ); 

class CTooltipWindow : public CWindowWnd, public INotifyUI 
{
public:
	CTooltipWindow() 
	{
		wnd_size.cx = 0; 
		wnd_size.cy = 0; 

		ShowTime = 0; 
		mouse_move_on = FALSE; 
	}

	~CTooltipWindow()
	{
		if( m_hWnd != NULL )
		{
			Close(); 
		}
	}

	void Hide()
	{
		if( TRUE == IsWindowVisible( GetHWND() ) )
		{
			::ShowWindow( GetHWND(), SW_HIDE ); 
		}

		if( TRUE == IsWindowEnabled( GetHWND() ) )
		{
			::EnableWindow( GetHWND(), FALSE ); 
		}

		mouse_move_on = FALSE; 
		dbg_print( MSG_IMPORTANT, "hide tooltip properties\n" ); 
	}

	void AdjustPostion()
	{
		CRect rcWnd;
		GetWindowRect(m_hWnd, &rcWnd);
		int nWidth = wnd_size.cx; 
		int nHeight = rcWnd.GetHeight();
		rcWnd.left = m_ptPos.x - 5;
		rcWnd.top = m_ptPos.y - 5;
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
		::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, wnd_size.cx, wnd_size.cy/*rcWnd.GetHeight()*/, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	LRESULT get_wnd_width()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CStdString _ItemText; 
		INT32 _ret; 
		HDC wnd_dc = NULL; 
		HFONT def_font; 
		HFONT old_font = NULL; 
		SIZE text_size; 
		LONG text_height; 
		INT32 line_pos; 
		INT32 old_line_pos; 
		LONG max_line_len; 
		LONG line_len; 

		do 
		{
			if( m_pOwner == NULL )
			{
				ret = ERROR_NOT_READY; 
				break; 
			}

			wnd_dc = GetDC( m_pm.GetPaintWindow() ); 

			if( wnd_dc == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "get dc for the paint window error\n" ); 
				break; 
			}

			def_font = m_pm.GetFont( 0 ); 
			if( def_font == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			old_font = SelectFont( wnd_dc, def_font ); 

			if( old_font == NULL )
			{
				DBG_BP(); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_ItemText = m_OwnerText.GetData(); 

			max_line_len = 0; 
			text_height = 0; 
			old_line_pos = 0; 
			text_size.cy = 0; 
			text_size.cx = 0; 

#define NEW_LINE_STRING L"\r\n"
			for( ; ; )
			{
				line_pos = _ItemText.Find( NEW_LINE_STRING, old_line_pos ); 
				if( line_pos < 0 )
				{
					_ret = GetTextExtentPoint32( wnd_dc, _ItemText.GetData() + old_line_pos, _ItemText.GetLength() - old_line_pos, &text_size ); 
					if( _ret == FALSE )
					{
						dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "get the length for painting of the tect error\n" ); 
						text_size.cx = 0; 
					}
	
					line_len = text_size.cx; 

					if( text_size.cx > MAX_TOOLTIP_WIDTH )
					{
						text_height += ( ( text_size.cx + MAX_TOOLTIP_WIDTH - 1 ) / MAX_TOOLTIP_WIDTH ) * text_size.cy; 
					}
					else
					{
						text_height += text_size.cy; 
					}

					if( line_len > ( INT32 )max_line_len )
					{
						max_line_len =  line_len; 
					}
					
					break; 
				}

				_ret = GetTextExtentPoint32( wnd_dc, _ItemText.GetData() + old_line_pos, line_pos - old_line_pos, &text_size ); 
				if( _ret == FALSE )
				{
					dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "get the length for painting of the tect error\n" ); 
					text_size.cx = 0; 
				}

				line_len = text_size.cx; 
				if( text_size.cx > MAX_TOOLTIP_WIDTH )
				{
					text_height += ( ( text_size.cx + MAX_TOOLTIP_WIDTH - 1 ) / MAX_TOOLTIP_WIDTH ) * text_size.cy; 
				}
				else
				{
					text_height += text_size.cy; 
				}

				if( line_len > ( INT32 )max_line_len )
				{
					max_line_len = line_len; 
				}

				old_line_pos = line_pos + ARRAYSIZE( NEW_LINE_STRING ) - 1; 
			}

			SelectFont( wnd_dc, old_font ); 

			if( max_line_len > MAX_TOOLTIP_WIDTH - TOOLTIP_TEXT_X_BORDER_SPACE )
			{
				wnd_size.cx = MAX_TOOLTIP_WIDTH; 
			}
			else if( max_line_len < MIN_TOOLTIP_WIDTH - TOOLTIP_TEXT_X_BORDER_SPACE )
			{
				wnd_size.cx = MIN_TOOLTIP_WIDTH; 
			}
			else
			{
				wnd_size.cx = max_line_len + TOOLTIP_TEXT_X_BORDER_SPACE; 
			}

			{
				dbg_print( MSG_IMPORTANT, "the text height for tooltip text is %u\n", text_height ); 

				wnd_size.cy = text_height + TOOLTIP_TEXT_Y_BORDER_SPACE; 
				if( wnd_size.cy > MAX_TOOLTIP_HEIGHT )
				{
					wnd_size.cy = MAX_TOOLTIP_HEIGHT; 
				}
				else if( wnd_size.cy < MIN_TOOLTIP_HEIGHT )
				{
					wnd_size.cy = MIN_TOOLTIP_HEIGHT; 
				}
			}
		}while( FALSE ); 

		if( wnd_dc != NULL )
		{
			::ReleaseDC( m_pm.GetPaintWindow(), wnd_dc ); 
		}

		return ret; 
	}

	LRESULT _get_wnd_width()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CStdString _ItemText; 
		INT32 _ret; 
		HDC wnd_dc = NULL; 
		HFONT def_font; 
		HFONT old_font = NULL; 
		SIZE text_size; 
		ULONG line_count; 
		INT32 line_pos; 
		INT32 old_line_pos; 
		ULONG max_line_len; 
		INT32 max_line_pos; 
		ULONG line_len; 

		do 
		{
			if( m_pOwner == NULL )
			{
				ret = ERROR_NOT_READY; 
				break; 
			}

			_ItemText = m_OwnerText.GetData(); 


			max_line_len = 0; 
			line_count = 1; 
			old_line_pos = 0; 
			max_line_pos = -1; 

			wnd_dc = GetDC( m_pm.GetPaintWindow() ); 

			if( wnd_dc == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "get dc for the paint window error\n" ); 
				break; 
			}

			def_font = m_pm.GetFont( 0 ); 
			if( def_font == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			old_font = SelectFont( wnd_dc, def_font ); 

			if( old_font == NULL )
			{
				DBG_BP(); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

#define NEW_LINE_STRING L"\r\n"
			for( ; ; )
			{
				line_pos = _ItemText.Find( NEW_LINE_STRING, old_line_pos ); 
				if( line_pos < 0 )
				{
					line_len = _ItemText.GetLength() - old_line_pos; 
					_ret = GetTextExtentPoint32( wnd_dc, _ItemText.GetData() + max_line_pos, max_line_len, &text_size ); 
					if( _ret == FALSE )
					{
						dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "get the length for painting of the tect error\n" ); 
					}

					if( line_len > ( INT32 )max_line_len )
					{
						max_line_pos = old_line_pos; 
						max_line_len =  line_len; 
					}

					break; 
				}

				line_len = line_pos - old_line_pos; 

				_ret = GetTextExtentPoint32( wnd_dc, _ItemText.GetData() + max_line_pos, max_line_len, &text_size ); 
				if( _ret == FALSE )
				{
					dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "get the length for painting of the tect error\n" ); 
				}

				if( line_len > ( INT32 )max_line_len )
				{
					max_line_pos = old_line_pos; 
					max_line_len = line_len; 
				}

				line_count += 1; 
				old_line_pos = line_pos + ARRAYSIZE( NEW_LINE_STRING ) - 1; 
			}

			ASSERT( max_line_pos != -1 ); 

			SelectFont( wnd_dc, old_font ); 

			wnd_size.cx = text_size.cx + TOOLTIP_TEXT_X_BORDER_SPACE; 
			if( wnd_size.cx > MAX_TOOLTIP_WIDTH )
			{
				wnd_size.cx = MAX_TOOLTIP_WIDTH; 
				line_count += 1; //这种计算方式也不够精确，最精确的方式还是对每一行都计算实际长度，再计算折行。
			}
			else if( wnd_size.cx < MIN_TOOLTIP_WIDTH )
			{
				wnd_size.cx = MIN_TOOLTIP_WIDTH; 
			}

			{
				dbg_print( MSG_IMPORTANT, "the line count for tooltip text is %u\n", line_count ); 

				wnd_size.cy = text_size.cy * line_count + TOOLTIP_TEXT_Y_BORDER_SPACE; 
				if( wnd_size.cy > MAX_TOOLTIP_HEIGHT )
				{
					wnd_size.cy = MAX_TOOLTIP_HEIGHT; 
				}
				else if( wnd_size.cy < MIN_TOOLTIP_HEIGHT )
				{
					wnd_size.cy = MIN_TOOLTIP_HEIGHT; 
				}
			}
		}while( FALSE ); 

		if( wnd_dc != NULL )
		{
			::ReleaseDC( m_pm.GetPaintWindow(), wnd_dc ); 
		}

		return ret; 
	}

	LRESULT OnMouseLeave( UINT32 uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE; 
		
		return 0; 
	}

	/***************************************************************************************
	如何使窗体消失：
	1.定时器
	2.鼠标离开窗体
		当鼠标离开这个控制的区域时，不应该关闭掉。
		应该是两种情况触发关闭：
			1.鼠标离开这个区域3秒后
			2.鼠标离开整个窗体后

	定时器的处理过程：
	1.
	***************************************************************************************/
	

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 Ret; 
		POINT Pos; 
		RECT WindowRect; 

		do 
		{
			bHandled = FALSE; 

			Pos.x = GET_X_LPARAM( lParam ); 
			Pos.y = GET_Y_LPARAM( lParam ); 

			Ret = GetWindowRect( GetHWND(), &WindowRect ); 

			if( Ret == FALSE )
			{
				break; 
			}

			Ret = ClientToScreen( GetHWND(), &Pos ); 
			if( Ret == FALSE )
			{
				break; 
			}

			if( PtInRect( &WindowRect, Pos ) == TRUE )
			{
				if( FALSE == mouse_move_on )
				{
					mouse_move_on = TRUE; 
				}

			}
			else
			{
				mouse_move_on = FALSE; 
			}
		}while( FALSE ); 

		return 0; 
	}
	
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pm.Init(m_hWnd);
		CDialogBuilder builder;
		CControlUI* pRoot = builder.Create( L"tooltip.xml", ( UINT )0, NULL, &m_pm );
		ASSERT(pRoot && "Failed to parse XML"); 
		if( pRoot == NULL )
		{

		}

		m_pm.AttachDialog( pRoot ); 
		m_pm.AddNotifier( this ); 
		m_pm.SetRoundCorner( 3, 3 ); 

		ShowTime = GetTickCount(); 
		return 0;
	}

	LRESULT init_ui( LPCWSTR text )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CStdString _ItemText; 
		CRichEditUI *tip; 

		do 
		{
			ASSERT( m_pOwner != NULL ); 

			if( m_pOwner == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( text != NULL )
			{
				m_OwnerText = text; 
			}
			else
			{
				m_OwnerText = L""; 
			}

			if( m_OwnerText.GetLength() == 0 )
			{
				m_OwnerText = m_pOwner->GetText(); 
				if( m_OwnerText.GetLength() == 0 )
				{
					m_OwnerText = m_pOwner->GetToolTip(); 
				}
			}

			get_wnd_width(); 

			tip = ( CRichEditUI* )m_pm.FindControl( L"tip" ); 
			ASSERT( tip != NULL ); 

			tip->SetText( m_OwnerText ); 
			
			{
				SIZE pos; 
				
				pos.cx = 0; 
				pos.cy = 0; 

				tip->SetScrollPos( pos ); 
			}
			
			AdjustPostion(); 

		}while( FALSE );

		return ret; 
	}

	LRESULT Init( CControlUI* pOwner, POINT pt, LPCWSTR text = NULL )
	{
		LRESULT ret; 
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

			if( m_hWnd == NULL )
			{
				paint_wnd = pOwner->GetManager()->GetPaintWindow(); 
				Create( paint_wnd, NULL, WS_POPUP, WS_EX_TOOLWINDOW ); 
			}

			ret = init_ui( text ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
			}

			HWND hWndParent = m_hWnd; 
			while( ::GetParent( hWndParent ) != NULL ) 
			{
				hWndParent = ::GetParent( hWndParent );
			}

			if( FALSE == IsWindowVisible( GetHWND() ) )
			{
				::ShowWindow( m_hWnd, SW_SHOW ); 
			}

			if( FALSE == IsWindowEnabled( GetHWND() ) )
			{
				::EnableWindow( GetHWND(), TRUE ); 
			}

			dbg_print( MSG_IMPORTANT, "show tooltip\n" ); 
			::SendMessage( hWndParent, WM_NCACTIVATE, TRUE, 0L ); 


		} while ( FALSE );

		return ret; 

	}

	LPCTSTR GetWindowClassName() const 
	{ 
		return _T( "TooltipWindow"); 
	} 

		void OnFinalMessage( HWND /*hWnd*/ ) 
		{} 

		void Notify( TNotifyUI& msg )
		{
			if( msg.sType == _T("itemselect") ) 
			{
			}
			else if( msg.sType == _T("itemclick") ) 
			{
			}
		}


		LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
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
	case WM_KEYDOWN:       lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
	case WM_MOUSEWHEEL:    break;
	case WM_MOUSEMOVE:		lRes = OnMouseMove( uMsg, wParam, lParam, bHandled ); break; 
	

	case WM_MOUSELEAVE:     lRes = OnMouseLeave( uMsg, wParam, lParam, bHandled ); break; 
	case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
	default:
		bHandled = FALSE;
			}
			if( bHandled ) return lRes;
			if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
			return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		}

public:
	BOOLEAN mouse_move_on; 
	SIZE wnd_size; 
	CPaintManagerUI m_pm;
	CControlUI* m_pOwner;
	CStdString m_OwnerText; 
	POINT m_ptPos; 
	ULONG ShowTime; 
};

LRESULT WINAPI start_tooltip_control( CTooltipWindow *tooltip ); 
LRESULT WINAPI stop_tooltip_control(); 

#endif //__UI_TOOLTIP_H__