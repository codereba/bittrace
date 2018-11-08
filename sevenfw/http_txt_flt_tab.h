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

 
 #ifndef __HTTP_TEXT_FLT_TAB_H__
#define __HTTP_TEXT_FLT_TAB_H__

#include "http_flt_tab.h"

INT32 http_txt_flt_add_filter_text( CHAR *text, ULONG text_type, ULONG flags, PVOID param ); 

INT32 http_flt_txt_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param ); 

class http_txt_flt_tab :
	public CDialogImpl<http_txt_flt_tab, http_flt_tab>,
	public CWinDataExchange<http_txt_flt_tab> 
{
public:
	enum { IDD = IDD_7FW_HTTP_TXT_FLT_TAB };

	http_txt_flt_tab()
	{
	}

	BEGIN_MSG_MAP(http_txt_flt_tab)
		MESSAGE_HANDLER( WM_DESTROY, on_uninit )
		MESSAGE_HANDLER( WM_INITDIALOG, on_init_dlg )
		MESSAGE_HANDLER( WM_SHOWWINDOW, on_show_wnd )
		MESSAGE_HANDLER( WM_SYSCOMMAND, on_sys_cmd )
		MESSAGE_HANDLER( WM_CHAR, on_key_down )
		MESSAGE_HANDLER( WM_KEYDOWN, on_key_down ) 
		COMMAND_HANDLER(IDOK, BN_CLICKED, on_ok_bn )
		COMMAND_HANDLER( ID_DEL_FLT_URL, BN_CLICKED, on_del_http_flt_txt )

		//MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor )
		//COMMAND_HANDLER(IDC_STATIC_IMG_MENU, STN_CLICKED, OnNorImgClicked)
		NOTIFY_HANDLER(IDC_LIST_FILTER_TEXT, LBN_SELCHANGE, on_list_notify)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP(http_txt_flt_tab)
	END_DDX_MAP()

	INT32 add_flt_info_to_list( HWND ListControl, 
		DWORD item_index, 
		ULONG type, 
		LPSTR filter_text )
	{
		INT32 ret;
		LV_ITEM lv_item;
		LV_ITEM lv_item_insert;
		INT32 nItemCount;
		CHAR *type_desc; 

		if( type > MAX_TAG_INDEX )
		{
			//( "enter add_flt_info_to_list type invalid \n" ); 
			return -1;
		}

		type_desc = get_text_type_desc( type ); 
		if( *type_desc == '\0' )
		{
			return -1; 
		}

		memset( &lv_item, 0, sizeof( lv_item ) );
		lv_item.mask = LVIF_TEXT;

		lv_item.pszText = type_desc;
		lv_item.iItem = item_index;
		lv_item.iSubItem = 1;

		memset( &lv_item_insert, 0, sizeof( lv_item_insert ) );
		lv_item_insert.mask = LVIF_TEXT;

		lv_item_insert.pszText = filter_text;
		lv_item_insert.iItem = item_index;
		lv_item_insert.iSubItem = 0;

		nItemCount = ListView_GetItemCount( ListControl );

		if( item_index >= nItemCount )
		{
			ret = ListView_InsertItem( ListControl, &lv_item_insert );
			if( ret < 0 )
			{
				return -1; 
			}
			ret = ListView_SetItem( ListControl, &lv_item );
			if( ret < 0 )
			{
				return -1; 
			}

		}
		else
		{
			ret = ListView_SetItem( ListControl, &lv_item_insert );
			if( ret < 0 )
			{ 
				return -1; 
			}

			ret = ListView_SetItem( ListControl, &lv_item );
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

#define PARA_TEXT_DESC "段落文本"
#define LABEL_TEXT_DESC "标签文本"
#define TABLE_TEXT_DESC "表格文本"
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

	LRESULT on_del_http_flt_txt(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		HWND hList; 
		INT32 ret;
		INT32 nSelItem;
		LV_ITEM lvItem;
		CHAR text[ 256 ]; 
		PFILTER_NAME_INPUT filter_text = NULL; 
		DWORD ret_length; 

		hList = GetDlgItem( IDC_LIST_FILTER_TEXT ); 

		nSelItem = ListView_GetSelectionMark( hList );

		if( nSelItem < 0 )
		{
			return FALSE;
		}

		ret = TRUE; 
		memset( &lvItem, 0, sizeof( lvItem ) );

		lvItem.cchTextMax = 256; 
		lvItem.pszText = text; 
		lvItem.mask = LVIF_TEXT; 
		lvItem.iItem = nSelItem; 
		lvItem.iSubItem = 0; 

		ret = ListView_GetItem( hList, &lvItem ); 
		if( ret == FALSE )
		{
			return FALSE; 
		}

		filter_text = ( PFILTER_NAME_INPUT )malloc( sizeof( FILTER_NAME_INPUT ) + strlen( text ) + sizeof( CHAR ) ); 
		if( NULL == filter_text )
		{
			ret = FALSE; 
			goto _return; 
		}

		filter_text->length = strlen( text ) + sizeof( CHAR ); 
		memcpy( filter_text->text, text, filter_text->length ); 

		lvItem.iSubItem = 1; 
		ret = ListView_GetItem( hList, &lvItem ); 
		if( ret == FALSE )
		{
			ret = FALSE; 
			goto _return; 
		}

		filter_text->type = text_type_from_desc( text ); 
		if( filter_text->type < 0 )
		{
			ret = FALSE; 
			goto _return; 
		}

		ret = do_ui_ctrl( HTTP_TXT_FLT_UI_ID, 
			HTTP_TXT_FLT_DEL_URL, 
			( PBYTE )filter_text, 
			sizeof( FILTER_NAME_INPUT ) + strlen( text ) + sizeof( CHAR ), 
			NULL, 
			0, 
			this ); 

		sprintf( text, "删除 %s %s %s \n", 
			get_text_type_desc( filter_text->type ), 
			filter_text->text, 
			ret ? "成功" : "失败" ); 

		if( ret == TRUE )
		{
			ret = ListView_DeleteItem( hList, nSelItem ); 
			if( ret == FALSE )
			{
				goto _return; 
			}

			flt_txt_count --; 
		}

_return:
		if( NULL != filter_text )
		{
			free( filter_text ); 
		}
		return ret; 
	}

	INT32 ui_init()
	{
		CHAR output[ MAX_PATH ]; 
		INT32 ret;
		INT32 i;
		HWND ListFilterText;
		LVCOLUMN lvc;
		HICON hMainIcon;
		HICON hSmallIcon;
		HMENU hMenu;
		HWND hSingleSel;

		CHAR *columns_title[] = 
		{ 
			"URL", 
				"文本类型"
		};

		DWORD columns_len[] = { 
			400, 
				260
		};

		ListFilterText = ( HWND )GetDlgItem( IDC_LIST_FILTER_TEXT );

		SendMessage( ListFilterText, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
			LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP );

		for( ; ; )
		{
			ret = ListView_DeleteColumn( ListFilterText, 1 );
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

			if (ListView_InsertColumn( ListFilterText, i, &lvc ) == -1) 
				return -1; 
		}

		hSingleSel = GetDlgItem( IDC_RADIO_PARAGRAPH );

		if( NULL == hSingleSel )
		{
			ASSERT( FALSE );
			return -1;
		}

		if( 0 > ::SendMessage( hSingleSel, BM_SETCHECK, 1, 0 ) )
		{
			return -1;
		}

		//MessageBox( NULL, "LOAD CONFIG!", NULL, MB_OK ); 

		ret = load_filter_text_from_file( http_txt_flt_add_filter_text, this, FILTER_TEXT_BACKUP_FILE );
		if( ret < 0 )
		{
			if( ret == FORMAT_ERROR )
			{
				::MessageBox( NULL, "配置文件格式不正确!", NULL, MB_OK ); 
			}
			//return ret; 
		}

		//{
		//	WORD Version = 0x0202; 
		//	WSADATA WsaData;
		//	ret = WSAStartup( Version, &WsaData );
		//}
		return 0;
	}

	INT32 mng_flt_txt( INT32 Mode )
	{
		INT32 ret; 
		HWND hEdit;
		INT32 Id;
		CHAR InputNames[ MAX_NAMES_LEN ];
		ULONG ret_length; 
		USHORT text_type; 

		text_type = get_flt_txt_type(); 
		if( 0 > text_type )
		{
			return FALSE; 
		}

		hEdit = GetDlgItem( IDC_EDIT_FILTER_TEXT );

		if( NULL == hEdit )
		{
			ASSERT( NULL != hEdit );
			return FALSE;
		}

		::GetWindowText( hEdit, InputNames, MAX_NAMES_LEN );
		if( strlen( InputNames ) == 0 ) 
		{
			return FALSE;
		}

		ret = http_txt_flt_add_filter_text( InputNames, text_type, TRUE, this ); 

		::SetWindowText( hEdit, "" ); 
		if( FALSE == ret )
		{
			return ret; 
		}

		return ret; 
	}

	INT32 get_flt_txt_type()
	{
		HWND hSingleSel;

		return 0; 

		hSingleSel = GetDlgItem( IDC_RADIO_PARAGRAPH );
		if( NULL == hSingleSel )
		{
			ASSERT( FALSE );
			return -1;
		}

		if( 0 < ::SendMessage( hSingleSel, BM_GETCHECK, 0, 0 ) )
		{
			return PARA_TAG_INDEX;
		}

		hSingleSel = GetDlgItem( IDC_RADIO_LABEL_TEXT );
		if( NULL == hSingleSel )
		{
			ASSERT( FALSE );
			return -1;
		}

		if( 0 < ::SendMessage( hSingleSel, BM_GETCHECK, 0, 0 ) )
		{
			return A_TAG_INDEX;
		}

		hSingleSel = GetDlgItem( IDC_RADIO_TABLE_TEXT );
		if( NULL == hSingleSel )
		{
			ASSERT( FALSE );
			return -1;
		}

		if( 0 < ::SendMessage( hSingleSel, BM_GETCHECK, 0, 0 ) )
		{
			return TD_TAG_INDEX;
		}

		return -1;
	}

	INT32 on_uninit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 ret; 
		save_all_filter_text(); 

		ret = do_ui_ctrl( HTTP_TXT_FLT_UI_ID, HTTP_TXT_FLT_INIT, NULL, 0, NULL, 0, this ); 

		return ret;
	}

	LRESULT on_show_wnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}


	LRESULT on_key_down(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		BOOL tmp; 
		bHandled = FALSE; 

		//::MessageBox( NULL, "key down\n", NULL, 0 ); 
		switch( wParam )
		{
		case VK_RETURN:
			on_ok_bn( NULL, IDOK, NULL, tmp ); 
			break; 
		}

		return 0; 
	}

	LRESULT on_init_dlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 ret;  
		//if (1 == GetPropertySheet().GetActiveIndex())
		//	GetPropertySheet().CenterWindow();

		//hBakBr = ::CreateSolidBrush( XCONFIG_DLG_BK_CLR ); 
		ret = ui_init(); 
		if( ret < 0 )
		{
			return ret; 
		}

		::MessageBox( NULL, "http txt flt tab do_ui_ctrl\n", NULL, 0 ); 
		ret = do_ui_ctrl( HTTP_TXT_FLT_UI_ID, 
			HTTP_TXT_FLT_INIT, 
			NULL, 
			0, 
			NULL, 
			0, 
			this ); 
		
		if( ret < 0 )
		{
			DestroyWindow(); 
		}

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
		// TODO: 在此添加控件通知处理程序代码

		mng_flt_txt( TRUE ); 
		return 0; 
	}

protected:
};

#endif //__HTTP_TEXT_FLT_TAB_H__