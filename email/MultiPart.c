/*
 * nPOP
 *
 * Multipart.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include <stdio.h>

#include "General.h"
#include "memory_mng.h"
#include "string_mng.h"
#include "code.h"
#include "mime.h"
#include "multipart.h"

/* Define */

/* Global Variables */
//extern OPTION op;

/* Local Function Prototypes */
static char *get_next_part(char *buf, char *Boundary);
static void get_content(char *buf, char *str, char **ret);
static BOOL get_content_value(char *Content, char *Attribute, char *ret);
static char *multipart_get_filename_rfc2231(char *buf);

/*
 * multipart_add - Part情報の追加
 */
MULTIPART *multipart_add(MULTIPART ***tpMultiPart, int cnt)
{
	MULTIPART **TmpMultiPart;

	TmpMultiPart = (MULTIPART **)mem_calloc(sizeof(MULTIPART *) * (cnt + 1));
	if (TmpMultiPart == NULL) {
		return NULL;
	}
	if (*tpMultiPart != NULL) {
		CopyMemory(TmpMultiPart, *tpMultiPart,
			sizeof(MULTIPART *) * cnt);
		mem_free((void **)&*tpMultiPart);
	}
	*(TmpMultiPart + cnt) = (MULTIPART *)mem_calloc(sizeof(MULTIPART));
	*tpMultiPart = TmpMultiPart;
	return *(TmpMultiPart + cnt);
}

/*
 * multipart_free - Part情報の解放
 */
void multipart_free(MULTIPART ***tpMultiPart, int cnt)
{
	int i;

	// Part情報の解放
	for (i = 0; i < cnt; i++) {
		if (*(*tpMultiPart + i) == NULL) {
			continue;
		}
		mem_free(&(*(*tpMultiPart + i))->ContentType);
		mem_free(&(*(*tpMultiPart + i))->Filename);
		mem_free(&(*(*tpMultiPart + i))->Encoding);

		mem_free(&*(*tpMultiPart + i));
	}
	mem_free((void **)&*tpMultiPart);
}

/*
 * get_next_part - 次のPartの位置を取得
 */
static char *get_next_part(char *buf, char *Boundary)
{
	char *p = buf;

	if (*p == '-' && *(p + 1) == '-' && str_cmp_ni(p + 2, Boundary, tstrlen(Boundary)) == 0) {
		return p;
	}
	while (1) {
		for (; !(*p == '\r' && *(p + 1) == '\n') && *p != '\0'; p++);
		if (*p == '\0') {
			break;
		}
		p += 2;
		if (!(*p == '-' && *(p + 1) == '-')) {
			continue;
		}
		if (str_cmp_ni(p + 2, Boundary, tstrlen(Boundary)) != 0) {
			continue;
		}
		break;
	}
	return p;
}

/*
 * get_content - コンテンツの取得
 */
static void get_content(char *buf, char *str, char **ret)
{
	char *p;
	int len;

	// 位置の取得
	p = GetHeaderStringPoint(buf, str);
	if (p == NULL) {
		*ret = NULL;
		return;
	}
	// サイズの取得
	len = GetHeaderStringSize(p, FALSE);
	*ret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (*ret == NULL) {
		return;
	}
	GetHeaderString(p, *ret, FALSE);
}

/*
 * get_content_value - コンテンツ内の指定した値を取得
 */
static BOOL get_content_value(char *Content, char *Attribute, char *ret)
{
	char *p = Content;
	char *r = ret;

	for (; *p == ' ' || *p == '\t'; p++);

	while (*p != '\0') {
		// attributeをチェック
		if (str_cmp_ni(p, Attribute, tstrlen(Attribute)) != 0) {
			// 次のparameterに移動
			for (; *p != '\0' && *p != ';'; p++);
			if (*p == ';') p++;
			for (; *p == ' ' || *p == '\t'; p++);
			continue;
		}
		p += tstrlen(Attribute);
		for (; *p == ' ' || *p == '\t'; p++);
		if (*p == '=') {
			p++;
		}
		for (; *p == ' ' || *p == '\t'; p++);
		if (*p == '\"') {
			p++;
		}
		// valueの取得
		for (; *p != '\0' && *p != '\"' && *p != ';'; p++, r++) {
			*r = *p;
		}
		*r = '\0';
		return TRUE;
	}
	return FALSE;
}

/*
 * multipart_get_filename_rfc2231 - ファイル名の取得 (RFC 2231)
 */
static char *multipart_get_filename_rfc2231(char *buf)
{
	char *p = buf, *r;
	char *tmp;
	char *ret;
	char **Names = NULL, **NamesTmp;
	int No;
	int AllocNo = 0;
	int len;
	BOOL EncFlag = FALSE;

	if (buf == NULL) {
		return NULL;
	}

	while (*p != '\0') {
		// attributeをチェック
		if (str_cmp_ni(p, "filename*", tstrlen("filename*")) != 0) {
			// 次のparameterに移動
			for (; *p != '\0' && *p != ';'; p++);
			if (*p == ';') p++;
			for (; *p == ' ' || *p == '\t'; p++);
			continue;
		}
		p += tstrlen("filename");
		if (*(p + 1) == '=') {
			// 単一の場合
			No = 0;
		} else {
			// 連番が付いている場合
			p++;
			No = a2i(p);
		}
		if (AllocNo <= No) {
			// ファイル名のリストの確保
			NamesTmp = (char **)mem_calloc(sizeof(char *) * (No + 1));
			if (NamesTmp == NULL) {
				break;
			}
			if (Names != NULL) {
				CopyMemory(NamesTmp, Names, sizeof(char *) * AllocNo);
				mem_free((void **)&Names);
			}
			Names = NamesTmp;
			AllocNo = No + 1;
		}
		for (; *p != '\0' && *p != '*' && *p != '='; p++);
		if (No == 0) {
			EncFlag = (*p == '*') ? TRUE : FALSE;
		}
		for (; *p == '*' || *p == '=' || *p == '\"'; p++);

		// valueの取得
		for (r = p; *p != '\0' && *p != '\"' && *p != ';'; p++);
		tmp = (char *)mem_alloc(sizeof(char) * (p - r + 1));
		if (tmp == NULL) {
			break;
		}
		str_cpy_n(tmp, r, p - r + 1);

		*(Names + No) = (char *)mem_alloc(sizeof(char) * (tstrlen(tmp) + 1));
		if (*(Names + No) == NULL) {
			mem_free(&tmp);
			break;
		}
		tstrcpy(*(Names + No), tmp);
		mem_free(&tmp);
	}
	if (Names == NULL) {
		return NULL;
	}

	// ファイル名の長さを計算
	for (No = 0, len = 0; No < AllocNo; No++) {
		if (*(Names + No) != NULL) {
			len += tstrlen(*(Names + No));
		}
	}
	ret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (ret != NULL) {
		// ファイル名を連結
		for (No = 0, r = ret; No < AllocNo; No++) {
			if (*(Names + No) != NULL) {
				r = str_cpy(r, *(Names + No));
			}
		}
		// デコード
		if (EncFlag == TRUE) {
			tmp = MIME_rfc2231_decode(ret);
			if (tmp != NULL) {
				mem_free(&ret);
				ret = tmp;
			}
		}
	}
	// ファイル名のリストの解放
	for (No = 0; No < AllocNo; No++) {
		if (*(Names + No) != NULL) {
			mem_free(&*(Names + No));
		}
	}
	mem_free((void **)&Names);
	return ret;
}

/*
 * multipart_get_filename - ファイル名の取得
 */
char *multipart_get_filename(char *buf, char *Attribute)
{
	char *p = buf;
	char *fname, *dname;
#ifdef UNICODE
	TCHAR *wdname;
#endif
	int len;

	if (buf == NULL) {
		return NULL;
	}

	fname = (char *)mem_alloc(sizeof(char) * (tstrlen(buf) + 1));
	if (fname == NULL) {
		return NULL;
	}
	if (get_content_value(buf, Attribute, fname) == FALSE) {
		mem_free(&fname);
		return NULL;
	}
	len = MIME_decode(fname, NULL);
#ifdef UNICODE
	wdname = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (wdname == NULL) {
		mem_free(&fname);
		return NULL;
	}
	MIME_decode(fname, wdname);
	mem_free(&fname);
	dname = alloc_tchar_to_char(wdname);
	mem_free(&wdname);
#else
	dname = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (dname == NULL) {
		mem_free(&fname);
		return NULL;
	}
	MIME_decode(fname, dname);
	mem_free(&fname);
#endif
	return dname;
}

/*
 * multipart_parse - Partを解析する (RFC 2046)
 */
int multipart_parse(char *ContentType, char *buf, MULTIPART ***tpMultiPart, int cnt)
{
	MULTIPART *tpMultiPartItem;
	char *Boundary;
	char *p;
	char *Content, *sPos;

	if (ContentType == NULL || buf == NULL) {
		return 0;
	}

	// boundaryの取得
	Boundary = (char *)mem_alloc(sizeof(char) * (tstrlen(ContentType) + 1));
	if (Boundary == NULL) {
		return 0;
	}
	if (get_content_value(ContentType, "boundary", Boundary) == FALSE) {
		mem_free(&Boundary);
		return 0;
	}

	// partの位置の取得
	p = get_next_part(buf, Boundary);

	while (1) {
		if (*p == '\0') {
			break;
		}
		p += (2 + tstrlen(Boundary));
		// パーツの終わりかチェック
		if (*p == '-' && *(p + 1) == '-') {
			break;
		}

		get_content(p, HEAD_CONTENTTYPE, &Content);
		if (Content != NULL &&
			str_cmp_ni(Content, "multipart", tstrlen("multipart")) == 0) {
			// 階層になっている場合は再帰する
			sPos = GetBodyPointa(p);
			cnt = multipart_parse(Content, sPos, tpMultiPart, cnt);
			mem_free(&Content);
			p = get_next_part(sPos, Boundary);
			continue;
		}

		// マルチパート情報の追加
		if ((tpMultiPartItem = multipart_add(tpMultiPart, cnt)) == NULL) {
			break;
		}
		cnt++;

		// ヘッダを取得
		tpMultiPartItem->ContentType = Content;
		get_content(p, HEAD_ENCODING, &tpMultiPartItem->Encoding);
		get_content(p, HEAD_DISPOSITION, &Content);

		// ファイル名の取得
		if ((tpMultiPartItem->Filename = multipart_get_filename_rfc2231(Content)) == NULL &&
			(tpMultiPartItem->Filename = multipart_get_filename(Content, "filename")) == NULL &&
			(tpMultiPartItem->Filename = multipart_get_filename(tpMultiPartItem->ContentType, "name")) == NULL) {
		}
		mem_free(&Content);

		// 本文の位置の取得
		tpMultiPartItem->sPos = GetBodyPointa(p);
		if (tpMultiPartItem->sPos == NULL) {
			break;
		}
		// 次のpartの位置を取得
		p = get_next_part(tpMultiPartItem->sPos, Boundary);
		if (*p != '\0') {
			tpMultiPartItem->ePos = p;
		}
	}
	mem_free(&Boundary);
	return cnt;
}

/*
 * multipart_create - マルチパートを作成する (RFC 2046, RFC 2183)
 */
int multipart_create(TCHAR *Filename, char *ContentType, char *Encoding, char **RetContentType, char *body, char **RetBody)
{
#define BREAK_LEN			76
#define	CTYPE_MULTIPART		"multipart/mixed;\r\n boundary=\""
#define CONTENT_DIPPOS		"Content-Disposition: attachment;"
#define ENCODING_BASE64		"Content-Transfer-Encoding: base64"
#define CONTENT_TYPE_NAME	";\r\n name=\""
	SYSTEMTIME st;
	TCHAR *fpath, *fname;
	TCHAR *f;
	TCHAR date[15];
	char *Boundary, *ctype;
	char *buf, *ret, *tmp;
	char *cfname;
	char *p, *ef;
	char *cBuf;
	char *b64str;
	char *cp, *cr;
	unsigned char digest[16];
	long FileSize;
	int i, len;
#ifdef UNICODE
	TCHAR *wtmp;
#endif

	if (Filename == NULL || *Filename == TEXT('\0')) {
		return MP_NO_ATTACH;
	}

	// バウンダリの生成
	GetLocalTime(&st);
	wsprintf(date, TEXT("%04d%02d%02d%02d%02d%02d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

#ifdef UNICODE
	cp = alloc_tchar_to_char(Filename);
	cr = alloc_tchar_to_char(date);
	HMAC_MD5(cp, tstrlen(cp), cr, tstrlen(cr), digest);
	mem_free(&cp);
	mem_free(&cr);
#else
	HMAC_MD5(Filename, lstrlen(Filename), date, lstrlen(date), digest);
#endif

	Boundary = (char *)mem_alloc(sizeof(char) * ((16 * 2) + 17 + 1));
	if (Boundary == NULL) {
		return MP_ERROR_ALLOC;
	}
	p = str_cpy(Boundary, "-----_MULTIPART_");
	for (i = 0; i < 16; i++) {
#ifdef UNICODE
		sprintf(p, "%02X", digest[i]);
#else
		wsprintf(p, "%02X", digest[i]);
#endif
		p += 2;
	}
	tstrcpy(p, "_");

	// マルチパートの作成
	len = 2 + tstrlen(Boundary) + 2 +
		tstrlen(HEAD_CONTENTTYPE) + 1 + tstrlen(ContentType) + 2 +
		tstrlen(HEAD_ENCODING) + 1 + tstrlen(Encoding) + 4 +
		tstrlen(body) + 4;
	ret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (ret == NULL) {
		mem_free(&Boundary);
		return MP_ERROR_ALLOC;
	}
	str_join(ret, "--", Boundary, "\r\n",
		HEAD_CONTENTTYPE, " ", ContentType, "\r\n",
		HEAD_ENCODING, " ", Encoding, "\r\n\r\n",
		body, "\r\n\r\n", (char *)-1);

	fpath = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Filename) + 1));
	if (fpath == NULL) {
		mem_free(&Boundary);
		mem_free(&ret);
		return MP_ERROR_ALLOC;
	}

	f = Filename;
	while (*f != TEXT('\0')) {
		f = str_cpy_f_t(fpath, f, ATTACH_SEP);
		fname = GetFileNameString(fpath);

		// ファイルを読み込む
		FileSize = file_get_size(fpath);
		if (FileSize < 0 || (cBuf = file_read(fpath, FileSize)) == NULL) {
			mem_free(&Boundary);
			mem_free(&ret);
			mem_free(&fpath);
			return MP_ERROR_FILE;
		}

		// エンコード
		b64str = (char *)mem_alloc(FileSize * 2 + 4);
		if (b64str == NULL) {
			mem_free(&Boundary);
			mem_free(&ret);
			mem_free(&fpath);
			mem_free(&cBuf);
			return MP_ERROR_ALLOC;
		}
		base64_encode(cBuf, b64str, FileSize);
		mem_free(&cBuf);

		// 折り返し
		cBuf = (char *)mem_alloc(tstrlen(b64str) + (tstrlen(b64str) / BREAK_LEN * 2) + 1);
		if (cBuf == NULL) {
			mem_free(&Boundary);
			mem_free(&ret);
			mem_free(&fpath);
			mem_free(&b64str);
			return MP_ERROR_ALLOC;
		}
		for (cp = b64str, cr = cBuf, i = 0; *cp != '\0'; cp++, i++) {
			if (i >= BREAK_LEN) {
				i = 0;
				*(cr++) = '\r';
				*(cr++) = '\n';
			}
			*(cr++) = *cp;
		}
		*cr = '\0';
		mem_free(&b64str);

		buf = cBuf;

		// MIME typeの取得
#ifdef UNICODE
		wtmp = GetMIME2Extension(NULL, fname);
		ctype = alloc_tchar_to_char(wtmp);
		mem_free(&wtmp);
#else
		ctype = GetMIME2Extension(NULL, fname);
#endif
		if (ctype == NULL) {
			mem_free(&Boundary);
			mem_free(&ret);
			mem_free(&fpath);
			mem_free(&buf);
			return MP_ERROR_ALLOC;
		}

		// 
		p = NULL;
		
//		if( op.EncodeType == 1 )
//		{
//#ifdef UNICODE
//			wtmp = MIME_encode(fname, FALSE, op.HeadCharset, op.HeadEncoding);
//			fname = wtmp;
//#else
//			p = MIME_encode(fname, FALSE, op.HeadCharset, op.HeadEncoding);
//			fname = p;
//#endif
//		}

		if (fname == NULL) {
			mem_free(&Boundary);
			mem_free(&ret);
			mem_free(&fpath);
			mem_free(&buf);
			mem_free(&ctype);
			mem_free(&p);
			return MP_ERROR_ALLOC;
		}

//#ifdef UNICODE
//		wtmp = MIME_rfc2231_encode(fname, op.HeadCharset);
//		ef = alloc_tchar_to_char(wtmp);
//		mem_free(&wtmp);
//#else
//		ef = MIME_rfc2231_encode(fname, op.HeadCharset);
//#endif

		if(ef == NULL) 
		{
			mem_free(&p);
			mem_free(&Boundary);
			mem_free(&ret);
			mem_free(&fpath);
			mem_free(&buf);
			mem_free(&ctype);
			return MP_ERROR_ALLOC;
		}

#ifdef UNICODE
		cfname = alloc_tchar_to_char(fname);
#else
		cfname = fname;
#endif

		// Partの追加
		len += (2 + tstrlen(Boundary) + 2 +
			tstrlen(HEAD_CONTENTTYPE) + 1 + tstrlen(ctype) + 2 +
			tstrlen(CONTENT_DIPPOS) + tstrlen(ef) + 2 +
			tstrlen(ENCODING_BASE64) + 4 +
			tstrlen(buf) + 4);
		
		//if( op.EncodeType == 1 ) 
		//{
		//	len += tstrlen(CONTENT_TYPE_NAME) + tstrlen(cfname) + 1;
		//}

		tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
		if (tmp == NULL) {
			mem_free(&p);
			mem_free(&Boundary);
			mem_free(&ret);
			mem_free(&fpath);
			mem_free(&buf);
			mem_free(&ctype);
			mem_free(&ef);
#ifdef UNICODE
			mem_free(&cfname);
#endif
			return MP_ERROR_ALLOC;
		}

#if 0
		//if (op.EncodeType == 1) 
		{

			str_join(tmp, ret, "--", Boundary, "\r\n",
				HEAD_CONTENTTYPE, " ", ctype, CONTENT_TYPE_NAME, cfname, "\"\r\n",
				CONTENT_DIPPOS, ef, "\r\n",
				ENCODING_BASE64, "\r\n\r\n",
				buf, "\r\n\r\n", (char *)-1);
	
		} 
		else 
#endif 
		{

			str_join(tmp, ret, "--", Boundary, "\r\n",
				HEAD_CONTENTTYPE, " ", ctype, "\r\n",
				CONTENT_DIPPOS, ef, "\r\n",
				ENCODING_BASE64, "\r\n\r\n",
				buf, "\r\n\r\n", (char *)-1);
		}

		mem_free(&p);
		mem_free(&ef);
		mem_free(&ctype);
		mem_free(&buf);
		mem_free(&ret);
#ifdef UNICODE
		mem_free(&cfname);
#endif
		ret = tmp;
	}
	mem_free(&fpath);

	// マルチパートの終わり
	len += (2 + tstrlen(Boundary) + 2 + 2);
	*RetBody = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (*RetBody == NULL) {
		mem_free(&Boundary);
		mem_free(&ret);
		return MP_ERROR_ALLOC;
	}
	str_join(*RetBody, ret, "--", Boundary, "--\r\n", (char *)-1);
	mem_free(&ret);

	// Content typeの生成
	*RetContentType = (char *)mem_alloc(
		sizeof(TCHAR) * (tstrlen(CTYPE_MULTIPART) + tstrlen(Boundary) + 2));
	if (*RetContentType == NULL) {
		mem_free(&Boundary);
		mem_free(&*RetBody);
		return MP_ERROR_ALLOC;
	}
	str_join(*RetContentType, CTYPE_MULTIPART, Boundary, "\"", (char *)-1);
	mem_free(&Boundary);
	return MP_ATTACH;
}
/* End of source */
