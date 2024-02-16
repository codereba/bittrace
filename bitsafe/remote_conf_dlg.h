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
 
#ifndef __REMOTE_CONF_DLG_H__
#define __REMOTE_CONF_DLG_H__

#include "General.h"
#include "erc_setting.h"
#include "email_setting_dlg.h"
#include "read_txt_box.h"

class remote_conf_dlg : public CWindowWnd, public INotifyUI 
{
public:
	remote_conf_dlg( PVOID context ) : m_pCloseBtn( NULL ), 
		erc_work_context( context )
	{
	};

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("remote_conf_dlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    }

#define ERC_ON_TEXT _T( "��������Զ�̿��Ʒ���" )
#define ERC_OFF_TEXT _T( "�رձ���Զ�̿��Ʒ���" )

	void set_erc_btn_text()
	{
		LPCTSTR erc_btn_text = NULL; 
		CButtonUI *erc_btn; 

		erc_btn = ( CButtonUI* )m_pm.FindControl( _T( "erc_sw_btn" ) ); 
		ASSERT( erc_btn != NULL ); 

		if( _check_email_conf_valid( &email_box_info ) != ERROR_SUCCESS )
		{
			erc_btn_text = _get_string_by_id( TEXT_ERC_OPEN_LOCAL_ERC_SERVICE, 
				ERC_ON_TEXT ); 
		}
		else
		{
			if( email_box_info.erc_on == TRUE )
			{
				erc_btn_text = _get_string_by_id( TEXT_ERC_CLOSE_LOCAL_ERC_SERVICE, 
					ERC_OFF_TEXT ); 
			}
			else
			{
				erc_btn_text = _get_string_by_id( TEXT_ERC_OPEN_LOCAL_ERC_SERVICE, 
					ERC_ON_TEXT ); ; 
			}
		}

		erc_btn->SetText( erc_btn_text ); 
	}

    void Init() 
    {
		CComboUI *action_combo; 
		LPCTSTR tmp_text; 
		CControlUI *ctrl; 
		LONG prev_width; ; 

		action_combo = ( CComboUI* )m_pm.FindControl( _T( "action_combo" ) );
		ASSERT( action_combo != NULL ); 

		//if( rule_type == SOCKET_RULE_TYPE )
		{
			action_combo->SelectListItem( 0 ); 
		}

		m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 


#ifdef _DEBUG
		//ASSERT( check_email_conf_valid( &email_box_info ) == ERROR_SUCCESS ); 
#endif //_DEBUG	

		set_erc_btn_text(); 

		set_ctrl_text( &m_pm, _T( "title_txt" ), TEXT_ERC_TITLE ); 
		set_ctrl_text( &m_pm, _T( "opera_txt" ), TEXT_ERC_OPERATION ); 
		set_ctrl_text( &m_pm, _T( "apply_btn" ), TEXT_ERC_SEND_CMD_BUTTON ); 
		
		ctrl = m_pm.FindControl( _T( "account_btn" ) ); 

		ASSERT( ctrl != NULL ); 

		_set_ctrl_text( ctrl, TEXT_ERC_EMAIL_ACCOUNT ); 

		prev_width = ctrl->GetFixedWidth(); 
		prev_width += 12; 

		ctrl->SetFixedWidth( prev_width ); 

		set_ctrl_text( &m_pm, _T( "erc_sw_btn" ), TEXT_ERC_OPEN_LOCAL_ERC_SERVICE ); 
		set_ctrl_text( &m_pm, _T( "time_txt" ), TEXT_ERC_HOLD_TIME ); 
		set_ctrl_text( &m_pm, _T( "hour_txt" ), TEXT_ERC_TIME_HOUR ); 
		set_ctrl_text( &m_pm, _T( "minite_txt" ), TEXT_ERC_TIME_MINUTE ); 
		set_ctrl_text( &m_pm, _T( "tip_btn" ), TEXT_ERC_ACCOUNT_SETTING_NOTICE ); 
		_set_ctrl_text( action_combo, TEXT_ERC_REMOTE_COMMAND_COMBO_TEXT ); 
		
		tmp_text = get_string_by_id( TEXT_ERC_REMOTE_COMMAND_COMBO_TIP ); 
		if( tmp_text != NULL )
		{
			action_combo->SetToolTip( tmp_text ); 
		}

		ctrl = action_combo->GetItemAt( 0 ); 
		_set_ctrl_text( ctrl, TEXT_WORK_MODE_BLOCK_ALL ); 
		ctrl = action_combo->GetItemAt( 1 ); 
		_set_ctrl_text( ctrl, TEXT_WORK_MODE_FREE ); 
		ctrl = action_combo->GetItemAt( 2 ); 
		_set_ctrl_text( ctrl, TEXT_WORK_MODE_ACL_CTRL ); 

		//m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
		//ASSERT( m_pRestoreBtn != NULL ); 
		//m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		//ASSERT( m_pMinBtn != NULL ); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

	INLINE LRESULT ask_erc_conf()
	{
		LRESULT ret; // = ERROR_SUCCESS; 
		dlg_ret_state ret_state; 
		LPCTSTR tmp_text; 

		ret = _check_email_conf_valid( &email_box_info ); 

		if( ret == ERROR_SUCCESS )
		{
			goto _return; 
		}

		tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_SETTING_NOW, 
			_T( "Զ�̿����ʼ��ʺŻ�δ��������,�Ƿ�������������?" ) ); 

		show_msg( GetHWND(), tmp_text, &ret_state ); 

		if( ret_state == OK_STATE )
		{
			set_erc_config(); 
		}

		ret = _check_email_conf_valid( &email_box_info ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
		
_return:
		return ret; 
	}

	LRESULT on_apply()
	{
		LRESULT ret = ERROR_SUCCESS; 
		ULONG selected; 
		ULONG hour; 
		ULONG minite; 
		CComboUI *action_combo; 
		CEditUI *time_edit; 
		TCHAR *time_str; 
		ULONG ctrl_mode = 0; 
		LPCTSTR tmp_text; 

		ask_erc_conf(); 

		action_combo = ( CComboUI* )m_pm.FindControl( _T( "action_combo" ) );
		ASSERT( action_combo != NULL ); 
		
		time_edit = ( CEditUI* )m_pm.FindControl( _T( "hour_edit" ) );
		ASSERT( time_edit != NULL ); 

		time_str = ( TCHAR* )time_edit->GetText().GetData(); 

		hour = _tcstol( time_str, &time_str, 10 ); 

		time_edit = ( CEditUI* )m_pm.FindControl( _T( "minite_edit" ) );
		ASSERT( time_edit != NULL ); 

		time_str = ( TCHAR* )time_edit->GetText().GetData(); ; 

		minite = _tcstol( time_str, &time_str, 10 ); 

		if( hour == 0 && minite == 0 )
		{
			//hour = 24; 

			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_TIME_MUST_BE_SET, 
				_T( "�����Բ�����ʱ��") ); 
			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		action_combo = ( CComboUI* )m_pm.FindControl( _T( "action_combo" ) );
		ASSERT( action_combo != NULL ); 

		selected = action_combo->GetCurSel(); 

		if( selected == 0 )
		{
			ctrl_mode = REMOTE_CONTROL_BLOCK; 			
		}
		else if( selected == 1 )
		{
			ctrl_mode = REMOTE_CONTROL_ALLOW; 
		}
		else if( selected == 2 )
		{
			ctrl_mode = REMOTE_CONTROL_ACL; 
		}
		else
		{
			ASSERT( FALSE && "ui lib fault invalid combo selection" ); 
		}

		ret = smtp_remote_control_async( ctrl_mode, hour, minite, NULL, NULL ); 

		Close(); 

_return:
		return ret; 
	}

	LRESULT set_erc_config()
	{
		LRESULT ret = ERROR_SUCCESS; 
		email_box mail_box; 
		LPCTSTR tmp_text; 

		ret = get_mail_box( &mail_box ); 
		if( ret != ERROR_SUCCESS )
		{
			memset( &mail_box, 0, sizeof( mail_box ) ); 
		}

		email_setting_dlg dlg( &mail_box ); 
		
		//dlg.set_output_param( &email_box_info ); 

		tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_TITLE, 
			_T( "Զ�̿�������" ) ); 

		dlg.Create( GetHWND(), tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
		dlg.SetIcon( IDI_MAIN_ICON ); 
		dlg.CenterWindow();
		dlg.ShowModal(); 

		if( dlg.setting_success() == TRUE )
		{
			ret = set_mail_box( &mail_box ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				goto _return; 
			}

			ret = save_erc_conf(); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_SUCCESSFULLY, 
				_T( "����Զ�̿�������ɹ�" ) ); 
			show_msg( GetHWND(), tmp_text ); 
		}
		else
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}

_return:
		return ret; 
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
				LPCTSTR tmp_text; 
				//TCHAR file_name[ MAX_PATH ]; 
				CStdString name = msg.pSender->GetName(); 

				//*file_name = _T( '\0' ); 
				if( name == _T( "apply_btn" ) )
				{
					on_apply(); 
				}
				else if( name == _T( "erc_sw_btn" ) )
				{
					ret = ask_erc_conf(); 
					if( ret != ERROR_SUCCESS )
					{
						goto _return; 
					}

					if( email_box_info.erc_on == FALSE )
					{
						ret = pop3_exec_remote_control( erc_work_context ); 
						if( ret == ERROR_SUCCESS )
						{
							email_box_info.erc_on = TRUE; 
							set_erc_btn_text(); 
						}
					}
					else
					{
						ret = stop_pop3_remote_control(); 
						if( ret == ERROR_SUCCESS )
						{
							email_box_info.erc_on = FALSE; 
							set_erc_btn_text(); 
						}
					}

					save_erc_conf(); 
				}
				else if( name == _T( "account_btn" ) )
				{
					set_erc_config(); 
				}
				else if( name == _T( "tip_btn" ) )
				{
#define ERC_DESC _T( "��ע��:\n  1.Զ�̿����ǽ������������ʼ���ʽ�����ʼ�������,�������ƶ˽��յ��ʼ�,������Ӧ����ķ�ʽ��ʵ��.\n  2.Զ�̿���ͨ���ʼ�������֧�����,��Ӧʱ�������������״̬�й�,ͨ����������.\n  3.��ʱ�������޷�Ƶ����Ӧ����,����Լ���.\n  4.����ʱ�䵽ʱ,�Զ��ָ�������ģʽ.\n  5.������Ϊ����ģʽʱ,�ڿ���ʱ��֮��,��������ͨ�Ž��ر�,Զ�̿��ƹ��ܽ��޷�����.\n  6.ע��,URL���ƹ���Ҫȥ�ʼ���������ͻ.\n  7.sohu�ʼ�������ʵ�ʲ��Կ���ʹ��,�Ƽ�ʹ��.\n  8.��������������Զ�̿��Ƶ��ʼ����Զ�ɾ��,����ע�ⲻҪ���ʼ������ı��е���ʼ�ı�����Ϊ��������ͬ:\"REMOTE COMMAND\",���ʹ��ר������.\n  9.���͵������ȡ��֮ǰ���͵�����,���Բ�Ҫ����ص���������(���鲻����2��),ͬʱҪ����Թ����ʼ�������Դ��Ƶ��ռ��.\n  10.ʹ�ù����ʼ���������һЩ���ȶ���,����ʱ�Ὣ�����Ϊ�����ʼ�,�������������ʼ�����.\n  11.��Ϊһ���������,���ذ�ȫ�޷��ṩר�÷�������ʵ��ȫ����ȶ��İ�ȫ����,�����ϣ�����õ�ǿ����,��֧������:\r\n" ) BITSAFE_EMAIL __t( "��http://www.codereba.com/bbs.\r\n\r\n  �뽫���������������̳:\r\nhttp://www.codereba.com/bbs." ) 

					tmp_text = _get_string_by_id( TEXT_ERC_NOTICE_CONTENT, ERC_DESC ); 
					show_txt( GetHWND(), tmp_text ); 
				}
			}
        }
        else if( msg.sType == _T( "setfocus" ) )
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
        CControlUI* pRoot = builder.Create( _T( "email_remote_conf.xml" ), (UINT)0, NULL, &m_pm );
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

private:
    CButtonUI* m_pCloseBtn;
	PVOID erc_work_context; 
	//LONG email_btn_len; 
};

LRESULT set_erc_config_now(); 
#endif //__REMOTE_CONF_DLG_H__