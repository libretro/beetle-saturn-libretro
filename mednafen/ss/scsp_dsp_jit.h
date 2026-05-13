/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scsp_dsp_jit.h - SCSP DSP JIT (aarch64 backend) public interface
**  Copyright (C) 2026 pstef
*/

#ifndef __MDFN_SS_SCSP_DSP_JIT_H
#define __MDFN_SS_SCSP_DSP_JIT_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct SS_SCSP;

extern bool setting_jit_scsp;

void SCSP_DSP_JIT_Init(struct SS_SCSP* scsp);
void SCSP_DSP_JIT_Reset(struct SS_SCSP* scsp);

/* Caller must already have run DecodeMPROG.  Leaves SCSP_DSP_JIT_Entry
 * NULL when the JIT isn't available on this platform. */
void SCSP_DSP_JIT_Compile(struct SS_SCSP* scsp);

/* NULL on non-aarch64 builds or before the first compile. */
extern void (*SCSP_DSP_JIT_Entry)(struct SS_SCSP*);

#ifdef __cplusplus
}
#endif

#endif
