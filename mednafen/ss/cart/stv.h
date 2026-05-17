/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* stv.h - ST-V cart emulation
**  Copyright (C) 2022 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __MDFN_SS_CART_STV_H
#define __MDFN_SS_CART_STV_H

#include <stdint.h>
#include <boolean.h>

#include "../../mednafen-types.h"   /* MDFN_COLD */

#ifdef __cplusplus
extern "C" {
#endif

struct CartInfo;
struct STVGameInfo;

// libretro-fork adaptation of upstream Mednafen's CART_STV_Init.
//
// Upstream takes a GameFile* with an open Stream and a VirtualFS pointer to
// open sibling ROM image files (Mednafen handles ZIP archives transparently
// through its VFS).  The libretro fork doesn't expose VirtualFS, so we take
// a (rom_dir, main_fname) pair and walk the STVGameInfo rom_layout opening
// each entry by path using libretro's filestream_open.
//
// `rom_dir`:    directory containing the ROM image files; no trailing slash.
// `main_fname`: filename of the ROM the user actually loaded (matched against
//               rom_layout[0].fname during DB lookup; passed here so we can
//               skip re-opening it when it equals rom_layout[i].fname).
// `sgi`:        STVGameInfo entry returned by DB_LookupSTV.
//
// Returns true on success, false on I/O error (short read, missing
// sibling file, etc.); CartInfo's resources are cleaned up on the
// failure path before returning.
bool CART_STV_Init(struct CartInfo* c, const char* rom_dir, const char* main_fname, const struct STVGameInfo* sgi) MDFN_COLD;

// Peek a byte from STV ROM, byte-addressed.  Used by stvio.c's InitEEPROM
// to read the game's onboard ROM header for cabinet detection.
uint8_t CART_STV_PeekROM(uint32_t A);

#ifdef __cplusplus
}
#endif

#endif
