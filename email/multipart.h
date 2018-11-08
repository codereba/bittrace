/*
 * ERC
 *
 * multipart.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_MULTIPART_H
#define _INC_MULTIPART_H

/* Include Files */
#include <windows.h>

/* Define */

/* Struct */
typedef struct _MULTIPART {
	char *ContentType;
	char *Filename;
	char *Encoding;
	char *sPos;
	char *ePos;
} MULTIPART;

/* Function Prototypes */
MULTIPART *multipart_add(MULTIPART ***tpMultiPart, int cnt);
void multipart_free(MULTIPART ***tpMultiPart, int cnt);
char *multipart_get_filename(char *buf, char *Attribute);
int multipart_parse(char *ContentType, char *buf, MULTIPART ***tpMultiPart, int cnt);
int multipart_create(TCHAR *Filename, char *ContentType, char *Encoding, char **RetContentType, char *body, char **RetBody);

#endif
/* End of source */
