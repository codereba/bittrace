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

#ifndef _NET_MON_TAB_H__
#define _NET_MON_TAB_H__

#include "netmon_.h"
#include "trace_log_api.h"

HRESULT net_mon_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param );

INLINE INT32 flt_proc( DWORD dwProcessId )
{	
	ULONG BLOCKED_PROCESSES_ID[] = { 
		SYSTEM_SYSTEM_PROCESS_ID, 
			SYSTEM_IDLE_PROCESS_ID
	};

	INT32 i;
	for( i = 0; i < ARRAY_SIZE( BLOCKED_PROCESSES_ID ); i ++ )
	{
		if( dwProcessId == BLOCKED_PROCESSES_ID[ i ] )
		{
			return FALSE;
		}
	}
	return TRUE;
}

class net_mon_tab :
	public CDialogImpl<net_mon_tab, ui_base_tab>,
	public CWinDataExchange<net_mon_tab>
{
public:
	enum { IDD = IDD_NET_MON_TAB };

	net_mon_tab()
	{
		//m_strFilter = __t("ICON image(*.ico)|*.ico|Bitmap image(*.bmp)|*.bmp||"); //|GIF image(*.gif)|*.gif|
		//ConvertFilterString(m_strFilter);
		////m_strImage = __t("");
		//m_strUrl = __t("");
		//m_strDesc = __t("");
		//m_strCaption = __t("");
		//m_pMainMenu = NULL;
		//m_pXmlDoc = NULL;
		//m_pRootNode = NULL;
	}

	BEGIN_MSG_MAP(net_mon_tab)
		MESSAGE_HANDLER(WM_INITDIALOG, on_init_dlg)
		MESSAGE_HANDLER(WM_SHOWWINDOW, on_show_wnd)
		COMMAND_HANDLER(IDOK, BN_CLICKED, on_ok_bn )
		//COMMAND_HANDLER(IDC_BUTTON_MENU_DELETE_, BN_CLICKED, OnDelBtnClicked)
		//MESSAGE_HANDLER(WM_PAINT, on_paint);
		//MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor )
		//MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor )
		//COMMAND_HANDLER(IDC_STATIC_IMG_MENU, STN_CLICKED, OnNorImgClicked)
		NOTIFY_ID_HANDLER(IDC_LIST_NET_TRAFFIC, on_list_notify)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP(net_mon_tab)
	END_DDX_MAP()

	INT32 update_traffic_sum( PBYTE output_buf, ULONG output_length, PVOID param )
	{
		INT32 ret; 
		HWND hAllTraffic; 
		PULARGE_INTEGER all_traffic; 
		CHAR output[ 256 ]; 

		hAllTraffic = GetDlgItem( IDC_STC_ALL_TRAFFIC );
		ASSERT( NULL != hAllTraffic );

		all_traffic = ( PULARGE_INTEGER )output_buf; 

		sprintf( output, "Uploaded: %d%d, Downloaded: %d%d\n", 
			all_traffic[ 1 ].LowPart, all_traffic[ 1 ].HighPart, 
			all_traffic[ 0 ].LowPart, all_traffic[ 0 ].HighPart 
			);

		::SetWindowText( hAllTraffic, output );
		return 0; 
	}

	#define MAX_DESC_INFO_LEN 256

	INT32 update_all_proc_traffic( PBYTE output_buf, ULONG output_length, PVOID param )
	{
		INT32 ret;
		HWND hAllTraffic;
		HWND hListProcess;
		DWORD dwItemIndex;
		HWND hDlg;
		PUI_CTRL ui_ctrl; 

		WCHAR szFileName[ MAX_PATH ];
		WCHAR szNativeFileName[ PROCESS_IMAGE_FILE_PATH_INFO_MAX_LENGTH ];

		PPROCESS_IO_INFO_OUTPUT pProcessIo;
		LARGE_INTEGER all_traffic[ 2 ];
		CHAR szOutputMsg[ 512 ];
		INT32 i;

		//hDlg = ( HWND )param;
		hAllTraffic = GetDlgItem( IDC_STC_ALL_TRAFFIC );
		ASSERT( NULL != hAllTraffic );

		hListProcess = GetDlgItem( IDC_LIST_NET_TRAFFIC );

		ASSERT( NULL != hListProcess );

		dwItemIndex = 0;

		pProcessIo = ( PPROCESS_IO_INFO_OUTPUT )output_buf; 

		for( i = 0; ( ULONG )i < output_length / sizeof( PROCESS_IO_INFO_OUTPUT ); i ++ )
		{
			DWORD wait_ret; 
			sprintf( szOutputMsg, "NetMgr Process Id: %d \n", // Sending Speed: %d\n", 
				pProcessIo[ i ].dwProcessId );
			//pProcessIo[ i ].SendingSpeed.LowPart );
			//DebugTrace( szOutputMsg );

			if( SYSTEM_SYSTEM_PROCESS_ID == pProcessIo[ i ].dwProcessId )
			{
				wcscpy( szFileName, L"System" );
			}
			else if( SYSTEM_IDLE_PROCESS_ID == pProcessIo[ i ].dwProcessId )
			{
				wcscpy( szFileName, L"System Idle Process" );
			}
			else if( FALSE == get_proc_img_path( pProcessIo[ i ].dwProcessId, szFileName, sizeof( szFileName ), NULL, 0 ) )
			{
				sprintf( szOutputMsg, "Get process %d image path failed\n", 
					pProcessIo[ i ].dwProcessId );

				//DebugTrace( szOutputMsg );
				continue;
			}

			wait_ret = WaitForSingleObject( proc_info_lock, INFINITE );
			if( WAIT_OBJECT_0 != wait_ret && WAIT_ABANDONED != wait_ret )
			{
				ASSERT( FALSE ); 
			}

			add_proc_info_to_list( hListProcess, 
				dwItemIndex ++, 
				pProcessIo[ i ].dwProcessId, 
				pProcessIo[ i ].AllSuccSendedDataSize.LowPart, 
				pProcessIo[ i ].AllSuccRecvedDataSize.LowPart, 
				pProcessIo[ i ].bStopSend, 
				pProcessIo[ i ].bStopRecv, 
				pProcessIo[ i ].SendingSpeed.LowPart, 
				pProcessIo[ i ].SuccSendedDataSizeOnce.LowPart, 
				pProcessIo[ i ].SuccRecvedDataSizeOnce.LowPart, 
				szFileName 
				);

			ReleaseMutex( proc_info_lock );
		}

		return 0; 
	}

	INT32 add_proc_info_to_list( HWND ListControl, 
	DWORD item_index, 
	DWORD dwProcessId, 
	DWORD dwAllUpLoad,
	DWORD dwAllDownLoad, 
	DWORD bBlockSend,
	DWORD bBlockRecv, 
	DWORD dwUpLoadSpeedControl, 
	DWORD dwRecvingSpeed,
	DWORD dwSendingSpeed, 
	LPWSTR lpwszProcessImagePath )
	{
		INT32 ret;
		CHAR desc[ MAX_DESC_INFO_LEN ];
		LV_ITEM lv_item;
		LV_ITEMW lv_path_item;
		DWORD out_str_len;
		INT32 nItemCount;

		memset( &lv_item, 0, sizeof( lv_item ) );
		lv_item.mask = LVIF_TEXT | LVIF_PARAM;
		lv_path_item.mask = LVIF_TEXT;

		nItemCount = ListView_GetItemCount( ListControl );

		sprintf( desc, "%u", dwProcessId );
		lv_item.pszText = desc;
		lv_item.lParam = ( LPARAM )dwProcessId;
		lv_item.iItem = item_index;
		lv_item.iSubItem = 0;
		if( item_index >= nItemCount )
		{
			ret = ListView_InsertItemA( ListControl, &lv_item );
		}
		else
		{
			ret = ListView_SetItemA( ListControl, &lv_item );
		}

		if( 0 > ret )
		{
			ASSERT( FALSE );
			return -1;
		}

		lv_path_item.pszText = lpwszProcessImagePath;
		lv_path_item.iItem = item_index;
		lv_path_item.iSubItem = 1;
		ret = ListView_SetItemW( ListControl, &lv_path_item );

		if( FALSE == ret )
		{
			ASSERT( FALSE );
			return -1;
		}

		lv_item.mask = LVIF_TEXT;
		sprintf( desc, "%u", dwAllUpLoad );
		lv_item.pszText = desc;
		lv_item.iItem = item_index;
		lv_item.iSubItem = 2;
		ret = ListView_SetItemA( ListControl, &lv_item );

		if( FALSE == ret )
		{
			ASSERT( FALSE );
			return -1;
		}

		lv_item.mask = LVIF_TEXT;
		sprintf( desc, "%u", dwAllDownLoad );
		lv_item.pszText = desc;
		lv_item.iItem = item_index;
		lv_item.iSubItem = 3;
		ret = ListView_SetItemA( ListControl, &lv_item );

		if( FALSE == ret )
		{
			ASSERT( FALSE );
			return -1;
		}

		lv_item.mask = LVIF_TEXT;
		*desc = '\0';
		if( bBlockSend )
		{
			strcat( desc, "BlkUp/" );
		}
		else
		{
			strcat( desc, "AllowUp/" );
		}

		if( bBlockRecv )
		{
			strcat( desc, "BlkDown" );
		}
		else
		{
			strcat( desc, "AllowDown" );
		}

		lv_item.pszText = desc;
		lv_item.iItem = item_index;
		lv_item.iSubItem = 4;
		ret = ListView_SetItemA( ListControl, &lv_item );

		if( FALSE == ret )
		{
			ASSERT( FALSE );
			return -1;
		}

		if( dwUpLoadSpeedControl == 0xFFFFFFFF )
		{
			dwUpLoadSpeedControl = 0;
		}

		lv_item.mask = LVIF_TEXT;
		sprintf( desc, "%u", dwUpLoadSpeedControl );
		lv_item.pszText = desc;
		lv_item.iItem = item_index;
		lv_item.iSubItem = 5;
		ret = ListView_SetItemA( ListControl, &lv_item );

		if( FALSE == ret )
		{
			ASSERT( FALSE );
			return -1;
		}

		lv_item.mask = LVIF_TEXT;
		sprintf( desc, "%u", dwRecvingSpeed );
		lv_item.pszText = desc;
		lv_item.iItem = item_index;
		lv_item.iSubItem = 6;
		ret = ListView_SetItemA( ListControl, &lv_item );

		if( FALSE == ret )
		{
			ASSERT( FALSE );
			return -1;
		}

		lv_item.mask = LVIF_TEXT;
		sprintf( desc, "%u", dwSendingSpeed );
		lv_item.pszText = desc;
		lv_item.iItem = item_index;
		lv_item.iSubItem = 7;
		ret = ListView_SetItemA( ListControl, &lv_item );

		if( FALSE == ret )
		{
			ASSERT( FALSE );
			return -1;
		}

		return TRUE;
	}

	#define SENDING_SPEED_STRING_LEN 20
	INT32 ctrl_proc_net_traffic( HWND hMainDlg )
	{
		INT32 ret;
		INT32 nSelItem;
		LV_ITEMW lvItem;
		DWORD dwProcessId;
		DWORD dwSendingSpeed;
		HANDLE hProcess;
		DWORD dwRet;
		PROCESS_INFORMATION_RECORD ProcessControl;
		CHAR szText[ SENDING_SPEED_STRING_LEN ];
		HWND hEdit;
		HWND hListProcess;
		HWND hCheckBlockSend;
		HWND hCheckBlockRecv;

		hEdit = GetDlgItem( IDC_EDIT_SENDING_SPEED );
		hCheckBlockSend = GetDlgItem( IDC_CHECK_BLOCK_SEND );
		hCheckBlockRecv = GetDlgItem( IDC_CHECK_BLOCK_RECEIVE );
		hListProcess = GetDlgItem( IDC_LIST_NET_TRAFFIC );

		ASSERT( NULL != hEdit && 
			NULL != hCheckBlockRecv &&
			NULL != hCheckBlockSend );

#define WORK_THREAD_SYNC_TIME 100
		//for( ; ; )
		//{
		//	dwRet = WaitForSingleObject( proc_info_lock, WORK_THREAD_SYNC_TIME );
		//	if( WAIT_OBJECT_0 == dwRet || WAIT_ABANDONED == dwRet )
		//	{
		//		break;
		//	}

		//	//ASSERT( dwRet == WAIT_FAILED );
		//}

		memset( &ProcessControl, 0, sizeof( ProcessControl ) );

		nSelItem = ListView_GetSelectionMark( hListProcess );

		if( nSelItem < 0 )
		{
			ret = -1;
			goto _return;
		}

		memset( &lvItem, 0, sizeof( lvItem ) );

		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = nSelItem;
		ret = ListView_GetItem( hListProcess, &lvItem );

		if( FALSE == ret )
		{
			ASSERT( FALSE );
			ret = -1; 
			goto _return;
		}

		dwProcessId = lvItem.lParam;

		if( FALSE == flt_proc( dwProcessId ) )
		{
			goto _return;
		}

		ProcessControl.bRemove = FALSE;

		if( BST_CHECKED == SendMessage( hCheckBlockRecv, BM_GETCHECK, 0, 0 ) )
		{
			ProcessControl.bStopRecv = TRUE;
		}
		else
		{
			ProcessControl.bStopRecv = FALSE;
		}

		if( BST_CHECKED == SendMessage( hCheckBlockSend, BM_GETCHECK, 0, 0 ) )
		{
			ProcessControl.bStopSend = TRUE;
		}
		else
		{
			ProcessControl.bStopSend = FALSE;
		}

		::GetWindowText( hEdit, szText, sizeof( szText ) );
		dwSendingSpeed = atoi( szText );

		if( 1024 > dwSendingSpeed && 0 < dwSendingSpeed )
		{
			dwSendingSpeed = 1024;
		}

		ProcessControl.SendingSpeed.QuadPart = dwSendingSpeed; 
		ret = get_proc_img_path( dwProcessId, 
			ProcessControl.szImageFileName, 
			sizeof( ProcessControl.szImageFileName ), 
			ProcessControl.szNativeImageFileName, 
			sizeof( ProcessControl.szNativeImageFileName )
			);

		if( FALSE == ret )
		{
			ret = -1; 
			goto _return;
		}

		ret = do_ui_ctrl( NET_MON_UI_ID, 
		NET_MON_SET_PROC_TRAFFIC, 
		( PBYTE )&ProcessControl, 
		sizeof( PROCESS_INFORMATION_RECORD ), 
		NULL, 
		0, 
		this ); 
_return:
		return ret;
	}

	INT32 trace_proc_net_traffic( HWND hMainDlg )
	{
		INT32 ret;
		INT32 nSelItem;
		LV_ITEMW lvItem;
		DWORD dwProcessId;
		DWORD dwRet;
		HWND hEdit;
		HWND hListProcess;
		HWND hCheckBlockSend;
		HWND hCheckBlockRecv;
		FLT_SETTINGS setting; 

		hEdit = GetDlgItem( IDC_EDIT_SENDING_SPEED );
		hListProcess = GetDlgItem( IDC_LIST_NET_TRAFFIC );

		ASSERT( NULL != hEdit ); 

#define WORK_THREAD_SYNC_TIME 100
		//for( ; ; )
		//{
		//	dwRet = WaitForSingleObject( proc_info_lock, WORK_THREAD_SYNC_TIME );
		//	if( WAIT_OBJECT_0 == dwRet || WAIT_ABANDONED == dwRet )
		//	{
		//		break;
		//	}

		//	//ASSERT( dwRet == WAIT_FAILED );
		//}

		nSelItem = ListView_GetSelectionMark( hListProcess );

		if( nSelItem < 0 )
		{
			ret = -1;
			goto _return;
		}

		memset( &lvItem, 0, sizeof( lvItem ) );

		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = nSelItem;
		ret = ListView_GetItem( hListProcess, &lvItem );

		if( FALSE == ret )
		{
			ASSERT( FALSE );
			ret = -1; 
			goto _return;
		}

		dwProcessId = lvItem.lParam;

		//if( FALSE == flt_proc( dwProcessId ) )
		//{
		//	goto _return;
		//}

		//ret = get_proc_img_path( dwProcessId, 
		//	ProcessControl.szImageFileName, 
		//	sizeof( ProcessControl.szImageFileName ), 
		//	ProcessControl.szNativeImageFileName, 
		//	sizeof( ProcessControl.szNativeImageFileName )
		//	);

		//if( FALSE == ret )
		//{
		//	ret = -1; 
		//	goto _return;
		//}

		setting.flt_lvl = detail_flt; 
		setting.proc_id = dwProcessId; 
		setting.thread_id = THREAD_ID_NONE; 
		setting.proc_name[ 0 ] = '\0'; 

		ret = do_ui_ctrl( NET_MON_UI_ID, 
			NET_MON_TRACE_PROC_TRAFFIC, 
			( PBYTE )&setting, 
			sizeof( FLT_SETTINGS ), 
			NULL, 
			0, 
			this ); 
_return:
		return ret;
	}

	INT32 delete_all_list_item( HWND hList, DWORD dwItemIndex )
	{
		INT32 i;
		INT32 j;
		INT32 nItemCount;
		LV_ITEM lvItem;

		nItemCount = ListView_GetItemCount( hList );

		for( i = dwItemIndex; i < nItemCount; i ++ )
		{
			ListView_DeleteItem( hList, dwItemIndex );
		}

		return TRUE;
	}

	INT32 initialize_ui( HWND hWnd )
	{
		INT32 ret;
		INT32 i;
		HWND ListProcessInfo;
		LVCOLUMN lvc;
		//HICON hMainIcon;
		//HICON hSmallIcon;
		HMENU hMenu;

		proc_info_lock = CreateMutex( NULL, FALSE, NULL );

		if( NULL == proc_info_lock )
		{
			return -1;
		}

		CHAR *columns_title[] = { "Process ID", 
			"Exe File", 
			"All UpLoaded", 
			"All DownLoaded",
			"Up/Down Blocking", 
			"UpLoad Speed Limit", 
			"UpLoad Speed", 
			"DownLoad Speed" 
		};

		DWORD columns_len[] = { 
			100, 
				350, 
				100, 
				100,
				120,  
				90, 
				90, 
				90
		};

		ASSERT( NULL != hWnd );

		hMenu = ::GetSystemMenu( hWnd, FALSE );

		//ASSERT( NULL != hMainIcon && 
			//NULL != hSmallIcon );

		//SendMessage( hWnd, WM_SETICON, TRUE, (LPARAM)hMainIcon );
		//SendMessage( hWnd, WM_SETICON, FALSE, (LPARAM)hSmallIcon );
		ListProcessInfo = ( HWND )GetDlgItem( IDC_LIST_NET_TRAFFIC );

		SendMessage( ListProcessInfo, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
			LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP );

		for( ; ; )
		{
			ret = ListView_DeleteColumn( ListProcessInfo, 1 );
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

			if (ListView_InsertColumn( ListProcessInfo, i, &lvc ) == -1) 
				return -1; 
		}

		return 0;
	}

	LRESULT on_show_wnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT on_list_notify( INT32 param, NMHDR *pnmhdr, BOOL &handled)
	{
		INT32 ret;
		DWORD dwNotifyCode;
		INT32 nSelItem;
		LV_ITEM lvItem;
		HWND hEdit;
		HWND hCheckRecv;
		HWND hCheckSend;
		CHAR szText[ MAX_PATH ];

		handled = TRUE; 

		hEdit = GetDlgItem( IDC_EDIT_SENDING_SPEED );
		hCheckRecv = GetDlgItem( IDC_CHECK_BLOCK_RECEIVE );
		hCheckSend = GetDlgItem( IDC_CHECK_BLOCK_SEND );

		ASSERT( NULL != hEdit &&
			NULL != hCheckRecv &&
			NULL != hCheckSend );

		dwNotifyCode = pnmhdr->code;

		nSelItem = ListView_GetSelectionMark( pnmhdr->hwndFrom );

		if( nSelItem < 0 )
		{
			return FALSE;
		}

		memset( &lvItem, 0, sizeof( lvItem ) );

		switch( dwNotifyCode )
		{
		case NM_CLICK:
			{
				lvItem.iSubItem = 0;
				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = nSelItem;
				ret = ListView_GetItem( pnmhdr->hwndFrom, &lvItem );
				if( FALSE == ret )
				{
					return FALSE;
				}

				if( FALSE == flt_proc( lvItem.lParam ) )
				{
					return FALSE;
				}

				lvItem.iSubItem = 5;
				lvItem.mask = LVIF_TEXT;
				lvItem.iItem = nSelItem;
				lvItem.pszText = szText;
				lvItem.cchTextMax = sizeof( szText );
				ret = ListView_GetItem( pnmhdr->hwndFrom, &lvItem );
				if( FALSE == ret )
				{
					return FALSE;
				}

				::SetWindowText( hEdit, lvItem.pszText );

				lvItem.iSubItem = 4;
				lvItem.mask = LVIF_TEXT;
				lvItem.iItem = nSelItem;
				ret = ListView_GetItem( pnmhdr->hwndFrom, &lvItem );
				if( FALSE == ret )
				{
					return FALSE;
				}

				if( NULL != strstr( lvItem.pszText, "BlkUp" ) )
				{
					SendMessage( hCheckSend, BM_SETCHECK, BST_CHECKED, 0 );
				}
				else
				{
					SendMessage( hCheckSend, BM_SETCHECK, BST_UNCHECKED, 0 );
				}

				if( NULL != strstr( lvItem.pszText, "BlkDown" ) )
				{
					SendMessage( hCheckRecv, BM_SETCHECK, BST_CHECKED, 0 );
				}
				else
				{
					SendMessage( hCheckRecv, BM_SETCHECK, BST_UNCHECKED, 0 );
				}

				break;
			}
		case NM_RCLICK:
			{
				if( 0 != show_popup_menu( m_hWnd, IDR_POPUP_TRACE_DATA_FLOW ) )
				{
					return FALSE; 
				}

				trace_proc_net_traffic( m_hWnd ); 
				break; 
			}
		case LVN_BEGINLABELEDIT:
			{
				break;
			}
		case LVN_ENDLABELEDIT:
			{
				break;
			}
		default:
			{
				break;
			}
		}

		return FALSE;
	}

	//HBRUSH hBakBr;
	LRESULT on_init_dlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 ret; 

		bHandled = TRUE; 
		ret = initialize_ui( m_hWnd ); 

		if( ret < 0 )
		{
			goto _err_return; 
		}

		ret = do_ui_ctrl( NET_MON_UI_ID, NET_MON_INIT, NULL, 0, NULL, 0, this ); 
		
		if( ret < 0 )
		{
			
			goto _err_return; 
		}

		return TRUE; 

_err_return:
		DestroyWindow(); 
		return 1;  // Let the system set the focus
	}

	LRESULT on_paint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		bHandled = FALSE;
		//bHandled = TRUE;
		return 0;
	}

	LRESULT on_ok_bn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		INT32 ret; 
		// TODO: �ڴ����ӿؼ�֪ͨ�����������
		for( ; ; )
		{
			DWORD wait_ret; 
			wait_ret = WaitForSingleObject( proc_info_lock, WORK_THREAD_SYNC_TIME );
			if( WAIT_OBJECT_0 == wait_ret || WAIT_ABANDONED == wait_ret )
			{
				break;
			}

			//ASSERT( wait_ret == WAIT_FAILED );
		}

		ret = ctrl_proc_net_traffic( m_hWnd ); 

		ReleaseMutex( proc_info_lock );
		return ( LRESULT )ret; 
	}

protected:
	HANDLE proc_info_lock; 

	//CXmlNode* m_pMainMenu;
	//CXmlNode* m_pRootNode;
	//CXml* m_pXmlDoc;
	//CImgListBoxImpl m_ListBox;
	////CBmpStaticImpl m_MenuStc;
	//CButtonXP m_BtnOK;
	//CButtonXP m_BtnCancel;
	//_bstr_t m_strUrl;
	//_bstr_t m_strDesc;
	//_bstr_t m_strCaption;
	////_bstr_t m_strImage;
	//_bstr_t m_strFilter;
};

#endif //_NET_MON_TAB_H__