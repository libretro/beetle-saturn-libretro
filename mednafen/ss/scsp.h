/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scsp.h:
**  Copyright (C) 2015-2020 Mednafen Team
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

#include "../state.h"

/* Phase-9a: class -> struct.  Members formerly under `private:`
 * are now (implicitly) public, preparing for eventual C
 * migration (C structs have no access control).  Member
 * functions remain methods of the struct -- they will be
 * converted to free functions in a later phase. */
/* Phase-9 (final): SS_SCSP nested types and enums pulled to file
 * scope so the SS_SCSP: : qualifier needed inside the converted
 * free-function bodies disappears.  Bringing scsp one more step
 * closer to plain-C compatibility.  Names are unchanged; only the
 * scope changes. */

enum
{
 GSREG_MVOL = 0,
 GSREG_DAC18B,
 GSREG_MEM4MB,
 GSREG_RBC,
 GSREG_MSLC,

 GSREG_SCIEB,
 GSREG_SCIPD,
 GSREG_MCIEB,
 GSREG_MCIPD,

 GSREG_EFREG0, GSREG_EFREG1, GSREG_EFREG2, GSREG_EFREG3, GSREG_EFREG4, GSREG_EFREG5, GSREG_EFREG6, GSREG_EFREG7,
 GSREG_EFREG8, GSREG_EFREG9, GSREG_EFREGA, GSREG_EFREGB, GSREG_EFREGC, GSREG_EFREGD, GSREG_EFREGE, GSREG_EFREGF
};

enum
{
 ENV_PHASE_ATTACK = 0,
 ENV_PHASE_DECAY1 = 1,
 ENV_PHASE_DECAY2 = 2,
 ENV_PHASE_RELEASE = 3
};

/* C-compat typedefs: in C the struct tag is not auto-aliased to a
 * type name, so a plain `SS_SCSP*` parameter at file scope fails to
 * parse without an explicit typedef.  Forward-declare all five
 * tag-to-typename aliases up front so the struct bodies below can
 * reference each other and the function decls further down can
 * spell `SS_SCSP*` directly. */
typedef struct SS_SCSP_Slot    SS_SCSP_Slot;
typedef struct SS_SCSP_Timer   SS_SCSP_Timer;
typedef struct SS_SCSP_DSPStep SS_SCSP_DSPStep;
typedef struct SS_SCSP_DSPS    SS_SCSP_DSPS;
typedef struct SS_SCSP         SS_SCSP;

enum  /* LOOP -- valid SS_SCSP_Slot::LoopMode values */
{
 LOOP_DISABLED    = 0,
 LOOP_NORMAL      = 1,
 LOOP_REVERSE     = 2,
 LOOP_ALTERNATING = 3
};

enum  /* SOURCE -- valid SS_SCSP_Slot::SourceControl values */
{
 SOURCE_MEMORY    = 0,
 SOURCE_NOISE     = 1,
 SOURCE_ZERO      = 2,
 SOURCE_UNDEFINED = 3
};

struct SS_SCSP_Slot
{
 uint32_t StartAddr;	// 20 bits, memory address.
 uint16_t LoopStart;	// 16 bits, in samples.
 uint16_t LoopEnd;	// 16 bits, in samples.
 //
 bool KeyBit;
 //
 bool WF8Bit;
 uint8_t LoopMode;

 uint8_t SourceControl;

 uint16_t SBXOR;

 uint8_t EnvRates[4];

 bool AttackHold;
 bool AttackLoopLink;
 uint8_t DecayLevel;

 uint8_t KRS;
 uint8_t TotalLevel;
 bool EGBypass;	// When true, force EG output to 0(no attenuation), but TL and ALFO still have an effect
 bool SoundDirect;	// When true, bypass EG, TL, ALFO volume control

 bool StackWriteInhibit;

 uint8_t ModLevel;
 uint8_t ModInputX;
 uint8_t ModInputY;

 uint8_t Octave;
 uint16_t FreqNum;

 uint8_t ALFOModLevel;
 uint8_t ALFOWaveform;

 uint8_t PLFOModLevel;
 uint8_t PLFOWaveform;

 uint8_t LFOFreq;

 bool LFOReset;

 // DSP mix stack
 uint8_t ToDSPSelect;
 uint8_t ToDSPLevel;

 int16_t DirectVolume[2];	// 1.14 fixed point, derived from DISDL and DIPAN
 int16_t EffectVolume[2];	// 1.14 fixed point, derived from EFSDL and EFPAN
 //
 //
 uint32_t ShortWaveMask;
 bool ShortWave;
 uint16_t CurrentAddr;
 uint32_t PhaseWhacker;
 bool InLoop;
 bool LoopSub;
 bool WFAllowAccess;
 uint8_t EnvPhase;	// ENV_PHASE_ATTACK ... ENV_PHASE_RELEASE (0...3)
 uint32_t EnvLevel;	// 0 ... 0x3FF

 uint8_t LFOCounter;
 uint16_t LFOTimeCounter;
};

enum
{
 MIDIF_INPUT_EMPTY = 0x01,
 MIDIF_INPUT_FULL  = 0x02,
 MIDIF_INPUT_OFLOW = 0x04,
 MIDIF_OUTPUT_EMPTY= 0x08,
 MIDIF_OUTPUT_FULL = 0x10
};

struct SS_SCSP_DSPStep
{
 uint8_t IRA;	// 6 bits
 uint8_t IWA;	// 5 bits
 uint8_t EWA;	// 4 bits
 uint8_t MASA;	// 5 bits
 uint8_t CRA;	// 6 bits
 uint8_t TWA;	// 7 bits, MDEC_CT added at runtime
 uint8_t TRA;	// 7 bits, MDEC_CT added at runtime
 uint8_t YSEL;	// 2 bits
 uint32_t flags;	// see DSPF_*
 uint8_t reads;	// DSPR_* bitmask of carried state this step consumes
 uint8_t writes;	// DSPW_* bitmask of carried state this step produces
 uint8_t live;	// 0 = dead step, RunDSP skips it
};

enum
{
 DSPF_NXADDR = 1u <<  0,
 DSPF_ADRGB  = 1u <<  1,
 DSPF_NOFL   = 1u <<  2,
 DSPF_BSEL   = 1u <<  3,
 DSPF_ZERO   = 1u <<  4,
 DSPF_NEGB   = 1u <<  5,
 DSPF_YRL    = 1u <<  6,
 DSPF_SHFT0  = 1u <<  7,
 DSPF_SHFT1  = 1u <<  8,
 DSPF_FRCL   = 1u <<  9,
 DSPF_ADRL   = 1u << 10,
 DSPF_EWT    = 1u << 11,
 DSPF_MRT    = 1u << 12,
 DSPF_MWT    = 1u << 13,
 DSPF_TABLE  = 1u << 14,
 DSPF_IWT    = 1u << 15,
 DSPF_XSEL   = 1u << 16,
 DSPF_TWT    = 1u << 17
};

enum
{
 DSPR_SFT  = 1u << 0,	// step consumes SFT_REG (any of EWT/TWT/FRCL/ADRL/MWT or BSEL)

 DSPW_SFT   = 1u << 0,	// SFT_REG written (always)
 DSPW_FRC   = 1u << 1,	// FRC_REG written (FRCL)
 DSPW_Y     = 1u << 2,	// Y_REG written (YRL)
 DSPW_ADRS  = 1u << 3,	// ADRS_REG written (ADRL)
 DSPW_TEMP  = 1u << 4,	// TEMP[TWA] written (TWT)
 DSPW_MEMS  = 1u << 5,	// MEMS[IWA] written (IWT)
 DSPW_EFREG = 1u << 6,	// EFREG[EWA] written (EWT)
 DSPW_RAM   = 1u << 7	// RAM pipeline state advanced (MRT or MWT)
};

struct SS_SCSP_DSPS
{
 uint64_t MPROG[0x80];
 SS_SCSP_DSPStep MPROG_Decoded[0x80];
 uint32_t TEMP[0x80];	// 24 bit
 uint32_t MEMS[0x20];	// 24 bit
 uint16_t COEF[64];	// 13 bit
 uint16_t MADRS[32];	// 16 bit

 uint32_t MIXS[0x10];	// 20 bit
 uint16_t EFREG[0x10];

 uint32_t INPUTS;	// 24 bit

 uint32_t SFT_REG;	// 26 bit
 uint16_t FRC_REG;	// 13 bit
 uint32_t Y_REG;		// 24 bit, latches INPUTS
 uint16_t ADRS_REG;	// 12 bit, latches output of A_SEL(which selects between shifter output and upper 8 bits of INPUTS

 uint16_t MDEC_CT;

 uint32_t RWAddr;

 bool WritePending;
 uint16_t WriteValue;

 uint8_t ReadPending;	// = 1 (NOFL=0), =2 (NOFL=1) at time or MRT
 uint32_t ReadValue;

 bool MPROG_Dirty;
};

/* SS_SCSP_Timer -- file-scope so the SS_SCSP_Timer typedef at the
 * top of this header resolves to the same type the SS_SCSP.Timers[3]
 * field is declared with. */
struct SS_SCSP_Timer
{
 uint8_t Control;
 uint8_t Counter;
 int32_t Reload;
};

/* SS_SCSP_SB_XOR_Table -- the 4-entry sign-bit XOR lookup used in
 * the CTL register-write paths to decode the per-slot
 * source-bit-XOR field (the (*SRV >> 9) & 0x3 selector picks one
 * of these 4 constants for s->SBXOR).  Marked `static const` so
 * each TU gets its own 8-byte read-only copy (the data is identical
 * across instances and TUs; no aliasing concern). */
static const uint16_t SS_SCSP_SB_XOR_Table[4] = { 0x0000, 0x7FFF, 0x8000, 0xFFFF };

struct SS_SCSP
{
 /* Phase-8f: RunSample's `template<typename T_out = int16_t>` form
  * was the only path-traveled instantiation -- sound.c's
  * one and only caller passes an int16_t* (the IBuffer slot) and
  * always relied on the default template argument.  The 18-bit-DAC
  * path inside the body still shifts the clamped accumulators up
  * by 2, but the result is then truncated back to int16_t at the
  * outlr[0/1] stores anyway, so the previous int32_t T_out branch
  * gained no actual precision -- it was dead code.  Hard-coding
  * int16_t retires the template, makes the method a regular class
  * member, and leaves no behaviour change. */
 /* Phase-8r1: 4 named member methods replace
  *   template<T, IsWrite> void RW(uint32_t, T&);
  * Source-folded per (T, IsWrite) tuple. */
 /* Phase-9 step 3: GetEXTSPtr / GetRAMPtr / WriteMIDI and the
  * RW_R8/R16/W8/W16 wrappers are now free functions after the
  * struct definition -- see SS_SCSP_* helpers below. */

 uint16_t SlotRegs[0x20][0x10];

 SS_SCSP_Slot Slots[32];

 uint16_t EXTS[2];
 uint16_t SoundStack[0x40];
 uint16_t SoundStackDelayer[4];

 uint16_t MasterVolume;	// 1.8 fixed point, derived from MVOL
 uint8_t MVOL;
 bool DAC18bit;
 bool Mem4Mb;

 uint32_t SlotMonitorWhich;
 uint16_t SlotMonitorData;

 bool KeyExecute;
 uint32_t LFSR;
 uint32_t GlobalCounter;

 //
 //
 struct
 {
  uint8_t InputFIFO[4];
  uint8_t InputRP, InputWP, InputCount;

  uint8_t OutputFIFO[4];
  uint8_t OutputRP, OutputWP, OutputCount;

  uint8_t Flags;
  //
  uint8_t SimuClockDivider;
  uint8_t TransmitBitCounter;
  uint16_t TransmitBuffer;

 } MIDI;
 //
 //
 uint16_t SCIEB;
 uint16_t SCIPD;

 uint16_t MCIEB;
 uint16_t MCIPD;

 uint8_t SCILV[3];
 //
 //
 SS_SCSP_Timer Timers[3];
 //
 //
 // DMEA, DRGA, and DTLG are apparently not altered by executing DMA.
 //
 uint32_t DMEA;
 uint16_t DRGA;
 uint16_t DTLG;

 bool DMA_Execute;
 bool DMA_Direction;
 bool DMA_Gate;
 //
 //
 uint8_t RBP;
 uint8_t RBL;

 // Carried-state write bitmask, plus the one read-side bit the liveness pass needs.

 //
 //

 SS_SCSP_DSPS DSP;

 uint16_t RAM[262144 * 2];	// *2 for dummy so we don't have to have so many conditionals in the playback code.

#ifdef MDFN_SS_SCSP_DSP_DYNAREC
 MDFN_ALIGN(8) uint8_t DynaRecPool[65536];
#endif
};

void SS_SCSP_Reset      (SS_SCSP* z, bool pwr) MDFN_COLD;
void SS_SCSP_StateAction(SS_SCSP* z, StateMem* sm, const unsigned load, const bool data_only, const char* sname) MDFN_COLD;
void SS_SCSP_RunSample  (SS_SCSP* z, int16_t* outlr);
void SS_SCSP_SetRegister(SS_SCSP* z, const unsigned id, const uint32_t value) MDFN_COLD;
void SS_SCSP_RW_u8_W0 (SS_SCSP* z, uint32_t A, uint8_t*  DBV_p);
void SS_SCSP_RW_u16_W0(SS_SCSP* z, uint32_t A, uint16_t* DBV_p);
void SS_SCSP_RW_u8_W1 (SS_SCSP* z, uint32_t A, uint8_t*  DBV_p);
void SS_SCSP_RW_u16_W1(SS_SCSP* z, uint32_t A, uint16_t* DBV_p);

/* Inline trivial getters. */
static FORCE_INLINE uint16_t* SS_SCSP_GetEXTSPtr(SS_SCSP* z) { return z->EXTS; }
static FORCE_INLINE uint16_t* SS_SCSP_GetRAMPtr (SS_SCSP* z) { return z->RAM;  }

/* RW front-ends -- pointer-arg form replaces former reference-arg `T&`. */
static FORCE_INLINE uint8_t  SS_SCSP_RW_R8 (SS_SCSP* z, uint32_t A)             { uint8_t  v; SS_SCSP_RW_u8_W0 (z, A, &v); return v; }
static FORCE_INLINE uint16_t SS_SCSP_RW_R16(SS_SCSP* z, uint32_t A)             { uint16_t v; SS_SCSP_RW_u16_W0(z, A, &v); return v; }
static FORCE_INLINE void     SS_SCSP_RW_W8 (SS_SCSP* z, uint32_t A, uint8_t  V) { SS_SCSP_RW_u8_W1 (z, A, &V); }
static FORCE_INLINE void     SS_SCSP_RW_W16(SS_SCSP* z, uint32_t A, uint16_t V) { SS_SCSP_RW_u16_W1(z, A, &V); }

