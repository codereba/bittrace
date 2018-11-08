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

#include <objbase.h>
#include <zmouse.h>
#include <exdisp.h>
#include <vector>
#include <sstream>
#include "antiarp_api.h"
#include "ui_ctrl.h"



#define WM_OUTPUT_ARP_INFO ( WM_USER + 101 )

#define START_ANTI_ARP_TITLE _T( "开启ARP攻击过滤" )
#define STOP_ANTI_ARP_TITLE _T( "停止ARP攻击过滤" )

#define ARP_HOST_MAC_LISTS_BUFF_LEN ( ULONG )( 1024 * 200 )
#define MAX_ARP_HOST_MAC_LIST_NUM ( ( ARP_HOST_MAC_LISTS_BUFF_LEN - sizeof( ARP_HOST_MAC_LISTS ) + sizeof( ARP_HOST_MAC_OUTPUT ) ) / sizeof( ARP_HOST_MAC_OUTPUT ) ) 

/*
*  线程函数中传入的结构体变量，使用线程为了使界面线程立即返回，防止卡住，你们懂得。
*/
struct Prama
{
	HWND hWnd;
	CListUI* pList;
	CButtonUI* pSearch;
	CStdString tDomain;
};

class anti_arp_dlg : public CWindowWnd, public INotifyUI, public action_ui_notify
{
public:
	anti_arp_dlg() : /*all_arp_info_count( 0 ), */
		anti_arp_started( FALSE )
	{ };

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("AntiArpDlg"); 
    };

	LRESULT set_anti_arp_work_mode( ULONG work_mode )
	{
		LRESULT ret = ERROR_SUCCESS; 

		if( work_mode != 0 )
		{
			ret = do_ui_ctrl( BITSAFE_UI_ID, 
				ANTI_ARP_START, 
				NULL, 
				0, 
				NULL, 
				0, 
				NULL, 
				this ); 
		}
		else 
		{
			ret = do_ui_ctrl( BITSAFE_UI_ID, 
				ANTI_ARP_STOP, 
				NULL, 
				0, 
				NULL, 
				0, 
				NULL, 
				this ); 
		}


		return ret; 
	}

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

    void Init() 
    {
		LRESULT ret = ERROR_SUCCESS; 
        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		arp_infos = static_cast< CTreeUI* >( m_pm.FindControl( _T( "arp_infos" ) ) ); 
		ASSERT( arp_infos != NULL ); 

		set_ctrl_text( &m_pm, _T( "anti_arp_btn" ), TEXT_ANTI_ARP_OPEN ); 
		set_ctrl_text( &m_pm, _T("anti_arp_notice_txt"), TEXT_ANTI_ARP_NOTICE ); 


		if( arp_infos != NULL)
		{
			if( arp_infos ->GetSubItemCount( UI_LIST_CONTAINER_ELEMENT_CLASS_ID ) > 0 )
			{
				arp_infos->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID );
			}
		}

		ret = ui_init(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			ANTI_ARP_INIT, 
			NULL, 
			0, 
			NULL, 
			0, 
			NULL, 
			( action_ui_notify* )this ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

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

	LRESULT on_ok_bn()
	{
		// TODO: 在此添加控件通知处理程序代码

		INT32 ret = 0; 
		CControlUI *ok_btn; 
		LPCTSTR bn_title = NULL; 
		ok_btn = m_pm.FindControl( _T( "anti_arp_btn" ) ); 
		ASSERT( ok_btn != NULL ); 

		if( anti_arp_started == FALSE )
		{
			ret = do_ui_ctrl( BITSAFE_UI_ID, 
				ANTI_ARP_START, 
				NULL, 
				0, 
				NULL, 
				0, 
				NULL, 
				this ); 

			if( ret == 0 )
			{
				anti_arp_started = TRUE; 

				bn_title = _get_string_by_id( TEXT_ANTI_ARP_CLOSE, STOP_ANTI_ARP_TITLE ); 
			}
		}
		else
		{
			ret = do_ui_ctrl( BITSAFE_UI_ID, 
				ANTI_ARP_STOP, 
				NULL, 
				0, 
				NULL, 
				0, 
				NULL, 
				this ); 

			if( ret == 0 )
			{
				anti_arp_started = FALSE; 
				bn_title = _get_string_by_id( TEXT_ANTI_ARP_OPEN, START_ANTI_ARP_TITLE ); 
			}
		}

		if( bn_title != NULL )
		{
			ok_btn->SetText( bn_title ); 
		}

		return ret; 
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
                SendMessage( WM_SYSCOMMAND, SC_MINIMIZE, 0 );
                return; 
            }

			else if( msg.pSender->GetName() == _T( "anti_arp_btn" ) )
			{
				on_ok_bn(); 
			}
        }

        else if( msg.sType == _T( "menu" ) ) 
        {
        }
    }

	LRESULT ui_init()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *ok_btn; 

		ok_btn = m_pm.FindControl( _T( "anti_arp_btn" ) ); 
		ASSERT( ok_btn != NULL ); 

		ok_btn->SetText( START_ANTI_ARP_TITLE ); 

		return ret; 
	}

#define ARP_IP_ADDR 0x00000001
#define ARP_MAC_ADDR 0x00000002

	LRESULT before_arp_info_output()
	{
		ASSERT( arp_infos != NULL ); 

		if( arp_infos->GetSubItemCount( UI_LIST_CONTAINER_ELEMENT_CLASS_ID ) > 0 )
		{
			arp_infos->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID ); 
		}

		return ERROR_SUCCESS; 
	}

	LRESULT add_arp_info_to_ui( 
		LPCTSTR arp_info, 
		PVOID context, 
		LONG flags, 
		PVOID *param_out )
	{
		LRESULT ret = ERROR_SUCCESS;
		Node* node_added; 
		tree_list_item_info item;

		ASSERT( arp_infos != NULL ); 

		if( flags == ARP_IP_ADDR )
		{
			item.folder = true; 
			item.empty = false; 
			item.description = arp_info; 

			node_added = arp_infos->AddNode( item, NULL ); 
			
			ASSERT( param_out != NULL ); 
			if( node_added == NULL )
			{
				ASSERT( FALSE ); 
				*param_out == NULL; 
				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
			else
			{
				*param_out = ( PVOID )node_added; 
			}
		}
		else if( flags == ARP_MAC_ADDR )
		{
			ASSERT( context != NULL ); 

			if( param_out != NULL )
			{
				*param_out = NULL; 
			}

			item.folder = false; 
			item.empty = false; 
			item.description = arp_info; 
			node_added = arp_infos->AddNode( item, ( Node* )context ); 

			if( node_added == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
		}

		return ret; 
	}

	LRESULT add_arp_info_to_list( 
		LPCTSTR ip_addr, 
		LPCTSTR mac_addr, 
		LONG item_index )
	{

		LRESULT ret = ERROR_SUCCESS; 
		INT32 item_count;

#ifdef _ARP_INFO_LIST_OUTPUT
		item_count = arp_info_list.size(); 

		if( item_index >= item_count )
		{
		}
		else
		{
			arp_info_list[ item_index ] = ip_addr; 
		}

		if( item_index + 1 >= item_count )
		{
			arp_info_list.push_back( mac_addr );
		}
		else
		{
			arp_info_list[ item_index + 1 ] = mac_addr; 
		}

#endif //_ARP_INFO_LIST_OUTPUT

_return:
		return ret;
	}

#define IP_ADDR_BUF_LEN 128
#define MAC_ADDR_BUF_LEN 128
	LRESULT output_arp_info( PARP_HOST_MAC_LISTS arp_info_list )
	{
		LRESULT ret = ERROR_SUCCESS; 
		PARP_HOST_MAC_OUTPUT HostMacList; 
		ULONG OutputHostsNum; 
		PVOID item_ip_addr; 
		TCHAR ip_addr[ IP_ADDR_BUF_LEN ]; 
		TCHAR mac_addr[ MAC_ADDR_BUF_LEN ]; 

		OutputHostsNum = arp_info_list->OutputHostNum; 
		if( OutputHostsNum > arp_info_list->AllHostNum || 
			OutputHostsNum > MAX_ARP_HOST_MAC_LIST_NUM )
		{
			ret = FALSE; 
			goto _return; 
		}

		before_arp_info_output(); 

		//all_arp_info_count = 0; 
		for( INT32 i = 0; ( ULONG )i < OutputHostsNum; i ++ )
		{ 
			PBYTE _IP; 
			LPCTSTR tmp_text; 
			//CTreeNode *TreeNode; 

			HostMacList = &arp_info_list->Hosts[ i ];
			_IP = ( PBYTE )&HostMacList->HostIP; 

			if( HostMacList->ArpMacAddrNum > MAX_ARP_PACK_MAC_NUM )
			{
				log_trace( ( MSG_ERROR, "Fatal error happen, program will exit now or program behavior will be unpredictable." ) ); 
				ASSERT( FALSE && "*** the count of mac addresses of one ip host is too much *** " ); 
				//break; 
				continue; 
			}

			if( HostMacList->ArpMacAddrNum == 0 )
			{
				continue; 
			}

#define ARP_IP_FMT_STRING_ENGLISH _T( "Host IP %d.%d.%d.%d Arp replied mac: \n" )
			
			tmp_text = _get_string_by_id( TEXT_ANTI_ARP_HOST_MAC_LIST, 
				_T( "主机 %d.%d.%d.%d 对应 mac 地址(ARP协议回复): \n" ) ); 
			_stprintf( ip_addr, tmp_text, _IP[ 0 ], 
				_IP[ 1 ], 
				_IP[ 2 ], 
				_IP[ 3 ] ); 

			log_trace( ( MSG_INFO, "output ip of arp rely information %ws\n", ip_addr ) ); 
			
			ret = add_arp_info_to_ui( ip_addr, NULL, ARP_IP_ADDR, &item_ip_addr ); 
			if( ret != ERROR_SUCCESS )
			{
				continue; 
			}

			ASSERT( item_ip_addr != NULL ); 

			for( INT32 j = 0; j < HostMacList->ArpMacAddrNum; j ++ )
			{
				_sntprintf( mac_addr, 64, _T( "0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x \n" ), 
					HostMacList->ArpMacAddrs[ 0 ][ 0 ], 
					HostMacList->ArpMacAddrs[ 0 ][ 1 ], 
					HostMacList->ArpMacAddrs[ 0 ][ 2 ], 
					HostMacList->ArpMacAddrs[ 0 ][ 3 ], 
					HostMacList->ArpMacAddrs[ 0 ][ 4 ], 
					HostMacList->ArpMacAddrs[ 0 ][ 5 ] ); 


				log_trace( ( MSG_INFO, "mac %d: %ws\n", mac_addr ) ); 
				
				ret = add_arp_info_to_ui( mac_addr, item_ip_addr, ARP_MAC_ADDR, NULL ); 
				if( ret != ERROR_SUCCESS )
				{
					continue; 
				}
			}

			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "add arp info failed \n" ) ); 
			}
		}

_return:
		return ret; 
	}

	LRESULT action_notify( action_ui_notify_type type, PVOID param )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CControlUI *control; 
		CStdString tip; 
		notify_data *data_notify; 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		data_notify = ( notify_data* )param; 

		if( type == ARP_INFO_NOTIFY )
		{
			ret = SendMessage( /*GetHWND(), */WM_OUTPUT_ARP_INFO, 0, ( LPARAM )data_notify->data ); 
		}
		else
		{
			ASSERT( FALSE ); 
		}

_return:
		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
		return ret; 
	}

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create( _T( "anti_arp_dlg.xml" ), ( UINT )0, NULL, &m_pm ); 
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
		LRESULT ret; 

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			ANTI_ARP_UNINIT, 
			NULL, 
			0, 
			NULL, 
			0, 
			NULL, 
			this ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "uninitialize the arp context failed \n" ) ); 
		}

        return 0;
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		bHandled = FALSE; 
		m_pm.RemoveNotifier(this);
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
        switch( uMsg )
		{
			case WM_OUTPUT_ARP_INFO:
				{
					ASSERT( lParam != NULL ); 

					lRes = output_arp_info( ( PARP_HOST_MAC_LISTS )lParam ); 
				}
				break; 
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
	CTreeUI *arp_infos; 

	INT32 anti_arp_started; 
};