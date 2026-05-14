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

#include <mednafen/state.h>


class AK93C45
{
 public:

 AK93C45(void) MDFN_COLD; //int32_t timestamp_rate);
 ~AK93C45(void) MDFN_COLD;

 void Init(void) MDFN_COLD;

 void Power(void) MDFN_COLD;

 void ResetTS(void);
 void SetTSFreq(const int32_t rate);

 bool UpdateBus(int32_t timestamp, bool cs, bool sk, bool di);

 void StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname) MDFN_COLD;
 //
 //
 //
 INLINE uint16_t PeekMem(unsigned a)
 {
  assert(a < 0x40);

  return mem[a];
 }

 INLINE void PokeMem(unsigned a, uint16_t v)
 {
  assert(a < 0x40);

  mem[a] = v;
 }

 uint16_t mem[0x40];

 private:
 bool write_enable;

 uint16_t addr;
 uint16_t data_buffer;
 uint8_t counter;
 uint8_t opcode;

 bool dout;

 //const int32_t ts_rate;
 bool prev_cs, prev_sk;

 enum
 {
  PHASE_IDLE = 0,
  PHASE_WAIT_START,
  PHASE_OPCODE,

  PHASE_ADDR,
  PHASE_DATA,

  PHASE_WRITE_PENDING,
  PHASE_WRITING 
 };
 uint32_t phase;

 int64_t write_finish_counter;

 int32_t tsratio;
 int32_t lastts;
};

#endif
