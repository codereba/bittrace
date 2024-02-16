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

#ifndef __FILTER_FRAME_H__
#define __FILTER_FRAME_H__

#include "ui_ctrl.h"
#include "filter_cond_db.h"
#include "action_display.h"
#include "filter_manage.h"
#include "trace_filter.h"
#include <vector>

#define DEFAULT_HILIGHT_COLOR RGB( 30, 10, 30 )

#ifdef LANG_EN
#define FILTER_CONDITION_UNKNOWN L"Unknown" //L"δ֪"
#define FILTER_CONDITION_IMAGE_PATH L"Image path"
#define FILTER_CONDITION_PROCESS_NAME L"Process name"
#define FILTER_CONDITION_COMMAND_LINE L"Command line"
#define FILTER_CONDITION_PROCESS_ID L"Process ID"
#define FILTER_CONDITION_PARENT_PROCESS_ID L"Parent process ID"
#define FILTER_CONDITION_THREAD_ID L"Thread ID"
#define FILTER_CONDITION_MAIN_TYPE L"Main Type"
#define FILTER_CONDITION_EVENT_TYPE L"Event Type"
#define FILTER_CONDITION_IO_MODE L"Read/Write"
#define FILTER_CONDITION_OBJECT L"Object"
#define FILTER_CONDITION_DETAIL L"Detail"
#define FILTER_CONDITION_DATA L"Data" //L"�¼�����"
#define FILTER_CONDITION_RESULT L"Result" //L"���"
#define FILTER_CONDITION_CALL_STACK L"Call stack" //L"���ö�ջ"
#define FILTER_CONDITION_DURATION L"Duration" //L"ʱ������"
#define FILTER_CONDITION_TIME L"Time" //L"���� ʱ��"
#define FILTER_CONDITION_RELATIVE_TIME L"Relative time" //L"���ʱ��"
#define FILTER_CONDITION_SESSION L"Session" //L"�Ự"
#define FILTER_CONDITION_USER_NAME L"User name" //L"�û�"
#define FILTER_CONDITION_USER_ID L"User identifier" //L"����ID��"
#define FILTER_CONDITION_EVENT_ID L"Event ID" //L"���к�"
#define FILTER_CONDITION_CORPORATION L"Corporation" //L"��˾"
#define FILTER_CONDITION_VERSION L"Version" //L"�汾"
#define FILTER_CONDITION_VIRTUAL L"Virtual" //L"���⻯"
#define FILTER_CONDITION_CPU_ARCH L"CPU arch" //L"CPU�ܹ�"

#define FILTER_COMPARE_MODE_CONTAIN L"Contain" //L"����"
#define FILTER_COMPARE_MODE_NOT_CONTAIN L"Not Contain" //L"������"
#define FILTER_COMPARE_MODE_EQUAL L"Equal" //L"����" 
#define FILTER_COMPARE_MODE_NOT_EUQAL L"Not equal" //L"������" 
#define FILTER_COMPARE_MODE_LOWER L"Lower" //L"С��" 
#define FILTER_COMPARE_MODE_GREATER L"Greater" //L"����"
#define FILTER_COMPARE_MODE_START L"Start" //L"��ʼ"
#define FILTER_COMPARE_MODE_END L"End" //L"����"
#define FILTER_COMPARE_MODE_UNKNOWN L"Unknown" //L"δ֪"

#define FILTER_MODE_INCLUDE_TEXT L"Inlude" //L"����"
#define FILTER_MODE_EXCLUDE_TEXT L"Exclude" //L"�ų�"
#define FILTER_MODE_UNKNOWN_TEXT L"Unknown" //L"δ֪"

#define PLEASE_SET_FILTER_CONDITION_TIP L"Filter condition is required." //_T( "�����ù�������ֵ" )
#define INPUT_FILTER_CONDITION_INVALID L"Input error" //_T( "���벻��ȷ" ) 
#else
#define FILTER_CONDITION_UNKNOWN L"δ֪"
#define FILTER_CONDITION_IMAGE_PATH L"����·��"
#define FILTER_CONDITION_PROCESS_NAME L"������"
#define FILTER_CONDITION_COMMAND_LINE L"������"
#define FILTER_CONDITION_PROCESS_ID L"����ID"
#define FILTER_CONDITION_PARENT_PROCESS_ID L"������ID"
#define FILTER_CONDITION_THREAD_ID L"�߳�ID"
#define FILTER_CONDITION_MAIN_TYPE L"������"
#define FILTER_CONDITION_EVENT_TYPE L"�¼�����"
#define FILTER_CONDITION_IO_MODE L"��/д"
#define FILTER_CONDITION_OBJECT L"����"
#define FILTER_CONDITION_DETAIL L"ϸ��"
#define FILTER_CONDITION_DATA L"�¼�����"
#define FILTER_CONDITION_RESULT L"���"
#define FILTER_CONDITION_CALL_STACK L"���ö�ջ"
#define FILTER_CONDITION_DURATION L"ʱ������"
#define FILTER_CONDITION_TIME L"���� ʱ��"
#define FILTER_CONDITION_RELATIVE_TIME L"���ʱ��"
#define FILTER_CONDITION_SESSION L"�Ự"
#define FILTER_CONDITION_USER_NAME L"�û�"
#define FILTER_CONDITION_USER_ID L"����ID��"
#define FILTER_CONDITION_EVENT_ID L"���к�"
#define FILTER_CONDITION_CORPORATION L"��˾"
#define FILTER_CONDITION_VERSION L"�汾"
#define FILTER_CONDITION_VIRTUAL L"���⻯"
#define FILTER_CONDITION_CPU_ARCH L"CPU�ܹ�"

#define FILTER_COMPARE_MODE_CONTAIN L"����"
#define FILTER_COMPARE_MODE_NOT_CONTAIN L"������"
#define FILTER_COMPARE_MODE_EQUAL L"����" 
#define FILTER_COMPARE_MODE_NOT_EUQAL L"������" 
#define FILTER_COMPARE_MODE_LOWER L"С��" 
#define FILTER_COMPARE_MODE_GREATER L"����"
#define FILTER_COMPARE_MODE_START L"��ʼ"
#define FILTER_COMPARE_MODE_END L"����"
#define FILTER_COMPARE_MODE_UNKNOWN L"δ֪"

#define FILTER_MODE_INCLUDE_TEXT L"����"
#define FILTER_MODE_EXCLUDE_TEXT L"�ų�"
#define FILTER_MODE_UNKNOWN_TEXT L"δ֪"

#define PLEASE_SET_FILTER_CONDITION_TIP L"�����ù�������ֵ" 
#define INPUT_FILTER_CONDITION_INVALID L"���벻��ȷ" 
#endif //LANG_EN

LRESULT WINAPI load_filter_infos_from_conf_file( LPCWSTR file_name ); 

LRESULT WINAPI save_filter_infos_to_conf_file( action_filter_info_array *filter_infos, 
											  LPCWSTR file_name, 
											  ULONG offset, 
											  ULONG limit, 
											  PVOID context ); 

LRESULT WINAPI input_filter_condition( action_filter_info *filter_condition ); 
INT32 WINAPI is_same_filter_info( action_filter_info *src_flt_info, action_filter_info *dst_flt_info ); 

NTSTATUS CALLBACK load_action_filter_cond( ULONG action_cond_id, 
										  action_filter_info *filter_cond, 
										  LPCWSTR time, 
										  PVOID work_context, 
										  ULONG *state_out ); 

#define INPUT_FILTER_INFORMATION_TO_LIST 0x00000001
LRESULT WINAPI input_default_filter_info( ULONG flags ); 


#define SHOW_WINDOW_CENTER 0x00000001
LRESULT WINAPI show_filter_dlg( CWindowWnd *parent, ULONG flags = SHOW_WINDOW_CENTER ); 

#define WM_ACTION_EVENT_FILTERING ( WM_USER + 1101 ) 

class filter_dlg : public CWindowWnd, public INotifyUI, public IDialogBuilderCallback
{
public:
	filter_dlg( CWindowWnd *wnd ); 

	//filter_dlg( CPaintManagerUI* pManager); 


/************************************************************
next functions:
1.window can do sizing,default size is bigger.
2.inner control can sizing by column header.
3.dumping logging speed fastest.

fastest log loading implementation:
1.level 1:data base
2.level 2:log memory cache
3.level 3:ui control property

next task:
1.hung when move slip bar.
2.drag column support
3.logging data precise.

************************************************************/
	
	~filter_dlg()
	{
	}; 

	NTSTATUS load_log_filter( action_filter_info *filter_cond ); 
	LRESULT load_all_log_filters(); 

	LRESULT filter_events(); 

	CControlUI* CreateControl(LPCTSTR pstrClass, CPaintManagerUI *pManager )
	{
		return NULL; 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T( "FILTER_DLG" ); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    }

	LRESULT get_selected_filter_info( action_filter_info **filter_info, BOOLEAN reseting = FALSE ); 
	LRESULT reset_flt_cond(); 
	LRESULT set_flt_cond( BOOLEAN remove_item ); 

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    }

	LRESULT WINAPI add_action_filter( r3_action_notify *action ); 
    
	LRESULT init_action_filter_list()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *Ctrl; 

		Ctrl = ( CControlUI * )m_pm.FindControl( L"flt_cond_container" ); 
		ASSERT( Ctrl != NULL ); 

		Ctrl->SetFixedWidth( 0 ); 

		Ctrl = ( CControlUI * )m_pm.FindControl( L"flt_cond_list" ); 
		ASSERT( Ctrl != NULL ); 

		Ctrl->SetFixedWidth( 0 ); 

		return ret; 
	}


	LRESULT WINAPI _Init(); 

    void OnPrepare(TNotifyUI& msg) 
    {}


	/***********************************
	ϵͳ��ģ�����ں��н���ά���������¼�����ʱ��ʹ�����µĽṹ��¼���ö�ջ��ص���Ϣ��
	1.ģ�������:���ƫ����
	
	ģ�����Ƽ����ں���ά�������ҽ���ֱ��ӳ����RING3�㡣ʹ�ü����ķ�ʽ��������б����ԵĶ�д��

	��ӡ���������󣬽��ַ����������������ٽ����ظ��Ĵ�ӡ��

	��ӡ�¼���Ϣ������
	1.�¼���ϢΪһ���ṹ�壬�����¼���Ϣ���ȶ�ͳһ���з��䣺
	 ȱ�㣺�ں����Ĵ�
	 ��λ,��ȡ�¼�����Ϣ������Էǳ���

	2.ʹ�ýṹ���������Բ�ͬ���ȵĸ����¼���Ϣ�������ж�λ��ͬʱ������Ӧ���ַ��������
	3.ʹ��һ��ͳһ�ķ������¼�����Ϣ�������Ϊһ����Ϣ��
	***********************************/
	/*
    * �ؼ��Ļص�������IListCallbackUI �е�һ���麯������Ⱦʱ������,��[1]�������˻ص�����
    */

    void Notify(TNotifyUI& msg)
    {
		CStdString name; 
		LRESULT ret; 
        
		name = msg.pSender->GetName(); 

		if( msg.sType == _T("windowinit") ) 
            OnPrepare(msg);
        else if( msg.sType == _T("click") ) 
        {
			if( name == _T("flt_load_btn" ) )
			{
				LRESULT ret; 
				WCHAR file_name[ MAX_PATH ]; 
				WCHAR app_path[ MAX_PATH ]; 
				ULONG cc_ret_len; 

				do 
				{
					ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					app_path[ cc_ret_len - 1 ] = L'\0'; 

					*file_name = _T( '\0' ); 
					ret = open_file_dlg( TRUE, L"xml", file_name, NULL, app_path ); 
					if( ret < 0 )
					{
						break; 
					}

					if( *file_name == L'\0' )
					{
						break; 
					}

					ret = delete_action_cond_from_db( NULL ); 

					if( ret != ERROR_SUCCESS )
					{

					}

					ret = release_filter_conds(); 
					if( ret != ERROR_SUCCESS )
					{

					}

					ret = load_filter_infos_from_conf_file( file_name ); 

					load_all_log_filters(); 

				}while( FALSE ); 
			}
			else if( name == _T("flt_save_btn" ) )
			{
				LRESULT ret; 
				WCHAR file_name[ MAX_PATH ]; 
				WCHAR app_path[ MAX_PATH ]; 
				ULONG cc_ret_len; 

				do 
				{
					ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					app_path[ cc_ret_len - 1 ] = L'\0'; 

					*file_name = _T( '\0' ); 
					ret = open_file_dlg( FALSE, NULL, file_name, NULL, app_path ); 
					if( ret < 0 )
					{
						break; 
					}


					if( *file_name == L'\0' )
					{
						break; 
					}
					ret = save_filter_infos_to_conf_file( &filter_infos.filter_infos, file_name, 0, 0, NULL ); 
				} while ( FALSE );

			}
            else if( name == _T( "close_btn" ) )
            {
				Close(); 
                return; 
            }

			if( name == _T( "flt_restore_btn" ) )
			{
				ret = input_default_filter_info( INPUT_FILTER_INFORMATION_TO_LIST ); 
				if( ret != ERROR_SUCCESS )
				{

				}

				ret = load_all_log_filters(); 
			}
			else if( name == _T( "flt_add_btn" ) )
			{
				set_flt_cond( FALSE ); 
			}
			else if( name == _T( "flt_del_btn" ) )
			{
				set_flt_cond( TRUE ); 
			}
			else if( name == _T( "flt_btn" ) )
			{
				COptionUI *check_box; 
				check_box = ( COptionUI* )m_pm.FindControl( _T( "flt_drop_chk" ) ); 
				ASSERT( check_box != NULL ); 
				if( check_box->IsSelected() )
				{
					set_event_filter_flags( DROP_FILTERED_EVENTS ); 
				}
				else
				{
					set_event_filter_flags( 0 ); 
				}

				filter_events();  
				Close(); 
			}
			else if( name == _T( "flt_reset_btn" ) )
			{
				CListUI *list; 

				reset_flt_cond(); 

				list = static_cast< CListUI* >( m_pm.FindControl( _T( "flt_cond_list" ) ) ); 

				ASSERT( list != NULL ); 

				list->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID ); 
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
		LRESULT ret = ERROR_SUCCESS; 
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		CDialogBuilder builder;
        CControlUI* pRoot; 

		do 
		{
			styleValue &= ~WS_CAPTION;
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			m_pm.Init(m_hWnd);

			//m_pm.SetTransparent(100);

			pRoot = builder.Create( _T("filter_dlg.xml"), 
				(UINT)0, 
				this, 
				&m_pm ); 

			ASSERT(pRoot && "Failed to parse XML");

			m_pm.AttachDialog(pRoot);
			m_pm.AddNotifier(this);

			set_ui_style( &m_pm ); 

			_Init(); 
		}while( FALSE );

		return ret;
    }

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT ret = ERROR_SUCCESS; 
		
		bHandled = TRUE;
		
		do 
		{
			switch( wParam )
			{
			case VK_ESCAPE:
				Close(); 
				break; 
			default: 
				bHandled = FALSE; 
				break; 
			}
		}while( FALSE );
		
		return ret; 
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
			case WM_CHAR:         
				lRes = OnChar( uMsg, wParam, lParam, bHandled ); 
				break; 
            default:
                bHandled = FALSE;
				break; 
        }
        if( bHandled ) return lRes;
        if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

	LRESULT change_to_local(); 

public:
	CPaintManagerUI m_pm; 
	CWindowWnd *main_wnd; 
};

LRESULT WINAPI check_action_filter_valid( action_filter_info *flt_info ); 

typedef struct _load_filter_ui_context
{
	CDialogBuilder *builder; 
	CListUI *list; 
	CPaintManagerUI *pm; 
	ULONG flags; 
	action_filter_info_array *filters; 
} load_filter_ui_context, *pload_filter_ui_context; 

INLINE LRESULT load_all_action_cond( ULONG row_offset, ULONG row_count, PVOID work_context )
{
	return output_action_cond_from_db( row_offset, row_count, work_context, load_action_filter_cond ); 
}

#endif //__FILTER_FRAME_H__