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
 * vector mode) instead of supplying an explicit vector number. */
enum { M68K_BUS_INT_ACK_AUTO = -1 };

/* C-compat typedef: in C the struct tag is not auto-aliased to a
 * type name, so the bare `M68K*` spellings used in the data-
 * member BusRMW function-pointer signature (inside this struct)
 * and in the M68K_* free-function declarations (after this struct)
 * fail to parse from a C TU.  Forward-declare the typedef up
 * front; same pattern scsp.h uses for SS_SCSP_Slot / SS_SCSP_Timer
 * / SS_SCSP / etc. */
typedef struct M68K M68K;

enum  /* XPENDING_MASK -- bits of M68K XPending */
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

/* Phase-9c: class -> struct.  See Phase-9a comment in scsp.h
 * for rationale.  M68K already had `//private:` (commented out)
 * markers, so all members were de facto public; this commit
 * simply formalizes the access. */
struct M68K
{

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

 /* Set by M68K_Construct / M68K M68K from the `rev_e` parameter
  * and never written again.  Was `const bool` -- contractual
  * single-init via the ctor's member-initializer list.  Dropped
  * the const so the free-function M68K_Construct can assign to
  * it (C-style construction has no member-initializer-list
  * syntax).  Set-once-at-construction is now preserved by
  * convention, not by compiler-enforced const-correctness. */
 bool Revision_E;

 //

 //
 //
 //
 //
 //
 // These externally-provided functions should add >= 4 to M68K timestamp per call:

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

 };

/* Free-function op declarations taking an explicit M68K* `z` first
 * parameter.  Bodies are in m68k_private.h; the m68k_instr*.inc call
 * sites use these via the macro-monomorphized HAM and op families. */

void RecalcInt(M68K* z);
uint8_t Read_u8(M68K* z, uint32_t addr);
uint16_t Read_u16(M68K* z, uint32_t addr);
uint32_t Read_u32(M68K* z, uint32_t addr);
void Write_u8(M68K* z, uint32_t addr, const uint8_t val);
void Write_u16(M68K* z, uint32_t addr, const uint16_t val);
void Write_u32(M68K* z, uint32_t addr, const uint32_t val);
void Write_u32_longdec(M68K* z, uint32_t addr, const uint32_t val);
void Push_u16(M68K* z, const uint16_t value);
void Push_u32(M68K* z, const uint32_t value);
uint16_t Pull_u16(M68K* z);
uint32_t Pull_u32(M68K* z);
uint16_t ReadOp(M68K* z);
bool GetC(M68K* z);
bool GetV(M68K* z);
bool GetZ(M68K* z);
bool GetN(M68K* z);
bool GetX(M68K* z);
void SetCX(M68K* z, bool val);
void CalcZN_u8(M68K* z, const uint8_t  val);
void CalcZN_u8_clear(M68K* z, const uint8_t  val);
void CalcZN_u16(M68K* z, const uint16_t val);
void CalcZN_u16_clear(M68K* z, const uint16_t val);
void CalcZN_u32(M68K* z, const uint32_t val);
void CalcZN_u32_clear(M68K* z, const uint32_t val);
uint8_t GetCCR(M68K* z);
void SetCCR(M68K* z, uint8_t val);
uint16_t GetSR(M68K* z);
void SetSR(M68K* z, uint16_t val);
bool GetSVisor(M68K* z);
void NO_INLINE Exception(M68K* z, unsigned which, unsigned vecnum);
void ORI_CCR(M68K* z);
void ORI_SR(M68K* z);
void ANDI_CCR(M68K* z);
void ANDI_SR(M68K* z);
void EORI_CCR(M68K* z);
void EORI_SR(M68K* z);
void Divide_u(M68K* z, uint16_t divisor, const unsigned dr);
void Divide_s(M68K* z, uint16_t divisor, const unsigned dr);
uint8_t DecimalSubtractX(M68K* z, const uint8_t src_data, const uint8_t dst_data);
void MOVEP_w_mem_to_reg(M68K* z, const unsigned ar, const unsigned dr);
void MOVEP_l_mem_to_reg(M68K* z, const unsigned ar, const unsigned dr);
void MOVEP_w_reg_to_mem(M68K* z, const unsigned ar, const unsigned dr);
void MOVEP_l_reg_to_mem(M68K* z, const unsigned ar, const unsigned dr);
void SWAP(M68K* z, const unsigned dr);
void EXG(M68K* z, uint32_t* a, uint32_t* b);
bool TestCond(M68K* z, unsigned cc);
void Bxx(M68K* z, unsigned cc, uint32_t disp);
void DBcc(M68K* z, unsigned cc, const unsigned dr);
void MOVE_USP(M68K* z, bool direction, const unsigned ar);
void UNLK(M68K* z, const unsigned ar);
void LINK(M68K* z, const unsigned ar);
void RTE(M68K* z);
void RTR(M68K* z);
void RTS(M68K* z);
void TRAP(M68K* z, const unsigned vf);
void TRAPV(M68K* z);
void ILLEGAL(M68K* z, const uint16_t instr);
void LINEA(M68K* z);
void LINEF(M68K* z);
void NOP(M68K* z);
void RESET(M68K* z);
void STOP(M68K* z);
bool CheckPrivilege(M68K* z);

/* M68K_* free-function API exposed to consumers of m68k.h.
 *
 * All declarations live inside an `extern "C" { ... }` block (gated
 * by __cplusplus so plain C consumers can include this header
 * directly) -- the matching definitions in m68k.c also use
 * `extern "C"` linkage.  This makes the wrappers callable from
 * both C++ and C TUs, with one well-defined ABI symbol per name.
 *
 * Trade-off vs the previous `static FORCE_INLINE` header-side
 * definitions:  we lose call-site inlining of the thunk body
 * (each wrapper became a real function call to a 1-2 instruction
 * out-of-line body in m68k.c), but gain a C-callable surface
 * that sound.c needs.  None of these
 * wrappers are on the M68K Run inner loop -- they're called
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
#ifdef M68K_SPLIT_SWITCH
void     M68K_RunSplit0          (M68K* z, uint16_t instr, const unsigned instr_b11_b9, const unsigned instr_b2_b0);
void     M68K_RunSplit1          (M68K* z, uint16_t instr, const unsigned instr_b11_b9, const unsigned instr_b2_b0);
#endif
void     M68K_SetExtHalted       (M68K* z, bool state);
void     M68K_StateAction        (M68K* z, StateMem* sm, const unsigned load,
                                  const bool data_only, const char* sname);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
