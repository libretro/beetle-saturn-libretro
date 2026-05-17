/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* m68k.cpp - Motorola 68000 CPU Emulator
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

// TODO: Check CHK
//
// TODO: Address errors(or just cheap out and mask off the lower bit on 16-bit memory accesses).
//
// TODO: Predec, postinc order for same address register.
//
// TODO: Fix instruction timings(currently execute too fast).
//
// TODO: Fix division timing, and make sure flags are ok for divide by zero.
//
// FIXME: Handle NMI differently; how to test?  Maybe MOVEM to interrupt control registers...
//
// TODO: Test MOVEM
//
/*
 Be sure to test the following thoroughly:
	SUBA -(a0), a0
	SUBX -(a0),-(a0)
	CMPM (a0)+,(a0)+

	SUBA -(a7), a7
	SUBX -(a7),-(a7)
	CMPM (a7)+,(a7)+
*/

#include "../../mednafen.h"
#include "m68k.h"

#include <tuple>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize ("no-crossjumping,no-gcse")
#endif

#include "m68k_private.h"

static MDFN_FASTCALL void Dummy_BusRESET(bool state) { }

/* All M68K_* free-function wrappers exposed by m68k.h live in this
 * `extern "C" { ... }` block so the symbol names are C-mangled
 * (i.e. unmangled).  Required because m68k.h's declarations are
 * also wrapped in `extern "C"` for C-consumer compatibility --
 * the definition and declaration linkage must match or the linker
 * gets two different mangled symbols and fails to resolve.
 *
 * The bodies are pure thunks: each forwards to the matching
 * struct M68K member method.  Bodies are in this TU (m68k.cpp)
 * because they reach Dummy_BusRESET / member methods / sources
 * not exposed in the public header. */
extern "C" {

void M68K_Construct(M68K* z, bool rev_e)
{
   z->Revision_E   = rev_e;

   z->BusReadInstr = NULL;
   z->BusRead8     = NULL;
   z->BusRead16    = NULL;
   z->BusWrite8    = NULL;
   z->BusWrite16   = NULL;
   z->BusRMW       = NULL;
   z->BusIntAck    = NULL;
   z->BusRESET     = Dummy_BusRESET;

   z->timestamp    = 0;
   z->XPending     = 0;
   z->IPL          = 0;

   z->Reset(true);
}

void     M68K_SetIPL             (M68K* z, uint8_t ipl_new)
{
 if(z->IPL < 0x7 && ipl_new == 0x7)
  z->XPending |= M68K::XPENDING_MASK_NMI;
 else if(ipl_new < 0x7)
  z->XPending &= ~M68K::XPENDING_MASK_NMI;

 z->IPL = ipl_new;
 z->RecalcInt();
}
void     M68K_SignalDTACKHalted  (M68K* z, uint32_t addr)               { z->SignalDTACKHalted(addr); }
void     M68K_SignalAddressError (M68K* z, uint32_t addr, uint8_t type) { z->SignalAddressError(addr, type); }
void     M68K_Reset              (M68K* z, bool pwr)                    { z->Reset(pwr); }
void     M68K_Run                (M68K* z, int32_t until)               { z->Run(until); }
void     M68K_SetExtHalted       (M68K* z, bool state)
{
 z->XPending &= ~M68K::XPENDING_MASK_EXTHALTED;
 if(state)
  z->XPending |= M68K::XPENDING_MASK_EXTHALTED;
}
void     M68K_StateAction        (M68K* z, StateMem* sm, const unsigned load,
                                  const bool data_only, const char* sname)
{
 /* Field-name strings below intentionally use the pre-Phase-9d
  * spelling (no `z->` prefix): they are persisted into savestate
  * files, so they must match the strings the member-function
  * StateAction implicitly produced via SFVAR(X) -> SFVARN((X), #X). */
 SFORMAT StateRegs[] =
 {
  SFPTR32N(&(z->DA)[0], (sizeof(z->DA) / sizeof(uint32_t)), "DA"),
  SFVARN(z->PC,          "PC"),
  SFVARN(z->SRHB,        "SRHB"),
  SFVARN(z->IPL,         "IPL"),

  SFVARN(z->Flag_Z,      "Flag_Z"),
  SFVARN(z->Flag_N,      "Flag_N"),
  SFVARN(z->Flag_X,      "Flag_X"),
  SFVARN(z->Flag_C,      "Flag_C"),
  SFVARN(z->Flag_V,      "Flag_V"),

  SFVARN(z->SP_Inactive, "SP_Inactive"),

  SFVARN(z->XPending,    "XPending"),

  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, sname, false);

 if(load)
  z->XPending &= M68K::XPENDING_MASK__VALID;
}
uint32_t M68K_GetRegister        (M68K* z, const unsigned id, char* const special, const uint32_t special_len)
{
 (void)special; (void)special_len; /* reserved-for-future-use, unused today */
 switch(id)
 {
  default:
	return 0xDEADBEEF;

  case M68K::GSREG_D0: case M68K::GSREG_D1: case M68K::GSREG_D2: case M68K::GSREG_D3:
  case M68K::GSREG_D4: case M68K::GSREG_D5: case M68K::GSREG_D6: case M68K::GSREG_D7:
	return z->D[id - M68K::GSREG_D0];

  case M68K::GSREG_A0: case M68K::GSREG_A1: case M68K::GSREG_A2: case M68K::GSREG_A3:
  case M68K::GSREG_A4: case M68K::GSREG_A5: case M68K::GSREG_A6: case M68K::GSREG_A7:
	return z->A[id - M68K::GSREG_A0];

  case M68K::GSREG_PC:
	return z->PC;

  case M68K::GSREG_SR:
	return z->GetSR();

  case M68K::GSREG_SSP:
	if(z->GetSVisor())
	 return z->A[7];
	else
	 return z->SP_Inactive;

  case M68K::GSREG_USP:
	if(!z->GetSVisor())
	 return z->A[7];
	else
	 return z->SP_Inactive;
 }
}
void     M68K_SetRegister        (M68K* z, const unsigned id, const uint32_t value)
{
 switch(id)
 {
  case M68K::GSREG_D0: case M68K::GSREG_D1: case M68K::GSREG_D2: case M68K::GSREG_D3:
  case M68K::GSREG_D4: case M68K::GSREG_D5: case M68K::GSREG_D6: case M68K::GSREG_D7:
	z->D[id - M68K::GSREG_D0] = value;
	break;

  case M68K::GSREG_A0: case M68K::GSREG_A1: case M68K::GSREG_A2: case M68K::GSREG_A3:
  case M68K::GSREG_A4: case M68K::GSREG_A5: case M68K::GSREG_A6: case M68K::GSREG_A7:
	z->A[id - M68K::GSREG_A0] = value;
	break;

  case M68K::GSREG_PC:
	z->PC = value;
	break;

  case M68K::GSREG_SR:
	z->SetSR(value);
	break;

  case M68K::GSREG_SSP:
	if(z->GetSVisor())
	 z->A[7] = value;
	else
	 z->SP_Inactive = value;
	break;

  case M68K::GSREG_USP:
	if(!z->GetSVisor())
	 z->A[7] = value;
	else
	 z->SP_Inactive = value;
	break;
 }
}

} /* extern "C" */

/* Phase-9 cleanup: M68K::M68K(const bool) and M68K::~M68K() retired.
 * Zero remaining callers after sound_glue.cpp -> sound_glue.c
 * (fd5bf98) switched from `static M68K SoundCPU(true);` to a
 * zero-initialised SoundCPU plus an explicit M68K_Construct call
 * in SoundGlue_Init().  The ctor body matched M68K_Construct's
 * body 1:1 (the prep commit 5cafd34's free-function counterpart
 * for the same work).  The dtor body was empty.  M68K is a pure-
 * data struct now -- no class methods need calling at end-of-
 * scope, no class methods need calling at construction. */

/* Phase-9d-3: M68K::StateAction, M68K::GetRegister, M68K::SetRegister
 * retired -- bodies live in their M68K_* extern "C" wrappers above.
 * Same incremental-fold pattern as Phase-9d-1 (SetIPL/SetExtHalted).
 * Class declarations dropped from m68k.h's `#ifdef __cplusplus`
 * gated region 1. */

//
//
//
enum
{
 VECNUM_RESET_SSP = 0,
 VECNUM_RESET_PC  = 1,
 VECNUM_BUS_ERROR = 2,
 VECNUM_ADDRESS_ERROR = 3,
 VECNUM_ILLEGAL = 4,
 VECNUM_ZERO_DIVIDE = 5,
 VECNUM_CHK = 6,
 VECNUM_TRAPV = 7,
 VECNUM_PRIVILEGE = 8,
 VECNUM_TRACE = 9,
 VECNUM_LINEA = 10,
 VECNUM_LINEF = 11,

 VECNUM_UNINI_INT = 15,

 VECNUM_SPURIOUS_INT = 24,
 VECNUM_INT_BASE = 24,

 VECNUM_TRAP_BASE = 32
};

enum
{
 EXCEPTION_RESET = 0,
 EXCEPTION_BUS_ERROR,
 EXCEPTION_ADDRESS_ERROR,
 EXCEPTION_ILLEGAL,
 EXCEPTION_ZERO_DIVIDE,
 EXCEPTION_CHK,
 EXCEPTION_TRAPV,
 EXCEPTION_PRIVILEGE,
 EXCEPTION_TRACE,

 EXCEPTION_INT,
 EXCEPTION_TRAP
};

//
// Instruction traps(TRAP, TRAPV, CHK, DIVS, DIVU):
//	Saved PC points to the instruction after the instruction that triggered the exception.
//
// Illegal instructions:
//
//
// Privilege violation:
// 	Saved PC points to the instruction that generated the privilege violation.
//
// Base exception timing is 34 cycles?
void NO_INLINE M68K::Exception(unsigned which, unsigned vecnum)
{
 const uint32_t PC_save = PC;
 const uint16_t SR_save = GetSR();

 SetSR((GetSR() & ~0x2000) | (1 << 13));
 SetSR((GetSR() & ~0x8000));
 
 if(which == EXCEPTION_INT)
 {
  unsigned evn;

  timestamp += 4;

  SetSR((GetSR() & ~0x0700) | ((IPL & 0x7) << 8));

  evn = BusIntAck(IPL);

  if(evn > 255)
   vecnum = vecnum + IPL;
  else
   vecnum = evn;

  timestamp += 2;
 }

 Push_u32(PC_save);
 Push_u16(SR_save);

 if(MDFN_UNLIKELY(which == EXCEPTION_BUS_ERROR || which == EXCEPTION_ADDRESS_ERROR))
 {
  Push_u16(0); // TODO: Instruction register
  Push_u32(0); // TODO: Access address
  Push_u16(0); // TODO: R/W, I/N, function code
 }

 PC = Read_u32(vecnum << 2);

 // TODO: Prefetch
 ReadOp();
 ReadOp();
 PC -= 4;
}

//
//
//

//
// TAS
//
MDFN_FASTCALL uint8_t TAS_Callback(M68K* zptr, uint8_t data)
{
 zptr->CalcZN_u8(data);
 zptr->Flag_C = false;
 zptr->Flag_V = false;

 data |= 0x80;
 return data;
}

void NO_INLINE M68K::Run(int32_t run_until_time)
{
 while(MDFN_LIKELY(timestamp < run_until_time))
 {
	 if(MDFN_UNLIKELY(XPending))
	 {
		 if(MDFN_LIKELY(!(XPending & (XPENDING_MASK_ERRORHALTED | XPENDING_MASK_DTACKHALTED | XPENDING_MASK_EXTHALTED))))
		 {
			 if(MDFN_UNLIKELY(XPending & (XPENDING_MASK_RESET | XPENDING_MASK_ADDRESS | XPENDING_MASK_BUS)))
			 {
				 if(XPending & XPENDING_MASK_RESET)
				 {
					 SetSR((GetSR() & ~0x2000) | (1 << 13));
					 SetSR((GetSR() & ~0x8000));
					 SetSR((GetSR() & ~0x0700) | (0x7 << 8));

					 A[7] = Read_u32(VECNUM_RESET_SSP << 2);
					 PC = Read_u32(VECNUM_RESET_PC << 2);
					 //
					 XPending &= ~XPENDING_MASK_RESET;
				 }
				 else
				 {
					 if(XPending & XPENDING_MASK_BUS)
						 Exception(EXCEPTION_BUS_ERROR, VECNUM_BUS_ERROR);
					 else
						 Exception(EXCEPTION_ADDRESS_ERROR, VECNUM_ADDRESS_ERROR);
					 // Clear bus/address error bits in XPending only after Exception() returns normally:
					 XPending &= ~(XPENDING_MASK_BUS | XPENDING_MASK_ADDRESS);
				 }

				 return;
			 }
			 else if(XPending & (XPENDING_MASK_INT | XPENDING_MASK_NMI))
			 {
				 assert(IPL == 0x7 || IPL > ((GetSR() >> 8) & 0x7));
				 XPending &= ~(XPENDING_MASK_STOPPED | XPENDING_MASK_INT | XPENDING_MASK_NMI);

				 Exception(EXCEPTION_INT, VECNUM_INT_BASE);

				 return;
			 }
		 }

		 // STOP and ExtHalted fallthrough:
		 timestamp += 4;
		 return;
	 }
	 //
	 //
	 //
	 uint16_t instr = ReadOp();
	 const unsigned instr_b11_b9 = (instr >> 9) & 0x7;
	 const unsigned instr_b2_b0 = instr & 0x7;
#ifdef M68K_SPLIT_SWITCH
	 if(instr & 0x8000)
	  RunSplit1(instr & 0x7fff, instr_b11_b9, instr_b2_b0);
	 else
	  RunSplit0(instr, instr_b11_b9, instr_b2_b0);
#else
	 switch(instr)
	 {
		 default: ILLEGAL(instr); break;
#include "m68k_instr.inc"
	 }
#endif
 }
}

//
// Reset() may be called from BusRESET, which is called from RESET, so ensure it continues working for that case.
//
void M68K::Reset(bool powering_up)
{
 if(powering_up)
 {
  PC = 0;

  for(unsigned i = 0; i < 8; i++)
   D[i] = 0;

  for(unsigned i = 0; i < 8; i++)
   A[i] = 0;

  SP_Inactive = 0;

  SetSR(0);
 }
 XPending = (XPending & ~(XPENDING_MASK_STOPPED | XPENDING_MASK_NMI | XPENDING_MASK_ADDRESS | XPENDING_MASK_BUS | XPENDING_MASK_ERRORHALTED | XPENDING_MASK_DTACKHALTED)) | XPENDING_MASK_RESET;
}


//
// Phase-9d-3: M68K::GetRegister / M68K::SetRegister bodies retired -- both
// now live in the M68K_GetRegister / M68K_SetRegister extern "C" wrappers
// at the top of this file.  See the matching note further up.
//
