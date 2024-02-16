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

#ifndef __FILTER_MENU_H__
#define __FILTER_MENU_H__

#include "menu_wnd.h"
#include "trace_log_api.h"

typedef struct _FILTER_MENU_INFO
{
	CStdString time; 
}FILTER_MENU_INFO, *PFILTER_MENU_INFO; 

class CFilterMenu: public CMenuWnd 
{
public:
	CFilterMenu() 
	{
		wnd_size.cx = 0; 
		wnd_size.cy = 0; 
	}

	~CFilterMenu()
	{

	}

	void Notify(TNotifyUI& msg)
	{
		if( msg.sType == _T("itemselect") ) 
		{
			Close();
		}
		else if( msg.sType == _T("itemclick") ) 
		{
			if( m_pOwner ) 
			{
				m_pOwner->GetManager()->SendNotify(m_pOwner, msg.pSender->GetName(), 0, 0, true);
			}
		}
		else if( msg.sType == _T("valuechanged") ) 
		{
			CSliderUI *slider; 
			CStdString text; 
			INT32 _value; 
			
			slider = ( CSliderUI* )msg.pSender; 
			
			_value = slider->GetValue(); 
			
			text.SmallFormat( L"%d",_value ); 

			slider->SetText( text.GetData() ); 

			if( m_pOwner ) 
			{
				m_pOwner->GetManager()->SendNotify(m_pOwner, msg.pSender->GetName(), 0, _value, true);
			}
		}
	}


	void init_extra_ui() 
	{
		CSliderUI* pSilder; 

		pSilder = static_cast<CSliderUI*>(m_pm.FindControl(_T("slider_data_size")));
		ASSERT( pSilder != NULL ); 

		pSilder->SetChangeStep( 1 ); 
		pSilder->SetMinValue( MIN_TRACE_DATA_SIZE ); 
		pSilder->SetMaxValue( MAX_TRACE_DATA_SIZE ); 
	}

	void AdjustPostion()
	{
		CRect rcWnd;
		GetWindowRect(m_hWnd, &rcWnd);
		int nWidth = wnd_size.cx; //rcWnd.GetWidth();
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
		::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, wnd_size.cx, rcWnd.GetHeight(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	LRESULT get_wnd_width()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CStdString _ItemText; 
		INT32 _ret; 
		HDC wnd_dc = NULL; 
		//ULONG error_code; 
		HFONT def_font; 
		HFONT old_font = NULL; 
		SIZE text_size; 

		do 
		{
			if( m_pOwner == NULL )
			{
				ret = ERROR_NOT_READY; 
				break; 
			}

			//L"������ʾ\"%s\""
			_ItemText.Format( L"Highlight:\"%s\"", m_pOwner->GetText() ); 

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

			_ret = GetTextExtentPoint32( wnd_dc, _ItemText.GetData(), _ItemText.GetLength(), &text_size ); 
			if( _ret == FALSE )
			{
				dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "get the length for painting of the tect error\n" ); 
			}

			SelectFont( wnd_dc, old_font ); 

#define MENU_TEXT_BORDER_SPACE 60
#define MAX_TOOLTIP_WIDTH 420
#define MIN_TOOLTIP_WIDTH 160
			wnd_size.cx = text_size.cx + MENU_TEXT_BORDER_SPACE; 
			if( wnd_size.cx > MAX_TOOLTIP_WIDTH )
			{
				wnd_size.cx = MAX_TOOLTIP_WIDTH; 
			}
			else if( wnd_size.cx < MIN_TOOLTIP_WIDTH )
			{
				wnd_size.cx = MIN_TOOLTIP_WIDTH; 
			}
		}while( FALSE ); 

		if( wnd_dc != NULL )
		{
			::ReleaseDC( m_pm.GetPaintWindow(), wnd_dc ); 
		}

		return ret; 
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pm.Init(m_hWnd);
		CDialogBuilder builder;
		CControlUI* pRoot = builder.Create( cfg_file_name, ( UINT )0, NULL, &m_pm );
		ASSERT(pRoot && "Failed to parse XML"); 
		if( pRoot == NULL )
		{

		}

		get_wnd_width(); 

		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);
		m_pm.SetRoundCorner(3, 3);

		return 0;
	}

	LRESULT init_ui( FILTER_MENU_INFO *menu_info )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CStdString _ItemText; 
		CStdString OwnerText; 
		CControlUI *MenuItem; 
		CStdString LongestItemText; 
		LONG LongestItemTextLength; 

		init_extra_ui(); 

		do 
		{
			ASSERT( m_pOwner != NULL ); 

			if( m_pOwner == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			OwnerText = m_pOwner->GetText(); 
			if( OwnerText.GetLength() == 0 )
			{
				OwnerText = m_pOwner->GetUserData(); 
			}

			LongestItemTextLength = 0; 

			MenuItem = m_pm.FindControl( L"menu_item_search" ); 
			ASSERT( MenuItem != NULL ); 

			//L"����\"%s\""
			_ItemText.Format( L"Search online:\"%s\"", OwnerText.GetData() ); 
			MenuItem->SetText( _ItemText.GetData() ); 

			if( LongestItemTextLength <= _ItemText.GetLength() )
			{
				LongestItemTextLength = _ItemText.GetLength(); 
				LongestItemText = _ItemText; 
			}

			MenuItem = m_pm.FindControl( L"menu_item_include_it" ); 
			ASSERT( MenuItem != NULL ); 

			//L"����\"%s\""
			_ItemText.Format( L"Include:\"%s\"", OwnerText.GetData() ); 
			MenuItem->SetText( _ItemText.GetData() ); 

			if( LongestItemTextLength < _ItemText.GetLength() )
			{
				LongestItemTextLength = _ItemText.GetLength(); 
				LongestItemText = _ItemText; 
			}

			MenuItem = m_pm.FindControl( L"menu_item_exclude_it" ); 
			ASSERT( MenuItem != NULL ); 

			//L"�ų�\"%s\""
			_ItemText.Format( L"Exclude:\"%s\"", OwnerText.GetData() ); 

			MenuItem->SetText( _ItemText.GetData() ); 

			if( LongestItemTextLength < _ItemText.GetLength() )
			{
				LongestItemTextLength = _ItemText.GetLength(); 
				LongestItemText = _ItemText; 
			}

			MenuItem = m_pm.FindControl( L"menu_item_hilight_it" ); 
			ASSERT( MenuItem != NULL ); 

			//L"������ʾ\"%s\""
			_ItemText.Format( L"Highlight:\"%s\"", OwnerText.GetData() ); 

			MenuItem->SetText( _ItemText.GetData() ); 

			if( LongestItemTextLength < _ItemText.GetLength() )
			{
				LongestItemTextLength = _ItemText.GetLength(); 
				LongestItemText = _ItemText; 
			}

			if( menu_info->time.GetLength() != 0 )
			{
				MenuItem = m_pm.FindControl( L"menu_item_begin_time" ); 
				ASSERT( MenuItem != NULL ); 

				//L"��\"%s\"Ϊ��ʼʱ��"
				_ItemText.Format( L"\"%s\"=Begin time", menu_info->time.GetData() ); 
				MenuItem->SetText( _ItemText.GetData() ); 

				if( LongestItemTextLength < _ItemText.GetLength() )
				{
					LongestItemTextLength = _ItemText.GetLength(); 
					LongestItemText = _ItemText; 
				}

				MenuItem = m_pm.FindControl( L"menu_item_end_time" ); 
				ASSERT( MenuItem != NULL ); 

				//L"��\"%s\"Ϊ����ʱ��"
				_ItemText.Format( L"\"%s\"=End time", menu_info->time.GetData() ); 
				MenuItem->SetText( _ItemText.GetData() ); 

				if( LongestItemTextLength < _ItemText.GetLength() )
				{
					LongestItemTextLength = _ItemText.GetLength(); 
					LongestItemText = _ItemText; 
				}
			}

			{
				INT32 _ret; 
				HDC wnd_dc = NULL; 
				//ULONG error_code; 
				HFONT def_font; 
				HFONT old_font = NULL; 
				SIZE text_size; 
				
				do 
				{
					wnd_dc = GetDC( m_pm.GetPaintWindow() ); 
					
					if( wnd_dc == NULL )
					{
						dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "get dc for the paint window error\n" ); 
						break; 
					}

					def_font = m_pm.GetFont( 0 ); 
					if( def_font == NULL )
					{
						break; 
					}

					old_font = SelectFont( wnd_dc, def_font ); 

					if( old_font == NULL )
					{
						DBG_BP(); 
						break; 
					}

					_ret = GetTextExtentPoint32( wnd_dc, LongestItemText.GetData(), LongestItemTextLength, &text_size ); 
					if( _ret == FALSE )
					{
						dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "get the length for painting of the tect error\n" ); 
					}

					SelectFont( wnd_dc, old_font ); 

					wnd_size.cx = text_size.cx + MENU_TEXT_BORDER_SPACE; 
					if( wnd_size.cx > MAX_TOOLTIP_WIDTH )
					{
						wnd_size.cx = MAX_TOOLTIP_WIDTH; 
					}
					else if( wnd_size.cx < MIN_TOOLTIP_WIDTH )
					{
						wnd_size.cx = MIN_TOOLTIP_WIDTH; 
					}
				}while( FALSE ); 

				if( wnd_dc != NULL )
				{
					::ReleaseDC( m_pm.GetPaintWindow(), wnd_dc ); 
				}
			}
			AdjustPostion(); 

			m_pm.GetRoot()->SetFixedWidth( wnd_size.cx ); 

		}while( FALSE );
		
		//_return:
		return ret; 
	}

	LRESULT Init(CControlUI* pOwner, FILTER_MENU_INFO *menu_info, POINT pt )
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
			//m_menu_info = menu_info; 

			if( *cfg_file_name == L'\0' )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			paint_wnd = pOwner->GetManager()->GetPaintWindow(); 
			Create( paint_wnd, NULL, WS_POPUP, WS_EX_TOOLWINDOW ); 

			ret = init_ui( menu_info ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
			}

			HWND hWndParent = m_hWnd;
			while( ::GetParent(hWndParent) != NULL ) 
			{
				hWndParent = ::GetParent( hWndParent );
			}

			::ShowWindow(m_hWnd, SW_SHOW);
			::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
		} while ( FALSE );

		return ret; 

	}

public:
	SIZE wnd_size; 
};

#endif //__FILTER_MENU_H__