/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_jit.h - SCU DSP JIT (aarch64 backend) public interface
**  Copyright (C) 2026 pstef
*/

#ifndef __MDFN_SS_SCU_DSP_JIT_H
#define __MDFN_SS_SCU_DSP_JIT_H

#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct DSPS;

void SCU_DSP_JIT_Init(void);
void SCU_DSP_JIT_Reset(void);

/* Entry stub: sets up the callee-saved frame, loads pinned regs from
 * DSPS, BLRs the handler at dsp->NextInstr.low32, then flushes pinned
 * regs back on return.  NULL on non-aarch64 builds or before init. */
extern void (*SCU_DSP_JIT_Entry)(struct DSPS*);

/* Returns NULL when JIT is not available; the caller must then use
 * the templated handler returned by DSP_DecodeInstruction. */
void (*SCU_DSP_JIT_CompileSlot(uint8_t pc, bool looped, uint32_t instr))(struct DSPS*);

#ifdef __cplusplus
}
#endif

#endif
