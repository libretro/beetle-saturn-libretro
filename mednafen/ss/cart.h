/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* cart.h - Expansion cart emulation
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

#ifndef __MDFN_SS_CART_H
#define __MDFN_SS_CART_H

#include <stdint.h>
#include <boolean.h>

#include <retro_inline.h>        /* INLINE */
#include "../mednafen-types.h"   /* MDFN_HIDE, MDFN_COLD */
#include "../state.h"

/* Formerly relied on being a no longer used header. Now valid as C too, so
   cart.c and the cart/ device .c files can include it.
   ss_event_handler is defined in ss.h (uses SH7095 struct,
   default args); mirror the typedef here -- it is just a
   function-pointer type. The CS01 and CS2M SetRW8W16 helpers, which
   were CartInfo member functions, are now free functions taking a
   CartInfo* (see below). */

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t (*ss_event_handler)(const int32_t timestamp);

struct CartInfo
{
 void (*Reset)(bool powering_up);
 void (*Kill)(void);

 void (*GetNVInfo)(const char** ext, void** nv_ptr, bool* nv16, uint64_t* nv_size);
 bool (*GetClearNVDirty)(void);

 void (*StateAction)(StateMem* sm, const unsigned load, const bool data_only);

 void (*AdjustTS)(const int32_t delta);

 // For calculating clock ratios.
 void (*SetCPUClock)(const int32_t master_clock, const int32_t cpu_divider);

 ss_event_handler EventHandler;

 // A >> 20
 struct
 {
  void (*Read16)(uint32_t A, uint16_t* DB);
  void (*Write8)(uint32_t A, uint16_t* DB);
  void (*Write16)(uint32_t A, uint16_t* DB);
 } CS01_RW[0x30];

 struct
 {
  void (*Read16)(uint32_t A, uint16_t* DB);
  void (*Write8)(uint32_t A, uint16_t* DB);
  void (*Write16)(uint32_t A, uint16_t* DB);
 } CS2M_RW[0x20];

};

/* Were CartInfo member functions; now free functions taking the
   CartInfo*. The w8/w16 args had C++ default values of nullptr --
   C has no default args, so every caller now passes them explicitly
   (NULL where the default was relied on).

   CS2M is only used inside cart.c (initialiser for the default
   dummy mapping); the public decl moved to file-static at that
   site.  The CS01 form is consumed by every per-cart file under
   mednafen/ss/cart/, so it stays in the header. */
void CartInfo_CS01_SetRW8W16(struct CartInfo *c, uint32_t Astart, uint32_t Aend,
                             void (*r16)(uint32_t A, uint16_t* DB),
                             void (*w8)(uint32_t A, uint16_t* DB),
                             void (*w16)(uint32_t A, uint16_t* DB));

static INLINE void CART_CS01_Read16_DB(uint32_t A, uint16_t* DB)  { MDFN_HIDE extern struct CartInfo Cart; Cart.CS01_RW[(size_t)(A >> 20) - (0x02000000 >> 20)].Read16 (A, DB); }
static INLINE void CART_CS01_Write8_DB(uint32_t A, uint16_t* DB)  { MDFN_HIDE extern struct CartInfo Cart; Cart.CS01_RW[(size_t)(A >> 20) - (0x02000000 >> 20)].Write8 (A, DB); }
static INLINE void CART_CS01_Write16_DB(uint32_t A, uint16_t* DB) { MDFN_HIDE extern struct CartInfo Cart; Cart.CS01_RW[(size_t)(A >> 20) - (0x02000000 >> 20)].Write16(A, DB); }

static INLINE void CART_CS2_Read16_DB(uint32_t A, uint16_t* DB)  { MDFN_HIDE extern struct CartInfo Cart; Cart.CS2M_RW[(A >> 1) & 0x1F].Read16 (A, DB); }
static INLINE void CART_CS2_Write8_DB(uint32_t A, uint16_t* DB)  { MDFN_HIDE extern struct CartInfo Cart; Cart.CS2M_RW[(A >> 1) & 0x1F].Write8 (A, DB); }
static INLINE void CART_CS2_Write16_DB(uint32_t A, uint16_t* DB) { MDFN_HIDE extern struct CartInfo Cart; Cart.CS2M_RW[(A >> 1) & 0x1F].Write16(A, DB); }

//
// Don't change the values for existing cart types, or a save state sanity check will break.
//
enum
{
 CART__RESERVED  = -1,
 CART_NONE	 = 0x000,

 CART_BACKUP_MEM = 0x100,

 CART_EXTRAM_1M	 = 0x200,
 CART_EXTRAM_4M	 = 0x201,

 CART_KOF95	 = 0x300,
 CART_ULTRAMAN	 = 0x301,

 CART_AR4MP	 = 0x400,

 CART_CS1RAM_16M = 0x500,

 CART_NLMODEM	 = 0x600,

 CART_STV	 = 0x700,	// Sega Titan Video (ST-V) arcade hardware

 CART_BOOTROM = 0xF00
};

// CART_Init dispatches to the requested cart type. For most types the cart
// loads its firmware/ROM file from the libretro firmware directory via
// MDFN_GetSettingS+filestream_open. CART_STV is different -- it loads a
// multi-file MAME-style ROM set, so the caller must pass:
//
//   rom_dir    : directory containing the ROM image files (no trailing /)
//   main_fname : filename of the file the user actually loaded (it'll match
//                STVGameInfo->rom_layout[0].fname for the detected game)
//   sgi        : STVGameInfo returned by DB_LookupSTV()
//
// All three default to nullptr; existing CART_Init(int) call sites continue
// to compile unchanged. Non-STV cart types ignore the extra arguments.
struct STVGameInfo;
bool CART_Init(const int cart_type, const char* rom_dir,
               const char* main_fname,
               const struct STVGameInfo* sgi) MDFN_COLD;
static INLINE ss_event_handler CART_GetEventHandler(void) { MDFN_HIDE extern struct CartInfo Cart; return Cart.EventHandler; }
static INLINE void CART_AdjustTS(const int32_t delta) { MDFN_HIDE extern struct CartInfo Cart; Cart.AdjustTS(delta); }
static INLINE void CART_SetCPUClock(const int32_t master_clock, const int32_t cpu_divider) { MDFN_HIDE extern struct CartInfo Cart; Cart.SetCPUClock(master_clock, cpu_divider); }
static INLINE void CART_Kill(void) { MDFN_HIDE extern struct CartInfo Cart; if(Cart.Kill) { Cart.Kill(); Cart.Kill = NULL; } }
static INLINE void CART_StateAction(StateMem* sm, const unsigned load, const bool data_only) { MDFN_HIDE extern struct CartInfo Cart; Cart.StateAction(sm, load, data_only); }
static INLINE void CART_GetNVInfo(const char** ext, void** nv_ptr, bool* nv16, uint64_t* nv_size) { MDFN_HIDE extern struct CartInfo Cart; Cart.GetNVInfo(ext, nv_ptr, nv16, nv_size); }
static INLINE bool CART_GetClearNVDirty(void) { MDFN_HIDE extern struct CartInfo Cart; return Cart.GetClearNVDirty(); }
static INLINE void CART_Reset(bool powering_up) { MDFN_HIDE extern struct CartInfo Cart; Cart.Reset(powering_up); }

#ifdef __cplusplus
}
#endif

#endif
