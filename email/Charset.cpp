/*
 * ERC
 *
 * charset.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */

#include "General.h"
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <mlang.h>

extern "C" {
#include "Charset.h"
#include "memory_mng.h"
#include "string_mng.h"
}

/* Define */

/* Global Variables */
MIMECPINFO* mimeCPInfos;
ULONG lFetchedCelt;

/* Local Function Prototypes */
static void codepage_init(void);
static void codepage_free(void);

/*
 * charset_init - OLEの初期化
 */
void charset_init()
{
#ifndef _WIN32_WCE
	CoInitialize(NULL);
#else
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
}

/*
 * charset_uninit - OLEの解放
 */
void charset_uninit()
{
	if (mimeCPInfos != NULL) {
		codepage_free();
	}
	CoUninitialize();
}

/*
 * codepage_init - コードページ情報の初期化
 */
static void codepage_init(void)
{
	IMultiLanguage *pMultiLanguage;
	IEnumCodePage *spEnumCodePage;
	LRESULT hr = S_OK;

	if (mimeCPInfos != NULL) {
		codepage_free();
	}
	CoCreateInstance(__uuidof(CMultiLanguage), NULL, CLSCTX_ALL, __uuidof(IMultiLanguage), (void**)&pMultiLanguage);
	if (pMultiLanguage == NULL) {
		return;
	}
	hr = pMultiLanguage->EnumCodePages(MIMECONTF_SAVABLE_MAILNEWS, &spEnumCodePage);
	if (FAILED(hr)) {
		pMultiLanguage->Release();
		return;
	}
	UINT nCodePage = 0;
	hr = pMultiLanguage->GetNumberOfCodePageInfo(&nCodePage);
	if (FAILED(hr)) {
		pMultiLanguage->Release();
		return;
	}
	// 列挙する
	mimeCPInfos = (MIMECPINFO*)::CoTaskMemAlloc(sizeof(MIMECPINFO) * nCodePage);
	hr = spEnumCodePage->Next(nCodePage, mimeCPInfos, &lFetchedCelt);
	if (FAILED(hr)) {
		::CoTaskMemFree(mimeCPInfos);
		mimeCPInfos = NULL;
		pMultiLanguage->Release();
		return;
	}
	pMultiLanguage->Release();
}

/*
 * codepage_init - コードページ情報の解放
 */
static void codepage_free(void)
{
	if (mimeCPInfos != NULL) {
		::CoTaskMemFree(mimeCPInfos);
		mimeCPInfos = NULL;
	}
	lFetchedCelt = 0;
}

/*
 * charset_to_cp - CharsetからCodePageを取得
 */
DWORD charset_to_cp(const BYTE charset)
{
#ifdef UNICODE
	return CP_ACP;
#else
	if (mimeCPInfos == NULL) {
		codepage_init();
	}
	for (int i = 0; i < (int)lFetchedCelt; i++) {
		if (mimeCPInfos[i].bGDICharset == charset) {
			return mimeCPInfos[i].uiFamilyCodePage;
		}
	}
	return CP_ACP;
#endif
}

/*
 * charset_encode - エンコード
 */
char *charset_encode(WCHAR *charset, WCHAR *buf, UINT len)
{
	IMultiLanguage *pMultiLanguage;
	MIMECSETINFO mimeInfo;
	DWORD mode = 0;
	DWORD encoding;
	char *ret;
	UINT src_len = len;
	UINT ret_len;

	CoCreateInstance(__uuidof(CMultiLanguage), NULL, CLSCTX_ALL, __uuidof(IMultiLanguage), (void**)&pMultiLanguage);
	if (pMultiLanguage == NULL) {
		return NULL;
	}
	mimeInfo.uiCodePage = 0;
	mimeInfo.uiInternetEncoding = 0;
	pMultiLanguage->GetCharsetInfo(charset, &mimeInfo);
	encoding = (mimeInfo.uiInternetEncoding == 0) ? mimeInfo.uiCodePage : mimeInfo.uiInternetEncoding;

	ret_len = 0;
	if (pMultiLanguage->ConvertStringFromUnicode(&mode, encoding, buf, &len, NULL, &ret_len) != S_OK) {
		pMultiLanguage->Release();
		return NULL;
	}
	ret = (char *)mem_alloc(sizeof(char) * (ret_len + 1));
	if (pMultiLanguage->ConvertStringFromUnicode(&mode, encoding, buf, &len, ret, &ret_len) != S_OK) {
		mem_free((void **)&ret);
		pMultiLanguage->Release();
		return NULL;
	}
	*(ret + ret_len) = '\0';
	pMultiLanguage->Release();
	return ret;
}

/*
 * charset_decode - デコード
 */
WCHAR *charset_decode(WCHAR *charset, char *buf, UINT len)
{
	IMultiLanguage *pMultiLanguage;
	MIMECSETINFO mimeInfo;
	DWORD mode = 0;
	DWORD encoding;
	WCHAR *wret;
	UINT ret_len;

	CoCreateInstance(__uuidof(CMultiLanguage), NULL, CLSCTX_ALL, __uuidof(IMultiLanguage), (void**)&pMultiLanguage);
	if (pMultiLanguage == NULL) {
		return NULL;
	}
	mimeInfo.uiCodePage = 0;
	mimeInfo.uiInternetEncoding = 0;
	pMultiLanguage->GetCharsetInfo(charset, &mimeInfo);
	encoding = (mimeInfo.uiInternetEncoding == 0) ? mimeInfo.uiCodePage : mimeInfo.uiInternetEncoding;

	ret_len = 0;
	if (pMultiLanguage->ConvertStringToUnicode(&mode, encoding, buf, &len, NULL, &ret_len) != S_OK) {
		pMultiLanguage->Release();
		return NULL;
	}
	wret = (WCHAR *)mem_alloc(sizeof(WCHAR) * (ret_len + 1));
	if (pMultiLanguage->ConvertStringToUnicode(&mode, encoding, buf, &len, wret, &ret_len) != S_OK) {
		mem_free((void **)&wret);
		pMultiLanguage->Release();
		return NULL;
	}
	*(wret + ret_len) = L'\0';
	pMultiLanguage->Release();
	return wret;
}

/*
 * charset_enum - キャラクタセットの列挙
 */
LRESULT charset_enum(HWND hWnd)
{
	TCHAR lst_buf[256];

	if (mimeCPInfos == NULL) {
		codepage_init();
	}
	for (int i = 0; i < (int)lFetchedCelt; i++) {
		if (*mimeCPInfos[i].wszBodyCharset == L'_') {
			continue;
		}
#ifdef UNICODE
		for (int j = 0; j < SendMessage(hWnd, CB_GETCOUNT, 0, 0); j++) {
			SendMessage(hWnd, CB_GETLBTEXT, j, (LPARAM)lst_buf);
			if (lstrcmpi(mimeCPInfos[i].wszBodyCharset, lst_buf) == 0) {
				SendMessage(hWnd, CB_DELETESTRING, j, 0);
				break;
			}
		}
		SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)mimeCPInfos[i].wszBodyCharset);
#else
		char *buf;

		buf = alloc_wchar_to_char(CP_ACP, mimeCPInfos[i].wszBodyCharset);
		if (buf == NULL) {
			return E_FAIL;
		}
		for (int j = 0; j < SendMessage(hWnd, CB_GETCOUNT, 0, 0); j++) {
			SendMessage(hWnd, CB_GETLBTEXT, j, (LPARAM)lst_buf);
			if (lstrcmpi(buf, lst_buf) == 0) {
				SendMessage(hWnd, CB_DELETESTRING, j, 0);
				break;
			}
		}
		SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)buf);
		mem_free((void **)&buf);
#endif
	}
	return S_OK;
}

/*
 * set_default_encode - デフォルトのエンコードを設定
 */
void set_default_encode(const UINT cp, TCHAR **HeadCharset, TCHAR **BodyCharset)
{
	if (mimeCPInfos == NULL) {
		codepage_init();
	}
	for (int i = 0; i < (int)lFetchedCelt; i++) {
		if (*mimeCPInfos[i].wszBodyCharset == L'_') {
			continue;
		}
		if (mimeCPInfos[i].uiFamilyCodePage == cp || mimeCPInfos[i].uiCodePage == cp) {
#ifdef UNICODE
			if (*HeadCharset == NULL) {
				*HeadCharset = alloc_copy_t(mimeCPInfos[i].wszHeaderCharset);
			}
			if (*BodyCharset == NULL) {
				*BodyCharset = alloc_copy_t(mimeCPInfos[i].wszBodyCharset);
			}
#else
			if (*HeadCharset == NULL) {
				*HeadCharset = alloc_wchar_to_char(CP_ACP, mimeCPInfos[i].wszHeaderCharset);
			}
			if (*BodyCharset == NULL) {
				*BodyCharset = alloc_wchar_to_char(CP_ACP, mimeCPInfos[i].wszBodyCharset);
			}
#endif
			return;
		}
	}
}
/* End of source */
