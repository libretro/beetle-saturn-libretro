/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* multitap.h:
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

#ifndef __MDFN_SS_INPUT_MULTITAP_H
#define __MDFN_SS_INPUT_MULTITAP_H

#include <mednafen/state.h>


class IODevice_Multitap final : public IODevice
{
 public:
 IODevice_Multitap() MDFN_COLD;
 virtual ~IODevice_Multitap() override MDFN_COLD;

 virtual void Power(void) override MDFN_COLD;
 virtual void StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname_prefix) override MDFN_COLD;
 virtual void Draw(MDFN_Surface* surface, const MDFN_Rect& drect, const int32_t* lw, int ifield, float gun_x_scale, float gun_x_offs) const override;
 virtual uint8_t UpdateBus(const sscpu_timestamp_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted) override;
 virtual void LineHook(const sscpu_timestamp_t timestamp, int32_t out_line, int32_t div, int32_t coord_adj) override;
 virtual void ResetTS(void) override;

 void ForceSubUpdate(const sscpu_timestamp_t timestamp);
 void SetSubDevice(unsigned sub_index, IODevice* device);
 IODevice* GetSubDevice(unsigned sub_index);

 private:

 uint8_t UASB(const sscpu_timestamp_t timestamp);

 IODevice* devices[6];
 uint8_t sub_state[6];
 uint8_t tmp[4];
 uint8_t id1;
 uint8_t id2;
 uint8_t data_out;
 bool tl;
 int32_t phase;
 uint8_t port_counter;
 uint8_t read_counter;
};


#endif
