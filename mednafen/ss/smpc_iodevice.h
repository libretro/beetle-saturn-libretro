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

#include <stdint.h>
#include <boolean.h>

#include "../state.h"
#include "../video/surface.h"

/* Formerly a struct hierarchy: `class IODevice` plus nine derived
   device classes, each in its own input/<device>.{h,cpp}. Converted
   to C following the pattern beetle-psx-libretro used for the same
   hierarchy (mednafen/psx/frontio.c): one base struct carrying a
   vtable pointer, concrete device structs that embed it as their
   first member, a static const vtable per device, and factory
   functions. The base and all nine devices now live together in
   smpc_iodevice.c, with the concrete device structs file-local. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IODevice        IODevice;
typedef struct IODevice_VTable IODevice_VTable;

/* Per-device dispatch table. Every concrete device type has one
   static const instance and points its embedded IODevice::vt at it.
   All polymorphic calls go through self->vt->Method(self, ...).
   Slots a device does not override point at the base IODevice_base_*
   implementation. */
struct IODevice_VTable
{
   void    (*Power)(IODevice *self_);
   void    (*TransformInput)(IODevice *self_, uint8_t *data,
                             int32_t gun_x_sn, int32_t gun_x_on, int32_t gun_x_d);
   void    (*UpdateInput)(IODevice *self_, const uint8_t *data,
                          int32_t time_elapsed);
   void    (*UpdateOutput)(IODevice *self_, uint8_t *data);
   void    (*StateAction)(IODevice *self_, StateMem *sm,
                          const unsigned load, const bool data_only,
                          const char *sname_prefix);
   void    (*Draw)(IODevice *self_, MDFN_Surface *surface,
                   const MDFN_Rect *drect, const int32_t *lw,
                   int ifield, int32_t gun_x_sn, int32_t gun_x_on, int32_t gun_x_d);
   uint8_t (*UpdateBus)(IODevice *self_, const int32_t timestamp,
                        const uint8_t smpc_out,
                        const uint8_t smpc_out_asserted);
   void    (*LineHook)(IODevice *self_, const int32_t timestamp,
                       int32_t out_line, int32_t div, int32_t coord_adj);
   void    (*ResetTS)(IODevice *self_);
   void    (*SetTSFreq)(IODevice *self_, const int32_t rate);
   /* Per-device teardown. NULL when the device owns no extra state.
      The IODevice block itself is freed by IODevice_Free. */
   void    (*Destroy)(IODevice *self_);
};

/* Shared state for every device. Was the data members of the C++
   base class; exposed here because every concrete-device struct
   embeds an IODevice as its first member. */
struct IODevice
{
   const IODevice_VTable *vt;
   int32_t NextEventTS;
   int32_t LastTS;
};

/* Base default-implementation vtable: empty Power/UpdateInput/etc.,
   UpdateBus returns smpc_out unchanged, ResetTS rebases the TS
   fields. Concrete device vtables reuse these slots for any method
   they do not override. */
extern const IODevice_VTable IODevice_base_vtable;

/* Base initializer: sets vt = &IODevice_base_vtable and the TS
   fields. Concrete *_Create functions call this, then override vt. */
void IODevice_Ctor(IODevice *self_);

/* Factory functions -- allocate, construct, return. The returned
   pointer is freed with IODevice_Free, which invokes the Destroy
   vtable slot (if any) before freeing the block. */
IODevice *IODevice_None_Create(void);
IODevice *IODevice_Gamepad_Create(void);
IODevice *IODevice_3DPad_Create(void);
IODevice *IODevice_Mouse_Create(void);
IODevice *IODevice_Wheel_Create(void);
IODevice *IODevice_Mission_Create(bool dual);
IODevice *IODevice_Gun_Create(void);
IODevice *IODevice_Keyboard_Create(void);
IODevice *IODevice_JPKeyboard_Create(void);
IODevice *IODevice_Multitap_Create(void);

void      IODevice_Free(IODevice *self_);

/* Multitap-specific helpers (were public methods on IODevice_Multitap). */
void      IODevice_Multitap_SetSubDevice(IODevice *mt, unsigned sub_index, IODevice *device);
IODevice *IODevice_Multitap_GetSubDevice(IODevice *mt, unsigned sub_index);
void      IODevice_Multitap_ForceSubUpdate(IODevice *mt, const int32_t timestamp);

/* Gun-specific helper (was a public non-virtual method). */
void      IODevice_Gun_SetCrosshairsColor(IODevice *gun, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif
