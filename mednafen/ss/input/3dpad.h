/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* 3dpad.h:
**  Copyright (C) 2016-2017 Mednafen Team
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

#ifndef __MDFN_SS_INPUT_3DPAD_H
#define __MDFN_SS_INPUT_3DPAD_H

#include <mednafen/state.h>


class IODevice_3DPad final : public IODevice
{
 public:
 IODevice_3DPad() MDFN_COLD;
 virtual ~IODevice_3DPad() override MDFN_COLD;

 virtual void Power(void) override MDFN_COLD;
 virtual void UpdateInput(const uint8_t* data, const int32_t time_elapsed) override;
 virtual void StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname_prefix) override MDFN_COLD;

 virtual uint8_t UpdateBus(const sscpu_timestamp_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted) override;

 private:
 uint16_t dbuttons;
 uint8_t thumb[2];
 uint8_t shoulder[2];

 uint8_t buffer[0x10];
 uint8_t data_out;
 bool tl;
 int8_t phase;
 bool mode;
};

#endif
