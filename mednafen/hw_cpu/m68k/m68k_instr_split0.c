#include "../../mednafen.h"
#include "m68k.h"
#include "m68k_private.h"

void M68K_RunSplit0(M68K* z, uint16_t instr, const unsigned instr_b11_b9, const unsigned instr_b2_b0)
{
 switch(instr)
 {
  default: ILLEGAL(z, instr); break;
#include "m68k_instr_split0.inc"
 }
}
