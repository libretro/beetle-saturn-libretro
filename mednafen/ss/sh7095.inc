/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sh7095.inc - Hitachi SH7095 Emulation
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
** along with z program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
 Emulation implementation notes/deficiencies:
	z->Cache multiple-way tag collisions (ostensibly an illegal condition) during reads/writes are not handled accurately.

	Overall timing is extremely approximate.

	DMA timing is very rough.  DMA burst mode probably isn't handled totally correctly, especially in
	relation to the other CPU and the SCU(is DMA burst mode even legal on the Saturn?).

	DMA channel bus priority handling is not correct(at least not for non-burst mode transfers).

	Misaligned memory accesses(that cause address errors) aren't emulated correctly(for
	future reference, there's a difference between how cache versus external bus is accessed
	for misaligned addresses, and perhaps a difference between 32-bit and 16-bit spaces as
	defined by the z->BSC too; and then there's misaligned OPM register access semantics...).

	Address errors occur one instruction too soon.

	SLEEP instruction, standby mode, and DMA burst stalls are handled by thoroughly (ab)using the
	IF/ID pipeline emulation and opcode dispatch mechanism.

	Individual module clocking disabling functionality of z->SBYCR isn't handled.

	Interrupts are typically delayed by one instruction, but when the interrupt is handled,
	the current interrupt state is used instead of the previous interrupt state.
	This can result in the exception-handling pseudo-op code being called when there's
	no "current" exception to process, which is a quasi-error condition; there's code to log
	z condition and recover from it.  An SH-2 code sequence that can cause z condition
	if there's currently a pending/unacked interrupt:
		<interrupt-enabling instruction>
		<interrupt-acknowledging instruction>
	This cooouuuld turn out to be a problem, and if it does, it can be fixed by having two interrupt-pending variables,
	with one being copied to the other in DoIDIF().

	Emulation of SH-2 master/slave bus sharing is very approximate.  On a real SH-2, internal bus cycles may affect
	bus grant timing, need to run tests(though emulating that behavior would probably be unrealistic performance-wise).

	The SCU C-bus DMA z->ExtHalt kludge doesn't work totally as intended when full cache emulation is enabled(and the
	interruptible/resumable slave CPU code is used), but it proooobably won't be a problem in practice because on
	a real Saturn, after the write that triggers the SCU DMA, a few CPU bus accesses can get in before the SCU hogs the bus.

	z->SCI, UBC, and z->BSC are mostly unemulated.
*/

/* TODO:
	Check address error exception order, sleep versus exceptions.

	Make sure SH7095_RecalcPendingIntPEX(z) is called in all places it needs to be called(which is arguably too many...).
*/

#ifdef MSB_FIRST
 #define NE32ASU8_IDX_ADJ(T, idx) (idx)
 #define SH7095_RBO_BE_U8(base, A) (((const uint8_t*)(base))[(A)])
#else
 #define NE32ASU8_IDX_ADJ(T, idx) ( ((idx) & ~(sizeof(T) - 1)) ^ (4 - (sizeof(T))) )
 #define SH7095_RBO_BE_U8(base, A) (((const uint8_t*)(base))[(A) ^ 1])
#endif

/* Single-load BE u32 read from uint16_t-array storage.  Across the bus
 * the SH7095 wants MSW-at-lower-address (BE word convention), but each
 * uint16_t in the fast-map slot is stored in native byte order.  On LE
 * host a 32-bit native load yields [low(LSW), high(LSW), high(MSW),
 * low(MSW)] packed as a uint32_t = LSW<<16 | MSW; rotate-by-16 swaps
 * the halves to give the desired MSW<<16 | LSW.  On BE host the native
 * 32-bit load is already MSW<<16 | LSW.  memcpy is the portable
 * single-load idiom: strict-aliasing safe (memcpy is exempt) and
 * alignment-safe (the compiler emits a single 32-bit load on host
 * arches that permit unaligned access -- x86, ARMv7+, modern MIPS --
 * and falls back to byte-by-byte without a fault elsewhere).  The
 * SH7095's 16-bit bus guarantees the incoming A is 2-byte aligned for
 * word/longword accesses (address-error exception elsewhere covers
 * misuse); the `A & ~1u` masks off the within-word bit for safety. */
/* Phase-9 step 4: forward declarations for free functions defined
 * later in this file. */
static void SH7095_RecalcMRWFP_0(SH7095* z);
static void SH7095_RecalcMRWFP_1_7(SH7095* z);
static void SH7095_DMA_RecalcRunning(SH7095* z);
static void SH7095_FRT_WDT_Recalc_NET(SH7095* z);
static void SH7095_FRT_WDT_Update(SH7095* z);
static void SH7095_INTC_Reset(SH7095* z);
static void SH7095_RecalcPendingIntPEX(SH7095* z);
static void SH7095_SCI_Reset(SH7095* z);
static void SH7095_WDT_Reset(SH7095* z, bool from_internal_wdt);
static void SH7095_WDT_StandbyReset(SH7095* z);
static void SH7095_FRT_Reset(SH7095* z);
static void SH7095_FRT_CheckOCR(SH7095* z);
static void SH7095_FRT_ClockFRC(SH7095* z);
static void SH7095_Cache_AssocPurge(SH7095* z, const uint32_t A);
static int  SH7095_Cache_FindWay(SH7095* z, SH7095_CacheEntry* const cent, const uint32_t ATM);
static uint8_t  SH7095_BSC_BusRead_u8 (SH7095* z, uint32_t A, const bool BurstHax, int32_t* SH2DMAHax);
static uint16_t SH7095_BSC_BusRead_u16(SH7095* z, uint32_t A, const bool BurstHax, int32_t* SH2DMAHax);
static uint32_t SH7095_BSC_BusRead_u32(SH7095* z, uint32_t A, const bool BurstHax, int32_t* SH2DMAHax);
static void SH7095_BSC_BusWrite_u8 (SH7095* z, uint32_t A, uint8_t V, const bool BurstHax, int32_t* SH2DMAHax);
static void SH7095_BSC_BusWrite_u16(SH7095* z, uint32_t A, uint16_t V, const bool BurstHax, int32_t* SH2DMAHax);
static void SH7095_BSC_BusWrite_u32(SH7095* z, uint32_t A, uint32_t V, const bool BurstHax, int32_t* SH2DMAHax);
static void SH7095_OnChipRegWrite_u8(SH7095* z, uint32_t A, uint32_t V);
static void SH7095_OnChipRegWrite_u16(SH7095* z, uint32_t A, uint32_t V);
static void SH7095_OnChipRegWrite_u32(SH7095* z, uint32_t A, uint32_t V);
static uint8_t  SH7095_Cache_ReadDataArray_u8 (SH7095* z, uint32_t A);
static uint16_t SH7095_Cache_ReadDataArray_u16(SH7095* z, uint32_t A);
static uint32_t SH7095_Cache_ReadDataArray_u32(SH7095* z, uint32_t A);
static uint32_t SH7095_Cache_ReadAddressArray (SH7095* z, uint32_t A);
static void SH7095_DIVU_S32_S32(SH7095* z);
static void SH7095_DIVU_S64_S32(SH7095* z);
static void SH7095_DMA_DoTransfer(SH7095* z, unsigned ch);
static void SH7095_DMA_StartSG(SH7095* z);
static bool SH7095_DMA_InBurst(SH7095* z);
static bool SH7095_DMA_RunCond(SH7095* z, unsigned ch);
static void SH7095_DMA_CheckEnterBurstHack(SH7095* z);
static void SH7095_SetCCR(SH7095* z, uint8_t V);
// SH7095_SetIRL is declared extern in sh7095.h since scu.inc calls it.
static void SH7095_SetFTCI(SH7095* z, bool state);
static void SH7095_StateAction_SlaveResume(SH7095* z, StateMem* sm, const unsigned load, const bool data_only, const char* sname);
static void SH7095_RunSlaveUntil_Debug(SH7095* z, const sscpu_timestamp_t bound_timestamp);
static void SH7095_SetRegister(SH7095* z, const unsigned id, const uint32_t value);
static void SH7095_DoIDIF_INLINE_C0_I0(SH7095* z);
static void SH7095_DoIDIF_INLINE_C0_I1(SH7095* z);
static void SH7095_DoIDIF_INLINE_C1_I0(SH7095* z);
static void SH7095_DoIDIF_INLINE_C1_I1(SH7095* z);
static void SH7095_DoIDIF_NI_C0_I0(SH7095* z);
static void SH7095_DoIDIF_NI_C0_I1(SH7095* z);
static void SH7095_DoIDIF_NI_C1_I0(SH7095* z);
static void SH7095_DoIDIF_NI_C1_I1(SH7095* z);
static void SH7095_ExtBusWrite_INLINE_SP0_u8(SH7095* z, uint32_t A, uint8_t V);
static void SH7095_ExtBusWrite_INLINE_SP0_u16(SH7095* z, uint32_t A, uint16_t V);
static void SH7095_ExtBusWrite_INLINE_SP0_u32(SH7095* z, uint32_t A, uint32_t V);
static void SH7095_ExtBusWrite_INLINE_SP1_u8(SH7095* z, uint32_t A, uint8_t V);
static void SH7095_ExtBusWrite_INLINE_SP1_u16(SH7095* z, uint32_t A, uint16_t V);
static void SH7095_ExtBusWrite_INLINE_SP1_u32(SH7095* z, uint32_t A, uint32_t V);
static void SH7095_Cache_WriteUpdate_u16(SH7095* z, uint32_t A, uint16_t V);
static void SH7095_Cache_WriteUpdate_u32(SH7095* z, uint32_t A, uint32_t V);
static void SH7095_Cache_WriteDataArray_u8(SH7095* z, uint32_t A, uint8_t V);
static void SH7095_Cache_WriteDataArray_u16(SH7095* z, uint32_t A, uint16_t V);
static void SH7095_Cache_WriteDataArray_u32(SH7095* z, uint32_t A, uint32_t V);
static void SH7095_Cache_WriteAddressArray(SH7095* z, uint32_t A, uint32_t V);

static INLINE uint32_t SH7095_RBO_BE_U32(const void* p)
{
 uint32_t v;
 memcpy(&v, p, 4);
#ifdef MSB_FIRST
 return v;
#else
 return (v << 16) | (v >> 16);
#endif
}

/* Compile-time guard: SH7095_RBO_BE_T below dispatches on sizeof(T)
 * with the uint32_t case as the ternary tail.  Without z guard a
 * stray SH7095_RBO_BE_T(uint64_t, ...) would silently fall into the
 * uint32_t arm and zero-extend.  The `char[1 - 2*!cond]` trick is
 * portable C90 (no _Static_assert, no __builtin_*) -- a non-positive
 * array dimension is a hard compile error.  Sized-as-void via cast,
 * fits inside the comma expression below. */
#define SH7095_CT_CHECK_T(T) ((void)sizeof(char[1 - 2*!(   \
        sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4)]))

/* SH7095_RBO_BE_T: BE-bus read of type T (uint8_t/16/32) over a
 * uint16_t-array fast-map slot.  Replaces the old ne16_rbo_be<T>
 * template call inside the MemRead macro and the z->IBuffer prefetch.
 * sizeof(T) is a compile-time constant per call site, so all but the
 * matching ternary branch fold out at every -O level (verified: zero
 * cmp/test/je/jmp instructions in the emitted code at -O0/-O1/-O2/-Os).
 * The u32 path uses SH7095_RBO_BE_U32's single 32-bit load + word
 * rotate -- ~4 fewer host insns per fetch than the prior two-uint16_t
 * loads + shift-or combine.  Hot on the z->IBuffer prefetch path (every
 * SH-2 sequential instruction fetch boundary).  CSE handles the
 * multi-mention of (base) and (A) at the current callsites (verified
 * against `SH7095_FastMap[A >> N], A` -- single FastMap load in the
 * emitted asm). */
#define SH7095_RBO_BE_T(T, base, A) \
   ( SH7095_CT_CHECK_T(T), \
     (sizeof(T) == 1) ? (T)SH7095_RBO_BE_U8((base), (A)) : \
     (sizeof(T) == 2) ? (T)((const uint16_t*)(base))[(A) >> 1] : \
     (T)SH7095_RBO_BE_U32((const uint8_t*)(base) + ((A) & ~1u)) )

void SH7095_Construct(SH7095* z, const char* const name_arg, const unsigned event_id_dma_arg, uint8_t (*exivecfn_arg)(void))
{
 z->event_id_dma  = event_id_dma_arg;
 z->ExIVecFetch   = exivecfn_arg;
 z->cpu_name      = name_arg;
 z->CBH_Setting   = false;
 z->EIC_Setting   = false;
 z->DM_Setting    = false;

 if(z == &CPU[1])
 {
    z->resume_id = 0;
 }

 SH7095_Init(z, false, false);
}

#include "sh7095_mem_dispatch_proto.inc"
// WARNING: Template arguments CacheEnabled and CacheBypassHack are only valid for region==0.
static void SH7095_RecalcMRWFP_0(SH7095* z)
{
 #define MAHL_(bs,w,nsc,cbh) \
		 if(z->CCR & SH7095_CCR_CE) { \
		  if(bs == 16) { z->MRFP16_I[0] = C_MemReadRT_w##w##_n##nsc##_c##cbh##_u16_r0_e1_im1; } \
		  if(bs == 32) { z->MRFP32_I[0] = C_MemReadRT_w##w##_n##nsc##_c##cbh##_u32_r0_e1_im1; } \
		  z->MRFP##bs[0] = C_MemReadRT_w##w##_n##nsc##_c##cbh##_u##bs##_r0_e1_i0; \
		  z->MRFPI[0]    = C_MemReadRT_w##w##_n##nsc##_c##cbh##_u32_r0_e1_i1; \
		  z->MWFP##bs[0] = C_MemWriteRT_w##w##_n##nsc##_u##bs##_r0_e1; \
		 } else { \
		  if(bs == 16) { z->MRFP16_I[0] = C_MemReadRT_w##w##_n##nsc##_c##cbh##_u16_r0_e0_i0; } \
		  if(bs == 32) { z->MRFP32_I[0] = C_MemReadRT_w##w##_n##nsc##_c##cbh##_u32_r0_e0_i0; } \
		  z->MRFP##bs[0] = C_MemReadRT_w##w##_n##nsc##_c##cbh##_u##bs##_r0_e0_i0; \
		  z->MRFPI[0]    = C_MemReadRT_w##w##_n##nsc##_c##cbh##_u32_r0_e0_i1; \
		  z->MWFP##bs[0] = C_MemWriteRT_w##w##_n##nsc##_u##bs##_r0_e0; \
		 }

 #define MAHL(w, nsc, cbh)		\
	{				\
	 MAHL_( 8, w, nsc, cbh)		\
	 MAHL_(16, w, nsc, cbh)		\
	 MAHL_(32, w, nsc, cbh) 	\
	}

 if(z == &CPU[0])
 {
  if(z->EIC_Setting)
  {
   if(z->DM_Setting)
    MAHL(0, m1, f)
   else
    MAHL(0, t, f)
  }
  else if(z->CBH_Setting)
  {
   MAHL(0, f, t)
  }
  else
  {
   MAHL(0, f, f)
  }
 }
 else
 {
  if(!z->EIC_Setting && z->CBH_Setting)
  {
   MAHL(1, f, t)
  }
  else
  {
   MAHL(1, f, f)
  }
 }
 #undef MAHL_
 #undef MAHL
}

static void SH7095_RecalcMRWFP_1_7(SH7095* z)
{
 #define MAHL_P(w, nsc, region) { \
		  z->MRFP8[region]    = C_MemReadRT_w##w##_n##nsc##_cf_u8_r##region##_e0_i0; \
		  z->MRFP16[region]   = C_MemReadRT_w##w##_n##nsc##_cf_u16_r##region##_e0_i0; \
		  z->MRFP32[region]   = C_MemReadRT_w##w##_n##nsc##_cf_u32_r##region##_e0_i0; \
		  z->MRFP16_I[region] = C_MemReadRT_w##w##_n##nsc##_cf_u16_r##region##_e0_i0; \
		  z->MRFP32_I[region] = C_MemReadRT_w##w##_n##nsc##_cf_u32_r##region##_e0_i0; \
		  z->MRFPI[region]    = C_MemReadRT_w##w##_n##nsc##_cf_u32_r##region##_e0_i1; \
		  z->MWFP8[region]    = C_MemWriteRT_w##w##_n##nsc##_u8_r##region##_e0; \
		  z->MWFP16[region]   = C_MemWriteRT_w##w##_n##nsc##_u16_r##region##_e0; \
		  z->MWFP32[region]   = C_MemWriteRT_w##w##_n##nsc##_u32_r##region##_e0; \
		  }

 #define MAHL(nsc, region)					\
		  if(z == &CPU[0])				\
		  { MAHL_P(0, nsc, region) }			\
		  else						\
		  { MAHL_P(1, f, region) }

 if(z->EIC_Setting)
 {
  if(z->DM_Setting)
  {
   MAHL(m1, 1)
  }
  else
  {
   MAHL(t, 1)
  }
  MAHL(f, 2)
  MAHL(f, 3)
  MAHL(f, 4)
  MAHL(f, 5)
  MAHL(f, 6)
  MAHL(f, 7)
 }
 else
 {
  MAHL(f, 1)
  MAHL(f, 2)
  MAHL(f, 3)
  MAHL(f, 4)
  MAHL(f, 5)
  MAHL(f, 6)
  MAHL(f, 7)
 }

 #undef MAHL
 #undef MAHL_P
}

void SH7095_Init(SH7095* z, const bool EmulateICache, const bool CacheBypassHack)
{
 z->CBH_Setting = CacheBypassHack;
 z->EIC_Setting = EmulateICache;
 SH7095_RecalcMRWFP_0(z);
 SH7095_RecalcMRWFP_1_7(z);
 //
 //
 //
 //
 // Initialize variables that won't be initialized elsewhere since they reflect the overall emulator timing state, or are cache variables
 // for signal inputs.
 //
 z->timestamp = 0;

 z->FRT.FTI = false;
 z->FRT.FTCI = false;
 z->IRL = 0;
 z->NMILevel = false;
 z->BSC.BCR1 &= 0x7FFF; //MD5Level = false;
 z->ExtHalt = false;
 z->ExtHaltDMA = false;

 z->resume_id = 0;

 SH7095_TruePowerOn(z);
}

//
// Initialize everything for determinism, especially state left "undefined" by reset/power exception handling.
//
void SH7095_TruePowerOn(SH7095* z)
{
 for(unsigned i = 0; i < 16; i++)
  z->R[i] = 0;

 for(unsigned i = 0; i < 16; i++)
  z->WB_until[i] = z->timestamp;

 z->UCRead_IF_Kludge = 0;

 z->MA_until = z->timestamp;
 z->MM_until = z->timestamp;
 z->write_finish_timestamp = z->timestamp;

 z->PC = 0;

 z->SR = 0;
 z->GBR = 0;
 z->VBR = 0;

 z->MACH = 0;
 z->MACL = 0;
 z->PR = 0;

 z->EPending = 0;
 z->Pipe_ID = 0;
 z->Pipe_IF = 0;
 z->IBuffer = 0;

 z->PC_IF = z->PC_ID = 0;

 memset(z->Cache, 0, sizeof(z->Cache));
 memset(z->Cache_LRU, 0, sizeof(z->Cache_LRU));
 z->CCRC_Replace_OR[0] = 0;
 z->CCRC_Replace_OR[1] = 0;
 z->CCRC_Replace_AND = 0;
 z->CCR = 0;
 //
 //
 //
 z->resume_id = 0;
 z->Resume_cent = NULL;
 z->Resume_instr = 0;
 z->Resume_way_match = 0;
 z->Resume_uint8_A = 0;
 z->Resume_uint16_A = 0;
 z->Resume_uint32_A = 0;
 z->Resume_unmasked_A = 0;
 z->Resume_uint32_V = 0;
 z->Resume_uint16_V = 0;
 z->Resume_uint8_V = 0;

 z->Resume_MAC_L_m0 = 0;
 z->Resume_MAC_L_m1 = 0;

 z->Resume_MAC_W_m0 = 0;
 z->Resume_MAC_W_m1 = 0;

 z->Resume_ea = 0;
 z->Resume_new_PC = 0;
 z->Resume_new_SR = 0;

 z->Resume_ipr = 0;
 z->Resume_exnum = 0;
 z->Resume_vecnum = 0;
 //
 //
 //
 z->IPRA = 0;
 z->IPRB = 0;
 z->VCRWDT = 0;
 z->VCRA = 0;
 z->VCRB = 0;
 z->VCRC = 0;
 z->VCRD = 0;
 z->ICR = 0;
 //
 //
 //
 z->BSC.BCR1 &= 0x8000;
 z->BSC.BCR2 = 0;
 z->BSC.WCR = 0;
 z->BSC.MCR = 0;

 z->BSC.RTCSR = 0;
 z->BSC.RTCSRM = 0;
 z->BSC.RTCNT = 0;
 z->BSC.RTCOR = 0;

 z->BSC.sdram_finish_time = z->timestamp;
 z->BSC.last_mem_time = z->timestamp;
 z->BSC.last_mem_addr = 0;
 z->BSC.last_mem_type = 0;
 //
 //
 //
 z->resume_id = 0;
 //
 //
 //
 z->SBYCR = 0;
 z->Standby = false;
 //
 //
 //
 z->FRT.lastts = z->timestamp;

 // Don't change z->FRT.FTI and z->FRT.FTCI here.

 z->FRT.FRC = 0;
 z->FRT.OCR[0] = z->FRT.OCR[1] = 0;
 z->FRT.FICR = 0;
 z->FRT.TIER = 0;
 z->FRT.FTCSR = 0;
 z->FRT.FTCSRM = 0;
 z->FRT.TCR = 0;
 z->FRT.TOCR = 0;
 z->FRT.RW_Temp = 0;

 z->FRT_WDT_ClockDivider = 0;

 z->WDT.WTCSR = 0;
 z->WDT.WTCSRM = 0;
 z->WDT.WTCNT = 0;
 z->WDT.RSTCSR = 0;
 z->WDT.RSTCSRM = 0;

 SH7095_FRT_WDT_Recalc_NET(z);
 //
 //
 //
 z->DMA_Timestamp = z->timestamp;
 z->DMA_SGEndTimestamp = z->timestamp;
 z->DMA_RoundRobinRockinBoppin = 0;
 z->DMA_PenaltyKludgeAmount = 0;
 z->DMA_PenaltyKludgeAccum = 0;
 memset(z->DMACH, 0, sizeof(z->DMACH));
 z->DMAOR = 0;
 z->DMAORM = 0;
 SH7095_DMA_RecalcRunning(z);
 //
 //
 //
 z->divide_finish_timestamp = z->timestamp;
 z->DVSR = 0;
 z->DVDNT = 0;
 z->DVDNTH = 0;
 z->DVDNTL = 0;
 z->DVDNTH_Shadow = 0;
 z->DVDNTL_Shadow = 0;
 z->VCRDIV = 0;
 z->DVCR = 0;
 //
 //
 //
 z->SCI.SMR = 0;
 z->SCI.BRR = 0;
 z->SCI.SCR = 0;
 z->SCI.TDR = 0;
 z->SCI.SSR = 0;
 z->SCI.SSRM = 0;
 z->SCI.RDR = 0;
 z->SCI.RSR = 0;
 z->SCI.TSR = 0;
}

void SH7095_SetActive(SH7095* z, bool active)
{
 if(active && z->timestamp == SS_EVENT_DISABLED_TS)
 {
  z->timestamp = SH7095_mem_timestamp;
  z->FRT.lastts = z->timestamp;

  z->DMA_Timestamp = SH7095_mem_timestamp;
  z->DMA_SGEndTimestamp = SH7095_mem_timestamp;

  SS_SetEventNT(&events[z->event_id_dma], SH7095_mem_timestamp + 1);
  //
  SH7095_Reset(z, true, false);
 }
 else if(!active && z->timestamp != SS_EVENT_DISABLED_TS)
 {
  SH7095_Reset(z, true, false);
  //
  z->timestamp = SS_EVENT_DISABLED_TS;
  SS_SetEventNT(&events[z->event_id_dma], SS_EVENT_DISABLED_TS);
 }
}

void SH7095_AdjustTS(SH7095* z, int32_t delta)
{
 if(MDFN_UNLIKELY(z->timestamp == SS_EVENT_DISABLED_TS))
  return;

 z->timestamp += delta;
 z->FRT.lastts += delta;

#define ATSD(v) v = ((sscpu_timestamp_t)(-1000000) > (sscpu_timestamp_t)(v + (delta)) ? (sscpu_timestamp_t)(-1000000) : (sscpu_timestamp_t)(v + (delta)))
 ATSD(z->DMA_Timestamp);
 ATSD(z->DMA_SGEndTimestamp);

 ATSD(z->MA_until);
 ATSD(z->MM_until);

 for(unsigned i = 0; i < 16; i++)
  ATSD(z->WB_until[i]);

 ATSD(z->write_finish_timestamp);
 ATSD(z->divide_finish_timestamp);

 ATSD(z->BSC.sdram_finish_time);
 ATSD(z->BSC.last_mem_time);
#undef ATSD

 z->FRT_WDT_ClockDivider &= 0x00FFFFFF;
 SH7095_FRT_WDT_Recalc_NET(z);
}

// de=1, dme=1, te=0, nmif=0, ae=0
static INLINE bool SH7095_DMA_RunCond(SH7095* z, unsigned ch)
{
 return ((z->DMAOR & 0x07) == 0x01) && ((z->DMACH[ch].CHCR & 0x03) == 0x01);
}

static bool SH7095_DMA_InBurst(SH7095* z)
{
 if((z->DMAOR & 0x08) && SH7095_DMA_RunCond(z, 0) && SH7095_DMA_RunCond(z, 1))
  return ((z->DMACH[0].CHCR | z->DMACH[1].CHCR) & 0x10);

 if(SH7095_DMA_RunCond(z, 0))
  return (z->DMACH[0].CHCR & 0x10);
 else if(SH7095_DMA_RunCond(z, 1))
  return (z->DMACH[1].CHCR & 0x10);

 return false;
}

static void SH7095_DMA_CheckEnterBurstHack(SH7095* z)
{
 if(SH7095_DMA_InBurst(z))
  SH7095_SetPEX(z, SH7095_PEX_PSEUDO_DMABURST);
}

// SH7095_RecalcPendingIntPEX(z) will be called higher up, at the end of SH7095_DMA_Update(z)
//
// Call SH7095_Bus* directly instead of through ExtBusRead, at least until we can work
// out all z disparate z->timestamp nonsense properly(maybe around the time we add proper bus controller emulation? ;)).
//
static INLINE void SH7095_DMA_DoTransfer(SH7095* z, unsigned ch)
{
 static const int8_t ainc[3][4] =
 {
  { 0, 1, -1, -1 },
  { 0, 2, -2, -2 },
  { 0, 4, -4, -4 },
 };
 const unsigned ts = (z->DMACH[ch].CHCR >> 10) & 3;
 const unsigned sm = (z->DMACH[ch].CHCR >> 12) & 3;
 const unsigned dm = (z->DMACH[ch].CHCR >> 14) & 3;
 uint32_t sar = z->DMACH[ch].SAR;
 uint32_t dar = z->DMACH[ch].DAR;
 uint32_t tcr = z->DMACH[ch].TCR;

 switch(ts)
 {
  case 0x00:	// 8-bit
	{
	 uint8_t buffer;

	 buffer = SH7095_BSC_BusRead_u8(z, sar & 0x07FFFFFF, false, &z->DMA_Timestamp);
	 SH7095_BSC_BusWrite_u8(z, dar & 0x07FFFFFF, buffer, false, &z->DMA_Timestamp);

	 sar += ainc[0][sm];
 	 dar += ainc[0][dm];
	 tcr = (tcr - 1) & 0xFFFFFF;
	}
	break;

  case 0x01:	// 16-bit
	{
	 uint16_t buffer;

	 buffer = SH7095_BSC_BusRead_u16(z, sar & 0x07FFFFFE, false, &z->DMA_Timestamp);
	 SH7095_BSC_BusWrite_u16(z, dar & 0x07FFFFFE, buffer, false, &z->DMA_Timestamp);

	 if(MDFN_UNLIKELY((sar | dar) & 0x1))
	 {
	  z->DMAOR |= 4;
	  z->DMAORM |= 4;
	  SH7095_DMA_RecalcRunning(z);
	  SH7095_SetPEX(z, SH7095_PEX_DMAADDR);
	 }

	 sar += ainc[1][sm];
 	 dar += ainc[1][dm];
	 tcr = (tcr - 1) & 0xFFFFFF;
	}
	break;

  case 0x02:	// 32-bit
	{
	 uint32_t buffer;

	 buffer = SH7095_BSC_BusRead_u32(z, sar & 0x07FFFFFC, false, &z->DMA_Timestamp);
	 SH7095_BSC_BusWrite_u32(z, dar & 0x07FFFFFC, buffer, false, &z->DMA_Timestamp);

	 if(MDFN_UNLIKELY((sar | dar) & 0x3))
	 {
	  z->DMAOR |= 4;
	  z->DMAORM |= 4;
          SH7095_DMA_RecalcRunning(z);
	  SH7095_SetPEX(z, SH7095_PEX_DMAADDR);
	 }

	 sar += ainc[2][sm];
 	 dar += ainc[2][dm];
	 tcr = (tcr - 1) & 0xFFFFFF;
	}
	break;

  case 0x03:	// 4 * 32-bit, a mess...
	{
	 uint32_t buffer[4];

	 if(MDFN_UNLIKELY((sar | dar) & 0x3))
	 {
	  z->DMAOR |= 4;
	  z->DMAORM |= 4;
	  SH7095_DMA_RecalcRunning(z);
	  SH7095_SetPEX(z, SH7095_PEX_DMAADDR);
	 }

	 for(unsigned i = 0; i < 4; i++)
	 {
	  buffer[i] = SH7095_BSC_BusRead_u32(z, (sar + (i << 2)) & 0x07FFFFFC, (bool)i, &z->DMA_Timestamp);
	 }

	 sar += 0x10;

	 for(unsigned i = 0; i < 4; i++)
	 {
	  SH7095_BSC_BusWrite_u32(z, dar & 0x07FFFFFC, buffer[i], false, &z->DMA_Timestamp);
	  dar += ainc[2][dm];
	  tcr = (tcr - 1) & 0xFFFFFF;
	  if(MDFN_UNLIKELY(!tcr))
	   break;
	 }
	}
	break;
 }

 if(!tcr)
 {
  z->DMACH[ch].CHCR |= 2;
  z->DMACH[ch].CHCRM |= 2;
  SH7095_DMA_RecalcRunning(z);
 }

 z->DMACH[ch].SAR = sar;
 z->DMACH[ch].DAR = dar;
 z->DMACH[ch].TCR = tcr;
}

sscpu_timestamp_t SH7095_DMA_Update(SH7095* z, sscpu_timestamp_t et)
{

 if(MDFN_UNLIKELY(et < z->DMA_Timestamp))
 {
  // may happen...look into it.
  return z->DMA_Timestamp + 32;
 }
 else if(MDFN_UNLIKELY(z->ExtHaltDMA))
 {
  z->DMA_Timestamp = et;
  return z->DMA_Timestamp + 128;
 }
 else
 {
  bool rr = z->DMA_RoundRobinRockinBoppin;

  if(z->DMAOR & 0x08) 	// Round robin
  {
   while(SH7095_DMA_RunCond(z, 0) || SH7095_DMA_RunCond(z, 1))
   {
    if(SH7095_DMA_RunCond(z, rr))
    {
     if(z->DMA_Timestamp >= et)
      goto TimeOver;

     SH7095_DMA_DoTransfer(z, rr);
    }
    rr = !rr;
   }
  }
  else	// ch 0 > ch1
  {
   while(SH7095_DMA_RunCond(z, 0))
   {
    if(z->DMA_Timestamp >= et)
     goto TimeOver;

    SH7095_DMA_DoTransfer(z, 0);
   }

   while(SH7095_DMA_RunCond(z, 1))
   {
    if(z->DMA_Timestamp >= et)
     goto TimeOver;

    SH7095_DMA_DoTransfer(z, 1);
   }
  }
  //
  //
  //
  TimeOver:;

  z->DMA_RoundRobinRockinBoppin = rr;
  if((et - 128) > z->DMA_Timestamp) z->DMA_Timestamp = (et - 128);

  SH7095_DMA_CheckEnterBurstHack(z);
  SH7095_RecalcPendingIntPEX(z);	// TODO: conditionalize(or make SH7095_RecalcPendingIntPEX(z) less expensive).
 }

 return et + ((et < z->DMA_SGEndTimestamp) ? 32 : 128);
}

// SH7095_DMA_StartSG(z) must be paired with a SH7095_DMA_Update(z, SH7095_mem_timestamp) somewhere before.
static void SH7095_DMA_StartSG(SH7095* z)
{
 z->DMA_SGEndTimestamp = z->DMA_Timestamp + 192;
 SS_SetEventNT(&events[z->event_id_dma], SH7095_mem_timestamp + 32);	// fixed + 32

 SH7095_DMA_CheckEnterBurstHack(z);
}

// Must be called after z->DMACH[0].CHCR or z->DMACH[1].CHCR or z->DMAOR changes.
//
// If SH7095_DMA_RunCond(z, ...) returns false, SAR and DAR and TCR for that channel
// should not be used(due to caching of variables in SH7095_DMA_DoTransfer(z)).
static NO_INLINE void SH7095_DMA_RecalcRunning(SH7095* z)
{
 z->DMA_PenaltyKludgeAmount = 0;

 for(unsigned ch = 0; ch < 2; ch++)
 {
  if(SH7095_DMA_RunCond(z, ch))
  {
   const int32_t npka = (((z->DMACH[ch].CHCR >> 10) & 3) == 3 && (z->DMACH[ch].SAR & z->DMACH[ch].DAR & 0x06000000) != 0x06000000) ? 23 : 19;

   if((npka) > z->DMA_PenaltyKludgeAmount) z->DMA_PenaltyKludgeAmount = (npka);
  }
 }

#if 1
 if(MDFN_UNLIKELY(ss_horrible_hacks & HORRIBLEHACK_NOSH2DMAPENALTY))
  z->DMA_PenaltyKludgeAmount = 0;
#endif
 //
 //
 //
}

INLINE void SH7095_DMA_BusTimingKludge(SH7095* z)
{
 z->timestamp += z->DMA_PenaltyKludgeAccum;
 z->DMA_PenaltyKludgeAccum = 0;
}

//
//
//
static NO_INLINE void SH7095_FRT_Reset(SH7095* z)
{
 z->FRT.FRC = 0x00;
 z->FRT.OCR[0] = z->FRT.OCR[1] = 0x00;
 z->FRT.FICR = 0x00;
 z->FRT.TIER = 0x00;
 z->FRT.FTCSR = 0x00;
 z->FRT.FTCSRM = 0x00;
 z->FRT.TCR = 0x00;
 z->FRT.TOCR = 0x00;
 z->FRT.RW_Temp = 0x00;	// SH7095_Reset_TBD_ or not?

 SH7095_FRT_WDT_Recalc_NET(z);
 SH7095_RecalcPendingIntPEX(z);
}

static INLINE void SH7095_FRT_CheckOCR(SH7095* z)
{
 if(z->FRT.FRC == z->FRT.OCR[0]) // OCRA
 {
  if(z->FRT.FTCSR & 0x0001)
   z->FRT.FRC = 0;

  if(!(z->FRT.FTCSR & 0x08))
  {
   z->FRT.FTCSR |= 0x08;
   z->FRT.FTCSRM |= 0x08;
   SH7095_RecalcPendingIntPEX(z);
  }
 }

 if(z->FRT.FRC == z->FRT.OCR[1]) // OCRB
 {
  if(!(z->FRT.FTCSR & 0x04))
  {
   z->FRT.FTCSR |= 0x04;
   z->FRT.FTCSRM |= 0x04;
   SH7095_RecalcPendingIntPEX(z);
  }
 }
}

static INLINE void SH7095_FRT_ClockFRC(SH7095* z)
{
 z->FRT.FRC++;
 if(!z->FRT.FRC)
 {
  if(!(z->FRT.FTCSR & 0x02))
  {
   z->FRT.FTCSR |= 0x02;	// OVF
   z->FRT.FTCSRM |= 0x02;
   SH7095_RecalcPendingIntPEX(z);
  }
 }
 //
 //
 //
 SH7095_FRT_CheckOCR(z);
}

static const uint8_t wdt_cstab[8] = { 1, /**/ 6, 7, 8, 9, 10, /**/ 12, 13 };

//
// Call after:
//	z->WDT.WTCSR, z->WDT.WTCNT, z->FRT.TCR, z->FRT.OCR[0], z->FRT.OCR[1] changes due to register write or similar.
//	z->timestamp >= z->FRT_WDT_NextTS (after call to SH7095_FRT_WDT_Update(z))
//
static void SH7095_FRT_WDT_Recalc_NET(SH7095* z)
{
 int32_t rt = 1000;

 if((z->FRT.TCR & 0x3) != 0x3)	// when == 3, count on rising edge of external clock(not handled here).
 {
  const uint32_t frt_clockshift = 3 + ((z->FRT.TCR & 0x3) << 1);	// /8, /32, /128, count at falling edge
  int32_t next_frc = 0x10000;

  if(z->FRT.OCR[0] > z->FRT.FRC)
   next_frc = z->FRT.OCR[0];

  if(z->FRT.OCR[1] > z->FRT.FRC)
   next_frc = z->FRT.OCR[1];

  rt = ((next_frc - z->FRT.FRC) << frt_clockshift) - (z->FRT_WDT_ClockDivider & ((1 << frt_clockshift) - 1));
 }

 if(z->WDT.WTCSR & 0x28)	// TME(0x20) and internal use standby NMI recover bit(0x08)
 {
  const unsigned wdt_clockshift = wdt_cstab[z->WDT.WTCSR & 0x7];
  int32_t wdt_rt;

  wdt_rt = ((0x100 - z->WDT.WTCNT) << wdt_clockshift) - (z->FRT_WDT_ClockDivider & ((1 << wdt_clockshift) - 1));
  if((wdt_rt) < rt) rt = (wdt_rt);
 }

 assert(rt > 0);

 z->FRT_WDT_NextTS = z->timestamp + rt;
}

static void SH7095_FRT_WDT_Update(SH7095* z)
{
 assert(z->timestamp >= z->FRT.lastts);

 uint32_t clocks = z->timestamp - z->FRT.lastts;

 z->FRT.lastts = z->timestamp;

 //
 //
 //
 const uint32_t PreAddCD = z->FRT_WDT_ClockDivider;
 z->FRT_WDT_ClockDivider += clocks;

 if((z->FRT.TCR & 0x3) != 0x3)	// when == 3, count on rising edge of external clock(not handled here).
 {
  const uint32_t frt_clockshift = 3 + ((z->FRT.TCR & 0x3) << 1);	// /8, /32, /128, count at falling edge
  uint32_t divided_clocks = (z->FRT_WDT_ClockDivider >> frt_clockshift) - (PreAddCD >> frt_clockshift);

  while(divided_clocks-- > 0)
  {
   SH7095_FRT_ClockFRC(z);
  }
 }

 // z->WDT:
 if(z->WDT.WTCSR & 0x28)	// TME(0x20) and internal use standby NMI recover bit(0x08)
 {
  const unsigned wdt_clockshift = wdt_cstab[z->WDT.WTCSR & 0x7];
  uint32_t divided_clocks = (z->FRT_WDT_ClockDivider >> wdt_clockshift) - (PreAddCD >> wdt_clockshift);
  uint32_t tmp_counter = z->WDT.WTCNT;

  tmp_counter += divided_clocks;
  z->WDT.WTCNT = tmp_counter;
  //
  //
  if(MDFN_UNLIKELY(tmp_counter >= 0x100))
  {
   if(MDFN_UNLIKELY(z->WDT.WTCSR & 0x08))
   {
    z->Standby = false;
    z->WDT.WTCNT = 0x00;
    z->WDT.WTCSR &= ~0x08;
   }
   else if(MDFN_UNLIKELY(z->WDT.WTCSR & 0x40))	// Watchdog timer mode
   {
    z->WDT.RSTCSR |= 0x80;

    z->WDT.WTCNT = 0;
    z->WDT.WTCSR = 0;

    if(z->WDT.RSTCSR & 0x40)	// RSTE
     SH7095_Reset(z, !(z->WDT.RSTCSR & 0x20), true);
   }
   else
   {
    if(!(z->WDT.WTCSR & 0x80))
    {
     z->WDT.WTCSR |= 0x80;
     z->WDT.WTCSRM |= 0x80;
     SH7095_RecalcPendingIntPEX(z);
    }
   }
  }
 }
}

void SH7095_SetFTI(SH7095* z, bool state)
{
 if(MDFN_UNLIKELY(z->timestamp == SS_EVENT_DISABLED_TS))
 {
  z->FRT.FTI = state;
  return;
 }

 SH7095_FRT_WDT_Update(z);
 //
 //
 bool prev = z->FRT.FTI;
 z->FRT.FTI = state;

 if((prev ^ state) & (prev ^ (z->FRT.TCR >> 7)))
 {
  z->FRT.FICR = z->FRT.FRC;
  if(!(z->FRT.FTCSR & 0x80))
  {
   z->FRT.FTCSR |= 0x80;
   z->FRT.FTCSRM |= 0x80;
  }
  SH7095_RecalcPendingIntPEX(z);
 }
}

static void SH7095_SetFTCI(SH7095* z, bool state)
{
 bool prev = z->FRT.FTCI;
 z->FRT.FTCI = state;

 if((z->FRT.TCR & 0x3) == 0x3)
 {
  if(!prev && state)
   SH7095_FRT_ClockFRC(z);
 }
}

static NO_INLINE void SH7095_WDT_Reset(SH7095* z, bool from_internal_wdt)
{
 z->WDT.WTCSR = 0x00;
 z->WDT.WTCSRM = 0x00;

 z->WDT.WTCNT = 0x00;

 if(!from_internal_wdt)
 {
  z->WDT.RSTCSR = 0x00;
  z->WDT.RSTCSRM = 0x00;
 }

 SH7095_FRT_WDT_Recalc_NET(z);
 SH7095_RecalcPendingIntPEX(z);
}

static NO_INLINE void SH7095_WDT_StandbyReset(SH7095* z)
{
 z->WDT.WTCSR &= 0x1F;
 z->WDT.WTCSRM &= 0x1F;

 z->WDT.RSTCSR = 0x00;
 z->WDT.RSTCSRM = 0x00;

 SH7095_FRT_WDT_Recalc_NET(z);
 SH7095_RecalcPendingIntPEX(z);
}

//
//
//
//
//

static INLINE uint64_t DIVU64_Partial(uint64_t dividend, uint32_t divisor)
{
 bool M, Q;

 Q = dividend >> 63;
 M = divisor >> 31;

 for(unsigned x = 0; x < 3; x++)
 {
	if(!(Q ^ M))
	 dividend -= (uint64_t)divisor << 32;
	else
	 dividend += (uint64_t)divisor << 32;

	Q = dividend >> 63;
        dividend <<= 1;
	dividend |= Q ^ 1 ^ M;
 }

 return dividend;
}

static INLINE void SH7095_DIVU_S32_S32(SH7095* z)
{
 if(!z->DVSR)
 {
  z->divide_finish_timestamp = z->MA_until + 2 + 6;

  z->DVCR |= 1;
  SH7095_RecalcPendingIntPEX(z);

  z->DVDNTH = (int32_t)z->DVDNT >> 29;

  if(!(z->DVCR & 2))
   z->DVDNT = z->DVDNTL = 0x7FFFFFFF + ((int32_t)z->DVDNT < 0);
  else
   z->DVDNT = z->DVDNTL = (z->DVDNT << 3) | (((int32_t)~z->DVDNT >> 31) & 7);
 }
 else
 {
  z->divide_finish_timestamp = z->MA_until + 1 + 39;

  if(z->DVSR == 0xFFFFFFFF && z->DVDNTL == 0x80000000)
  {
   z->DVDNT = z->DVDNTL = 0x80000000;
   z->DVDNTH = 0;
  }
  else
  {
   z->DVDNTH = (int32_t)z->DVDNTL % (int32_t)z->DVSR;
   z->DVDNT = z->DVDNTL = (int32_t)z->DVDNTL / (int32_t)z->DVSR;
  }
 }
 z->DVDNTH_Shadow = z->DVDNTH;
 z->DVDNTL_Shadow = z->DVDNTL;
}

static INLINE void SH7095_DIVU_S64_S32(SH7095* z)
{
 const int32_t divisor = z->DVSR;
 const int64_t dividend = ((int64_t)z->DVDNTH << 32) | z->DVDNTL;
 int64_t quotient;

 if(!divisor)
  goto Overflow;

 if((uint64_t)dividend == (1ULL << 63) && (uint32_t)divisor == ~(uint32_t)0)
  goto Overflow;

 quotient = dividend / divisor;

 if(quotient == 2147483648LL && divisor < 0 && (dividend % divisor) == 0)	// Ugh, maybe we should just implement it properly the long way...
  goto SkipOVCheck;

 if(quotient < -2147483647LL || quotient > 2147483647LL)
 {
  Overflow:
  z->divide_finish_timestamp = z->timestamp + 6;
  z->DVCR |= 1;
  SH7095_RecalcPendingIntPEX(z);
  //
  uint64_t tmp = DIVU64_Partial(dividend, divisor);
  z->DVDNTH = tmp >> 32;

  if(z->DVCR & 2)
   z->DVDNT = z->DVDNTL = tmp;
  else
   z->DVDNT = z->DVDNTL = 0x7FFFFFFF + ((int32_t)((dividend >> 32) ^ divisor) < 0);
 }
 else
 {
  SkipOVCheck:
  z->divide_finish_timestamp = z->timestamp + 39;
  z->DVDNTH = dividend % divisor;
  z->DVDNT = z->DVDNTL = quotient;
 }
 z->DVDNTH_Shadow = z->DVDNTH;
 z->DVDNTL_Shadow = z->DVDNTL;
}

//
//
// Begin z->SCI
//
//
static void SH7095_SCI_Reset(SH7095* z)
{
 z->SCI.SMR = 0x00;
 z->SCI.BRR = 0xFF;
 z->SCI.SCR = 0x00;
 z->SCI.TDR = 0xFF;
 z->SCI.SSR = 0x84;
 z->SCI.SSRM = 0x00;
 z->SCI.RDR = 0x00;
 //
 z->SCI.RSR = 0x00;
 z->SCI.TSR = 0x00;

 SH7095_RecalcPendingIntPEX(z);
}

//
//
// End z->SCI
//
//

//
// Misaligned/wrong-sized accesses aren't handled correctly, it's a mess, but probably doesn't matter.
//

static NO_INLINE MDFN_HOT void SH7095_OnChipRegWrite_u8(SH7095* z, uint32_t A, uint32_t V)
{
 if(A & 0x100)
 {
  A &= 0xFC;

  SH7095_SetPEX(z, SH7095_PEX_CPUADDR);
  V |= (uint8_t)V << 8;

  switch(A)
  {
   default:
	break;

   //
   // Division Unit registers
   //
   case 0x20:
   case 0x00:
	z->DVSR = V;
	break;

   case 0x24:
   case 0x04:
	z->DVDNT = V;
	z->DVDNTL = V;
	z->DVDNTH = (int32_t)V >> 31;
	SH7095_DIVU_S32_S32(z);
	break;

   case 0x28:
   case 0x08:
	z->DVCR = V & 0x3;
	break;

   case 0x2C:
   case 0x0C:
	z->VCRDIV = V;
	break;

   case 0x30:
   case 0x10:
	z->DVDNTH = V;
	break;

   case 0x34:
   case 0x14:
	z->DVDNTL = V;
	SH7095_DIVU_S64_S32(z);
	break;

   // ?
   case 0x38:
   case 0x18:
	z->DVDNTH_Shadow = V;
	break;

   case 0x3C:
   case 0x1C:
	z->DVDNTL_Shadow = V;
	break;
   //
   //
   //

   //
   // DMA registers
   //
   case 0x80:
   case 0x90:
	z->DMACH[(A >> 4) & 1].SAR = V;
	break;

   case 0x84:
   case 0x94:
	z->DMACH[(A >> 4) & 1].DAR = V;
	break;

   case 0x88:
   case 0x98:
	z->DMACH[(A >> 4) & 1].TCR = V & 0xFFFFFF;
	break;

   case 0x8C:
   case 0x9C:
	SH7095_DMA_Update(z, SH7095_mem_timestamp);
	{
	 const unsigned ch = (A >> 4) & 1;

	 z->DMACH[ch].CHCR = (V & ~2) | (z->DMACH[ch].CHCR & (V | z->DMACH[ch].CHCRM) & 2);
	}
	SH7095_DMA_RecalcRunning(z);
	SH7095_DMA_StartSG(z);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0xA0:
   case 0xA8:
	z->DMACH[(A >> 3) & 1].VCR = V;
	break;

   case 0xB0:
	SH7095_DMA_Update(z, SH7095_mem_timestamp);
	z->DMAOR = (V & 0x9) | (z->DMAOR & (V | z->DMAORM) & 0x6);
	SH7095_DMA_RecalcRunning(z);
	SH7095_DMA_StartSG(z);
	break;

   //
   // z->BSC registers
   //
   case 0xE0:	// BCR1
	break;

   case 0xE4:	// BCR2
	break;

   case 0xE8:	// WCR
	break;

   case 0xEC:	// MCR
	break;

   case 0xF0:	// RTCSR
	break;

   case 0xF4:	// RTCNT
	break;

   case 0xF8: // RTCOR
	break;
  }
 }
 else
 {
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  shift = ((A & 1) ^ 1) << 3;
  mask = 0xFF << shift;

  switch(A & 0xFF)
  {
   default:
	break;

   //
   // Free-running timer registers.
   //
   //
   // TIER
   case 0x10:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.TIER = V;
	SH7095_RecalcPendingIntPEX(z);
	break;

   // FTCSR
   case 0x11:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.FTCSR = (z->FRT.FTCSR & (z->FRT.FTCSRM | V) & 0x8E) | (V & 0x01);
	SH7095_RecalcPendingIntPEX(z);
	SH7095_FRT_CheckOCR(z);
	break;

   // FRCH
   case 0x12:
	z->FRT.RW_Temp = V;
	break;

   // FRCL
   case 0x13:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.FRC = (z->FRT.RW_Temp << 8) | (V & 0xFF);
	SH7095_FRT_CheckOCR(z);
	SH7095_FRT_WDT_Recalc_NET(z);
	break;

   // OCRA/B H
   case 0x14:
	z->FRT.RW_Temp = V;
	break;

   // OCRA/B L
   case 0x15:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.OCR[(z->FRT.TOCR >> 4) & 1] = (z->FRT.RW_Temp << 8) | V;
	SH7095_FRT_CheckOCR(z);
	SH7095_FRT_WDT_Recalc_NET(z);
	break;

   // TCR
   case 0x16:
	{
	 SH7095_FRT_WDT_Update(z);
	 //
	 //const uint8_t old_TCR = z->FRT.TCR;
 	 z->FRT.TCR = V;
	 //
	 //
	 //
	 SH7095_FRT_WDT_Recalc_NET(z);
	}
	break;

   // TOCR
   case 0x17:
	z->FRT.TOCR = V & 0x1F;
	break;

   //
   //
   //
   case 0x71:
   case 0x72:
	z->DMACH[(A & 1) ^ 1].DRCR = V & 0x3;
	break;

   //
   // z->WDT registers
   //
   case 0x80:
   case 0x88:
	SH7095_FRT_WDT_Update(z);
	z->WDT.RSTCSRM = 0;
	SH7095_FRT_WDT_Recalc_NET(z);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0x82:
   case 0x8A:
	SH7095_FRT_WDT_Update(z);
	z->WDT.RSTCSRM = 0;
	break;

   case 0x81:
   case 0x83:
   case 0x84:
   case 0x85:
   case 0x86:
   case 0x87:
   case 0x89:
   case 0x8B:
   case 0x8C:
   case 0x8D:
   case 0x8E:
   case 0x8F:
	z->WDT.RSTCSRM = 0;
	break;

   //
   //
   //
   case 0x91:
	z->SBYCR = V;

	if(z->WDT.WTCSR & 0x20)
	 z->SBYCR &= 0x7F;

	break;

   case 0x92:	// No 0x93...0x9F mirrors for writes.
	SH7095_SetCCR(z, V);
	break;
   //
   //
   //
   case 0x60:
   case 0x61:
	z->IPRB = (z->IPRB &~ mask) | ((V << shift) & mask & 0xFF00);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0x62:
   case 0x63:
	z->VCRA = (z->VCRA &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;

   case 0x64:
   case 0x65:
	z->VCRB = (z->VCRB &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;

   case 0x66:
   case 0x67:
	z->VCRC = (z->VCRC &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;

   case 0x68:
   case 0x69:
	z->VCRD = (z->VCRD &~ mask) | ((V << shift) & mask & 0x7F00);
	break;

   case 0xE0:
   case 0xE1:
	z->ICR = (z->ICR &~ mask) | ((V << shift) & mask & 0x0101);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0xE2:
   case 0xE3:
	z->IPRA = (z->IPRA &~ mask) | ((V << shift) & mask & 0xFFF0);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0xE4:
   case 0xE5:
	z->VCRWDT = (z->VCRWDT &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;
  }
 }
}

static NO_INLINE MDFN_HOT void SH7095_OnChipRegWrite_u16(SH7095* z, uint32_t A, uint32_t V)
{
 if(A & 0x100)
 {
  A &= 0xFE;

  switch(A)
  {
   default:
	break;

   //
   // Division Unit registers
   //
   case 0x20:
   case 0x00:
	z->DVSR = V;
	break;

   case 0x24:
   case 0x04:
	z->DVDNT = V;
	z->DVDNTL = V;
	z->DVDNTH = (int32_t)V >> 31;
	SH7095_DIVU_S32_S32(z);
	break;

   case 0x28:
   case 0x08:
	z->DVCR = V & 0x3;
	break;

   case 0x2C:
   case 0x0C:
	z->VCRDIV = V;
	break;

   case 0x30:
   case 0x10:
	z->DVDNTH = V;
	break;

   case 0x34:
   case 0x14:
	z->DVDNTL = V;
	SH7095_DIVU_S64_S32(z);
	break;

   // ?
   case 0x38:
   case 0x18:
	z->DVDNTH_Shadow = V;
	break;

   case 0x3C:
   case 0x1C:
	z->DVDNTL_Shadow = V;
	break;
   //
   //
   //

   //
   // DMA registers
   //
   case 0x80:
   case 0x90:
	z->DMACH[(A >> 4) & 1].SAR = V;
	break;

   case 0x84:
   case 0x94:
	z->DMACH[(A >> 4) & 1].DAR = V;
	break;

   case 0x88:
   case 0x98:
	z->DMACH[(A >> 4) & 1].TCR = V & 0xFFFFFF;
	break;

   case 0x8C:
   case 0x9C:
	SH7095_DMA_Update(z, SH7095_mem_timestamp);
	{
	 const unsigned ch = (A >> 4) & 1;

	 z->DMACH[ch].CHCR = (V & ~2) | (z->DMACH[ch].CHCR & (V | z->DMACH[ch].CHCRM) & 2);
	}
	SH7095_DMA_RecalcRunning(z);
	SH7095_DMA_StartSG(z);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0xA0:
   case 0xA8:
	z->DMACH[(A >> 3) & 1].VCR = V;
	break;

   case 0xB0:
	SH7095_DMA_Update(z, SH7095_mem_timestamp);
	z->DMAOR = (V & 0x9) | (z->DMAOR & (V | z->DMAORM) & 0x6);
	SH7095_DMA_RecalcRunning(z);
	SH7095_DMA_StartSG(z);
	break;

   //
   // z->BSC registers
   //
   case 0xE0:	// BCR1
	break;

   case 0xE4:	// BCR2
	break;

   case 0xE8:	// WCR
	break;

   case 0xEC:	// MCR
	break;

   case 0xF0:	// RTCSR
	break;

   case 0xF4:	// RTCNT
	break;

   case 0xF8: // RTCOR
	break;
  }
 }
 else
 {
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  switch(A & 0xFF)
  {
   default:
	break;

   //
   // Free-running timer registers.
   //
   //
   // TIER
   case 0x10:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.TIER = V;
	SH7095_RecalcPendingIntPEX(z);
	break;

   // FTCSR
   case 0x11:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.FTCSR = (z->FRT.FTCSR & (z->FRT.FTCSRM | V) & 0x8E) | (V & 0x01);
	SH7095_RecalcPendingIntPEX(z);
	SH7095_FRT_CheckOCR(z);
	break;

   // FRCH
   case 0x12:
	z->FRT.RW_Temp = V;
	break;

   // FRCL
   case 0x13:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.FRC = (z->FRT.RW_Temp << 8) | (V & 0xFF);
	SH7095_FRT_CheckOCR(z);
	SH7095_FRT_WDT_Recalc_NET(z);
	break;

   // OCRA/B H
   case 0x14:
	z->FRT.RW_Temp = V;
	break;

   // OCRA/B L
   case 0x15:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.OCR[(z->FRT.TOCR >> 4) & 1] = (z->FRT.RW_Temp << 8) | V;
	SH7095_FRT_CheckOCR(z);
	SH7095_FRT_WDT_Recalc_NET(z);
	break;

   // TCR
   case 0x16:
	{
	 SH7095_FRT_WDT_Update(z);
	 //
	 //const uint8_t old_TCR = z->FRT.TCR;
 	 z->FRT.TCR = V;
	 //
	 //
	 //
	 SH7095_FRT_WDT_Recalc_NET(z);
	}
	break;

   // TOCR
   case 0x17:
	z->FRT.TOCR = V & 0x1F;
	break;

   //
   //
   //
   case 0x71:
   case 0x72:
	z->DMACH[(A & 1) ^ 1].DRCR = V & 0x3;
	break;

   //
   // z->WDT registers
   //
   case 0x80:
   case 0x88:
	SH7095_FRT_WDT_Update(z);
 if((V & 0xFF00) == 0x5A00)
 {
  if(z->WDT.WTCSR & 0x20)
   z->WDT.WTCNT = V;
 }
 else if((V & 0xFF00) == 0xA500)
 {
  z->WDT.WTCSR = (z->WDT.WTCSR & (z->WDT.WTCSRM | V) & 0x80) | (V & 0x67);

  if(z->WDT.WTCSR & 0x20)
   z->SBYCR &= 0x7F;
  else
  {
   z->WDT.WTCSR &= ~0x80;	// Seemingly undocumented...
   z->WDT.WTCNT = 0;
  }
 }
	z->WDT.RSTCSRM = 0;
	SH7095_FRT_WDT_Recalc_NET(z);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0x82:
   case 0x8A:
	SH7095_FRT_WDT_Update(z);
 if(V == 0xA500)
 {
  // Clear OVF bit
  z->WDT.RSTCSR &= ~z->WDT.RSTCSRM;
 }
 else if((V & 0xFF00) == 0x5A00)
 {
  // Write RSTE and RSTS bits
  z->WDT.RSTCSR = (z->WDT.RSTCSR & 0x80) | (V & 0x60);
 }
	z->WDT.RSTCSRM = 0;
	break;

   case 0x81:
   case 0x83:
   case 0x84:
   case 0x85:
   case 0x86:
   case 0x87:
   case 0x89:
   case 0x8B:
   case 0x8C:
   case 0x8D:
   case 0x8E:
   case 0x8F:
	z->WDT.RSTCSRM = 0;
	break;

   //
   //
   //
   case 0x91:
	z->SBYCR = V;

	if(z->WDT.WTCSR & 0x20)
	 z->SBYCR &= 0x7F;

	break;

   case 0x92:	// No 0x93...0x9F mirrors for writes.
	SH7095_SetCCR(z, V);
	break;
   //
   //
   //
   case 0x60:
   case 0x61:
	z->IPRB = (z->IPRB &~ mask) | ((V << shift) & mask & 0xFF00);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0x62:
   case 0x63:
	z->VCRA = (z->VCRA &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;

   case 0x64:
   case 0x65:
	z->VCRB = (z->VCRB &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;

   case 0x66:
   case 0x67:
	z->VCRC = (z->VCRC &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;

   case 0x68:
   case 0x69:
	z->VCRD = (z->VCRD &~ mask) | ((V << shift) & mask & 0x7F00);
	break;

   case 0xE0:
   case 0xE1:
	z->ICR = (z->ICR &~ mask) | ((V << shift) & mask & 0x0101);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0xE2:
   case 0xE3:
	z->IPRA = (z->IPRA &~ mask) | ((V << shift) & mask & 0xFFF0);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0xE4:
   case 0xE5:
	z->VCRWDT = (z->VCRWDT &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;
  }
 }
}

static NO_INLINE MDFN_HOT void SH7095_OnChipRegWrite_u32(SH7095* z, uint32_t A, uint32_t V)
{
 if(A & 0x100)
 {
  A &= 0xFC;

  switch(A)
  {
   default:
	break;

   //
   // Division Unit registers
   //
   case 0x20:
   case 0x00:
	z->DVSR = V;
	break;

   case 0x24:
   case 0x04:
	z->DVDNT = V;
	z->DVDNTL = V;
	z->DVDNTH = (int32_t)V >> 31;
	SH7095_DIVU_S32_S32(z);
	break;

   case 0x28:
   case 0x08:
	z->DVCR = V & 0x3;
	break;

   case 0x2C:
   case 0x0C:
	z->VCRDIV = V;
	break;

   case 0x30:
   case 0x10:
	z->DVDNTH = V;
	break;

   case 0x34:
   case 0x14:
	z->DVDNTL = V;
	SH7095_DIVU_S64_S32(z);
	break;

   // ?
   case 0x38:
   case 0x18:
	z->DVDNTH_Shadow = V;
	break;

   case 0x3C:
   case 0x1C:
	z->DVDNTL_Shadow = V;
	break;
   //
   //
   //

   //
   // DMA registers
   //
   case 0x80:
   case 0x90:
	z->DMACH[(A >> 4) & 1].SAR = V;
	break;

   case 0x84:
   case 0x94:
	z->DMACH[(A >> 4) & 1].DAR = V;
	break;

   case 0x88:
   case 0x98:
	z->DMACH[(A >> 4) & 1].TCR = V & 0xFFFFFF;
	break;

   case 0x8C:
   case 0x9C:
	SH7095_DMA_Update(z, SH7095_mem_timestamp);
	{
	 const unsigned ch = (A >> 4) & 1;

	 z->DMACH[ch].CHCR = (V & ~2) | (z->DMACH[ch].CHCR & (V | z->DMACH[ch].CHCRM) & 2);
	}
	SH7095_DMA_RecalcRunning(z);
	SH7095_DMA_StartSG(z);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0xA0:
   case 0xA8:
	z->DMACH[(A >> 3) & 1].VCR = V;
	break;

   case 0xB0:
	SH7095_DMA_Update(z, SH7095_mem_timestamp);
	z->DMAOR = (V & 0x9) | (z->DMAOR & (V | z->DMAORM) & 0x6);
	SH7095_DMA_RecalcRunning(z);
	SH7095_DMA_StartSG(z);
	break;

   //
   // z->BSC registers
   //
   case 0xE0:	// BCR1
	if((V & 0xFFFF0000) == 0xA55A0000)
	{
 	 z->BSC.BCR1 = (z->BSC.BCR1 & 0x8000) | (V & 0x1FF7);
	}
	break;

   case 0xE4:	// BCR2
	if((V & 0xFFFF0000) == 0xA55A0000)
	{
 	 z->BSC.BCR2 = V & 0xFC;
	}
	break;

   case 0xE8:	// WCR
	if((V & 0xFFFF0000) == 0xA55A0000)
	{
 	 z->BSC.WCR = V & 0xFFFF;
	}
	break;

   case 0xEC:	// MCR
	if((V & 0xFFFF0000) == 0xA55A0000)
	{
 	 z->BSC.MCR = V & 0xFEFC;
	}
	break;

   case 0xF0:	// RTCSR
	if((V & 0xFFFF0000) == 0xA55A0000)
	{
 	 z->BSC.RTCSR = (V & 0x78) | (z->BSC.RTCSR & 0x80 & (V | ~z->BSC.RTCSRM));
	}
	break;

   case 0xF4:	// RTCNT
	if((V & 0xFFFF0000) == 0xA55A0000)
	{
 	 z->BSC.RTCNT = V & 0xFF;
	}
	break;

   case 0xF8: // RTCOR
	if((V & 0xFFFF0000) == 0xA55A0000)
	{
 	 z->BSC.RTCOR = V & 0xFF;
	}
	break;
  }
 }
 else
 {
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  shift = ((A & 1) ^ 1) << 3;
  mask = 0xFF << shift;

  shift ^= 8;

  SH7095_SetPEX(z, SH7095_PEX_CPUADDR);

  switch(A & 0xFF)
  {
   default:
	break;

   //
   // Free-running timer registers.
   //
   //
   // TIER
   case 0x10:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.TIER = V;
	SH7095_RecalcPendingIntPEX(z);
	break;

   // FTCSR
   case 0x11:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.FTCSR = (z->FRT.FTCSR & (z->FRT.FTCSRM | V) & 0x8E) | (V & 0x01);
	SH7095_RecalcPendingIntPEX(z);
	SH7095_FRT_CheckOCR(z);
	break;

   // FRCH
   case 0x12:
	z->FRT.RW_Temp = V;
	break;

   // FRCL
   case 0x13:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.FRC = (z->FRT.RW_Temp << 8) | (V & 0xFF);
	SH7095_FRT_CheckOCR(z);
	SH7095_FRT_WDT_Recalc_NET(z);
	break;

   // OCRA/B H
   case 0x14:
	z->FRT.RW_Temp = V;
	break;

   // OCRA/B L
   case 0x15:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.OCR[(z->FRT.TOCR >> 4) & 1] = (z->FRT.RW_Temp << 8) | V;
	SH7095_FRT_CheckOCR(z);
	SH7095_FRT_WDT_Recalc_NET(z);
	break;

   // TCR
   case 0x16:
	{
	 SH7095_FRT_WDT_Update(z);
	 //
	 //const uint8_t old_TCR = z->FRT.TCR;
 	 z->FRT.TCR = V;
	 //
	 //
	 //
	 SH7095_FRT_WDT_Recalc_NET(z);
	}
	break;

   // TOCR
   case 0x17:
	z->FRT.TOCR = V & 0x1F;
	break;

   //
   //
   //
   case 0x71:
   case 0x72:
	z->DMACH[(A & 1) ^ 1].DRCR = V & 0x3;
	break;

   //
   // z->WDT registers
   //
   case 0x80:
   case 0x88:
	SH7095_FRT_WDT_Update(z);
	z->WDT.RSTCSRM = 0;
	SH7095_FRT_WDT_Recalc_NET(z);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0x82:
   case 0x8A:
	SH7095_FRT_WDT_Update(z);
	z->WDT.RSTCSRM = 0;
	break;

   case 0x81:
   case 0x83:
   case 0x84:
   case 0x85:
   case 0x86:
   case 0x87:
   case 0x89:
   case 0x8B:
   case 0x8C:
   case 0x8D:
   case 0x8E:
   case 0x8F:
	z->WDT.RSTCSRM = 0;
	break;

   //
   //
   //
   case 0x91:
	z->SBYCR = V;

	if(z->WDT.WTCSR & 0x20)
	 z->SBYCR &= 0x7F;

	break;

   case 0x92:	// No 0x93...0x9F mirrors for writes.
	SH7095_SetCCR(z, V);
	break;
   //
   //
   //
   case 0x60:
   case 0x61:
	z->IPRB = (z->IPRB &~ mask) | ((V << shift) & mask & 0xFF00);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0x62:
   case 0x63:
	z->VCRA = (z->VCRA &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;

   case 0x64:
   case 0x65:
	z->VCRB = (z->VCRB &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;

   case 0x66:
   case 0x67:
	z->VCRC = (z->VCRC &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;

   case 0x68:
   case 0x69:
	z->VCRD = (z->VCRD &~ mask) | ((V << shift) & mask & 0x7F00);
	break;

   case 0xE0:
   case 0xE1:
	z->ICR = (z->ICR &~ mask) | ((V << shift) & mask & 0x0101);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0xE2:
   case 0xE3:
	z->IPRA = (z->IPRA &~ mask) | ((V << shift) & mask & 0xFFF0);
	SH7095_RecalcPendingIntPEX(z);
	break;

   case 0xE4:
   case 0xE5:
	z->VCRWDT = (z->VCRWDT &~ mask) | ((V << shift) & mask & 0x7F7F);
	break;
  }
 }
}

static INLINE uint8_t SH7095_OnChipRegRead_INLINE_u8(SH7095* z, uint32_t A)
{
 if(A & 0x100)
 {
  uint32_t ret = 0;

  z->MA_until++;

  A &= 0xFC;

  SH7095_SetPEX(z, SH7095_PEX_CPUADDR);

  switch(A)
  {
   default:
	break;

   //
   // Division Unit registers
   //
   case 0x20:
   case 0x22:
   case 0x00:
   case 0x02:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVSR;
	break;

   case 0x24:
   case 0x26:
   case 0x04:
   case 0x06:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNT;
	break;

   case 0x28:
   case 0x2A:
   case 0x08:
   case 0x0A:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVCR;
	break;

   case 0x2C:
   case 0x2E:
   case 0x0C:
   case 0x0E:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->VCRDIV;
	break;

   case 0x30:
   case 0x32:
   case 0x10:
   case 0x12:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTH;
	break;

   case 0x34:
   case 0x36:
   case 0x14:
   case 0x16:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTL;
	break;

   // ?
   case 0x38:
   case 0x3A:
   case 0x18:
   case 0x1A:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTH_Shadow;
	break;

   case 0x3C:
   case 0x3E:
   case 0x1C:
   case 0x1E:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTL_Shadow;
	break;
   //
   //
   //

   //
   // DMA registers
   //
   case 0x80:
   case 0x90:
	ret = z->DMACH[(A >> 4) & 1].SAR;
	break;

   case 0x84:
   case 0x94:
	ret = z->DMACH[(A >> 4) & 1].DAR;
	break;

   case 0x88:
   case 0x98:
	ret = z->DMACH[(A >> 4) & 1].TCR;
	break;

   case 0x8C:
   case 0x9C:
	{
	 const unsigned ch = (A >> 4) & 1;

	 ret = z->DMACH[ch].CHCR;
	 z->DMACH[ch].CHCRM = 0;
	}
	break;

   case 0xA0:
   case 0xA8:
	ret = z->DMACH[(A >> 3) & 1].VCR;
	break;

   case 0xB0:
	ret = z->DMAOR;
	z->DMAORM = 0;
	break;

   //
   // z->BSC registers
   //
   case 0xE0:	// BCR1
   case 0xE2:
	ret = z->BSC.BCR1;
	break;

   case 0xE4:	// BCR2
   case 0xE6:
	ret = z->BSC.BCR2;
	break;

   case 0xE8:	// WCR
   case 0xEA:
	ret = z->BSC.WCR;
	break;

   case 0xEC:	// MCR
   case 0xEE:
	ret = z->BSC.MCR;
	break;

   case 0xF0:	// RTCSR
   case 0xF2:
	ret = z->BSC.RTCSR;
	z->BSC.RTCSRM = z->BSC.RTCSR & 0x80;

	break;

   case 0xF4:	// RTCNT
   case 0xF6:
	ret = z->BSC.RTCNT;

	break;

   case 0xF8:	// RTCOR
   case 0xFA:
	ret = z->BSC.RTCOR;
	break;
  }

  ret >>= ((A & 1) ^ 1) << 3;

  return ret;
 }
 else
 {
  const unsigned Am = (uint8_t)A;
  const unsigned shift = ((((A & 1) ^ 1) << 3));
  uint16_t ret = 0;

  if(Am < 0x20)
   z->MA_until = (z->MA_until + 11) &~ 1; // FIXME: not quite right. //3;
  else if((Am >= 0x60 && Am < 0xA0) || Am >= 0xE0)
   z->MA_until += 3;
  else
   z->MA_until += 1;

  switch(Am)
  {
   default:
	break;

   //
   // z->FRT registers.  Some weirdness with 16-bit reads duplicating the lower 8 bits in the upper 8-bits, but the upper 8-bits are masked
   // with the last data written to the z->FRT area or something...not emulated here.
   //
   case 0x10:
	ret = z->FRT.TIER | 1;
	break;

   case 0x11:
	SH7095_FRT_WDT_Update(z);
	//
	ret = z->FRT.FTCSR;
	z->FRT.FTCSRM = 0x00;
	break;

   case 0x12:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.RW_Temp = z->FRT.FRC;
	ret = z->FRT.FRC >> 8;
	break;

   case 0x13:
	ret = z->FRT.RW_Temp;
	break;

   case 0x14:
	ret = z->FRT.OCR[(z->FRT.TOCR >> 4) & 1] >> 8;
	break;

   case 0x15:
	ret = z->FRT.OCR[(z->FRT.TOCR >> 4) & 1] & 0xFF;
	break;

   case 0x16:
	ret = z->FRT.TCR;
	break;

   case 0x17:
	ret = z->FRT.TOCR | 0xE0;
	break;

   case 0x18:
	z->FRT.RW_Temp = z->FRT.FICR;
	ret = z->FRT.FICR >> 8;
	break;

   case 0x19:
	ret = z->FRT.RW_Temp;
	break;

   //
   //
   //
   case 0x71:
   case 0x72:
	ret = z->DMACH[(A & 1) ^ 1].DRCR;
	break;

   //
   // z->WDT registers
   //
   case 0x80:
   case 0x88:
	SH7095_FRT_WDT_Update(z);
	ret = z->WDT.WTCSR | 0x18;
	z->WDT.WTCSRM = 0x00;
	break;

   case 0x81:
   case 0x89:
	SH7095_FRT_WDT_Update(z);
	ret = z->WDT.WTCNT;
	break;

   case 0x82:
   case 0x85:
   case 0x86:
   case 0x87:
   case 0x8A:
   case 0x8D:
   case 0x8E:
   case 0x8F:
	ret = 0xFF;
	break;

   case 0x83:
   case 0x8B:
	SH7095_FRT_WDT_Update(z);
	ret = z->WDT.RSTCSR | 0x1F;
	z->WDT.RSTCSRM = (z->WDT.RSTCSR & 0x80);
	break;

   // FIXME: z->WDT open bus of a sort.
   // case 0x84:
   // case 0x8C:

   //
   //
   //

   case 0x91:
	ret = z->SBYCR;
	break;

   case 0x92 ... 0x9F:
	//
	// TODO: Emulate jankiness of 16-bit reads from mirror addresses.
	//
	ret = z->CCR | (z->CCR << 8);
	break;
   //
   //
   //
   case 0x60:
   case 0x61:
	ret = z->IPRB >> shift;
	break;

   case 0x62:
   case 0x63:
	ret = z->VCRA >> shift;
	break;

   case 0x64:
   case 0x65:
	ret = z->VCRB >> shift;
	break;

   case 0x66:
   case 0x67:
	ret = z->VCRC >> shift;
	break;

   case 0x68:
   case 0x69:
	ret = z->VCRD >> shift;
	break;

   case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
	ret = 0xFFFF >> shift;
	break;

   case 0xE0:
   case 0xE1:
	ret = (z->ICR | (z->NMILevel << 15)) >> shift;
	break;

   case 0xE2:
   case 0xE3:
	ret = z->IPRA >> shift;
	break;

   case 0xE4:
   case 0xE5:
	ret = z->VCRWDT >> shift;
	break;

   case 0xE6: case 0xE7:
   case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
   case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7:
   case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
	ret = 0xFFFF >> shift;
	break;
  }

  return ret;
 }
}

static INLINE uint16_t SH7095_OnChipRegRead_INLINE_u16(SH7095* z, uint32_t A)
{
 if(A & 0x100)
 {
  uint32_t ret = 0;

  z->MA_until++;

  A &= 0xFE;

  switch(A)
  {
   default:
	break;

   //
   // Division Unit registers
   //
   case 0x20:
   case 0x22:
   case 0x00:
   case 0x02:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVSR;
	break;

   case 0x24:
   case 0x26:
   case 0x04:
   case 0x06:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNT;
	break;

   case 0x28:
   case 0x2A:
   case 0x08:
   case 0x0A:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVCR;
	break;

   case 0x2C:
   case 0x2E:
   case 0x0C:
   case 0x0E:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->VCRDIV;
	break;

   case 0x30:
   case 0x32:
   case 0x10:
   case 0x12:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTH;
	break;

   case 0x34:
   case 0x36:
   case 0x14:
   case 0x16:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTL;
	break;

   // ?
   case 0x38:
   case 0x3A:
   case 0x18:
   case 0x1A:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTH_Shadow;
	break;

   case 0x3C:
   case 0x3E:
   case 0x1C:
   case 0x1E:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTL_Shadow;
	break;
   //
   //
   //

   //
   // DMA registers
   //
   case 0x80:
   case 0x90:
	ret = z->DMACH[(A >> 4) & 1].SAR;
	break;

   case 0x84:
   case 0x94:
	ret = z->DMACH[(A >> 4) & 1].DAR;
	break;

   case 0x88:
   case 0x98:
	ret = z->DMACH[(A >> 4) & 1].TCR;
	break;

   case 0x8C:
   case 0x9C:
	{
	 const unsigned ch = (A >> 4) & 1;

	 ret = z->DMACH[ch].CHCR;
	 z->DMACH[ch].CHCRM = 0;
	}
	break;

   case 0xA0:
   case 0xA8:
	ret = z->DMACH[(A >> 3) & 1].VCR;
	break;

   case 0xB0:
	ret = z->DMAOR;
	z->DMAORM = 0;
	break;

   //
   // z->BSC registers
   //
   case 0xE0:	// BCR1
   case 0xE2:
	ret = z->BSC.BCR1;
	break;

   case 0xE4:	// BCR2
   case 0xE6:
	ret = z->BSC.BCR2;
	break;

   case 0xE8:	// WCR
   case 0xEA:
	ret = z->BSC.WCR;
	break;

   case 0xEC:	// MCR
   case 0xEE:
	ret = z->BSC.MCR;
	break;

   case 0xF0:	// RTCSR
   case 0xF2:
	ret = z->BSC.RTCSR;
	z->BSC.RTCSRM = z->BSC.RTCSR & 0x80;

	break;

   case 0xF4:	// RTCNT
   case 0xF6:
	ret = z->BSC.RTCNT;

	break;

   case 0xF8:	// RTCOR
   case 0xFA:
	ret = z->BSC.RTCOR;
	break;
  }

  return ret;
 }
 else
 {
  const unsigned Am = (uint8_t)A;
  const unsigned shift = (0);
  uint16_t ret = 0;

  if(Am < 0x20)
   z->MA_until = (z->MA_until + 11) &~ 1; // FIXME: not quite right. //3;
  else if((Am >= 0x60 && Am < 0xA0) || Am >= 0xE0)
   z->MA_until += 3;
  else
   z->MA_until += 1;

  switch(Am)
  {
   default:
	break;

   //
   // z->FRT registers.  Some weirdness with 16-bit reads duplicating the lower 8 bits in the upper 8-bits, but the upper 8-bits are masked
   // with the last data written to the z->FRT area or something...not emulated here.
   //
   case 0x10:
	ret = z->FRT.TIER | 1;
	break;

   case 0x11:
	SH7095_FRT_WDT_Update(z);
	//
	ret = z->FRT.FTCSR;
	z->FRT.FTCSRM = 0x00;
	break;

   case 0x12:
	SH7095_FRT_WDT_Update(z);
	//
	z->FRT.RW_Temp = z->FRT.FRC;
	ret = z->FRT.FRC >> 8;
	break;

   case 0x13:
	ret = z->FRT.RW_Temp;
	break;

   case 0x14:
	ret = z->FRT.OCR[(z->FRT.TOCR >> 4) & 1] >> 8;
	break;

   case 0x15:
	ret = z->FRT.OCR[(z->FRT.TOCR >> 4) & 1] & 0xFF;
	break;

   case 0x16:
	ret = z->FRT.TCR;
	break;

   case 0x17:
	ret = z->FRT.TOCR | 0xE0;
	break;

   case 0x18:
	z->FRT.RW_Temp = z->FRT.FICR;
	ret = z->FRT.FICR >> 8;
	break;

   case 0x19:
	ret = z->FRT.RW_Temp;
	break;

   //
   //
   //
   case 0x71:
   case 0x72:
	ret = z->DMACH[(A & 1) ^ 1].DRCR;
	break;

   //
   // z->WDT registers
   //
   case 0x80:
   case 0x88:
	SH7095_FRT_WDT_Update(z);
	ret = z->WDT.WTCSR | 0x18;
	z->WDT.WTCSRM = 0x00;
	break;

   case 0x81:
   case 0x89:
	SH7095_FRT_WDT_Update(z);
	ret = z->WDT.WTCNT;
	break;

   case 0x82:
   case 0x85:
   case 0x86:
   case 0x87:
   case 0x8A:
   case 0x8D:
   case 0x8E:
   case 0x8F:
	ret = 0xFF;
	break;

   case 0x83:
   case 0x8B:
	SH7095_FRT_WDT_Update(z);
	ret = z->WDT.RSTCSR | 0x1F;
	z->WDT.RSTCSRM = (z->WDT.RSTCSR & 0x80);
	break;

   // FIXME: z->WDT open bus of a sort.
   // case 0x84:
   // case 0x8C:

   //
   //
   //

   case 0x91:
	ret = z->SBYCR;
	break;

   case 0x92 ... 0x9F:
	//
	// TODO: Emulate jankiness of 16-bit reads from mirror addresses.
	//
	ret = z->CCR | (z->CCR << 8);
	break;
   //
   //
   //
   case 0x60:
   case 0x61:
	ret = z->IPRB >> shift;
	break;

   case 0x62:
   case 0x63:
	ret = z->VCRA >> shift;
	break;

   case 0x64:
   case 0x65:
	ret = z->VCRB >> shift;
	break;

   case 0x66:
   case 0x67:
	ret = z->VCRC >> shift;
	break;

   case 0x68:
   case 0x69:
	ret = z->VCRD >> shift;
	break;

   case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
	ret = 0xFFFF >> shift;
	break;

   case 0xE0:
   case 0xE1:
	ret = (z->ICR | (z->NMILevel << 15)) >> shift;
	break;

   case 0xE2:
   case 0xE3:
	ret = z->IPRA >> shift;
	break;

   case 0xE4:
   case 0xE5:
	ret = z->VCRWDT >> shift;
	break;

   case 0xE6: case 0xE7:
   case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
   case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7:
   case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
	ret = 0xFFFF >> shift;
	break;
  }

  return ret;
 }
}

static INLINE uint32_t SH7095_OnChipRegRead_INLINE_u32(SH7095* z, uint32_t A)
{
 if(A & 0x100)
 {
  uint32_t ret = 0;

  z->MA_until++;

  A &= 0xFC;

  switch(A)
  {
   default:
	break;

   //
   // Division Unit registers
   //
   case 0x20:
   case 0x22:
   case 0x00:
   case 0x02:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVSR;
	break;

   case 0x24:
   case 0x26:
   case 0x04:
   case 0x06:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNT;
	break;

   case 0x28:
   case 0x2A:
   case 0x08:
   case 0x0A:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVCR;
	break;

   case 0x2C:
   case 0x2E:
   case 0x0C:
   case 0x0E:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->VCRDIV;
	break;

   case 0x30:
   case 0x32:
   case 0x10:
   case 0x12:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTH;
	break;

   case 0x34:
   case 0x36:
   case 0x14:
   case 0x16:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTL;
	break;

   // ?
   case 0x38:
   case 0x3A:
   case 0x18:
   case 0x1A:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTH_Shadow;
	break;

   case 0x3C:
   case 0x3E:
   case 0x1C:
   case 0x1E:
	if((z->divide_finish_timestamp) > z->MA_until) z->MA_until = (z->divide_finish_timestamp);
	ret = z->DVDNTL_Shadow;
	break;
   //
   //
   //

   //
   // DMA registers
   //
   case 0x80:
   case 0x90:
	ret = z->DMACH[(A >> 4) & 1].SAR;
	break;

   case 0x84:
   case 0x94:
	ret = z->DMACH[(A >> 4) & 1].DAR;
	break;

   case 0x88:
   case 0x98:
	ret = z->DMACH[(A >> 4) & 1].TCR;
	break;

   case 0x8C:
   case 0x9C:
	{
	 const unsigned ch = (A >> 4) & 1;

	 ret = z->DMACH[ch].CHCR;
	 z->DMACH[ch].CHCRM = 0;
	}
	break;

   case 0xA0:
   case 0xA8:
	ret = z->DMACH[(A >> 3) & 1].VCR;
	break;

   case 0xB0:
	ret = z->DMAOR;
	z->DMAORM = 0;
	break;

   //
   // z->BSC registers
   //
   case 0xE0:	// BCR1
   case 0xE2:
	ret = z->BSC.BCR1;
	break;

   case 0xE4:	// BCR2
   case 0xE6:
	ret = z->BSC.BCR2;
	break;

   case 0xE8:	// WCR
   case 0xEA:
	ret = z->BSC.WCR;
	break;

   case 0xEC:	// MCR
   case 0xEE:
	ret = z->BSC.MCR;
	break;

   case 0xF0:	// RTCSR
   case 0xF2:
	ret = z->BSC.RTCSR;
	z->BSC.RTCSRM = z->BSC.RTCSR & 0x80;

	break;

   case 0xF4:	// RTCNT
   case 0xF6:
	ret = z->BSC.RTCNT;

	break;

   case 0xF8:	// RTCOR
   case 0xFA:
	ret = z->BSC.RTCOR;
	break;
  }

  return ret;
 }
 else
 {
  const unsigned Am = (uint8_t)A;
  uint16_t ret = 0;

  if(Am < 0x20)
   z->MA_until = (z->MA_until + 11) &~ 1; // FIXME: not quite right. //3;
  else if((Am >= 0x60 && Am < 0xA0) || Am >= 0xE0)
   z->MA_until += 3;
  else
   z->MA_until += 1;

  SH7095_SetPEX(z, SH7095_PEX_CPUADDR);

  return ret;
 }
}

static INLINE void SH7095_BSC_BusWrite_u8(SH7095* z, uint32_t A, uint8_t V, const bool BurstHax, int32_t* SH2DMAHax)
{
 uint32_t DB = SH7095_DB;

 if(!BurstHax)
  SH7095_mem_timestamp += (SH7095_mem_timestamp == z->BSC.last_mem_time) & z->BSC.last_mem_type;

 if(A < 0x02000000)	// CS0, configured as 16-bit
 {
  /* sizeof(uint8_t) == 1: shift = ((A & 1) ^ 1) << 3,  mask = 0xFF << shift */
  const uint32_t shift = ((A & 1) ^ 1) << 3;
  const uint32_t mask = 0xFFu << shift;

  DB = (DB & ~mask) | ((uint32_t)V << shift);
  BusRW_DB_CS0_u8_W1(A, &DB, BurstHax, SH2DMAHax);
 }
 else if(A >= 0x06000000)	// CS3; 32-bit
 {
  const uint32_t shift = ((A & 3) ^ 3) << 3;
  const uint32_t mask = 0xFFu << shift;

  DB = (DB & ~mask) | ((uint32_t)V << shift);
  BusRW_DB_CS3_u8_W1(A, &DB, BurstHax, SH2DMAHax);

  if(!BurstHax)
  {
   if(!SH2DMAHax)
   {
    if(SH7095_mem_timestamp < z->BSC.sdram_finish_time)
     SH7095_mem_timestamp = z->BSC.sdram_finish_time;

    SH7095_mem_timestamp += 2;
    z->BSC.sdram_finish_time = SH7095_mem_timestamp + 2;
   }
   else
    *SH2DMAHax += 3;
  }
 }
 else	// CS1, CS2; 32-bit
 {
  const uint32_t shift = ((A & 3) ^ 3) << 3;
  const uint32_t mask = 0xFFu << shift;

  DB = (DB & ~mask) | ((uint32_t)V << shift);
  BusRW_DB_CS12_u8_W1(A, &DB, BurstHax, SH2DMAHax);
 }

 SH7095_DB = DB;

 if(!BurstHax)
 {
  z->BSC.last_mem_addr = A;
  z->BSC.last_mem_type = 0;
  z->BSC.last_mem_time = SH7095_mem_timestamp;
 }
}

static INLINE void SH7095_BSC_BusWrite_u16(SH7095* z, uint32_t A, uint16_t V, const bool BurstHax, int32_t* SH2DMAHax)
{
 uint32_t DB = SH7095_DB;

 if(!BurstHax)
  SH7095_mem_timestamp += (SH7095_mem_timestamp == z->BSC.last_mem_time) & z->BSC.last_mem_type;

 if(A < 0x02000000)	// CS0, configured as 16-bit
 {
  /* sizeof(uint16_t) == 2: shift = ((A & 1) ^ 0) << 3,  mask = 0xFFFF << shift */
  const uint32_t shift = (A & 1) << 3;
  const uint32_t mask = 0xFFFFu << shift;

  DB = (DB & ~mask) | ((uint32_t)V << shift);
  BusRW_DB_CS0_u16_W1(A, &DB, BurstHax, SH2DMAHax);
 }
 else if(A >= 0x06000000)	// CS3; 32-bit
 {
  const uint32_t shift = ((A & 3) ^ 2) << 3;
  const uint32_t mask = 0xFFFFu << shift;

  DB = (DB & ~mask) | ((uint32_t)V << shift);
  BusRW_DB_CS3_u16_W1(A, &DB, BurstHax, SH2DMAHax);

  if(!BurstHax)
  {
   if(!SH2DMAHax)
   {
    if(SH7095_mem_timestamp < z->BSC.sdram_finish_time)
     SH7095_mem_timestamp = z->BSC.sdram_finish_time;

    SH7095_mem_timestamp += 2;
    z->BSC.sdram_finish_time = SH7095_mem_timestamp + 2;
   }
   else
    *SH2DMAHax += 3;
  }
 }
 else	// CS1, CS2; 32-bit
 {
  const uint32_t shift = ((A & 3) ^ 2) << 3;
  const uint32_t mask = 0xFFFFu << shift;

  DB = (DB & ~mask) | ((uint32_t)V << shift);
  BusRW_DB_CS12_u16_W1(A, &DB, BurstHax, SH2DMAHax);
 }

 SH7095_DB = DB;

 if(!BurstHax)
 {
  z->BSC.last_mem_addr = A;
  z->BSC.last_mem_type = 0;
  z->BSC.last_mem_time = SH7095_mem_timestamp;
 }
}

static INLINE void SH7095_BSC_BusWrite_u32(SH7095* z, uint32_t A, uint32_t V, const bool BurstHax, int32_t* SH2DMAHax)
{
 uint32_t DB = SH7095_DB;

 if(!BurstHax)
  SH7095_mem_timestamp += (SH7095_mem_timestamp == z->BSC.last_mem_time) & z->BSC.last_mem_type;

 if(A < 0x02000000)	// CS0, configured as 16-bit -- split into two 16-bit accesses
 {
  // TODO/FIXME: Don't allow DMA transfers to occur between the two 16-bit accesses.
  //if(!SH2DMAHax)
  // SH7095_BusLock++;

  DB = (DB & 0xFFFF0000) | (V >> 16);
  BusRW_DB_CS0_u16_W1(A, &DB, BurstHax, SH2DMAHax);

  DB = (DB & 0xFFFF0000) | (uint16_t)V;
  BusRW_DB_CS0_u16_W1(A | 2, &DB, BurstHax, SH2DMAHax);

  //if(!SH2DMAHax)
  // SH7095_BusLock--;
 }
 else if(A >= 0x06000000)	// CS3; 32-bit
 {
  /* sizeof(uint32_t) == 4: shift = 0, mask = 0xFFFFFFFFu */
  DB = V;
  BusRW_DB_CS3_u32_W1(A, &DB, BurstHax, SH2DMAHax);

  if(!BurstHax)
  {
   if(!SH2DMAHax)
   {
    if(SH7095_mem_timestamp < z->BSC.sdram_finish_time)
     SH7095_mem_timestamp = z->BSC.sdram_finish_time;

    SH7095_mem_timestamp += 2;
    z->BSC.sdram_finish_time = SH7095_mem_timestamp + 2;
   }
   else
    *SH2DMAHax += 3;
  }
 }
 else	// CS1, CS2; 32-bit
 {
  DB = V;
  BusRW_DB_CS12_u32_W1(A, &DB, BurstHax, SH2DMAHax);
 }

 SH7095_DB = DB;

 if(!BurstHax)
 {
  z->BSC.last_mem_addr = A;
  z->BSC.last_mem_type = 0;
  z->BSC.last_mem_time = SH7095_mem_timestamp;
 }
}

static INLINE uint8_t SH7095_BSC_BusRead_u8(SH7095* z, uint32_t A, const bool BurstHax, int32_t* SH2DMAHax)
{
 uint32_t DB = SH7095_DB;
 uint8_t ret;

 if(!BurstHax)
  SH7095_mem_timestamp += (SH7095_mem_timestamp == z->BSC.last_mem_time) & (bool)((z->BSC.last_mem_addr ^ A) & 0x06000000);

 if(A < 0x02000000)	// CS0, configured as 16-bit
 {
  BusRW_DB_CS0_u8_W0(A, &DB, BurstHax, SH2DMAHax);
  ret = DB >> (((A & 1) ^ 1) << 3);
 }
 else if(A >= 0x06000000)	// CS3; 32-bit
 {
  BusRW_DB_CS3_u8_W0(A, &DB, BurstHax, SH2DMAHax);
  ret = DB >> (((A & 3) ^ 3) << 3);

  if(!BurstHax)
  {
   if(!SH2DMAHax)
   {
    if(SH7095_mem_timestamp < z->BSC.sdram_finish_time)
     SH7095_mem_timestamp = z->BSC.sdram_finish_time;

    SH7095_mem_timestamp += 7;
   }
   else
    *SH2DMAHax += 6;
  }
 }
 else	// CS1, CS2; 32-bit
 {
  BusRW_DB_CS12_u8_W0(A, &DB, BurstHax, SH2DMAHax);
  ret = DB >> (((A & 3) ^ 3) << 3);
 }

 SH7095_DB = DB;

 if(!BurstHax)
 {
  z->BSC.last_mem_addr = A;
  z->BSC.last_mem_type = 1;
  z->BSC.last_mem_time = SH7095_mem_timestamp;
 }

 return ret;
}

static INLINE uint16_t SH7095_BSC_BusRead_u16(SH7095* z, uint32_t A, const bool BurstHax, int32_t* SH2DMAHax)
{
 uint32_t DB = SH7095_DB;
 uint16_t ret;

 if(!BurstHax)
  SH7095_mem_timestamp += (SH7095_mem_timestamp == z->BSC.last_mem_time) & (bool)((z->BSC.last_mem_addr ^ A) & 0x06000000);

 if(A < 0x02000000)	// CS0, configured as 16-bit
 {
  BusRW_DB_CS0_u16_W0(A, &DB, BurstHax, SH2DMAHax);
  ret = DB >> ((A & 1) << 3);
 }
 else if(A >= 0x06000000)	// CS3; 32-bit
 {
  BusRW_DB_CS3_u16_W0(A, &DB, BurstHax, SH2DMAHax);
  ret = DB >> (((A & 3) ^ 2) << 3);

  if(!BurstHax)
  {
   if(!SH2DMAHax)
   {
    if(SH7095_mem_timestamp < z->BSC.sdram_finish_time)
     SH7095_mem_timestamp = z->BSC.sdram_finish_time;

    SH7095_mem_timestamp += 7;
   }
   else
    *SH2DMAHax += 6;
  }
 }
 else	// CS1, CS2; 32-bit
 {
  BusRW_DB_CS12_u16_W0(A, &DB, BurstHax, SH2DMAHax);
  ret = DB >> (((A & 3) ^ 2) << 3);
 }

 SH7095_DB = DB;

 if(!BurstHax)
 {
  z->BSC.last_mem_addr = A;
  z->BSC.last_mem_type = 1;
  z->BSC.last_mem_time = SH7095_mem_timestamp;
 }

 return ret;
}

static INLINE uint32_t SH7095_BSC_BusRead_u32(SH7095* z, uint32_t A, const bool BurstHax, int32_t* SH2DMAHax)
{
 uint32_t DB = SH7095_DB;
 uint32_t ret;

 if(!BurstHax)
  SH7095_mem_timestamp += (SH7095_mem_timestamp == z->BSC.last_mem_time) & (bool)((z->BSC.last_mem_addr ^ A) & 0x06000000);

 if(A < 0x02000000)	// CS0, configured as 16-bit -- split into two 16-bit accesses
 {
  // TODO/FIXME: Don't allow DMA transfers to occur between the two 16-bit accesses.
  //if(!SH2DMAHax)
  // SH7095_BusLock++;

  BusRW_DB_CS0_u16_W0(A, &DB, BurstHax, SH2DMAHax);
  ret = DB << 16;

  BusRW_DB_CS0_u16_W0(A | 2, &DB, BurstHax, SH2DMAHax);
  ret |= (uint16_t)DB;

  //if(!SH2DMAHax)
  // SH7095_BusLock--;
 }
 else if(A >= 0x06000000)	// CS3; 32-bit
 {
  BusRW_DB_CS3_u32_W0(A, &DB, BurstHax, SH2DMAHax);
  ret = DB;	/* shift folds to 0 for sizeof(T)==4 */

  if(!BurstHax)
  {
   if(!SH2DMAHax)
   {
    if(SH7095_mem_timestamp < z->BSC.sdram_finish_time)
     SH7095_mem_timestamp = z->BSC.sdram_finish_time;

    SH7095_mem_timestamp += 7;
   }
   else
    *SH2DMAHax += 6;
  }
 }
 else	// CS1, CS2; 32-bit
 {
  BusRW_DB_CS12_u32_W0(A, &DB, BurstHax, SH2DMAHax);
  ret = DB;
 }

 SH7095_DB = DB;

 if(!BurstHax)
 {
  z->BSC.last_mem_addr = A;
  z->BSC.last_mem_type = 1;
  z->BSC.last_mem_time = SH7095_mem_timestamp;
 }

 return ret;
}

/* === ExtBusRead_INLINE_SP*_u**_BH*  (12 variants) ===
 * Body folded at source level per (SP, BH) tuple. */

static INLINE uint8_t SH7095_ExtBusRead_INLINE_SP0_u8_BH0(SH7095* z, uint32_t A)
{
 uint8_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 ret = SH7095_BSC_BusRead_u8(z, A, false, NULL);

 return ret;
}

static INLINE uint8_t SH7095_ExtBusRead_INLINE_SP0_u8_BH1(SH7095* z, uint32_t A)
{
 uint8_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 ret = SH7095_BSC_BusRead_u8(z, A, true, NULL);

 return ret;
}

static INLINE uint16_t SH7095_ExtBusRead_INLINE_SP0_u16_BH0(SH7095* z, uint32_t A)
{
 uint16_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 ret = SH7095_BSC_BusRead_u16(z, A, false, NULL);

 return ret;
}

static INLINE uint16_t SH7095_ExtBusRead_INLINE_SP0_u16_BH1(SH7095* z, uint32_t A)
{
 uint16_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 ret = SH7095_BSC_BusRead_u16(z, A, true, NULL);

 return ret;
}

static INLINE uint32_t SH7095_ExtBusRead_INLINE_SP0_u32_BH0(SH7095* z, uint32_t A)
{
 uint32_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 ret = SH7095_BSC_BusRead_u32(z, A, false, NULL);

 return ret;
}

static INLINE uint32_t SH7095_ExtBusRead_INLINE_SP0_u32_BH1(SH7095* z, uint32_t A)
{
 uint32_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 ret = SH7095_BSC_BusRead_u32(z, A, true, NULL);

 return ret;
}

static INLINE uint8_t SH7095_ExtBusRead_INLINE_SP1_u8_BH0(SH7095* z, uint32_t A)
{
 uint8_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 if(SH7095_mem_timestamp > (z->BSC.last_mem_time + 1))
  SH7095_mem_timestamp += 2;
 else
  SH7095_mem_timestamp++;

 ret = SH7095_BSC_BusRead_u8(z, A, false, NULL);

 SH7095_mem_timestamp++;

 return ret;
}

static INLINE uint8_t SH7095_ExtBusRead_INLINE_SP1_u8_BH1(SH7095* z, uint32_t A)
{
 uint8_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 ret = SH7095_BSC_BusRead_u8(z, A, true, NULL);

 return ret;
}

static INLINE uint16_t SH7095_ExtBusRead_INLINE_SP1_u16_BH0(SH7095* z, uint32_t A)
{
 uint16_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 if(SH7095_mem_timestamp > (z->BSC.last_mem_time + 1))
  SH7095_mem_timestamp += 2;
 else
  SH7095_mem_timestamp++;

 ret = SH7095_BSC_BusRead_u16(z, A, false, NULL);

 SH7095_mem_timestamp++;

 return ret;
}

static INLINE uint16_t SH7095_ExtBusRead_INLINE_SP1_u16_BH1(SH7095* z, uint32_t A)
{
 uint16_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 ret = SH7095_BSC_BusRead_u16(z, A, true, NULL);

 return ret;
}

static INLINE uint32_t SH7095_ExtBusRead_INLINE_SP1_u32_BH0(SH7095* z, uint32_t A)
{
 uint32_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 if(SH7095_mem_timestamp > (z->BSC.last_mem_time + 1))
  SH7095_mem_timestamp += 2;
 else
  SH7095_mem_timestamp++;

 ret = SH7095_BSC_BusRead_u32(z, A, false, NULL);

 SH7095_mem_timestamp++;

 return ret;
}

static INLINE uint32_t SH7095_ExtBusRead_INLINE_SP1_u32_BH1(SH7095* z, uint32_t A)
{
 uint32_t ret;

 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 ret = SH7095_BSC_BusRead_u32(z, A, true, NULL);

 return ret;
}

/* === ExtBusWrite_INLINE_SP*_u**  (6 variants) === */

static INLINE void SH7095_ExtBusWrite_INLINE_SP0_u8(SH7095* z, uint32_t A, uint8_t V)
{
 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 SH7095_BSC_BusWrite_u8(z, A, V, false, NULL);

 z->write_finish_timestamp = SH7095_mem_timestamp;
}

static INLINE void SH7095_ExtBusWrite_INLINE_SP0_u16(SH7095* z, uint32_t A, uint16_t V)
{
 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 SH7095_BSC_BusWrite_u16(z, A, V, false, NULL);

 z->write_finish_timestamp = SH7095_mem_timestamp;
}

static INLINE void SH7095_ExtBusWrite_INLINE_SP0_u32(SH7095* z, uint32_t A, uint32_t V)
{
 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 SH7095_BSC_BusWrite_u32(z, A, V, false, NULL);

 z->write_finish_timestamp = SH7095_mem_timestamp;
}

static INLINE void SH7095_ExtBusWrite_INLINE_SP1_u8(SH7095* z, uint32_t A, uint8_t V)
{
 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 SH7095_mem_timestamp++;

 SH7095_BSC_BusWrite_u8(z, A, V, false, NULL);

 if(SH7095_mem_timestamp < z->BSC.sdram_finish_time)
  SH7095_mem_timestamp = z->BSC.sdram_finish_time;
 SH7095_mem_timestamp++;

 z->write_finish_timestamp = SH7095_mem_timestamp;
}

static INLINE void SH7095_ExtBusWrite_INLINE_SP1_u16(SH7095* z, uint32_t A, uint16_t V)
{
 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 SH7095_mem_timestamp++;

 SH7095_BSC_BusWrite_u16(z, A, V, false, NULL);

 if(SH7095_mem_timestamp < z->BSC.sdram_finish_time)
  SH7095_mem_timestamp = z->BSC.sdram_finish_time;
 SH7095_mem_timestamp++;

 z->write_finish_timestamp = SH7095_mem_timestamp;
}

static INLINE void SH7095_ExtBusWrite_INLINE_SP1_u32(SH7095* z, uint32_t A, uint32_t V)
{
 A &= (1U << 27) - 1;

 if(z->timestamp > SH7095_mem_timestamp)
  SH7095_mem_timestamp = z->timestamp;

 z->DMA_PenaltyKludgeAccum += z->DMA_PenaltyKludgeAmount;

 SH7095_mem_timestamp++;

 SH7095_BSC_BusWrite_u32(z, A, V, false, NULL);

 if(SH7095_mem_timestamp < z->BSC.sdram_finish_time)
  SH7095_mem_timestamp = z->BSC.sdram_finish_time;
 SH7095_mem_timestamp++;

 z->write_finish_timestamp = SH7095_mem_timestamp;
}

/* === ExtBusRead_NI_w*_SP*_u**_BH*  (18 variants) ===
 * Generated only for (w, SP) tuples actually invoked by the
 * three #define macro contexts. */

static NO_INLINE MDFN_HOT uint8_t ExtBusRead_NI_w0_SP0_u8_BH0(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u8_BH0(&CPU[0], A);
}

static NO_INLINE MDFN_HOT uint8_t ExtBusRead_NI_w0_SP0_u8_BH1(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u8_BH1(&CPU[0], A);
}

static NO_INLINE MDFN_HOT uint16_t ExtBusRead_NI_w0_SP0_u16_BH0(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u16_BH0(&CPU[0], A);
}

static NO_INLINE MDFN_HOT uint16_t ExtBusRead_NI_w0_SP0_u16_BH1(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u16_BH1(&CPU[0], A);
}

static NO_INLINE MDFN_HOT uint32_t ExtBusRead_NI_w0_SP0_u32_BH0(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u32_BH0(&CPU[0], A);
}

static NO_INLINE MDFN_HOT uint32_t ExtBusRead_NI_w0_SP0_u32_BH1(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u32_BH1(&CPU[0], A);
}

static NO_INLINE MDFN_HOT uint8_t ExtBusRead_NI_w1_SP0_u8_BH0(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u8_BH0(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint8_t ExtBusRead_NI_w1_SP0_u8_BH1(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u8_BH1(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint16_t ExtBusRead_NI_w1_SP0_u16_BH0(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u16_BH0(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint16_t ExtBusRead_NI_w1_SP0_u16_BH1(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u16_BH1(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint32_t ExtBusRead_NI_w1_SP0_u32_BH0(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u32_BH0(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint32_t ExtBusRead_NI_w1_SP0_u32_BH1(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP0_u32_BH1(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint8_t ExtBusRead_NI_w1_SP1_u8_BH0(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP1_u8_BH0(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint8_t ExtBusRead_NI_w1_SP1_u8_BH1(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP1_u8_BH1(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint16_t ExtBusRead_NI_w1_SP1_u16_BH0(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP1_u16_BH0(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint16_t ExtBusRead_NI_w1_SP1_u16_BH1(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP1_u16_BH1(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint32_t ExtBusRead_NI_w1_SP1_u32_BH0(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP1_u32_BH0(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint32_t ExtBusRead_NI_w1_SP1_u32_BH1(uint32_t A)
{
 return SH7095_ExtBusRead_INLINE_SP1_u32_BH1(&CPU[1], A);
}

/* === ExtBusWrite_NI_w*_SP*_u**  (9 variants) === */

static NO_INLINE MDFN_HOT void ExtBusWrite_NI_w0_SP0_u8(uint32_t A, uint8_t V)
{
 SH7095_ExtBusWrite_INLINE_SP0_u8(&CPU[0], A, V);
}

static NO_INLINE MDFN_HOT void ExtBusWrite_NI_w0_SP0_u16(uint32_t A, uint16_t V)
{
 SH7095_ExtBusWrite_INLINE_SP0_u16(&CPU[0], A, V);
}

static NO_INLINE MDFN_HOT void ExtBusWrite_NI_w0_SP0_u32(uint32_t A, uint32_t V)
{
 SH7095_ExtBusWrite_INLINE_SP0_u32(&CPU[0], A, V);
}

static NO_INLINE MDFN_HOT void ExtBusWrite_NI_w1_SP0_u8(uint32_t A, uint8_t V)
{
 SH7095_ExtBusWrite_INLINE_SP0_u8(&CPU[1], A, V);
}

static NO_INLINE MDFN_HOT void ExtBusWrite_NI_w1_SP0_u16(uint32_t A, uint16_t V)
{
 SH7095_ExtBusWrite_INLINE_SP0_u16(&CPU[1], A, V);
}

static NO_INLINE MDFN_HOT void ExtBusWrite_NI_w1_SP0_u32(uint32_t A, uint32_t V)
{
 SH7095_ExtBusWrite_INLINE_SP0_u32(&CPU[1], A, V);
}

static NO_INLINE MDFN_HOT void ExtBusWrite_NI_w1_SP1_u8(uint32_t A, uint8_t V)
{
 SH7095_ExtBusWrite_INLINE_SP1_u8(&CPU[1], A, V);
}

static NO_INLINE MDFN_HOT void ExtBusWrite_NI_w1_SP1_u16(uint32_t A, uint16_t V)
{
 SH7095_ExtBusWrite_INLINE_SP1_u16(&CPU[1], A, V);
}

static NO_INLINE MDFN_HOT void ExtBusWrite_NI_w1_SP1_u32(uint32_t A, uint32_t V)
{
 SH7095_ExtBusWrite_INLINE_SP1_u32(&CPU[1], A, V);
}

static NO_INLINE MDFN_HOT uint8_t OnChipRegRead_NI_u8(uint32_t A)
{
 return SH7095_OnChipRegRead_INLINE_u8(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint16_t OnChipRegRead_NI_u16(uint32_t A)
{
 return SH7095_OnChipRegRead_INLINE_u16(&CPU[1], A);
}

static NO_INLINE MDFN_HOT uint32_t OnChipRegRead_NI_u32(uint32_t A)
{
 return SH7095_OnChipRegRead_INLINE_u32(&CPU[1], A);
}

//
//
//
static const struct
{
 uint8_t AND;
 uint8_t OR;
} LRU_Update_Tab[4] =
{
 { (1 << 2) | (1 << 1) | (1 << 0), /**/ (0 << 5) | (0 << 4) | (0 << 3) },	// Way 0
 { (1 << 4) | (1 << 3) | (1 << 0), /**/ (1 << 5) | (0 << 2) | (0 << 1) },	// Way 1
 { (1 << 5) | (1 << 3) | (1 << 1), /**/ (1 << 4) | (1 << 2) | (0 << 0) },	// Way 2
 { (1 << 5) | (1 << 4) | (1 << 2), /**/ (1 << 3) | (1 << 1) | (1 << 0) },	// Way 3
};

static const int8_t LRU_Replace_Tab[0x40] =
{
 /* 0x00 */ 0x03, 0x02,   -1, 0x02, 0x03,   -1, 0x01, 0x01,   -1, 0x02,   -1, 0x02,   -1,   -1, 0x01, 0x01,
 /* 0x10 */ 0x03,   -1,   -1,   -1, 0x03,   -1, 0x01, 0x01,   -1,   -1,   -1,   -1,   -1,   -1, 0x01, 0x01,
 /* 0x20 */ 0x03, 0x02,   -1, 0x02, 0x03,   -1,   -1,   -1,   -1, 0x02,   -1, 0x02,   -1,   -1,   -1,   -1,
 /* 0x30 */ 0x03,   -1,   -1,   -1, 0x03,   -1,   -1,   -1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static INLINE uint32_t cmov_eq_thing(const uint32_t reg_compval, const uint32_t mem_compval, uint32_t var, const uint32_t repl_var)
{
#if defined(__i386__) || defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(__i386) || defined(_M_IX86)
 asm(	"cmpl %1, %2\n\t"
	"cmove %3,%0\n\t"
	: "+r"(var)
	: "r"(reg_compval), "g"(mem_compval), "r"(repl_var)
	: "cc");
#else
#ifdef __GNUC__
  #warning "Conditional move inline assembly not being used."
#endif
 var = ((reg_compval == mem_compval) ? repl_var : var);
#endif
 return var;
}

static INLINE void SH7095_Cache_AssocPurge(SH7095* z, const uint32_t A)
{
 const uint32_t ATM = A & (0x7FFFF << 10);
 SH7095_CacheEntry* cent = &z->Cache[(A >> 4) & 0x3F];

 // Ignore two-way-mode bit in z->CCR here.
 cent->Tag[0] |= (ATM == cent->Tag[0]);	// Set invalid bit to 1.
 cent->Tag[1] |= (ATM == cent->Tag[1]);
 cent->Tag[2] |= (ATM == cent->Tag[2]);
 cent->Tag[3] |= (ATM == cent->Tag[3]);
}

#if defined(__SSE2__)
#include <emmintrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

static INLINE int SH7095_Cache_FindWay(SH7095* z, SH7095_CacheEntry* const cent, const uint32_t ATM)
{
#if defined(__SSE2__)
 __m128i atm = _mm_set1_epi32(ATM);
 __m128i v1234 = _mm_setr_epi32(1, 2, 3, 4);
 __m128i tmp;

 tmp = _mm_cmpeq_epi32(_mm_load_si128((__m128i*)cent->Tag), atm);
 tmp = _mm_and_si128(tmp, v1234);
 tmp = _mm_max_epi16(tmp, _mm_shuffle_epi32(tmp, (3 << 0) | (2 << 2) | (1 << 4) | (0 << 6)));
 tmp = _mm_max_epi16(tmp, _mm_shuffle_epi32(tmp, (1 << 0) | (0 << 2)));

 return _mm_cvtsi128_si32(tmp) - 1;
#elif defined(__aarch64__)
 // 4-way tag match in 5 NEON ops, replacing 4 dependent csels via cmov_eq_thing.
 // vmaxvq_u32 does horizontal max across the 128-bit register in one instruction.
 const uint32x4_t atm   = vdupq_n_u32(ATM);
 const uint32x4_t v1234 = { 1u, 2u, 3u, 4u };
 const uint32x4_t tags  = vld1q_u32(cent->Tag);
 const uint32x4_t hits  = vandq_u32(vceqq_u32(tags, atm), v1234);
 return (int)vmaxvq_u32(hits) - 1;
#else
 int way_match = -1;

 way_match = cmov_eq_thing(ATM, cent->Tag[0], way_match, 0);
 way_match = cmov_eq_thing(ATM, cent->Tag[1], way_match, 1);
 way_match = cmov_eq_thing(ATM, cent->Tag[2], way_match, 2);
 way_match = cmov_eq_thing(ATM, cent->Tag[3], way_match, 3);

 return way_match;
#endif
}

INLINE void SH7095_Cache_WriteUpdate_u8(SH7095* z, uint32_t A, uint8_t V)
{
 const uint32_t ATM = A & (0x7FFFF << 10);
 const unsigned ena = (A >> 4) & 0x3F;
 SH7095_CacheEntry* cent = &z->Cache[ena];
 const int way_match = SH7095_Cache_FindWay(z, cent, ATM);

 if(MDFN_LIKELY(way_match >= 0))
 {
  z->Cache_LRU[ena] = (z->Cache_LRU[ena] & LRU_Update_Tab[way_match].AND) | LRU_Update_Tab[way_match].OR;
  memcpy(&cent->Data[way_match][NE32ASU8_IDX_ADJ(uint8_t, A & 0x0F)], &V, sizeof(uint8_t));
 }
}

static INLINE void SH7095_Cache_WriteUpdate_u16(SH7095* z, uint32_t A, uint16_t V)
{
 const uint32_t ATM = A & (0x7FFFF << 10);
 const unsigned ena = (A >> 4) & 0x3F;
 SH7095_CacheEntry* cent = &z->Cache[ena];
 const int way_match = SH7095_Cache_FindWay(z, cent, ATM);

 if(MDFN_LIKELY(way_match >= 0))
 {
  z->Cache_LRU[ena] = (z->Cache_LRU[ena] & LRU_Update_Tab[way_match].AND) | LRU_Update_Tab[way_match].OR;
  memcpy(&cent->Data[way_match][NE32ASU8_IDX_ADJ(uint16_t, A & 0x0F)], &V, sizeof(uint16_t));
 }
}

static INLINE void SH7095_Cache_WriteUpdate_u32(SH7095* z, uint32_t A, uint32_t V)
{
 const uint32_t ATM = A & (0x7FFFF << 10);
 const unsigned ena = (A >> 4) & 0x3F;
 SH7095_CacheEntry* cent = &z->Cache[ena];
 const int way_match = SH7095_Cache_FindWay(z, cent, ATM);

 if(MDFN_LIKELY(way_match >= 0))
 {
  z->Cache_LRU[ena] = (z->Cache_LRU[ena] & LRU_Update_Tab[way_match].AND) | LRU_Update_Tab[way_match].OR;
  memcpy(&cent->Data[way_match][NE32ASU8_IDX_ADJ(uint32_t, A & 0x0F)], &V, sizeof(uint32_t));
 }
}

static INLINE void SH7095_Cache_WriteDataArray_u8(SH7095* z, uint32_t A, uint8_t V)
{
 const unsigned way = (A >> 10) & 0x3;
 const unsigned ena = (A >> 4) & 0x3F;

 memcpy(&z->Cache[ena].Data[way][NE32ASU8_IDX_ADJ(uint8_t, A & 0x0F)], &V, sizeof(uint8_t));
}

static INLINE void SH7095_Cache_WriteDataArray_u16(SH7095* z, uint32_t A, uint16_t V)
{
 const unsigned way = (A >> 10) & 0x3;
 const unsigned ena = (A >> 4) & 0x3F;

 memcpy(&z->Cache[ena].Data[way][NE32ASU8_IDX_ADJ(uint16_t, A & 0x0F)], &V, sizeof(uint16_t));
}

static INLINE void SH7095_Cache_WriteDataArray_u32(SH7095* z, uint32_t A, uint32_t V)
{
 const unsigned way = (A >> 10) & 0x3;
 const unsigned ena = (A >> 4) & 0x3F;

 memcpy(&z->Cache[ena].Data[way][NE32ASU8_IDX_ADJ(uint32_t, A & 0x0F)], &V, sizeof(uint32_t));
}

static INLINE uint8_t SH7095_Cache_ReadDataArray_u8(SH7095* z, uint32_t A)
{
 const unsigned way = (A >> 10) & 0x3;
 const unsigned ena = (A >> 4) & 0x3F;
 uint8_t tmp;

 memcpy(&tmp, &z->Cache[ena].Data[way][NE32ASU8_IDX_ADJ(uint8_t, A & 0x0F)], sizeof(uint8_t));
 return tmp;
}

static INLINE uint16_t SH7095_Cache_ReadDataArray_u16(SH7095* z, uint32_t A)
{
 const unsigned way = (A >> 10) & 0x3;
 const unsigned ena = (A >> 4) & 0x3F;
 uint16_t tmp;

 memcpy(&tmp, &z->Cache[ena].Data[way][NE32ASU8_IDX_ADJ(uint16_t, A & 0x0F)], sizeof(uint16_t));
 return tmp;
}

static INLINE uint32_t SH7095_Cache_ReadDataArray_u32(SH7095* z, uint32_t A)
{
 const unsigned way = (A >> 10) & 0x3;
 const unsigned ena = (A >> 4) & 0x3F;
 uint32_t tmp;

 memcpy(&tmp, &z->Cache[ena].Data[way][NE32ASU8_IDX_ADJ(uint32_t, A & 0x0F)], sizeof(uint32_t));
 return tmp;
}

/* Phase-8h: detempleted -- body never used T. */
static INLINE void SH7095_Cache_WriteAddressArray(SH7095* z, uint32_t A, uint32_t V)
{
 const unsigned way = (z->CCR >> 6) & 0x3;
 const unsigned ena = (A >> 4) & 0x3F;

 z->Cache[ena].Tag[way] = (A & (0x7FFFF << 10)) | (!(A & 0x4));
 z->Cache_LRU[ena] = (V >> 4) & 0x3F;
}

/* Phase-8h: detempleted -- body never used T; returns a 32-bit
 * tag value, callers truncate via `retval = (T)...` assignment. */
static INLINE uint32_t SH7095_Cache_ReadAddressArray(SH7095* z, uint32_t A)
{
 const unsigned way = (z->CCR >> 6) & 0x3;
 const unsigned ena = (A >> 4) & 0x3F;

 return (z->Cache[ena].Tag[way] & (0x7FFFF << 10)) | ((~z->Cache[ena].Tag[way] & 1) << 2) | (z->Cache_LRU[ena] << 4);
}

#define MemRead(IsInstr, T, region, CacheEnabled, A_, outval_, A, unmasked_A, cent, way_match)			\
{														\
 __label__ EBRCase, MemReturn;											\
 T retval;													\
 retval = 0;													\
 A = (A_);													\
 unmasked_A = A;												\
														\
 if(IsInstr <= 0)												\
 {														\
  if(MDFN_UNLIKELY(A & (sizeof(T) - 1)))									\
  {														\
   A &= ~(sizeof(T) - 1);											\
   SH7095_SetPEX(z, SH7095_PEX_CPUADDR);												\
  }														\
 }														\
														\
 if(IsInstr <= 0)												\
 { if((z->timestamp + 1) > z->MA_until) z->MA_until = (z->timestamp + 1); }						\
 else														\
  z->timestamp = ((sscpu_timestamp_t)(z->MA_until) > (sscpu_timestamp_t)(z->timestamp) ? (sscpu_timestamp_t)(z->MA_until) : (sscpu_timestamp_t)(z->timestamp));							\
														\
														\
 switch(region)													\
 {														\
  case 0:													\
	if(MDFN_LIKELY(CacheEnabled))										\
	{													\
	 uint32_t ATM;												\
														\
	 ATM = A & (0x7FFFF << 10);										\
	 cent = &z->Cache[(A >> 4) & 0x3F];									\
														\
	 way_match = SH7095_Cache_FindWay(z, cent, ATM);									\
														\
	 if(MDFN_UNLIKELY(way_match < 0)) /* z->Cache miss! */							\
	 {													\
	  way_match = LRU_Replace_Tab[z->Cache_LRU[(A >> 4) & 0x3F] & z->CCRC_Replace_AND] | z->CCRC_Replace_OR[(bool)IsInstr];		\
														\
	  if(MDFN_UNLIKELY(way_match < 0))									\
	   goto EBRCase;											\
														\
	  /*                        */										\
	  /* Load cache line.       */										\
	  /*                        */										\
	  cent->Tag[way_match] = ATM;										\
	  /* Don't use ATM after z point. */									\
	  CHECK_EXIT_RESUME();											\
	  {													\
	   unsigned di = (A + 4 + 0) & 0xC; { uint32_t vtmp__ = ExtBusRead(uint32_t, false, (A &~ 0xF) + di); memcpy(&cent->Data[way_match][di], &vtmp__, 4); }	\
	  }													\
	  for(unsigned i = 4; i < 16; i += 4)									\
	  {													\
	   unsigned di = (A + 4 + i) & 0xC; { uint32_t vtmp__ = ExtBusRead(uint32_t,  true, (A &~ 0xF) + di); memcpy(&cent->Data[way_match][di], &vtmp__, 4); }	\
	  }													\
	  if(IsInstr <= 0)											\
	  { if((SH7095_mem_timestamp + 1) > z->MA_until) z->MA_until = (SH7095_mem_timestamp + 1); }				\
	  else													\
	   z->timestamp = SH7095_mem_timestamp;									\
	 }													\
														\
	 									\
											\
	 z->Cache_LRU[(A >> 4) & 0x3F] = (z->Cache_LRU[(A >> 4) & 0x3F] & LRU_Update_Tab[way_match].AND) | LRU_Update_Tab[way_match].OR;	\
														\
         /* Ugggghhhh.... */											\
         if(CacheBypassHack && FMIsWriteable[A >> SH7095_EXT_MAP_GRAN_BITS])					\
	 {													\
          retval = SH7095_RBO_BE_T(T, SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS], A);				\
	  goto MemReturn;											\
	 }													\
														\
  	 memcpy(&retval, &cent->Data[way_match][NE32ASU8_IDX_ADJ(T, A & 0x0F)], sizeof(T));			\
	 goto MemReturn;											\
	}													\
	/* Fall-through, no break here */									\
  case 1:													\
	EBRCase:												\
	CHECK_EXIT_RESUME();											\
	{													\
	 retval = ExtBusRead(T, false, A);								\
														\
	 if(IsInstr <= 0)											\
	 { if((SH7095_mem_timestamp + 1) > z->MA_until) z->MA_until = (SH7095_mem_timestamp + 1); }				\
	 else													\
	 {													\
	  z->timestamp = SH7095_mem_timestamp;									\
	  z->UCRead_IF_Kludge = true;										\
	 }													\
														\
	 goto MemReturn;											\
	}													\
														\
  case 2:													\
  case 5:													\
	/*								*/					\
	/* Associative purge(apparently returns open bus of some sort)	*/					\
	/*								*/					\
	SH7095_Cache_AssocPurge(z, A);											\
	retval = ~0;												\
	goto MemReturn;												\
														\
  case 3:													\
	/*				   		*/							\
	/* Direct cache address/tag access 		*/							\
	/*				   		*/							\
	/* Note: bits 0, 1, 3, 29, 30, 31 are some sort	*/							\
	/*       of open-bus(unemulated). 		*/							\
	/*				   		*/							\
	retval = SH7095_Cache_ReadAddressArray(z, A);									\
	goto MemReturn;												\
														\
  case 4:													\
  case 6:													\
	/*			    */										\
	/* Direct cache data access */										\
	/*			    */										\
	if(sizeof(T) == 1) retval = SH7095_Cache_ReadDataArray_u8(z, A);							\
	else if(sizeof(T) == 2) retval = SH7095_Cache_ReadDataArray_u16(z, A);						\
	else retval = SH7095_Cache_ReadDataArray_u32(z, A);								\
	goto MemReturn;												\
														\
  case 7:													\
	retval = OnChipRegRead(T, unmasked_A);									\
	goto MemReturn;												\
 }														\
 MemReturn: outval_ = retval;											\
}

#define MemWrite(T, region, CacheEnabled, A_, V_, A, unmasked_A, V)		\
{										\
 A = (A_);									\
 unmasked_A = A;								\
 V = (V_);									\
										\
 if(MDFN_UNLIKELY(A & (sizeof(T) - 1)))						\
 {										\
  A &= ~(sizeof(T) - 1);							\
  SH7095_SetPEX(z, SH7095_PEX_CPUADDR);								\
 }										\
										\
 if((z->timestamp + 1) > z->MA_until) z->MA_until = (z->timestamp + 1);		\
										\
										\
 switch(region)									\
 {										\
  case 0:									\
	if(MDFN_LIKELY(CacheEnabled))						\
	{									\
	 if(sizeof(T) == 1) SH7095_Cache_WriteUpdate_u8(z, A, V);		\
	 else if(sizeof(T) == 2) SH7095_Cache_WriteUpdate_u16(z, A, V);		\
	 else SH7095_Cache_WriteUpdate_u32(z, A, V);				\
	}									\
	/* Fall-through, no break */						\
  case 1:									\
  	if((z->write_finish_timestamp + 1) > z->MA_until) z->MA_until = (z->write_finish_timestamp + 1);	\
										\
	if(!SH7095_BusLock) { CHECK_EXIT_RESUME(); }				\
	ExtBusWrite(T, A, V);							\
	break;									\
										\
  case 2:									\
  case 5:									\
	/*		      */						\
	/* Associative purge. */						\
	/*		      */						\
	SH7095_Cache_AssocPurge(z, A);							\
	break;									\
										\
  case 3:									\
	/*				   */					\
	/* Direct cache address/tag access */					\
	/*				   */					\
	/* TODO: Check non-32 bit access   */					\
	z->timestamp++;								\
	if((z->timestamp + 1) > z->MA_until) z->MA_until = (z->timestamp + 1);	\
										\
	SH7095_Cache_WriteAddressArray(z, A, V);					\
	break;									\
										\
  case 4:									\
  case 6:									\
	/*			    */						\
	/* Direct cache data access */						\
	/*			    */						\
	if(sizeof(T) == 1) SH7095_Cache_WriteDataArray_u8(z, A, V);					\
	else if(sizeof(T) == 2) SH7095_Cache_WriteDataArray_u16(z, A, V);					\
	else SH7095_Cache_WriteDataArray_u32(z, A, V);							\
	break;									\
										\
  case 7:									\
	if(sizeof(T) == 1)      SH7095_OnChipRegWrite_u8(z, unmasked_A, V);	\
	else if(sizeof(T) == 2) SH7095_OnChipRegWrite_u16(z, unmasked_A, V);	\
	else                    SH7095_OnChipRegWrite_u32(z, unmasked_A, V);	\
	break;									\
 }										\
}

#define CHECK_EXIT_RESUME() { if(NeedSlaveCall > 0) SH7095_RunSlaveUntil(&CPU[1], z->timestamp); if(NeedSlaveCall < 0) SH7095_RunSlaveUntil_Debug(&CPU[1], z->timestamp); }
#define OnChipRegRead(T, A)							\
        ( (sizeof(T) == 1) ? (T)SH7095_OnChipRegRead_INLINE_u8(z, A)		\
        : (sizeof(T) == 2) ? (T)SH7095_OnChipRegRead_INLINE_u16(z, A)		\
                           : (T)SH7095_OnChipRegRead_INLINE_u32(z, A) )
/* Phase-8p: master-CPU dispatch.  `which` is the outer template arg
 * (compile-time const, 0 or 1), sizeof(T) is concrete at macro
 * expansion, BurstHax is a literal at every call site -- gcc -O2
 * folds the ternary chain to a single direct call. */
#define ExtBusRead(T, BurstHax, A)                                                                                            \
          ( (sizeof(T) == 1) ? (T)((BurstHax) ? ((which) ? ExtBusRead_NI_w1_SP0_u8_BH1(A)  : ExtBusRead_NI_w0_SP0_u8_BH1(A))   \
                                              : ((which) ? ExtBusRead_NI_w1_SP0_u8_BH0(A)  : ExtBusRead_NI_w0_SP0_u8_BH0(A)))  \
          : (sizeof(T) == 2) ? (T)((BurstHax) ? ((which) ? ExtBusRead_NI_w1_SP0_u16_BH1(A) : ExtBusRead_NI_w0_SP0_u16_BH1(A))  \
                                              : ((which) ? ExtBusRead_NI_w1_SP0_u16_BH0(A) : ExtBusRead_NI_w0_SP0_u16_BH0(A))) \
                             : (T)((BurstHax) ? ((which) ? ExtBusRead_NI_w1_SP0_u32_BH1(A) : ExtBusRead_NI_w0_SP0_u32_BH1(A))  \
                                              : ((which) ? ExtBusRead_NI_w1_SP0_u32_BH0(A) : ExtBusRead_NI_w0_SP0_u32_BH0(A))) )
#define ExtBusWrite(T, A, V)                                                                                       \
do {                                                                                                                \
  if(sizeof(T) == 1)      { if(which) ExtBusWrite_NI_w1_SP0_u8 (A, V); else ExtBusWrite_NI_w0_SP0_u8 (A, V); }      \
  else if(sizeof(T) == 2) { if(which) ExtBusWrite_NI_w1_SP0_u16(A, V); else ExtBusWrite_NI_w0_SP0_u16(A, V); }      \
  else                    { if(which) ExtBusWrite_NI_w1_SP0_u32(A, V); else ExtBusWrite_NI_w0_SP0_u32(A, V); }      \
} while(0)
/* Phase-9 step 5 part 2: 196 named C functions replace the four
 * mega-templates that used to live here.  Position matches the old
 * defs -- MemRead / MemWrite / ExtBusRead / ExtBusWrite / OnChipRegRead /
 * CHECK_EXIT_RESUME macros are all in scope at this point. */
#include "sh7095_mem_dispatch.inc"

#undef CHECK_EXIT_RESUME
#undef OnChipRegRead
#undef ExtBusRead
#undef ExtBusWrite

static INLINE void SH7095_SetCCR(SH7095* z, uint8_t V)
{
 if(V & SH7095_CCR_CP)
 {
  for(unsigned entry = 0; entry < 64; entry++)
  {
   z->Cache_LRU[entry] = 0;
   for(unsigned way = 0; way < 4; way++)
    z->Cache[entry].Tag[way] |= 1;	// Set invalid bit to 1.
  }
  V &= ~SH7095_CCR_CP;
 }

 //if(MDFN_LIKELY(z->CCR != V))
 {
  z->CCR = V;
  //
  //
  z->CCRC_Replace_AND = (z->CCR & SH7095_CCR_TW) ? 0x1 : 0x3F;
  z->CCRC_Replace_OR[0] = (z->CCR & SH7095_CCR_OD) ? -1 : 0;
  z->CCRC_Replace_OR[1] = (z->CCR & SH7095_CCR_ID) ? -1 : 0;

  SH7095_RecalcMRWFP_0(z);
 }
}

/*

*/
NO_INLINE void SH7095_Reset(SH7095* z, bool power_on_reset, bool from_internal_wdt)
{
 if(z->timestamp == SS_EVENT_DISABLED_TS)
  return;

 if(power_on_reset)
 {
  z->FRT.lastts = z->timestamp;

  z->MA_until = z->timestamp;
  z->MM_until = z->timestamp;

  for(unsigned i = 0; i < 16; i++)
   z->WB_until[i] = z->timestamp;

  z->UCRead_IF_Kludge = 0;

  z->write_finish_timestamp = z->timestamp;
  z->divide_finish_timestamp = z->timestamp;

  z->DMA_Timestamp = z->timestamp;
  z->DMA_SGEndTimestamp = z->timestamp;
 }
 //
 z->VBR = 0;
 z->SR |= 0xF << 4;
 SH7095_SetCCR(z, 0);
 //
 if(power_on_reset)
 {
  z->BSC.BCR1 = (z->BSC.BCR1 & 0x8000) | 0x03F0;
  z->BSC.BCR2 = 0xFC;
  z->BSC.WCR = 0xAAFF;
  z->BSC.MCR = 0x0000;

  z->BSC.sdram_finish_time = z->timestamp;
  z->BSC.last_mem_time = z->timestamp;
  z->BSC.last_mem_addr = 0;
  z->BSC.last_mem_type = 0;

  z->BSC.RTCSR = 0x00;
  z->BSC.RTCSRM = 0x00;
  z->BSC.RTCNT = 0x00;
  z->BSC.RTCOR = 0x00;
 }
 //

 for(unsigned ch = 0; ch < 2; ch++)
 {
  z->DMACH[ch].CHCR = 0x00;
  z->DMACH[ch].CHCRM = 0x00;
  z->DMACH[ch].DRCR = 0x00;
 }
 z->DMAOR = 0x00;
 SH7095_DMA_RecalcRunning(z);
 SH7095_RecalcPendingIntPEX(z);
 //
 SH7095_INTC_Reset(z);
 //
 z->DVCR = 0;
 SH7095_RecalcPendingIntPEX(z);
 //
 SH7095_FRT_Reset(z);
 SH7095_WDT_Reset(z, from_internal_wdt);
 //
 SH7095_SCI_Reset(z);
 //
 z->SBYCR = 0;
 z->Standby = false;
 //
 //
 //
 z->EPending = 0;
 SH7095_SetPEX(z, power_on_reset ? SH7095_PEX_POWERON : SH7095_PEX_RESET);
 z->Pipe_ID = z->EPending;
 z->resume_id = 0;
}

static NO_INLINE void SH7095_INTC_Reset(SH7095* z)
{
 z->IPRA = 0;
 z->IPRB = 0;
 z->VCRA = 0;
 z->VCRB = 0;
 z->VCRC = 0;
 z->VCRD = 0;
 z->VCRWDT = 0;
 z->ICR = 0;

 SH7095_RecalcPendingIntPEX(z);
}

void SH7095_SetNMI(SH7095* z, bool level)
{
 if(z->NMILevel != level && level == (bool)(z->ICR & 0x100))
 {
  SH7095_SetPEX(z, SH7095_PEX_NMI);

  if(z->Standby)
  {
   z->WDT.WTCSR |= 0x08;
   SH7095_FRT_WDT_Recalc_NET(z);
  }
  else
  {
   z->DMAOR |= 0x02;	// TODO: NMIF; set always, or only when DMA was active?
   SH7095_DMA_RecalcRunning(z);
  }
 }
 z->NMILevel = level;
}

void SH7095_SetMD5(SH7095* z, bool level)
{
 z->BSC.BCR1 = (z->BSC.BCR1 & 0x7FFF) | (level << 15);
}

void SH7095_SetIRL(SH7095* z, unsigned level)
{
 assert(level < 16);

 z->IRL = level;
 SH7095_RecalcPendingIntPEX(z);
}

void SH7095_ForceInternalEventUpdates(SH7095* z)
{
 if(MDFN_UNLIKELY(z->timestamp == SS_EVENT_DISABLED_TS))
  return;

 SH7095_FRT_WDT_Update(z);
 SH7095_FRT_WDT_Recalc_NET(z);
}

//
// Default priority(for same ipr value), highest to lowest:
//  NMI
//  User break
//  IRL15
//  [...]
//  IRL1
//  DIVU
//  DMAC0
//  DMAC1
//  z->WDT
//  REF
//  z->SCI-ERI
//  z->SCI-RXI
//  z->SCI-TXI
//  z->SCI-TEI
//  z->FRT-ICI
//  z->FRT-OCI
//  z->FRT-OVI
//
//
static INLINE uint8_t SH7095_GetPendingInt(SH7095* z, uint8_t* vecnum_out)
{
 unsigned ipr;
 unsigned vecnum;
 unsigned tmp_ipr;

 ipr = z->IRL;
 vecnum = (z->IRL >> 1) + SH7095_VECNUM_INT_BASE;

 if(vecnum_out && (z->ICR & 0x1) && z->IRL > 0) // External vec fetch has side effects, make sure to only do it if vecnum_out is non-NULL and z->ICR & 0x1, and if z is the interrupt being serviced.
  vecnum = ~0U;

 //
 //
 //

 if((z->DVCR & 0x3) == 0x3 && (tmp_ipr = ((z->IPRA >> 12) & 0xF)) > ipr)
 {
  ipr = tmp_ipr;
  vecnum = (z->VCRDIV & 0x7F);
 }

 for(unsigned ch = 0; ch < 2; ch++)
 {
  if((z->DMACH[ch].CHCR & 0x6) == 0x6 && (tmp_ipr = ((z->IPRA >> 8) & 0xF)) > ipr)
  {
   ipr = tmp_ipr;
   vecnum = (z->DMACH[ch].VCR & 0x7F);
  }
 }

 if((z->WDT.WTCSR & 0x80) && (tmp_ipr = ((z->IPRA >> 4) & 0xF)) > ipr)
 {
  ipr = tmp_ipr;
  vecnum = (z->VCRWDT >> 8) & 0x7F;
 }

 //
 //
 //
 //
 //
 //
 const uint32_t frt_ip_tmp = (z->FRT.FTCSR & z->FRT.TIER & 0x8E);
 if(frt_ip_tmp && (tmp_ipr = ((z->IPRB >> 8) & 0xF)) > ipr)
 {
  ipr = tmp_ipr;

  if(frt_ip_tmp & 0x80)	// ICI
   vecnum = (z->VCRC >> 8) & 0x7F;
  else if(frt_ip_tmp & 0x0C)	// OCIA+OCIB
   vecnum = (z->VCRC >> 0) & 0x7F;
  else 			// OVI
   vecnum = (z->VCRD >> 8) & 0x7F;
 }

 if(vecnum_out)
 {
  if(vecnum == ~0U)
   vecnum = z->ExIVecFetch();

  *vecnum_out = vecnum;
 }

 return ipr;
}

//
// Call after changes to:
//	z->IRL
//	z->SR
//
//	z->IPRA
//	z->IPRB
//
//	z->DMACH[*].CHCR
//
//	z->DVCR
//
//	z->FRT.FTCSR
//	z->FRT.TIER
//
static NO_INLINE void SH7095_RecalcPendingIntPEX(SH7095* z)
{
 if(SH7095_GetPendingInt(z, NULL) > ((z->SR >> 4) & 0xF))
  SH7095_SetPEX(z, SH7095_PEX_INT);
 else
  SH7095_ClearPEX(z, SH7095_PEX_INT);
}

static const uint8_t InstrDecodeTab[65536] =
{
 #include "sh7095_idecodetab.inc"
};

#ifdef SH7095_OP_PAIR_PROFILE
namespace {
 struct OpPairProf {
  uint32_t counter[256][256];
  uint8_t prev_op;
 };
 OpPairProf op_pair_prof[2];
}

#define OP_PAIR_TICK(which_idx)							\
 do {										\
  const unsigned _wi = (which_idx);						\
  const uint8_t _curr = (uint8_t)(z->Pipe_ID >> 24);					\
  op_pair_prof[_wi].counter[op_pair_prof[_wi].prev_op][_curr]++;		\
  op_pair_prof[_wi].prev_op = _curr;						\
 } while(0)

void SS_DumpOpPairProfile(void)
{
 const char* paths[2] = { "/tmp/op_pairs_master.bin", "/tmp/op_pairs_slave.bin" };
 for(unsigned i = 0; i < 2; i++) {
  FILE* f = fopen(paths[i], "wb");
  if(!f) continue;
  fwrite(&op_pair_prof[i].counter[0][0], sizeof(uint32_t), 256u * 256u, f);
  fclose(f);
 }
}
#else
#define OP_PAIR_TICK(which_idx) do {} while(0)
#endif

/*								*/
/* TODO: Stop reading from memory when an exception is pending? */
/*								*/
#define FetchIF()							\
{									\
 if(EmulateICache)							\
 {									\
  z->Pipe_IF = (uint16_t)z->IBuffer;						\
									\
  if(!(z->PC & 0x2))							\
  {									\
   MemReadInstr(z->PC, z->IBuffer);						\
   z->Pipe_IF = z->IBuffer >> 16;						\
  }									\
 }									\
 else									\
 {									\
  if(z->timestamp < (z->MA_until - ((int32_t)(z->PC & 0x2) << 28)))		\
   z->timestamp = z->MA_until;						\
									\
  z->Pipe_IF = *(uint16_t*)(SH7095_FastMap[z->PC >> SH7095_EXT_MAP_GRAN_BITS] + z->PC);	\
									\
  if(MDFN_UNLIKELY((int32_t)z->PC < 0))      /* Mr. Boooones */		\
   z->Pipe_IF = SH7095_Cache_ReadDataArray_u16(z, z->PC);				\
 }									\
 z->timestamp++;								\
}

#define FetchIF_ForceIBufferFill()					\
{									\
 if(EmulateICache)							\
 {									\
  MemReadInstr(z->PC &~ 2, z->IBuffer);					\
  /*z->Pipe_IF = (uint16_t)(z->IBuffer >> (((z->PC & 2) ^ 2) << 3));*/		\
  z->Pipe_IF = (uint16_t)z->IBuffer;						\
  if(!(z->PC & 0x2))							\
   z->Pipe_IF = z->IBuffer >> 16;						\
 }									\
 else									\
 {									\
  if(z->timestamp < z->MA_until)						\
   z->timestamp = z->MA_until;						\
									\
  z->Pipe_IF = *(uint16_t*)(SH7095_FastMap[z->PC >> SH7095_EXT_MAP_GRAN_BITS] + z->PC);	\
									\
  if(MDFN_UNLIKELY((int32_t)z->PC < 0))      /* Mr. Boooones */		\
   z->Pipe_IF = SH7095_Cache_ReadDataArray_u16(z, z->PC);				\
 }									\
 z->timestamp++;								\
}

#define DoID(IntPreventNext)						\
{									\
 uint32_t op = InstrDecodeTab[z->Pipe_IF];					\
 uint32_t epo = z->EPending;							\
									\
 if(IntPreventNext)							\
 {									\
  epo &= ~(1U << (SH7095_PEX_INT + SH7095_EPENDING_PEXBITS_SHIFT));			\
  if(!(epo & (0xFF << SH7095_EPENDING_PEXBITS_SHIFT)))				\
   epo = 0;								\
 }									\
									\
 z->Pipe_ID = z->Pipe_IF | (op << 24) | epo;					\
}

#define DoIDIF_MACRO(IntPreventNext)					\
{									\
 DoID(IntPreventNext);							\
 FetchIF();								\
}

/* */
/* Not totally correct, but simplifies things...probably :p */
/* */
#define DelayBranch(target_)						\
{									\
 {									\
  const uint32_t target = (target_);					\
  z->PC = target;								\
 }									\
									\
 /* Redecode the opcode from the 16-bit instruction to makes sure */	\
 /* exceptions won't be taken in the delay slot(due to op field   */	\
 /* being forced to 0xFF by OR'ing with z->EPending in the previous  */ 	\
 /* call to DoID()), and forces usage of the second half of the   */	\
 /* opcode table so we can generate illegal slot instruction      */	\
 /* exceptions as appropriate. */					\
 z->Pipe_ID = (uint16_t)z->Pipe_ID;						\
 z->Pipe_ID |= (InstrDecodeTab[z->Pipe_ID] | 0x80) << 24;			\
									\
 if(MDFN_UNLIKELY(z->PC & 1))						\
 {									\
  /* Pending for the instruction after the delay slot instruction. */	\
  SH7095_SetPEX(z, SH7095_PEX_CPUADDR);							\
 }									\
 else									\
 {									\
  /* This discards the previously-fetched instruction after the */	\
  /* delay slot instruction.  Poor, poor instruction. */		\
  if(EmulateICache)							\
   z->UCRead_IF_Kludge = false;						\
  FetchIF_ForceIBufferFill()						\
  if(EmulateICache)							\
   z->timestamp -= (z->PC >> 1) & z->UCRead_IF_Kludge; /* WTFBBQ?! */		\
 }									\
}

#define Branch(target_)					\
{									\
 {									\
  const uint32_t target = (target_);					\
									\
  z->PC = target;								\
 }									\
									\
 if(MDFN_UNLIKELY(z->PC & 1))						\
 {									\
  SH7095_SetPEX(z, SH7095_PEX_CPUADDR);							\
  DoID(false);								\
 }									\
 else									\
 {									\
  /* This sequence discards the previously-fetched two instructions */	\
  /* after the branch instruction. */					\
  if(EmulateICache)							\
   z->UCRead_IF_Kludge = false;						\
  FetchIF_ForceIBufferFill();						\
  if(EmulateICache)							\
   z->timestamp -= (z->PC >> 1) & z->UCRead_IF_Kludge; /* WTFBBQ?! */		\
  z->PC += 2;								\
  DoIDIF(false);							\
 }									\
}

/* Remember to use with BEGIN_OP_DLYIDIF instead of BEGIN_OP */
#define UCDelayBranch(target)						\
{									\
 z->PC = target;								\
									\
 z->Pipe_ID = z->Pipe_IF | ((InstrDecodeTab[z->Pipe_IF] | 0x80) << 24);		\
									\
 z->timestamp++;								\
									\
 if(MDFN_UNLIKELY(z->PC & 1))						\
 {									\
  /* Pending for the instruction after the delay slot instruction. */	\
  SH7095_SetPEX(z, SH7095_PEX_CPUADDR);							\
 }									\
 else									\
 {									\
  FetchIF_ForceIBufferFill();						\
 }									\
}

/* Remember to use with BEGIN_OP_DLYIDIF instead of BEGIN_OP */
#define UCRelDelayBranch(disp) UCDelayBranch(z->PC + (disp))

#define CondRelBranch(cond, disp)	\
{					\
 if(cond)				\
  Branch(z->PC + (disp));	\
}

#define CondRelDelayBranch(cond, disp)	\
{					\
 if(cond)				\
  DelayBranch(z->PC + (disp));		\
}

/* SH7095_Reset_TBD_/Poweron exception handling. */
#define Exception_ResPow(vecnum, new_PC)				\
{									\
 z->timestamp += 2;							\
 z->timestamp = ((sscpu_timestamp_t)(z->MA_until) > (sscpu_timestamp_t)(z->timestamp) ? (sscpu_timestamp_t)(z->MA_until) : (sscpu_timestamp_t)(z->timestamp));		\
									\
 MemRead32(((vecnum) + 0) << 2, new_PC);				\
 MemRead32(((vecnum) + 1) << 2, z->R[15]);					\
}

#define Exception(vecnum, new_PC)				\
{									\
 z->timestamp += 2;							\
 z->timestamp = ((sscpu_timestamp_t)(z->MA_until) > (sscpu_timestamp_t)(z->timestamp) ? (sscpu_timestamp_t)(z->MA_until) : (sscpu_timestamp_t)(z->timestamp));		\
									\
 /* Save z->SR to stack */							\
 /* Save z->PC to stack */							\
 /* Read exception vector table */					\
 z->R[15] -= 4;								\
 MemWrite32(z->R[15], z->SR);							\
 z->timestamp++;								\
 z->R[15] -= 4;								\
 MemWrite32(z->R[15], z->PC);							\
 z->timestamp++;								\
 z->timestamp++;								\
 MemRead32(z->VBR + ((vecnum) << 2), new_PC);				\
 z->timestamp++;								\
									\
}

/*
 z->PC == 0
 instr = [?]
 ID = [?]
 IF = [0]

 z->PC == 2
 instr = [?]
 ID = [0]
 IF = [2]

 z->PC == 4
 instr = [0]
 ID = [2]
 IF = [4]

*/

/* Phase-8n: DoIDIF macro dispatches to one of 4 named
 * NO_INLINE variants based on (EmulateICache, IntPreventNext).
 * Both bools are compile-time at macro expansion (EmulateICache
 * is the enclosing Step/RunLoop template arg, IntPreventNext
 * is a macro arg passed as a literal at every callsite), so
 * gcc -O2 folds the 4-way if-chain to a single call. */
#define DoIDIF(IntPreventNext)							\
do {										\
 if(EmulateICache)								\
 {										\
  if(IntPreventNext) SH7095_DoIDIF_NI_C1_I1(z);						\
  else               SH7095_DoIDIF_NI_C1_I0(z);						\
 }										\
 else										\
 {										\
  if(IntPreventNext) SH7095_DoIDIF_NI_C0_I1(z);						\
  else               SH7095_DoIDIF_NI_C0_I0(z);						\
 }										\
} while(0)

#define OnChipRegRead(T, A)							\
        ( (sizeof(T) == 1) ? (T)SH7095_OnChipRegRead_INLINE_u8(z, A)		\
        : (sizeof(T) == 2) ? (T)SH7095_OnChipRegRead_INLINE_u16(z, A)		\
                           : (T)SH7095_OnChipRegRead_INLINE_u32(z, A) )
/* Phase-8p: master-CPU dispatch.  `which` is the outer template arg
 * (compile-time const, 0 or 1), sizeof(T) is concrete at macro
 * expansion, BurstHax is a literal at every call site -- gcc -O2
 * folds the ternary chain to a single direct call. */
#define ExtBusRead(T, BurstHax, A)                                                                                            \
          ( (sizeof(T) == 1) ? (T)((BurstHax) ? ((which) ? ExtBusRead_NI_w1_SP0_u8_BH1(A)  : ExtBusRead_NI_w0_SP0_u8_BH1(A))   \
                                              : ((which) ? ExtBusRead_NI_w1_SP0_u8_BH0(A)  : ExtBusRead_NI_w0_SP0_u8_BH0(A)))  \
          : (sizeof(T) == 2) ? (T)((BurstHax) ? ((which) ? ExtBusRead_NI_w1_SP0_u16_BH1(A) : ExtBusRead_NI_w0_SP0_u16_BH1(A))  \
                                              : ((which) ? ExtBusRead_NI_w1_SP0_u16_BH0(A) : ExtBusRead_NI_w0_SP0_u16_BH0(A))) \
                             : (T)((BurstHax) ? ((which) ? ExtBusRead_NI_w1_SP0_u32_BH1(A) : ExtBusRead_NI_w0_SP0_u32_BH1(A))  \
                                              : ((which) ? ExtBusRead_NI_w1_SP0_u32_BH0(A) : ExtBusRead_NI_w0_SP0_u32_BH0(A))) )
#define ExtBusWrite(T, A, V)                                                                                       \
do {                                                                                                                \
  if(sizeof(T) == 1)      { if(which) ExtBusWrite_NI_w1_SP0_u8 (A, V); else ExtBusWrite_NI_w0_SP0_u8 (A, V); }      \
  else if(sizeof(T) == 2) { if(which) ExtBusWrite_NI_w1_SP0_u16(A, V); else ExtBusWrite_NI_w0_SP0_u16(A, V); }      \
  else                    { if(which) ExtBusWrite_NI_w1_SP0_u32(A, V); else ExtBusWrite_NI_w0_SP0_u32(A, V); }      \
} while(0)

#define MemReadInstr(A, outval) { outval = (z->MRFPI[(A) >> 29](A)); }
#define MemRead8(A, outval) { outval = (z->MRFP8[(A) >> 29](A)); }
#define MemRead16(A, outval) { outval = (z->MRFP16[(A) >> 29](A)); }
#define MemRead32(A, outval) { outval = (z->MRFP32[(A) >> 29](A)); }
#define MemRead16_I(A, outval) { outval = (z->MRFP16_I[(A) >> 29](A)); }
#define MemRead32_I(A, outval) { outval = (z->MRFP32_I[(A) >> 29](A)); }

#define MemWrite8(A, V) z->MWFP8[(A) >> 29]((A), (V));
#define MemWrite16(A, V) z->MWFP16[(A) >> 29]((A), (V));
#define MemWrite32(A, V) z->MWFP32[(A) >> 29]((A), (V));
#define CHECK_EXIT_RESUME() { if(EmulateICache && !which) { SH7095_RunSlaveUntil(&CPU[1], z->timestamp); } }
#define CONST_VAR(T, n) const T n
#define RESUME_VAR(T, n) T n

static INLINE void SH7095_DoIDIF_INLINE_C0_I0(SH7095* z)
{
 const bool EmulateICache = false;
 DoIDIF_MACRO(false);
}

static INLINE void SH7095_DoIDIF_INLINE_C0_I1(SH7095* z)
{
 const bool EmulateICache = false;
 DoIDIF_MACRO(true);
}

static INLINE void SH7095_DoIDIF_INLINE_C1_I0(SH7095* z)
{
 const bool EmulateICache = true;
 DoIDIF_MACRO(false);
}

static INLINE void SH7095_DoIDIF_INLINE_C1_I1(SH7095* z)
{
 const bool EmulateICache = true;
 DoIDIF_MACRO(true);
}

static NO_INLINE MDFN_HOT void SH7095_DoIDIF_NI_C0_I0(SH7095* z)
{
 SH7095_DoIDIF_INLINE_C0_I0(z);
}

static NO_INLINE MDFN_HOT void SH7095_DoIDIF_NI_C0_I1(SH7095* z)
{
 SH7095_DoIDIF_INLINE_C0_I1(z);
}

static NO_INLINE MDFN_HOT void SH7095_DoIDIF_NI_C1_I0(SH7095* z)
{
 SH7095_DoIDIF_INLINE_C1_I0(z);
}

static NO_INLINE MDFN_HOT void SH7095_DoIDIF_NI_C1_I1(SH7095* z)
{
 SH7095_DoIDIF_INLINE_C1_I1(z);
}

FORCE_INLINE void SH7095_Step_w0_C0(SH7095* z)
{
 const unsigned which = 0;
 const bool EmulateICache = false;
 //
 // Ideally, we would place SPEPRecover: after the z->FRT event check, but doing
 // so causes gcc(multiple versions) to produce inconceivably awful code under certain conditions
 // (such as disabling all the SS_DBG stuff at compile-time) because it thinks it's an important loop
 // or something?(even with all our branch hinting!)
 //
 SPEPRecover:;

 if(MDFN_UNLIKELY(z->timestamp >= z->FRT_WDT_NextTS))
 {
  SH7095_FRT_WDT_Update(z);
  SH7095_FRT_WDT_Recalc_NET(z);
 }

 const uint32_t instr = (uint16_t)z->Pipe_ID;
 const unsigned instr_nyb1 = (instr >> 4) & 0xF;
 const unsigned instr_nyb2 = (instr >> 8) & 0xF;
// asm volatile("":: "a"(instr_nyb1), "d"(instr_nyb2));
 OP_PAIR_TICK(which);
 //
 #include "sh7095_ops.inc"
 //
}

FORCE_INLINE void SH7095_Step_w0_C1(SH7095* z)
{
 const unsigned which = 0;
 const bool EmulateICache = true;
 SPEPRecover:;

 if(MDFN_UNLIKELY(z->timestamp >= z->FRT_WDT_NextTS))
 {
  SH7095_FRT_WDT_Update(z);
  SH7095_FRT_WDT_Recalc_NET(z);
 }

 const uint32_t instr = (uint16_t)z->Pipe_ID;
 const unsigned instr_nyb1 = (instr >> 4) & 0xF;
 const unsigned instr_nyb2 = (instr >> 8) & 0xF;
 OP_PAIR_TICK(which);
 //
 #include "sh7095_ops.inc"
 //
}

FORCE_INLINE void SH7095_Step_w1_C0(SH7095* z)
{
 const unsigned which = 1;
 const bool EmulateICache = false;
 SPEPRecover:;

 if(MDFN_UNLIKELY(z->timestamp >= z->FRT_WDT_NextTS))
 {
  SH7095_FRT_WDT_Update(z);
  SH7095_FRT_WDT_Recalc_NET(z);
 }

 const uint32_t instr = (uint16_t)z->Pipe_ID;
 const unsigned instr_nyb1 = (instr >> 4) & 0xF;
 const unsigned instr_nyb2 = (instr >> 8) & 0xF;
 OP_PAIR_TICK(which);
 //
 #include "sh7095_ops.inc"
 //
}

#undef CHECK_EXIT_RESUME
#undef DoIDIF
#undef OnChipRegRead
#undef ExtBusRead
#undef ExtBusWrite
#undef MemReadInstr
#undef MemRead8
#undef MemRead16
#undef MemRead32
#undef MemRead16_I
#undef MemRead32_I
#undef MemWrite8
#undef MemWrite16
#undef MemWrite32
#undef CONST_VAR
#undef RESUME_VAR
//
//
//
//
//
#if defined(__GNUC__) && !defined(__clang__)
 #pragma GCC push_options
 #pragma GCC optimize("O2,no-gcse") //no-unroll-loops,no-peel-loops,no-crossjumping")
#endif

#define DoIDIF(IntPreventNext) DoIDIF_MACRO(IntPreventNext)

#define OnChipRegRead(T, A)							\
        ( (sizeof(T) == 1) ? (T)OnChipRegRead_NI_u8(A)				\
        : (sizeof(T) == 2) ? (T)OnChipRegRead_NI_u16(A)				\
                           : (T)OnChipRegRead_NI_u32(A) )
/* Phase-8p: slave-CPU dispatch (w=1, SP=true fixed). */
#define ExtBusRead(T, BurstHax, A)                                              \
          ( (sizeof(T) == 1) ? (T)((BurstHax) ? ExtBusRead_NI_w1_SP1_u8_BH1(A)  : ExtBusRead_NI_w1_SP1_u8_BH0(A))   \
          : (sizeof(T) == 2) ? (T)((BurstHax) ? ExtBusRead_NI_w1_SP1_u16_BH1(A) : ExtBusRead_NI_w1_SP1_u16_BH0(A))  \
                             : (T)((BurstHax) ? ExtBusRead_NI_w1_SP1_u32_BH1(A) : ExtBusRead_NI_w1_SP1_u32_BH0(A))  )
#define ExtBusWrite(T, A, V)                            \
do {                                                    \
  if(sizeof(T) == 1)      ExtBusWrite_NI_w1_SP1_u8 (A, V); \
  else if(sizeof(T) == 2) ExtBusWrite_NI_w1_SP1_u16(A, V); \
  else                    ExtBusWrite_NI_w1_SP1_u32(A, V); \
} while(0)

#define MemReadInstr(A, outval) MemRead(true,  uint32_t, ((A) >> 29), (z->CCR & SH7095_CCR_CE), (A), outval, /**/ z->Resume_uint32_A, z->Resume_unmasked_A, z->Resume_cent, z->Resume_way_match)
#define MemRead8(A, outval)	MemRead(false, uint8_t,  ((A) >> 29), (z->CCR & SH7095_CCR_CE), (A), outval, /**/ z->Resume_uint8_A,  z->Resume_unmasked_A, z->Resume_cent, z->Resume_way_match)
#define MemRead16(A, outval)	MemRead(false, uint16_t, ((A) >> 29), (z->CCR & SH7095_CCR_CE), (A), outval, /**/ z->Resume_uint16_A, z->Resume_unmasked_A, z->Resume_cent, z->Resume_way_match)
#define MemRead32(A, outval)	MemRead(false, uint32_t, ((A) >> 29), (z->CCR & SH7095_CCR_CE), (A), outval, /**/ z->Resume_uint32_A, z->Resume_unmasked_A, z->Resume_cent, z->Resume_way_match)
#define MemRead16_I(A, outval)	MemRead(-1,    uint16_t, ((A) >> 29), (z->CCR & SH7095_CCR_CE), (A), outval, /**/ z->Resume_uint16_A, z->Resume_unmasked_A, z->Resume_cent, z->Resume_way_match)
#define MemRead32_I(A, outval)	MemRead(-1,    uint32_t, ((A) >> 29), (z->CCR & SH7095_CCR_CE), (A), outval, /**/ z->Resume_uint32_A, z->Resume_unmasked_A, z->Resume_cent, z->Resume_way_match)

#define MemWrite8(A, V)  MemWrite(uint8_t,  ((A) >> 29), (z->CCR & SH7095_CCR_CE), (A), (V), /**/ z->Resume_uint8_A,  z->Resume_unmasked_A, z->Resume_uint8_V)
#define MemWrite16(A, V) MemWrite(uint16_t, ((A) >> 29), (z->CCR & SH7095_CCR_CE), (A), (V), /**/ z->Resume_uint16_A, z->Resume_unmasked_A, z->Resume_uint16_V)
#define MemWrite32(A, V) MemWrite(uint32_t, ((A) >> 29), (z->CCR & SH7095_CCR_CE), (A), (V), /**/ z->Resume_uint32_A, z->Resume_unmasked_A, z->Resume_uint32_V)

#define CHECK_EXIT_RESUME__(n)		\
	{					\
	 if(z->timestamp >= bound_timestamp)	\
	 {					\
	  z->resume_id = (n);			\
	  return;				\
	 }					\
	 Resume_ ## n:;				\
	}

#define CHECK_EXIT_RESUME_(n) CHECK_EXIT_RESUME__(n)
#define CHECK_EXIT_RESUME() CHECK_EXIT_RESUME_(__COUNTER__)

#define instr z->Resume_instr
#define instr_nyb1 ((instr >> 4) & 0xF)
#define instr_nyb2 ((instr >> 8) & 0xF)
#define opexec_MAC_L_m0 z->Resume_MAC_L_m0
#define opexec_MAC_L_m1 z->Resume_MAC_L_m1
#define opexec_MAC_W_m0 z->Resume_MAC_W_m0
#define opexec_MAC_W_m1 z->Resume_MAC_W_m1
#define opexec_ea z->Resume_ea
#define opexec_new_PC z->Resume_new_PC
#define opexec_new_SR z->Resume_new_SR
#define opexec_ipr z->Resume_ipr
#define opexec_exnum z->Resume_exnum
#define opexec_vecnum z->Resume_vecnum
#define CONST_VAR(T, n) T n; n
/* RESUME_VAR(T, n) -- compile-time check that the declared type T
 * matches the actual type of `n` (which is z->Resume_<something>
 * once the opexec_* macros have expanded).  Three language paths:
 *
 *   C++  -- std::is_same<T, decltype(n)>::value
 *           (decltype is 11, std::is_same lives in <type_traits>
 *           which mednafen-types.h already pulls in for TUs).
 *
 *   C (GCC / clang) -- __builtin_types_compatible_p(T, __typeof__(n))
 *           (both are GCC extensions, available since gcc 2.x, also
 *           accepted by clang -- no C11 / C23 dependency).
 *
 *   C (other, e.g. MSVC) -- sizeof(T) == sizeof(n)
 *           Partial fallback.  Catches size mismatches (int8 vs int32)
 *           but not signedness or unrelated-types-with-same-size
 *           mismatches.  Better than no check; preserves MSVC C89
 *           buildability without dropping the assertion entirely on
 *           GCC / clang where the real check is available.
 *
 * The condition expression is wrapped in an extra set of parens so
 * the comma inside `std::is_same<T, decltype(n)>` / inside
 * `__builtin_types_compatible_p(T, __typeof__(n))` is not seen by
 * the preprocessor as a MDFN_STATIC_ASSERT argument separator.
 * Either branch funnels through MDFN_STATIC_ASSERT, so the actual
 * static_assert / _Static_assert / negative-array fallback choice
 * is the macro's problem, not ours. */
#ifdef __cplusplus
 #define RESUME_VAR(T, n) MDFN_STATIC_ASSERT((std::is_same<T, decltype(n)>::value), "Resume variable type mismatch.")
#elif defined(__GNUC__) || defined(__clang__)
 #define RESUME_VAR(T, n) MDFN_STATIC_ASSERT((__builtin_types_compatible_p(T, __typeof__(n))), "Resume variable type mismatch.")
#else
 #define RESUME_VAR(T, n) MDFN_STATIC_ASSERT(sizeof(T) == sizeof(n), "Resume variable size mismatch (full type check unavailable on this compiler).")
#endif
#define SH7095_NEED_RESUME_TABLE_INIT 1

#define SH7095_DEBUG_MODE 0
#include "sh7095s_cnorm.inc"
MDFN_STATIC_ASSERT(__COUNTER__ == 5000, "Unexpected __COUNTER__ value.");
NO_CLONE NO_INLINE MDFN_HOT void SH7095_RunSlaveUntil(SH7095* z, const sscpu_timestamp_t bound_timestamp)
{
 #include "sh7095s_rsu.inc"
}
#undef SH7095_DEBUG_MODE

#define SH7095_DEBUG_MODE 1
#include "sh7095s_cnorm_dm.inc"
MDFN_STATIC_ASSERT(__COUNTER__ == 10000, "Unexpected __COUNTER__ value.");
static NO_CLONE NO_INLINE MDFN_COLD void SH7095_RunSlaveUntil_Debug(SH7095* z, const sscpu_timestamp_t bound_timestamp)
{
 #include "sh7095s_rsu.inc"
}
#undef SH7095_DEBUG_MODE

#if defined(__GNUC__) && !defined(__clang__)
 #pragma GCC pop_options
#endif
#undef DoIDIF
#undef OnChipRegRead
#undef ExtBusRead
#undef ExtBusWrite
#undef MemReadInstr
#undef MemRead8
#undef MemRead16
#undef MemRead32
#undef MemRead16_I
#undef MemRead32_I
#undef MemWrite8
#undef MemWrite16
#undef MemWrite32
#undef CHECK_EXIT_RESUME__
#undef CHECK_EXIT_RESUME_
#undef CHECK_EXIT_RESUME
#undef instr
#undef instr_nyb1
#undef instr_nyb2
#undef opexec_MAC_L_m0
#undef opexec_MAC_L_m1
#undef opexec_MAC_W_m0
#undef opexec_MAC_W_m1
#undef opexec_ea
#undef opexec_new_PC
#undef opexec_new_SR
#undef opexec_ipr
#undef opexec_exnum
#undef opexec_vecnum
#undef CONST_VAR
#undef RESUME_VAR
#undef SH7095_NEED_RESUME_TABLE_INIT
//
//
//
//
//
//
void SH7095_StateAction(SH7095* z, StateMem* sm, const unsigned load, const bool data_only, const char* sname)
{
 /* Field-name strings below intentionally omit the "z->" prefix:
  * they are persisted into savestate files, so they must match
  * the pre-phase-9 (member-function StateAction) spelling. */
 SFORMAT StateRegs[] =
 {
  SFPTR32N(&(z->R)[0], (sizeof(z->R) / sizeof(uint32_t)), "R"),
  SFVARN(z->PC, "PC"),
  SFPTR32N(&(z->CtrlRegs)[0], (sizeof(z->CtrlRegs) / sizeof(uint32_t)), "CtrlRegs"),

  SFVARN(z->timestamp,              "timestamp"),
  SFVARN(z->MA_until,               "MA_until"),
  SFVARN(z->MM_until,               "MM_until"),
  SFVARN(z->write_finish_timestamp, "write_finish_timestamp"),
  SFPTR32N(&(z->WB_until)[0], (sizeof(z->WB_until) / sizeof(sscpu_timestamp_t)), "WB_until"),
  SFVARN(z->UCRead_IF_Kludge, "UCRead_IF_Kludge"),
  SFPTR32N(&(z->SysRegs)[0], (sizeof(z->SysRegs) / sizeof(uint32_t)), "SysRegs"),

  SFVARN(z->EPending, "EPending"),
  SFVARN(z->Pipe_ID,  "Pipe_ID"),
  SFVARN(z->Pipe_IF,  "Pipe_IF"),

  SFVARN(z->IBuffer,  "IBuffer"),

  SFPTR32N(z->Cache->Tag, 4, 64, sizeof(*z->Cache), z->Cache, "Cache->Tag"),
  SFPTR8N(&(z->Cache_LRU)[0], (sizeof(z->Cache_LRU) / sizeof(uint8_t)), "Cache->LRU"),
  // Cast because it's stored as native-endian 32-bit.  SFPTR32N (explicit
  // name) rather than SFPTR32 (auto #x name): the typedef rename changed the
  // cast spelling from (uint32*) to (uint32_t*), and SFPTR32 would stringize
  // that into a different savestate field name.  Pass the original name
  // verbatim to keep savestates byte-compatible.
  SFPTR32N((uint32_t*)z->Cache->Data, 16, 64, sizeof(*z->Cache), z->Cache, "(uint32*)Cache->Data"),

  SFVARN(z->CCR, "CCR"),

  SFVARN(z->NMILevel, "NMILevel"),
  SFVARN(z->IRL,      "IRL"),

  SFVARN(z->IPRA,   "IPRA"),
  SFVARN(z->IPRB,   "IPRB"),
  SFVARN(z->VCRWDT, "VCRWDT"),
  SFVARN(z->VCRA,   "VCRA"),
  SFVARN(z->VCRB,   "VCRB"),
  SFVARN(z->VCRC,   "VCRC"),
  SFVARN(z->VCRD,   "VCRD"),
  SFVARN(z->ICR,    "ICR"),

 //
 //
 //
  SFVARN(z->BSC.BCR1,   "BSC.BCR1"),
  SFVARN(z->BSC.BCR2,   "BSC.BCR2"),
  SFVARN(z->BSC.WCR,    "BSC.WCR"),
  SFVARN(z->BSC.MCR,    "BSC.MCR"),
  SFVARN(z->BSC.RTCSR,  "BSC.RTCSR"),
  SFVARN(z->BSC.RTCSRM, "BSC.RTCSRM"),
  SFVARN(z->BSC.RTCNT,  "BSC.RTCNT"),
  SFVARN(z->BSC.RTCOR,  "BSC.RTCOR"),

  SFVARN(z->BSC.sdram_finish_time, "BSC.sdram_finish_time"),
  SFVARN(z->BSC.last_mem_time,     "BSC.last_mem_time"),
  SFVARN(z->BSC.last_mem_addr,     "BSC.last_mem_addr"),
  SFVARN(z->BSC.last_mem_type,     "BSC.last_mem_type"),

  SFVARN(z->SBYCR,   "SBYCR"),
  SFVARN(z->Standby, "Standby"),

  SFVARN(z->FRT.lastts, "FRT.lastts"),
  SFVARN(z->FRT.FTI,    "FRT.FTI"),
  SFVARN(z->FRT.FTCI,   "FRT.FTCI"),
  SFVARN(z->FRT.FRC,    "FRT.FRC"),
  SFPTR16N(&(z->FRT.OCR)[0], (sizeof(z->FRT.OCR) / sizeof(uint16_t)), "FRT.OCR"),
  SFVARN(z->FRT.FICR,    "FRT.FICR"),
  SFVARN(z->FRT.TIER,    "FRT.TIER"),
  SFVARN(z->FRT.FTCSR,   "FRT.FTCSR"),
  SFVARN(z->FRT.FTCSRM,  "FRT.FTCSRM"),
  SFVARN(z->FRT.TCR,     "FRT.TCR"),
  SFVARN(z->FRT.TOCR,    "FRT.TOCR"),
  SFVARN(z->FRT.RW_Temp, "FRT.RW_Temp"),

  SFVARN(z->FRT_WDT_ClockDivider, "FRT_WDT_ClockDivider"),

  SFVARN(z->FRT_WDT_NextTS, "FRT_WDT_NextTS"),

  SFVARN(z->WDT.WTCSR,   "WDT.WTCSR"),
  SFVARN(z->WDT.WTCSRM,  "WDT.WTCSRM"),
  SFVARN(z->WDT.WTCNT,   "WDT.WTCNT"),
  SFVARN(z->WDT.RSTCSR,  "WDT.RSTCSR"),
  SFVARN(z->WDT.RSTCSRM, "WDT.RSTCSRM"),

 // SFVAR(DMA_ClockCounter),
 // SFVAR(DMA_SGCounter),
  SFVARN(z->DMA_Timestamp,             "DMA_Timestamp"),
  SFVARN(z->DMA_SGEndTimestamp,        "DMA_SGEndTimestamp"),
  SFVARN(z->DMA_RoundRobinRockinBoppin,"DMA_RoundRobinRockinBoppin"),

  SFVARN(z->DMA_PenaltyKludgeAmount, "DMA_PenaltyKludgeAmount"),
  SFVARN(z->DMA_PenaltyKludgeAccum,  "DMA_PenaltyKludgeAccum"),

  SFVARN(z->DMACH->SAR,   2, sizeof(*z->DMACH), z->DMACH, "DMACH->SAR"),
  SFVARN(z->DMACH->DAR,   2, sizeof(*z->DMACH), z->DMACH, "DMACH->DAR"),
  SFVARN(z->DMACH->TCR,   2, sizeof(*z->DMACH), z->DMACH, "DMACH->TCR"),
  SFVARN(z->DMACH->CHCR,  2, sizeof(*z->DMACH), z->DMACH, "DMACH->CHCR"),
  SFVARN(z->DMACH->CHCRM, 2, sizeof(*z->DMACH), z->DMACH, "DMACH->CHCRM"),
  SFVARN(z->DMACH->VCR,   2, sizeof(*z->DMACH), z->DMACH, "DMACH->VCR"),
  SFVARN(z->DMACH->DRCR,  2, sizeof(*z->DMACH), z->DMACH, "DMACH->DRCR"),

  SFVARN(z->DMAOR,  "DMAOR"),
  SFVARN(z->DMAORM, "DMAORM"),

  SFVARN(z->divide_finish_timestamp, "divide_finish_timestamp"),
  SFVARN(z->DVSR,         "DVSR"),
  SFVARN(z->DVDNT,        "DVDNT"),
  SFVARN(z->DVDNTH,       "DVDNTH"),
  SFVARN(z->DVDNTL,       "DVDNTL"),
  SFVARN(z->DVDNTH_Shadow,"DVDNTH_Shadow"),
  SFVARN(z->DVDNTL_Shadow,"DVDNTL_Shadow"),
  SFVARN(z->VCRDIV,       "VCRDIV"),
  SFVARN(z->DVCR,         "DVCR"),

  SFVARN(z->SCI.SMR,  "SCI.SMR"),
  SFVARN(z->SCI.BRR,  "SCI.BRR"),
  SFVARN(z->SCI.SCR,  "SCI.SCR"),
  SFVARN(z->SCI.TDR,  "SCI.TDR"),
  SFVARN(z->SCI.SSR,  "SCI.SSR"),
  SFVARN(z->SCI.SSRM, "SCI.SSRM"),
  SFVARN(z->SCI.RDR,  "SCI.RDR"),

  SFVARN(z->SCI.RSR, "SCI.RSR"),
  SFVARN(z->SCI.TSR, "SCI.TSR"),

  SFVARN(z->ExtHalt,    "ExtHalt"),
  SFVARN(z->ExtHaltDMA, "ExtHaltDMA"),

  SFVARN(z->PC_IF, "PC_IF"),
  SFVARN(z->PC_ID, "PC_ID"),

  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, sname, false);

 if(z == &CPU[1])
  SH7095_StateAction_SlaveResume(z, sm, load, data_only, "SH2-S-Resume");
}

static void SH7095_StateAction_SlaveResume(SH7095* z, StateMem* sm, const unsigned load, const bool data_only, const char* sname)
{
 int32_t ResumePointI = -1;
 uint32_t Resume_cent_I = 0;

 /* Savestate format compatibility: the persisted field is still
  * `ResumePointI` (int32_t), holding the 0..511 index that the
  * pre-conversion code used as a lookup into the now-deleted
  * `ResumeTable[512]` array.  The table mapped index N to label
  * `&&Resume_(top - N)` where `top` is 5512 (non-debug) or 10512
  * (debug).  Now that the resume mechanism stores an integer id
  * directly (5001..5512 or 10001..10512), translate to/from the
  * legacy index form on save/load:
  *
  *    ResumePointI = top - resume_id     (save)
  *    resume_id    = top - ResumePointI  (load)
  *
  * with -1 / 0 standing in for "no resume pending".  Old saves
  * load to the same instruction-handler resume point as before. */
 const uint16_t top = z->DM_Setting ? 10512 : 5512;

 if(!load)
 {
  if(z->resume_id)
   ResumePointI = (int32_t)top - (int32_t)z->resume_id;

  if(z->Resume_cent)
  {
   Resume_cent_I = z->Resume_cent - z->Cache;
   assert(Resume_cent_I < 0x40);
  }
 }

 /* As in SH7095_StateAction, the persisted field-name strings
  * must drop the "z->" prefix to remain savestate-compatible
  * with the pre-phase-9 (member-function) spelling.  Pure
  * locals (ResumePointI, Resume_cent_I) stringize cleanly. */
 SFORMAT StateRegs[] =
 {
  SFVAR(ResumePointI),
  SFVARN(z->Resume_uint8_A,    "Resume_uint8_A"),
  SFVARN(z->Resume_uint16_A,   "Resume_uint16_A"),
  SFVARN(z->Resume_uint32_A,   "Resume_uint32_A"),
  SFVARN(z->Resume_unmasked_A, "Resume_unmasked_A"),
  SFVAR(Resume_cent_I),
  SFVARN(z->Resume_way_match,  "Resume_way_match"),
  SFVARN(z->Resume_uint8_V,    "Resume_uint8_V"),
  SFVARN(z->Resume_uint16_V,   "Resume_uint16_V"),
  SFVARN(z->Resume_uint32_V,   "Resume_uint32_V"),
  SFVARN(z->Resume_instr,      "Resume_instr"),

  SFVARN(z->Resume_MAC_L_m0,   "Resume_MAC_L_m0"),
  SFVARN(z->Resume_MAC_L_m1,   "Resume_MAC_L_m1"),

  SFVARN(z->Resume_MAC_W_m0,   "Resume_MAC_W_m0"),
  SFVARN(z->Resume_MAC_W_m1,   "Resume_MAC_W_m1"),

  SFVARN(z->Resume_ea,         "Resume_ea"),
  SFVARN(z->Resume_new_PC,     "Resume_new_PC"),
  SFVARN(z->Resume_new_SR,     "Resume_new_SR"),

  SFVARN(z->Resume_ipr,        "Resume_ipr"),
  SFVARN(z->Resume_exnum,      "Resume_exnum"),
  SFVARN(z->Resume_vecnum,     "Resume_vecnum"),

  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, sname, load < 0x00102600);

 if(load)
 {
  z->resume_id = (ResumePointI >= 0)
                 ? (uint16_t)((int32_t)top - (ResumePointI & 511))
                 : 0;
  z->Resume_uint16_A &= ~1;
  z->Resume_uint32_A &= ~3;
  z->Resume_cent = &z->Cache[Resume_cent_I & 0x3F];
  z->Resume_way_match = ((int)(-1) > (int)(((int)(3) < (int)(z->Resume_way_match) ? (int)(3) : (int)(z->Resume_way_match))) ? (int)(-1) : (int)(((int)(3) < (int)(z->Resume_way_match) ? (int)(3) : (int)(z->Resume_way_match))));
  z->Resume_instr &= 0xFFFF;
 }
}

//
// Only call after all emulated devices variables and RAM have been loaded from the save state.
//
void SH7095_PostStateLoad(SH7095* z, const unsigned state_version, const bool recorded_needicache, const bool needicache)
{
 //
 // Fixup cache tags.
 //
 for(unsigned entry = 0; entry < 64; entry++)
 {
  SH7095_CacheEntry* const ce = &z->Cache[entry];

  for(unsigned way = 0; way < 4; way++)
  {
   const uint32_t ATM = ce->Tag[way] & (0x7FFFF << 10);
   const bool invalid = ce->Tag[way] & ~(0x7FFFF << 10);	// Handles backwards compat when invalid was bit31 instead of bit0.

   ce->Tag[way] = ATM | invalid;
  }
 }
 //
 SH7095_SetCCR(z, z->CCR);
 //
 if(!recorded_needicache && state_version < 0x00102600)
 {
  for(unsigned i = 0; i < 16; i++)
   z->WB_until[i] = z->timestamp - 100;
 }

 if(needicache && !recorded_needicache)
 {
  //
  // Handle loading a state where instruction cache emulation was not previously enabled in the emulator, but is enabled now(e.g. via database).
  //
  // May not cover all cases, needs more testing.
  //
  //
  //const uint32_t old_IBuffer = z->IBuffer;
  const uint32_t A = (z->PC - 2) & ~3;

  if((int32_t)A < 0)
   memcpy(&z->IBuffer, &z->Cache[(A >> 4) & 0x3F].Data[(A >> 10) & 0x3][NE32ASU8_IDX_ADJ(uint32_t, A & 0x0F)], sizeof(uint32_t));
  else
   z->IBuffer = SH7095_RBO_BE_T(uint32_t, SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS], A);

 }
 //
 //
 //
 if(state_version < 0x00102300)
 {
  z->ExtHaltDMA = z->ExtHalt;
 }

 if(state_version < 0x00102600 && z->timestamp >= 0x40000000)
 {
  z->timestamp = SS_EVENT_DISABLED_TS;
 }

 if(z->timestamp != SS_EVENT_DISABLED_TS)
 {
  if(state_version < 0x00102600)
  {
   z->DMA_Timestamp = SH7095_mem_timestamp - 128;
   z->DMA_SGEndTimestamp = SH7095_mem_timestamp + 192;

   z->BSC.sdram_finish_time = SH7095_mem_timestamp - 100;
   z->BSC.last_mem_time = SH7095_mem_timestamp - 100;
  }

  /* clamp() folded: each was clamp(&x, -1000000, 1000000) on an
   * int32_t (sscpu_timestamp_t). Folded as a single ternary-assignment
   * statement -- no if-statements -- so the unbraced for-loop body
   * below cannot dangle. */
  z->timestamp = (z->timestamp < -1000000) ? -1000000 : (z->timestamp > 1000000) ? 1000000 : z->timestamp;
  z->FRT.lastts = (z->FRT.lastts < -1000000) ? -1000000 : (z->FRT.lastts > 1000000) ? 1000000 : z->FRT.lastts;
  z->DMA_Timestamp = (z->DMA_Timestamp < -1000000) ? -1000000 : (z->DMA_Timestamp > 1000000) ? 1000000 : z->DMA_Timestamp;
  z->DMA_SGEndTimestamp = (z->DMA_SGEndTimestamp < -1000000) ? -1000000 : (z->DMA_SGEndTimestamp > 1000000) ? 1000000 : z->DMA_SGEndTimestamp;

  z->MA_until = (z->MA_until < -1000000) ? -1000000 : (z->MA_until > 1000000) ? 1000000 : z->MA_until;
  z->MM_until = (z->MM_until < -1000000) ? -1000000 : (z->MM_until > 1000000) ? 1000000 : z->MM_until;

  for(unsigned i = 0; i < 16; i++)
   z->WB_until[i] = (z->WB_until[i] < -1000000) ? -1000000 : (z->WB_until[i] > 1000000) ? 1000000 : z->WB_until[i];

  z->write_finish_timestamp = (z->write_finish_timestamp < -1000000) ? -1000000 : (z->write_finish_timestamp > 1000000) ? 1000000 : z->write_finish_timestamp;
  z->divide_finish_timestamp = (z->divide_finish_timestamp < -1000000) ? -1000000 : (z->divide_finish_timestamp > 1000000) ? 1000000 : z->divide_finish_timestamp;

  z->BSC.sdram_finish_time = (z->BSC.sdram_finish_time < -1000000) ? -1000000 : (z->BSC.sdram_finish_time > 1000000) ? 1000000 : z->BSC.sdram_finish_time;
  z->BSC.last_mem_time = (z->BSC.last_mem_time < -1000000) ? -1000000 : (z->BSC.last_mem_time > 1000000) ? 1000000 : z->BSC.last_mem_time;
 }
 //
 //
 //
 z->UCRead_IF_Kludge &= 1;
}

//
//
//
//
//
//
//
//

static INLINE uint32_t SH7095_GetRegister(SH7095* z, const unsigned id, char* const special, const uint32_t special_len)
{
 uint32_t ret = 0xDEADBEEF;

 switch(id)
 {
  case SH7095_GSREG_PC_ID:
	ret = z->PC_ID;
	break;

  case SH7095_GSREG_PC_IF:
	ret = z->PC_IF;
	break;

  case SH7095_GSREG_PID:
	ret = z->Pipe_ID;
	break;

  case SH7095_GSREG_PIF:
	ret = z->Pipe_IF;
	break;

  case SH7095_GSREG_EP:
	ret = z->EPending;
	break;

  case SH7095_GSREG_RPC:
	ret = z->PC;
	break;

  case SH7095_GSREG_R0: case SH7095_GSREG_R1: case SH7095_GSREG_R2: case SH7095_GSREG_R3: case SH7095_GSREG_R4: case SH7095_GSREG_R5: case SH7095_GSREG_R6: case SH7095_GSREG_R7:
  case SH7095_GSREG_R8: case SH7095_GSREG_R9: case SH7095_GSREG_R10: case SH7095_GSREG_R11: case SH7095_GSREG_R12: case SH7095_GSREG_R13: case SH7095_GSREG_R14: case SH7095_GSREG_R15:
	ret = z->R[id - SH7095_GSREG_R0];
	break;

  case SH7095_GSREG_SR:
	ret = z->SR;
	break;

  case SH7095_GSREG_GBR:
	ret = z->GBR;
	break;

  case SH7095_GSREG_VBR:
	ret = z->VBR;
	break;

  case SH7095_GSREG_MACH:
	ret = z->MACH;
	break;

  case SH7095_GSREG_MACL:
	ret = z->MACL;
	break;

  case SH7095_GSREG_PR:
	ret = z->PR;
	break;

  //
  //
  //
  case SH7095_GSREG_NMIL:
	ret = z->NMILevel;
	break;

  case SH7095_GSREG_IRL:
	ret = z->IRL;
	break;

  case SH7095_GSREG_IPRA:
	ret = z->IPRA;
	break;

  case SH7095_GSREG_IPRB:
	ret = z->IPRB;
	break;

  case SH7095_GSREG_VCRWDT:
	ret = z->VCRWDT;
	break;

  case SH7095_GSREG_VCRA:
	ret = z->VCRA;
	break;

  case SH7095_GSREG_VCRB:
	ret = z->VCRB;
	break;

  case SH7095_GSREG_VCRC:
	ret = z->VCRC;
	break;

  case SH7095_GSREG_VCRD:
	ret = z->VCRD;
	break;

  case SH7095_GSREG_ICR:
	ret = z->ICR;
	break;
  //
  //
  //
  case SH7095_GSREG_DVSR:
	ret = z->DVSR;
	break;

  case SH7095_GSREG_DVDNT:
	ret = z->DVDNT;
	break;

  case SH7095_GSREG_DVDNTH:
	ret = z->DVDNTH;
	break;

  case SH7095_GSREG_DVDNTL:
	ret = z->DVDNTL;
	break;

  case SH7095_GSREG_DVDNTHS:
	ret = z->DVDNTH_Shadow;
	break;

  case SH7095_GSREG_DVDNTLS:
	ret = z->DVDNTL_Shadow;
	break;

  case SH7095_GSREG_VCRDIV:
	ret = z->VCRDIV;
	break;

  case SH7095_GSREG_DVCR:
	ret = z->DVCR;
	break;
  //
  //
  //
  case SH7095_GSREG_WTCSR:
	ret = z->WDT.WTCSR;
	break;

  case SH7095_GSREG_WTCSRM:
	ret = z->WDT.WTCSRM;
	break;

  case SH7095_GSREG_WTCNT:
	ret = z->WDT.WTCNT;
	break;

  case SH7095_GSREG_RSTCSR:
	ret = z->WDT.RSTCSR;
	break;

  case SH7095_GSREG_RSTCSRM:
	ret = z->WDT.RSTCSRM;
	break;
  //
  //
  //
  case SH7095_GSREG_DMAOR:
	ret = z->DMAOR;
	break;

  case SH7095_GSREG_DMAORM:
	ret = z->DMAORM;
	break;

  case SH7095_GSREG_DMA0_SAR:
  case SH7095_GSREG_DMA1_SAR:
	ret = z->DMACH[id == SH7095_GSREG_DMA1_SAR].SAR;
	break;

  case SH7095_GSREG_DMA0_DAR:
  case SH7095_GSREG_DMA1_DAR:
	ret = z->DMACH[id == SH7095_GSREG_DMA1_DAR].DAR;
	break;

  case SH7095_GSREG_DMA0_TCR:
  case SH7095_GSREG_DMA1_TCR:
	ret = z->DMACH[id == SH7095_GSREG_DMA1_TCR].TCR;
	break;

  case SH7095_GSREG_DMA0_CHCR:
  case SH7095_GSREG_DMA1_CHCR:
	ret = z->DMACH[id == SH7095_GSREG_DMA1_CHCR].CHCR;
	break;

  case SH7095_GSREG_DMA0_CHCRM:
  case SH7095_GSREG_DMA1_CHCRM:
	ret = z->DMACH[id == SH7095_GSREG_DMA1_CHCRM].CHCRM;
	break;

  case SH7095_GSREG_DMA0_VCR:
  case SH7095_GSREG_DMA1_VCR:
	ret = z->DMACH[id == SH7095_GSREG_DMA1_VCR].VCR;
	break;

  case SH7095_GSREG_DMA0_DRCR:
  case SH7095_GSREG_DMA1_DRCR:
	ret = z->DMACH[id == SH7095_GSREG_DMA1_DRCR].DRCR;
	break;
  //
  //
  //
  case SH7095_GSREG_FRC:
	ret = z->FRT.FRC;
	break;

  case SH7095_GSREG_OCR0:
	ret = z->FRT.OCR[0];
	break;

  case SH7095_GSREG_OCR1:
	ret = z->FRT.OCR[1];
	break;

  case SH7095_GSREG_FICR:
	ret = z->FRT.FICR;
	break;

  case SH7095_GSREG_TIER:
	ret = z->FRT.TIER;
	break;

  case SH7095_GSREG_FTCSR:
	ret = z->FRT.FTCSR;
	break;

  case SH7095_GSREG_FTCSRM:
	ret = z->FRT.FTCSRM;
	break;

  case SH7095_GSREG_TCR:
	ret = z->FRT.TCR;
	break;

  case SH7095_GSREG_TOCR:
	ret = z->FRT.TOCR;
	break;

  case SH7095_GSREG_RWT:
	ret = z->FRT.RW_Temp;
	break;
  //
  //
  //
  case SH7095_GSREG_CCR:
	ret = z->CCR;
	break;

  case SH7095_GSREG_SBYCR:
	ret = z->SBYCR;
	break;
 }

 return ret;
}

static void SH7095_SetRegister(SH7095* z, const unsigned id, const uint32_t value)
{
 switch(id)
 {
  //case GSREG_PC: break;

  case SH7095_GSREG_PID:
	z->Pipe_ID = value;
	break;

  case SH7095_GSREG_PIF:
	z->Pipe_IF = value;
	break;

  //case SH7095_GSREG_EP:
  //	z->EPending = value;
  //	break;

  case SH7095_GSREG_RPC:
	z->PC = value;
	break;

  case SH7095_GSREG_R0: case SH7095_GSREG_R1: case SH7095_GSREG_R2: case SH7095_GSREG_R3: case SH7095_GSREG_R4: case SH7095_GSREG_R5: case SH7095_GSREG_R6: case SH7095_GSREG_R7:
  case SH7095_GSREG_R8: case SH7095_GSREG_R9: case SH7095_GSREG_R10: case SH7095_GSREG_R11: case SH7095_GSREG_R12: case SH7095_GSREG_R13: case SH7095_GSREG_R14: case SH7095_GSREG_R15:
	z->R[id - SH7095_GSREG_R0] = value;
	break;

  case SH7095_GSREG_SR:
	z->SR = value & 0x3F3;
	SH7095_RecalcPendingIntPEX(z);
	break;

  case SH7095_GSREG_GBR:
	z->GBR = value;
	break;

  case SH7095_GSREG_VBR:
	z->VBR = value;
	break;

  case SH7095_GSREG_MACH:
	z->MACH = value;
	break;

  case SH7095_GSREG_MACL:
	z->MACL = value;
	break;

  case SH7095_GSREG_PR:
	z->PR = value;
	break;

  //
  //
  //
  case SH7095_GSREG_FTCSR:	
	z->FRT.FTCSR = value & 0x8F;
	SH7095_RecalcPendingIntPEX(z);
	SH7095_FRT_CheckOCR(z);
	break;

  case SH7095_GSREG_FTCSRM:	
	z->FRT.FTCSRM = value & 0x8F;
	break;
/*
  case SH7095_GSREG_TCR:
	z->FRT.TCR = value;
	SH7095_FRT_WDT_Recalc_NET(z);
	break;
*/
  //
  //
  //
  case SH7095_GSREG_CCR:
	SH7095_SetCCR(z, value);
	break;
 }
}

//
//
//
//
static INLINE void SH7095_CheckRWBreakpoints(const SH7095* z, void (*MRead)(unsigned len, uint32_t addr), void (*MWrite)(unsigned len, uint32_t addr))
{
 uint32_t lpid = z->Pipe_ID;
 //
 //
 //
 //SPEPRecover:;
 const uint32_t instr = (uint16_t)lpid;
 const unsigned instr_nyb1 = (instr >> 4) & 0xF;
 const unsigned instr_nyb2 = (instr >> 8) & 0xF;

 switch(lpid >> 24)
 {
 #include "sh7095_opdefs.inc"
 #define BEGIN_BP_OP(x) OP_##x {
 #define END_BP_OP } break;

 //
 // MOV.W @(disp,z->PC),Rn
 //
 BEGIN_BP_OP(MOV_W_PCREL_REG)
	const unsigned d = (instr >> 0) & 0xff;
	const uint32_t ea = z->PC + (d << 1);

	MRead(2, ea);
 END_BP_OP

 //
 // MOV.L @(disp,z->PC),Rn
 //
 BEGIN_BP_OP(MOV_L_PCREL_REG)
	const unsigned d = (instr >> 0) & 0xff;
	const uint32_t ea = (z->PC &~ 0x3) + (d << 2);

	MRead(4, ea);
 END_BP_OP

 //
 // MOV.B Rm,@Rn
 //
 BEGIN_BP_OP(MOV_B_REG_REGINDIR)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[n];

	MWrite(1, ea);
 END_BP_OP

 //
 // MOV.W Rm,@Rn
 //
 BEGIN_BP_OP(MOV_W_REG_REGINDIR)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[n];

	MWrite(2, ea);
 END_BP_OP

 //
 // MOV.L Rm,@Rn
 //
 BEGIN_BP_OP(MOV_L_REG_REGINDIR)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[n];

	MWrite(4, ea);
 END_BP_OP

 //
 // MOV.B @Rm,Rn
 //
 BEGIN_BP_OP(MOV_B_REGINDIR_REG)
	const unsigned m = instr_nyb1;
	const uint32_t ea = z->R[m];

	MRead(1, ea);
 END_BP_OP

 //
 // MOV.W @Rm,Rn
 //
 BEGIN_BP_OP(MOV_W_REGINDIR_REG)
	const unsigned m = instr_nyb1;
	const uint32_t ea = z->R[m];

	MRead(2, ea);
 END_BP_OP

 //
 // MOV.L @Rm,Rn
 //
 BEGIN_BP_OP(MOV_L_REGINDIR_REG)
	const unsigned m = instr_nyb1;
	const uint32_t ea = z->R[m];

	MRead(4, ea);
 END_BP_OP

 //
 // MOV.B Rm,@-Rn
 //
 BEGIN_BP_OP(MOV_B_REG_REGINDIRPD)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[n] - 1;

	MWrite(1, ea);
 END_BP_OP

 //
 // MOV.W Rm,@-Rn
 //
 BEGIN_BP_OP(MOV_W_REG_REGINDIRPD)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[n] - 2;

	MWrite(2, ea);
 END_BP_OP

 //
 // MOV.L Rm,@-Rn
 //
 BEGIN_BP_OP(MOV_L_REG_REGINDIRPD)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[n] - 4;

	MWrite(4, ea);
 END_BP_OP

 //
 // MOV.B @Rm+,Rn
 //
 BEGIN_BP_OP(MOV_B_REGINDIRPI_REG)
	const unsigned m = instr_nyb1;
	const uint32_t ea = z->R[m];

	MRead(1, ea);
 END_BP_OP

 //
 // MOV.W @Rm+,Rn
 //
 BEGIN_BP_OP(MOV_W_REGINDIRPI_REG)
	const unsigned m = instr_nyb1;
	const uint32_t ea = z->R[m];

	MRead(2, ea);
 END_BP_OP

 //
 // MOV.L @Rm+,Rn
 //
 BEGIN_BP_OP(MOV_L_REGINDIRPI_REG)
	const unsigned m = instr_nyb1;
	const uint32_t ea = z->R[m];

	MRead(4, ea);
 END_BP_OP

 //
 // MOV.B R0,@(disp,Rn)
 //
 BEGIN_BP_OP(MOV_B_REG0_REGINDIRDISP)
	const unsigned n = instr_nyb1;
	const unsigned d = (instr >> 0) & 0xf;
	const uint32_t ea = z->R[n] + (d << 0);

	MWrite(1, ea);
 END_BP_OP

 //
 // MOV.W R0,@(disp,Rn)
 //
 BEGIN_BP_OP(MOV_W_REG0_REGINDIRDISP)
	const unsigned n = instr_nyb1;
	const unsigned d = (instr >> 0) & 0xf;
	const uint32_t ea = z->R[n] + (d << 1);

	MWrite(2, ea);
 END_BP_OP

 //
 // MOV.L Rm,@(disp,Rn)
 //
 BEGIN_BP_OP(MOV_L_REG_REGINDIRDISP)
	const unsigned n = instr_nyb2;
	const unsigned d = (instr >> 0) & 0xf;
	const uint32_t ea = z->R[n] + (d << 2);

	MWrite(4, ea);
 END_BP_OP

 //
 // MOV.B @(disp,Rm),R0
 //
 BEGIN_BP_OP(MOV_B_REGINDIRDISP_REG0)
	const unsigned m = instr_nyb1;
	const unsigned d = (instr >> 0) & 0xf;
	const uint32_t ea = z->R[m] + (d << 0);

	MRead(1, ea);
 END_BP_OP

 //
 // MOV.W @(disp,Rm),R0
 //
 BEGIN_BP_OP(MOV_W_REGINDIRDISP_REG0)
	const unsigned m = instr_nyb1;
	const unsigned d = (instr >> 0) & 0xf;
	const uint32_t ea = z->R[m] + (d << 1);

	MRead(2, ea);
 END_BP_OP

 //
 // MOV.L @(disp,Rm),Rn
 //
 BEGIN_BP_OP(MOV_L_REGINDIRDISP_REG)
	const unsigned m = instr_nyb1;
	const unsigned d = (instr >> 0) & 0xf;
	const uint32_t ea = z->R[m] + (d << 2);

	MRead(4, ea);
 END_BP_OP

 //
 // MOV.B Rm,@(R0,Rn)
 //
 BEGIN_BP_OP(MOV_B_REG_IDXREGINDIR)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[0] + z->R[n];

	MWrite(1, ea);
 END_BP_OP

 //
 // MOV.W Rm,@(R0,Rn)
 //
 BEGIN_BP_OP(MOV_W_REG_IDXREGINDIR)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[0] + z->R[n];

	MWrite(2, ea);
 END_BP_OP

 //
 // MOV.L Rm,@(R0,Rn)
 //
 BEGIN_BP_OP(MOV_L_REG_IDXREGINDIR)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[0] + z->R[n];

	MWrite(4, ea);
 END_BP_OP

 //
 // MOV.B @(R0,Rm),Rn
 //
 BEGIN_BP_OP(MOV_B_IDXREGINDIR_REG)
	const unsigned m = instr_nyb1;
	const uint32_t ea = z->R[0] + z->R[m];

	MRead(1, ea);
 END_BP_OP

 //
 // MOV.W @(R0,Rm),Rn
 //
 BEGIN_BP_OP(MOV_W_IDXREGINDIR_REG)
	const unsigned m = instr_nyb1;
	const uint32_t ea = z->R[0] + z->R[m];

	MRead(2, ea);
 END_BP_OP

 //
 // MOV.L @(R0,Rm),Rn
 //
 BEGIN_BP_OP(MOV_L_IDXREGINDIR_REG)
	const unsigned m = instr_nyb1;
	const uint32_t ea = z->R[0] + z->R[m];

	MRead(4, ea);
 END_BP_OP

 //
 // MOV.B R0,@(disp,z->GBR)
 //
 BEGIN_BP_OP(MOV_B_REG0_GBRINDIRDISP)
	const unsigned d = (instr >> 0) & 0xff;
	const uint32_t ea = z->GBR + (d << 0);

	MWrite(1, ea);
 END_BP_OP

 //
 // MOV.W R0,@(disp,z->GBR)
 //
 BEGIN_BP_OP(MOV_W_REG0_GBRINDIRDISP)
	const unsigned d = (instr >> 0) & 0xff;
	const uint32_t ea = z->GBR + (d << 1);

	MWrite(2, ea);
 END_BP_OP

 //
 // MOV.L R0,@(disp,z->GBR)
 //
 BEGIN_BP_OP(MOV_L_REG0_GBRINDIRDISP)
	const unsigned d = (instr >> 0) & 0xff;
	const uint32_t ea = z->GBR + (d << 2);

	MWrite(4, ea);
 END_BP_OP

 //
 // MOV.B @(disp,z->GBR),R0
 //
 BEGIN_BP_OP(MOV_B_GBRINDIRDISP_REG0)
	const unsigned d = (instr >> 0) & 0xff;
	const uint32_t ea = z->GBR + (d << 0);

	MRead(1, ea);
 END_BP_OP

 //
 // MOV.W @(disp,z->GBR),R0
 //
 BEGIN_BP_OP(MOV_W_GBRINDIRDISP_REG0)
	const unsigned d = (instr >> 0) & 0xff;
	const uint32_t ea = z->GBR + (d << 1);

	MRead(2, ea);
 END_BP_OP

 //
 // MOV.L @(disp,z->GBR),R0
 //
 BEGIN_BP_OP(MOV_L_GBRINDIRDISP_REG0)
	const unsigned d = (instr >> 0) & 0xff;
	const uint32_t ea = z->GBR + (d << 2);

	MRead(4, ea);
 END_BP_OP

 //
 // MAC.L @Rm+,@Rn+
 //
 BEGIN_BP_OP(MAC_L)
	const unsigned n = instr_nyb2;
	const unsigned m = instr_nyb1;

	MRead(4, z->R[m]);
        MRead(4, z->R[n] + ((m == n) << 2));
 END_BP_OP

 //
 // MAC.W @Rm+,@Rn+
 //
 BEGIN_BP_OP(MAC_W)
	const unsigned n = instr_nyb2;
	const unsigned m = instr_nyb1;

	MRead(2, z->R[m]);
        MRead(2, z->R[n] + ((m == n) << 1));
 END_BP_OP

 //
 // AND.B #imm,@(R0,z->GBR)
 //
 BEGIN_BP_OP(AND_B_IMM_IDXGBRINDIR)
	const uint32_t ea = z->R[0] + z->GBR;

	MRead(1, ea);
	MWrite(1, ea);
 END_BP_OP

 //
 // OR.B #imm,@(R0,z->GBR)
 //
 BEGIN_BP_OP(OR_B_IMM_IDXGBRINDIR)
	const uint32_t ea = z->R[0] + z->GBR;

	MRead(1, ea);
	MWrite(1, ea);
 END_BP_OP

 //
 // TAS.B @Rn
 //
 BEGIN_BP_OP(TAS_B_REGINDIR)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[n];

	MRead(1, ea);
	MWrite(1, ea);
 END_BP_OP

 //
 // TST.B #imm,@(R0,z->GBR)
 //
 BEGIN_BP_OP(TST_B_IMM_IDXGBRINDIR)
	const uint32_t ea = z->R[0] + z->GBR;

	MRead(1, ea);
 END_BP_OP

 //
 // XOR.B #imm,@(R0,z->GBR)
 //
 BEGIN_BP_OP(XOR_B_IMM_IDXGBRINDIR)
	const uint32_t ea = z->R[0] + z->GBR;

	MRead(1, ea);
	MWrite(1, ea);
 END_BP_OP

 //
 // LDC.L
 //
 BEGIN_BP_OP(LDC_L)
	const unsigned m = instr_nyb2;
	const uint32_t ea = z->R[m];

	MRead(4, ea);
 END_BP_OP

 //
 // LDS.L
 //
 BEGIN_BP_OP(LDS_L)
	const unsigned m = instr_nyb2;
	const uint32_t ea = z->R[m];

	MRead(4, ea);
 END_BP_OP

 //
 // RTE
 //
 BEGIN_BP_OP(RTE)
	MRead(4, z->R[15]);
	MRead(4, 4 + z->R[15]);
 END_BP_OP

 //
 // STC.L
 //
 BEGIN_BP_OP(STC_L)
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[n] - 4;

	MWrite(4, ea);
 END_BP_OP

 //
 // STS.L
 //
 BEGIN_BP_OP(STS_L)	// Pipeline same as ordinary store instruction
	const unsigned n = instr_nyb2;
	const uint32_t ea = z->R[n] - 4;

	MWrite(4, ea);
 END_BP_OP

 #undef BEGIN_BP_OP
 #undef END_BP_OP
 }
}