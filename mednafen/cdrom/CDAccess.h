#ifndef __MDFN_CDROMFILE_H
#define __MDFN_CDROMFILE_H

#include <stdint.h>
#include <stdio.h>
#include <boolean.h>

#include "CDUtility.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CDAccess: polymorphic disc-image reader.
 *
 * Previously a C++ abstract base class with three pure-virtual
 * methods (Read_Raw_Sector / Fast_Read_Raw_PW_TSRE / Read_TOC) plus
 * a virtual destructor.  Now a plain C struct with an explicit
 * function-pointer vtable, following the same pattern
 * beetle-psx-libretro converted to.  Each backend (Image / CCD /
 * CHD) defines a concrete struct that embeds `CDAccess base` as
 * its first member, plus a factory function CDAccess_<X>_New that
 * allocates the struct and installs the four function pointers.
 *
 * Lifecycle: CDAccess_Open dispatches on file extension to the
 * right factory.  Use cda->Method(cda, ...) shape for every vtable
 * call, and cda->destroy(cda) to tear down.  All four vtable slots
 * are required - no NULL checks in the dispatch path.
 */
struct CDAccess
{
   bool (*Read_Raw_Sector)      (struct CDAccess *self, uint8_t *buf,   int32_t lba);
   bool (*Fast_Read_Raw_PW_TSRE)(struct CDAccess *self, uint8_t *pwbuf, int32_t lba);
   bool (*Read_TOC)             (struct CDAccess *self, TOC *toc);
   void (*destroy)              (struct CDAccess *self);
};

typedef struct CDAccess CDAccess;

/* Dispatches on path extension (.ccd, .chd, else Image).  Returns
 * NULL on failure. */
CDAccess *CDAccess_Open(const char *path, bool image_memcache);

#ifdef __cplusplus
}
#endif

#endif
