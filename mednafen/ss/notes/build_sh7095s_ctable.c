/* Regenerator for sh7095s_ctable.inc and sh7095s_ctable_dm.inc.
 *
 * Both files are switch-case dispatch tables for the
 * SH7095_RunSlaveUntil resume mechanism.  They map a numeric
 * resume_id (saved by the CHECK_EXIT_RESUME__ macro at the yield
 * site) back to the corresponding `Resume_NNNN:` label inside
 * the function body, via `case N: goto Resume_N;`.
 *
 * Pre-conversion, these were arrays of GCC `&&Resume_NNNN` label
 * addresses for `goto *ptr;` computed-goto dispatch.  See
 * sh7095.inc's CHECK_EXIT_RESUME__ macro definition and
 * sh7095s_rsu.inc's function-entry switch for the consumer side.
 *
 * Run:
 *   gcc -O2 build_sh7095s_ctable.c -o build_sh7095s_ctable
 *   ./build_sh7095s_ctable        > ../sh7095s_ctable.inc
 *   ./build_sh7095s_ctable debug  > ../sh7095s_ctable_dm.inc
 *
 * Range bounds (5001..5392, 10001..10392) match the static_assert
 * invariants in sh7095_ops.inc that pin the __COUNTER__ values to
 * the 5000 + 393 / 10000 + 393 base offsets.  Update the +393 in
 * both this generator and the matching assertion if the number of
 * CHECK_EXIT_RESUME() expansions in the slave path changes.
 */
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
 int debug = (argc > 1 && !strcmp(argv[1], "debug"));
 unsigned base = debug ? 10000 : 5000;
 unsigned first = base + 1;
 unsigned last  = base + 392;
 unsigned i;

 /* Header comment block. */
 printf("/* Switch-case dispatch entries for the %s SH7095_RunSlaveUntil%s\n",
        debug ? "debug" : "non-debug", debug ? "_Debug" : "");
 printf(" * resume path.  Numbered to match the __COUNTER__ values that\n");
 printf(" * CHECK_EXIT_RESUME() expansions assign as `Resume_NNNN:` labels.\n");
 printf(" *\n");
 printf(" * Range: %u .. %u (%u entries).  Regenerate via\n",
        first, last, last - first + 1);
 printf(" * notes/build_sh7095s_ctable.c%s\n", debug ? " debug" : "");
 printf(" *\n");
 printf(" * Consumes ZERO __COUNTER__ values; the resume-id integers are\n");
 printf(" * compile-time constants in each `case` label. */\n");

 for(i = first; i <= last; i++)
  printf(" case %u: goto Resume_%u;\n", i, i);

 return 0;
}
