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

#include "setting_menu_ui.h"
#include "conf_file.h"
#include "tab_builder.h"
#include "bitsafe_rule_conf.h"
#include "name_rule_edit_dlg.h"

typedef enum _setting_mode
{
	FW_SETTING, 
	DEFENSE_SETTING
} setting_mode; 

class setting_dlg : public CWindowWnd, public INotifyUI, public IListCallbackUI
{
public:
	setting_dlg( setting_mode work_mode ) : 
		cur_rule_type( FILE_RULE_TYPE )
	{
		mode = work_mode; 
		memset( &all_rule_lists, 0, sizeof( all_rule_lists ) ); 
    };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("SettingDlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

	LRESULT init_setting_dlg_text( setting_mode cur_mode )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CListHeaderUI *list_header; 
		CControlUI *ctrl; 

		ASSERT( all_rule_lists[ FILE_RULE_TYPE ] != NULL ); 
		ASSERT( all_rule_lists[ URL_RULE_TYPE ] != NULL ); 
		ASSERT( all_rule_lists[ REG_RULE_TYPE ] != NULL ); 
		ASSERT( all_rule_lists[ SOCKET_RULE_TYPE ] != NULL ); 

		switch( cur_mode )
		{
		case DEFENSE_SETTING: 
			list_header = all_rule_lists[ FILE_RULE_TYPE ]->GetHeader(); 
			ASSERT( list_header != NULL ); 

			ctrl = list_header->GetItemAt( 0 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_FILE_RULE_APP_NAME ); 

			ctrl = list_header->GetItemAt( 1 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_FILE_RULE_FILE_PATH ); 

			ctrl = list_header->GetItemAt( 2 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_FILE_RULE_ACTION ); 

			ctrl = list_header->GetItemAt( 3 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_FILE_RULE_DESCRIPTION ); 

			list_header = all_rule_lists[ REG_RULE_TYPE ]->GetHeader(); 
			ASSERT( list_header != NULL ); 

			ctrl = list_header->GetItemAt( 0 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_FILE_RULE_APP_NAME ); 

			ctrl = list_header->GetItemAt( 1 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_REG_RULE_REG_PATH ); 

			ctrl = list_header->GetItemAt( 2 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_FILE_RULE_ACTION ); 

			ctrl = list_header->GetItemAt( 3 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_FILE_RULE_DESCRIPTION ); 

			break; 
		case FW_SETTING: 
			list_header = all_rule_lists[ URL_RULE_TYPE ]->GetHeader(); 
			ASSERT( list_header != NULL ); 

			ctrl = list_header->GetItemAt( 0 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_URL_RULE_APP_NAME ); 

			ctrl = list_header->GetItemAt( 1 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_URL_RULE_URL ); 


			ctrl = list_header->GetItemAt( 2 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_URL_RULE_ACTION ); 


			ctrl = list_header->GetItemAt( 3 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_URL_RULE_DESCRIPTION ); 

			list_header = all_rule_lists[ SOCKET_RULE_TYPE ]->GetHeader(); 
			ASSERT( list_header != NULL ); 

			ctrl = list_header->GetItemAt( 0 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_URL_RULE_APP_NAME ); 

			ctrl = list_header->GetItemAt( 1 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_IP_RULE_SOURCE_IP_BEGIN ); 

			ctrl = list_header->GetItemAt( 2 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_IP_RULE_SOURCE_IP_END ); 

			ctrl = list_header->GetItemAt( 3 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_IP_RULE_SOURCE_PORT_BEGIN ); 

			ctrl = list_header->GetItemAt( 4 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_IP_RULE_SOURCE_PORT_END ); 

			ctrl = list_header->GetItemAt( 5 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_IP_RULE_DEST_IP_BEGIN ); 

			ctrl = list_header->GetItemAt( 6 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_IP_RULE_DEST_IP_END ); 

			ctrl = list_header->GetItemAt( 7 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_IP_RULE_DEST_PORT_BEGIN ); 

			ctrl = list_header->GetItemAt( 8 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_IP_RULE_DEST_PORT_END ); 

			ctrl = list_header->GetItemAt( 9 ); 
			ASSERT( ctrl != NULL ); 
			_set_ctrl_text( ctrl, TEXT_IP_RULE_PROTO ); 

			break; 
		default:
			break; 
		}

		set_ctrl_text( &m_pm, _T( "add_btn" ), TEXT_RULE_CTRL_BUTTON_ADD ); 
		set_ctrl_text( &m_pm, _T( "edit_btn" ), TEXT_RULE_CTRL_BUTTON_EDIT ); 
		set_ctrl_text( &m_pm, _T( "del_btn" ), TEXT_RULE_CTRL_BUTTON_DELETE ); 

		return ret; 
	}

    void Init() 
    {
		INT32 ret; 
		INT32 i; 
		COptionUI *tab_btn; 

        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
  //      m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
  //      ASSERT( m_pMaxBtn != NULL ); 
		//m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
  //      ASSERT( m_pRestoreBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		all_rule_lists[ FILE_RULE_TYPE ] = static_cast<CListUI*>( m_pm.FindControl( _T( "file_rule_list" ) ) );
		ASSERT( all_rule_lists[ FILE_RULE_TYPE ] != NULL ); 
		all_rule_lists[ FILE_RULE_TYPE ]->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID );
		all_rule_lists[ FILE_RULE_TYPE ]->SetTextCallback( this ); 

		all_rule_lists[ URL_RULE_TYPE ] = static_cast<CListUI*>( m_pm.FindControl( _T( "url_rule_list" ) ) );
		ASSERT( all_rule_lists[ URL_RULE_TYPE ] != NULL ); 
		all_rule_lists[ URL_RULE_TYPE ]->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID );
		all_rule_lists[ URL_RULE_TYPE ]->SetTextCallback( this ); 

		all_rule_lists[ REG_RULE_TYPE ] = static_cast<CListUI*>( m_pm.FindControl( _T( "reg_rule_list" ) ) );
		ASSERT( all_rule_lists[ REG_RULE_TYPE ] != NULL ); 
		all_rule_lists[ REG_RULE_TYPE ]->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID );
		all_rule_lists[ REG_RULE_TYPE ]->SetTextCallback( this ); 

		all_rule_lists[ SOCKET_RULE_TYPE ] = static_cast<CListUI*>( m_pm.FindControl( _T( "ip_rule_list" ) ) );
		ASSERT( all_rule_lists[ SOCKET_RULE_TYPE ] != NULL ); 
		all_rule_lists[ SOCKET_RULE_TYPE ]->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID );
		all_rule_lists[ SOCKET_RULE_TYPE ]->SetTextCallback( this );

		tab_ctrl = ( CTabLayoutUI* )m_pm.FindControl( _T( "frame" ) ); 
		ASSERT( tab_ctrl != NULL ); 

		if( mode == DEFENSE_SETTING )
		{
			tab_btn = static_cast< COptionUI* >( m_pm.FindControl( _T( "tab2_btn" ) ) ); 
			ASSERT( tab_btn != NULL ); 

			//tab_ctrl->SelectItem( 2 ); 

			tab_btn->SetSelectedHotImage( _T( "reg_btn_down.png" ) ); 
			tab_btn->SetSelectedImage( _T( "reg_btn_down.png" ) ); 
			tab_btn->SetPushedImage( _T( "reg_btn_down.png" ) ); 
			tab_btn->SetNormalImage( _T( "reg_btn_nor.png" ) ); 
			tab_btn->SetHotImage( _T( "reg_btn_hover.png" ) ); 

			tab_btn = static_cast< COptionUI* >( m_pm.FindControl( _T( "tab1_btn" ) ) ); 
			ASSERT( tab_btn != NULL ); 

			tab_btn->SetSelectedHotImage( _T( "file_btn_down.png" ) ); 
			tab_btn->SetSelectedImage( _T( "file_btn_down.png" ) ); 
			tab_btn->SetPushedImage( _T( "file_btn_down.png" ) ); 
			tab_btn->SetNormalImage( _T( "file_btn_nor.png" ) ); 
			tab_btn->SetHotImage( _T( "file_btn_hover.png" ) ); 

			init_setting_dlg_text( mode ); 

			for( i = 0; i < all_rules[ FILE_RULE_TYPE ].size(); i ++ )
			{
				add_list_item( all_rule_lists[ FILE_RULE_TYPE ], i ); 
			}

			tab_btn->Activate(); 
		}
		else if( mode == FW_SETTING )
		{
			tab_btn = static_cast< COptionUI* >( m_pm.FindControl( _T( "tab1_btn" ) ) ); 
			ASSERT( tab_btn != NULL ); 

			//tab_ctrl->SelectItem( 0 ); 
			tab_btn->SetSelectedHotImage( _T( "url_btn_down.png" ) ); 
			tab_btn->SetSelectedImage( _T( "url_btn_down.png" ) ); 
			tab_btn->SetPushedImage( _T( "url_btn_down.png" ) ); 
			tab_btn->SetNormalImage( _T( "url_btn_nor.png" ) ); 
			tab_btn->SetHotImage( _T( "url_btn_hover.png" ) ); 

			tab_btn->Activate(); 

			ASSERT( all_rule_lists[ URL_RULE_TYPE ] != NULL ); 

			for( i = 0; i < all_rules[ URL_RULE_TYPE ].size(); i ++ )
			{
				add_list_item( all_rule_lists[ URL_RULE_TYPE ], i ); 
			}
			
			tab_btn = static_cast< COptionUI* >( m_pm.FindControl( _T( "tab2_btn" ) ) ); 
			ASSERT( tab_btn != NULL ); 

			tab_btn->SetSelectedHotImage( _T( "ip_btn_down.png" ) ); 
			tab_btn->SetSelectedImage( _T( "ip_btn_down.png" ) ); 
			tab_btn->SetPushedImage( _T( "ip_btn_down.png" ) ); 
			tab_btn->SetNormalImage( _T( "ip_btn_nor.png" ) ); 
			tab_btn->SetHotImage( _T( "ip_btn_hover.png" ) ); 
		}
		else
		{
			ASSERT( FALSE ); 
		}

		//ret = get_rule( FILE_RULE_TYPE, load_defense_rule, alloc_defense_rule, &settings ); 
		//if( ret != 0 )
		//{
		//	__Trace( _T( "get defense app rule failed \n" ) ); 
		//}
	}

    void OnPrepare(TNotifyUI& msg) 
    {
    }
 
    /*
    * 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
    */
    LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
    {
        TCHAR szBuf[ITEM_TEXT_BUF_LEN] = {0};
		access_rule_desc *rule_input; 
		access_rule_queue *cur_action_rules; 

		log_trace( ( MSG_INFO, "enter %s index is %d, sub index is %d\n", __FUNCTION__, iIndex, iSubItem ) ); 
		cur_action_rules = &all_rules[ cur_rule_type ]; 

		ASSERT( iIndex < cur_action_rules->size() ); 
		rule_input = &cur_action_rules->operator [](  iIndex )->rule; 

		get_rule_column_text( szBuf, ITEM_TEXT_BUF_LEN, rule_input, 
			cur_action_rules->operator [](  iIndex )->desc, iSubItem ); 

        pControl->SetUserData(szBuf);

		log_trace( ( MSG_INFO, "leave %s outout is %ws \n", szBuf ) ); 
        return pControl->GetUserData();
    }

	LRESULT delete_rule_from_ui()
	{
		LRESULT ret = ERROR_SUCCESS; 
		INT32 sel_index; 
		ULONG del_state; 
		LPCTSTR tmp_text; 

		sel_index = all_rule_lists[ cur_rule_type ]->GetCurSel(); 
		if( sel_index < 0 )
		{
			tmp_text = _get_string_by_id( TEXT_CONFIG_DIALOG_NO_SELECT_TIP, 
				_T( "没有选中需要编辑的条目" ) ); 

			show_msg( GetHWND(), tmp_text, NULL ); 
			goto _return; 
		}

		ret = delete_rule_action( sel_index, cur_rule_type, &del_state ); 
		
		if( ( del_state | DELETED_FROM_DB ) != 0 )
		{
			all_rule_lists[ cur_rule_type ]->RemoveItemAt( sel_index ); 
		}

		if( ret != ERROR_SUCCESS )
		{
			if( ret == ERROR_DEL_SUPER_RULE )
			{
				tmp_text = _get_string_by_id( TEXT_CONFIG_DIALOG_DONT_DELETE_SUPER_TIP, 
					_T( "默认规则为保证系统正常工作或关键安全性所必须的定义,对其修改将使系统无法正常工作或极易受到攻击" ) ); 
				show_msg( GetHWND(), tmp_text ); 
			}

			log_trace( (MSG_ERROR, "delete the action rule failed \n" ) ); 
			goto _return; 
		}
		else
		{
			tmp_text = _get_string_by_id( TEXT_CONFIG_DIALOG_DELETE_SUCCESS_TIP, 
				_T( "删除规则成功" ) ); 
			show_msg( GetHWND(), tmp_text ); 
		}

_return:
		return ret; 
	}

	LRESULT modify_rule_from_ui()
	{
		LRESULT ret = ERROR_SUCCESS; 
		INT32 sel_index; 
		access_rule_desc action_rule; 
		LPCTSTR tmp_text; 

		sel_index = all_rule_lists[ cur_rule_type ]->GetCurSel(); 

		if( sel_index < 0 )
		{
			tmp_text = _get_string_by_id( TEXT_CONFIG_DIALOG_NO_SELECT_TIP, 
				_T( "没有选中需要编辑的条目" ) ); 
			show_msg( GetHWND(), tmp_text, NULL ); 
			goto _return; 
		}

		ret = modify_rule_action( GetHWND(), sel_index, &action_rule, cur_rule_type ); 
		if( ret != ERROR_SUCCESS )
		{
			tmp_text = _get_string_by_id( TEXT_CONFIG_DIALOG_DONT_DELETE_SUPER_TIP, 
				_T( "默认规则为保证系统正常工作或关键安全性所必须的定义,对其修改将使系统无法正常工作或极易受到攻击" ) ); 

			if( ret == ERROR_DEL_SUPER_RULE )
			{
				show_msg( GetHWND(), tmp_text ); 
			}

			log_trace( ( MSG_INFO, "modify the action rule failed \n" ) ); 
			goto _return; 
		}

_return:
		return ret; 
	}

    void Notify(TNotifyUI& msg)
    {
		CStdString name; 
		access_rule_desc action_rule; 
		LRESULT ret; 

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

			name = msg.pSender->GetName(); 
			if( name == _T( "add_btn" ) )
			{
				//access_rule_desc action_rule; 

				ret = add_rule_action( GetHWND(), cur_rule_type ); 
				if( ret != ERROR_SUCCESS )
				{
					log_trace( ( MSG_ERROR, "add the action rule failed\n" ) ); 
					goto _return; 
				}

				ASSERT( all_rule_lists[ cur_rule_type ] != NULL ); 
				ASSERT( all_rules[ cur_rule_type ].size() > 0 ); 

				add_list_item( all_rule_lists[ cur_rule_type ], all_rules[ cur_rule_type ].size() - 1 ); 
			}
			else if( name == _T( "edit_btn" ) )
			{
				modify_rule_from_ui(); 
			}
			else if( name == _T( "del_btn" ) )
			{
				delete_rule_from_ui(); 
			}
			else if( name == _T( "cancel_btn" ) )
			{
				Close(); 
			}
        }
		else if( msg.sType == _T( "selectchanged" ) )
		{
			name = msg.pSender->GetName(); 

			ASSERT( tab_ctrl != NULL ); 

			__Trace( _T( "the set focus target control is %s \n" ), name ); 

			if(name==_T("tab1_btn"))
			{
				if( mode == DEFENSE_SETTING )
				{
					tab_ctrl->SelectItem( 2 ); 
					cur_rule_type = FILE_RULE_TYPE; 
				}
				else if( mode == FW_SETTING )
				{
					tab_ctrl->SelectItem( 0 ); 
					cur_rule_type = URL_RULE_TYPE; 
				}
			}
			else if( name == _T( "tab2_btn" ) )
			{
				INT32 i; 

				if( mode == DEFENSE_SETTING )
				{
					tab_ctrl->SelectItem( 3 ); 
					cur_rule_type = REG_RULE_TYPE; 

					ASSERT( all_rule_lists[ REG_RULE_TYPE ] != NULL ); 

					for( i = 0; i < all_rules[ REG_RULE_TYPE ].size(); i ++ )
					{
						add_list_item( all_rule_lists[ REG_RULE_TYPE ], ( ULONG )i ); 
					}
				}
				else if( mode == FW_SETTING )
				{
					tab_ctrl->SelectItem( 1 ); 
					cur_rule_type = SOCKET_RULE_TYPE; 

					ASSERT( all_rule_lists[ SOCKET_RULE_TYPE ] != NULL ); 

					for( i = 0; i < all_rules[ SOCKET_RULE_TYPE ].size(); i ++ )
					{
						add_list_item( all_rule_lists[ SOCKET_RULE_TYPE ], ( ULONG )i ); 
					}
				}
			}
		}
        else if(msg.sType==_T("setfocus"))
        {
        }
        else if( msg.sType == _T("itemclick") ) 
        {
        }
        else if( msg.sType == _T("itemactivate") ) 
        {
			modify_rule_from_ui(); 
        }
        else if(msg.sType == _T("menu")) 
        {
			log_trace( ( MSG_INFO, "the menu notify sender is 0x%0.8x, current selected rule list is 0x%0.8x\n", msg.pSender, all_rule_lists[ cur_rule_type ] ) ); 

            if( msg.pSender != all_rule_lists[ cur_rule_type ] ) 
			{
				return; 
			}

            setting_menu* pMenu = new setting_menu();
            if( pMenu == NULL )
			{ 
				return; 
			}

            POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
            ::ClientToScreen(*this, &pt);
            pMenu->Init(msg.pSender, pt);
        }
        else if( msg.sType == _T( "menu_del" ) ) 
		{
			delete_rule_from_ui(); 
        }
		else if( msg.sType == _T( "menu_edit" ) )
		{
			modify_rule_from_ui(); 
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
        CControlUI* pRoot = builder.Create(_T("setting_dlg.xml"), (UINT)0, &cb, &m_pm);
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

	CListUI *all_rule_lists[ MAX_ACTION_RULE_TYPE ]; 
	CTabLayoutUI *tab_ctrl; 
	access_rule_type cur_rule_type; 
	setting_mode mode; 
};