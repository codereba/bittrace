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

#ifndef __ANALYZE_CONFIG_H__
#define __ANALYZE_CONFIG_H__

#define SYMBOL_PATH_TEMPLATE L"srv*%s*http://msdl.microsoft.com/download/symbols;cache*%s"
#define MAX_SYMBOL_PATH ( MAX_PATH * 3 )
#define NO_TIP_DEFAULT_SYMBOL_CHECK 0x00000001
#define FORMAT_LOCAL_SYMBOL_PATH 0x00000002

typedef struct _panalyze_setting
{
	WCHAR dbg_help_path[ MAX_PATH ]; 
	WCHAR symbol_path[ MAX_SYMBOL_PATH ]; 
} analyze_setting, *panalyze_setting; 

extern analyze_setting event_analyze_help; 

analyze_setting *WINAPI get_analyze_help_conf(); 
ULONG WINAPI get_analyze_ui_flags(); 
LRESULT WINAPI set_analyze_help_conf( LPCWSTR symbol_path, LPCWSTR dbg_help_path, ULONG flags ); 
LRESULT WINAPI load_common_conf_callback(); 

#endif // __ANALYZE_CONFIG_H__