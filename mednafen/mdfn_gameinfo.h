#ifndef __MDFN_MDFN_GAMEINFO_H
#define __MDFN_MDFN_GAMEINFO_H

#include <stdint.h>

/* The MDFNGI "game info" struct and the MDFNGameInfo global.

   Factored out of git.h so it can be included from plain C
   translation units. git.h itself is C++ (it pulls in <algorithm>,
   <string>, <vector>, <map>), but the MDFNGI typedef is pure POD and
   several C files now need it -- e.g. smpc_iodevice.c, where the gun
   device reads MDFNGameInfo's nominal_width and mouse_* fields when
   drawing crosshairs. git.h #includes this header in place of its
   former inline copy, so the C++ side sees the identical type. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MDFNGI
{
   /* Time base for EmulateSpecStruct::MasterCycles, as a Q32.32
      fixed-point Hz value: integral Hz in the high 32 bits, fractional
      Hz in the low 32.  Must be >= (1 << 32) (i.e. >= 1.0 Hz).  All or
      part of the fractional component may be ignored in some
      timekeeping operations to prevent integer overflow, so it is
      unwise to have a fractional component when the integral component
      is very small (less than say, 10000).

      Read by the input-update timing math in ss.c (Emulate's elapsed
      time conversion and CART_SetCPUClock); written once by InitCommon
      when the master clock for the selected region is known, as
      (int64_t)hz << 32 -- an exact integer scale (a power-of-two shift
      spends no mantissa bits), keeping the timing path free of host
      floating point. */
   int64_t MasterClock;

   /* Logical display width in nominal-coordinates units.  Used by
      smpc_iodevice.c's gun crosshair / hit-point calculation to
      convert nominal coordinates into visible-pixel coordinates.
      Set by VDP2REND_Init when display geometry is known; init
      value is the conservative 320 used before the first geometry
      update. */
   int nominal_width;

   /* Mouse / gun coordinate transform.  In smpc_iodevice.c the gun
      device combines these as:

        cx = (input_x - mouse_offs_x) * (visible_pixels / mouse_scale_x)
        cy = (input_y - mouse_offs_y) + visible_top

      Set by VDP2REND_Init when display geometry is known. */
   float mouse_scale_x;
   float mouse_offs_x, mouse_offs_y;
} MDFNGI;

extern MDFNGI *MDFNGameInfo;

#ifdef __cplusplus
}
#endif

#endif
