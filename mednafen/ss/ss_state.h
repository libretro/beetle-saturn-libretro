/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss_state.h:  Phase-7b cross-TU declarations between ss.cpp and ss_state.c.
**              The 8 file-I/O entry points listed here used to be file-static
**              in ss.cpp; promoting them to TU-external linkage lets them
**              live in their own pure-C TU (ss_state.c) while ss.cpp's
**              InitCommon / Emulate / Cleanup paths still call them by name.
**
**  Copyright (C) 2015-2023 Mednafen Team
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

#ifndef __MDFN_SS_STATE_H
#define __MDFN_SS_STATE_H

#include <stdint.h>
#include <mednafen/mednafen-types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BackupRAM "fresh-format" prefix.  Used by InitCommon (ss.cpp)
 * to stamp the first 0x40 bytes of BackupRAM when the emulator
 * boots, and by LoadBackupRAM (ss_state.c) to restore that
 * stamp after a short/failed save-file read.  Defined in
 * ss_state.c so the I/O path owns it; ss.cpp pulls it via this
 * extern. */
extern const uint8_t BRAM_Init_Data[0x10];

/* The eight file-I/O functions extracted from ss.cpp.  Naming
 * is unchanged from the file-static versions they replaced. */
void SS_SaveBackupRAM(void)   MDFN_COLD;
void SS_LoadBackupRAM(void)   MDFN_COLD;
void SS_SaveCartNV(void)      MDFN_COLD;
void SS_LoadCartNV(void)      MDFN_COLD;
void SS_SaveRTC(void)         MDFN_COLD;
void SS_LoadRTC(void)         MDFN_COLD;
void SS_BackupBackupRAM(void) MDFN_COLD;
void SS_BackupCartNV(void)    MDFN_COLD;

#ifdef __cplusplus
}
#endif

#endif
