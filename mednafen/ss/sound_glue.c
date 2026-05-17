/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sound_glue.c - Saturn sound-module glue (Phase-6c split from
**                sound.cpp, Phase-9 renamed from sound_glue.cpp
**                once SS_SCSP and M68K both shed their C++ class
**                surface).  Keeps the SS_SCSP / M68K struct
**                instances, the eight M68K bus callbacks (which
**                need access to those file-static globals), and
**                exposes everything sound.c needs through plain
**                SoundGlue_* C-linkage wrappers.
**
**  Copyright (C) 2015-2021 Mednafen Team
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

#include "../mednafen.h"
#include "../hw_cpu/m68k/m68k.h"
#include "../jump.h"

#include "ss.h"
#include "sound.h"
#include "sound_internal.h"
#include "scu.h"
#include "cdb.h"

#include "scsp.h"

/* The two file-static struct instances that drive the Saturn
 * sound module.  Both are zero-initialised at program load
 * (file scope -> implicit zero); SoundGlue_Init() finishes the
 * setup by calling M68K_Construct() on SoundCPU (in lieu of the
 * C++ ctor-call this used to spell as `static M68K SoundCPU
 * (true);`) and SS_SCSP_Reset(&SCSP, true) (in lieu of what
 * SS_SCSP::SS_SCSP() used to do implicitly).  sound.c never
 * sees these struct types directly -- it only reaches them
 * through the SoundGlue_* wrappers below. */
static SS_SCSP SCSP;
static M68K SoundCPU;

/* SCSP IRQ-line and main-CPU-int callbacks.  These get pulled in
 * by scsp.inc and called from the SCSP state machine; they touch
 * the M68K IPL line and the SCU interrupt latch respectively. */
static INLINE void SCSP_SoundIntChanged(SS_SCSP* s, unsigned level)
{
 M68K_SetIPL(&SoundCPU, level);
}

static INLINE void SCSP_MainIntChanged(SS_SCSP* s, bool state)
{
 SCU_SetInt(SCU_INT_SCSP, state);
}

#include "scsp.inc"

/* ===================================================================
 * M68K SoundCPU bus callbacks
 *
 * Installed in SoundCPU's BusRead8 / BusRead16 / BusWrite8 / BusWrite16
 * / BusReadInstr / BusRMW / BusIntAck / BusRESET function-pointer
 * fields from SoundGlue_Init().  M68K execution dispatches into these
 * for every external memory access.
 *
 * They live in this TU because the bodies reach SS_SCSP_RW_* (need
 * scsp.h's SS_SCSP type visible -- it lives in this file via scsp.inc)
 * and the file-static SoundCPU and SCSP instances.  Their function-
 * pointer addresses are stored in M68K's fields; the calling code
 * (M68K::Run, deep in m68k.cpp) only sees the pointer values, so
 * the calling convention is the only ABI constraint -- MDFN_FASTCALL
 * on both sides.
 *
 * The bus-access bodies need three pieces of cross-TU state owned
 * by sound.c: SOUND_next_scsp_time (the SCSP-sample boundary timer
 * read at every bus access), SOUND_jbuf (the longjmp recovery point
 * set up in SOUND_Update), and SOUND_RunSCSP (defined below; not in
 * sound.c, but called from sound.c too).
 * =================================================================== */

#define SOUNDCPU_BUSREAD_BODY(T_t, WIDTH_TAG, SIZEMASK)                                            \
{                                                                                                  \
 if(MDFN_UNLIKELY(A & (0xE00000 | (SIZEMASK))))                                                    \
 {                                                                                                 \
  SoundCPU.timestamp += 4;                                                                         \
                                                                                                   \
  if(A & (SIZEMASK))                                                                               \
   M68K_SignalAddressError(&SoundCPU, A, 0x3);                                                            \
  else                                                                                             \
   M68K_SignalDTACKHalted(&SoundCPU, A);                                                                  \
                                                                                                   \
  MDFN_longjmp(SOUND_jbuf);                                                                        \
 }                                                                                                 \
 /* */                                                                                             \
 T_t ret;                                                                                          \
                                                                                                   \
 SoundCPU.timestamp += 4;                                                                          \
                                                                                                   \
 if(MDFN_UNLIKELY(SoundCPU.timestamp >= SOUND_next_scsp_time))                                     \
  SOUND_RunSCSP();                                                                                 \
                                                                                                   \
 ret = SS_SCSP_RW_R##WIDTH_TAG(&SCSP, A & 0x1FFFFF);                                                         \
                                                                                                   \
 SoundCPU.timestamp += 2;                                                                          \
                                                                                                   \
 return ret;                                                                                       \
}

static MDFN_FASTCALL uint8_t  SoundCPU_BusRead_u8 (uint32_t A) SOUNDCPU_BUSREAD_BODY(uint8_t,  8,  0x0)
static MDFN_FASTCALL uint16_t SoundCPU_BusRead_u16(uint32_t A) SOUNDCPU_BUSREAD_BODY(uint16_t, 16, 0x1)

#undef SOUNDCPU_BUSREAD_BODY

static MDFN_FASTCALL uint16_t SoundCPU_BusReadInstr(uint32_t A)
{
 if(MDFN_UNLIKELY(A & 0xE00001))
 {
  SoundCPU.timestamp += 4;

  if(A & 1)
   M68K_SignalAddressError(&SoundCPU, A, 0x2);
  else
   M68K_SignalDTACKHalted(&SoundCPU, A);

  MDFN_longjmp(SOUND_jbuf);
 }
 //
 uint16_t ret;

 SoundCPU.timestamp += 4;

 //if(MDFN_UNLIKELY(SoundCPU.timestamp >= SOUND_next_scsp_time))
 // SOUND_RunSCSP();

 // Fast path: instruction fetch goes to sound RAM (0x000000-0x07FFFF).
 // The 68K can't sensibly execute from the mirror region or SCSP registers,
 // so fall back to SCSP.RW only for the unlikely pathological case (which
 // preserves its existing open-bus-returns-0 / register-read behavior).
 if(MDFN_LIKELY(A < 0x80000))
  ret = SS_SCSP_GetRAMPtr(&SCSP)[A >> 1];
 else
  ret = SS_SCSP_RW_R16(&SCSP, A & 0x1FFFFF);

 SoundCPU.timestamp += 2;

 return ret;
}

#define SOUNDCPU_BUSWRITE_BODY(T_t, WIDTH_TAG, SIZEMASK)                                           \
{                                                                                                  \
 if(MDFN_UNLIKELY(A & (0xE00000 | (SIZEMASK))))                                                    \
 {                                                                                                 \
  SoundCPU.timestamp += 4;                                                                         \
                                                                                                   \
  if(A & (SIZEMASK))                                                                               \
   M68K_SignalAddressError(&SoundCPU, A, 0x1);                                                            \
  else                                                                                             \
   M68K_SignalDTACKHalted(&SoundCPU, A);                                                                  \
                                                                                                   \
  MDFN_longjmp(SOUND_jbuf);                                                                        \
 }                                                                                                 \
 /* */                                                                                             \
 SoundCPU.timestamp += 2;                                                                          \
                                                                                                   \
 if(MDFN_UNLIKELY(SoundCPU.timestamp >= SOUND_next_scsp_time))                                     \
  SOUND_RunSCSP();                                                                                 \
                                                                                                   \
 SoundCPU.timestamp += 2;                                                                          \
                                                                                                   \
 SS_SCSP_RW_W##WIDTH_TAG(&SCSP, A & 0x1FFFFF, V);                                                            \
 SoundCPU.timestamp += 2;                                                                          \
}

static MDFN_FASTCALL void SoundCPU_BusWrite_u8 (uint32_t A, uint8_t  V) SOUNDCPU_BUSWRITE_BODY(uint8_t,  8,  0x0)
static MDFN_FASTCALL void SoundCPU_BusWrite_u16(uint32_t A, uint16_t V) SOUNDCPU_BUSWRITE_BODY(uint16_t, 16, 0x1)

#undef SOUNDCPU_BUSWRITE_BODY

static MDFN_FASTCALL void SoundCPU_BusRMW(uint32_t A, uint8_t (MDFN_FASTCALL *cb)(M68K*, uint8_t))
{
 if(MDFN_UNLIKELY(A & 0xE00000))
 {
  SoundCPU.timestamp += 4;
  M68K_SignalDTACKHalted(&SoundCPU, A);
  MDFN_longjmp(SOUND_jbuf);
 }
 //
 uint8_t tmp;

 SoundCPU.timestamp += 4;

 if(MDFN_UNLIKELY(SoundCPU.timestamp >= SOUND_next_scsp_time))
  SOUND_RunSCSP();

 tmp = SS_SCSP_RW_R8(&SCSP, A & 0x1FFFFF);

 tmp = cb(&SoundCPU, tmp);

 SoundCPU.timestamp += 6;

 SS_SCSP_RW_W8(&SCSP, A & 0x1FFFFF, tmp);

 SoundCPU.timestamp += 2;
}

static MDFN_FASTCALL unsigned SoundCPU_BusIntAck(uint8_t level)
{
 SoundCPU.timestamp += 10;

 return M68K_BUS_INT_ACK_AUTO;
}

static MDFN_FASTCALL void SoundCPU_BusRESET(bool state)
{
 if(state)
  M68K_Reset(&SoundCPU, false);
}

/* ===================================================================
 * SoundGlue_* wrappers exposed to sound.c
 *
 * No `extern "C" { ... }` block any more -- this file is now C
 * (post Phase-9 rename from sound_glue.cpp to sound_glue.c).
 * sound_internal.h still wraps the matching declarations in
 * `#ifdef __cplusplus extern "C" { ... } #endif` so any future
 * C++ consumer would see the C-linkage names; for this TU plain
 * C linkage is the default and matches what sound.c expects.
 * =================================================================== */

void SoundGlue_Init(void)
{
 /* Phase-9: replace what M68K::M68K(true) and SS_SCSP::SS_SCSP()
  * used to do implicitly at program load.  M68K_Construct does
  * what the M68K(rev_e=true) constructor did: stash Revision_E,
  * null the 7 bus-callback slots, install Dummy_BusRESET as
  * BusRESET's default, zero timestamp/XPending/IPL, then power-
  * on Reset.  The 8 Bus-callback slots below overwrite the
  * nulls that M68K_Construct just installed (BusRESET overwrites
  * Dummy_BusRESET, which was just a stop-gap default for code
  * paths that fire before SoundGlue_Init -- there are no such
  * paths in practice). */
 M68K_Construct(&SoundCPU, true);

 /* Zero the dummy half of RAM (so out-of-range playback reads
  * return 0) and reset SS_SCSP state -- what SS_SCSP::SS_SCSP()
  * used to do.  The struct itself is zero-initialised at program
  * load via the file-scope `static SS_SCSP SCSP;` declaration. */
 memset(SS_SCSP_GetRAMPtr(&SCSP) + 0x40000, 0x00, 0x40000 * sizeof(uint16_t));
 SS_SCSP_Reset(&SCSP, true);

 SoundCPU.BusRead8 = SoundCPU_BusRead_u8;
 SoundCPU.BusRead16 = SoundCPU_BusRead_u16;

 SoundCPU.BusWrite8 = SoundCPU_BusWrite_u8;
 SoundCPU.BusWrite16 = SoundCPU_BusWrite_u16;

 SoundCPU.BusReadInstr = SoundCPU_BusReadInstr;

 SoundCPU.BusRMW = SoundCPU_BusRMW;

 SoundCPU.BusIntAck = SoundCPU_BusIntAck;
 SoundCPU.BusRESET = SoundCPU_BusRESET;

 SS_SetPhysMemMap(0x05A00000, 0x05A7FFFF, SS_SCSP_GetRAMPtr(&SCSP), 0x80000, true);
 // TODO: MEM4B: SS_SetPhysMemMap(0x05A00000, 0x05AFFFFF, SS_SCSP_GetRAMPtr(&SCSP), 0x40000, true);
}

/* M68K SoundCPU accessors / forwarders. */
int32_t SoundGlue_M68K_GetTimestamp(void) { return SoundCPU.timestamp; }
void    SoundGlue_M68K_ResetTimestamp(void) { SoundCPU.timestamp = 0; }
void    SoundGlue_M68K_Run(int32_t until)   { M68K_Run(&SoundCPU, until); }
void    SoundGlue_M68K_Reset(bool pwr)      { M68K_Reset(&SoundCPU, pwr); }
void    SoundGlue_M68K_SetExtHalted(bool s) { M68K_SetExtHalted(&SoundCPU, s); }

void SoundGlue_M68K_StateAction(StateMem* sm, const unsigned load, const bool data_only,
                                const char* sname)
{
 M68K_StateAction(&SoundCPU, sm, load, data_only, sname);
}

uint32_t SoundGlue_M68K_GetRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 return M68K_GetRegister(&SoundCPU, id, special, special_len);
}

void SoundGlue_M68K_SetRegister(const unsigned id, const uint32_t value)
{
 M68K_SetRegister(&SoundCPU, id, value);
}

/* SS_SCSP forwarders. */
void     SoundGlue_SCSP_Reset(bool pwr)              { SS_SCSP_Reset(&SCSP, pwr); }
uint16_t SoundGlue_SCSP_RW_R16(uint32_t A)           { return SS_SCSP_RW_R16(&SCSP, A); }
void     SoundGlue_SCSP_RW_W8 (uint32_t A, uint8_t  V) { SS_SCSP_RW_W8(&SCSP, A, V); }
void     SoundGlue_SCSP_RW_W16(uint32_t A, uint16_t V) { SS_SCSP_RW_W16(&SCSP, A, V); }

uint8_t SoundGlue_SCSP_PeekRAM(uint32_t A)
{
 /* ne16_rbo_be<uint8_t> folded. */
#ifdef MSB_FIRST
 return ((const uint8_t*)SS_SCSP_GetRAMPtr(&SCSP))[A & 0x7FFFF];
#else
 return ((const uint8_t*)SS_SCSP_GetRAMPtr(&SCSP))[(A & 0x7FFFF) ^ 1];
#endif
}

void SoundGlue_SCSP_PokeRAM(uint32_t A, uint8_t V)
{
 /* ne16_wbo_be<uint8_t> folded. */
#ifdef MSB_FIRST
 ((uint8_t*)SS_SCSP_GetRAMPtr(&SCSP))[A & 0x7FFFF] = V;
#else
 ((uint8_t*)SS_SCSP_GetRAMPtr(&SCSP))[(A & 0x7FFFF) ^ 1] = V;
#endif
}

void SoundGlue_SCSP_StateAction(StateMem* sm, const unsigned load, const bool data_only,
                                const char* sname)
{
 SS_SCSP_StateAction(&SCSP, sm, load, data_only, sname);
}

uint32_t SoundGlue_SCSP_GetRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 return SS_SCSP_GetRegister(&SCSP, id, special, special_len);
}

void SoundGlue_SCSP_SetRegister(const unsigned id, const uint32_t value)
{
 SS_SCSP_SetRegister(&SCSP, id, value);
}

/* Runs one SCSP sample.  Called from sound.c's SOUND_Update orchestration
 * loop and from the bus-callback hot path; defined here because the body
 * accesses SCSP (C++ class instance). */
void SOUND_RunSCSP(void)
{
 CDB_GetCDDA(SS_SCSP_GetEXTSPtr(&SCSP));
 //
 //
 int16_t* const bp = IBuffer[IBufferCount];
 SS_SCSP_RunSample(&SCSP, bp);
 //bp[0] = rand();
 //bp[1] = rand();
 bp[0] = (bp[0] * 27 + 16) >> 5;
 bp[1] = (bp[1] * 27 + 16) >> 5;

/*
 // TODO?  Need to measure frequency response more reliably first, ideally after capacitor
 // replacement.  Should probably be controlled by a boolean setting, too.
 for(unsigned lr = 0; lr < 2; lr++)
 {
  static int32_t filt[2];
  filt[lr] += (((int64_t)(int32_t)((uint32_t)bp[lr] << 16) - filt[lr]) * 60500) >> 16;
  bp[lr] = filt[lr] >> 16;
 }
*/

 IBufferCount = (IBufferCount + 1) & 1023;
 SOUND_next_scsp_time += 256;
}
