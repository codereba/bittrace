#ifndef __URL_HASH_TABLE_H__
#define __URL_HASH_TABLE_H__

#define DOMAIN_NAME_FOUND 0x02
#define FILE_PATH_FOUND 0x01
#define NONE_FOUND 0

typedef struct _filter_url_info
{
	USHORT type; 
	USHORT domain_name_len; 
	CHAR *domain_name; 
	USHORT file_path_len; 
	CHAR *file_path; 
} filter_url_info, *pfilter_url_info; 

typedef struct __http_filter_url
{
	LIST_ENTRY list_entry; 
	filter_url_info filter_url; 
} http_filter_url, *phttp_filter_url; 

#define DOMAIN_NAME_EXACT_MATCH 0x00000001
#define ONLY_CHECK_DOMAIN_NAME 0x00000002
#define PATH_INCLUDE_MATCH 0x00000004
#define PATH_EXACT_MATCH 0x00000008

#define WHOLD_URL_EXACT_MATCH 0x01
#define PROT_PREFIX "://" 

NTSTATUS depart_url( CHAR *url, ULONG url_len, CHAR **domain_name, CHAR **file_path ); 
ULONG calc_url_hash_code( LPCSTR domain_name, ULONG domain_name_len, LPCSTR file_path, ULONG file_path_len, pcommon_hash_table info_hash_table ); 
NTSTATUS __set_http_filter_url( PFILTER_URL_INPUT filter_url, ULONG flag, pcommon_hash_table info_hash_table ); 
NTSTATUS find_url_in_hash_table( CHAR *url, ULONG url_len, pcommon_hash_table url_hash_table ); 
NTSTATUS __find_url_in_hash_table( LPCSTR domain_name, LPCSTR file_path, pcommon_hash_table url_hash_table ); 
NTSTATUS delete_filter_url( CHAR* url, ULONG url_len, pcommon_hash_table url_hash_table ); 
NTSTATUS CALLBACK release_url_info( PLIST_ENTRY info ); 
NTSTATUS CALLBACK get_all_http_filter_names2( PFILTER_NAMES_OUTPUT filter_texts_output, PULONG all_length, pcommon_hash_table info_hash_table ); 

#endif //__URL_HASH_TABLE_H__