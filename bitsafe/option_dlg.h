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

#include "tab_builder.h"
#include "msg_box.h"
#include "common_api.h"

typedef enum _option_type
{
	PROTECT_OPTION, 
	LANG_OPTION, 
	GENERAL_OPTION
} option_type; 

class option_dlg : public CWindowWnd, public INotifyUI
{
public:
	option_dlg( action_ui_notify *notifier = NULL ) 
	{
		ui_notify = notifier; 
	};

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("OptionDlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

#define SET_BTN_STATE 0x0000001
	LRESULT set_ctrl_ui_by_driver_state( bitsafe_function_type type, bitsafe_function_state load_state, ULONG flags )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *ctrl = NULL; 

		ASSERT( TRUE == is_valid_bitsafe_function_state( load_state ) ); 

		switch( type )
		{
		case NET_PACKET_FLT_FUNCTION: 
			//ASSERT( FALSE && "have not control this driver directly" ); 
			if( TRUE == is_vista_or_later_ex() )
			{
				ctrl = NULL; 

			}
			else
			{
				ctrl = m_pm.FindControl( _T( "netbase_btn" ) ); 
			}
			break; 

		case HTTP_TEXT_FLT_FUNCTION:
			//ASSERT( FALSE && "have not use text filter" ); 
			//ctrl = m_pm.FindControl( _T( "url_btn" ) ); 
			//switch( all_drv_states[ i ].state )
			//{
			//case DRV_STATE_UNLOADED:
			//	( ( COptionUI* )ctrl )->Selected( true ); 
			//	break; 
			//case DRV_STATE_LOADED:
			//	( ( COptionUI* )ctrl )->Selected( false ); 
			//	break; 
			//default: 
			//	break; 
			//}
			ctrl = NULL; 
			break; 
		case HTTP_URL_FLT_FUNCTION:
			ctrl = m_pm.FindControl( _T( "url_btn" ) ); 
			break; 
		case ANTI_ARP_ATTACK_FUNCTION:
			ctrl = m_pm.FindControl( _T( "arp_btn" ) ); 
			break; 
		case NET_COMM_MANAGE_FUNCTION: 

#ifdef DELAY_WIN7_SUPPORT
			ctrl = m_pm.FindControl( _T( "fw_btn" ) ); 
#else
			if( TRUE == is_vista_or_later_ex() )
			{
				ctrl = NULL; 
			}
			else
			{
				ctrl = m_pm.FindControl( _T( "fw_btn" ) ); 
			}
#endif //DELAY_WIN7_SUPPORT
			break; 
#ifndef DELAY_WIN7_SUPPORT
		case NET_FW_DRIVER: 	
			if( TRUE == is_vista_or_later_ex() )
			{
				ctrl = m_pm.FindControl( _T( "fw_btn" ) ); 
			}
			else
			{
				ctrl = NULL; 
			}
			break; 
#endif //DELAY_WIN7_SUPPORT

		//case SYS_MANAGE_FUNCTION: 
		//	ctrl = m_pm.FindControl( _T( "defense_btn" ) ); 
		//	break; 
		case TRACE_LOG_FUNCTION: 
			//ASSERT( FALSE && "have not control this driver directly" ); 

			//ctrl = m_pm.FindControl( _T( "url_btn" ) ); 
			//switch( all_drv_states[ i ].state )
			//{
			//case DRV_STATE_UNLOADED:
			//	( ( COptionUI* )ctrl )->Selected( true ); 
			//	break; 
			//case DRV_STATE_LOADED:
			//	( ( COptionUI* )ctrl )->Selected( false ); 
			//	break; 
			//default: 
			//	break; 
			//}
			ctrl = NULL; 
			break; 
		//case SEVEN_FW_WIN7_DRIVER: 
		//	if( TRUE == is_vista_or_later_ex() )
		//	{
		//		ctrl = m_pm.FindControl( _T( "netbase_btn" ) ); 
		//	}
		//	else
		//	{
		//		ctrl = NULL; 
		//	}
		//	break; 
		case REG_MANAGE_FUNCTION:
			//if( TRUE == is_vista_or_later_ex() )
			//{
			//	ctrl = m_pm.FindControl( _T( "reg_btn" ) ); 
			//}
			//else
			//{
				ctrl = m_pm.FindControl( _T( "reg_btn" ) ); ; 
			//}			
			break; 
		case FILE_MANAGE_FUNCTION:
			ctrl = m_pm.FindControl( _T( "fs_btn" ) ); 
			ASSERT( ctrl != NULL ); 
			break; 
		default: 
			ASSERT( FALSE && "invalid driver type " ); 

			ctrl = NULL; 
			break; 
		}
		ASSERT( TRUE == is_valid_bitsafe_function_state( load_state ) ); 


		if( ctrl == NULL )
		{
			ret = ERROR_NOT_FOUND; 
			goto _return; 
		}

		switch( load_state )
		{
		case BITSAFE_FUNCTION_OPENED:
			{
				RECT rc_closed; 
				rc_closed.left = 26; 
				rc_closed.right = 0; 
				rc_closed.top = 0; 
				rc_closed.bottom = 0; 

				if( SET_BTN_STATE == flags )
				{
					( ( COptionUI* )ctrl )->Selected( true ); 
				}

				_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_CLOSED_BTN, _T( "�ѹر�" ) ); 

				( ( CButtonUI* )ctrl )->SetTextPadding( rc_closed ); 
			}
			break; 
		case BITSAFE_FUNCTION_DISABLED:
			{
				RECT rc_opened; 
				rc_opened.left = 13; 
				rc_opened.right = 0; 
				rc_opened.top = 0; 
				rc_opened.bottom = 0; 

				if( SET_BTN_STATE == flags )
				{
					( ( COptionUI* )ctrl )->Selected( false ); 
				}
	
				_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_OPENED_BTN, _T( "�ѿ���" ) ); 

				( ( CButtonUI* )ctrl )->SetTextPadding( rc_opened ); 
			}
			break; 
		default: 
			break; 
		}

_return:
		return ret; 
	}

	LRESULT set_drv_load_state_ui()
	{
		LRESULT ret = ERROR_SUCCESS; 
		LRESULT _ret; 
		INT32 i; 
		CControlUI *ctrl = NULL; 
		bitsafe_function_state load_state; 
		ULONG checked_state_num = 0; 

		for( i = ( INT32 )FUNCTION_TYPE_BEGIN; i <= ( INT32 )FUNCTION_TYPE_END; i ++ )
		{
			ret = check_bitsafe_function_state( ( bitsafe_function_type )i, &load_state ); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BP(); 
				load_state = BITSAFE_FUNCTION_DISABLED; 
				log_trace( ( MSG_ERROR, "load driver state erro 0x%0.8x", ret ) ); 
			}
			
			//else
			{

				_ret = set_ctrl_ui_by_driver_state( ( bitsafe_function_type )i, load_state, SET_BTN_STATE ); 

				if( _ret == ERROR_SUCCESS )
				{
					checked_state_num ++; 
				}
				else
				{
					ret = _ret; 
				}
			}
		}

		//if( checked_state_num != 6 )
		//{
		//	DBG_BP(); 
		//}

		return ret; 
	}


	LRESULT set_option_dlg_lang()
	{
		LRESULT ret; 
		CControlUI *ctrl; 

		ctrl = static_cast<CButtonUI*>(m_pm.FindControl( _T( "ui_btn" ) ) ); 
		ASSERT( ctrl != NULL ); 
		ret = _set_ctrl_text( ctrl, TEXT_OPTION_DIALOG_UI_TAB, L"����" ); 
		ASSERT( ret == ERROR_SUCCESS ); 

		ctrl = static_cast<CButtonUI*>(m_pm.FindControl( _T( "protect_btn" ) ) ); 
		ret = _set_ctrl_text( ctrl, TEXT_OPTION_DIALOG_SECURITY_TAB, L"��������" ); 
		ASSERT( ret == ERROR_SUCCESS ); 

		return ERROR_SUCCESS; 
	}

	void Init() 
    {
		INT32 ret = ERROR_SUCCESS; 
		//LRESULT _ret; 
		//INT32 i; 
		//CControlUI *ctrl; 
		COptionUI *option; 

        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
  //      m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
  //      ASSERT( m_pMaxBtn != NULL ); 
		//m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
  //      ASSERT( m_pRestoreBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		option = static_cast<COptionUI*>(m_pm.FindControl( _T( "help_btn" ) ) ); 
		ASSERT( option != NULL ); 

		//tab_ctrl->SelectItem( 0 ); 
		option->Activate(); 

		//set_option_dlg_lang(); 
		//set_ui_tab_lang(); 

		//ret = load_driver_conf( &all_drv_states ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	ASSERT( FALSE ); 
		//	goto _return; 
		//}
		

		//ret = get_rule( FILE_RULE_TYPE, load_defense_rule, alloc_defense_rule, &settings ); 
		//if( ret != 0 )
		//{
		//	__Trace( _T( "get defense app rule failed \n" ) ); 
		//}
_return:
		if( ret != ERROR_SUCCESS )
		{
			Close(); 
		}

		return; 
	}

    void OnPrepare(TNotifyUI& msg) 
    {
    }

	LRESULT ctrl_drv( bitsafe_function_type type, bitsafe_function_state state )
	{
		LRESULT ret = ERROR_SUCCESS; 
		LRESULT _ret; 
		sw_ctrl drv_state_ctrl; 

		if( FALSE == is_valid_bitsafe_function_type( type ) )
		{
			DBG_BP(); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		if( FALSE == is_valid_bitsafe_function_state( state ) )
		{
			DBG_BP(); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		drv_state_ctrl.id = ( ULONG )type; 
		drv_state_ctrl.state = ( ULONG )state; 


#ifdef UNLOAD_DRIVER_BY_UNINSTALL
		switch( state )
		{
		case BITSAFE_FUNCTION_DISABLED: 
			_ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_INSTALL_DRIVER, ( PBYTE )( PVOID )&cur_sel, sizeof( cur_sel ), NULL, 0, NULL, NULL ); 
			if( _ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				//goto _return; 
			}

			_ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_LOAD_DRIVER, ( PBYTE )( PVOID )&cur_sel, sizeof( cur_sel ), NULL, 0, NULL, NULL ); 
			if( _ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				//goto _return; 

				ret = _ret; 
			}

			break; 
		case BITSAFE_FUNCTION_OPENED:

#ifdef UNLOAD_DRIVER_BY_UNINSTALL
			_ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_UNLOAD_DRIVER, ( PBYTE )( PVOID )&cur_sel, sizeof( cur_sel ), NULL, 0, NULL, NULL ); 
			if( _ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				//goto _return; 
			}
			
			_ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_UNINSTALL_DRIVER, ( PBYTE )( PVOID )&cur_sel, sizeof( cur_sel ), NULL, 0, NULL, NULL ); 
			if( _ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 

				ret = _ret; 
				//goto _return; 
			}
#endif //UNLOAD_DRIVER_BY_UNINSTALL

			break; 
		default: 
			ASSERT( FALSE && "invalid driver type" ); 
			break; 
		}
#endif //UNLOAD_DRIVER_BY_UNINSTALL

		_ret = do_ui_ctrl( BITSAFE_UI_ID, BITSAFE_SET_DRIVER_WORK_MODE, ( PBYTE )( PVOID )&drv_state_ctrl, sizeof( drv_state_ctrl ), NULL, 0, NULL, NULL ); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
			log_trace( ( MSG_ERROR, "set the driver work mode error 0x%0.8x\n", _ret ) ); 
			
			//show this tip to user by a tip window.
			//show_msg( NULL, _T( "����ģ�鵱ǰû����������,���������޸�?" ) ); 
			ASSERT( FALSE ); 
			//goto _return; 
		}
		else
		{
			_ret = set_ctrl_ui_by_driver_state( type, state, 0 ); 
			if( _ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "set the ui state for the driver state error 0x%0.8x\n", _ret ) ); 
				DBG_BP(); 
			}
		}

		_ret = config_bitsafe_function_state( type, state ); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "set the driver load state config error 0x%0.8x\n", _ret ) ); 
			DBG_BP(); 
		}

_return:
		return ret; 
	}


	LRESULT set_ui_tab_lang()
	{
		LPCTSTR tmp_text; 
		CComboUI *combo; 
		//CControlUI *ctrl; 

		set_ctrl_text( &m_pm, _T( "title_label" ), 	TEXT_LANG_TITLE, _T( "ѡ������" ) ); 
		set_ctrl_text( &m_pm, _T( "ui_ok_btn" ), TEXT_COMMON_BUTTON_CONFIRM, _T( "����" ) ); 
		//set_ctrl_text( &m_pm, _T( "cancel_btn" ), TEXT_COMMON_BUTTON_CANCEL, _T( "ȡ��" ) ); 
		set_ctrl_text( &m_pm, _T( "lang_tip_label" ), TEXT_LANG_TIP_LABEL, _T( "����" ) ); 

		combo = ( CComboUI* )m_pm.FindControl( _T( "lang_combo" ) ); 
		ASSERT( combo != NULL ); 

		tmp_text = get_string_by_id( TEXT_LANG_SELECT_TOOLTIP ); 
		if( tmp_text != NULL )
		{
			combo->SetToolTip( tmp_text ); 
		}

		return ERROR_SUCCESS; 
	}

	LRESULT on_analyze_tab_selected()
	{
		LRESULT ret = ERROR_SUCCESS; 
		LANG_ID cur_sel; 
		CEditUI *edit; 

		set_ui_tab_lang(); 

		edit = ( CEditUI * )m_pm.FindControl( _T( "symbol_path" ) ); 
		ASSERT( edit != NULL ); 

		edit->SetText( get_analyze_help_conf()->symbol_path ); 

		return ret; 
	}

	LRESULT on_ui_tab_selected()
	{
		LRESULT ret = ERROR_SUCCESS; 
		LANG_ID cur_sel; 
		CComboUI *combo; 

		set_ui_tab_lang(); 

		combo = ( CComboUI* )m_pm.FindControl( _T( "lang_combo" ) ); 
		ASSERT( combo != NULL ); 

		cur_sel = get_cur_lang_id(); 
		if( cur_sel == LANG_ID_EN )
		{
			combo->SelectListItem( 1 ); 
		}
		else if( cur_sel == LANG_ID_CN )
		{
			combo->SelectListItem( 0 ); 			
		}
		else
		{
			ASSERT( FALSE && "invalid work mode " ); 
		}

_return:
		return ret; 
	}

	LRESULT set_shield_tab_lang()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *ctrl; 

		do 
		{
			ctrl = m_pm.FindControl( _T( "txt_url" ) ); 
			ASSERT( ctrl != NULL ); 

			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_URL_FIREWALL, _T( "URL����ǽ" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_arp" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_ARP_FIREWALL, _T( "ARP����ǽ" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_netbase" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_BASE_FIREWALL, _T( "�������ǽ����" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_fw" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_TRFFIC_FIREWALL, _T( "�������ǽ" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_defense" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_DEFENSE_FIREWALL, _T( "��������" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_reg" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_REG_SHIELD, _T( "ע�������ǽ" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_fs" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_FILE_SHIELD, _T( "�ļ�ϵͳ����ǽ" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_desc_url" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_URL_FIREWALL_DESC, _T( "������վ�����ɫ��ȫ" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_desc_arp" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_ARP_FIREWALL_DESC, _T( "������������,���ARP��ƭ����" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_desc_netbase" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_BASE_FIREWALL_DESC, _T( "��������ͨ�ŵĻ���" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_desc_fw" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_TRFFIC_FIREWALL_DESC, _T( "��������ͨ��͸������" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_desc_defense" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_DEFENSE_FIREWALL_DESC, _T( "����ϵͳ��Ϊ��ȷ��ȫ" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_desc_fs" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_FILE_SHIELD_DESC, _T( "��������,�ļ���ȫ�ι�" ) ); 

			ctrl = m_pm.FindControl( _T( "txt_desc_reg" ) ); 
			ASSERT( ctrl != NULL ); 
			( ( CLabelUI* )ctrl )->SetFont( 0 ); 
			_set_ctrl_text( ctrl, TEXT_SECURITY_TAB_REG_SHIELD_DESC, _T( "����ע������ð�ȫ" ) ); 

		}while( FALSE );
		return ret; 
	}

	LRESULT on_shield_tab_selected()
	{
		LRESULT ret; 

		ret = set_shield_tab_lang(); 
		ret = set_drv_load_state_ui(); 

		return ret; 
	}

	void Notify(TNotifyUI& msg)
	{
		CStdString name; 
		bool is_sel; 
		INT32 cur_sel; 
		LRESULT ret = ERROR_SUCCESS; 
		LPCTSTR tmp_text; 

		if( msg.sType == _T( "windowinit" ) ) 
			OnPrepare(msg);
		else if( msg.sType == _T( "click" ) ) 
		{
			if( msg.pSender == m_pCloseBtn ) 
			{
				Close(); 
				return; 
			}
			else if( msg.pSender == m_pMinBtn ) 
			{ 
				SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return; 
			}
			//else if( msg.pSender == m_pMaxBtn ) 
			//{ 
			//    //SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
			//}
			//else if( msg.pSender == m_pRestoreBtn ) 
			//{ 
			//    SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
			//}

			//HTTP_TEXT_FLT_DRIVER; 
			//TRACE_LOG_DRIVER, 

			name = msg.pSender->GetName(); 

			if( name == _T( "url_btn" ) )
			{
				is_sel = ( ( COptionUI* )msg.pSender )->IsSelected(); 
				
				if( is_sel == true )
				{
					ret = ctrl_drv( HTTP_URL_FLT_DRIVER, BITSAFE_FUNCTION_DISABLED ); 
				}
				else
				{
					ret = ctrl_drv( HTTP_URL_FLT_DRIVER, BITSAFE_FUNCTION_OPENED ); 
				}

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					//goto _return; 
				}
			}
			else if( name == _T( "arp_btn" ) )
			{
				is_sel = ( ( COptionUI* )msg.pSender )->IsSelected(); 

				if( is_sel == true )
				{
					ret = ctrl_drv( ANTI_ARP_ATTACK_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
				}
				else
				{
					ret = ctrl_drv( ANTI_ARP_ATTACK_FUNCTION, BITSAFE_FUNCTION_OPENED ); 
				}

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					//goto _return; 
				}
			}
			else if( name == _T( "netbase_btn" ) )
			{
				is_sel = ( ( COptionUI* )msg.pSender )->IsSelected(); 

				if( TRUE == is_vista_or_later_ex() )
				{
					if( is_sel == true )
					{
						ret = ctrl_drv( SEVEN_FW_WIN7_DRIVER, BITSAFE_FUNCTION_DISABLED ); 
					}
					else
					{
						ret = ctrl_drv( SEVEN_FW_WIN7_DRIVER, BITSAFE_FUNCTION_OPENED ); 
					}
				}
				else
				{
					if( is_sel == true )
					{
						ret = ctrl_drv( NET_PACKET_FLT_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
					}
					else
					{
						ret = ctrl_drv( NET_PACKET_FLT_FUNCTION, BITSAFE_FUNCTION_OPENED ); 
					}
				}

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					//goto _return; 
				}
			}
			else if( name == _T( "fw_btn" ) )
			{
				is_sel = ( ( COptionUI* )msg.pSender )->IsSelected(); 

				if( TRUE == is_vista_or_later_ex() )
				{
					if( is_sel == true )
					{
						ret = ctrl_drv( NET_FW_DRIVER, BITSAFE_FUNCTION_DISABLED ); 
					}
					else
					{
						ret = ctrl_drv( NET_FW_DRIVER, BITSAFE_FUNCTION_OPENED ); 
					}
				}
				else
				{
					if( is_sel == true )
					{
						ret = ctrl_drv( NET_COMM_MANAGE_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
					}
					else
					{
						ret = ctrl_drv( NET_COMM_MANAGE_FUNCTION, BITSAFE_FUNCTION_OPENED ); 
					}
				}

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					//goto _return; 
				}
			}
			else if( name == _T( "defense_btn" ) )
			{
				is_sel = ( ( COptionUI* )msg.pSender )->IsSelected(); 

				if( is_sel == true )
				{
					ret = ctrl_drv( SYS_MANAGE_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
				}
				else
				{
					ret = ctrl_drv( SYS_MANAGE_FUNCTION, BITSAFE_FUNCTION_OPENED ); 
				}

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					//goto _return; 
				}
			}
			else if( name == _T( "fs_btn" ) )
			{
				is_sel = ( ( COptionUI* )msg.pSender )->IsSelected(); 

				if( is_sel == true )
				{
					ret = ctrl_drv( FILE_MANAGE_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
				}
				else
				{
					ret = ctrl_drv( FILE_MANAGE_FUNCTION, BITSAFE_FUNCTION_OPENED ); 
				}

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					//goto _return; 
				}
			}
			else if( name == _T( "reg_btn" ) )
			{
				is_sel = ( ( COptionUI* )msg.pSender )->IsSelected(); 

				if( is_sel == true )
				{
					ret = ctrl_drv( REG_MANAGE_FUNCTION, BITSAFE_FUNCTION_DISABLED ); 
				}
				else
				{
					ret = ctrl_drv( REG_MANAGE_FUNCTION, BITSAFE_FUNCTION_OPENED ); 
				}

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					//goto _return; 
				}
			}
			else if( name == _T( "symbol_ok_btn" ) )
			{
				LRESULT ret; 
				CEditUI *edit; 
				COptionUI *option; 
				CStdString symbol_path; 
				ULONG flags; 

				do 
				{
					edit = ( CEditUI* )m_pm.FindControl( _T( "symbol_path" ) ); 
					ASSERT( edit != NULL ); 

					option = ( COptionUI* )m_pm.FindControl( _T( "auto_download" ) ); 
					ASSERT( option != NULL ); 

					symbol_path = edit->GetText(); 

					if( symbol_path.GetLength() == 0 )
					{
						tmp_text = _T( "��������ȷ�ķ��ű���Ŀ¼·��" ); 

						show_msg( GetHWND(), tmp_text ); 
						log_trace( ( MSG_ERROR, "!!!set current work mode failed \n" ) ); 
						break; 
					}

					ret = check_dir_exist( symbol_path ); 

					if( option->IsSelected() == true || ret != ERROR_SUCCESS )
					{
						flags = NO_TIP_DEFAULT_SYMBOL_CHECK | FORMAT_LOCAL_SYMBOL_PATH; 
					}
					else
					{
						flags = NO_TIP_DEFAULT_SYMBOL_CHECK; 
					}

					ret = set_analyze_help_conf( symbol_path.GetData(), NULL, flags ); 
					if( ret != ERROR_SUCCESS )
					{

					}

					ret = save_common_conf(); 

					if( ret != ERROR_SUCCESS )
					{
						tmp_text = _T( "���渨������ʧ��" ); 

						show_msg( GetHWND(), tmp_text ); 
						log_trace( ( MSG_ERROR, "!!!set current work mode failed \n" ) ); 
					}
					else
					{
						tmp_text = _T( "���渨�����óɹ�" ); 

						show_msg( GetHWND(), tmp_text );  
					}
				}while( FALSE );

			}
			else if( name == _T( "sym_browser" ) )
			{
				CEditUI *edit; 
				CStdString symbol_path; 
				WCHAR file_name[ MAX_PATH ]; 

				do 
				{
					*file_name = _T( '\0' ); 

					edit = ( CEditUI* )m_pm.FindControl( _T( "symbol_path" ) ); 
					ASSERT( edit != NULL ); 

					ret = open_folder_dlg( ( LPWSTR )file_name, ARRAYSIZE( file_name ) ); 
					if( ret < 0 )
					{
						break; 
					}

					edit->SetText( file_name ); 
				}while( FALSE );
			}
			else if( name == _T( "ui_ok_btn" ) )
			{
				CComboUI *combo; 
				LANG_ID cur_lang; 
				LANG_ID sel_lang; 

				cur_lang = get_cur_lang_id(); 

				combo = ( CComboUI* )m_pm.FindControl( _T( "lang_combo" ) ); 
				ASSERT( combo != NULL ); 

				cur_sel = combo->GetCurSel(); 
				if( cur_sel == 0 )
				{
					sel_lang = LANG_ID_CN; 
				}
				else if( cur_sel == 1 )
				{
					sel_lang = LANG_ID_EN; 
				}
				else
				{
					ASSERT( FALSE && "invalid work mode " ); 
					sel_lang = LANG_ID_CN; 
				}

				if( sel_lang != cur_lang )
				{
					notify_data data_notify; 

					ret = set_cur_lang( sel_lang ); 

					data_notify.data_len = sizeof( LANG_ID ); 
					data_notify.data = &sel_lang; 

					if( ui_notify != NULL )
					{
						ret = ui_notify->action_notify( UI_LANG_NOTIFY, &data_notify ); 
					}

					set_option_dlg_lang(); 
					set_ui_tab_lang(); 
					//save_lang_conf(); 
				}

				if( ret != ERROR_SUCCESS )
				{
					tmp_text = _get_string_by_id( TEXT_LANG_SET_FAILED, _T( "��������ʧ��" ) ); 

					show_msg( GetHWND(), tmp_text ); 
					log_trace( ( MSG_ERROR, "!!!set current work mode failed \n" ) ); 
				}
				else
				{
					tmp_text = _get_string_by_id( TEXT_LANG_SET_SUCCESSFULLY, _T( "�������Գɹ�" ) ); 

					show_msg( GetHWND(), tmp_text );  
				}

				//Close(); 
			}
			//else if( name == _T( "ui_cancel_btn" ) )
			//{
			//	Close(); 
			//}
		}

		else if( msg.sType == _T( "selectchanged" ) )
		{
			CTabLayoutUI* tab_container = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("frame")));
			name = msg.pSender->GetName(); 
			//LPCTSTR tmp_text; 

			ASSERT( tab_ctrl != NULL ); 

			__Trace( _T( "the set focus target control is %s \n" ), name ); 
			
			if( name == _T( "trans_btn" ) )
			{
				tab_container->SelectItem( 0 ); 
				on_analyze_tab_selected(); 
			}
			//else if(name==_T("ui_btn"))
			//{
			//	//tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_SUMMARY, SUMMARY_TITLE ); 

			//	tab_container->SelectItem( 0 );
			//	on_ui_tab_selected(); 
			//}
			//else if(name==_T("protect_btn"))
			//{
			//	//tmp_text = _get_string_by_id( TEXT_MAIN_BUTTON_NETWORK, NET_MANAGE_TITLE ); 

			//	tab_container->SelectItem( 1 );
			//	on_shield_tab_selected(); 
			//}
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
		else if(msg.sType == _T("menu")) 
		{
			//setting_menu* pMenu = new setting_menu();
			//if( pMenu == NULL )
			//{ 
			//	return; 
			//}
			//POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
			//::ClientToScreen(*this, &pt);
			//pMenu->Init(msg.pSender, pt);
		}
_return:
		return; 
	}

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		
		hide_sys_menu( GetHWND() ); 

		m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
		tabs_builder cb; 
        CControlUI* pRoot = builder.Create(_T("option_dlg.xml"), (UINT)0, &cb, &m_pm);
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
	CButtonUI* m_pMinBtn; 

	CTabLayoutUI *tab_ctrl; 

	action_ui_notify *ui_notify; 

	//drv_open_state all_drv_states; 
};