/*
 * ERC
 *
 * code.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_CODE_H
#define _INC_CODE_H

/* Include Files */
#include <windows.h>

/* Define */

/* Struct */

/* Function Prototypes */
char *base64_decode(char *buf, char *ret);
void base64_encode(char *buf, char *ret, int size);
#ifdef UNICODE
void base64_encode_t(TCHAR *buf, TCHAR *ret, int size);
#else
#define base64_encode_t base64_encode
#endif

char *QuotedPrintable_decode(char *buf, char *ret);
void QuotedPrintable_encode(unsigned char *buf, char *ret, int break_size, const BOOL body);

char *URL_decode(char *buf, char *ret);
void URL_encode(unsigned char *buf, char *ret);

#endif
/* End of source */
