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
 
 #include "StdAfx.h"
#include "sevenfw_common.h"
#include "url_rule_dlg.h"

INT32 http_url_flt_add_filter_text( LPCTSTR text, ULONG text_type, ULONG flags, PVOID param )
{
	INT32 ret = 0; 
	PFILTER_URL_INPUT filter_text = NULL; 
	CHAR output[ MAX_PATH ]; 
	url_rule_dlg *http_url_flt; 
#ifdef _UNICODE
	LPSTR text_out = NULL; 
	ULONG need_len; 
#endif //_UNICODE

	ASSERT( NULL != text ); 
	ASSERT( NULL != param ); 

	http_url_flt = ( url_rule_dlg* )param; 

	if( text_type > MAX_TAG_INDEX )
	{
		ASSERT( FALSE ); 
		ret = -1; 
		goto _return; 
	}

	filter_text = ( PFILTER_URL_INPUT  )malloc( sizeof( FILTER_URL_INPUT ) + _tcslen( text ) + sizeof( TCHAR ) ); 
	if( NULL == filter_text )
	{
		ret = -1; 
		goto _return; 
	}

#ifdef _UNICODE
	{
		ret = unicode_to_mcbs( text, NULL, 0, &need_len ); 
		if( ret < 0 )
		{
			if( ret != -ENOMEM )
			{
				goto _return; 
			}
		}

		text_out = ( LPSTR )malloc( sizeof( CHAR ) * need_len ); 
		if( text_out == NULL )
		{
			ret = -ENOMEM; 
			goto _return; 
		}

		ret = unicode_to_mcbs( text, text_out, need_len, NULL ); 
		if( ret < 0 )
		{
			goto _return; 
		}

		memcpy( filter_text->url, text_out, need_len ); 

		filter_text->length = need_len; 
	}
#else
	filter_text->length = _tcslen( text ) + sizeof( CHAR ); 
	//filter_text->type = text_type;
	memcpy( filter_text->url, text, filter_text->length ); 
#endif //_UNICODE
	
	ret = do_ui_ctrl( SEVENFW_UI_ID, 
		HTTP_URL_FLT_ADD_URL, 
		( PBYTE )filter_text, 
		sizeof( FILTER_URL_INPUT ) + _tcslen( text ) + sizeof( CHAR ), 
		NULL, 
		0, 
		NULL, 
		param ); 
	if( flags == 1 )
	{
		sprintf( output, "Ìí¼Ó %s %s return %d \n", 
			filter_text->url, 
			ret < 0 ? "Ê§°Ü" : "³É¹¦", 
			ret ); 
		MessageBoxA( NULL, output, NULL, 0 );
	}

	if( ret == ERROR_SUCCESS )
	{
		ret = http_url_flt->add_flt_info_to_list( http_url_flt->rules_count++, text_type, text ); 
		ret = save_filter_to_file( text, text_type, FILTER_URL_BACKUP_FILE ); 
	}
	else 
	{
		if( ret != ERROR_LIST_ITEM_ALREADY_EXIST )
		{
			goto _return; 
		}
	}

_return:
	if( text_out != NULL )
	{
		free( text_out ); 
	}
	return ret; 
}
