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

#include "../../mednafen.h"

/* M68K_BUS_INT_ACK_AUTO -- BusIntAck callback can return this to
 * tell M68K to use automatic interrupt-acknowledge vectoring (auto-
 * vector mode) instead of supplying an explicit vector number.
 * File-scope so consumers can spell it without the `M68K::` class-
 * scope qualifier (needed once sound_glue.cpp becomes sound_glue.c
 * -- C has no class-scope qualifier syntax).  Value matches the
 * former class-scoped `M68K::BUS_INT_ACK_AUTO` exactly. */
enum { M68K_BUS_INT_ACK_AUTO = -1 };

/* C-compat typedef: in C the struct tag is not auto-aliased to a
 * type name, so the bare `M68K*` spellings used in the data-
 * member BusRMW function-pointer signature (inside this struct)
 * and in the M68K_* free-function declarations (after this struct)
 * fail to parse from a C TU.  Forward-declare the typedef up
 * front; same pattern scsp.h uses for SS_SCSP_Slot / SS_SCSP_Timer
 * / SS_SCSP / etc. */
typedef struct M68K M68K;

/* Phase-9d-7: enums that previously lived inside struct M68K hoisted
 * to file scope.  Anonymous enums inside a struct generate a
 * "declaration does not declare anything" warning under -Wall in
 * both C and C++; the values were only ever reachable as M68K::X
 * from C++ TUs (the anonymous-enum-in-struct pattern doesn't
 * inject names at file scope in C, so C consumers couldn't see
 * them at all).  File-scope placement makes them visible to both
 * C and C++ consumers under their plain names and silences the
 * 5 advisory warnings sound_glue.c was reporting against m68k.h.
 *
 * The named `enum AddressMode` also moves out; it's used as a
 * template parameter type by the HAM struct and the op templates,
 * which now reference plain `AddressMode` instead of
 * `AddressMode`.  In C++ a regular (non-class) enum
 * introduces its members into the enclosing scope, so
 * bare-name references inside class methods (e.g. `case
 * PC_DISP:` inside HAM's switch on `am`) continue to work
 * via file-scope lookup. */

enum  /* XPENDING_MASK -- bits of M68K::XPending */
{
 XPENDING_MASK_INT          = 0x0001,
 XPENDING_MASK_NMI          = 0x0002,
 XPENDING_MASK_RESET        = 0x0010,
 XPENDING_MASK_ADDRESS      = 0x0020,
 XPENDING_MASK_BUS          = 0x0040,
 XPENDING_MASK_STOPPED      = 0x0100, /* via STOP instruction */

 XPENDING_MASK_ERRORHALTED  = 0x0400, /* address/bus error during address/bus error handling */

 XPENDING_MASK_DTACKHALTED  = 0x0800,
 XPENDING_MASK_EXTHALTED    = 0x1000,

 /* For save-state sanitising: */
 XPENDING_MASK__VALID = XPENDING_MASK_INT | XPENDING_MASK_NMI | XPENDING_MASK_RESET | XPENDING_MASK_ADDRESS | XPENDING_MASK_BUS | XPENDING_MASK_STOPPED | XPENDING_MASK_ERRORHALTED | XPENDING_MASK_DTACKHALTED | XPENDING_MASK_EXTHALTED
};

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

enum  /* VECNUM -- vector numbers for Exception() */
{
 VECNUM_RESET_SSP     = 0,
 VECNUM_RESET_PC      = 1,
 VECNUM_BUS_ERROR     = 2,
 VECNUM_ADDRESS_ERROR = 3,
 VECNUM_ILLEGAL       = 4,
 VECNUM_ZERO_DIVIDE   = 5,
 VECNUM_CHK           = 6,
 VECNUM_TRAPV         = 7,
 VECNUM_PRIVILEGE     = 8,
 VECNUM_TRACE         = 9,
 VECNUM_LINEA         = 10,
 VECNUM_LINEF         = 11,

 VECNUM_UNINI_INT     = 15,

 VECNUM_SPURIOUS_INT  = 24,
 VECNUM_INT_BASE      = 24,

 VECNUM_TRAP_BASE     = 32
};

enum  /* EXCEPTION class -- first arg to Exception() */
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

enum  /* GSREG -- id parameter of M68K_GetRegister / M68K_SetRegister */
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

/* Phase-9c: class -> struct.  See Phase-9a comment in scsp.h
 * for rationale.  M68K already had `//private:` (commented out)
 * markers, so all members were de facto public; this commit
 * simply formalizes the access. */
struct M68K
{

#ifdef __cplusplus
 /* C++-only: class methods reachable on this struct.  C
  * consumers see this header as a plain data struct (same
  * layout, same member offsets).  All bodies live in
  * m68k.cpp / m68k_instr.inc / m68k_instr_split{0,1}.cpp;
  * C consumers reach them via the `extern "C"` M68K_* free
  * functions declared at the bottom of this header. */

 /* Phase-9: M68K::M68K(rev_e) and M68K::~M68K() retired.  Zero
  * callers after sound_glue.cpp -> sound_glue.c switched to
  * M68K_Construct.  M68K is pure-data now; instances are
  * zero-initialised at file scope and finalised with an
  * explicit M68K_Construct(&inst, rev_e) call. */

 void Run(int32_t run_until_time);

 /* Phase-9d-4: Reset retired from the class.  Body moved inline into the
  * M68K_Reset extern "C" wrapper in m68k.cpp.  Both prior callers
  * (M68K_Construct in m68k.cpp and the M68K_Reset wrapper) now reach the
  * body the same way the BusRESET callback path always did: through
  * M68K_Reset(z, ...).  No class-method dispatch step in between. */

 /* Phase-9d-1: SetIPL and SetExtHalted retired from the class.
  * Bodies moved inline into the M68K_SetIPL / M68K_SetExtHalted
  * extern "C" wrappers in m68k.cpp -- they were already 1-line
  * forwarders to z->SetIPL(...) / z->SetExtHalted(...) and the
  * 8 / 4 line bodies don't need the dispatch round-trip. */


 /* Phase-9d-5: SignalDTACKHalted and SignalAddressError retired from
  * the class.  Bodies moved inline into the M68K_SignalDTACKHalted /
  * M68K_SignalAddressError extern "C" wrappers in m68k.cpp.  These
  * are called from external bus read/write handlers, followed by a
  * longjmp() back to above Run(), so an extra dispatch round-trip
  * through a class method was pure cost. */

 /* Phase-9d-3: StateAction retired -- body moved to the
  * M68K_StateAction extern "C" wrapper in m68k.cpp (parallel to
  * Phase-9d-1's SetIPL/SetExtHalted treatment).  Field-name strings
  * in the savestate SFORMAT array intentionally keep their pre-9d
  * spellings so on-disk savestate compat is preserved. */

#endif /* __cplusplus */

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

 /* Phase-9d-7: XPENDING_MASK_* enum hoisted to file scope above
  * struct M68K. */

 /* Set by M68K_Construct / M68K::M68K from the `rev_e` parameter
  * and never written again.  Was `const bool` -- contractual
  * single-init via the ctor's member-initializer list.  Dropped
  * the const so the free-function M68K_Construct can assign to
  * it (C-style construction has no member-initializer-list
  * syntax).  Set-once-at-construction is now preserved by
  * convention, not by compiler-enforced const-correctness. */
 bool Revision_E;

#ifdef __cplusplus
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

#endif /* __cplusplus */

 /* Phase-9d-7: enum AddressMode hoisted to file scope above
  * struct M68K.  Bare-name references (DATA_REG_DIR etc.)
  * inside class methods continue to resolve via file-scope
  * lookup; `AddressMode` is used as a template parameter type
  * (was `AddressMode`) by HAM and the op templates. */

#ifdef __cplusplus
 //
 // MOVE byte and word: instructions, 2 cycle penalty for source predecrement only
 //  	2 cycle penalty for (d8, An, Xn) for both source and dest ams
 //  	2 cycle penalty for (d8, PC, Xn) for dest am
 //

 //
 // Careful on declaration order of HAM objects(needs to be source then dest).
 //
 template<typename T, AddressMode am>
 struct HAM;

 bool GetC(void);
 bool GetV(void);
 bool GetZ(void);
 bool GetN(void);
 bool GetX(void);

 void SetCX(bool val);

 /* Phase-9d-6: CalcZN<T, Z_OnlyClear> template retired.  The six
  * named width-typed variants below remain as class methods; the
  * dispatch that the template was doing now lives in
  * m68k_private.h's CALC_ZN(z, T, val) and CALC_ZN_CLEAR(z, T, val)
  * macros which expand to a `sizeof(T)`-keyed if/else-if chain
  * selecting the right named call.  gcc -O2 constant-folds the
  * sizeof check so each compile-time-known T at the call site
  * collapses to the right single call (byte-equivalent to the
  * prior template instantiation).
  *
  * The named methods are still T-typed (uint8_t / uint16_t /
  * uint32_t) by their parameter so the existing template-op
  * callers don't need to cast values. */
 void CalcZN_u8       (const uint8_t  val);
 void CalcZN_u8_clear (const uint8_t  val);
 void CalcZN_u16      (const uint16_t val);
 void CalcZN_u16_clear(const uint16_t val);
 void CalcZN_u32      (const uint32_t val);
 void CalcZN_u32_clear(const uint32_t val);

 uint8_t GetCCR(void);
 void SetCCR(uint8_t val);
 uint16_t GetSR(void);
 void SetSR(uint16_t val);

 bool GetSVisor(void);
#endif /* __cplusplus */

 //
 /* Phase-9d-7: VECNUM_* and EXCEPTION_* enums hoisted to file
  * scope above struct M68K.  Used by the Exception() method
  * (declared just below in the __cplusplus block) and by every
  * op-body that raises an exception. */

#ifdef __cplusplus
 void NO_INLINE Exception(unsigned which, unsigned vecnum);

 template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
 void ADD(HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 template<typename T, AddressMode SAM, AddressMode DAM>
 void ADDX(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 /* Phase-8e: Subtract's `bool X_form` template parameter moved to
  * a runtime first-arg.  T, DT, SAM, DAM stay HAM-locked. */
 template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
 DT Subtract(bool X_form, HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
 void SUB(HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
 void SUBX(HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 /* Phase-9d-10: NEG/NEGX retired from struct M68K scope; now
  * free templates below struct M68K, in the same block as EXT
  * (added in Phase-9d-9). */

 template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
 void CMP(HAM<T, SAM> &src, HAM<DT, DAM> &dst);

 template<typename T, AddressMode SAM, AddressMode DAM>
 void CHK(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 /* Phase-9d-12: OR, EOR, AND retired from struct M68K scope; see
  * the free-template block after struct M68K closes. */

 void ORI_CCR(void);
 void ORI_SR(void);
 void ANDI_CCR(void);
 void ANDI_SR(void);
 void EORI_CCR(void);
 void EORI_SR(void);

 template<typename T, AddressMode SAM>
 void MULU(HAM<T, SAM> &src, const unsigned dr);

 template<typename T, AddressMode SAM>
 void MULS(HAM<T, SAM> &src, const unsigned dr);

 /* Phase-8b: Divide<sdiv> retired -- two callers (DIVU, DIVS) each
  * with a concrete `sdiv` value, no T-parametric dispatch needed. */
 void Divide_u(uint16_t divisor, const unsigned dr);
 void Divide_s(uint16_t divisor, const unsigned dr);

 template<typename T, AddressMode SAM>
 void DIVU(HAM<T, SAM> &src, const unsigned dr);

 template<typename T, AddressMode SAM>
 void DIVS(HAM<T, SAM> &src, const unsigned dr);

 template<typename T, AddressMode SAM, AddressMode DAM>
 void ABCD(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 uint8_t DecimalSubtractX(const uint8_t src_data, const uint8_t dst_data);

 template<typename T, AddressMode SAM, AddressMode DAM>
 void SBCD(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 template<typename T, AddressMode DAM>
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

 /* Phase-9d-12: BTST, BCHG, BCLR, BSET retired from struct M68K
  * scope; see the free-template block after struct M68K closes. */

 template<typename T, AddressMode SAM, AddressMode DAM>
 void MOVE(HAM<T, SAM> &src, HAM<T, DAM> &dst);

 /* Phase-9d-11: MOVEA retired from struct M68K scope; see the
  * free-template block after struct M68K closes. */

 /* Phase-8e: MOVEM_to_MEM's `bool pseudo_predec` moved to runtime. */
 template<typename T, AddressMode DAM>
 void MOVEM_to_MEM(bool pseudo_predec, const uint16_t reglist, HAM<T, DAM> &dst);

 /* Phase-8e: MOVEM_to_REGS's `bool pseudo_postinc` moved to runtime. */
 template<typename T, AddressMode SAM>
 void MOVEM_to_REGS(bool pseudo_postinc, HAM<T, SAM> &src, const uint16_t reglist);

 /* Phase-8e: ShiftBase's `bool Arithmetic, bool ShiftLeft` moved
  * to runtime first-args.  The four ASL/ASR/LSL/LSR wrappers
  * (kept as templates because they take HAM<T, TAM>&) now pass
  * the booleans by runtime constants -- gcc -O2 inlines them
  * and constprops the bools, so callsites generate identical
  * instruction streams to the previous template-instantiation
  * form. */
 template<typename T, AddressMode TAM>
 void ShiftBase(bool Arithmetic, bool ShiftLeft, HAM<T, TAM> &targ, unsigned count);

 template<typename T, AddressMode TAM>
 void ASL(HAM<T, TAM> &targ, unsigned count);

 template<typename T, AddressMode TAM>
 void ASR(HAM<T, TAM> &targ, unsigned count);

 template<typename T, AddressMode TAM>
 void LSL(HAM<T, TAM> &targ, unsigned count);

 template<typename T, AddressMode TAM>
 void LSR(HAM<T, TAM> &targ, unsigned count);

 /* Phase-8e: RotateBase's `bool X_Form, bool ShiftLeft` moved
  * to runtime first-args.  Same shape as ShiftBase. */
 template<typename T, AddressMode TAM>
 void RotateBase(bool X_Form, bool ShiftLeft, HAM<T, TAM> &targ, unsigned count);

 template<typename T, AddressMode TAM>
 void ROL(HAM<T, TAM> &targ, unsigned count);

 template<typename T, AddressMode TAM>
 void ROR(HAM<T, TAM> &targ, unsigned count);

 template<typename T, AddressMode TAM>
 void ROXL(HAM<T, TAM> &targ, unsigned count);

 template<typename T, AddressMode TAM>
 void ROXR(HAM<T, TAM> &targ, unsigned count);

 /* Phase-9d-9 + Phase-9d-10: EXT, TAS, TST, CLR, NOT retired from
  * struct M68K scope.  All five are now free templates living after
  * struct M68K's closing brace; see the declaration block down
  * there. */

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

 /* Phase-9d-11: Scc, JSR, JMP, MOVE_from_SR, MOVE_to_CCR,
  * MOVE_to_SR retired from struct M68K scope; see the
  * free-template block after struct M68K closes. */

 /* Phase-8d: MOVE_USP bool template parameter -> runtime arg.
  * Just two callers per build (the two MOVE-USP-direction
  * combinations privileged supervisor mode permits); the body
  * is a 3-line ternary that doesn't benefit from compile-time
  * folding the bool.  Cleaner as a runtime first-arg. */
 void MOVE_USP(bool direction, const unsigned ar);

 /* Phase-9d-11: LEA, PEA retired from struct M68K scope; see
  * the free-template block after struct M68K closes. */
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
#endif /* __cplusplus */
 //
 //
 //
 //
 //
 // These externally-provided functions should add >= 4 to M68K::timestamp per call:

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
 /* Phase-9d-7: GSREG_* enum hoisted to file scope above struct M68K.
  * Consumers (sh7095.inc, vdp2.c via sh-side dispatch) pass these
  * values into M68K_GetRegister/M68K_SetRegister.  File-scope
  * placement lets C callers refer to them directly. */

 /* Phase-9d-3: GetRegister / SetRegister retired -- bodies moved to
  * the M68K_GetRegister / M68K_SetRegister extern "C" wrappers in
  * m68k.cpp.  The whole `#ifdef __cplusplus` sub-block here used to
  * hold those two declarations alone; it's gone now. */
};

/* Phase-9d-9 + Phase-9d-10: Free-template op declarations live at
 * file scope, outside struct M68K, taking an explicit M68K* `z`
 * first parameter.  Bodies are in m68k_private.h; the
 * m68k_instr*.inc call sites pass `this` as the M68K pointer (the
 * .inc files are #included inside M68K::Run() / M68K::RunSplit{0,1}()
 * where `this` is in scope).  The HAM<T,AM>& parameter type still
 * references M68K::HAM (HAM detempleting is a later commit). */
#ifdef __cplusplus
template<typename T, AddressMode DAM>
void EXT(M68K* z, M68K::HAM<T, DAM> &dst);

template<typename DT, AddressMode DAM>
void NEG(M68K* z, M68K::HAM<DT, DAM> &dst);

template<typename DT, AddressMode DAM>
void NEGX(M68K* z, M68K::HAM<DT, DAM> &dst);

template<typename T, AddressMode DAM>
void NOT(M68K* z, M68K::HAM<T, DAM> &dst);

template<typename T, AddressMode DAM>
void CLR(M68K* z, M68K::HAM<T, DAM> &dst);

template<typename T, AddressMode DAM>
void TST(M68K* z, M68K::HAM<T, DAM> &dst);

template<typename T, AddressMode DAM>
void TAS(M68K* z, M68K::HAM<T, DAM> &dst);

/* Phase-9d-11: 9-op single-arg-with-extra family. */
template<typename T, AddressMode SAM>
void MOVEA(M68K* z, M68K::HAM<T, SAM> &src, const unsigned ar);

template<typename T, AddressMode DAM>
void Scc(M68K* z, unsigned cc, M68K::HAM<T, DAM> &dst);

template<typename T, AddressMode TAM>
void JSR(M68K* z, M68K::HAM<T, TAM> &targ);

template<typename T, AddressMode TAM>
void JMP(M68K* z, M68K::HAM<T, TAM> &targ);

template<typename T, AddressMode DAM>
void MOVE_from_SR(M68K* z, M68K::HAM<T, DAM> &dst);

template<typename T, AddressMode SAM>
void MOVE_to_CCR(M68K* z, M68K::HAM<T, SAM> &src);

template<typename T, AddressMode SAM>
void MOVE_to_SR(M68K* z, M68K::HAM<T, SAM> &src);

template<typename T, AddressMode SAM>
void LEA(M68K* z, M68K::HAM<T, SAM> &src, const unsigned ar);

template<typename T, AddressMode SAM>
void PEA(M68K* z, M68K::HAM<T, SAM> &src);

/* Phase-9d-12: bitwise (AND/OR/EOR) + bit-test (BTST/BCHG/BCLR/BSET). */
template<typename T, AddressMode SAM, AddressMode DAM>
void AND(M68K* z, M68K::HAM<T, SAM> &src, M68K::HAM<T, DAM> &dst);

template<typename T, AddressMode SAM, AddressMode DAM>
void OR(M68K* z, M68K::HAM<T, SAM> &src, M68K::HAM<T, DAM> &dst);

template<typename T, AddressMode SAM, AddressMode DAM>
void EOR(M68K* z, M68K::HAM<T, SAM> &src, M68K::HAM<T, DAM> &dst);

template<typename T, AddressMode TAM>
void BTST(M68K* z, M68K::HAM<T, TAM> &targ, unsigned wb);

template<typename T, AddressMode TAM>
void BCHG(M68K* z, M68K::HAM<T, TAM> &targ, unsigned wb);

template<typename T, AddressMode TAM>
void BCLR(M68K* z, M68K::HAM<T, TAM> &targ, unsigned wb);

template<typename T, AddressMode TAM>
void BSET(M68K* z, M68K::HAM<T, TAM> &targ, unsigned wb);
#endif /* __cplusplus */

/* M68K_* free-function API exposed to consumers of m68k.h.
 *
 * All declarations live inside an `extern "C" { ... }` block (gated
 * by __cplusplus so plain C consumers can include this header
 * directly) -- the matching definitions in m68k.cpp also use
 * `extern "C"` linkage.  This makes the wrappers callable from
 * both C++ and C TUs, with one well-defined ABI symbol per name.
 *
 * Trade-off vs the previous `static FORCE_INLINE` header-side
 * definitions:  we lose call-site inlining of the thunk body
 * (each wrapper became a real function call to a 1-2 instruction
 * out-of-line body in m68k.cpp), but gain a C-callable surface
 * that sound_glue.cpp -> sound_glue.c needs.  None of these
 * wrappers are on the M68K::Run inner loop -- they're called
 * from external orchestration code (IRQ change, savestate,
 * reset, scheduler step, debugger register read/write) -- so
 * the per-call function-call overhead is negligible in profile
 * terms.  Phase-9 step 3's original comment about codegen
 * folding under -O2 stops applying here; cross-TU inlining is
 * now LTO-dependent.
 */
#ifdef __cplusplus
extern "C" {
#endif

void     M68K_Construct          (M68K* z, bool rev_e) MDFN_COLD;

void     M68K_SetIPL             (M68K* z, uint8_t ipl_new);
void     M68K_SignalDTACKHalted  (M68K* z, uint32_t addr);
void     M68K_SignalAddressError (M68K* z, uint32_t addr, uint8_t type);

void     M68K_Reset              (M68K* z, bool pwr) MDFN_COLD;
void     M68K_Run                (M68K* z, int32_t until);
void     M68K_SetExtHalted       (M68K* z, bool state);
void     M68K_StateAction        (M68K* z, StateMem* sm, const unsigned load,
                                  const bool data_only, const char* sname);
uint32_t M68K_GetRegister        (M68K* z, const unsigned id, char* const special,
                                  const uint32_t special_len);
void     M68K_SetRegister        (M68K* z, const unsigned id, const uint32_t value);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
