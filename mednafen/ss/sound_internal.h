/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sound_internal.h:  Phase-6c shared declarations between sound.c and
**                    sound_glue.cpp.
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

#ifndef __MDFN_SS_SOUND_INTERNAL_H
#define __MDFN_SS_SOUND_INTERNAL_H

#include <stdint.h>
#include <boolean.h>

#include "../mednafen-types.h"
#include "../state.h"
#include "../jump.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Cross-TU shared state between sound.c (orchestration) and
 * sound_glue.cpp (C++-class side).  Defined in sound.c. */

/* SCSP-sample boundary timer in SH-2 cycles; tracked by the
 * SOUND_Update loop and updated by RunSCSP after each sample. */
extern int32_t SOUND_next_scsp_time;

/* setjmp / longjmp recovery point.  SOUND_Update places the
 * setjmp here at the top of every invocation; the M68K bus
 * callbacks longjmp() out on bus or address errors to abort
 * the current instruction. */
extern MDFN_jmp_buf SOUND_jbuf;

/* IBuffer is part of the public sound.h ABI -- declared as
 * `extern int16_t IBuffer[1024][2]` there for libretro.cpp's
 * audio-output read path.  IBufferCount is a sound-internal
 * write cursor. */
extern uint32_t IBufferCount;

/* Runs one SCSP sample through the renderer, pushes the result
 * into IBuffer, and advances next_scsp_time by 256.  Lives on
 * the glue side because the body calls SCSP.RunSample() and
 * CDB_GetCDDA(SCSP.GetEXTSPtr()); declared here so SOUND_Update
 * can call it from sound.c. */
void SOUND_RunSCSP(void);

/* Glue wrappers around the M68K SoundCPU class instance.
 * The M68K class object lives on the C++ side; sound.c reaches
 * it through these extern "C" thin wrappers, which under LTO
 * inline to the same code the direct C++ method call would. */
void     SoundGlue_Init(void);
int32_t  SoundGlue_M68K_GetTimestamp(void);
void     SoundGlue_M68K_ResetTimestamp(void);
void     SoundGlue_M68K_Run(int32_t until);
void     SoundGlue_M68K_Reset(bool powering_up);
void     SoundGlue_M68K_SetExtHalted(bool state);
void     SoundGlue_M68K_StateAction(StateMem* sm, const unsigned load, const bool data_only,
                                    const char* sname);
uint32_t SoundGlue_M68K_GetRegister(const unsigned id, char* const special, const uint32_t special_len);
void     SoundGlue_M68K_SetRegister(const unsigned id, const uint32_t value);

/* Glue wrappers around the SS_SCSP class instance. */
void     SoundGlue_SCSP_Reset(bool powering_up);
uint16_t SoundGlue_SCSP_RW_R16(uint32_t A);
void     SoundGlue_SCSP_RW_W8(uint32_t A, uint8_t V);
void     SoundGlue_SCSP_RW_W16(uint32_t A, uint16_t V);
uint8_t  SoundGlue_SCSP_PeekRAM(uint32_t A);
void     SoundGlue_SCSP_PokeRAM(uint32_t A, uint8_t V);
void     SoundGlue_SCSP_StateAction(StateMem* sm, const unsigned load, const bool data_only,
                                    const char* sname);
uint32_t SoundGlue_SCSP_GetRegister(const unsigned id, char* const special, const uint32_t special_len);
void     SoundGlue_SCSP_SetRegister(const unsigned id, const uint32_t value);

#ifdef __cplusplus
}
#endif

#endif
