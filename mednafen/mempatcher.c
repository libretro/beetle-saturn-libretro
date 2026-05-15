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
#include "settings-common.h"
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
} CHEATF;

/* cheats: was std::vector<CHEATF>. Plain grow-on-append array; CHEATF
 * is POD so realloc-based growth is fine. */
static CHEATF  *cheats       = NULL;
static size_t   cheats_count = 0;
static size_t   cheats_cap   = 0;

static int savecheats;
static uint32_t resultsbytelen = 1;
static bool resultsbigendian = 0;
static bool CheatsActive = true;

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
   * target, but the bool return is now meaningful for any
   * future caller-side propagation.  The current caller
   * (ss.cpp's InitFastMemMap, which is void-returning) does
   * not yet check this; if RAMPtrs is NULL, the subsequent
   * MDFNMP_AddRAM calls will NULL-deref. */
  return false;
 }

 CheatsActive = MDFN_GetSettingB("cheats");
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

void MDFNMP_RegSearchable(uint32_t addr, uint32_t size)
{
 MDFNMP_AddRAM(size, addr, NULL);
}

void MDFNMP_InstallReadPatches(void)
{
 if(!CheatsActive) return;

#if 0
 {
  unsigned int x;
  size_t ci;
  for(x = 0; x < 8; x++)
   for(ci = 0; ci < SubCheats_count[x]; ci++)
   {
    SUBCHEAT *chit = &SubCheats[x][ci];
    if(MDFNGameInfo->InstallReadPatch)
     MDFNGameInfo->InstallReadPatch(chit->addr);
   }
 }
#endif
}

void MDFNMP_RemoveReadPatches(void)
{
#if 0
 if(MDFNGameInfo->RemoveReadPatches)
  MDFNGameInfo->RemoveReadPatches();
#endif
}

/* This function doesn't allocate any memory for "name" */
static int AddCheatEntry(char *name, char *conditions, uint32_t addr, uint64_t val, uint64_t compare, int status, char type, unsigned int length, bool bigendian)
{
 CHEATF temp;

 memset(&temp, 0, sizeof(CHEATF));

 temp.name=name;
 temp.conditions = conditions;
 temp.addr=addr;
 temp.val=val;
 temp.status=status;
 temp.compare=compare;
 temp.length = length;
 temp.bigendian = bigendian;
 temp.type=type;

 if(cheats_count >= cheats_cap)
 {
  size_t newcap = cheats_cap ? cheats_cap * 2 : 8;
  CHEATF *np    = (CHEATF *)realloc(cheats, newcap * sizeof(CHEATF));
  if(!np)
   return(0);
  cheats     = np;
  cheats_cap = newcap;
 }
 cheats[cheats_count++] = temp;
 return(1);
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

   RebuildSubCheats();
}

int MDFNI_AddCheat(const char *name, uint32_t addr, uint64_t val, uint64_t compare, char type, unsigned int length, bool bigendian)
{
 char *t;

 if(!(t = strdup(name)))
  return(0);

 if(!AddCheatEntry(t, NULL, addr,val,compare,1,type, length, bigendian))
 {
  free(t);
  return(0);
 }

 savecheats = 1;

 MDFNMP_RemoveReadPatches();
 RebuildSubCheats();
 MDFNMP_InstallReadPatches();

 return(1);
}

int MDFNI_DelCheat(uint32_t which)
{
 free(cheats[which].name);
 /* erase element 'which': shift the tail down one slot. */
 if((size_t)which + 1 < cheats_count)
  memmove(&cheats[which], &cheats[which + 1],
          (cheats_count - which - 1) * sizeof(CHEATF));
 cheats_count--;

 savecheats=1;

 MDFNMP_RemoveReadPatches();
 RebuildSubCheats();
 MDFNMP_InstallReadPatches();

 return(1);
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

 while(sscanf(string, "%u %c %63s %63s %63s", &bytelen, &endian, address, operation, value) == 5 && passed)
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
  string = strchr(string, ',');
  if(string == NULL)
   break;
  else
   string++;
 }

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
               uint32_t page = ((chit->addr + x) / PageSize) % NumPages;
               if(RAMPtrs[page])
               {
                  uint64_t tmpval = chit->val;

                  if(chit->bigendian)
                     tmpval >>= (chit->length - 1 - x) * 8;
                  else
                     tmpval >>= x * 8;

                  RAMPtrs[page][(chit->addr + x) % PageSize] = tmpval;
               }
            }
      }
   }
}


void MDFNI_ListCheats(int (*callb)(char *name, uint32_t a, uint64_t v, uint64_t compare, int s, char type, unsigned int length, bool bigendian, void *data), void *data)
{
 size_t ci;

 for(ci = 0; ci < cheats_count; ci++)
 {
  CHEATF *chit = &cheats[ci];
  if(!callb(chit->name, chit->addr, chit->val, chit->compare, chit->status, chit->type, chit->length, chit->bigendian, data)) break;
 }
}

int MDFNI_GetCheat(uint32_t which, char **name, uint32_t *a, uint64_t *v, uint64_t *compare, int *s, char *type, unsigned int *length, bool *bigendian)
{
 CHEATF *next = &cheats[which];

 if(name)
  *name=next->name;
 if(a)
  *a=next->addr; 
 if(v)
  *v=next->val;
 if(s)
  *s=next->status;
 if(compare)
  *compare=next->compare;
 if(type)
  *type=next->type;
 if(length)
  *length = next->length;
 if(bigendian)
  *bigendian = next->bigendian;
 return(1);
}

static uint8_t CharToNibble(char thechar)
{
 const char lut[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
 int x;

 thechar = toupper(thechar);

 for(x = 0; x < 16; x++)
  if(lut[x] == thechar)
   return(x);

 return(0xFF);
}

bool MDFNI_DecodeGBGG(const char *instr, uint32_t *a, uint8_t *v, uint8_t *c, char *type)
{
 char str[10];
 int len;
 int x;
 uint32_t tmp_address;
 uint8_t tmp_value;
 uint8_t tmp_compare = 0;

 for(x = 0; x < 9; x++)
 {
  while(*instr && CharToNibble(*instr) == 255)
   instr++;
  if(!(str[x] = *instr)) break;
  instr++;
 }
 str[9] = 0;

 len = strlen(str);

 if(len != 9 && len != 6)
  return(0);

 tmp_address =  (CharToNibble(str[5]) << 12) | (CharToNibble(str[2]) << 8) | (CharToNibble(str[3]) << 4) | (CharToNibble(str[4]) << 0);
 tmp_address ^= 0xF000;
 tmp_value = (CharToNibble(str[0]) << 4) | (CharToNibble(str[1]) << 0);

 if(len == 9)
 {
  tmp_compare = (CharToNibble(str[6]) << 4) | (CharToNibble(str[8]) << 0);
  tmp_compare = (tmp_compare >> 2) | ((tmp_compare << 6) & 0xC0);
  tmp_compare ^= 0xBA;
 }

 *a = tmp_address;
 *v = tmp_value;

 if(len == 9)
 {
  *c = tmp_compare;
  *type = 'C';
 }
 else
 {
  *c = 0;
  *type = 'S';
 }

 return(1);
}

static int GGtobin(char c)
{
 static char lets[16]={'A','P','Z','L','G','I','T','Y','E','O','X','U','K','S','V','N'};
 int x;

 for(x=0;x<16;x++)
  if(lets[x] == toupper(c)) return(x);
 return(0);
}

/* Returns 1 on success, 0 on failure. Sets *a,*v,*c. */
int MDFNI_DecodeGG(const char *str, uint32_t *a, uint8_t *v, uint8_t *c, char *type)
{
   uint16_t A;
   uint8_t V,C;
   uint8_t t;
   int s;

   A=0x8000;
   V=0;
   C=0;

   s=strlen(str);
   if(s!=6 && s!=8) return(0);

   t=GGtobin(*str++);
   V|=(t&0x07);
   V|=(t&0x08)<<4;

   t=GGtobin(*str++);
   V|=(t&0x07)<<4;
   A|=(t&0x08)<<4;

   t=GGtobin(*str++);
   A|=(t&0x07)<<4;
   //if(t&0x08) return(0);	/* 8-character code?! */

   t=GGtobin(*str++);
   A|=(t&0x07)<<12;
   A|=(t&0x08);

   t=GGtobin(*str++);
   A|=(t&0x07);
   A|=(t&0x08)<<8;

   if(s==6)
   {
      t=GGtobin(*str++);
      A|=(t&0x07)<<8;
      V|=(t&0x08);

      *a=A;
      *v=V;
      *type = 'S';
      *c = 0;
   }
   else
   {
      t=GGtobin(*str++);
      A|=(t&0x07)<<8;
      C|=(t&0x08);

      t=GGtobin(*str++);
      C|=(t&0x07);
      C|=(t&0x08)<<4;

      t=GGtobin(*str++);
      C|=(t&0x07)<<4;
      V|=(t&0x08);
      *a=A;
      *v=V;
      *c=C;
      *type = 'C';
   }

   return(1);
}

int MDFNI_DecodePAR(const char *str, uint32_t *a, uint8_t *v, uint8_t *c, char *type)
{
 int boo[4];
 if(strlen(str)!=8) return(0);

 sscanf(str,"%02x%02x%02x%02x",boo,boo+1,boo+2,boo+3);

 *c = 0;

 if(1)
 {
  *a=(boo[3]<<8)|(boo[2]+0x7F);
  *v=0;
 }
 else
 {
  *v=boo[3];
  *a=boo[2]|(boo[1]<<8);
 }

 *type = 'S';
 return(1);
}

/* name can be NULL if the name isn't going to be changed. */
int MDFNI_SetCheat(uint32_t which, const char *name, uint32_t a, uint64_t v, uint64_t compare, int s, char type, unsigned int length, bool bigendian)
{
 CHEATF *next = &cheats[which];

 if(name)
 {
  char *t;

  if((t=(char *)realloc(next->name,strlen(name+1))))
  {
   next->name=t;
   strcpy(next->name,name);
  }
  else
   return(0);
 }
 next->addr=a;
 next->val=v;
 next->status=s;
 next->compare=compare;
 next->type=type;
 next->length = length;
 next->bigendian = bigendian;

 RebuildSubCheats();
 savecheats=1;

 return(1);
}

/* Convenience function. */
int MDFNI_ToggleCheat(uint32_t which)
{
 cheats[which].status = !cheats[which].status;
 savecheats = 1;
 RebuildSubCheats();

 return(cheats[which].status);
}

static void SettingChanged(const char *name)
{
 MDFNMP_RemoveReadPatches();

 CheatsActive = MDFN_GetSettingB("cheats");

 RebuildSubCheats();

 MDFNMP_InstallReadPatches();
}


MDFNSetting MDFNMP_Settings[] =
{
 { "cheats", MDFNSF_NOFLAGS, "Enable cheats.", NULL, MDFNST_BOOL, "1", NULL, NULL, NULL, SettingChanged },
 { NULL}
};
