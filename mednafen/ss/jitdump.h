/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* jitdump.h - shared Linux perf jitdump writer for the DSP JITs
**  Copyright (C) 2026 pstef
*/

#ifndef __MDFN_SS_JITDUMP_H
#define __MDFN_SS_JITDUMP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * One per-process jitdump stream shared by the SCU and SCSP DSP JITs:
 * two would O_TRUNC the same /tmp/jit-<pid>.dump and clobber each
 * other's header.  Compiled only under WANT_DSP_JIT_PERF_DUMP on
 * aarch64; elsewhere the stubs below are no-ops.
 */
#if defined(WANT_DSP_JIT_PERF_DUMP) && (defined(__aarch64__) || defined(__arm64__) || defined(__x86_64__))

void SS_JitDump_Open(void);
void SS_JitDump_Emit(const char* name, const void* code_addr, size_t code_size);

#else

static inline void SS_JitDump_Open(void) {}
static inline void SS_JitDump_Emit(const char* name, const void* code_addr, size_t code_size)
{ (void)name; (void)code_addr; (void)code_size; }

#endif

#ifdef __cplusplus
}
#endif

#endif
