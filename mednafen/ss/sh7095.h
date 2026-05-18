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

#include "../state.h"

/* C-compat typedefs: in C the struct tag is not auto-aliased to a
 * type name (that aliasing is C++ name-injection).  Forward-declare
 * the tags as typedefs so `SH7095*` and `SH7095_CacheEntry*` work
 * as type references in both languages, and so SH7095_CacheEntry
 * inside the SH7095 struct body parses cleanly in C. */
typedef struct SH7095 SH7095;
typedef struct SH7095_CacheEntry SH7095_CacheEntry;

 struct SH7095_CacheEntry
 {
  // Rather than have separate validity bits, we're putting an INvalidity bit(invalid when =1)
  // in the lower bit of the Tag variables.
  uint32_t Tag[4];
  uint8_t Data[4][16];
 };

 enum // must be in range of 0 ... 7
 {
  SH7095_PEX_POWERON = 0,
  SH7095_PEX_RESET   = 1,
  SH7095_PEX_CPUADDR = 2,
  SH7095_PEX_DMAADDR = 3,
  SH7095_PEX_INT     = 4,
  SH7095_PEX_NMI     = 5,
  SH7095_PEX_PSEUDO_DMABURST = 6,
  SH7095_PEX_PSEUDO_EXTHALT = 7
 };

 enum { SH7095_EPENDING_PEXBITS_SHIFT = 16 };

 enum { SH7095_EPENDING_OP_OR = 0xFF000000 };

 enum
 {
  SH7095_EXCEPTION_POWERON = 0,// Power-on
  SH7095_EXCEPTION_RESET,	// "Manual" reset
  SH7095_EXCEPTION_ILLINSTR,	// General illegal instruction
  SH7095_EXCEPTION_ILLSLOT,	// Slot illegal instruction
  SH7095_EXCEPTION_CPUADDR,	// CPU address error
  SH7095_EXCEPTION_DMAADDR,	// DMA Address error
  SH7095_EXCEPTION_NMI,	// NMI
  SH7095_EXCEPTION_BREAK,	// User break
  SH7095_EXCEPTION_TRAP,	// Trap instruction
  SH7095_EXCEPTION_INT,	// Interrupt
 };

 enum
 {
  SH7095_VECNUM_POWERON   =  0,	// Power-on
  SH7095_VECNUM_RESET     =  2,	// "Manual" reset
  SH7095_VECNUM_ILLINSTR  =  4,	// General illegal instruction
  SH7095_VECNUM_ILLSLOT   =  6,	// Slot illegal instruction
  SH7095_VECNUM_CPUADDR   =  9,	// CPU address error
  SH7095_VECNUM_DMAADDR   = 10,	// DMA Address error
  SH7095_VECNUM_NMI	   = 11,	// NMI
  SH7095_VECNUM_BREAK     = 12,	// User break

  SH7095_VECNUM_TRAP_BASE = 32,	// Trap instruction
  SH7095_VECNUM_INT_BASE  = 64,	// Interrupt
 };

 enum
 {
  SH7095_EPENDING_IVECNUM_SHIFT = 8,	// 8 bits
  SH7095_EPENDING_E_SHIFT = 16,	// 8 bits
  SH7095_EPENDING_IPRIOLEV_SHIFT = 28	// 4 bits
 };

 enum { SH7095_CCR_CE = 0x01 };

 enum { SH7095_CCR_ID = 0x02 };

 enum { SH7095_CCR_OD = 0x04 };

 enum { SH7095_CCR_TW = 0x08 };

 enum { SH7095_CCR_CP = 0x10 };

 enum { SH7095_CCR_W0 = 0x40 };

 enum { SH7095_CCR_W1 = 0x80 };

 enum
 {
  // SH7095_GSREG_PC_ID and SH7095_GSREG_PC_IF are only valid when Step<true>() was called most recently(but they may be invalid
  // for a while after <false>, too...).
  SH7095_GSREG_PC_ID = 0,
  SH7095_GSREG_PC_IF,

  SH7095_GSREG_PID,
  SH7095_GSREG_PIF,

  SH7095_GSREG_EP,

  SH7095_GSREG_RPC,

  SH7095_GSREG_R0,  SH7095_GSREG_R1,  SH7095_GSREG_R2,  SH7095_GSREG_R3,  SH7095_GSREG_R4,  SH7095_GSREG_R5,  SH7095_GSREG_R6,  SH7095_GSREG_R7,
  SH7095_GSREG_R8,  SH7095_GSREG_R9,  SH7095_GSREG_R10, SH7095_GSREG_R11, SH7095_GSREG_R12, SH7095_GSREG_R13, SH7095_GSREG_R14, SH7095_GSREG_R15,

  SH7095_GSREG_SR,
  SH7095_GSREG_GBR,
  SH7095_GSREG_VBR,

  SH7095_GSREG_MACH,
  SH7095_GSREG_MACL,
  SH7095_GSREG_PR,
  //
  //
  //
  SH7095_GSREG_NMIL,
  SH7095_GSREG_IRL,
  SH7095_GSREG_IPRA,
  SH7095_GSREG_IPRB,
  SH7095_GSREG_VCRWDT,
  SH7095_GSREG_VCRA,
  SH7095_GSREG_VCRB,
  SH7095_GSREG_VCRC,
  SH7095_GSREG_VCRD,
  SH7095_GSREG_ICR,
  //
  //
  //
  SH7095_GSREG_DVSR,
  SH7095_GSREG_DVDNT,
  SH7095_GSREG_DVDNTH,
  SH7095_GSREG_DVDNTL,
  SH7095_GSREG_DVDNTHS,
  SH7095_GSREG_DVDNTLS,
  SH7095_GSREG_VCRDIV,
  SH7095_GSREG_DVCR,

  //
  //
  //
  SH7095_GSREG_WTCSR,
  SH7095_GSREG_WTCSRM,
  SH7095_GSREG_WTCNT,
  SH7095_GSREG_RSTCSR,
  SH7095_GSREG_RSTCSRM,
  //
  //
  //
  SH7095_GSREG_DMAOR,
  SH7095_GSREG_DMAORM,

  SH7095_GSREG_DMA0_SAR,
  SH7095_GSREG_DMA0_DAR,
  SH7095_GSREG_DMA0_TCR,
  SH7095_GSREG_DMA0_CHCR,
  SH7095_GSREG_DMA0_CHCRM,
  SH7095_GSREG_DMA0_VCR,
  SH7095_GSREG_DMA0_DRCR,

  SH7095_GSREG_DMA1_SAR,
  SH7095_GSREG_DMA1_DAR,
  SH7095_GSREG_DMA1_TCR,
  SH7095_GSREG_DMA1_CHCR,
  SH7095_GSREG_DMA1_CHCRM,
  SH7095_GSREG_DMA1_VCR,
  SH7095_GSREG_DMA1_DRCR,

  SH7095_GSREG_FRC,
  SH7095_GSREG_OCR0,
  SH7095_GSREG_OCR1,
  SH7095_GSREG_FICR,
  SH7095_GSREG_TIER,
  SH7095_GSREG_FTCSR,
  SH7095_GSREG_FTCSRM,
  SH7095_GSREG_TCR,
  SH7095_GSREG_TOCR,
  SH7095_GSREG_RWT,

  SH7095_GSREG_CCR,
  SH7095_GSREG_SBYCR
 };
struct SH7095
{

 // Slave only

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

 uint32_t EPending;

 uint32_t Pipe_ID;
 uint32_t Pipe_IF;
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
 sscpu_timestamp_t WB_until[16];

 //
 //
 // Cache:
 //
 //
 MDFN_ALIGN(16) SH7095_CacheEntry Cache[64];

 uint8_t Cache_LRU[64];
 int32_t CCRC_Replace_OR[2];	// Cached cache var, calculated from the ID and OD bits of CCR in SetCCR()
 uint8_t CCRC_Replace_AND;	// Cached cache var, calculated from the TW bit of CCR in SetCCR()
 uint8_t CCR;	// Cache Enable	// Instruction Replacement Disable	// Data Replacement Disable	// Two-Way Mode	// Cache Purge	//	//
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

 uint32_t UCRead_IF_Kludge;

 //
 // Exit/Resume stuff for slave CPU with icache emulation(RunSlaveUntil())
 //
 uint16_t resume_id;
 SH7095_CacheEntry* Resume_cent;
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

 //
 // DMA unit registers and related state
 //
 unsigned event_id_dma;
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
 //
 //
 //
 bool ExtHalt;
 uint8_t ExtHaltDMA;

 uint8_t (*ExIVecFetch)(void);

 //
 //
 //
 //
 //
 //

  bool CBH_Setting;
 bool EIC_Setting;
 bool DM_Setting;
 uint32_t PC_IF, PC_ID;	// Debug-related variables.
 const char* cpu_name;
};

/* Phase-9 step 4: SH7095 public API as free functions. */
void SH7095_SetIRL                     (SH7095* z, unsigned level);
void SH7095_Init                       (SH7095* z, bool EmulateICache, bool CacheBypassHack) MDFN_COLD;
void SH7095_Reset                      (SH7095* z, bool power_on_reset, bool from_internal_wdt) MDFN_COLD;
void SH7095_TruePowerOn                (SH7095* z) MDFN_COLD;
void SH7095_AdjustTS                   (SH7095* z, int32_t delta);
void SH7095_SetActive                  (SH7095* z, bool active);
void SH7095_SetNMI                     (SH7095* z, bool level);
void SH7095_SetMD5                     (SH7095* z, bool level);
void SH7095_SetFTI                     (SH7095* z, bool state);
void SH7095_Cache_WriteUpdate_u8       (SH7095* z, uint32_t A, uint8_t V);
sscpu_timestamp_t SH7095_DMA_Update    (SH7095* z, sscpu_timestamp_t ts);
void SH7095_ForceInternalEventUpdates  (SH7095* z);
void SH7095_Step_w0_C0                 (SH7095* z);
void SH7095_Step_w0_C1                 (SH7095* z);
void SH7095_Step_w1_C0                 (SH7095* z);
void SH7095_DMA_BusTimingKludge        (SH7095* z);
void SH7095_RunSlaveUntil              (SH7095* z, sscpu_timestamp_t ts);
void SH7095_StateAction                (SH7095* z, StateMem* sm, unsigned load, bool data_only, const char* sname) MDFN_COLD;
void SH7095_PostStateLoad              (SH7095* z, unsigned state_version, bool recorded_ni, bool ni) MDFN_COLD;

/* Phase-9 step 4 cont.: small inline helpers (SR/MAC/PEX accessors,
 * pending-int probe) moved off the SH7095 struct as free functions.
 * Same bodies, taking SH7095* z explicitly so the struct can be pure
 * data in the eventual C migration. */
static FORCE_INLINE void     SH7095_SetT      (SH7095* z, bool v)      { z->SR &= ~1; z->SR |= v; }
static FORCE_INLINE bool     SH7095_GetT      (const SH7095* z)        { return z->SR & 1; }
static FORCE_INLINE bool     SH7095_GetS      (const SH7095* z)        { return (bool)(z->SR & 0x002); }
static FORCE_INLINE bool     SH7095_GetQ      (const SH7095* z)        { return (bool)(z->SR & 0x100); }
static FORCE_INLINE bool     SH7095_GetM      (const SH7095* z)        { return (bool)(z->SR & 0x200); }
static FORCE_INLINE void     SH7095_SetQ      (SH7095* z, bool new_q)  { z->SR = (z->SR & ~0x100) | (new_q << 8); }
static FORCE_INLINE void     SH7095_SetM      (SH7095* z, bool new_m)  { z->SR = (z->SR & ~0x200) | (new_m << 9); }
static FORCE_INLINE uint64_t SH7095_GetMAC64  (const SH7095* z)        { return z->MACL | ((uint64_t)z->MACH << 32); }
static FORCE_INLINE void     SH7095_SetMAC64  (SH7095* z, uint64_t nv) { z->MACL = nv; z->MACH = nv >> 32; }
static FORCE_INLINE void     SH7095_SetPEX    (SH7095* z, const unsigned which)
{
 z->EPending |= (1U << (which + SH7095_EPENDING_PEXBITS_SHIFT));
 z->EPending |= SH7095_EPENDING_OP_OR;
}
static FORCE_INLINE void     SH7095_ClearPEX  (SH7095* z, const unsigned which)
{
 z->EPending &= ~(1U << (which + SH7095_EPENDING_PEXBITS_SHIFT));
 if(!(z->EPending & (0xFF << SH7095_EPENDING_PEXBITS_SHIFT)))
  z->EPending = 0;
}
static FORCE_INLINE void SH7095_SetExtHalt(SH7095* z, bool state)
{
 z->ExtHalt = state;
 if(z->ExtHalt)
  SH7095_SetPEX(z, SH7095_PEX_PSEUDO_EXTHALT);
 z->ExtHaltDMA = (z->ExtHaltDMA & ~1) | state;
}
static FORCE_INLINE void SH7095_SetExtHaltDMAKludgeFromVDP2(SH7095* z, bool state)
{
 z->ExtHaltDMA = (z->ExtHaltDMA & ~2) | (state << 1);
}
/* SH7095_GetPendingInt is defined in sh7095.inc; call sites in
 * sh7095_ops.inc (which is itself included inside sh7095.inc)
 * see the definition through the chain. */

void SH7095_Construct  (SH7095* z, const char* name, unsigned event_id_dma, uint8_t (*exivecfn)(void)) MDFN_COLD;
void SH7095_Step_w0_C0 (SH7095* z);
void SH7095_Step_w0_C1 (SH7095* z);
void SH7095_Step_w1_C0 (SH7095* z);

#endif
