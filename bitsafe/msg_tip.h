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

#include "ui_config.h"
#include "wnd_state.h"
#include "mem_region_list.h"
#include "popup_tip_wnd.h"
#include "conf_file.h"

typedef LRESULT ( CALLBACK *popup_callback )( sys_action_output *action_event, PVOID context, PVOID context2 ); 

class action_log_notify
{
public:
	virtual LRESULT log_notify( sys_action_output *action ) = 0; 
}; 

class event_tip_dlg : public popup_tip_wnd, public wnd_state, public INotifyUI 
{
public:
	event_tip_dlg() : need_record( RECORD_NONE )
	{
		action_event = NULL; 
		log_notifier = NULL; 
		popup_return_callback = NULL; 
		need_record = RECORD_NONE; 

		ret_status = CANCEL_STATE; 
    };

	~event_tip_dlg()
	{
		if( NULL != action_event )
		{
			free( action_event ); 
		}
	}

	VOID set_popup_callback( popup_callback callback_func )
	{
		popup_return_callback = callback_func; 
	}

	VOID set_log_notifier( action_log_notify *notifier)
	{
		log_notifier = notifier; 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("TEventTip"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
		check_record_mode(); 
		if( NULL == popup_return_callback )
		{
			ASSERT( FALSE ); 
		}
		else
		{
			//if( FALSE == check_sys_action_input_valid( action_event) )
			//{
			//	ASSERT( FALSE ); 
			//}
			//else
			//{
				popup_return_callback( action_event, this, log_notifier ); 
			//}
		}

		if( NULL != action_event )
		{
			free( action_event ); 
			action_event = NULL; 
		}

		delete this; 
    };

	ACTION_RECORD_TYPE is_need_record()
	{
		return need_record; 
	}

    void Init() 
    {
  //      m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		//ASSERT( m_pCloseBtn != NULL ); 
		ret_status = CANCEL_STATE; 

		set_ctrl_text( &m_pm, _T( "record_action" ), TEXT_ACTION_RESPONSE_RECORD_ACTION ); 
		set_ctrl_text( &m_pm, _T( "record_type" ), TEXT_ACTION_RESPONSE_RECORD_ACTION_TYPE ); 
		set_ctrl_text( &m_pm, _T( "record_app" ), TEXT_ACTION_RESPONSE_TRUST_APP ); 
		set_ctrl_text( &m_pm, _T( "yes_btn" ), TEXT_COMMON_BUTTON_YES ); 
		set_ctrl_text( &m_pm, _T( "no_btn" ), TEXT_COMMON_BUTTON_NO ); 
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
   //         if( msg.pSender == m_pCloseBtn ) 
   //         {
			//	ret_status = CANCEL_STATE; 
			//	Close(); 
   //             return; 
   //         }
			//else
			{
				CStdString name = msg.pSender->GetName(); 

				if( name == _T( "yes_btn" ) )
				{
					ret_status = OK_STATE; 
					Close(); 
				}
				else if( name == _T( "no_btn" ) )
				{
					ret_status = CANCEL_STATE; 
					Close(); 
				}
			}
        }
		else if( msg.sType == _T( "selectchanged" ) )
		{
			COptionUI *record_btn; 

			COptionUI *ctrl_rec_action; 
			COptionUI *ctrl_rec_type; 
			COptionUI *ctrl_rec_app; 

			CStdString name = msg.pSender->GetName(); 

			//��������TCP�������
			//��������UDP�������
			//���������������
			//�����˲���
			//�������в���

			ctrl_rec_action = ( COptionUI* )m_pm.FindControl( _T( "record_action" ) ); 
			ctrl_rec_type = ( COptionUI* )m_pm.FindControl( _T( "record_type" ) ); 
			ctrl_rec_app = ( COptionUI* )m_pm.FindControl( _T( "record_app" ) ); 

			record_btn = ( COptionUI* )msg.pSender; 

			__Trace( _T( "this option button is selected %s \n" ), name ); 

			if( record_btn->IsSelected() )
			{
				if( name == _T( "record_action" ) )
				{
					//need_record = RECORD_APP_ACTION; 
					ctrl_rec_app->Selected( false ); 
					//ctrl_rec_action->Selected( false ); 
					ctrl_rec_type->Selected( false ); 
				}
				else if( name == _T( "record_type" ) )
				{
					//need_record = RECORD_APP_ACTION_TYPE; 
					ctrl_rec_app->Selected( false ); 
					ctrl_rec_action->Selected( false ); 
					//ctrl_rec_type->Selected( false ); 
				}
				else if( name == _T( "record_app" ) )
				{
					//need_record = RECORD_APP; 
					//ctrl_rec_app->Selected( false ); 
					ctrl_rec_action->Selected( false ); 
					ctrl_rec_type->Selected( false ); 
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
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_EX_TOOLWINDOW);
       
		styleValue = ::GetWindowLong(*this, GWL_EXSTYLE);
		::SetWindowLong( *this, GWL_STYLE, styleValue | WS_EX_TOOLWINDOW ); 

		m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("text_msg_tip.xml"), (UINT)0, NULL, &m_pm);
        ASSERT(pRoot && "Failed to parse XML");
        m_pm.AttachDialog(pRoot);
        m_pm.AddNotifier(this); 
	

		//ModifyStyle( WS_CAPTION, 0, SWP_FRAMECHANGED ); // removes title bar

		// Start creating two fonts, one underlined , other non underlined
		//
		// LOGFONT structure for font properties
		//LOGFONT lf;
		//::ZeroMemory (&lf, sizeof (lf));
		//lf.lfHeight = 100;
		//lf.lfWeight = FW_BOLD;
		//lf.lfUnderline = TRUE;

		//_tcscpy (lf.lfFaceName, _T("Arial"));

		//// Prepare for an underlined font
		//m_fontMessageUnderline = ::CreateFontIndirect( &lf );

		//// Prepare  an non undelined font
		//lf.lfUnderline = FALSE;
		//m_fontMessageNoUnderline = ::CreateFontIndirect( &lf );

		// Initialize the cursor.
		m_hCursor = ::LoadCursor(NULL, IDC_ARROW); // Use IDC_HAND if it compiles

		set_ui_style( &m_pm, NO_ALPHA ); 
		Init();
        return 0;
    }

	LRESULT check_record_mode()
	{
		LRESULT ret = ERROR_SUCCESS; 
		COptionUI *record_btn; 

		need_record = RECORD_NONE; 

		record_btn = ( COptionUI* )m_pm.FindControl( _T( "record_app" ) ); 
		ASSERT( record_btn != NULL ); 
		if( record_btn->IsSelected() )
		{
			need_record = RECORD_APP; 
			goto _return; 
		}

		record_btn = ( COptionUI* )m_pm.FindControl( _T( "record_type" ) ); 
		ASSERT( record_btn != NULL ); 
		if( record_btn->IsSelected() )
		{
			need_record = RECORD_APP_ACTION_TYPE; 
			goto _return; 
		}

		record_btn = ( COptionUI* )m_pm.FindControl( _T( "record_action" ) ); 
		ASSERT( record_btn != NULL ); 
		if( record_btn->IsSelected() )
		{
			need_record = RECORD_APP_ACTION; 
			goto _return; 
		}

_return:
		return ret; 
	}

    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE; 
		//check_record_mode(); 
		//if( NULL == popup_return_callback )
		//{
		//	ASSERT( FALSE ); 
		//}
		//else
		//{
		//	if( FALSE == check_sys_action_input_valid( action_event) )
		//	{
		//		ASSERT( FALSE ); 
		//	}
		//	else
		//	{
		//		popup_return_callback( action_event, this ); 
		//	}
		//}

        return 0;
    }

	static void set_show_time( ULONG time )
	{
#define MAX_SHOW_TIME 30000

		if( time > MAX_SHOW_TIME )
		{
			show_time = MAX_SHOW_TIME; 
		}
		else
		{
			show_time = time; 
		}
	}

	static void set_popup_speed( ULONG time )
	{
#define LOWEST_POPUP_SPEED 5

		if( time > LOWEST_POPUP_SPEED )
		{
			popup_speed = LOWEST_POPUP_SPEED; 
		}
		else
		{
			popup_speed = time; 
		}
	}

	static void set_one_popup_unit( ULONG unit )
	{
#define MAX_ONE_POPUP_UNIT 5

		if( unit > MAX_ONE_POPUP_UNIT )
		{
			one_popup_unit = MAX_ONE_POPUP_UNIT; 
		}
		else
		{
			one_popup_unit = unit; 
		}
	}


    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        return 0;
    }

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if( TRUE == ::IsIconic( *this ) ) 
		{
			bHandled = FALSE;
		}

		return ( wParam == 0 ) ? TRUE : FALSE; 
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

				//can't recognize the button on the caption to region of caption.
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
            DestroyWindow( GetHWND() ); 

			//::PostQuitMessage(0L);
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

		log_trace( ( MSG_INFO, "begin process message %u\n", uMsg ) ); 

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
			case WM_MOUSEMOVE:
				lRes = on_mouse_move(uMsg, wParam, lParam, bHandled); 
				break; 	
			case WM_TIMER:
				lRes = on_timer(uMsg, wParam, lParam, bHandled); 
				break; 
			case WM_SETCURSOR:
				lRes = on_set_cursor(  uMsg, wParam, lParam, bHandled ); 
				break; 
			case WM_MOUSELEAVE:
				lRes = on_mouse_leave(  uMsg, wParam, lParam, bHandled ); 
				break; 
			case WM_MOUSEHOVER:
				lRes = on_mouse_hover(  uMsg, wParam, lParam, bHandled ); 
				break; 
			default:
				bHandled = FALSE;
		}
		if( bHandled ) return lRes;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;

		log_trace( ( MSG_INFO, "end process message %u\n", uMsg ) ); 

		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	LRESULT update_window_elapsed_time()
	{
		log_trace( ( MSG_INFO, "updating elapsed time %u\n", time_elapsed ) ); 
	
		return 0; 
	}

	static event_tip_dlg* create_popup(  UINT32 msg_show_time,         
		UINT32 msg_popup_speed, 
		UINT32 popup_unit, 
		HWND parent)
	{
		event_tip_dlg* tip_wnd = new event_tip_dlg(); 

		if( NULL == tip_wnd )
		{
			goto _return; 
		}

		tip_wnd->m_parent = parent;

		tip_wnd->msg_show_time = msg_show_time;
		tip_wnd->creation_delay = msg_popup_speed; 
		tip_wnd->popup_unit = popup_unit; 
		//tip_wnd->time_elapsed = 0; 

		tip_wnd->m_hCursor = NULL;

_return:
		return tip_wnd; 
	}

	LRESULT set_action_event( sys_action_output *action_event_input )
	{
		LRESULT ret = ERROR_SUCCESS; 

		ret = is_valid_action_output( action_event_input ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		if( NULL == action_event )
		{
			action_event = ( sys_action_output* )malloc( action_event_input->size/*sizeof( sys_action_output )*/ ); 
			if( action_event == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				goto _return; 
			}
		}
	
		memcpy( action_event, action_event_input, action_event_input->size/*sizeof( sys_action_output )*/ ); 

_return:
		return ret; 
	}

	LRESULT _set_action_event( sys_action_desc *action_event_input, PVOID data, ULONG data_len )
	{
		LRESULT ret = ERROR_SUCCESS; 
		ULONG copy_len; 

		if( is_valid_action_type( action_event_input->type ) != TRUE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		if( NULL == action_event )
		{
			action_event = ( sys_action_output* )malloc( sizeof( sys_action_output ) + DEFAULT_OUTPUT_DATA_REGION_SIZE ); 
			if( action_event == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				goto _return; 
			}
		}

		if( data_len > DEFAULT_OUTPUT_DATA_REGION_SIZE )
		{
			copy_len = DEFAULT_OUTPUT_DATA_REGION_SIZE; 
		}
		else
		{
			copy_len = data_len; 
		}

		action_event->magic = SYS_ACTION_OUTPUT_MAGIC; 
		action_event->size = copy_len + FIELD_OFFSET( sys_action_output, data ); 

		memcpy( &action_event->action, action_event_input, sizeof( sys_action_desc ) ); 
		memcpy( action_event->data, data, data_len ); 

_return:
		return ret; 
	}

public:
    CPaintManagerUI m_pm; 
	sys_action_output *action_event; 
	action_log_notify *log_notifier; 
	popup_callback popup_return_callback; 

	static ULONG popup_speed; 
	static ULONG show_time; 
	static ULONG one_popup_unit; 

private: 
	ACTION_RECORD_TYPE need_record; 
};

//typedef enum _ACTION_RECORD_TYPE
//{
//	RECORD_NONE, 
//	RECORD_APP_ACTION, 
//	RECORD_APP_ACTION_TYPE, 
//	RECORD_APP 
//} ACTION_RECORD_TYPE, *PACTION_RECORD_TYPE; 


#define R3_SYS_EVENT_RESPONSE 0x00000001 

LRESULT CALLBACK on_popup_closed_r3( sys_action_output *action_event, PVOID context ); 

LRESULT CALLBACK on_popup_closed( sys_action_output *action_event, PVOID context ); 

LRESULT show_event_tip( HWND parent, 
					   LPCTSTR msg, 
					   LPCTSTR data_dump, 
					   sys_action_output* action_event, 
					   LPCTSTR title = NULL, 
					   ULONG mode = 0, 
					   action_log_notify *log_notifier = NULL, 
					   ULONG flags = 0 ); 

LRESULT _show_event_tip( HWND parent, 
						LPCTSTR msg, 
						LPCTSTR data_dump, 
						sys_action_desc* sys_action, 
						PVOID data, 
						ULONG data_len, 
						LPCTSTR title, 
						ULONG mode, 
						action_log_notify *log_notifier, 
						ULONG flags ); 

LRESULT __show_event_tip( HWND parent, 
						 LPCTSTR msg, 
						 LPCTSTR data_dump, 
						 sys_action_info* action_event, 
						 LPCTSTR title, 
						 ULONG mode, 
						 action_log_notify *log_notifier, 
						 ULONG flags ); 

//{
//	LRESULT ret = ERROR_SUCCESS; 
//	event_tip_dlg *msg_tip; 
//	CControlUI *msg_txt; 
//	HWND wnd; 
//	INT32 _ret; 
//
//	//BOOL _ret = SetForegroundWindow( parent ); 
//	//if( _ret == FALSE )
//	//{
//	//	ASSERT( FALSE ); 
//	//	log_trace( ( DBG_MSG_AND_ERROR_OUT, "set main window to foreground failed\n") ); 
//	//}
//
//	msg_tip = event_tip_dlg::create_popup( 50, 100, parent ); 
//	if( msg_tip == NULL )
//	{
//		ret = ERROR_NOT_ENOUGH_MEMORY; 
//		goto _return; 
//	}
//
//	msg_tip->set_action_event( action_event ); 
//	msg_tip->set_popup_callback( on_popup_closed ); 
//
//	//if( NULL == msg_tip.Create( parent, title, UI_WNDSTYLE_DIALOG | WS_EX_TOPMOST, 0L, 0, 0, 1024, 738) )
//	//{
//	//	ret = ERROR_ERRORS_ENCOUNTERED; 
//	//	goto _return; 
//	//}
//
//	wnd = msg_tip->Create( NULL, title, WS_VISIBLE | WS_OVERLAPPEDWINDOW, MAX_INT32_VAL, MAX_INT32_VAL, 0, 0, 0 )
//	if( NULL == wnd )
//	{
//		ret = GetLastError(); 
//		goto _return; 
//	}
//
//	if( title == NULL )
//	{
//		title = _T( "" ); 
//	}
//
//	msg_tip->SetIcon( IDI_MAIN_ICON ); 
//	msg_txt = msg_tip->m_pm.FindControl( _T( "msg_txt" ) ); 
//
//	ASSERT( msg_txt != NULL ); 
//	msg_txt->SetText( msg ); 
//
//	if( title != NULL )
//	{
//		msg_txt = msg_tip->m_pm.FindControl( _T( "title_txt" ) ); 
//
//		ASSERT( msg_txt != NULL ); 
//		msg_txt->SetText( title ); 
//	}
//
//	ret = msg_tip->popup_msg(); 
//
//	_ret = ( INT32 )::SetTimer( wndmsg_tip->GetHWND(), IDT_POP_WINDOW_TIMER, msg_tip->creation_delay, NULL );
//	if( _ret == 0 )
//	{
//		ret = GetLastError(); 
//		goto _return; 
//	}
//
//	//msg_tip.CenterWindow();
//	//msg_tip.ShowModal(); 
//
//_return: 
//	return ret; 
//}

LRESULT init_popup_wnd_context(); 
LRESULT uninit_popup_wnd_context(); 