/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sound.c - Sound Emulation, C-side orchestration.
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

/* Phase-6c sound.cpp -> sound.c split.  The C++ class instances
 * (SS_SCSP SCSP, M68K SoundCPU) plus all M68K bus callbacks live
 * in sound_glue.cpp; this file holds the C orchestration:
 *  - the public SOUND_* ABI surface
 *  - the SOUND_Update inner loop and its 32.32 fixed-point timer
 *    bookkeeping (run_until_time, lastts, clock_ratio)
 *  - the setjmp recovery point that the bus callbacks longjmp out
 *    of on bus / address errors
 *  - the SCSP IBuffer ring (output side of the renderer; filled
 *    by SOUND_RunSCSP on the glue side, drained by libretro.cpp
 *    via the IBuffer extern in sound.h)
 *
 * Bus between SCU and SCSP looks to be 8-bit, maybe implement
 * that, but first test to see how the bus access cycle(s) work
 * with respect to reading from registers whose values may change
 * between the individual byte reads.  (May not be worth emulating
 * if it could possibly trigger problems in games.)
 */

#include "ss.h"
#include "sound.h"
#include "sound_internal.h"

#include <string.h>
#include "../state.h"

/* 32.32 fixed-point cycle accumulator.  run_until_time tracks the
 * 68K cycle target derived from the SH-2 timestamp + clock ratio;
 * the SOUND_Update loop runs SoundCPU until M68K timestamp >=
 * (run_until_time >> 32). */
static int64_t run_until_time;
static sscpu_timestamp_t lastts;
static uint32_t clock_ratio;

/* Cross-TU shared state -- declared extern in sound_internal.h. */
int32_t SOUND_next_scsp_time;
MDFN_jmp_buf SOUND_jbuf;

int16_t IBuffer[1024][2];
uint32_t IBufferCount;

void SOUND_Init(void)
{
 memset(IBuffer, 0, sizeof(IBuffer));
 IBufferCount = 0;

 run_until_time = 0;
 SOUND_next_scsp_time = 0;
 lastts = 0;

 SoundGlue_Init();
}

uint8_t SOUND_PeekRAM(uint32_t A)
{
 return SoundGlue_SCSP_PeekRAM(A);
}

void SOUND_PokeRAM(uint32_t A, uint8_t V)
{
 SoundGlue_SCSP_PokeRAM(A, V);
}

/* Roll the 68K timestamp back to 0, propagating the offset to
 * next_scsp_time and run_until_time so the running cycle target
 * stays well-defined across SH-2 timestamp resets. */
static INLINE void ResetTS_68K(void)
{
 const int32_t ts = SoundGlue_M68K_GetTimestamp();
 SOUND_next_scsp_time -= ts;
 run_until_time -= (int64_t)ts << 32;
 SoundGlue_M68K_ResetTimestamp();
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
 SoundGlue_SCSP_Reset(powering_up);
 SoundGlue_M68K_Reset(powering_up);
}

void SOUND_Reset68K(void)
{
 SoundGlue_M68K_Reset(false);
}

void SOUND_ResetSCSP(void)
{
 SoundGlue_SCSP_Reset(false);
}

void SOUND_Kill(void)
{
}

void SOUND_Set68KActive(bool active)
{
 SoundGlue_M68K_SetExtHalted(!active);
}

uint16_t SOUND_Read16(uint32_t A)
{
 return SoundGlue_SCSP_RW_R16(A);
}

void SOUND_Write8(uint32_t A, uint8_t V)
{
 SoundGlue_SCSP_RW_W8(A, V);
}

void SOUND_Write16(uint32_t A, uint16_t V)
{
 SoundGlue_SCSP_RW_W16(A, V);
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

 cpu_ts = SoundGlue_M68K_GetTimestamp();

 if(MDFN_LIKELY(cpu_ts < (run_until_time >> 32)))
 {
  do
  {
   int32_t next_time = ((int32_t)(SOUND_next_scsp_time) < (int32_t)(run_until_time >> 32)
                        ? (int32_t)(SOUND_next_scsp_time)
                        : (int32_t)(run_until_time >> 32));

   SoundGlue_M68K_Run(next_time);

   cpu_ts = SoundGlue_M68K_GetTimestamp();

   if(cpu_ts >= SOUND_next_scsp_time)
    SOUND_RunSCSP();
  } while(MDFN_LIKELY((cpu_ts = SoundGlue_M68K_GetTimestamp()) < (run_until_time >> 32)));
 }
 else
 {
  while(SOUND_next_scsp_time < (run_until_time >> 32))
   SOUND_RunSCSP();
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
 ts = SoundGlue_M68K_GetTimestamp();
 SOUND_next_scsp_time -= ts;
 run_until_time -= (int64_t)ts << 32;

 MDFNSS_StateAction(sm, load, data_only, StateRegs, "SOUND", false);

 ts = SoundGlue_M68K_GetTimestamp();
 SOUND_next_scsp_time += ts;
 run_until_time += (int64_t)ts << 32;
 //

 SoundGlue_M68K_StateAction(sm, load, data_only, "M68K");
 SoundGlue_SCSP_StateAction(sm, load, data_only, "SCSP");
}

uint32_t SOUND_GetSCSPRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 return SoundGlue_SCSP_GetRegister(id, special, special_len);
}

void SOUND_SetSCSPRegister(const unsigned id, const uint32_t value)
{
 SoundGlue_SCSP_SetRegister(id, value);
}

uint32_t SOUND_GetM68KRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 return SoundGlue_M68K_GetRegister(id, special, special_len);
}

void SOUND_SetM68KRegister(const unsigned id, const uint32_t value)
{
 SoundGlue_M68K_SetRegister(id, value);
}
