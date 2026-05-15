/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss_c_abi.h:
**  Copyright (C) 2015-2020 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
*/

/* ---------------------------------------------------------------------------
** Single source of truth for constants/types that cross the C <-> C++ boundary.
**
** Background: the Saturn core is being incrementally converted from C++ to C.
** A converted C file (e.g. vdp1.c) can no longer #include the still-C++
** ss.h / scu.h. The FIRST conversion pass worked around this by RE-TYPING the
** enum values it needed by hand into vdp1.c -- and miscounted, producing
** SS_EVENT_VDP1 == 7 (should be 6), SS_EVENT_VDP2 == 8 (should be 7) and
** SCU_INT_VDP1 == 4 (should be 13). That broke VDP1 event scheduling and
** wedged games right after the BIOS handoff (Mega Man X4, etc).
**
** Rule going forward, and for the upcoming VDP2 / SCU conversions:
**   DO NOT transcribe a cross-boundary constant into a converted .c file.
**   Put it here. This header is plain C (also valid C++), so the C++ owners
**   (ss.h / scu.h) include it too -- there is exactly ONE definition, so the
**   C and C++ sides cannot drift. tools/check_no_mirrored_constants.sh fails
**   the build if a converted .c file locally redefines a name that a C++
**   header also defines.
** ------------------------------------------------------------------------- */

#ifndef __MDFN_SS_SS_C_ABI_H
#define __MDFN_SS_SS_C_ABI_H

#include <stdint.h>

/* --- ss.h: emulator event scheduler -------------------------------------- */
/* Order is load-bearing: these index events[] in ss.cpp. Keep verbatim. */
enum
{
 SS_EVENT__SYNFIRST = 0,

 SS_EVENT_SH2_M_DMA,
 SS_EVENT_SH2_S_DMA,

 SS_EVENT_SCU_DMA,
 SS_EVENT_SCU_DSP,

 SS_EVENT_SMPC,

 SS_EVENT_VDP1,
 SS_EVENT_VDP2,

 SS_EVENT_CDB,

 SS_EVENT_SOUND,

 SS_EVENT_CART,

 SS_EVENT_MIDSYNC,
 /* SS_EVENT_SCU_INT, */

 SS_EVENT__SYNLAST,
 SS_EVENT__COUNT
};

/* event_list_entry: layout-compatible POD shared with ss.cpp's events[].
** sscpu_timestamp_t is int32_t; spelled int32_t here so this header has no
** include-order dependency. */
typedef struct event_list_entry
{
 int32_t event_time;
} event_list_entry;

/* Sentinel "no event scheduled" timestamp. Savestate-visible, so the value
** is fixed. ss.h spells it `enum : sscpu_timestamp_t { ... }`; plain enum
** here (0x7FFFFFFF fits in int) -- only ever assigned to / compared with
** int32_t timestamps, never used type-sensitively. */
enum { SS_EVENT_DISABLED_TS = 0x7FFFFFFF };

/* --- ss.h: "horrible hacks" bitmask -------------------------------------- */
enum
{
 HORRIBLEHACK_NOSH2DMALINE106    = (1U << 0),
 HORRIBLEHACK_NOSH2DMAPENALTY    = (1U << 1),
 HORRIBLEHACK_VDP1VRAM5000FIX    = (1U << 2),
 HORRIBLEHACK_VDP1RWDRAWSLOWDOWN = (1U << 3),
 HORRIBLEHACK_VDP1INSTANT        = (1U << 4)
 /* HORRIBLEHACK_SCUINTDELAY     = (1U << 5), */
};

/* --- scu.h: SCU interrupt vectors ---------------------------------------- */
enum
{
 SCU_INT_VBIN = 0x00,
 SCU_INT_VBOUT,
 SCU_INT_HBIN,
 SCU_INT_TIMER0,
 SCU_INT_TIMER1,
 SCU_INT_DSP,
 SCU_INT_SCSP,
 SCU_INT_SMPC,
 SCU_INT_PAD,

 SCU_INT_L2DMA,
 SCU_INT_L1DMA,
 SCU_INT_L0DMA,

 SCU_INT_DMA_ILL,

 SCU_INT_VDP1,

 SCU_INT_EXT0 = 0x10,
 SCU_INT_EXTF = 0x1F
};

#endif
