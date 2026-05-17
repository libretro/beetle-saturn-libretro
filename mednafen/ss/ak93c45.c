/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* ak93c45.c:
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

/* Converted from ak93c45.cpp. AK93C45 was a simple C++ class -- no
   inheritance, no virtual methods, no templates -- so the conversion
   is mechanical: the class becomes a struct, the methods become free
   functions taking an AK93C45*, and the constructor/destructor fold
   into AK93C45_Init (the destructor only zeroed lastts, which Init
   already does). The PHASE_* enum is namespaced to AK93C45_PHASE_*
   in the header. */

#include <stdint.h>
#include <string.h>
#include <boolean.h>

#include "../state.h"

#include "ak93c45.h"

void AK93C45_Init(AK93C45 *self)
{
   memset(self->mem, 0xFF, sizeof(self->mem));
   self->lastts  = 0;
   self->tsratio = 0;

   AK93C45_Power(self);
}

void AK93C45_ResetTS(AK93C45 *self)
{
   self->lastts = 0;

   if(self->write_finish_counter < 0)
      self->write_finish_counter = 0;
}

void AK93C45_SetTSFreq(AK93C45 *self, const int32_t rate)
{
   self->tsratio = 1000000 * (1ULL << 32) / rate;
}

void AK93C45_Power(AK93C45 *self)
{
   self->write_enable = false;

   self->addr        = 0;
   self->data_buffer = 0;
   self->counter     = 0;
   self->opcode      = 0;
   self->dout        = true;

   self->prev_cs = false;
   self->prev_sk = true;

   self->write_finish_counter = 0;

   self->phase = AK93C45_PHASE_IDLE;
}

bool AK93C45_UpdateBus(AK93C45 *self, int32_t timestamp, bool cs, bool sk, bool di)
{
   int64_t clocks;

   clocks = (int64_t)(timestamp - self->lastts) * self->tsratio;
   self->lastts = timestamp;

   self->write_finish_counter -= clocks;
   if(self->phase == AK93C45_PHASE_WRITING && self->write_finish_counter <= 0)
   {
      /* printf("Write finished\n"); */
      self->dout  = true;
      self->phase = AK93C45_PHASE_IDLE;
   }
   /* */
   if(!cs)
   {
      if(self->phase == AK93C45_PHASE_WRITE_PENDING)
      {
         /* printf("Write: %04x %04x\n", addr, data_buffer); */

         if(self->addr == 0xFFFF)
            memset(self->mem, self->data_buffer, sizeof(self->mem));
         else
            self->mem[self->addr & 0x3F] = self->data_buffer;

         self->phase = AK93C45_PHASE_WRITING;
         self->write_finish_counter = (int64_t)10000 << 32;
      }
      else if(self->phase != AK93C45_PHASE_WRITING)
         self->phase = AK93C45_PHASE_IDLE;

      self->dout = true;
   }
   else
   {
      if(self->phase == AK93C45_PHASE_WRITING)
         self->dout = false;
      else if(/*prev_cs &&*/ sk && !self->prev_sk)
      {
         switch(self->phase)
         {
            case AK93C45_PHASE_IDLE:
               if(!di)
                  self->phase++;
               break;

            case AK93C45_PHASE_WAIT_START:
               if(di)
               {
                  self->phase++;
                  self->counter = 2;
                  self->opcode  = 0;
               }
               break;

            case AK93C45_PHASE_OPCODE:
               self->opcode <<= 1;
               self->opcode |= di;
               self->counter--;
               if(!self->counter)
               {
                  self->phase++;
                  self->counter = 6;
                  self->addr    = 0;
               }
               break;

            case AK93C45_PHASE_ADDR:
               self->addr <<= 1;
               self->addr |= di;
               self->counter--;
               if(!self->counter)
               {
                  /* printf("Op: 0x%01x, Address: 0x%02x\n", opcode, addr); */
                  if(self->opcode == 0x2)	/* Read */
                  {
                     self->phase++;
                     self->dout        = false;
                     self->data_buffer = self->mem[self->addr & 0x3F];
                     self->addr        = (self->addr + 1) & 0x3F;
                     self->counter     = 16;
                  }
                  else if(self->opcode == 0x1)	/* Write */
                  {
                     self->data_buffer = 0x0000;
                     self->phase++;
                     self->counter = 16;
                  }
                  else if(self->opcode == 0x0)
                  {
                     switch(self->addr & 0x30)
                     {
                        case 0x00:
                           self->write_enable = false;
                           self->phase = AK93C45_PHASE_IDLE;
                           break;

                        case 0x30:
                           self->write_enable = true;
                           self->phase = AK93C45_PHASE_IDLE;
                           break;

                        case 0x10:
                           self->addr        = 0xFFFF;
                           self->data_buffer = 0x0000;
                           self->phase++;
                           self->counter = 16;
                           break;
                     }
                  }
               }
               break;

            case AK93C45_PHASE_DATA:
               if(self->opcode == 0x2)	/* Read */
               {
                  self->dout = (bool)(self->data_buffer & 0x8000);
                  /* printf("read dout: %d\n", dout); */
                  self->data_buffer <<= 1;
                  self->counter--;
                  if(!self->counter)
                  {
                     self->data_buffer = self->mem[self->addr & 0x3F];
                     self->addr        = (self->addr + 1) & 0x3F;
                     self->counter     = 16;
                  }
               }
               else /* Write */
               {
                  self->data_buffer <<= 1;
                  self->data_buffer |= di;

                  self->counter--;
                  if(!self->counter)
                  {
                     self->phase = AK93C45_PHASE_WRITE_PENDING;
                  }
               }
               break;
         }
      }
   }

   /* */
   self->prev_cs = cs;
   self->prev_sk = sk;

   return self->dout;
}

void AK93C45_StateAction(AK93C45 *self, StateMem *sm, const unsigned load, const bool data_only, const char *sname)
{
   SFORMAT StateRegs[] =
   {
      SFPTR16N(&(self->mem)[0], (sizeof(self->mem) / sizeof(uint16_t)), "mem"),
      SFVAR(self->write_enable),
      SFVAR(self->addr),
      SFVAR(self->data_buffer),
      SFVAR(self->counter),
      SFVAR(self->opcode),
      SFVAR(self->dout),
      SFVAR(self->prev_cs),
      SFVAR(self->prev_sk),
      SFVAR(self->phase),
      SFVAR(self->write_finish_counter),

      SFEND
   };

   MDFNSS_StateAction(sm, load, data_only, StateRegs, sname, false);

   if(load)
   {

   }
}
