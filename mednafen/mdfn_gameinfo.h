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
   /* Time base for EmulateSpecStruct::MasterCycles.
      MasterClock must be >= MDFN_MASTERCLOCK_FIXED(1.0). All or part
      of the fractional component may be ignored in some timekeeping
      operations to prevent integer overflow, so it is unwise to have
      a fractional component when the integral component is very
      small (less than say, 10000). */
   #define MDFN_MASTERCLOCK_FIXED(n)	((int64_t)((double)(n) * (1LL << 32)))
   int64_t MasterClock;

   int lcm_width;
   int lcm_height;

   void *dummy_separator;

   int nominal_width;
   int nominal_height;

   int fb_width;	/* Width of the framebuffer (not necessarily width of the image). MDFN_Surface width should be >= this. */
   int fb_height;	/* Height of the framebuffer passed to the Emulate() function (not necessarily height of the image). */

   uint8_t MD5[16];

   /* For mouse relative motion. */
   double mouse_sensitivity;

   /* For absolute coordinates (IDIT_X_AXIS and IDIT_Y_AXIS), usually
      mapped to a mouse (hence the naming). */
   float mouse_scale_x, mouse_scale_y;
   float mouse_offs_x, mouse_offs_y;
} MDFNGI;

extern MDFNGI *MDFNGameInfo;

#ifdef __cplusplus
}
#endif

#endif
