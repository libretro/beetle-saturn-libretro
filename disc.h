#ifndef __DISC_H__
#define __DISC_H__

#include <libretro.h>
#include "mednafen/mednafen-types.h"

/* disc.c's entry points are C functions; the surrounding extern "C"
 * guard is defensive header hygiene for any future C++ consumer.
 * All current callers (libretro.c, ss.c, mempatcher.c) are C and
 * consume these through normal C linkage; the guard is a no-op at
 * every current compile. */
#ifdef __cplusplus
extern "C" {
#endif

extern void extract_basename(char *buf, const char *path, size_t size);
extern void extract_directory(char *buf, const char *path, size_t size);

// These routines handle disc drive front-end.

extern unsigned disk_get_image_index(void);

void disc_init(void);
void disc_register_environment(retro_environment_t environ_cb);

void disc_cleanup(void);

bool DetectRegion( unsigned* region );

bool DiscSanityChecks(void);

void disc_select( unsigned disc_num );

bool disc_load_content( const char *name, uint8_t* fd_id, char* sgid, char *sgname, char *sgarea, bool image_memcache );

#ifdef __cplusplus
}
#endif

#endif
