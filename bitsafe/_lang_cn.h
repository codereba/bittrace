#ifndef __LANG_CN_H__
#define __LANG_CN_H__

#include "string_common.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	extern pcchar_t all_strings_cn[]; 

#ifdef _DEBUG
	extern ULONG all_strings_cn_count; 
#define LAST_CN_STRING __t( "如果使用2.0.0.288之前版本,进行升级后,请先不要点击安装提示的确认按钮进行安装,而是先进行卸载后,然后重启WINDOWS,再运行,不然可能由于版本问题,导致系统出错。造成的不便请谅解." ) 

#endif //_DEBUG

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__LANG_CN_H__