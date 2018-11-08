#ifndef __SEVEN_FW_API_H__
#define __SEVEN_FW_API_H__

#define BLACK_RULE 0
#define RECOMEND_BLACK_RULE 1
#define WHITE_RULE 2
#define RECOMEND_WHITE_RULE 3
#define SUPER_WHITE_RULE 4
#define BASE_WHITE_RULE 5
#define RULE_MODE_NUM 6

#define PARA_TAG_INDEX 0 
#define TD_TAG_INDEX 1 
#define A_TAG_INDEX 2
#define MAX_TAG_INDEX 2 

#define ADDED_NEW_FILTER 1

#define RULE_MODE_NONE 0xFF

typedef enum __BLOCK_MODE
{
	BLOCK_ALL_MODE, 
	FREE_MODE, 
	GREEN_MODE, 
	LIMIT_MODE
} BLOCK_MODE;

#pragma pack( push )
#pragma pack( 1 )

#pragma warning(push)
#pragma warning(disable:4200) //nonstandard extension used : zero-sized array in struct/union

typedef struct __FILTER_NAME_INPUTA
{
	USHORT type; 
	USHORT length; 
	CHAR text[ 0 ]; 
} FILTER_NAME_INPUTA, *PFILTER_NAME_INPUTA; 

typedef struct __FILTER_NAME_INPUTW
{
	USHORT type; 
	USHORT length; 
	WCHAR text[ 0 ]; 
} FILTER_NAME_INPUTW, *PFILTER_NAME_INPUTW; 

#ifdef _UNICODE
typedef FILTER_NAME_INPUTW FILTER_NAME_INPUT, *PFILTER_NAME_INPUT;  
#else
typedef FILTER_NAME_INPUTA FILTER_NAME_INPUT, *PFILTER_NAME_INPUT; 
#endif //_UNICODE

typedef struct _FILTER_URL_INPUT
{
	ULONG length; 
	CHAR url[ 0 ]; 
} FILTER_URL_INPUT, *PFILTER_URL_INPUT; 

typedef struct _FILTER_URLS_INPUT
{
	ULONG size;
	ULONG flag;
	CHAR urls[ 1 ]; // end by all zero item;
} FILTER_URLS_INPUT, *PFILTER_URLS_INPUT; 

#pragma warning(pop)

typedef struct __FILTER_NAMES_OUTPUT
{
	ULONG all_need_length; 
	FILTER_NAME_INPUT name_output; 
} FILTER_NAMES_OUTPUT, *PFILTER_NAMES_OUTPUT; 
#pragma pack( pop )

#define INLINE __inline

#define HTTP_TXT_FLT_DEV_DOS_NAME L"\\DosDevices\\HTTP_TXT_FLT"
#define HTTP_URL_FLT_DEV_DOS_NAME L"\\DosDevices\\HTTP_URL_FLT"

#define HTTP_TXT_FLT_DEV_FILE_NAME _T( "\\\\.\\HTTP_TXT_FLT" )
#define HTTP_URL_FLT_DEV_FILE_NAME _T( "\\\\.\\HTTP_URL_FLT" )

#endif //__SEVEN_FW_API_H__