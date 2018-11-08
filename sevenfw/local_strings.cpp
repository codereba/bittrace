/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
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

#include "common_func.h"
#include "local_strings.h"

pcchar_t *all_strings_sel = NULL; 
//ULONG lang_id = LANG_ID_CHINESE; 

LRESULT set_cur_lang( LANG_ID id )
{
	LRESULT ret = ERROR_SUCCESS; 

	if( FALSE == is_valid_lang_id( id ) )
	{
		ASSERT( FALSE && "how input the invalid language id" ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	switch( id )
	{
	case LANG_ID_EN: 
		all_strings_sel = all_strings_en; 
		break; 
	default:
		all_strings_sel = all_strings_cn; 
		break; 
	}

_return:
	return ret; 
}

LANG_ID get_cur_lang_id()
{
	LANG_ID cur_lan_id; 
	if( all_strings_sel == all_strings_en )
	{
		cur_lan_id = LANG_ID_EN; 
	}
	else
	{
		cur_lan_id = LANG_ID_CN; 
	}

	return cur_lan_id; 
}
