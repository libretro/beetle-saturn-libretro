/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* CDAccess_CCD.h:
**  Copyright (C) 2013-2016 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
*/

#ifndef __MDFN_CDACCESS_CCD_H
#define __MDFN_CDACCESS_CCD_H

#include <stdint.h>
#include <stddef.h>
#include <boolean.h>

#include "../cdstream.h"
#include "CDAccess.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Concrete backend for the CDAccess vtable defined in CDAccess.h.
 * The CCD format is the CloneCD descriptor: a small INI-like text
 * file pointing at a .img (raw sector data) and a .sub (96-byte
 * subchannel data per sector) pair.  Parsing happens once in
 * CDAccess_CCD_New; after that all reads are fixed-stride lookups
 * into img/sub via the cdstream / sub_data members.
 *
 * CDAccess base MUST be the first member - dispatch from CDAccess*
 * casts back to this struct via its embedded base pointer's address. */
struct CDAccess_CCD
{
   CDAccess base;

   cdstream *img_stream;
   uint8_t  *sub_data;

   size_t img_numsectors;
   TOC    tocd;
};

typedef struct CDAccess_CCD CDAccess_CCD;

CDAccess *CDAccess_CCD_New(const char *path, bool image_memcache);

#ifdef __cplusplus
}
#endif

#endif
