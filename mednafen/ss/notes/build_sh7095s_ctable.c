/* Regenerator for sh7095s_ctable.inc.
 *
 * This file is a switch-case dispatch table for the
 * SH7095_RunSlaveUntil resume mechanism.  It maps a numeric
 * resume_id (saved by the CHECK_EXIT_RESUME__ macro at the yield
 * site) back to the corresponding `Resume_NNNN:` label inside
 * the function body, via `case N: goto Resume_N;`.
 *
 * Pre-conversion, this was an array of GCC `&&Resume_NNNN` label
 * addresses for `goto *ptr;` computed-goto dispatch.  See
 * sh7095.inc's CHECK_EXIT_RESUME__ macro definition and
 * sh7095s_rsu.inc's function-entry switch for the consumer side.
 *
 * Run:
 *   gcc -O2 build_sh7095s_ctable.c -o build_sh7095s_ctable
 *   ./build_sh7095s_ctable        > ../sh7095s_ctable.inc
 *
 * Range bounds (5001..5392) match the static_assert invariants in
 * sh7095_ops.inc that pin the __COUNTER__ values to the 5000 + 393
 * base offset.  Update the +393 in both this generator and the
 * matching assertion if the number of CHECK_EXIT_RESUME() expansions
 * in the slave path changes.
 *
 * (The companion `debug`-mode generation that produced
 * sh7095s_ctable_dm.inc has been removed: the debug-mode dispatch
 * path was a Mednafen-standalone debugger UI hook unused by this
 * libretro core.)
 */
#include <stdio.h>

int main(void)
{
 unsigned first = 5001;
 unsigned last  = 5392;
 unsigned i;

 /* Header comment block. */
 printf("/* Switch-case dispatch entries for the SH7095_RunSlaveUntil\n");
 printf(" * resume path.  Numbered to match the __COUNTER__ values that\n");
 printf(" * CHECK_EXIT_RESUME() expansions assign as `Resume_NNNN:` labels.\n");
 printf(" *\n");
 printf(" * Range: %u .. %u (%u entries).  Regenerate via\n",
        first, last, last - first + 1);
 printf(" * notes/build_sh7095s_ctable.c\n");
 printf(" *\n");
 printf(" * Consumes ZERO __COUNTER__ values; the resume-id integers are\n");
 printf(" * compile-time constants in each `case` label. */\n");

 for(i = first; i <= last; i++)
  printf(" case %u: goto Resume_%u;\n", i, i);

 return 0;
}
