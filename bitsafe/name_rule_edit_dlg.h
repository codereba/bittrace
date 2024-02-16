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
 
 #pragma once

#include "bitsafe_common.h"
#include "conf_file.h"
#include "bitsafe_rule_conf.h"
#include "msg_box.h"

#define URL_RULE_TITLE _T( "url·��������������" )
#define URL_NAME_TXT _T( "url·��������")
#define FILE_RULE_TITLE _T( "�ļ�������" )
#define FILE_NAME_TXT _T( "�ļ�·��" )
#define REG_RULE_TITLE _T( "ע���������" )
#define REG_NAME_TXT _T("ע���·��")

class name_rule_edit_dlg : public CWindowWnd, public INotifyUI 
{
public:
	name_rule_edit_dlg( access_rule_desc *param_output, access_rule_type type = URL_RULE_TYPE, INT32 init_val = FALSE ) : successed( FALSE )
	{
		output_param = param_output; 
		rule_type = type; 
		need_init = init_val; 
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T( "name_rule_edit_dlg" ); 
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
		CControlUI *title_txt; 
		CControlUI *name_txt; 
		CControlUI *edit; 
		CComboUI *action_combo; 
        
		m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 

		title_txt = m_pm.FindControl( _T( "title_txt" ) ); 
		ASSERT( title_txt != NULL ); 

		name_txt = m_pm.FindControl( _T( "name_txt" ) ); 
		ASSERT( name_txt != NULL ); 
		
		action_combo = ( CComboUI* )m_pm.FindControl( _T( "action_combo" ) ); 
		ASSERT( action_combo != NULL ); 

		init_access_desc_param_ptr( output_param->type, &output_param->desc ); 

		if( need_init == TRUE )
		{
			edit = m_pm.FindControl( _T( "name_edit" ) ); 
			ASSERT( edit != NULL ); 
			output_param->desc.params[ 1 ]->common.name[ _MAX_URL_LEN - 1 ] = L'\0'; 
			edit->SetText( output_param->desc.params[ 1 ]->common.name ); 

			edit = m_pm.FindControl( _T( "app_name_edit" ) ); 
			ASSERT( edit != NULL ); 
			output_param->desc.params[ 0 ]->common.name[ _MAX_FILE_NAME_LEN - 1 ] = L'\0';  
			edit->SetText( output_param->desc.params[ 0 ]->common.name ); 

			edit = m_pm.FindControl( _T( "desc_edit" ) ); 
			ASSERT( edit != NULL ); 
			//output_param->addition[ _MAX_DESC_LEN - 1 ] = L'\0';  
			edit->SetText( _T("") ); 

			if( output_param->resp == ACTION_BLOCK )
			{
				action_combo->SelectListItem( 0 ); 
			}
			else if( output_param->resp == ACTION_ALLOW )
			{
				action_combo->SelectListItem( 1 ); 
			}
			else if( output_param->resp == ACTION_LEARN )
			{
				ASSERT( /*rule_type != SOCKET_RULE_TYPE 
					&& */rule_type != URL_RULE_TYPE ); 

				action_combo->SelectListItem( 2 ); 
			}
		}

		CControlUI *ctrl; 
		ctrl = action_combo->GetItemAt( 0 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_RESPONSE_BLOCK ); 

		ctrl = action_combo->GetItemAt( 1 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_RESPONSE_ALLOW); 

		ctrl = action_combo->GetItemAt( 0 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_RESPONSE_LEARN ); 

		set_ctrl_text( &m_pm, _T( "apply_btn" ), TEXT_COMMON_BUTTON_CONFIRM ); 
		set_ctrl_text( &m_pm, _T( "app_name_txt" ), TEXT_NAME_EDIT_APP ); 
		set_ctrl_text( &m_pm, _T( "desc_txt" ), TEXT_NAME_EDIT_DESC ); 
		set_ctrl_text( &m_pm, _T( "opera_txt" ), TEXT_NAME_EDIT_OPERA ); 

		LPCTSTR tmp_text; 
		if( rule_type == REG_RULE_TYPE )
		{
			tmp_text = _get_string_by_id( TEXT_NAME_EDIT_REG_RULE_TITLE, REG_RULE_TITLE ); 

			title_txt->SetText( tmp_text ); 

			tmp_text = _get_string_by_id( TEXT_NAME_EDIT_REG_NAME, REG_NAME_TXT ); 
			name_txt->SetText( tmp_text ); 
		}
		else if( rule_type == FILE_RULE_TYPE )
		{
			tmp_text = _get_string_by_id( TEXT_NAME_EDIT_FILE_RULE_TITLE, FILE_RULE_TITLE ); 

			title_txt->SetText( tmp_text ); 
			
			tmp_text = _get_string_by_id( TEXT_NAME_EDIT_FILE_NAME, FILE_NAME_TXT ); 
			
			name_txt->SetText( tmp_text ); 
		}
		else if( rule_type == URL_RULE_TYPE )
		{
			tmp_text = _get_string_by_id( TEXT_NAME_EDIT_URL_RULE_TITLE, URL_RULE_TITLE ); 

			title_txt->SetText( tmp_text ); 
			
			tmp_text = _get_string_by_id( TEXT_NAME_EDIT_URL_NAME, URL_NAME_TXT ); 

			name_txt->SetText( tmp_text ); 

			action_combo->RemoveItemAt( 2 ); 
			action_combo->SelectListItem( 0 ); 
		}

		//m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
  //      ASSERT( m_pRestoreBtn != NULL ); 
		//m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		//ASSERT( m_pMinBtn != NULL ); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

	void on_apply()
	{
		INT32 ret; 
		//TCHAR file_name[ MAX_PATH ]; 
		//*file_name = _T( '\0' ); 
		CEditUI *name_edit; 
		CEditUI *app_name_edit; 
		CEditUI *desc_edit; 
		CComboUI *action_combo; 
		ULONG selected; 

		if( output_param == NULL )
		{
			goto _return; 
		}

		name_edit = ( CEditUI* )m_pm.FindControl( _T( "name_edit" ) ); 
		ASSERT( name_edit != NULL ); 

		app_name_edit = ( CEditUI* )m_pm.FindControl( _T( "app_name_edit" ) ); 
		ASSERT( name_edit != NULL ); 

		desc_edit = ( CEditUI* )m_pm.FindControl( _T( "desc_edit" ) ); 
		ASSERT( name_edit != NULL ); 

		action_combo = ( CComboUI* )m_pm.FindControl( _T( "action_combo" ) ); 
		ASSERT( action_combo != NULL ); 

		if( name_edit->GetText().GetLength() == 0 )
		{
			show_msg( GetHWND(), _T( "·���Ǳ�������Ĺؼ�����" ), NULL, _T( "�������" ) ); 
			goto _return; ; 
		}

		if( name_edit->GetText().GetLength() > _MAX_URL_LEN )
		{
			show_msg( GetHWND(), _T( "·�����Ȳ��ô���1024" ), NULL, _T( "�������" ) ); 
			goto _return; ; 
		}

		if( desc_edit->GetText().GetLength() > _MAX_DESC_LEN )
		{
			show_msg( GetHWND(), _T( "�������Ȳ��ô���512" ), NULL, _T( "�������" ) ); 
			goto _return; 
		}

		if( app_name_edit->GetText().GetLength() > _MAX_FILE_NAME_LEN )
		{
			show_msg( GetHWND(), _T( "�������Ȳ��ô���260" ), NULL, _T( "�������" ) ); 
			goto _return; 
		}

		selected = action_combo->GetCurSel(); 

		if( selected == 0 )
		{
			output_param->resp = ACTION_BLOCK; 
		}
		else if( selected == 1 )
		{
			output_param->resp = ACTION_ALLOW; 
		}
		else if( selected == 2 )
		{
			output_param->resp = ACTION_LEARN; 
		}

		output_param->type = rule_type; 
		//wcsncpy( output_param->addition, desc_edit->GetText().GetData(), _MAX_DESC_LEN ) ; 

		output_param->desc.common.app.is_cls = FALSE; 
		output_param->desc.common.app.type = APP_DEFINE; 
		wcsncpy( output_param->desc.common.app.app.app_name , app_name_edit->GetText().GetData(), _MAX_DESC_LEN ); 

		output_param->desc.common.param0.is_cls = FALSE; 
		output_param->desc.common.param0.type = get_rule_param_type( rule_type, 1 ); 
		; 
		wcscpy( output_param->desc.common.param0.url.url, name_edit->GetText().GetData() ); 
		
		output_param->type = rule_type; 

		successed = TRUE; 
		Close(); 
_return:
		return; 
	}

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("windowinit") ) 
            OnPrepare(msg);
		else if( msg.sType == _T( "return" ) )
		{
			on_apply(); 
		}
        else if( msg.sType == _T("click") ) 
        {
            if( msg.pSender == m_pCloseBtn ) 
            {
				Close(); 
                return; 
            }
			else
			{
				CStdString name = msg.pSender->GetName(); 
				if( name == _T( "apply_btn" ) )
				{
					on_apply(); 
				}
			}
        }
        else if( msg.sType == _T( "setfocus" ) )
        {
        }
    }

	INT32 create_rule_success()
	{
		return successed; 
	}

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("name_rule_edit.xml"), (UINT)0, NULL, &m_pm);
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
        // ��ʱ�����յ�WM_NCDESTROY���յ�wParamΪSC_CLOSE��WM_SYSCOMMAND
        if( wParam == SC_CLOSE ) {
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
	VOID set_output_param( access_rule_desc *param )
	{
		output_param = param; 
	}

private:
	access_rule_desc *output_param; 
    CButtonUI* m_pCloseBtn;
    access_rule_type rule_type; 
	INT32 need_init; 
	INT32 successed; 
};