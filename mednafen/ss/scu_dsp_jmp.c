/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_jmp.cpp - SCU DSP JMP Instructions Emulation
**  Copyright (C) 2015-2016 Mednafen Team
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

#include "ss.h"
#include "scu.h"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("Os")
#endif

#include "scu_dsp_common.inc"

/* Phase-5b: was `template<bool looped, unsigned cond> static NO_INLINE
 * NO_CLONE void JMPInstr(DSPS* dsp)` -- a 2x128 template that, after
 * the underlying DSP_TestCond folding, collapses to 2 x 65 = 130
 * unique function instantiations (the low-6-bit cond values 0x00..0x3F
 * all share the cond&0x40==0 short-circuit and emit the same code per
 * looped value, so only the 0x00 representative is emitted; 0x40..0x7F
 * each emit their own variant).
 *
 * Monomorphized here via X-macro into 130 named functions
 * (JMPInstr_<LOOPED>_<COND>) following the same pattern phase 3f-3j
 * used for the vdp2_render function-table templates.  The body macro
 * is identical to the template body; LOOPED and COND become literal
 * constants at expansion, the DSP_TestCond / DSP_InstrPre call args
 * fold under the helper's FORCE_INLINE + const-arg treatment from
 * phase 5a, and codegen byte-matches the pre-template form. */

#define JMPInstr_BODY(LOOPED, COND)                                          \
{                                                                            \
 const uint32_t instr = DSP_InstrPre(dsp, LOOPED);                           \
                                                                             \
 if(DSP_TestCond(dsp, COND))                                                 \
  dsp->PC = (uint8_t)instr;                                                  \
                                                                             \
 DSP_TailDispatch(dsp);                                                      \
}

#define JMPInstr_NAME(LOOPED, COND) JMPInstr_##LOOPED##_##COND

#define DEFINE_JMPInstr(LOOPED, COND)                                        \
 static NO_INLINE NO_CLONE void JMPInstr_NAME(LOOPED, COND)(struct DSPS* dsp)\
 JMPInstr_BODY(LOOPED, COND)

/* The 65 representative cond values: 0x00 (covers 0x00..0x3F) +
 * 0x40..0x7F (each unique). */
#define DJI_FOR_EACH_COND(M, LOOPED) \
 M(LOOPED, 0x00) \
 M(LOOPED, 0x40) \
 M(LOOPED, 0x41) \
 M(LOOPED, 0x42) \
 M(LOOPED, 0x43) \
 M(LOOPED, 0x44) \
 M(LOOPED, 0x45) \
 M(LOOPED, 0x46) \
 M(LOOPED, 0x47) \
 M(LOOPED, 0x48) \
 M(LOOPED, 0x49) \
 M(LOOPED, 0x4A) \
 M(LOOPED, 0x4B) \
 M(LOOPED, 0x4C) \
 M(LOOPED, 0x4D) \
 M(LOOPED, 0x4E) \
 M(LOOPED, 0x4F) \
 M(LOOPED, 0x50) \
 M(LOOPED, 0x51) \
 M(LOOPED, 0x52) \
 M(LOOPED, 0x53) \
 M(LOOPED, 0x54) \
 M(LOOPED, 0x55) \
 M(LOOPED, 0x56) \
 M(LOOPED, 0x57) \
 M(LOOPED, 0x58) \
 M(LOOPED, 0x59) \
 M(LOOPED, 0x5A) \
 M(LOOPED, 0x5B) \
 M(LOOPED, 0x5C) \
 M(LOOPED, 0x5D) \
 M(LOOPED, 0x5E) \
 M(LOOPED, 0x5F) \
 M(LOOPED, 0x60) \
 M(LOOPED, 0x61) \
 M(LOOPED, 0x62) \
 M(LOOPED, 0x63) \
 M(LOOPED, 0x64) \
 M(LOOPED, 0x65) \
 M(LOOPED, 0x66) \
 M(LOOPED, 0x67) \
 M(LOOPED, 0x68) \
 M(LOOPED, 0x69) \
 M(LOOPED, 0x6A) \
 M(LOOPED, 0x6B) \
 M(LOOPED, 0x6C) \
 M(LOOPED, 0x6D) \
 M(LOOPED, 0x6E) \
 M(LOOPED, 0x6F) \
 M(LOOPED, 0x70) \
 M(LOOPED, 0x71) \
 M(LOOPED, 0x72) \
 M(LOOPED, 0x73) \
 M(LOOPED, 0x74) \
 M(LOOPED, 0x75) \
 M(LOOPED, 0x76) \
 M(LOOPED, 0x77) \
 M(LOOPED, 0x78) \
 M(LOOPED, 0x79) \
 M(LOOPED, 0x7A) \
 M(LOOPED, 0x7B) \
 M(LOOPED, 0x7C) \
 M(LOOPED, 0x7D) \
 M(LOOPED, 0x7E) \
 M(LOOPED, 0x7F)

DJI_FOR_EACH_COND(DEFINE_JMPInstr, 0)
DJI_FOR_EACH_COND(DEFINE_JMPInstr, 1)

#undef DEFINE_JMPInstr
#undef DJI_FOR_EACH_COND

MDFN_HIDE extern void (*const DSP_JMPFuncTable[2][128])(struct DSPS*) =
{
 #include "scu_dsp_jmptab.inc"
};

#undef JMPInstr_NAME
#undef JMPInstr_BODY
