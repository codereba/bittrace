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

 
 #include "common_func.h"
#include "seven_fw_api.h"
#include "http_url_flt_tab.h"
#include "seven_fw_ioctl.h"

INT32 http_url_flt_add_filter_text( CHAR *text, ULONG text_type, ULONG flags, PVOID param )
{
	INT32 ret; 
	HWND hList; 
	PFILTER_URL_INPUT filter_text = NULL; 
	CHAR output[ MAX_PATH ]; 
	http_url_flt_tab *http_url_flt; 

	ASSERT( NULL != text ); 
	ASSERT( NULL != param ); 

	http_url_flt = ( http_url_flt_tab* )param; 

	if( text_type > MAX_TAG_INDEX )
	{
		ASSERT( FALSE ); 
		return -1; 
	}

	hList = http_url_flt->GetDlgItem( IDC_LIST_FILTER_TEXT );
	if( NULL == hList )
	{
		ASSERT( NULL != hList );
		return -1;
	}

	filter_text = ( PFILTER_URL_INPUT  )malloc( sizeof( FILTER_URL_INPUT ) + strlen( text ) + sizeof( CHAR ) ); 
	if( NULL == filter_text )
	{
		return -1; 
	}

	filter_text->length = strlen( text ) + sizeof( CHAR ); 
	//filter_text->type = text_type;

	memcpy( filter_text->url, text, filter_text->length ); 
	ret = do_ui_ctrl( HTTP_URL_FLT_UI_ID, 
		HTTP_URL_FLT_ADD_URL, 
		( PBYTE )filter_text, 
		sizeof( FILTER_URL_INPUT ) + strlen( text ) + sizeof( CHAR ), 
		NULL, 
		0, 
		param ); 
	if( flags == 1 )
	{
		//sprintf( output, "添加 %s %s %s \n", 
		// get_text_type_desc( filter_text->type ), 
		// filter_text->text, 
		// ret ? "成功" : "失败" ); 

		sprintf( output, "添加 %s %s return %d \n", 
			filter_text->url, 
			ret < 0 ? "失败" : "成功", 
			ret ); 
		MessageBox( NULL, output, NULL, 0 );
	}

	if( ret == ADDED_NEW_FILTER )
	{
		ret = http_url_flt->add_flt_info_to_list( hList, http_url_flt->flt_txt_count++, text_type, text ); 
		ret = save_filter_to_file( text, text_type, FILTER_URL_BACKUP_FILE ); 
	}
	else if( ret < 0 )
	{
		goto _return; 
	}

_return:
	return ret; 
}

HRESULT http_flt_url_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param )
{
	HRESULT ret = ERROR_SUCCESS; 
	http_url_flt_tab *ui_tab; 

	switch( ui_action_id )
	{
	case HTTP_URL_FLT_OUTPUT:
		if( ret_val != ERROR_SUCCESS )
		{
			break; 
		}

		ui_tab = ( http_url_flt_tab* )param; 
		ret = ui_tab->output_seted_filter_texts( ( PFILTER_NAMES_OUTPUT )data, length ); 
		break; 
	default:
		break; 
	}

	return ret; 
}
