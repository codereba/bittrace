#ifndef __LANG_EN_H__
#define __LANG_EN_H__

#include "string_common.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern pcchar_t all_strings_en[]; 

#ifdef _DEBUG
extern ULONG all_strings_en_count; 
#define LAST_EN_STRING __t( "last en string" )
#endif //_DEBUG

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__LANG_EN_H__