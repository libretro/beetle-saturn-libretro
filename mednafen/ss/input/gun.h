/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* gun.h:
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

#ifndef __MDFN_SS_INPUT_GUN_H
#define __MDFN_SS_INPUT_GUN_H

#include <mednafen/state.h>

class IODevice_Gun final : public IODevice
{
 public:
 IODevice_Gun() MDFN_COLD;
 virtual ~IODevice_Gun() override MDFN_COLD;

 virtual void Power(void) override MDFN_COLD;
 virtual void TransformInput(uint8_t* const data, float gun_x_scale, float gun_x_offs) const override;
 virtual void UpdateInput(const uint8_t* data, const int32_t time_elapsed) override;
 virtual void StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname_prefix) override;
 virtual void Draw(MDFN_Surface* surface, const MDFN_Rect& drect, const int32_t* lw, int ifield, float gun_x_scale, float gun_x_offs) const override;

 virtual uint8_t UpdateBus(const sscpu_timestamp_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted) override;

 virtual void LineHook(const sscpu_timestamp_t timestamp, int32_t out_line, int32_t div, int32_t coord_adj) override;

 void SetCrosshairsColor(uint32_t color);

 private:

 void UpdateLight(const sscpu_timestamp_t timestamp);

 uint8_t state;

 int32_t osshot_counter;
 bool prev_ossb;

 //
 int32_t nom_coord[2];
 bool light_phase;
 int32_t light_phase_counter;

 int chair_r, chair_g, chair_b;
};

#endif
