/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scsp.inc - SCSP Emulation
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

/*
 TODO:
	Test the relative timing between timers, LFO, EG, and the DSP ringbuffer position.

	Open bus or something seems to get into the audio when a slot's playback ends by loop end, but not by env level being max, under
	certain conditions when the 68K is running.  Test further, maybe emulate in the future?

	Open bus emulation and cycle timing granularity instead of sample(someday?); be careful with DSP input/output buffering
	with respect to dry/direct output path delay, and the mapping of the buffers to the address space.  Emulating the extra wait
	states on 68K accesses caused by PCM playback and DSP memory accesses may be required to fix the problem of game music playing back
	a very teensy bit too fast.

	Hook up MIDI interface emulation to the outside somehow.

	Proper reset/power-on state.

	Mem4Mb

	DSP: Handle instruction with MRT=1 and MWT=1 correctly.

	DSP: Handle IWT=1 when MRT=0 and MWT=0 for the instruction a couple instructions back(NOFL influences the value...)

	DSP: Test IWT=1 when MRT=0 and MWT=1 for the intruction a couple back.
*/

/* Local big-endian-bus access macros replacing the old ne16_*_be /
 * ne64_*_be template family.  T is uint8_t / uint16_t / uint32_t.  base
 * is either a uint16_t-array (for ne16-style 16-bit-bus storage) or
 * a uint64_t-array (for ne64-style 64-bit-bus storage, MPROG only).
 * sizeof(T) decides the access width; #ifdef MSB_FIRST picks the
 * sub-word byte/half index for non-aligned-T accesses. */
#ifdef MSB_FIRST
 #define SCSP_RBO_BE_U8(base, A)   (((const uint8_t*)(base))[(A)])
 #define SCSP_WBO_BE_U8(base, A, V) (((uint8_t*)(base))[(A)] = (V))
#else
 #define SCSP_RBO_BE_U8(base, A)   (((const uint8_t*)(base))[(A) ^ 1])
 #define SCSP_WBO_BE_U8(base, A, V) (((uint8_t*)(base))[(A) ^ 1] = (V))
#endif

/* ne16_rwbo_be<T, IsWrite> equivalent.  pDB points at the host
 * uint32_t carrying the bus value (low bits hold the actual T).  All
 * width branches resolve at compile time. */
#define SCSP_NE16_RW(T, IsWrite, base, byte_off, pDB)                          \
do {                                                                           \
   uint32_t boff__ = (byte_off);                                                 \
   if(IsWrite)                                                                 \
   {                                                                           \
      if(sizeof(T) == 1)                                                       \
         SCSP_WBO_BE_U8((base), boff__, (uint8_t)*(pDB));                        \
      else if(sizeof(T) == 2)                                                  \
         ((uint16_t*)(base))[boff__ >> 1] = (uint16_t)*(pDB);                      \
      else /* sizeof(T) == 4 */                                                \
      {                                                                        \
         ((uint16_t*)(base))[(boff__ >> 1) + 0] = (uint32_t)*(pDB) >> 16;          \
         ((uint16_t*)(base))[(boff__ >> 1) + 1] = (uint32_t)*(pDB);                \
      }                                                                        \
   }                                                                           \
   else                                                                       \
   {                                                                           \
      if(sizeof(T) == 1)                                                       \
         *(pDB) = SCSP_RBO_BE_U8((base), boff__);                              \
      else if(sizeof(T) == 2)                                                  \
         *(pDB) = ((const uint16_t*)(base))[boff__ >> 1];                        \
      else /* sizeof(T) == 4 */                                                \
         *(pDB) = ((uint32_t)((const uint16_t*)(base))[boff__ >> 1] << 16)         \
                | ((const uint16_t*)(base))[(boff__ >> 1) + 1];                  \
   }                                                                           \
} while(0)

/* ne16_rbo_be<T> equivalent (read-only). */
#define SCSP_NE16_R(T, base, byte_off) \
   ( (sizeof(T) == 1) ? (T)SCSP_RBO_BE_U8((base), (byte_off)) :                \
     (sizeof(T) == 2) ? (T)((const uint16_t*)(base))[(byte_off) >> 1] :          \
     (T)(((uint32_t)((const uint16_t*)(base))[(byte_off) >> 1] << 16)              \
       | ((const uint16_t*)(base))[((byte_off) >> 1) + 1]) )

/* ne64_rwbo_be<T, IsWrite> equivalent.  T is uint16_t/uint32_t for
 * MPROG accesses.  base is uint64_t[].  The BE 64-bit bus arranges
 * bytes within each uint64_t such that the byte at offset (8 - sizeof(T) - sub)
 * within the host word is the T at sub-byte offset (for MSB host)
 * — or the XOR'd offset on LE host.  We compute the byte address
 * of the T inside the uint64_t array, then memcpy/typed-load. */
#ifdef MSB_FIRST
 #define SCSP_NE64_BYTE_OFF(T, byte_off) ((byte_off) &~ (sizeof(T) - 1))
#else
 #define SCSP_NE64_BYTE_OFF(T, byte_off) \
    ( ((byte_off) &~ (sizeof(T) - 1)) ^ (8 - (sizeof(T))) )
#endif

#define SCSP_NE64_RW(T, IsWrite, base, byte_off, pDB)                          \
do {                                                                           \
   uint8_t *ptr__ = (uint8_t*)(base) + SCSP_NE64_BYTE_OFF(T, (byte_off));          \
   if(IsWrite)                                                                 \
   {                                                                           \
      T v__ = (T)*(pDB);                                                       \
      memcpy(ptr__, &v__, sizeof(T));                                          \
   }                                                                           \
   else                                                                       \
   {                                                                           \
      T v__;                                                                   \
      memcpy(&v__, ptr__, sizeof(T));                                          \
      *(pDB) = v__;                                                            \
   }                                                                           \
} while(0)


/* Phase-9 (final): internal forward declarations for free functions
 * defined later in this file.  Required because C/C++ resolves
 * names left-to-right and several methods call each other. */
static void SS_SCSP_RecalcSoundInt(SS_SCSP* z);
static void SS_SCSP_RecalcMainInt (SS_SCSP* z);
static void SS_SCSP_MIDI_Reset(SS_SCSP* z);
static void SS_SCSP_MIDI_Run(SS_SCSP* z);
static void SS_SCSP_MIDI_WriteOutput(SS_SCSP* z, uint8_t V);
static uint8_t SS_SCSP_MIDI_ReadInput(SS_SCSP* z);
static void SS_SCSP_RunDMA(SS_SCSP* z);
static void SS_SCSP_RunDSP(SS_SCSP* z);
static void SS_SCSP_DecodeMPROG(SS_SCSP* z);
static void SS_SCSP_RunEG(SS_SCSP* z, SS_SCSP_Slot* s, const unsigned key_eg_scale, const uint32_t sc, const uint32_t scxc);
static uint8_t SS_SCSP_GetALFO(SS_SCSP* z, SS_SCSP_Slot* s);
static int SS_SCSP_GetPLFO(SS_SCSP* z, SS_SCSP_Slot* s);
static void SS_SCSP_RunLFO(SS_SCSP* z, SS_SCSP_Slot* s);
static void SS_SCSP_RecalcShortWaveMask(SS_SCSP* z, SS_SCSP_Slot* s);


static void SS_SCSP_RecalcSoundInt(SS_SCSP* z)	// Sound CPU interrupts
{
 unsigned mask_test;
 unsigned lvmasked[3];
 unsigned out_level = 0;

 mask_test = z->SCIPD & z->SCIEB;
 if(mask_test &~ 0xFF)
  mask_test = (mask_test & 0xFF) | 0x80;

 lvmasked[0] = z->SCILV[0] & mask_test;
 lvmasked[1] = z->SCILV[1] & mask_test;
 lvmasked[2] = z->SCILV[2] & mask_test;

 if(lvmasked[2])
 {
  out_level |= 0x4;
  lvmasked[1] &= lvmasked[2];
  lvmasked[0] &= lvmasked[2];
 }

 if(lvmasked[1])
 {
  out_level |= 0x2;
  lvmasked[0] &= lvmasked[1];
 }

 out_level |= (bool)lvmasked[0];
 //
 //
 SCSP_SoundIntChanged(z, out_level);
}

static void SS_SCSP_RecalcMainInt(SS_SCSP* z)	// Main CPU interrupts
{
 SCSP_MainIntChanged(z, (bool)(z->MCIPD & z->MCIEB));
}

static INLINE void SS_SCSP_RecalcShortWaveMask(SS_SCSP* z, SS_SCSP_Slot* s)
{
 const unsigned lev = s->LoopEnd & 0x780;

 s->ShortWaveMask = 0xFFFFFFFF;
 if(s->ShortWave && lev)
  s->ShortWaveMask = (1U << MDFN_tzcount16(lev)) - 1;
}

void SS_SCSP_Reset(SS_SCSP* z, bool powering_up)
{
 //
 // May need to add a DecodeSlotReg() function or something similar if we implement 
 // more aggressive slot register value optimizations on writes in the future.
 //
 memset(z->SlotRegs, 0, sizeof(z->SlotRegs));
 memset(z->Slots, 0, sizeof(z->Slots));

 for(unsigned i = 0; i < 32; i++)
 {
  // Some SSFs require this; TODO: test to see if this is correct for a reset,
  // and if not, move it into special SSF loading code.
  z->Slots[i].EnvLevel = 0x3FF;
  z->Slots[i].EnvPhase = ENV_PHASE_RELEASE;

  z->Slots[i].LFOTimeCounter = 1;
  //
  SS_SCSP_RecalcShortWaveMask(z, &z->Slots[i]);
 }

 for(unsigned i = 0; i < 2; i++)
  z->EXTS[i] = 0;

 memset(z->SoundStack, 0, sizeof(z->SoundStack));
 memset(z->SoundStackDelayer, 0, sizeof(z->SoundStackDelayer));

 if(powering_up)
 {
  for(size_t i = 0; i < 0x40000; i++)
   z->RAM[i] = 0x0000; // or some other value?
 }

 //
 //
 z->MVOL = 0;
 z->MasterVolume = 0;

 z->SlotMonitorWhich = 0;
 z->SlotMonitorData = 0;

 z->DAC18bit = false;
 z->Mem4Mb = false;

 z->KeyExecute = false;
 z->LFSR = 1;
 z->GlobalCounter = 0;

 SS_SCSP_MIDI_Reset(z);
 //
 //
 z->DMEA = 0;
 z->DRGA = 0;
 z->DTLG = 0;

 z->DMA_Execute = false;
 z->DMA_Direction = false;
 z->DMA_Gate = false;
 //
 //
 for(unsigned i = 0; i < 3; i++)
 {
  z->Timers[i].Control = 0;
  z->Timers[i].Counter = 0;
  z->Timers[i].Reload = -1;
 }

 //
 //
 z->RBP = 0;
 z->RBL = 0;

 memset(&z->DSP, 0, sizeof(z->DSP));
 z->DSP.MDEC_CT = 0;
 //
 //
 z->SCIEB = 0;
 z->SCIPD = 0;

 z->MCIEB = 0;
 z->MCIPD = 0;

 for(unsigned i = 0; i < 3; i++)
  z->SCILV[i] = 0;

 SS_SCSP_RecalcSoundInt(z);
 SS_SCSP_RecalcMainInt(z);
}

//
//
//
static INLINE uint8_t SS_SCSP_MIDI_ReadInput(SS_SCSP* z)
{
 uint8_t ret = z->MIDI.InputFIFO[z->MIDI.InputRP]; // TODO: May not be correct for InputCount == 0; test.

 if(z->MIDI.InputCount)
 {
  z->MIDI.InputRP = (z->MIDI.InputRP + 1) & 0x3;
  z->MIDI.InputCount--;
  z->MIDI.Flags &= ~MIDIF_INPUT_FULL;
  z->MIDI.Flags &= ~MIDIF_INPUT_OFLOW; // TODO: Test.

  if(!z->MIDI.InputCount)
  {
   z->MIDI.Flags |= MIDIF_INPUT_EMPTY;
 
   z->SCIPD &= ~0x008;
   z->MCIPD &= ~0x008;
   SS_SCSP_RecalcSoundInt(z);
   SS_SCSP_RecalcMainInt(z);
  }
 }

 return ret;
}

void SS_SCSP_MIDI_WriteInput(SS_SCSP* z, uint8_t V)
{
 if(z->MIDI.InputCount == 4)
 {
  z->MIDI.Flags |= MIDIF_INPUT_OFLOW;
  return;
 }

 z->MIDI.InputFIFO[z->MIDI.InputWP] = V;
 z->MIDI.InputWP = (z->MIDI.InputWP + 1) & 0x3;
 z->MIDI.InputCount++;

 z->MIDI.Flags &= ~MIDIF_INPUT_EMPTY;
 if(z->MIDI.InputCount == 4)
  z->MIDI.Flags |= MIDIF_INPUT_FULL;

 z->SCIPD |= 0x008;
 z->MCIPD |= 0x008;
 SS_SCSP_RecalcSoundInt(z);
 SS_SCSP_RecalcMainInt(z);
}

static INLINE void SS_SCSP_MIDI_WriteOutput(SS_SCSP* z, uint8_t V)
{
 if(z->MIDI.OutputCount == 4)	// TODO: See if write is completely discarded, or replaces last entry in FIFO.
  return;

 z->MIDI.OutputFIFO[z->MIDI.OutputWP] = V;
 z->MIDI.OutputWP = (z->MIDI.OutputWP + 1) & 0x3;
 z->MIDI.OutputCount++;

 z->MIDI.Flags &= ~MIDIF_OUTPUT_EMPTY;
 if(z->MIDI.OutputCount == 4)
  z->MIDI.Flags |= MIDIF_OUTPUT_FULL;

 z->SCIPD &= ~0x200;
 z->MCIPD &= ~0x200;
 SS_SCSP_RecalcSoundInt(z);
 SS_SCSP_RecalcMainInt(z);
}

static INLINE void SS_SCSP_MIDI_Run(SS_SCSP* z)
{
 z->MIDI.SimuClockDivider += 32;
 if(z->MIDI.SimuClockDivider >= 45)
 {
  z->MIDI.SimuClockDivider -= 45;

  if(!z->MIDI.TransmitBitCounter)
  {
   if(z->MIDI.OutputCount)
   {
    z->MIDI.TransmitBitCounter = 10;
    z->MIDI.TransmitBuffer = (uint32_t)(z->MIDI.OutputFIFO[z->MIDI.OutputRP] | ~0xFF) << 1;
    z->MIDI.OutputRP = (z->MIDI.OutputRP + 1) & 0x3;
    z->MIDI.OutputCount--;

    z->MIDI.Flags &= ~MIDIF_OUTPUT_FULL;
    if(!z->MIDI.OutputCount)
    {
     z->MIDI.Flags |= MIDIF_OUTPUT_EMPTY;

     z->SCIPD |= 0x200;
     z->MCIPD |= 0x200;
     SS_SCSP_RecalcSoundInt(z);
     SS_SCSP_RecalcMainInt(z);
    }
   }
  }

  z->MIDI.TransmitBitCounter -= (bool)z->MIDI.TransmitBitCounter;
 }
}

static INLINE void SS_SCSP_MIDI_Reset(SS_SCSP* z)
{
 memset(z->MIDI.InputFIFO, 0, sizeof(z->MIDI.InputFIFO));
 memset(z->MIDI.OutputFIFO, 0, sizeof(z->MIDI.OutputFIFO));

 z->MIDI.InputRP = z->MIDI.InputWP = z->MIDI.InputCount = 0;
 z->MIDI.OutputRP = z->MIDI.OutputWP = z->MIDI.OutputCount = 0;

 z->MIDI.Flags = MIDIF_INPUT_EMPTY | MIDIF_OUTPUT_EMPTY;

 z->MIDI.SimuClockDivider = 0;
 z->MIDI.TransmitBitCounter = 0;
 z->MIDI.TransmitBuffer = 0x00;

 //z->SCIPD &= ~0x200;
 //z->MCIPD &= ~0x200;
 //SS_SCSP_RecalcSoundInt(z);
 //SS_SCSP_RecalcMainInt(z);
}
//
//
//

static INLINE void SDL_PAN_ToVolume(int16_t* outvol, const unsigned level, const unsigned pan)
{
 const bool pan_which = (bool)(pan & 0x10);
 unsigned basev;
 unsigned panv;

 basev = (0x80 << level);

 if(!level)
  basev = 0;

 panv = basev >> ((pan & 0x0F) >> 1);
 if(pan & 0x01)
  panv -= (panv >> 2);

 if((pan & 0x0F) == 0x0F)
  panv = 0;

 outvol[ pan_which] = panv;
 outvol[!pan_which] = basev;
}

/* Phase-8r1: SS_SCSP::RW retired into 4 named member methods
 * via source-fold (tools/fold_source.py).  Last SS_SCSP template;
 * completes the SS_SCSP module.  Each variant body is the
 * template body with template parameters substituted to
 * literals and dead branches eliminated. */

/* === SS_SCSP::RW variants (2 T x 2 IsWrite -- 4 tuples) === */
INLINE void SS_SCSP_RW_u8_W0(SS_SCSP* z, uint32_t A, uint8_t* DBV_p)
{

 if(A < 0x100000)
 {
  if(MDFN_UNLIKELY(A >= 0x80000))
  {
   (*DBV_p) = 0;
  }
  else
  {
   SCSP_NE16_RW(uint8_t, false, z->RAM, A, &(*DBV_p));
  }
  return;
 }

 A &= 0xFFF;

 if(A < 0x400)
 {
  //
  // SS_SCSP_Slot regs
  //
  const unsigned slotnum = (A >> 5) & 0x1F;

  SCSP_NE16_RW(uint8_t, false, z->SlotRegs[slotnum], A & 0x1F, &(*DBV_p));

  return;
 }

 if(A < 0x430)
 {
  // OldReg = (OldReg &~ mask) | (((*DBV_p) << shift) & mask & whatever);
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  {
   shift = ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  //
  // Common regs
  //
  switch((A >> 1) & 0x1F)
  {
   case 0x00:	// z->MVOL (W), DB (W), M4 (W)
	(*DBV_p) = 0;
	break;

   case 0x01:	// z->RBP (W), z->RBL (W)
	(*DBV_p) = 0;
	break;

   case 0x02:	// z->MIDI input buffer, Input Empty, Input Full, Input Overflow, Output Empty, Output Full (all R)
	{
	 if(!shift)
	 {
	  // TODO: Test correct order of flags latching returning versus input fetching/latching(also maybe take into consideration
	  // 16-bit access from SCU being split into 2x 8-bit accesses on the real thing...).
	  unsigned tmp = z->MIDI.Flags << 8;
	  tmp |= SS_SCSP_MIDI_ReadInput(z);
	  (*DBV_p) = tmp & mask;
	 }
	 else
	  (*DBV_p) = z->MIDI.Flags;
	}
	break;

   case 0x03:	// MOBUF (W)
	(*DBV_p) = 0;
	break;

   case 0x04:	// CA/SGC/EG (R), MSLC (W)
	{
	 (*DBV_p) = (z->SlotMonitorData & mask) >> shift;
	}
	break;

   case 0x05:
   case 0x06:
   case 0x07:
	(*DBV_p) = 0;
	break;

   case 0x08:	// TEST
	(*DBV_p) = 0;
	break;

   case 0x09:	// z->DMEA(low) (W)
	(*DBV_p) = 0;
	break;

   case 0x0A:	// z->DRGA (W), z->DMEA(high) (W)
	(*DBV_p) = 0;
	break;

   case 0x0B: 	// z->DTLG(W), EX(R/W), DI(R/W), GA(R/W)
	{
	 uint16_t tmp = (z->DMA_Execute << 12) | (z->DMA_Direction << 13) | (z->DMA_Gate << 14);

	 (*DBV_p) = (tmp & mask) >> shift;
	}
	break;

   case 0x0C:	// TIMA(W), TACTL(W)
   case 0x0D:	// TIMB(W), TBCTL(W)
   case 0x0E:	// TIMC(W), TCCTL(W)
	(*DBV_p) = 0;
	break;

   case 0x0F:	// z->SCIEB (R/W)
	(*DBV_p) = (z->SCIEB & mask) >> shift;
	break;

   case 0x10:	// z->SCIPD (R) (b5 can be written, like z->MCIPD)
	(*DBV_p) = (z->SCIPD & mask) >> shift;
	break;

   case 0x11: 	// SCIRE (W)
	(*DBV_p) = 0;
	break;

   case 0x12:	// SCILV0 (W)
   case 0x13:	// SCILV1 (W)
   case 0x14:	// SCILV2 (W)
	(*DBV_p) = 0;
	break;

   case 0x15:	// z->MCIEB (W)
	(*DBV_p) = 0;
	break;

   case 0x16:	// z->MCIPD (R) (when b5=1 is written, set corresponding bit to 1; writing 0 has no apparent effect)
	(*DBV_p) = (z->MCIPD & mask) >> shift;
	break;

   case 0x17:	// MCIRE (W)
	(*DBV_p) = 0;
	break;

   case 0x18:
   case 0x19:
   case 0x1A:
   case 0x1B:
   case 0x1C:
   case 0x1D:
   case 0x1E:
   case 0x1F:
	(*DBV_p) = 0;
	break;
  }

  return;
 }

 if(A >= 0x600 && A <= 0x67F)
 {
  //
  // Sound stack data
  //
  SCSP_NE16_RW(uint8_t, false, z->SoundStack, A & 0x7F, &(*DBV_p));

  return;
 }

 if(A >= 0x700 && A <= 0x77F)
 {
  //
  // z->DSP coefficients
  //
  const unsigned index = (A & 0x7F) >> 1;
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  {
   shift = ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  (*DBV_p) = ((z->DSP.COEF[index] << 3) & mask) >> shift;

  return;
 }

 if(A >= 0x780 && A <= 0x7BF)
 {
  //
  // z->DSP memory addresses
  //
  SCSP_NE16_RW(uint8_t, false, z->DSP.MADRS, A & 0x3F, &(*DBV_p));

  return;
 }

 if(A >= 0x800 && A <= 0xBFF)
 {
  //
  // z->DSP microprogram
  //
  SCSP_NE64_RW(uint8_t, false, z->DSP.MPROG, A & 0x3FF, &(*DBV_p));

  return;
 }

 //
 // z->DSP work buffer
 //
 if(A >= 0xC00 && A <= 0xDFF)
 {
  const unsigned index = (A & 0x1FF) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 8 : 0;

  {
   shift += ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  if(!(A & 2))
   mask &= 0xFF;

  (*DBV_p) = (z->DSP.TEMP[index] & mask) >> shift;

  return;
 }

 //
 // z->DSP memory read stack
 //
 if(A >= 0xE00 && A <= 0xE7F)
 {
  const unsigned index = (A & 0x7F) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 8 : 0;

  {
   shift += ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  if(!(A & 2))
   mask &= 0xFF;

  (*DBV_p) = (z->DSP.MEMS[index] & mask) >> shift;

  return;
 }

 //
 // z->DSP mix stack
 //
 if(A >= 0xE80 && A <= 0xEBF)
 {
  const unsigned index = (A & 0x3F) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 4 : 0;

  {
   shift += ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  if(!(A & 2))
   mask &= 0x0F;

  (*DBV_p) = (z->DSP.MIXS[index] & mask) >> shift;

  return;
 }

 //
 //
 //
 if(A >= 0xEC0 && A <= 0xEDF)
 {
  SCSP_NE16_RW(uint8_t, false, z->DSP.EFREG, A & 0x1F, &(*DBV_p));

  return;
 }

 if(A >= 0xEE0 && A <= 0xEE3)
 {
  (*DBV_p) = SCSP_NE16_R(uint8_t, z->EXTS, A & 0x3);

  return;
 }

 (*DBV_p) = 0;
}

INLINE void SS_SCSP_RW_u16_W0(SS_SCSP* z, uint32_t A, uint16_t* DBV_p)
{

 if(A < 0x100000)
 {
  if(MDFN_UNLIKELY(A >= 0x80000))
  {
   (*DBV_p) = 0;
  }
  else
  {
   SCSP_NE16_RW(uint16_t, false, z->RAM, A, &(*DBV_p));
  }
  return;
 }

 A &= 0xFFF;

 if(A < 0x400)
 {
  //
  // SS_SCSP_Slot regs
  //
  const unsigned slotnum = (A >> 5) & 0x1F;

  SCSP_NE16_RW(uint16_t, false, z->SlotRegs[slotnum], A & 0x1F, &(*DBV_p));

  return;
 }

 if(A < 0x430)
 {
  // OldReg = (OldReg &~ mask) | (((*DBV_p) << shift) & mask & whatever);
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  //
  // Common regs
  //
  switch((A >> 1) & 0x1F)
  {
   case 0x00:	// z->MVOL (W), DB (W), M4 (W)
	(*DBV_p) = 0;
	break;

   case 0x01:	// z->RBP (W), z->RBL (W)
	(*DBV_p) = 0;
	break;

   case 0x02:	// z->MIDI input buffer, Input Empty, Input Full, Input Overflow, Output Empty, Output Full (all R)
	{
	 if(!shift)
	 {
	  // TODO: Test correct order of flags latching returning versus input fetching/latching(also maybe take into consideration
	  // 16-bit access from SCU being split into 2x 8-bit accesses on the real thing...).
	  unsigned tmp = z->MIDI.Flags << 8;
	  tmp |= SS_SCSP_MIDI_ReadInput(z);
	  (*DBV_p) = tmp & mask;
	 }
	 else
	  (*DBV_p) = z->MIDI.Flags;
	}
	break;

   case 0x03:	// MOBUF (W)
	(*DBV_p) = 0;
	break;

   case 0x04:	// CA/SGC/EG (R), MSLC (W)
	{
	 (*DBV_p) = (z->SlotMonitorData & mask) >> shift;
	}
	break;

   case 0x05:
   case 0x06:
   case 0x07:
	(*DBV_p) = 0;
	break;

   case 0x08:	// TEST
	(*DBV_p) = 0;
	break;

   case 0x09:	// z->DMEA(low) (W)
	(*DBV_p) = 0;
	break;

   case 0x0A:	// z->DRGA (W), z->DMEA(high) (W)
	(*DBV_p) = 0;
	break;

   case 0x0B: 	// z->DTLG(W), EX(R/W), DI(R/W), GA(R/W)
	{
	 uint16_t tmp = (z->DMA_Execute << 12) | (z->DMA_Direction << 13) | (z->DMA_Gate << 14);

	 (*DBV_p) = (tmp & mask) >> shift;
	}
	break;

   case 0x0C:	// TIMA(W), TACTL(W)
   case 0x0D:	// TIMB(W), TBCTL(W)
   case 0x0E:	// TIMC(W), TCCTL(W)
	(*DBV_p) = 0;
	break;

   case 0x0F:	// z->SCIEB (R/W)
	(*DBV_p) = (z->SCIEB & mask) >> shift;
	break;

   case 0x10:	// z->SCIPD (R) (b5 can be written, like z->MCIPD)
	(*DBV_p) = (z->SCIPD & mask) >> shift;
	break;

   case 0x11: 	// SCIRE (W)
	(*DBV_p) = 0;
	break;

   case 0x12:	// SCILV0 (W)
   case 0x13:	// SCILV1 (W)
   case 0x14:	// SCILV2 (W)
	(*DBV_p) = 0;
	break;

   case 0x15:	// z->MCIEB (W)
	(*DBV_p) = 0;
	break;

   case 0x16:	// z->MCIPD (R) (when b5=1 is written, set corresponding bit to 1; writing 0 has no apparent effect)
	(*DBV_p) = (z->MCIPD & mask) >> shift;
	break;

   case 0x17:	// MCIRE (W)
	(*DBV_p) = 0;
	break;

   case 0x18:
   case 0x19:
   case 0x1A:
   case 0x1B:
   case 0x1C:
   case 0x1D:
   case 0x1E:
   case 0x1F:
	(*DBV_p) = 0;
	break;
  }

  return;
 }

 if(A >= 0x600 && A <= 0x67F)
 {
  //
  // Sound stack data
  //
  SCSP_NE16_RW(uint16_t, false, z->SoundStack, A & 0x7F, &(*DBV_p));

  return;
 }

 if(A >= 0x700 && A <= 0x77F)
 {
  //
  // z->DSP coefficients
  //
  const unsigned index = (A & 0x7F) >> 1;
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  (*DBV_p) = ((z->DSP.COEF[index] << 3) & mask) >> shift;

  return;
 }

 if(A >= 0x780 && A <= 0x7BF)
 {
  //
  // z->DSP memory addresses
  //
  SCSP_NE16_RW(uint16_t, false, z->DSP.MADRS, A & 0x3F, &(*DBV_p));

  return;
 }

 if(A >= 0x800 && A <= 0xBFF)
 {
  //
  // z->DSP microprogram
  //
  SCSP_NE64_RW(uint16_t, false, z->DSP.MPROG, A & 0x3FF, &(*DBV_p));

  return;
 }

 //
 // z->DSP work buffer
 //
 if(A >= 0xC00 && A <= 0xDFF)
 {
  const unsigned index = (A & 0x1FF) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 8 : 0;

  mask = 0xFFFF << shift;

  if(!(A & 2))
   mask &= 0xFF;

  (*DBV_p) = (z->DSP.TEMP[index] & mask) >> shift;

  return;
 }

 //
 // z->DSP memory read stack
 //
 if(A >= 0xE00 && A <= 0xE7F)
 {
  const unsigned index = (A & 0x7F) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 8 : 0;

  mask = 0xFFFF << shift;

  if(!(A & 2))
   mask &= 0xFF;

  (*DBV_p) = (z->DSP.MEMS[index] & mask) >> shift;

  return;
 }

 //
 // z->DSP mix stack
 //
 if(A >= 0xE80 && A <= 0xEBF)
 {
  const unsigned index = (A & 0x3F) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 4 : 0;

  mask = 0xFFFF << shift;

  if(!(A & 2))
   mask &= 0x0F;

  (*DBV_p) = (z->DSP.MIXS[index] & mask) >> shift;

  return;
 }

 //
 //
 //
 if(A >= 0xEC0 && A <= 0xEDF)
 {
  SCSP_NE16_RW(uint16_t, false, z->DSP.EFREG, A & 0x1F, &(*DBV_p));

  return;
 }

 if(A >= 0xEE0 && A <= 0xEE3)
 {
  (*DBV_p) = SCSP_NE16_R(uint16_t, z->EXTS, A & 0x3);

  return;
 }

 (*DBV_p) = 0;
}

INLINE void SS_SCSP_RW_u8_W1(SS_SCSP* z, uint32_t A, uint8_t* DBV_p)
{

 if(A < 0x100000)
 {
  if(MDFN_UNLIKELY(A >= 0x80000))
  {
   
  }
  else
  {
   SCSP_NE16_RW(uint8_t, true, z->RAM, A, &(*DBV_p));
  }
  return;
 }

 A &= 0xFFF;

 if(A < 0x400)
 {
  //
  // SS_SCSP_Slot regs
  //
  const unsigned slotnum = (A >> 5) & 0x1F;

  SCSP_NE16_RW(uint8_t, true, z->SlotRegs[slotnum], A & 0x1F, &(*DBV_p));

  {
   SS_SCSP_Slot* s = &z->Slots[slotnum];
   uint16_t& SRV = z->SlotRegs[slotnum][(A >> 1) & 0xF];

   switch((A >> 1) & 0xF)
   {
    case 0x00:
	z->KeyExecute |= (bool)(SRV & 0x1000);
	SRV &= 0x0FFF;

	s->KeyBit = (SRV >> 11) & 0x1;
	s->SBXOR = z->SB_XOR_Table[(SRV >> 9) & 0x3];
	s->SourceControl = (SRV >> 7) & 0x3;
	s->LoopMode = (SRV >> 5) & 0x3;
	s->WF8Bit = (SRV >> 4) & 0x1;
	s->StartAddr = (s->StartAddr & 0xFFFF) | ((SRV & 0xF) << 16);
	break;

    case 0x01:
	s->StartAddr = (s->StartAddr &~ 0xFFFF) | SRV;
	break;

    case 0x02:
	s->LoopStart = SRV;
	break;

    case 0x03:
	s->LoopEnd = SRV;
	//
	SS_SCSP_RecalcShortWaveMask(z, s);
	break;

    case 0x04:
	s->EnvRates[ENV_PHASE_ATTACK] = SRV & 0x1F;
	s->AttackHold = (SRV >> 5) & 0x1;
	s->EnvRates[ENV_PHASE_DECAY1] = (SRV >> 6) & 0x1F;
	s->EnvRates[ENV_PHASE_DECAY2] = (SRV >> 11) & 0x1F;
	break;

    case 0x05:
	s->EnvRates[ENV_PHASE_RELEASE] = SRV & 0x1F;
	s->DecayLevel = (SRV >> 5) & 0x1F;
	s->KRS = (SRV >> 10) & 0xF;
	s->AttackLoopLink = (SRV >> 14) & 0x1;
	s->EGBypass = (SRV >> 15) & 0x1;
	break;

    case 0x06:
	SRV &= 0x0FFF;

	s->TotalLevel = SRV & 0xFF;
	s->SoundDirect = (SRV >> 8) & 0x1;
	s->StackWriteInhibit = (SRV >> 9) & 0x1;
	break;

    case 0x07:
        s->ModInputY = SRV & 0x3F;
	s->ModInputX = (SRV >> 6) & 0x3F;
	s->ModLevel = (SRV >> 12) & 0xF;
	break;

    case 0x08:
	s->FreqNum = SRV & 0x7FF;
	s->Octave = (SRV >> 11) & 0xF;
	s->ShortWave = (SRV >> 15) & 0x1;
	//
	SS_SCSP_RecalcShortWaveMask(z, s);
	break;

    case 0x09:
	s->ALFOModLevel = SRV & 0x7;
	s->ALFOWaveform = (SRV >> 3) & 0x3;
	s->PLFOModLevel = (SRV >> 5) & 0x7;
	s->PLFOWaveform = (SRV >> 8) & 0x3;
	s->LFOFreq = (SRV >> 10) & 0x1F;
	s->LFOReset = (SRV >> 15) & 0x1;
	break;

    case 0x0A:
	SRV &= 0x00FF;
	s->ToDSPLevel = SRV & 0x7;
	s->ToDSPSelect = (SRV >> 3) & 0xF;
	break;

    case 0x0B:
	SDL_PAN_ToVolume(s->DirectVolume, (SRV >> 13) & 0x7, (SRV >> 8) & 0x1F);
	SDL_PAN_ToVolume(s->EffectVolume, (SRV >>  5) & 0x7, (SRV >> 0) & 0x1F);
	break;

    case 0x0C: case 0x0D: case 0x0E: case 0x0F:
	SRV = 0;
	break;
   }
  }

  return;
 }

 if(A < 0x430)
 {
  // OldReg = (OldReg &~ mask) | (((*DBV_p) << shift) & mask & whatever);
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  {
   shift = ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  //
  // Common regs
  //
  switch((A >> 1) & 0x1F)
  {
   case 0x00:	// z->MVOL (W), DB (W), M4 (W)
	{
	 uint16_t tmp = z->MVOL | (z->DAC18bit << 8) | (z->Mem4Mb << 9);

	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);

	 z->MVOL = (tmp & 0xF);
	 z->DAC18bit = (tmp >> 8) & 1;
	 z->Mem4Mb = (tmp >> 9) & 1;

         //
	 {
	  unsigned mv;

	  mv = 0x2 << (z->MVOL >> 1);
	  if(!(z->MVOL & 1))
	   mv -= (mv >> 2);

	  if(!z->MVOL)
	   mv = 0;

	  z->MasterVolume = mv;
         }
	}
	break;

   case 0x01:	// z->RBP (W), z->RBL (W)
	{
	 uint16_t tmp = z->RBP | (z->RBL << 7);
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 z->RBP = tmp & 0x7F;
	 z->RBL = (tmp >> 7) & 0x3;
	}
	break;

   case 0x02:	// z->MIDI input buffer, Input Empty, Input Full, Input Overflow, Output Empty, Output Full (all R)
	
	break;

   case 0x03:	// MOBUF (W)
	{
	 if(!shift)
	 {
	  SS_SCSP_MIDI_WriteOutput(z, (*DBV_p));
	 }
	}
	break;

   case 0x04:	// CA/SGC/EG (R), MSLC (W)
	{
	 uint16_t tmp = (z->SlotMonitorWhich << 11);
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 z->SlotMonitorWhich = (tmp >> 11) & 0x1F;
	}
	break;

   case 0x05:
   case 0x06:
   case 0x07:
	
	break;

   case 0x08:	// TEST
	{
	 //
	 // 0x80: Mute? messes up sound cpu too?
	 //
	 // 0x20: forces LFO frequency to max? and blocks memory access to waveform playback? messes up sound cpu too?
	 // 0x40: blocks memory access to waveform playback? messes up sound cpu too?
	 // 0x60: force wfallowaccess = 0 ? messes up sound cpu too?
	 //
	 // 0x08: appears to block SCU access to SCSP z->RAM(causes hang)
	 // 0x04: kills sound cpu(blocks memory access?)
	 // 0x02: blocks slot waveform playback from reading memory?
	 // 0x01: messes up dsp or blocks dsp memory access? the ears are not happy!
	 //
	}
	break;

   case 0x09:	// z->DMEA(low) (W)
	{
	 uint16_t tmp = z->DMEA << 1;
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 z->DMEA = tmp >> 1;
	}
	break;

   case 0x0A:	// z->DRGA (W), z->DMEA(high) (W)
	{
	 uint16_t tmp = ((z->DMEA >> 3) & 0xF000) | (z->DRGA << 1);
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 z->DMEA = (z->DMEA & 0x7FFF) | ((tmp & 0xF000) << 3);
	 z->DRGA = (tmp >> 1) & 0x7FF;
	}
	break;

   case 0x0B: 	// z->DTLG(W), EX(R/W), DI(R/W), GA(R/W)
	{
	 //const bool prev_execute = z->DMA_Execute;
	 uint16_t tmp = (z->DTLG << 1) | (z->DMA_Execute << 12) | (z->DMA_Direction << 13) | (z->DMA_Gate << 14);

	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);

	 z->DTLG = (tmp >> 1) & 0x7FF;
	 z->DMA_Execute |= (tmp >> 12) & 0x1;
	 z->DMA_Direction = (tmp >> 13) & 0x1;
	 z->DMA_Gate = (tmp >> 14) & 0x1;

	 SS_SCSP_RunDMA(z);
	}
	break;

   case 0x0C:	// TIMA(W), TACTL(W)
   case 0x0D:	// TIMB(W), TBCTL(W)
   case 0x0E:	// TIMC(W), TCCTL(W)
	{
	 SS_SCSP_Timer* t = &z->Timers[((A >> 1) & 0x1F) - 0x0C];
	 uint16_t tmp = (t->Control << 8);
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 t->Control = (tmp >> 8) & 0x7;

	 if(!shift)
	  t->Reload = (*DBV_p) & 0xFF;
	}
	break;

   case 0x0F:	// z->SCIEB (R/W)
	{
	 z->SCIEB = (z->SCIEB &~ mask) | (((*DBV_p) << shift) & mask & 0x7FF);
	 SS_SCSP_RecalcSoundInt(z);
	}
	break;

   case 0x10:	// z->SCIPD (R) (b5 can be written, like z->MCIPD)
	{
	 z->SCIPD |= (((*DBV_p) << shift) & mask & 0x020);
	 SS_SCSP_RecalcSoundInt(z);
	}
	break;

   case 0x11: 	// SCIRE (W)
	{
	 z->SCIPD &= ~(((*DBV_p) << shift) & mask);
	 SS_SCSP_RecalcSoundInt(z);
	}
	break;

   case 0x12:	// SCILV0 (W)
   case 0x13:	// SCILV1 (W)
   case 0x14:	// SCILV2 (W)
	{
	 const unsigned index = ((A >> 1) & 0x1F) - 0x12;

	 z->SCILV[index] = (z->SCILV[index] &~ mask) | (((*DBV_p) << shift) & mask & 0x00FF);
	 SS_SCSP_RecalcSoundInt(z);
	}
	break;

   case 0x15:	// z->MCIEB (W)
	{
	 z->MCIEB = (z->MCIEB &~ mask) | (((*DBV_p) << shift) & mask & 0x7FF);
	 SS_SCSP_RecalcMainInt(z);
	}
	break;

   case 0x16:	// z->MCIPD (R) (when b5=1 is written, set corresponding bit to 1; writing 0 has no apparent effect)
	{
	 z->MCIPD |= (((*DBV_p) << shift) & mask & 0x020);
	 SS_SCSP_RecalcMainInt(z);
	}
	break;

   case 0x17:	// MCIRE (W)
	{
	 z->MCIPD &= ~(((*DBV_p) << shift) & mask);
	 SS_SCSP_RecalcMainInt(z);
	}
	break;

   case 0x18:
   case 0x19:
   case 0x1A:
   case 0x1B:
   case 0x1C:
   case 0x1D:
   case 0x1E:
   case 0x1F:
	{
	}
	break;
  }

  return;
 }

 if(A >= 0x600 && A <= 0x67F)
 {
  //
  // Sound stack data
  //
  SCSP_NE16_RW(uint8_t, true, z->SoundStack, A & 0x7F, &(*DBV_p));

  return;
 }

 if(A >= 0x700 && A <= 0x77F)
 {
  //
  // z->DSP coefficients
  //
  const unsigned index = (A & 0x7F) >> 1;
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  {
   shift = ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  z->DSP.COEF[index] = (((z->DSP.COEF[index] << 3) &~ mask) | (((*DBV_p) << shift) & mask)) >> 3;

  return;
 }

 if(A >= 0x780 && A <= 0x7BF)
 {
  //
  // z->DSP memory addresses
  //
  SCSP_NE16_RW(uint8_t, true, z->DSP.MADRS, A & 0x3F, &(*DBV_p));

  return;
 }

 if(A >= 0x800 && A <= 0xBFF)
 {
  //
  // z->DSP microprogram
  //
  SCSP_NE64_RW(uint8_t, true, z->DSP.MPROG, A & 0x3FF, &(*DBV_p));

  z->DSP.MPROG_Dirty = true;

  return;
 }

 //
 // z->DSP work buffer
 //
 if(A >= 0xC00 && A <= 0xDFF)
 {
  const unsigned index = (A & 0x1FF) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 8 : 0;

  {
   shift += ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  if(!(A & 2))
   mask &= 0xFF;

  z->DSP.TEMP[index] = (z->DSP.TEMP[index] &~ mask) | (((*DBV_p) << shift) & mask & 0xFFFFFF);

  return;
 }

 //
 // z->DSP memory read stack
 //
 if(A >= 0xE00 && A <= 0xE7F)
 {
  const unsigned index = (A & 0x7F) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 8 : 0;

  {
   shift += ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  if(!(A & 2))
   mask &= 0xFF;

  z->DSP.MEMS[index] = (z->DSP.MEMS[index] &~ mask) | (((*DBV_p) << shift) & mask & 0xFFFFFF);

  return;
 }

 //
 // z->DSP mix stack
 //
 if(A >= 0xE80 && A <= 0xEBF)
 {
  const unsigned index = (A & 0x3F) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 4 : 0;

  {
   shift += ((A & 1) ^ 1) << 3;
   mask = 0xFF << shift;
  }

  if(!(A & 2))
   mask &= 0x0F;

  z->DSP.MIXS[index] = (z->DSP.MIXS[index] &~ mask) | (((*DBV_p) << shift) & mask & 0xFFFFFF);

  return;
 }

 //
 //
 //
 if(A >= 0xEC0 && A <= 0xEDF)
 {
  SCSP_NE16_RW(uint8_t, true, z->DSP.EFREG, A & 0x1F, &(*DBV_p));

  return;
 }

 if(A >= 0xEE0 && A <= 0xEE3)
 {

  return;
 }

}

INLINE void SS_SCSP_RW_u16_W1(SS_SCSP* z, uint32_t A, uint16_t* DBV_p)
{

 if(A < 0x100000)
 {
  if(MDFN_UNLIKELY(A >= 0x80000))
  {
   
  }
  else
  {
   SCSP_NE16_RW(uint16_t, true, z->RAM, A, &(*DBV_p));
  }
  return;
 }

 A &= 0xFFF;

 if(A < 0x400)
 {
  //
  // SS_SCSP_Slot regs
  //
  const unsigned slotnum = (A >> 5) & 0x1F;

  SCSP_NE16_RW(uint16_t, true, z->SlotRegs[slotnum], A & 0x1F, &(*DBV_p));

  {
   SS_SCSP_Slot* s = &z->Slots[slotnum];
   uint16_t& SRV = z->SlotRegs[slotnum][(A >> 1) & 0xF];

   switch((A >> 1) & 0xF)
   {
    case 0x00:
	z->KeyExecute |= (bool)(SRV & 0x1000);
	SRV &= 0x0FFF;

	s->KeyBit = (SRV >> 11) & 0x1;
	s->SBXOR = z->SB_XOR_Table[(SRV >> 9) & 0x3];
	s->SourceControl = (SRV >> 7) & 0x3;
	s->LoopMode = (SRV >> 5) & 0x3;
	s->WF8Bit = (SRV >> 4) & 0x1;
	s->StartAddr = (s->StartAddr & 0xFFFF) | ((SRV & 0xF) << 16);
	break;

    case 0x01:
	s->StartAddr = (s->StartAddr &~ 0xFFFF) | SRV;
	break;

    case 0x02:
	s->LoopStart = SRV;
	break;

    case 0x03:
	s->LoopEnd = SRV;
	//
	SS_SCSP_RecalcShortWaveMask(z, s);
	break;

    case 0x04:
	s->EnvRates[ENV_PHASE_ATTACK] = SRV & 0x1F;
	s->AttackHold = (SRV >> 5) & 0x1;
	s->EnvRates[ENV_PHASE_DECAY1] = (SRV >> 6) & 0x1F;
	s->EnvRates[ENV_PHASE_DECAY2] = (SRV >> 11) & 0x1F;
	break;

    case 0x05:
	s->EnvRates[ENV_PHASE_RELEASE] = SRV & 0x1F;
	s->DecayLevel = (SRV >> 5) & 0x1F;
	s->KRS = (SRV >> 10) & 0xF;
	s->AttackLoopLink = (SRV >> 14) & 0x1;
	s->EGBypass = (SRV >> 15) & 0x1;
	break;

    case 0x06:
	SRV &= 0x0FFF;

	s->TotalLevel = SRV & 0xFF;
	s->SoundDirect = (SRV >> 8) & 0x1;
	s->StackWriteInhibit = (SRV >> 9) & 0x1;
	break;

    case 0x07:
        s->ModInputY = SRV & 0x3F;
	s->ModInputX = (SRV >> 6) & 0x3F;
	s->ModLevel = (SRV >> 12) & 0xF;
	break;

    case 0x08:
	s->FreqNum = SRV & 0x7FF;
	s->Octave = (SRV >> 11) & 0xF;
	s->ShortWave = (SRV >> 15) & 0x1;
	//
	SS_SCSP_RecalcShortWaveMask(z, s);
	break;

    case 0x09:
	s->ALFOModLevel = SRV & 0x7;
	s->ALFOWaveform = (SRV >> 3) & 0x3;
	s->PLFOModLevel = (SRV >> 5) & 0x7;
	s->PLFOWaveform = (SRV >> 8) & 0x3;
	s->LFOFreq = (SRV >> 10) & 0x1F;
	s->LFOReset = (SRV >> 15) & 0x1;
	break;

    case 0x0A:
	SRV &= 0x00FF;
	s->ToDSPLevel = SRV & 0x7;
	s->ToDSPSelect = (SRV >> 3) & 0xF;
	break;

    case 0x0B:
	SDL_PAN_ToVolume(s->DirectVolume, (SRV >> 13) & 0x7, (SRV >> 8) & 0x1F);
	SDL_PAN_ToVolume(s->EffectVolume, (SRV >>  5) & 0x7, (SRV >> 0) & 0x1F);
	break;

    case 0x0C: case 0x0D: case 0x0E: case 0x0F:
	SRV = 0;
	break;
   }
  }

  return;
 }

 if(A < 0x430)
 {
  // OldReg = (OldReg &~ mask) | (((*DBV_p) << shift) & mask & whatever);
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  //
  // Common regs
  //
  switch((A >> 1) & 0x1F)
  {
   case 0x00:	// z->MVOL (W), DB (W), M4 (W)
	{
	 uint16_t tmp = z->MVOL | (z->DAC18bit << 8) | (z->Mem4Mb << 9);

	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);

	 z->MVOL = (tmp & 0xF);
	 z->DAC18bit = (tmp >> 8) & 1;
	 z->Mem4Mb = (tmp >> 9) & 1;

         //
	 {
	  unsigned mv;

	  mv = 0x2 << (z->MVOL >> 1);
	  if(!(z->MVOL & 1))
	   mv -= (mv >> 2);

	  if(!z->MVOL)
	   mv = 0;

	  z->MasterVolume = mv;
         }
	}
	break;

   case 0x01:	// z->RBP (W), z->RBL (W)
	{
	 uint16_t tmp = z->RBP | (z->RBL << 7);
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 z->RBP = tmp & 0x7F;
	 z->RBL = (tmp >> 7) & 0x3;
	}
	break;

   case 0x02:	// z->MIDI input buffer, Input Empty, Input Full, Input Overflow, Output Empty, Output Full (all R)
	
	break;

   case 0x03:	// MOBUF (W)
	{
	 if(!shift)
	 {
	  SS_SCSP_MIDI_WriteOutput(z, (*DBV_p));
	 }
	}
	break;

   case 0x04:	// CA/SGC/EG (R), MSLC (W)
	{
	 uint16_t tmp = (z->SlotMonitorWhich << 11);
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 z->SlotMonitorWhich = (tmp >> 11) & 0x1F;
	}
	break;

   case 0x05:
   case 0x06:
   case 0x07:
	
	break;

   case 0x08:	// TEST
	{
	 //
	 // 0x80: Mute? messes up sound cpu too?
	 //
	 // 0x20: forces LFO frequency to max? and blocks memory access to waveform playback? messes up sound cpu too?
	 // 0x40: blocks memory access to waveform playback? messes up sound cpu too?
	 // 0x60: force wfallowaccess = 0 ? messes up sound cpu too?
	 //
	 // 0x08: appears to block SCU access to SCSP z->RAM(causes hang)
	 // 0x04: kills sound cpu(blocks memory access?)
	 // 0x02: blocks slot waveform playback from reading memory?
	 // 0x01: messes up dsp or blocks dsp memory access? the ears are not happy!
	 //
	}
	break;

   case 0x09:	// z->DMEA(low) (W)
	{
	 uint16_t tmp = z->DMEA << 1;
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 z->DMEA = tmp >> 1;
	}
	break;

   case 0x0A:	// z->DRGA (W), z->DMEA(high) (W)
	{
	 uint16_t tmp = ((z->DMEA >> 3) & 0xF000) | (z->DRGA << 1);
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 z->DMEA = (z->DMEA & 0x7FFF) | ((tmp & 0xF000) << 3);
	 z->DRGA = (tmp >> 1) & 0x7FF;
	}
	break;

   case 0x0B: 	// z->DTLG(W), EX(R/W), DI(R/W), GA(R/W)
	{
	 //const bool prev_execute = z->DMA_Execute;
	 uint16_t tmp = (z->DTLG << 1) | (z->DMA_Execute << 12) | (z->DMA_Direction << 13) | (z->DMA_Gate << 14);

	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);

	 z->DTLG = (tmp >> 1) & 0x7FF;
	 z->DMA_Execute |= (tmp >> 12) & 0x1;
	 z->DMA_Direction = (tmp >> 13) & 0x1;
	 z->DMA_Gate = (tmp >> 14) & 0x1;

	 SS_SCSP_RunDMA(z);
	}
	break;

   case 0x0C:	// TIMA(W), TACTL(W)
   case 0x0D:	// TIMB(W), TBCTL(W)
   case 0x0E:	// TIMC(W), TCCTL(W)
	{
	 SS_SCSP_Timer* t = &z->Timers[((A >> 1) & 0x1F) - 0x0C];
	 uint16_t tmp = (t->Control << 8);
	 tmp = (tmp &~ mask) | (((*DBV_p) << shift) & mask);
	 t->Control = (tmp >> 8) & 0x7;

	 if(!shift)
	  t->Reload = (*DBV_p) & 0xFF;
	}
	break;

   case 0x0F:	// z->SCIEB (R/W)
	{
	 z->SCIEB = (z->SCIEB &~ mask) | (((*DBV_p) << shift) & mask & 0x7FF);
	 SS_SCSP_RecalcSoundInt(z);
	}
	break;

   case 0x10:	// z->SCIPD (R) (b5 can be written, like z->MCIPD)
	{
	 z->SCIPD |= (((*DBV_p) << shift) & mask & 0x020);
	 SS_SCSP_RecalcSoundInt(z);
	}
	break;

   case 0x11: 	// SCIRE (W)
	{
	 z->SCIPD &= ~(((*DBV_p) << shift) & mask);
	 SS_SCSP_RecalcSoundInt(z);
	}
	break;

   case 0x12:	// SCILV0 (W)
   case 0x13:	// SCILV1 (W)
   case 0x14:	// SCILV2 (W)
	{
	 const unsigned index = ((A >> 1) & 0x1F) - 0x12;

	 z->SCILV[index] = (z->SCILV[index] &~ mask) | (((*DBV_p) << shift) & mask & 0x00FF);
	 SS_SCSP_RecalcSoundInt(z);
	}
	break;

   case 0x15:	// z->MCIEB (W)
	{
	 z->MCIEB = (z->MCIEB &~ mask) | (((*DBV_p) << shift) & mask & 0x7FF);
	 SS_SCSP_RecalcMainInt(z);
	}
	break;

   case 0x16:	// z->MCIPD (R) (when b5=1 is written, set corresponding bit to 1; writing 0 has no apparent effect)
	{
	 z->MCIPD |= (((*DBV_p) << shift) & mask & 0x020);
	 SS_SCSP_RecalcMainInt(z);
	}
	break;

   case 0x17:	// MCIRE (W)
	{
	 z->MCIPD &= ~(((*DBV_p) << shift) & mask);
	 SS_SCSP_RecalcMainInt(z);
	}
	break;

   case 0x18:
   case 0x19:
   case 0x1A:
   case 0x1B:
   case 0x1C:
   case 0x1D:
   case 0x1E:
   case 0x1F:
	{
	}
	break;
  }

  return;
 }

 if(A >= 0x600 && A <= 0x67F)
 {
  //
  // Sound stack data
  //
  SCSP_NE16_RW(uint16_t, true, z->SoundStack, A & 0x7F, &(*DBV_p));

  return;
 }

 if(A >= 0x700 && A <= 0x77F)
 {
  //
  // z->DSP coefficients
  //
  const unsigned index = (A & 0x7F) >> 1;
  unsigned mask = 0xFFFF;
  unsigned shift = 0;

  z->DSP.COEF[index] = (((z->DSP.COEF[index] << 3) &~ mask) | (((*DBV_p) << shift) & mask)) >> 3;

  return;
 }

 if(A >= 0x780 && A <= 0x7BF)
 {
  //
  // z->DSP memory addresses
  //
  SCSP_NE16_RW(uint16_t, true, z->DSP.MADRS, A & 0x3F, &(*DBV_p));

  return;
 }

 if(A >= 0x800 && A <= 0xBFF)
 {
  //
  // z->DSP microprogram
  //
  SCSP_NE64_RW(uint16_t, true, z->DSP.MPROG, A & 0x3FF, &(*DBV_p));

  z->DSP.MPROG_Dirty = true;

  return;
 }

 //
 // z->DSP work buffer
 //
 if(A >= 0xC00 && A <= 0xDFF)
 {
  const unsigned index = (A & 0x1FF) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 8 : 0;

  mask = 0xFFFF << shift;

  if(!(A & 2))
   mask &= 0xFF;

  z->DSP.TEMP[index] = (z->DSP.TEMP[index] &~ mask) | (((*DBV_p) << shift) & mask & 0xFFFFFF);

  return;
 }

 //
 // z->DSP memory read stack
 //
 if(A >= 0xE00 && A <= 0xE7F)
 {
  const unsigned index = (A & 0x7F) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 8 : 0;

  mask = 0xFFFF << shift;

  if(!(A & 2))
   mask &= 0xFF;

  z->DSP.MEMS[index] = (z->DSP.MEMS[index] &~ mask) | (((*DBV_p) << shift) & mask & 0xFFFFFF);

  return;
 }

 //
 // z->DSP mix stack
 //
 if(A >= 0xE80 && A <= 0xEBF)
 {
  const unsigned index = (A & 0x3F) >> 2;
  unsigned mask;
  unsigned shift = (A & 2) ? 4 : 0;

  mask = 0xFFFF << shift;

  if(!(A & 2))
   mask &= 0x0F;

  z->DSP.MIXS[index] = (z->DSP.MIXS[index] &~ mask) | (((*DBV_p) << shift) & mask & 0xFFFFFF);

  return;
 }

 //
 //
 //
 if(A >= 0xEC0 && A <= 0xEDF)
 {
  SCSP_NE16_RW(uint16_t, true, z->DSP.EFREG, A & 0x1F, &(*DBV_p));

  return;
 }

 if(A >= 0xEE0 && A <= 0xEE3)
 {

  return;
 }

}



//
// Gate bit only forces the value to 0, the read still seems to occur(or at the very least timing side effects occur for the case of reg->mem).
//
static NO_INLINE void SS_SCSP_RunDMA(SS_SCSP* z)
{
 if(!z->DMA_Execute)
  return;

 uint32_t length = z->DTLG;
 uint32_t mem_addr = z->DMEA;
 uint32_t reg_addr = z->DRGA;
 bool dir = z->DMA_Direction;
 bool gate = z->DMA_Gate;

 while(length)
 {
  if(dir)
  {
   uint16_t tmp;

   SS_SCSP_RW_u16_W0(z, 0x100000 | (reg_addr << 1), &tmp);

   if(gate)
    tmp = 0;

   if(MDFN_LIKELY(mem_addr < 0x40000))
    z->RAM[mem_addr] = tmp;
  }
  else
  {
   uint16_t tmp = z->RAM[mem_addr];

   if(gate)
    tmp = 0;

   SS_SCSP_RW_u16_W1(z, 0x100000 | (reg_addr << 1), &tmp);
  }

  reg_addr = (reg_addr + 1) & 0x000007FF;
  mem_addr = (mem_addr + 1) & 0x0007FFFF;
  length = length - 1;
 }

 z->DMA_Execute = false;
 z->SCIPD |= 0x10;
 z->MCIPD |= 0x10;
 SS_SCSP_RecalcSoundInt(z);
 SS_SCSP_RecalcMainInt(z);
}


static INLINE void SS_SCSP_RunEG(SS_SCSP* z, SS_SCSP_Slot* s, const unsigned key_eg_scale, const uint32_t sc, const uint32_t scxc)
{
 const uint32_t EnvLevelCache = s->EnvLevel;
 const unsigned ERateNoScale = s->EnvRates[s->EnvPhase];
 const unsigned ERate = ((unsigned)(0x1F) < (unsigned)(key_eg_scale + ERateNoScale) ? (unsigned)(0x1F) : (unsigned)(key_eg_scale + ERateNoScale));
 const unsigned ERateWBT = (0x18 - ((unsigned)(0x18) < (unsigned)(ERate) ? (unsigned)(0x18) : (unsigned)(ERate))) >> 1;
 const bool ShiftAdj = (ERate < 0x18) & ERate;
 bool ClockEG;

 //
 // The equivalent using SampleCounter instead of SampleCounterXC would be like:
 //
 //	When ShiftAdj is 0, look for a 1 at bit position ERateWBT, and lower bits must be 0, unless ERateWBT is 0, in which case
 //	bit0 must be 0 instead of 1.
 //
 //	When ShiftAdj is 1, look for a 1 at bit position ERateWBT + 1 or(inclusive) bit position ERateWBT + 2, and lower bits
 //	must be 0.
 //
 ClockEG = ((scxc | (scxc >> ShiftAdj)) >> (ERateWBT + ShiftAdj)) & (bool)ERateNoScale;

 if(s->EnvPhase == ENV_PHASE_ATTACK && (ERateNoScale + key_eg_scale) >= 0x20)
  ClockEG = false;

 if(ClockEG)
 {
  const int32_t inc_base = (s->EnvPhase == ENV_PHASE_ATTACK) ? ~EnvLevelCache : 16;
  const unsigned ermaxo = ((unsigned)(0x18) > (unsigned)(((unsigned)(0x1E) < (unsigned)(ERate) ? (unsigned)(0x1E) : (unsigned)(ERate))) ? (unsigned)(0x18) : (unsigned)(((unsigned)(0x1E) < (unsigned)(ERate) ? (unsigned)(0x1E) : (unsigned)(ERate))));
  const uint32_t srac = ((0x20 - ermaxo) >> 1) + (ermaxo & 1 & (sc >> (ERateWBT + 1)));
  int32_t NewEnvLevel;

  NewEnvLevel = EnvLevelCache + (inc_base >> srac);

  if(NewEnvLevel > 0x3FF)
   NewEnvLevel = 0x3FF;

  if(NewEnvLevel < 0)
   NewEnvLevel = 0;

  s->EnvLevel = NewEnvLevel;
 }
 //
 // Using EnvLevelCache instead of s->EnvLevel here is intentional:
 //
 if(s->EnvPhase == ENV_PHASE_DECAY1)
 {
  if((EnvLevelCache >> 5) == s->DecayLevel)
   s->EnvPhase = ENV_PHASE_DECAY2;
 }
 else if(s->EnvPhase == ENV_PHASE_ATTACK)
 {
  if((s->AttackLoopLink && s->InLoop) || (!s->AttackLoopLink && EnvLevelCache == 0))
   s->EnvPhase = ENV_PHASE_DECAY1;
 }
}

//
// Take care in handling LFSR, or else the volume of noise-ALFO-modulated noise will
// be quite off, or have a DC bias.
//
static INLINE uint8_t SS_SCSP_GetALFO(SS_SCSP* z, SS_SCSP_Slot* s)
{
 uint8_t ret;

 switch(s->ALFOWaveform)
 {
  default:
  case 0:	// Saw
	ret = s->LFOCounter &~ 1;
	break;

  case 1:	// Square
	ret = ((int8_t)s->LFOCounter >> 7) &~ 1;
	break;

  case 2:	// Triangle
	ret = (unsigned)(s->LFOCounter ^ ((int8_t)s->LFOCounter >> 7)) << 1;
	break;

  case 3:	// Noise
	ret = z->LFSR &~ 1;
	break;
 }

 ret >>= (7 - s->ALFOModLevel);

 if(!s->ALFOModLevel)
  ret = 0;

 return ret;
}

static INLINE int SS_SCSP_GetPLFO(SS_SCSP* z, SS_SCSP_Slot* s)
{
 int ret;

 switch(s->PLFOWaveform)
 {
  default:
  case 0:	// Saw
	ret = (int8_t)(s->LFOCounter &~ 1);
	break;

  case 1:	// Square
	ret = (int8_t)((s->LFOCounter & 0x80) ? 0x80 : 0x7E);
	break;

  case 2:	// Triangle
	ret = (int8_t)(((s->LFOCounter & 0x3F) ^ ((s->LFOCounter & 0x40) ? 0x3F : 0x00) ^ ((s->LFOCounter & 0x80) ? 0x7F : 0x00)) << 1);
	break;

  case 3:	// Noise
	ret = (int8_t)(z->LFSR &~ 1);
	break;
 }

 ret >>= (7 - s->PLFOModLevel);

 if(!s->PLFOModLevel)
  ret = 0;

 ret = ((0x40 ^ (s->FreqNum >> 4)) * ret) >> 6;

 return ret;
}

static INLINE void SS_SCSP_RunLFO(SS_SCSP* z, SS_SCSP_Slot* s)
{
 s->LFOTimeCounter--;
 if(!s->LFOTimeCounter)
 {
  s->LFOCounter++;
  s->LFOTimeCounter = (((8 - (s->LFOFreq & 0x3)) << 7) >> (s->LFOFreq >> 2)) - 4;
 }

 if(s->LFOReset)
  s->LFOCounter = 0;
}

//
//
//
#ifdef MDFN_SS_SCSP_DSP_DYNAREC
 #include "scsp_dsp_dynarec.inc"
#else

static INLINE uint32_t dspfloat_to_int(const uint16_t inv)
{
 const uint32_t sign_xor = (int32_t)((inv & 0x8000) << 16) >> 1;
 const uint32_t exp = (inv >> 11) & 0xF;
 uint32_t ret;

 ret = inv & 0x7FF;
 if(exp < 12)
  ret |= 0x800;
 ret <<= 11 + 8;
 ret ^= sign_xor;
 ret = (int32_t)ret >> (8 + ((unsigned)(11) < (unsigned)(exp) ? (unsigned)(11) : (unsigned)(exp)));

 return ret & 0xFFFFFF;
}

static INLINE uint32_t int_to_dspfloat(const uint32_t inv)
{
 const uint32_t invsl8 = inv << 8;
 const uint32_t sign_xor = (int32_t)invsl8 >> 31;
 uint32_t exp, shift;
 uint32_t ret;

 exp = MDFN_lzcount32(((invsl8 ^ sign_xor) << 1) | (1 << 19));
 shift = exp - (bool)(exp == 12); //((uint32_t)(11) < (uint32_t)(exp) ? (uint32_t)(11) : (uint32_t)(exp));

 ret = (int32_t)invsl8 >> (19 - shift);
 ret &= 0x87FF;
 ret |= exp << 11;

 return ret;
}

//
// Instruction field order/width RE'ing notes:
//
// Bit     0: NXADDR
// Bit     1: ADRGB
// Bit   2-6: MASA
// Bit     8: NOFL (disables floating-point conversion when =1, instead just shifting by 8); has effect with MRT=1 or MWT=1
// Bit  9-14: CRA (Coefficient read address, input into Y_SEL)
// Bit    16: BSEL
// Bit    17: ZERO
// Bit    18: NEGB (apparently no effect when ZERO=1)
// Bit    19: YRL
// Bit    20: SHFT0
// Bit    21: SHFT1
// Bit    22: FRCL
// Bit    23: ADRL (latches A_SEL output into ADRS_REG)
// Bit 24-27: EWA(EFREG write address)
// Bit    28: EWT(EFREG write enable)
// Bit    29: MRT  (Memory read trigger; to read: [MWR=1] [whatever instruction] [IWT=1]
// Bit    30: MWT  (Memory write trigger)
// Bit    31: TABLE
// Bit 32-36: IWA (MEMS write address)
// Bit    37: IWT (MEMS write trigger)
// Bit 38-43: IRA (0x00-0x1F MEMS, 0x20-0x2F MIXS)
// Bit 45-46: YSEL
// Bit    47: XSEL
// Bit 48-54: TWA(temp write address) Seems to be an offset added to a counter changed each sample.
// Bit    55: TWT(temp write trigger)  WARNING: Setting this to 1 for all 128 steps apparently can cause a CPU to freeze up if it tries to read/write TEMP afterward.
// Bit 56-62: TRA(temp read address)
static void SS_SCSP_DecodeMPROG(SS_SCSP* z)
{
 for(unsigned step = 0; step < 128; step++)
 {
  const uint64_t instr = z->DSP.MPROG[step];
  SS_SCSP_DSPStep& s = z->DSP.MPROG_Decoded[step];

  s.MASA = (instr >>  2) & 0x1F;
  s.CRA  = (instr >>  9) & 0x3F;
  s.EWA  = (instr >> 24) & 0x0F;
  s.IWA  = (instr >> 32) & 0x1F;
  s.IRA  = (instr >> 38) & 0x3F;
  s.YSEL = (instr >> 45) & 0x03;
  s.TWA  = (instr >> 48) & 0x7F;
  s.TRA  = (instr >> 56) & 0x7F;

  uint32_t f = 0;
  f |= ((instr >>  0) & 1) ? DSPF_NXADDR : 0;
  f |= ((instr >>  1) & 1) ? DSPF_ADRGB  : 0;
  f |= ((instr >>  8) & 1) ? DSPF_NOFL   : 0;
  f |= ((instr >> 16) & 1) ? DSPF_BSEL   : 0;
  f |= ((instr >> 17) & 1) ? DSPF_ZERO   : 0;
  f |= ((instr >> 18) & 1) ? DSPF_NEGB   : 0;
  f |= ((instr >> 19) & 1) ? DSPF_YRL    : 0;
  f |= ((instr >> 20) & 1) ? DSPF_SHFT0  : 0;
  f |= ((instr >> 21) & 1) ? DSPF_SHFT1  : 0;
  f |= ((instr >> 22) & 1) ? DSPF_FRCL   : 0;
  f |= ((instr >> 23) & 1) ? DSPF_ADRL   : 0;
  f |= ((instr >> 28) & 1) ? DSPF_EWT    : 0;
  f |= ((instr >> 29) & 1) ? DSPF_MRT    : 0;
  f |= ((instr >> 30) & 1) ? DSPF_MWT    : 0;
  f |= ((instr >> 31) & 1) ? DSPF_TABLE  : 0;
  f |= ((instr >> 37) & 1) ? DSPF_IWT    : 0;
  f |= ((instr >> 47) & 1) ? DSPF_XSEL   : 0;
  f |= ((instr >> 55) & 1) ? DSPF_TWT    : 0;
  s.flags = f;

  s.reads = (f & (DSPF_EWT | DSPF_TWT | DSPF_FRCL | DSPF_ADRL | DSPF_MWT | DSPF_BSEL)) ? DSPR_SFT : 0;

  uint8_t writes = DSPW_SFT;
  if(f & DSPF_FRCL)                  writes |= DSPW_FRC;
  if(f & DSPF_YRL)                   writes |= DSPW_Y;
  if(f & DSPF_ADRL)                  writes |= DSPW_ADRS;
  if(f & DSPF_TWT)                   writes |= DSPW_TEMP;
  if(f & DSPF_IWT)                   writes |= DSPW_MEMS;
  if(f & DSPF_EWT)                   writes |= DSPW_EFREG;
  if(f & (DSPF_MRT | DSPF_MWT))      writes |= DSPW_RAM;
  s.writes = writes;

  s.live = 1;		// filled in by liveness pass
 }

 // Liveness pass: a step is dead if its only observable writes are SFT_REG and/or
 // INPUTS, and the next step (wrapping for step 127 -> step 0 across the sample
 // boundary) doesn't observe either of them.  INPUTS is preserved when IRA is in
 // [0x32..0x3F]; otherwise the step writes INPUTS via MEMS/MIXS/z->EXTS.
 for(unsigned step = 0; step < 128; step++)
 {
  const SS_SCSP_DSPStep& cur = z->DSP.MPROG_Decoded[step];
  const SS_SCSP_DSPStep& nxt = z->DSP.MPROG_Decoded[(step + 1) & 0x7F];
  const uint32_t nf = nxt.flags;

  const bool has_other_writes  = (cur.writes & ~(unsigned)DSPW_SFT) != 0;
  const bool sft_observed      = (nxt.reads & DSPR_SFT) != 0;

  const bool cur_writes_inputs    = !((cur.IRA & 0x30) == 0x30 && (cur.IRA & 0x0E) != 0);
  const bool nxt_preserves_inputs =  ((nxt.IRA & 0x30) == 0x30 && (nxt.IRA & 0x0E) != 0);
  const bool nxt_uses_inputs      = (nf & DSPF_YRL) || (nf & DSPF_XSEL) ||
                                    ((nf & DSPF_ADRL) && !((nf & DSPF_SHFT0) && (nf & DSPF_SHFT1)));
  const bool inputs_observed      = cur_writes_inputs && nxt_preserves_inputs && nxt_uses_inputs;

  z->DSP.MPROG_Decoded[step].live = (has_other_writes || sft_observed || inputs_observed) ? 1 : 0;
 }

 // A z->RAM op triggered by step N (MRT/MWT) is not serviced in that step:
 // RWAddr/ReadPending/WritePending are latched at its tail, and step N+1's body
 // commits it (z->RAM read -> ReadValue, or z->RAM <- WriteValue) *before* doing its
 // own IWT.  If step N+1 were elided, that commit would slip onto a later live
 // step, past the IWT consumer at N+2, which would then latch a stale ReadValue.
 // So pin the successor live, wrapping at the sample boundary (the z->RAM pipeline
 // persists into the next SS_SCSP_RunDSP).
 for(unsigned step = 0; step < 128; step++)
 {
  if(z->DSP.MPROG_Decoded[step].flags & (DSPF_MRT | DSPF_MWT))
   z->DSP.MPROG_Decoded[(step + 1) & 0x7F].live = 1;
 }

 z->DSP.MPROG_Dirty = false;
}

static INLINE void SS_SCSP_RunDSP(SS_SCSP* z)
{
 if(z->DSP.MPROG_Dirty)
  SS_SCSP_DecodeMPROG(z);

 for(unsigned step = 0; step < 128; step++)
 {
  const SS_SCSP_DSPStep& s = z->DSP.MPROG_Decoded[step];
  if(!s.live) continue;
  const uint32_t f = s.flags;
  const unsigned IRA = s.IRA;
  const unsigned TEMPWriteAddr = (s.TWA + z->DSP.MDEC_CT) & 0x7F;
  const unsigned TEMPReadAddr  = (s.TRA + z->DSP.MDEC_CT) & 0x7F;

  //
  //
  if(IRA & 0x20)
  {
   if(IRA & 0x10)
   {
    if(!(IRA & 0xE))
     z->DSP.INPUTS = z->EXTS[IRA & 0x1] << 8;
   }
   else
   {
    z->DSP.INPUTS = z->DSP.MIXS[IRA & 0xF] << 4;
   }
  }
  else
  {
   z->DSP.INPUTS = z->DSP.MEMS[IRA & 0x1F];
  }

  const int32_t INPUTS = sign_x_to_s32(24, z->DSP.INPUTS);
  //
  // Y multiplicand selector.  YSEL is a 2-bit field; expressing it as a
  // runtime-indexed local array makes the compiler materialize a 4-entry
  // stack table and read back through `ldrh [sp, idx, lsl #1]` with a
  // variable index — a store-to-load forwarding stall, the single hottest
  // line in SS_SCSP_RunDSP.  A csel chain keeps the four candidates in registers.
  const uint16_t y_frc   = z->DSP.FRC_REG;
  const uint16_t y_coef  = z->DSP.COEF[s.CRA];
  const uint16_t y_yhi   = (z->DSP.Y_REG >> 11) & 0x1FFF;
  const uint16_t y_ylo   = (z->DSP.Y_REG >>  4) & 0x0FFF;
  const uint16_t y_lopr  = (s.YSEL & 1) ? y_coef : y_frc;
  const uint16_t y_hipr  = (s.YSEL & 1) ? y_ylo  : y_yhi;
  const uint16_t y_input = (s.YSEL & 2) ? y_hipr : y_lopr;
  //
  //
  //
  if(f & DSPF_YRL)
  {
   z->DSP.Y_REG = INPUTS & 0xFFFFFF;
  }
  //
  //
  //
  const unsigned shft0 = (f >> 7) & 1;
  const unsigned shft1 = (f >> 8) & 1;
  int32_t ShifterOutput = (uint32_t)sign_x_to_s32(26, z->DSP.SFT_REG) << (shft0 ^ shft1);

  if(!shft1)
  {
   if(ShifterOutput > 0x7FFFFF)
    ShifterOutput = 0x7FFFFF;
   else if(ShifterOutput < -0x800000)
    ShifterOutput = 0x800000;
  }
  ShifterOutput &= 0xFFFFFF;
  //
  //
  if(f & DSPF_FRCL)
  {
   // F_SEL: shft0&shft1 picks low-12 vs high-13 bits of ShifterOutput.
   z->DSP.FRC_REG = (shft0 & shft1) ? (unsigned)(ShifterOutput & 0xFFF)
                                 : (unsigned)(ShifterOutput >> 11);
  }
  //
  //
  {
   const int32_t TEMP = sign_x_to_s32(24, z->DSP.TEMP[TEMPReadAddr]);
   // X_SEL: DSPF_XSEL picks INPUTS over TEMP for the multiplier's X side.
   const int32_t x_input = (f & DSPF_XSEL) ? INPUTS : TEMP;
   const uint32_t Product = ((int64_t)sign_x_to_s32(13, y_input) * x_input) >> 12;
   // SGA: DSPF_BSEL picks SFT_REG over TEMP for the SGA add input.
   uint32_t SGAOutput = (f & DSPF_BSEL) ? z->DSP.SFT_REG : (uint32_t)TEMP;

   if(f & DSPF_NEGB)
    SGAOutput = -SGAOutput;

   if(f & DSPF_ZERO)
    SGAOutput = 0;

   z->DSP.SFT_REG = (Product + SGAOutput) & 0x3FFFFFF;
  }
  //
  //
  if(f & DSPF_EWT)
   z->DSP.EFREG[s.EWA] = (ShifterOutput >> 8);

  if(f & DSPF_TWT)
   z->DSP.TEMP[TEMPWriteAddr] = ShifterOutput;

  if(f & DSPF_IWT)
  {
   z->DSP.MEMS[s.IWA] = z->DSP.ReadValue;
  }
  //
  //
  if(z->DSP.ReadPending)
  {
   uint16_t tmp = z->RAM[z->DSP.RWAddr];
   z->DSP.ReadValue = (z->DSP.ReadPending == 2) ? (tmp << 8) : dspfloat_to_int(tmp);
   z->DSP.ReadPending = false;
  }
  else if(z->DSP.WritePending)
  {
   if(!(z->DSP.RWAddr & 0x40000))
    z->RAM[z->DSP.RWAddr] = z->DSP.WriteValue;

   z->DSP.WritePending = false;
  }

  {
   uint16_t addr;

   addr = z->DSP.MADRS[s.MASA];
   addr += (f & DSPF_NXADDR) ? 1 : 0;

   if(f & DSPF_ADRGB)
   {
    addr += sign_x_to_s32(12, z->DSP.ADRS_REG);
   }

   if(!(f & DSPF_TABLE))
   {
    addr += z->DSP.MDEC_CT;
    addr &= (0x2000 << z->RBL) - 1;
   }

   z->DSP.RWAddr = (addr + (z->RBP << 12)) & 0x7FFFF;

   if(f & DSPF_MRT)
   {
    z->DSP.ReadPending = (f & DSPF_NOFL) ? 2 : 1;
   }
   if(f & DSPF_MWT)
   {
    z->DSP.WritePending = true;
    z->DSP.WriteValue = (f & DSPF_NOFL) ? (ShifterOutput >> 8) : int_to_dspfloat(ShifterOutput);
   }
  }
  //
  //
  if(f & DSPF_ADRL)
  {
   // A_SEL: shft0&shft1 picks ShifterOutput>>12 vs INPUTS>>16. INPUTS is
   // sign-extended above; both branches truncate to 12 bits.
   z->DSP.ADRS_REG = (shft0 & shft1) ? (uint16_t)(ShifterOutput >> 12)
                                  : (uint16_t)((INPUTS >> 16) & 0xFFF);
  }
 }

 if(!z->DSP.MDEC_CT)
  z->DSP.MDEC_CT = (0x2000 << z->RBL);
 z->DSP.MDEC_CT--;
}
#endif
//
//
//
/* Phase-8f: RunSample detempleted -- hardcoded T_out = int16_t.
 * See the matching note in scsp.h. */
INLINE void SS_SCSP_RunSample(SS_SCSP* z, int16_t* outlr)
{
 const uint32_t SampleCounter = z->GlobalCounter >> 5;
 const uint32_t SampleCounterXC = (SampleCounter ^ (SampleCounter - 1)) & (SampleCounter ^ 1);
 int32_t out_accum[2] = { 0, 0 };

 SS_SCSP_MIDI_Run(z);

 for(unsigned i = 0; i < 3; i++)
 {
  SS_SCSP_Timer* t = &z->Timers[i];
  const bool DoClock = !(SampleCounter & ((1U << t->Control) - 1));

  if(DoClock)
  {
   if(t->Reload >= 0)
   {
    t->Counter = t->Reload;
    t->Reload = -1;
   }
   else
    t->Counter++;

   if(t->Counter == 0xFF)
   {
    z->SCIPD |= 0x40 << i;
    z->MCIPD |= 0x40 << i;
   }
  }
 }

 z->SCIPD |= 0x400;
 z->MCIPD |= 0x400;
 SS_SCSP_RecalcSoundInt(z);
 SS_SCSP_RecalcMainInt(z);

 //
 //
 //
 SS_SCSP_RunDSP(z);

 for(unsigned i = 0; i < 0x10; i++)
  z->DSP.MIXS[i] = 0;
 //
 //
 //
 for(unsigned slot = 0; slot < 32; slot++)
 {
  SS_SCSP_Slot* s = &z->Slots[slot];
  unsigned key_eg_scale;

  s->WFAllowAccess &= (s->EnvLevel < 0x3C0 || s->EGBypass);
  //
  //
  if(s->KRS == 0xF)
   key_eg_scale = 0x00;
  else
   key_eg_scale = ((int)(0x00) > (int)(((int)(0x0F) < (int)(s->KRS + (s->Octave ^ 0x8) - 0x8) ? (int)(0x0F) : (int)(s->KRS + (s->Octave ^ 0x8) - 0x8))) ? (int)(0x00) : (int)(((int)(0x0F) < (int)(s->KRS + (s->Octave ^ 0x8) - 0x8) ? (int)(0x0F) : (int)(s->KRS + (s->Octave ^ 0x8) - 0x8))));

  SS_SCSP_RunEG(z, s, key_eg_scale, SampleCounter, SampleCounterXC);

  if(z->KeyExecute && (s->EnvPhase == ENV_PHASE_RELEASE) == s->KeyBit)
  {
   if(s->KeyBit)
   {
    s->PhaseWhacker = 0;
    s->CurrentAddr = 0;
    s->InLoop = false;
    s->LoopSub = false;
    s->WFAllowAccess = true;
    s->EnvPhase = ENV_PHASE_ATTACK;

    if((s->EnvRates[ENV_PHASE_ATTACK] + key_eg_scale) >= 0x20)
     s->EnvLevel = 0x000;
    else
     s->EnvLevel = 0x280;
   }
   else
    s->EnvPhase = ENV_PHASE_RELEASE;
  }
  //
  //
  if(!s->InLoop)
  {
   if((uint16_t)(s->CurrentAddr + 1) > s->LoopStart)
   {
    if(s->LoopMode == 2)
    {
     s->CurrentAddr += -(s->LoopStart + s->LoopEnd);
     s->LoopSub = true;
    }

    s->InLoop = true;
   }
  }
  else
  {
   const uint16_t ca = 1 + (s->LoopSub ? ~s->CurrentAddr : s->CurrentAddr);
   const uint16_t comp = (s->LoopSub && (s->LoopMode & 0x2)) ? s->LoopStart : s->LoopEnd;

   if(s->LoopSub ^ (ca > comp))
   {
    if(s->LoopMode == 0)
     s->WFAllowAccess = false;
    
    if(s->LoopMode == 3)
    {
     s->LoopSub = !s->LoopSub;
     if(s->LoopSub)
      s->CurrentAddr -= s->LoopEnd << 1;
     else
      s->CurrentAddr += s->LoopStart << 1;
    }
    else
    {
     if(s->LoopSub && !(s->LoopMode & 0x2))
      s->CurrentAddr += s->LoopEnd - s->LoopStart;
     else
      s->CurrentAddr += s->LoopStart - s->LoopEnd;
    }
   }
  }
 }

 for(unsigned slot = 0; slot < 32; slot++)
 {
  SS_SCSP_Slot* s = &z->Slots[slot];
  uint32_t mdata = 0;
  uint16_t sample = 0;

  if(s->SourceControl == 1)
   sample = z->LFSR << 8;

  sample ^= s->SBXOR;	// For zero and noise case only; waveform playback needs it to occur before linear interpolation.
  //
  //
  if(s->WFAllowAccess)
  {
   uint32_t modalizer_int[2];
   uint32_t tmppw = s->PhaseWhacker;
   uint16_t tmpa = s->CurrentAddr;
   //
   //
   if(s->LoopSub)
   {
    tmppw = ~tmppw;
    tmpa = ~tmpa;
   }

   mdata |= ((tmpa >> 12) << 7);
   //
   //
   uint32_t sia;
   int16_t s0, s1;

   {
    //
    // TODO/FIXME: Proper handling of the slot 31->0 buggy FM interpolation case with respect to reverse looping(ns->LoopSub)
    // requires sub-sample timing emulation.
    //
    SS_SCSP_Slot* ns = &z->Slots[(slot + 1) & 0x1F];
    uint32_t modalizer;
    uint32_t ns_sia;

    modalizer  = (int16_t)z->SoundStack[(z->GlobalCounter + s->ModInputX) & 0x3F];
    modalizer += (int16_t)z->SoundStack[(z->GlobalCounter + s->ModInputY) & 0x3F];
    modalizer = ((modalizer << 6) >> (0x10 - s->ModLevel)) & ~1;

    if(s->ModLevel <= 0x04)
     modalizer = 0;

    sia = modalizer + ((tmppw >> (14 - 6)) & 0x3F);
    ns_sia = modalizer + (((ns->PhaseWhacker >> (14 - 6)) ^ (ns->LoopSub ? 0x3F : 0x00)) & 0x3F);

    modalizer_int[0] = sign_x_to_s32(11,    sia >> 6);
    modalizer_int[1] = sign_x_to_s32(11, ns_sia >> 6);
    //
    sia &= 0x3F;
   }

   if(s->WF8Bit)
   {
    const uint32_t addr0 = (s->StartAddr + ((modalizer_int[0] + (uint16_t)(tmpa + 0)) & s->ShortWaveMask)) & 0xFFFFF;
    const uint32_t addr1 = (s->StartAddr + ((modalizer_int[1] + (uint16_t)(tmpa + 1)) & s->ShortWaveMask)) & 0xFFFFF;

    s0 = SCSP_RBO_BE_U8(z->RAM, addr0) << 8;
    s1 = SCSP_RBO_BE_U8(z->RAM, addr1) << 8;
   }
   else
   {
    s0 = z->RAM[((s->StartAddr >> 1) + ((modalizer_int[0] + (uint16_t)(tmpa + 0)) & s->ShortWaveMask)) & 0x7FFFF];
    s1 = z->RAM[((s->StartAddr >> 1) + ((modalizer_int[1] + (uint16_t)(tmpa + 1)) & s->ShortWaveMask)) & 0x7FFFF];
   }

   s0 ^= s->SBXOR;
   s1 ^= s->SBXOR;

   if(s->SourceControl == 0)
   {
    sample = ((s0 * (0x40 - sia)) + (s1 * sia)) >> 6;
   }

   s->PhaseWhacker += (((0x400 ^ s->FreqNum) + SS_SCSP_GetPLFO(z, s)) << (s->Octave ^ 0x8)) >> 4;
   s->CurrentAddr += s->PhaseWhacker >> 14;
   s->PhaseWhacker &= (1U << 14) - 1;
  }
  //
  //

  SS_SCSP_RunLFO(z, s);	// Run between PLFO fetching and ALFO fetching.

  // Do z->LFSR clocking between sample fetching and ALFO fetching.
  z->LFSR = (z->LFSR >> 1) | (((z->LFSR >> 5) ^ z->LFSR) & 1) << 16;

  
  {
   int32_t vlevel;

   vlevel = ((s->EnvPhase == ENV_PHASE_ATTACK && s->AttackHold) || s->EGBypass) ? 0 : s->EnvLevel;
   //
   mdata |= (s->EnvPhase << 5) | (vlevel >> 5);
   //
   if(!s->SoundDirect)
   {
    vlevel += s->TotalLevel << 2;
    vlevel += SS_SCSP_GetALFO(z, s);

    if(vlevel > 0x3FF)
     vlevel = 0x3FF;

    sample = ((int16_t)sample * ((vlevel & 0x3F) ^ 0x7F)) >> ((vlevel >> 6) + 7);
   }
  }

  {
   uint64_t packed;
   memcpy(&packed, z->SoundStackDelayer, sizeof(z->SoundStackDelayer));

   if(!z->Slots[(z->GlobalCounter - 4) & 0x1F].StackWriteInhibit)
   {
#ifdef MSB_FIRST
    z->SoundStack[(z->GlobalCounter - 4) & 0x3F] = (uint16_t)packed;
#else
    z->SoundStack[(z->GlobalCounter - 4) & 0x3F] = (uint16_t)(packed >> 48);
#endif
   }

#ifdef MSB_FIRST
   packed = (packed >> 16) | ((uint64_t)(uint16_t)sample << 48);
#else
   packed = (packed << 16) | (uint16_t)sample;
#endif
   memcpy(z->SoundStackDelayer, &packed, sizeof(z->SoundStackDelayer));
  }
  //
  //
  if(z->SlotMonitorWhich == slot)
   z->SlotMonitorData = mdata;
  //
  //
  if(s->ToDSPLevel)
   z->DSP.MIXS[s->ToDSPSelect] = (z->DSP.MIXS[s->ToDSPSelect] + (((uint32_t)(int16_t)sample << 4) >> (7 - s->ToDSPLevel))) & 0xFFFFF;
  //
  //
  out_accum[0] += ((int16_t)sample * s->DirectVolume[0]) >> 14;
  out_accum[1] += ((int16_t)sample * s->DirectVolume[1]) >> 14;

  {
   const uint16_t eff_sample = (slot & 0x10) ? ((slot & 0xE) ? 0 : z->EXTS[slot & 0x1]) : z->DSP.EFREG[slot];

   out_accum[0] += ((int16_t)eff_sample * s->EffectVolume[0]) >> 14;
   out_accum[1] += ((int16_t)eff_sample * s->EffectVolume[1]) >> 14;
  }
  //
  //
  z->GlobalCounter++;
 }

 z->KeyExecute = false;

 //
 //
 //
 out_accum[0] = (out_accum[0] * z->MasterVolume) >> 8;
 out_accum[1] = (out_accum[1] * z->MasterVolume) >> 8;

 out_accum[0] = ((int32_t)(32767) < (int32_t)(((int32_t)(-32768) > (int32_t)(out_accum[0]) ? (int32_t)(-32768) : (int32_t)(out_accum[0]))) ? (int32_t)(32767) : (int32_t)(((int32_t)(-32768) > (int32_t)(out_accum[0]) ? (int32_t)(-32768) : (int32_t)(out_accum[0]))));
 out_accum[1] = ((int32_t)(32767) < (int32_t)(((int32_t)(-32768) > (int32_t)(out_accum[1]) ? (int32_t)(-32768) : (int32_t)(out_accum[1]))) ? (int32_t)(32767) : (int32_t)(((int32_t)(-32768) > (int32_t)(out_accum[1]) ? (int32_t)(-32768) : (int32_t)(out_accum[1]))));

 if(z->DAC18bit)
 {
  // Doesn't seem to improve precision.
  out_accum[0] = (uint32_t)out_accum[0] << 2;
  out_accum[1] = (uint32_t)out_accum[1] << 2;
 }

 outlr[0] = out_accum[0];
 outlr[1] = out_accum[1];
}

//
//
//
void SS_SCSP_StateAction(SS_SCSP* z, StateMem* sm, const unsigned load, const bool data_only, const char* sname)
{
 /* Field-name strings below intentionally omit the "z->" prefix:
  * they are persisted into savestate files, so they must match
  * the pre-phase-9 (member-function StateAction) spelling. */
 SFORMAT StateRegs[] =
 {
  SFPTR16N(&(z->SlotRegs)[0][0], (sizeof(z->SlotRegs) / sizeof(uint16_t)), "SlotRegs"),

  SFVARN(z->Slots->PhaseWhacker,   32, sizeof(*z->Slots), z->Slots, "Slots->PhaseWhacker"),
  SFVARN(z->Slots->CurrentAddr,    32, sizeof(*z->Slots), z->Slots, "Slots->CurrentAddr"),
  SFVARN(z->Slots->InLoop,         32, sizeof(*z->Slots), z->Slots, "Slots->InLoop"),
  SFVARN(z->Slots->LoopSub,        32, sizeof(*z->Slots), z->Slots, "Slots->LoopSub"),
  SFVARN(z->Slots->WFAllowAccess,  32, sizeof(*z->Slots), z->Slots, "Slots->WFAllowAccess"),
  SFVARN(z->Slots->EnvLevel,       32, sizeof(*z->Slots), z->Slots, "Slots->EnvLevel"),
  SFVARN(z->Slots->EnvPhase,       32, sizeof(*z->Slots), z->Slots, "Slots->EnvPhase"),
  SFVARN(z->Slots->LFOCounter,     32, sizeof(*z->Slots), z->Slots, "Slots->LFOCounter"),
  SFVARN(z->Slots->LFOTimeCounter, 32, sizeof(*z->Slots), z->Slots, "Slots->LFOTimeCounter"),

  SFPTR16N(&(z->EXTS)[0], (sizeof(z->EXTS) / sizeof(uint16_t)), "EXTS"),

  SFPTR16N(&(z->SoundStack)[0], (sizeof(z->SoundStack) / sizeof(uint16_t)), "SoundStack"),
  SFPTR16N(&(z->SoundStackDelayer)[0], (sizeof(z->SoundStackDelayer) / sizeof(uint16_t)), "SoundStackDelayer"),

  SFVARN(z->MasterVolume, "MasterVolume"),
  SFVARN(z->MVOL,         "MVOL"),
  SFVARN(z->DAC18bit,     "DAC18bit"),
  SFVARN(z->Mem4Mb,       "Mem4Mb"),

  SFVARN(z->SlotMonitorWhich, "SlotMonitorWhich"),
  SFVARN(z->SlotMonitorData,  "SlotMonitorData"),

  SFVARN(z->KeyExecute,    "KeyExecute"),
  SFVARN(z->LFSR,          "LFSR"),
  SFVARN(z->GlobalCounter, "GlobalCounter"),

  SFPTR8N(&(z->MIDI.InputFIFO)[0], (sizeof(z->MIDI.InputFIFO) / sizeof(uint8_t)), "MIDI.InputFIFO"),
  SFVARN(z->MIDI.InputRP,    "MIDI.InputRP"),
  SFVARN(z->MIDI.InputWP,    "MIDI.InputWP"),
  SFVARN(z->MIDI.InputCount, "MIDI.InputCount"),
  SFPTR8N(&(z->MIDI.OutputFIFO)[0], (sizeof(z->MIDI.OutputFIFO) / sizeof(uint8_t)), "MIDI.OutputFIFO"),
  SFVARN(z->MIDI.OutputRP,           "MIDI.OutputRP"),
  SFVARN(z->MIDI.OutputWP,           "MIDI.OutputWP"),
  SFVARN(z->MIDI.OutputCount,        "MIDI.OutputCount"),
  SFVARN(z->MIDI.Flags,              "MIDI.Flags"),
  SFVARN(z->MIDI.SimuClockDivider,   "MIDI.SimuClockDivider"),
  SFVARN(z->MIDI.TransmitBitCounter, "MIDI.TransmitBitCounter"),
  SFVARN(z->MIDI.TransmitBuffer,     "MIDI.TransmitBuffer"),

  SFVARN(z->SCIEB, "SCIEB"),
  SFVARN(z->SCIPD, "SCIPD"),

  SFVARN(z->MCIEB, "MCIEB"),
  SFVARN(z->MCIPD, "MCIPD"),

  SFPTR8N(&(z->SCILV)[0], (sizeof(z->SCILV) / sizeof(uint8_t)), "SCILV"),

  SFVARN(z->Timers->Control, 3, sizeof(*z->Timers), z->Timers, "Timers->Control"),
  SFVARN(z->Timers->Counter, 3, sizeof(*z->Timers), z->Timers, "Timers->Counter"),
  SFVARN(z->Timers->Reload,  3, sizeof(*z->Timers), z->Timers, "Timers->Reload"),

  SFVARN(z->DMEA, "DMEA"),
  SFVARN(z->DRGA, "DRGA"),
  SFVARN(z->DTLG, "DTLG"),

  SFVARN(z->DMA_Execute,   "DMA_Execute"),
  SFVARN(z->DMA_Direction, "DMA_Direction"),
  SFVARN(z->DMA_Gate,      "DMA_Gate"),

  SFVARN(z->RBP, "RBP"),
  SFVARN(z->RBL, "RBL"),

  SFPTR64N(&(z->DSP.MPROG)[0], (sizeof(z->DSP.MPROG) / sizeof(uint64_t)), "DSP.MPROG"),
  SFPTR32N(&(z->DSP.TEMP)[0],  (sizeof(z->DSP.TEMP)  / sizeof(uint32_t)), "DSP.TEMP"),
  SFPTR32N(&(z->DSP.MEMS)[0],  (sizeof(z->DSP.MEMS)  / sizeof(uint32_t)), "DSP.MEMS"),
  SFPTR16N(&(z->DSP.COEF)[0],  (sizeof(z->DSP.COEF)  / sizeof(uint16_t)), "DSP.COEF"),
  SFPTR16N(&(z->DSP.MADRS)[0], (sizeof(z->DSP.MADRS) / sizeof(uint16_t)), "DSP.MADRS"),
  SFPTR32N(&(z->DSP.MIXS)[0],  (sizeof(z->DSP.MIXS)  / sizeof(uint32_t)), "DSP.MIXS"),
  SFPTR16N(&(z->DSP.EFREG)[0], (sizeof(z->DSP.EFREG) / sizeof(uint16_t)), "DSP.EFREG"),

  SFVARN(z->DSP.INPUTS, "DSP.INPUTS"),

  SFVARN(z->DSP.SFT_REG,  "DSP.SFT_REG"),
  SFVARN(z->DSP.FRC_REG,  "DSP.FRC_REG"),
  SFVARN(z->DSP.Y_REG,    "DSP.Y_REG"),
  SFVARN(z->DSP.ADRS_REG, "DSP.ADRS_REG"),

  SFVARN(z->DSP.MDEC_CT,  "DSP.MDEC_CT"),

  SFVARN(z->DSP.RWAddr,   "DSP.RWAddr"),

  SFVARN(z->DSP.WritePending, "DSP.WritePending"),
  SFVARN(z->DSP.WriteValue,   "DSP.WriteValue"),

  SFVARN(z->DSP.ReadPending,  "DSP.ReadPending"),
  SFVARN(z->DSP.ReadValue,    "DSP.ReadValue"),
  //
  SFPTR16N(z->RAM, 262144, "RAM"),

  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, sname, false);

 if(load)
 {
  /* Range-based `for(auto& s : z->Slots)` retired for C-compat
   * (will be needed once sound_glue.cpp -> .c).  Same iteration. */
  {
   unsigned i;
   for(i = 0; i < 32; i++)
   {
    SS_SCSP_Slot* s = &z->Slots[i];
    s->EnvLevel &= 0x3FF;
    s->EnvPhase &= 0x3;
   }
  }

  z->SlotMonitorWhich &= 0x1F;

  z->MIDI.InputRP &= 0x3;
  z->MIDI.InputWP &= 0x3;

  z->MIDI.OutputRP &= 0x3;
  z->MIDI.OutputWP &= 0x3;

  z->DMEA &= 0x7FFFF;
  z->DRGA &= 0x7FF;
  z->DTLG &= 0x7FF;

  z->RBP &= 0x7F;
  z->RBL &= 0x3;

  z->DSP.RWAddr &= 0x7FFFF;
  
  z->DSP.MPROG_Dirty = true;

  for(uint32_t A = 0x100000; A < 0x100400; A += 2)
  {
   SS_SCSP_RW_u16_W1(z, A, MDAP(z->SlotRegs) + ((A & 0x3FE) >> 1));
  }
  SS_SCSP_RecalcSoundInt(z);
  SS_SCSP_RecalcMainInt(z);
 }

}

//
//
//
uint32_t SS_SCSP_GetRegister(SS_SCSP* z, const unsigned id, char* const special, const uint32_t special_len)
{
 uint32_t ret = 0xDEADBEEF;

 switch(id)
 {
  case GSREG_MVOL:
	ret = z->MVOL;
	break;

  case GSREG_DAC18B:
	ret = z->DAC18bit;
	break;

  case GSREG_MEM4MB:
	ret = z->Mem4Mb;
	break;

  case GSREG_RBC:
	ret = z->RBP | (z->RBL << 7);
	break;

  case GSREG_MSLC:
	ret = z->SlotMonitorWhich;
	break;

  case GSREG_SCIEB:
	ret = z->SCIEB;
	break;

  case GSREG_SCIPD:
	ret = z->SCIPD;
	break;

  case GSREG_MCIEB:
	ret = z->MCIEB;
	break;

  case GSREG_MCIPD:
	ret = z->MCIPD;
	break;

  case GSREG_EFREG0 ... GSREG_EFREGF:
	ret = z->DSP.EFREG[id - GSREG_EFREG0];
	break;
 }

 return ret;
}

void SS_SCSP_SetRegister(SS_SCSP* z, const unsigned id, const uint32_t value)
{
 switch(id)
 {
  //case GSREG_MVOL: z->MVOL = value & 0xF; // TODO cache

  case GSREG_DAC18B:
	z->DAC18bit = value & 1;
	break;

  case GSREG_MEM4MB:
	z->Mem4Mb = value & 1;
	break;

  case GSREG_RBC:
	z->RBP = value & 0x7F;
	z->RBL = (value >> 7) & 0x3;
	break;

  case GSREG_MSLC:
	z->SlotMonitorWhich = value & 0x1F;
	break;

  case GSREG_EFREG0 ... GSREG_EFREGF:
	z->DSP.EFREG[id - GSREG_EFREG0] = (uint16_t)value;
	break;
 }
}

