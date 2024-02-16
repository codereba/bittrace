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

//#include "menu_ui.h"
#include "sys_mng_api.h"
#include "trace_log_api.h"

#define WM_SYS_LOG_OUTPUT ( WM_USER + 1001 ) 
#define OUTPUT_REG_MSG 0x00000001
#define OUTPUT_FILE_MSG 0x00000002
#define OUTPUT_NETWORK_MSG 0x00000004
#define OUTPUT_DRIVER_LOAD_MSG 0x00000008
#define OUTPUT_ALL 0xffffffff
INLINE INT32 InsertTailText( HWND hEdit, CHAR *Text )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	CHARRANGE cr;
	ASSERT( NULL != hEdit );
	ASSERT( NULL != Text );

	cr.cpMin = -1;
	cr.cpMax = -1;
	SendMessageA( hEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessageA( hEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)Text);
	SendMessageA( hEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessageA( hEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)"\r\n" );

	_ret = PostMessageA( hEdit, WM_VSCROLL, SB_BOTTOM,0 ); 
	if( FALSE == _ret )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	return ret; 
}

class log_dlg : public CWindowWnd, public INotifyUI
{
public:
	log_dlg() 
	{
		msg_flt_flags = OUTPUT_ALL; 
	}

	LRESULT output_log( LPCTSTR log_msg, sys_action_type type )
	{
		LRESULT ret = ERROR_SUCCESS; 

		ASSERT( log_out != NULL ); 

		ASSERT( NULL != log_msg ); 
		ASSERT( is_valid_action_type( type ) ); 

		switch( type )
		{
		case ICMP_SEND: 
		case ICMP_RECV: 
		case LOCATE_URL:
		case SOCKET_CONNECT:
		case SOCKET_SEND:
		case SOCKET_RECV:
			if( 0 == ( msg_flt_flags & OUTPUT_NETWORK_MSG ) )
			{
				goto _return; 
			}
			break; 

		case MODIFY_FILE:
		case DELETE_FILE: 
			if( 0 == ( msg_flt_flags & OUTPUT_FILE_MSG ) )
			{
				goto _return; 
			}
			break; 

		case MODIFY_KEY:
			if( 0 == ( msg_flt_flags & OUTPUT_REG_MSG ) )
			{
				goto _return; 
			}
			break; 

		case OTHER_ACTION:
		case INSTALL_HOOK:
		case ACCESS_MEM:
		case CREATE_PROC:
		case TERMINATE_PROC:
		case INSTALL_DRV:
		case ACCESS_COM:
			if( 0 == ( msg_flt_flags & OUTPUT_DRIVER_LOAD_MSG ) )
			{
				goto _return; 
			}
			break; 
		default:
			break; 
		}

#ifndef _UNICODE; 
		{
			LPWSTR unicode; 
			ULONG need_len; 

			unicode = ( LPWSTR )malloc( MAX_MSG_LEN * sizeof( WCHAR ) ); 
			if( unicode == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				goto _return; 
			}

			ret = mcbs_to_unicode( ( LPSTR )log_msg , unicode, MAX_MSG_LEN, &need_len ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return; 
			}

			log_out->AppendText( unicode ); 
		}
#else
		log_out->AppendText( log_msg ); 
		log_out->AppendText( _T("\r\n" ) ); 
#endif
		
_return:
		return ret; 
	}

//	BOOL SetFiltering( HWND hMainDlg )
//	{
//		HWND CheckBox;
//		INT32 ret;
//		INT32 CheckRet; 
//		DWORD ret_length; 
//		ULONG Index;
//
//		ULONG RegMngFilterIndex[] = {
//			NtCreateKeyFilter, 
//				NtQueryValueKeyFilter, 
//				NtDeleteKeyFilter, 
//				NtDeleteValueKeyFilter, 
//				NtSetValueKeyFilter, 
//				NtRestoreKeyFilter, 
//				NtOpenKeyFilter
//		};
//
//		ULONG ProcessMngFilterIndex[] = {
//			NtCreateProcessFilter, 
//				NtCreateProcessExFilter, 
//				NtCreateUserProcessFilter, 
//				NtCreateThreadOrNtCreateThreadExFilter, 
//				NtOpenThreadFilter, 
//				NtTerminateProcessFilter, 
//		};
//
//		ULONG FileMngFilterIndex[] = {
//			NtDeleteFileFilter, 
//				NtOpenFileFilter, 
//				NtCreateFileFilter
//		};
//
//		ULONG DrvMngFilterIndex[] = {
//			NtLoadDriverFilter 
//		};
//
//		FILTER_INDEX_TABLE AllFilterIndexes[] = {
//			{ DrvMngFilterIndex, ARRAY_SIZE( DrvMngFilterIndex ) }, 
//			{ FileMngFilterIndex, ARRAY_SIZE( FileMngFilterIndex ) }, 
//			{ RegMngFilterIndex, ARRAY_SIZE( RegMngFilterIndex ) }, 
//			{ ProcessMngFilterIndex, ARRAY_SIZE( ProcessMngFilterIndex ) } 
//		};
//
//		ASSERT( NULL != hMainDlg );
//
//		if( NULL == sys_act_info )
//		{
//			return FALSE;
//		}
//
//#define FILTER_CHECK_BOX_ID_BEGIN 0
//#define FILTER_CHECK_BOX_ID_END 3
//
//		ASSERT( FILTER_CHECK_BOX_ID_END - FILTER_CHECK_BOX_ID_BEGIN <= NtApiFilterEnd );
//
//		sys_act_info->Size = 0;
//		for( Index = FILTER_CHECK_BOX_ID_BEGIN; Index <= FILTER_CHECK_BOX_ID_END; Index ++ )
//		{
//			//CheckBox = GetDlgItem( Index );
//
//			//ASSERT( NULL != CheckBox );
//			//CheckRet = ( INT32 )SendMessage( CheckBox, BM_GETCHECK, 0, 0 );
//
//			CheckRet = BST_CHECKED; 
//
//			if( CheckRet == BST_CHECKED )
//			{
//				INT32 i; 
//				CHAR Output[ 1024 ];
//				for( i = 0; i < AllFilterIndexes[ Index - FILTER_CHECK_BOX_ID_BEGIN ].IndexNumber; i ++ )
//				{
//					sprintf( Output, "Add filter index: table %d, index %d, value %d \n", Index - FILTER_CHECK_BOX_ID_BEGIN, 
//						i, 
//						AllFilterIndexes[ Index - FILTER_CHECK_BOX_ID_BEGIN ].FilterIndexes[ i ] ); 
//					OutputDebugStringA( Output ); 
//
//					sys_act_info->FilterIndexes[ sys_act_info->Size ] = AllFilterIndexes[ Index - FILTER_CHECK_BOX_ID_BEGIN ].FilterIndexes[ i ];
//					sys_act_info->Size += 1;
//				}
//			}
//		}
//
//		if( sys_act_info->Size == 0 )
//		{
//			return TRUE;
//		}
//
//		ret = do_ui_ctrl( BITSAFE_UI_ID, SYS_MON_START, ( PBYTE )sys_act_info, /*sizeof( HOOK_FILTER_INFO ) + sizeof( ULONG ) * sys_act_info->Size */MAX_HOOK_INFO_SIZE, NULL, 0, NULL, this ); 
//		//ASSERT( TRUE == ret ); 
//		return ret;
//	}

	LRESULT set_log_filter()
	{
		LRESULT ret = ERROR_SUCCESS; 
		COptionUI *option_btn; 

		msg_flt_flags = 0; 

		option_btn = ( COptionUI* )m_pm.FindControl( _T( "common_msg_btn" ) ); 
		ASSERT( option_btn != NULL ); 
		if( option_btn->IsSelected() )
		{
			msg_flt_flags |= OUTPUT_DRIVER_LOAD_MSG; 
		}

		option_btn = ( COptionUI* )m_pm.FindControl( _T( "file_msg_btn" ) ); 
		ASSERT( option_btn != NULL ); 
		if( option_btn->IsSelected() )
		{
			msg_flt_flags |= OUTPUT_FILE_MSG; 
		}

		option_btn = ( COptionUI* )m_pm.FindControl( _T( "reg_msg_btn" ) ); 
		ASSERT( option_btn != NULL ); 
		if( option_btn->IsSelected() )
		{
			msg_flt_flags |= OUTPUT_REG_MSG; 
		}

		option_btn = ( COptionUI* )m_pm.FindControl( _T( "net_msg_btn" ) ); 
		ASSERT( option_btn != NULL ); 
		if( option_btn->IsSelected() )
		{
			msg_flt_flags |= OUTPUT_NETWORK_MSG; 
		}

		return ret; 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("LogDlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    }

	VOID set_lang_text()
	{
		set_ctrl_text( &m_pm, _T( "common_msg_btn" ), TEXT_RUN_TIME_LOG_COMMON_MSG, _T( "һ��ϵͳ��Ϣ" ) ); 
		set_ctrl_text( &m_pm, _T( "file_msg_btn" ), TEXT_RUN_TIME_LOG_FILE_MSG, _T( "�ļ���Ϊ��Ϣ" ) ); 
		set_ctrl_text( &m_pm, _T( "reg_msg_btn" ), TEXT_RUN_TIME_LOG_REG_MSG, _T( "ע�����Ϊ��Ϣ" ) ); 
		set_ctrl_text( &m_pm, _T( "net_msg_btn" ), TEXT_RUN_TIME_LOG_NETWORK_MSG, _T( "����ͨ����Ϣ" ) ); 
		set_ctrl_text( &m_pm, _T( "ok_btn" ), TEXT_COMMON_BUTTON_CONFIRM, _T( "����" ) );
	}

    void Init() 
    {
		COptionUI *option; 

		m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
		m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
		ASSERT( m_pMaxBtn != NULL ); 
		m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
		ASSERT( m_pRestoreBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		log_out = ( CRichEditUI* )m_pm.FindControl( _T( "log_edit" ) ); 
		ASSERT( log_out != NULL ); 

		option = ( COptionUI* )m_pm.FindControl( _T( "common_msg_btn" ) ); 
		ASSERT( option != NULL ); 
		option->Selected( true ); 

		option = ( COptionUI* )m_pm.FindControl( _T( "file_msg_btn" ) ); 
		ASSERT( option != NULL ); 
		option->Selected( true ); 

		option = ( COptionUI* )m_pm.FindControl( _T( "reg_msg_btn" ) ); 
		ASSERT( option != NULL ); 
		option->Selected( true ); 

		option = ( COptionUI* )m_pm.FindControl( _T( "net_msg_btn" ) ); 
		ASSERT( option != NULL ); 
		option->Selected( true ); 

		set_lang_text(); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("windowinit") ) 
            OnPrepare(msg);
        else if( msg.sType == _T("click") ) 
        {
            if( msg.pSender == m_pCloseBtn ) 
            {
				SetVisible( false ); 
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
			else
			{
				CStdString name; 

				name = msg.pSender->GetName(); 

				if( name == _T("ok_btn") )
				{
					set_log_filter(); 
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
        }
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);

        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("log_dlg.xml"), (UINT)0, NULL, &m_pm);
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
		m_pm.RemoveNotifier( this ); 

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

	
	LRESULT on_output_sys_msg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = 0; 
		
		if( FALSE == is_valid_action_type( ( sys_action_type )wParam ) )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		if( 0 == lParam )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		ret = output_log( ( LPCTSTR )lParam, ( sys_action_type )wParam ); 

_return:
		return ret; 
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
			case WM_SYS_LOG_OUTPUT:lRes = on_output_sys_msg( uMsg, wParam, lParam, bHandled ); break;
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
    CButtonUI* m_pMaxBtn;
    CButtonUI* m_pRestoreBtn;
    CButtonUI* m_pMinBtn;
	CRichEditUI *log_out; 
	ULONG msg_flt_flags; 
};