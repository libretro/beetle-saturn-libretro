/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* smpc_iodevice.h:
**  Copyright (C) 2015-2017 Mednafen Team
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

#ifndef __MDFN_SS_SMPC_IODEVICE_H
#define __MDFN_SS_SMPC_IODEVICE_H

#include <mednafen/state.h>

class IODevice
{
 public:

 IODevice() MDFN_COLD;
 virtual ~IODevice() MDFN_COLD;

 virtual void Power(void) MDFN_COLD;

 virtual void TransformInput(uint8_t* const data, float gun_x_scale, float gun_x_offs) const;
 //
 // time_elapsed is emulated time elapsed since last call to UpdateInput(), in microseconds;
 // it's mostly for keyboard emulation, to keep the implementation from becoming unnecessarily complex.
 //
 virtual void UpdateInput(const uint8_t* data, const int32_t time_elapsed);
 virtual void UpdateOutput(uint8_t* data);
 virtual void StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname_prefix) MDFN_COLD;

 virtual void Draw(MDFN_Surface* surface, const MDFN_Rect& drect, const int32_t* lw, int ifield, float gun_x_scale, float gun_x_offs) const;

 //
 // timestamp passed to UpdateBus() and LineHook() may exceed that as specified by NextEventTS under certain conditions(behind emulated multitap) for performance reasons,
 // so write code that can handle this.
 //
 virtual uint8_t UpdateBus(const sscpu_timestamp_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);
 virtual void LineHook(const sscpu_timestamp_t timestamp, int32_t out_line, int32_t div, int32_t coord_adj);
 virtual void ResetTS(void);
 // SetTSFreq is called when the emulator's master clock changes; ST-V's
 // EEPROM (AK93C45) needs this to recompute its time-base ratio. Base
 // implementation is empty -- standard Saturn input devices don't care.
 virtual void SetTSFreq(const int32_t rate);
 sscpu_timestamp_t NextEventTS = SS_EVENT_DISABLED_TS;
 sscpu_timestamp_t LastTS = 0;
};

#endif
