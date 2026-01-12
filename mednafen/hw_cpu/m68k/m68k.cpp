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

#include <mednafen/mednafen.h>
#include "m68k.h"

#include <tuple>

#pragma GCC optimize ("no-crossjumping,no-gcse")

#include "m68k_private.h"

static MDFN_FASTCALL void Dummy_BusRESET(bool state) { }

M68K::M68K(const bool rev_e) : Revision_E(rev_e),
	       BusReadInstr(nullptr), BusRead8(nullptr), BusRead16(nullptr),
	       BusWrite8(nullptr), BusWrite16(nullptr),
	       BusRMW(nullptr),
	       BusIntAck(nullptr),
	       BusRESET(Dummy_BusRESET)
{
 timestamp = 0;
 XPending = 0;
 IPL = 0;
 Reset(true);
}

M68K::~M68K()
{

}

void M68K::StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(DA),
  SFVAR(PC),
  SFVAR(SRHB),
  SFVAR(IPL),

  SFVAR(Flag_Z),
  SFVAR(Flag_N),
  SFVAR(Flag_X),
  SFVAR(Flag_C),
  SFVAR(Flag_V),

  SFVAR(SP_Inactive),

  SFVAR(XPending),

  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, sname, false);

 if(load)
  XPending &= XPENDING_MASK__VALID;
}

void M68K::SetIPL(uint8 ipl_new)
{
 if(IPL < 0x7 && ipl_new == 0x7)
  XPending |= XPENDING_MASK_NMI;
 else if(ipl_new < 0x7)
  XPending &= ~XPENDING_MASK_NMI;

 IPL = ipl_new;
 RecalcInt();
}

void M68K::SetExtHalted(bool state)
{
 XPending &= ~XPENDING_MASK_EXTHALTED;
 if(state)
  XPending |= XPENDING_MASK_EXTHALTED;
}

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
 const uint32 PC_save = PC;
 const uint16 SR_save = GetSR();

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

 Push<uint32>(PC_save);
 Push<uint16>(SR_save);

 if(MDFN_UNLIKELY(which == EXCEPTION_BUS_ERROR || which == EXCEPTION_ADDRESS_ERROR))
 {
  Push<uint16>(0); // TODO: Instruction register
  Push<uint32>(0); // TODO: Access address
  Push<uint16>(0); // TODO: R/W, I/N, function code
 }

 PC = Read<uint32>(vecnum << 2);

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
MDFN_FASTCALL uint8 TAS_Callback(M68K* zptr, uint8 data)
{
 zptr->CalcZN<uint8>(data);
 zptr->Flag_C = false;
 zptr->Flag_V = false;

 data |= 0x80;
 return data;
}

void NO_INLINE M68K::Run(int32 run_until_time)
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

					 A[7] = Read<uint32>(VECNUM_RESET_SSP << 2);
					 PC = Read<uint32>(VECNUM_RESET_PC << 2);
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
	 uint16 instr = ReadOp();
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
//
//
uint32 M68K::GetRegister(unsigned which, char* special, const uint32 special_len)
{
 switch(which)
 {
  default:
	return 0xDEADBEEF;

  case GSREG_D0: case GSREG_D1: case GSREG_D2: case GSREG_D3:
  case GSREG_D4: case GSREG_D5: case GSREG_D6: case GSREG_D7:
	return D[which - GSREG_D0];

  case GSREG_A0: case GSREG_A1: case GSREG_A2: case GSREG_A3:
  case GSREG_A4: case GSREG_A5: case GSREG_A6: case GSREG_A7:
	return A[which - GSREG_A0];

  case GSREG_PC:
	return PC;

  case GSREG_SR:
	return GetSR();

  case GSREG_SSP:
	if(GetSVisor())
	 return A[7];
	else
	 return SP_Inactive;

  case GSREG_USP:
	if(!GetSVisor())
	 return A[7];
	else
	 return SP_Inactive;
 }
}

void M68K::SetRegister(unsigned which, uint32 value)
{
 switch(which)
 {
  case GSREG_D0: case GSREG_D1: case GSREG_D2: case GSREG_D3:
  case GSREG_D4: case GSREG_D5: case GSREG_D6: case GSREG_D7:
	D[which - GSREG_D0] = value;
	break;

  case GSREG_A0: case GSREG_A1: case GSREG_A2: case GSREG_A3:
  case GSREG_A4: case GSREG_A5: case GSREG_A6: case GSREG_A7:
	A[which - GSREG_A0] = value;
	break;

  case GSREG_PC:
	PC = value;
	break;

  case GSREG_SR:
	SetSR(value);
	break;

  case GSREG_SSP:
	if(GetSVisor())
	 A[7] = value;
	else
	 SP_Inactive = value;
	break;

  case GSREG_USP:
	if(!GetSVisor())
	 A[7] = value;
	else
	 SP_Inactive = value;
	break;
 }
}
