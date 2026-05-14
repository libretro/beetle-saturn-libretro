/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* stvio.h:
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

#ifndef __MDFN_SS_STVIO_H
#define __MDFN_SS_STVIO_H

#include <mednafen/state.h>

#include "../cdstream.h"

#include "smpc_iodevice.h"
#include "db.h"

void STVIO_Init(const STVGameInfo* sgi) MDFN_COLD;
void STVIO_Reset(bool powering_up) MDFN_COLD;
void STVIO_StateAction(StateMem* sm, const unsigned load, const bool data_only) MDFN_COLD;

void STVIO_LoadNV(cdstream* s) MDFN_COLD;
void STVIO_SaveNV(cdstream* s) MDFN_COLD;

void STVIO_WriteIOGA(const sscpu_timestamp_t timestamp, uint8_t A, uint8_t V) MDFN_HOT;
uint8_t STVIO_ReadIOGA(const sscpu_timestamp_t timestamp, uint8_t A) MDFN_HOT;

void STVIO_TransformInput(void);
void STVIO_UpdateInput(int32_t elapsed_time);
void STVIO_SetInput(unsigned port, const char* type, uint8_t* ptr) MDFN_COLD;
void STVIO_SetCrosshairsColor(unsigned port, uint32_t color) MDFN_COLD;

void STVIO_InsertCoin(void);

IODevice* STVIO_GetSMPCDevice(bool sport) MDFN_COLD;

#endif
