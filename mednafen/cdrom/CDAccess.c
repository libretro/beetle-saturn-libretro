/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "CDAccess.h"

/* Backend factories.  Each is implemented in CDAccess_<X>.c.  Pre-
 * C-conversion these lived in CDAccess_<X>.cpp and used std::string /
 * std::map heavily, so the factories were `extern "C"` to bridge from
 * the C dispatcher; the parsers are pure C now and the bridge is no
 * longer needed.  Plain C decls below. */
CDAccess *CDAccess_Image_New(const char *path, bool image_memcache);
CDAccess *CDAccess_CCD_New  (const char *path, bool image_memcache);
#ifdef HAVE_CHD
CDAccess *CDAccess_CHD_New  (const char *path, bool image_memcache);
#endif

static int has_ext(const char *path, const char *ext3)
{
   size_t n = strlen(path);
   if (n < 4 || path[n - 4] != '.')
      return 0;
   return strcasecmp(path + n - 3, ext3) == 0;
}

CDAccess *CDAccess_Open(const char *path, bool image_memcache)
{
   if (has_ext(path, "ccd"))
      return CDAccess_CCD_New(path, image_memcache);
#ifdef HAVE_CHD
   if (has_ext(path, "chd"))
      return CDAccess_CHD_New(path, image_memcache);
#endif
   return CDAccess_Image_New(path, image_memcache);
}
