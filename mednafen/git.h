#ifndef __MDFN_GIT_H
#define __MDFN_GIT_H

#include <libretro.h>

#include "video/surface.h"

#include "state.h"

struct MemoryPatch;

/* EmulateSpecStruct now lives in mednafen/emuspec.h so it can be
   included from C TUs (the libretro entry-point is converted to C
   in the same commit that introduced emuspec.h).  This include
   gives C++ TUs the same POD typedef they had before; the only
   semantic change is that the C++-only default member initializers
   are gone -- callers that previously did `EmulateSpecStruct spec;`
   relying on those defaults now zero-init explicitly. */
#include "emuspec.h"

//===========================================

#include "mdfn_gameinfo.h"

//===========================================

int StateAction(StateMem *sm, int load, int data_only);

extern retro_log_printf_t log_cb;

#endif
