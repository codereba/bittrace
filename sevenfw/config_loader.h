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

 #pragma once

#define FORMAT_ERROR -101

#define FILTER_TEXT_BACKUP_FILE _T( "texts" )
#define FILTER_URL_BACKUP_FILE _T( "ulrs" )

typedef INT32 ( treat_filter_text )( LPCTSTR filt_rule, ULONG type, ULONG flags, PVOID param ); 


#define LINE_TERM _T( "\r\n" )
#define LINE_TERM_LEN ( CONST_STR_LEN( LINE_TERM ) * sizeof( TCHAR ) )

//#define LINE_TERM _T( "\r\n" )
//#define LINE_TERM_LEN 2
#define ERR_EXCEED_MAX_LN_LEN -141
#define ERR_DISCONNECT -142
#define MAX_MSG_LEVEL 16
#define TEXT_POOL_LEN ( 4096 * 4 )

#define MAX_TEXT_LEN 1024
#define FILTER_TEXT_DELIM ':'

#ifdef __cplusplus
extern "C"
{
#endif

INT32 get_text_and_type( LPTSTR text_ln, LPTSTR text, ULONG text_len, ULONG *text_type, CHAR *file_name ); 
LRESULT load_filter_text_from_file( treat_filter_text treat_func, PVOID param, LPCTSTR file_name ); 
INT32 save_filter_to_file( LPCTSTR text, ULONG type, LPCTSTR file_name ); 
INT32 del_filter_text_from_file( LPCTSTR text, ULONG text_len, LPCTSTR file_name ); 
INT32 reset_filter_text_file( LPCTSTR file_name ); 
#ifdef __cplusplus
}
#endif
