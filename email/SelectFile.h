/*
 * ERC
 *
 * SelectFile.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef SELECT_FILE_H
#define SELECT_FILE_H

/* Include Files */
#include <windows.h>

/* Define */

/* Struct */

/* Function Prototypes */
BOOL SelectFile(HWND hDlg, HINSTANCE hInst, int mode_open, TCHAR *fname, TCHAR *ret);

#endif
/* End of source */
