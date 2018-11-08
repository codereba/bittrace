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

#pragma once

#define PARA_TAG_INDEX 0 
#define TD_TAG_INDEX 1 
#define A_TAG_INDEX 2
#define MAX_TAG_INDEX 2 

#define NOT_SUPP_LEX 11
#define FIND_KEY_END 1
#define WORD_TAG_END 0x01
#define array_size( arr ) ( sizeof( arr ) / sizeof( arr[ 0 ] ) )

#pragma pack( push )
#pragma pack( 1 )

typedef struct __http_filter_name
{
	LIST_ENTRY list_entry; 
	FILTER_NAME_INPUT filter_name; 
} http_filter_name, *phttp_filter_name; 

typedef struct __http_filter_texts
{
	LIST_ENTRY filter_texts; 
	KSPIN_LOCK sp_lock; 
} http_filter_texts, *phttp_filter_texts;

typedef struct _tag_info
{
	CHAR *tag; 
	ULONG tag_len; 
	http_filter_texts texts; 
} tag_info, *ptag_info; 

#pragma pack( pop )

#define TAG_BEGIN_PREFIX "<"
#define TAG_END_PREFIX "</"
#define TAG_TAIL ">"

#define BEGIN_TAG 1
#define END_TAG 2

#define A_TAG "a"
#define TD_TAG "td"
#define PARA_TAG "p"
#define PRE_TAG "pre"
#define HTML_TAG "html"

#define CASE_DIFF ( INT32 )( 'A'-'a' )
#define TAG_END( tag ) ( TAG_END_PREFIX tag TAG_TAIL )
#define TAG_BEGIN( tag ) ( TAG_BEGIN_PREFIX tag )

#define TAG_END_LEN( tag ) ( CONST_STR_LEN( tag ) + CONSTR_STR_LEN( TAG_END_PREFIX ) + CONST_STR_LEN( TAG_TAIL ) )
#define TAG_BEGIN_LEN( tag ) ( CONST_STR_LEN( tag ) + CONST_STR_LEN( TAG_BEGIN_PREFIX ) + CONST_STR_LEN( TAG_TAIL ) )

#define _TAG_END_LEN( tag_len ) (  tag_len + CONST_STR_LEN( TAG_END_PREFIX ) + CONST_STR_LEN( TAG_TAIL ) )
#define _TAG_BEGIN_LEN( tag_len ) ( tag_len + CONST_STR_LEN( TAG_BEGIN_PREFIX ) + CONST_STR_LEN( TAG_TAIL ) )

#ifndef _VERBOSE
#define addreply
#else
#define addreply printf
#endif

#define HTTP_SERVER_IP "192.168.102.128"
#define HTTP_SERVER_PORT 8081

#define CONTENT_REMAIN_LEN( pos ) (int) (xmlcontent + file_len - pos )

typedef int ( *analyzelucenceresfunc)( char * );

extern common_hash_table tcpip_info_hash_table; 
extern common_hash_table url_info_hash_table; 

#ifdef __cplusplus
extern "C" {
#endif

	CHAR* memsrch_tag( CHAR **tag, LONG tag_len, CHAR *data, LONG data_len, ULONG flags );  
	LONG memmem( const CHAR *mem_find, LONG mem_find_len, const CHAR *mem, LONG memlen ); 
	unsigned int http_request( char *pArgs, char *url_link, analyzelucenceresfunc analyzefunc ); 
	INT32 simple_http_filter( char *html_cont, ULONG cont_len, ULONG src_ip, USHORT src_port, ULONG dst_ip, USHORT dst_port, ULONG tcp_status ); 
	NTSTATUS init_http_filter_list(); 
	NTSTATUS release_http_filter_list(); 
	INT32 set_http_filter_name( PFILTER_NAME_INPUT filter_name, ULONG flag ); 
	CHAR* memsrch_nocase( char *mem_find, LONG mem_find_len, char *mem, LONG memlen ); 
	ULONG get_all_http_filter_names( PFILTER_NAMES_OUTPUT all_filter_texts, ULONG length ); 
	INT32 get_all_http_filter_urls( PFILTER_NAMES_OUTPUT filter_texts_output, PULONG all_length ); 
	INT32 set_http_filter_url( PFILTER_URL_INPUT filter_url, ULONG flag ); 
	NTSTATUS set_http_filter_urls( PFILTER_URLS_INPUT filter_urls, ULONG input_len, ULONG flag ); 
	VOID get_tcp_pack_status( struct _TCP* tcp_hdr, ULONG *status ); 
	//INT32 url_filter( CHAR *html_cont, ULONG cont_len, ULONG src_ip, USHORT src_port, ULONG dst_ip, USHORT dst_port ); 
	__inline CHAR *memsrch( const CHAR *mem_find, LONG mem_find_len, const CHAR *mem, LONG mem_len )
	{
		LONG finded_pos; 
		finded_pos = memmem( mem_find, mem_find_len, mem, mem_len ); 

		if( 0 > finded_pos )
		{
			return NULL; 
		}

		ASSERT( finded_pos <= mem_len - mem_find_len ); 

		return ( CHAR* )mem + finded_pos; 
	}

	INT32 check_reset_block_tcpip( CHAR *data, ULONG length, ULONG src_ip, USHORT src_port, ULONG dst_ip, USHORT dst_port, ULONG tcp_status, ULONG *flags ); 

	INLINE NTSTATUS filter_url_addr( LPCSTR domain_name, LPCSTR file_path )
	{
		NTSTATUS ntstatus = STATUS_SUCCESS; 

		ASSERT( domain_name != NULL ); 

		DBGPRINT( ( "enter filter_url_addr domain_name %s file path %s \n", domain_name, file_path == NULL ? "NULL" : file_path ) ); 

		ntstatus = __find_url_in_hash_table( domain_name, file_path, &url_info_hash_table ); 

		return ntstatus; 
	}

#ifdef __cplusplus
}
#endif