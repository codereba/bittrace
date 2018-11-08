/*
 * ERC
 *
 * Font.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include <windows.h>

#include "memory_mng.h"
#include "string_mng.h"
#include "font.h"

/* Define */

/* Global Variables */

/* Local Function Prototypes */

/*
 * font_create - �t�H���g���쐬����
 */
HFONT font_create(HWND hWnd, FONT_INFO *fi)
{
	LOGFONT lf;
	HDC hdc;

	ZeroMemory(&lf, sizeof(LOGFONT));

	hdc = GetDC(hWnd);
	lf.lfHeight = -(int)((fi->size * GetDeviceCaps(hdc,LOGPIXELSY)) / 72);
	ReleaseDC(hWnd, hdc);

	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = fi->weight;
	lf.lfItalic = fi->italic;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = fi->charset;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	lstrcpy(lf.lfFaceName, fi->name);
	return CreateFontIndirect((CONST LOGFONT *)&lf);
}

/*
 * font_copy - �t�H���g�̃R�s�[���쐬
 */
HFONT font_copy(const HFONT hfont)
{
	LOGFONT lf;

	ZeroMemory(&lf, sizeof(LOGFONT));
	if (GetObject(hfont, sizeof(LOGFONT), &lf) == 0) {
		return NULL;
	}
	return CreateFontIndirect((CONST LOGFONT *)&lf);
}

/*
 * font_get_charset - �t�H���g�̃L�����Z�b�g�擾
 */
int font_get_charset(const HDC hdc)
{
	TEXTMETRIC tm;

	GetTextMetrics(hdc, &tm);
	return tm.tmCharSet;
}

/*
 * font_select - �t�H���g�̑I��
 */
#ifndef _WIN32_WCE
BOOL font_select(const HWND hWnd, FONT_INFO *fi)
{
	CHOOSEFONT cf;
	LOGFONT lf;
	HDC hdc;

	// �t�H���g���̍쐬
	ZeroMemory(&lf, sizeof(LOGFONT));
	hdc = GetDC(NULL);
	lf.lfHeight = -(int)((fi->size * GetDeviceCaps(hdc, LOGPIXELSY)) / 72);
	ReleaseDC(NULL, hdc);
	lf.lfWeight = fi->weight;
	lf.lfItalic = fi->italic;
	lf.lfCharSet = fi->charset;
	lstrcpy(lf.lfFaceName, fi->name);
	// �t�H���g�I���_�C�A���O��\��
	ZeroMemory(&cf, sizeof(CHOOSEFONT));
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hWnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	cf.nFontType = SCREEN_FONTTYPE;
	if (ChooseFont(&cf) == FALSE) {
		return FALSE;
	}
	// �ݒ�擾
	mem_free(&fi->name);
	fi->name = alloc_copy_t(lf.lfFaceName);
	fi->weight = lf.lfWeight;
	fi->italic = lf.lfItalic;
	fi->charset = lf.lfCharSet;
	fi->size = cf.iPointSize / 10;
	return TRUE;
}
#endif
/* End of source */
