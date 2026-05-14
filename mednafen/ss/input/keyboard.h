/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* keyboard.h:
**  Copyright (C) 2017 Mednafen Team
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

#ifndef __MDFN_SS_INPUT_KEYBOARD_H
#define __MDFN_SS_INPUT_KEYBOARD_H

#include <mednafen/state.h>

class IODevice_Keyboard final : public IODevice
{
 public:
 IODevice_Keyboard() MDFN_COLD;
 virtual ~IODevice_Keyboard() override MDFN_COLD;

 virtual void Power(void) override MDFN_COLD;
 virtual void UpdateInput(const uint8_t* data, const int32_t time_elapsed) override;
 virtual void UpdateOutput(uint8_t* data) override;
 virtual void StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname_prefix) override;

 virtual uint8_t UpdateBus(const sscpu_timestamp_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted) override;

 private:

 uint64_t phys[4];
 uint64_t processed[4];
 uint8_t lock;
 uint8_t lock_pend;
 uint16_t simbutt;
 uint16_t simbutt_pend;
 enum : int { fifo_size = 16 };
 uint16_t fifo[fifo_size];
 uint8_t fifo_rdp;
 uint8_t fifo_wrp;
 uint8_t fifo_cnt;
 enum
 {
  LOCK_SCROLL = 0x01,
  LOCK_NUM = 0x02,
  LOCK_CAPS = 0x04
 };

 int16_t rep_sc;
 int32_t rep_dcnt;

 int16_t mkbrk_pend;
 uint8_t buffer[12];
 uint8_t data_out;
 bool tl;
 int8_t phase;
};

#endif
