/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scsp_dsp_difftest.c - SCSP MPROG interpreter vs. aarch64 JIT differential test
**
** Not part of the core build.  Pulls scsp.inc into a standalone TU with
** the two sound.c interrupt callbacks stubbed, so SS_SCSP_RunDSPStep /
** SS_SCSP_RunDSPInterpreter / SS_SCSP_DecodeMPROG are the real ones, and
** links scsp_dsp_jit.c.  Random 128-step MPROG programs run over random
** DSP state (TEMP/MEMS/COEF/MADRS/MIXS/EXTS/ring RAM/RBL/RBP) through both
** paths for several samples each; the whole DSP block, EFREG and the ring
** RAM are compared after every sample.
**
** Build (from the repo root, aarch64 host or cross + qemu-user):
**
**   F="-O2 -DHAVE_MMAP -D__LIBRETRO__ -DWANT_JIT -fno-strict-aliasing -DLSB_FIRST
**      -I. -Imednafen -Imednafen/include -Ilibretro-common/include -Imednafen/ss"
**   for f in scsp_dsp_jit a64emit jitdump; do $CC $F -c mednafen/ss/$f.c -o /tmp/$f.o; done
**   $CC $F -c mednafen/ss/tests/scsp_dsp_difftest.c -o /tmp/scspdiff.o
**   $CC -static /tmp/scsp_dsp_jit.o /tmp/a64emit.o /tmp/jitdump.o /tmp/scspdiff.o -o scsp_dsp_difftest
**
** Run:  scsp_dsp_difftest [programs] [samples] [seed]
**   Exit status is non-zero on any divergence; the first diverging sample
**   is reported with the MPROG word of every live step.
*/

#include "../../mednafen.h"
#include "../ss.h"
#include "../scsp.h"
#include "../scsp_dsp_jit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool setting_jit_scsp = false;

static INLINE void SCSP_SoundIntChanged(unsigned level) { (void)level; }
static INLINE void SCSP_MainIntChanged(bool state) { (void)state; }

#include "../scsp.inc"

/* SS_SCSP_StateAction is compiled in but never called here. */
int MDFNSS_StateAction(void* sm, int load, int data_only, SFORMAT* sf, const char* name, bool optional)
{ (void)sm; (void)load; (void)data_only; (void)sf; (void)name; (void)optional; return 0; }

/* Same trampolines sound.c provides for the JIT's helper-BL path. */
void SCSP_DSP_run_step(SS_SCSP* scsp, unsigned step) { SS_SCSP_RunDSPStep(scsp, step); }
void SCSP_DSP_run_interpreter(SS_SCSP* scsp) { SS_SCSP_RunDSPInterpreter(scsp); }

static uint64_t rng_s = 0x9E3779B97F4A7C15ull;
static uint32_t rnd(void)
{
 rng_s ^= rng_s << 13; rng_s ^= rng_s >> 7; rng_s ^= rng_s << 17;
 return (uint32_t)(rng_s >> 11);
}

static SS_SCSP* SA;   /* interpreter instance */
static SS_SCSP* SB;   /* JIT instance */
static SS_SCSP* S0;   /* snapshot before the sample */

static void randomize(SS_SCSP* z)
{
 unsigned i;
 memset(z, 0, sizeof *z);
 for(i = 0; i < 0x80; i++)
 {
  z->DSP.MPROG[i] = ((uint64_t)rnd() << 32) | rnd();
  if((rnd() & 7) == 0) z->DSP.MPROG[i] = 0;              /* some dead steps */
  z->DSP.TEMP[i] = sign_x_to_s32(24, rnd());
 }
 for(i = 0; i < 0x20; i++) z->DSP.MEMS[i] = sign_x_to_s32(24, rnd());
 for(i = 0; i < 64;   i++) z->DSP.COEF[i] = rnd() & 0x1FFF;
 for(i = 0; i < 32;   i++) z->DSP.MADRS[i] = rnd() & 0xFFFF;
 for(i = 0; i < 0x10; i++) z->DSP.MIXS[i] = rnd() & 0xFFFFF;
 for(i = 0; i < 0x10; i++) z->DSP.EFREG[i] = rnd() & 0xFFFF;
 z->EXTS[0] = rnd() & 0xFFFF; z->EXTS[1] = rnd() & 0xFFFF;
 z->DSP.INPUTS = sign_x_to_s32(24, rnd());
 z->DSP.SFT_REG = rnd() & 0x3FFFFFF;
 z->DSP.FRC_REG = rnd() & 0x1FFF;
 z->DSP.Y_REG = rnd() & 0xFFFFFF;
 z->DSP.ADRS_REG = rnd() & 0xFFF;
 z->DSP.MDEC_CT = rnd() & 0x1FFF;
 z->DSP.RWAddr = rnd() & 0x7FFFF;
 z->DSP.WriteValue = rnd() & 0xFFFF;
 z->DSP.ReadPending = rnd() % 3;
 z->DSP.WritePending = (rnd() & 1);
 z->DSP.ReadValue = sign_x_to_s32(24, rnd());
 z->RBL = rnd() & 3;
 z->RBP = rnd() & 0x7F;
 for(i = 0; i < 262144 * 2; i++) z->RAM[i] = (uint16_t)rnd();
 z->DSP.MPROG_Dirty = true;
}

static int compare(unsigned sample)
{
 int bad = 0;
 unsigned i;
#define CMPF(f) do { if(memcmp(&SA->f, &SB->f, sizeof(SA->f))) { printf("  sample %u: mismatch in " #f "\n", sample); bad = 1; } } while(0)
 CMPF(DSP.TEMP); CMPF(DSP.MEMS); CMPF(DSP.EFREG); CMPF(DSP.INPUTS);
 CMPF(DSP.SFT_REG); CMPF(DSP.FRC_REG); CMPF(DSP.Y_REG); CMPF(DSP.ADRS_REG);
 CMPF(DSP.MDEC_CT); CMPF(DSP.RWAddr); CMPF(DSP.WriteValue);
 CMPF(DSP.ReadPending); CMPF(DSP.WritePending); CMPF(DSP.ReadValue);
#undef CMPF
 for(i = 0; i < 262144 * 2; i++)
  if(SA->RAM[i] != SB->RAM[i]) { printf("  sample %u: RAM[%05x] interp %04x jit %04x (was %04x); pre-sample RWAddr=%05x WV=%04x RP=%u WP=%u\n", sample, i, SA->RAM[i], SB->RAM[i], S0->RAM[i], S0->DSP.RWAddr, S0->DSP.WriteValue, S0->DSP.ReadPending, S0->DSP.WritePending); bad = 1; }
 return bad;
}

int main(int argc, char** argv)
{
 unsigned nprog = argc > 1 ? (unsigned)atoi(argv[1]) : 100;
 unsigned nsamp = argc > 2 ? (unsigned)atoi(argv[2]) : 64;
 unsigned p, s, fails = 0;

 if(argc > 3) rng_s = strtoull(argv[3], NULL, 0);

 SA = (SS_SCSP*)calloc(1, sizeof(SS_SCSP));
 SB = (SS_SCSP*)calloc(1, sizeof(SS_SCSP));
 S0 = (SS_SCSP*)calloc(1, sizeof(SS_SCSP));
 if(!SA || !SB || !S0) return 2;

 setting_jit_scsp = true;
 SCSP_DSP_JIT_Init(SB);

 for(p = 0; p < nprog; p++)
 {
  int bad = 0;
  randomize(SA);
  memcpy(SB, SA, sizeof *SA);

  /* Interpreter: setting off routes SS_SCSP_RunDSP to the interpreter. */
  memcpy(S0, SA, sizeof *SA);
  setting_jit_scsp = false;
  SS_SCSP_RunDSP(SA);            /* first sample also decodes */

  setting_jit_scsp = true;
  SCSP_DSP_JIT_Reset(SB);
  SS_SCSP_RunDSP(SB);            /* decodes + compiles */
  if(!SCSP_DSP_JIT_Entry) { printf("JIT unavailable\n"); return 2; }
  bad |= compare(0);

  for(s = 1; s < nsamp && !bad; s++)
  {
   memcpy(S0, SA, sizeof *SA);
   setting_jit_scsp = false; SS_SCSP_RunDSP(SA);
   setting_jit_scsp = true;  SS_SCSP_RunDSP(SB);
   bad |= compare(s);
  }
  if(bad)
  {
   unsigned i;
   fails++;
   printf("program %u FAILED (RBL=%u RBP=%02x)\n", p, SA->RBL, SA->RBP);
   for(i = 0; i < 0x80; i++)
    if(SA->DSP.MPROG_Decoded[i].live)
     printf("    MPROG[%02x] = %016llx\n", i, (unsigned long long)SA->DSP.MPROG[i]);
   if(fails > 3) break;
  }
 }
 printf("%u programs, %u failures\n", nprog, fails);
 return fails != 0;
}
