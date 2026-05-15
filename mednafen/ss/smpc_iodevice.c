/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* smpc_iodevice.c:
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

/* The Saturn input-device hierarchy, converted from a C++ virtual
   class hierarchy (one base + nine derived devices, formerly in
   input/<device>.{h,cpp}) to C.

   Pattern follows beetle-psx-libretro's mednafen/psx/frontio.c, which
   did the same conversion for the equivalent PSX hierarchy: a base
   struct carrying a const-vtable pointer, concrete device structs
   that embed the base as their first member, one static const vtable
   per device, and factory functions. Concrete device structs are
   file-local here; external code only ever holds an IODevice* and
   dispatches through self->vt->Method(self, ...).

   The nine devices in original-file order: gamepad, 3dpad, mouse,
   wheel, mission, gun, keyboard, jpkeyboard, multitap. */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <retro_inline.h>

#include <mednafen/state.h>
#include <mednafen/mdfn_gameinfo.h>

#include "smpc_iodevice.h"
#include "../video/surface.h"
#include "../math_ops.h"        /* MDFN_lzcount64, for the keyboard device */
#include "libretro_settings.h"

/* ss.h is a C++ header (class SH7095, default args, ...), so it
   cannot be included here. Cross-boundary constants come from the
   shared C/C++ leaf header instead of being re-typed -- see
   ss_c_abi.h. */
#include "ss_c_abi.h"

/* trio_snprintf was just `#define trio_snprintf snprintf` in the C++
   build's mednafen.h shim; use snprintf directly. */

/* ------------------------------------------------------------------ */
/* Forward declarations: every static method, vtable, and *_Create     */
/* function, so file order does not matter.                            */
/* ------------------------------------------------------------------ */

/* base */
static void    IODevice_base_Power(IODevice *self_);
static void    IODevice_base_TransformInput(IODevice *self_, uint8_t *data, float gun_x_scale, float gun_x_offs);
static void    IODevice_base_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed);
static void    IODevice_base_UpdateOutput(IODevice *self_, uint8_t *data);
static void    IODevice_base_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static void    IODevice_base_Draw(IODevice *self_, MDFN_Surface *surface, const MDFN_Rect *drect, const int32_t *lw, int ifield, float gun_x_scale, float gun_x_offs);
static uint8_t IODevice_base_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);
static void    IODevice_base_LineHook(IODevice *self_, const int32_t timestamp, int32_t out_line, int32_t div, int32_t coord_adj);
static void    IODevice_base_ResetTS(IODevice *self_);
static void    IODevice_base_SetTSFreq(IODevice *self_, const int32_t rate);

/* ================================================================== */
/* Base IODevice                                                      */
/* ================================================================== */

/* These were inline empty/passthrough definitions in smpc.cpp
   (IODevice::Power, ::UpdateInput, etc.). They are the default
   vtable slots that concrete devices reuse for methods they do not
   override. */

static void IODevice_base_Power(IODevice *self_) { (void)self_; }

static void IODevice_base_TransformInput(IODevice *self_, uint8_t *data, float gun_x_scale, float gun_x_offs)
{
   (void)self_; (void)data; (void)gun_x_scale; (void)gun_x_offs;
}

static void IODevice_base_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed)
{
   (void)self_; (void)data; (void)time_elapsed;
}

static void IODevice_base_UpdateOutput(IODevice *self_, uint8_t *data)
{
   (void)self_; (void)data;
}

static void IODevice_base_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   (void)self_; (void)sm; (void)load; (void)data_only; (void)sname_prefix;
}

static void IODevice_base_Draw(IODevice *self_, MDFN_Surface *surface, const MDFN_Rect *drect, const int32_t *lw, int ifield, float gun_x_scale, float gun_x_offs)
{
   (void)self_; (void)surface; (void)drect; (void)lw; (void)ifield; (void)gun_x_scale; (void)gun_x_offs;
}

static uint8_t IODevice_base_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   (void)self_; (void)timestamp; (void)smpc_out_asserted;
   return smpc_out;
}

static void IODevice_base_LineHook(IODevice *self_, const int32_t timestamp, int32_t out_line, int32_t div, int32_t coord_adj)
{
   (void)self_; (void)timestamp; (void)out_line; (void)div; (void)coord_adj;
}

static void IODevice_base_ResetTS(IODevice *self_)
{
   if(self_->NextEventTS < SS_EVENT_DISABLED_TS)
   {
      self_->NextEventTS -= self_->LastTS;
      assert(self_->NextEventTS >= 0);
   }
   self_->LastTS = 0;
}

static void IODevice_base_SetTSFreq(IODevice *self_, const int32_t rate)
{
   (void)self_; (void)rate;
}

const IODevice_VTable IODevice_base_vtable =
{
   IODevice_base_Power,
   IODevice_base_TransformInput,
   IODevice_base_UpdateInput,
   IODevice_base_UpdateOutput,
   IODevice_base_StateAction,
   IODevice_base_Draw,
   IODevice_base_UpdateBus,
   IODevice_base_LineHook,
   IODevice_base_ResetTS,
   IODevice_base_SetTSFreq,
   NULL                          /* Destroy: base owns no extra state */
};

void IODevice_Ctor(IODevice *self_)
{
   self_->vt          = &IODevice_base_vtable;
   self_->NextEventTS = SS_EVENT_DISABLED_TS;
   self_->LastTS      = 0;
}

IODevice *IODevice_None_Create(void)
{
   IODevice *d = (IODevice *)calloc(1, sizeof(IODevice));
   if (!d)
      return NULL;
   IODevice_Ctor(d);
   return d;
}

void IODevice_Free(IODevice *self_)
{
   if(!self_)
      return;
   if(self_->vt->Destroy)
      self_->vt->Destroy(self_);
   free(self_);
}

/* ================================================================== */
/* IODevice_Gamepad -- Digital Gamepad                                */
/* ================================================================== */

typedef struct
{
   IODevice base;
   uint16_t buttons;
} IODevice_Gamepad;

static void    IODevice_Gamepad_Power(IODevice *self_);
static void    IODevice_Gamepad_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed);
static void    IODevice_Gamepad_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static uint8_t IODevice_Gamepad_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);

static void IODevice_Gamepad_Power(IODevice *self_)
{
   (void)self_;
}

static void IODevice_Gamepad_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed)
{
   IODevice_Gamepad *self = (IODevice_Gamepad *)self_;
   (void)time_elapsed;
   self->buttons = (~(data[0] | (data[1] << 8))) &~ 0x3000;
}

static void IODevice_Gamepad_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   IODevice_Gamepad *self = (IODevice_Gamepad *)self_;
   SFORMAT StateRegs[] =
   {
      SFVAR(self->buttons),
      SFEND
   };
   char section_name[64];
   snprintf(section_name, sizeof(section_name), "%s_Gamepad", sname_prefix);

   if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name, true) && load)
      IODevice_Gamepad_Power(self_);
   else if(load)
      self->buttons = (self->buttons | 0x4000) &~ 0x3000;
}

static uint8_t IODevice_Gamepad_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_Gamepad *self = (IODevice_Gamepad *)self_;
   uint8_t tmp;
   (void)timestamp;

   tmp = (self->buttons >> ((smpc_out >> 5) << 2)) & 0xF;

   return 0x10 | (smpc_out & (smpc_out_asserted | 0xE0)) | (tmp &~ smpc_out_asserted);
}

static const IODevice_VTable IODevice_Gamepad_vtable =
{
   IODevice_Gamepad_Power,
   IODevice_base_TransformInput,
   IODevice_Gamepad_UpdateInput,
   IODevice_base_UpdateOutput,
   IODevice_Gamepad_StateAction,
   IODevice_base_Draw,
   IODevice_Gamepad_UpdateBus,
   IODevice_base_LineHook,
   IODevice_base_ResetTS,
   IODevice_base_SetTSFreq,
   NULL
};

IODevice *IODevice_Gamepad_Create(void)
{
   IODevice_Gamepad *self = (IODevice_Gamepad *)calloc(1, sizeof(IODevice_Gamepad));
   if (!self)
      return NULL;
   IODevice_Ctor(&self->base);
   self->base.vt = &IODevice_Gamepad_vtable;
   self->buttons = 0xCFFF;
   return &self->base;
}

/* ================================================================== */
/* IODevice_3DPad -- 3D Control Pad                                   */
/* ================================================================== */

typedef struct
{
   IODevice base;
   uint16_t dbuttons;
   uint8_t  thumb[2];
   uint8_t  shoulder[2];
   uint8_t  buffer[0x10];
   uint8_t  data_out;
   bool     tl;
   int8_t   phase;
   bool     mode;
} IODevice_3DPad;

static void    IODevice_3DPad_Power(IODevice *self_);
static void    IODevice_3DPad_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed);
static void    IODevice_3DPad_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static uint8_t IODevice_3DPad_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);

static void IODevice_3DPad_Power(IODevice *self_)
{
   IODevice_3DPad *self = (IODevice_3DPad *)self_;
   self->phase    = -1;
   self->tl       = true;
   self->data_out = 0x01;
}

static void IODevice_3DPad_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed)
{
   IODevice_3DPad *self = (IODevice_3DPad *)self_;
   const uint16_t dtmp  = (uint16_t)(data[0] | (data[1] << 8));
   unsigned axis, w;
   (void)time_elapsed;

   self->dbuttons = (self->dbuttons & 0x8800) | (dtmp & 0x0FFF);
   self->mode     = (bool)(dtmp & 0x1000);

   for(axis = 0; axis < 2; axis++)
   {
      const unsigned off = 0x2 + (axis << 1);
      int32_t tmp = (uint16_t)(data[off] | (data[off + 1] << 8));

      if(tmp >= (32768 - 128) && tmp < 32768)
         tmp = 32768;

      tmp = (tmp * 255 + 32767) / 65535;
      self->thumb[axis] = tmp;
   }

   for(w = 0; w < 2; w++)
   {
      const unsigned off = 0x6 + (w << 1);
      self->shoulder[w] = (((uint16_t)(data[off] | (data[off + 1] << 8))) * 255 + 32767) / 65535;

      /* May not be right for digital mode, but shouldn't matter too much: */
      if(self->shoulder[w] <= 0x55)
         self->dbuttons &= ~(0x0800 << (w << 2));
      else if(self->shoulder[w] >= 0x8E)
         self->dbuttons |= 0x0800 << (w << 2);
   }
}

static void IODevice_3DPad_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   IODevice_3DPad *self = (IODevice_3DPad *)self_;
   SFORMAT StateRegs[] =
   {
      SFVAR(self->dbuttons),
      SFVAR(self->mode),

      SFPTR8N(&(self->thumb)[0], (sizeof(self->thumb) / sizeof(uint8_t)), "thumb"),
      SFPTR8N(&(self->shoulder)[0], (sizeof(self->shoulder) / sizeof(uint8_t)), "shoulder"),

      SFPTR8N(&(self->buffer)[0], (sizeof(self->buffer) / sizeof(uint8_t)), "buffer"),
      SFVAR(self->data_out),
      SFVAR(self->tl),

      SFVAR(self->phase),
      SFEND
   };
   char section_name[64];
   snprintf(section_name, sizeof(section_name), "%s_3DPad", sname_prefix);

   if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name, true) && load)
      IODevice_3DPad_Power(self_);
   else if(load)
   {
      if(self->phase < 0)
         self->phase = -1;
      else
         self->phase &= 0xF;
   }
}

static uint8_t IODevice_3DPad_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_3DPad *self = (IODevice_3DPad *)self_;
   uint8_t tmp;
   (void)timestamp;

   if(smpc_out & 0x40)
   {
      self->phase    = -1;
      self->tl       = true;
      self->data_out = 0x01;
   }
   else
   {
      if((bool)(smpc_out & 0x20) != self->tl)
      {
         if(self->phase < 15)
         {
            self->tl = !self->tl;
            self->phase++;
         }

         if(!self->phase)
         {
            if(self->mode)
            {
               self->buffer[ 0] = 0x1;
               self->buffer[ 1] = 0x6;
               self->buffer[ 2] = (((self->dbuttons >>  0) & 0xF) ^ 0xF);
               self->buffer[ 3] = (((self->dbuttons >>  4) & 0xF) ^ 0xF);
               self->buffer[ 4] = (((self->dbuttons >>  8) & 0xF) ^ 0xF);
               self->buffer[ 5] = (((self->dbuttons >> 12) & 0xF) ^ 0xF);
               self->buffer[ 6] = (self->thumb[0] >> 4) & 0xF;
               self->buffer[ 7] = (self->thumb[0] >> 0) & 0xF;
               self->buffer[ 8] = (self->thumb[1] >> 4) & 0xF;
               self->buffer[ 9] = (self->thumb[1] >> 0) & 0xF;
               self->buffer[10] = (self->shoulder[0] >> 4) & 0xF;
               self->buffer[11] = (self->shoulder[0] >> 0) & 0xF;
               self->buffer[12] = (self->shoulder[1] >> 4) & 0xF;
               self->buffer[13] = (self->shoulder[1] >> 0) & 0xF;
               self->buffer[14] = 0x0;
               self->buffer[15] = 0x1;
            }
            else
            {
               self->phase = 8;
               self->buffer[ 8] = 0x0;
               self->buffer[ 9] = 0x2;
               self->buffer[10] = (((self->dbuttons >>  0) & 0xF) ^ 0xF);
               self->buffer[11] = (((self->dbuttons >>  4) & 0xF) ^ 0xF);
               self->buffer[12] = (((self->dbuttons >>  8) & 0xF) ^ 0xF);
               self->buffer[13] = (((self->dbuttons >> 12) & 0xF) ^ 0xF);
               self->buffer[14] = 0x0;
               self->buffer[15] = 0x1;
            }
         }

         self->data_out = self->buffer[self->phase];
      }
   }

   tmp = (self->tl << 4) | self->data_out;

   return (smpc_out & (smpc_out_asserted | 0xE0)) | (tmp &~ smpc_out_asserted);
}

static const IODevice_VTable IODevice_3DPad_vtable =
{
   IODevice_3DPad_Power,
   IODevice_base_TransformInput,
   IODevice_3DPad_UpdateInput,
   IODevice_base_UpdateOutput,
   IODevice_3DPad_StateAction,
   IODevice_base_Draw,
   IODevice_3DPad_UpdateBus,
   IODevice_base_LineHook,
   IODevice_base_ResetTS,
   IODevice_base_SetTSFreq,
   NULL
};

IODevice *IODevice_3DPad_Create(void)
{
   IODevice_3DPad *self = (IODevice_3DPad *)calloc(1, sizeof(IODevice_3DPad));
   if (!self)
      return NULL;
   IODevice_Ctor(&self->base);
   self->base.vt   = &IODevice_3DPad_vtable;
   self->dbuttons  = 0;
   self->mode      = false;
   return &self->base;
}

/* ================================================================== */
/* IODevice_Mouse -- Saturn Mouse                                     */
/* ================================================================== */

typedef struct
{
   IODevice base;
   int32_t  accum_xdelta;
   int32_t  accum_ydelta;
   uint8_t  buttons;
   uint8_t  buffer[0x10];
   uint8_t  data_out;
   bool     tl;
   int8_t   phase;
} IODevice_Mouse;

static void    IODevice_Mouse_Power(IODevice *self_);
static void    IODevice_Mouse_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed);
static void    IODevice_Mouse_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static uint8_t IODevice_Mouse_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);

static void IODevice_Mouse_Power(IODevice *self_)
{
   IODevice_Mouse *self = (IODevice_Mouse *)self_;
   self->phase        = -1;
   self->tl           = true;
   self->data_out     = 0x00;
   self->accum_xdelta = 0;
   self->accum_ydelta = 0;
}

static void IODevice_Mouse_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed)
{
   IODevice_Mouse *self = (IODevice_Mouse *)self_;
   (void)time_elapsed;
   self->accum_xdelta += (int16_t)(uint16_t)(data[0] | (data[1] << 8));
   self->accum_ydelta -= (int16_t)(uint16_t)(data[2] | (data[3] << 8));
   self->buttons       = data[4] & 0xF;
}

static void IODevice_Mouse_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   IODevice_Mouse *self = (IODevice_Mouse *)self_;
   SFORMAT StateRegs[] =
   {
      SFVAR(self->buttons),
      SFVAR(self->accum_xdelta),
      SFVAR(self->accum_ydelta),

      SFPTR8N(&(self->buffer)[0], (sizeof(self->buffer) / sizeof(uint8_t)), "buffer"),
      SFVAR(self->data_out),
      SFVAR(self->tl),

      SFVAR(self->phase),
      SFEND
   };
   char section_name[64];
   snprintf(section_name, sizeof(section_name), "%s_Mouse", sname_prefix);

   if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name, true) && load)
      IODevice_Mouse_Power(self_);
   else if(load)
   {
      if(self->phase < 0)
         self->phase = -1;
      else
         self->phase &= 0xF;
   }
}

static uint8_t IODevice_Mouse_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_Mouse *self = (IODevice_Mouse *)self_;
   uint8_t tmp;
   (void)timestamp;

   if(smpc_out & 0x40)
   {
      if(smpc_out & 0x20)
      {
         if(!self->tl)
            self->accum_xdelta = self->accum_ydelta = 0;

         self->phase    = -1;
         self->tl       = true;
         self->data_out = 0x00;
      }
      else
      {
         if(self->tl)
            self->tl = false;
      }
   }
   else
   {
      if(self->phase < 0)
      {
         uint8_t flags = 0;
         int i;

         if(self->accum_xdelta < 0)
            flags |= 0x1;

         if(self->accum_ydelta < 0)
            flags |= 0x2;

         if(self->accum_xdelta > 255 || self->accum_xdelta < -256)
         {
            flags |= 0x4;
            self->accum_xdelta = (self->accum_xdelta < 0) ? -256 : 255;
         }

         if(self->accum_ydelta > 255 || self->accum_ydelta < -256)
         {
            flags |= 0x8;
            self->accum_ydelta = (self->accum_ydelta < 0) ? -256 : 255;
         }

         self->buffer[0] = 0xB;
         self->buffer[1] = 0xF;
         self->buffer[2] = 0xF;
         self->buffer[3] = flags;
         self->buffer[4] = self->buttons;
         self->buffer[5] = (self->accum_xdelta >> 4) & 0xF;
         self->buffer[6] = (self->accum_xdelta >> 0) & 0xF;
         self->buffer[7] = (self->accum_ydelta >> 4) & 0xF;
         self->buffer[8] = (self->accum_ydelta >> 0) & 0xF;

         for(i = 9; i < 16; i++)
            self->buffer[i] = self->buffer[8];

         self->phase++;
      }

      if((bool)(smpc_out & 0x20) != self->tl)
      {
         self->phase = (self->phase + 1) & 0xF;
         self->tl    = !self->tl;

         if(self->phase == 8)
            self->accum_xdelta = self->accum_ydelta = 0;
      }
      self->data_out = self->buffer[self->phase];
   }

   tmp = (self->tl << 4) | self->data_out;

   return (smpc_out & (smpc_out_asserted | 0xE0)) | (tmp &~ smpc_out_asserted);
}

static const IODevice_VTable IODevice_Mouse_vtable =
{
   IODevice_Mouse_Power,
   IODevice_base_TransformInput,
   IODevice_Mouse_UpdateInput,
   IODevice_base_UpdateOutput,
   IODevice_Mouse_StateAction,
   IODevice_base_Draw,
   IODevice_Mouse_UpdateBus,
   IODevice_base_LineHook,
   IODevice_base_ResetTS,
   IODevice_base_SetTSFreq,
   NULL
};

IODevice *IODevice_Mouse_Create(void)
{
   IODevice_Mouse *self = (IODevice_Mouse *)calloc(1, sizeof(IODevice_Mouse));
   if (!self)
      return NULL;
   IODevice_Ctor(&self->base);
   self->base.vt = &IODevice_Mouse_vtable;
   self->buttons = 0;
   return &self->base;
}

/* ================================================================== */
/* IODevice_Wheel -- Arcade Racer / racing wheel                      */
/* ================================================================== */

typedef struct
{
   IODevice base;
   uint16_t dbuttons;
   uint8_t  wheel;
   uint8_t  buffer[0x10];
   uint8_t  data_out;
   bool     tl;
   int8_t   phase;
} IODevice_Wheel;

static void    IODevice_Wheel_Power(IODevice *self_);
static void    IODevice_Wheel_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed);
static void    IODevice_Wheel_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static uint8_t IODevice_Wheel_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);

static void IODevice_Wheel_Power(IODevice *self_)
{
   IODevice_Wheel *self = (IODevice_Wheel *)self_;
   self->phase    = -1;
   self->tl       = true;
   self->data_out = 0x01;
}

static void IODevice_Wheel_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed)
{
   IODevice_Wheel *self = (IODevice_Wheel *)self_;
   int32_t tmp;
   (void)time_elapsed;

   self->dbuttons = (self->dbuttons & 0xC) | (((uint16_t)(data[0] | (data[1] << 8))) & 0x07F3);

   tmp = 32767
       + (uint16_t)(data[0x2 + 2] | (data[0x2 + 2 + 1] << 8))
       - (uint16_t)(data[0x2 + 0] | (data[0x2 + 0 + 1] << 8));

   self->wheel = 1 + tmp * 253 / 65534;

   if(self->wheel >= 0x6F)
      self->dbuttons &= ~0x4;
   else if(self->wheel <= 0x67)
      self->dbuttons |= 0x4;

   if(self->wheel <= 0x8F)
      self->dbuttons &= ~0x8;
   else if(self->wheel >= 0x97)
      self->dbuttons |= 0x8;
}

static void IODevice_Wheel_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   IODevice_Wheel *self = (IODevice_Wheel *)self_;
   SFORMAT StateRegs[] =
   {
      SFVAR(self->dbuttons),
      SFVAR(self->wheel),

      SFPTR8N(&(self->buffer)[0], (sizeof(self->buffer) / sizeof(uint8_t)), "buffer"),
      SFVAR(self->data_out),
      SFVAR(self->tl),

      SFVAR(self->phase),
      SFEND
   };
   char section_name[64];
   snprintf(section_name, sizeof(section_name), "%s_Wheel", sname_prefix);

   if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name, true) && load)
      IODevice_Wheel_Power(self_);
   else if(load)
   {
      if(self->phase < 0)
         self->phase = -1;
      else
         self->phase &= 0xF;
   }
}

static uint8_t IODevice_Wheel_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_Wheel *self = (IODevice_Wheel *)self_;
   uint8_t tmp;
   (void)timestamp;

   if(smpc_out & 0x40)
   {
      self->phase    = -1;
      self->tl       = true;
      self->data_out = 0x01;
   }
   else
   {
      if((bool)(smpc_out & 0x20) != self->tl)
      {
         if(self->phase < 0)
         {
            self->buffer[ 0] = 0x1;
            self->buffer[ 1] = 0x3;
            self->buffer[ 2] = (((self->dbuttons >>  0) & 0xF) ^ 0xF);
            self->buffer[ 3] = (((self->dbuttons >>  4) & 0xF) ^ 0xF);
            self->buffer[ 4] = (((self->dbuttons >>  8) & 0xF) ^ 0xF);
            self->buffer[ 5] = (((self->dbuttons >> 12) & 0xF) ^ 0xF);
            self->buffer[ 6] = ((self->wheel >> 4) & 0xF);
            self->buffer[ 7] = ((self->wheel >> 0) & 0xF);
            self->buffer[ 8] = 0x0;
            self->buffer[ 9] = 0x1;
            self->buffer[10] = 0x1;
            self->buffer[11] = ((self->wheel >> 0) & 0xF);
            self->buffer[12] = 0x0;
            self->buffer[13] = 0x1;
            self->buffer[14] = 0x1;
            self->buffer[15] = 0x1;
         }

         self->phase    = (self->phase + 1) & 0xF;
         self->data_out = self->buffer[self->phase];
         self->tl       = !self->tl;
      }
   }

   tmp = (self->tl << 4) | self->data_out;

   return (smpc_out & (smpc_out_asserted | 0xE0)) | (tmp &~ smpc_out_asserted);
}

static const IODevice_VTable IODevice_Wheel_vtable =
{
   IODevice_Wheel_Power,
   IODevice_base_TransformInput,
   IODevice_Wheel_UpdateInput,
   IODevice_base_UpdateOutput,
   IODevice_Wheel_StateAction,
   IODevice_base_Draw,
   IODevice_Wheel_UpdateBus,
   IODevice_base_LineHook,
   IODevice_base_ResetTS,
   IODevice_base_SetTSFreq,
   NULL
};

IODevice *IODevice_Wheel_Create(void)
{
   IODevice_Wheel *self = (IODevice_Wheel *)calloc(1, sizeof(IODevice_Wheel));
   if (!self)
      return NULL;
   IODevice_Ctor(&self->base);
   self->base.vt  = &IODevice_Wheel_vtable;
   self->dbuttons = 0;
   return &self->base;
}

/* ================================================================== */
/* IODevice_Mission -- Mission Stick (single and dual / "twin")       */
/* ================================================================== */

/* Real mission stick has bugs and quirks that aren't emulated here
   (like apparently latching/updating the physical input state at the
   end of the read sequence instead of near the beginning like other
   controllers do, resulting in increased latency). */

typedef struct
{
   IODevice base;
   uint16_t dbuttons;
   uint16_t afeswitches;
   uint8_t  afspeed;
   uint8_t  axes[2][3];
   uint8_t  buffer[0x20];
   uint8_t  data_out;
   bool     tl;
   int8_t   phase;
   uint8_t  afcounter;
   bool     afphase;
   bool     dual;
} IODevice_Mission;

static void    IODevice_Mission_Power(IODevice *self_);
static void    IODevice_Mission_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed);
static void    IODevice_Mission_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static uint8_t IODevice_Mission_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);

static void IODevice_Mission_Power(IODevice *self_)
{
   IODevice_Mission *self = (IODevice_Mission *)self_;
   self->phase    = -1;
   self->tl       = true;
   self->data_out = 0x01;

   /* Power-on state not tested: */
   self->afcounter = 0;
   self->afphase   = false;
}

static void IODevice_Mission_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed)
{
   IODevice_Mission *self = (IODevice_Mission *)self_;
   const uint32_t dtmp    = (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
   unsigned stick, axis;
   (void)time_elapsed;

   self->dbuttons    = (self->dbuttons & 0xF) | ((dtmp & 0xFFF) << 4);
   self->afeswitches = ((dtmp >> 12) & 0x8FF) << 4;
   self->afspeed     = (dtmp >> 20) & 0x7;

   for(stick = 0; stick < (self->dual ? 2u : 1u); stick++)
   {
      for(axis = 0; axis < 3; axis++)
      {
         const unsigned off = 0x3 + ((axis + (stick * 3)) * 2);
         int32_t tmp = (uint16_t)(data[off] | (data[off + 1] << 8));

         self->axes[stick][axis] = (tmp * 255 + 32767) / 65535;
      }
   }
}

static void IODevice_Mission_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   IODevice_Mission *self = (IODevice_Mission *)self_;
   SFORMAT StateRegs[] =
   {
      SFVAR(self->dbuttons),
      SFVAR(self->afeswitches),
      SFVAR(self->afspeed),

      SFVAR(self->afcounter),
      SFVAR(self->afphase),

      SFPTR8N(&(self->axes)[0][0], (sizeof(self->axes) / sizeof(uint8_t)), "&axes[0][0]"),

      SFPTR8N(&(self->buffer)[0], (sizeof(self->buffer) / sizeof(uint8_t)), "buffer"),
      SFVAR(self->data_out),
      SFVAR(self->tl),

      SFVAR(self->phase),
      SFEND
   };
   char section_name[64];
   snprintf(section_name, sizeof(section_name), "%s_Mission", sname_prefix);

   if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name, true) && load)
      IODevice_Mission_Power(self_);
   else if(load)
   {
      self->afspeed %= 7;

      if(self->phase < 0)
         self->phase = -1;
      else
         self->phase &= 0x1F;
   }
}

static uint8_t IODevice_Mission_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_Mission *self = (IODevice_Mission *)self_;
   uint8_t tmp;
   (void)timestamp;

   if(smpc_out & 0x40)
   {
      self->phase    = -1;
      self->tl       = true;
      self->data_out = 0x01;
   }
   else
   {
      if((bool)(smpc_out & 0x20) != self->tl)
      {
         if(self->phase < (self->dual ? 21 : 13))
         {
            self->tl = !self->tl;
            self->phase++;
         }

         if(!self->phase)
         {
            unsigned dbaf = self->dbuttons & ((self->afphase - 1) | ~self->afeswitches);
            unsigned c = 0;
            unsigned stick;

            /* Digital Left */
            self->dbuttons |=  ((self->axes[0][0] <= 0x56) ? 0x4 : 0);
            self->dbuttons &= ~((self->axes[0][0] >= 0x6C) ? 0x4 : 0);

            /* Digital Right */
            self->dbuttons |=  ((self->axes[0][0] >= 0xAB) ? 0x8 : 0);
            self->dbuttons &= ~((self->axes[0][0] <= 0x95) ? 0x8 : 0);

            /* Digital Up */
            self->dbuttons |=  ((self->axes[0][1] <= 0x54) ? 0x1 : 0);
            self->dbuttons &= ~((self->axes[0][1] >= 0x6A) ? 0x1 : 0);

            /* Digital Down */
            self->dbuttons |=  ((self->axes[0][1] >= 0xA9) ? 0x2 : 0);
            self->dbuttons &= ~((self->axes[0][1] <= 0x94) ? 0x2 : 0);

            if(!self->afcounter)
            {
               static const uint8_t speedtab[7] = { 12, 8, 7, 5, 4, 4/* ? */, 1 };
               self->afphase   = !self->afphase;
               self->afcounter = speedtab[self->afspeed];
            }
            self->afcounter--;

            self->buffer[c++] = 0x1;
            self->buffer[c++] = self->dual ? 0x9 : 0x5;
            self->buffer[c++] = (((dbaf >>  0) & 0xF) ^ 0xF);
            self->buffer[c++] = (((dbaf >>  4) & 0xF) ^ 0xF);
            self->buffer[c++] = (((dbaf >>  8) & 0xF) ^ 0xF);
            self->buffer[c++] = (((dbaf >> 12) & 0xF) ^ 0xF);

            for(stick = 0; stick < (self->dual ? 2u : 1u); stick++)
            {
               if(stick)
               {
                  /* Not sure, looks like something buggy. */
                  self->buffer[c++] = 0x0;
                  self->buffer[c++] = 0x0;
               }

               self->buffer[c++] = (self->axes[stick][0] >> 4) & 0xF;
               self->buffer[c++] = (self->axes[stick][0] >> 0) & 0xF;
               self->buffer[c++] = (self->axes[stick][1] >> 4) & 0xF;
               self->buffer[c++] = (self->axes[stick][1] >> 0) & 0xF;
               self->buffer[c++] = (self->axes[stick][2] >> 4) & 0xF;
               self->buffer[c++] = (self->axes[stick][2] >> 0) & 0xF;
            }
            self->buffer[c++] = 0x0;
            self->buffer[c++] = 0x1;
         }

         self->data_out = self->buffer[self->phase];
      }
   }

   tmp = (self->tl << 4) | self->data_out;

   return (smpc_out & (smpc_out_asserted | 0xE0)) | (tmp &~ smpc_out_asserted);
}

static const IODevice_VTable IODevice_Mission_vtable =
{
   IODevice_Mission_Power,
   IODevice_base_TransformInput,
   IODevice_Mission_UpdateInput,
   IODevice_base_UpdateOutput,
   IODevice_Mission_StateAction,
   IODevice_base_Draw,
   IODevice_Mission_UpdateBus,
   IODevice_base_LineHook,
   IODevice_base_ResetTS,
   IODevice_base_SetTSFreq,
   NULL
};

IODevice *IODevice_Mission_Create(bool dual)
{
   IODevice_Mission *self = (IODevice_Mission *)calloc(1, sizeof(IODevice_Mission));
   if (!self)
      return NULL;
   IODevice_Ctor(&self->base);
   self->base.vt     = &IODevice_Mission_vtable;
   self->dbuttons    = 0;
   self->afeswitches = 0;
   self->afspeed     = 0;
   self->dual        = dual;
   return &self->base;
}

/* ================================================================== */
/* IODevice_Gun -- Virtua Gun / Stunner light gun                     */
/* ================================================================== */

typedef struct
{
   IODevice base;
   uint8_t  state;
   int32_t  osshot_counter;
   bool     prev_ossb;
   int32_t  nom_coord[2];
   bool     light_phase;
   int32_t  light_phase_counter;
   int      chair_r, chair_g, chair_b;
} IODevice_Gun;

static void    IODevice_Gun_Power(IODevice *self_);
static void    IODevice_Gun_TransformInput(IODevice *self_, uint8_t *data, float gun_x_scale, float gun_x_offs);
static void    IODevice_Gun_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed);
static void    IODevice_Gun_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static void    IODevice_Gun_Draw(IODevice *self_, MDFN_Surface *surface, const MDFN_Rect *drect, const int32_t *lw, int ifield, float gun_x_scale, float gun_x_offs);
static uint8_t IODevice_Gun_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);
static void    IODevice_Gun_LineHook(IODevice *self_, const int32_t timestamp, int32_t out_line, int32_t div, int32_t coord_adj);
static void    IODevice_Gun_UpdateLight(IODevice_Gun *self, const int32_t timestamp);

void IODevice_Gun_SetCrosshairsColor(IODevice *gun, uint32_t color)
{
   IODevice_Gun *self = (IODevice_Gun *)gun;
   self->chair_r = (color >> 16) & 0xFF;
   self->chair_g = (color >>  8) & 0xFF;
   self->chair_b = (color >>  0) & 0xFF;
}

static void crosshair_plot(MDFN_Surface *surface, uint32_t *lpix,
                           int x, int y,
                           int chair_r, int chair_g, int chair_b)
{
   int r, g, b, a;
   int nr, ng, nb;
   (void)surface;
   (void)y;

   /* surface->DecodeColor(...) used to do this via a member function
      on MDFN_Surface; the surface header port from
      beetle-psx-libretro made MDFN_DecodeColor a free function and
      removed the (unused) palette and per-instance format. We don't
      need alpha here; just take it as a sink. */
   MDFN_DecodeColor(lpix[x], &r, &g, &b, &a);
   (void)a;

   nr = (r + chair_r * 3) >> 2;
   ng = (g + chair_g * 3) >> 2;
   nb = (b + chair_b * 3) >> 2;

   if((int)((abs(r - nr) - 0x40) & (abs(g - ng) - 0x40) & (abs(b - nb) - 0x40)) < 0)
   {
      if((nr | ng | nb) & 0x80)
      {
         nr >>= 1;
         ng >>= 1;
         nb >>= 1;
      }
      else
      {
         nr ^= 0x80;
         ng ^= 0x80;
         nb ^= 0x80;
      }
   }

   lpix[x] = MAKECOLOR(nr, ng, nb, 0);
}

static void IODevice_Gun_Power(IODevice *self_)
{
   IODevice_Gun *self = (IODevice_Gun *)self_;
   self->osshot_counter = -1;
   self->prev_ossb      = false;
   self->state         |= 0x40;
}

static void IODevice_Gun_TransformInput(IODevice *self_, uint8_t *data, float gun_x_scale, float gun_x_offs)
{
   int32_t tmp = (int16_t)(uint16_t)(data[0] | (data[1] << 8));
   (void)self_;

   tmp = floorf(0.5 + tmp * gun_x_scale + gun_x_offs);
   tmp = ((int32_t)(-32768) > (int32_t)(((int32_t)(32767) < (int32_t)(tmp) ? (int32_t)(32767) : (int32_t)(tmp))) ? (int32_t)(-32768) : (int32_t)(((int32_t)(32767) < (int32_t)(tmp) ? (int32_t)(32767) : (int32_t)(tmp))));

   /* MDFN_en16lsb folded: write host int into 2 LE bytes. */
   data[0] = tmp;
   data[1] = tmp >> 8;
}

static void IODevice_Gun_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed)
{
   IODevice_Gun *self = (IODevice_Gun *)self_;
   bool cur_ossb;

   self->nom_coord[0] = (int16_t)(uint16_t)(data[0] | (data[1] << 8));
   self->nom_coord[1] = (int16_t)(uint16_t)(data[2] | (data[3] << 8));

   self->state = ((((~(unsigned)data[4]) << 4) & 0x30) | 0x0C) | (self->state & 0x40);

   cur_ossb = (bool)(data[4] & 0x4);

   if(self->osshot_counter >= 0)
   {
      const int32_t osshot_total = 250000;

      self->osshot_counter += time_elapsed;
      if(self->osshot_counter >= osshot_total)
         self->osshot_counter = -1;
      else
      {
         self->nom_coord[0] = -16384;
         self->nom_coord[1] = -16384;

         if(self->osshot_counter >= osshot_total * 2 / 3)
            self->state |= 0x10;
         else if(self->osshot_counter >= osshot_total * 1 / 3)
            self->state &= ~0x10;
         else
            self->state |= 0x10;
      }
   }
   else if((self->prev_ossb ^ cur_ossb) & cur_ossb)
      self->osshot_counter = 0;

   self->prev_ossb = cur_ossb;
}

static void IODevice_Gun_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   IODevice_Gun *self = (IODevice_Gun *)self_;
   SFORMAT StateRegs[] =
   {
      SFVAR(self->state),

      SFVAR(self->light_phase),
      SFVAR(self->light_phase_counter),
      SFVAR(self->base.NextEventTS),

      SFVAR(self->osshot_counter),
      SFVAR(self->prev_ossb),

      SFPTR32N(&(self->nom_coord)[0], (sizeof(self->nom_coord) / sizeof(int32_t)), "nom_coord"),

      SFEND
   };
   char section_name[64];
   snprintf(section_name, sizeof(section_name), "%s_Gun", sname_prefix);

   if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name, true) && load)
      IODevice_Gun_Power(self_);
   else if(load)
   {
      /* state |= 0x40; */
   }
}

static void IODevice_Gun_Draw(IODevice *self_, MDFN_Surface *surface, const MDFN_Rect *drect, const int32_t *lw, int ifield, float gun_x_scale, float gun_x_offs)
{
   IODevice_Gun *self = (IODevice_Gun *)self_;

   switch(setting_gun_crosshair)
   {
      case SETTING_GUN_CROSSHAIR_OFF:
         return;

      case SETTING_GUN_CROSSHAIR_CROSS:
      {
         int oy;
         for(oy = -8; oy <= 8; oy++)
         {
            int32_t y = drect->y + (((self->nom_coord[1] - MDFNGameInfo->mouse_offs_y) + oy) * ((ifield >= 0) ? 2 : 1)) + (ifield == 1);
            uint32_t *lpix;
            int32_t cx, xmin, xmax, x;

            if(y < drect->y || (y - drect->y) >= drect->h)
               continue;

            lpix = surface->pixels + y * surface->pitchinpix;
            cx   = floorf(0.5 + (((self->nom_coord[0] - gun_x_offs) / gun_x_scale) - MDFNGameInfo->mouse_offs_x) * lw[y] / MDFNGameInfo->mouse_scale_x);

            xmin = drect->x + cx;
            xmax = xmin + ((lw[y] * 2 + MDFNGameInfo->nominal_width) / (MDFNGameInfo->nominal_width * 2)) - 1;

            if(!oy)
            {
               int32_t ehw = (lw[y] * 16 + MDFNGameInfo->nominal_width) / (MDFNGameInfo->nominal_width * 2);

               xmin -= ehw;
               xmax += ehw;
            }

            xmin = ((int32_t)(drect->x) > (int32_t)(xmin) ? (int32_t)(drect->x) : (int32_t)(xmin));
            xmax = ((int32_t)(drect->x + lw[y] - 1) < (int32_t)(xmax) ? (int32_t)(drect->x + lw[y] - 1) : (int32_t)(xmax));

            for(x = xmin; x <= xmax; x++)
               crosshair_plot(surface, lpix, x, y, self->chair_r, self->chair_g, self->chair_b);
         }
      }
      break;

      case SETTING_GUN_CROSSHAIR_DOT:
      {
         int oy;
         for(oy = -1; oy <= 1; oy++)
         {
            int32_t y = drect->y + (((self->nom_coord[1] - MDFNGameInfo->mouse_offs_y) + oy) * ((ifield >= 0) ? 2 : 1)) + (ifield == 1);
            uint32_t *lpix;
            int32_t cx, xmin, xmax, ehw, x;

            if(y < drect->y || (y - drect->y) >= drect->h)
               continue;

            lpix = surface->pixels + y * surface->pitchinpix;
            cx   = floorf(0.5 + (((self->nom_coord[0] - gun_x_offs) / gun_x_scale) - MDFNGameInfo->mouse_offs_x) * lw[y] / MDFNGameInfo->mouse_scale_x);

            xmin = drect->x + cx;
            xmax = xmin + ((lw[y] * 2 + MDFNGameInfo->nominal_width) / (MDFNGameInfo->nominal_width * 2)) - 1;

            ehw  = (lw[y] * 2 + MDFNGameInfo->nominal_width) / (MDFNGameInfo->nominal_width * 2);

            xmin -= ehw;
            xmax += ehw;

            xmin = ((int32_t)(drect->x) > (int32_t)(xmin) ? (int32_t)(drect->x) : (int32_t)(xmin));
            xmax = ((int32_t)(drect->x + lw[y] - 1) < (int32_t)(xmax) ? (int32_t)(drect->x + lw[y] - 1) : (int32_t)(xmax));

            for(x = xmin; x <= xmax; x++)
               crosshair_plot(surface, lpix, x, y, self->chair_r, self->chair_g, self->chair_b);
         }
      }
      break;
   }
}

static void IODevice_Gun_UpdateLight(IODevice_Gun *self, const int32_t timestamp)
{
   self->light_phase_counter -= (timestamp - self->base.LastTS);
   self->base.LastTS = timestamp;
   if(self->light_phase_counter <= 0)
   {
      if(!self->light_phase)
      {
         self->state &= ~0x40;
         self->light_phase         = true;
         self->light_phase_counter = 16;
         self->base.NextEventTS    = timestamp + self->light_phase_counter;
      }
      else
      {
         self->state |= 0x40;
         self->light_phase_counter = 0x7FFFFFFF;
         self->base.NextEventTS    = SS_EVENT_DISABLED_TS;
      }
   }
}

static uint8_t IODevice_Gun_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_Gun *self = (IODevice_Gun *)self_;
   IODevice_Gun_UpdateLight(self, timestamp);
   return ((smpc_out & smpc_out_asserted) | (self->state &~ smpc_out_asserted)) & 0x7C;
}

static void IODevice_Gun_LineHook(IODevice *self_, const int32_t timestamp, int32_t out_line, int32_t div, int32_t coord_adj)
{
   IODevice_Gun *self = (IODevice_Gun *)self_;
   IODevice_Gun_UpdateLight(self, timestamp);

   if(abs((int)((uint32_t)self->nom_coord[1] - (uint32_t)out_line)) <= 1)
   {
      if(self->nom_coord[0] >= 0 && self->nom_coord[0] < 21472)
      {
         int32_t pd = (self->nom_coord[0] + coord_adj) * 4 / div;

         if(pd >= 1)
         {
            self->state              |= 0x40;
            self->light_phase         = false;
            self->light_phase_counter = pd;
            self->base.NextEventTS    = timestamp + self->light_phase_counter;
         }
      }
   }
}

static const IODevice_VTable IODevice_Gun_vtable =
{
   IODevice_Gun_Power,
   IODevice_Gun_TransformInput,
   IODevice_Gun_UpdateInput,
   IODevice_base_UpdateOutput,
   IODevice_Gun_StateAction,
   IODevice_Gun_Draw,
   IODevice_Gun_UpdateBus,
   IODevice_Gun_LineHook,
   IODevice_base_ResetTS,
   IODevice_base_SetTSFreq,
   NULL
};

IODevice *IODevice_Gun_Create(void)
{
   IODevice_Gun *self = (IODevice_Gun *)calloc(1, sizeof(IODevice_Gun));
   if (!self)
      return NULL;
   IODevice_Ctor(&self->base);
   self->base.vt            = &IODevice_Gun_vtable;
   self->state              = 0x4C;
   self->light_phase        = true;
   self->light_phase_counter = 0x7FFFFFFF;
   return &self->base;
}

/* ================================================================== */
/* IODevice_Keyboard -- US PS/2 keyboard via the Saturn adapter       */
/* ================================================================== */

/* PS/2 keyboard adapter seems to do PS/2 processing near/at the end
   of a Saturn-side read sequence, which creates about 1 frame of
   extra latency in practice. We handle things a bit differently
   here, to avoid the latency.

   The keyboard emulated doesn't have special Windows-keyboard keys,
   as they don't appear to work correctly with the PS/2 adapter
   (scancode field is updated, but no make nor break bits are set to
   1), and it's good to have some non-shared keys for input grabbing
   toggling purposes. */

enum { KB_FIFO_SIZE = 16 };

enum
{
   KB_LOCK_SCROLL = 0x01,
   KB_LOCK_NUM    = 0x02,
   KB_LOCK_CAPS   = 0x04
};

typedef struct
{
   IODevice base;
   uint64_t phys[4];
   uint64_t processed[4];
   uint8_t  lock;
   uint8_t  lock_pend;
   uint16_t simbutt;
   uint16_t simbutt_pend;
   uint16_t fifo[KB_FIFO_SIZE];
   uint8_t  fifo_rdp;
   uint8_t  fifo_wrp;
   uint8_t  fifo_cnt;
   int16_t  rep_sc;
   int32_t  rep_dcnt;
   int16_t  mkbrk_pend;
   uint8_t  buffer[12];
   uint8_t  data_out;
   bool     tl;
   int8_t   phase;
} IODevice_Keyboard;

static void    IODevice_Keyboard_Power(IODevice *self_);
static void    IODevice_Keyboard_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed);
static void    IODevice_Keyboard_UpdateOutput(IODevice *self_, uint8_t *data);
static void    IODevice_Keyboard_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static uint8_t IODevice_Keyboard_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);

static void IODevice_Keyboard_Power(IODevice *self_)
{
   IODevice_Keyboard *self = (IODevice_Keyboard *)self_;

   self->phase    = -1;
   self->tl       = true;
   self->data_out = 0x01;

   self->simbutt = self->simbutt_pend = 0;
   self->lock    = self->lock_pend    = 0;

   self->mkbrk_pend = 0;
   memset(self->buffer, 0, sizeof(self->buffer));

   memset(self->processed, 0, sizeof(self->processed));
   memset(self->fifo, 0, sizeof(self->fifo));
   self->fifo_rdp = 0;
   self->fifo_wrp = 0;
   self->fifo_cnt = 0;

   self->rep_sc   = -1;
   self->rep_dcnt = 0;
}

static void IODevice_Keyboard_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed)
{
   IODevice_Keyboard *self = (IODevice_Keyboard *)self_;
   unsigned i;

   /* MDFN_de64lsb / MDFN_de16lsb folded: byte-wise LE construction. */
   self->phys[0] = (uint64_t)data[0x00] | ((uint64_t)data[0x01] << 8) | ((uint64_t)data[0x02] << 16) | ((uint64_t)data[0x03] << 24)
                 | ((uint64_t)data[0x04] << 32) | ((uint64_t)data[0x05] << 40) | ((uint64_t)data[0x06] << 48) | ((uint64_t)data[0x07] << 56);
   self->phys[1] = (uint64_t)data[0x08] | ((uint64_t)data[0x09] << 8) | ((uint64_t)data[0x0A] << 16) | ((uint64_t)data[0x0B] << 24)
                 | ((uint64_t)data[0x0C] << 32) | ((uint64_t)data[0x0D] << 40) | ((uint64_t)data[0x0E] << 48) | ((uint64_t)data[0x0F] << 56);
   self->phys[2] = (uint16_t)(data[0x10] | (data[0x11] << 8));
   self->phys[3] = 0;
   /* */
   if(self->rep_dcnt > 0)
      self->rep_dcnt -= time_elapsed;

   for(i = 0; i < 4; i++)
   {
      uint64_t tmp = self->phys[i] ^ self->processed[i];
      unsigned bp;

      while((bp = (63 ^ MDFN_lzcount64(tmp))) < 64)
      {
         const uint64_t mask = ((uint64_t)1 << bp);
         const int sc = ((i << 6) + bp);

         if(self->fifo_cnt >= (KB_FIFO_SIZE - (sc == 0x82)))
            goto fifo_oflow_abort;

         if(self->phys[i] & mask)
         {
            self->rep_sc = sc;
#if 1
            self->rep_dcnt = 400000;
#else
            self->rep_dcnt = 250000;
#endif
            self->fifo[self->fifo_wrp] = 0x800 | sc;
            self->fifo_wrp = (self->fifo_wrp + 1) % KB_FIFO_SIZE;
            self->fifo_cnt++;
         }

         if(!(self->phys[i] & mask) == (sc != 0x82))
         {
            if(self->rep_sc == sc)
               self->rep_sc = -1;

            self->fifo[self->fifo_wrp] = 0x100 | sc;
            self->fifo_wrp = (self->fifo_wrp + 1) % KB_FIFO_SIZE;
            self->fifo_cnt++;
         }

         self->processed[i] = (self->processed[i] & ~mask) | (self->phys[i] & mask);
         tmp &= ~mask;
      }
   }

   if(self->rep_sc >= 0)
   {
      while(self->rep_dcnt <= 0)
      {
         if(self->fifo_cnt >= KB_FIFO_SIZE)
            goto fifo_oflow_abort;

         self->fifo[self->fifo_wrp] = 0x800 | self->rep_sc;
         self->fifo_wrp = (self->fifo_wrp + 1) % KB_FIFO_SIZE;
         self->fifo_cnt++;

         self->rep_dcnt += 33333;
      }
   }

   fifo_oflow_abort:;
}

static void IODevice_Keyboard_UpdateOutput(IODevice *self_, uint8_t *data)
{
   IODevice_Keyboard *self = (IODevice_Keyboard *)self_;
   data[0x12] = self->lock;
}

static void IODevice_Keyboard_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   IODevice_Keyboard *self = (IODevice_Keyboard *)self_;
   SFORMAT StateRegs[] =
   {
      SFPTR16N(&(self->fifo)[0], (sizeof(self->fifo) / sizeof(uint16_t)), "fifo"),
      SFVAR(self->fifo_rdp),
      SFVAR(self->fifo_wrp),
      SFVAR(self->fifo_cnt),

      SFPTR64N(&(self->phys)[0], (sizeof(self->phys) / sizeof(uint64_t)), "phys"),
      SFPTR64N(&(self->processed)[0], (sizeof(self->processed) / sizeof(uint64_t)), "processed"),

      SFVAR(self->simbutt),
      SFVAR(self->simbutt_pend),
      SFVAR(self->lock),
      SFVAR(self->lock_pend),

      SFVAR(self->rep_sc),
      SFVAR(self->rep_dcnt),

      SFVAR(self->mkbrk_pend),
      SFPTR8N(&(self->buffer)[0], (sizeof(self->buffer) / sizeof(uint8_t)), "buffer"),

      SFVAR(self->data_out),
      SFVAR(self->tl),

      SFVAR(self->phase),
      SFEND
   };
   char section_name[64];
   snprintf(section_name, sizeof(section_name), "%s_Keyboard", sname_prefix);

   if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name, true) && load)
      IODevice_Keyboard_Power(self_);
   else if(load)
   {
      if(self->rep_sc >= 0 && self->rep_dcnt < 0)
         self->rep_dcnt = 0;

      self->fifo_rdp %= KB_FIFO_SIZE;
      self->fifo_wrp %= KB_FIFO_SIZE;
      self->fifo_cnt %= KB_FIFO_SIZE + 1;
      if(self->phase < 0)
         self->phase = -1;
      else
         self->phase %= 12;
   }
}

static uint8_t IODevice_Keyboard_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_Keyboard *self = (IODevice_Keyboard *)self_;
   (void)timestamp;

   if(smpc_out & 0x40)
   {
      self->phase    = -1;
      self->tl       = true;
      self->data_out = 0x01;
   }
   else
   {
      if((bool)(smpc_out & 0x20) != self->tl)
      {
         self->tl = !self->tl;
         self->phase += (self->phase < 11);

         if(!self->phase)
         {
            if(self->mkbrk_pend == (uint8_t)self->mkbrk_pend && self->fifo_cnt)
            {
               bool p;

               self->mkbrk_pend = self->fifo[self->fifo_rdp];
               self->fifo_rdp = (self->fifo_rdp + 1) % KB_FIFO_SIZE;
               self->fifo_cnt--;

               p = self->mkbrk_pend & 0x800;

               switch(self->mkbrk_pend & 0xFF)
               {
                  case 0x89: /*  Up */ self->simbutt_pend = self->simbutt & ~(1 <<  0); self->simbutt_pend &= ~(p <<  1); self->simbutt_pend |= (p <<  0); break;
                  case 0x8A: /*Down */ self->simbutt_pend = self->simbutt & ~(1 <<  1); self->simbutt_pend &= ~(p <<  0); self->simbutt_pend |= (p <<  1); break;
                  case 0x86: /*Left */ self->simbutt_pend = self->simbutt & ~(1 <<  2); self->simbutt_pend &= ~(p <<  3); self->simbutt_pend |= (p <<  2); break;
                  case 0x8D: /*Right*/ self->simbutt_pend = self->simbutt & ~(1 <<  3); self->simbutt_pend &= ~(p <<  2); self->simbutt_pend |= (p <<  3); break;
                  case 0x22: /*   X */ self->simbutt_pend = self->simbutt & ~(1 <<  4); self->simbutt_pend |= (p <<  4); break;
                  case 0x21: /*   C */ self->simbutt_pend = self->simbutt & ~(1 <<  5); self->simbutt_pend |= (p <<  5); break;
                  case 0x1A: /*   Z */ self->simbutt_pend = self->simbutt & ~(1 <<  6); self->simbutt_pend |= (p <<  6); break;
                  case 0x76: /* Esc */ self->simbutt_pend = self->simbutt & ~(1 <<  7); self->simbutt_pend |= (p <<  7); break;
                  case 0x23: /*   D */ self->simbutt_pend = self->simbutt & ~(1 <<  8); self->simbutt_pend |= (p <<  8); break;
                  case 0x1B: /*   S */ self->simbutt_pend = self->simbutt & ~(1 <<  9); self->simbutt_pend |= (p <<  9); break;
                  case 0x1C: /*   A */ self->simbutt_pend = self->simbutt & ~(1 << 10); self->simbutt_pend |= (p << 10); break;
                  case 0x24: /*   E */ self->simbutt_pend = self->simbutt & ~(1 << 11); self->simbutt_pend |= (p << 11); break;
                  case 0x15: /*   Q */ self->simbutt_pend = self->simbutt & ~(1 << 15); self->simbutt_pend |= (p << 15); break;

                  case 0x7E: /* Scrl */ self->lock_pend = self->lock ^ (p ? KB_LOCK_SCROLL : 0); break;
                  case 0x77: /* Num  */ self->lock_pend = self->lock ^ (p ? KB_LOCK_NUM : 0);    break;
                  case 0x58: /* Caps */ self->lock_pend = self->lock ^ (p ? KB_LOCK_CAPS : 0);   break;
               }
            }
            self->buffer[ 0] = 0x3;
            self->buffer[ 1] = 0x4;
            self->buffer[ 2] = (((self->simbutt_pend >>  0) ^ 0xF) & 0xF);
            self->buffer[ 3] = (((self->simbutt_pend >>  4) ^ 0xF) & 0xF);
            self->buffer[ 4] = (((self->simbutt_pend >>  8) ^ 0xF) & 0xF);
            self->buffer[ 5] = (((self->simbutt_pend >> 12) ^ 0xF) & 0x8) | 0x0;
            self->buffer[ 6] = self->lock_pend;
            self->buffer[ 7] = ((self->mkbrk_pend >> 8) & 0xF) | 0x6;
            self->buffer[ 8] =  (self->mkbrk_pend >> 4) & 0xF;
            self->buffer[ 9] =  (self->mkbrk_pend >> 0) & 0xF;
            self->buffer[10] = 0x0;
            self->buffer[11] = 0x1;
         }

         if(self->phase == 9)
         {
            self->mkbrk_pend = (uint8_t)self->mkbrk_pend;
            self->lock       = self->lock_pend;
            self->simbutt    = self->simbutt_pend;
         }

         self->data_out = self->buffer[self->phase];
      }
   }

   return (smpc_out & (smpc_out_asserted | 0xE0)) | (((self->tl << 4) | self->data_out) &~ smpc_out_asserted);
}

static const IODevice_VTable IODevice_Keyboard_vtable =
{
   IODevice_Keyboard_Power,
   IODevice_base_TransformInput,
   IODevice_Keyboard_UpdateInput,
   IODevice_Keyboard_UpdateOutput,
   IODevice_Keyboard_StateAction,
   IODevice_base_Draw,
   IODevice_Keyboard_UpdateBus,
   IODevice_base_LineHook,
   IODevice_base_ResetTS,
   IODevice_base_SetTSFreq,
   NULL
};

IODevice *IODevice_Keyboard_Create(void)
{
   IODevice_Keyboard *self = (IODevice_Keyboard *)calloc(1, sizeof(IODevice_Keyboard));
   if (!self)
      return NULL;
   IODevice_Ctor(&self->base);
   self->base.vt = &IODevice_Keyboard_vtable;
   /* phys{0,0,0,0} -- calloc already zeroed everything. */
   return &self->base;
}

/* ================================================================== */
/* IODevice_JPKeyboard -- Japanese Saturn keyboard                    */
/* ================================================================== */

/* Compared to the PS/2 keyboard adapter + PS/2 keyboard, the Japanese
   Saturn keyboard has: a few extra Japanese language keys (and
   related keymap/layout differences), no numpad, a Pause key that
   behaves like a normal key, Caps/Scroll Lock repeat that does not
   oscillate the lock state, and key-repeat delay counters clocked by
   read sequences rather than an independent internal clock.

   Reuses the KB_FIFO_SIZE and KB_LOCK_* constants from the keyboard
   device above. Note the savestate section name is "%s_Keyboard"
   (not "_JPKeyboard") -- kept verbatim from the original for
   savestate compatibility. */

typedef struct
{
   IODevice base;
   uint64_t phys[4];
   uint64_t processed[4];
   uint8_t  lock;
   uint8_t  lock_pend;
   uint16_t simbutt;
   uint16_t simbutt_pend;
   uint16_t fifo[KB_FIFO_SIZE];
   uint8_t  fifo_rdp;
   uint8_t  fifo_wrp;
   uint8_t  fifo_cnt;
   uint8_t  rep_sc;
   uint8_t  rep_sc_pend;
   uint8_t  rep_dcnt;
   uint8_t  rep_dcnt_pend;
   int16_t  mkbrk_pend;
   uint8_t  buffer[12];
   uint8_t  data_out;
   bool     tl;
   int8_t   phase;
} IODevice_JPKeyboard;

static void    IODevice_JPKeyboard_Power(IODevice *self_);
static void    IODevice_JPKeyboard_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed);
static void    IODevice_JPKeyboard_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static uint8_t IODevice_JPKeyboard_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);

static void IODevice_JPKeyboard_Power(IODevice *self_)
{
   IODevice_JPKeyboard *self = (IODevice_JPKeyboard *)self_;

   self->phase    = -1;
   self->tl       = true;
   self->data_out = 0x01;

   self->simbutt = self->simbutt_pend = 0;
   self->lock    = self->lock_pend    = 0;

   self->mkbrk_pend = 0;
   memset(self->buffer, 0, sizeof(self->buffer));

   memset(self->processed, 0, sizeof(self->processed));
   memset(self->fifo, 0, sizeof(self->fifo));
   self->fifo_rdp = 0;
   self->fifo_wrp = 0;
   self->fifo_cnt = 0;

   self->rep_sc   = self->rep_sc_pend   = 0;
   self->rep_dcnt = self->rep_dcnt_pend = 0;
}

static void IODevice_JPKeyboard_UpdateInput(IODevice *self_, const uint8_t *data, int32_t time_elapsed)
{
   IODevice_JPKeyboard *self = (IODevice_JPKeyboard *)self_;
   unsigned i;
   (void)time_elapsed;

   /* MDFN_de64lsb / MDFN_de16lsb folded: byte-wise LE construction. */
   self->phys[0] = (uint64_t)data[0x00] | ((uint64_t)data[0x01] << 8) | ((uint64_t)data[0x02] << 16) | ((uint64_t)data[0x03] << 24)
                 | ((uint64_t)data[0x04] << 32) | ((uint64_t)data[0x05] << 40) | ((uint64_t)data[0x06] << 48) | ((uint64_t)data[0x07] << 56);
   self->phys[1] = (uint64_t)data[0x08] | ((uint64_t)data[0x09] << 8) | ((uint64_t)data[0x0A] << 16) | ((uint64_t)data[0x0B] << 24)
                 | ((uint64_t)data[0x0C] << 32) | ((uint64_t)data[0x0D] << 40) | ((uint64_t)data[0x0E] << 48) | ((uint64_t)data[0x0F] << 56);
   self->phys[2] = (uint16_t)(data[0x10] | (data[0x11] << 8));
   self->phys[3] = 0;
   /* */

   for(i = 0; i < 4; i++)
   {
      uint64_t tmp = self->phys[i] ^ self->processed[i];
      unsigned bp;

      while((bp = (63 ^ MDFN_lzcount64(tmp))) < 64)
      {
         const uint64_t mask = ((uint64_t)1 << bp);
         const int sc = ((i << 6) + bp);

         if(self->fifo_cnt >= KB_FIFO_SIZE)
            goto fifo_oflow_abort;

         if(self->phys[i] & mask)
         {
            self->fifo[self->fifo_wrp] = 0x800 | sc;
            self->fifo_wrp = (self->fifo_wrp + 1) % KB_FIFO_SIZE;
            self->fifo_cnt++;
         }

         if(!(self->phys[i] & mask))
         {
            self->fifo[self->fifo_wrp] = 0x100 | sc;
            self->fifo_wrp = (self->fifo_wrp + 1) % KB_FIFO_SIZE;
            self->fifo_cnt++;
         }

         self->processed[i] = (self->processed[i] & ~mask) | (self->phys[i] & mask);
         tmp &= ~mask;
      }
   }
   fifo_oflow_abort:;
}

static void IODevice_JPKeyboard_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   IODevice_JPKeyboard *self = (IODevice_JPKeyboard *)self_;
   SFORMAT StateRegs[] =
   {
      SFPTR16N(&(self->fifo)[0], (sizeof(self->fifo) / sizeof(uint16_t)), "fifo"),
      SFVAR(self->fifo_rdp),
      SFVAR(self->fifo_wrp),
      SFVAR(self->fifo_cnt),

      SFPTR64N(&(self->phys)[0], (sizeof(self->phys) / sizeof(uint64_t)), "phys"),
      SFPTR64N(&(self->processed)[0], (sizeof(self->processed) / sizeof(uint64_t)), "processed"),

      SFVAR(self->simbutt),
      SFVAR(self->simbutt_pend),
      SFVAR(self->lock),
      SFVAR(self->lock_pend),

      SFVAR(self->rep_sc),
      SFVAR(self->rep_sc_pend),
      SFVAR(self->rep_dcnt),
      SFVAR(self->rep_dcnt_pend),

      SFVAR(self->mkbrk_pend),
      SFPTR8N(&(self->buffer)[0], (sizeof(self->buffer) / sizeof(uint8_t)), "buffer"),

      SFVAR(self->data_out),
      SFVAR(self->tl),

      SFVAR(self->phase),
      SFEND
   };
   char section_name[64];
   snprintf(section_name, sizeof(section_name), "%s_Keyboard", sname_prefix);

   if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name, true) && load)
      IODevice_JPKeyboard_Power(self_);
   else if(load)
   {
      self->fifo_rdp %= KB_FIFO_SIZE;
      self->fifo_wrp %= KB_FIFO_SIZE;
      self->fifo_cnt %= KB_FIFO_SIZE + 1;
      if(self->phase < 0)
         self->phase = -1;
      else
         self->phase %= 12;
   }
}

static uint8_t IODevice_JPKeyboard_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_JPKeyboard *self = (IODevice_JPKeyboard *)self_;
   (void)timestamp;

   if(smpc_out & 0x40)
   {
      self->phase    = -1;
      self->tl       = true;
      self->data_out = 0x01;
   }
   else
   {
      if((bool)(smpc_out & 0x20) != self->tl)
      {
         if(self->phase < 11)
         {
            self->tl = !self->tl;
            self->phase++;
         }

         if(!self->phase)
         {
            if(self->mkbrk_pend == (uint8_t)self->mkbrk_pend)
            {
               if(self->fifo_cnt)
               {
                  bool p;

                  self->mkbrk_pend = self->fifo[self->fifo_rdp];
                  self->fifo_rdp = (self->fifo_rdp + 1) % KB_FIFO_SIZE;
                  self->fifo_cnt--;

                  p = self->mkbrk_pend & 0x800;

                  if(p)
                  {
                     self->rep_sc_pend   = self->mkbrk_pend & 0xFF;
                     self->rep_dcnt_pend = 30;
                  }
                  else
                  {
                     if(self->rep_sc == (self->mkbrk_pend & 0xFF))
                        self->rep_dcnt_pend = 0;
                  }

                  switch(self->mkbrk_pend & 0xFF)
                  {
                     case 0x89: /*  Up */ self->simbutt_pend = self->simbutt & ~(1 <<  0); self->simbutt_pend &= ~(p <<  1); self->simbutt_pend |= (p <<  0); break;
                     case 0x8A: /*Down */ self->simbutt_pend = self->simbutt & ~(1 <<  1); self->simbutt_pend &= ~(p <<  0); self->simbutt_pend |= (p <<  1); break;
                     case 0x86: /*Left */ self->simbutt_pend = self->simbutt & ~(1 <<  2); self->simbutt_pend &= ~(p <<  3); self->simbutt_pend |= (p <<  2); break;
                     case 0x8D: /*Right*/ self->simbutt_pend = self->simbutt & ~(1 <<  3); self->simbutt_pend &= ~(p <<  2); self->simbutt_pend |= (p <<  3); break;
                     case 0x22: /*   X */ self->simbutt_pend = self->simbutt & ~(1 <<  4); self->simbutt_pend |= (p <<  4); break;
                     case 0x21: /*   C */ self->simbutt_pend = self->simbutt & ~(1 <<  5); self->simbutt_pend |= (p <<  5); break;
                     case 0x1A: /*   Z */ self->simbutt_pend = self->simbutt & ~(1 <<  6); self->simbutt_pend |= (p <<  6); break;
                     case 0x76: /* Esc */ self->simbutt_pend = self->simbutt & ~(1 <<  7); self->simbutt_pend |= (p <<  7); break;
                     case 0x23: /*   D */ self->simbutt_pend = self->simbutt & ~(1 <<  8); self->simbutt_pend |= (p <<  8); break;
                     case 0x1B: /*   S */ self->simbutt_pend = self->simbutt & ~(1 <<  9); self->simbutt_pend |= (p <<  9); break;
                     case 0x1C: /*   A */ self->simbutt_pend = self->simbutt & ~(1 << 10); self->simbutt_pend |= (p << 10); break;
                     case 0x24: /*   E */ self->simbutt_pend = self->simbutt & ~(1 << 11); self->simbutt_pend |= (p << 11); break;
                     case 0x15: /*   Q */ self->simbutt_pend = self->simbutt & ~(1 << 15); self->simbutt_pend |= (p << 15); break;

                     case 0x7E: /* Scrl */ self->lock_pend = self->lock ^ (p ? KB_LOCK_SCROLL : 0); break;
                     case 0x58: /* Caps */ self->lock_pend = self->lock ^ (p ? KB_LOCK_CAPS : 0);   break;
                  }
               }
               else if(self->rep_dcnt > 0)
               {
                  self->rep_dcnt_pend = self->rep_dcnt - 1;

                  if(!self->rep_dcnt_pend)
                  {
                     self->mkbrk_pend    = 0x800 | self->rep_sc;
                     self->rep_dcnt_pend = 6;
                  }
               }
            }
            self->buffer[ 0] = 0x3;
            self->buffer[ 1] = 0x4;
            self->buffer[ 2] = (((self->simbutt_pend >>  0) ^ 0xF) & 0xF);
            self->buffer[ 3] = (((self->simbutt_pend >>  4) ^ 0xF) & 0xF);
            self->buffer[ 4] = (((self->simbutt_pend >>  8) ^ 0xF) & 0xF);
            self->buffer[ 5] = (((self->simbutt_pend >> 12) ^ 0xF) & 0x8) | 0x0;
            self->buffer[ 6] = self->lock_pend;
            self->buffer[ 7] = ((self->mkbrk_pend >> 8) & 0xF) | 0x6;
            self->buffer[ 8] =  (self->mkbrk_pend >> 4) & 0xF;
            self->buffer[ 9] =  (self->mkbrk_pend >> 0) & 0xF;
            self->buffer[10] = 0x0;
            self->buffer[11] = 0x1;
         }

         if(self->phase == 9)
         {
            self->mkbrk_pend = (uint8_t)self->mkbrk_pend;
            self->lock       = self->lock_pend;
            self->simbutt    = self->simbutt_pend;
            self->rep_dcnt   = self->rep_dcnt_pend;
            self->rep_sc     = self->rep_sc_pend;
         }

         self->data_out = self->buffer[self->phase];
      }
   }

   return (smpc_out & (smpc_out_asserted | 0xE0)) | (((self->tl << 4) | self->data_out) &~ smpc_out_asserted);
}

static const IODevice_VTable IODevice_JPKeyboard_vtable =
{
   IODevice_JPKeyboard_Power,
   IODevice_base_TransformInput,
   IODevice_JPKeyboard_UpdateInput,
   IODevice_base_UpdateOutput,
   IODevice_JPKeyboard_StateAction,
   IODevice_base_Draw,
   IODevice_JPKeyboard_UpdateBus,
   IODevice_base_LineHook,
   IODevice_base_ResetTS,
   IODevice_base_SetTSFreq,
   NULL
};

IODevice *IODevice_JPKeyboard_Create(void)
{
   IODevice_JPKeyboard *self = (IODevice_JPKeyboard *)calloc(1, sizeof(IODevice_JPKeyboard));
   if (!self)
      return NULL;
   IODevice_Ctor(&self->base);
   self->base.vt = &IODevice_JPKeyboard_vtable;
   /* phys{0,0,0,0} -- calloc already zeroed everything. */
   return &self->base;
}

/* ================================================================== */
/* IODevice_Multitap -- 6-Player Multitap                             */
/* ================================================================== */

/* Holds six sub-device pointers and presents them to the SMPC as one
   device. Nested polymorphism: the sub-devices are themselves
   IODevice* and are dispatched through their own vtables. The
   UpdateBus state machine uses a __COUNTER__-driven computed-phase
   switch (the WAIT_UNTIL / WR_NYB macros) -- this is preprocessor
   machinery and works identically in C; the macros are kept
   file-local and reference `self`, which the one function that uses
   them (IODevice_Multitap_UpdateBus) has in scope. */

typedef struct
{
   IODevice  base;
   IODevice *devices[6];
   uint8_t   sub_state[6];
   uint8_t   tmp[4];
   uint8_t   id1;
   uint8_t   id2;
   uint8_t   data_out;
   bool      tl;
   int32_t   phase;
   uint8_t   port_counter;
   uint8_t   read_counter;
} IODevice_Multitap;

static void    IODevice_Multitap_Power(IODevice *self_);
static void    IODevice_Multitap_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix);
static void    IODevice_Multitap_Draw(IODevice *self_, MDFN_Surface *surface, const MDFN_Rect *drect, const int32_t *lw, int ifield, float gun_x_scale, float gun_x_offs);
static uint8_t IODevice_Multitap_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted);
static void    IODevice_Multitap_LineHook(IODevice *self_, const int32_t timestamp, int32_t out_line, int32_t div, int32_t coord_adj);
static void    IODevice_Multitap_ResetTS(IODevice *self_);

/* Dispatch helper: invoke a sub-device's UpdateBus through its vtable. */
#define SUBDEV_UPDATEBUS(d, ts, so, soa) ((d)->vt->UpdateBus((d), (ts), (so), (soa)))

static void IODevice_Multitap_Draw(IODevice *self_, MDFN_Surface *surface, const MDFN_Rect *drect, const int32_t *lw, int ifield, float gun_x_scale, float gun_x_offs)
{
   IODevice_Multitap *self = (IODevice_Multitap *)self_;
   unsigned i;
   /* devices[] starts NULL (calloc) and is populated lazily by
      IODevice_Multitap_SetSubDevice; guard like Power() already does. */
   for(i = 0; i < 6; i++)
      if(self->devices[i])
         self->devices[i]->vt->Draw(self->devices[i], surface, drect, lw, ifield, gun_x_scale, gun_x_offs);
}

static void IODevice_Multitap_LineHook(IODevice *self_, const int32_t timestamp, int32_t out_line, int32_t div, int32_t coord_adj)
{
   IODevice_Multitap *self = (IODevice_Multitap *)self_;
   unsigned i;
   for(i = 0; i < 6; i++)
      if(self->devices[i])
         self->devices[i]->vt->LineHook(self->devices[i], timestamp, out_line, div, coord_adj);

   self->base.LastTS = timestamp;
}

static void IODevice_Multitap_ResetTS(IODevice *self_)
{
   IODevice_Multitap *self = (IODevice_Multitap *)self_;
   unsigned i;
   self->base.LastTS = 0;

   for(i = 0; i < 6; i++)
      if(self->devices[i])
         self->devices[i]->vt->ResetTS(self->devices[i]);
}

static void IODevice_Multitap_Power(IODevice *self_)
{
   IODevice_Multitap *self = (IODevice_Multitap *)self_;
   unsigned i;

   self->phase    = -2;
   self->tl       = true;
   self->data_out = 0x01;

   memset(self->tmp, 0x00, sizeof(self->tmp));
   self->id1          = 0;
   self->id2          = 0;
   self->port_counter = 0;
   self->read_counter = 0;

   for(i = 0; i < 6; i++)
   {
      if(self->devices[i])
      {
         self->sub_state[i] = 0x60;
         SUBDEV_UPDATEBUS(self->devices[i], self->devices[i]->LastTS, self->sub_state[i], 0x60);
         self->devices[i]->vt->Power(self->devices[i]);
      }
   }
}

void IODevice_Multitap_SetSubDevice(IODevice *mt, unsigned sub_index, IODevice *device)
{
   IODevice_Multitap *self = (IODevice_Multitap *)mt;
   assert(sub_index < 6);
   self->devices[sub_index] = device;
   SUBDEV_UPDATEBUS(self->devices[sub_index], self->devices[sub_index]->LastTS, self->sub_state[sub_index], 0x60);
}

IODevice *IODevice_Multitap_GetSubDevice(IODevice *mt, unsigned sub_index)
{
   IODevice_Multitap *self = (IODevice_Multitap *)mt;
   assert(sub_index < 6);
   return self->devices[sub_index];
}

void IODevice_Multitap_ForceSubUpdate(IODevice *mt, const int32_t timestamp)
{
   IODevice_Multitap *self = (IODevice_Multitap *)mt;
   unsigned i;
   for(i = 0; i < 6; i++)
      if(self->devices[i])
         SUBDEV_UPDATEBUS(self->devices[i], timestamp, self->sub_state[i], 0x60);

   self->base.LastTS = timestamp;
}

static void IODevice_Multitap_StateAction(IODevice *self_, StateMem *sm, const unsigned load, const bool data_only, const char *sname_prefix)
{
   IODevice_Multitap *self = (IODevice_Multitap *)self_;
   unsigned i;
   SFORMAT StateRegs[] =
   {
      SFPTR8N(&(self->sub_state)[0], (sizeof(self->sub_state) / sizeof(uint8_t)), "sub_state"),
      SFPTR8N(&(self->tmp)[0], (sizeof(self->tmp) / sizeof(uint8_t)), "tmp"),
      SFVAR(self->id1),
      SFVAR(self->id2),

      SFVAR(self->data_out),
      SFVAR(self->tl),

      SFVAR(self->phase),
      SFVAR(self->port_counter),
      SFVAR(self->read_counter),
      SFEND
   };
   char section_name[32];
   snprintf(section_name, sizeof(section_name), "%s_Multitap", sname_prefix);

   if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name, true) && load)
      IODevice_Multitap_Power(self_);
   else if(load)
   {
      self->port_counter %= 6;
   }

   for(i = 0; i < 6; i++)
   {
      char snsp[32];

      snprintf(snsp, sizeof(snsp), "%sP%u", section_name, i);
      if(self->devices[i])
         self->devices[i]->vt->StateAction(self->devices[i], sm, load, data_only, snsp);
   }
}

/* The UpdateBus state machine. PhaseBias and the WAIT_UNTIL / WR_NYB
   macros are exactly as in the original multitap.cpp -- a
   __COUNTER__-based computed-phase resumable switch. They reference
   `self`, `smpc_out`, and `timestamp`, all of which
   IODevice_Multitap_UpdateBus has in scope. UASB (Update And Status
   of sub-device at port_counter) is folded into a macro for the same
   reason. */

enum { MULTITAP_PhaseBias = __COUNTER__ + 1 };

#define WAIT_UNTIL(cond)  {						\
			    case __COUNTER__:				\
			    if(!(cond))					\
			    {						\
			     self->phase = __COUNTER__ - MULTITAP_PhaseBias - 1;	\
			     goto BreakOut;				\
			    }						\
			   }

#define WR_NYB(v) { WAIT_UNTIL((bool)(smpc_out & 0x20) != self->tl); self->data_out = (v) & 0xF; self->tl = !self->tl; }

#define UASB() (self->devices[self->port_counter]->vt->UpdateBus(self->devices[self->port_counter], timestamp, self->sub_state[self->port_counter], 0x60))

static uint8_t IODevice_Multitap_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_Multitap *self = (IODevice_Multitap *)self_;

   if(smpc_out & 0x40)
   {
      self->phase    = -1;
      self->tl       = true;
      self->data_out = 0x01;
   }
   else
   {
      switch(self->phase + MULTITAP_PhaseBias)
      {
         for(;;)
         {
            default:
            case __COUNTER__:

            WAIT_UNTIL(self->phase == -1);

            WR_NYB(0x4);
            WR_NYB(0x1);
            WR_NYB(0x6);
            WR_NYB(0x0);
            /* */
            /* */
            self->port_counter = 0;

            do
            {
               self->sub_state[self->port_counter] = 0x60;
               UASB();
               /* ... */
               self->tmp[0] = UASB();
               self->id1 = ((((self->tmp[0] >> 3) | (self->tmp[0] >> 2)) & 1) << 3) | ((((self->tmp[0] >> 1) | (self->tmp[0] >> 0)) & 1) << 2);

               self->sub_state[self->port_counter] = 0x20;
               UASB();
               /* ... */
               self->tmp[1] = UASB();
               self->id1 |= ((((self->tmp[1] >> 3) | (self->tmp[1] >> 2)) & 1) << 1) | ((((self->tmp[1] >> 1) | (self->tmp[1] >> 0)) & 1) << 0);

               if(self->id1 == 0xB) /* Digital pad */
               {
                  WR_NYB(0x0);
                  WR_NYB(0x2);

                  self->sub_state[self->port_counter] = 0x40;
                  UASB();
                  WR_NYB(self->tmp[1] & 0xF);
                  self->tmp[2] = UASB();

                  self->sub_state[self->port_counter] = 0x00;
                  UASB();
                  WR_NYB(self->tmp[2] & 0xF);
                  self->tmp[3] = UASB();

                  WR_NYB(self->tmp[3] & 0xF);
                  WR_NYB((self->tmp[0] & 0xF) | 0x7);
               }
               else if(self->id1 == 0x3 || self->id1 == 0x5) /* Analog */
               {
                  self->sub_state[self->port_counter] = 0x00;
                  WAIT_UNTIL(!(UASB() & 0x10));
                  self->id2 = ((UASB() & 0xF) << 4);

                  self->sub_state[self->port_counter] = 0x20;
                  WAIT_UNTIL(UASB() & 0x10);
                  self->id2 |= ((UASB() & 0xF) << 0);

                  if(self->id1 == 0x3)
                     self->id2 = 0xE3;

                  WR_NYB(self->id2 >> 4);
                  WR_NYB(self->id2 >> 0);

                  self->read_counter = 0;
                  while(self->read_counter < (self->id2 & 0xF))
                  {
                     self->sub_state[self->port_counter] = 0x00;
                     WAIT_UNTIL(!(UASB() & 0x10));
                     WR_NYB(UASB() & 0xF);

                     self->sub_state[self->port_counter] = 0x20;
                     WAIT_UNTIL(UASB() & 0x10);
                     WR_NYB(UASB() & 0xF);

                     self->read_counter++;
                  }
               }
               else
               {
                  WR_NYB(0xF);
                  WR_NYB(0xF);
               }

               self->sub_state[self->port_counter] = 0x60;
               UASB();
            } while(++self->port_counter < 6);
            self->port_counter = 0;	/* Save state consistency. */

            /* */
            /* */
            WR_NYB(0x0);
            WR_NYB(0x1);
         }
      }
   }

   BreakOut:;

   self->base.LastTS = timestamp;

   return (smpc_out & (smpc_out_asserted | 0xE0)) | (((self->tl << 4) | self->data_out) &~ smpc_out_asserted);
}

#undef WAIT_UNTIL
#undef WR_NYB
#undef UASB
#undef SUBDEV_UPDATEBUS

static const IODevice_VTable IODevice_Multitap_vtable =
{
   IODevice_Multitap_Power,
   IODevice_base_TransformInput,
   IODevice_base_UpdateInput,
   IODevice_base_UpdateOutput,
   IODevice_Multitap_StateAction,
   IODevice_Multitap_Draw,
   IODevice_Multitap_UpdateBus,
   IODevice_Multitap_LineHook,
   IODevice_Multitap_ResetTS,
   IODevice_base_SetTSFreq,
   NULL
};

IODevice *IODevice_Multitap_Create(void)
{
   IODevice_Multitap *self = (IODevice_Multitap *)calloc(1, sizeof(IODevice_Multitap));
   if (!self)
      return NULL;
   IODevice_Ctor(&self->base);
   self->base.vt = &IODevice_Multitap_vtable;
   return &self->base;
}
