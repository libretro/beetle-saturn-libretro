/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sound.cpp - Sound Emulation
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

// TODO: Bus between SCU and SCSP looks to be 8-bit, maybe implement that, but
// first test to see how the bus access cycle(s) work with respect to reading from
// registers whose values may change between the individual byte reads.
// (May not be worth emulating if it could possibly trigger problems in games)

#include "../mednafen.h"
#include "../hw_cpu/m68k/m68k.h"
#include "../jump.h"

#include "ss.h"
#include "sound.h"
#include "scu.h"
#include "cdb.h"

#include "scsp.h"

static SS_SCSP SCSP;

static M68K SoundCPU(true);
static int64_t run_until_time;	// 32.32
static int32_t next_scsp_time;

static uint32_t clock_ratio;
static sscpu_timestamp_t lastts;

static MDFN_jmp_buf jbuf;

int16_t IBuffer[1024][2];
uint32_t IBufferCount;

static INLINE void SCSP_SoundIntChanged(SS_SCSP* s, unsigned level)
{
 SoundCPU.SetIPL(level);
}

static INLINE void SCSP_MainIntChanged(SS_SCSP* s, bool state)
{
 SCU_SetInt(SCU_INT_SCSP, state);
}

#include "scsp.inc"

//
//
/* Phase-6a: was `template<typename T> static MDFN_FASTCALL T
 * SoundCPU_BusRead(uint32_t A)`.  Detemplated into two named
 * functions (uint8_t / uint16_t) so the body's spelling can
 * eventually compile as C; the only template-driven differences
 * are the access width and the alignment-check mask, both folded
 * into the per-size body. */
static MDFN_FASTCALL uint8_t  SoundCPU_BusRead_u8(uint32_t A);
static MDFN_FASTCALL uint16_t SoundCPU_BusRead_u16(uint32_t A);

static MDFN_FASTCALL uint16_t SoundCPU_BusReadInstr(uint32_t A);

/* Phase-6a: was `template<typename T> static MDFN_FASTCALL void
 * SoundCPU_BusWrite(uint32_t A, T V)`.  Same detemplate as the
 * Read pair. */
static MDFN_FASTCALL void SoundCPU_BusWrite_u8 (uint32_t A, uint8_t  V);
static MDFN_FASTCALL void SoundCPU_BusWrite_u16(uint32_t A, uint16_t V);

static MDFN_FASTCALL void SoundCPU_BusRMW(uint32_t A, uint8_t (MDFN_FASTCALL *cb)(M68K*, uint8_t));
static MDFN_FASTCALL unsigned SoundCPU_BusIntAck(uint8_t level);
static MDFN_FASTCALL void SoundCPU_BusRESET(bool state);
//
//

void SOUND_Init(void)
{
 memset(IBuffer, 0, sizeof(IBuffer));
 IBufferCount = 0;

 run_until_time = 0;
 next_scsp_time = 0;
 lastts = 0;

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

uint8_t SOUND_PeekRAM(uint32_t A)
{
 /* ne16_rbo_be<uint8_t> folded. */
#ifdef MSB_FIRST
 return ((const uint8_t*)SCSP.GetRAMPtr())[A & 0x7FFFF];
#else
 return ((const uint8_t*)SCSP.GetRAMPtr())[(A & 0x7FFFF) ^ 1];
#endif
}

void SOUND_PokeRAM(uint32_t A, uint8_t V)
{
 /* ne16_wbo_be<uint8_t> folded. */
#ifdef MSB_FIRST
 ((uint8_t*)SCSP.GetRAMPtr())[A & 0x7FFFF] = V;
#else
 ((uint8_t*)SCSP.GetRAMPtr())[(A & 0x7FFFF) ^ 1] = V;
#endif
}

static INLINE void ResetTS_68K(void)
{
 next_scsp_time -= SoundCPU.timestamp;
 run_until_time -= (int64_t)SoundCPU.timestamp << 32;
 SoundCPU.timestamp = 0;
}

void SOUND_AdjustTS(const int32_t delta)
{
 ResetTS_68K();
 //
 //
 lastts += delta;
}

void SOUND_Reset(bool powering_up)
{
 SCSP.Reset(powering_up);
 SoundCPU.Reset(powering_up);
}

void SOUND_Reset68K(void)
{
 SoundCPU.Reset(false);
}

void SOUND_ResetSCSP(void)
{
 SCSP.Reset(false);
}

void SOUND_Kill(void)
{
}

void SOUND_Set68KActive(bool active)
{
 SoundCPU.SetExtHalted(!active);
}

uint16_t SOUND_Read16(uint32_t A)
{
 return SCSP.RW_R16(A);
}

void SOUND_Write8(uint32_t A, uint8_t V)
{
 SCSP.RW_W8(A, V);
}

void SOUND_Write16(uint32_t A, uint16_t V)
{
 SCSP.RW_W16(A, V);
}

static NO_INLINE void RunSCSP(void)
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
 next_scsp_time += 256;
}

// Ratio between SH-2 clock and 68K clock (sound clock / 2)
void SOUND_SetClockRatio(uint32_t ratio)
{
 clock_ratio = ratio;
}

sscpu_timestamp_t SOUND_Update(sscpu_timestamp_t timestamp)
{
 run_until_time += ((uint64_t)(timestamp - lastts) * clock_ratio);
 lastts = timestamp;
 //
 //
 MDFN_setjmp(jbuf);

 if(MDFN_LIKELY(SoundCPU.timestamp < (run_until_time >> 32)))
 {
  do
  {
   int32_t next_time = ((int32_t)(next_scsp_time) < (int32_t)(run_until_time >> 32) ? (int32_t)(next_scsp_time) : (int32_t)(run_until_time >> 32));

   SoundCPU.Run(next_time);

   if(SoundCPU.timestamp >= next_scsp_time)
    RunSCSP();
  } while(MDFN_LIKELY(SoundCPU.timestamp < (run_until_time >> 32)));
 }
 else
 {
  while(next_scsp_time < (run_until_time >> 32))
   RunSCSP();
 }

 return timestamp + 128;	// FIXME
}

void SOUND_StartFrame(double rate, uint32_t quality)
{
}

void SOUND_StateAction(StateMem* sm, const unsigned load, const bool data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(next_scsp_time),
  SFVAR(run_until_time),

  SFEND
 };

 //
 next_scsp_time -= SoundCPU.timestamp;
 run_until_time -= (int64_t)SoundCPU.timestamp << 32;

 MDFNSS_StateAction(sm, load, data_only, StateRegs, "SOUND", false);

 next_scsp_time += SoundCPU.timestamp;
 run_until_time += (int64_t)SoundCPU.timestamp << 32;
 //

 SoundCPU.StateAction(sm, load, data_only, "M68K");
 SCSP.StateAction(sm, load, data_only, "SCSP");
}

//
//
//
/* Phase-6a: monomorphized via macro -- the only template-dependent
 * pieces are sizeof(T) and the alignment-error type code (0x3 for
 * misaligned read of word); the rest of the body is identical
 * across instantiations.  Phase-6b also passes the SS_SCSP RW entry
 * tag (`R8` / `R16`) so the macro can paste the non-template member
 * name for the read call. */
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
  MDFN_longjmp(jbuf);                                                                              \
 }                                                                                                 \
 /* */                                                                                             \
 T_t ret;                                                                                          \
                                                                                                   \
 SoundCPU.timestamp += 4;                                                                          \
                                                                                                   \
 if(MDFN_UNLIKELY(SoundCPU.timestamp >= next_scsp_time))                                           \
  RunSCSP();                                                                                       \
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

  MDFN_longjmp(jbuf);
 }
 //
 uint16_t ret;

 SoundCPU.timestamp += 4;

 //if(MDFN_UNLIKELY(SoundCPU.timestamp >= next_scsp_time))
 // RunSCSP();

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

/* Phase-6a: same monomorphization scheme as the BusRead pair.
 * Phase-6b: routes through SCSP.RW_W{8,16}. */
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
  MDFN_longjmp(jbuf);                                                                              \
 }                                                                                                 \
 /* */                                                                                             \
 SoundCPU.timestamp += 2;                                                                          \
                                                                                                   \
 if(MDFN_UNLIKELY(SoundCPU.timestamp >= next_scsp_time))                                           \
  RunSCSP();                                                                                       \
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
  MDFN_longjmp(jbuf);
 }
 //
 uint8_t tmp;

 SoundCPU.timestamp += 4;

 if(MDFN_UNLIKELY(SoundCPU.timestamp >= next_scsp_time))
  RunSCSP();

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

uint32_t SOUND_GetSCSPRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 return SCSP.GetRegister(id, special, special_len);
}

void SOUND_SetSCSPRegister(const unsigned id, const uint32_t value)
{
 SCSP.SetRegister(id, value);
}

uint32_t SOUND_GetM68KRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 return SoundCPU.GetRegister(id, special, special_len);
}

void SOUND_SetM68KRegister(const unsigned id, const uint32_t value)
{
 SoundCPU.SetRegister(id, value);
}
