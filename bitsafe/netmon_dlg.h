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
//#include <objbase.h>
#include <zmouse.h>
//#include <exdisp.h>
#include <vector>
//#include <sstream>
//#include "menu_ui.h"

#include "tdifw_api.h"
#include "trace_log_api.h"
#include "msg_box.h"

#ifdef _UNICODE
typedef std::wstring record_string; 
#else
typedef std::string record_string; 
#endif //_UNICODE

typedef enum _net_block_state
{
	PROC_NET_BLOCK, 
	PROC_NET_START
} net_block_state; 

typedef struct _network_info
{
	ULONG proc_id;
	record_string proc_exe_file;
	ULONG proc_download;
	ULONG proc_upload;
	net_block_state up_blk_state; 
	net_block_state down_blk_state; 
	ULONG proc_up_speed;
	ULONG proc_down_speed;
	ULONG proc_up_speed_limit;
} network_info, *pnetwork_info; 

extern std::vector< network_info > all_network_info; 

LRESULT net_mon_ui_output( ULONG ui_action_id, 
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

#define SENDING_SPEED_STRING_LEN 20
typedef enum _proc_net_ctrl_mode
{
	STOP_PROC_NET, 
	START_PROC_NET, 
	CTRL_PROC_UP_SPEED 
} proc_net_ctrl_mode, *pproc_net_ctrl_mode; 


//CHAR *columns_title[] = { "Process ID", 
//"Exe File", 
//"All UpLoaded", 
//"All DownLoaded",
//"Up/Down Blocking", 
//"UpLoad Speed Limit", 
//"UpLoad Speed", 
//"DownLoad Speed" 
//};

class netmon_dlg : public CWindowWnd, public INotifyUI, public IListCallbackUI, public action_ui_notify
{
public:
	netmon_dlg() : m_pCloseBtn( NULL ), 
		//m_pMaxBtn( NULL ), 
		m_pRestoreBtn( NULL ), 
		m_pMinBtn( NULL ), 
		net_info_list( NULL ), 
		speed_edit( NULL )
	{
	};

	INT32 update_traffic_sum( PBYTE output_buf, ULONG output_length, PVOID param )
	{
		INT32 ret;  
		PULARGE_INTEGER all_traffic; 
		CHAR output[ 256 ]; 

		all_traffic = ( PULARGE_INTEGER )output_buf; 

#define CHINESE_NET_DONW_UP_TIP_FMT "共上载: %d%d, 共下载: %d%d\n"
#define ENGLISH_NET_DOWN_UP_TIP_FMT "Uploaded: %d%d, Downloaded: %d%d\n"
		
		sprintf( output, CHINESE_NET_DONW_UP_TIP_FMT, 
			all_traffic[ 1 ].LowPart, all_traffic[ 1 ].HighPart, 
			all_traffic[ 0 ].LowPart, all_traffic[ 0 ].HighPart 
			);

		return 0; 
	}

	LRESULT action_notify( action_ui_notify_type type, PVOID param )
	{
		LRESULT ret = ERROR_SUCCESS; 
		notify_data *data_notify; 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
		ASSERT( param != NULL ); 

		data_notify = ( notify_data* )param; 

		if( type == PROCESS_NET_DATA_COUNT )
		{
			ret = update_all_proc_traffic( ( PBYTE )data_notify->data, data_notify->data_len, NULL ); 
		}

		log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
		return ret; 
	}

#define MAX_DESC_INFO_LEN 256

	INT32 update_all_proc_traffic( PBYTE output_buf, ULONG output_length, PVOID param )
	{
		INT32 ret;
		DWORD dwItemIndex;
		//HWND hDlg;
		PUI_CTRL ui_ctrl; 

		WCHAR szFileName[ MAX_PATH ];
		WCHAR szNativeFileName[ MAX_IMAGE_FILE_PATH ];

		PROCESS_TRAFFIC_OUTPUT *pProcessIo;
		LARGE_INTEGER all_traffic[ 2 ];
		CHAR szOutputMsg[ 512 ];
		INT32 i;

		dwItemIndex = 0;

		log_trace( ( MSG_INFO, "enter %s data len is %d, unit len is %d\n", __FUNCTION__, output_length, sizeof( PROCESS_TRAFFIC_OUTPUT ) ) ); 

		pProcessIo = ( PROCESS_TRAFFIC_OUTPUT* )output_buf; 

		for( i = 0; ( ULONG )i < output_length / sizeof( PROCESS_TRAFFIC_OUTPUT ); i ++ )
		{
			DWORD wait_ret; 
			sprintf( szOutputMsg, "NetMgr Process Id: %d \n", // Sending Speed: %d\n", 
				pProcessIo[ i ].proc_id );
			//pProcessIo[ i ].SendingSpeed.LowPart );
			//DebugTrace( szOutputMsg );

			if( SYSTEM_SYSTEM_PROCESS_ID == pProcessIo[ i ].proc_id )
			{
				wcscpy( szFileName, L"System" );
			}
			else if( SYSTEM_IDLE_PROCESS_ID == pProcessIo[ i ].proc_id )
			{
				wcscpy( szFileName, L"System Idle Process" );
			}
			else if( FALSE == get_proc_img_path( pProcessIo[ i ].proc_id, szFileName, sizeof( szFileName ), NULL, 0 ) )
			{
				sprintf( szOutputMsg, "Get process %d image path failed\n", 
					pProcessIo[ i ].proc_id );

				//DebugTrace( szOutputMsg );
				continue;
			}

			wait_ret = WaitForSingleObject( proc_info_lock, INFINITE );
			if( WAIT_OBJECT_0 != wait_ret && WAIT_ABANDONED != wait_ret )
			{
				ASSERT( FALSE ); 
			}

			add_proc_info_to_list( dwItemIndex ++, 
				pProcessIo[ i ].proc_id, 
				pProcessIo[ i ].all_sended_data.LowPart, 
				pProcessIo[ i ].all_recved_data.LowPart, 
				pProcessIo[ i ].stop_send, 
				pProcessIo[ i ].stop_recv, 
				pProcessIo[ i ].send_speed_ctrl.LowPart, 
				pProcessIo[ i ].recv_speed.LowPart, 
				pProcessIo[ i ].send_speed.LowPart, 
				szFileName 
				);

			ReleaseMutex( proc_info_lock );
		}

		return 0; 
	}

	INT32 add_proc_info_to_list( DWORD item_index, 
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
		INT32 ret = 0;
		TCHAR desc[ MAX_DESC_INFO_LEN ];
		DWORD out_str_len;
		INT32 item_count;
		network_info proc_net_info; 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		_sntprintf( desc, MAX_DESC_INFO_LEN, _T( "%u" ), dwProcessId );

		if( dwUpLoadSpeedControl == 0xFFFFFFFF )
		{
			dwUpLoadSpeedControl = 0;
		}

		proc_net_info.up_blk_state = bBlockSend == TRUE ? PROC_NET_BLOCK : PROC_NET_START; 
		proc_net_info.down_blk_state = bBlockRecv == TRUE ? PROC_NET_BLOCK : PROC_NET_START; 

		proc_net_info.proc_down_speed = dwRecvingSpeed; 
		proc_net_info.proc_up_speed = dwSendingSpeed; 
		proc_net_info.proc_up_speed_limit = dwUpLoadSpeedControl; 
		proc_net_info.proc_download = dwAllDownLoad; 
		proc_net_info.proc_upload = dwAllUpLoad; 
		proc_net_info.proc_exe_file = lpwszProcessImagePath; 
		proc_net_info.proc_id = dwProcessId; 

		item_count = all_network_info.size(); 

		if( ( ULONG )item_index >= item_count )
		{
			all_network_info.push_back( proc_net_info ); 
		}
		else
		{
			all_network_info[ item_index ] = proc_net_info; 
		}

		if( ( ULONG )item_index >= item_count )
		{
			ASSERT( net_info_list != NULL ); 

			//CListUI* info_list = static_cast<CListUI*>(m_pm.FindControl(_T("netmon_list"))); 
			//ASSERT( info_list != NULL ); 

			ret = add_list_item( net_info_list, item_index ); 
			if( ret != ERROR_SUCCESS )
			{
				goto _return;
			}
		}

_return:
		return ret;
	}

	INT32 trace_proc_net_traffic( HWND hMainDlg )
	{
		INT32 ret;
		INT32 nSelItem;
		DWORD dwProcessId;
		DWORD dwRet;
		HWND hEdit;
		HWND hListProcess;
		HWND hCheckBlockSend;
		HWND hCheckBlockRecv;
		FLT_SETTINGS setting; 

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

		//dwProcessId = lvItem.lParam;

		//if( FALSE == flt_proc( dwProcessId ) )
		//{
		//	goto _return;
		//}

		//ret = get_proc_img_path( dwProcessId, 
		//	proc_control.szImageFileName, 
		//	sizeof( proc_control.szImageFileName ), 
		//	proc_control.szNativeImageFileName, 
		//	sizeof( proc_control.szNativeImageFileName )
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

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			NET_MON_TRACE_PROC_TRAFFIC, 
			( PBYTE )&setting, 
			sizeof( FLT_SETTINGS ), 
			NULL, 
			0, 
			NULL, 
			( action_ui_notify* )this ); 
_return:
		return ret;
	}

	INT32 delete_all_list_item()
	{
		//CListUI *list; 
		all_network_info.clear();
		
		//list = ( CListUI* )m_pm.FindControl( _T( "netmon_list" ) ); 
		//ASSERT( list != NULL ); 

		net_info_list->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID ); 

		return TRUE;
	}

	INT32 initialize_ui()
	{
		INT32 ret = ERROR_SUCCESS;
		//INT32 i;

		proc_info_lock = CreateMutex( NULL, FALSE, NULL );

		if( NULL == proc_info_lock )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		//DWORD columns_len[] = { 
		//	100, 
		//		350, 
		//		100, 
		//		100,
		//		120,  
		//		90, 
		//		90, 
		//		90
		//}; 

_return:
		return ret;
	}

	LRESULT ctrl_proc_net_traffic( proc_net_ctrl_mode mode )
	{
		LRESULT ret = ERROR_SUCCESS;
		INT32 select_item;
		DWORD proc_id;
		DWORD send_speed;
		PROCESS_TRAFFIC_CTRL proc_control; 
		LPCTSTR tmp_text; 
		//CHAR szText[ SENDING_SPEED_STRING_LEN ];

#ifdef WINDOW_MODE
		LV_ITEMW lvItem;
		HANDLE hProcess;
		DWORD dwRet;
#endif //WINDOW_MODE

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

		ASSERT( net_info_list != NULL ); 

		select_item = net_info_list->GetCurSel(); 
		if( select_item < 0 )
		{
			ret = ERROR_NOT_READY; 
			tmp_text = _get_string_by_id( TEXT_NETWORK_CONN_SELECT_TARGET_TIP, 
				_T( "请选中需要控制流量的进程" ) ); 

			show_msg( GetHWND(), tmp_text, NULL, _T( "注意" ), 0 ); 
			goto _return; 
		}

		ASSERT( select_item < all_network_info.size() ); 

		proc_id = all_network_info[ select_item ].proc_id; 

		if( FALSE == flt_proc( proc_id ) )
		{
			goto _return;
		}

		memset( &proc_control, 0, sizeof( proc_control
			) );

		proc_control.remove_op = FALSE;

		//{
		//	proc_control.bStopRecv = TRUE;
		//}

		ret = get_proc_img_path( proc_id, 
			proc_control.image_file, 
			sizeof( proc_control.image_file ), 
			proc_control.native_image_file, 
			sizeof( proc_control.native_image_file )
			);

		if( FALSE == ret )
		{
			ret = -1; 
			goto _return;
		}

		proc_control.send_speed_ctrl.QuadPart = 0; 

		proc_control.stop_recv = FALSE; 
		proc_control.stop_send = FALSE; 

		switch( mode )
		{
		case STOP_PROC_NET:
			{
				proc_control.stop_send = TRUE; 
				proc_control.stop_recv = TRUE;
			}
			break; 
		case START_PROC_NET:
			{
				//proc_control.bStopSend = FALSE; 
				//proc_control.bStopRecv = FALSE;
			}
			break; 
		case CTRL_PROC_UP_SPEED:
			{
				CStdString text; 
				TCHAR *tmp; 

				ASSERT( speed_edit != NULL ); 

				text = speed_edit->GetText(); 

				if( _tcsdigit( text.GetData() ) == FALSE )
				{
					ret = ERROR_INVALID_PARAMETER; 
					goto _return; 
				}

				send_speed = _tcstoul( text.GetData(), &tmp, 10 ); 

#define SPEED_MIN_LIMIT 1024
				if( SPEED_MIN_LIMIT > send_speed && 0 < send_speed )
				{
					send_speed = SPEED_MIN_LIMIT;
				}

				//if( BST_CHECKED == SendMessage( hCheckBlockSend, BM_GETCHECK, 0, 0 ) )
				proc_control.send_speed_ctrl.QuadPart = send_speed; 
			}
			break; 
		default:
			ASSERT( FALSE && "invalid process network control mode" ); 
			break; 
		}

		ret = do_ui_ctrl( BITSAFE_UI_ID, 
			NET_MON_SET_PROC_TRAFFIC, 
			( PBYTE )&proc_control, 
			sizeof( PROCESS_TRAFFIC_CTRL ), 
			NULL, 
			0, 
			NULL, 
			( action_ui_notify* )this ); 
_return:
		return ret;
	}

	LRESULT on_list_notify( INT32 param, NMHDR *pnmhdr, BOOL &handled)
	{
		LRESULT ret;
		DWORD dwNotifyCode;
		//INT32 nSelItem;
		//LV_ITEM lvItem;
		//HWND hEdit;
		//HWND hCheckRecv;
		//HWND hCheckSend;
		//CHAR szText[ MAX_PATH ];

		handled = TRUE; 

		//hEdit = GetDlgItem( IDC_EDIT_SENDING_SPEED );
		//hCheckRecv = GetDlgItem( IDC_CHECK_BLOCK_RECEIVE );
		//hCheckSend = GetDlgItem( IDC_CHECK_BLOCK_SEND );

		//ASSERT( NULL != hEdit &&
		//	NULL != hCheckRecv &&
		//	NULL != hCheckSend );

		//dwNotifyCode = pnmhdr->code;

		//nSelItem = ListView_GetSelectionMark( pnmhdr->hwndFrom );

		//if( nSelItem < 0 )
		//{
		//	return FALSE;
		//}

		//memset( &lvItem, 0, sizeof( lvItem ) );

		switch( dwNotifyCode )
		{
		case NM_CLICK:
			{
				//lvItem.iSubItem = 0;
				//lvItem.mask = LVIF_PARAM;
				//lvItem.iItem = nSelItem;
				//ret = ListView_GetItem( pnmhdr->hwndFrom, &lvItem );
				//if( FALSE == ret )
				//{
				//	return FALSE;
				//}

				//if( FALSE == flt_proc( lvItem.lParam ) )
				//{
				//	return FALSE;
				//}

				//lvItem.iSubItem = 5;
				//lvItem.mask = LVIF_TEXT;
				//lvItem.iItem = nSelItem;
				//lvItem.pszText = szText;
				//lvItem.cchTextMax = sizeof( szText );
				//ret = ListView_GetItem( pnmhdr->hwndFrom, &lvItem );
				//if( FALSE == ret )
				//{
				//	return FALSE;
				//}

				//::SetWindowText( hEdit, lvItem.pszText );

				//lvItem.iSubItem = 4;
				//lvItem.mask = LVIF_TEXT;
				//lvItem.iItem = nSelItem;
				//ret = ListView_GetItem( pnmhdr->hwndFrom, &lvItem );
				//if( FALSE == ret )
				//{
				//	return FALSE;
				//}

				//if( NULL != strstr( lvItem.pszText, "BlkUp" ) )
				//{
				//	SendMessage( hCheckSend, BM_SETCHECK, BST_CHECKED, 0 );
				//}
				//else
				//{
				//	SendMessage( hCheckSend, BM_SETCHECK, BST_UNCHECKED, 0 );
				//}

				//if( NULL != strstr( lvItem.pszText, "BlkDown" ) )
				//{
				//	SendMessage( hCheckRecv, BM_SETCHECK, BST_CHECKED, 0 );
				//}
				//else
				//{
				//	SendMessage( hCheckRecv, BM_SETCHECK, BST_UNCHECKED, 0 );
				//}

				break;
			}
		case NM_RCLICK:
			{
				//if( 0 != show_popup_menu( m_hWnd, IDR_POPUP_TRACE_DATA_FLOW ) )
				//{
				//	return FALSE; 
				//}

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

		return ERROR_ERRORS_ENCOUNTERED;
	}

	LRESULT on_init_dlg()
	{
		LRESULT ret = ERROR_SUCCESS; 
		INT32 i; 
		CListUI* info_list = static_cast<CListUI*>(m_pm.FindControl( _T( "netmon_list" ) ) ); 
		ASSERT( info_list != NULL ); 

		for( i = 0; ( ULONG )i < all_network_info.size(); i ++ )
		{
			add_list_item( info_list, i ); 
		}

		ret = initialize_ui(); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		ret = do_ui_ctrl( BITSAFE_UI_ID, NET_MON_INIT, NULL, 0, NULL, 0, NULL, ( action_ui_notify* )this ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

_return:
		if( ret != ERROR_SUCCESS )
		{
			Close(); 
		}
		return ret;  // Let the system set the focus
	}

	LRESULT on_ctrl_proc_net( proc_net_ctrl_mode mode )
	{
		LRESULT ret; 
		// TODO: 在此添加控件通知处理程序代码
		for( ; ; )
		{
			DWORD wait_ret; 
			wait_ret = WaitForSingleObject( proc_info_lock, WORK_THREAD_SYNC_TIME );
			if( WAIT_OBJECT_0 == wait_ret || WAIT_ABANDONED == wait_ret )
			{
				break;
			}

			ASSERT( wait_ret == WAIT_FAILED || wait_ret == WAIT_TIMEOUT ); 
		}

		ret = ctrl_proc_net_traffic( mode ); 

		ReleaseMutex( proc_info_lock );
		return ret; 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("NetmonDlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

	void add_arp_info()
	{
	}

    void Init() 
    {
		CListHeaderUI *header; 
		CControlUI *ctrl; 

        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
        //m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("max_btn")));
        //ASSERT( m_pMaxBtn != NULL ); 
		m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("nor_btn")));
        ASSERT( m_pRestoreBtn != NULL ); 
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("min_btn")));
		ASSERT( m_pMinBtn != NULL ); 

		net_info_list  = static_cast<CListUI*>(m_pm.FindControl( _T( "netmon_list" ) ) );
		ASSERT( net_info_list != NULL ); 
		
		header = net_info_list->GetHeader(); 
		ASSERT( header != NULL ); 

		ctrl = header->GetItemAt( 0 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_NETWORK_CONN_PROC_ID ); 

		ctrl = header->GetItemAt( 1 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_NETWORK_CONN_MODULE_FILE ); 

		ctrl = header->GetItemAt( 2 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_NETWORK_CONN_ALL_UPLOAD ); 
		
		ctrl = header->GetItemAt( 3 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_NETWORK_CONN_ALL_DWONLOAD ); 
		
		ctrl = header->GetItemAt( 4 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_NETWORK_CONN_BLOCK_STATUS ); 
		
		ctrl = header->GetItemAt( 5 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_NETWORK_CONN_UPLOAD_SPEED ); 
		
		ctrl = header->GetItemAt( 6 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_NETWORK_CONN_UPLOAD_SPEED_LIMIT ); 
		
		ctrl = header->GetItemAt( 7 ); 
		ASSERT( ctrl != NULL ); 
		_set_ctrl_text( ctrl, TEXT_NETWORK_CONN_DOWNLOAD_SPEED ); 

		set_ctrl_text( &m_pm, _T( "stop_btn" ), TEXT_RESPONSE_BLOCK ); 
		set_ctrl_text( &m_pm, _T( "start_btn" ), TEXT_RESPONSE_ALLOW ); 
		set_ctrl_text( &m_pm, _T( "speed_btn" ), TEXT_NETWORK_CONN_UPLOAD_SPEED_CTRL ); 

		speed_edit = static_cast< CEditUI* >( m_pm.FindControl( _T( "speed_edit" ) ) );  
		ASSERT( speed_edit != NULL ); 

		net_info_list->RemoveSubAllItem( UI_LIST_CONTAINER_ELEMENT_CLASS_ID );

		net_info_list->SetTextCallback( this ); 

		on_init_dlg(); 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }
 
    /*
    * 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
    */
    LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
    {
        TCHAR szBuf[MAX_PATH] = {0};
		network_info *info; 
		LPCTSTR tmp_text; 

		ASSERT( iIndex < all_network_info.size() ); 

		info = &all_network_info[ iIndex ]; 

        switch( iSubItem )
        {
		case 0:
			_sntprintf( szBuf, MAX_PATH, _T( "%u" ), info->proc_id ); 
			break;
		case 1:
			_tcsncpy( szBuf, info->proc_exe_file.c_str(), MAX_PATH ); 
			break; 
		case 2:
			_sntprintf( szBuf, MAX_PATH, _T( "%u" ), info->proc_upload ); 
			break; 
		case 3:
			_sntprintf( szBuf, MAX_PATH, _T( "%u" ), info->proc_download ); 
			break; 
		case 4:
			{
				*szBuf = _T( '\0' ); 

				if( info->up_blk_state == PROC_NET_BLOCK )
				{
					//_T( "BlkUp/" )
					tmp_text = _get_string_by_id( TEXT_NETWORK_CONN_BLOCK_UP, 
						_T( "阻止上传/" ) ); 
					_tcsncat( szBuf, tmp_text, MAX_DESC_INFO_LEN - _tcslen( szBuf ) ); 
				}
				else
				{
					tmp_text = _get_string_by_id( TEXT_NETWORK_CONN_ALLOW_UP, 
						_T( "允许上传/" ) ); 
					_tcsncat( szBuf, tmp_text, MAX_DESC_INFO_LEN - _tcslen( szBuf ) ); 
				}

				if( info->down_blk_state == PROC_NET_BLOCK )
				{
					tmp_text = _get_string_by_id( TEXT_NETWORK_CONN_BLOCK_DOWN, 
						_T( "阻止下载" ) ); 
					_tcsncat( szBuf, tmp_text, MAX_DESC_INFO_LEN - _tcslen( szBuf ) ); 
				}
				else
				{
					tmp_text = _get_string_by_id( TEXT_NETWORK_CONN_ALLOW_DOWN, 
						_T( "允许下载" ) ); 
					_tcsncat( szBuf, tmp_text, MAX_DESC_INFO_LEN - _tcslen( szBuf ) ); 
				}

				//_tcscpy(szBuf, proc_blk_state[ iIndex ].c_str() ); 

			}
			break; 
		case 5:
			_sntprintf( szBuf, MAX_PATH, _T( "%u" ), info->proc_up_speed ); 
			break; 
		case 6:
			_sntprintf( szBuf, MAX_PATH, _T( "%u" ), info->proc_up_speed_limit ); 
			break; 
		case 7:
			_sntprintf( szBuf, MAX_PATH, _T( "%u" ), info->proc_down_speed ); 
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
			//{
//#ifdef _UNICODE		
//				int iLen = proc_ids[iIndex].length();
//				LPWSTR lpText = new WCHAR[iLen + 1];
//				::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
//				::MultiByteToWideChar(CP_ACP, 0, proc_ids[iIndex].c_str(), -1, (LPWSTR)lpText, iLen) ;
//				_stprintf(szBuf, lpText);
//				delete[] lpText;
//#else
				//_stprintf(szBuf, proc_ids[iIndex].c_str());
//#endif
		//	}
		//	break;
		//case 0:
		//	{
//#ifdef _UNICODE		
//				int iLen = proc_ids[iIndex].length();
//				LPWSTR lpText = new WCHAR[iLen + 1];
//				::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
//				::MultiByteToWideChar(CP_ACP, 0, proc_ids[iIndex].c_str(), -1, (LPWSTR)lpText, iLen) ;
//				_stprintf(szBuf, lpText);
//				delete[] lpText;
//#else
				//_stprintf(szBuf, proc_ids[iIndex].c_str());
//#endif
			//}
            //break;
        }
        pControl->SetUserData(szBuf);
        return pControl->GetUserData();
    }

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("windowinit") ) 
            OnPrepare( msg );
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
			else
			{
				CStdString name; 

				name = msg.pSender->GetName(); 

				if( name == _T( "stop_btn" ) )
				{
					on_ctrl_proc_net( STOP_PROC_NET ); 
				}
				else if( name == _T( "start_btn" ) )
				{
					on_ctrl_proc_net( START_PROC_NET ); 
				}
				else if( name == _T( "speed_btn" ) )
				{
					ASSERT( speed_edit != NULL ); 

					on_ctrl_proc_net( CTRL_PROC_UP_SPEED ); 
					speed_edit->SetText( _T( "0" ) ); 
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
        //else if(msg.sType == _T("menu")) 
        //{
        //    if( msg.pSender->GetName() != _T("netmon_list") ) return;
        //    menu_ui* pMenu = new menu_ui();
        //    if( pMenu == NULL ) { return; }
        //    POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
        //    ::ClientToScreen(*this, &pt);
        //    pMenu->Init(msg.pSender, pt);
        //}
        //else if( msg.sType == _T("menu_Delete") ) {
        //    CListUI* pList = static_cast<CListUI*>(msg.pSender);
        //    int nSel = pList->GetCurSel();
        //    if( nSel < 0 ) return;
        //    pList->RemoveAt(nSel);
        //    proc_ids.erase(proc_ids.begin() + nSel);
        //    proc_ids.erase(proc_ids.begin() + nSel);   
        //}
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("netmon_dlg.xml"), (UINT)0, NULL, &m_pm);
        ASSERT(pRoot && "Failed to parse XML");
        m_pm.AttachDialog(pRoot);
        m_pm.AddNotifier(this); 
		set_ui_style( &m_pm ); 
        Init();
        return 0;
    }

    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		LRESULT ret; 
        bHandled = FALSE;
		//ret = do_ui_ctrl( BITSAFE_UI_ID, NET_MON_UNINIT, NULL, 0, NULL, 0, NULL, NULL ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	log_trace( ( MSG_ERROR, "unintialize the net monitor ui context failed \n" ) ); 
		//}
		return 0;
    }

	INT32 uninit_netmon_dlg()
	{
		INT32 ret = 0; 
		ret = do_ui_ctrl( BITSAFE_UI_ID, NET_MON_UNINIT, NULL, 0, NULL, 0, NULL, ( action_ui_notify* )this ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		CloseHandle( proc_info_lock ); 
_return:
		return ret; 
	}

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
		//m_pm.RemoveAllTimers(); 
		m_pm.RemoveNotifier(this);
		uninit_netmon_dlg(); 
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

		hide_sys_menu( GetHWND() ); 

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
    //CButtonUI* m_pMaxBtn;
    CButtonUI* m_pRestoreBtn;
    CButtonUI* m_pMinBtn;
	CListUI* net_info_list; 
	CEditUI *speed_edit; 

	HANDLE proc_info_lock; 
};