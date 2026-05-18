/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss_state.h:  cross-TU declarations for the save-state and
**              backup-RAM/cart-NV I/O entry points whose bodies
**              live in ss.c.  The eight SS_{Save,Load,Backup}*
**              functions are presently only called from inside
**              ss.c -- they were promoted to TU-external linkage
**              during the original C++ -> C migration (when they
**              lived in a separate ss_state.c TU) and the ss_state.c
**              TU has since been merged back into ss.c.  The
**              decls remain because the ninth symbol declared
**              here -- LibRetro_StateAction -- is consumed cross-
**              TU by mednafen/state.c and scu.h's state-action
**              dispatcher; the rest are grouped alongside it for
**              cohesion.  BRAM_Init_Data is the BackupRAM "fresh
**              format" prefix that ss.c stamps at boot and
**              restores on a failed save-file read.
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
#include "../mednafen-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* BackupRAM "fresh-format" prefix.  Stamped by InitCommon at boot
 * into the first 0x10 bytes of BackupRAM; restored by
 * SS_LoadBackupRAM after a short or failed save-file read.  Body
 * lives in ss.c. */
extern const uint8_t BRAM_Init_Data[0x10];

/* The eight file-I/O entry points called from ss.c's InitCommon /
 * Emulate / Cleanup paths.  Naming is unchanged from when these
 * were file-static in the pre-conversion code. */
void SS_SaveBackupRAM(void)   MDFN_COLD;
void SS_LoadBackupRAM(void)   MDFN_COLD;
void SS_SaveCartNV(void)      MDFN_COLD;
void SS_LoadCartNV(void)      MDFN_COLD;
void SS_SaveRTC(void)         MDFN_COLD;
void SS_LoadRTC(void)         MDFN_COLD;
void SS_BackupBackupRAM(void) MDFN_COLD;
void SS_BackupCartNV(void)    MDFN_COLD;

/* Emulator state save/load orchestration.  Reaches into ss.c
 * globals (NeedEmuICache, BIOS_SHA256, ActiveCartType,
 * BackupRAM_StateHelper, WorkRAML/H, UpdateInputLastBigTS,
 * SH7095_DB) and dispatches to SH7095's StateAction /
 * PostStateLoad methods through the SH7095_{M,S}_StateAction /
 * SH7095_{M,S}_PostStateLoad wrappers defined in ss.c.  Called
 * from mednafen/state.c and from scu.h's state-action dispatcher. */
int LibRetro_StateAction(StateMem* sm, const unsigned load) MDFN_COLD;

#ifdef __cplusplus
}
#endif

#endif
