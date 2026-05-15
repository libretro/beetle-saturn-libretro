#ifndef __DISC_H__
#define __DISC_H__

#include <libretro.h>
/* disc.h's only need from git.h is MDFNGI (for disc_load_content's
 * signature).  MDFNGI is POD and factored out into mdfn_gameinfo.h
 * which is C-includable; git.h's other content (class MDFNGI's
 * surroundings -- std::vector / std::string / std::exception) is
 * C++-only and not needed here. */
#include "mednafen/mdfn_gameinfo.h"
#include "mednafen/mednafen-types.h"

/* disc.cpp -> disc.c: definitions move to C linkage, but
 * libretro.cpp (C++) calls these.  Force C linkage on both sides
 * so the C++ caller doesn't mangle and the linker resolves
 * against the unmangled symbols disc.c emits.  Same pattern as
 * ss.h / smpc.h / sound.h / input.h / db.h. */
#ifdef __cplusplus
extern "C" {
#endif

extern void extract_basename(char *buf, const char *path, size_t size);
extern void extract_directory(char *buf, const char *path, size_t size);

// These routines handle disc drive front-end.

extern unsigned disk_get_image_index(void);

void disc_init( retro_environment_t environ_cb );

void disc_cleanup(void);

bool DetectRegion( unsigned* region );

bool DiscSanityChecks(void);

void disc_select( unsigned disc_num );

bool disc_load_content( MDFNGI* game_inteface, const char *name, uint8_t* fd_id, char* sgid, char *sgname, char *sgarea, bool image_memcache );

#ifdef __cplusplus
}
#endif

#endif
