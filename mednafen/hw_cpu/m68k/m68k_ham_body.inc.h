/* m68k_ham_body.inc.h
 *
 * Phase-9d-15: HAM detempleting via preprocessor monomorphization.
 *
 * This file is the macro-parameterised body of what used to be the
 * `template<typename T, AddressMode am> struct M68K::HAM` class
 * template.  It's #included multiple times from m68k_private.h
 * (or wherever) with HAM_NAME / HAM_T / HAM_TSIZE / HAM_AM set
 * differently each time, producing one concrete struct + function
 * set per (T, am) combination.
 *
 * Because HAM_T and HAM_AM are macro-substituted to concrete
 * literals at each instantiation, every `if (HAM_AM == X)` and
 * `sizeof(HAM_T)` collapses to a compile-time constant inside the
 * generated function body, and the compiler dead-code-eliminates
 * the non-matching arms.  This produces machine code identical to
 * the C++ template specialization that was used pre-9d-15.
 *
 * Required macros at instantiation:
 *   HAM_NAME    - the struct's concrete name (e.g. M68K_HAM_u16_ABS_LONG)
 *   HAM_T       - the value type (uint8_t / uint16_t / uint32_t / void)
 *   HAM_TSIZE   - sizeof(HAM_T) as integer literal (1 / 2 / 4 / 0)
 *   HAM_AM      - the AddressMode literal (ABS_LONG / DATA_REG_DIR / ...)
 *
 * The header undefs everything at the end so re-inclusion is clean.
 */

#ifndef HAM_NAME
#error "HAM_NAME must be defined before including m68k_ham_body.inc.h"
#endif
#ifndef HAM_T
#error "HAM_T must be defined before including m68k_ham_body.inc.h"
#endif
#ifndef HAM_TSIZE
#error "HAM_TSIZE must be defined before including m68k_ham_body.inc.h"
#endif
#ifndef HAM_AM
#error "HAM_AM must be defined before including m68k_ham_body.inc.h"
#endif

/* Token-paste helpers.  Two levels because ## suppresses macro
 * expansion of its operands, so we need an indirection. */
#define HAM_CAT2(a, b) a ## b
#define HAM_CAT(a, b) HAM_CAT2(a, b)
#define HAM_FN(s) HAM_CAT(HAM_NAME, s)

struct HAM_NAME
{
 M68K* zptr;
 uint32_t ea;
 uint32_t ext;
 unsigned reg;
 bool have_ea;
};

/* Forward decl of calcea (it's the "private" helper called by
 * read/write/getea/jump).  GCC sees the static-inline body below
 * at every call site within this TU's worth of HAM_NAME functions
 * and inlines it. */
static MDFN_FORCE_INLINE void HAM_FN(_calcea) (struct HAM_NAME* h, const int predec_penalty);

/* ctor #1: single-arg "from PC/instruction stream" */
static MDFN_FORCE_INLINE void HAM_FN(_init_self) (struct HAM_NAME* h, M68K* z)
{
 h->zptr = z;
 h->reg = 0;
 h->have_ea = false;

 /* HAM_AM and HAM_TSIZE are compile-time constants here.  Only
  * the matching arm survives optimization. */
 if (HAM_AM == PC_DISP || HAM_AM == PC_INDEX)
 {
  h->ea = z->PC;
  h->ext = ReadOp(z);
 }
 else if (HAM_AM == ABS_SHORT)
 {
  h->ext = ReadOp(z);
 }
 else if (HAM_AM == ABS_LONG)
 {
  h->ext = ReadOp(z) << 16;
  h->ext |= ReadOp(z);
 }
 else if (HAM_AM == IMMEDIATE)
 {
  if (HAM_TSIZE == 4)
  {
   h->ext = ReadOp(z) << 16;
   h->ext |= ReadOp(z);
  }
  else
  {
   h->ext = ReadOp(z);
  }
 }
 /* else: nothing extra to read; the unguarded ext/ea will be set
  * by init_arg or calcea later. */
}

/* ctor #2: two-arg "from register field + optional displacement" */
static MDFN_FORCE_INLINE void HAM_FN(_init_arg) (struct HAM_NAME* h, M68K* z, uint32_t arg)
{
 h->zptr = z;
 h->reg = arg;
 h->have_ea = false;

 if (HAM_AM == ADDR_REG_INDIR_DISP || HAM_AM == ADDR_REG_INDIR_INDX)
  h->ext = ReadOp(z);
 else if (HAM_AM == IMMEDIATE)
  h->ext = arg;
 /* DATA_REG_DIR / ADDR_REG_DIR / ADDR_REG_INDIR / ADDR_REG_INDIR_POST
  * / ADDR_REG_INDIR_PRE: no extra setup. */
}

/* Compute effective address.  Cached after first call. */
static MDFN_FORCE_INLINE void HAM_FN(_calcea) (struct HAM_NAME* h, const int predec_penalty)
{
 if (h->have_ea)
  return;
 h->have_ea = true;

 if (HAM_AM == ADDR_REG_INDIR)
 {
  h->ea = h->zptr->A[h->reg];
 }
 else if (HAM_AM == ADDR_REG_INDIR_POST)
 {
  h->ea = h->zptr->A[h->reg];
  h->zptr->A[h->reg] += (HAM_TSIZE == 1 && h->reg == 0x7) ? 2 : HAM_TSIZE;
 }
 else if (HAM_AM == ADDR_REG_INDIR_PRE)
 {
  h->zptr->timestamp += predec_penalty;
  h->zptr->A[h->reg] -= (HAM_TSIZE == 1 && h->reg == 0x7) ? 2 : HAM_TSIZE;
  h->ea = h->zptr->A[h->reg];
 }
 else if (HAM_AM == ADDR_REG_INDIR_DISP)
 {
  h->ea = h->zptr->A[h->reg] + (int16_t)h->ext;
 }
 else if (HAM_AM == ADDR_REG_INDIR_INDX)
 {
  h->zptr->timestamp += 2;
  h->ea = h->zptr->A[h->reg] + (int8_t)h->ext + ((h->ext & 0x800) ? h->zptr->DA[h->ext >> 12] : (int16_t)h->zptr->DA[h->ext >> 12]);
 }
 else if (HAM_AM == ABS_SHORT)
 {
  h->ea = (int16_t)h->ext;
 }
 else if (HAM_AM == ABS_LONG)
 {
  h->ea = h->ext;
 }
 else if (HAM_AM == PC_DISP)
 {
  h->ea = h->ea + (int16_t)h->ext;
 }
 else if (HAM_AM == PC_INDEX)
 {
  h->zptr->timestamp += 2;
  h->ea = h->ea + (int8_t)h->ext + ((h->ext & 0x800) ? h->zptr->DA[h->ext >> 12] : (int16_t)h->zptr->DA[h->ext >> 12]);
 }
 /* DATA_REG_DIR / ADDR_REG_DIR / IMMEDIATE have no ea. */
}

/* write — only valid for non-PC, non-IMMEDIATE modes.  HAM_T must
 * be a real value type, not void.  Skip generation when HAM_TSIZE
 * is 0 (the std::tuple<> case used by LEA/PEA/JMP/JSR). */
#if HAM_TSIZE != 0
static MDFN_FORCE_INLINE void HAM_FN(_write) (struct HAM_NAME* h, HAM_T val, const int predec_penalty)
{
 if (HAM_AM == ADDR_REG_DIR)
 {
  h->zptr->A[h->reg] = val;
 }
 else if (HAM_AM == DATA_REG_DIR)
 {
#ifdef MSB_FIRST
  memcpy((uint8_t*)&h->zptr->D[h->reg] + (4 - HAM_TSIZE), &val, HAM_TSIZE);
#else
  memcpy((uint8_t*)&h->zptr->D[h->reg] + 0, &val, HAM_TSIZE);
#endif
 }
 else if (HAM_AM == ADDR_REG_INDIR || HAM_AM == ADDR_REG_INDIR_POST
       || HAM_AM == ADDR_REG_INDIR_PRE || HAM_AM == ADDR_REG_INDIR_DISP
       || HAM_AM == ADDR_REG_INDIR_INDX || HAM_AM == ABS_SHORT
       || HAM_AM == ABS_LONG)
 {
  HAM_FN(_calcea) (h, predec_penalty);
  if (HAM_TSIZE == 1)
   Write_u8(h->zptr, h->ea, val);
  else if (HAM_TSIZE == 2)
   Write_u16(h->zptr, h->ea, val);
  else if (HAM_AM == ADDR_REG_INDIR_PRE)
   Write_u32_longdec(h->zptr, h->ea, val);
  else
   Write_u32(h->zptr, h->ea, val);
 }
}

static MDFN_FORCE_INLINE HAM_T HAM_FN(_read) (struct HAM_NAME* h)
{
 if (HAM_AM == DATA_REG_DIR)
  return h->zptr->D[h->reg];
 else if (HAM_AM == ADDR_REG_DIR)
  return h->zptr->A[h->reg];
 else if (HAM_AM == IMMEDIATE)
  return h->ext;
 else
 {
  HAM_FN(_calcea) (h, 2);
  if (HAM_TSIZE == 1)
   return Read_u8(h->zptr, h->ea);
  else if (HAM_TSIZE == 2)
   return Read_u16(h->zptr, h->ea);
  else
   return Read_u32(h->zptr, h->ea);
 }
 /* Unreachable for valid HAM_AM. */
}

/* rmw — only valid for HAM_T==uint8_t per BusRMW's signature.
 * TAS is the only caller, and it asserts T==uint8_t. */
#if HAM_TSIZE == 1
static MDFN_FORCE_INLINE void HAM_FN(_rmw) (struct HAM_NAME* h, HAM_T (MDFN_FASTCALL *cb)(M68K*, HAM_T))
{
 if (HAM_AM == DATA_REG_DIR)
 {
  HAM_T tmp = cb(h->zptr, h->zptr->D[h->reg]);
#ifdef MSB_FIRST
  memcpy((uint8_t*)&h->zptr->D[h->reg] + (4 - HAM_TSIZE), &tmp, HAM_TSIZE);
#else
  memcpy((uint8_t*)&h->zptr->D[h->reg] + 0, &tmp, HAM_TSIZE);
#endif
 }
 else
 {
  HAM_FN(_calcea) (h, 2);
  h->zptr->BusRMW(h->ea, cb);
 }
}
#endif /* HAM_TSIZE == 1 */
#endif /* HAM_TSIZE != 0 */

/* jump / getea are valid even for HAM_TSIZE==0 (the addressing-mode-
 * only forms used by LEA/PEA/JMP/JSR). */
static MDFN_FORCE_INLINE void HAM_FN(_jump) (struct HAM_NAME* h)
{
 HAM_FN(_calcea) (h, 0);
 h->zptr->PC = h->ea;
}

static MDFN_FORCE_INLINE uint32_t HAM_FN(_getea) (struct HAM_NAME* h)
{
 HAM_FN(_calcea) (h, 0);
 return h->ea;
}

/* Cleanup so the file can be re-included for the next instantiation. */
#undef HAM_NAME
#undef HAM_T
#undef HAM_TSIZE
#undef HAM_AM
#undef HAM_CAT2
#undef HAM_CAT
#undef HAM_FN
