/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* m68k.h - Motorola 68000 CPU Emulator
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

#ifndef __MDFN_M68K_H
#define __MDFN_M68K_H

#include <mednafen/mednafen.h>

/* Phase-9c: class -> struct.  See Phase-9a comment in scsp.h
 * for rationale.  M68K already had `//private:` (commented out)
 * markers, so all members were de facto public; this commit
 * simply formalizes the access. */
struct M68K
{

 M68K(const bool rev_e = false) MDFN_COLD;
 ~M68K() MDFN_COLD;

 void Run(int32_t run_until_time);

 void Reset(bool powering_up) MDFN_COLD;

 void SetIPL(uint8_t ipl_new);
 void SetExtHalted(bool state);


 //
 // SignalDTACKHalted() and SignalAddressError() should be called from the external
 // bus read/write handlers as appropriate, followed by a longjmp() to above
 // Run().
 //
 INLINE void SignalDTACKHalted(uint32_t addr)
 {
  XPending |= XPENDING_MASK_DTACKHALTED;
 }

 INLINE void SignalAddressError(uint32_t addr, uint8_t type)
 {
  if(XPending & (XPENDING_MASK_ADDRESS | XPENDING_MASK_BUS | XPENDING_MASK_RESET))
  {
   XPending |= XPENDING_MASK_ERRORHALTED;
  }

  XPending |= XPENDING_MASK_ADDRESS;
 }

 void StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname);

 //
 //
 //
 //
 //
 //
 //
 //
 union
 {
  uint32_t DA[16];
  struct
  {
   uint32_t D[8];
   uint32_t A[8];
  };
 };
 int32_t timestamp;

 uint32_t PC;
 uint8_t SRHB;
 uint8_t IPL;

 bool Flag_Z, Flag_N;
 bool Flag_X, Flag_C, Flag_V;

 uint32_t SP_Inactive;
 uint32_t XPending;
 enum
 {
  XPENDING_MASK_INT 	= 0x0001,
  XPENDING_MASK_NMI	= 0x0002,
  XPENDING_MASK_RESET	= 0x0010,
  XPENDING_MASK_ADDRESS = 0x0020,
  XPENDING_MASK_BUS	= 0x0040,
  XPENDING_MASK_STOPPED	= 0x0100,	// via STOP instruction

  XPENDING_MASK_ERRORHALTED = 0x0400,	// address/bus error during address/bus error handling.

  XPENDING_MASK_DTACKHALTED = 0x0800,
  XPENDING_MASK_EXTHALTED   = 0x1000,

  // For save state sanitizing:
  XPENDING_MASK__VALID = XPENDING_MASK_INT | XPENDING_MASK_NMI | XPENDING_MASK_RESET | XPENDING_MASK_ADDRESS | XPENDING_MASK_BUS | XPENDING_MASK_STOPPED | XPENDING_MASK_ERRORHALTED | XPENDING_MASK_DTACKHALTED | XPENDING_MASK_EXTHALTED
 };

 const bool Revision_E;

 //private:
 void RecalcInt(void);

 /* Phase-8a: M68K's width-template family.  Read<T> / Write<T,
  * long_dec> stay as 1-line dispatchers for the 2 T-parametric
  * call sites still in HAM<T, AM>::Read / ::Write (and Read<T>
  * also in MOVEM_to_REGS' body).  Their bodies forward to the
  * named uX variants below.  Both retire when HAM detemplates.
  *
  * Push<T> / Pull<T> are gone -- every caller used a concrete
  * uint16_t / uint32_t at the call site, so the 4 named variants
  * below replace them outright.
  */
 uint8_t  Read_u8(uint32_t addr);
 uint16_t Read_u16(uint32_t addr);
 uint32_t Read_u32(uint32_t addr);

 void     Write_u8(uint32_t addr, const uint8_t val);
 void     Write_u16(uint32_t addr, const uint16_t val);
 /* For 32-bit writes the M68K has two bus-ordering modes:
  *   default -- high half first   (BusWrite16(addr, hi); BusWrite16(addr+2, lo))
  *   _longdec -- low half first   (BusWrite16(addr+2, lo); BusWrite16(addr, hi))
  * The longdec variant is what Push_u32 uses, mirroring the old
  * `Write<uint32_t, true>` template instantiation. */
 void     Write_u32(uint32_t addr, const uint32_t val);
 void     Write_u32_longdec(uint32_t addr, const uint32_t val);

 void     Push_u16(const uint16_t value);
 void     Push_u32(const uint32_t value);
 uint16_t Pull_u16(void);
 uint32_t Pull_u32(void);

 uint16_t ReadOp(void);

#ifdef M68K_SPLIT_SWITCH
 void RunSplit0(uint16_t instr, const unsigned instr_b11_b9, const unsigned instr_b2_b0);
 void RunSplit1(uint16_t instr, const unsigned instr_b11_b9, const unsigned instr_b2_b0);
#endif

 enum AddressMode
 {
  DATA_REG_DIR,
  ADDR_REG_DIR,

  ADDR_REG_INDIR,
  ADDR_REG_INDIR_POST,
  ADDR_REG_INDIR_PRE,

  ADDR_REG_INDIR_DISP,

  ADDR_REG_INDIR_INDX,

  ABS_SHORT,
  ABS_LONG,

  PC_DISP,
  PC_INDEX,

  IMMEDIATE
 };

 //
 // MOVE byte and word: instructions, 2 cycle penalty for source predecrement only
 //  	2 cycle penalty for (d8, An, Xn) for both source and dest ams
 //  	2 cycle penalty for (d8, PC, Xn) for dest am
 //

 //
 // Careful on declaration order of HAM objects(needs to be source then dest).
 //
 template<typename T, M68K::AddressMode am>
 struct HAM;

 bool GetC(void);
 bool GetV(void);
 bool GetZ(void);
 bool GetN(void);
 bool GetX(void);

 void SetCX(bool val);

 /* Phase-8b: named width-typed CalcZN variants.  The CalcZN<T,
  * Z_OnlyClear> template below is kept as a thin dispatcher for
  * the 15+ T-parametric call sites inside ADD/ADDX/SUB/SUBX/etc.
  * templates that still take a T parameter; concrete-type call
  * sites use the named methods directly.  The dispatcher retires
  * when those parent templates also detemplate (much later
  * phases -- they're all locked behind HAM detempleting). */
 void CalcZN_u8       (const uint8_t  val);
 void CalcZN_u8_clear (const uint8_t  val);
 void CalcZN_u16      (const uint16_t val);
 void CalcZN_u16_clear(const uint16_t val);
 void CalcZN_u32      (const uint32_t val);
 void CalcZN_u32_clear(const uint32_t val);

 template<typename T, bool Z_OnlyClear = false>
 void CalcZN(const T val);

 uint8_t GetCCR(void);
 void SetCCR(uint8_t val);
 uint16_t GetSR(void);
 void SetSR(uint16_t val);

 bool GetSVisor(void);

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

 void NO_INLINE Exception(unsigned which, unsigned vecnum);

 template<typename T, typename DT, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void ADD(HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 template<typename T, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void ADDX(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 /* Phase-8e: Subtract's `bool X_form` template parameter moved to
  * a runtime first-arg.  T, DT, SAM, DAM stay HAM-locked. */
 template<typename T, typename DT, M68K::AddressMode SAM, M68K::AddressMode DAM>
 DT Subtract(bool X_form, HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 template<typename T, typename DT, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void SUB(HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 template<typename T, typename DT, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void SUBX(HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 template<typename DT, M68K::AddressMode DAM>
 void NEG(HAM<DT, DAM> &dst);

 template<typename DT, M68K::AddressMode DAM>
 void NEGX(HAM<DT, DAM> &dst);

 template<typename T, typename DT, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void CMP(HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 template<typename T, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void CHK(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void OR(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void EOR(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void AND(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 void ORI_CCR(void);
 void ORI_SR(void);
 void ANDI_CCR(void);
 void ANDI_SR(void);
 void EORI_CCR(void);
 void EORI_SR(void);

 template<typename T, M68K::AddressMode SAM>
 void MULU(HAM<T, SAM> &src, const unsigned dr);

 template<typename T, M68K::AddressMode SAM>
 void MULS(HAM<T, SAM> &src, const unsigned dr);

 /* Phase-8b: Divide<sdiv> retired -- two callers (DIVU, DIVS) each
  * with a concrete `sdiv` value, no T-parametric dispatch needed. */
 void Divide_u(uint16_t divisor, const unsigned dr);
 void Divide_s(uint16_t divisor, const unsigned dr);

 template<typename T, M68K::AddressMode SAM>
 void DIVU(HAM<T, SAM> &src, const unsigned dr);

 template<typename T, M68K::AddressMode SAM>
 void DIVS(HAM<T, SAM> &src, const unsigned dr);

 template<typename T, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void ABCD(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 uint8_t DecimalSubtractX(const uint8_t src_data, const uint8_t dst_data);

 template<typename T, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void SBCD(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode DAM>
 void NBCD(HAM<T, DAM> &dst);

 /* Phase-8d: MOVEP<T, reg_to_mem> retired.  Only 4 instantiations
  * (T = uint16_t / uint32_t cross-product with reg_to_mem = false /
  * true), and the body's `if(reg_to_mem)` branch and the loop's
  * sizeof(T) are both compile-time folded by the template form;
  * the 4 named methods below carry the post-folding bodies
  * directly. */
 void MOVEP_w_mem_to_reg(const unsigned ar, const unsigned dr);
 void MOVEP_l_mem_to_reg(const unsigned ar, const unsigned dr);
 void MOVEP_w_reg_to_mem(const unsigned ar, const unsigned dr);
 void MOVEP_l_reg_to_mem(const unsigned ar, const unsigned dr);

 template<typename T, M68K::AddressMode TAM>
 void BTST(HAM<T, TAM> &targ, unsigned wb);

 template<typename T, M68K::AddressMode TAM>
 void BCHG(HAM<T, TAM> &targ, unsigned wb);

 template<typename T, M68K::AddressMode TAM>
 void BCLR(HAM<T, TAM> &targ, unsigned wb);

 template<typename T, M68K::AddressMode TAM>
 void BSET(HAM<T, TAM> &targ, unsigned wb);

 template<typename T, M68K::AddressMode SAM, M68K::AddressMode DAM>
 void MOVE(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode SAM>
 void MOVEA(HAM<T, SAM> &src, const unsigned ar);

 /* Phase-8e: MOVEM_to_MEM's `bool pseudo_predec` moved to runtime. */
 template<typename T, M68K::AddressMode DAM>
 void MOVEM_to_MEM(bool pseudo_predec, const uint16_t reglist, HAM<T, DAM> &dst);

 /* Phase-8e: MOVEM_to_REGS's `bool pseudo_postinc` moved to runtime. */
 template<typename T, M68K::AddressMode SAM>
 void MOVEM_to_REGS(bool pseudo_postinc, HAM<T, SAM> &src, const uint16_t reglist);

 /* Phase-8e: ShiftBase's `bool Arithmetic, bool ShiftLeft` moved
  * to runtime first-args.  The four ASL/ASR/LSL/LSR wrappers
  * (kept as templates because they take HAM<T, TAM>&) now pass
  * the booleans by runtime constants -- gcc -O2 inlines them
  * and constprops the bools, so callsites generate identical
  * instruction streams to the previous template-instantiation
  * form. */
 template<typename T, M68K::AddressMode TAM>
 void ShiftBase(bool Arithmetic, bool ShiftLeft, HAM<T, TAM> &targ, unsigned count);

 template<typename T, M68K::AddressMode TAM>
 void ASL(HAM<T, TAM> &targ, unsigned count);

 template<typename T, M68K::AddressMode TAM>
 void ASR(HAM<T, TAM> &targ, unsigned count);

 template<typename T, M68K::AddressMode TAM>
 void LSL(HAM<T, TAM> &targ, unsigned count);

 template<typename T, M68K::AddressMode TAM>
 void LSR(HAM<T, TAM> &targ, unsigned count);

 /* Phase-8e: RotateBase's `bool X_Form, bool ShiftLeft` moved
  * to runtime first-args.  Same shape as ShiftBase. */
 template<typename T, M68K::AddressMode TAM>
 void RotateBase(bool X_Form, bool ShiftLeft, HAM<T, TAM> &targ, unsigned count);

 template<typename T, M68K::AddressMode TAM>
 void ROL(HAM<T, TAM> &targ, unsigned count);

 template<typename T, M68K::AddressMode TAM>
 void ROR(HAM<T, TAM> &targ, unsigned count);

 template<typename T, M68K::AddressMode TAM>
 void ROXL(HAM<T, TAM> &targ, unsigned count);

 template<typename T, M68K::AddressMode TAM>
 void ROXR(HAM<T, TAM> &targ, unsigned count);

 template<typename T, M68K::AddressMode DAM>
 void TAS(HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode DAM>
 void TST(HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode DAM>
 void CLR(HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode DAM>
 void NOT(HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode DAM>
 void EXT(HAM<T, DAM> &dst);

 void SWAP(const unsigned dr);

 void EXG(uint32_t* a, uint32_t* b);

 /* Phase-8c: BCC condition-code family detempleted.
  *
  * TestCond / Bxx / DBcc lose their `unsigned cc` template
  * parameter entirely -- cc moves from a template-time constant
  * into a runtime first-argument.  Their bodies' switch(cc) used
  * to fold to a single arm per instantiation; with cc as a
  * runtime value gcc still emits the switch as a jump table or
  * branch tree depending on density, and the per-callsite cost
  * is the same single conditional that the old per-instantiation
  * folded body produced.
  *
  * Scc keeps its T and DAM template parameters (still HAM-locked)
  * but `cc` moves to a runtime argument too -- one fewer template
  * dimension, cleaner instr.inc call sites. */
 bool TestCond(unsigned cc);
 void Bxx(unsigned cc, uint32_t disp);
 void DBcc(unsigned cc, const unsigned dr);

 template<typename T, M68K::AddressMode DAM>
 void Scc(unsigned cc, HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode TAM>
 void JSR(HAM<T, TAM> &targ);

 template<typename T, M68K::AddressMode TAM>
 void JMP(HAM<T, TAM> &targ);

 template <typename T, M68K::AddressMode DAM>
 void MOVE_from_SR(HAM<T, DAM> &dst);

 template<typename T, M68K::AddressMode SAM>
 void MOVE_to_CCR(HAM<T, SAM> &src);

 template<typename T, M68K::AddressMode SAM>
 void MOVE_to_SR(HAM<T, SAM> &src);

 /* Phase-8d: MOVE_USP bool template parameter -> runtime arg.
  * Just two callers per build (the two MOVE-USP-direction
  * combinations privileged supervisor mode permits); the body
  * is a 3-line ternary that doesn't benefit from compile-time
  * folding the bool.  Cleaner as a runtime first-arg. */
 void MOVE_USP(bool direction, const unsigned ar);

 template<typename T, M68K::AddressMode SAM>
 void LEA(HAM<T, SAM> &src, const unsigned ar);

 template<typename T, M68K::AddressMode SAM>
 void PEA(HAM<T, SAM> &src);
 void UNLK(const unsigned ar);
 void LINK(const unsigned ar);
 void RTE(void);
 void RTR(void);
 void RTS(void);
 void TRAP(const unsigned vf);
 void TRAPV(void);
 void ILLEGAL(const uint16_t instr);
 void LINEA(void);
 void LINEF(void);
 void NOP(void);
 void RESET(void);
 void STOP(void);

 bool CheckPrivilege(void);
 //
 //
 //
 //
 //
 // These externally-provided functions should add >= 4 to M68K::timestamp per call:
 enum { BUS_INT_ACK_AUTO = -1 };

 uint16_t (MDFN_FASTCALL *BusReadInstr)(uint32_t A);
 uint8_t (MDFN_FASTCALL *BusRead8)(uint32_t A);
 uint16_t (MDFN_FASTCALL *BusRead16)(uint32_t A);
 void (MDFN_FASTCALL *BusWrite8)(uint32_t A, uint8_t V);
 void (MDFN_FASTCALL *BusWrite16)(uint32_t A, uint16_t V);
 //
 //
 void (MDFN_FASTCALL *BusRMW)(uint32_t A, uint8_t (MDFN_FASTCALL *cb)(M68K*, uint8_t));
 unsigned (MDFN_FASTCALL *BusIntAck)(uint8_t level);
 void (MDFN_FASTCALL *BusRESET)(bool state);	// Optional; Calling Reset(false) from this callback *is* permitted.

 //
 //
 //
 //
 //
 //
 /* Phase-9c: access modifier dropped. */
 enum
 {
  GSREG_D0 = 0,
  GSREG_D1,
  GSREG_D2,
  GSREG_D3,
  GSREG_D4,
  GSREG_D5,
  GSREG_D6,
  GSREG_D7,

  GSREG_A0 = 8,
  GSREG_A1,
  GSREG_A2,
  GSREG_A3,
  GSREG_A4,
  GSREG_A5,
  GSREG_A6,
  GSREG_A7,

  GSREG_PC = 16,
  GSREG_SR,
  GSREG_SSP,
  GSREG_USP
 };

 uint32_t GetRegister(unsigned which, char* special = nullptr, const uint32_t special_len = 0);
 void SetRegister(unsigned which, uint32_t value);
};

/* Phase-9 step 3: free-function wrappers around M68K members used by
 * sound_glue.cpp.  Pure inline forwarders; codegen folds to direct
 * member access under -O2.  Member function bodies remain in
 * m68k_private.h / m68k.cpp for now and will be converted to true
 * free functions in a later phase (gated on retirement of the
 * HAM cascade). */
static INLINE void M68K_SetIPL             (M68K* z, uint8_t ipl_new)             { z->SetIPL(ipl_new); }
static INLINE void M68K_SignalDTACKHalted  (M68K* z, uint32_t addr)                { z->SignalDTACKHalted(addr); }
static INLINE void M68K_SignalAddressError (M68K* z, uint32_t addr, uint8_t type)  { z->SignalAddressError(addr, type); }

static INLINE void M68K_Reset              (M68K* z, bool pwr)                     { z->Reset(pwr); }
static INLINE void M68K_Run                (M68K* z, int32_t until)                { z->Run(until); }
static INLINE void M68K_SetExtHalted       (M68K* z, bool state)                   { z->SetExtHalted(state); }
static INLINE void M68K_StateAction        (M68K* z, StateMem* sm, const unsigned load,
                                            const bool data_only, const char* sname)
 { z->StateAction(sm, load, data_only, sname); }
static INLINE uint32_t M68K_GetRegister    (M68K* z, const unsigned id, char* const special, const uint32_t special_len)
 { return z->GetRegister(id, special, special_len); }
static INLINE void M68K_SetRegister        (M68K* z, const unsigned id, const uint32_t value)
 { z->SetRegister(id, value); }


#endif
