/*
 * nPOP
 *
 * String.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#include "General.h"
/* Include Files */
#include <windows.h>

#include "memory_mng.h"
#include "string_mng.h"

/* Define */
#define to_lower_t(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)
#define to_lower(c)			((c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c)

/* Global Variables */

/* Local Function Prototypes */
static int str_len_n(const TCHAR *buf, int len);

/*
 * a2i - 数字の文字列を数値(int)に変換する
 */
int a2i(const char *str)
{
	int num = 0;
	int m = 1;

	if (*str == '-') {
		m = -1;
		str++;
	} else if (*str == '+') {
		str++;
	}

	for (; *str >= '0' && *str <= '9'; str++) {
		num = 10 * num + (*str - '0');
	}
	return num * m;
}

/*
 * delete_ctrl_char - コントロール文字を削除する
 */
void delete_ctrl_char(TCHAR *buf)
{
	TCHAR *p, *r;

	for (p = r = buf; *p != TEXT('\0'); p++) {
		if (*p == TEXT('\t')) {
			*(r++) = TEXT(' ');
		} else if (*p != TEXT('\r') && *p != TEXT('\n')) {
			*(r++) = *p;
		}
	}
	*r = TEXT('\0');
}

/*
 * alloc_copy_t - バッファを確保して文字列をコピーする
 */
TCHAR *alloc_copy_t(const TCHAR *buf)
{
	TCHAR *ret;

	if (buf == NULL) {
		return NULL;
	}
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(buf) + 1));
	if (ret != NULL) {
		lstrcpy(ret, buf);
	}
	return ret;
}

/*
 * alloc_copy - バッファを確保して文字列をコピーする
 */
#ifdef UNICODE
char *alloc_copy(const char *buf)
{
	char *ret;

	if (buf == NULL) {
		return NULL;
	}
	ret = (char *)mem_alloc(sizeof(char) * (tstrlen(buf) + 1));
	if (ret != NULL) {
		tstrcpy(ret, buf);
	}
	return ret;
}
#endif

/*
 * alloc_tchar_to_char -  allocate and convert TCHAR string to char string

 */
#ifdef UNICODE
char *alloc_tchar_to_char(TCHAR *str)
{
	char *cchar = NULL;
	int len;

	if( str == NULL )
	{
		goto _return; 
	}

	len = tchar_to_char_size( str ); 

	cchar = ( char* )mem_alloc( len + 1 );
	
	if( cchar == NULL )
	{
		goto _return; 
	}

	tchar_to_char( str, cchar, len );

_return:
	return cchar;
}

#endif

/*
 * alloc_char_to_tchar - allocate and convert TCHAR string to char string
 */
#ifdef UNICODE
TCHAR *alloc_char_to_tchar(char *str)
{
	TCHAR *tchar = NULL;
	int len;

	if( str == NULL ) 
	{
		goto _return; 
	}

	len = char_to_tchar_size( str ); 

	tchar = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( len + 1 ) ); 
	
	if( tchar == NULL )
	{
		goto _return; 
	}

	char_to_tchar( str, tchar, len );

_return:
	return tchar;
}
#endif

/*
 * alloc_wchar_to_char - ワイド文字をASCII文字に変換
 */
#ifndef UNICODE
char *alloc_wchar_to_char(const UINT cp, WCHAR *str)
{
	char *cchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = WideCharToMultiByte(cp, 0, str, -1, NULL, 0, NULL, NULL);
	cchar = (char *)mem_alloc(len + 1);
	if (cchar == NULL) {
		return NULL;
	}
	WideCharToMultiByte(cp, 0, str, -1, cchar, len, NULL, NULL);
	return cchar;
}
#endif

/*
 * alloc_char_to_wchar - ASCII文字をワイド文字に変換
 */
#ifndef UNICODE
WCHAR *alloc_char_to_wchar(const UINT cp, char *str)
{
	WCHAR *tchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = MultiByteToWideChar(cp, 0, str, -1, NULL, 0);
	tchar = (WCHAR *)mem_alloc(sizeof(WCHAR) * (len + 1));
	if (tchar == NULL) {
		return NULL;
	}
	MultiByteToWideChar(cp, 0, str, -1, tchar, len);
	return tchar;
}
#endif

/*
 * str_join_t - 文字列を連結して最後の文字のアドレスを返す
 */
TCHAR * __cdecl str_join_t(TCHAR *ret, ... )
{
	va_list buf;
	TCHAR *str;

	va_start(buf, ret);

	str = va_arg(buf, TCHAR *);
	
	while( str != (TCHAR *)-1 ) 
	{
		if( str != NULL )
		{
			while ( *( ret++ ) = *( str++ ) );
			ret--;
		}

		str = va_arg(buf, TCHAR *);
	}

	va_end(buf);
	return ret;
}

/*
 * str_join - 文字列を連結して最後の文字のアドレスを返す
 */
#ifdef UNICODE
char * __cdecl str_join(char *ret, ... )
{
	va_list buf;
	char *str;

	va_start(buf, ret);

	str = va_arg(buf, char *);
	while (str != (char *)-1) {
		if (str != NULL) {
			while (*(ret++) = *(str++));
			ret--;
		}
		str = va_arg(buf, char *);
	}

	va_end(buf);
	return ret;
}
#endif

/*
 * str_cpy_t - 文字列をコピーして最後の文字のアドレスを返す
 */
TCHAR *str_cpy_t(TCHAR *ret, TCHAR *buf)
{
	if (buf == NULL) {
		*ret = TEXT('\0');
		return ret;
	}
 	while (*(ret++) = *(buf++));
	ret--;
	return ret;
}

/*
 * str_cpy - 文字列をコピーして最後の文字のアドレスを返す
 */
#ifdef UNICODE
char *str_cpy(char *ret, char *buf)
{
	if (buf == NULL) {
		*ret = '\0';
		return ret;
	}
	while (*(ret++) = *(buf++));
	ret--;
	return ret;
}
#endif

/*
 * str_cpy_n_t - 指定された文字数まで文字列をコピーする
 */
void str_cpy_n_t(TCHAR *ret, TCHAR *buf, int len)
{
	while (--len && (*(ret++) = *(buf++)));
	*ret = TEXT('\0');
}

/*
 * str_cpy_n - 指定された文字数まで文字列をコピーする
 */
#ifdef UNICODE
void str_cpy_n(char *ret, char *buf, int len)
{
	while (--len && (*(ret++) = *(buf++)));
	*ret = '\0';
}
#endif

/*
 * str_cpy_f_t - 指定の文字までの文字列をコピーする
 */
TCHAR *str_cpy_f_t(TCHAR *ret, TCHAR *buf, TCHAR c)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != c && *p != TEXT('\0'); p++, r++) {
#ifndef UNICODE
		// 2バイトコードの場合は2バイト進める
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			*(r++) = *(p++);
		}
#endif
		*r = *p;
	}
	*r = TEXT('\0');
	return ((*p == c) ? p + 1 : p);
}

/*
 * str_cpy_f - 指定の文字までの文字列をコピーする
 */
#ifdef UNICODE
char *str_cpy_f(char *ret, char *buf, char c)
{
	char *p, *r;

	for (p = buf, r = ret; *p != c && *p != '\0'; p++, r++) {
		*r = *p;
	}
	*r = '\0';
	return ((*p == c) ? p + 1 : p);
}
#endif

/*
 * str_cat_n - 指定された文字数まで文字列を追加する
 */
void str_cat_n(TCHAR *ret, char *buf, int len)
{
	TCHAR *p, *r;
	int i;

#ifdef UNICODE
	TCHAR *tBuf;

	p = tBuf = alloc_char_to_tchar(buf);
	if (p == NULL) {
		return;
	}
#else
	p = buf;
#endif

	i = lstrlen(ret);
	r = ret + i;
	while (*p != TEXT('\0') && i < len) {
		*(r++) = *(p++);
		i++;
	}
	*r = TEXT('\0');

#ifdef UNICODE
	mem_free(&tBuf);
#endif
}

/*
 * str_cmp_i_t - 文字列の大文字小文字を区別しない比較を行う (TCHAR)
 */
int str_cmp_i_t(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	while (to_lower_t(*buf1) == to_lower_t(*buf2)) {
		if (*buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}

/*
 * str_cmp_i - 文字列の大文字小文字を区別しない比較を行う
 */
#ifdef UNICODE
int str_cmp_i(const char *buf1, const char *buf2)
{
	while (to_lower(*buf1) == to_lower(*buf2)) {
		if (*buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}
#endif

/*
 * str_cmp_ni_t - 文字列の大文字小文字を区別しない比較を行う (TCHAR)
 */
int str_cmp_ni_t(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	while (to_lower_t(*buf1) == to_lower_t(*buf2)) {
		len--;
		if (len <= 0 || *buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}

/*
 * str_cmp_ni - 文字列の大文字小文字を区別しない比較を行う
 */
#ifdef UNICODE
int str_cmp_ni(const char *buf1, const char *buf2, int len)
{
	while (to_lower(*buf1) == to_lower(*buf2)) {
		len--;
		if (len <= 0 || *buf1 == '\0') {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}
#endif

/*
 * str_match_t - 2つの文字列をワイルドカード(*)を使って比較を行う
 */
BOOL str_match_t(const TCHAR *ptn, const TCHAR *str)
{
	switch (*ptn) {
	case TEXT('\0'):
		return (*str == TEXT('\0'));
	case TEXT('*'):
		if (*(ptn + 1) == TEXT('\0')) {
			return TRUE;
		}
		if (str_match_t(ptn + 1, str) == TRUE) {
			return TRUE;
		}
		while (*str != TEXT('\0')) {
			str++;
			if (str_match_t(ptn + 1, str) == TRUE) {
				return TRUE;
			}
		}
		return FALSE;
	case TEXT('?'):
		return (*str != TEXT('\0')) && str_match_t(ptn + 1, str + 1);
	default:
		while (to_lower_t(*ptn) == to_lower_t(*str)) {
			if (*ptn == TEXT('\0')) {
				return TRUE;
			}
			ptn++;
			str++;
			if (*ptn == TEXT('*') || *ptn == TEXT('?')) {
				return str_match_t(ptn, str);
			}
		}
		return FALSE;
	}
}

/*
 * str_match - 2 string compare function have the wildcard support.etc: (*)
 */
#ifdef UNICODE
BOOL str_match( const char *ptn, const char *str )
{
	switch( *ptn )
	{
	case '\0':
		return (*str == '\0');
	case '*':
		if( *( ptn + 1 ) == '\0' )
		{
			return TRUE;
		}

		if( str_match( ptn + 1, str ) == TRUE )
		{
			return TRUE;
		}
		
		while( *str != '\0' )
		{
			str++;
			if( str_match( ptn + 1, str ) == TRUE )
			{
				return TRUE;
			}
		}

		return FALSE;
	
	case '?':
		return ( *str != '\0' ) && str_match( ptn + 1, str + 1 );
	
	default:
		while ( to_lower( *ptn ) == to_lower( *str ) )
		{
			if( *ptn == '\0' )
			{
				return TRUE;
			}
			
			ptn++;
			str++;
			if( *ptn == '*' || *ptn == '?' )
			{
				return str_match( ptn, str );
			}
		}

		return FALSE;
	}
}
#endif

/*
 * str_len_n - 比較用の文字列の長さを取得する
 */
static int str_len_n(const TCHAR *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (*buf == TEXT('\0')) {
			break;
		}
		buf++;
	}
	return i;
}

/*
 * str_find - 文字列内に含まれる文字列を検索して位置を返す
 */
TCHAR *str_find(TCHAR *ptn, TCHAR *str, int case_flag)
{
	TCHAR *p;
	int len1, len2;

	len1 = lstrlen(ptn);
	for (p = str; *p != TEXT('\0'); p++) {
		len2 = str_len_n(p, len1);
		if (CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
			(case_flag) ? 0 : NORM_IGNORECASE, p, len2, ptn, len1) == 2) {
			break;
		}
#ifndef UNICODE
		// 2バイトコードの場合は2バイト進める
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
		}
#endif
	}
	return p;
}
/* End of source */
