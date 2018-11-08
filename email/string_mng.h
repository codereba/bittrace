/*
 * ERC
 *
 * string_mng.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_STRING_H
#define _INC_STRING_H

/* Include Files */
#include <windows.h>

/* Define */
#ifdef UNICODE
#define tchar_to_char_size(wbuf) (WideCharToMultiByte(CP_ACP, 0, wbuf, -1, NULL, 0, NULL, NULL))
#else
#define tchar_to_char_size(wbuf) (lstrlen(wbuf) + 1)
#endif

#ifdef UNICODE
#define tchar_to_char(wbuf, ret, len) (WideCharToMultiByte(CP_ACP, 0, wbuf, -1, ret, len, NULL, NULL))
#else
#define tchar_to_char(wbuf, ret, len) (str_cpy_n_t(ret, wbuf, len))
#endif

#ifdef UNICODE
#define char_to_tchar_size(buf) (MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0))
#else
#define char_to_tchar_size(buf) (lstrlen(buf) + 1)
#endif

#ifdef UNICODE
#define char_to_tchar(buf, wret, len) (MultiByteToWideChar(CP_ACP, 0, buf, -1, wret, len))
#else
#define char_to_tchar(buf, wret, len) (str_cpy_n_t(wret, buf, len))
#endif

#ifdef UNICODE
#define tstrcpy		strcpy
#else
#define tstrcpy		lstrcpy
#endif

#ifdef UNICODE
#define tstrcat		strcat
#else
#define tstrcat		lstrcat
#endif

#ifdef UNICODE
#define tstrcmp		strcmp
#else
#define tstrcmp		lstrcmp
#endif

#ifdef UNICODE
#define tstrlen		strlen
#else
#define tstrlen		lstrlen
#endif

/* Struct */

/* Function Prototypes */

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
int a2i(const char *str);
void delete_ctrl_char(TCHAR *buf);

TCHAR *alloc_copy_t(const TCHAR *buf);
#ifdef UNICODE
char *alloc_copy(const char *buf);
#else
#define alloc_copy alloc_copy_t
#endif
#ifdef UNICODE
char *alloc_tchar_to_char(TCHAR *str);
TCHAR *alloc_char_to_tchar(char *str);
#else
#define alloc_tchar_to_char alloc_copy_t
#define alloc_char_to_tchar alloc_copy_t
#endif
#ifndef UNICODE
char *alloc_wchar_to_char(const UINT cp, WCHAR *str);
WCHAR *alloc_char_to_wchar(const UINT cp, char *str);
#endif

TCHAR * __cdecl str_join_t(TCHAR *ret, ... );
#ifdef UNICODE
char * __cdecl str_join(char *ret, ... );
#else
#define str_join str_join_t
#endif

TCHAR *str_cpy_t(TCHAR *ret, TCHAR *buf);
#ifdef UNICODE
char *str_cpy(char *ret, char *buf);
#else
#define str_cpy str_cpy_t
#endif
void str_cpy_n_t(TCHAR *ret, TCHAR *buf, int len);
#ifdef UNICODE
void str_cpy_n(char *ret, char *buf, int len);
#else
#define str_cpy_n str_cpy_n_t
#endif
TCHAR *str_cpy_f_t(TCHAR *ret, TCHAR *buf, TCHAR c);
#ifdef UNICODE
char *str_cpy_f(char *ret, char *buf, char c);
#else
#define str_cpy_f str_cpy_f_t
#endif
void str_cat_n(TCHAR *ret, char *buf, int len);

int str_cmp_i_t(const TCHAR *buf1, const TCHAR *buf2, int len);
#ifdef UNICODE
int str_cmp_i(const char *buf1, const char *buf2);
#else
#define str_cmp_i lstrcmpi
#endif
int str_cmp_ni_t(const TCHAR *buf1, const TCHAR *buf2, int len);
#ifdef UNICODE
int str_cmp_ni(const char *buf1, const char *buf2, int len);
#else
#define str_cmp_ni str_cmp_ni_t
#endif

BOOL str_match_t(const TCHAR *ptn, const TCHAR *str);
#ifdef UNICODE
BOOL str_match(const char *ptn, const char *str);
#else
#define str_match str_match_t
#endif
TCHAR *str_find(TCHAR *ptn, TCHAR *str, int case_flag);

#ifdef __cplusplus
};
#endif //__cplusplus
#endif
/* End of source */
