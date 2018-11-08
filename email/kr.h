// Korean encode

#ifndef __KR_H__
#define __KR_H__

/* #include <stddef.h> */

/*
 * iso-2022-kr : 7bit
 * ks_c_5601   : 8bit
 */

#define CHARSET_ISO_2022_KR "ISO-2022-KR"

/*
 * “ü—Í‚ª³‚µ‚­‚È‚¢ê‡‚Ì‹““®‚Í•s–¾
 */

void   iso2022kr_ksc5601(const char *, char *);
size_t iso2022kr_ksc5601_len(const char *);
void   ksc5601_iso2022kr(const char *, char *);
size_t ksc5601_iso2022kr_len(const char *);

#endif
