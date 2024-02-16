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
 
#ifndef __FILTER_MANAGE_DLG_H__
#define __FILTER_MANAGE_DLG_H__

#include "ui_ctrl.h"
#include "action_display.h"
#include "filter_config_file.h"
#include <vector>

#define DEFAULT_HILIGHT_COLOR RGB( 30, 10, 30 )

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
		//filter_infos.clear(); 

		//if( filter_infos != NULL )
		//{
		//	INT32 i; 
		//	for( i = 0; i < filter_infos.size(); i ++ )
		//	{
		//		if( filter_infos[ i ] != NULL )
		//		{
		//			free( filter_infos[ i ] ); 
		//		}
		//	}
		//	//free( filter_infos ); 
		//}

		//uninit_cs_lock( action_log_lock ); 

		//{
		//	INT32 i; 
		//	for( i = 0; i < action_logs.size(); i ++ )
		//	{
		//		if( action_logs[ i ] != NULL )
		//		{
		//			delete action_logs[ i ]; 
		//		}
		//		else
		//		{
		//			ASSERT( FALSE ); 
		//		}
		//	}
		//}

		//action_logs.clear(); 
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
        return _T( "FILTER_MANAGE_DLG" ); 
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
		//CListUI *List; 
		CControlUI *Ctrl; 

		Ctrl = ( CControlUI * )m_pm.FindControl( L"flt_cond_container" ); 
		ASSERT( Ctrl != NULL ); 

		Ctrl->SetFixedWidth( 0 ); 

		Ctrl = ( CControlUI * )m_pm.FindControl( L"flt_cond_list" ); 
		ASSERT( Ctrl != NULL ); 

		Ctrl->SetFixedWidth( 0 ); 

		//Ctrl = ( CControlUI * )m_pm.FindControl( L"form_layout" ); 
		//ASSERT( Ctrl != NULL ); 

		//List = ( CListLogUI * )m_pm.FindControl( L"" ); 
		//ASSERT( List != NULL ); 

		return ret; 
	}


	LRESULT WINAPI _Init(); 

    void OnPrepare(TNotifyUI& msg) 
    {

    }


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

    void Notify(TNotifyUI& msg)
    {
		CStdString name; 
		LRESULT ret; 
        
		name = msg.pSender->GetName(); 

		if( msg.sType == _T("windowinit") ) 
            OnPrepare(msg);
        else if( msg.sType == _T("click") ) 
        {
            if( name == _T( "close_btn" ) )
            {
				Close(); 
                return; 
            }
    //        else if( msg.pSender == m_pMinBtn ) 
    //        { 
    //            SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
    //            return; 
    //        }
    //        else if( msg.pSender == m_pMaxBtn ) 
    //        { 
    //            SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
    //        }
    //        else 
				//if( msg.pSender == m_pRestoreBtn ) 
    //        { 
    //            SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
    //        }
			//else 
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
				//filter_infos.push_back( flt_info ); 
				filter_events();  
				Close(); 
			}
			else if( name == _T( "flt_reset_btn" ) )
			{
				//bool _ret; 
				CListUI *list; 

				reset_flt_cond(); 

				list = static_cast< CListUI* >( m_pm.FindControl( _T( "flt_cond_list" ) ) ); 

				//FindSubControlByName( this, _T( "flt_cond_list" ) ); 
				ASSERT( list != NULL ); 

				list->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID ); 
				//if( _ret == false )
				//{
				//}
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
  //      else if(msg.sType == _T("menu")) 
  //      {
  //          if( msg.pSender->GetName() != _T("domainlist") ) return;
  //          menu_ui* pMenu = new menu_ui();
  //          if( pMenu == NULL ) { return; }
  //          POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
  //          ::ClientToScreen(*this, &pt);
  //          pMenu->Init(msg.pSender, pt);
  //      }
  //      else if( msg.sType == _T("menu_Delete") ) {
  //          CListUI* pList = static_cast<CListUI*>(msg.pSender);
  //          int nSel = pList->GetCurSel();
  //          if( nSel < 0 ) return;
  //          pList->RemoveAt(nSel);
	
		//remove_list_item( nSel ); 
		//}
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

	//LRESULT on_action_event_filter(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	//{
	//	LRESULT ret = ERROR_SUCCESS; 

	//	do 
	//	{
	//		bHandled = TRUE; 

	//	}while( FALSE );
	//
	//	return ret; 
	//}

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
				//case WM_ACTION_EVENT_LOG:
			//	{
			//		lRes = on_action_event_log(uMsg, wParam, lParam, bHandled); 
			//		break;
			//	}
            default:
                bHandled = FALSE;
				break; 
        }
        if( bHandled ) return lRes;
        if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

public:
	CPaintManagerUI m_pm; 
	CWindowWnd *main_wnd; 

private:
};

LRESULT WINAPI check_action_filter_valid( action_filter_info *flt_info ); 

typedef struct _load_filter_context
{
	CDialogBuilder *builder; 
	CListUI *list; 
	CPaintManagerUI *pm; 
	ULONG flags; 
	action_filter_info_array *filters; 
} load_filter_ui_context, *pload_filter_context; 

INLINE LRESULT load_all_action_cond( LPCWSTR conf_file_name, 
									ULONG row_offset, 
									ULONG row_count, 
									PVOID work_context )
{
	return output_action_cond_from_conf_file( conf_file_name, row_offset, row_count, work_context, load_action_filter_cond ); 
}

#endif //__FILTER_MANAGE_DLG_H__