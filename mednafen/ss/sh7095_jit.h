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
** when the op byte says "ordinary instruction" (< 0x80: not a delay
** slot, no exception pending) and a handler exists.  Anything else --
** delay slots, pending exceptions, and every instruction without a
** native body yet -- runs the interpreter's own switch unchanged.  The
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

/* Handler for instruction word `instr` (normal variant), compiling it
 * on first use.  NULL when the word has no native body; the caller
 * then runs the interpreter.  Cheap enough to call per instruction:
 * one table load. */
void (*SH2JIT_Handler(uint32_t instr))(struct SH7095*);

/* Handler table, indexed by instruction word; NULL = not compiled yet or
 * unsupported (SH2JIT_Handler distinguishes via a second byte table). */
extern void (*SH2JIT_Table[65536])(struct SH7095*);

void SH2JIT_Init(void);
bool SH2JIT_Available(void);

#ifdef __cplusplus
}
#endif

#endif
