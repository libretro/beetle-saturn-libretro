#include <mednafen/mednafen.h>
#include "m68k.h"
#include "m68k_private.h"

void M68K::RunSplit1(uint16 instr, const unsigned instr_b11_b9, const unsigned instr_b2_b0)
{
 switch(instr)
 {
  default: ILLEGAL(instr); break;
#include "m68k_instr_split1.inc"
 }
}
