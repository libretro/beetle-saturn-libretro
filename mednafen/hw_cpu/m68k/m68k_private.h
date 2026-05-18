#ifndef __MDFN_M68K_PRIVATE_H
#define __MDFN_M68K_PRIVATE_H

#include "../../mednafen.h"
#include "m68k.h"

INLINE void RecalcInt(M68K* z)
{
 z->XPending &= ~XPENDING_MASK_INT;

 if(z->IPL > (z->SRHB & 0x7))
  z->XPending |= XPENDING_MASK_INT;
}

/* Phase-8a: named width-typed Read methods.  The Read<T> template
 * below is kept as a thin dispatcher for the two T-parametric
 * call sites that remain in m68k_private.h after phase 8e:
 *   - HAM<T, AM>::Read   (the addressing-mode helper)
 *   - MOVEM_to_REGS body (T from its template param)
 * Both retire when the HAM cascade lands. */
INLINE uint8_t Read_u8(M68K* z, uint32_t addr)
{
 return z->BusRead8(addr);
}

INLINE uint16_t Read_u16(M68K* z, uint32_t addr)
{
 return z->BusRead16(addr);
}

INLINE uint32_t Read_u32(M68K* z, uint32_t addr)
{
 uint32_t ret;

 ret  = z->BusRead16(addr) << 16;
 ret |= z->BusRead16(addr + 2);

 return ret;
}

INLINE uint16_t ReadOp(M68K* z)
{
 uint16_t ret;

 ret = z->BusReadInstr(z->PC);
 z->PC += 2;

 return ret;
}

INLINE void Write_u8(M68K* z, uint32_t addr, const uint8_t val)
{
 z->BusWrite8(addr, val);
}

INLINE void Write_u16(M68K* z, uint32_t addr, const uint16_t val)
{
 z->BusWrite16(addr, val);
}

INLINE void Write_u32(M68K* z, uint32_t addr, const uint32_t val)
{
 z->BusWrite16(addr,     val >> 16);
 z->BusWrite16(addr + 2, val);
}

INLINE void Write_u32_longdec(M68K* z, uint32_t addr, const uint32_t val)
{
 z->BusWrite16(addr + 2, val);
 z->BusWrite16(addr,     val >> 16);
}

INLINE void Push_u16(M68K* z, const uint16_t value)
{
 z->A[7] -= 2;
 Write_u16(z, z->A[7], value);
}

INLINE void Push_u32(M68K* z, const uint32_t value)
{
 z->A[7] -= 4;
 Write_u32_longdec(z, z->A[7], value);
}

INLINE uint16_t Pull_u16(M68K* z)
{
 uint16_t ret = Read_u16(z, z->A[7]);
 z->A[7] += 2;
 return ret;
}

INLINE uint32_t Pull_u32(M68K* z)
{
 uint32_t ret = Read_u32(z, z->A[7]);
 z->A[7] += 4;
 return ret;
}

INLINE bool GetC(M68K* z) { return z->Flag_C; }
INLINE bool GetV(M68K* z) { return z->Flag_V; }
INLINE bool GetZ(M68K* z) { return z->Flag_Z; }
INLINE bool GetN(M68K* z) { return z->Flag_N; }
INLINE bool GetX(M68K* z) { return z->Flag_X; }

INLINE void SetCX(M68K* z, bool val)
{
 z->Flag_C = (val);
 z->Flag_X = (val);
}

//
// Z_OnlyClear should be true for ADDX, SUBX, NEGX, ABCD, SBCD, NBCD.
//
// History: Phase-8b broke a single template<T, Z_OnlyClear> CalcZN
// body into the six named methods below (three widths, two Z
// behaviours) and kept the template as a thin dispatcher.
// CALC_ZN(z, T, val) / CALC_ZN_CLEAR(z, T, val) macros further
// below do the size-keyed dispatch at the call site.  The named
// methods are the live API now and stay class members of M68K.
//

INLINE void CalcZN_u8(M68K* z, const uint8_t val)
{
 z->Flag_Z = (val == 0);
 z->Flag_N = ((int8_t)val < 0);
}

INLINE void CalcZN_u8_clear(M68K* z, const uint8_t val)
{
 if(val != 0)
  z->Flag_Z = false;
 z->Flag_N = ((int8_t)val < 0);
}

INLINE void CalcZN_u16(M68K* z, const uint16_t val)
{
 z->Flag_Z = (val == 0);
 z->Flag_N = ((int16_t)val < 0);
}

INLINE void CalcZN_u16_clear(M68K* z, const uint16_t val)
{
 if(val != 0)
  z->Flag_Z = false;
 z->Flag_N = ((int16_t)val < 0);
}

INLINE void CalcZN_u32(M68K* z, const uint32_t val)
{
 z->Flag_Z = (val == 0);
 z->Flag_N = ((int32_t)val < 0);
}

INLINE void CalcZN_u32_clear(M68K* z, const uint32_t val)
{
 if(val != 0)
  z->Flag_Z = false;
 z->Flag_N = ((int32_t)val < 0);
}

#define CALC_ZN(z, T, val) \
 do { \
  if      (sizeof(T) == 4) CalcZN_u32((z), (val)); \
  else if (sizeof(T) == 2) CalcZN_u16((z), (val)); \
  else                     CalcZN_u8((z), (val)); \
 } while(0)

#define CALC_ZN_CLEAR(z, T, val) \
 do { \
  if      (sizeof(T) == 4) CalcZN_u32_clear((z), (val)); \
  else if (sizeof(T) == 2) CalcZN_u16_clear((z), (val)); \
  else                     CalcZN_u8_clear((z), (val)); \
 } while(0)

INLINE uint8_t GetCCR(M68K* z)
{
 return (GetC(z) << 0) | (GetV(z) << 1) | (GetZ(z) << 2) | (GetN(z) << 3) | (GetX(z) << 4);
}

INLINE void SetCCR(M68K* z, uint8_t val)
{
 z->Flag_C   = ((val >> 0) & 1);
 z->Flag_V   = ((val >> 1) & 1);
 z->Flag_Z   = ((val >> 2) & 1);
 z->Flag_N   = ((val >> 3) & 1);
 z->Flag_X   = ((val >> 4) & 1);
}

INLINE uint16_t GetSR(M68K* z)
{
 return GetCCR(z) | (z->SRHB << 8);
}

INLINE void SetSR(M68K* z, uint16_t val)
{
 const uint8_t new_srhb = (val >> 8) & 0xA7;

 z->Flag_C   = ((val >> 0) & 1);
 z->Flag_V   = ((val >> 1) & 1);
 z->Flag_Z   = ((val >> 2) & 1);
 z->Flag_N   = ((val >> 3) & 1);
 z->Flag_X   = ((val >> 4) & 1);

 if((z->SRHB ^ new_srhb) & 0x20)	// Supervisor mode change
 {
  uint32_t tmp     = z->A[7];
  z->A[7]          = z->SP_Inactive;
  z->SP_Inactive   = tmp;
 }

 z->SRHB = new_srhb;
 RecalcInt(z);
}

INLINE bool GetSVisor(M68K* z)
{
 return (bool)(GetSR(z) & 0x2000);
}

//
//
//

//
// ADD
//

//
// ADDX
//

//
// Used to implement SUB, SUBA, SUBX, NEG, NEGX
//
// first-arg.  Three places used the template-time X_form:
//   1. static_assert -- dropped (the dispatch table is the
//      real safety net; SUBX-style m,m never gets generated
//      with X_form=false, and SUB-style register modes are
//      already constrained by the dispatch).
//   2. (X_form ? GetX() : 0) -- works identically at runtime.
//   3. CalcZN<DT, X_form>(result) -- replaced with a runtime
//      branch selecting between CalcZN<DT, true> (the
//      Z-only-clears form) and CalcZN<DT, false> (the
//      full-Z form).
//

//
// SUB
//

//
// SUBX
//

//
// NEG
//

//
// NEGX
//

//
// CMP
//

//
// CHK
//
// Exception on dst < 0 || dst > src

//
// OR
//
// class scope.  Standard pattern (Flag_X -> z->Flag_X, timestamp
// -> z->timestamp, CALC_ZN(this,...) -> CALC_ZN(z,...)); BTST/BCHG
// /BCLR/BSET below follow the same transform with only Flag_Z.
//

//
// EOR
//

//
// AND
//

//
// ORI CCR
//
INLINE void ORI_CCR(M68K* z)
{
 const uint8_t imm = ReadOp(z);

 SetCCR(z, GetCCR(z) | imm);

 //
 //
 z->timestamp += 8;
 ReadOp(z);
 z->PC -= 2;
}

//
// ORI SR
//
INLINE void ORI_SR(M68K* z)
{
 const uint16_t imm = ReadOp(z);

 SetSR(z, GetSR(z) | imm);

 //
 //
 z->timestamp += 8;
 ReadOp(z);
 z->PC -= 2;
}

//
// ANDI CCR
//
INLINE void ANDI_CCR(M68K* z)
{
 const uint8_t imm = ReadOp(z);

 SetCCR(z, GetCCR(z) & imm);

 //
 //
 z->timestamp += 8;
 ReadOp(z);
 z->PC -= 2;
}

//
// ANDI SR
//
INLINE void ANDI_SR(M68K* z)
{
 const uint16_t imm = ReadOp(z);

 SetSR(z, GetSR(z) & imm);

 //
 //
 z->timestamp += 8;
 ReadOp(z);
 z->PC -= 2;
}

//
// EORI CCR
//
INLINE void EORI_CCR(M68K* z)
{
 const uint8_t imm = ReadOp(z);

 SetCCR(z, GetCCR(z) ^ imm);

 //
 //
 z->timestamp += 8;
 ReadOp(z);
 z->PC -= 2;
}

//
// EORI SR
//
INLINE void EORI_SR(M68K* z)
{
 const uint16_t imm = ReadOp(z);

 SetSR(z, GetSR(z) ^ imm);

 //
 //
 z->timestamp += 8;
 ReadOp(z);
 z->PC -= 2;
}

//
// MULU
//

//
// MULS
//

INLINE void Divide_u(M68K* z, uint16_t divisor, const unsigned dr)
{
 uint32_t dividend = z->D[dr];
 uint32_t tmp;
 bool oflow = false;
 int i;

 if(!divisor)
 {
  Exception(z, EXCEPTION_ZERO_DIVIDE, VECNUM_ZERO_DIVIDE);
  return;
 }

 tmp = dividend;

 for(i = 0; i < 16; i++)
 {
  bool lb = false;
  bool ob;

  if(tmp >= ((uint32_t)divisor << 15))
  {
   tmp -= divisor << 15;
   lb = true;
  }

  ob = tmp >> 31;
  tmp = (tmp << 1) | lb;

  if(ob)
   oflow = true;
 }

 if((uint32_t)(tmp >> 16) >= divisor)
  oflow = true;

 /* Doesn't affect X flag */
 CalcZN_u16(z, (uint16_t)tmp);
 z->Flag_C = false;
 z->Flag_V = oflow;

 if(!oflow)
  z->D[dr] = tmp;
}

INLINE void Divide_s(M68K* z, uint16_t divisor, const unsigned dr)
{
 uint32_t dividend = z->D[dr];
 uint32_t tmp;
 bool neg_quotient = false;
 bool neg_remainder = false;
 bool oflow = false;
 int i;

 if(!divisor)
 {
  Exception(z, EXCEPTION_ZERO_DIVIDE, VECNUM_ZERO_DIVIDE);
  return;
 }

 neg_quotient = (dividend >> 31) ^ (divisor >> 15);
 if(dividend & 0x80000000)
 {
  dividend = -dividend;
  neg_remainder = true;
 }

 if(divisor & 0x8000)
  divisor = -divisor;

 tmp = dividend;

 for(i = 0; i < 16; i++)
 {
  bool lb = false;
  bool ob;

  if(tmp >= ((uint32_t)divisor << 15))
  {
   tmp -= divisor << 15;
   lb = true;
  }

  ob = tmp >> 31;
  tmp = (tmp << 1) | lb;

  if(ob)
   oflow = true;
 }

 if((tmp & 0xFFFF) > (uint32_t)(0x7FFF + neg_quotient))
  oflow = true;

 if((uint32_t)(tmp >> 16) >= divisor)
  oflow = true;

 if(!oflow)
 {
  if(neg_quotient)
   tmp = ((-tmp) & 0xFFFF) | (tmp & 0xFFFF0000);

  if(neg_remainder)
   tmp = (((-(tmp >> 16)) << 16) & 0xFFFF0000) | (tmp & 0xFFFF);
 }

 /* Doesn't affect X flag */
 CalcZN_u16(z, (uint16_t)tmp);
 z->Flag_C = false;
 z->Flag_V = oflow;

 if(!oflow)
  z->D[dr] = tmp;
}

//
// DIVU
//

//
// DIVS
//

//
// ABCD
//

INLINE uint8_t DecimalSubtractX(M68K* z, const uint8_t src_data, const uint8_t dst_data)
{
 bool V = false;
 uint32_t tmp;

 tmp = dst_data - src_data - GetX(z);

 const bool adj0 = ((dst_data ^ src_data ^ tmp) & 0x10);
 const bool adj1 = (tmp & 0x100);

 if(adj0)
 {
  uint8_t prev_tmp = tmp;
  tmp -= 0x06;
  V |= (prev_tmp & 0x80) & (~tmp & 0x80);
 }

 if(adj1)
 {
  uint8_t prev_tmp = tmp;
  tmp -= 0x60;
  V |= (prev_tmp & 0x80) & (~tmp & 0x80);
 }

 z->Flag_V = V;
 CalcZN_u8_clear(z, tmp);
 SetCX(z, (bool)(tmp >> 8));

 return tmp;
}

//
// SBCD
//

//
// NBCD
//

//
// MOVEP
//
// Four monomorphized bodies (W/L x reg-to-mem/mem-to-reg) -- 2- or
// 4-iteration loops over the EA/shift schedule.  reg_to_mem picks
// between byte-out-to-bus and byte-in-from-bus.
//

/* MOVEP.W (Dn -> mem):  upper-half-of-Dn[15:8], Dn[7:0]
 * written into (An+disp), (An+disp+2). */
INLINE void MOVEP_w_reg_to_mem(M68K* z, const unsigned ar, const unsigned dr)
{
 const int16_t ext = ReadOp(z);
 uint32_t ea = z->A[ar] + (int16_t)ext;
 unsigned shift = 8; /* (sizeof(uint16_t) - 1) << 3 */

 Write_u8(z, ea, z->D[dr] >> shift);
 ea += 2;
 shift -= 8;
 Write_u8(z, ea, z->D[dr] >> shift);
}

/* MOVEP.W (mem -> Dn):  two bytes from (An+disp), (An+disp+2)
 * packed back into Dn[15:0]. */
INLINE void MOVEP_w_mem_to_reg(M68K* z, const unsigned ar, const unsigned dr)
{
 const int16_t ext = ReadOp(z);
 uint32_t ea = z->A[ar] + (int16_t)ext;
 unsigned shift = 8;

 z->D[dr] &= ~(0xFF << shift);
 z->D[dr] |= Read_u8(z, ea) << shift;
 ea += 2;
 shift -= 8;
 z->D[dr] &= ~(0xFF << shift);
 z->D[dr] |= Read_u8(z, ea) << shift;
}

/* MOVEP.L (Dn -> mem):  four bytes from Dn[31:24..7:0] written
 * into (An+disp), (An+disp+2), (An+disp+4), (An+disp+6). */
INLINE void MOVEP_l_reg_to_mem(M68K* z, const unsigned ar, const unsigned dr)
{
 const int16_t ext = ReadOp(z);
 uint32_t ea = z->A[ar] + (int16_t)ext;
 unsigned shift = 24; /* (sizeof(uint32_t) - 1) << 3 */
 unsigned i;

 for(i = 0; i < 4; i++)
 {
  Write_u8(z, ea, z->D[dr] >> shift);
  ea += 2;
  shift -= 8;
 }
}

/* MOVEP.L (mem -> Dn):  four bytes packed back into Dn[31:0]. */
INLINE void MOVEP_l_mem_to_reg(M68K* z, const unsigned ar, const unsigned dr)
{
 const int16_t ext = ReadOp(z);
 uint32_t ea = z->A[ar] + (int16_t)ext;
 unsigned shift = 24;
 unsigned i;

 for(i = 0; i < 4; i++)
 {
  z->D[dr] &= ~(0xFF << shift);
  z->D[dr] |= Read_u8(z, ea) << shift;
  ea += 2;
  shift -= 8;
 }
}

//
// MOVE
//

//
// MOVEM to memory
//
// The `Write<T, pseudo_predec>` call inside the loop expanded
// to one of the named width-typed writes (Write_u8, Write_u16,
// Write_u32 / Write_u32_longdec).  We inline that dispatch
// directly so we don't need to go through the Write<T, bool>
// dispatcher template (whose long_dec parameter would still
// need to be compile-time).  T stays a template parameter so
// the sizeof(T) ladder folds.
//

//
// MOVEM to regs(from memory)
//

//
// Phase-8e: ShiftBase's `bool Arithmetic, bool ShiftLeft` template
// parameters moved to runtime first-args.  The 4 ASL/ASR/LSL/LSR
// wrappers pass them as concrete `true`/`false` literals, so gcc
// -O2 still constprops the bools at every callsite -- the
// instruction stream emitted for each wrapper is identical to
// what the previous 4-instantiation template form produced
// (verified by per-TU `size` diff: zero text delta on the
// m68k_split0 TU which holds the bulk of the shift/rotate
// dispatch).
//

//
// Phase-8e: RotateBase's `bool X_Form, bool ShiftLeft` template
// parameters moved to runtime first-args.  Same shape as ShiftBase.
//

//
//
//

MDFN_FASTCALL uint8_t TAS_Callback(M68K* zptr, uint8_t data);

//
//
//

// templates parallel to NEG/NEGX above (and EXT in bee65cf).  Each takes
// an explicit M68K* z; member access patterns (`Flag_X`, `timestamp`)
// become `z->Flag_X`/`z->timestamp`; CALC_ZN(this, ...) -> CALC_ZN(z, ...).
// TAS has no member access in its body, so z is unused there (kept in
// the signature for consistency with the rest of the op family).
//

//
// TST
//

//
// CLR
//

//
// NOT
//

//
// EXT
//

//
// SWAP
//
INLINE void SWAP(M68K* z, const unsigned dr)
{
 z->D[dr] = (z->D[dr] << 16) | (z->D[dr] >> 16);

 CalcZN_u32(z, z->D[dr]);
 z->Flag_C = false;
 z->Flag_V = false;
}

//
// EXG (doesn't affect flags)
//
INLINE void EXG(M68K* z, uint32_t* a, uint32_t* b)
{
 uint32_t tmp;
 z->timestamp += 2;

 tmp = *a;
 *a  = *b;
 *b  = tmp;
}

//
//
//

INLINE bool TestCond(M68K* z, unsigned cc)
{
 switch(cc)
 {
  case 0x00:	// TRUE
	return true;

  case 0x01:	// FALSE
	return false;

  case 0x02:	// HI
	return !GetC(z) && !GetZ(z);

  case 0x03:	// LS
	return GetC(z) || GetZ(z);

  case 0x04:	// CC/HS
	return !GetC(z);

  case 0x05:	// CS/LO
	return GetC(z);

  case 0x06:	// NE
	return !GetZ(z);

  case 0x07:	// EQ
	return GetZ(z);

  case 0x08:	// VC
	return !GetV(z);

  case 0x09:	// VS
	return GetV(z);

  case 0x0A:	// PL
	return !GetN(z);

  case 0x0B:	// MI
	return GetN(z);

  case 0x0C:	// GE
	return GetN(z) == GetV(z);

  case 0x0D:	// LT
	return GetN(z) != GetV(z);

  case 0x0E:	// GT
	return GetN(z) == GetV(z) && !GetZ(z);

  case 0x0F:	// LE
	return GetN(z) != GetV(z) || GetZ(z);
 }
 return false; /* unreachable, but keeps -Wreturn-type happy now
                * that cc is no longer a static-assert'd template arg */
}

//
// Bcc, BRA, BSR
//
//  (caller of this function should sign-extend the 8-bit displacement)
//
INLINE void Bxx(M68K* z, unsigned cc, uint32_t disp)
{
 const uint32_t BPC = z->PC;

 /* cc == 0x01 here means BSR (Branch to Subroutine), not "Bcc-False"
  * -- override to TRUE so the branch is always taken. */
 if(TestCond(z, (cc == 0x01) ? 0x00 : cc))
 {
  const uint16_t disp16 = (int16_t)ReadOp(z);

  if(!disp)
   disp = (int16_t)disp16;
  else
   z->PC -= 2;

  if(cc == 0x01)
   Push_u32(z, z->PC);

  z->timestamp += 2;
  z->PC = BPC + disp;
 }
 else
 {
  if(!disp)
   ReadOp(z);

  z->timestamp += 4;
 }
}

INLINE void DBcc(M68K* z, unsigned cc, const unsigned dr)
{
 const uint32_t BPC = z->PC;
 uint32_t disp;

 disp = (int16_t)ReadOp(z);

 if(!TestCond(z, cc))
 {
  const uint16_t result = z->D[dr] - 1;

  z->timestamp += 2;
  z->D[dr] = (z->D[dr] & 0xFFFF0000) | result;

  if(result != 0xFFFF)
   z->PC = BPC + disp;
  else
   z->timestamp += 4;
 }
 else
  z->timestamp += 4;
}

//
// Scc
//

//
// JSR
//

//
// JMP
//

//
// MOVE from SR
//

//
// MOVE to CCR
//

//
// MOVE to SR
//

//
// MOVE to/from USP
//
INLINE void MOVE_USP(M68K* z, bool direction, const unsigned ar)
{
 if(!direction)
  z->SP_Inactive = z->A[ar];
 else
  z->A[ar] = z->SP_Inactive;
}

//
// LEA
//

//
// PEA
//

//
// UNLK
//
INLINE void UNLK(M68K* z, const unsigned ar)
{
 z->A[7] = z->A[ar];
 z->A[ar] = Pull_u32(z);
}

//
// LINK
//
INLINE void LINK(M68K* z, const unsigned ar)
{
 const uint32_t disp = (int16_t)ReadOp(z);

 Push_u32(z, z->A[ar]);
 z->A[ar] = z->A[7];
 z->A[7] += disp;
}

//
// RTE
//
INLINE void RTE(M68K* z)
{
 uint16_t new_SR;

 new_SR = Pull_u16(z);
 z->PC = Pull_u32(z);

 SetSR(z, new_SR);
}

//
// RTR
//
INLINE void RTR(M68K* z)
{
 SetCCR(z, Pull_u16(z));
 z->PC = Pull_u32(z);
}

//
// RTS
//
INLINE void RTS(M68K* z)
{
 z->PC = Pull_u32(z);
}

//
// TRAP
//
INLINE void TRAP(M68K* z, const unsigned vf)
{
 Exception(z, EXCEPTION_TRAP, VECNUM_TRAP_BASE + vf);
}

//
// TRAPV
//
INLINE void TRAPV(M68K* z)
{
 if(GetV(z))
  Exception(z, EXCEPTION_TRAPV, VECNUM_TRAPV);
}

//
// ILLEGAL
//
INLINE void ILLEGAL(M68K* z, const uint16_t instr)
{
 z->PC -= 2;
 Exception(z, EXCEPTION_ILLEGAL, VECNUM_ILLEGAL);
}

INLINE void LINEA(M68K* z)
{
 z->PC -= 2;
 Exception(z, EXCEPTION_ILLEGAL, VECNUM_LINEA);
}

INLINE void LINEF(M68K* z)
{
 z->PC -= 2;
 Exception(z, EXCEPTION_ILLEGAL, VECNUM_LINEF);
}

//
// NOP
//
INLINE void NOP(M68K* z)
{

}

//
// RESET
//
INLINE void RESET(M68K* z)
{
 z->timestamp += 2;
 //
 z->BusRESET(true);
 z->timestamp += 124;
 z->BusRESET(false);
 //
 z->timestamp += 2;
}

//
// STOP
//
INLINE void STOP(M68K* z)
{
 uint16_t new_SR = ReadOp(z);

 SetSR(z, new_SR);
 z->XPending |= XPENDING_MASK_STOPPED;
}

INLINE bool CheckPrivilege(M68K* z)
{
 if(MDFN_UNLIKELY(!GetSVisor(z)))
 {
  z->PC -= 2;
  Exception(z, EXCEPTION_PRIVILEGE, VECNUM_PRIVILEGE);
  return false;
 }

 return true;
}

#include "m68k_ham_instances.inc.h"

/* Phase-9d-15b: op detempleting via the same preprocessor approach.
 * The 50 op templates in m68k_private.h are auto-converted (via
 * tools/gen_op_macros.py) to macro-parameterised body headers, one
 * per op.  m68k_op_instances.inc.h instantiates them for the 1534
 * (op, HAM-arg) combinations used by m68k_instr*.inc PLUS the
 * helper-template (Subtract / ShiftBase / RotateBase) instantiations
 * reachable from those ops.  Nothing references the macro instances
 * yet -- they coexist with the C++ templates and get dead-code-
 * eliminated since no caller exists.  9d-15c switches over the
 * .inc files and removes the templates. */
#include "m68k_op_instances.inc.h"

#endif
