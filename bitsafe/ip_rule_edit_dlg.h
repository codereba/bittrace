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
 
 #pragma once

//#include "menu_ui.h"
#include "bitsafe_common.h"
#include "conf_file.h"
#include "bitsafe_rule_conf.h"
#include "msg_box.h"

class ip_rule_edit_dlg : public CWindowWnd, public INotifyUI 
{
public:
	ip_rule_edit_dlg( access_rule_desc *param_output, access_rule_type type = URL_RULE_TYPE, INT32 need_init = FALSE ) : success( FALSE )
	{
		output_param = param_output; 
		rule_type = type; 
		this->need_init = need_init; 
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("ip_rule_edit_dlg"); 
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
		LRESULT ret = ERROR_SUCCESS; 
		CEditUI *app_name_edit; 
		CEditUI *sip_begin_edit; 
		CEditUI *dip_begin_edit; 
		CEditUI *sport_begin_edit; 
		CEditUI *dport_begin_edit; 
		CEditUI *sip_end_edit; 
		CEditUI *dip_end_edit; 
		CEditUI *sport_end_edit; 
		CEditUI *dport_end_edit; 
		CComboUI *action_combo; 
		CComboUI *prot_combo; 
		CEditUI *desc_edit;  
		ULONG ip_addr; 
		ULONG selected; 
		USHORT port_val; 

		if( output_param == NULL )
		{
			goto _return; 
		}

		app_name_edit = ( CEditUI* )m_pm.FindControl( _T( "app_name_edit" ) ); 
		ASSERT( app_name_edit != NULL ); 

		sip_begin_edit = ( CEditUI* )m_pm.FindControl( _T( "sip_begin_edit" ) );
		ASSERT( sip_begin_edit != NULL ); 

		dip_begin_edit = ( CEditUI* )m_pm.FindControl( _T( "dip_begin_edit" ) ); 
		ASSERT( dip_begin_edit != NULL ); 

		sport_begin_edit = ( CEditUI* )m_pm.FindControl( _T( "sport_begin_edit" ) );
		ASSERT( sport_begin_edit != NULL ); 

		dport_begin_edit = ( CEditUI* )m_pm.FindControl( _T( "dport_begin_edit" ) );
		ASSERT( dport_begin_edit != NULL ); 

		sip_end_edit = ( CEditUI* )m_pm.FindControl( _T( "sip_end_edit" ) );
		ASSERT( sip_end_edit != NULL ); 

		dip_end_edit = ( CEditUI* )m_pm.FindControl( _T( "dip_end_edit" ) );
		ASSERT( dip_end_edit != NULL ); 

		sport_end_edit = ( CEditUI* )m_pm.FindControl( _T( "sport_end_edit" ) );
		ASSERT( sport_end_edit != NULL ); 

		dport_end_edit = ( CEditUI* )m_pm.FindControl( _T( "dport_end_edit" ) );
		ASSERT( dport_end_edit != NULL ); 

		prot_combo = ( CComboUI* )m_pm.FindControl( _T( "prot_combo" ) );
		ASSERT( prot_combo != NULL ); 

		action_combo = ( CComboUI* )m_pm.FindControl( _T( "action_combo" ) );
		ASSERT( action_combo != NULL ); 

		desc_edit = ( CEditUI* )m_pm.FindControl( _T( "desc_edit" ) );  
		ASSERT( desc_edit != NULL ); 
		
		ret = ip_addr_2_str( output_param->desc.socket.src_ip.ip.ip_begin, tmp, 64 ); 

		sip_begin_edit->SetText( tmp ); 

		ret = ip_addr_2_str( output_param->desc.socket.src_ip.ip.ip_end, tmp, 64 ); 

		sip_end_edit->SetText( tmp ); 

		ret = ip_addr_2_str( output_param->desc.socket.dest_ip.ip.ip_begin, tmp, 64 ); 

		dip_begin_edit->SetText( tmp ); 

		ret = ip_addr_2_str( output_param->desc.socket.dest_ip.ip.ip_end, tmp, 64 ); 

		dip_end_edit->SetText( tmp ); 

		ret = port_2_str( output_param->desc.socket.src_port.port.port_begin, tmp, 64 ); 

		sport_begin_edit->SetText( tmp ); 

		ret = port_2_str( output_param->desc.socket.src_port.port.port_end, tmp, 64 ); 

		sport_begin_edit->SetText( tmp ); 

		ret = port_2_str( output_param->desc.socket.dest_port.port.port_begin, tmp, 64 ); 

		dport_begin_edit->SetText( tmp ); 

		ret = port_2_str( output_param->desc.socket.dest_port.port.port_end, tmp, 64 ); 

		dport_end_edit->SetText( tmp ); 

		if( output_param->desc.socket.app.app.app_name[ _MAX_FILE_NAME_LEN - 1 ] != L'\0' )
		{
			output_param->desc.socket.app.app.app_name[ _MAX_FILE_NAME_LEN - 1 ] = L'\0';  
		}

		app_name_edit->SetText( output_param->desc.socket.app.app.app_name ); 
		
		//if( output_param->addition[ _MAX_DESC_LEN - 1 ] != L'\0' )
		//{
		//	output_param->addition[ _MAX_DESC_LEN - 1 ] != L'\0'; 
		//}

		desc_edit->SetText( _T( "" ) ); 

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
			action_combo->SelectListItem( 2 ); 
		}

		if( output_param->desc.socket.src_port.port.type == PROT_TCP )
		{
			prot_combo->SelectListItem( 0 ); 
		}
		else if(output_param->desc.socket.src_port.port.type == PROT_UDP )
		{
			prot_combo->SelectListItem( 1 ); 
		}
		else if( output_param->desc.socket.src_port.port.type == 0 )
		{
			prot_combo->SelectListItem( 2 ); 
		}

_return:
		return ret; 
	}
	; 
    void Init() 
    {
		CControlUI *title_txt; 
		CControlUI *name_txt; 
		CComboUI *action_combo; 
		CComboUI *prot_combo; 
		LPCTSTR tmp_text; 
		CControlUI *ctrl; 

		action_combo = ( CComboUI* )m_pm.FindControl( _T( "action_combo" ) );
		ASSERT( action_combo != NULL ); 

		//if( rule_type == SOCKET_RULE_TYPE )
		{
			action_combo->SelectListItem( 0 ); 
			action_combo->RemoveItemAt( 2 ); 
		}

		m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 

		prot_combo = ( CComboUI* )m_pm.FindControl( _T( "prot_combo" ) );
		ASSERT( prot_combo != NULL ); 

		tmp_text = get_string_by_id( TEXT_IP_EDIT_PROT_SEL_TIP ); 
		if( tmp_text != NULL )
		{
			prot_combo->SetToolTip( tmp_text ); 
		}

		ctrl = prot_combo->GetItemAt( 0 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_IP_EDIT_PROT_TCP ); 

		ctrl = prot_combo->GetItemAt( 1 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_IP_EDIT_PROT_UDP ); 

		ctrl = prot_combo->GetItemAt( 2 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_IP_EDIT_PROT_ALL ); 

		//title_txt = m_pm.FindControl( _T( "title_txt" ) ); 
		//ASSERT( title_txt != NULL ); 
		
		//name_txt = m_pm.FindControl( _T( "name_txt" ) ); 
		//ASSERT( name_txt != NULL ); 

		//m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
  //      ASSERT( m_pRestoreBtn != NULL ); 
		//m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		//ASSERT( m_pMinBtn != NULL ); 

		set_ctrl_text( &m_pm, _T( "opera_txt" ), TEXT_IP_EDIT_OPERA ); 
		set_ctrl_text( &m_pm, _T( "target_txt" ), TEXT_IP_EDIT_APP ); 
		set_ctrl_text( &m_pm, _T( "desc_txt" ), TEXT_IP_EDIT_DESC ); 
		set_ctrl_text( &m_pm, _T( "type_txt" ), TEXT_IP_EDIT_PROTOCOL ); 
		set_ctrl_text( &m_pm, _T( "dport_txt" ), TEXT_IP_EDIT_DEST_PORT ); 
		set_ctrl_text( &m_pm, _T( "sport_txt" ), TEXT_IP_EDIT_SOURCE_PORT ); 
		set_ctrl_text( &m_pm, _T( "dip_txt" ), TEXT_IP_EDIT_DEST_IP ); 
		set_ctrl_text( &m_pm, _T( "sip_txt" ), TEXT_IP_EDIT_SOURCE_IP ); 
		set_ctrl_text( &m_pm, _T( "apply_btn" ), TEXT_COMMON_BUTTON_CONFIRM ); ; 

		if( need_init == TRUE )
		{
			init_input_param(); 
		}
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

	LRESULT on_apply()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CEditUI *app_name_edit; 
		CEditUI *sip_begin_edit; 
		CEditUI *dip_begin_edit; 
		CEditUI *sport_begin_edit; 
		CEditUI *dport_begin_edit; 
		CEditUI *sip_end_edit; 
		CEditUI *dip_end_edit; 
		CEditUI *sport_end_edit; 
		CEditUI *dport_end_edit; 
		CComboUI *action_combo; 
		CComboUI *prot_combo; 
		CEditUI *desc_edit;  
		ULONG ip_addr; 
		ULONG selected; 
		USHORT port_val; 
		LPCTSTR tmp_text; 

		if( output_param == NULL )
		{
			goto _return; 
		}

		init_access_rule( SOCKET_RULE_TYPE, output_param ); 

		output_param->desc.socket.app.type = APP_DEFINE; 
		output_param->desc.socket.src_ip.type = IP_DEFINE; 
		output_param->desc.socket.src_port.type = PORT_DEFINE; 
		output_param->desc.socket.dest_port.type = PORT_DEFINE; 
		output_param->desc.socket.dest_ip.type = IP_DEFINE; 

		app_name_edit = ( CEditUI* )m_pm.FindControl( _T( "app_name_edit" ) ); 
		ASSERT( app_name_edit != NULL ); 

		sip_begin_edit = ( CEditUI* )m_pm.FindControl( _T( "sip_begin_edit" ) );
		ASSERT( sip_begin_edit != NULL ); 

		dip_begin_edit = ( CEditUI* )m_pm.FindControl( _T( "dip_begin_edit" ) ); 
		ASSERT( dip_begin_edit != NULL ); 

		sport_begin_edit = ( CEditUI* )m_pm.FindControl( _T( "sport_begin_edit" ) );
		ASSERT( sport_begin_edit != NULL ); 

		dport_begin_edit = ( CEditUI* )m_pm.FindControl( _T( "dport_begin_edit" ) );
		ASSERT( dport_begin_edit != NULL ); 

		sip_end_edit = ( CEditUI* )m_pm.FindControl( _T( "sip_end_edit" ) );
		ASSERT( sip_end_edit != NULL ); 

		dip_end_edit = ( CEditUI* )m_pm.FindControl( _T( "dip_end_edit" ) );
		ASSERT( dip_end_edit != NULL ); 

		sport_end_edit = ( CEditUI* )m_pm.FindControl( _T( "sport_end_edit" ) );
		ASSERT( sport_end_edit != NULL ); 

		dport_end_edit = ( CEditUI* )m_pm.FindControl( _T( "dport_end_edit" ) );
		ASSERT( dport_end_edit != NULL ); 

		prot_combo = ( CComboUI* )m_pm.FindControl( _T( "prot_combo" ) );
		ASSERT( prot_combo != NULL ); 

		action_combo = ( CComboUI* )m_pm.FindControl( _T( "action_combo" ) );
		ASSERT( action_combo != NULL ); 

		desc_edit = ( CEditUI* )m_pm.FindControl( _T( "desc_edit" ) );  
		ASSERT( desc_edit != NULL ); 

		output_param->type = SOCKET_RULE_TYPE; 
		if( desc_edit->GetText().GetLength() > _MAX_DESC_LEN )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_DESC_LENGTH_ERROR, 
				_T( "描述信息长度不得大于512" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		if( app_name_edit->GetText().GetLength() > _MAX_FILE_NAME_LEN )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_APP_PATH_LENGTH_ERROR, 
				_T( "目标程序路径不得大于260" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		ret = str_2_ipv4_addr( sip_begin_edit->GetText(), &ip_addr ); 
		output_param->desc.socket.src_ip.type = IP_DEFINE; 

		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_BEGIN_SOURCE_IP_FMT_ERROR, 
				_T( "源起始IP地址格式不正确" ) ); 
			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		output_param->desc.socket.src_ip.ip.ip_begin = ip_addr; 

		ret = str_2_ipv4_addr( sip_end_edit->GetText(), &ip_addr ); 

		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_END_SOURCE_IP_FMT_ERROR, 
				_T( "源结束IP地址格式不正确" ) ); 
			
			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		output_param->desc.socket.src_ip.ip.ip_end = ip_addr; 

		ret = str_2_ipv4_addr( dip_begin_edit->GetText(), &ip_addr ); 

		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_BEGIN_DESTINATION_IP_FMT_ERROR, 
				_T( "目标起始IP地址格式不正确" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		output_param->desc.socket.dest_ip.ip.ip_begin = ip_addr; 

		ret = str_2_ipv4_addr( dip_end_edit->GetText(), &ip_addr ); 

		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_END_DESTINATION_IP_FMT_ERROR, 
				_T( "目标结束IP地址格式不正确" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		output_param->desc.socket.dest_ip.ip.ip_end = ip_addr; 

		ret = str_2_port( sport_begin_edit->GetText().GetData(), &port_val ); 
		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_BEGIN_SOURCE_PORT_FMT_ERROR, 
				_T( "源起始端口格式不正确" ) ); 
			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}
		output_param->desc.socket.src_port.port.port_begin = port_val; 
		
		ret = str_2_port( sport_end_edit->GetText().GetData(), &port_val ); 
		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_END_SOURCE_PORT_FMT_ERROR, 
				_T( "源结束端口格式不正确" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}
		output_param->desc.socket.src_port.port.port_end = port_val; 
		
		ret = str_2_port( dport_begin_edit->GetText().GetData(), &port_val ); 
		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_BEGIN_DESTINATION_PORT_FMT_ERROR, 
				_T( "目标起始端口格式不正确" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}
		output_param->desc.socket.dest_port.port.port_begin = port_val; 
		
		ret = str_2_port( dport_end_edit->GetText().GetData(), &port_val ); 
		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_END_DESTINATION_PORT_FMT_ERROR, 
				_T( "目标结束端口格式不正确" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}
		output_param->desc.socket.dest_port.port.port_end = port_val; 

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
		else
		{
			ASSERT( FALSE ); 
		}

		selected = prot_combo->GetCurSel(); 
		if( selected == 0 )
		{
			output_param->desc.socket.src_port.port.type = PROT_TCP; 
			output_param->desc.socket.dest_port.port.type = PROT_TCP; 
		}
		else if( selected == 1 )
		{
			output_param->desc.socket.src_port.port.type = PROT_UDP; 
			output_param->desc.socket.dest_port.port.type = PROT_UDP; 
		}
		else if( selected == 2 )
		{
			output_param->desc.socket.src_port.port.type = ALL_PROT; 
			output_param->desc.socket.dest_port.port.type = ALL_PROT; 
		}
		else
		{
			ASSERT( FALSE ); 
		}

		if( desc_edit->GetText().GetLength() > _MAX_DESC_LEN )
		{	
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_DESC_LENGTH_ERROR, 
				_T( "描述长度不得大于512" ) ); 

			show_msg( GetHWND(), tmp_text , NULL, _get_string_by_id( TEXT_ACL_EDIT_INPUT_ERROR, 
				_T( "输入错误" ) ) ); 
			goto _return; 
		}

		//output_param->type = SOCKET_RULE_TYPE; 
		//wcsncpy( output_param->addition, desc_edit->GetText().GetData(), _MAX_DESC_LEN ) ; 

		output_param->desc.common.app.is_cls = FALSE; 
		output_param->desc.common.app.type = APP_DEFINE; 
		wcsncpy( output_param->desc.common.app.app.app_name, app_name_edit->GetText().GetData(), _MAX_DESC_LEN ); 
		output_param->type = SOCKET_RULE_TYPE; 
		
		if( output_param->desc.socket.src_ip.ip.ip_begin == 0 
			&& output_param->desc.socket.src_ip.ip.ip_end == 0 
			&& output_param->desc.socket.dest_ip.ip.ip_begin == 0 
			&& output_param->desc.socket.dest_ip.ip.ip_end == 0 )
		{
			tmp_text = _get_string_by_id( TEXT_ACL_EDIT_SOURCE_IP_AND_DEST_IP_ALL_IS_NULL_ERROR, 
				_T( "源IP地址他目标IP不可同时不设定" ) ); 

			show_msg( GetHWND(), tmp_text ); 
			goto _return; 
		}

		//if( output_param->desc.socket.src_port.port.port_begin == 0 
		//	&& output_param->desc.socket.src_port.port.port_end == 0 
		//	&&output_param->desc.socket.dest_port.port.port_begin == 0 
		//	&& output_param->desc.socket.dest_port.port.port_end == 0 )
		//{
		//	show_msg( GetHWND(), _T( "源端口和目标端口不可以同时不设定" ) ); 
		//	goto _return; 
		//}

		success = TRUE; 

		//show_msg( GetHWND(), _T( "添加套接字规则成功" ) ); 

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

	INT32 create_rule_success()
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
        CControlUI* pRoot = builder.Create( _T( "ip_rule_edit.xml" ), (UINT)0, NULL, &m_pm );
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
	VOID set_output_param( access_rule_desc *param )
	{
		output_param = param; 
	}

private:
	access_rule_desc *output_param; 
    CButtonUI* m_pCloseBtn;
    access_rule_type rule_type; 
	INT32 need_init; 
	INT32 success; 
};