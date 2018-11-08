/*
 * ERC
 *
 * Font.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_FONT_H
#define _INC_FONT_H

/* Include Files */
#include <windows.h>

/* Define */

/* Struct */
typedef struct _FONT_INFO {
	TCHAR *name;
	int size;
	int weight;
	int italic;
	int charset;
} FONT_INFO;

/* Function Prototypes */
HFONT font_create(HWND hWnd, FONT_INFO *fi);
HFONT font_copy(const HFONT hfont);
int font_get_charset(const HDC hdc);
#ifndef _WIN32_WCE
BOOL font_select(const HWND hWnd, FONT_INFO *fi);
#endif

#endif
/* End of source */
