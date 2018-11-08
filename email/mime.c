/*
 * ERC
 *
 * mime.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "memory_mng.h"
#include "string_mng.h"
#include "jp.h"
#include "Charset.h"
#include "code.h"
#include "mime.h"
#include "multipart.h"

/* Define */
#define ESC						0x1B

#define HEAD_LINELEN			800
#define HEAD_ENCODE_LINELEN		15
#define BODY_ENCODE_LINELEN		76

#define ENC_TYPE_7BIT			0
#define ENC_TYPE_8BIT			1
#define ENC_TYPE_BASE64			2
#define ENC_TYPE_Q_PRINT		3

/* Global Variables */
//extern int font_charset;

// 
typedef struct _ENCODE_INFO {
	TCHAR *buf;
	char *encode_buf;
	BOOL encode;

	struct _ENCODE_INFO *next;
} ENCODE_INFO;

/* Local Function Prototypes */
static BOOL is_8bit_char_t(TCHAR *str);
static TCHAR *get_token(TCHAR *p, BOOL *encode);
static TCHAR *get_token_address(TCHAR *p, BOOL *encode);
static int get_encode_wrap_len(TCHAR *buf, int len);
static ENCODE_INFO *encode_info_create(TCHAR *buf, TCHAR *charset, BOOL Address);
static void encode_info_free(ENCODE_INFO *eb);
static int MIME_encode_size(ENCODE_INFO *eb, TCHAR *charset_t, int encoding);
static int MIME_rfc2231_encode_size(ENCODE_INFO *eb, TCHAR *charset_t);
static TCHAR *MIME_body_decode_charset(char *buf, char *ContentType);

/*
 * is_8bit_char_t - 8bit文字が含まれるかチェック
 */
static BOOL is_8bit_char_t(TCHAR *str)
{
#ifdef UNICODE
	char buf[BUF_SIZE];
	int len;
	int i;
	BOOL bret;

	len = WideCharToMultiByte(CP_ACP, 0, str, 1, NULL, 0, NULL, &bret);
	if (bret == TRUE) {
		return TRUE;
	}
	WideCharToMultiByte(CP_ACP, 0, str, 1, buf, len, NULL, NULL);
	for (i = 0; i < len; i++) {
		if ((unsigned char)*buf & (unsigned char)0x80) {
			return TRUE;
		}
	}
	return FALSE;
#else
	return (((unsigned char)*str & (unsigned char)0x80) ? TRUE : FALSE);
#endif
}

/*
 * MIME_charset_encode - 文字コードをcharsetで指定されたコードに変換
 */
char *MIME_charset_encode(const UINT cp, TCHAR *buf, TCHAR *charset)
{
	char *ret = NULL;

	if (lstrcmpi(charset, TEXT(CHARSET_US_ASCII)) == 0) {
		ret = NULL;
	} else if (lstrcmpi(charset, TEXT(CHARSET_ISO_2022_JP)) == 0) {
		// SJIS to ISO-2022-JP
#ifdef UNICODE
		char *cbuf;

		cbuf = alloc_tchar_to_char(buf);
		if (cbuf == NULL) {
			return NULL;
		}
		ret = (char *)mem_alloc(tstrlen(cbuf) * 4 + 1);
		if (ret == NULL) {
			mem_free(&cbuf);
			return NULL;
		}
		sjis_iso2022jp(cbuf, ret);
		mem_free(&cbuf);
#else
		ret = (char *)mem_alloc(tstrlen(buf) * 4 + 1);
		if (ret == NULL) {
			return NULL;
		}
		sjis_iso2022jp(buf, ret);
#endif
	} else {
#ifdef UNICODE
		ret = charset_encode(charset, buf, -1);
#else
		WCHAR *wcharset;
		WCHAR *wbuf;

		wcharset = alloc_char_to_wchar(CP_ACP, charset);
		wbuf = alloc_char_to_wchar(cp, buf);
		ret = charset_encode(wcharset, wbuf, -1);
		mem_free(&wcharset);
		mem_free(&wbuf);
#endif
	}
	return ret;
}

/*
 * MIME_charset_decode - charsetで指定された文字コードを内部コードに変換
 */
TCHAR *MIME_charset_decode(const UINT cp, char *buf, TCHAR *charset)
{
	TCHAR *ret = NULL;

	if (lstrcmpi(charset, TEXT(CHARSET_US_ASCII)) == 0) {
		ret = NULL;
	} else if (lstrcmpi(charset, TEXT(CHARSET_ISO_2022_JP)) == 0) {
		// ISO-2022-JP to SJIS
#ifdef UNICODE
		char *cret = (char *)mem_alloc(tstrlen(buf) + 1);
		if (cret == NULL) {
			return NULL;
		}
		iso2022jp_sjis(buf, cret);
		ret = alloc_char_to_tchar(cret);
		mem_free(&cret);
#else
		ret = (char *)mem_alloc(tstrlen(buf) + 1);
		if (ret == NULL) {
			return NULL;
		}
		iso2022jp_sjis(buf, ret);
#endif
	} else {
#ifdef UNICODE
		ret = charset_decode(charset, buf, -1);
#else
		WCHAR *wcharset;
		WCHAR *wret;

		wcharset = alloc_char_to_wchar(CP_ACP, charset);
		wret = charset_decode(wcharset, buf, -1);
		mem_free(&wcharset);
		ret = alloc_wchar_to_char(cp, wret);
		mem_free(&wret);
#endif
	}
	return ret;
}

/*
 * get_token - 単語取得
 */
static TCHAR *get_token(TCHAR *p, BOOL *encode)
{
	*encode = FALSE;
	for (; *p != TEXT('\0'); p++) {
		if (*encode == FALSE && is_8bit_char_t(p) == TRUE) {
			// エンコードの必要あり
			*encode = TRUE;
#ifndef UNICODE
		} else if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
			*encode = TRUE;
#endif
		} else if (*p == TEXT(' ')) {
			for (; *p == TEXT(' '); p++);
			break;
		}
	}
	return p;
}

/*
 * get_token_address - メールアドレス用の単語取得
 */
static TCHAR *get_token_address(TCHAR *p, BOOL *encode)
{
	*encode = FALSE;
	switch (*p)
	{
	case TEXT('<'):
	case TEXT('>'):
	case TEXT('('):
	case TEXT(')'):
	case TEXT(','):
		p++;
		break;

	case TEXT('\"'):
		for (p++; *p != TEXT('\0'); p++) {
			if (*encode == FALSE && is_8bit_char_t(p) == TRUE) {
				// エンコードの必要あり
				*encode = TRUE;
			}
#ifdef UNICODE
			if (WideCharToMultiByte(CP_ACP, 0, p, 1, NULL, 0, NULL, NULL) > 1) {
#else
			if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
#endif
				p++;
				*encode = TRUE;
				continue;
			}
			if (*p == TEXT('\\')) {
				p++;
				continue;
			}
			if (*p == TEXT('\"')) {
				p++;
				break;
			}
		}
		break;

	default:
		for (; *p != TEXT('\0'); p++) {
			if (*encode == FALSE && is_8bit_char_t(p) == TRUE) {
				// エンコードの必要あり
				*encode = TRUE;
			}
#ifdef UNICODE
			if (WideCharToMultiByte(CP_ACP, 0, p, 1, NULL, 0, NULL, NULL) > 1) {
#else
			if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
#endif
				p++;
				*encode = TRUE;
				continue;
			}
			if (*p == TEXT('\\')) {
				p++;
				continue;
			}
			if (*p == TEXT('<') || *p == TEXT('>') || *p == TEXT('(') || *p == TEXT(')') || *p == TEXT('\"')) {
				break;
			}
		}
		break;
	}
	return p;
}

/*
 * get_encode_wrap_len - エンコード部の折り返し位置取得
 */
static int get_encode_wrap_len(TCHAR *buf, int len)
{
	TCHAR *p;
	int i = 0;

	if ((int)lstrlen(buf) < (int)len) {
		return 0;
	}
	for (p = buf; *p != TEXT('\0'); p++, i++) {
		if (i >= len) {
			return i;
		}
#ifdef UNICODE
		if (WideCharToMultiByte(CP_ACP, 0, p, 1, NULL, 0, NULL, NULL) > 1) {
#else
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
#endif
			p++;
			i++;
		}
	}
	return 0;
}

/*
 * encode_info_create - エンコード情報の作成
 */
static ENCODE_INFO *encode_info_create(TCHAR *buf, TCHAR *charset, BOOL Address)
{
	ENCODE_INFO top_eb;
	ENCODE_INFO *eb;
	ENCODE_INFO *tmp_eb;
	TCHAR *p, *r;
	BOOL encode;
	int len;

	top_eb.next = NULL;
	eb = &top_eb;

	// エンコード情報の作成
	p = buf;
	while (*p != TEXT('\0')) {
		r = p;
		if (Address == FALSE) {
			p = get_token(r, &encode);
		} else {
			p = get_token_address(r, &encode);
		}

		eb->next = mem_calloc(sizeof(ENCODE_INFO));
		if (eb->next == NULL) {
			encode_info_free(top_eb.next);
			return NULL;
		}
		eb = eb->next;
		eb->encode = encode;

		eb->buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (p - r + 2));
		if (eb->buf == NULL) {
			encode_info_free(top_eb.next);
			return NULL;
		}
		str_cpy_n_t(eb->buf, r, p - r + 1);
	}

	// マージ
	eb = top_eb.next;
	while (eb->next != NULL) {
		if (eb->encode == eb->next->encode) {
			tmp_eb = eb->next;

			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(eb->buf) + lstrlen(tmp_eb->buf) + 1));
			if (p == NULL) {
				encode_info_free(top_eb.next);
				return NULL;
			}
			r = str_cpy_t(p, eb->buf);
			r = str_cpy_t(r, tmp_eb->buf);
			mem_free(&eb->buf);
			eb->buf = p;
			eb->next = tmp_eb->next;
			mem_free(&tmp_eb->buf);
			mem_free(&tmp_eb);
		} else {
			eb = eb->next;
		}
	}

	// 折り返し
	for (eb = top_eb.next; eb != NULL; eb = eb->next) {
		len = 0;
		if (eb->encode == TRUE) {
			len = get_encode_wrap_len(eb->buf, HEAD_ENCODE_LINELEN);
			if ((int)lstrlen(eb->buf) <= (int)len) {
				len = 0;
			}
		} else {
			if (lstrlen(eb->buf) >= HEAD_LINELEN) {
				len = HEAD_LINELEN;
			}
		}
		if (len != 0) {
			tmp_eb = eb->next;

			eb->next = mem_calloc(sizeof(ENCODE_INFO));
			if (eb->next == NULL) {
				encode_info_free(top_eb.next);
				encode_info_free(tmp_eb);
				return NULL;
			}
			eb->next->encode = eb->encode;
			eb->next->next = tmp_eb;
			eb->next->buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(eb->buf) - len + 1));
			if (eb->next->buf == NULL) {
				encode_info_free(top_eb.next);
				return NULL;
			}
			lstrcpy(eb->next->buf, eb->buf + len);

			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 2));
			if (p == NULL) {
				encode_info_free(top_eb.next);
				return NULL;
			}
			str_cpy_n_t(p, eb->buf, len + 1);
			mem_free(&eb->buf);
			eb->buf = p;
		}
	}

	// エンコード
	for (eb = top_eb.next; eb != NULL; eb = eb->next) {
		if (eb->encode == TRUE && (eb->encode_buf = MIME_charset_encode(CP_ACP, eb->buf, charset)) != NULL) {
			mem_free(&eb->buf);
		} else {
#ifdef UNICODE
			eb->encode_buf = alloc_tchar_to_char(eb->buf);
			mem_free(&eb->buf);
#else
			eb->encode_buf = eb->buf;
#endif
		}
		eb->buf = NULL;
	}
	return top_eb.next;
}

/*
 * encode_info_free - エンコード情報の解放
 */
static void encode_info_free(ENCODE_INFO *eb)
{
	if (eb == NULL) {
		return;
	}
	encode_info_free(eb->next);

	mem_free(&eb->buf);
	mem_free(&eb->encode_buf);
	mem_free(&eb);
}

/*
 * MIME_encode_size - MIMEエンコードしたサイズの取得
 */
static int MIME_encode_size(ENCODE_INFO *eb, TCHAR *charset_t, int encoding)
{
	int i;
	int len = 0;

	for (; eb != NULL; eb = eb->next) {
		if (eb->encode_buf == NULL) {
			continue;
		}
		len += 3;			// "\r\n "

		if (eb->encode == FALSE || encoding == ENC_TYPE_7BIT || encoding == ENC_TYPE_8BIT) {
			len += tstrlen(eb->encode_buf);
		} else {
			i = tstrlen(eb->encode_buf);
			switch (encoding)
			{
			case ENC_TYPE_BASE64:
			default:
				// Base64
				i = i * 2 + 4;
				break;

			case ENC_TYPE_Q_PRINT:
				// quoted-printable
				i = i * 3;
				break;
			}
			len += 2;		// =?
			len += lstrlen(charset_t);
			len++;			// ?
			len++;			// B or Q
			len++;			// ?
			len += i;
			len += 2;		// ?=
		}
	}
	return len;
}

/*
 * MIME_encode - MIMEエンコード (RFC 2047, RFC 2045)
 */
TCHAR *MIME_encode(TCHAR *wbuf, BOOL Address, TCHAR *charset_t, int encoding)
{
	ENCODE_INFO *top_eb, *eb;
#ifdef UNICODE
	TCHAR *ret;
#endif
	char *cret;
	char *EncType;
	char *charset;
	char *tmp;
	char *r;
	int i;

#ifdef UNICODE
	// char に変換
	charset = alloc_tchar_to_char(charset_t);
	if (charset == NULL) {
		return NULL;
	}
#else
	charset = charset_t;
#endif

	// エンコード情報の作成
	top_eb = eb = encode_info_create(wbuf, charset_t, Address);
	if (top_eb == NULL) {
#ifdef UNICODE
		mem_free(&charset);
#endif
		return NULL;
	}

	// エンコード後のバッファ確保
	i = MIME_encode_size(eb, charset_t, encoding);
	cret = (char *)mem_alloc(i + 1);
	if (cret == NULL) {
		encode_info_free(top_eb);
#ifdef UNICODE
		mem_free(&charset);
#endif
		return NULL;
	}
	r = cret;

	for (; eb != NULL; eb = eb->next) {
		if (eb->encode_buf == NULL) {
			continue;
		}
		if (top_eb != eb) {
			r = str_cpy(r, "\r\n ");
		}
		if (eb->encode == FALSE || encoding == ENC_TYPE_7BIT || encoding == ENC_TYPE_8BIT) {
			r = str_cpy(r, eb->encode_buf);
		} else {
			switch (encoding)
			{
			case ENC_TYPE_BASE64:
			default:
				// Base64
				tmp = (char *)mem_alloc(tstrlen(eb->encode_buf) * 2 + 4 + 1);
				if (tmp == NULL) {
					encode_info_free(top_eb);
					mem_free(&cret);
#ifdef UNICODE
					mem_free(&charset);
#endif
					return NULL;
				}
				base64_encode(eb->encode_buf, tmp, 0);
				mem_free(&eb->encode_buf);
				eb->encode_buf = tmp;
				EncType = "B";
				break;

			case ENC_TYPE_Q_PRINT:
				// quoted-printable
				tmp = (char *)mem_alloc(tstrlen(eb->encode_buf) * 3 + 1);
				if (tmp == NULL) {
					encode_info_free(top_eb);
					mem_free(&cret);
#ifdef UNICODE
					mem_free(&charset);
#endif
					return NULL;
				}
				QuotedPrintable_encode(eb->encode_buf, tmp, 0, FALSE);
				mem_free(&eb->encode_buf);
				eb->encode_buf = tmp;
				EncType = "Q";
				break;
			}
			r = str_cpy(r, "=?");
			r = str_cpy(r, charset);
			r = str_cpy(r, "?");
			r = str_cpy(r, EncType);
			r = str_cpy(r, "?");
			r = str_cpy(r, eb->encode_buf);
			r = str_cpy(r, "?=");
		}
	}
	*r = '\0';

	encode_info_free(top_eb);

#ifdef UNICODE
	mem_free(&charset);

	// TCHAR に変換
	ret = alloc_char_to_tchar(cret);
	if (ret == NULL) {
		mem_free(&cret);
		return NULL;
	}
	mem_free(&cret);
	return ret;
#else
	return cret;
#endif
}

/*
 * MIME_decode - MIMEのデコード (RFC 2047, RFC 2045)
 *
 * =?charset?B?content?=
 * =?charset?Q?content?=
 */
int MIME_decode(char *buf, TCHAR *ret)
{
	char *p, *t;
	TCHAR *r, *s;
	char *non_enc_pt;
	char *enc_st;
	char *charset_st, *charset_en;
	TCHAR *charset;
	char *encoding;
	char *content;
	char *dec_buf;
	TCHAR *retbuf;
	BOOL jis_flag = FALSE;
	BOOL enc_flag = FALSE;
	BOOL enc_err = FALSE;
	int ret_len = 0;

	p = buf;
	r = ret;
	non_enc_pt = p;
	while (*p != '\0') {
		if (*p != '=' || *(p + 1) != '?' || enc_err == TRUE) {
			enc_err = FALSE;
			if (jis_flag == FALSE &&
				*p == ESC && *(p + 1) == '$' && (*(p + 2) == '@' || *(p + 2) == 'B')) {
				// JISコードが含まれている
				jis_flag = TRUE;
			}
			if (enc_flag == TRUE &&
				*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
				enc_flag = FALSE;
			}
			p++;
			continue;
		}
		if (enc_flag == FALSE && p != non_enc_pt) {
			if (ret != NULL) {
				// コピー
#ifdef UNICODE
				MultiByteToWideChar(CP_ACP, 0, non_enc_pt, (p - non_enc_pt), r, (p - non_enc_pt));
#else
				str_cpy_n_t(r, non_enc_pt, (p - non_enc_pt) + 1);
#endif
				r += p - non_enc_pt;
			}
			ret_len += p - non_enc_pt;
			non_enc_pt = p;
		}
		enc_st = p;
		p += 2;

		// 文字セット
		charset_st = p;
		for (; *p != '\0' && *p != '?'; p++);
		if (*p == '\0') {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
		charset_en = p;
		p++;
		// エンコード方法
		encoding = p;
		for (; *p != '\0' && *p != '?'; p++);
		if (*p == '\0') {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
		p++;
		// エンコード終了位置
		for (t = p; *t != '\0' && !(*t == '?' && *(t + 1) == '='); t++);
		if (*t == '\0') {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
		// エンコード部分のコピー
		content = (char *)mem_alloc(sizeof(char) * (t - p + 1));
		if (content == NULL) {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
		str_cpy_n(content, p, (t - p) + 1);
		// デコード
		switch (*encoding) {
		case 'b': case 'B':
			dec_buf = (char *)mem_alloc(sizeof(char) * (t - p + 1));
			if (dec_buf == NULL) {
				p = enc_st;
				enc_err = TRUE;
				continue;
			}
			base64_decode(content, dec_buf);
			mem_free(&content);
			break;
		case 'q': case 'Q':
			dec_buf = (char *)mem_alloc(sizeof(char) * (t - p + 1));
			if (dec_buf == NULL) {
				p = enc_st;
				enc_err = TRUE;
				continue;
			}
			QuotedPrintable_decode(content, dec_buf);
			mem_free(&content);
			break;
		default:
			dec_buf = content;
			break;
		}
		p = t + 2;

		// 文字セットのコピー
		charset = mem_alloc(sizeof(TCHAR) * ((charset_en - charset_st) + 1));
		if (charset == NULL) {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, charset_st, (charset_en - charset_st), charset, (charset_en - charset_st));
		*(charset + (charset_en - charset_st)) = TEXT('\0');
#else
		str_cpy_n_t(charset, charset_st, (charset_en - charset_st) + 1);
#endif
		// 文字コードの変換
		retbuf = MIME_charset_decode(CP_ACP, dec_buf, charset);
		if (retbuf != NULL) {
			mem_free(&dec_buf);
		} else {
#ifdef UNICODE
			retbuf = alloc_char_to_tchar(dec_buf);
			mem_free(&dec_buf);
#else
			retbuf = dec_buf;
#endif
		}
		mem_free(&charset);

		// 文字列のコピー
		for (s = retbuf; *s != TEXT('\0'); s++) {
#ifndef UNICODE
			if (IsDBCSLeadByte((BYTE)*s) == TRUE && *(s + 1) != TEXT('\0')) {
				if (ret != NULL) {
					*(r++) = *(s++);
					*(r++) = *s;
				}
				ret_len += 2;
				continue;
			}
#endif
			if (*s != TEXT('\r') && *s != TEXT('\n')) {
				if (ret != NULL) {
					*(r++) = *s;
				}
				ret_len++;
			}
		}
		mem_free(&retbuf);

		enc_flag = TRUE;
		non_enc_pt = p;
	}
	if (enc_flag == FALSE && p != non_enc_pt) {
		if (ret != NULL) {
			// コピー
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0, non_enc_pt, (p - non_enc_pt), r, (p - non_enc_pt));
#else
			str_cpy_n_t(r, non_enc_pt, (p - non_enc_pt) + 1);
#endif
			r += p - non_enc_pt;
		}
		ret_len += p - non_enc_pt;
	}
	if (ret == NULL) {
		return ret_len;
	}
	*r = TEXT('\0');

	if (jis_flag == TRUE) {
		// JISの場合はJIS->SJISを行う
#ifdef UNICODE
		char *cret, *cbuf;
		int len;

		cbuf = alloc_tchar_to_char(ret);
		len = tstrlen(cbuf);
		if (cbuf != NULL) {
			cret = mem_alloc(len + 1);
			if (cret != NULL) {
				iso2022jp_sjis(cbuf, cret);
				char_to_tchar(cret, ret, tstrlen(cret));
				mem_free(&cret);
			}
			mem_free(&cbuf);
		}
#else
		char *tmp;
		tmp = (char *)mem_alloc(tstrlen(ret) + 1);
		if (tmp != NULL) {
			iso2022jp_sjis(ret, tmp);
			tstrcpy(ret, tmp);
			mem_free(&tmp);
		}
#endif
	}
	return ret_len;
}

/*
 * MIME_rfc2231_encode_size - ファイル名のエンコードのサイズ (RFC 2231)
 */
static int MIME_rfc2231_encode_size(ENCODE_INFO *eb, TCHAR *charset_t)
{
	int len = 0;

	for (; eb != NULL; eb = eb->next) {
		len += tstrlen(eb->encode_buf) * 3;
		len += (lstrlen(TEXT("\r\n filename**=;")) + 5);
		if (eb->next != NULL) {
			len++;			// ;
		}
	}
	len += lstrlen(charset_t) + 2;
	return len;
}

/*
 * MIME_rfc2231_encode - ファイル名のエンコード (RFC 2231)
 */
TCHAR *MIME_rfc2231_encode(TCHAR *wbuf, TCHAR *charset_t)
{
	ENCODE_INFO *top_eb, *eb;
	TCHAR *ret;
	TCHAR *r, *t;
	char *tmp;
	TCHAR *p;
	int Len;
	int cnt = 0;
	int i;

	for (p = wbuf; *p != TEXT('\0'); p++) {
		if (is_8bit_char_t(p) == TRUE) {
			break;
		}
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
		}
#endif
	}

	if (*p == TEXT('\0')) {
		i = (lstrlen(TEXT("\r\n filename=\"\"")) + lstrlen(wbuf));
		ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (i + 1));
		if (ret != NULL) {
			wsprintf(ret, TEXT("\r\n filename=\"%s\""), wbuf);
		}
		return ret;
	}

	// エンコード情報の作成
	top_eb = eb = encode_info_create(wbuf, charset_t, FALSE);
	if (top_eb == NULL) {
		return NULL;
	}

	// エンコード後のバッファ確保
	i = MIME_rfc2231_encode_size(eb, charset_t);
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (i + 1));
	if (ret == NULL) {
		encode_info_free(top_eb);
		return NULL;
	}
	r = ret;

	for (; eb != NULL; eb = eb->next) {
		// URLエンコード
		tmp = (char *)mem_alloc(tstrlen(eb->encode_buf) * 3 + 1);
		if (tmp == NULL) {
			encode_info_free(top_eb);
			return NULL;
		}
		URL_encode(eb->encode_buf, tmp);
		mem_free(&eb->encode_buf);
		eb->encode_buf = tmp;

#ifdef UNICODE
		// TCHAR に変換
		t = alloc_char_to_tchar(eb->encode_buf);
		if (t == NULL) {
			encode_info_free(top_eb);
			return NULL;
		}
#else
		t = eb->encode_buf;
#endif
		if (cnt == 0) {
			Len = wsprintf(r, TEXT("\r\n filename*%d*=%s''%s"), cnt, charset_t, t);
		} else {
			Len = wsprintf(r, TEXT("\r\n filename*%d*=%s"), cnt, t);
		}
#ifdef UNICODE
		mem_free(&t);
#endif
		r += Len;
		cnt++;

		if (eb->next != NULL) {
			*(r++) = TEXT(';');
		}
	}
	encode_info_free(top_eb);
	return ret;
}

/*
 * MIME_rfc2231_decode - ファイル名のデコード (RFC 2231)
 */
char *MIME_rfc2231_decode(char *buf)
{
#ifdef UNICODE
	TCHAR *wcharset;
	TCHAR *wret;
#endif
	char *charset;
	char *lang = NULL;
	char *r;
	char *ret;
	char *tmp;

	charset = (char *)mem_alloc(sizeof(char) * (tstrlen(buf) + 1));
	if (charset == NULL) {
		return NULL;
	}
	r = str_cpy_f(charset, buf, '\'');
	if (*r != '\0') {
		lang = (char *)mem_alloc(sizeof(char) * (tstrlen(r) + 1));
		if (lang == NULL) {
			mem_free(&charset);
			return NULL;
		}
		r = str_cpy_f(lang, r, '\'');
	}
	if (*r == '\0') {
		r = buf;
		*charset = '\0';
	}

	// URLデコード
	tmp = (char *)mem_alloc(tstrlen(r) + 1);
	if (tmp == NULL) {
		mem_free(&charset);
		mem_free(&lang);
		return NULL;
	}
	URL_decode(r, tmp);

	// キャラクタセットの変換
#ifdef UNICODE
	wcharset = alloc_char_to_tchar(charset);
	if (wcharset == NULL) {
		mem_free(&charset);
		mem_free(&lang);
		return NULL;
	}
	wret = MIME_charset_decode(CP_ACP, tmp, wcharset);
	mem_free(&wcharset);
	ret = alloc_tchar_to_char(wret);
	mem_free(&wret);
#else
	ret = MIME_charset_decode(CP_ACP, tmp, charset);
#endif
	if (ret != NULL) {
		mem_free(&tmp);
	} else {
		ret = tmp;
	}
	mem_free(&charset);
	mem_free(&lang);
	return ret;
}

/*
 * MIME_create_encode_header - エンコードヘッダ作成
 */
BOOL MIME_create_encode_header(TCHAR *charset, int encoding, char *ret_content_type, char *ret_encoding)
{
#ifdef UNICODE
	TCHAR buf[BUF_SIZE];

	// Content-Type
	wsprintf(buf, TEXT("text/plain; charset=\"%s\""), charset);
	tchar_to_char(buf, ret_content_type, BUF_SIZE);
#else
	// Content-Type
	wsprintf(ret_content_type, TEXT("text/plain; charset=\"%s\""), charset);
#endif

	// Content-Transfer-Encoding
	switch (encoding) {
	case ENC_TYPE_7BIT:
	default:
		tstrcpy(ret_encoding, ENCODE_7BIT);
		break;

	case ENC_TYPE_8BIT:
		tstrcpy(ret_encoding, ENCODE_8BIT);
		break;

	case ENC_TYPE_BASE64:
		// Base64
		tstrcpy(ret_encoding, ENCODE_BASE64);
		break;

	case ENC_TYPE_Q_PRINT:
		// quoted-printable
		tstrcpy(ret_encoding, ENCODE_Q_PRINT);
		break;
	}
	return TRUE;
}

/*
 * MIME_body_encode - 本文のエンコード (RFC 822, RFC 2822, RFC 2045)
 */
char *MIME_body_encode(TCHAR *body, TCHAR *charset_t, int encoding, char *ret_content_type, char *ret_encoding, TCHAR *ErrStr)
{
	TCHAR *buf;
	char *ret;
	TCHAR *p;
	char *cret;
	char *tmp;
	char *cp, *cr;
	int i;
	BOOL encode = FALSE;

	buf = (body != NULL) ? body : TEXT("");
	for (p = buf; *p != TEXT('\0'); p++) {
		if (is_8bit_char_t(p) == TRUE) {
			encode = TRUE;
			break;
		}
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
		}
#endif
	}

	if (encode == FALSE) {
		// US-ASCII
		MIME_create_encode_header(TEXT(CHARSET_US_ASCII), 0, ret_content_type, ret_encoding);
		ret = alloc_tchar_to_char(buf);
	} 
	else 
	{
		MIME_create_encode_header(charset_t, encoding, ret_content_type, ret_encoding);

		// charset
		cret = MIME_charset_encode(charset_to_cp( ( BYTE )FONT_CHARSET ), buf, charset_t );
		
		if( cret == NULL )
		{
#ifdef UNICODE
			cret = alloc_tchar_to_char(buf);
#else
			cret = alloc_copy_t(buf);
#endif
		}

		switch( encoding )
		{
		case ENC_TYPE_7BIT:
		case ENC_TYPE_8BIT:
		default:
			break;

		case ENC_TYPE_BASE64:
			// Base64
			tmp = (char *)mem_alloc(tstrlen(cret) * 2 + 4 + 1);
			if (tmp == NULL) {
				mem_free(&cret);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}
			base64_encode(cret, tmp, 0);
			mem_free(&cret);

			// 折り返し
			cret = (char *)mem_alloc(tstrlen(tmp) + (tstrlen(tmp) / BODY_ENCODE_LINELEN * 2) + 1);
			if (cret == NULL) {
				mem_free(&tmp);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}
			for (cp = tmp, cr = cret, i = 0; *cp != '\0'; cp++, i++) {
				if (i >= BODY_ENCODE_LINELEN) {
					i = 0;
					*(cr++) = '\r';
					*(cr++) = '\n';
				}
				*(cr++) = *cp;
			}
			*cr = '\0';
			mem_free(&tmp);
			break;

		case ENC_TYPE_Q_PRINT:
			// quoted-printable
			tmp = (char *)mem_alloc(tstrlen(cret) * 4 + 1);
			if (tmp == NULL) {
				mem_free(&cret);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}
			QuotedPrintable_encode(cret, tmp, BODY_ENCODE_LINELEN, TRUE);
			mem_free(&cret);
			cret = tmp;
			break;
		}
		ret = cret;
	}
	return ret;
}

/*
 * MIME_body_decode_charset - ContentTypeで指定された文字コードを内部コードに変換
 */
static TCHAR *MIME_body_decode_charset(char *buf, char *ContentType)
{
	TCHAR *charset;
	TCHAR *p;
	char *r;
	TCHAR *ret = NULL;

	r = ContentType;
	while (r != NULL && *r != '\0') {
		for (; *r == ' '; r++);
		if (str_cmp_ni(r, "charset", tstrlen("charset")) != 0) {
			for (; *r != '\0' && *r != ';'; r++);
			if (*r != '\0') {
				r++;
			}
			continue;
		}
		r += tstrlen("charset");
		for (; *r == ' '; r++);
		if (*r != '=') {
			break;
		}
		r++;
		for (; *r == ' '; r++);
		if (*r == '\"') {
			r++;
		}
		for (; *r == TEXT(' '); r++);

		charset = alloc_char_to_tchar(r);
		if (charset == NULL) {
			return NULL;
		}
		for (p = charset; *p != TEXT('\0') && *p != TEXT('\"') && *p != TEXT(';'); p++);
		*p = TEXT('\0');

		ret = MIME_charset_decode(charset_to_cp((BYTE)FONT_CHARSET), buf, charset);
		if (ret != NULL) {
			mem_free(&charset);
			return ret;
		}
		mem_free(&charset);
		break;
	}
	if (lstrcmpi(STR_DEFAULT_BODY_CHARSET, TEXT(CHARSET_ISO_2022_JP)) == 0) {
#ifdef UNICODE
		char *cret;

		cret = mem_alloc(tstrlen(buf) + 1);
		if (cret != NULL) {
			iso2022jp_sjis(buf, cret);
		}
		ret = alloc_char_to_tchar(cret);
		mem_free(&cret);
#else
		ret = mem_alloc(tstrlen(buf) + 1);
		if (ret != NULL) {
			iso2022jp_sjis(buf, ret);
		}
#endif
	} else {
		ret = alloc_char_to_tchar(buf);
	}
	return ret;
}

/*
 * MIME_body_decode_transfer - Content-Transfer-Encoding にしたがってデコード
 */
char *MIME_body_decode_transfer(MAILITEM *mail_item, char *body)
{
	char *enc_buf, *enc_ret = NULL;
	int encode = 0;

	enc_buf = (char *)mem_alloc(tstrlen(body) + 1);
	if (enc_buf == NULL) {
		return NULL;
	}
	tstrcpy(enc_buf, body);

	if (mail_item->Encoding == NULL || mail_item->ContentType == NULL ||
		str_cmp_ni_t(mail_item->ContentType, TEXT("text"), lstrlen(TEXT("text"))) != 0) {
		// テキストではない
		return enc_buf;
	}

	// デコード
	if (str_cmp_ni_t(mail_item->Encoding, TEXT(ENCODE_BASE64), lstrlen(TEXT(ENCODE_BASE64))) == 0) {
		encode = ENC_TYPE_BASE64;
	} else if (str_cmp_ni_t(mail_item->Encoding, TEXT(ENCODE_Q_PRINT), lstrlen(TEXT(ENCODE_Q_PRINT))) == 0) {
		encode = ENC_TYPE_Q_PRINT;
	}
	if (encode != 0) {
		enc_ret = (char *)mem_alloc(tstrlen(enc_buf) + 1);
		if (enc_ret != NULL) {
			((encode == ENC_TYPE_BASE64) ? base64_decode : QuotedPrintable_decode)(enc_buf, enc_ret);
			mem_free(&enc_buf);
			enc_buf = enc_ret;
		}
	}
	mem_free(&mail_item->Encoding);
	mail_item->Encoding = NULL;
	return enc_buf;
}

/*
 * MIME_body_decode - 本文のデコード (RFC 822, RFC 2822, RFC 2045)
 */
TCHAR *MIME_body_decode(MAILITEM *mail_item, BOOL ViewSrc, MULTIPART ***tpPart, int *cnt)
{
#ifdef UNICODE
	char *ContentType;
#endif
	char *body = NULL;
	TCHAR *buf;
	TCHAR *r;
	TCHAR *wenc_ret = NULL;
	char *enc_ret = NULL;
	int encode = 0;
	int i;
	int TextIndex = -1;

	*cnt = 0;
	if (mail_item->Multipart == TRUE && ViewSrc == FALSE) {
		// マルチパートを解析する
#ifdef UNICODE
		ContentType = alloc_tchar_to_char(mail_item->ContentType);
		*cnt = multipart_parse(ContentType, mail_item->Body, tpPart, 0);
		mem_free(&ContentType);
#else
		*cnt = multipart_parse(mail_item->ContentType, mail_item->Body, tpPart, 0);
#endif
	}

	if (*cnt == 0 && multipart_add(tpPart, 0) != NULL) {
		// マルチパートではない or ソース表示
		if (ViewSrc == FALSE) {
			(**tpPart)->ContentType = alloc_tchar_to_char(mail_item->ContentType);
			(**tpPart)->Encoding = alloc_tchar_to_char(mail_item->Encoding);
		}
		(**tpPart)->sPos = mail_item->Body;
		if (ViewSrc == TRUE || (**tpPart)->ContentType == NULL ||
			str_cmp_ni((**tpPart)->ContentType, "text", tstrlen("text")) == 0) {
			// テキスト
			TextIndex = 0;
		} else {
			// ファイル名の取得
#ifdef UNICODE
			ContentType = alloc_tchar_to_char(mail_item->ContentType);
			(**tpPart)->Filename = multipart_get_filename(ContentType, "name");
			mem_free(&ContentType);
#else
			(**tpPart)->Filename = multipart_get_filename(mail_item->ContentType, "name");
#endif
		}
		*cnt = 1;
	} else {
		// テキストのパートを検索
		for (i = 0; i < *cnt; i++) {
			if ((*(*tpPart + i))->ContentType == NULL ||
				str_cmp_ni((*(*tpPart + i))->ContentType, "text", tstrlen("text")) == 0) {
				TextIndex = i;
				break;
			}
		}
	}

	if (*cnt > 0 && TextIndex != -1) {
		// 本文の取得
		if ((*(*tpPart + TextIndex))->ePos == NULL) {
			body = alloc_copy((*(*tpPart + TextIndex))->sPos);
			if (body == NULL) {
				return NULL;
			}
		} else {
			i = (*(*tpPart + TextIndex))->ePos - (*(*tpPart + TextIndex))->sPos;
			body = (char *)mem_alloc(sizeof(char) * (i + 1));
			if (body == NULL) {
				return NULL;
			}
			if (i == 0) {
				*body = '\0';
			} else {
				str_cpy_n(body, (*(*tpPart + TextIndex))->sPos, i - 1);
			}
		}

		// デコード
		if ((*(*tpPart + TextIndex))->Encoding != NULL) {
			if (str_cmp_i((*(*tpPart + TextIndex))->Encoding, ENCODE_BASE64) == 0) {
				encode = ENC_TYPE_BASE64;
			} else if (str_cmp_i((*(*tpPart + TextIndex))->Encoding, ENCODE_Q_PRINT) == 0) {
				encode = ENC_TYPE_Q_PRINT;
			}
		}
		if (encode != 0) {
			enc_ret = (char *)mem_alloc(tstrlen(body) + 1);
			if (enc_ret != NULL) {
				((encode == ENC_TYPE_BASE64) ? base64_decode : QuotedPrintable_decode)(body, enc_ret);
				mem_free(&body);
				body = enc_ret;
			}
		}

		// キャラクタセットの変換
		wenc_ret = MIME_body_decode_charset(body, (*(*tpPart + TextIndex))->ContentType);
		if (wenc_ret == NULL) {
#ifdef UNICODE
			wenc_ret = alloc_char_to_tchar(body);
			mem_free(&body);
#else
			wenc_ret = body;
#endif
		} else {
			mem_free(&body);
		}
	}

	if (wenc_ret == NULL) {
#ifdef UNICODE
		r = wenc_ret = alloc_char_to_tchar(mail_item->Body);
#else
		r = mail_item->Body;
#endif
	} else {
		r = wenc_ret;
	}
	if (r != NULL) {
		buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(r) + 1));
		if (buf != NULL) {
			DelDot(r, buf);
			mem_free(&wenc_ret);
			r = buf;
		}
	}
	return r;
}
/* End of source */
