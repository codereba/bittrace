// Korean encode

#include "General.h"
#include "string_mng.h"

#define IS_KSC(c) ((unsigned char)(c)>(unsigned char)0xa0&& \
  (unsigned char)(c)<(unsigned char)0xff)

#define SI 0x0f
#define SO 0x0e

void ksc5601_iso2022kr(const char *kscs, char *ret)
{
  int ksc=0;
  int i, mode;
  const char *ptr=kscs;

  while (*ptr!='\0') {
    mode=0; /* ascii */
    i=0;
    if (ksc==0) {
      while (ptr[i]!='\n'&&ptr[i]!='\0') {
        if (IS_KSC(ptr[i])) {
            ksc=1; /* ksc mode */
            *ret++=0x1b;
            *ret++='$';
            *ret++=')';
            *ret++='C';
            break;
        }
        ++i;
      }
    }

    while (*ptr!='\n'&&*ptr!='\0') {
      if (mode==0&&IS_KSC(*ptr)) {
        /* ksc code under ascii mode */
        *ret++=SO;
        *ret++=0x7f&*ptr;
        mode=1; /* ksc mode */
      } else if (mode==0&&!IS_KSC(*ptr)) {
        /* non ksc code under ascii mode */
        *ret++=*ptr;
      } else if (mode==1&&IS_KSC(*ptr)) {
        /* ksc code under ksc mode */
        *ret++=0x7f&*ptr;
      } else {
        /* non ksc code under ksc mode */
        *ret++=SI;
        *ret++=*ptr;
        mode=0; /* ascii */
      }
      ++ptr;
    }
    if (mode==1) {
      *ret++=SI;
    }
    if (*ptr=='\0') {
      break;
    }
    *ret++=*ptr++;
  }
  *ret='\0';
}

size_t ksc5601_iso2022kr_len(const char *kscs)
{
  size_t sz=0;
  int ksc=0;
  int i, mode;
  const char *ptr=kscs;

  while (*ptr!='\0') {
    mode=0; /* ascii */
    i=0;
    if (ksc==0) {
      while (ptr[i]!='\n'&&ptr[i]!='\0') {
        if (IS_KSC(ptr[i])) {
            ksc=1; /* ksc mode */
            sz+=4;
            break;
        }
        ++i;
      }
    }

    while (*ptr!='\n'&&*ptr!='\0') {
      if (mode==0&&IS_KSC(*ptr)) {
        /* ksc code under ascii mode */
        sz+=2;
        mode=1; /* ksc mode */
      } else if (mode==0&&!IS_KSC(*ptr)) {
        /* non ksc code under ascii mode */
        sz+=1;
      } else if (mode==1&&IS_KSC(*ptr)) {
        /* ksc code under ksc mode */
        sz+=1;
      } else {
        /* non ksc code under ksc mode */
        sz+=2;
        mode=0; /* ascii */
      }
      ++ptr;
    }
    if (mode==1) {
      sz+=1;
    }
    if (*ptr=='\0') {
      break;
    }
    sz+=1;
    ptr++;
  }
  return sz;
}

void iso2022kr_ksc5601(const char *krs, char *ret)
{
  int mode=0;
  const char *ptr=krs;

  while (*ptr!='\0') {
    if (!str_cmp_ni(ptr, "\033$)C", 4)) {
      ptr+=4;
      continue;
    }

    switch (*ptr) {
    case SO:
      mode=1;
      break;
    case SI:
      mode=0;
      break;
    default:
      if (mode==0) {
        *ret++=*ptr;
      } else {
        if (*ptr!=0x20&&*ptr!=0x09) {
          *ret++=0x80|*ptr;
        } else {
          *ret++=*ptr;
        }
      }
    }
    ++ptr;
  }
  *ret = '\0';
}

size_t iso2022kr_ksc5601_len(const char *krs)
{
  size_t sz=0;
  int kr=0, mode;
  const char *ptr=krs;

  while (*ptr!='\0') {
    mode=0; /* ascii */
    if (!str_cmp_ni(ptr, "\033$)C", 4)) {
      kr=1;
      ptr+=4;
      continue;
    }

    switch (*ptr) {
    case SO:
      mode=1;
      break;
    case SI:
      mode=0;
      break;
    default:
      if (mode==0) {
        sz+=1;
      } else {
        if (*ptr!=0x20&&*ptr!=0x09) {
          sz+=1;
        } else {
          sz+=1;
        }
      }
    }
    ++ptr;
  }

  return sz;
}
