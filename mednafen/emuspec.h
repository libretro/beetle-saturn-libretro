#ifndef __MDFN_EMUSPEC_H
#define __MDFN_EMUSPEC_H

#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "video/surface.h"

/* The EmulateSpecStruct POD typedef, factored out of git.h so it can
   be included from plain C translation units.  git.h itself is C++
   (it pulls in <algorithm>, <string>, <vector>, <map>), but
   EmulateSpecStruct is pure POD and several TUs now need it.

   git.h #includes this header in place of its former inline copy,
   so the C++ side sees the identical type.

   Pre-conversion the struct had C++ default member initializers
   (every field defaulted to NULL / 0 / false).  Those are dropped
   here because they're C++-only syntax; only one TU in the tree
   (the libretro entry-point, converted to C in the same commit
   that introduced this header) ever stack-instantiated the struct
   without an explicit initializer, and that call site moved to
   `EmulateSpecStruct spec = {0};` which yields the same zero-init
   semantics. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EmulateSpecStruct
{
   /* Width(32-bit) must be equal to width and >= the "fb_width" in
      the MDFNGI struct for the emulated system.  Height must be >=
      to the "fb_height" specified there.  surface->pixels is
      written to by the system emulation code. */
   MDFN_Surface* surface;

   /* Horizontal / vertical offsets of the image, and the size of
      the image.  If the emulated system sets the elements of
      LineWidths, then the width(w) of this structure is ignored
      while drawing the image. */
   MDFN_Rect DisplayRect;

   /* Pointer to an array of int32_t, number of elements = fb_height,
      set by the driver code.  Individual elements written to by
      system emulation code.  If the emulated system doesn't support
      multiple screen widths per frame, or if you handle such a
      situation by outputting at a constant width-per-frame, you can
      ignore this.  If you do wish to use this, you must set all
      elements every frame. */
   int32_t *LineWidths;

   /* If InterlaceOn is true, assume field height is 1/2
      DisplayRect.h, and only every other line in surface (with the
      start line defined by InterlacedField) has valid data (it's up
      to internal Mednafen code to deinterlace it). */
   bool InterlaceOn;
   bool InterlaceField;

   /* Skip rendering this frame if true.  Set by the driver code. */
   int skip;

   /* Number of frames currently in internal sound buffer.  Set by
      the system emulation code, to be read by the driver code. */
   int32_t SoundBufSize;

   /* Number of cycles that this frame consumed, using
      MDFNGI::MasterClock as a time base.  Set by emulation code.
      MasterCycles value at last MidSync(), 0 if mid sync isn't
      implemented for the emulation module in use. */
   int64_t MasterCycles;
} EmulateSpecStruct;

#ifdef __cplusplus
}
#endif

#endif
