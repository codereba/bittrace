/*
 * ERC
 *
 * charset.h
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef INC_CHARSET_H
#define INC_CHARSET_H

/* Include Files */

/* Define */

/* Struct */

/* Function Prototypes */
void charset_init(void);
void charset_uninit(void);
DWORD charset_to_cp(const BYTE charset);
char *charset_encode(WCHAR *charset, WCHAR *buf, UINT len);
WCHAR *charset_decode(WCHAR *charset, char *buf, UINT len);
LRESULT charset_enum(HWND hWnd);
void set_default_encode(const UINT cp, TCHAR **HeadCharset, TCHAR **BodyCharset);

#endif
/* End of source */
