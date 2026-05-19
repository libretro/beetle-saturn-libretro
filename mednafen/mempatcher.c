/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <compat/msvc.h>
#endif

#include <boolean.h>
#include <libretro.h>

#include "settings.h"
#include "mempatcher.h"

extern retro_log_printf_t log_cb;

static uint8_t **RAMPtrs = NULL;
static uint32_t PageSize;
static uint32_t NumPages;

typedef struct __CHEATF
{
           char *name;
           char *conditions;

           uint32_t addr;
           uint64_t val;
           uint64_t compare;

           unsigned int length;
           bool bigendian;
           unsigned int icount; // Instance count
           char type;   /* 'R' for replace, 'S' for substitute(GG), 'C' for substitute with compare */
           int status;

           /* libretro frontend slot owning this entry.  Multiple
            * CHEATF rows may share the same frontend_slot when a
            * single retro_cheat_set call decoded into more than one
            * write operation (a '+'-joined multi-line code).  Used
            * by MDFNMP_SetCheat to in-place compact away the prior
            * occupants of a slot before appending the new ones. */
           unsigned int frontend_slot;
} CHEATF;

/* cheats: was std::vector<CHEATF>. Plain grow-on-append array; CHEATF
 * is POD so realloc-based growth is fine. */
static CHEATF  *cheats       = NULL;
static size_t   cheats_count = 0;
static size_t   cheats_cap   = 0;

static int savecheats;
static uint32_t resultsbytelen = 1;
static bool resultsbigendian = 0;
/* CheatsActive is recomputed by MDFNMP_RecomputeCheatsActive any time
 * the cheats[] array gains/loses an enabled entry (SetCheat, Flush).
 * Starts false: at MDFNMP_Init time the cheats[] array is empty. */
static bool CheatsActive = false;

/* SubCheats[8]: was std::vector<SUBCHEAT>[8]. Eight independent
 * grow-on-append arrays; SUBCHEAT is POD. */
static bool      SubCheatsOn         = 0;
static SUBCHEAT *SubCheats[8]        = { NULL };
static size_t    SubCheats_count[8]  = { 0 };
static size_t    SubCheats_cap[8]    = { 0 };

static void RebuildSubCheats(void)
{
 size_t ci;
 int x;

 SubCheatsOn = 0;
 for(x = 0; x < 8; x++)
  SubCheats_count[x] = 0;

 if(!CheatsActive) return;

 for(ci = 0; ci < cheats_count; ci++)
 {
  CHEATF *chit = &cheats[ci];
  if(chit->status && chit->type != 'R')
  {
   unsigned int x;
   for(x = 0; x < chit->length; x++)
   {
    SUBCHEAT tmpsub;
    unsigned int shiftie;
    unsigned int bucket;

    if(chit->bigendian)
     shiftie = (chit->length - 1 - x) * 8;
    else
     shiftie = x * 8;

    tmpsub.addr = chit->addr + x;
    tmpsub.value = (chit->val >> shiftie) & 0xFF;
    if(chit->type == 'C')
     tmpsub.compare = (chit->compare >> shiftie) & 0xFF;
    else
     tmpsub.compare = -1;

    bucket = (chit->addr + x) & 0x7;
    if(SubCheats_count[bucket] >= SubCheats_cap[bucket])
    {
     size_t newcap = SubCheats_cap[bucket] ? SubCheats_cap[bucket] * 2 : 8;
     SUBCHEAT *np  = (SUBCHEAT *)realloc(SubCheats[bucket], newcap * sizeof(SUBCHEAT));
     if(!np)
      return;
     SubCheats[bucket]     = np;
     SubCheats_cap[bucket] = newcap;
    }
    SubCheats[bucket][SubCheats_count[bucket]++] = tmpsub;
    SubCheatsOn = 1;
   }
  }
 }
}

bool MDFNMP_Init(uint32_t ps, uint32_t numpages)
{
 PageSize = ps;
 NumPages = numpages;

 RAMPtrs = (uint8_t **)calloc(numpages, sizeof(uint8_t *));
 if (!RAMPtrs)
 {
  /* Pre-conversion C++ used `new uint8_t*[]` which threw
   * std::bad_alloc on OOM; the conversion to calloc dropped
   * the check.  The allocation is small (~tens of pointers
   * in practice), so failure is unlikely on any realistic
   * target, but the bool return is meaningful: ss.c's
   * InitFastMemMap propagates it (see the `if(!MDFNMP_Init(...))
   * return false;` at ss.c:1241), and the downstream
   * MDFNMP_AddRAM / MDFNMP_ApplyPeriodicCheats paths -- which
   * dereference RAMPtrs unconditionally -- are therefore
   * unreachable on the failure path. */
  return false;
 }

 /* CheatsActive starts false (file-scope default); it flips to true
  * once the libretro frontend pushes at least one enabled cheat via
  * retro_cheat_set -> MDFNMP_SetCheat -> MDFNMP_RecomputeCheatsActive. */
 return true;
}

void MDFNMP_Kill(void)
{
   unsigned int x;
   size_t ci;

   if(RAMPtrs)
   {
      free(RAMPtrs);
      RAMPtrs = NULL;
   }
   NumPages = 0;
   PageSize = 0;

   /* Free per-cheat strings.  MDFN_FlushGameCheats normally does this
    * on the retro_unload_game path before MDFNMP_Kill is reached,
    * setting cheats_count to 0 -- so the loop below is a no-op in
    * that flow.  Defending against Kill being called without a
    * preceding Flush (another caller in the future, refactor that
    * drops the Flush, etc.) so we never leak the name / conditions
    * strings. */
   for(ci = 0; ci < cheats_count; ci++)
   {
      free(cheats[ci].name);
      free(cheats[ci].conditions); /* free(NULL) is well-defined */
   }
   /* Free the CHEATF backing array itself, which Flush deliberately
    * keeps allocated to preserve capacity across game loads.  Without
    * this, the realloc'd array survives every unload and is only
    * reclaimed at process exit by the OS (a valgrind-class leak, not
    * a runtime-pressure leak, but still part of "Kill should undo
    * everything Init / runtime mutation accumulated"). */
   free(cheats);
   cheats       = NULL;
   cheats_count = 0;
   cheats_cap   = 0;

   /* Same treatment for the eight SubCheats buckets.  RebuildSubCheats
    * (called from Flush) only resets the counts -- the realloc'd
    * SUBCHEAT* arrays survive until here. */
   for(x = 0; x < 8; x++)
   {
      free(SubCheats[x]);
      SubCheats[x]       = NULL;
      SubCheats_count[x] = 0;
      SubCheats_cap[x]   = 0;
   }
}

void MDFNMP_AddRAM(uint32_t size, uint32_t A, uint8_t *RAM)
{
 uint32_t AB = A / PageSize;
 unsigned int x;

 size /= PageSize;

 for(x = 0; x < size; x++)
 {
  RAMPtrs[AB + x] = RAM;
  if(RAM) // Don't increment the RAM pointer if we're passed a NULL pointer
   RAM += PageSize;
 }
}

static void MDFNMP_RecomputeCheatsActive(void)
{
 size_t ci;
 CheatsActive = false;
 for(ci = 0; ci < cheats_count; ci++)
 {
  if(cheats[ci].status)
  {
   CheatsActive = true;
   break;
  }
 }
 /* RebuildSubCheats early-returns if CheatsActive is false, so it
  * also serves to clear the per-bucket SubCheats arrays when the
  * last enabled cheat was just disabled or flushed. */
 RebuildSubCheats();
}

/* Compact-remove every cheats[] entry whose frontend_slot == slot,
 * preserving the relative order of the surviving entries.  Freed
 * strings are released before the slot's row is overwritten by the
 * shift.  Called from MDFNMP_SetCheat ahead of appending new ops.
 *
 * O(N) per call where N = cheats_count.  For the typical libretro
 * cheat list (a handful of slots), N is small enough that the
 * single-pass scan is the right tradeoff against a more complex
 * slot->indices index. */
static void mdfnmp_drop_slot(unsigned slot)
{
 size_t i, j = 0;
 for(i = 0; i < cheats_count; i++)
 {
  if(cheats[i].frontend_slot == slot)
  {
   free(cheats[i].name);
   free(cheats[i].conditions); /* free(NULL) is well-defined */
  }
  else
  {
   if(i != j)
    cheats[j] = cheats[i];
   j++;
  }
 }
 cheats_count = j;
}

/* Replace the set of cheats associated with libretro frontend slot
 * `slot` with the `op_count` operations in `ops[]`.  All ops share
 * the single `enabled` flag (libretro's API only carries one
 * enable bit per slot).  Each op becomes its own CHEATF row of
 * type 'R' with frontend_slot == slot so the next SetCheat or
 * Flush can find and remove them together.
 *
 * op_count == 0 is valid and means "drop this slot's entries
 * without adding any" -- useful when the parser couldn't decode
 * the new code but the frontend still expects the prior cheats at
 * `slot` to be gone.
 *
 * CheatsActive is recomputed at the tail. */
void MDFNMP_SetCheat(unsigned slot, bool enabled,
                     const MDFNCheatOp *ops, size_t op_count)
{
 size_t i;

 mdfnmp_drop_slot(slot);

 if(cheats_count + op_count > cheats_cap)
 {
  size_t newcap = cheats_cap ? cheats_cap : 16;
  CHEATF *np;
  while(newcap < cheats_count + op_count)
   newcap *= 2;
  np = (CHEATF *)realloc(cheats, newcap * sizeof(CHEATF));
  if(!np)
  {
   /* Slot's prior entries are already gone; appending the new
    * ones failed, so the slot ends up empty.  RecomputeCheatsActive
    * below handles that consistently. */
   MDFNMP_RecomputeCheatsActive();
   return;
  }
  cheats = np;
  cheats_cap = newcap;
 }

 for(i = 0; i < op_count; i++)
 {
  CHEATF *chit = &cheats[cheats_count++];
  memset(chit, 0, sizeof(*chit));
  chit->frontend_slot = slot;
  chit->addr          = ops[i].addr;
  chit->val           = ops[i].val;
  chit->length        = ops[i].length;
  chit->bigendian     = ops[i].bigendian;
  chit->type          = 'R';
  chit->status        = enabled ? 1 : 0;
  /* Name is purely diagnostic in this fork; strdup failure leaves
   * name=NULL which Flush / Kill handle via free(NULL)-is-fine. */
  chit->name          = strdup("retro");
 }

 MDFNMP_RecomputeCheatsActive();
}

void MDFN_LoadGameCheats(void)
{
 RebuildSubCheats();
}

void MDFN_FlushGameCheats(void)
{
   size_t ci;

   for(ci = 0; ci < cheats_count; ci++)
   {
      free(cheats[ci].name);
      if(cheats[ci].conditions)
         free(cheats[ci].conditions);
   }
   cheats_count = 0;

   /* RecomputeCheatsActive walks the (now-empty) cheats[] and sets
    * CheatsActive = false, then calls RebuildSubCheats to clear the
    * per-bucket arrays.  Reset path: retro_cheat_reset hits this. */
   MDFNMP_RecomputeCheatsActive();
}

/*
 Condition format(ws = white space):
 
  <variable size><ws><endian><ws><address><ws><operation><ws><value>
	  [,second condition...etc.]

  Value should be unsigned integer, hex(with a 0x prefix) or
  base-10.  

  Operations:
   >=
   <=
   >
   <
   ==
   !=
   &	// Result of AND between two values is nonzero
   !&   // Result of AND between two values is zero
   ^    // same, XOR
   !^
   |	// same, OR
   !|

  Full example:

  2 L 0xADDE == 0xDEAD, 1 L 0xC000 == 0xA0

*/

static bool TestConditions(const char *string)
{
 char address[64];
 char operation[64];
 char value[64];
 char endian;
 unsigned int bytelen;
 bool passed = 1;

 /* Inline replacement for trio_sscanf(string, "%u %c %63s %63s %63s", ...).
  * sscanf is dragged in by libc even for a single call site; doing
  * the parse by hand drops the dependency on this TU.  Field shape
  * mirrors the original format string:
  *     %u    -> bytelen     (leading whitespace skipped)
  *     %c    -> endian      (single char after whitespace)
  *     %63s  -> address
  *     %63s  -> operation
  *     %63s  -> value
  * On any field failing to parse, jump to the `done:` label and
  * return the current `passed` value (mirrors the pre-fix
  * 'sscanf == 5' loop guard).  The original advancement to the
  * next condition in a comma-separated conditions list is preserved
  * at the bottom of the loop body via strchr(string, ','). */
 while (passed)
 {
  const char *p = string;
  char *e;
  unsigned long ul;
  int field;

  /* whitespace, then %u */
  while (*p == ' ' || *p == '\t') p++;
  ul = strtoul(p, &e, 10);
  if (e == p) break;
  bytelen = (unsigned int)ul;
  p = e;

  /* whitespace, then %c */
  while (*p == ' ' || *p == '\t') p++;
  if (!*p) break;
  endian = *p++;

  /* three %63s fields */
  for (field = 0; field < 3; field++)
  {
   char  *dst    = (field == 0) ? address : (field == 1) ? operation : value;
   size_t n      = 0;
   while (*p == ' ' || *p == '\t') p++;
   while (p[n] && p[n] != ' ' && p[n] != '\t' && n < 63) n++;
   if (n == 0) goto done;
   memcpy(dst, p, n);
   dst[n] = 0;
   p += n;
  }

  {
  uint32_t v_address;
  uint64_t v_value;
  uint64_t value_at_address;

  if(address[0] == '0' && address[1] == 'x')
   v_address = strtoul(address + 2, NULL, 16);
  else
   v_address = strtoul(address, NULL, 10);

  if(value[0] == '0' && value[1] == 'x')
   v_value = strtoull(value + 2, NULL, 16);
  else
   v_value = strtoull(value, NULL, 0);

  value_at_address = 0;

#if 0
  {
   unsigned int x;
   for(x = 0; x < bytelen; x++)
   {
    unsigned int shiftie;

    if(endian == 'B')
     shiftie = (bytelen - 1 - x) * 8;
    else
     shiftie = x * 8;
    value_at_address |= MDFNGameInfo->MemRead(v_address + x) << shiftie;
   }
  }
#endif

  if(!strcmp(operation, ">="))
  {
   if(!(value_at_address >= v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "<="))
  {
   if(!(value_at_address <= v_value))
    passed = 0;
  }
  else if(!strcmp(operation, ">"))
  {
   if(!(value_at_address > v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "<"))
  {
   if(!(value_at_address < v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "==")) 
  {
   if(!(value_at_address == v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "!="))
  {
   if(!(value_at_address != v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "&"))
  {
   if(!(value_at_address & v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "!&"))
  {
   if(value_at_address & v_value)
    passed = 0;
  }
  else if(!strcmp(operation, "^"))
  {
   if(!(value_at_address ^ v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "!^"))
  {
   if(value_at_address ^ v_value)
    passed = 0;
  }
  else if(!strcmp(operation, "|"))
  {
   if(!(value_at_address | v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "!|"))
  {
   if(value_at_address | v_value)
    passed = 0;
  }
  } /* end inner-scope block for v_address / v_value / value_at_address */

  /* Advance past the comma to the next condition (a comma-separated
   * conditions list; the original code's advancement, preserved). */
  string = strchr(string, ',');
  if(string == NULL)
   break;
  else
   string++;
 }

done:
 return(passed);
}

void MDFNMP_ApplyPeriodicCheats(void)
{
   size_t ci;

   if(!CheatsActive)
      return;

   for(ci = 0; ci < cheats_count; ci++)
   {
      CHEATF *chit = &cheats[ci];
      if(chit->status && chit->type == 'R')
      {
         unsigned int x;
         if(!chit->conditions || TestConditions(chit->conditions))
            for(x = 0; x < chit->length; x++)
            {
               uint32_t page;
               uint32_t byte_off;
               uint64_t tmpval = chit->val;

               if(chit->bigendian)
                  tmpval >>= (chit->length - 1 - x) * 8;
               else
                  tmpval >>= x * 8;

               page     = ((chit->addr + x) / PageSize) % NumPages;
               byte_off = (chit->addr + x) % PageSize;

               /* RAM regions registered via MDFNMP_AddRAM are passed
                * as `uint8_t*` but the Saturn's WorkRAM is actually
                * stored as `uint16_t[]` in host byte order so that
                * the SH-2 fast-map can do native 16-bit loads.  The
                * SH-2's byte-access macro SH7095_RBO_BE_U8 (see
                * sh7095.inc) compensates for this on LE hosts by
                * XOR-ing the byte address with 1 to address the
                * Saturn-BE byte within each LE-stored uint16_t.
                *
                * Cheats target Saturn-conceived byte addresses, so
                * apply the same swizzle here on LE builds.  On BE
                * builds the storage matches Saturn order and no
                * swizzle is needed.
                *
                * Caveat for future regions: any RAM passed to
                * MDFNMP_AddRAM that is genuinely a byte array
                * (not a uint16_t[]) would need a per-region opt-out
                * here.  All current Saturn regions are uint16_t[],
                * so a single global gate suffices for now. */
#ifndef MSB_FIRST
               byte_off ^= 1;
#endif

               if(RAMPtrs[page])
                  RAMPtrs[page][byte_off] = (uint8_t)tmpval;
            }
      }
   }
}
