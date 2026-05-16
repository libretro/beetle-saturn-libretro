/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sound_glue.cpp - C++ side of the Saturn sound module.  Phase-6c split out
**                  from sound.cpp so the orchestration half can become C
**                  (see sound.c); this file keeps the SS_SCSP / M68K class
**                  instances, the M68K bus callbacks (which need C++-side
**                  access to the class globals), and exposes everything
**                  the C side needs through extern "C" SoundGlue_* wrappers.
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

/* The two C++ class instances that drive the Saturn sound module.
 * Both are file-static here; sound.c never sees the class types
 * directly -- it only reaches them through the extern "C" wrappers
 * below. */
static SS_SCSP SCSP;
static M68K SoundCPU(true);

/* SCSP IRQ-line and main-CPU-int callbacks.  These get pulled in
 * by scsp.inc and called from the SCSP state machine; they touch
 * the M68K IPL line and the SCU interrupt latch respectively. */
static INLINE void SCSP_SoundIntChanged(SS_SCSP* s, unsigned level)
{
 SoundCPU.SetIPL(level);
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
 * They live on the C++ side because the bodies reach SCSP.RW_* (member
 * call, needs class visibility) and the global SoundCPU and SCSP
 * instances.  Their function-pointer addresses are stored in M68K's
 * fields; the calling code (M68K::Run, deep in m68k.cpp) only sees
 * the pointer values, so the calling convention is the only ABI
 * constraint -- MDFN_FASTCALL on both sides.
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
   SoundCPU.SignalAddressError(A, 0x3);                                                            \
  else                                                                                             \
   SoundCPU.SignalDTACKHalted(A);                                                                  \
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
 ret = SCSP.RW_R##WIDTH_TAG(A & 0x1FFFFF);                                                         \
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
   SoundCPU.SignalAddressError(A, 0x2);
  else
   SoundCPU.SignalDTACKHalted(A);

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
  ret = SCSP.GetRAMPtr()[A >> 1];
 else
  ret = SCSP.RW_R16(A & 0x1FFFFF);

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
   SoundCPU.SignalAddressError(A, 0x1);                                                            \
  else                                                                                             \
   SoundCPU.SignalDTACKHalted(A);                                                                  \
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
 SCSP.RW_W##WIDTH_TAG(A & 0x1FFFFF, V);                                                            \
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
  SoundCPU.SignalDTACKHalted(A);
  MDFN_longjmp(SOUND_jbuf);
 }
 //
 uint8_t tmp;

 SoundCPU.timestamp += 4;

 if(MDFN_UNLIKELY(SoundCPU.timestamp >= SOUND_next_scsp_time))
  SOUND_RunSCSP();

 tmp = SCSP.RW_R8(A & 0x1FFFFF);

 tmp = cb(&SoundCPU, tmp);

 SoundCPU.timestamp += 6;

 SCSP.RW_W8(A & 0x1FFFFF, tmp);

 SoundCPU.timestamp += 2;
}

static MDFN_FASTCALL unsigned SoundCPU_BusIntAck(uint8_t level)
{
 SoundCPU.timestamp += 10;

 return M68K::BUS_INT_ACK_AUTO;
}

static MDFN_FASTCALL void SoundCPU_BusRESET(bool state)
{
 if(state)
  SoundCPU.Reset(false);
}

/* ===================================================================
 * extern "C" wrappers exposed to sound.c
 * =================================================================== */

extern "C" {

void SoundGlue_Init(void)
{
 SoundCPU.BusRead8 = SoundCPU_BusRead_u8;
 SoundCPU.BusRead16 = SoundCPU_BusRead_u16;

 SoundCPU.BusWrite8 = SoundCPU_BusWrite_u8;
 SoundCPU.BusWrite16 = SoundCPU_BusWrite_u16;

 SoundCPU.BusReadInstr = SoundCPU_BusReadInstr;

 SoundCPU.BusRMW = SoundCPU_BusRMW;

 SoundCPU.BusIntAck = SoundCPU_BusIntAck;
 SoundCPU.BusRESET = SoundCPU_BusRESET;

 SS_SetPhysMemMap(0x05A00000, 0x05A7FFFF, SCSP.GetRAMPtr(), 0x80000, true);
 // TODO: MEM4B: SS_SetPhysMemMap(0x05A00000, 0x05AFFFFF, SCSP.GetRAMPtr(), 0x40000, true);
}

/* M68K SoundCPU accessors / forwarders. */
int32_t SoundGlue_M68K_GetTimestamp(void) { return SoundCPU.timestamp; }
void    SoundGlue_M68K_ResetTimestamp(void) { SoundCPU.timestamp = 0; }
void    SoundGlue_M68K_Run(int32_t until)   { SoundCPU.Run(until); }
void    SoundGlue_M68K_Reset(bool pwr)      { SoundCPU.Reset(pwr); }
void    SoundGlue_M68K_SetExtHalted(bool s) { SoundCPU.SetExtHalted(s); }

void SoundGlue_M68K_StateAction(StateMem* sm, const unsigned load, const bool data_only,
                                const char* sname)
{
 SoundCPU.StateAction(sm, load, data_only, sname);
}

uint32_t SoundGlue_M68K_GetRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 return SoundCPU.GetRegister(id, special, special_len);
}

void SoundGlue_M68K_SetRegister(const unsigned id, const uint32_t value)
{
 SoundCPU.SetRegister(id, value);
}

/* SS_SCSP forwarders. */
void     SoundGlue_SCSP_Reset(bool pwr)              { SCSP.Reset(pwr); }
uint16_t SoundGlue_SCSP_RW_R16(uint32_t A)           { return SCSP.RW_R16(A); }
void     SoundGlue_SCSP_RW_W8 (uint32_t A, uint8_t  V) { SCSP.RW_W8 (A, V); }
void     SoundGlue_SCSP_RW_W16(uint32_t A, uint16_t V) { SCSP.RW_W16(A, V); }

uint8_t SoundGlue_SCSP_PeekRAM(uint32_t A)
{
 /* ne16_rbo_be<uint8_t> folded. */
#ifdef MSB_FIRST
 return ((const uint8_t*)SCSP.GetRAMPtr())[A & 0x7FFFF];
#else
 return ((const uint8_t*)SCSP.GetRAMPtr())[(A & 0x7FFFF) ^ 1];
#endif
}

void SoundGlue_SCSP_PokeRAM(uint32_t A, uint8_t V)
{
 /* ne16_wbo_be<uint8_t> folded. */
#ifdef MSB_FIRST
 ((uint8_t*)SCSP.GetRAMPtr())[A & 0x7FFFF] = V;
#else
 ((uint8_t*)SCSP.GetRAMPtr())[(A & 0x7FFFF) ^ 1] = V;
#endif
}

void SoundGlue_SCSP_StateAction(StateMem* sm, const unsigned load, const bool data_only,
                                const char* sname)
{
 SCSP.StateAction(sm, load, data_only, sname);
}

uint32_t SoundGlue_SCSP_GetRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 return SCSP.GetRegister(id, special, special_len);
}

void SoundGlue_SCSP_SetRegister(const unsigned id, const uint32_t value)
{
 SCSP.SetRegister(id, value);
}

/* Runs one SCSP sample.  Called from sound.c's SOUND_Update orchestration
 * loop and from the bus-callback hot path; defined here because the body
 * accesses SCSP (C++ class instance). */
void SOUND_RunSCSP(void)
{
 CDB_GetCDDA(SCSP.GetEXTSPtr());
 //
 //
 int16_t* const bp = IBuffer[IBufferCount];
 SCSP.RunSample(bp);
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

} /* extern "C" */
