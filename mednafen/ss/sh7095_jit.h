/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sh7095_jit.h - SH7095 per-instruction JIT: public interface
**
** Model.  The interpreter's SH7095_Step_w0_C0 is, per instruction: the
** FRT/WDT timestamp check, then a switch on the op byte of Pipe_ID
** (decoded id, | 0x80 in a delay slot, 0xFF with an exception pending)
** whose case fetches and decodes the next instruction (DoIDIF), stalls
** on register write-back hazards (WB_EX_CHECK), performs the operation
** and finally does PC += 2.  Everything the Saturn's timing depends on
** -- the timestamp arithmetic, MA_until, the fetch, the master/slave
** interleave at instruction granularity -- lives in that sequence, so a
** bit-exact JIT keeps every piece of it and only removes what is pure
** interpretation overhead: the decode-table lookups, the operand field
** extraction and the generic register indexing.
**
** So this is a template JIT keyed on the 16-bit instruction word: one
** specialised handler per word (register numbers and immediates folded
** to constants), compiled on first use, dispatched by the C-side step
** unless the op byte is the exception pseudo-op (0xFF), when a handler
** exists.  Delay-slot variants share a non-branch op's body verbatim,
** so they dispatch too; pending exceptions and every instruction
** without a native body run the interpreter's own switch unchanged.  The
** handler fetches through the interpreter's out-of-line DoIDIF, so
** Pipe_ID/Pipe_IF/IBuffer and the timing side effects are exactly the
** interpreter's, and the two dispatch paths can alternate freely at any
** instruction.  There is no guest-PC keying and therefore nothing to
** invalidate on code writes.
**
** tools/headless_runner.c with RUNNER_HASH_EVERY is the oracle: the
** whole machine state must hash identically with the option on and off.
*/

#ifndef __MDFN_SS_SH7095_JIT_H
#define __MDFN_SS_SH7095_JIT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SH7095;

/* Fetch/decode helper the handlers call; the interpreter's own. */
void SH7095_DoIDIF_NI_C0_I0(struct SH7095* z);

/* Fills tab[65536] with the decoder's op id for each word (sh7095.inc). */
void SH7095_JIT_BuildOpIDTable(uint8_t* tab);

/* MAC.L / MAC.W arithmetic on z->Resume_MAC_* (sh7095.inc). */
void SH7095_JIT_MACL_Arith(struct SH7095* z);
void SH7095_JIT_MACW_Arith(struct SH7095* z);

/* Branch primitives (sh7095.inc): the interpreter's macros. */
void SH7095_JIT_UCDelayBranch_C0(struct SH7095* z, uint32_t target);
void SH7095_JIT_DelayBranch_C0(struct SH7095* z, uint32_t target);
void SH7095_JIT_Branch_C0(struct SH7095* z, uint32_t target);

/* Tail of the interpreter's conditional-branch pair fusion (sh7095.inc). */
void SH7095_JIT_FusedCondBranch_C0(struct SH7095* z, bool cond);

/* Handler for instruction word `instr` (normal variant), compiling it
 * on first use.  NULL when the word has no native body; the caller
 * then runs the interpreter.  Cheap enough to call per instruction:
 * one table load. */
void (*SH2JIT_Handler(uint32_t instr))(struct SH7095*);

/* Handler table, indexed by instruction word; NULL = not compiled yet or
 * unsupported (SH2JIT_Handler distinguishes via a second byte table). */
extern void (*SH2JIT_Table[65536])(struct SH7095*);

/* 1 for compiled words whose handler touches no memory (the SH2JIT_VERIFY
 * self-check may run such a handler on a copy of the CPU). */
extern uint8_t SH2JIT_Pure[65536];

/* The decoder's op id per word (filled by SH2JIT_Init); the step checks
 * the op byte against it before dispatching. */
extern uint8_t SH2JIT_OpID[65536];

/* master/slave = &CPU[0]/&CPU[1], mem_ts = &SH7095_mem_timestamp,
 * next_event_ts = &next_event_ts: the loop state the dispatch stub reads. */
void SH2JIT_Init(struct SH7095* master, struct SH7095* slave, int32_t* mem_ts, const int32_t* next_event_ts, int32_t quantum);

/* Run a chain of master instructions from the current Pipe_ID: at least
 * the current one, then more while the dispatch stub's checks pass.
 * The caller has verified the first instruction dispatches. */
void SH2JIT_RunChain(struct SH7095* z);

/* Block compiler (master with the slave powered off): run a block from
 * the current step boundary; false if none can be compiled here. */
bool SH2JIT_RunBlock(struct SH7095* z);
void SH2JIT_InvalidateBlocks(void);
extern uint64_t SH2JIT_BlockStats[4];   /* entries, compiles, validation misses, uncompilable */

/* Make every chain exit after one instruction (SH2JIT_VERIFY builds). */
void SH2JIT_SetSingleStep(bool on);

/* ss.c: SH2JIT_Init with this TU's CPU[1], SH7095_mem_timestamp, next_event_ts. */
void SS_SH2JIT_Init(void);

/* Diagnostics (SH2JIT_COUNT builds): native instructions, chain entries, fallbacks. */
extern uint64_t SH2JIT_NativeCount, SH2JIT_ChainCount, SH2JIT_FallbackCount;
void SH2JIT_SetCounting(bool on);
bool SH2JIT_Available(void);

#ifdef __cplusplus
}
#endif

#endif
