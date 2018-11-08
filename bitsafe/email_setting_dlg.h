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
 
 #ifndef __EMAIL_SETTING_H__
#define __EMAIL_SETTING_H__

#include "msg_box.h"
#include <UIUtil.h>
#include "ui_config.h"
#include "General.h"
#include "erc_setting.h"

class email_setting_dlg : public CWindowWnd, public INotifyUI 
{
public:
	email_setting_dlg( email_box *user_info_out ) : user_info( user_info_out ), 
		success( FALSE )
	{
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("email_setting_dlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

	LRESULT init_input_param()
	{
		TCHAR tmp[ 64 ]; 
		WCHAR *unicode_str; 
		LRESULT ret = ERROR_SUCCESS; 
		CEditUI *name_edit; 
		CEditUI *email_edit; 
		CEditUI *smtp_svr_edit; 
		CEditUI *smtp_port_edit; 
		CEditUI *pop3_svr_edit; 
		CEditUI *pop3_port_edit; 
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

		smtp_svr_edit = ( CEditUI* )m_pm.FindControl( _T( "smtp_svr_edit" ) ); 
		ASSERT( smtp_svr_edit != NULL ); 

		smtp_port_edit = ( CEditUI* )m_pm.FindControl( _T( "smtp_port_edit" ) );
		ASSERT( smtp_port_edit != NULL ); 

		pop3_svr_edit = ( CEditUI* )m_pm.FindControl( _T( "pop_svr_edit" ) ); 
		ASSERT( pop3_svr_edit != NULL ); 

		pop3_port_edit = ( CEditUI* )m_pm.FindControl( _T( "pop_port_edit" ) );
		ASSERT( pop3_port_edit != NULL ); 

		pwd_edit = ( CEditUI* )m_pm.FindControl( _T( "pwd_edit" ) );
		ASSERT( pwd_edit != NULL ); 

		if( user_info->name[ MAX_EMAIL_NAME_LEN - 1 ] != '\0' )
		{
			user_info->name[ MAX_EMAIL_NAME_LEN - 1 ] = '\0'; 
		}

//#ifdef _UNICODE
		unicode_str = user_info->name; 
//#else
//		unicode_str = StringConvertor::AnsiToWide( user_info->name ); 
//		if( unicode_str == NULL )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			goto _return; 
//		}
//#endif //_UNICODE

		name_edit->SetText( unicode_str ); 

//#ifndef _UNICODE
//		StringConvertor::StringFree( unicode_str ); 
//#endif //_UNICODE

		if( user_info->email[ MAX_EMAIL_NAME_LEN - 1 ] != '\0' )
		{
			user_info->email[ MAX_EMAIL_NAME_LEN - 1 ] = '\0'; 
		}

//#ifdef _UNICODE
		unicode_str = user_info->email; 
//#else
//		unicode_str = StringConvertor::AnsiToWide( user_info->email ); 
//
//		if( unicode_str == NULL )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			goto _return; 
//		}
//#endif //_UNICODE

		email_edit->SetText( unicode_str ); 

//#ifndef _UNICODE
//		StringConvertor::StringFree( unicode_str ); 
//#endif //_UNICODE

		if( user_info->server[ MAX_EMAIL_NAME_LEN - 1 ] != '\0' )
		{
			user_info->server[ MAX_EMAIL_NAME_LEN - 1 ] = '\0'; 
		}

//#ifdef _UNICODE
		unicode_str = user_info->server; 
//#else
//		unicode_str = StringConvertor::AnsiToWide( user_info->server ); 
//
//		if( unicode_str == NULL )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			goto _return; 
//		}
//#endif //_UNICODE

		smtp_svr_edit->SetText( unicode_str ); 

//#ifndef _UNICODE
//		StringConvertor::StringFree( unicode_str ); 
//#endif //_UNICODE

		if( user_info->pwd[ MAX_EMAIL_NAME_LEN - 1 ] != '\0' )
		{
			user_info->pwd[ MAX_EMAIL_NAME_LEN - 1 ] = '\0'; 
		}

//#ifdef _UNICODE
		unicode_str = user_info->pwd; 
//#else
//		unicode_str = StringConvertor::AnsiToWide( user_info->pwd ); 
//
//		if( unicode_str == NULL )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			goto _return; 
//		}
//#endif //_UNICODE

		pwd_edit->SetText( unicode_str ); 

//#ifndef _UNICODE
//		StringConvertor::StringFree( unicode_str ); 
//#endif //_UNICODE

		ret = port_2_str( user_info->port, tmp, 64 ); 

		smtp_port_edit->SetText( tmp ); 
		
		ret = port_2_str( user_info->pop3_port, tmp, 64 ); 

		pop3_port_edit->SetText( tmp ); 

//#ifdef _UNICODE
		unicode_str = user_info->server; 
//#else
//		unicode_str = StringConvertor::AnsiToWide( user_info->server ); 
//
//		if( unicode_str == NULL )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			goto _return; 
//		}
//#endif //_UNICODE

		smtp_svr_edit->SetText( unicode_str ); 
//
//#ifndef _UNICODE
//		StringConvertor::StringFree( unicode_str ); 
//#endif //_UNICODE

//#ifdef _UNICODE
		unicode_str = user_info->pop3_server; 
//#else
//		unicode_str = StringConvertor::AnsiToWide( user_info->pop3_server ); 
//
//		if( unicode_str == NULL )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			goto _return; 
//		}
//#endif //_UNICODE

		pop3_svr_edit->SetText( unicode_str ); 

//#ifndef _UNICODE
//		StringConvertor::StringFree( unicode_str ); 
//#endif //_UNICODE

		//ret = port_2_str( user_info->pop3_port, tmp, 64 ); 

		//pop3_port_edit->SetText( tmp ); 

		//if( output_param->addition[ _MAX_DESC_LEN - 1 ] != L'\0' )
		//{
		//	output_param->addition[ _MAX_DESC_LEN - 1 ] != L'\0'; 
		//}

_return:

//#ifndef _UNICODE
//		if( unicode_str != NULL )
//		{
//			StringConvertor::StringFree( unicode_str ); 
//		}
//#endif //_UNICODE

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
		
		set_ctrl_text( &m_pm, _T( "title_txt" ), TEXT_ERC_ACCOUNT_SETTING_TITLE ); 
		set_ctrl_text( &m_pm, _T( "apply_btn" ), TEXT_COMMON_BUTTON_CONFIRM ); 
		set_ctrl_text( &m_pm, _T( "test_btn" ), TEXT_ERC_ACCOUNT_SETTING_TEST ); 
		set_ctrl_text( &m_pm, _T( "name_txt" ), TEXT_ERC_ACCOUNT_SETTING_DISPLAY_NAME ); 
		set_ctrl_text( &m_pm, _T( "email_txt" ), TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT ); 
		set_ctrl_text( &m_pm, _T( "pwd_txt" ), TEXT_ERC_ACCOUNT_SETTING_PASSWORD ); 
		set_ctrl_text( &m_pm, _T( "smtp_svr_txt" ), TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_IP); 
		set_ctrl_text( &m_pm, _T( "smtp_port_txt" ), TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_PORT ); 
		set_ctrl_text( &m_pm, _T( "pop_svr_txt" ), TEXT_ERC_ACCOUNT_SETTING_POP3_SERVER_IP ); 
		set_ctrl_text( &m_pm, _T( "pop_svr_port" ), TEXT_ERC_ACCOUNT_SETTING_POP3_SERVER_PORT ); 
		set_ctrl_text( &m_pm, _T( "name_tip_txt" ), TEXT_ERC_ACCOUNT_SETTING_DISPLAY_NAME_TIP ); 
		set_ctrl_text( &m_pm, _T( "email_tip_txt" ), TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT_TIP ); 
		set_ctrl_text( &m_pm, _T( "smtp_tip_txt" ), TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_IP_TIP ); 
		set_ctrl_text( &m_pm, _T( "port_tip_txt" ), TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_PORT_TIP ); 
		set_ctrl_text( &m_pm, _T( "notation_txt" ), TEXT_ERC_ACCOUNT_SETTING_NOTICE_TEXT ); 
		
		init_input_param(); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

	LRESULT load_email_setting( email_box *mail_box )
	{
		LRESULT ret = ERROR_INVALID_PARAMETER; 
		USHORT port_val; 
		CEditUI *name_edit; 
		CEditUI *email_edit; 
		CEditUI *smtp_svr_edit; 
		CEditUI *smtp_port_edit; 
		CEditUI *pop_svr_edit; 
		CEditUI *pop_port_edit; 
		CEditUI *pwd_edit; 
		CStdString input_str; 
		LPCTSTR tmp_text; 

		if( mail_box == NULL )
		{
			goto _return; 
		}

		name_edit = ( CEditUI* )m_pm.FindControl( _T( "name_edit" ) ); 
		ASSERT( name_edit != NULL ); 

		email_edit = ( CEditUI* )m_pm.FindControl( _T( "email_edit" ) );
		ASSERT( email_edit != NULL ); 

		smtp_svr_edit = ( CEditUI* )m_pm.FindControl( _T( "smtp_svr_edit" ) ); 
		ASSERT( smtp_svr_edit != NULL ); 

		smtp_port_edit = ( CEditUI* )m_pm.FindControl( _T( "smtp_port_edit" ) );
		ASSERT( smtp_port_edit != NULL ); 

		pop_svr_edit = ( CEditUI* )m_pm.FindControl( _T( "pop_svr_edit" ) ); 
		ASSERT( pop_svr_edit != NULL ); 

		pop_port_edit = ( CEditUI* )m_pm.FindControl( _T( "pop_port_edit" ) );
		ASSERT( pop_port_edit != NULL ); 

		pwd_edit = ( CEditUI* )m_pm.FindControl( _T( "pwd_edit" ) );
		ASSERT( pwd_edit != NULL ); 

		input_str = name_edit->GetText(); 

		if( input_str.GetLength() > MAX_EMAIL_NAME_LEN - 1 )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_DISPLAY_NAME_LENGTH_ERROR, 
				_T( "显示名称长度不得大于260" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		//if( input_str.IsEmpty() == TRUE )
		//{
		//	show_msg( GetHWND(), _T( "显示名称" ) ); 
		//	goto _return; 
		//}

		input_str = email_edit->GetText(); 
		if( input_str.GetLength() > MAX_EMAIL_NAME_LEN )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT_LENGTH_ERROR, 
				_T( "EMAIL帐号长度不得大于260" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}


		if( _tcschr( input_str, _T( '@' ) ) == NULL )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT_FORMAT_ERROR, 
				_T( "EMAIL帐号格式不正确" ) ); 
			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		if( input_str.IsEmpty() == TRUE )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT_MUST_SET, 
				_T( "EMAIL帐号必须设置" ) ); 
			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		input_str = smtp_svr_edit->GetText(); 
		if( input_str.GetLength() > MAX_EMAIL_NAME_LEN )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_LENGTH_ERROR, 
				_T( "SMTP邮件服务器名长度不得大于260" ) ); 
			
			show_msg( GetHWND(), tmp_text); 
			goto _return; 
		}

		if( input_str.IsEmpty() == TRUE )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_MUST_SET, 
				_T( "SMTP邮件服务器名必须设置" ) ); 
			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		input_str = pop_svr_edit->GetText(); 

		if( input_str.GetLength() > MAX_EMAIL_NAME_LEN )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_POP3_SERVER_LENGTH_ERROR, 
				_T( "POP3邮件服务器名长度不得大于260" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		if( input_str.IsEmpty() == TRUE )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_POP3_SERVER_MUST_SET, 
				_T( "POP3邮件服务器名必须设置" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		input_str = pwd_edit->GetText(); 
		if( input_str.GetLength() > MAX_EMAIL_NAME_LEN )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_PASSWORD_LENGTH_ERROR, 
				_T( "密码长度不得大于260" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		if( input_str.IsEmpty() == TRUE )
		{
			dlg_ret_state ret_status; 

			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_NO_NEED_PASSWORD_TIP, 
				_T( "不需要密码?请确认." ) ); 

			show_msg( GetHWND(), tmp_text, &ret_status ); 
			
			if( ret_status != OK_STATE )
			{
				goto _return; 
			}
		}

		ret = str_2_port( smtp_port_edit->GetText().GetData(), &port_val ); 
		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_SMTP_PORT_FMT_ERROR, 
				_T( "SMTP端口格式不正确" ) ); 
			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		mail_box->port = port_val;  

		ret = str_2_port( pop_port_edit->GetText().GetData(), &port_val ); 
		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_POP3_PORT_FMT_ERROR, 
				_T( "POP3端口格式不正确" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		mail_box->pop3_port = port_val;  

		_tcsncpy( mail_box->email, email_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 
		_tcsncpy( mail_box->name, name_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 
		_tcsncpy( mail_box->pwd, pwd_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 
		_tcsncpy( mail_box->server, smtp_svr_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 
		_tcsncpy( mail_box->pop3_server, pop_svr_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 

		//wcsncpy( mail_box->, email_edit->GetText().GetData(), MAX_EMAIL_NAME_LEN - 1 ); 

		//if( output_param->desc.socket.src_port.port.port_begin == 0 
		//	&& output_param->desc.socket.src_port.port.port_end == 0 
		//	&&output_param->desc.socket.dest_port.port.port_begin == 0 
		//	&& output_param->desc.socket.dest_port.port.port_end == 0 )
		//{
		//	show_msg( GetHWND(), _T( "源端口和目标端口不可以同时不设定" ) ); 
		//	goto _return; 
		//}

		success = TRUE; 
		ret = ERROR_SUCCESS; 

		//app_name_edit->SetText( _T( "" ) ); 
		//sip_begin_edit->SetText( _T( "" ) );  
		//dip_begin_edit->SetText( _T( "" ) ); ; 
		//sport_begin_edit->SetText( _T( "" ) ); 
		//dport_begin_edit->SetText( _T( "" ) ); ; 
		//sip_end_edit->SetText( _T( "" ) ); 
		//dip_end_edit->SetText( _T( "" ) ); 
		//sport_end_edit->SetText( _T( "" ) ); 
		//dport_end_edit->SetText( _T( "" ) ); 
_return: 
		return ret; 
	}

	LRESULT on_apply()
	{
		LRESULT ret = ERROR_SUCCESS; 

		ret = load_email_setting( user_info ); 
		
		if( ret == ERROR_SUCCESS )
		{
			Close(); 
		}

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
				LRESULT ret; 
				//TCHAR file_name[ MAX_PATH ]; 
				CStdString name = msg.pSender->GetName(); 

				//*file_name = _T( '\0' ); 
				if( name == _T( "apply_btn" ) )
				{
					on_apply(); 
				}
				else if( name == _T( "test_btn" ) )
				{
					LPCTSTR tmp_text; 
					email_box mail_test; 
					mail_test.erc_on = FALSE; 

					memset( &mail_test, 0, sizeof( mail_test ) ); 

					ret = load_email_setting( &mail_test ); 
					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					if( success == TRUE )
					{
						ret = test_erc_conf( &mail_test ); 
						if( ret == ERROR_SUCCESS )
						{
							tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_TEST_EMAIL_ACCOUNT_SUCCESSFULLY, 
								_T( "测试邮箱成功" ) ); 

							show_msg( GetHWND(), tmp_text ); 
						}
						else
						{
							tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_TEST_EMAIL_ACCOUNT_FAILED, 
								_T( "测试邮箱失败" ) ); 

							show_msg( GetHWND(), tmp_text ); 
						}
					}
				}
			}
        }
        else if(msg.sType==_T("setfocus"))
        {
        }
_return:
		return; 
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create( _T( "email_setting.xml" ), (UINT)0, NULL, &m_pm );
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

#endif //__EMAIL_SETTING_H__