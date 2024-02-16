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
 
#ifndef __HTTP_FLT_TAB_H__
#define __HTTP_FLT_TAB_H__

#include "seven_fw_api.h"
#include "config_loader.h"

#define MAX_NAMES_LEN 2048

class http_flt_tab : public ui_base_tab
{
public:

	http_flt_tab()
	{
		flt_txt_count = 0; 
	}

	INT32 on_uninit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 ret; 
		save_all_filter_text(); 

		ret = do_ui_ctrl( HTTP_TXT_FLT_UI_ID, HTTP_URL_FLT_INIT, NULL, 0, NULL, 0, this ); 
		
		return ret;
	}

	INT32 on_sys_cmd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 ret; 
		ret = 0; 
		return ret; 
	}

	INT32 save_all_filter_text()
	{
		HWND hList; 
		INT32 ret;
		INT32 cur_item;
		LV_ITEM lvItem;
		CHAR text[ 256 ]; 
		CHAR filter_text[ 256 ]; 
		ULONG type; 

		ret = reset_filter_text_file( FILTER_TEXT_BACKUP_FILE); 
		if( ret < 0 )
		{
			ret = FALSE; 
			goto _return; 
		}

		hList = GetDlgItem( IDC_LIST_FILTER_TEXT ); 

		cur_item = 0; 

		ret = TRUE; 

		memset( &lvItem, 0, sizeof( lvItem ) );

		lvItem.cchTextMax = 256; 
		lvItem.pszText = text; 
		lvItem.mask = LVIF_TEXT; 

		for( ; ; )
		{
			lvItem.iItem = cur_item; 
			lvItem.iSubItem = 0; 
			ret = ListView_GetItem( hList, &lvItem ); 
			if( ret == FALSE )
			{
				return FALSE; 
			}

			strcpy( filter_text, text ); 

			lvItem.iSubItem = 1; 
			ret = ListView_GetItem( hList, &lvItem ); 
			if( ret == FALSE )
			{
				ret = FALSE; 
				goto _return; 
			}

			type = text_type_from_desc( text ); 
			if( type < 0 )
			{
				ret = FALSE; 
				goto _return; 
			}

			ret = save_filter_to_file( filter_text, type, FILTER_TEXT_BACKUP_FILE  ); 
			if( 0 > ret )
			{
				ret = FALSE; 
				goto _return; 
			}

			cur_item += 1; 
		}

_return:
		return ret; 
	}

	INT32 CleanRemainListItem( HWND hList, DWORD dwItemIndex )
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

	LRESULT on_list_notify( INT32 param, NMHDR *pnmhdr, BOOL &handled)
	{
		HWND hList; 
		INT32 ret;
		DWORD dwNotifyCode;
		INT32 nSelItem;

		hList = GetDlgItem( IDC_LIST_FILTER_TEXT ); 

		if( hList != pnmhdr->hwndFrom )
		{
			return FALSE; 
		}

		dwNotifyCode = pnmhdr->code;

		nSelItem = ListView_GetSelectionMark( hList );

		if( nSelItem < 0 )
		{
			return FALSE;
		}

		ret = TRUE; 
		switch( dwNotifyCode )
		{
		case NM_RCLICK:
			{
				if( 0 != show_popup_menu( m_hWnd, ID_POPUP_DEL_TEXT ) )
				{
					return FALSE; 
				}
			}
			break; 
		case NM_CLICK:
			{
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

_return:
		return ret;
	}

	virtual CHAR *get_text_type_desc( ULONG type )
	{
		return ""; 
	}

	INT32 output_seted_filter_texts( PFILTER_NAMES_OUTPUT flt_names, ULONG length )
	{
		INT32 ret; 
		ULONG ret_length = length; 
		PFILTER_NAMES_OUTPUT filter_text; 
		PFILTER_NAME_INPUT filter_text_get; 
		HWND hList; 
		PUI_CTRL ui_ctrl; 

		hList = GetDlgItem( IDC_LIST_FILTER_TEXT ); 
		ASSERT( hList != NULL ); 

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
				ASSERT( *( filter_text_get->text ) == '\0' );

				ret = 0; 
				break; 
			}

			if( filter_text_get->length + sizeof( FILTER_NAME_INPUT ) > ret_length )
			{
				ASSERT( FALSE ); 
				ret = -1; 
				break; 
			}

			if( filter_text_get->text[ filter_text_get->length - 1 ] != '\0' )
			{
				filter_text_get->text[ filter_text_get->length - 1 ] = '\0'; 
			}

			ret = add_flt_info_to_list( hList, flt_txt_count++, 0, filter_text_get->text ); 
			ret = save_filter_to_file( filter_text_get->text, 0, FILTER_TEXT_BACKUP_FILE ); 
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

		return ret; 
	}

	LRESULT on_show_wnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
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
		// TODO: �ڴ����ӿؼ�֪ͨ�����������
		return 0; 
	}

	virtual INT32 add_flt_info_to_list( HWND ListControl, 
		DWORD item_index, 
		ULONG type, 
		LPSTR filter_text ) = 0; 

	virtual ULONG text_type_from_desc( CHAR *desc )
	{
		return 0; 
	}

public:
	ULONG flt_txt_count; 
};
#endif//__HTTP_FLT_TAB_H__