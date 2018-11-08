/*
 * nPOP
 *
 * jp.c (Japanese encode)
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "memory_mng.h"
#include "string_mng.h"

/* Define */
#define ESC						0x1B

#define	IsKanji(c)				(((unsigned char)c >= (unsigned char)0x81 && (unsigned char)c <= (unsigned char)0x9F) || \
									((unsigned char)c >= (unsigned char)0xE0 && (unsigned char)c <= (unsigned char)0xFC))
#define	IsHankaku(c)			((unsigned char)c >= (unsigned char)0xa0 && (unsigned char)c <= (unsigned char)0xdf)

#define CHARSET_ISO_2022_JP		"ISO-2022-JP"

/* Global Variables */
//extern OPTION op;

/* Local Function Prototypes */
static int HanToZen(unsigned int *zenkaku, unsigned char *str);
static void JisShift(int *ph,int *pl);
static void SjisShift(int *ph, int *pl);

/*
 * IsDependenceString - ShiftJISの機種依存文字が含まれているかチェック
 */
int IsDependenceString(TCHAR *buf)
{
	unsigned char *p;
	unsigned char c,d;
	int ret = -1;
	int index = 0;
#ifdef UNICODE
	char *cBuf;
#endif

	if (buf == NULL) {
		return ret;
	}
	
	//if( ( op.HeadCharset == NULL || lstrcmpi( op.HeadCharset, TEXT( CHARSET_ISO_2022_JP ) ) != 0 ) 
	//	&& ( op.BodyCharset == NULL || lstrcmpi( op.BodyCharset, TEXT( CHARSET_ISO_2022_JP ) ) != 0 ) )
	//{
	//	return ret;
	//}

#ifdef UNICODE
	cBuf = alloc_tchar_to_char(buf);
	if (cBuf == NULL) {
		return ret;
	}
	p = (unsigned char *)cBuf;
#else
	p = buf;
#endif
	while ((c = *(p++)) != '\0') {
		if (IsKanji(c) == TRUE) {
			d = *(p++);
			if (
				// 特殊文字区点コード, 13区
				(c == 0x87 && (d >= 0x40 && d <= 0x9C)) ||

				// NEC選定IBM拡張文字区点コード, 89区～92区
				(c == 0xED && (d >= 0x40 && d <= 0xFF)) ||
				(c == 0xEE && (d >= 0x00 && d <= 0xFC)) ||

				// IBM拡張文字区点コード, 115区～119区
				(c == 0xFA && (d >= 0x40 && d <= 0xFF)) ||
				(c == 0xFB && (d >= 0x00 && d <= 0xFF)) ||
				(c == 0xFC && (d >= 0x00 && d <= 0x4B))
				) {
				ret = index;
				break;
			}
#ifndef UNICODE
			index++;
#endif
		}
		index++;
	}
#ifdef UNICODE
	mem_free(&cBuf);
#endif
	return ret;
}

/*
 * HanToZen - 半角カナを全角カナに変換
 */
static int HanToZen(unsigned int *zenkaku, unsigned char *str)
{
	const unsigned int z[64] = {
		0x2121,0x2123,0x2156,0x2157,0x2122,0x2126,0x2572,0x2521,
		0x2523,0x2525,0x2527,0x2529,0x2563,0x2565,0x2567,0x2543,
		0x213c,0x2522,0x2524,0x2526,0x2528,0x252a,0x252b,0x252d,
		0x252f,0x2531,0x2533,0x2535,0x2537,0x2539,0x253b,0x253d,
		0x253f,0x2541,0x2544,0x2546,0x2548,0x254a,0x254b,0x254c,
		0x254d,0x254e,0x254f,0x2552,0x2555,0x2558,0x255b,0x255e,
		0x255f,0x2560,0x2561,0x2562,0x2564,0x2566,0x2568,0x2569,
		0x256a,0x256b,0x256c,0x256d,0x256f,0x2573,0x212b,0x212c};
	typedef struct {
		unsigned char han;
		unsigned int zen;
	} TBL;
	const TBL daku[] = {
		{0xb3,0x2574},{0xb6,0x252c},{0xb7,0x252e},{0xb8,0x2530},
		{0xb9,0x2532},{0xba,0x2534},{0xbb,0x2536},{0xbc,0x2538},
		{0xbd,0x253a},{0xbe,0x253c},{0xbf,0x253e},{0xc0,0x2540},
		{0xc1,0x2542},{0xc2,0x2545},{0xc3,0x2547},{0xc4,0x2549},
		{0xca,0x2550},{0xcb,0x2553},{0xcc,0x2556},{0xcd,0x2559},
		{0xce,0x255c},{0,0}};
	const TBL handaku[] = {
		{0xca,0x2551},{0xcb,0x2554},{0xcc,0x2557},{0xcd,0x255a},
		{0xce,0x255d},{0,0}};
	int i;

	if (*(str+1) == 0xde) {           /* 濁音符 */
		for (i = 0; (daku + i)->zen != 0; i++)
			if (*str == (daku + i)->han) {
			*zenkaku = (daku + i)->zen;
			return 2;
		}
	} else if (*(str+1) == 0xdf) {    /* 半濁音符 */
	for (i = 0; (handaku + i)->zen != 0; i++)
		if (*str == (handaku + i)->han) {
			*zenkaku = (handaku + i)->zen;
			return 2;
		}
	}
	*zenkaku = z[*str - 0xa0];
	return 1;
}

/*
 * JisShift - SJISの2バイトをJISに変換
 */
static void JisShift(int *ph,int *pl)
{
	*ph = *ph & 0xff;
	*pl = *pl & 0xff;

	if (*ph <= 0x9F) {
		if (*pl < 0x9F) {
			*ph = (*ph << 1) - 0xE1;
		} else {
			*ph = (*ph << 1) - 0xE0;
		}
	} else {
		if (*pl < 0x9F) {
			*ph = (*ph << 1) - 0x161;
		} else {
			*ph = (*ph << 1) - 0x160;
		}
	}

	if (*pl < 0x7F) {
		*pl -= 0x1F;
	} else if (*pl < 0x9F) {
		*pl -= 0x20;
	} else {
		*pl -= 0x7E;
	}
}

/*
 * sjis_iso2022jp - SJISをISO-2022-JP(JIS)に変換 (RFC 1468)
 */
void sjis_iso2022jp(unsigned char *buf,unsigned char *ret)
{
	unsigned char *p,*r;
	int knaji;
	int c,d;
	int rc;
	unsigned int e;
	unsigned int hib, lob;

	p = buf;
	r = ret;
	knaji = 0;

	while ((c = *(p++)) != '\0') {
		switch (knaji) {
		case 0:
			if (IsKanji(c) == TRUE || IsHankaku(c) == TRUE) {
				*(r++) = ESC;
				*(r++) = '$';
				*(r++) = 'B';

				if (IsKanji(c) == TRUE) {
					d = *(p++);
					JisShift(&c,&d);
					*(r++) = c;
					*(r++) = d;
				} else {
					rc = HanToZen(&e,p - 1);
					if (rc != 1) {
						p++;
					}
					hib = (e >> 8) & 0xff;
					lob = e & 0xff;
					*(r++) = hib;
					*(r++) = lob;
				}
				knaji = 1;
			} else {
				*(r++) = c;
			}
			break;
		case 1:
			if (IsKanji(c) == FALSE && IsHankaku(c) == FALSE) {
				*(r++) = ESC;
				*(r++) = '(';
				*(r++) = 'B';
				*(r++) = c;
				knaji = 0;
			} else {
				if (IsKanji(c) == TRUE) {
					d = *(p++);
					JisShift(&c,&d);
					*(r++) = c;
					*(r++) = d;
				} else {
					rc = HanToZen(&e,p - 1);
					if (rc != 1) {
						p++;
					}
					hib = (e >> 8) & 0xff;
					lob = e & 0xff;
					*(r++) = hib;
					*(r++) = lob;
				}
			}
			break;
		}
	}
	if (knaji == 1) {
		*(r++) = ESC;
		*(r++) = '(';
		*(r++) = 'B';
	}
	*r = '\0';
}

/*
 * SjisShift - JISの2バイトをSJISに変換
 */
static void SjisShift(int *ph, int *pl)
{
	if (*ph & 1) {
		if (*pl < 0x60) {
			*pl+=0x1F;
		} else {
			*pl+=0x20;
		}
	} else {
		*pl+=0x7E;
	}
	if (*ph < 0x5F) {
		*ph = (*ph + 0xE1) >> 1;
	} else {
		*ph = (*ph + 0x161) >> 1;
	}
}

/*
 * iso2022jp_sjis - ISO-2022-JP(JIS)をSJISに変換 (RFC 1468)
 */
char *iso2022jp_sjis(char *buf, char *ret)
{
	unsigned int c, d, j;
	BOOL jiskanji, hankaku;
	char *p, *r;

	*ret = '\0';

	p = buf;
	r = ret;
	j = 0;
	jiskanji = FALSE;
	hankaku = FALSE;
	while (*p != '\0') {
		j++;
		c = *(p++);
		if (c == ESC) {
			if (*p == '\0') {
				break;
			}
			if ((c = *(p++)) == '$') {
				if ((c = *(p++)) == '@' || c == 'B') {
					jiskanji = TRUE;
				} else {
					*(r++) = ESC;
					*(r++) = '$';
					if (c != '\0') {
						*(r++) = c;
					}
				}
			} else if (c == '(') {
				if ((c = *(p++)) == 'H' || c == 'J' || c == 'B') {
					jiskanji = FALSE;
				} else {
					jiskanji = FALSE;
					*(r++) = ESC;
					*(r++) = '(';
					if (c != '\0') {
						*(r++) = c;
					}
				}
			} else if (c == '*') {
				if ((c = *(p++)) == 'B') {
					hankaku = FALSE;
				} else if (c == 'I') {
					hankaku = TRUE;
				}
			} else if (hankaku == TRUE && c == 'N') {
				c = *(p++);
				*(r++) = c + 0x80;
			} else if (c == 'K') {
				jiskanji = TRUE;
			} else if (c == 'H') {
				jiskanji = FALSE;
			} else {
				*(r++) = ESC;
				if (c != '\0') {
					*(r++) = c;
				}
			}
		} else if (jiskanji && (c == '\r' || c == '\n')) {
			jiskanji = FALSE;
			*(r++) = c;
		} else if (jiskanji && c >= 0x21 && c <= 0x7E) {
			if (*p == '\0') {
				break;
			}
			if ((d = *(p++)) >= 0x21 && d <= 0x7E) {
				SjisShift((int *)&c, (int *)&d);
			}
			*(r++) = c;
			if (d != '\0') {
				*(r++) = d;
			}
		} else if (c >= 0xA1 && c <= 0xFE) {
			if (*p == '\0') {
				break;
			}
			if ((d = *(p++)) >= 0xA1 && d <= 0xFE) {
				d &= 0x7E;
				c &= 0x7E;
				SjisShift((int *)&c, (int *)&d);
			}
			*(r++) = c;
			if (d != '\0') {
				*(r++) = d;
			}
		} else if (c == 0x0E) {
			while (*p != '\0' && *p != 0x0F && *p != '\r' && *p != '\n' && *p != ESC) {
				*(r++) = *(p++) + 0x80;
			}
			if (*p == 0x0F) {
				p++;
			}
		} else {
			*(r++) = c;
		}
	}
	*r = '\0';
	return r;
}
/* End of source */
