/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu.inc - SCU Emulation
**  Copyright (C) 2015-2021 Mednafen Team
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

// TODO: Investigate different instruction dispatch mechanisms for DSP to take advantage of modern branch
//	 prediction, to improve performance and reduce instruction cache footprint.
// TODO: Test slave SH-2 IRQ handling.
// TODO: Open bus, and correct propagation of open bus from one bus to another.
// TODO: Consider logging DMA status register reads.
// TODO: Test indirect DMA table alignment requirements.
// TODO: Indirect same-bus DMA has kind of weird effects that might actually be useful(and hence used), so test!
//
// TODO: Test and improve emulation of DSP single-stepping.
//
// TODO: Running DMA forces DSP into full execution mode when single-stepping; test the exact semantics, and implement it.
//
// TODO: Better emulation of SH-2 CPU to SCU write timings:
//		A-bus write buffer
//
//		Higher B-bus base overhead, unless the last read or write to the specific memory region(VDP1 VRAM, VDP1 FB, etc.) being accessed
//		was in the same page; pages seem to be 512 bytes for VDP1 VRAM, maybe 32 bytes for FB RAM but probably more complex.
//
// TODO: SH-2 CPU read overhead when reading from VDP1 VRAM and VDP1 FB RAM should be significantly higher.
//

/*
 Notes(assuming NTSC, HRES=0, VRES=0, LSMD=0):
   Timer0 notes:
	Counter is effectively forced to 0 while timer enable bit is 0, but it won't generate an IRQ if T0C is also 0(unless timer enable bit is set
	to 1 again)...

	Hrm...T0C of 0-263 causes interrupts.  In interlace mode though, 263 causes an interrupt at half the rate...
   Timer1 notes:
	T1S of 1-426(HRES=0x00) causes interrupt for each line, 0 and 427+ causes it every other line?


 The effect of writes to MCIEB on the interrupt output signal(as examined indirectly via IST) from the SCSP appears to be delayed somewhat;
 is the SCU buffering writes, or is the SCSP, or is it something else entirely?

 Reads from A-bus and B-bus are treated by the SCU as always 32-bit, regardless of the actual size(writes are handled properly, though).

 SCU DMA read from VRAM is reportedly unreliable?

 DMA speed for accesses to A-/B-bus, with the exceptions of A-bus CS2, should be best-case in this code
 (writes often take longer on the Saturn if the DMA write address doesn't increment).

 On a Saturn, SCU DSP DMA to program RAM starts up when the instruction is executed, but the first write to program RAM(via whatever PC is at that time)
 takes a few cycles.  Upon completion, PC is set to TOP, and the prefetched instruction is flushed.  MVI to PC should be placed immediately after the DMA
 instruction, so that PC and TOP will be set properly, and to ensure instructions aren't being fetched at the time a write to program RAM occurs.
 MVI to WAO/RAO can also be used for serializing(though it won't set up TOP and PC, and the new WAO/RAO won't affect the in-progress DMA...TODO: test with hold=1).
 END/ENDI also kinda works, since the DMA forces execution to continue, but not sure about exact semantics and if any serializing/waiting occurs or if there is the potential for glitches.
 In Mednafen, it's handled by doing the DMA instantaneously to a 256-entry buffer, and upon execution of the appropriate MVI or END/ENDI instruction, the buffer contents
 are committed to program RAM and PC is set to TOP.
*/

static const char* IntNames[0x20] =
{
 "VBIN",
 "VBOUT",
 "HBIN",
 "TIMER0",
 "TIMER1",
 "DSP",
 "SCSP",
 "SMPC",
 "PAD",
 "L2DMA",
 "L1DMA",
 "L0DMA",
 "DMA_ILL",
 "VDP1",
 NULL,
 NULL,

 "EXT0", "EXT1", "EXT2", "EXT3", "EXT4", "EXT5", "EXT6", "EXT7", "EXT8", "EXT9", "EXTA", "EXTB", "EXTC", "EXTD", "EXTE", "EXTF"
};

#include <compat/intrinsics.h>
#include "scu_dsp_common.inc"

static void DSP_Reset(bool powering_up);

enum { DMA_UpdateTimingGran = 127 };

enum { DSP_UpdateTimingGran = 64 };	// Probably should keep it a multiple of 2.

/* C-compat typedefs for the two DMA struct types defined below.
 * In C the struct tag is not auto-aliased to a type name, so
 * uses of `DMAWriteTabS` / `DMALevelS` without the `struct` keyword
 * fail to parse.  Forward-declare both typedefs here so the
 * struct bodies below and the function-level uses (`DMALevelS*
 * d = &DMALevel[...]`) all resolve in either language. */
typedef struct DMAWriteTabS DMAWriteTabS;
typedef struct DMALevelS DMALevelS;

struct DMAWriteTabS
{
 int16_t write_addr_delta;
 uint8_t write_size;
 uint8_t compare;
};

static const struct
{
 const DMAWriteTabS acb[2/*bus*/][8/*add setting*/][4/*write align*/][12/*count*/][5];
 const DMAWriteTabS aciv1[4][24][8];
} dma_write_tab =
{
 //
 {
  {
   #include "scu_actab.inc"
  },
  {
   #include "scu_btab.inc"
  }
 },

 //
 {
  #include "scu_aciv1tab.inc"
 }
};

static struct DMALevelS
{
 uint32_t StartReadAddr;
 uint32_t StartWriteAddr;
 uint32_t StartByteCount;

 bool ReadAdd;
 uint8_t WriteAdd;

 bool Enable;
 int8_t Active; // -1, 0, 1
 bool GoGoGadget;

 bool Indirect;
 bool ReadUpdate;
 bool WriteUpdate;
 uint8_t SF;

 sscpu_timestamp_t FinishTime;

 //
 //
 //
 uint32_t (*ReadFunc)(uint32_t offset);
 uint32_t WriteBus;
 //
 uint32_t CurReadBase;
 uint32_t CurReadSub;

 uint32_t CurWriteAddr;
 uint32_t CurByteCount;

 uint64_t Buffer;
 //
 const DMAWriteTabS* WATable;
 //
 uint32_t (*TableReadFunc)(uint32_t offset);	// Also serves as a kind of "CurIndirect" cache of "Indirect" variable.
 uint32_t CurTableAddr;
 bool FinalTransfer;
} DMALevel[3];

static sscpu_timestamp_t SCU_DMA_TimeCounter;
static sscpu_timestamp_t SCU_DMA_RunUntil;
static int32_t SCU_DMA_ReadOverhead;	// range -whatever to 0.
static uint32_t SCU_DMA_SDRAM_Slowdown_Counter;

static uint32_t SCU_DMA_VDP1WriteIgnoreKludge;

static void RecalcDMAHalt(void);
static void CheckDMAStart(DMALevelS* d);
static void CheckDMASFByInt(unsigned int_which);
static INLINE void CheckForceDMAFinish(void);

static sscpu_timestamp_t BBus_SH2_WriteFinishTS;

static uint32_t IAsserted;
static uint32_t IPending;
static uint32_t IMask;

static uint32_t ABusIProhibit;
static uint32_t ASR0, ASR1;
static uint8_t AREF;
static bool RSEL;

static uint8_t ILevel, IVec;

static INLINE bool CheckDoMasterInt(void)
{
 if(ILevel == 0)
 {
  static const uint8_t internal_tab[16 + 1] =
  {
   0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8,
   0x8, 0x6, 0x6, 0x5, 0x3, 0x2, 0x0, 0x0,
   0x0
  };

  static const uint8_t external_tab[16 + 1] =
  {
   0x7, 0x7, 0x7, 0x7, 0x4, 0x4, 0x4, 0x4,
   0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
   0x0
  };
  const uint32_t ipmd = IPending &~ (int16_t)IMask;
  unsigned wi   = MDFN_tzcount16(ipmd & 0xFFFF);
  unsigned we   = MDFN_tzcount16(ipmd >> 16);
  unsigned olev = internal_tab[wi];
  unsigned ovec = 0x40 + wi;
  unsigned bpos = wi;

  if(external_tab[we] > internal_tab[wi])
  {
   olev = external_tab[we];
   ovec = 0x50 + we;
   bpos = 16 + we;
  }

  if(olev != 0)
  {
   ILevel = olev;
   IVec = ovec;
   IPending &= ~(1U << bpos);

   return true;
  }
 }

 return false;
}

static int32_t Timer0_Counter;
static int32_t Timer0_Compare;
static bool Timer0_Met;

static int32_t Timer1_Reload;
static int32_t Timer1_Counter;
static bool Timer1_Mode;
static bool Timer1_Met;

static bool Timer_Enable;

static bool HB_FromVDP2, VB_FromVDP2;

//
//
//

static uint8_t SCU_MSH2VectorFetch(void)
{
 uint8_t ret = IVec;

 if(MDFN_UNLIKELY(IVec == 0x40 /* || IVec == 0x41 */))	// VB In, apply cheats.
  MDFNMP_ApplyPeriodicCheats();

 IMask = 0xBFFF;
 ILevel = 0;
 SH7095_SetIRL(&CPU[0], ILevel);

 return ret;
}

static uint8_t SCU_SSH2VectorFetch(void)
{
 if(VB_FromVDP2)
  return 0x43;

 return 0x41;
// return 0xFF;	// FIXME?
}

static INLINE void ABusIRQCheck(void)
{
 const uint32_t tt = (ABusIProhibit ^ IAsserted) & (IAsserted & ~0xFFFF);

 IPending |= tt;
 ABusIProhibit |= IAsserted & ~0xFFFF;

 if(tt)
 {
  if(CheckDoMasterInt())
   SH7095_SetIRL(&CPU[0], ILevel);
 }
}

static INLINE void SetInt(unsigned which, bool active)
{
 const uint32_t old_IAsserted = IAsserted;

 IAsserted &= ~(1U << which);
 IAsserted |= (unsigned)active << which;

 if(which >= 16)
  ABusIRQCheck();
 else
 {
  if((old_IAsserted ^ IAsserted) & IAsserted)
  {

   IPending |= 1U << which;
   CheckDMASFByInt(which);
   if(CheckDoMasterInt())
    SH7095_SetIRL(&CPU[0], ILevel);
  }
 }
}

void SCU_SetInt(unsigned which, bool active)
{
 SetInt(which, active);
}

static INLINE void Timer0_Check(void)
{
 if(Timer_Enable)
 {
  Timer0_Met = (Timer0_Counter == Timer0_Compare);
  SetInt(SCU_INT_TIMER0, Timer0_Met);
 }
}

static INLINE void Timer1_Check(void)
{
 if(Timer_Enable)
 {
  Timer1_Met |= (Timer1_Counter == 0 && (!Timer1_Mode || Timer0_Met));
  SetInt(SCU_INT_TIMER1, Timer1_Met);
 }
}

int32_t SCU_SetHBVB(int32_t pclocks, bool new_HB_FromVDP2, bool new_VB_FromVDP2)
{
 const bool HB_Start = (HB_FromVDP2 ^ new_HB_FromVDP2) & new_HB_FromVDP2;
 const bool VB_End   = (VB_FromVDP2 ^ new_VB_FromVDP2) & VB_FromVDP2;

 if(Timer_Enable)
 {
  if(VB_End)
   Timer0_Counter = 0;

  if(HB_Start)
   Timer0_Counter = (Timer0_Counter + 1) & 0x1FF;

  Timer0_Check();

  if(pclocks > 0)
  {
   Timer1_Counter = (Timer1_Counter - pclocks) & 0x1FF;
   Timer1_Check();
  }

  if(Timer1_Met && HB_Start)
  {
   Timer1_Met = false;
   Timer1_Counter = Timer1_Reload;

   SetInt(SCU_INT_TIMER1, Timer1_Met);
  }
 }

 SetInt(SCU_INT_HBIN, new_HB_FromVDP2);
 SetInt(SCU_INT_VBIN, new_VB_FromVDP2);
 SetInt(SCU_INT_VBOUT, !new_VB_FromVDP2);

 //
 //
 SH7095_SetIRL(&CPU[1], ((new_VB_FromVDP2 | new_HB_FromVDP2) << 1) | (new_VB_FromVDP2 << 2));
 //
 //
 //
 HB_FromVDP2 = new_HB_FromVDP2;
 VB_FromVDP2 = new_VB_FromVDP2;

 return Timer1_Counter ? Timer1_Counter : 0x200;
}

/* Phase-7e: promoted from file-static so ss_init.c’s Emulate can call it. */
void SCU_AdjustTS(const int32_t delta)
{
 SCU_DMA_TimeCounter += delta;
 SCU_DMA_RunUntil += delta;
 for(unsigned level___ = 0; level___ < 3; level___++)
 {
  DMALevelS* d = &DMALevel[level___];
  if(d->Active < 0)
   d->FinishTime += delta;
 }

 //
 //
 //
 if(DSP.T0_Until > 0x10000000)
  DSP.T0_Until = 0x10000000;

 //DSP.LastTS = ((sscpu_timestamp_t)(-1000000) > (sscpu_timestamp_t)(DSP.LastTS + delta) ? (sscpu_timestamp_t)(-1000000) : (sscpu_timestamp_t)(DSP.LastTS + delta));

 DSP.LastTS += delta;
 // TODO: Fix properly.
 if(DSP.LastTS < 0)
  DSP.LastTS = 0;
 //
 //
 //
 BBus_SH2_WriteFinishTS = ((sscpu_timestamp_t)(-1000000) > (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS + delta) ? (sscpu_timestamp_t)(-1000000) : (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS + delta));
}

//
// TODO: Test to see if the entire data bus or only parts are asserted for uint8_t and uint16_t reads
//
/* Phase-8q2: SCU_RegRW_DB retired into 6 named variants via
 * source-fold (tools/fold_source.py).  Each variant body is
 * the template body with template parameters substituted to
 * literals AND dead branches eliminated at source level. */

/* === SCU_RegRW_DB variants (3 T x 2 IsWrite) === */
static INLINE void SCU_RegRW_DB_u8_W0(uint32_t A, uint32_t* DB)
{
  switch(A & 0xFC)
  {
   default:
	*DB = 0;
	break;

   case 0xA4:
	*DB = IPending;
	break;

   case 0xA8:
	*DB = 0; //ABusIAck;
	break;

   case 0xC4:
	*DB = RSEL;
	break;

   case 0xC8:
	*DB = 0x4;
	break;
   //
   //
   //
   case 0x00:
   case 0x20:
   case 0x40:
	{
	 const DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 *DB = d->StartReadAddr;
	}
	break;

   case 0x04:
   case 0x24:
   case 0x44:
	{
	 const DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 *DB = d->StartWriteAddr;
	}
	break;

   case 0x7C:
	{
	 uint32_t tmp = 0;

	 for(unsigned level = 0; level < 3; level++)
	 {
	  DMALevelS* d = &DMALevel[level];

	  if(d->Active)
	  {
	   tmp |= 0x10 << (level << 2);
	  }
	 }

	 if(DMALevel[0].Active && (DMALevel[1].Active || DMALevel[2].Active))
	  tmp |= 1U << 16;

	 if(DMALevel[1].Active && DMALevel[2].Active)
	  tmp |= 1U << 17;

	 *DB = tmp;
	}
	break;

   case 0x80:
	SS_SetEventNT(&events[SS_EVENT_SCU_DSP], SCU_UpdateDSP(SH7095_mem_timestamp));	// TODO: Remove?
	{
	 uint32_t tmp;

	 tmp = DSP.PC;
	 tmp |= (DSP.T0_Until < DSP.CycleCounter) << 23;
	 tmp |= DSP.FlagS << 22;
	 tmp |= DSP.FlagZ << 21;
	 tmp |= DSP.FlagC << 20;
	 tmp |= DSP.FlagV << 19;
	 tmp |= DSP.FlagEnd << 18;
	 tmp |= DSPS_IsRunning(&DSP) << 16;
	 *DB = tmp;
	 //
	 DSP.FlagV = false;
	 DSP.FlagEnd = false;
	 SCU_SetInt(SCU_INT_DSP, false);
	}
	break;

   case 0x8C:
	if(!DSPS_IsRunning(&DSP))
	 *DB = ((uint32_t*)DSP.DataRAM)[DSP.RA++];
	else
	 *DB = 0xFFFFFFFF;
	break;
  }
 
}

static INLINE void SCU_RegRW_DB_u8_W1(uint32_t A, uint32_t* DB)
{
 unsigned mask;

  mask = 0xFF << (((A & 3) ^ 3) << 3); 

  switch(A & 0xFC)
  {
   default:
	break;

   case 0x90:	// Timer 0 Compare
	{
	 VDP2_Update(SH7095_mem_timestamp);
	 Timer0_Compare = (Timer0_Compare &~ mask) | (*DB & mask & 0x3FF);
	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	}
	break;

   case 0x94:	// Timer 1 Reload Value
	{
	 VDP2_Update(SH7095_mem_timestamp);
	 Timer1_Reload = (Timer1_Reload &~ mask) | (*DB & mask & 0x1FF);
	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	}
	break;

   case 0x98:	// Timer Control
	{
	 VDP2_Update(SH7095_mem_timestamp);
	 uint32_t tmp = (Timer1_Mode << 8) | (Timer_Enable << 0);
	 tmp = (tmp &~ mask) | (*DB & mask);
	 Timer1_Mode = (tmp >> 8) & 1;
	 Timer_Enable = (tmp >> 0) & 1;

	 if(!Timer_Enable)
	 {
	  Timer0_Counter = 0;
	 }

	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	}
	break;

   case 0xA0:
	IMask = (IMask &~ mask) | (*DB & mask & 0xBFFF);

	if(CheckDoMasterInt())
	{
/*
	 if(MDFN_UNLIKELY(ss_horrible_hacks & HORRIBLEHACK_SCUINTDELAY))
	  SS_SetEventNT(&events[SS_EVENT_SCU_INT], SH7095_mem_timestamp + 8);
	 else
*/
	  SH7095_SetIRL(&CPU[0], ILevel);
	}
	break;

   case 0xA4:
	IPending &= *DB | ~mask;
	break;

   case 0xA8:
	if(*DB & mask & 0x0001)
	{
	 ABusIProhibit = 0; //&= ~IAsserted;
	 ABusIRQCheck();
	}
	break;

   case 0xB0:
	ASR0 = (ASR0 &~ mask) | (*DB & mask & 0xFFFDFFFD);
	break;

   case 0xB4:
	ASR1 = (ASR1 &~ mask) | (*DB & mask & 0xF00DFFFD);
	break;

   case 0xB8:
	AREF = (AREF &~ mask) | (*DB & mask & 0x1F);
	break;

   case 0xC4:
	RSEL = (RSEL &~ mask) | (*DB & mask & 0x1);
	break;

   case 0x00:
   case 0x20:
   case 0x40:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 d->StartReadAddr = (d->StartReadAddr &~ mask) | (*DB & mask & 0x07FFFFFF);
	}
	break;

   case 0x04:
   case 0x24:
   case 0x44:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 d->StartWriteAddr = (d->StartWriteAddr &~ mask) | (*DB & mask & 0x07FFFFFF);
	}
	break;

   case 0x08:
   case 0x28:
   case 0x48:
	{
	 const unsigned level = (A >> 5) & 0x3;
	 DMALevelS* d = &DMALevel[level];

	 d->StartByteCount = (d->StartByteCount &~ mask) | (*DB & mask & (level ? 0x00000FFF : 0x000FFFFF));
        }
	break;

   case 0x0C:
   case 0x2C:
   case 0x4C:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];
	 uint32_t tmp = (d->ReadAdd << 8) | (d->WriteAdd << 0);

	 tmp = (tmp &~ mask) | (*DB & mask);

	 d->ReadAdd = (tmp >> 8) & 0x1;
	 d->WriteAdd = (tmp >> 0) & 0x7;
	}
	break;

   case 0x10:
   case 0x30:
   case 0x50:
	{
	 const unsigned level = (A >> 5) & 0x3;
	 DMALevelS* d = &DMALevel[level];
	 uint32_t tmp = (d->Enable << 8);

	 tmp = (tmp &~ mask) | (*DB & mask);	 
	 d->Enable = (tmp >> 8) & 0x1;

	 if((tmp & 0x1) && d->Enable && d->SF == 0x7)
	 {
	  SCU_UpdateDMA(SH7095_mem_timestamp);

	  d->GoGoGadget = true;
	  CheckDMAStart(d);

	  SS_SetEventNT(&events[SS_EVENT_SCU_DMA], SCU_UpdateDMA(SH7095_mem_timestamp));
	 }
	}
	break;

   case 0x14:
   case 0x34:
   case 0x54:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];
	 uint32_t tmp = (d->Indirect << 24) | (d->ReadUpdate << 16) | (d->WriteUpdate << 8) | (d->SF << 0);

	 tmp = (tmp &~ mask) | (*DB & mask);

	 d->Indirect = (tmp >> 24) & 0x1;
	 d->ReadUpdate = (tmp >> 16) & 0x1;
	 d->WriteUpdate = (tmp >> 8) & 0x1;
	 d->SF = (tmp >> 0) & 0x7;
	}
	break;

   case 0x60:
	// TODO: Test
	if(*DB & mask & 0x1)
	{
	 SCU_DMA_ReadOverhead = 0;
	 for(unsigned level = 0; level < 3; level++)
	 {
	  DMALevelS* d = &DMALevel[level];

	  d->Active = false;
	  d->GoGoGadget = false;
	 }
	 RecalcDMAHalt();
	}
	break;

   case 0x80:
	SCU_UpdateDSP(SH7095_mem_timestamp);

	if(*DB & (1U << 25))	// Pause start
	 DSP.State |= STATE_MASK_PAUSE;
	else if(*DB & (1U << 26))	// Pause stop
	 DSP.State &= ~STATE_MASK_PAUSE;
	else
	{
	 if(*DB & (1U << 16))	// Execute
	  DSP.State |= STATE_MASK_EXECUTE;
	 else if(!(*DB & (1U << 16)))	// Execute stop
	 {
	  if(DSP.State & STATE_MASK_EXECUTE)
	  {
	   DSP.State &= ~STATE_MASK_EXECUTE;

	   if(DSP.CycleCounter < 0)
	    DSP.CycleCounter = 0;

	   if(DSP.T0_Until < 0)
	    DSP.T0_Until = 0;
	  }
	 }

	 if(MDFN_UNLIKELY(*DB & (1U << 17)))	// Step
	 {
	  if(DSP.State == 0)
	  {
	   ((void (*)(DSPS*))(DSP_INSTR_BASE_UIPT + (uintptr_t)(DSP_INSTR_RECOVER_TCAST)DSP.NextInstr))(&DSP);
	   if(DSP.CycleCounter < -(DSP_EndCCSubVal / 2))	// Ugh
	    DSP.CycleCounter += DSP_EndCCSubVal;
	  }
	 }
        }

	if(*DB & (1U << 15))	// PC load
	{
	 DSP.PC = *DB;
	 DSP.NextInstr = DSP_DecodeInstruction(0, false);
	 DSP.PRAMDMABufCount = 0;	// Kludgy~
	}

	SS_SetEventNT(&events[SS_EVENT_SCU_DSP], (DSPS_IsRunning(&DSP) ? SH7095_mem_timestamp + (DSP_UpdateTimingGran / 2) : SS_EVENT_DISABLED_TS));
	break;

   case 0x84:
	if(!DSPS_IsRunning(&DSP))
	 DSP.ProgRAM[DSP.PC++] = DSP_DecodeInstruction(*DB, false);
	break;

   case 0x88:
	DSP.RA = *DB;
	break;

   case 0x8C:
	if(!DSPS_IsRunning(&DSP))
	 ((uint32_t*)DSP.DataRAM)[DSP.RA++] = *DB;
	break;
  }
 
}

static INLINE void SCU_RegRW_DB_u16_W0(uint32_t A, uint32_t* DB)
{
  switch(A & 0xFC)
  {
   default:
	*DB = 0;
	break;

   case 0xA4:
	*DB = IPending;
	break;

   case 0xA8:
	*DB = 0; //ABusIAck;
	break;

   case 0xC4:
	*DB = RSEL;
	break;

   case 0xC8:
	*DB = 0x4;
	break;
   //
   //
   //
   case 0x00:
   case 0x20:
   case 0x40:
	{
	 const DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 *DB = d->StartReadAddr;
	}
	break;

   case 0x04:
   case 0x24:
   case 0x44:
	{
	 const DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 *DB = d->StartWriteAddr;
	}
	break;

   case 0x7C:
	{
	 uint32_t tmp = 0;

	 for(unsigned level = 0; level < 3; level++)
	 {
	  DMALevelS* d = &DMALevel[level];

	  if(d->Active)
	  {
	   tmp |= 0x10 << (level << 2);
	  }
	 }

	 if(DMALevel[0].Active && (DMALevel[1].Active || DMALevel[2].Active))
	  tmp |= 1U << 16;

	 if(DMALevel[1].Active && DMALevel[2].Active)
	  tmp |= 1U << 17;

	 *DB = tmp;
	}
	break;

   case 0x80:
	SS_SetEventNT(&events[SS_EVENT_SCU_DSP], SCU_UpdateDSP(SH7095_mem_timestamp));	// TODO: Remove?
	{
	 uint32_t tmp;

	 tmp = DSP.PC;
	 tmp |= (DSP.T0_Until < DSP.CycleCounter) << 23;
	 tmp |= DSP.FlagS << 22;
	 tmp |= DSP.FlagZ << 21;
	 tmp |= DSP.FlagC << 20;
	 tmp |= DSP.FlagV << 19;
	 tmp |= DSP.FlagEnd << 18;
	 tmp |= DSPS_IsRunning(&DSP) << 16;
	 *DB = tmp;
	 //
	 DSP.FlagV = false;
	 DSP.FlagEnd = false;
	 SCU_SetInt(SCU_INT_DSP, false);
	}
	break;

   case 0x8C:
	if(!DSPS_IsRunning(&DSP))
	 *DB = ((uint32_t*)DSP.DataRAM)[DSP.RA++];
	else
	 *DB = 0xFFFFFFFF;
	break;
  }
 
}

static INLINE void SCU_RegRW_DB_u16_W1(uint32_t A, uint32_t* DB)
{
 unsigned mask;

  mask = 0xFFFF << (((A & 2) ^ 2) << 3); 

  switch(A & 0xFC)
  {
   default:
	break;

   case 0x90:	// Timer 0 Compare
	{
	 VDP2_Update(SH7095_mem_timestamp);
	 Timer0_Compare = (Timer0_Compare &~ mask) | (*DB & mask & 0x3FF);
	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	}
	break;

   case 0x94:	// Timer 1 Reload Value
	{
	 VDP2_Update(SH7095_mem_timestamp);
	 Timer1_Reload = (Timer1_Reload &~ mask) | (*DB & mask & 0x1FF);
	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	}
	break;

   case 0x98:	// Timer Control
	{
	 VDP2_Update(SH7095_mem_timestamp);
	 uint32_t tmp = (Timer1_Mode << 8) | (Timer_Enable << 0);
	 tmp = (tmp &~ mask) | (*DB & mask);
	 Timer1_Mode = (tmp >> 8) & 1;
	 Timer_Enable = (tmp >> 0) & 1;

	 if(!Timer_Enable)
	 {
	  Timer0_Counter = 0;
	 }

	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	}
	break;

   case 0xA0:
	IMask = (IMask &~ mask) | (*DB & mask & 0xBFFF);

	if(CheckDoMasterInt())
	{
/*
	 if(MDFN_UNLIKELY(ss_horrible_hacks & HORRIBLEHACK_SCUINTDELAY))
	  SS_SetEventNT(&events[SS_EVENT_SCU_INT], SH7095_mem_timestamp + 8);
	 else
*/
	  SH7095_SetIRL(&CPU[0], ILevel);
	}
	break;

   case 0xA4:
	IPending &= *DB | ~mask;
	break;

   case 0xA8:
	if(*DB & mask & 0x0001)
	{
	 ABusIProhibit = 0; //&= ~IAsserted;
	 ABusIRQCheck();
	}
	break;

   case 0xB0:
	ASR0 = (ASR0 &~ mask) | (*DB & mask & 0xFFFDFFFD);
	break;

   case 0xB4:
	ASR1 = (ASR1 &~ mask) | (*DB & mask & 0xF00DFFFD);
	break;

   case 0xB8:
	AREF = (AREF &~ mask) | (*DB & mask & 0x1F);
	break;

   case 0xC4:
	RSEL = (RSEL &~ mask) | (*DB & mask & 0x1);
	break;

   case 0x00:
   case 0x20:
   case 0x40:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 d->StartReadAddr = (d->StartReadAddr &~ mask) | (*DB & mask & 0x07FFFFFF);
	}
	break;

   case 0x04:
   case 0x24:
   case 0x44:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 d->StartWriteAddr = (d->StartWriteAddr &~ mask) | (*DB & mask & 0x07FFFFFF);
	}
	break;

   case 0x08:
   case 0x28:
   case 0x48:
	{
	 const unsigned level = (A >> 5) & 0x3;
	 DMALevelS* d = &DMALevel[level];

	 d->StartByteCount = (d->StartByteCount &~ mask) | (*DB & mask & (level ? 0x00000FFF : 0x000FFFFF));
        }
	break;

   case 0x0C:
   case 0x2C:
   case 0x4C:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];
	 uint32_t tmp = (d->ReadAdd << 8) | (d->WriteAdd << 0);

	 tmp = (tmp &~ mask) | (*DB & mask);

	 d->ReadAdd = (tmp >> 8) & 0x1;
	 d->WriteAdd = (tmp >> 0) & 0x7;
	}
	break;

   case 0x10:
   case 0x30:
   case 0x50:
	{
	 const unsigned level = (A >> 5) & 0x3;
	 DMALevelS* d = &DMALevel[level];
	 uint32_t tmp = (d->Enable << 8);

	 tmp = (tmp &~ mask) | (*DB & mask);	 
	 d->Enable = (tmp >> 8) & 0x1;

	 if((tmp & 0x1) && d->Enable && d->SF == 0x7)
	 {
	  SCU_UpdateDMA(SH7095_mem_timestamp);

	  d->GoGoGadget = true;
	  CheckDMAStart(d);

	  SS_SetEventNT(&events[SS_EVENT_SCU_DMA], SCU_UpdateDMA(SH7095_mem_timestamp));
	 }
	}
	break;

   case 0x14:
   case 0x34:
   case 0x54:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];
	 uint32_t tmp = (d->Indirect << 24) | (d->ReadUpdate << 16) | (d->WriteUpdate << 8) | (d->SF << 0);

	 tmp = (tmp &~ mask) | (*DB & mask);

	 d->Indirect = (tmp >> 24) & 0x1;
	 d->ReadUpdate = (tmp >> 16) & 0x1;
	 d->WriteUpdate = (tmp >> 8) & 0x1;
	 d->SF = (tmp >> 0) & 0x7;
	}
	break;

   case 0x60:
	// TODO: Test
	if(*DB & mask & 0x1)
	{
	 SCU_DMA_ReadOverhead = 0;
	 for(unsigned level = 0; level < 3; level++)
	 {
	  DMALevelS* d = &DMALevel[level];

	  d->Active = false;
	  d->GoGoGadget = false;
	 }
	 RecalcDMAHalt();
	}
	break;

   case 0x80:
	SCU_UpdateDSP(SH7095_mem_timestamp);

	if(*DB & (1U << 25))	// Pause start
	 DSP.State |= STATE_MASK_PAUSE;
	else if(*DB & (1U << 26))	// Pause stop
	 DSP.State &= ~STATE_MASK_PAUSE;
	else
	{
	 if(*DB & (1U << 16))	// Execute
	  DSP.State |= STATE_MASK_EXECUTE;
	 else if(!(*DB & (1U << 16)))	// Execute stop
	 {
	  if(DSP.State & STATE_MASK_EXECUTE)
	  {
	   DSP.State &= ~STATE_MASK_EXECUTE;

	   if(DSP.CycleCounter < 0)
	    DSP.CycleCounter = 0;

	   if(DSP.T0_Until < 0)
	    DSP.T0_Until = 0;
	  }
	 }

	 if(MDFN_UNLIKELY(*DB & (1U << 17)))	// Step
	 {
	  if(DSP.State == 0)
	  {
	   ((void (*)(DSPS*))(DSP_INSTR_BASE_UIPT + (uintptr_t)(DSP_INSTR_RECOVER_TCAST)DSP.NextInstr))(&DSP);
	   if(DSP.CycleCounter < -(DSP_EndCCSubVal / 2))	// Ugh
	    DSP.CycleCounter += DSP_EndCCSubVal;
	  }
	 }
        }

	if(*DB & (1U << 15))	// PC load
	{
	 DSP.PC = *DB;
	 DSP.NextInstr = DSP_DecodeInstruction(0, false);
	 DSP.PRAMDMABufCount = 0;	// Kludgy~
	}

	SS_SetEventNT(&events[SS_EVENT_SCU_DSP], (DSPS_IsRunning(&DSP) ? SH7095_mem_timestamp + (DSP_UpdateTimingGran / 2) : SS_EVENT_DISABLED_TS));
	break;

   case 0x84:
	if(!DSPS_IsRunning(&DSP))
	 DSP.ProgRAM[DSP.PC++] = DSP_DecodeInstruction(*DB, false);
	break;

   case 0x88:
	DSP.RA = *DB;
	break;

   case 0x8C:
	if(!DSPS_IsRunning(&DSP))
	 ((uint32_t*)DSP.DataRAM)[DSP.RA++] = *DB;
	break;
  }
 
}

static INLINE void SCU_RegRW_DB_u32_W0(uint32_t A, uint32_t* DB)
{
  switch(A & 0xFC)
  {
   default:
	*DB = 0;
	break;

   case 0xA4:
	*DB = IPending;
	break;

   case 0xA8:
	*DB = 0; //ABusIAck;
	break;

   case 0xC4:
	*DB = RSEL;
	break;

   case 0xC8:
	*DB = 0x4;
	break;
   //
   //
   //
   case 0x00:
   case 0x20:
   case 0x40:
	{
	 const DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 *DB = d->StartReadAddr;
	}
	break;

   case 0x04:
   case 0x24:
   case 0x44:
	{
	 const DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 *DB = d->StartWriteAddr;
	}
	break;

   case 0x7C:
	{
	 uint32_t tmp = 0;

	 for(unsigned level = 0; level < 3; level++)
	 {
	  DMALevelS* d = &DMALevel[level];

	  if(d->Active)
	  {
	   tmp |= 0x10 << (level << 2);
	  }
	 }

	 if(DMALevel[0].Active && (DMALevel[1].Active || DMALevel[2].Active))
	  tmp |= 1U << 16;

	 if(DMALevel[1].Active && DMALevel[2].Active)
	  tmp |= 1U << 17;

	 *DB = tmp;
	}
	break;

   case 0x80:
	SS_SetEventNT(&events[SS_EVENT_SCU_DSP], SCU_UpdateDSP(SH7095_mem_timestamp));	// TODO: Remove?
	{
	 uint32_t tmp;

	 tmp = DSP.PC;
	 tmp |= (DSP.T0_Until < DSP.CycleCounter) << 23;
	 tmp |= DSP.FlagS << 22;
	 tmp |= DSP.FlagZ << 21;
	 tmp |= DSP.FlagC << 20;
	 tmp |= DSP.FlagV << 19;
	 tmp |= DSP.FlagEnd << 18;
	 tmp |= DSPS_IsRunning(&DSP) << 16;
	 *DB = tmp;
	 //
	 DSP.FlagV = false;
	 DSP.FlagEnd = false;
	 SCU_SetInt(SCU_INT_DSP, false);
	}
	break;

   case 0x8C:
	if(!DSPS_IsRunning(&DSP))
	 *DB = ((uint32_t*)DSP.DataRAM)[DSP.RA++];
	else
	 *DB = 0xFFFFFFFF;
	break;
  }
 
}

static INLINE void SCU_RegRW_DB_u32_W1(uint32_t A, uint32_t* DB)
{
 unsigned mask;

  mask = 0xFFFFFFFF; 

  switch(A & 0xFC)
  {
   default:
	break;

   case 0x90:	// Timer 0 Compare
	{
	 VDP2_Update(SH7095_mem_timestamp);
	 Timer0_Compare = (Timer0_Compare &~ mask) | (*DB & mask & 0x3FF);
	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	}
	break;

   case 0x94:	// Timer 1 Reload Value
	{
	 VDP2_Update(SH7095_mem_timestamp);
	 Timer1_Reload = (Timer1_Reload &~ mask) | (*DB & mask & 0x1FF);
	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	}
	break;

   case 0x98:	// Timer Control
	{
	 VDP2_Update(SH7095_mem_timestamp);
	 uint32_t tmp = (Timer1_Mode << 8) | (Timer_Enable << 0);
	 tmp = (tmp &~ mask) | (*DB & mask);
	 Timer1_Mode = (tmp >> 8) & 1;
	 Timer_Enable = (tmp >> 0) & 1;

	 if(!Timer_Enable)
	 {
	  Timer0_Counter = 0;
	 }

	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	}
	break;

   case 0xA0:
	IMask = (IMask &~ mask) | (*DB & mask & 0xBFFF);

	if(CheckDoMasterInt())
	{
/*
	 if(MDFN_UNLIKELY(ss_horrible_hacks & HORRIBLEHACK_SCUINTDELAY))
	  SS_SetEventNT(&events[SS_EVENT_SCU_INT], SH7095_mem_timestamp + 8);
	 else
*/
	  SH7095_SetIRL(&CPU[0], ILevel);
	}
	break;

   case 0xA4:
	IPending &= *DB | ~mask;
	break;

   case 0xA8:
	if(*DB & mask & 0x0001)
	{
	 ABusIProhibit = 0; //&= ~IAsserted;
	 ABusIRQCheck();
	}
	break;

   case 0xB0:
	ASR0 = (ASR0 &~ mask) | (*DB & mask & 0xFFFDFFFD);
	break;

   case 0xB4:
	ASR1 = (ASR1 &~ mask) | (*DB & mask & 0xF00DFFFD);
	break;

   case 0xB8:
	AREF = (AREF &~ mask) | (*DB & mask & 0x1F);
	break;

   case 0xC4:
	RSEL = (RSEL &~ mask) | (*DB & mask & 0x1);
	break;

   case 0x00:
   case 0x20:
   case 0x40:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 d->StartReadAddr = (d->StartReadAddr &~ mask) | (*DB & mask & 0x07FFFFFF);
	}
	break;

   case 0x04:
   case 0x24:
   case 0x44:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];

	 d->StartWriteAddr = (d->StartWriteAddr &~ mask) | (*DB & mask & 0x07FFFFFF);
	}
	break;

   case 0x08:
   case 0x28:
   case 0x48:
	{
	 const unsigned level = (A >> 5) & 0x3;
	 DMALevelS* d = &DMALevel[level];

	 d->StartByteCount = (d->StartByteCount &~ mask) | (*DB & mask & (level ? 0x00000FFF : 0x000FFFFF));
        }
	break;

   case 0x0C:
   case 0x2C:
   case 0x4C:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];
	 uint32_t tmp = (d->ReadAdd << 8) | (d->WriteAdd << 0);

	 tmp = (tmp &~ mask) | (*DB & mask);

	 d->ReadAdd = (tmp >> 8) & 0x1;
	 d->WriteAdd = (tmp >> 0) & 0x7;
	}
	break;

   case 0x10:
   case 0x30:
   case 0x50:
	{
	 const unsigned level = (A >> 5) & 0x3;
	 DMALevelS* d = &DMALevel[level];
	 uint32_t tmp = (d->Enable << 8);

	 tmp = (tmp &~ mask) | (*DB & mask);	 
	 d->Enable = (tmp >> 8) & 0x1;

	 if((tmp & 0x1) && d->Enable && d->SF == 0x7)
	 {
	  SCU_UpdateDMA(SH7095_mem_timestamp);

	  d->GoGoGadget = true;
	  CheckDMAStart(d);

	  SS_SetEventNT(&events[SS_EVENT_SCU_DMA], SCU_UpdateDMA(SH7095_mem_timestamp));
	 }
	}
	break;

   case 0x14:
   case 0x34:
   case 0x54:
	{
	 DMALevelS* d = &DMALevel[(A >> 5) & 0x3];
	 uint32_t tmp = (d->Indirect << 24) | (d->ReadUpdate << 16) | (d->WriteUpdate << 8) | (d->SF << 0);

	 tmp = (tmp &~ mask) | (*DB & mask);

	 d->Indirect = (tmp >> 24) & 0x1;
	 d->ReadUpdate = (tmp >> 16) & 0x1;
	 d->WriteUpdate = (tmp >> 8) & 0x1;
	 d->SF = (tmp >> 0) & 0x7;
	}
	break;

   case 0x60:
	// TODO: Test
	if(*DB & mask & 0x1)
	{
	 SCU_DMA_ReadOverhead = 0;
	 for(unsigned level = 0; level < 3; level++)
	 {
	  DMALevelS* d = &DMALevel[level];

	  d->Active = false;
	  d->GoGoGadget = false;
	 }
	 RecalcDMAHalt();
	}
	break;

   case 0x80:
	SCU_UpdateDSP(SH7095_mem_timestamp);

	if(*DB & (1U << 25))	// Pause start
	 DSP.State |= STATE_MASK_PAUSE;
	else if(*DB & (1U << 26))	// Pause stop
	 DSP.State &= ~STATE_MASK_PAUSE;
	else
	{
	 if(*DB & (1U << 16))	// Execute
	  DSP.State |= STATE_MASK_EXECUTE;
	 else if(!(*DB & (1U << 16)))	// Execute stop
	 {
	  if(DSP.State & STATE_MASK_EXECUTE)
	  {
	   DSP.State &= ~STATE_MASK_EXECUTE;

	   if(DSP.CycleCounter < 0)
	    DSP.CycleCounter = 0;

	   if(DSP.T0_Until < 0)
	    DSP.T0_Until = 0;
	  }
	 }

	 if(MDFN_UNLIKELY(*DB & (1U << 17)))	// Step
	 {
	  if(DSP.State == 0)
	  {
	   ((void (*)(DSPS*))(DSP_INSTR_BASE_UIPT + (uintptr_t)(DSP_INSTR_RECOVER_TCAST)DSP.NextInstr))(&DSP);
	   if(DSP.CycleCounter < -(DSP_EndCCSubVal / 2))	// Ugh
	    DSP.CycleCounter += DSP_EndCCSubVal;
	  }
	 }
        }

	if(*DB & (1U << 15))	// PC load
	{
	 DSP.PC = *DB;
	 DSP.NextInstr = DSP_DecodeInstruction(0, false);
	 DSP.PRAMDMABufCount = 0;	// Kludgy~
	}

	SS_SetEventNT(&events[SS_EVENT_SCU_DSP], (DSPS_IsRunning(&DSP) ? SH7095_mem_timestamp + (DSP_UpdateTimingGran / 2) : SS_EVENT_DISABLED_TS));
	break;

   case 0x84:
	if(!DSPS_IsRunning(&DSP))
	 DSP.ProgRAM[DSP.PC++] = DSP_DecodeInstruction(*DB, false);
	break;

   case 0x88:
	DSP.RA = *DB;
	break;

   case 0x8C:
	if(!DSPS_IsRunning(&DSP))
	 ((uint32_t*)DSP.DataRAM)[DSP.RA++] = *DB;
	break;
  }
 
}

/* Phase-8q1: BBusRW_DB / ABusRW_DB retired into named
 * per-(T, IsWrite, SH32) variants.  Only the tuples actually
 * invoked via the call graph (concrete and T-parametric
 * callers) are emitted -- the impossible BBusRW_DB read
 * variants with T != uint16_t are never generated.
 *
 * Each variant body is source-folded for its tuple via
 * tools/fold_source.py (phase 8q1-fold-fixup): template
 * parameters are substituted to literals, dead branches
 * eliminated, and `static_assert(...)`s evaluated and
 * removed.  Each variant carries ONLY the code that runs
 * for its tuple -- no constexpr-shadow indirection. */

/* === BBusRW_DB variants (6 used tuples) === */
static INLINE void BBusRW_DB_u8_W1_SH0(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 

 if(time_thing != NULL)
 {
  
   *time_thing = ((sscpu_timestamp_t)(*time_thing) > (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS) ? (sscpu_timestamp_t)(*time_thing) : (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS));

    *time_thing += 2;
    BBus_SH2_WriteFinishTS = *time_thing;

 }

 //
 // VDP1
 //
 if(A >= 0x05C00000 && A <= 0x05D7FFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  ( 6) ;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;

    if(MDFN_UNLIKELY(A >= 0x05D00000))
    {
     const bool ignore_write = (A >= 0x5D00004 && SCU_DMA_VDP1WriteIgnoreKludge > 0) | (SCU_DMA_VDP1WriteIgnoreKludge & 0x1);

     SCU_DMA_VDP1WriteIgnoreKludge++;
     if(ignore_write)
      return;
    }
    else
     SCU_DMA_VDP1WriteIgnoreKludge = 0;
   
  }

  //if(IsWrite && (A & 0x100000) && SH32)
  //{
  // assert(0);
  //}

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  9;

   CheckEventsByMemTS();
  }

   if(time_thing != NULL)
    VDP1_Write_CheckDrawSlowdown(A, *time_thing);
   else if(sh2_dma_time_thing != NULL)
    VDP1_Write_CheckDrawSlowdown(A, *sh2_dma_time_thing);

   VDP1_Write8_DB(A, *DB);

  return;
 }

 //
 // VDP2
 //
 if(A >= 0x05E00000 && A <= 0x05FBFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  ( 5) ;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;
   //if(A & 0x00100000)
   // *dma_time_thing -= 1;
  }

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  3;

   CheckEventsByMemTS();
  }

   uint32_t expenalty;

   expenalty = VDP2_Write8_DB(A, *DB);

   if(dma_time_thing != NULL)
    *dma_time_thing -= expenalty;

  return;
 }

 //
 // SCSP
 // 
 if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 13;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 13;
  }

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  17;
  }

   SOUND_Write8(A & 0x1FFFFF, *DB >> (((A & 1) ^ 1) << 3));

  return;
 }
 //
 //
 //
 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;

}

static INLINE void BBusRW_DB_u16_W1_SH0(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 

 if(time_thing != NULL)
 {
  
   *time_thing = ((sscpu_timestamp_t)(*time_thing) > (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS) ? (sscpu_timestamp_t)(*time_thing) : (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS));

    *time_thing += 2;
    BBus_SH2_WriteFinishTS = *time_thing;

 }

 //
 // VDP1
 //
 if(A >= 0x05C00000 && A <= 0x05D7FFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  ( 6) ;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;

    if(MDFN_UNLIKELY(A >= 0x05D00000))
    {
     const bool ignore_write = (A >= 0x5D00004 && SCU_DMA_VDP1WriteIgnoreKludge > 0) | (SCU_DMA_VDP1WriteIgnoreKludge & 0x1);

     SCU_DMA_VDP1WriteIgnoreKludge++;
     if(ignore_write)
      return;
    }
    else
     SCU_DMA_VDP1WriteIgnoreKludge = 0;
   
  }

  //if(IsWrite && (A & 0x100000) && SH32)
  //{
  // assert(0);
  //}

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  9;

   CheckEventsByMemTS();
  }

   if(time_thing != NULL)
    VDP1_Write_CheckDrawSlowdown(A, *time_thing);
   else if(sh2_dma_time_thing != NULL)
    VDP1_Write_CheckDrawSlowdown(A, *sh2_dma_time_thing);

   VDP1_Write16_DB(A, *DB);

  return;
 }

 //
 // VDP2
 //
 if(A >= 0x05E00000 && A <= 0x05FBFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  ( 5) ;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;
   //if(A & 0x00100000)
   // *dma_time_thing -= 1;
  }

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  3;

   CheckEventsByMemTS();
  }

   uint32_t expenalty;

   expenalty = VDP2_Write16_DB(A, *DB);

   if(dma_time_thing != NULL)
    *dma_time_thing -= expenalty;

  return;
 }

 //
 // SCSP
 // 
 if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 13;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 13;
  }

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  17;
  }

   SOUND_Write16(A & 0x1FFFFF, *DB);

  return;
 }
 //
 //
 //
 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;

}

static INLINE void BBusRW_DB_u16_W1_SH1(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 

 if(time_thing != NULL)
 {
  
 }

 //
 // VDP1
 //
 if(A >= 0x05C00000 && A <= 0x05D7FFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  ( 0 ) ;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;

    if(MDFN_UNLIKELY(A >= 0x05D00000))
    {
     const bool ignore_write = (A >= 0x5D00004 && SCU_DMA_VDP1WriteIgnoreKludge > 0) | (SCU_DMA_VDP1WriteIgnoreKludge & 0x1);

     SCU_DMA_VDP1WriteIgnoreKludge++;
     if(ignore_write)
      return;
    }
    else
     SCU_DMA_VDP1WriteIgnoreKludge = 0;
   
  }

  //if(IsWrite && (A & 0x100000) && SH32)
  //{
  // assert(0);
  //}

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  ((A & 0x100000) ? 0 : 1) ;

   CheckEventsByMemTS();
  }

   if(time_thing != NULL)
    VDP1_Write_CheckDrawSlowdown(A, *time_thing);
   else if(sh2_dma_time_thing != NULL)
    VDP1_Write_CheckDrawSlowdown(A, *sh2_dma_time_thing);

   VDP1_Write16_DB(A, *DB);

  return;
 }

 //
 // VDP2
 //
 if(A >= 0x05E00000 && A <= 0x05FBFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  ( 0 ) ;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;
   //if(A & 0x00100000)
   // *dma_time_thing -= 1;
  }

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  1 ;

   CheckEventsByMemTS();
  }

   uint32_t expenalty;

   expenalty = VDP2_Write16_DB(A, *DB);

   if(dma_time_thing != NULL)
    *dma_time_thing -= expenalty;

  return;
 }

 //
 // SCSP
 // 
 if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 13;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 13;
  }

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  13 ;
  }

   SOUND_Write16(A & 0x1FFFFF, *DB);

  return;
 }
 //
 //
 //
 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;

}

static INLINE void BBusRW_DB_u32_W1_SH0(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 

 if(time_thing != NULL)
 {
  
   *time_thing = ((sscpu_timestamp_t)(*time_thing) > (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS) ? (sscpu_timestamp_t)(*time_thing) : (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS));

    *time_thing += 2;
    BBus_SH2_WriteFinishTS = *time_thing;

 }

 //
 // VDP1
 //
 if(A >= 0x05C00000 && A <= 0x05D7FFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  ( 6) ;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;

    if(MDFN_UNLIKELY(A >= 0x05D00000))
    {
     const bool ignore_write = (A >= 0x5D00004 && SCU_DMA_VDP1WriteIgnoreKludge > 0) | (SCU_DMA_VDP1WriteIgnoreKludge & 0x1);

     SCU_DMA_VDP1WriteIgnoreKludge++;
     if(ignore_write)
      return;
    }
    else
     SCU_DMA_VDP1WriteIgnoreKludge = 0;
   
  }

  //if(IsWrite && (A & 0x100000) && SH32)
  //{
  // assert(0);
  //}

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  9;

   CheckEventsByMemTS();
  }

   if(time_thing != NULL)
    VDP1_Write_CheckDrawSlowdown(A, *time_thing);
   else if(sh2_dma_time_thing != NULL)
    VDP1_Write_CheckDrawSlowdown(A, *sh2_dma_time_thing);

   VDP1_Write16_DB(A, *DB);

  return;
 }

 //
 // VDP2
 //
 if(A >= 0x05E00000 && A <= 0x05FBFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  ( 5) ;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;
   //if(A & 0x00100000)
   // *dma_time_thing -= 1;
  }

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  3;

   CheckEventsByMemTS();
  }

   uint32_t expenalty;

   expenalty = VDP2_Write16_DB(A, *DB);

   if(dma_time_thing != NULL)
    *dma_time_thing -= expenalty;

  return;
 }

 //
 // SCSP
 // 
 if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 13;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 13;
  }

  if(time_thing != NULL)
  {
   BBus_SH2_WriteFinishTS +=  17;
  }

   SOUND_Write16(A & 0x1FFFFF, *DB);

  return;
 }
 //
 //
 //
 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;

}

static INLINE void BBusRW_DB_u16_W0_SH0(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 

 if(time_thing != NULL)
 {
  
   *time_thing = ((sscpu_timestamp_t)(*time_thing) > (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS) ? (sscpu_timestamp_t)(*time_thing) : (sscpu_timestamp_t)(BBus_SH2_WriteFinishTS));

 }

 //
 // VDP1
 //
 if(A >= 0x05C00000 && A <= 0x05D7FFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  10;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;

  }

  //if(IsWrite && (A & 0x100000) && SH32)
  //{
  // assert(0);
  //}

  if(time_thing != NULL)
  {
   
    *time_thing += 14;

   CheckEventsByMemTS();
  }

   if(time_thing != NULL)
    VDP1_Read_CheckDrawSlowdown(A, *time_thing);
   else if(sh2_dma_time_thing != NULL)
    VDP1_Read_CheckDrawSlowdown(A, *sh2_dma_time_thing);

   *DB = VDP1_Read16_DB(A);

  return;
 }

 //
 // VDP2
 //
 if(A >= 0x05E00000 && A <= 0x05FBFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  10;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;
   //if(A & 0x00100000)
   // *dma_time_thing -= 1;
  }

  if(time_thing != NULL)
  {
   *time_thing += 20;

   CheckEventsByMemTS();
  }

   *DB = VDP2_Read16_DB(A);

  return;
 }

 //
 // SCSP
 // 
 if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 13;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 13;
  }

  if(time_thing != NULL)
  {
   *time_thing += 24;
  }

  *DB = SOUND_Read16(A & 0x1FFFFF); 

  return;
 }
 //
 //
 //
 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;

 *DB = 0;
}

static INLINE void BBusRW_DB_u16_W0_SH1(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 

 if(time_thing != NULL)
 {
  
 }

 //
 // VDP1
 //
 if(A >= 0x05C00000 && A <= 0x05D7FFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  10;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;

  }

  //if(IsWrite && (A & 0x100000) && SH32)
  //{
  // assert(0);
  //}

  if(time_thing != NULL)
  {
   
    *time_thing += 14;

   CheckEventsByMemTS();
  }

   if(time_thing != NULL)
    VDP1_Read_CheckDrawSlowdown(A, *time_thing);
   else if(sh2_dma_time_thing != NULL)
    VDP1_Read_CheckDrawSlowdown(A, *sh2_dma_time_thing);

   *DB = VDP1_Read16_DB(A);

  return;
 }

 //
 // VDP2
 //
 if(A >= 0x05E00000 && A <= 0x05FBFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing +=  10;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 1;
   //if(A & 0x00100000)
   // *dma_time_thing -= 1;
  }

  if(time_thing != NULL)
  {
   *time_thing += 20;

   CheckEventsByMemTS();
  }

   *DB = VDP2_Read16_DB(A);

  return;
 }

 //
 // SCSP
 // 
 if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 13;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 13;
  }

  if(time_thing != NULL)
  {
   *time_thing += 24;
  }

  *DB = SOUND_Read16(A & 0x1FFFFF); 

  return;
 }
 //
 //
 //
 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;

 *DB = 0;
}


/* === ABusRW_DB variants (5 used tuples) === */
static INLINE void ABusRW_DB_u8_W1_SH0(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 //
 // A-Bus CS0 and CS1
 //
 if(A >= 0x02000000 && A <= 0x04FFFFFF)
 {
  {
   const unsigned buscfg = ASR0 >> ((A & 0x04000000) ? 0 : 16);

   if(time_thing != NULL)
   {
    if(false && ((buscfg >> 15) & 0x1))
    {
     // TODO/FIXME
     *time_thing += 2;
    }
    else
    {
     if(true || !((buscfg >> 2) & 0x3))
      *time_thing += 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + true)) & 1);
     else
      *time_thing += 2 + ((buscfg >> 8) & 0xF);
    }
   }

   // TODO: SCU seems to have its own internal write buffering...or something else complex going on, that complicates
   // getting the SH-2 DMA timing right. 
   if(sh2_dma_time_thing != NULL)
    *sh2_dma_time_thing += 1;

   if(dma_time_thing != NULL)
   {
    if((buscfg >> 2) & 0x3)
     *dma_time_thing -= 2 + ((buscfg >> 8) & 0xF);
    else
     *dma_time_thing -= 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + true)) & 1);
   }
  }

   CART_CS01_Write8_DB(A, DB);

  return;
 }

 //
 // A-bus Dummy
 //
 if(MDFN_UNLIKELY(A >= 0x05000000 && A <= 0x057FFFFF))
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 16;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 16;
  }

  return;
 }

 //
 // A-Bus CS2
 //
 if(A >= 0x05800000 && A <= 0x058FFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 8;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 8;
  }

  if(time_thing)
  {
   *time_thing += 8;
  }

  if((A & 0x7FFF) < 0x1000)
  {
   const uint32_t offset = (A & 0x3F) >> 2;
   const uint32_t mask = (false) ? 0xFFFF : (0xFF << (((A & 1) ^ 1) << 3));

    CDB_Write_DBM(offset, *DB, mask);
   
   return;
  }

   CART_CS2_Write8_DB(A, DB);

  return;
 }

 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;
}

static INLINE void ABusRW_DB_u16_W1_SH0(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 //
 // A-Bus CS0 and CS1
 //
 if(A >= 0x02000000 && A <= 0x04FFFFFF)
 {
  {
   const unsigned buscfg = ASR0 >> ((A & 0x04000000) ? 0 : 16);

   if(time_thing != NULL)
   {
    if(false && ((buscfg >> 15) & 0x1))
    {
     // TODO/FIXME
     *time_thing += 2;
    }
    else
    {
     if(true || !((buscfg >> 2) & 0x3))
      *time_thing += 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + true)) & 1);
     else
      *time_thing += 2 + ((buscfg >> 8) & 0xF);
    }
   }

   // TODO: SCU seems to have its own internal write buffering...or something else complex going on, that complicates
   // getting the SH-2 DMA timing right. 
   if(sh2_dma_time_thing != NULL)
    *sh2_dma_time_thing += 1;

   if(dma_time_thing != NULL)
   {
    if((buscfg >> 2) & 0x3)
     *dma_time_thing -= 2 + ((buscfg >> 8) & 0xF);
    else
     *dma_time_thing -= 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + true)) & 1);
   }
  }

   CART_CS01_Write16_DB(A, DB);

  return;
 }

 //
 // A-bus Dummy
 //
 if(MDFN_UNLIKELY(A >= 0x05000000 && A <= 0x057FFFFF))
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 16;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 16;
  }

  return;
 }

 //
 // A-Bus CS2
 //
 if(A >= 0x05800000 && A <= 0x058FFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 8;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 8;
  }

  if(time_thing)
  {
   *time_thing += 8;
  }

  if((A & 0x7FFF) < 0x1000)
  {
   const uint32_t offset = (A & 0x3F) >> 2;
   const uint32_t mask = (true) ? 0xFFFF : (0xFF << (((A & 1) ^ 1) << 3));

    CDB_Write_DBM(offset, *DB, mask);
   
   return;
  }

   CART_CS2_Write16_DB(A, DB);

  return;
 }

 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;
}

static INLINE void ABusRW_DB_u16_W1_SH1(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 //
 // A-Bus CS0 and CS1
 //
 if(A >= 0x02000000 && A <= 0x04FFFFFF)
 {
  {
   const unsigned buscfg = ASR0 >> ((A & 0x04000000) ? 0 : 16);

   if(time_thing != NULL)
   {
    if(false && ((buscfg >> 15) & 0x1))
    {
     // TODO/FIXME
     *time_thing += 2;
    }
    else
    {
     if(false || !((buscfg >> 2) & 0x3))
      *time_thing += 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + true)) & 1);
     else
      *time_thing += 2 + ((buscfg >> 8) & 0xF);
    }
   }

   // TODO: SCU seems to have its own internal write buffering...or something else complex going on, that complicates
   // getting the SH-2 DMA timing right. 
   if(sh2_dma_time_thing != NULL)
    *sh2_dma_time_thing += 1;

   if(dma_time_thing != NULL)
   {
    if((buscfg >> 2) & 0x3)
     *dma_time_thing -= 2 + ((buscfg >> 8) & 0xF);
    else
     *dma_time_thing -= 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + true)) & 1);
   }
  }

   CART_CS01_Write16_DB(A, DB);

  return;
 }

 //
 // A-bus Dummy
 //
 if(MDFN_UNLIKELY(A >= 0x05000000 && A <= 0x057FFFFF))
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 16;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 16;
  }

  return;
 }

 //
 // A-Bus CS2
 //
 if(A >= 0x05800000 && A <= 0x058FFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 8;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 8;
  }

  if(time_thing)
  {
   *time_thing += 8;
  }

  if((A & 0x7FFF) < 0x1000)
  {
   const uint32_t offset = (A & 0x3F) >> 2;
   const uint32_t mask = (true) ? 0xFFFF : (0xFF << (((A & 1) ^ 1) << 3));

    CDB_Write_DBM(offset, *DB, mask);
   
   return;
  }

   CART_CS2_Write16_DB(A, DB);

  return;
 }

 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;
}

static INLINE void ABusRW_DB_u16_W0_SH0(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 //
 // A-Bus CS0 and CS1
 //
 if(A >= 0x02000000 && A <= 0x04FFFFFF)
 {
  {
   const unsigned buscfg = ASR0 >> ((A & 0x04000000) ? 0 : 16);

   if(time_thing != NULL)
   {
    if(true && ((buscfg >> 15) & 0x1))
    {
     // TODO/FIXME
     *time_thing += 2;
    }
    else
    {
     if(true || !((buscfg >> 2) & 0x3))
      *time_thing += 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + false)) & 1);
     else
      *time_thing += 2 + ((buscfg >> 8) & 0xF);
    }
   }

   // TODO: SCU seems to have its own internal write buffering...or something else complex going on, that complicates
   // getting the SH-2 DMA timing right. 
   if(sh2_dma_time_thing != NULL)
    *sh2_dma_time_thing += 1;

   if(dma_time_thing != NULL)
   {
    if((buscfg >> 2) & 0x3)
     *dma_time_thing -= 2 + ((buscfg >> 8) & 0xF);
    else
     *dma_time_thing -= 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + false)) & 1);
   }
  }

  CART_CS01_Read16_DB(A, DB);

  return;
 }

 //
 // A-bus Dummy
 //
 if(MDFN_UNLIKELY(A >= 0x05000000 && A <= 0x057FFFFF))
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 16;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 16;
  }

  return;
 }

 //
 // A-Bus CS2
 //
 if(A >= 0x05800000 && A <= 0x058FFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 8;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 8;
  }

  if(time_thing)
  {
   *time_thing += 8;
  }

  if((A & 0x7FFF) < 0x1000)
  {
   const uint32_t offset = (A & 0x3F) >> 2;

    if(true || !(A & 0x80000)) // CD block seems to effectively ignore second read access in 32-bit reads somehow, tested to occur HIRQ and the FIFO at least...
     *DB = CDB_Read(offset);
   
   return;
  }

  CART_CS2_Read16_DB(A, DB);

  return;
 }

 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;
}

static INLINE void ABusRW_DB_u16_W0_SH1(uint32_t A, uint16_t* DB, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)	// add to time_thing, subtract from dma_time_thing
{
 //
 // A-Bus CS0 and CS1
 //
 if(A >= 0x02000000 && A <= 0x04FFFFFF)
 {
  {
   const unsigned buscfg = ASR0 >> ((A & 0x04000000) ? 0 : 16);

   if(time_thing != NULL)
   {
    if(true && ((buscfg >> 15) & 0x1))
    {
     // TODO/FIXME
     *time_thing += 2;
    }
    else
    {
     if(false || !((buscfg >> 2) & 0x3))
      *time_thing += 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + false)) & 1);
     else
      *time_thing += 2 + ((buscfg >> 8) & 0xF);
    }
   }

   // TODO: SCU seems to have its own internal write buffering...or something else complex going on, that complicates
   // getting the SH-2 DMA timing right. 
   if(sh2_dma_time_thing != NULL)
    *sh2_dma_time_thing += 1;

   if(dma_time_thing != NULL)
   {
    if((buscfg >> 2) & 0x3)
     *dma_time_thing -= 2 + ((buscfg >> 8) & 0xF);
    else
     *dma_time_thing -= 5 + ((buscfg >> 4) & 0xF) + ((buscfg >> (13 + false)) & 1);
   }
  }

  CART_CS01_Read16_DB(A, DB);

  return;
 }

 //
 // A-bus Dummy
 //
 if(MDFN_UNLIKELY(A >= 0x05000000 && A <= 0x057FFFFF))
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 16;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 16;
  }

  return;
 }

 //
 // A-Bus CS2
 //
 if(A >= 0x05800000 && A <= 0x058FFFFF)
 {
  if(sh2_dma_time_thing != NULL)
   *sh2_dma_time_thing += 8;

  if(dma_time_thing != NULL)
  {
   *dma_time_thing -= 8;
  }

  if(time_thing)
  {
   *time_thing += 8;
  }

  if((A & 0x7FFF) < 0x1000)
  {
   const uint32_t offset = (A & 0x3F) >> 2;

    if(false || !(A & 0x80000)) // CD block seems to effectively ignore second read access in 32-bit reads somehow, tested to occur HIRQ and the FIFO at least...
     *DB = CDB_Read(offset);
   
   return;
  }

  CART_CS2_Read16_DB(A, DB);

  return;
 }

 if(sh2_dma_time_thing != NULL)
  *sh2_dma_time_thing += 1;

 if(dma_time_thing != NULL)
  *dma_time_thing -= 1;
}


/* Phase-8k: ABus_Write_DB32<T> retired -- 3 named static
 * functions cover the three width slices the template was
 * instantiated for (uint8_t, uint16_t, uint32_t).  The u32
 * variant carries the 32-bit-into-two-16-bit-accesses path
 * the template's `if(sizeof(T) == 4)` branch held; the u8
 * and u16 variants take the single-access "else" branch
 * with the byte / halfword shift folded against their
 * respective widths (DB32 >> 24 for u8, DB32 >> 16 or 0
 * for u16 depending on A & 2).
 *
 * Caller sites pick the right named variant either via a
 * direct concrete call (the dsp T0 burst path, line ~1961)
 * or via a sizeof(T) ladder when called from inside the
 * SCU_FromSH2_BusRW_DB / DMA_Write templates -- T is a
 * template-time constant in those callers so the ladder
 * folds away under -O2.  ABusRW_DB<T, true> calls inside
 * the u8/u16 bodies are now concrete-T template
 * instantiations (uint8_t / uint16_t respectively).
 */
static INLINE void ABus_Write_DB32_u8(uint32_t A, uint32_t DB32, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)
{
 uint16_t tmp = DB32 >> (((A & 2) ^ 2) << 3);

 ABusRW_DB_u8_W1_SH0(A, &tmp, time_thing, dma_time_thing, sh2_dma_time_thing);
}

static INLINE void ABus_Write_DB32_u16(uint32_t A, uint32_t DB32, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)
{
 uint16_t tmp = DB32 >> (((A & 2) ^ 2) << 3);

 ABusRW_DB_u16_W1_SH0(A, &tmp, time_thing, dma_time_thing, sh2_dma_time_thing);
}

static INLINE void ABus_Write_DB32_u32(uint32_t A, uint32_t DB32, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)
{
 uint16_t tmp;

 tmp = DB32 >> 16;
 ABusRW_DB_u16_W1_SH0(A, &tmp, time_thing, dma_time_thing, sh2_dma_time_thing);

 tmp = DB32 >> 0;
 ABusRW_DB_u16_W1_SH1(A | 2, &tmp, time_thing, dma_time_thing, sh2_dma_time_thing);
}

// Lower 2 bits of A should be 0
static INLINE uint32_t ABus_Read(uint32_t A, int32_t* time_thing, int32_t* dma_time_thing, int32_t* sh2_dma_time_thing)
{
 uint32_t ret;
 uint16_t tmp = 0xFFFF;

 ABusRW_DB_u16_W0_SH0(A, &tmp, time_thing, dma_time_thing, sh2_dma_time_thing);
 ret = tmp << 16;

 ABusRW_DB_u16_W0_SH1(A | 2, &tmp, time_thing, dma_time_thing, sh2_dma_time_thing);
 ret |= tmp << 0;

 return ret;
}

/* Phase-8q3: SCU_FromSH2_BusRW_DB retired into 6 named variants
 * via source-fold (tools/fold_source.py).  Last SCU template --
 * completes the SCU tree.  Each variant body is the template
 * body with template parameters substituted to literals AND
 * dead branches eliminated at source level; the phase-8q2
 * sizeof(T)+IsWrite dispatch ladder for SCU_RegRW_DB collapses
 * to a single direct named call. */

/* === SCU_FromSH2_BusRW_DB variants (3 T x 2 IsWrite) === */
static INLINE void SCU_FromSH2_BusRW_DB_u8_W0(uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{
 //
 // A bus
 //
 if(A >= 0x02000000 && A <= 0x058FFFFF)
 {
  CheckForceDMAFinish();

  *DB = ABus_Read(A &~ 0x3, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

  return;
 }

 //
 // B bus
 //
 if(A >= 0x05A00000 && A <= 0x05FBFFFF)
 {
  CheckForceDMAFinish();

   uint16_t tmp = 0;

   BBusRW_DB_u16_W0_SH0(A, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);
   *DB = tmp << 16;

   BBusRW_DB_u16_W0_SH1(A | 2, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);
   *DB |= tmp << 0;
  
  return;
 }

 //
 // SCU registers
 //
 if(A >= 0x05FE0000 && A <= 0x05FEFFFF)
 {
  if(!SH2DMAHax)
  {
   SH7095_mem_timestamp +=  8;
   CheckEventsByMemTS();
  }
  else
   *SH2DMAHax -=  8;

  /* Phase-8q2: sizeof(T) + IsWrite fold at SCU_FromSH2_BusRW_DB
   * template instantiation. */
  
   SCU_RegRW_DB_u8_W0 (A, DB);
  
  return;
 }

 // TODO: (investigate 0x5A80000-0x5AFFFFF open bus region)
 //
 //if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 //{
 // return 0;
 //}

 if(!SH2DMAHax)
  SH7095_mem_timestamp +=  7;
 else
  *SH2DMAHax -=  7;
}

static INLINE void SCU_FromSH2_BusRW_DB_u8_W1(uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{
 //
 // A bus
 //
 if(A >= 0x02000000 && A <= 0x058FFFFF)
 {
  CheckForceDMAFinish();

   /* Phase-8k: sizeof(T) folds at template-instantiation time. */
   ABus_Write_DB32_u8 (A, *DB, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

  return;
 }

 //
 // B bus
 //
 if(A >= 0x05A00000 && A <= 0x05FBFFFF)
 {
  CheckForceDMAFinish();

    uint16_t tmp = *DB >> (((A & 2) ^ 2) << 3);

    /* Phase-8q1: sizeof(T) folds at SCU_FromSH2_BusRW_DB instantiation.
     * The outer `if(sizeof(T) == 4)` already split off the u32 case,
     * so T here is u8 or u16. */
    BBusRW_DB_u8_W1_SH0 (A, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

  return;
 }

 //
 // SCU registers
 //
 if(A >= 0x05FE0000 && A <= 0x05FEFFFF)
 {
  if(!SH2DMAHax)
  {
   SH7095_mem_timestamp +=  4 ;
   CheckEventsByMemTS();
  }
  else
   *SH2DMAHax -=  4 ;

  /* Phase-8q2: sizeof(T) + IsWrite fold at SCU_FromSH2_BusRW_DB
   * template instantiation. */
  
   SCU_RegRW_DB_u8_W1 (A, DB);
  
  return;
 }

 // TODO: (investigate 0x5A80000-0x5AFFFFF open bus region)
 //
 //if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 //{
 // return 0;
 //}

 if(!SH2DMAHax)
  SH7095_mem_timestamp +=  4 ;
 else
  *SH2DMAHax -=  4 ;
}

static INLINE void SCU_FromSH2_BusRW_DB_u16_W0(uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{
 //
 // A bus
 //
 if(A >= 0x02000000 && A <= 0x058FFFFF)
 {
  CheckForceDMAFinish();

  *DB = ABus_Read(A &~ 0x3, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

  return;
 }

 //
 // B bus
 //
 if(A >= 0x05A00000 && A <= 0x05FBFFFF)
 {
  CheckForceDMAFinish();

   uint16_t tmp = 0;

   BBusRW_DB_u16_W0_SH0(A, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);
   *DB = tmp << 16;

   BBusRW_DB_u16_W0_SH1(A | 2, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);
   *DB |= tmp << 0;
  
  return;
 }

 //
 // SCU registers
 //
 if(A >= 0x05FE0000 && A <= 0x05FEFFFF)
 {
  if(!SH2DMAHax)
  {
   SH7095_mem_timestamp +=  8;
   CheckEventsByMemTS();
  }
  else
   *SH2DMAHax -=  8;

  /* Phase-8q2: sizeof(T) + IsWrite fold at SCU_FromSH2_BusRW_DB
   * template instantiation. */
  
   SCU_RegRW_DB_u16_W0(A, DB);
  
  return;
 }

 // TODO: (investigate 0x5A80000-0x5AFFFFF open bus region)
 //
 //if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 //{
 // return 0;
 //}

 if(!SH2DMAHax)
  SH7095_mem_timestamp +=  7;
 else
  *SH2DMAHax -=  7;
}

static INLINE void SCU_FromSH2_BusRW_DB_u16_W1(uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{
 //
 // A bus
 //
 if(A >= 0x02000000 && A <= 0x058FFFFF)
 {
  CheckForceDMAFinish();

   /* Phase-8k: sizeof(T) folds at template-instantiation time. */
   ABus_Write_DB32_u16(A, *DB, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

  return;
 }

 //
 // B bus
 //
 if(A >= 0x05A00000 && A <= 0x05FBFFFF)
 {
  CheckForceDMAFinish();

    uint16_t tmp = *DB >> (((A & 2) ^ 2) << 3);

    /* Phase-8q1: sizeof(T) folds at SCU_FromSH2_BusRW_DB instantiation.
     * The outer `if(sizeof(T) == 4)` already split off the u32 case,
     * so T here is u8 or u16. */
    BBusRW_DB_u16_W1_SH0(A, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

  return;
 }

 //
 // SCU registers
 //
 if(A >= 0x05FE0000 && A <= 0x05FEFFFF)
 {
  if(!SH2DMAHax)
  {
   SH7095_mem_timestamp +=  4 ;
   CheckEventsByMemTS();
  }
  else
   *SH2DMAHax -=  4 ;

  /* Phase-8q2: sizeof(T) + IsWrite fold at SCU_FromSH2_BusRW_DB
   * template instantiation. */
  
   SCU_RegRW_DB_u16_W1(A, DB);
  
  return;
 }

 // TODO: (investigate 0x5A80000-0x5AFFFFF open bus region)
 //
 //if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 //{
 // return 0;
 //}

 if(!SH2DMAHax)
  SH7095_mem_timestamp +=  4 ;
 else
  *SH2DMAHax -=  4 ;
}

static INLINE void SCU_FromSH2_BusRW_DB_u32_W0(uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{
 //
 // A bus
 //
 if(A >= 0x02000000 && A <= 0x058FFFFF)
 {
  CheckForceDMAFinish();

  *DB = ABus_Read(A &~ 0x3, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

  return;
 }

 //
 // B bus
 //
 if(A >= 0x05A00000 && A <= 0x05FBFFFF)
 {
  CheckForceDMAFinish();

   uint16_t tmp = 0;

   BBusRW_DB_u16_W0_SH0(A, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);
   *DB = tmp << 16;

   BBusRW_DB_u16_W0_SH1(A | 2, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);
   *DB |= tmp << 0;
  
  return;
 }

 //
 // SCU registers
 //
 if(A >= 0x05FE0000 && A <= 0x05FEFFFF)
 {
  if(!SH2DMAHax)
  {
   SH7095_mem_timestamp +=  8;
   CheckEventsByMemTS();
  }
  else
   *SH2DMAHax -=  8;

  /* Phase-8q2: sizeof(T) + IsWrite fold at SCU_FromSH2_BusRW_DB
   * template instantiation. */
  
   SCU_RegRW_DB_u32_W0(A, DB);
  
  return;
 }

 // TODO: (investigate 0x5A80000-0x5AFFFFF open bus region)
 //
 //if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 //{
 // return 0;
 //}

 if(!SH2DMAHax)
  SH7095_mem_timestamp +=  7;
 else
  *SH2DMAHax -=  7;
}

static INLINE void SCU_FromSH2_BusRW_DB_u32_W1(uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{
 //
 // A bus
 //
 if(A >= 0x02000000 && A <= 0x058FFFFF)
 {
  CheckForceDMAFinish();

   /* Phase-8k: sizeof(T) folds at template-instantiation time. */
   ABus_Write_DB32_u32(A, *DB, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

  return;
 }

 //
 // B bus
 //
 if(A >= 0x05A00000 && A <= 0x05FBFFFF)
 {
  CheckForceDMAFinish();

    uint16_t tmp;

    tmp = *DB >> 16;
    BBusRW_DB_u16_W1_SH0(A, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

    tmp = *DB >> 0;
    BBusRW_DB_u16_W1_SH1(A | 2, &tmp, SH2DMAHax ? NULL : &SH7095_mem_timestamp, NULL, SH2DMAHax);

  return;
 }

 //
 // SCU registers
 //
 if(A >= 0x05FE0000 && A <= 0x05FEFFFF)
 {
  if(!SH2DMAHax)
  {
   SH7095_mem_timestamp +=  4 ;
   CheckEventsByMemTS();
  }
  else
   *SH2DMAHax -=  4 ;

  /* Phase-8q2: sizeof(T) + IsWrite fold at SCU_FromSH2_BusRW_DB
   * template instantiation. */
  
   SCU_RegRW_DB_u32_W1(A, DB);
  
  return;
 }

 // TODO: (investigate 0x5A80000-0x5AFFFFF open bus region)
 //
 //if(A >= 0x05A00000 && A <= 0x05BFFFFF)
 //{
 // return 0;
 //}

 if(!SH2DMAHax)
  SH7095_mem_timestamp +=  4 ;
 else
  *SH2DMAHax -=  4 ;
}





//
// Offset should have lower 2 bits as 0.
//
static uint32_t DMA_ReadABus(uint32_t offset)
{
 return ABus_Read(offset, NULL, &SCU_DMA_ReadOverhead, NULL);
}

static uint32_t DMA_ReadBBus(uint32_t offset)
{
 uint32_t ret;
 uint16_t tmp = 0;

 BBusRW_DB_u16_W0_SH0(offset | 0, &tmp, NULL, &SCU_DMA_ReadOverhead, NULL);
 ret = tmp << 16;

 BBusRW_DB_u16_W0_SH1(offset | 2, &tmp, NULL, &SCU_DMA_ReadOverhead, NULL);
 ret |= tmp << 0;

 return ret;
}

static uint32_t DMA_ReadCBus(uint32_t offset)
{
 //
 // TODO: Determine the nature of this slowdown/overhead.
 //
 SCU_DMA_SDRAM_Slowdown_Counter++;
 if(SCU_DMA_SDRAM_Slowdown_Counter >= 31)
 {
  SCU_DMA_SDRAM_Slowdown_Counter = 0;
  SCU_DMA_ReadOverhead -= 6;
 }

 /* ne16_rbo_be<uint32_t>(WorkRAMH, byte_off): aligned BE 32-bit
  * read from uint16_t array.  Both halves are host-endian; MSB-first
  * combination gives host-endian uint32_t on any host. */
 {
  const uint32_t idx_ = (offset & 0xFFFFC) >> 1;
  return ((uint32_t)WorkRAMH[idx_] << 16) | WorkRAMH[idx_ + 1];
 }
}

static INLINE int AddressToBus(uint32_t A)
{
 int ret = -1;

 if(A >= 0x02000000 && A <= 0x058FFFFF)
  ret = 0;
 else if(A >= 0x05A00000 && A <= 0x05FBFFFF)
  ret = 1;
 else if(A >= 0x06000000)
  ret = 2;

 return ret;
}

static uint32_t (*const rftab[3])(uint32_t) = { DMA_ReadABus, DMA_ReadBBus, DMA_ReadCBus };

static bool StartDMATransfer(DMALevelS* d, const uint32_t ra, const uint32_t wa, const uint32_t bc)
{
 int rb, wb;

 SCU_DMA_VDP1WriteIgnoreKludge = 0;

 rb = AddressToBus(ra);
 wb = AddressToBus(wa);

 if(MDFN_UNLIKELY(rb == -1))
  return false;

 if(MDFN_UNLIKELY(wb == -1))
  return false;

 if(MDFN_UNLIKELY(rb == wb))
  return false;

 d->CurReadBase = ra &~ 0x3;
 d->CurReadSub = ra & 0x3;

 d->CurWriteAddr = wa;

 d->CurByteCount = bc;

 d->ReadFunc = rftab[rb];
 d->WriteBus = wb;

 d->Buffer = d->ReadFunc(d->CurReadBase);

 if(wb != 0x1 && d->WriteAdd == 0x1)
  d->WATable = &dma_write_tab.aciv1[wa & 0x3][(bc < 16) ? bc : (16 | (bc & 0x7))][0];
 else
  d->WATable = &dma_write_tab.acb[wb == 1][d->WriteAdd][wa & 0x3][(bc < 12) ? bc : (8 | (bc & 0x3))][0];

 return true;
}

static bool NextIndirect(DMALevelS* d)
{
 // count, dest, src
 uint32_t tmp[3];

 for(unsigned i = 0; i < 3; i++)
 {
  tmp[i] = d->TableReadFunc(d->CurTableAddr);
  d->CurTableAddr += (d->ReadAdd ? 4 : 0);
 }

 d->FinalTransfer = (bool)(tmp[2] & 0x80000000);

 tmp[0] &= 0xFFFFF;

 if(!tmp[0])
  tmp[0] = 0x100000;

 return StartDMATransfer(d, tmp[2] & 0x07FFFFFF, tmp[1] & 0x07FFFFFF, tmp[0]);
}

bool SCU_CheckVDP1HaltKludge(void)
{
 bool ret = false;

 for(int level = 2; level >= 0; level--)
 {
  DMALevelS* d = &DMALevel[level];

  if(d->Active > 0)
  {
   if(d->WriteBus == 1 && d->ReadFunc == DMA_ReadCBus && d->CurWriteAddr >= 0x5C00000 && d->CurWriteAddr <= 0x5DFFFFF)
    ret = true;
   //else if(d->WriteBus == 2 && d->ReadFunc == DMA_ReadBBus && d->CurReadBase >= 0x5C00000 && d->CurReadBase <= 0x5C7FFFF)
   // ret = true;

   break;
  }
 }

 return ret;
}

static void RecalcDMAHalt(void)
{
 bool Halted = false;

 for(int level = 2; level >= 0; level--)
 {
  DMALevelS* d = &DMALevel[level];

  if(d->Active > 0)
  {
   if(d->WriteBus == 2 || d->ReadFunc == DMA_ReadCBus)
    Halted = true;
   break;
  }
 }

 SH7095_SetExtHalt(&CPU[0], Halted);
 SH7095_SetExtHalt(&CPU[1], Halted);
}

// TODO: Alter write tables to use -1, 0, 1 for 1, 2, 4

static void CheckDMAStart(DMALevelS* d)
{
 if(!d->Active && d->GoGoGadget)
 {
  d->GoGoGadget = false;
  d->FinalTransfer = true;
  d->TableReadFunc = NULL;

  if(d->Indirect)
  {
   int tb;

   d->CurTableAddr = d->StartWriteAddr & 0x07FFFFFC;	// Tested, lower 2 bits are 0 on DMA end when write address update enabled.
   tb = AddressToBus(d->CurTableAddr);

   if(tb < 0) { }
   else
   {
    d->TableReadFunc = rftab[tb];

    if(NextIndirect(d))
    {
     d->Active = true;
     RecalcDMAHalt();
    }
   }
  }
  else
  {
   if(!StartDMATransfer(d, d->StartReadAddr, d->StartWriteAddr, (!d->StartByteCount) ? ((d - DMALevel) ? 0x1000 : 0x100000) : d->StartByteCount))
   {
    SCU_SetInt(SCU_INT_DMA_ILL, true);
    SCU_SetInt(SCU_INT_DMA_ILL, false);
   }
   else
   {
    d->Active = true;
    RecalcDMAHalt();
   }
  }
 }
}

static void CheckDMASFByInt(unsigned int_which)
{
 static const uint8_t sf_to_int_tab[7] =
 {
  SCU_INT_VBIN,	SCU_INT_VBOUT, SCU_INT_HBIN, SCU_INT_TIMER0,
  SCU_INT_TIMER1, SCU_INT_SCSP, SCU_INT_VDP1
 };

 for(unsigned level = 0; level < 3; level++)
 {
  DMALevelS* d = &DMALevel[level];

  if(d->Enable && d->SF < 0x7 && sf_to_int_tab[d->SF] == int_which)
  {
   d->GoGoGadget = true;
   CheckDMAStart(d);
  }
 }
}

/* Phase-8m: DMA_Read<unsigned count> retired.  The template was
 * instantiated for count = 1 / 2 / 4 (matching the three SCU DMA
 * transfer widths) via the 9 call sites in DMA_Loop_0 / _1 / _2
 * (themselves the phase-8g detempled variants of DMA_Loop).
 * Three named static helpers below carry the post-folding body
 * with count substituted as a literal; the `4 - count` and
 * `d->CurReadSub += count` terms collapse to constants at
 * compile time. */
static INLINE uint32_t DMA_Read_c1(DMALevelS* d)
{
 int shift = ((0x3 ^ (d->CurReadSub & 0x3)) - (0x3 ^ (d->CurWriteAddr & 0x3 & 3))) * 8;

 d->CurReadSub += 1;
 if(d->CurReadSub > 4)
 {
  if((d->CurReadSub - 1) < 4)
   shift += 32;

  d->CurReadSub -= 4;
  d->CurReadBase += (d->ReadAdd ? 4 : 0);
  //
  SCU_DMA_TimeCounter -= SCU_DMA_ReadOverhead;
  SCU_DMA_ReadOverhead = 0;
  uint32_t tmp = d->ReadFunc(d->CurReadBase);
  d->Buffer <<= 32;
  d->Buffer |= tmp;
 }

 if(shift > 0)
  return d->Buffer >> shift;
 else
  return d->Buffer << -shift;
}

static INLINE uint32_t DMA_Read_c2(DMALevelS* d)
{
 int shift = ((0x3 ^ (d->CurReadSub & 0x3)) - (0x3 ^ (d->CurWriteAddr & 0x3 & 2))) * 8;

 d->CurReadSub += 2;
 if(d->CurReadSub > 4)
 {
  if((d->CurReadSub - 2) < 4)
   shift += 32;

  d->CurReadSub -= 4;
  d->CurReadBase += (d->ReadAdd ? 4 : 0);
  //
  SCU_DMA_TimeCounter -= SCU_DMA_ReadOverhead;
  SCU_DMA_ReadOverhead = 0;
  uint32_t tmp = d->ReadFunc(d->CurReadBase);
  d->Buffer <<= 32;
  d->Buffer |= tmp;
 }

 if(shift > 0)
  return d->Buffer >> shift;
 else
  return d->Buffer << -shift;
}

static INLINE uint32_t DMA_Read_c4(DMALevelS* d)
{
 /* count == 4 folds: `4 - count` is 0 so the second 0x3 ^ () term
  * masks against 0, leaving shift = (0x3 ^ (d->CurReadSub & 0x3))
  * - 0x3) * 8. */
 int shift = ((0x3 ^ (d->CurReadSub & 0x3)) - 0x3) * 8;

 d->CurReadSub += 4;
 if(d->CurReadSub > 4)
 {
  if((d->CurReadSub - 4) < 4)
   shift += 32;

  d->CurReadSub -= 4;
  d->CurReadBase += (d->ReadAdd ? 4 : 0);
  //
  SCU_DMA_TimeCounter -= SCU_DMA_ReadOverhead;
  SCU_DMA_ReadOverhead = 0;
  uint32_t tmp = d->ReadFunc(d->CurReadBase);
  d->Buffer <<= 32;
  d->Buffer |= tmp;
 }

 if(shift > 0)
  return d->Buffer >> shift;
 else
  return d->Buffer << -shift;
}

/* Phase-8q2: DMA_Write retired into 9 named variants via
 * source-fold.  All 9 (WriteBus, T) tuples are reached by
 * concrete callers in DMA_Loop_WriteBus{0,1,2}. */

/* === DMA_Write variants (3 WriteBus x 3 T) === */
static INLINE void DMA_Write_WB0_u8(DMALevelS* d, uint32_t DB)
{
 const uint32_t A = d->CurWriteAddr &~ (1 - 1);
 int32_t WriteOverhead = 0;

  /* Phase-8k: sizeof(T) folds at template-instantiation time. */
  ABus_Write_DB32_u8 (A, DB, NULL, &WriteOverhead, NULL);

 SCU_DMA_TimeCounter -= WriteOverhead;
 SCU_DMA_ReadOverhead = ((int32_t)(0) < (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead) ? (int32_t)(0) : (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead));
 d->CurByteCount -= 1;
}

static INLINE void DMA_Write_WB0_u16(DMALevelS* d, uint32_t DB)
{
 const uint32_t A = d->CurWriteAddr &~ (2 - 1);
 int32_t WriteOverhead = 0;

  /* Phase-8k: sizeof(T) folds at template-instantiation time. */
  ABus_Write_DB32_u16(A, DB, NULL, &WriteOverhead, NULL);

 SCU_DMA_TimeCounter -= WriteOverhead;
 SCU_DMA_ReadOverhead = ((int32_t)(0) < (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead) ? (int32_t)(0) : (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead));
 d->CurByteCount -= 2;
}

static INLINE void DMA_Write_WB0_u32(DMALevelS* d, uint32_t DB)
{
 const uint32_t A = d->CurWriteAddr &~ (4 - 1);
 int32_t WriteOverhead = 0;

  /* Phase-8k: sizeof(T) folds at template-instantiation time. */
  ABus_Write_DB32_u32(A, DB, NULL, &WriteOverhead, NULL);

 SCU_DMA_TimeCounter -= WriteOverhead;
 SCU_DMA_ReadOverhead = ((int32_t)(0) < (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead) ? (int32_t)(0) : (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead));
 d->CurByteCount -= 4;
}

static INLINE void DMA_Write_WB1_u8(DMALevelS* d, uint32_t DB)
{
 const uint32_t A = d->CurWriteAddr &~ (1 - 1);
 int32_t WriteOverhead = 0;

  uint16_t DB16;

  DB16 = DB >> (((A & 0x2) ^ 0x2) * 8);
  /* Phase-8q1: sizeof(T) folds at DMA_Write template instantiation. */
  BBusRW_DB_u8_W1_SH0 (A, &DB16, NULL, &WriteOverhead, NULL);

 SCU_DMA_TimeCounter -= WriteOverhead;
 SCU_DMA_ReadOverhead = ((int32_t)(0) < (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead) ? (int32_t)(0) : (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead));
 d->CurByteCount -= 1;
}

static INLINE void DMA_Write_WB1_u16(DMALevelS* d, uint32_t DB)
{
 const uint32_t A = d->CurWriteAddr &~ (2 - 1);
 int32_t WriteOverhead = 0;

  uint16_t DB16;

  DB16 = DB >> (((A & 0x2) ^ 0x2) * 8);
  /* Phase-8q1: sizeof(T) folds at DMA_Write template instantiation. */
  BBusRW_DB_u16_W1_SH0(A, &DB16, NULL, &WriteOverhead, NULL);

 SCU_DMA_TimeCounter -= WriteOverhead;
 SCU_DMA_ReadOverhead = ((int32_t)(0) < (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead) ? (int32_t)(0) : (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead));
 d->CurByteCount -= 2;
}

static INLINE void DMA_Write_WB1_u32(DMALevelS* d, uint32_t DB)
{
 const uint32_t A = d->CurWriteAddr &~ (4 - 1);
 int32_t WriteOverhead = 0;

  uint16_t DB16;

  DB16 = DB >> (((A & 0x2) ^ 0x2) * 8);
  /* Phase-8q1: sizeof(T) folds at DMA_Write template instantiation. */
  BBusRW_DB_u32_W1_SH0(A, &DB16, NULL, &WriteOverhead, NULL);

 SCU_DMA_TimeCounter -= WriteOverhead;
 SCU_DMA_ReadOverhead = ((int32_t)(0) < (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead) ? (int32_t)(0) : (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead));
 d->CurByteCount -= 4;
}

static INLINE void DMA_Write_WB2_u8(DMALevelS* d, uint32_t DB)
{
 const uint32_t A = d->CurWriteAddr &~ (1 - 1);
 int32_t WriteOverhead = 0;

  /* ne16_wbo_be<T>(WorkRAMH, byte_off, val) folded. */
  const uint32_t boff_ = A & 0xFFFFF;
  const uint8_t val_ = DB >> (((A & 3) ^ (4 - 1)) << 3);
  
#ifdef MSB_FIRST
   ((uint8_t*)WorkRAMH)[boff_] = val_;
#else
   ((uint8_t*)WorkRAMH)[boff_ ^ 1] = val_;
#endif

 SCU_DMA_TimeCounter -= WriteOverhead;
 SCU_DMA_ReadOverhead = ((int32_t)(0) < (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead) ? (int32_t)(0) : (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead));
 d->CurByteCount -= 1;
}

static INLINE void DMA_Write_WB2_u16(DMALevelS* d, uint32_t DB)
{
 const uint32_t A = d->CurWriteAddr &~ (2 - 1);
 int32_t WriteOverhead = 0;

  /* ne16_wbo_be<T>(WorkRAMH, byte_off, val) folded. */
  const uint32_t boff_ = A & 0xFFFFF;
  const uint16_t val_ = DB >> (((A & 3) ^ (4 - 2)) << 3);
  WorkRAMH[boff_ >> 1] = val_;

 SCU_DMA_TimeCounter -= WriteOverhead;
 SCU_DMA_ReadOverhead = ((int32_t)(0) < (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead) ? (int32_t)(0) : (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead));
 d->CurByteCount -= 2;
}

static INLINE void DMA_Write_WB2_u32(DMALevelS* d, uint32_t DB)
{
 const uint32_t A = d->CurWriteAddr &~ (4 - 1);
 int32_t WriteOverhead = 0;

  /* ne16_wbo_be<T>(WorkRAMH, byte_off, val) folded. */
  const uint32_t boff_ = A & 0xFFFFF;
  const uint32_t val_ = DB >> (((A & 3) ^ (4 - 4)) << 3);
  
   WorkRAMH[(boff_ >> 1) + 0] = (uint32_t)val_ >> 16;
   WorkRAMH[(boff_ >> 1) + 1] = val_;

 SCU_DMA_TimeCounter -= WriteOverhead;
 SCU_DMA_ReadOverhead = ((int32_t)(0) < (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead) ? (int32_t)(0) : (int32_t)(SCU_DMA_ReadOverhead - WriteOverhead));
 d->CurByteCount -= 4;
}

/* Phase-8g: DMA_Loop<WriteBus> retired -- three concrete
 * instantiations (WriteBus = 0 / 1 / 2 for the SCU's A-bus,
 * B-bus, and "C" / DSP DMA paths), each used exactly once in
 * the LoopFuncs[] dispatch table.  The 25-line body is dupli-
 * cated three times below, once per WriteBus value, with the
 * template parameter inlined as a literal.  DMA_Write<W, T>
 * stays templated (W is one of 0/1/2 per body; T comes from
 * the per-byte-width switch).
 *
 * Each instance keeps the original's NO_INLINE attribute --
 * DMA_Loop was deliberately not inlined into UpdateDMAInner
 * to keep its stack frame separate from the SCU register-RW
 * hot path, and the function-pointer table use below means
 * inlining isn't an option anyway. */
static bool NO_INLINE DMA_Loop_0(DMALevelS* d)
{
 while(MDFN_LIKELY(d->Active > 0 && SCU_DMA_TimeCounter < SCU_DMA_RunUntil))
 {
  switch(d->WATable->write_size)
  {
   case 0x1: DMA_Write_WB0_u8(d, DMA_Read_c1(d)); break;
   case 0x2: DMA_Write_WB0_u16(d, DMA_Read_c2(d)); break;
   case 0x4: DMA_Write_WB0_u32(d, DMA_Read_c4(d)); break;
  }
  d->CurWriteAddr += d->WATable->write_addr_delta;

  if(d->CurByteCount <= (uint32_t)(int8_t)d->WATable->compare)
   d->WATable++;

  if(MDFN_UNLIKELY(!d->CurByteCount))
  {
   SCU_DMA_TimeCounter -= SCU_DMA_ReadOverhead;
   SCU_DMA_ReadOverhead = 0;
   return true;
  }
 }

 return false;
}

static bool NO_INLINE DMA_Loop_1(DMALevelS* d)
{
 while(MDFN_LIKELY(d->Active > 0 && SCU_DMA_TimeCounter < SCU_DMA_RunUntil))
 {
  switch(d->WATable->write_size)
  {
   case 0x1: DMA_Write_WB1_u8(d, DMA_Read_c1(d)); break;
   case 0x2: DMA_Write_WB1_u16(d, DMA_Read_c2(d)); break;
   case 0x4: DMA_Write_WB1_u32(d, DMA_Read_c4(d)); break;
  }
  d->CurWriteAddr += d->WATable->write_addr_delta;

  if(d->CurByteCount <= (uint32_t)(int8_t)d->WATable->compare)
   d->WATable++;

  if(MDFN_UNLIKELY(!d->CurByteCount))
  {
   SCU_DMA_TimeCounter -= SCU_DMA_ReadOverhead;
   SCU_DMA_ReadOverhead = 0;
   return true;
  }
 }

 return false;
}

static bool NO_INLINE DMA_Loop_2(DMALevelS* d)
{
 while(MDFN_LIKELY(d->Active > 0 && SCU_DMA_TimeCounter < SCU_DMA_RunUntil))
 {
  switch(d->WATable->write_size)
  {
   case 0x1: DMA_Write_WB2_u8(d, DMA_Read_c1(d)); break;
   case 0x2: DMA_Write_WB2_u16(d, DMA_Read_c2(d)); break;
   case 0x4: DMA_Write_WB2_u32(d, DMA_Read_c4(d)); break;
  }
  d->CurWriteAddr += d->WATable->write_addr_delta;

  if(d->CurByteCount <= (uint32_t)(int8_t)d->WATable->compare)
   d->WATable++;

  if(MDFN_UNLIKELY(!d->CurByteCount))
  {
   SCU_DMA_TimeCounter -= SCU_DMA_ReadOverhead;
   SCU_DMA_ReadOverhead = 0;
   return true;
  }
 }

 return false;
}

//
// TODO: Check start read/write address updating when wrapping to next bus or beyond end of SDRAM.
//
static INLINE void UpdateDMAInner(DMALevelS* d)
{
 static bool (*const LoopFuncs[3])(DMALevelS*) = { DMA_Loop_0, DMA_Loop_1, DMA_Loop_2 };

 if(MDFN_UNLIKELY(LoopFuncs[d->WriteBus](d)))
 {
  if(d->TableReadFunc && !d->FinalTransfer)
  {
   NextIndirect(d);
  }
  else
  {
   if(d->ReadUpdate && !d->TableReadFunc)
      d->StartReadAddr = (d->CurReadBase + d->CurReadSub) & 0x07FFFFFF;

   if(d->WriteUpdate)
   {
    if(d->TableReadFunc)
     d->StartWriteAddr = d->CurTableAddr & 0x07FFFFFF;
    else
     d->StartWriteAddr = d->CurWriteAddr & 0x07FFFFFF;
   }

   d->FinishTime = SCU_DMA_TimeCounter;
   d->Active = -1;
  }
 }
}

static void SCU_DoDMAEnd(const unsigned level)
{
 static const unsigned itab[3] = { SCU_INT_L0DMA, SCU_INT_L1DMA, SCU_INT_L2DMA };
 DMALevel[level].Active = false;
 RecalcDMAHalt();
 SCU_SetInt(itab[level], true);
 SCU_SetInt(itab[level], false);
 CheckDMAStart(&DMALevel[level]);    
}

sscpu_timestamp_t SCU_UpdateDMA(sscpu_timestamp_t timestamp)
{
 if(timestamp < SH7095_mem_timestamp)
  return SH7095_mem_timestamp;
 //
 //
 //
 SCU_DMA_TimeCounter = ((int32_t)(((int32_t)(SCU_DMA_RunUntil) < (int32_t)(timestamp) ? (int32_t)(SCU_DMA_RunUntil) : (int32_t)(timestamp))) > (int32_t)(SCU_DMA_TimeCounter) ? (int32_t)(((int32_t)(SCU_DMA_RunUntil) < (int32_t)(timestamp) ? (int32_t)(SCU_DMA_RunUntil) : (int32_t)(timestamp))) : (int32_t)(SCU_DMA_TimeCounter));
 SCU_DMA_RunUntil = timestamp + DMA_UpdateTimingGran;

 for(int level = 2; level >= 0; level--)
 {
  DMALevelS* d = &DMALevel[level];

  while(d->Active && SCU_DMA_TimeCounter < SCU_DMA_RunUntil)
  {
   UpdateDMAInner(d);

   if(MDFN_UNLIKELY(d->Active < 0))
   {
    if(MDFN_UNLIKELY(timestamp >= d->FinishTime))
     SCU_DoDMAEnd(level);
    else
     return d->FinishTime;
   }
  }
 }

 return SCU_DMA_RunUntil;
}

//
// Check to see if DMA is active, and if so, force the highest-priority DMA
// to finish early(kind of hacky).
//
static NO_INLINE void ForceDMAFinish(void)
{
 for(int level = 2; level >= 0; level--)
 {
  if(!DMALevel[level].Active)
   continue;

  if(DMALevel[level].Active > 0)
  {
   const sscpu_timestamp_t rus = SCU_DMA_RunUntil;
   //
   SCU_DMA_RunUntil = 0x7FFFFFFF;
   UpdateDMAInner(&DMALevel[level]);
   //
   SCU_DMA_RunUntil = rus;
  }

  if(DMALevel[level].Active < 0)
   SCU_DoDMAEnd(level);

  break;
 }

 SCU_DMA_TimeCounter = SCU_DMA_RunUntil;
}

static INLINE void CheckForceDMAFinish(void)
{
 if(MDFN_LIKELY(!(DMALevel[0].Active | DMALevel[1].Active | DMALevel[2].Active)))
  return;

 ForceDMAFinish();
}

//
//
//
/* Phase-5e: cross-TU symbol -- declared in scu_dsp_common.inc
 * (extern "C" wrapped on the C++ side, plain decl on the C side),
 * defined here in ss.cpp's TU.  `extern "C"` on the definition
 * forces the unmangled symbol so the scu_dsp_*.c files link
 * against the same `DSP` global.  Gated for C compilation: in C
 * the linkage is already C (no mangling), so the `extern "C"`
 * wrapper is C++-only syntax to suppress -- not semantics. */
#ifdef __cplusplus
extern "C" {
#endif
struct DSPS DSP;
#ifdef __cplusplus
}
#endif

sscpu_timestamp_t SCU_UpdateDSP(sscpu_timestamp_t timestamp)
{
 int32_t cycles = timestamp - DSP.LastTS;
 DSP.LastTS = timestamp;
 //
 //
 //
 DSP.T0_Until += cycles;	// Overflow prevented in SCU_ResetTS
 DSP.CycleCounter += cycles;
 if(DSP.CycleCounter > DSP_UpdateTimingGran)
  DSP.CycleCounter = DSP_UpdateTimingGran;

 if(MDFN_UNLIKELY(!DSPS_IsRunning(&DSP)))
  return SS_EVENT_DISABLED_TS;

 {
  DSPS* const dsp = &DSP;
  // Threaded dispatch: the first handler tail-chains through the
  // remaining cycles via DSP_TailDispatch and only returns once the
  // budget is exhausted (or the DSP stopped).
  if(MDFN_LIKELY(dsp->CycleCounter > 0))
   ((void (*)(DSPS*))(DSP_INSTR_BASE_UIPT + (uintptr_t)(DSP_INSTR_RECOVER_TCAST)dsp->NextInstr))(dsp);
 }

 if(MDFN_UNLIKELY(!DSPS_IsRunning(&DSP)))
 {
  DSP.CycleCounter += DSP_EndCCSubVal;
  return SS_EVENT_DISABLED_TS;
 }

 return timestamp + DSP_UpdateTimingGran;
}

static void DSP_Reset(bool powering_up)
{
 DSP.State = 0;
 DSP.T0_Until = 0x10000000;
 DSP.CycleCounter = 0;

 if(powering_up)
 {
  for(unsigned i = 0; i < 256; i++)
   DSP.ProgRAM[i] = DSP_DecodeInstruction(0, false);

  for(unsigned i = 0; i < 256; i++)
   ((uint32_t*)DSP.DataRAM)[i] = 0;
 }

 DSP.PC = 0;
 DSP.RA = 0;
 DSP.FlagZ = false;
 DSP.FlagS = false;
 DSP.FlagV = false;
 DSP.FlagC = false;
 DSP.FlagEnd = false;
 SCU_SetInt(SCU_INT_DSP, false);

 DSP.NextInstr = DSP_DecodeInstruction(0, false);

 DSP.TOP = 0;
 DSP.LOP = 0;

 DSP.AC.T = 0;
 DSP.P.T = 0;

 for(unsigned i = 0; i < 4; i++)
  DSP.CT[i] = 0;

 DSP.RX = 0;
 DSP.RY = 0;

 DSP.RAO = 0;
 DSP.WAO = 0;

 DSP.PRAMDMABufCount = 0;
 for(unsigned i = 0; i < 256; i++)
  DSP.PRAMDMABuf[i] = 0;
}

/* Phase-5e: cross-TU symbols.  See the matching extern "C" decl
 * in scu_dsp_common.inc -- ss.cpp emits these as unmangled
 * symbols here so scu_dsp_misc.c / scu_dsp_mvi.c link against
 * the same names.  DSP_Init's address is also used as the base
 * pointer for the DSP_INSTR_BASE_UIPT macro on 64-bit hosts, so
 * the same symbol address must be visible to both compilers. */
#ifdef __cplusplus
extern "C" {
#endif

MDFN_COLD void DSP_Init(void)
{
 DSP.LastTS = 0;
}

void DSP_FinishPRAMDMA(void)
{
 if(DSP.T0_Until < DSP.CycleCounter)
  DSP.CycleCounter = DSP.T0_Until &~ 1;
 DSP.T0_Until = DSP.CycleCounter;

 for(uint32_t i = 0; i < DSP.PRAMDMABufCount; i++)
  DSP.ProgRAM[DSP.PC++] = DSP_DecodeInstruction(DSP.PRAMDMABuf[i & 0xFF], false);

 DSP.PRAMDMABufCount = 0;
 //
 DSP.PC = DSP.TOP;
 DSP.NextInstr = DSP_DecodeInstruction(0, false);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* Phase-5d: was `template<bool looped, bool hold, bool format, bool dir,
 * unsigned drw> static NO_INLINE NO_CLONE void DMAInstr(DSPS* dsp)` --
 * 2 x 2 x 2 x 2 x 8 = 128 instantiations (no hand-dedup in
 * scu_dsp_dmatab.inc; every slot has its own unique template-arg
 * tuple).  Monomorphized via X-macro into 128 named functions,
 * extending the phase 5b / 5c / 5d-GenInstr pattern one more axis. */

#define DMAInstr_BODY(LOOPED, HOLD, FORMAT, DIR, DRW)                                              \
{                                                                                                  \
 const uint32_t instr = DSP_InstrPre(dsp, LOOPED);                                                 \
 const unsigned add_mode = (instr >> 15) & 0x7;                                                    \
 uint8_t count;	/* 0 = 256 */                                                                      \
                                                                                                   \
 if(dsp->T0_Until < dsp->CycleCounter)                                                             \
 {                                                                                                 \
  /* TODO: Rework how DSP DMA is handled if this condition occurs                                  \
   * with the same data RAM bank in any games. */                                                  \
  dsp->CycleCounter = dsp->T0_Until &~ 1;                                                          \
 }                                                                                                 \
                                                                                                   \
 dsp->T0_Until = dsp->CycleCounter;                                                                \
                                                                                                   \
 if(FORMAT)                                                                                        \
 {                                                                                                 \
  const unsigned crw = instr & 0x3;                                                                \
  const bool ctinc = instr & 0x4;                                                                  \
                                                                                                   \
  count = dsp->DataRAM[crw][dsp->CT[crw]] & 0xFF;                                                  \
  dsp->CT[crw] = (dsp->CT[crw] + ctinc) & 0x3F;                                                    \
 }                                                                                                 \
 else                                                                                              \
  count = instr & 0xFF;                                                                            \
                                                                                                   \
 /* do { } while(0) lets bus-error paths "break" out so we still                                   \
  * fall through to DSP_TailDispatch at the end (the original                                      \
  * dispatcher would re-enter on the next loop iteration). */                                      \
 do {                                                                                              \
 if(DIR)                                                                                           \
 {                                                                                                 \
  const uint32_t addr_add_amount = (1 << add_mode) &~ 1;                                           \
  uint32_t addr = (dsp->WAO << 2) & 0x07FFFFFF;                                                    \
  const int WriteBus = AddressToBus(addr);                                                         \
                                                                                                   \
  if(MDFN_UNLIKELY(WriteBus == -1))                                                                \
  {                                                                                                \
   break;                                                                                          \
  }                                                                                                \
                                                                                                   \
  /* Fast path: a contiguous run of 16-bit writes landing entirely within the                      \
   * VDP2 register/RAM window. Collapse the per-halfword renderer-queue pushes                     \
   * (the hot part of DSP DMA) into a single burst command; the SH2-side array                     \
   * writes and their VRAM-bank access penalties still happen per halfword. */                     \
  {                                                                                                \
   const unsigned words = count ? count : 256;                                                     \
   const unsigned n16 = words << 1;                                                                \
                                                                                                   \
   if(WriteBus == 1 && addr >= 0x05E00000 &&                                                       \
      (uint64_t)addr + (uint64_t)(n16 - 1) * addr_add_amount <= 0x05FBFFFF)                        \
   {                                                                                               \
    uint16_t burstbuf[512];                                                                        \
    unsigned k = 0;                                                                                \
                                                                                                   \
    do                                                                                             \
    {                                                                                              \
     uint32_t DB;                                                                                  \
                                                                                                   \
     if((DRW) & 0x4)                                                                               \
      DB = 0xFFFFFFFF;                                                                             \
     else                                                                                          \
     {                                                                                             \
      DB = dsp->DataRAM[DRW][dsp->CT[DRW]];                                                        \
      dsp->CT[DRW] = (dsp->CT[DRW] + 1) & 0x3F;                                                    \
     }                                                                                             \
                                                                                                   \
     burstbuf[k++] = DB >> 16;                                                                     \
     burstbuf[k++] = DB & 0xFFFF;                                                                  \
    } while(--count);                                                                              \
                                                                                                   \
    dsp->T0_Until -= (int32_t)n16;                                                                 \
    dsp->T0_Until -= (int32_t)VDP2_Write16Burst_DB(addr, n16, add_mode, burstbuf);                 \
    addr += n16 * addr_add_amount;                                                                 \
                                                                                                   \
    if(!(HOLD))                                                                                    \
     dsp->WAO = (addr + 2) >> 2;                                                                   \
                                                                                                   \
    break;                                                                                         \
   }                                                                                               \
  }                                                                                                \
                                                                                                   \
  /* Read from data RAM, write to external bus */                                                  \
  do                                                                                               \
  {                                                                                                \
   uint32_t DB;                                                                                    \
                                                                                                   \
   if((DRW) & 0x4)                                                                                 \
    DB = 0xFFFFFFFF;                                                                               \
   else                                                                                            \
   {                                                                                               \
    DB = dsp->DataRAM[DRW][dsp->CT[DRW]];                                                          \
    dsp->CT[DRW] = (dsp->CT[DRW] + 1) & 0x3F;                                                      \
   }                                                                                               \
                                                                                                   \
   if(WriteBus == 2)                                                                               \
   {                                                                                               \
    /* ne16_wbo_be<uint32_t>(WorkRAMH, byte_off, DB) folded. */                                    \
    const uint32_t idx_ = (addr & 0xFFFFC) >> 1;                                                   \
    WorkRAMH[idx_ + 0] = DB >> 16;                                                                 \
    WorkRAMH[idx_ + 1] = DB;                                                                       \
    addr += addr_add_amount;                                                                       \
    dsp->T0_Until -= 2;                                                                            \
   }                                                                                               \
   else if(WriteBus == 1)                                                                          \
   {                                                                                               \
    uint16_t DB16;                                                                                 \
                                                                                                   \
    DB16 = DB >> 16;                                                                               \
    BBusRW_DB_u16_W1_SH0(addr, &DB16, NULL, &dsp->T0_Until, NULL);                                  \
                                                                                                   \
    addr += addr_add_amount;                                                                       \
                                                                                                   \
    DB16 = DB;                                                                                     \
    BBusRW_DB_u16_W1_SH1(addr, &DB16, NULL, &dsp->T0_Until, NULL);                            \
                                                                                                   \
    addr += addr_add_amount;                                                                       \
   }                                                                                               \
   else if(WriteBus == 0)                                                                          \
   {                                                                                               \
    ABus_Write_DB32_u32(addr, DB, NULL, &dsp->T0_Until, NULL);                                     \
                                                                                                   \
    addr += addr_add_amount;                                                                       \
   }                                                                                               \
  } while(--count);                                                                                \
                                                                                                   \
  if(!(HOLD))                                                                                      \
   dsp->WAO = (addr + 2) >> 2;                                                                     \
 }                                                                                                 \
 else                                                                                              \
 {                                                                                                 \
  const uint32_t addr_add_amount = (1 << (add_mode & 0x2)) &~ 1;                                   \
  uint32_t addr = (dsp->RAO << 2) & 0x07FFFFFF;                                                    \
  const int ReadBus = AddressToBus(addr);                                                          \
                                                                                                   \
  if(MDFN_UNLIKELY(ReadBus == -1))                                                                 \
  {                                                                                                \
   break;                                                                                          \
  }                                                                                                \
                                                                                                   \
  dsp->PRAMDMABufCount = 0;                                                                        \
                                                                                                   \
  /* Read from external bus, write to data RAM or program RAM */                                   \
  do                                                                                               \
  {                                                                                                \
   uint32_t DB = 0;                                                                                \
                                                                                                   \
   if(ReadBus == 2)                                                                                \
   {                                                                                               \
    /* ne16_rbo_be<uint32_t>(WorkRAMH, byte_off) folded. */                                        \
    const uint32_t idx_ = (addr & 0xFFFFF) >> 1;                                                   \
    DB = ((uint32_t)WorkRAMH[idx_] << 16) | WorkRAMH[idx_ + 1];                                    \
    dsp->T0_Until -= 2;                                                                            \
                                                                                                   \
    addr += addr_add_amount;                                                                       \
   }                                                                                               \
   else if(ReadBus == 1)                                                                           \
   {                                                                                               \
    uint16_t tmp = 0;                                                                              \
                                                                                                   \
    BBusRW_DB_u16_W0_SH0(addr | 0, &tmp, NULL, &dsp->T0_Until, NULL);                              \
    DB = tmp << 16;                                                                                \
                                                                                                   \
    BBusRW_DB_u16_W0_SH1(addr | 2, &tmp, NULL, &dsp->T0_Until, NULL);                        \
    DB |= tmp << 0;                                                                                \
                                                                                                   \
    addr += 4;                                                                                     \
   }                                                                                               \
   else if(ReadBus == 0)                                                                           \
   {                                                                                               \
    DB = ABus_Read(addr, NULL, &dsp->T0_Until, NULL);                                                    \
                                                                                                   \
    addr += addr_add_amount;                                                                       \
   }                                                                                               \
                                                                                                   \
   if((DRW) & 0x4)                                                                                 \
   {                                                                                               \
    if(!((DRW) & 0x3))                                                                             \
    {                                                                                              \
     dsp->PRAMDMABuf[dsp->PRAMDMABufCount++ & 0xFF] = DB;                                          \
    }                                                                                              \
   }                                                                                               \
   else                                                                                            \
   {                                                                                               \
    dsp->DataRAM[DRW][dsp->CT[DRW]] = DB;                                                          \
    dsp->CT[DRW] = (dsp->CT[DRW] + 1) & 0x3F;                                                      \
   }                                                                                               \
  } while(--count);                                                                                \
                                                                                                   \
  if(!(HOLD))                                                                                      \
   dsp->RAO = addr >> 2;                                                                           \
 }                                                                                                 \
 } while(0);                                                                                       \
                                                                                                   \
 DSP_TailDispatch(dsp);                                                                            \
}

#define DMAInstr_NAME(LOOPED, HOLD, FORMAT, DIR, DRW) \
 DMAInstr_##LOOPED##_##HOLD##_##FORMAT##_##DIR##_##DRW

#define DEFINE_DMAInstr(LOOPED, HOLD, FORMAT, DIR, DRW)                                            \
 static NO_INLINE NO_CLONE void DMAInstr_NAME(LOOPED, HOLD, FORMAT, DIR, DRW)(struct DSPS* dsp)    \
 DMAInstr_BODY(LOOPED, HOLD, FORMAT, DIR, DRW)

/* Nested enumerators: DRW (0..7) -> DIR (0..1) -> FORMAT (0..1) -> HOLD (0..1) -> LOOPED (0..1) */
#define DDMA_FOR_EACH_DRW(M, LOOPED, HOLD, FORMAT, DIR)                                            \
 M(LOOPED, HOLD, FORMAT, DIR, 0x00) M(LOOPED, HOLD, FORMAT, DIR, 0x01)                             \
 M(LOOPED, HOLD, FORMAT, DIR, 0x02) M(LOOPED, HOLD, FORMAT, DIR, 0x03)                             \
 M(LOOPED, HOLD, FORMAT, DIR, 0x04) M(LOOPED, HOLD, FORMAT, DIR, 0x05)                             \
 M(LOOPED, HOLD, FORMAT, DIR, 0x06) M(LOOPED, HOLD, FORMAT, DIR, 0x07)

#define DDMA_FOR_EACH_DIR(M, LOOPED, HOLD, FORMAT)                                                 \
 DDMA_FOR_EACH_DRW(M, LOOPED, HOLD, FORMAT, 0x0) DDMA_FOR_EACH_DRW(M, LOOPED, HOLD, FORMAT, 0x1)

#define DDMA_FOR_EACH_FORMAT(M, LOOPED, HOLD)                                                      \
 DDMA_FOR_EACH_DIR(M, LOOPED, HOLD, 0x0) DDMA_FOR_EACH_DIR(M, LOOPED, HOLD, 0x1)

#define DDMA_FOR_EACH_HOLD(M, LOOPED)                                                              \
 DDMA_FOR_EACH_FORMAT(M, LOOPED, 0x0) DDMA_FOR_EACH_FORMAT(M, LOOPED, 0x1)

DDMA_FOR_EACH_HOLD(DEFINE_DMAInstr, 0)
DDMA_FOR_EACH_HOLD(DEFINE_DMAInstr, 1)

#undef DEFINE_DMAInstr
#undef DDMA_FOR_EACH_HOLD
#undef DDMA_FOR_EACH_FORMAT
#undef DDMA_FOR_EACH_DIR
#undef DDMA_FOR_EACH_DRW

/* Phase-5e: extern "C" so the table-symbol name matches the unmangled
 * scu_dsp_common.inc declaration that the C-side scu_dsp_misc.c sees
 * (DSP_DecodeInstruction inlines a read of this table in the LPS path). */
#ifdef __cplusplus
extern "C" {
#endif
MDFN_HIDE void (*const DSP_DMAFuncTable[2][8][8])(struct DSPS*) =
{
 #include "scu_dsp_dmatab.inc"
};
#ifdef __cplusplus
} /* extern "C" */
#endif

#undef DMAInstr_NAME
#undef DMAInstr_BODY

//
//
//

/* Phase-7f: promoted from file-static so ss_init.c’s InitCommon can call it. */
MDFN_COLD void SCU_Init(void)
{
 SCU_DMA_TimeCounter = 0;
 SCU_DMA_RunUntil = 0;
 IAsserted = 0;
 HB_FromVDP2 = false;
 VB_FromVDP2 = false;

 BBus_SH2_WriteFinishTS = 0;

 DSP_Init();
}

void SCU_Reset(bool powering_up)
{
 ILevel = IVec = 0;
 IMask = 0xBFFF;
 IPending = 0;
 ABusIProhibit = 0;
 ASR0 = 0;
 ASR1 = 0;
 AREF = 0;
 RSEL = 0;
 //
 //
 if(powering_up)
 {
  // TODO: check if only power-on, or reset too.
  Timer0_Counter = 0;
  Timer0_Compare = 0;
  Timer0_Met = false;

  Timer1_Reload = 0;
  Timer1_Counter = 0;
  Timer1_Mode = 0;
  Timer1_Met = false;

  Timer_Enable = false;
 }
 //
 //
 if(powering_up)
  memset(DMALevel, 0x00, sizeof(DMALevel));

 for(unsigned level___ = 0; level___ < 3; level___++)
 {
  DMALevelS* d = &DMALevel[level___];
  d->ReadAdd = true;
  d->WriteAdd = 0x1;

  d->Enable = false;
  d->GoGoGadget = false;
  d->Active = false;

  d->Indirect = false;
  d->ReadUpdate = false;
  d->WriteUpdate = false;
  d->SF = 0x7;

  d->WATable = &dma_write_tab.acb[0][0][0][0][0];
  d->ReadFunc = rftab[0];
  d->TableReadFunc = NULL;
 }
 //SCU_DMA_CycleCounter = 0;
 SCU_DMA_ReadOverhead = 0;
 
 SCU_DMA_SDRAM_Slowdown_Counter = 0;

 SCU_DMA_VDP1WriteIgnoreKludge = 0;

 RecalcDMAHalt();

 DSP_Reset(powering_up);
 //
 //
 SH7095_SetIRL(&CPU[0], ILevel);
}

/* Phase-7d: promoted from file-static -- ss_state.c calls this. */
MDFN_COLD void SCU_StateAction(StateMem* sm, const unsigned load, const bool data_only)
{
 uint32_t DSP_ProgRAM[256];
 uint32_t DSP_NextInstr;
 bool DSP_NextInstrLooped;
 uint32_t DMALevel_WATable[3];	
 uint8_t DMALevel_ReadFunc[3];		// rftab, don't allow NULL
 uint8_t DMALevel_TableReadFunc[3];	// rftab, also allow NULL

 SFORMAT StateRegs[] =
 {
  SFVAR(DSP.LastTS),
  SFVAR(DSP.CycleCounter),
  SFVAR(DSP.T0_Until),
  SFVAR(DSP.State),

  SFVAR(DSP_NextInstr),
  SFVAR(DSP_NextInstrLooped),

  SFVAR(DSP.PC),
  SFVAR(DSP.RA),

  SFVAR(DSP.FlagZ),
  SFVAR(DSP.FlagS),
  SFVAR(DSP.FlagV),
  SFVAR(DSP.FlagC),

  SFVAR(DSP.FlagEnd),

  SFVAR(DSP.TOP),
  SFVAR(DSP.LOP),

  SFVAR(DSP.AC.T),
  SFVAR(DSP.P.T),

  SFPTR8N(&(DSP.CT)[0], (sizeof(DSP.CT) / sizeof(uint8_t)), "DSP.CT"),

  SFVAR(DSP.RX),
  SFVAR(DSP.RY),

  SFVAR(DSP.RAO),
  SFVAR(DSP.WAO),

  SFPTR32N(&(DSP.DataRAM)[0][0], (sizeof(DSP.DataRAM) / sizeof(uint32_t)), "&DSP.DataRAM[0][0]"),

  SFPTR32N(&(DSP_ProgRAM)[0], (sizeof(DSP_ProgRAM) / sizeof(uint32_t)), "DSP_ProgRAM"),

  SFPTR32N(&(DSP.PRAMDMABuf)[0], (sizeof(DSP.PRAMDMABuf) / sizeof(uint32_t)), "DSP.PRAMDMABuf"),
  SFVAR(DSP.PRAMDMABufCount),

  SFVAR(DMALevel->StartReadAddr, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->StartWriteAddr, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->StartByteCount, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->ReadAdd, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->WriteAdd, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->Enable, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->Active, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->GoGoGadget, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->Indirect, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->ReadUpdate, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->WriteUpdate, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->SF, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->FinishTime, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->WriteBus, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->CurReadBase, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->CurReadSub, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->CurWriteAddr, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->CurByteCount, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->Buffer, 3, sizeof(*DMALevel), DMALevel),
  SFPTR32N(&(DMALevel_WATable)[0], (sizeof(DMALevel_WATable) / sizeof(uint32_t)), "DMALevel_WATable"),
  SFPTR8N(&(DMALevel_ReadFunc)[0], (sizeof(DMALevel_ReadFunc) / sizeof(uint8_t)), "DMALevel_ReadFunc"),
  SFPTR8N(&(DMALevel_TableReadFunc)[0], (sizeof(DMALevel_TableReadFunc) / sizeof(uint8_t)), "DMALevel_TableReadFunc"),
  SFVAR(DMALevel->CurTableAddr, 3, sizeof(*DMALevel), DMALevel),
  SFVAR(DMALevel->FinalTransfer, 3, sizeof(*DMALevel), DMALevel),

  SFVAR(SCU_DMA_TimeCounter),
  SFVAR(SCU_DMA_RunUntil),
  SFVAR(SCU_DMA_ReadOverhead),
  SFVAR(SCU_DMA_SDRAM_Slowdown_Counter),
  SFVAR(SCU_DMA_VDP1WriteIgnoreKludge),

  SFVAR(BBus_SH2_WriteFinishTS),

  SFVAR(IAsserted),
  SFVAR(IPending),
  SFVAR(IMask),

  SFVAR(ABusIProhibit),
  SFVAR(ASR0),
  SFVAR(ASR1),
  SFVAR(AREF),
  SFVAR(RSEL),

  SFVAR(ILevel),
  SFVAR(IVec),

  SFVAR(Timer0_Counter),
  SFVAR(Timer0_Compare),
  SFVAR(Timer0_Met),

  SFVAR(Timer1_Reload),
  SFVAR(Timer1_Counter),
  SFVAR(Timer1_Mode),
  SFVAR(Timer1_Met),

  SFVAR(Timer_Enable),

  SFVAR(HB_FromVDP2),
  SFVAR(VB_FromVDP2),

  SFEND
 };

 //
 // Do unconditionally, and not just if !load(in case some vars are missing from save state).
 //
 for(unsigned i = 0; i < 256; i++)
  DSP_ProgRAM[i] = DSP.ProgRAM[i] >> 32;

 DSP_NextInstr = DSP.NextInstr >> 32;
 DSP_NextInstrLooped = (DSP.NextInstr == DSP_DecodeInstruction(DSP.NextInstr >> 32, true));
 //
 //
 const DMAWriteTabS* const dwt_ptrs[2] = { (const DMAWriteTabS*)dma_write_tab.acb, (const DMAWriteTabS*)dma_write_tab.aciv1 };
 const size_t dwt_counts[2] = { sizeof(dma_write_tab.acb) / sizeof(DMAWriteTabS), sizeof(dma_write_tab.aciv1) / sizeof(DMAWriteTabS) };

 for(unsigned level = 0; level < 3; level++)
 {
  const DMALevelS* d = &DMALevel[level];

  if(d->WATable >= dwt_ptrs[0] && d->WATable < (dwt_ptrs[0] + dwt_counts[0]))
   DMALevel_WATable[level] = d->WATable - dwt_ptrs[0];
  else if(d->WATable >= dwt_ptrs[1] && d->WATable < (dwt_ptrs[1] + dwt_counts[1]))
   DMALevel_WATable[level] = 0x80000000 | (d->WATable - dwt_ptrs[1]);
  else
   abort();	// FIXME: NULL pointer on init...

  for(unsigned rb = 0; rb < 3 + 1; rb++)
  {
   if(rb >= 3)
    abort();

   if(d->ReadFunc == rftab[rb])
   {
    DMALevel_ReadFunc[level] = rb;
    break;
   }
  }

  DMALevel_TableReadFunc[level] = 0xFF;
  for(unsigned trb = 0; trb < 3; trb++)
  {
   if(d->TableReadFunc == rftab[trb])
   {
    DMALevel_TableReadFunc[level] = trb;
    break;
   }
  }
 }
 //
 //
 //
 if(load)
 {
  // For old save states that didn't save these variables.
  ASR0 = 0x03301FF0;
  ASR1 = 0x10001FF0;
  AREF = 0x13;
  RSEL = 0x1;
 }

 MDFNSS_StateAction(sm, load, data_only, StateRegs, "SCU", false);

 if(load)
 {
  ILevel &= 0xF;
  //
  //
  for(unsigned i = 0; i < 256; i++)
   DSP.ProgRAM[i] = DSP_DecodeInstruction(DSP_ProgRAM[i], false);

  if(DSP_NextInstrLooped)
   DSP.NextInstr = DSP_DecodeInstruction(DSP_NextInstr, true);
  else
   DSP.NextInstr = DSP_DecodeInstruction(DSP_NextInstr, false);

  for(unsigned i = 0; i < 4; i++)
   DSP.CT[i] &= 0x3F;

  DSP.LOP &= 0xFFF;

  if(load < 0x00102102)
   DSP.PRAMDMABufCount = 0;
  else
   DSP.PRAMDMABufCount = ((uint32_t)(DSP.PRAMDMABufCount) < (uint32_t)(0x100) ? (uint32_t)(DSP.PRAMDMABufCount) : (uint32_t)(0x100));

  //
  //
  for(unsigned level = 0; level < 3; level++)
  {
   DMALevelS* d = &DMALevel[level];

   d->StartReadAddr &= 0x07FFFFFF;
   d->StartWriteAddr &= 0x07FFFFFF;
   d->StartByteCount &= level ? 0x00000FFF : 0x000FFFFF;

   d->ReadAdd &= 0x1;
   d->WriteAdd &= 0x7;

   d->WriteBus %= 3;
   //
   //
   {
    bool which = DMALevel_WATable[level] >> 31;
    uint32_t index = DMALevel_WATable[level] & 0x7FFFFFFF;

    if(index < dwt_counts[which]) // || !dwt_ptrs[which][index].write_size)
     d->WATable = dwt_ptrs[which] + index;
   }

   if(DMALevel_ReadFunc[level] < 3)
    d->ReadFunc = rftab[DMALevel_ReadFunc[level]];

   if(DMALevel_TableReadFunc[level] == 0xFF)
    d->TableReadFunc = NULL;
   else if(DMALevel_TableReadFunc[level] < 3)
    d->TableReadFunc = rftab[DMALevel_TableReadFunc[level]];
  }
  //
  //
  Timer0_Counter &= 0x1FF;
  Timer0_Compare &= 0x3FF;
  Timer1_Counter &= 0x1FF;
  Timer1_Reload &= 0x1FF;
 }
}



//
//
//
//
//
//
MDFN_COLD uint32_t SCU_DSP_PeekProgRAM(uint8_t A)
{
 return DSP.ProgRAM[A] >> 32;
}

uint32_t SCU_GetRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 uint32_t ret = 0xDEADBEEF;

 switch(id)
 {
  case SCU_GSREG_ILEVEL:
	ret = ILevel;
	break;

  case SCU_GSREG_IVEC:
	ret = IVec;
	break;

  case SCU_GSREG_IASSERTED:
	ret = IAsserted;
	break;

  case SCU_GSREG_IPENDING:
	ret = IPending;
	break;

  case SCU_GSREG_IMASK:
	ret = IMask;
	break;
  //
  //
  //
  case SCU_GSREG_D0MD:
  case SCU_GSREG_D1MD:
  case SCU_GSREG_D2MD:
	{
	 DMALevelS* d = &DMALevel[id - SCU_GSREG_D0MD];

	 ret = (d->Indirect << 24) | (d->ReadUpdate << 16) | (d->WriteUpdate << 8) | (d->SF << 0);
	}
	break;
  //
  //
  //
  case SCU_GSREG_ASR0_CS0:
  case SCU_GSREG_ASR0_CS1:
  case SCU_GSREG_ASR1_CS2:
  case SCU_GSREG_ASR1_CSD:
	{
	 const unsigned v[4] = { (uint16_t)(ASR0 >> 16), (uint16_t)(ASR0 >> 0), (uint16_t)(ASR1 >> 16), (uint16_t)(ASR1 >> 0) };

	 ret = v[id - SCU_GSREG_ASR0_CS0];
	}
	break;

  case SCU_GSREG_AREF:
	ret = AREF;
	break;

  case SCU_GSREG_RSEL:
	ret = RSEL;
	break;
  //
  //
  //
  case SCU_GSREG_T0CNT:
	ret = Timer0_Counter;
	break;

  case SCU_GSREG_T0CMP:
	ret = Timer0_Compare;
	break;

  case SCU_GSREG_T0MET:
	ret = Timer0_Met;
	break;

  case SCU_GSREG_T1RLV:
	ret = Timer1_Reload;
	break;

  case SCU_GSREG_T1CNT:
	ret = Timer1_Counter;
	break;

  case SCU_GSREG_T1MOD:
	ret = Timer1_Mode;
	break;

  case SCU_GSREG_T1MET:
	ret = Timer1_Met;
	break;

  case SCU_GSREG_TENBL:
	ret = Timer_Enable;
	break;

  case SCU_GSREG_DSP_EXEC:
	ret = (bool)(DSP.State & STATE_MASK_EXECUTE);
	break;

  case SCU_GSREG_DSP_PAUSE:
	ret = (bool)(DSP.State & STATE_MASK_PAUSE);
	break;

  case SCU_GSREG_DSP_PC:
	ret = DSP.PC;
	break;

  case SCU_GSREG_DSP_END:
	ret = DSP.FlagEnd;
	break;
 }
 return ret;
}

void SCU_SetRegister(const unsigned id, const uint32_t value)
{
 switch(id)
 {
  case SCU_GSREG_IPENDING:
	IPending = value & 0xFFFF3FFF;
	break;

  case SCU_GSREG_IMASK:
	IMask = value & 0xBFFF;
	break;
  //
  //
  case SCU_GSREG_ASR0_CS0:
	ASR0 = (ASR0 & 0x0000FFFF) | ((value & 0xFFFD) << 16);
	break;
  
  case SCU_GSREG_ASR0_CS1:
	ASR0 = (ASR0 & 0xFFFF0000) | ((value & 0xFFFD) <<  0);
	break;

  case SCU_GSREG_ASR1_CS2:
	ASR1 = (ASR1 & 0x0000FFFF) | ((value & 0xF00D) << 16);
	break;

  case SCU_GSREG_ASR1_CSD:
	ASR1 = (ASR1 & 0xFFFF0000) | ((value & 0xFFFD) <<  0);
	break;

  case SCU_GSREG_AREF:
	AREF = value & 0x1F;
	break;

  case SCU_GSREG_RSEL:
	RSEL = value & 0x1;
	break;
  //
  //
  case SCU_GSREG_T0CNT:
	//Timer0_Counter = value & 0x1FF;
	break;

  case SCU_GSREG_T0CMP:
	Timer0_Compare = value & 0x3FF;
	break;

  case SCU_GSREG_T0MET:
	//Timer0_Met = value & 0x1;
	break;

  case SCU_GSREG_T1RLV:
	Timer1_Reload = value & 0x1FF;
	break;

  case SCU_GSREG_T1CNT:
	//Timer1_Counter = value & 0x1FF;
	break;

  case SCU_GSREG_T1MOD:
	Timer1_Mode = value & 0x1;
	break;

  case SCU_GSREG_T1MET:
	//Timer1_Met = value & 0x1;
	break;

  case SCU_GSREG_TENBL:
	Timer_Enable = value & 0x1;
	break;
  //

 }

 if(CheckDoMasterInt())
  SH7095_SetIRL(&CPU[0], ILevel);
}
