/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sound.c - Saturn sound module (SCSP renderer + M68K SoundCPU driver).
**
**           Single C TU covering the whole Saturn sound subsystem:
**           the public SOUND_* ABI surface, the SCSP register/RAM I/O
**           paths, the M68K SoundCPU bus callbacks, the SCSP IBuffer
**           ring buffer (drained by libretro.c via IBuffer in
**           sound.h), and the SOUND_Update inner loop that runs the
**           SoundCPU and the SCSP renderer in lockstep.
**
**           The 68K bus is 8-bit between SCU and SCSP -- worth
**           checking how bus access cycles interact with reads of
**           registers whose values may change between individual
**           byte reads.  May not be worth emulating if it could
**           trigger problems in games.
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
#include "scu.h"
#include "cdb.h"
#include "scsp.h"

#include <string.h>
#include "../state.h"

/* The two file-static struct instances that drive the Saturn sound
 * module.  Both are zero-initialised at program load; SOUND_Init()
 * finishes the setup by calling M68K_Construct() on SoundCPU and
 * SS_SCSP_Reset(&SCSP, true). */
static SS_SCSP SCSP;
static M68K SoundCPU;

/* setjmp/longjmp recovery point.  SOUND_Update places the setjmp at
 * the top of every invocation; the bus callbacks longjmp() out on
 * bus or address errors to abort the current instruction. */
static MDFN_jmp_buf SOUND_jbuf;

/* SCSP-sample boundary timer in SH-2 cycles; tracked by the
 * SOUND_Update loop and updated by RunSCSP after each sample. */
static int32_t SOUND_next_scsp_time;

/* 32.32 fixed-point cycle accumulator.  run_until_time tracks the
 * 68K cycle target derived from the SH-2 timestamp + clock ratio;
 * the SOUND_Update loop runs SoundCPU until M68K timestamp >=
 * (run_until_time >> 32). */
static int64_t run_until_time;
static sscpu_timestamp_t lastts;
static uint32_t clock_ratio;

int16_t IBuffer[1024][2];
uint32_t IBufferCount;

/* ST-V vs Saturn sound CPU bus-mapping flag.  Set by SOUND_Init from
 * cart_type==CART_STV.  When true, the SoundCPU bus callbacks treat
 * the 0x000000-0x7FFFFF address range (everything below the
 * 0x800000-0xFFFFFF SCSP region) as unmapped: reads return -1, instr
 * fetches return 0xFFFF, writes are dropped, RMW operates on 0xFF.
 * When false (Saturn), the same region triggers M68K
 * SignalDTACKHalted and longjmps out of the SoundCPU run loop --
 * which on real Saturn HW is correct, but on ST-V would kill the
 * sound CPU before it can do anything.
 *
 * Restores upstream Mednafen's `template<bool TA_STV>` specialisation
 * dropped by the C++ -> C source-fold pass. */
static bool sound_stv_mapping;

/* SCSP IRQ-line and main-CPU-int callbacks.  Pulled in by scsp.inc
 * and called from the SCSP state machine; touch the M68K IPL line
 * and the SCU interrupt latch respectively. */
static INLINE void SCSP_SoundIntChanged(unsigned level)
{
 M68K_SetIPL(&SoundCPU, level);
}

static INLINE void SCSP_MainIntChanged(bool state)
{
 SCU_SetInt(SCU_INT_SCSP, state);
}

#include "scsp.inc"

/* Forward decl: bus-callback bodies call RunSCSP, defined further
 * down (after the bus callbacks for source-order readability). */
static void RunSCSP(void);

/* ===================================================================
 * M68K SoundCPU bus callbacks
 *
 * Installed in SoundCPU's BusRead8 / BusRead16 / BusWrite8 /
 * BusWrite16 / BusReadInstr / BusRMW / BusIntAck / BusRESET
 * function-pointer fields from SOUND_Init().  M68K execution
 * dispatches into these for every external memory access.
 * =================================================================== */

#define SOUNDCPU_BUSREAD_BODY(T_t, WIDTH_TAG, SIZEMASK)                                            \
{                                                                                                  \
 if(MDFN_UNLIKELY(A & (0xE00000 | (SIZEMASK))))                                                    \
 {                                                                                                 \
  SoundCPU.timestamp += 4;                                                                         \
                                                                                                   \
  if(A & (SIZEMASK))                                                                               \
   M68K_SignalAddressError(&SoundCPU, A, 0x3);                                                     \
  else if(sound_stv_mapping && !(A & 0x800000))                                                    \
   return (T_t)-1;                                                                                 \
  else                                                                                             \
   M68K_SignalDTACKHalted(&SoundCPU, A);                                                           \
                                                                                                   \
  MDFN_longjmp(SOUND_jbuf);                                                                        \
 }                                                                                                 \
 /* */                                                                                             \
 T_t ret;                                                                                          \
                                                                                                   \
 SoundCPU.timestamp += 4;                                                                          \
                                                                                                   \
 if(MDFN_UNLIKELY(SoundCPU.timestamp >= SOUND_next_scsp_time))                                     \
  RunSCSP();                                                                                       \
                                                                                                   \
 ret = SS_SCSP_RW_R##WIDTH_TAG(&SCSP, A & 0x1FFFFF);                                               \
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
  else if(sound_stv_mapping && !(A & 0x800000))
   return 0xFFFF;
  else
   M68K_SignalDTACKHalted(&SoundCPU, A);

  MDFN_longjmp(SOUND_jbuf);
 }
 //
 uint16_t ret;

 SoundCPU.timestamp += 4;

 //if(MDFN_UNLIKELY(SoundCPU.timestamp >= SOUND_next_scsp_time))
 // RunSCSP();

 /* Fast path: instruction fetch goes to sound RAM (0x000000-0x07FFFF).
  * The 68K can't sensibly execute from the mirror region or SCSP
  * registers, so fall back to SCSP.RW only for the unlikely
  * pathological case (which preserves its existing open-bus-returns-0
  * / register-read behavior). */
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
   M68K_SignalAddressError(&SoundCPU, A, 0x1);                                                     \
  else if(sound_stv_mapping && !(A & 0x800000))                                                    \
   return;                                                                                         \
  else                                                                                             \
   M68K_SignalDTACKHalted(&SoundCPU, A);                                                           \
                                                                                                   \
  MDFN_longjmp(SOUND_jbuf);                                                                        \
 }                                                                                                 \
 /* */                                                                                             \
 SoundCPU.timestamp += 2;                                                                          \
                                                                                                   \
 if(MDFN_UNLIKELY(SoundCPU.timestamp >= SOUND_next_scsp_time))                                     \
  RunSCSP();                                                                                       \
                                                                                                   \
 SoundCPU.timestamp += 2;                                                                          \
                                                                                                   \
 SS_SCSP_RW_W##WIDTH_TAG(&SCSP, A & 0x1FFFFF, V);                                                  \
 SoundCPU.timestamp += 2;                                                                          \
}

static MDFN_FASTCALL void SoundCPU_BusWrite_u8 (uint32_t A, uint8_t  V) SOUNDCPU_BUSWRITE_BODY(uint8_t,  8,  0x0)
static MDFN_FASTCALL void SoundCPU_BusWrite_u16(uint32_t A, uint16_t V) SOUNDCPU_BUSWRITE_BODY(uint16_t, 16, 0x1)

#undef SOUNDCPU_BUSWRITE_BODY

static MDFN_FASTCALL void SoundCPU_BusRMW(uint32_t A, uint8_t (MDFN_FASTCALL *cb)(M68K*, uint8_t))
{
 if(MDFN_UNLIKELY(A & 0xE00000))
 {
  if(sound_stv_mapping && !(A & 0x800000))
  {
   uint8_t tmp;
   SoundCPU.timestamp += 2;
   tmp = 0xFF;
   tmp = cb(&SoundCPU, tmp);
   SoundCPU.timestamp += 4;
   SoundCPU.timestamp += 2;
   return;
  }

  SoundCPU.timestamp += 4;
  M68K_SignalDTACKHalted(&SoundCPU, A);
  MDFN_longjmp(SOUND_jbuf);
 }
 //
 uint8_t tmp;

 SoundCPU.timestamp += 4;

 if(MDFN_UNLIKELY(SoundCPU.timestamp >= SOUND_next_scsp_time))
  RunSCSP();

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

/* Runs one SCSP sample through the renderer, pushes the result into
 * IBuffer, and advances next_scsp_time by 256.  Called from the
 * SOUND_Update inner loop and from the bus-callback hot path. */
static void RunSCSP(void)
{
 int16_t* bp;

 CDB_GetCDDA(SS_SCSP_GetEXTSPtr(&SCSP));
 //
 //
 bp = IBuffer[IBufferCount];
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

/* ===================================================================
 * Public SOUND_* ABI
 * =================================================================== */

void SOUND_Init(bool stv_mapping)
{
 sound_stv_mapping = stv_mapping;

 memset(IBuffer, 0, sizeof(IBuffer));
 IBufferCount = 0;

 run_until_time = 0;
 SOUND_next_scsp_time = 0;
 lastts = 0;

 /* M68K_Construct stashes Revision_E, nulls the 7 bus-callback
  * slots, installs Dummy_BusRESET as BusRESET's default, zeroes
  * timestamp/XPending/IPL, then power-on Reset.  The 8 Bus-
  * callback slots below overwrite the nulls. */
 M68K_Construct(&SoundCPU, true);

 /* Zero the dummy half of RAM (so out-of-range playback reads
  * return 0) and reset SS_SCSP state.  The struct itself is
  * zero-initialised at program load via the file-scope `static
  * SS_SCSP SCSP;` declaration. */
 memset(SS_SCSP_GetRAMPtr(&SCSP) + 0x40000, 0x00, 0x40000 * sizeof(uint16_t));
 SS_SCSP_Reset(&SCSP, true);

 SoundCPU.BusRead8     = SoundCPU_BusRead_u8;
 SoundCPU.BusRead16    = SoundCPU_BusRead_u16;
 SoundCPU.BusWrite8    = SoundCPU_BusWrite_u8;
 SoundCPU.BusWrite16   = SoundCPU_BusWrite_u16;
 SoundCPU.BusReadInstr = SoundCPU_BusReadInstr;
 SoundCPU.BusRMW       = SoundCPU_BusRMW;
 SoundCPU.BusIntAck    = SoundCPU_BusIntAck;
 SoundCPU.BusRESET     = SoundCPU_BusRESET;

 SS_SetPhysMemMap(0x05A00000, 0x05A7FFFF, SS_SCSP_GetRAMPtr(&SCSP), 0x80000, true);
 /* TODO: MEM4B: SS_SetPhysMemMap(0x05A00000, 0x05AFFFFF, SS_SCSP_GetRAMPtr(&SCSP), 0x40000, true); */
}

/* Roll the 68K timestamp back to 0, propagating the offset to
 * next_scsp_time and run_until_time so the running cycle target
 * stays well-defined across SH-2 timestamp resets. */
static INLINE void ResetTS_68K(void)
{
 const int32_t ts = SoundCPU.timestamp;
 SOUND_next_scsp_time -= ts;
 run_until_time -= (int64_t)ts << 32;
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
 SS_SCSP_Reset(&SCSP, powering_up);
 M68K_Reset(&SoundCPU, powering_up);
}

void SOUND_Reset68K(void)
{
 M68K_Reset(&SoundCPU, false);
}

void SOUND_ResetSCSP(void)
{
 SS_SCSP_Reset(&SCSP, false);
}

void SOUND_Kill(void)
{
}

void SOUND_Set68KActive(bool active)
{
 M68K_SetExtHalted(&SoundCPU, !active);
}

uint16_t SOUND_Read16(uint32_t A)
{
 return SS_SCSP_RW_R16(&SCSP, A);
}

void SOUND_Write8(uint32_t A, uint8_t V)
{
 SS_SCSP_RW_W8(&SCSP, A, V);
}

void SOUND_Write16(uint32_t A, uint16_t V)
{
 SS_SCSP_RW_W16(&SCSP, A, V);
}

/* Ratio between SH-2 clock and 68K clock (sound clock / 2) */
void SOUND_SetClockRatio(uint32_t ratio)
{
 clock_ratio = ratio;
}

sscpu_timestamp_t SOUND_Update(sscpu_timestamp_t timestamp)
{
 int32_t cpu_ts;

 run_until_time += ((uint64_t)(timestamp - lastts) * clock_ratio);
 lastts = timestamp;
 //
 //
 MDFN_setjmp(SOUND_jbuf);

 cpu_ts = SoundCPU.timestamp;

 if(MDFN_LIKELY(cpu_ts < (run_until_time >> 32)))
 {
  do
  {
   int32_t next_time = ((int32_t)(SOUND_next_scsp_time) < (int32_t)(run_until_time >> 32)
                        ? (int32_t)(SOUND_next_scsp_time)
                        : (int32_t)(run_until_time >> 32));

   M68K_Run(&SoundCPU, next_time);

   cpu_ts = SoundCPU.timestamp;

   if(cpu_ts >= SOUND_next_scsp_time)
    RunSCSP();
  } while(MDFN_LIKELY((cpu_ts = SoundCPU.timestamp) < (run_until_time >> 32)));
 }
 else
 {
  while(SOUND_next_scsp_time < (run_until_time >> 32))
   RunSCSP();
 }

 return timestamp + 128;	/* FIXME */
}

void SOUND_StartFrame(double rate, uint32_t quality)
{
}

void SOUND_StateAction(StateMem* sm, const unsigned load, const bool data_only)
{
 int32_t ts;

 SFORMAT StateRegs[] =
 {
  SFVAR(SOUND_next_scsp_time),
  SFVAR(run_until_time),

  SFEND
 };

 //
 ts = SoundCPU.timestamp;
 SOUND_next_scsp_time -= ts;
 run_until_time -= (int64_t)ts << 32;

 MDFNSS_StateAction(sm, load, data_only, StateRegs, "SOUND", false);

 ts = SoundCPU.timestamp;
 SOUND_next_scsp_time += ts;
 run_until_time += (int64_t)ts << 32;
 //

 M68K_StateAction(&SoundCPU, sm, load, data_only, "M68K");
 SS_SCSP_StateAction(&SCSP, sm, load, data_only, "SCSP");
}

