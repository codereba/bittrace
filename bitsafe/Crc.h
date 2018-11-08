/*
 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of Encryption for the Masses 2.02a, which is
 Copyright (c) 1998-2000 Paul Le Roux and which is governed by the 'License
 Agreement for Encryption for the Masses'. Modifications and additions to
 the original source code (contained in this file) and all other portions
 of this file are Copyright (c) 2003-2008 TrueCrypt Developers Association
 and are governed by the TrueCrypt License 2.8 the full text of which is
 contained in the file License.txt included in TrueCrypt binary and source
 code distribution packages. */

#ifndef __CRC_H__
#define __CRC_H__

#include "XEDefs.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define UPDC32(octet, crc)\
  (unsigned __int32)((crc_32_tab[(((unsigned __int32)(crc)) ^ ((unsigned char)(octet))) & 0xff] ^ (((unsigned __int32)(crc)) >> 8)))

unsigned __int32 GetCrc32 (unsigned char *data, int length);
unsigned __int32 crc32int (unsigned __int32 *data);
BOOL crc32_selftests (void);

extern unsigned __int32 crc_32_tab[];

#if defined(__cplusplus)
}
#endif

#endif // __CRC_H__