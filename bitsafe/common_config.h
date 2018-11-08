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

#ifndef COLORSKIN_HPP
#define COLORSKIN_HPP

#include "ui_config.h"
#include <vector>

//CRITICAL_SECTION ui_setting_lock; 
#define BITSAFE_UI_CONF_FILE _T( "data8.dat" )

#define WM_THEME_NOTIFY ( WM_USER + 1001 )
LRESULT CALLBACK on_themes_found( LPCTSTR root_path, WIN32_FIND_DATA *find_data, PVOID context ); 

typedef LRESULT ( CALLBACK *on_ui_conf_loaded )( PVOID context ); 

LRESULT get_common_conf_file_name( WCHAR *conf_file_path, ULONG name_len ); 
LRESULT load_common_conf( on_ui_conf_loaded ui_loaded_callback, PVOID context ); 
LRESULT save_common_conf(); 
LRESULT load_theme_conf( LPCWSTR theme_conf_file, theme_setting *theme_out ); 
LRESULT load_theme_setting( CPaintManagerUI *main_wnd_pm ); 
LRESULT load_all_themes( file_found_callback on_file_found, PVOID context ); 
LRESULT get_theme_define_from_path( LPCTSTR theme_path, theme_define *theme_found ); 
LRESULT get_theme_define( LPCTSTR theme_path, theme_define *theme_out ); 

inline VOID make_abs_file_path( CStdString &file_name_out, LPCTSTR file_name )
{
	ASSERT( file_name != NULL ); 
	file_name_out = _T( "filepath=\'" ); 
	file_name_out += file_name; 
	file_name_out.Append( _T( "\'" ) ); 
}

inline LRESULT _change_skin_theme( CPaintManagerUI *pm, theme_define *cur_theme )
{
	LRESULT ret = ERROR_SUCCESS; 
	CControlUI *ctrl; 
	CStdString bk_file; 

	ASSERT( cur_theme != NULL ); 
	ASSERT( pm != NULL ); 

	if( *cur_theme->bk_file == 0 )
	{
		ASSERT( FALSE ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	make_abs_file_path( bk_file, cur_theme->bk_file ); 

	ctrl = pm->FindControl( _T( "form_layout" ) ); 
	ctrl->SetBkImage( bk_file.GetData() ); 

_return: 
	return ret; 
}

class ColorSkinWindow : public CWindowWnd, public INotifyUI
{
public:
	ColorSkinWindow();

	LPCTSTR GetWindowClassName() const;

	virtual void OnFinalMessage(HWND hWnd);

	void Notify(TNotifyUI& msg);

	void Init();

	virtual CStdString GetSkinFile();

	virtual CStdString GetSkinFolder();

	virtual LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	RECT parent_window_rect_;
	CPaintManagerUI paint_manager; 
};

class ui_custom_dlg : public CWindowWnd, public INotifyUI
{
public:
	ui_custom_dlg( CPaintManagerUI *main_wnd_mp = NULL ) 
	{
		m_main_wnd_pm = main_wnd_mp; 
	}

	~ui_custom_dlg()
	{
		all_themes.clear(); 
	}

	LRESULT change_skin_theme( ULONG *theme_sel )
	{
		LRESULT ret = ERROR_SUCCESS; 
		//CControlUI *ctrl; 
		INT32 cur_sel; 
		theme_define *cur_theme; 
		CStdString bk_file; 

		if( theme_sel != NULL )
		{
			*theme_sel = 0; 
		}

		cur_sel = theme_infos->GetCurSel(); 
		if( cur_sel < 0 )
		{
			ret = ERROR_NOT_READY; 
			goto _return; 
		}

		if( ( ULONG )cur_sel > all_themes.size() )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			ASSERT( FALSE ); 
			goto _return; 
		}

		cur_theme = &all_themes[ cur_sel ]; 
		if( *cur_theme->bk_file == 0 )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		ret = _change_skin_theme( m_main_wnd_pm, cur_theme ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}

		if( theme_sel != NULL )
		{
			*theme_sel = cur_sel; 
		}

_return: 
		return ret; 
	}
	
	LPCTSTR GetWindowClassName() const 
	{ 
		return _T( "ui_custom_dlg" ); 
	}

	UINT GetClassStyle() const 
	{ 
		return /*UI_CLASSSTYLE_FRAME | */CS_DBLCLKS; 
	}

	void OnFinalMessage( HWND /*hWnd*/) 
	{ 
	
	}

	void Init() 
	{
		LPCWSTR tmp_text; 
		CControlUI *ctrl; 
		CSliderUI *slider; 

		ctrl = m_pm.FindControl( _T( "skin_btn" ) ); 
		ASSERT( ctrl != NULL ); 


		ctrl = m_pm.FindControl( _T( "slider_l" ) ); 
		ASSERT( ctrl != NULL ); 
		ctrl->SetEnabled( false ); 
		ctrl->SetVisible( false ); 

		ctrl = m_pm.FindControl( _T( "slider_s" ) ); 
		ASSERT( ctrl != NULL ); 

		ctrl->SetEnabled( false ); 
		ctrl->SetVisible( false ); 

		ctrl = m_pm.FindControl( _T( "slider_h" ) ); 
		ASSERT( ctrl != NULL ); 
		ctrl->SetEnabled( false ); 
		ctrl->SetVisible( false ); 

		tmp_text = L"Tranparent"; //_get_string_by_id( TEXT_SKIN_CUSTOM_TRANPARENT, _T( "透明度:" ) ); 

		ctrl = m_pm.FindControl( _T( "lbl_alpha" ) ); 
		ASSERT( ctrl != NULL ); 

		ctrl->SetText( tmp_text ); 

		slider = ( CSliderUI* )m_pm.FindControl( _T( "slider_alpha" ) ); 
		ASSERT( slider != NULL ); 

		slider->SetValue( ( int )get_tranparent() ); 

		theme_infos = static_cast< CImageListUI* >( m_pm.FindControl( _T( "themes_list" ) ) ); 
		ASSERT( theme_infos != NULL ); 

		set_ctrl_text_from_name( &m_pm, _T( "skin_btn" ), _T( "Customize" ) ); 

		if( theme_infos != NULL)
		{
			if( theme_infos->GetSubItemCount( UI_LIST_CONTAINER_ELEMENT_CLASS_ID ) > 0 )
			{
				theme_infos->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID );
			}
		}
	}

	bool OnHChanged( void* param ) 
	{
		TNotifyUI* pMsg = (TNotifyUI*)param;
		if( pMsg->sType == _T( "valuechanged" ) ) 
		{
			short H, S, L;
			CPaintManagerUI::GetHSL(&H, &S, &L);
			CPaintManagerUI::SetHSL(true, (static_cast<CSliderUI*>(pMsg->pSender))->GetValue(), S, L);
		}
		return true;
	}

	bool OnSChanged( void* param ) 
	{
		TNotifyUI* pMsg = (TNotifyUI*)param;
		if( pMsg->sType == _T( "valuechanged" ) ) 
		{
			short H, S, L;
			CPaintManagerUI::GetHSL(&H, &S, &L);
			CPaintManagerUI::SetHSL(true, H, (static_cast<CSliderUI*>(pMsg->pSender))->GetValue(), L);
		}
		return true;
	}

	bool OnLChanged( void* param ) 
	{
		TNotifyUI* pMsg = (TNotifyUI*)param;
		if( pMsg->sType == _T("valuechanged") ) 
		{
			short H, S, L;
			CPaintManagerUI::GetHSL(&H, &S, &L);
			CPaintManagerUI::SetHSL(true, H, S, (static_cast<CSliderUI*>(pMsg->pSender))->GetValue());
		}
		return true;
	}

	bool OnAlphaChanged(void* param) 
	{
		TNotifyUI* pMsg = (TNotifyUI*)param; 
		ULONG alpha_val; 

		alpha_val = ( static_cast<CSliderUI*>(pMsg->pSender))->GetValue(); 
		
		ASSERT( alpha_val >= 0 && alpha_val <= 256 ); 

		if( pMsg->sType == _T("valuechanged") ) 
		{
			m_pm.SetTransparent( alpha_val );
		
			if( m_main_wnd_pm	!= NULL )
			{
				m_main_wnd_pm->SetTransparent( alpha_val ); 
			}

			set_transparent( alpha_val ); 
		}
		return true;
	}

	void OnPrepare() 
	{
		CSliderUI* pSilder; 
		
		pSilder = static_cast<CSliderUI*>(m_pm.FindControl(_T("slider_alpha")));
		ASSERT( pSilder != NULL ); 
		pSilder->OnNotify += MakeDelegate(this, &ui_custom_dlg::OnAlphaChanged);
		
		pSilder = static_cast<CSliderUI*>(m_pm.FindControl(_T("slider_h")));
		ASSERT( pSilder != NULL ); 
		pSilder->OnNotify += MakeDelegate(this, &ui_custom_dlg::OnHChanged);
		
		pSilder = static_cast<CSliderUI*>(m_pm.FindControl(_T("slider_s")));
		ASSERT( pSilder != NULL ); 
		pSilder->OnNotify += MakeDelegate(this, &ui_custom_dlg::OnSChanged);

		pSilder = static_cast<CSliderUI*>(m_pm.FindControl(_T("slider_l")));
		ASSERT( pSilder != NULL ); 
		pSilder->OnNotify += MakeDelegate(this, &ui_custom_dlg::OnLChanged); 

		if( theme_infos->GetSubItemCount( UI_LIST_CONTAINER_ELEMENT_CLASS_ID ) > 0 )
		{
			theme_infos->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID ); 
		}

		load_all_themes( on_themes_found, ( PVOID )this );  
	}

	void Notify(TNotifyUI& msg)
	{
		LRESULT ret = ERROR_SUCCESS; 

		if( msg.sType == _T( "windowinit" ) ) 
		{
			OnPrepare(); 
		}
		else if( msg.sType == _T("click") ) 
		{	
			if( msg.pSender->GetName() == _T( "skin_btn" ) ) 
			{
				ULONG theme_sel; 
				ret = change_skin_theme( &theme_sel ); 
				if( ret == ERROR_SUCCESS )
				{
					ASSERT( theme_sel >= 0 && theme_sel < all_themes.size() ); 

					set_theme_no( ( BYTE )theme_sel ); 
				}
			}
			else if( msg.pSender->GetName() == _T( "close_btn" ) )
			{
				Close(); 
			}
		}
	}

	LRESULT add_theme_thumb_to_ui( 
		LPCTSTR thumb_file, 
		theme_define *theme_found, 
		PVOID context, 
		LONG flags, 
		PVOID *param_out )
	{
		LRESULT ret = ERROR_SUCCESS;
		ImageNode* node_added; 
		image_list_item_info item;

		ASSERT( thumb_file != NULL ); 
		ASSERT( theme_found != NULL ); 

		item.folder = false; 
		item.empty = false; 
		item.description = theme_found->setting.theme_desc; 
		item.icon_type = ICON_FILE_NAME; 
		item.icon = thumb_file; 

		node_added = theme_infos->AddNode( item, NULL ); 

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

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong( *this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );

		m_pm.Init( m_hWnd );
		CDialogBuilder builder;
		CControlUI* pRoot = builder.Create(_T("ui_custom.xml"), (UINT)0, NULL, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);
		Init();

		set_ui_style( &m_pm ); 
		return 0;
	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret; 
		bHandled = FALSE; 
		ret = save_common_conf(); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "save ui configuration error" ); 
			log_trace( ( MSG_ERROR, "save ui configuration error 0x%0.8x\n", ret ) ); 
			goto _return; 
		}

_return:
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

	LRESULT on_theme_notify( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		LRESULT ret = ERROR_SUCCESS; 
		theme_define *theme_found = NULL; 
		CStdString theme_path; 
		//CStdString bk_image; 
		CStdString thumb_image; 

		log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 

		all_themes.resize( all_themes.size() + 1 ); 

		theme_found = &all_themes[ all_themes.size() - 1 ]; 
		memset( theme_found, 0, sizeof( *theme_found ) ); 

		theme_path = ( LPCTSTR )wParam; 
		theme_path.Append( _T( "\\" ) ); 
		theme_path.Append( ( LPCTSTR )lParam ); 

		ret = get_theme_define_from_path( theme_path.GetData(), theme_found ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		thumb_image = _T( "filepath=\'" ); 
		thumb_image += theme_found->thumb_file; 
		thumb_image.Append( _T( "\'" ) ); 

		ret = add_theme_thumb_to_ui( thumb_image.GetData(), theme_found, NULL, 0, NULL ); 
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
			case WM_ERASEBKGND:		lRes = 1; bHandled = TRUE; break; 
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
	CPaintManagerUI *m_main_wnd_pm; 
	CImageListUI *theme_infos; 
	std::vector< theme_define > all_themes; 
};

#endif // COLORSKIN_HPP