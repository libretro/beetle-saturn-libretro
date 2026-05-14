/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_common.inc:
**  Copyright (C) 2015-2018 Mednafen Team
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

#if defined(__loongarch64__) || defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__arm64__) || defined(__ppc64__) || defined(__PPC64__) || defined(__ppce64le__)
 #define DSP_INSTR_BASE_UIPT ((uintptr_t)DSP_Init)
 #define DSP_INSTR_RECOVER_TCAST int32_t
#else
 #define DSP_INSTR_BASE_UIPT 0
 #define DSP_INSTR_RECOVER_TCAST uint32_t
#endif

// See loop in "SCU_UpdateDSP()" in scu.inc, and END/ENDI handling in scu_dsp_misc.cpp
enum { DSP_EndCCSubVal = 1000000 };

void DSP_Init(void) MDFN_COLD;
void DSP_FinishPRAMDMA(void);

union DSPR48
{
#ifdef MSB_FIRST
 struct
 {
  uint16_t dummy;
  uint16_t H;
  uint32_t L;
 };
#else
 struct
 {
  uint32_t L;
  uint16_t H;
  uint16_t dummy;
 };
#endif
 uint64_t T;	// Upper 16 bits may be non-zero "garbage", so don't assume they're zero in code that reads from this variable.
};

struct DSPS
{
 sscpu_timestamp_t LastTS;
 int32_t CycleCounter;
 int32_t T0_Until;
 //
 enum
 {
  // No execute = 0x00000000		<= 0
  // Paused + No execute = 0x80000000	<= 0
  // Paused + Execute = 0x80000001	<= 0
  // Execute = 0x00000001	> 0
  STATE_MASK_PAUSE = 0x80000000,
  STATE_MASK_EXECUTE = 0x00000001,
 };
 int32_t State;

 FORCE_INLINE bool IsRunning(void)	// returns true if not stopped and not paused.
 {
  return State > 0;
 }

 uint64_t NextInstr;

 uint8_t PC;
 uint8_t RA;

 bool FlagZ;
 bool FlagS;
 bool FlagV;
 bool FlagC;

 bool FlagEnd;

 uint8_t TOP;
 uint16_t LOP;	// 12 bits

 DSPR48 AC;
 DSPR48 P;

 union
 {
  uint8_t CT[4];	// 6 bits(each)
  uint32_t CT32;
 };

 uint32_t RX;
 uint32_t RY;

 uint32_t RAO;
 uint32_t WAO;

 uint32_t DataRAM[4][64];

 uint64_t ProgRAM[256];	// Upper 32 bits = actual raw instruction, lower 32 bits = horrible emulator handler pointer madness

 //
 // See comments at top of scu.inc regarding DSP DMA to program RAM.
 //
 uint32_t PRAMDMABuf[256];
 uint32_t PRAMDMABufCount;
};


// ALU Op: bits 26-29	- *16
//   X Op: bits 23-25	- * 8
//   Y Op: bits 17-19	- * 8
//  D1 Op: bits 12-13	- * 4
MDFN_HIDE extern void (*const DSP_GenFuncTable[2][16][8][8][4])(DSPS*);

// Hold/Format/Direction: bits 12-14
//  Hold: bit 14
//  Format: bit 13
//  Direction: bit 12
// RAM: bits 8-10
//
MDFN_HIDE extern void (*const DSP_DMAFuncTable[2][8][8])(DSPS*);

//
// Dest: bits 26-29
// Condition: bits 19-25
//
MDFN_HIDE extern void (*const DSP_MVIFuncTable[2][16][128])(DSPS*);

//
// Condition: bits 19-25
//
MDFN_HIDE extern void (*const DSP_JMPFuncTable[2][128])(DSPS*);


//
// LPS, BTM, END, ENDI(bits 29-31 = 0x7)
// bits 27-28
//
MDFN_HIDE extern void (*const DSP_MiscFuncTable[2][4])(DSPS*);

MDFN_HIDE extern DSPS DSP;

template<bool looped = false>
static FORCE_INLINE uint64_t DSP_DecodeInstruction(const uint32_t instr)
{
 void (*aal)(DSPS*);

 switch((instr >> 28) & 0xF)
 {
  default:
	aal = DSP_GenFuncTable[looped][0][0][0][0];
	break;

  case 0x0:
  case 0x1:
  case 0x2:
  case 0x3:
	aal = DSP_GenFuncTable[looped][(instr >> 26) & 0xF][(instr >> 23) & 0x7][(instr >> 17) & 0x7][(instr >> 12) & 0x3];
	break;

  case 0x8:
  case 0x9:
  case 0xA:
  case 0xB:
	aal = DSP_MVIFuncTable[looped][(instr >> 26) & 0xF][(instr >> 19) & 0x7F];
	break;

  case 0xC:
	aal = DSP_DMAFuncTable[looped][(instr >> 12) & 0x7][(instr >> 8) & 0x7];
	break;

  case 0xD:
	aal = DSP_JMPFuncTable[looped][(instr >> 19) & 0x7F];
	break;

  case 0xE:
  case 0xF:
	aal = DSP_MiscFuncTable[looped][(instr >> 27) & 0x3];
	break;
 }

 return ((uint64_t)instr << 32) | (uint32_t)((uintptr_t)aal - DSP_INSTR_BASE_UIPT);
}

template<bool looped>
static FORCE_INLINE uint32_t DSP_InstrPre(DSPS* dsp)
{
 const uint32_t instr = dsp->NextInstr >> 32;

 if(!looped || !dsp->LOP)
 {
  dsp->NextInstr = dsp->ProgRAM[dsp->PC];
  dsp->PC++;
 }

 if(looped)
  dsp->LOP = (dsp->LOP - 1) & 0x0FFF;

 return instr;
}

//
// At the tail of every handler: account for this instruction's cycle
// cost, decide whether the dispatcher should keep running, and call
// the next handler.  The next handler's pointer was already prefetched
// into dsp->NextInstr by DSP_InstrPre at the top of this handler.
//
// SCU_UpdateDSP caps CycleCounter at DSP_UpdateTimingGran (=64) on
// entry, and the DMA / END paths can only lower it -- so the chain is
// bounded to ~32 calls.  Modern optimizing compilers turn the call
// below into a tail jump (verified on GCC 15 / -O2 / aarch64); even
// when they don't, the bounded depth keeps the stack safe.
//
static FORCE_INLINE void DSP_TailDispatch(DSPS* dsp)
{
 dsp->CycleCounter -= 2;
 if(MDFN_UNLIKELY(dsp->CycleCounter <= 0 || !dsp->IsRunning()))
  return;

 void (* const next)(DSPS*) = (void (*)(DSPS*))(DSP_INSTR_BASE_UIPT + (uintptr_t)(DSP_INSTR_RECOVER_TCAST)dsp->NextInstr);
 next(dsp);
}

template<unsigned cond>
static FORCE_INLINE bool DSP_TestCond(DSPS* dsp)
{
 if(!(cond & 0x40))
  return true;
 //
 //
 //
 bool ret = false;

 if(cond & 0x1)
  ret |= dsp->FlagZ;

 if(cond & 0x2)
  ret |= dsp->FlagS;

 if(cond & 0x4)
  ret |= dsp->FlagC;

 if(cond & 0x8)
  ret |= (dsp->T0_Until < dsp->CycleCounter);

 //if(cond & 0x10)	// ?

 return ret == (bool)(cond & 0x20);
}
