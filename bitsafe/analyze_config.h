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