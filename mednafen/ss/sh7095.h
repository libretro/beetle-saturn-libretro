/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sh7095.h:
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

#ifndef __MDFN_SH7095_H
#define __MDFN_SH7095_H

#include <mednafen/state.h>

class SH7095 final
{
 public:

 SH7095(const char* const name_arg, const unsigned dma_event_id_arg, uint8_t (*exivecfn_arg)(void)) MDFN_COLD;
 ~SH7095() MDFN_COLD;

 void Init(const bool EmulateICache, const bool CacheBypassHack) MDFN_COLD;

 void StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname) MDFN_COLD;
 void StateAction_SlaveResume(StateMem* sm, const unsigned load, const bool data_only, const char* sname) MDFN_COLD;
 void PostStateLoad(const unsigned state_version, const bool recorded_needicache, const bool needicache) MDFN_COLD;

 void ForceInternalEventUpdates(void);
 void AdjustTS(int32_t delta);
 void SetActive(bool active);

 void TruePowerOn(void) MDFN_COLD;
 void Reset(bool power_on_reset, bool from_internal_wdt = false) MDFN_COLD;
 void SetNMI(bool level);
 void SetIRL(unsigned level);
 void SetMD5(bool level);

 void SetFTI(bool state);
 void SetFTCI(bool state);

 INLINE void SetExtHalt(bool state)
 {
  ExtHalt = state;

  if(ExtHalt)
   SetPEX(PEX_PSEUDO_EXTHALT);	// Only SetPEX() here, ClearPEX() is called in the pseudo exception handling code as necessary.

  ExtHaltDMA = (ExtHaltDMA & ~1) | state;
 }

 INLINE void SetExtHaltDMAKludgeFromVDP2(bool state)
 {
  ExtHaltDMA = (ExtHaltDMA & ~2) | (state << 1);
 }

 // When entering Step(), EmulateICache must match what was passed to Init()
 template<unsigned which, bool EmulateICache>
 void Step(void);

 // Slave only
 NO_CLONE NO_INLINE void RunSlaveUntil(sscpu_timestamp_t bound_timestamp) MDFN_HOT;
 NO_CLONE NO_INLINE void RunSlaveUntil_Debug(sscpu_timestamp_t bound_timestamp) MDFN_COLD;

 //private:
 uint32_t R[16];
 uint32_t PC;

 // Control registers
 union
 {
  struct
  {
   uint32_t SR;
   uint32_t GBR;
   uint32_t VBR;
  };
  uint32_t CtrlRegs[3];
 };

 sscpu_timestamp_t timestamp;
 sscpu_timestamp_t MM_until;
 sscpu_timestamp_t MA_until;
 sscpu_timestamp_t write_finish_timestamp;

 INLINE void SetT(bool new_value) { SR &= ~1; SR |= new_value; }
 INLINE bool GetT(void) { return SR & 1; }

 INLINE bool GetS(void) { return (bool)(SR & 0x002); }
 INLINE bool GetQ(void) { return (bool)(SR & 0x100); }
 INLINE bool GetM(void) { return (bool)(SR & 0x200); }
 INLINE void SetQ(bool new_q) { SR = (SR &~ 0x100) | (new_q << 8); }
 INLINE void SetM(bool new_m) { SR = (SR &~ 0x200) | (new_m << 9); }

 // System registers
 union
 {
  struct
  {
   uint32_t MACH;
   uint32_t MACL;
   uint32_t PR;
  };
  uint32_t SysRegs[3];
 };

 INLINE uint64_t GetMAC64(void) { return MACL | ((uint64_t)MACH << 32); }
 INLINE void SetMAC64(uint64_t nv) { MACL = nv; MACH = nv >> 32; }

 enum // must be in range of 0 ... 7
 {
  PEX_POWERON = 0,
  PEX_RESET   = 1,
  PEX_CPUADDR = 2,
  PEX_DMAADDR = 3,
  PEX_INT     = 4,
  PEX_NMI     = 5,
  PEX_PSEUDO_DMABURST = 6,
  PEX_PSEUDO_EXTHALT = 7
 };
 enum { EPENDING_PEXBITS_SHIFT = 16 };
 enum { EPENDING_OP_OR = 0xFF000000 };

 uint32_t EPending;

 INLINE void SetPEX(const unsigned which)
 {
  EPending |= (1U << (which + EPENDING_PEXBITS_SHIFT));
  EPending |= EPENDING_OP_OR;
 }

 INLINE void ClearPEX(const unsigned which)
 {
  EPending &= ~(1U << (which + EPENDING_PEXBITS_SHIFT));

  if(!(EPending & (0xFF << EPENDING_PEXBITS_SHIFT)))
   EPending = 0;
 }

 uint32_t Pipe_ID;
 uint32_t Pipe_IF;

 enum
 {
  EXCEPTION_POWERON = 0,// Power-on
  EXCEPTION_RESET,	// "Manual" reset
  EXCEPTION_ILLINSTR,	// General illegal instruction
  EXCEPTION_ILLSLOT,	// Slot illegal instruction
  EXCEPTION_CPUADDR,	// CPU address error
  EXCEPTION_DMAADDR,	// DMA Address error
  EXCEPTION_NMI,	// NMI
  EXCEPTION_BREAK,	// User break
  EXCEPTION_TRAP,	// Trap instruction
  EXCEPTION_INT,	// Interrupt
 };

 enum
 {
  VECNUM_POWERON   =  0,	// Power-on
  VECNUM_RESET     =  2,	// "Manual" reset
  VECNUM_ILLINSTR  =  4,	// General illegal instruction
  VECNUM_ILLSLOT   =  6,	// Slot illegal instruction
  VECNUM_CPUADDR   =  9,	// CPU address error
  VECNUM_DMAADDR   = 10,	// DMA Address error
  VECNUM_NMI	   = 11,	// NMI
  VECNUM_BREAK     = 12,	// User break

  VECNUM_TRAP_BASE = 32,	// Trap instruction
  VECNUM_INT_BASE  = 64,	// Interrupt
 };

 enum
 {
  EPENDING_IVECNUM_SHIFT = 8,	// 8 bits
  EPENDING_E_SHIFT = 16,	// 8 bits
  EPENDING_IPRIOLEV_SHIFT = 28	// 4 bits
 };

 uint32_t Exception(const unsigned exnum, const unsigned vecnum);

 //
 //
 //
 uint32_t IBuffer;
 uint32_t (MDFN_FASTCALL *MRFPI[8])(uint32_t A);

 uint8_t (MDFN_FASTCALL *MRFP8[8])(uint32_t A);
 uint16_t (MDFN_FASTCALL *MRFP16[8])(uint32_t A);
 uint32_t (MDFN_FASTCALL *MRFP32[8])(uint32_t A);

 uint16_t (MDFN_FASTCALL *MRFP16_I[8])(uint32_t A);
 uint32_t (MDFN_FASTCALL *MRFP32_I[8])(uint32_t A);

 void (MDFN_FASTCALL *MWFP8[8])(uint32_t A, uint8_t);
 void (MDFN_FASTCALL *MWFP16[8])(uint32_t A, uint16_t);
 void (MDFN_FASTCALL *MWFP32[8])(uint32_t A, uint32_t);

 void RecalcMRWFP_0(void);
 void RecalcMRWFP_1_7(void);

 sscpu_timestamp_t WB_until[16];

 //
 //
 // Cache:
 //
 //
 struct CacheEntry
 {
  // Rather than have separate validity bits, we're putting an INvalidity bit(invalid when =1)
  // in the lower bit of the Tag variables.
  uint32_t Tag[4];
  uint8_t Data[4][16];
 };
 alignas(16) CacheEntry Cache[64];

 uint8_t Cache_LRU[64];
 int32_t CCRC_Replace_OR[2];	// Cached cache var, calculated from the ID and OD bits of CCR in SetCCR()
 uint8_t CCRC_Replace_AND;	// Cached cache var, calculated from the TW bit of CCR in SetCCR()
 uint8_t CCR;

 void SetCCR(uint8_t V);
 enum { CCR_CE = 0x01 };	// Cache Enable
 enum { CCR_ID = 0x02 };	// Instruction Replacement Disable
 enum { CCR_OD = 0x04 };	// Data Replacement Disable
 enum { CCR_TW = 0x08 };	// Two-Way Mode
 enum { CCR_CP = 0x10 };	// Cache Purge
 enum { CCR_W0 = 0x40 };	//
 enum { CCR_W1 = 0x80 };	//

 void Cache_AssocPurge(const uint32_t A);

 int Cache_FindWay(CacheEntry* const cent, const uint32_t ATM);

 /* Phase-8h: Cache_WriteAddressArray, Cache_ReadAddressArray,
  * and Cache_CheckReadIncoherency lost their `template<typename T>`
  * parameter -- none of their bodies use T.  The two address-array
  * accessors handle a 32-bit tag word that's always the same width
  * regardless of the macro-passed T (V truncates implicitly on
  * write, and the caller's `retval = (T)...` truncates on read).
  * Cache_CheckReadIncoherency had an empty body and is gone
  * entirely; macro callsites drop the line.
  *
  * Phase-8i: Cache_WriteDataArray, Cache_ReadDataArray, and
  * Cache_WriteUpdate retired too -- but these DID use sizeof(T)
  * for the memcpy byte-count and NE32ASU8_IDX_ADJ byte-offset.
  * Each splits into three named width variants (uint8_t /
  * uint16_t / uint32_t).  Callers at template instantiation
  * sites pick the right one via a `sizeof(T)` if-chain that
  * gcc -O2 folds when T is known at macro-expansion time. */
 void Cache_WriteAddressArray(uint32_t A, uint32_t V);

 void Cache_WriteDataArray_u8 (uint32_t A, uint8_t  V);
 void Cache_WriteDataArray_u16(uint32_t A, uint16_t V);
 void Cache_WriteDataArray_u32(uint32_t A, uint32_t V);

 uint32_t Cache_ReadAddressArray(uint32_t A);

 uint8_t  Cache_ReadDataArray_u8 (uint32_t A);
 uint16_t Cache_ReadDataArray_u16(uint32_t A);
 uint32_t Cache_ReadDataArray_u32(uint32_t A);

 void Cache_WriteUpdate_u8 (uint32_t A, uint8_t  V);
 void Cache_WriteUpdate_u16(uint32_t A, uint16_t V);
 void Cache_WriteUpdate_u32(uint32_t A, uint32_t V);
 //
 // End cache stuff
 //

 //
 // Bus State Controller
 //
 struct
 {
  uint16_t BCR1;
  uint8_t BCR2;
  uint16_t WCR;
  uint16_t MCR;

  uint8_t RTCSR;
  uint8_t RTCSRM;
  uint8_t RTCNT;
  uint8_t RTCOR;
  //
  //
  //
  sscpu_timestamp_t sdram_finish_time;
  sscpu_timestamp_t last_mem_time;
  uint32_t last_mem_addr;
  uint32_t last_mem_type;
 } BSC;

 template<typename T>
 void INLINE BSC_BusWrite(uint32_t A, T V, const bool BurstHax, int32_t* SH2DMAHax);

 template<typename T>
 T INLINE BSC_BusRead(uint32_t A, const bool BurstHax, int32_t* SH2DMAHax);

 uint32_t UCRead_IF_Kludge;

 //
 // Exit/Resume stuff for slave CPU with icache emulation(RunSlaveUntil())
 //
 const void* ResumePoint;
 CacheEntry* Resume_cent;
 uint32_t Resume_instr;
 int Resume_way_match;
 uint32_t Resume_uint8_A;
 uint32_t Resume_uint16_A;
 uint32_t Resume_uint32_A;
 uint32_t Resume_unmasked_A;
 uint32_t Resume_uint32_V;
 uint16_t Resume_uint16_V;
 uint8_t Resume_uint8_V;

 int32_t Resume_MAC_L_m0;
 int32_t Resume_MAC_L_m1;

 int16_t Resume_MAC_W_m0;
 int16_t Resume_MAC_W_m1;

 uint32_t Resume_ea;
 uint32_t Resume_new_PC;
 uint32_t Resume_new_SR;

 uint8_t Resume_ipr;
 uint8_t Resume_exnum;
 uint8_t Resume_vecnum;

 //
 //
 // Interrupt controller registers and related state
 //
 //
 void INTC_Reset(void) MDFN_COLD;

 bool NMILevel;
 uint8_t IRL;

 uint16_t IPRA;
 uint16_t IPRB;
 uint16_t VCRWDT;
 uint16_t VCRA;
 uint16_t VCRB;
 uint16_t VCRC;
 uint16_t VCRD;
 uint16_t ICR;

 //
 //
 //
 uint8_t SBYCR;
 bool Standby;

 //
 //
 // Free-running timer registers and related state
 //
 //
 struct
 {
  sscpu_timestamp_t lastts;	// Internal timestamp related.

  bool FTI;
  bool FTCI;

  uint16_t FRC;
  uint16_t OCR[2];
  uint16_t FICR;
  uint8_t TIER;
  uint8_t FTCSR;
  uint8_t FTCSRM;	// Bits set to 1 like FTCSR, but unconditionally reset all bits to 0 on FTCSR read.
  uint8_t TCR;
  uint8_t TOCR;
  uint8_t RW_Temp;
 } FRT;

 void FRT_Reset(void) MDFN_COLD;

 void FRT_CheckOCR(void);
 void FRT_ClockFRC(void);

 void FRT_WDT_Update(void);
 void FRT_WDT_Recalc_NET(void);
 uint32_t FRT_WDT_ClockDivider;
 sscpu_timestamp_t FRT_WDT_NextTS;

 //
 //
 // Watchdog timer registers and related state.
 //
 //
 struct
 {
  uint8_t WTCSR;	// We don't let a CPU program set bit3 to 1, but we do set bit3 to 1 as part of the standby NMI recovery process(for internal use).
  uint8_t WTCSRM;
  uint8_t WTCNT;
  uint8_t RSTCSR;
  uint8_t RSTCSRM;
 } WDT;

 void WDT_Reset(bool from_internal_wdt) MDFN_COLD;	// Reset-reset only, NOT standby reset!
 void WDT_StandbyReset(void) MDFN_COLD;

 //
 // DMA unit registers and related state
 //
 bool DMA_RunCond(unsigned ch);
 bool DMA_InBurst(void);
 void DMA_CheckEnterBurstHack(void);
 void DMA_DoTransfer(unsigned ch);
 sscpu_timestamp_t DMA_Update(sscpu_timestamp_t);	// Takes/return external timestamp
 void DMA_StartSG(void);

 void DMA_RecalcRunning(void);
 void DMA_BusTimingKludge(void);

 const unsigned event_id_dma;
 sscpu_timestamp_t DMA_Timestamp;
 sscpu_timestamp_t DMA_SGEndTimestamp; // For smaller granularity scheduling for DMA_Update() after start of DMA.
 bool DMA_RoundRobinRockinBoppin;

 uint32_t DMA_PenaltyKludgeAmount;
 uint32_t DMA_PenaltyKludgeAccum;

 struct
 {
  uint32_t SAR;
  uint32_t DAR;
  uint32_t TCR;	// 24-bit, value of 0 = 2^24 tranfers
  uint16_t CHCR;
  uint16_t CHCRM;
  uint8_t VCR;
  uint8_t DRCR;
 } DMACH[2];

 uint8_t DMAOR;
 uint8_t DMAORM;


 //
 //
 // Division unit registers and related state
 //
 //
 void DIVU_S32_S32(void);
 void DIVU_S64_S32(void);

 sscpu_timestamp_t divide_finish_timestamp;
 uint32_t DVSR;
 uint32_t DVDNT;
 uint32_t DVDNTH;
 uint32_t DVDNTL;
 uint32_t DVDNTH_Shadow;
 uint32_t DVDNTL_Shadow;
 uint16_t VCRDIV;
 uint8_t DVCR;

 struct
 {
  uint8_t SMR;	// Mode
  uint8_t BRR;	// Bit rate
  uint8_t SCR;	// Control
  uint8_t TDR;	// Transmit data
  uint8_t SSR, SSRM;	// Status
  uint8_t RDR;	// Receive data

  uint8_t RSR;	// Receive shift register
  uint8_t TSR;	// Transmit shift register
 } SCI;

 void SCI_Reset(void) MDFN_COLD;

 //
 //
 //
 bool ExtHalt;
 uint8_t ExtHaltDMA;

 uint8_t (*const ExIVecFetch)(void);
 uint8_t GetPendingInt(uint8_t*);
 void RecalcPendingIntPEX(void);

 template<bool EmulateICache, bool IntPreventNext>
 INLINE void DoIDIF_INLINE(void);

 template<bool EmulateICache, bool IntPreventNext>
 NO_INLINE void DoIDIF_NI(void) MDFN_HOT;

 template<bool SlavePenalty, typename T, bool BurstHax>
 INLINE T ExtBusRead_INLINE(uint32_t A);

 template<bool SlavePenalty, typename T>
 INLINE void ExtBusWrite_INLINE(uint32_t A, T V);

 template<typename T>
 NO_INLINE void OnChipRegWrite(uint32_t A, uint32_t V) MDFN_HOT;

 template<typename T>
 INLINE T OnChipRegRead_INLINE(uint32_t A);

 template<unsigned which, int NeedSlaveCall, bool CacheBypassHack, typename T, unsigned region, bool CacheEnabled, int32_t IsInstr>
 INLINE T MemReadRT(uint32_t A);

 template<unsigned which, int NeedSlaveCall, typename T, unsigned region, bool CacheEnabled>
 INLINE void MemWriteRT(uint32_t A, T V);
 //
 //
 //
 //
 //
 //
 public:

 enum
 {
  // GSREG_PC_ID and GSREG_PC_IF are only valid when Step<true>() was called most recently(but they may be invalid
  // for a while after <false>, too...).
  GSREG_PC_ID = 0,
  GSREG_PC_IF,

  GSREG_PID,
  GSREG_PIF,

  GSREG_EP,

  GSREG_RPC,

  GSREG_R0,  GSREG_R1,  GSREG_R2,  GSREG_R3,  GSREG_R4,  GSREG_R5,  GSREG_R6,  GSREG_R7,
  GSREG_R8,  GSREG_R9,  GSREG_R10, GSREG_R11, GSREG_R12, GSREG_R13, GSREG_R14, GSREG_R15,

  GSREG_SR,
  GSREG_GBR,
  GSREG_VBR,

  GSREG_MACH,
  GSREG_MACL,
  GSREG_PR,
  //
  //
  //
  GSREG_NMIL,
  GSREG_IRL,
  GSREG_IPRA,
  GSREG_IPRB,
  GSREG_VCRWDT,
  GSREG_VCRA,
  GSREG_VCRB,
  GSREG_VCRC,
  GSREG_VCRD,
  GSREG_ICR,
  //
  //
  //
  GSREG_DVSR,
  GSREG_DVDNT,
  GSREG_DVDNTH,
  GSREG_DVDNTL,
  GSREG_DVDNTHS,
  GSREG_DVDNTLS,
  GSREG_VCRDIV,
  GSREG_DVCR,

  //
  //
  //
  GSREG_WTCSR,
  GSREG_WTCSRM,
  GSREG_WTCNT,
  GSREG_RSTCSR,
  GSREG_RSTCSRM,
  //
  //
  //
  GSREG_DMAOR,
  GSREG_DMAORM,

  GSREG_DMA0_SAR,
  GSREG_DMA0_DAR,
  GSREG_DMA0_TCR,
  GSREG_DMA0_CHCR,
  GSREG_DMA0_CHCRM,
  GSREG_DMA0_VCR,
  GSREG_DMA0_DRCR,

  GSREG_DMA1_SAR,
  GSREG_DMA1_DAR,
  GSREG_DMA1_TCR,
  GSREG_DMA1_CHCR,
  GSREG_DMA1_CHCRM,
  GSREG_DMA1_VCR,
  GSREG_DMA1_DRCR,

  GSREG_FRC,
  GSREG_OCR0,
  GSREG_OCR1,
  GSREG_FICR,
  GSREG_TIER,
  GSREG_FTCSR,
  GSREG_FTCSRM,
  GSREG_TCR,
  GSREG_TOCR,
  GSREG_RWT,

  GSREG_CCR,
  GSREG_SBYCR
 };

 uint32_t GetRegister(const unsigned id, char* const special, const uint32_t special_len);
 void SetRegister(const unsigned id, const uint32_t value) MDFN_COLD;

 void CheckRWBreakpoints(void (*MRead)(unsigned len, uint32_t addr), void (*MWrite)(unsigned len, uint32_t addr)) const;
 private:
 bool CBH_Setting;
 bool EIC_Setting;
 bool DM_Setting;
 uint32_t PC_IF, PC_ID;	// Debug-related variables.
 const char* const cpu_name;
 const void*const* ResumeTableP[2];
};
#endif
