/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* ak93c45.h:
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

#ifndef __MDFN_SS_AK93C45_H
#define __MDFN_SS_AK93C45_H

#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <assert.h>

#include <mednafen/mednafen-types.h>   /* MDFN_COLD */
#include <retro_inline.h>              /* INLINE */
#include <mednafen/state.h>

/* The AK93C45 serial EEPROM (used by the ST-V cart hardware).
   Converted from a C++ class to a C struct + free functions; it had
   no inheritance, no virtual methods, and no templates, so the
   conversion is mechanical. The sole consumer is stvio.c, which
   holds one by-value instance and now calls the free functions as
   AK93C45_Method(&eep, ...). */

#ifdef __cplusplus
extern "C" {
#endif

enum
{
   AK93C45_PHASE_IDLE = 0,
   AK93C45_PHASE_WAIT_START,
   AK93C45_PHASE_OPCODE,

   AK93C45_PHASE_ADDR,
   AK93C45_PHASE_DATA,

   AK93C45_PHASE_WRITE_PENDING,
   AK93C45_PHASE_WRITING
};

typedef struct
{
   uint16_t mem[0x40];

   bool     write_enable;

   uint16_t addr;
   uint16_t data_buffer;
   uint8_t  counter;
   uint8_t  opcode;

   bool     dout;

   bool     prev_cs, prev_sk;

   uint32_t phase;

   int64_t  write_finish_counter;

   int32_t  tsratio;
   int32_t  lastts;
} AK93C45;

void AK93C45_Init(AK93C45 *self) MDFN_COLD;
void AK93C45_Power(AK93C45 *self) MDFN_COLD;

void AK93C45_ResetTS(AK93C45 *self);
void AK93C45_SetTSFreq(AK93C45 *self, const int32_t rate);

bool AK93C45_UpdateBus(AK93C45 *self, int32_t timestamp, bool cs, bool sk, bool di);

void AK93C45_StateAction(AK93C45 *self, StateMem *sm, const unsigned load, const bool data_only, const char *sname) MDFN_COLD;

static INLINE uint16_t AK93C45_PeekMem(AK93C45 *self, unsigned a)
{
   assert(a < 0x40);

   return self->mem[a];
}

static INLINE void AK93C45_PokeMem(AK93C45 *self, unsigned a, uint16_t v)
{
   assert(a < 0x40);

   self->mem[a] = v;
}

#ifdef __cplusplus
}
#endif

#endif
