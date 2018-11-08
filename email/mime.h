/*
 * ERC
 *
 * mime.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_MIME_H
#define _INC_MIME_H

/* Include Files */
#include <windows.h>
#include "multipart.h"

/* Define */

/* Struct */

/* Function Prototypes */
char *MIME_charset_encode(const UINT cp, TCHAR *buf, TCHAR *charset);
TCHAR *MIME_charset_decode(const UINT cp, char *buf, TCHAR *charset);
TCHAR *MIME_encode(TCHAR *wbuf, BOOL Address, TCHAR *charset_t, int encoding);
BOOL MIME_decode(char *buf, TCHAR *ret);
TCHAR *MIME_rfc2231_encode(TCHAR *wbuf, TCHAR *charset_t);
char *MIME_rfc2231_decode(char *buf);
BOOL MIME_create_encode_header(TCHAR *charset, int encoding, char *ret_content_type, char *ret_encoding);
char *MIME_body_encode(TCHAR *body, TCHAR *charset_t, int encoding, char *ret_content_type, char *ret_encoding, TCHAR *ErrStr);
char *MIME_body_decode_transfer(MAILITEM *mail_item, char *body);
TCHAR *MIME_body_decode(MAILITEM *mail_item, BOOL ViewSrc, MULTIPART ***tpPart, int *cnt);

#endif
/* End of source */
