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

#include <objbase.h>
#include <zmouse.h>
#include <exdisp.h>
#include <vector>
#include <sstream>
//#include "menu_ui.h"
#include "seven_fw_api.h"
#include "config_loader.h"

typedef struct _url_rule_items
{
	std::vector<record_string> url_rules;
} url_rule_items, *purl_rule_items; 

#define MAX_NAMES_LEN 2048

INT32 http_url_flt_add_filter_text( LPCTSTR text, ULONG text_type, ULONG flags, PVOID param ); 

#define PARA_TEXT_DESC "�����ı�"
#define LABEL_TEXT_DESC "��ǩ�ı�"
#define TABLE_TEXT_DESC "�����ı�"

class url_rule_dlg : public CWindowWnd, public INotifyUI, public IListCallbackUI
{
public:
	url_rule_dlg() : rules_count( 0 )
	{
    };

	ULONG text_type_from_desc( CHAR *desc )
	{
		ASSERT( NULL != desc ); 

		if( strcmp( desc, PARA_TEXT_DESC ) == 0 )
		{
			return PARA_TAG_INDEX; 
		}
		else if( strcmp( desc, LABEL_TEXT_DESC ) == 0 )
		{
			return A_TAG_INDEX; 
		}
		else if( strcmp( desc, TABLE_TEXT_DESC ) == 0 )
		{
			return TD_TAG_INDEX; 
		}
		else
		{
			return -1; 
		}
	}

	INT32 save_all_filter_text()
	{
		INT32 ret = 0;
		ULONG type; 
		ULONG item_count; 
		INT32 i = 0; 
		//WCHAR unicode[ 2048 ]; 

		ret = reset_filter_text_file( FILTER_TEXT_BACKUP_FILE ); 
		if( ret < 0 )
		{
			goto _return; 
		}

		item_count = url_rule.url_rules.size(); 

		for( i = 0; i < item_count; i ++ )
		{
			//mcbs_to_unicode( url_rule.url_rules[ i ].c_str(), unicode, 2048, NULL ); 
			ret = save_filter_to_file( url_rule.url_rules[ i ].c_str(), type, FILTER_TEXT_BACKUP_FILE  ); 
			if( 0 > ret )
			{
				goto _return; 
			}
		}

_return:
		return ret; 
	}

	CHAR *get_text_type_desc( ULONG type )
	{
		CHAR *type_desc = ""; 

		switch( type )
		{
		case PARA_TAG_INDEX:
			type_desc = PARA_TEXT_DESC; 
			break; 
		case A_TAG_INDEX:
			type_desc = LABEL_TEXT_DESC; 
			break; 
		case TD_TAG_INDEX:
			type_desc = TABLE_TEXT_DESC; 
			break; 
		default:
			break; 
		}

		return type_desc; 
	}

	INT32 add_flt_info_to_list( DWORD item_index, 
		ULONG type, 
		LPCTSTR filt_rule )
	{
		INT32 ret = 0;
		CHAR *type_desc; 
		ULONG item_count; 

		//::MessageBox( NULL, "enter add_flt_info_to_list", NULL, 0 ); 

		if( type > MAX_TAG_INDEX )
		{
			//( "enter add_flt_info_to_list type invalid \n" ); 
			ret = -1; 
			goto _return;
		}

		type_desc = get_text_type_desc( type ); 

		item_count = url_rule.url_rules.size();

		if( item_index >= item_count )
		{
			url_rule.url_rules.push_back( filt_rule );
		}
		else
		{
			url_rule.url_rules[ item_index ] = filt_rule;
		}

		if( item_index >= item_count )
		{
			CListTextElementUI* new_element = new CListTextElementUI;
			if( new_element == NULL )
			{
				ret = -1; 
				goto _return; 
			}

			if( new_element != NULL )
			{
				new_element->SetTag( item_index );
			}
			else
			{
				ret = -ENOMEM; 
			}
		}

_return:
		return ret;
	}

	INT32 on_uninit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 ret; 
		save_all_filter_text(); 

		ret = do_ui_ctrl( BITSAFE_UI_ID, HTTP_URL_FLT_UNINIT, NULL, 0, NULL, 0, NULL, this ); 

		return ret;
	}

	INT32 CleanRemainListItem( HWND hList, DWORD dwItemIndex )
	{
		url_rule.url_rules.clear(); 
		return TRUE;
	}

	LRESULT on_del_http_flt_url()
	{
		INT32 ret = 0;
		CListUI *list; 
		PFILTER_URL_INPUT filt_rule = NULL; 
		DWORD ret_len; 
		INT32 sel_item; 

		list = static_cast<CListUI*>( m_pm.FindControl( _T( "url_list" ) ) ); 
		ASSERT( list != NULL ); 

		sel_item = list->GetCurSel(); 

		if( sel_item < 0 )
		{
			ret = -1; 
			goto _return; 
		}

		filt_rule = ( PFILTER_URL_INPUT )malloc( sizeof( FILTER_URL_INPUT ) + url_rule.url_rules[ sel_item ].length() + sizeof( CHAR ) ); 
		if( NULL == filt_rule )
		{
			ret = -ENOMEM; 
			goto _return; 
		}

		filt_rule->length = url_rule.url_rules[ sel_item ].length() + sizeof( CHAR ); 
		memcpy( filt_rule->url, url_rule.url_rules[ sel_item ].c_str(), filt_rule->length ); 

		//filt_rule->type = text_type_from_desc( text ); 
		//if( filt_rule->type < 0 )
		//{
		//	ret = FALSE; 
		//	goto _return; 
		//}

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			HTTP_URL_FLT_DEL_URL, 
			( PBYTE )filt_rule, 
			sizeof( FILTER_URL_INPUT ) + url_rule.url_rules[ sel_item ].length()+ sizeof( CHAR ), 
			NULL, 
			0, 
			NULL, 
			this ); 

		dbg_print( MSG_WARNING | DBG_DBG_BUF_OUT, "ɾ�� %s %s \n", 
			filt_rule->url, 
			ret == 0 ? "�ɹ�" : "ʧ��" ); 

		if( ret == 0 )
		{
			list->RemoveSubItemAt( sel_item );
			remove_url_rule( sel_item ); 
		}

_return:
		if( NULL != filt_rule )
		{
			free( filt_rule ); 
		}
		return ret; 
	}

	LRESULT ui_init()
	{
		LRESULT ret = 0;

		ret = load_filter_text_from_file( http_url_flt_add_filter_text, this, FILTER_URL_BACKUP_FILE );
		if( ret < 0 )
		{
			if( ret == FORMAT_ERROR )
			{
				::MessageBox( NULL, _T( "�����ļ���ʽ����ȷ!" ), NULL, MB_OK ); 
				goto _return; 
			}

			ret = 0; 
		}

		//{
		//	WORD Version = 0x0202; 
		//	WSADATA WsaData;
		//	ret = WSAStartup( Version, &WsaData );
		//}
_return:
		return ret;
	}

	INT32 mng_flt_txt()
	{
		INT32 ret = 0; 
		USHORT text_type; 
		CEditUI *edit; 

		text_type = 0; 

		edit = static_cast< CEditUI* >( m_pm.FindControl( _T( "url_input_edit" ) ) ); 
		ASSERT( edit != NULL ); 

		ret = http_url_flt_add_filter_text( edit->GetText().GetData(), text_type, TRUE, this ); 

		edit->SetText( _T("") ); 
		return ret; 
	}

	INT32 output_seted_filter_texts( PFILTER_NAMES_OUTPUT flt_names, ULONG length )
	{
		INT32 ret; 
		ULONG ret_length = length; 
		PFILTER_NAMES_OUTPUT filter_text; 
		PFILTER_NAME_INPUT filter_text_get; 
		//HWND hList; 
		PUI_CTRL ui_ctrl; 
#ifdef _UNICODE
		LPTSTR text_out = NULL; 
		ULONG buf_len; 
		ULONG need_len; 

		ret = init_op_temp_buf( ( PBYTE* )&text_out, 2048, &buf_len ); 
		if( ret< 0 )
		{
			goto _return; 
		}

#endif //_UNICODE

		//hList = GetDlgItem( IDC_LIST_FILTER_TEXT ); 
		//ASSERT( hList != NULL ); 

		//MessageBox( NULL, "get the seted text \n", NULL, 0 ); 

		//	CHAR output[ 512 ]; 
		//	sprintf( output, "get the seted text length is %d \n", ret_length ); 

		//	MessageBox( NULL, output, NULL, 0 ); 
		//}

		//DBG_BP(); 

		filter_text = flt_names; 
		filter_text_get = &filter_text->name_output; 

		for( ; ; )
		{
			if( filter_text_get->length == 0 )
			{
				//ASSERT( *( filter_text_get->text ) == '\0' );

				ret = 0; 
				break; 
			}

			if( filter_text_get->length + sizeof( FILTER_NAME_INPUT ) > ret_length )
			{
				ASSERT( FALSE ); 
				ret = -1; 
				break; 
			}

			if( filter_text_get->text[ filter_text_get->length - 1 ] != _T( '\0' ) )
			{
				filter_text_get->text[ filter_text_get->length - 1 ] = _T( '\0' ); 
			}

#ifdef _UNICODE
			{
				ret = mcbs_to_unicode( ( CHAR* )filter_text_get->text, NULL, 0, &need_len ); 
				if( ret < 0 )
				{
					if( ret != -ENOMEM )
					{
						goto _return; 
					}
				}

				ret = realloc_if_need( ( PBYTE* )&text_out, 0, &buf_len, need_len ); 
				if( ret < 0 )
				{
					goto _return; 
				}

				ret = mcbs_to_unicode( ( CHAR* )filter_text_get->text, text_out, need_len, NULL ); 
				if( ret < 0 )
				{
					goto _return; 
				}

				ret = add_flt_info_to_list( rules_count++, 0, text_out ); 
				ret = save_filter_to_file( text_out, 0, FILTER_TEXT_BACKUP_FILE ); 
			}
#else
			ret = add_flt_info_to_list( rules_count++, 0, filter_text_get->text ); 
			ret = save_filter_to_file( filter_text_get->text, 0, FILTER_TEXT_BACKUP_FILE ); 
#endif 
			if( 0 > ret )
			{
				ret = -1; 
			}
			else
			{
				ret = 0; 
			}

			if( filter_text_get->length + sizeof( FILTER_NAME_INPUT ) ==  ret_length )
			{
				//MessageBox( NULL, "OK", NULL, 0 ); 

				ret = 0; 
				break; 
			}

			filter_text_get = ( PFILTER_NAME_INPUT )( ( PBYTE )filter_text_get + filter_text_get->length + sizeof( FILTER_NAME_INPUT ) ); 
		}

_return:
#ifdef _UNICODE
		if( text_out != NULL )
		{
			free( text_out ); 
		}
#endif 
		return ret; 
	}

	LRESULT on_init_dlg()
	{
		INT32 ret = 0; 

		ret = ui_init(); 
		if( ret < 0 )
		{
			goto _return; 
		}

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			HTTP_URL_FLT_INIT, 
			NULL, 
			0, 
			NULL, 
			0, 
			NULL, 
			this ); 

		if( ret < 0 )
		{se
			goto _return; 
		}

		//DBG_BP(); 

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			HTTP_URL_FLT_GET_ADDED_FLT_URL, 
			NULL, 
			0, 
			NULL, 
			0, 
			NULL, 
			this ); 

		if( ret < 0 )
		{
			goto _return; 
		}

_return:
		if( ret < 0 )
		{
			Close(); 
		}

		return ret;  // Let the system set the focus
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("HttpUrlRuleDlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

	VOID remove_url_rule( INT32 index )
	{
		if( index >= url_rule.url_rules.size() )
		{
			ASSERT( FALSE && "the url rule index to remove is invalid" ); 
			goto _return; 
		}

		url_rule.url_rules.erase( url_rule.url_rules.begin() + index ); 

_return:
		return; 
	}

    void Init() 
    {
		CListUI *list; 

        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
        m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
        ASSERT( m_pMaxBtn != NULL ); 
		m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
        ASSERT( m_pRestoreBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		list = static_cast< CListUI* >( m_pm.FindControl( _T( "url_rule_list" ) ) ); 

		ASSERT( list != NULL ); 
		list->SetTextCallback(this);

		on_init_dlg(); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }
/
    LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
    {
        TCHAR szBuf[MAX_PATH] = {0};
        switch (iSubItem)
        {
        case 0:
			{
				_stprintf( szBuf, url_rule.url_rules[ iIndex ].c_str() ); 
			}
            break;
        }
        pControl->SetUserData( szBuf );
        return pControl->GetUserData();
    }

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("windowinit") ) 
            OnPrepare(msg);
        else if( msg.sType == _T("click") ) 
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
            //    SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
            //}
            else if( msg.pSender == m_pRestoreBtn ) 
            { 
                SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
            }
			else if( msg.pSender->GetName() == _T( "url_input_btn" ) )
			{
				mng_flt_txt(); 
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
        }
   //     else if(msg.sType == _T("menu")) 
   //     {
   //         if( msg.pSender->GetName() != _T("domainlist") ) return;
   //         menu_ui* pMenu = new menu_ui();
   //         if( pMenu == NULL ) { return; }
   //         POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
   //         ::ClientToScreen(*this, &pt);
   //         pMenu->Init(msg.pSender, pt);
   //     }
   //     else if( msg.sType == _T("menu_Delete") ) {
   //         CListUI* pList = static_cast<CListUI*>(msg.pSender);
   //         int nSel = pList->GetCurSel();
   //         if( nSel < 0 ) return;
   //         pList->RemoveAt(nSel);

			//remove_url_rule( nSel ); 
   //     }
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("url_rule_dlg.xml"), (UINT)0, NULL, &m_pm);
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

public:
	ULONG rules_count; 

protected:
	url_rule_items url_rule; 

private:
    CButtonUI* m_pCloseBtn;
    CButtonUI* m_pMaxBtn;
    CButtonUI* m_pRestoreBtn;
    CButtonUI* m_pMinBtn;
    //...
};