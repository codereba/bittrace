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

#include "common_func.h"
#include "local_strings.h"

pcchar_t *all_strings_sel = NULL; 

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
