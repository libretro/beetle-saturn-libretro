/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* m68k.c - Motorola 68000 CPU Emulator
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

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize ("no-crossjumping,no-gcse")
#endif

#include "m68k_private.h"

static MDFN_FASTCALL void Dummy_BusRESET(bool state) { }

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

   M68K_Reset(z, true);
}

void     M68K_SetIPL             (M68K* z, uint8_t ipl_new)
{
 if(z->IPL < 0x7 && ipl_new == 0x7)
  z->XPending |= XPENDING_MASK_NMI;
 else if(ipl_new < 0x7)
  z->XPending &= ~XPENDING_MASK_NMI;

 z->IPL = ipl_new;
 RecalcInt(z);
}
void     M68K_SignalDTACKHalted  (M68K* z, uint32_t addr)
{
 /* Called from external bus read/write handlers, followed by a
  * longjmp() back to above Run().  `addr` is currently unused
  * here but kept in the API signature to leave room for richer
  * bus-error diagnostics. */
 (void)addr;
 z->XPending |= XPENDING_MASK_DTACKHALTED;
}
void     M68K_SignalAddressError (M68K* z, uint32_t addr, uint8_t type)
{
 /* Same external-bus-handler entry path as SignalDTACKHalted; the
  * (addr, type) tuple is reserved for future bus-error reporting. */
 (void)addr; (void)type;
 if(z->XPending & (XPENDING_MASK_ADDRESS | XPENDING_MASK_BUS | XPENDING_MASK_RESET))
 {
  z->XPending |= XPENDING_MASK_ERRORHALTED;
 }

 z->XPending |= XPENDING_MASK_ADDRESS;
}
void     M68K_Reset              (M68K* z, bool pwr)
{
 /* Reset() may be called from BusRESET (a callback function-pointer set by
  * the integrator -- in our case sound_glue.c's SoundGlue_M68K_Reset), which
  * itself is called from the m68k_private.h:2104/2106 RESET-instruction
  * handler.  The callback path resolves to M68K_Reset(&SoundCPU, ...) again,
  * so this body must continue to work re-entrantly. */
 if(pwr)
 {
  z->PC = 0;

  for(unsigned i = 0; i < 8; i++)
   z->D[i] = 0;

  for(unsigned i = 0; i < 8; i++)
   z->A[i] = 0;

  z->SP_Inactive = 0;

  SetSR(z, 0);
 }
 z->XPending = (z->XPending & ~(XPENDING_MASK_STOPPED | XPENDING_MASK_NMI | XPENDING_MASK_ADDRESS | XPENDING_MASK_BUS | XPENDING_MASK_ERRORHALTED | XPENDING_MASK_DTACKHALTED)) | XPENDING_MASK_RESET;
}
void     M68K_SetExtHalted       (M68K* z, bool state)
{
 z->XPending &= ~XPENDING_MASK_EXTHALTED;
 if(state)
  z->XPending |= XPENDING_MASK_EXTHALTED;
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
  z->XPending &= XPENDING_MASK__VALID;
}
void     M68K_SetRegister        (M68K* z, const unsigned id, const uint32_t value)
{
 switch(id)
 {
  case GSREG_D0: case GSREG_D1: case GSREG_D2: case GSREG_D3:
  case GSREG_D4: case GSREG_D5: case GSREG_D6: case GSREG_D7:
	z->D[id - GSREG_D0] = value;
	break;

  case GSREG_A0: case GSREG_A1: case GSREG_A2: case GSREG_A3:
  case GSREG_A4: case GSREG_A5: case GSREG_A6: case GSREG_A7:
	z->A[id - GSREG_A0] = value;
	break;

  case GSREG_PC:
	z->PC = value;
	break;

  case GSREG_SR:
	SetSR(z, value);
	break;

  case GSREG_SSP:
	if(GetSVisor(z))
	 z->A[7] = value;
	else
	 z->SP_Inactive = value;
	break;

  case GSREG_USP:
	if(!GetSVisor(z))
	 z->A[7] = value;
	else
	 z->SP_Inactive = value;
	break;
 }
}

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
void NO_INLINE Exception(M68K* z, unsigned which, unsigned vecnum)
{
 const uint32_t PC_save = z->PC;
 const uint16_t SR_save = GetSR(z);

 SetSR(z, (GetSR(z) & ~0x2000) | (1 << 13));
 SetSR(z, (GetSR(z) & ~0x8000));

 if(which == EXCEPTION_INT)
 {
  unsigned evn;

  z->timestamp += 4;

  SetSR(z, (GetSR(z) & ~0x0700) | ((z->IPL & 0x7) << 8));

  evn = z->BusIntAck(z->IPL);

  if(evn > 255)
   vecnum = vecnum + z->IPL;
  else
   vecnum = evn;

  z->timestamp += 2;
 }

 Push_u32(z, PC_save);
 Push_u16(z, SR_save);

 if(MDFN_UNLIKELY(which == EXCEPTION_BUS_ERROR || which == EXCEPTION_ADDRESS_ERROR))
 {
  Push_u16(z, 0); // TODO: Instruction register
  Push_u32(z, 0); // TODO: Access address
  Push_u16(z, 0); // TODO: R/W, I/N, function code
 }

 z->PC = Read_u32(z, vecnum << 2);

 // TODO: Prefetch
 ReadOp(z);
 ReadOp(z);
 z->PC -= 4;
}

//
//
//

//
// TAS
//
MDFN_FASTCALL uint8_t TAS_Callback(M68K* zptr, uint8_t data)
{
 CalcZN_u8(zptr, data);
 zptr->Flag_C = false;
 zptr->Flag_V = false;

 data |= 0x80;
 return data;
}

void NO_INLINE M68K_Run(M68K* z, int32_t run_until_time)
{
 while(MDFN_LIKELY(z->timestamp < run_until_time))
 {
	 if(MDFN_UNLIKELY(z->XPending))
	 {
		 if(MDFN_LIKELY(!(z->XPending & (XPENDING_MASK_ERRORHALTED | XPENDING_MASK_DTACKHALTED | XPENDING_MASK_EXTHALTED))))
		 {
			 if(MDFN_UNLIKELY(z->XPending & (XPENDING_MASK_RESET | XPENDING_MASK_ADDRESS | XPENDING_MASK_BUS)))
			 {
				 if(z->XPending & XPENDING_MASK_RESET)
				 {
					 SetSR(z, (GetSR(z) & ~0x2000) | (1 << 13));
					 SetSR(z, (GetSR(z) & ~0x8000));
					 SetSR(z, (GetSR(z) & ~0x0700) | (0x7 << 8));

					 z->A[7] = Read_u32(z, VECNUM_RESET_SSP << 2);
					 z->PC = Read_u32(z, VECNUM_RESET_PC << 2);
					 //
					 z->XPending &= ~XPENDING_MASK_RESET;
				 }
				 else
				 {
					 if(z->XPending & XPENDING_MASK_BUS)
						 Exception(z, EXCEPTION_BUS_ERROR, VECNUM_BUS_ERROR);
					 else
						 Exception(z, EXCEPTION_ADDRESS_ERROR, VECNUM_ADDRESS_ERROR);
					 // Clear bus/address error bits in z->XPending only after Exception(z, z) returns normally:
					 z->XPending &= ~(XPENDING_MASK_BUS | XPENDING_MASK_ADDRESS);
				 }

				 return;
			 }
			 else if(z->XPending & (XPENDING_MASK_INT | XPENDING_MASK_NMI))
			 {
				 assert(z->IPL == 0x7 || z->IPL > ((GetSR(z) >> 8) & 0x7));
				 z->XPending &= ~(XPENDING_MASK_STOPPED | XPENDING_MASK_INT | XPENDING_MASK_NMI);

				 Exception(z, EXCEPTION_INT, VECNUM_INT_BASE);

				 return;
			 }
		 }

		 // STOP and ExtHalted fallthrough:
		 z->timestamp += 4;
		 return;
	 }
	 //
	 //
	 //
	 uint16_t instr = ReadOp(z);
	 const unsigned instr_b11_b9 = (instr >> 9) & 0x7;
	 const unsigned instr_b2_b0 = instr & 0x7;
#ifdef M68K_SPLIT_SWITCH
	 if(instr & 0x8000)
	  M68K_RunSplit1(z, instr & 0x7fff, instr_b11_b9, instr_b2_b0);
	 else
	  M68K_RunSplit0(z, instr, instr_b11_b9, instr_b2_b0);
#else
	 switch(instr)
	 {
		 default: ILLEGAL(z, instr); break;
#include "m68k_instr.inc"
	 }
#endif
 }
}
