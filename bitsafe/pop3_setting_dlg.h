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
 
 #ifndef __SMTP_SETTING_H__
#define __SMTP_SETTING_H__

#include "msg_box.h"
#include <UIUtil.h>
#include "ui_config.h"
#include "General.h"

class pop_setting_dlg : public CWindowWnd, public INotifyUI 
{
public:
	pop_setting_dlg( email_box *user_info_out ) : user_info( user_info_out ), 
		success( FALSE )
	{
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("pop_setting_dlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

	HRESULT init_input_param()
	{
		TCHAR tmp[ 64 ]; 
		WCHAR *unicode_str; 
		HRESULT ret = ERROR_SUCCESS; 
		CEditUI *name_edit; 
		CEditUI *email_edit; 
		CEditUI *server_edit; 
		CEditUI *port_edit; 
		CEditUI *pwd_edit; 
		USHORT port_val; 

		if( user_info == NULL )
		{
			ret = ERROR_NOT_READY; 
			goto _return; 
		}

		if( *user_info->email == '\0' )
		{
			goto _return; 
		}

		name_edit = ( CEditUI* )m_pm.FindControl( _T( "name_edit" ) ); 
		ASSERT( name_edit != NULL ); 

		email_edit = ( CEditUI* )m_pm.FindControl( _T( "email_edit" ) );
		ASSERT( email_edit != NULL ); 

		server_edit = ( CEditUI* )m_pm.FindControl( _T( "server_edit" ) ); 
		ASSERT( server_edit != NULL ); 

		port_edit = ( CEditUI* )m_pm.FindControl( _T( "port_edit" ) );
		ASSERT( port_edit != NULL ); 

		pwd_edit = ( CEditUI* )m_pm.FindControl( _T( "pwd_edit" ) );
		ASSERT( pwd_edit != NULL ); 

		if( user_info->name[ MAX_EMAIL_NAME_LEN - 1 ] != '\0' )
		{
			user_info->name[ MAX_EMAIL_NAME_LEN - 1 ] = '\0'; 
		}

#ifdef _UNICODE
		unicode_str = user_info->name; 
#else
		unicode_str = StringConvertor::AnsiToWide( user_info->name ); 
		if( unicode_str == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}
#endif //_UNICODE

		name_edit->SetText( unicode_str ); 

#ifndef _UNICODE
		StringConvertor::StringFree( unicode_str ); 
#endif //_UNICODE

		if( user_info->email[ MAX_EMAIL_NAME_LEN - 1 ] != '\0' )
		{
			user_info->email[ MAX_EMAIL_NAME_LEN - 1 ] = '\0'; 
		}

#ifdef _UNICODE
		unicode_str = user_info->email; 
#else
		unicode_str = StringConvertor::AnsiToWide( user_info->email ); 

		if( unicode_str == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}
#endif //_UNICODE

		email_edit->SetText( unicode_str ); 

#ifndef _UNICODE
		StringConvertor::StringFree( unicode_str ); 
#endif //_UNICODE

		if( user_info->server[ MAX_EMAIL_NAME_LEN - 1 ] != '\0' )
		{
			user_info->server[ MAX_EMAIL_NAME_LEN - 1 ] = '\0'; 
		}

#ifdef _UNICODE
		unicode_str = user_info->server; 
#else
		unicode_str = StringConvertor::AnsiToWide( user_info->server ); 

		if( unicode_str == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}
#endif //_UNICODE

		email_edit->SetText( unicode_str ); 

#ifndef _UNICODE
		StringConvertor::StringFree( unicode_str ); 
#endif //_UNICODE

		if( user_info->pwd[ MAX_EMAIL_NAME_LEN - 1 ] != '\0' )
		{
			user_info->pwd[ MAX_EMAIL_NAME_LEN - 1 ] = '\0'; 
		}

#ifdef _UNICODE
		unicode_str = user_info->pwd; 
#else
		unicode_str = StringConvertor::AnsiToWide( user_info->pwd ); 

		if( unicode_str == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}
#endif //_UNICODE

		email_edit->SetText( unicode_str ); 

#ifndef _UNICODE
		StringConvertor::StringFree( unicode_str ); 
#endif //_UNICODE

		ret = port_2_str( user_info->port, tmp, 64 ); 

		port_edit->SetText( tmp ); 
		
		//if( output_param->addition[ _MAX_DESC_LEN - 1 ] != L'\0' )
		//{
		//	output_param->addition[ _MAX_DESC_LEN - 1 ] != L'\0'; 
		//}

_return:

#ifndef _UNICODE
		if( unicode_str != NULL )
		{
			StringConvertor::StringFree( unicode_str ); 
		}
#endif //_UNICODE

		return ret; 
	}
	; 
    void Init() 
    {
        
		m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 

		//title_txt = m_pm.FindControl( _T( "title_txt" ) ); 
		//ASSERT( title_txt != NULL ); 
		
		//name_txt = m_pm.FindControl( _T( "name_txt" ) ); 
		//ASSERT( name_txt != NULL ); 

		//m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
  //      ASSERT( m_pRestoreBtn != NULL ); 
		//m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		//ASSERT( m_pMinBtn != NULL ); 
		
		init_input_param(); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

	HRESULT on_apply()
	{
		HRESULT ret = ERROR_SUCCESS; 
		USHORT port_val; 
		CEditUI *name_edit; 
		CEditUI *email_edit; 
		CEditUI *server_edit; 
		CEditUI *port_edit; 
		CEditUI *pwd_edit; 

		if( user_info == NULL )
		{
			goto _return; 
		}

		name_edit = ( CEditUI* )m_pm.FindControl( _T( "name_edit" ) ); 
		ASSERT( name_edit != NULL ); 

		email_edit = ( CEditUI* )m_pm.FindControl( _T( "email_edit" ) );
		ASSERT( email_edit != NULL ); 

		server_edit = ( CEditUI* )m_pm.FindControl( _T( "server_edit" ) ); 
		ASSERT( server_edit != NULL ); 

		port_edit = ( CEditUI* )m_pm.FindControl( _T( "port_edit" ) );
		ASSERT( port_edit != NULL ); 

		pwd_edit = ( CEditUI* )m_pm.FindControl( _T( "pwd_edit" ) );
		ASSERT( pwd_edit != NULL ); 

		if( name_edit->GetText().GetLength() > MAX_EMAIL_NAME_LEN - 1 )
		{
			show_msg( GetHWND(), _T( "��ʾ���Ƴ��Ȳ��ô���260" ) ); 
			goto _return; 
		}

		if( email_edit->GetText().GetLength() > MAX_EMAIL_NAME_LEN )
		{
			show_msg( GetHWND(), _T( "EMAIL�ʺų��Ȳ��ô���260" ) ); 
			goto _return; 
		}

		if( server_edit->GetText().GetLength() > MAX_EMAIL_NAME_LEN )
		{
			show_msg( GetHWND(), _T( "SMTP�ʼ������������Ȳ��ô���260" ) ); 
			goto _return; 
		}

		if( pwd_edit->GetText().GetLength() > MAX_EMAIL_NAME_LEN )
		{
			show_msg( GetHWND(), _T( "���볤�Ȳ��ô���260" ) ); 
			goto _return; 
		}
		
		ret = str_2_port( port_edit->GetText().GetData(), &port_val ); 
		if( ret != ERROR_SUCCESS )
		{
			show_msg( GetHWND(), _T( "�˿ڸ�ʽ����ȷ" ) ); 
			goto _return; 
		}
		user_info->port = port_val;  

		wcsncpy( user_info->email, email_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 
		wcsncpy( user_info->name, name_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 
		wcsncpy( user_info->pwd, pwd_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 
		wcsncpy( user_info->server, server_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 
		//wcsncpy( user_info->, email_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 

		//if( output_param->desc.socket.src_port.port.port_begin == 0 
		//	&& output_param->desc.socket.src_port.port.port_end == 0 
		//	&&output_param->desc.socket.dest_port.port.port_begin == 0 
		//	&& output_param->desc.socket.dest_port.port.port_end == 0 )
		//{
		//	show_msg( GetHWND(), _T( "Դ�˿ں�Ŀ��˿ڲ�����ͬʱ���趨" ) ); 
		//	goto _return; 
		//}

		success = TRUE; 

		//app_name_edit->SetText( _T( "" ) ); 
		//sip_begin_edit->SetText( _T( "" ) );  
		//dip_begin_edit->SetText( _T( "" ) ); ; 
		//sport_begin_edit->SetText( _T( "" ) ); 
		//dport_begin_edit->SetText( _T( "" ) ); ; 
		//sip_end_edit->SetText( _T( "" ) ); 
		//dip_end_edit->SetText( _T( "" ) ); 
		//sport_end_edit->SetText( _T( "" ) ); 
		//dport_end_edit->SetText( _T( "" ) ); 
		Close(); 

_return:
		return ret; 
	}

	INT32 setting_success()
	{
		return success; 
	}

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("windowinit") ) 
		{
            OnPrepare(msg);
		}
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
				HRESULT ret; 
				//TCHAR file_name[ MAX_PATH ]; 
				CStdString name = msg.pSender->GetName(); 

				//*file_name = _T( '\0' ); 
				if( name == _T( "apply_btn" ) )
				{
					on_apply(); 
				}			
			}
        }
        else if(msg.sType==_T("setfocus"))
        {
        }
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create( _T( "pop3_setting.xml" ), (UINT)0, NULL, &m_pm );
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
	VOID set_output_param( email_box *user )
	{
		user_info = user; 
	}

private:

	CButtonUI *m_pCloseBtn; 
	email_box *user_info; 
	INT32 success; 
};

#endif //__SMTP_SETTING_H__