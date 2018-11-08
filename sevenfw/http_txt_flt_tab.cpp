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
#include "http_txt_flt_tab.h"

INT32 http_txt_flt_add_filter_text( CHAR *text, ULONG text_type, ULONG flags, PVOID param )
{
	INT32 ret; 
	HWND hList; 
	PFILTER_NAME_INPUT filter_text = NULL; 
	CHAR output[ MAX_PATH ]; 
	http_txt_flt_tab *http_txt_flt; 

	ASSERT( NULL != text ); 
	ASSERT( NULL != param ); 

	http_txt_flt = ( http_txt_flt_tab* )param; 

	if( text_type > MAX_TAG_INDEX )
	{
		ASSERT( FALSE ); 
		return -1; 
	}

	hList = http_txt_flt->GetDlgItem( IDC_LIST_FILTER_TEXT );
	if( NULL == hList )
	{
		ASSERT( NULL != hList );
		return -1;
	}

	filter_text = ( PFILTER_NAME_INPUT  )malloc( sizeof( FILTER_NAME_INPUT ) + strlen( text ) + sizeof( CHAR ) ); 
	if( NULL == filter_text )
	{
		return -1; 
	}

	filter_text->length = strlen( text ) + sizeof( CHAR ); 
	filter_text->type = text_type;

	memcpy( filter_text->text, text, filter_text->length ); 
	ret = do_ui_ctrl( HTTP_TXT_FLT_UI_ID, 
		HTTP_TXT_FLT_ADD_URL, 
		( PBYTE )filter_text, 
		sizeof( FILTER_NAME_INPUT ) + strlen( text ) + sizeof( CHAR ), 
		NULL, 
		0, 
		param ); 
	if( flags == 1 )
	{
		sprintf( output, "Ìí¼Ó %s %s %s return %d \n", 
		 http_txt_flt->get_text_type_desc( filter_text->type ), 
		 filter_text->text, 
		 ret < 0 ? "Ê§°Ü" : "³É¹¦", 
		 ret ); 
	}

	if( ret == ADDED_NEW_FILTER )
	{
			ret = http_txt_flt->add_flt_info_to_list( hList, http_txt_flt->flt_txt_count++, text_type, text ); 
			ASSERT( 0 == ret ); 

			ret = save_filter_to_file( text, text_type, FILTER_TEXT_BACKUP_FILE ); 
			ASSERT( TRUE == ret ); 
	}
	else if( ret < 0 )
	{
		goto _return; 
	}

_return:
	return ret; 
}


INT32 http_flt_txt_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param )
{
	INT32 ret; 
	http_txt_flt_tab *ui_tab; 

	ret = 0; 

	switch( ui_action_id )
	{
	case HTTP_TXT_FLT_OUTPUT:
		if( ret_val < 0 )
		{
			break; 
		}

		ui_tab = ( http_txt_flt_tab* )param; 
		ret = ui_tab->output_seted_filter_texts( ( PFILTER_NAMES_OUTPUT )data, length ); 
		break; 
	default:
		break; 
	}

	return ret; 
}