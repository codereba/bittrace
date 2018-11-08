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

#ifdef _DEBUG_MEM_LEAKS
#undef _DEBUG_MEM_LEAKS
#endif //_DEBUG_MEM_LEAKS

#include "StdAfx.h"
#include "common_func.h"
#include "action_type.h"
#include "trace_log_api.h"
#include "bitsafe_common.h"
#include "sqlite3.h"
#include "search_dlg.h"
#include "msg_box.h"

/**************************************************************************
搜索功能:
是否加入选择框，让用户选择搜索的范围？

**************************************************************************/

/*********************************************************************************
极简功能设计:
1.对常见的分析目标进行分类，归集，建立分析模式。
*********************************************************************************/
LRESULT search_dlg::init_search_text_history()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{

	}while( FALSE );

	return ntstatus; 
}

LRESULT WINAPI check_search_text_valid( LPCWSTR search_text )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( search_text != NULL ); 

	}while( FALSE );

	return ret; 
}

LRESULT search_dlg::_Init()
{
	LRESULT ret = ERROR_SUCCESS; 
	CEditUI *edit; 

	do 
	{
		ret = init_search_text_history(); 
		if( ret != ERROR_SUCCESS )
		{
		}

		edit = ( CEditUI* )m_pm.FindControl( L"key_word_edit" ); 
		ASSERT( edit != NULL ); 

		//设置焦点
		edit->SetFocus(); 

#ifdef LANG_EN
#define SEARCH_KEY_WORD_LABLE_TEXT L"Key word"
#define SEARCH_DIRECT_OPTION_TEXT L"Search up"
#define SEARCH_BUTTON_TEXT L"Search"
#define CANCEL_BUTTON_TEXT L"Cancel"

		set_ctrl_text_from_name( &m_pm, L"key_word_label", SEARCH_KEY_WORD_LABLE_TEXT ); 
		set_ctrl_tooltip_from_name( &m_pm, L"key_word_edit", SEARCH_KEY_WORD_LABLE_TEXT ); 
		set_ctrl_text_from_name( &m_pm, L"search_dir", SEARCH_DIRECT_OPTION_TEXT ); 
		set_ctrl_text_from_name( &m_pm, L"search_btn", SEARCH_BUTTON_TEXT ); 
		set_ctrl_text_from_name( &m_pm, L"cancel_btn", CANCEL_BUTTON_TEXT ); 

#else
#define SEARCH_KEY_WORD_LABLE_TEXT L"关键字"
#define SEARCH_DIRECT_OPTION_TEXT L"向上搜索"
#define SEARCH_BUTTON_TEXT L"查找"
#define CANCEL_BUTTON_TEXT L"取消"
#endif //LANG_EN

	}while( FALSE );

	return ret; 
}
