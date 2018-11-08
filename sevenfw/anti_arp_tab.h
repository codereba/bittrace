/*
 * Copyright 2010 JiJie Shi
 *
 * This file is part of NetMonitor.
 *
 * NetMonitor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NetMonitor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NetMonitor.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  2007.6.10	Ji Jie Shi modified it for this ui lib. 
 */

 
 #ifndef __ANTI_ARAP_TAB_H__
#define __ANTI_ARAP_TAB_H__

#include "antiarp_api.h"
#include "ui_ctrl.h"

#define START_ANTI_ARP_TITLE _T( "开启ARP攻击过滤" )
#define STOP_ANTI_ARP_TITLE _T( "停止ARP攻击过滤" )

#define ARP_HOST_MAC_LISTS_BUFF_LEN ( ULONG )( 1024 * 300 )
#define MAX_ARP_HOST_MAC_LIST_NUM ( ( ARP_HOST_MAC_LISTS_BUFF_LEN - sizeof( ARP_HOST_MAC_LISTS ) + sizeof( ARP_HOST_MAC_OUTPUT ) ) / sizeof( ARP_HOST_MAC_OUTPUT ) ) 

INT32 anti_arp_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param ); 

class anti_arp_tab :
	public CDialogImpl<anti_arp_tab, ui_base_tab>,
	public CWinDataExchange<anti_arp_tab>
{
public:
	enum { IDD = IDD_ANTI_ARP_TAB };

	anti_arp_tab() : list_item_index( 0 ), 
		anti_arp_started( FALSE )
	{
	}

	BEGIN_MSG_MAP(anti_arp_tab)
		MESSAGE_HANDLER( WM_INITDIALOG, on_init_dlg )
		MESSAGE_HANDLER( WM_SHOWWINDOW, on_show_wnd )
		COMMAND_HANDLER( IDOK, BN_CLICKED, on_ok_bn )
		//MESSAGE_HANDLER(WM_PAINT, on_paint);
		//MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor )
		//MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor )
		//NOTIFY_ID_HANDLER( IDOK, on_set_work_mode )
		//COMMAND_HANDLER(IDC_LIST_ALL_MENUITEM, LBN_SELCHANGE, OnListSelChange)
		CHAR output[ 1024 ]; 
		sprintf( output, "current msg is 0x%x \n", uMsg ); 
		DebugTrace( output ); 

		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP(anti_arp_tab)
	END_DDX_MAP()

	//LRESULT on_set_work_mode( INT32 param, NMHDR *pnmhdr, BOOL &handled)
	//{
	//	INT32 ret; 
	//	INT32 state; 
	//	HWND check_box; 

	//	handled = TRUE; 

	//	dbg_print( 0, DBG_DBG_BUF_OUT, "notify code is %d \n", pnmhdr->code ); 

	//	if( pnmhdr->code != BN_CLICKED )
	//	{
	//		return 0; 
	//	}

	//	check_box = GetDlgItem( IDC_CHECK_WORK_MODE ); 
	//	ASSERT( check_box != NULL ); 
	//	MessageBox( "set work mode" ); 
	//	state = ( INT32 )::SendMessage( check_box, BM_GETCHECK, 0, 0 ); 

	//	if( state == BST_CHECKED )
	//	{
	//		ret = do_ui_ctrl( ANTI_ARP_UI_ID, 
	//			ANTI_ARP_START, 
	//			NULL, 
	//			0, 
	//			NULL, 
	//			0, 
	//			this ); 
	//	}
	//	else if( state == BST_UNCHECKED )
	//	{
	//		ret = do_ui_ctrl( ANTI_ARP_UI_ID, 
	//			ANTI_ARP_STOP, 
	//			NULL, 
	//			0, 
	//			NULL, 
	//			0, 
	//			this ); 
	//	}
	//	else
	//	{
	//		ASSERT( FALSE ); 
	//	}

	//	return 0; 
	//}

	LRESULT on_show_wnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	INT32 ui_init()
	{
		INT32 ret; 
		INT32 i; 
		HWND list; 
		HWND ok_bn; 
		//HWND check_box; 
		LVCOLUMN lvc;

		CHAR *columns_title[] = 
		{ 
			"Arp info"
		};

		DWORD columns_len[] = { 
			700
		};

		ok_bn = 

		list = ( HWND )GetDlgItem( IDC_LIST_ARP_INFO );

		ASSERT( list != NULL ); 
		ASSERT( ok_bn != NULL ); 

		::SetWindowText( ok_bn, START_ANTI_ARP_TITLE ); 

		SendMessage( list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
			LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP );

		for( ; ; )
		{
			ret = ListView_DeleteColumn( list, 1 );
			if( FALSE == ret )
			{
				break;
			}
		}

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		for ( i = 0; i < sizeof( columns_title ) / sizeof( columns_title[ 0 ] ); i++ ) 
		{ 
			lvc.pszText = columns_title[ i ];    
			lvc.cx = columns_len[ i ];

			lvc.fmt = LVCFMT_LEFT;                             

			if (ListView_InsertColumn( list, i, &lvc ) == -1) 
				return -1; 
		}

		return 0; 
	}

	//HBRUSH hBakBr;
	LRESULT on_init_dlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 ret; 
		//if (1 == GetPropertySheet().GetActiveIndex())
		//	GetPropertySheet().CenterWindow();

		//hBakBr = ::CreateSolidBrush( XCONFIG_DLG_BK_CLR ); 

		ret = ui_init(); 
		if( ret < 0 )
		{
			goto error_return; 
		}

		ret = do_ui_ctrl( ANTI_ARP_UI_ID, 
			ANTI_ARP_INIT, 
			NULL, 
			0, 
			NULL, 
			0, 
			this ); 

		if( 0 > ret )
		{
			goto error_return; 
		}
	
		bHandled = TRUE; 
		return TRUE;  // Let the system set the focus

error_return:
		DestroyWindow(); 
		return TRUE;
	}

	INT32 output_arp_info( PARP_HOST_MAC_LISTS arp_info_list )
	{
		INT32 ret; 
		PARP_HOST_MAC_OUTPUT HostMacList; 
		ULONG OutputHostsNum; 
		CString ip_addr; 
		CString mac_addr; 

		OutputHostsNum = arp_info_list->OutputHostNum; 

		if( OutputHostsNum > arp_info_list->AllHostNum || 
			OutputHostsNum > MAX_ARP_HOST_MAC_LIST_NUM )
		{
			return FALSE; 
		}

		//ArpHostMacList.DeleteAllNode(); 

		ip_addr.GetBufferSetLength( 128 ); 

		list_item_index = 0; 

		for( INT32 i = 0; ( ULONG )i < OutputHostsNum; i ++ )
		{ 
			PBYTE _IP; 
			//CTreeNode *TreeNode; 

			HostMacList = &arp_info_list->Hosts[ i ];
			_IP = ( PBYTE )&HostMacList->HostIP; 

			if( HostMacList->ArpMacAddrNum > MAX_ARP_PACK_MAC_NUM )
			{
				if( IDOK == MessageBox( "Fatal error happen, program will exit now or program behavior will be unpredictable.", NULL, MB_OKCANCEL ) )
				{
					EndDialog( 0 ); 
				}

				continue; 
			}

			if( HostMacList->ArpMacAddrNum == 0 )
			{
				continue; 
			}

			ip_addr.Format( "Host IP %d.%d.%d.%d Arp replied mac: \n", _IP[ 0 ], 
				_IP[ 1 ], 
				_IP[ 2 ], 
				_IP[ 3 ] ); 

			OutputDebugString( ip_addr.GetBuffer(0) ); 
			//TreeNode = ArpHostMacList.InsertChild(NULL, output, TEXT_CLR, HOST_IP_BK_CLR, HL_TEXT_CLR, HL_HOST_IP_BK_CLR, FALSE );

			//output.Format( " TreeNode is 0x%0.8x \n", TreeNode ); 
			//OutputDebugString( output.GetBuffer() ); 

			//for( INT32 j = 0; j < HostMacList->ArpMacAddrNum; j ++ )
			//{
			mac_addr.Format( "0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x \n", 
				HostMacList->ArpMacAddrs[ 0 ][ 0 ], 
				HostMacList->ArpMacAddrs[ 0 ][ 1 ], 
				HostMacList->ArpMacAddrs[ 0 ][ 2 ], 
				HostMacList->ArpMacAddrs[ 0 ][ 3 ], 
				HostMacList->ArpMacAddrs[ 0 ][ 4 ], 
				HostMacList->ArpMacAddrs[ 0 ][ 5 ] ); 

			OutputDebugString( mac_addr.GetBuffer(0) ); 
			//ArpHostMacList.InsertChild(NULL, output, TEXT_CLR, HOST_MAC_BK_CLR, HL_TEXT_CLR, HL_HOST_MAC_BK_CLR, FALSE );
			//}

			HWND list; 
			list = GetDlgItem( IDC_LIST_ARP_INFO ); 

			ret = add_arp_info_to_list( list, list_item_index, ip_addr.GetBuffer(0), mac_addr.GetBuffer( 0 ) );
			if( ret == 0 )
			{
				list_item_index += 2; 
			}

			//OutputDebugString( output.GetBuffer() ); 
		}

		//ArpHostMacList.Invalidate(); 

		return TRUE; 
	}

	INT32 add_arp_info_to_list( HWND ListControl, 
		DWORD item_index, 
		LPCSTR ip_addr, 
		LPCSTR mac_addr )
	{
		INT32 ret;
		LV_ITEM lv_item;
		LV_ITEM lv_item_append;
		INT32 nItemCount;

		memset( &lv_item, 0, sizeof( lv_item ) );
		lv_item.mask = LVIF_TEXT;

		lv_item.pszText = ( LPSTR )ip_addr;
		lv_item.iItem = item_index;
		lv_item.iSubItem = 0;

		memset( &lv_item_append, 0, sizeof( lv_item_append ) );
		lv_item_append.mask = LVIF_TEXT;

		lv_item_append.pszText = ( LPSTR )mac_addr;
		lv_item_append.iItem = item_index + 1;
		lv_item_append.iSubItem = 0;

		nItemCount = ListView_GetItemCount( ListControl );

		if( item_index >= nItemCount )
		{
			ret = ListView_InsertItem( ListControl, &lv_item );
			if( ret < 0 )
			{
				return -1; 
			}
		}
		else
		{
			ret = ListView_SetItem( ListControl, &lv_item );
			if( ret < 0 )
			{ 
				return -1; 
			}
		}

		if( item_index + 1 >= nItemCount )
		{
			ret = ListView_InsertItem( ListControl, &lv_item_append );
			if( ret < 0 )
			{
				return -1; 
			}
		}
		else
		{
			ret = ListView_SetItem( ListControl, &lv_item_append );
			if( ret < 0 )
			{ 
				return -1; 
			}
		}

		if( 0 > ret )
		{
			ASSERT( FALSE );
			return -1;
		}

		return 0;
	}

	LRESULT on_paint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		bHandled = FALSE;
		//bHandled = TRUE;
		return 0;
	}

	LRESULT on_ok_bn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		// TODO: 在此添加控件通知处理程序代码

		INT32 ret; 
		HWND bn_ok; 
		LPCSTR bn_title = NULL; 
		bn_ok = GetDlgItem( IDOK ); 
		ASSERT( bn_ok != NULL ); 

		if( anti_arp_started == FALSE )
		{
			ret = do_ui_ctrl( ANTI_ARP_UI_ID, 
				ANTI_ARP_START, 
				NULL, 
				0, 
				NULL, 
				0, 
				this ); 

			if( ret == 0 )
			{
				anti_arp_started = TRUE; 
				bn_title = STOP_ANTI_ARP_TITLE; 
			}
		}
		else
		{
			ret = do_ui_ctrl( ANTI_ARP_UI_ID, 
				ANTI_ARP_STOP, 
				NULL, 
				0, 
				NULL, 
				0, 
				this ); 

			if( ret == 0 )
			{
				anti_arp_started = FALSE; 
				bn_title = START_ANTI_ARP_TITLE; 
			}
		}

		if( bn_title != NULL )
		{
			::SetWindowText( bn_ok, bn_title ); 
		}
		return 0; 
	}

protected:
	INT32 anti_arp_started; 
	ULONG list_item_index;
};

#endif //__ANTI_ARAP_TAB_H__
