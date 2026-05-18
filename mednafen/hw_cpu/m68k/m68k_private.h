#ifndef __MDFN_M68K_PRIVATE_H
#define __MDFN_M68K_PRIVATE_H

#include "../../mednafen.h"
#include "m68k.h"

INLINE void M68K::RecalcInt(void)
{
 XPending &= ~XPENDING_MASK_INT;

 if(IPL > (SRHB & 0x7))
  XPending |= XPENDING_MASK_INT;
}

/* Phase-8a: named width-typed Read methods.  The Read<T> template
 * below is kept as a thin dispatcher for the two T-parametric
 * call sites that remain in m68k_private.h after phase 8e:
 *   - HAM<T, AM>::Read   (the addressing-mode helper)
 *   - MOVEM_to_REGS body (T from its template param)
 * Both retire when the HAM cascade lands. */
INLINE uint8_t M68K::Read_u8(uint32_t addr)
{
 return BusRead8(addr);
}

INLINE uint16_t M68K::Read_u16(uint32_t addr)
{
 return BusRead16(addr);
}

INLINE uint32_t M68K::Read_u32(uint32_t addr)
{
 uint32_t ret;

 ret  = BusRead16(addr) << 16;
 ret |= BusRead16(addr + 2);

 return ret;
}

INLINE uint16_t M68K::ReadOp(void)
{
 uint16_t ret;

 ret = BusReadInstr(PC);
 PC += 2;

 return ret;
}

/* Phase-8a: named width-typed Write methods.  The Write<T, long_dec>
 * template below is kept as a thin dispatcher for the one remaining
 * T-parametric call site (HAM<T, AM>::Write); phase 8e dropped
 * MOVEM_to_MEM's usage of it by inlining the width-dispatch
 * directly.  Retires when HAM detemplates.
 *
 * The two 32-bit variants differ only in bus-write ordering:
 *  - Write_u32          : high half first (default for non-predec)
 *  - Write_u32_longdec  : low half first  (used by Push_u32 and by
 *                                          MOVEM_to_MEM in predec mode) */
INLINE void M68K::Write_u8(uint32_t addr, const uint8_t val)
{
 BusWrite8(addr, val);
}

INLINE void M68K::Write_u16(uint32_t addr, const uint16_t val)
{
 BusWrite16(addr, val);
}

INLINE void M68K::Write_u32(uint32_t addr, const uint32_t val)
{
 BusWrite16(addr,     val >> 16);
 BusWrite16(addr + 2, val);
}

INLINE void M68K::Write_u32_longdec(uint32_t addr, const uint32_t val)
{
 BusWrite16(addr + 2, val);
 BusWrite16(addr,     val >> 16);
}


/* Phase-8a: Push and Pull were `template<typename T>` member
 * methods.  Every caller used a concrete uint16_t or uint32_t, so
 * the templates are gone -- the two width-typed variants below
 * are the entire callsite ABI.  Push_u32 uses Write_u32_longdec
 * (low half first) to match the M68K's 32-bit predec semantics
 * for stack pushes. */
INLINE void M68K::Push_u16(const uint16_t value)
{
 A[7] -= 2;
 Write_u16(A[7], value);
}

INLINE void M68K::Push_u32(const uint32_t value)
{
 A[7] -= 4;
 Write_u32_longdec(A[7], value);
}

INLINE uint16_t M68K::Pull_u16(void)
{
 uint16_t ret = Read_u16(A[7]);
 A[7] += 2;
 return ret;
}

INLINE uint32_t M68K::Pull_u32(void)
{
 uint32_t ret = Read_u32(A[7]);
 A[7] += 4;
 return ret;
}

//
// MOVE byte and word: instructions, 2 cycle penalty for source predecrement only
//  	2 cycle penalty for (d8, An, Xn) for both source and dest ams
//  	2 cycle penalty for (d8, PC, Xn) for dest am
//

//
// Careful on declaration order of HAM objects(needs to be source then dest).
//
template<typename T, AddressMode am>
struct M68K::HAM
{
 INLINE HAM(M68K* z) : zptr(z), reg(0), have_ea(false)
 {
  static_assert(am == PC_DISP || am == PC_INDEX || am == ABS_SHORT || am == ABS_LONG || am == IMMEDIATE, "Wrong arg count.");

  switch(am)
  {
   case PC_DISP:   // (d16, PC)
   case PC_INDEX:  // PC with index
	ea = zptr->PC;
	ext = zptr->ReadOp();
	break;

   case ABS_SHORT: // (xxxx).W
	ext = zptr->ReadOp();
	break;

   case ABS_LONG: // (xxxx).L
	ext = zptr->ReadOp() << 16;
	ext |= zptr->ReadOp();
	break;

   case IMMEDIATE: // Immediate
	if(sizeof(T) == 4)
	{
	 ext = zptr->ReadOp() << 16;
	 ext |= zptr->ReadOp();
	}
	else
	{
	 ext = zptr->ReadOp();
	}
	break;
  }
 }

 INLINE HAM(M68K* z, uint32_t arg) : zptr(z), reg(arg), have_ea(false)
 {
  static_assert(am != PC_DISP && am != PC_INDEX && am != ABS_SHORT && am != ABS_LONG, "Wrong arg count.");

  static_assert(am != ADDR_REG_DIR || sizeof(T) != 1, "Wrong size for address reg direct read");

  switch(am)
  {
   case DATA_REG_DIR:
   case ADDR_REG_DIR:
   case ADDR_REG_INDIR:
   case ADDR_REG_INDIR_POST:
   case ADDR_REG_INDIR_PRE:
   	break;
  
   case ADDR_REG_INDIR_DISP:	// (d16, An)
   case ADDR_REG_INDIR_INDX: 	// (d8, An, Xn)
	ext = zptr->ReadOp();
	break;

   case IMMEDIATE: 	// Immediate (quick)
	ext = arg;
	break;
  }
 }

 private:
 INLINE void calcea(const int predec_penalty)
 {
  if(have_ea)
   return;

  have_ea = true;

  switch(am)
  {
   default:
	break;

   case ADDR_REG_INDIR:
	ea = zptr->A[reg];
	break;

   case ADDR_REG_INDIR_POST:
	ea = zptr->A[reg];
	zptr->A[reg] += (sizeof(T) == 1 && reg == 0x7) ? 2 : sizeof(T);
	break;

   case ADDR_REG_INDIR_PRE:
	zptr->timestamp += predec_penalty;
	zptr->A[reg] -= (sizeof(T) == 1 && reg == 0x7) ? 2 : sizeof(T);
	ea = zptr->A[reg];
	break;

   case ADDR_REG_INDIR_DISP:
	ea = zptr->A[reg] + (int16_t)ext;
	break;

   case ADDR_REG_INDIR_INDX:
	zptr->timestamp += 2;
	ea = zptr->A[reg] + (int8_t)ext + ((ext & 0x800) ? zptr->DA[ext >> 12] : (int16_t)zptr->DA[ext >> 12]);
	break;	

   case ABS_SHORT:
	ea = (int16_t)ext;
	break;

   case ABS_LONG:
	ea = ext;
	break;

   case PC_DISP:
	ea = ea + (int16_t)ext;
	break;

   case PC_INDEX:
	zptr->timestamp += 2;
	ea = ea + (int8_t)ext + ((ext & 0x800) ? zptr->DA[ext >> 12] : (int16_t)zptr->DA[ext >> 12]);
	break;
  }
 }
 public:

 //
 // TODO: check pre-decrement 32-bit->2x 16-bit write order
 //

 INLINE void write(const T val, const int predec_penalty = 2)
 {
  static_assert(am != PC_DISP && am != PC_INDEX && am != IMMEDIATE, "What");

  static_assert(am != ADDR_REG_DIR || sizeof(T) == 4, "Wrong size for address reg direct write");

  switch(am)
  {
   case ADDR_REG_DIR:
	zptr->A[reg] = val;
	break;

   case DATA_REG_DIR:
	#ifdef MSB_FIRST
	memcpy((uint8_t*)&zptr->D[reg] + (4 - sizeof(T)), &val, sizeof(T));
	#else
	memcpy((uint8_t*)&zptr->D[reg] + 0, &val, sizeof(T));
	#endif
	break;

   case ADDR_REG_INDIR:
   case ADDR_REG_INDIR_POST:
   case ADDR_REG_INDIR_PRE:
   case ADDR_REG_INDIR_DISP:
   case ADDR_REG_INDIR_INDX:
   case ABS_SHORT:
   case ABS_LONG:
	calcea(predec_penalty);
	/* Phase-8s-pre: Write<T, long_dec> inlined.  sizeof(T) and
	 * `am == ADDR_REG_INDIR_PRE` fold at HAM<T,am> template
	 * instantiation. */
	if(sizeof(T) == 1)      zptr->Write_u8 (ea, val);
	else if(sizeof(T) == 2) zptr->Write_u16(ea, val);
	else if(am == ADDR_REG_INDIR_PRE) zptr->Write_u32_longdec(ea, val);
	else                    zptr->Write_u32(ea, val);
	break;
  }
 }

 INLINE T read(void)
 {
  switch(am)
  {
   case DATA_REG_DIR:
	return zptr->D[reg];

   case ADDR_REG_DIR:
	return zptr->A[reg];

   case IMMEDIATE:
	return ext;

   case ADDR_REG_INDIR:
   case ADDR_REG_INDIR_POST:
   case ADDR_REG_INDIR_PRE:
   case ADDR_REG_INDIR_DISP:
   case ADDR_REG_INDIR_INDX:
   case ABS_SHORT:
   case ABS_LONG:
   case PC_DISP:
   case PC_INDEX:
	calcea(2);
	/* Phase-8s-pre: Read<T>(ea) inlined.  sizeof(T) folds at
	 * HAM<T,am> template instantiation. */
	if(sizeof(T) == 1)      return zptr->Read_u8 (ea);
	else if(sizeof(T) == 2) return zptr->Read_u16(ea);
	else                    return zptr->Read_u32(ea);
  }
 }

 INLINE void rmw(T (MDFN_FASTCALL *cb)(M68K*, T))
 {
  static_assert(am != PC_DISP && am != PC_INDEX && am != IMMEDIATE, "What");

  switch(am)
  {
   case DATA_REG_DIR:
	{
	 T tmp = cb(zptr, zptr->D[reg]);
	 #ifdef MSB_FIRST
	 memcpy((uint8_t*)&zptr->D[reg] + (4 - sizeof(T)), &tmp, sizeof(T));
	 #else
	 memcpy((uint8_t*)&zptr->D[reg] + 0, &tmp, sizeof(T));
	 #endif
	}
	break;

   case ADDR_REG_INDIR:
   case ADDR_REG_INDIR_POST:
   case ADDR_REG_INDIR_PRE:
   case ADDR_REG_INDIR_DISP:
   case ADDR_REG_INDIR_INDX:
   case ABS_SHORT:
   case ABS_LONG:
	calcea(2);

	zptr->BusRMW(ea, cb);
	break;
  }
 }


 INLINE void jump(void)
 {
  calcea(0);
  zptr->PC = ea;
 }

 INLINE uint32_t getea(void)
 {
  static_assert(am == ADDR_REG_INDIR || am == ADDR_REG_INDIR_DISP || am == ADDR_REG_INDIR_INDX || am == ABS_SHORT || am == ABS_LONG || am == PC_DISP || am == PC_INDEX, "Wrong addressing mode");
  calcea(0);
  return ea;
 }

 M68K* zptr;

 uint32_t ea;
 uint32_t ext;
 const unsigned reg;

 private:
 bool have_ea;
};



INLINE bool M68K::GetC(void) { return Flag_C; }
INLINE bool M68K::GetV(void) { return Flag_V; }
INLINE bool M68K::GetZ(void) { return Flag_Z; }
INLINE bool M68K::GetN(void) { return Flag_N; }
INLINE bool M68K::GetX(void) { return Flag_X; }

INLINE void M68K::SetCX(bool val)
{
 Flag_C = (val);
 Flag_X = (val);
}

//
// Z_OnlyClear should be true for ADDX, SUBX, NEGX, ABCD, SBCD, NBCD.
//
// History: Phase-8b broke a single template<T, Z_OnlyClear> CalcZN
// body into the six named methods below (three widths, two Z
// behaviours) and kept the template as a thin dispatcher.
// Phase-9d-6 retired the dispatcher entirely; the
// CALC_ZN(z, T, val) / CALC_ZN_CLEAR(z, T, val) macros further
// below do the size-keyed dispatch at the call site.  The named
// methods are the live API now and stay class members of M68K.
//

INLINE void M68K::CalcZN_u8(const uint8_t val)
{
 Flag_Z = (val == 0);
 Flag_N = ((int8_t)val < 0);
}

INLINE void M68K::CalcZN_u8_clear(const uint8_t val)
{
 if(val != 0)
  Flag_Z = false;
 Flag_N = ((int8_t)val < 0);
}

INLINE void M68K::CalcZN_u16(const uint16_t val)
{
 Flag_Z = (val == 0);
 Flag_N = ((int16_t)val < 0);
}

INLINE void M68K::CalcZN_u16_clear(const uint16_t val)
{
 if(val != 0)
  Flag_Z = false;
 Flag_N = ((int16_t)val < 0);
}

INLINE void M68K::CalcZN_u32(const uint32_t val)
{
 Flag_Z = (val == 0);
 Flag_N = ((int32_t)val < 0);
}

INLINE void M68K::CalcZN_u32_clear(const uint32_t val)
{
 if(val != 0)
  Flag_Z = false;
 Flag_N = ((int32_t)val < 0);
}

/* Phase-9d-6: CalcZN<T, Z_OnlyClear> template retired.
 *
 * Previously CalcZN<T, Z_OnlyClear> was a one-line dispatcher template
 * that selected one of the six concrete CalcZN_uN[_clear] methods
 * above based on `sizeof(T)` and `Z_OnlyClear`.  Phase-8b broke the
 * body out into the six named variants and kept the template as the
 * single entry point for the 15+ T-parametric call sites inside
 * ADD/SUB/MOVE/etc. op templates.
 *
 * The CALC_ZN(z, T, val) / CALC_ZN_CLEAR(z, T, val) macros below
 * replace the template.  `T` is a type (template parameter of the
 * enclosing op or a concrete typedef); sizeof(T) is a compile-time
 * constant in every current caller.  Each macro emits an if/else-if
 * chain that selects the right named method, and gcc -O2 dead-code-
 * eliminates the unused arms after sizeof(T) is folded -- producing
 * byte-equivalent codegen to the prior template instantiation.
 *
 * Why macros, not non-template inline:  a free inline taking the
 * width as a runtime arg would still be constant-foldable for
 * compile-time-known T, but it adds a function-call frame the
 * optimizer has to chew through.  Macros keep the call site flat
 * and identical to what the template was already producing.
 *
 * Why the `z->` qualifier:  the called methods (CalcZN_u8 etc.)
 * are still class members of struct M68K.  When the macro is
 * expanded inside an M68K method body (which is where every caller
 * is today), `z` is just `this`; when the op templates eventually
 * detemplate to free functions taking `M68K* z`, the same spelling
 * keeps working without macro changes.
 */
#define CALC_ZN(z, T, val) \
 do { \
  if      (sizeof(T) == 4) (z)->CalcZN_u32((val)); \
  else if (sizeof(T) == 2) (z)->CalcZN_u16((val)); \
  else                     (z)->CalcZN_u8 ((val)); \
 } while(0)

#define CALC_ZN_CLEAR(z, T, val) \
 do { \
  if      (sizeof(T) == 4) (z)->CalcZN_u32_clear((val)); \
  else if (sizeof(T) == 2) (z)->CalcZN_u16_clear((val)); \
  else                     (z)->CalcZN_u8_clear ((val)); \
 } while(0)

INLINE uint8_t M68K::GetCCR(void)
{
 return (GetC() << 0) | (GetV() << 1) | (GetZ() << 2) | (GetN() << 3) | (GetX() << 4);
}

INLINE void M68K::SetCCR(uint8_t val)
{
 Flag_C   = ((val >> 0) & 1);
 Flag_V   = ((val >> 1) & 1);
 Flag_Z   = ((val >> 2) & 1);
 Flag_N   = ((val >> 3) & 1);
 Flag_X   = ((val >> 4) & 1);
}

INLINE uint16_t M68K::GetSR(void)
{
 return GetCCR() | (SRHB << 8);
}

INLINE void M68K::SetSR(uint16_t val)
{
 const uint8_t new_srhb = (val >> 8) & 0xA7;

 Flag_C   = ((val >> 0) & 1);
 Flag_V   = ((val >> 1) & 1);
 Flag_Z   = ((val >> 2) & 1);
 Flag_N   = ((val >> 3) & 1);
 Flag_X   = ((val >> 4) & 1);

 if((SRHB ^ new_srhb) & 0x20)	// Supervisor mode change
 {
  std::swap(A[7], SP_Inactive);
 }

 SRHB = new_srhb;
 RecalcInt();
}

INLINE bool M68K::GetSVisor(void)
{
 return (bool)(GetSR() & 0x2000);
}

//
//
//

//
// ADD
//
template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
INLINE void M68K::ADD(HAM<T, SAM> &src, HAM<DT, DAM> &dst)
{
 static_assert(DAM == ADDR_REG_DIR || std::is_same<T, DT>::value, "Type mismatch");

 uint32_t const src_data = (DT)static_cast<typename std::make_signed<T>::type>(src.read());
 uint32_t const dst_data = dst.read();
 uint64_t const result = (uint64_t)dst_data + src_data;

 if(DAM == ADDR_REG_DIR)
 {
  if(sizeof(T) != 4 || SAM == DATA_REG_DIR || SAM == ADDR_REG_DIR || SAM == IMMEDIATE)
   timestamp += 4;
  else
   timestamp += 2;
 }
 else if(DAM == DATA_REG_DIR && sizeof(DT) == 4)
 {
  if(SAM == DATA_REG_DIR || SAM == IMMEDIATE)
   timestamp += 4;
  else
   timestamp += 2;
 }

 if(DAM != ADDR_REG_DIR)
 {
  CALC_ZN(this, DT, result);
  SetCX((result >> (sizeof(DT) * 8)) & 1);
  Flag_V = ((((~(dst_data ^ src_data)) & (dst_data ^ result)) >> (sizeof(DT) * 8 - 1)) & 1);
 }

 dst.write(result);
}


//
// ADDX
//
template<typename T, AddressMode SAM, AddressMode DAM>
INLINE void M68K::ADDX(HAM<T, SAM> &src, HAM<T, DAM> &dst)
{
 uint32_t const src_data = src.read();
 uint32_t const dst_data = dst.read();
 uint64_t const result = (uint64_t)dst_data + src_data + GetX();

 if(DAM != DATA_REG_DIR)
 {
  timestamp += 2;
 }
 else
 {
  if(sizeof(T) == 4)
   timestamp += 4;
 }

 CALC_ZN_CLEAR(this, T, result);
 SetCX((result >> (sizeof(T) * 8)) & 1);
 Flag_V = ((((~(dst_data ^ src_data)) & (dst_data ^ result)) >> (sizeof(T) * 8 - 1)) & 1);

 dst.write(result);
}


//
// Used to implement SUB, SUBA, SUBX, NEG, NEGX
//
// Phase-8e: `bool X_form` template parameter moved to runtime
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
template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
INLINE DT M68K::Subtract(bool X_form, HAM<T, SAM> &src, HAM<DT, DAM> &dst)
{
 static_assert(DAM == ADDR_REG_DIR || std::is_same<T, DT>::value, "Type mismatch");

 uint32_t const src_data = (DT)static_cast<typename std::make_signed<T>::type>(src.read());
 uint32_t const dst_data = dst.read();
 const uint64_t result = (uint64_t)dst_data - src_data - (X_form ? GetX() : 0);

 if(DAM == ADDR_REG_DIR)	// SUBA, SUBQ(A) only.
 {
  if(sizeof(T) != 4 || SAM == DATA_REG_DIR || SAM == ADDR_REG_DIR || SAM == IMMEDIATE)
   timestamp += 4;
  else
   timestamp += 2;
 }
 else if(DAM == DATA_REG_DIR)	// SUB, SUBQ, SUBX only.
 {
  if(sizeof(DT) == 4)
  {
   if(SAM == DATA_REG_DIR || SAM == IMMEDIATE)
    timestamp += 4;
   else
    timestamp += 2;
  }
 }
 else if(DAM == IMMEDIATE)	// NEG, NEGX only and always.
 {
  if(sizeof(T) == 4)
  {
   timestamp += 2;
  }
 }
 else if(SAM != IMMEDIATE && SAM != ADDR_REG_DIR && SAM != DATA_REG_DIR) // SUBX m,m
 {
  timestamp += 2;
 }


 if(DAM != ADDR_REG_DIR)
 {
  if(X_form)
   CALC_ZN_CLEAR(this, DT, result);
  else
   CALC_ZN(this, DT, result);
  SetCX((result >> (sizeof(DT) * 8)) & 1);
  Flag_V = (((((dst_data ^ src_data)) & (dst_data ^ result)) >> (sizeof(DT) * 8 - 1)) & 1);
 }

 return result;
}


//
// SUB
//
template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
INLINE void M68K::SUB(HAM<T, SAM> &src, HAM<DT, DAM> &dst)
{
 dst.write(Subtract(false, src, dst));
}


//
// SUBX
//
template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
INLINE void M68K::SUBX(HAM<T, SAM> &src, HAM<DT, DAM> &dst)
{
 dst.write(Subtract(true, src, dst));
}


//
// NEG
//
// Phase-9d-10: extracted to free template; calls z->Subtract since
// Subtract is still a class method.
//
template<typename DT, AddressMode DAM>
INLINE void NEG(M68K* z, M68K::HAM<DT, DAM> &dst)
{
 M68K::HAM<DT, IMMEDIATE> dummy_zero(z, 0);

 dst.write(z->Subtract(false, dst, dummy_zero));
}


//
// NEGX
//
template<typename DT, AddressMode DAM>
INLINE void NEGX(M68K* z, M68K::HAM<DT, DAM> &dst)
{
 M68K::HAM<DT, IMMEDIATE> dummy_zero(z, 0);

 dst.write(z->Subtract(true, dst, dummy_zero));
}


//
// CMP
//
template<typename T, typename DT, AddressMode SAM, AddressMode DAM>
INLINE void M68K::CMP(HAM<T, SAM> &src, HAM<DT, DAM> &dst)
{
 static_assert(DAM == ADDR_REG_DIR || std::is_same<T, DT>::value, "Type mismatch");

 // Doesn't affect X flag
 uint32_t const src_data = (DT)static_cast<typename std::make_signed<T>::type>(src.read());
 uint32_t const dst_data = dst.read();
 uint64_t const result = (uint64_t)dst_data - src_data;

 CALC_ZN(this, DT, result);
 Flag_C = ((result >> (sizeof(DT) * 8)) & 1);
 Flag_V = (((((dst_data ^ src_data)) & (dst_data ^ result)) >> (sizeof(DT) * 8 - 1)) & 1);
}


//
// CHK
//
// Exception on dst < 0 || dst > src
template<typename T, AddressMode SAM, AddressMode DAM>
INLINE void M68K::CHK(HAM<T, SAM> &src, HAM<T, DAM> &dst)
{
 uint32_t const src_data = src.read();
 uint32_t const dst_data = dst.read();
 
 timestamp += 6;

 CALC_ZN(this, T, dst_data);
 if(GetN())
 {
  Exception(EXCEPTION_CHK, VECNUM_CHK);
 }
 else
 {
  // 7 - 1
  uint64_t const result = (uint64_t)dst_data - src_data;

  CALC_ZN(this, T, result);
  Flag_C = ((result >> (sizeof(T) * 8)) & 1);
  Flag_V = (((((dst_data ^ src_data)) & (dst_data ^ result)) >> (sizeof(T) * 8 - 1)) & 1);

  if(GetN() == GetV() && !GetZ())
  {
   Exception(EXCEPTION_CHK, VECNUM_CHK);
  }
 }
}


//
// OR
//
// Phase-9d-12: bitwise family (AND/OR/EOR) extracted from M68K
// class scope.  Standard pattern (Flag_X -> z->Flag_X, timestamp
// -> z->timestamp, CALC_ZN(this,...) -> CALC_ZN(z,...)); BTST/BCHG
// /BCLR/BSET below follow the same transform with only Flag_Z.
//
template<typename T, AddressMode SAM, AddressMode DAM>
INLINE void OR(M68K* z, M68K::HAM<T, SAM> &src, M68K::HAM<T, DAM> &dst)
{
 T const src_data = src.read();
 T const dst_data = dst.read();
 T const result = dst_data | src_data;

 if(sizeof(T) == 4 && DAM == DATA_REG_DIR)
 {
  if(SAM == IMMEDIATE || SAM == DATA_REG_DIR)
   z->timestamp += 4;
  else
   z->timestamp += 2;
 }

 CALC_ZN(z, T, result);
 z->Flag_C = (false);
 z->Flag_V = false;

 dst.write(result);
}


//
// EOR
//
template<typename T, AddressMode SAM, AddressMode DAM>
INLINE void EOR(M68K* z, M68K::HAM<T, SAM> &src, M68K::HAM<T, DAM> &dst)
{
 T const src_data = src.read();
 T const dst_data = dst.read();
 T const result = dst_data ^ src_data;

 if(sizeof(T) == 4 && DAM == DATA_REG_DIR)
 {
  if(SAM == IMMEDIATE || SAM == DATA_REG_DIR)
   z->timestamp += 4;
  else
   z->timestamp += 2;
 }

 CALC_ZN(z, T, result);
 z->Flag_C = false;
 z->Flag_V = false;

 dst.write(result);
}


//
// AND
//
template<typename T, AddressMode SAM, AddressMode DAM>
INLINE void AND(M68K* z, M68K::HAM<T, SAM> &src, M68K::HAM<T, DAM> &dst)
{
 T const src_data = src.read();
 T const dst_data = dst.read();
 T const result = dst_data & src_data;

 if(sizeof(T) == 4 && DAM == DATA_REG_DIR)
 {
  if(SAM == IMMEDIATE || SAM == DATA_REG_DIR)
   z->timestamp += 4;
  else
   z->timestamp += 2;
 }

 CALC_ZN(z, T, result);
 z->Flag_C = false;
 z->Flag_V = false;

 dst.write(result);
}


//
// ORI CCR
//
INLINE void M68K::ORI_CCR(void)
{
 const uint8_t imm = ReadOp();

 SetCCR(GetCCR() | imm);

 //
 //
 timestamp += 8;
 ReadOp();
 PC -= 2;
}


//
// ORI SR
//
INLINE void M68K::ORI_SR(void)
{
 const uint16_t imm = ReadOp();

 SetSR(GetSR() | imm);

 //
 //
 timestamp += 8;
 ReadOp();
 PC -= 2;
}


//
// ANDI CCR
//
INLINE void M68K::ANDI_CCR(void)
{
 const uint8_t imm = ReadOp();

 SetCCR(GetCCR() & imm);

 //
 //
 timestamp += 8;
 ReadOp();
 PC -= 2;
}


//
// ANDI SR
//
INLINE void M68K::ANDI_SR(void)
{
 const uint16_t imm = ReadOp();

 SetSR(GetSR() & imm);

 //
 //
 timestamp += 8;
 ReadOp();
 PC -= 2;
}


//
// EORI CCR
//
INLINE void M68K::EORI_CCR(void)
{
 const uint8_t imm = ReadOp();

 SetCCR(GetCCR() ^ imm);

 //
 //
 timestamp += 8;
 ReadOp();
 PC -= 2;
}


//
// EORI SR
//
INLINE void M68K::EORI_SR(void)
{
 const uint16_t imm = ReadOp();

 SetSR(GetSR() ^ imm);

 //
 //
 timestamp += 8;
 ReadOp();
 PC -= 2;
}


//
// MULU
//
template<typename T, AddressMode SAM>
INLINE void M68K::MULU(HAM<T, SAM> &src, const unsigned dr)
{
 // Doesn't affect X flag
 static_assert(sizeof(T) == 2, "Wrong type.");

 T const src_data = src.read();
 uint32_t const result = (uint32_t)(uint16_t)D[dr] * (uint32_t)src_data;

 timestamp += 34;

 for(uint32_t tmp = src_data; tmp; tmp &= tmp - 1)
  timestamp += 2;

 CalcZN_u32(result);
 Flag_C = false;
 Flag_V = false;

 D[dr] = result;
}


//
// MULS
//
template<typename T, AddressMode SAM>
INLINE void M68K::MULS(HAM<T, SAM> &src, const unsigned dr)
{
 // Doesn't affect X flag
 static_assert(sizeof(T) == 2, "Wrong type.");

 T const src_data = src.read();
 uint32_t const result = (int16_t)D[dr] * (int16_t)src_data;

 timestamp += 34;

 for(uint32_t tmp = src_data << 1, i = 0; i < 16; tmp >>= 1, i++)
  timestamp += (tmp ^ (tmp << 1)) & 2;

 CalcZN_u32(result);
 Flag_C = false;
 Flag_V = false;

 D[dr] = result;
}


/* Phase-8b: Divide<sdiv> retired.  The previous template body's
 * `if(sdiv)` branches fold per-instantiation, so the two named
 * methods below are bit-equivalent to the original template's two
 * instantiations (sdiv=false -> Divide_u for DIVU, sdiv=true ->
 * Divide_s for DIVS). */
INLINE void M68K::Divide_u(uint16_t divisor, const unsigned dr)
{
 uint32_t dividend = D[dr];
 uint32_t tmp;
 bool oflow = false;
 int i;

 if(!divisor)
 {
  Exception(EXCEPTION_ZERO_DIVIDE, VECNUM_ZERO_DIVIDE);
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
 CalcZN_u16((uint16_t)tmp);
 Flag_C = false;
 Flag_V = oflow;

 if(!oflow)
  D[dr] = tmp;
}

INLINE void M68K::Divide_s(uint16_t divisor, const unsigned dr)
{
 uint32_t dividend = D[dr];
 uint32_t tmp;
 bool neg_quotient = false;
 bool neg_remainder = false;
 bool oflow = false;
 int i;

 if(!divisor)
 {
  Exception(EXCEPTION_ZERO_DIVIDE, VECNUM_ZERO_DIVIDE);
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
 CalcZN_u16((uint16_t)tmp);
 Flag_C = false;
 Flag_V = oflow;

 if(!oflow)
  D[dr] = tmp;
}


//
// DIVU
//
template<typename T, AddressMode SAM>
INLINE void M68K::DIVU(HAM<T, SAM> &src, const unsigned dr)
{
 static_assert(sizeof(T) == 2, "Wrong type.");

 T const src_data = src.read();

 Divide_u(src_data, dr);
}


//
// DIVS
//
template<typename T, AddressMode SAM>
INLINE void M68K::DIVS(HAM<T, SAM> &src, const unsigned dr)
{
 // Doesn't affect X flag
 static_assert(sizeof(T) == 2, "Wrong type.");

 T const src_data = src.read();

 Divide_s(src_data, dr);
}


//
// ABCD
//
template<typename T, AddressMode SAM, AddressMode DAM>
INLINE void M68K::ABCD(HAM<T, SAM> &src, HAM<T, DAM> &dst)	// ...XYZ, now I know my ABCs~
{
 static_assert(sizeof(T) == 1, "Wrong size.");

 bool V = false;
 uint8_t const src_data = src.read();
 uint8_t const dst_data = dst.read();
 uint32_t tmp;

 tmp = dst_data + src_data + GetX();

 if(((dst_data ^ src_data ^ tmp) & 0x10) || (tmp & 0xF) >= 0x0A)
 {
  uint8_t prev_tmp = tmp;
  tmp += 0x06;
  V |= ((~prev_tmp & 0x80) & (tmp & 0x80));
 }

 if(tmp >= 0xA0)
 {
  uint8_t prev_tmp = tmp;
  tmp += 0x60;
  V |= ((~prev_tmp & 0x80) & (tmp & 0x80));
 }

 CalcZN_u8_clear(tmp);
 SetCX((bool)(tmp >> 8));
 Flag_V = V;

 if(DAM == DATA_REG_DIR)
  timestamp += 2;
 else
  timestamp += 4;

 dst.write(tmp);
}


INLINE uint8_t M68K::DecimalSubtractX(const uint8_t src_data, const uint8_t dst_data)
{
 bool V = false;
 uint32_t tmp;

 tmp = dst_data - src_data - GetX();

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

 Flag_V = V;
 CalcZN_u8_clear(tmp);
 SetCX((bool)(tmp >> 8));

 return tmp;
}

//
// SBCD
//
template<typename T, AddressMode SAM, AddressMode DAM>
INLINE void M68K::SBCD(HAM<T, SAM> &src, HAM<T, DAM> &dst)
{
 static_assert(sizeof(T) == 1, "Wrong size.");
 uint8_t const src_data = src.read();
 uint8_t const dst_data = dst.read();

 if(DAM == DATA_REG_DIR)
  timestamp += 2;
 else
  timestamp += 4;

 dst.write(DecimalSubtractX(src_data, dst_data));
}


//
// NBCD
//
template<typename T, AddressMode DAM>
INLINE void M68K::NBCD(HAM<T, DAM> &dst)
{
 static_assert(sizeof(T) == 1, "Wrong size.");
 uint8_t const dst_data = dst.read();

 timestamp += 2;

 dst.write(DecimalSubtractX(dst_data, 0));
}

//
// MOVEP
//
// Phase-8d: was `template<typename T, bool reg_to_mem> M68K::MOVEP`;
// the 4 named bodies below are the post-folding result of the four
// template instantiations.  sizeof(T) was 2 or 4 (T = uint16_t /
// uint32_t), giving 2- or 4-iteration loops with shift = 8 or 24
// initial.  reg_to_mem picked between byte-out-to-bus and
// byte-in-from-bus over the same EA/shift schedule.
//

/* MOVEP.W (Dn -> mem):  upper-half-of-Dn[15:8], Dn[7:0]
 * written into (An+disp), (An+disp+2). */
INLINE void M68K::MOVEP_w_reg_to_mem(const unsigned ar, const unsigned dr)
{
 const int16_t ext = ReadOp();
 uint32_t ea = A[ar] + (int16_t)ext;
 unsigned shift = 8; /* (sizeof(uint16_t) - 1) << 3 */

 Write_u8(ea, D[dr] >> shift);
 ea += 2;
 shift -= 8;
 Write_u8(ea, D[dr] >> shift);
}

/* MOVEP.W (mem -> Dn):  two bytes from (An+disp), (An+disp+2)
 * packed back into Dn[15:0]. */
INLINE void M68K::MOVEP_w_mem_to_reg(const unsigned ar, const unsigned dr)
{
 const int16_t ext = ReadOp();
 uint32_t ea = A[ar] + (int16_t)ext;
 unsigned shift = 8;

 D[dr] &= ~(0xFF << shift);
 D[dr] |= Read_u8(ea) << shift;
 ea += 2;
 shift -= 8;
 D[dr] &= ~(0xFF << shift);
 D[dr] |= Read_u8(ea) << shift;
}

/* MOVEP.L (Dn -> mem):  four bytes from Dn[31:24..7:0] written
 * into (An+disp), (An+disp+2), (An+disp+4), (An+disp+6). */
INLINE void M68K::MOVEP_l_reg_to_mem(const unsigned ar, const unsigned dr)
{
 const int16_t ext = ReadOp();
 uint32_t ea = A[ar] + (int16_t)ext;
 unsigned shift = 24; /* (sizeof(uint32_t) - 1) << 3 */
 unsigned i;

 for(i = 0; i < 4; i++)
 {
  Write_u8(ea, D[dr] >> shift);
  ea += 2;
  shift -= 8;
 }
}

/* MOVEP.L (mem -> Dn):  four bytes packed back into Dn[31:0]. */
INLINE void M68K::MOVEP_l_mem_to_reg(const unsigned ar, const unsigned dr)
{
 const int16_t ext = ReadOp();
 uint32_t ea = A[ar] + (int16_t)ext;
 unsigned shift = 24;
 unsigned i;

 for(i = 0; i < 4; i++)
 {
  D[dr] &= ~(0xFF << shift);
  D[dr] |= Read_u8(ea) << shift;
  ea += 2;
  shift -= 8;
 }
}


template<typename T, AddressMode TAM>
INLINE void BTST(M68K* z, M68K::HAM<T, TAM> &targ, unsigned wb)
{
 T const src_data = targ.read();
 wb &= (sizeof(T) << 3) - 1;

 z->Flag_Z = (((src_data >> wb) & 1) == 0);
}

template<typename T, AddressMode TAM>
INLINE void BCHG(M68K* z, M68K::HAM<T, TAM> &targ, unsigned wb)
{
 T const src_data = targ.read();
 wb &= (sizeof(T) << 3) - 1;

 z->Flag_Z = (((src_data >> wb) & 1) == 0);

 targ.write(src_data ^ (1U << wb));
}

template<typename T, AddressMode TAM>
INLINE void BCLR(M68K* z, M68K::HAM<T, TAM> &targ, unsigned wb)
{
 T const src_data = targ.read();
 wb &= (sizeof(T) << 3) - 1;

 z->Flag_Z = (((src_data >> wb) & 1) == 0);

 targ.write(src_data & ~(1U << wb));
}

template<typename T, AddressMode TAM>
INLINE void BSET(M68K* z, M68K::HAM<T, TAM> &targ, unsigned wb)
{
 T const src_data = targ.read();
 wb &= (sizeof(T) << 3) - 1;

 z->Flag_Z = (((src_data >> wb) & 1) == 0);

 targ.write(src_data | (1U << wb));
}



//
// MOVE
//
template<typename T, AddressMode SAM, AddressMode DAM>
INLINE void M68K::MOVE(HAM<T, SAM> &src, HAM<T, DAM> &dst)
{
 T const tmp = src.read();

 if(DAM != ADDR_REG_DIR)
 {
  CALC_ZN(this, T, tmp);
  Flag_V = false;
  Flag_C = false;
 }

 dst.write(tmp);
}

template<typename T, AddressMode SAM>
INLINE void MOVEA(M68K* z, M68K::HAM<T, SAM> &src, const unsigned ar)
{
 uint32_t const src_data = static_cast<typename std::make_signed<T>::type>(src.read());

 z->A[ar] = src_data;
}


//
// MOVEM to memory
//
// Phase-8e: `bool pseudo_predec` moved to runtime first-arg.
// The `Write<T, pseudo_predec>` call inside the loop expanded
// to one of the named width-typed writes (Write_u8, Write_u16,
// Write_u32 / Write_u32_longdec).  We inline that dispatch
// directly so we don't need to go through the Write<T, bool>
// dispatcher template (whose long_dec parameter would still
// need to be compile-time).  T stays a template parameter so
// the sizeof(T) ladder folds.
//
template<typename T, AddressMode DAM>
INLINE void M68K::MOVEM_to_MEM(bool pseudo_predec, const uint16_t reglist, HAM<T, DAM> &dst)
{
 static_assert(DAM != ADDR_REG_INDIR_PRE && DAM != ADDR_REG_INDIR_POST, "Wrong address mode.");

 uint32_t ea = dst.getea();

 for(unsigned i = 0; i < 16; i++)
 {
  if(reglist & (1U << i))
  {
   if(pseudo_predec)
    ea -= sizeof(T);

   const T val = DA[pseudo_predec ? (15 - i) : i];

   if(sizeof(T) == 4)
   {
    if(pseudo_predec)
     Write_u32_longdec(ea, val);
    else
     Write_u32(ea, val);
   }
   else if(sizeof(T) == 2)
    Write_u16(ea, val);
   else
    Write_u8(ea, val);

   if(!pseudo_predec)
    ea += sizeof(T);
  }
 }

 if(pseudo_predec)
  A[dst.reg] = ea;
}


//
// MOVEM to regs(from memory)
//
// Phase-8e: `bool pseudo_postinc` moved to runtime first-arg.
//
template<typename T, AddressMode SAM>
INLINE void M68K::MOVEM_to_REGS(bool pseudo_postinc, HAM<T, SAM> &src, const uint16_t reglist)
{
 static_assert(SAM != ADDR_REG_INDIR_PRE && SAM != ADDR_REG_INDIR_POST, "Wrong address mode.");

 uint32_t ea = src.getea();

 for(unsigned i = 0; i < 16; i++)
 {
  if(reglist & (1U << i))
  {
   /* Phase-8s-pre: Read<T>(ea) inlined.  sizeof(T) folds at
    * MOVEM_to_REGS template instantiation. */
   T tmp;
   if(sizeof(T) == 1)      tmp = Read_u8 (ea);
   else if(sizeof(T) == 2) tmp = Read_u16(ea);
   else                    tmp = Read_u32(ea);

   DA[i] = static_cast<typename std::make_signed<T>::type>(tmp);

   ea += sizeof(T);
  }
 }

 Read_u16(ea);	// or should be <T> ?

 if(pseudo_postinc)
  A[src.reg] = ea;
}


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
template<typename T, AddressMode TAM>
INLINE void M68K::ShiftBase(bool Arithmetic, bool ShiftLeft, HAM<T, TAM> &targ, unsigned count)
{
 T vchange = 0;
 T result = targ.read();
 count &= 0x3F;

 if(TAM == DATA_REG_DIR)
  timestamp += (sizeof(T) == 4) ? 4 : 2;

 // X is unaffected with a shift count of 0!
 if(count == 0)
  Flag_C = false;
 else
 {
  bool shifted_out = false;

  do
  {
   if(TAM == DATA_REG_DIR)
    timestamp += 2;

   if(ShiftLeft)
    shifted_out = (result >> (sizeof(T) * 8 - 1)) & 1;
   else
    shifted_out = result & 1;

   if(Arithmetic)
   {
    const T prev = result;

    if(ShiftLeft)
     result = result << 1;
    else
     result = static_cast<typename std::make_signed<T>::type>(result) >> 1;

    vchange |= prev ^ result;
   }
   else
   {
    if(ShiftLeft)
     result = result << 1;
    else
     result = result >> 1;
   }
  } while(--count != 0);

  SetCX(shifted_out);
 }

 CALC_ZN(this, T, result);

 if(Arithmetic)
  Flag_V = ((vchange >> (sizeof(T) * 8 - 1)) & 1);
 else
  Flag_V = (false);

 targ.write(result);
}

template<typename T, AddressMode TAM>
INLINE void M68K::ASL(HAM<T, TAM> &targ, unsigned count)
{
 ShiftBase(true, true, targ, count);
}

template<typename T, AddressMode TAM>
INLINE void M68K::ASR(HAM<T, TAM> &targ, unsigned count)
{
 ShiftBase(true, false, targ, count);
}

template<typename T, AddressMode TAM>
INLINE void M68K::LSL(HAM<T, TAM> &targ, unsigned count)
{
 ShiftBase(false, true, targ, count);
}

template<typename T, AddressMode TAM>
INLINE void M68K::LSR(HAM<T, TAM> &targ, unsigned count)
{
 ShiftBase(false, false, targ, count);
}

//
// Phase-8e: RotateBase's `bool X_Form, bool ShiftLeft` template
// parameters moved to runtime first-args.  Same shape as ShiftBase.
//
template<typename T, AddressMode TAM>
INLINE void M68K::RotateBase(bool X_Form, bool ShiftLeft, HAM<T, TAM> &targ, unsigned count)
{
 T result = targ.read();
 count &= 0x3F;

 if(TAM == DATA_REG_DIR)
  timestamp += (sizeof(T) == 4) ? 4 : 2;

 if(count == 0)
 {
  if(X_Form)
   Flag_C = GetX();
  else
   Flag_C = false;
 }
 else
 {
  bool shifted_out = GetX();

  do
  {
   const bool shift_in = shifted_out;

   if(TAM == DATA_REG_DIR)
    timestamp += 2;

   if(ShiftLeft)
   {
    shifted_out = (result >> (sizeof(T) * 8 - 1)) & 1;
    result <<= 1;
    result |= (X_Form ? shift_in : shifted_out);
   }
   else
   {
    shifted_out = (result & 1);
    result >>= 1;
    result |= (T)(X_Form ? shift_in : shifted_out) << (sizeof(T) * 8 - 1);
   }
  } while(--count != 0);

  Flag_C  = shifted_out;
  if(X_Form)
   Flag_X = shifted_out;
 }

 CALC_ZN(this, T, result);
 Flag_V = (false);

 targ.write(result);
}

template<typename T, AddressMode TAM>
INLINE void M68K::ROL(HAM<T, TAM> &targ, unsigned count)
{
 RotateBase(false, true, targ, count);
}

template<typename T, AddressMode TAM>
INLINE void M68K::ROR(HAM<T, TAM> &targ, unsigned count)
{
 RotateBase(false, false, targ, count);
}

template<typename T, AddressMode TAM>
INLINE void M68K::ROXL(HAM<T, TAM> &targ, unsigned count)
{
 RotateBase(true, true, targ, count);
}

template<typename T, AddressMode TAM>
INLINE void M68K::ROXR(HAM<T, TAM> &targ, unsigned count)
{
 RotateBase(true, false, targ, count);
}

//
//
//

MDFN_FASTCALL uint8_t TAS_Callback(M68K* zptr, uint8_t data);

//
//
//

// Phase-9d-10: TAS/TST/CLR/NOT extracted from M68K class scope to free
// templates parallel to NEG/NEGX above (and EXT in bee65cf).  Each takes
// an explicit M68K* z; member access patterns (`Flag_X`, `timestamp`)
// become `z->Flag_X`/`z->timestamp`; CALC_ZN(this, ...) -> CALC_ZN(z, ...).
// TAS has no member access in its body, so z is unused there (kept in
// the signature for consistency with the rest of the op family).
//

template<typename T, AddressMode DAM>
INLINE void TAS(M68K* z, M68K::HAM<T, DAM> &dst)
{
 static_assert(std::is_same<T, uint8_t>::value, "Wrong type");
 (void)z; /* TAS_Callback is a file-scope free function (not a method);
           * dst.rmw passes it M68K* via dst.zptr internally, so the
           * outer z isn't read here. */
 dst.rmw(TAS_Callback);
}

//
// TST
//
template<typename T, AddressMode DAM>
INLINE void TST(M68K* z, M68K::HAM<T, DAM> &dst)
{
 T const dst_data = dst.read();

 CALC_ZN(z, T, dst_data);

 z->Flag_C = false;
 z->Flag_V = false;
}


//
// CLR
//
template<typename T, AddressMode DAM>
INLINE void CLR(M68K* z, M68K::HAM<T, DAM> &dst)
{
 dst.read();

 if(sizeof(T) == 4 && DAM == DATA_REG_DIR)
  z->timestamp += 2;

 z->Flag_Z = true;
 z->Flag_N = false;

 z->Flag_C = false;
 z->Flag_V = false;

 dst.write(0);
}

//
// NOT
//
template<typename T, AddressMode DAM>
INLINE void NOT(M68K* z, M68K::HAM<T, DAM> &dst)
{
 T result = dst.read();

 if(sizeof(T) == 4 && DAM == DATA_REG_DIR)
  z->timestamp += 2;

 result = ~result;

 CALC_ZN(z, T, result);
 z->Flag_C = false;
 z->Flag_V = false;

 dst.write(result);
}


//
// EXT
//
// Phase-9d-9: moved out of M68K class scope to a free template
// taking M68K* z + a still-templated HAM<T, DAM>& dst.  This is the
// first step into the load-bearing template surface; it lets the
// pattern get verified on a small, isolated op (4 call sites across
// the three .inc files) before being applied to the rest of the 50-
// op family.  HAM itself is unchanged -- still M68K::HAM<T, AM> --
// because detempleting HAM is a separate (bigger) commit that
// touches all 8,527 call sites in one go.
//
template<typename T, AddressMode DAM>
INLINE void EXT(M68K* z, M68K::HAM<T, DAM> &dst)
{
 static_assert(std::is_same<T, uint16_t>::value || std::is_same<T, uint32_t>::value, "Wrong type");
 T result = dst.read();

 if(std::is_same<T, uint16_t>::value)
  result = (int8_t)result;
 else
  result = (int16_t)result;

 CALC_ZN(z, T, result);
 z->Flag_C = false;
 z->Flag_V = false;

 dst.write(result);
}

//
// SWAP
//
INLINE void M68K::SWAP(const unsigned dr)
{
 D[dr] = (D[dr] << 16) | (D[dr] >> 16);

 CalcZN_u32(D[dr]);
 Flag_C = false;
 Flag_V = false;
}


//
// EXG (doesn't affect flags)
//
INLINE void M68K::EXG(uint32_t* a, uint32_t* b)
{
 timestamp += 2;

 std::swap(*a, *b); 
}

//
//
//

/* Phase-8c: TestCond, Bxx, DBcc fully detempleted.  Scc keeps its
 * T / DAM template parameters (still HAM-locked) but its cc
 * parameter moved to a runtime first-arg too. */
INLINE bool M68K::TestCond(unsigned cc)
{
 switch(cc)
 {
  case 0x00:	// TRUE
	return true;

  case 0x01:	// FALSE
	return false;

  case 0x02:	// HI
	return !GetC() && !GetZ();

  case 0x03:	// LS
	return GetC() || GetZ();

  case 0x04:	// CC/HS
	return !GetC();

  case 0x05:	// CS/LO
	return GetC();

  case 0x06:	// NE
	return !GetZ();

  case 0x07:	// EQ
	return GetZ();

  case 0x08:	// VC
	return !GetV();

  case 0x09:	// VS
	return GetV();

  case 0x0A:	// PL
	return !GetN();

  case 0x0B:	// MI
	return GetN();

  case 0x0C:	// GE
	return GetN() == GetV();

  case 0x0D:	// LT
	return GetN() != GetV();

  case 0x0E:	// GT
	return GetN() == GetV() && !GetZ();

  case 0x0F:	// LE
	return GetN() != GetV() || GetZ();
 }
 return false; /* unreachable, but keeps -Wreturn-type happy now
                * that cc is no longer a static-assert'd template arg */
}

//
// Bcc, BRA, BSR
//
//  (caller of this function should sign-extend the 8-bit displacement)
//
INLINE void M68K::Bxx(unsigned cc, uint32_t disp)
{
 const uint32_t BPC = PC;

 /* cc == 0x01 here means BSR (Branch to Subroutine), not "Bcc-False"
  * -- override to TRUE so the branch is always taken. */
 if(TestCond((cc == 0x01) ? 0x00 : cc))
 {
  const uint16_t disp16 = (int16_t)ReadOp();

  if(!disp)
   disp = (int16_t)disp16;
  else
   PC -= 2;

  if(cc == 0x01)
   Push_u32(PC);

  timestamp += 2;
  PC = BPC + disp;
 }
 else
 {
  if(!disp)
   ReadOp();

  timestamp += 4;
 }
}

INLINE void M68K::DBcc(unsigned cc, const unsigned dr)
{
 const uint32_t BPC = PC;
 uint32_t disp;

 disp = (int16_t)ReadOp();

 if(!TestCond(cc))
 {
  const uint16_t result = D[dr] - 1;

  timestamp += 2;
  D[dr] = (D[dr] & 0xFFFF0000) | result;

  if(result != 0xFFFF)
   PC = BPC + disp;
  else
   timestamp += 4;
 }
 else
  timestamp += 4;
}


//
// Scc
//
template<typename T, AddressMode DAM>
INLINE void Scc(M68K* z, unsigned cc, M68K::HAM<T, DAM> &dst)
{
 static_assert(std::is_same<T, uint8_t>::value, "Wrong type");

 T const result = z->TestCond(cc) ? ~(T)0 : 0;

 if(DAM == DATA_REG_DIR && result)
  z->timestamp += 2;

 dst.write(result);
}


//
// JSR
//
template<typename T, AddressMode TAM>
INLINE void JSR(M68K* z, M68K::HAM<T, TAM> &targ)
{
 z->Push_u32(z->PC);
 targ.jump();
}


//
// JMP
//
template<typename T, AddressMode TAM>
INLINE void JMP(M68K* z, M68K::HAM<T, TAM> &targ)
{
 (void)z; /* No class state read; targ.jump() goes through dst.zptr. */
 targ.jump();
}


//
// MOVE from SR
//
template <typename T, AddressMode DAM>
INLINE void MOVE_from_SR(M68K* z, M68K::HAM<T, DAM> &dst)
{
 static_assert(std::is_same<T, uint16_t>::value, "Wrong type");

 dst.read();

 if(DAM == DATA_REG_DIR)
  z->timestamp += 2;

 dst.write(z->GetSR());
}


//
// MOVE to CCR
//
template<typename T, AddressMode SAM>
INLINE void MOVE_to_CCR(M68K* z, M68K::HAM<T, SAM> &src)
{
 static_assert(std::is_same<T, uint16_t>::value, "Wrong type");

 z->SetCCR(src.read());

 z->timestamp += 8;
}

//
// MOVE to SR
//
template<typename T, AddressMode SAM>
INLINE void MOVE_to_SR(M68K* z, M68K::HAM<T, SAM> &src)
{
 static_assert(std::is_same<T, uint16_t>::value, "Wrong type");

 z->SetSR(src.read());

 z->timestamp += 8;
}


//
// MOVE to/from USP
//
INLINE void M68K::MOVE_USP(bool direction, const unsigned ar)
{
 if(!direction)
  SP_Inactive = A[ar];
 else
  A[ar] = SP_Inactive;
}


//
// LEA
//
template<typename T, AddressMode SAM>
INLINE void LEA(M68K* z, M68K::HAM<T, SAM> &src, const unsigned ar)
{
 const uint32_t ea = src.getea();

 z->A[ar] = ea;
}


//
// PEA
//
template<typename T, AddressMode SAM>
INLINE void PEA(M68K* z, M68K::HAM<T, SAM> &src)
{
 const uint32_t ea = src.getea();

 z->Push_u32(ea);
}

//
// UNLK
//
INLINE void M68K::UNLK(const unsigned ar)
{
 A[7] = A[ar];
 A[ar] = Pull_u32();
}


//
// LINK
//
INLINE void M68K::LINK(const unsigned ar)
{
 const uint32_t disp = (int16_t)ReadOp();

 Push_u32(A[ar]);
 A[ar] = A[7];
 A[7] += disp;
}





//
// RTE
//
INLINE void M68K::RTE(void)
{
 uint16_t new_SR;

 new_SR = Pull_u16();
 PC = Pull_u32();

 SetSR(new_SR);
}


//
// RTR
//
INLINE void M68K::RTR(void)
{
 SetCCR(Pull_u16());
 PC = Pull_u32();
}


//
// RTS
//
INLINE void M68K::RTS(void)
{
 PC = Pull_u32();
}


//
// TRAP
//
INLINE void M68K::TRAP(const unsigned vf)
{
 Exception(EXCEPTION_TRAP, VECNUM_TRAP_BASE + vf);
}


//
// TRAPV
//
INLINE void M68K::TRAPV(void)
{
 if(GetV())
  Exception(EXCEPTION_TRAPV, VECNUM_TRAPV);
}


//
// ILLEGAL
//
INLINE void M68K::ILLEGAL(const uint16_t instr)
{
 PC -= 2;
 Exception(EXCEPTION_ILLEGAL, VECNUM_ILLEGAL);
}


INLINE void M68K::LINEA(void)
{
 PC -= 2;
 Exception(EXCEPTION_ILLEGAL, VECNUM_LINEA);
}

INLINE void M68K::LINEF(void)
{
 PC -= 2;
 Exception(EXCEPTION_ILLEGAL, VECNUM_LINEF);
}


//
// NOP
//
INLINE void M68K::NOP(void)
{

}


//
// RESET
//
INLINE void M68K::RESET(void)
{
 timestamp += 2;
 //
 BusRESET(true);
 timestamp += 124;
 BusRESET(false);
 //
 timestamp += 2;
}


//
// STOP
//
INLINE void M68K::STOP(void)
{
 uint16_t new_SR = ReadOp();

 SetSR(new_SR);
 XPending |= XPENDING_MASK_STOPPED;
}


INLINE bool M68K::CheckPrivilege(void)
{
 if(MDFN_UNLIKELY(!GetSVisor()))
 {
  PC -= 2;
  Exception(EXCEPTION_PRIVILEGE, VECNUM_PRIVILEGE);
  return false;
 }

 return true;
}

/* Phase-9d-15a: HAM detempleting via preprocessor monomorphization.
 * The 36 (T, AddressMode) HAM combinations actually used in
 * m68k_instr*.inc are materialised here as concrete C structs
 * named M68K_HAM_<TSIZE>_<AM> plus a set of static inline functions
 * (init_self, init_arg, calcea, read, write, rmw, jump, getea).
 *
 * The macro-instantiated bodies are byte-equivalent to the C++
 * template specialisations of M68K::HAM<T, am> -- verified by
 * disassembly comparison.  Nothing references the macro instances
 * yet; this commit lays the infrastructure.  Subsequent commits
 * detemplate the 50 op templates the same way and switch the
 * m68k_instr*.inc call sites over. */
#include "m68k_ham_instances.inc.h"

#endif
