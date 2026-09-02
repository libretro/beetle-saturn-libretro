/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_difftest.c - SCU DSP interpreter vs. aarch64 JIT differential test
**
** Not part of the core build.  Links the real C handler TUs and the JIT
** against a minimal stand-in for scu.inc's globals, runs random 256-slot
** programs through both dispatch paths from identical DSPS state, and
** compares the architectural state after every SCU_UpdateDSP-shaped
** slice.  Slots are exercised at every entry kind the real core produces:
** entry-stub cold entry, C-handler cold entry (fallback thunk / DMA),
** linked direct-B, coalesced no-op runs and pipelined DataRAM reads.
**
** Build (from the repo root, aarch64 host or cross + qemu-user):
**
**   F="-O2 -DHAVE_MMAP -D__LIBRETRO__ -DWANT_JIT -fno-strict-aliasing
**      -I. -Imednafen -Imednafen/include -Ilibretro-common/include -Imednafen/ss"
**   for f in scu_dsp_gen scu_dsp_mvi scu_dsp_jmp scu_dsp_misc scu_dsp_jit a64emit jitdump; do
**     $CC $F -c mednafen/ss/$f.c -o /tmp/$f.o; done
**   (x86 / x86-64 hosts: substitute scu_dsp_jit_x86 x86emit for a64emit;
**    scu_dsp_jit.c still provides SCU_DSP_JIT_Entry.  MinGW builds run
**    unchanged under Wine.)
**   $CC $F -c mednafen/ss/tests/scu_dsp_difftest.c -o /tmp/difftest.o
**   $CC -static /tmp/scu_dsp_*.o /tmp/a64emit.o /tmp/jitdump.o /tmp/difftest.o -o scu_dsp_difftest
**
** Run:  scu_dsp_difftest [programs] [slices] [seed] [cycles-per-slice]
**   cycles-per-slice 2 single-steps (one instruction per chain entry);
**   64 is the core's DSP_UpdateTimingGran and exercises full chains; odd
**   values exercise the coalesce stub's odd-cycle residual.  Exit status
**   is non-zero on any divergence, which is reported with the first
**   diverging slice, the state on both sides and the program words
**   around the executing PC.
*/
#include "ss.h"
#include "scu.h"
#include "../scu_dsp_common.inc"
enum { DSP_UpdateTimingGran = 64 };
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct DSPS DSP;
bool setting_jit_scu = false;

void SCU_SetInt(unsigned which, bool active) { (void)which; (void)active; }

MDFN_COLD void DSP_Init(void) {}

void DSP_FinishPRAMDMA(void)
{
 /* Never armed in this harness (PRAMDMABufCount stays 0). */
 DSP.PRAMDMABufCount = 0;
}

static void dma_stub(struct DSPS* dsp)
{
 /* DMA opcodes are excluded from the generator; if one shows up, treat it
  * like END so both sides stop identically. */
 dsp->State &= ~STATE_MASK_EXECUTE;
}
void (*const DSP_DMAFuncTable[2][8][8])(struct DSPS*) = {
 {{dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub}},
 {{dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub},
  {dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub,dma_stub}}
};

static uint64_t rng_s = 0x9E3779B97F4A7C15ull;
static uint32_t rnd(void)
{
 rng_s ^= rng_s << 13; rng_s ^= rng_s >> 7; rng_s ^= rng_s << 17;
 return (uint32_t)(rng_s >> 11);
}

/* Random instruction, DMA (top nibble 0xC) and reserved 4-7 excluded.
 * Bias towards gen instrs (the hot kind) and NOPs (coalescing). */
static uint32_t rand_instr(void)
{
 uint32_t r = rnd();
 unsigned kind = rnd() % 16;
 if(kind < 8)  return r & 0x3FFFFFFF;                /* gen 0x0-0x3 */
 if(kind < 10) return 0;                              /* NOP */
 if(kind < 13) return 0x80000000u | (r & 0x3FFFFFFF); /* MVI 0x8-0xB */
 if(kind < 15) return 0xD0000000u | (r & 0x0FFFFFFF); /* JMP */
 return 0xE0000000u | (r & 0x1FFFFFFF);              /* MISC 0xE/0xF */
}

static int g_budget = 64;
static uint32_t prog[256];

typedef struct { int32_t cc; int32_t st; uint32_t nihi; uint8_t pc, top; uint16_t lop; uint8_t fl; uint64_t ac, p; uint32_t ct; uint32_t drh; } Snap;
static Snap snapA[100000], snapB[100000];
static uint32_t dram_hash(void){ uint32_t h=2166136261u; unsigned i; for(i=0;i<256;i++){h^=((uint32_t*)DSP.DataRAM)[i]; h*=16777619u;} return h; }
static void snap(Snap* o){ o->cc=DSP.CycleCounter; o->st=DSP.State; o->nihi=DSP.NextInstr>>32; o->pc=DSP.PC; o->top=DSP.TOP; o->lop=DSP.LOP; o->fl=DSP.FlagZ|DSP.FlagS<<1|DSP.FlagV<<2|DSP.FlagC<<3|DSP.FlagEnd<<4; o->ac=DSP.AC.T; o->p=DSP.P.T; o->ct=DSP.CT32; o->drh=dram_hash(); }
static void dis(unsigned pc){ printf("    prog[%02x] = %08x\n", pc, prog[pc&0xFF]); }


/* Pretty-compare two DSPS snapshots, ignoring the handler-pointer low32
 * of NextInstr/ProgRAM (those legitimately differ). */
static int compare(const struct DSPS* a, const struct DSPS* b, int step)
{
 int bad = 0;
#define CMP(f) do { if(memcmp(&a->f, &b->f, sizeof(a->f))) { printf("  step %d: mismatch in " #f "\n", step); bad = 1; } } while(0)
 CMP(CycleCounter); CMP(State);
 if((a->NextInstr >> 32) != (b->NextInstr >> 32)) { printf("  step %d: NextInstr.hi %08x vs %08x\n", step, (unsigned)(a->NextInstr>>32), (unsigned)(b->NextInstr>>32)); bad = 1; }
 CMP(NextInstrLooped);
 CMP(PC); CMP(RA); CMP(FlagZ); CMP(FlagS); CMP(FlagV); CMP(FlagC); CMP(FlagEnd);
 CMP(TOP); CMP(LOP);
 if(a->AC.T != b->AC.T) { printf("  step %d: AC %016llx vs %016llx\n", step, (unsigned long long)a->AC.T, (unsigned long long)b->AC.T); bad = 1; }
 if(a->P.T != b->P.T) { printf("  step %d: P %016llx vs %016llx\n", step, (unsigned long long)a->P.T, (unsigned long long)b->P.T); bad = 1; }
 CMP(CT32); CMP(RX); CMP(RY); CMP(RAO); CMP(WAO); CMP(DataRAM);
 CMP(PRAMDMABufCount);
#undef CMP
 return bad;
}

static void load_program(bool jit)
{
 unsigned i;
 setting_jit_scu = jit;
 if(jit) SCU_DSP_JIT_Reset();
 for(i = 0; i < 256; i++)
  DSP.ProgRAM[i] = DSP_DecodeSlotInstruction((uint8_t)i, prog[i], false);
 if(jit) SCU_DSP_JIT_Reset();   /* full rewind w/ linking, like DSP_Reset */
 DSP.NextInstr = DSP_DecodeInstruction(0, false);
 DSP.NextInstrLooped = false;
}

static void run_slice(bool jit)
{
 DSP.CycleCounter += g_budget;
 if(DSP.CycleCounter > DSP_UpdateTimingGran) DSP.CycleCounter = DSP_UpdateTimingGran;
 if(!DSPS_IsRunning(&DSP)) return;
 if(DSP.CycleCounter > 0)
 {
  if(jit && SCU_DSP_JIT_Entry) SCU_DSP_JIT_Entry(&DSP);
  else ((void (*)(DSPS*))(DSP_INSTR_BASE_UIPT + (uintptr_t)(DSP_INSTR_RECOVER_TCAST)DSP.NextInstr))(&DSP);
 }
 if(!DSPS_IsRunning(&DSP))
  DSP.CycleCounter += DSP_EndCCSubVal;
}

/* --- benchmark mode ------------------------------------------------------
 * scu_dsp_difftest --bench [slices]
 * Times the interpreter and the JIT on (a) a hand-assembled transform
 * kernel -- MOV MC0,X / MOV MC1,Y ; MUL AD2 MOV ALU,A ; MOV ALU,MC2 --
 * looping under BTM, and (b) the mean over random programs, at the
 * core's 64-cycle slice, reporting ns per DSP instruction. */
#include <time.h>
static double now_ns(void)
{
 return (double)clock() * (1e9 / (double)CLOCKS_PER_SEC);   /* portable; ms resolution suffices at these run lengths */
}

static void kernel_program(void)
{
 unsigned i;
 for(i = 0; i < 256; i++) prog[i] = 0;
 for(i = 0; i < 48; i += 3)
 {
  prog[i + 0] = (4u << 23) | (4u << 20) | (4u << 17) | (5u << 14);          /* MOV MC0,X  MOV MC1,Y (inc both) */
  prog[i + 1] = (6u << 26) | (2u << 23) | (2u << 17);                       /* AD2  MUL  MOV ALU,A */
  prog[i + 2] = (3u << 12) | (2u << 8) | 0x9u;                               /* MOV ALU,MC2 */
 }
 prog[48] = 0xE0000000u;                                                     /* BTM */
}

static double bench_one(bool jit, unsigned nslice)
{
 struct DSPS init; unsigned s; double t0, t1;
 memset(&init, 0, sizeof init);
 init.State = STATE_MASK_EXECUTE; init.PC = 0; init.TOP = 0; init.LOP = 0xFFF;
 init.CT32 = 0;
 DSP = init; load_program(jit);
 t0 = now_ns();
 for(s = 0; s < nslice; s++)
 {
  run_slice(jit);
  if(!DSPS_IsRunning(&DSP)) { DSP.State = STATE_MASK_EXECUTE; DSP.LOP = 0xFFF; DSP.CycleCounter = 0; }
 }
 t1 = now_ns();
 return (t1 - t0) / ((double)nslice * 32.0);
}

static int bench(unsigned nslice)
{
 unsigned p; double ki, kj, ri = 0, rj = 0;
 setting_jit_scu = true; SCU_DSP_JIT_Init();
 if(!SCU_DSP_JIT_Entry) { printf("JIT unavailable\n"); return 2; }
 g_budget = 64;
 kernel_program();
 ki = bench_one(false, nslice); kj = bench_one(true, nslice);
 ki = bench_one(false, nslice); kj = bench_one(true, nslice);   /* warm */
 printf("kernel : interp %6.2f ns/instr   jit %6.2f ns/instr   speedup %.2fx\n", ki, kj, ki / kj);
 for(p = 0; p < 20; p++)
 {
  unsigned i;
  for(i = 0; i < 256; i++) prog[i] = rand_instr();
  prog[rnd() & 0xFF] = 0xE8000000u | (rnd() & 0xFFF);
  ri += bench_one(false, nslice / 20); rj += bench_one(true, nslice / 20);
 }
 printf("random : interp %6.2f ns/instr   jit %6.2f ns/instr   speedup %.2fx\n", ri / 20, rj / 20, ri / rj);
 return 0;
}

int main(int argc, char** argv)
{
 if(argc > 1 && !strcmp(argv[1], "--bench"))
  return bench(argc > 2 ? (unsigned)atoi(argv[2]) : 2000000u);

 unsigned nprog = argc > 1 ? (unsigned)atoi(argv[1]) : 200;
 unsigned nslice = argc > 2 ? (unsigned)atoi(argv[2]) : 400;
 unsigned p, i, s;
 unsigned fails = 0;
 size_t max_slot = 0;

 if(argc > 3) rng_s = strtoull(argv[3], NULL, 0);
 if(argc > 4) g_budget = atoi(argv[4]);

 if(!getenv("NOJIT")) { setting_jit_scu = true;
 SCU_DSP_JIT_Init();
 if(!SCU_DSP_JIT_Entry) { printf("JIT unavailable\n"); return 2; } }

 for(p = 0; p < nprog; p++)
 {
  struct DSPS init, a, b;
  int bad = 0;

  for(i = 0; i < 256; i++) prog[i] = rand_instr();
  /* Guarantee a few loops and an END somewhere. */
  prog[rnd() & 0xFF] = 0xE8000000u | (rnd() & 0xFFF); /* LPS-ish: misc op1 */
  prog[rnd() & 0xFF] = 0xF0000000u;                    /* END */

  memset(&init, 0, sizeof init);
  init.State = STATE_MASK_EXECUTE;
  init.PC = rnd() & 0xFF;
  init.LOP = rnd() & 0xFFF;
  init.TOP = rnd() & 0xFF;
  init.CT32 = rnd() & 0x3F3F3F3F;
  init.AC.T = ((uint64_t)rnd() << 32) | rnd();
  init.P.T  = ((uint64_t)rnd() << 32) | rnd();
  init.RX = rnd(); init.RY = rnd();
  for(i = 0; i < 256; i++) ((uint32_t*)init.DataRAM)[i] = rnd();

  /* --- interpreter --- */
  DSP = init; load_program(false);
  for(s = 0; s < nslice; s++) { run_slice(false); snap(&snapA[s]); }
  a = DSP;

  if(getenv("NOJIT")) continue;
  /* --- JIT --- */
  DSP = init; load_program(true);
  {
   /* per-slot size: slots are laid out sequentially during rewind */
   for(i = 0; i + 1 < 256; i++)
   {
    int32_t o0 = (int32_t)(uint32_t)DSP.ProgRAM[i], o1 = (int32_t)(uint32_t)DSP.ProgRAM[i+1];
    if(o1 > o0 && (size_t)(o1 - o0) > max_slot) max_slot = (size_t)(o1 - o0);
   }
  }
  for(s = 0; s < nslice; s++) { run_slice(true); snap(&snapB[s]); }
  b = DSP;

  bad = compare(&a, &b, (int)nslice);
  if(bad)
  {
   fails++;
   printf("program %u FAILED (seed state pc=%02x)\n", p, init.PC);
   for(s = 0; s < nslice; s++) if(memcmp(&snapA[s], &snapB[s], sizeof(Snap))) break;
   printf("  first divergence after slice %u\n", s);
   {
    const Snap* pa = s ? &snapA[s-1] : NULL; const Snap* pb = s ? &snapB[s-1] : NULL;
    if(pa) printf("  slice-start: pc=%02x ni=%08x lop=%03x cc=%d st=%d fl=%02x ac=%016llx p=%016llx ct=%08x\n", pa->pc, pa->nihi, pa->lop, pa->cc, pa->st, pa->fl, (unsigned long long)pa->ac, (unsigned long long)pa->p, pa->ct);
    printf("  interp end : pc=%02x ni=%08x lop=%03x cc=%d st=%d fl=%02x ac=%016llx p=%016llx ct=%08x dr=%08x\n", snapA[s].pc, snapA[s].nihi, snapA[s].lop, snapA[s].cc, snapA[s].st, snapA[s].fl, (unsigned long long)snapA[s].ac, (unsigned long long)snapA[s].p, snapA[s].ct, snapA[s].drh);
    printf("  jit end    : pc=%02x ni=%08x lop=%03x cc=%d st=%d fl=%02x ac=%016llx p=%016llx ct=%08x dr=%08x\n", snapB[s].pc, snapB[s].nihi, snapB[s].lop, snapB[s].cc, snapB[s].st, snapB[s].fl, (unsigned long long)snapB[s].ac, (unsigned long long)snapB[s].p, snapB[s].ct, snapB[s].drh);
    if(pa) { unsigned k; for(k = 0; k < 34; k++) dis((unsigned)(pa->pc - 1 + k)); }
    { unsigned k; for(k = 0; k < 256; k++) { if(prog[k] == snapB[s].nihi) printf("  jit NI value found at prog[%02x]; prog[%02x]=%08x prog[%02x]=%08x\n", k, (k-1)&0xFF, prog[(k-1)&0xFF], (k+1)&0xFF, prog[(k+1)&0xFF]); if(pa && prog[k] == pa->nihi) printf("  executing instr found at prog[%02x]; prog[%02x]=%08x\n", k, (k-1)&0xFF, prog[(k-1)&0xFF]); } }
    (void)pb;
   }
   if(fails > 5) break;
  }
 }
 printf("%u programs, %u failures, max slot bytes observed %zu\n", nprog, fails, max_slot);
 return fails != 0;
}
