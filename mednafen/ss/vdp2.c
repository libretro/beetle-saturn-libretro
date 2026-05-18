/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* vdp2.c - VDP2 Emulation
**  Copyright (C) 2015-2019 Mednafen Team
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

// TODO: Emulate brokenness(missing vblank) that occurs when switching from 240-height mode to 224-height mode around lines 224-239.

// TODO: Update output signals on Reset()?  Might have to call VDP2::Reset() before other _Reset() then...and take special care in the
// SMPC clock change code.

#include "ss.h"
#include "vdp1.h"
#include "vdp2.h"
#include "scu.h"
#include "smpc.h"

#include "vdp2_common.h"
#include "vdp2_render.h"

/* This TU was formerly wrapped in `namespace VDP2 { ... }` -- now
 * converted to C, with the 19 public entry points renamed to the
 * VDP2_ prefix that the extern "C" proxies in this file used to
 * provide.  Internal callsites between the namespace's own
 * functions (only `Update(SH7095_mem_timestamp)`, hit at 3 sites
 * in the SCU / register-write paths) were updated to the prefixed
 * names.  mednafen.h / general.h / sh7095.h dropped: the no longer used
 * git.h transitive that mednafen.h pulled in is no longer
 * tolerable, general.h was unused, and sh7095.h was only providing
 * the SH7095 class -- this TU never used the class, only the
 * SH7095_mem_timestamp global which ss.h declares.
 *
 * Local forward decl: the SetExtHaltDMAKludgeFromVDP2 class method
 * call in the HORRIBLEHACK_NOSH2DMALINE106 path goes through a
 * matching extern "C" proxy in ss.c (where the CPU[2] global
 * lives).  Same forward-decl pattern smpc.c uses. */
extern void SH7095_SetExtHaltDMAKludge(int cpu, bool state);

static bool PAL;
static sscpu_timestamp_t lastts;
//
//
//
static uint16_t RawRegs[0x100];	// For debugging

static bool DisplayOn;
static bool BorderMode;
bool ExLatchEnable;
static bool ExSyncEnable;
static bool ExBGEnable;
static bool DispAreaSelect;

static bool VRAMSize;

static uint8_t HRes, VRes;
static uint8_t InterlaceMode;
enum { IM_NONE, IM_ILLEGAL, IM_SINGLE, IM_DOUBLE };

static uint16_t RAMCTL_Raw;
static uint8_t CRAM_Mode;
enum
{
 CRAM_MODE_RGB555_1024	= 0,
 CRAM_MODE_RGB555_2048	= 1,
 CRAM_MODE_RGB888_1024	= 2,
 CRAM_MODE_ILLEGAL	= 3
};

static uint16_t BGON;
static uint8_t VCPRegs[4][8];
static uint32_t VRAMPenalty[4];

static uint32_t RPTA;
static uint8_t RPRCTL[2];
static uint8_t KTAOF[2];

static struct
{
 uint16_t YStart, YEnd;
 bool YEndMet;
 bool YIn;
} Window[2];

static uint16_t VRAM[262144];

static uint16_t CRAM[2048];

static struct
{
 // Signed values are stored sign-extended to the full 32 bits.
 int32_t Xst, Yst, Zst;	// 1.12.10
 int32_t DXst, DYst;	// 1. 2.10
 int32_t DX, DY;		// 1. 2.10
 int32_t RotMatrix[6];	// 1. 3.10
 int32_t Px, Py, Pz;	// 1.13. 0
 int32_t Cx, Cy, Cz;	// 1.13. 0
 int32_t Mx, My;		// 1.13.10
 int32_t kx, ky;		// 1. 7.16

 uint32_t KAst;		// 0.16.10
 uint32_t DKAst;		// 1. 9.10
 uint32_t DKAx;		// 1. 9.10
 //
 //
 //
 uint32_t XstAccum, YstAccum;	// 1.12.10 (sorta)
 uint32_t KAstAccum;		//     .10
} RotParams[2];

static void FetchRotParams(const bool field)
{
 uint32_t a = RPTA & 0x7FFBE;

 /* ne16_rbo_be<uint32_t>(VRAM, idx*2) folded.  VRAM is uint16_t[]
  * holding a big-endian 16-bit bus.  A 32-bit big-endian read
  * from byte-offset (idx*2) is two consecutive uint16_t reads
  * where the first holds the upper half and the second holds
  * the lower half.  Expression works identically on BE and LE
  * host: the result is always a host-endian uint32_t built from
  * host-endian uint16_t halves in MSB-first order. */
#define BE32_VRAM(idx_expr) (((uint32_t)VRAM[(idx_expr)] << 16) | VRAM[(idx_expr) + 1])

 for(unsigned i = 0; i < 2; i++)
 {
  __typeof__(RotParams[i]) *rp = &RotParams[i];

  rp->Xst = sign_x_to_s32(23, BE32_VRAM((a + 0x00) & 0x3FFFF) >> 6);
  rp->Yst = sign_x_to_s32(23, BE32_VRAM((a + 0x02) & 0x3FFFF) >> 6);
  rp->Zst = sign_x_to_s32(23, BE32_VRAM((a + 0x04) & 0x3FFFF) >> 6);

  rp->DXst = sign_x_to_s32(13, BE32_VRAM((a + 0x06) & 0x3FFFF) >> 6);
  rp->DYst = sign_x_to_s32(13, BE32_VRAM((a + 0x08) & 0x3FFFF) >> 6);

  rp->DX = sign_x_to_s32(13, BE32_VRAM((a + 0x0A) & 0x3FFFF) >> 6);
  rp->DY = sign_x_to_s32(13, BE32_VRAM((a + 0x0C) & 0x3FFFF) >> 6);

  for(unsigned m = 0; m < 6; m++)
  {
   rp->RotMatrix[m] = sign_x_to_s32(14, BE32_VRAM((a + 0x0E + (m << 1)) & 0x3FFFF) >> 6);
  }

  rp->Px = sign_x_to_s32(14, VRAM[(a + 0x1A) & 0x3FFFF]);
  rp->Py = sign_x_to_s32(14, VRAM[(a + 0x1B) & 0x3FFFF]);
  rp->Pz = sign_x_to_s32(14, VRAM[(a + 0x1C) & 0x3FFFF]);

  rp->Cx = sign_x_to_s32(14, VRAM[(a + 0x1E) & 0x3FFFF]);
  rp->Cy = sign_x_to_s32(14, VRAM[(a + 0x1F) & 0x3FFFF]);
  rp->Cz = sign_x_to_s32(14, VRAM[(a + 0x20) & 0x3FFFF]);

  rp->Mx = sign_x_to_s32(24, BE32_VRAM((a + 0x22) & 0x3FFFF) >> 6);
  rp->My = sign_x_to_s32(24, BE32_VRAM((a + 0x24) & 0x3FFFF) >> 6);

  rp->kx = sign_x_to_s32(24, BE32_VRAM((a + 0x26) & 0x3FFFF));
  rp->ky = sign_x_to_s32(24, BE32_VRAM((a + 0x28) & 0x3FFFF));

  rp->KAst = BE32_VRAM((a + 0x2A) & 0x3FFFF) >> 6;
  rp->DKAst = sign_x_to_s32(20, BE32_VRAM((a + 0x2C) & 0x3FFFF) >> 6);
  rp->DKAx = sign_x_to_s32(20, BE32_VRAM((a + 0x2E) & 0x3FFFF) >> 6);

  a += 0x40;
  //
  // Interlace mode doesn't seem to affect operation?
  //
  // const bool imft = (InterlaceMode == IM_DOUBLE && field);

  if(RPRCTL[i] & 0x01)
   rp->XstAccum = rp->Xst; // + rp->DXst * imft;
  else
   rp->XstAccum += rp->DXst; // << (InterlaceMode == IM_DOUBLE);

  if(RPRCTL[i] & 0x02)
   rp->YstAccum = rp->Yst; // + rp->DYst * imft;
  else
   rp->YstAccum += rp->DYst; // << (InterlaceMode == IM_DOUBLE);

  if(RPRCTL[i] & 0x04)
   rp->KAstAccum = (KTAOF[i] << 26) + rp->KAst; // + rp->DKAst * imft;
  else
   rp->KAstAccum += rp->DKAst; // << (InterlaceMode == IM_DOUBLE);
 }
#undef BE32_VRAM
}

enum
{
 VPHASE_ACTIVE = 0,

 VPHASE_BOTTOM_BORDER,
 VPHASE_BOTTOM_BLANKING,

 VPHASE_VSYNC,

 VPHASE_TOP_BLANKING,
 VPHASE_TOP_BORDER,

 VPHASE__COUNT
};

static const int32_t VTimings[2][4][VPHASE__COUNT] = // End lines
{
 { // NTSC:
  { 0x0E0, 0xE8, 0xED, 0xF0, 0x0FF, 0x107 },
  { 0x0F0, 0xF0, 0xF5, 0xF8, 0x107, 0x107 },
  { 0x0E0, 0xE8, 0xED, 0xF0, 0x0FF, 0x107 },
  { 0x0F0, 0xF0, 0xF5, 0xF8, 0x107, 0x107 },
 },
 { // PAL:
  // btm brdr begin, btm blnk begin, vsync begin, /***/ top blnk begin, top brdr begin, total
  { 0x0E0, 0x100, 0x103, /***/ 0x103 + 3/*?*/, 0x119, 0x139 },
  { 0x0F0, 0x108, 0x10B, /***/ 0x10B + 3/*?*/, 0x121, 0x139 },
  { 0x100, 0x110, 0x113, /***/ 0x113 + 3/*?*/, 0x129, 0x139 },
  { 0x100, 0x110, 0x113, /***/ 0x113 + 3/*?*/, 0x129, 0x139 },
 },
};

static bool Out_VB;	// VB output signal

static uint32_t VPhase;
/*static*/ MDFN_HIDE int32_t VCounter;
static bool InternalVB;
static bool Odd;

static uint32_t CRTLineCounter;
static bool Clock28M;
//
static int SurfInterlaceField;
//
//
//

//  (No 0)  8 accesses, No split: 0 added cycles
//  (No 4)  4 accesses, No split: 1 added cycles
//  (No 6)  2 accesses, No split: 2 added cycles
//  (No 7)  1 accesses, No split: 3, Split: 3.51? added cycles
//  (No 8)  0 accesses, No split: 4, Split: 5.34? added cycles
static INLINE void RecalcVRAMPenalty(void)
{
 if(InternalVB)
  VRAMPenalty[0] = VRAMPenalty[1] = VRAMPenalty[2] = VRAMPenalty[3] = 0;
 else
 {
  const unsigned VRAM_Mode = (RAMCTL_Raw >> 8) & 0x3;
  const unsigned RDBS_Mode = (RAMCTL_Raw & 0xFF);
  const size_t sh = ((HRes & 0x6) ? 0 : 4);
  uint8_t vcp_type_penalty[0x10];

  for(unsigned vcp_type = 0; vcp_type < 0x10; vcp_type++)
  {
   if((vcp_type < 0x8) || vcp_type == 0xC || vcp_type == 0xD)
   {
    bool penalty = (bool)(BGON & (1U << (vcp_type & 0x3)));
    vcp_type_penalty[vcp_type] = penalty;
   }
   else
    vcp_type_penalty[vcp_type] = false;
  }

  for(unsigned bank = 0; bank < 4; bank++)
  {
   const unsigned esb = bank & (2 | ((VRAM_Mode >> (bank >> 1)) & 1));
   const uint8_t rdbs = (RDBS_Mode >> (esb << 1)) & 0x3;
   uint32_t tmp = 0;

   if(BGON & 0x20)
   {
    if(bank >= 2 || ((BGON & 0x10) && rdbs != RDBS_UNUSED))
     tmp = 8;
   }
   else
   {
    if((BGON & 0x10) && rdbs != RDBS_UNUSED)
     tmp = 8;
    else if(BGON & 0x0F)
    {
     tmp += vcp_type_penalty[VCPRegs[esb][0]];
     tmp += vcp_type_penalty[VCPRegs[esb][1]];
     tmp += vcp_type_penalty[VCPRegs[esb][2]];
     tmp += vcp_type_penalty[VCPRegs[esb][3]];

     tmp += vcp_type_penalty[VCPRegs[esb][sh + 0]];
     tmp += vcp_type_penalty[VCPRegs[esb][sh + 1]];
     tmp += vcp_type_penalty[VCPRegs[esb][sh + 2]];
     tmp += vcp_type_penalty[VCPRegs[esb][sh + 3]];
    }
   }

   static const uint8_t tab[9] = { 0, 0, 0, 0, 1, 1, 2, 3, 4 };
   VRAMPenalty[bank] = tab[tmp];
  }
 }
}

enum
{
 HPHASE_ACTIVE = 0,

 HPHASE_RIGHT_BORDER,
 HPHASE_HSYNC,

 // ... ? ? ?

 HPHASE__COUNT
};

static const int32_t HTimings[2][HPHASE__COUNT] =
{
 { 0x140, 0x15B, 0x1AB },
 { 0x160, 0x177, 0x1C7 },
};

static uint32_t HPhase;
/*static*/ MDFN_HIDE int32_t HCounter;

static uint16_t Latched_VCNT, Latched_HCNT;
static bool HVIsExLatched;
bool ExLatchIn;
bool ExLatchPending;

static INLINE unsigned GetNLVCounter(void)
{
 unsigned ret;

 if(VPhase >= VPHASE_VSYNC)
  ret = VCounter + (0x200 - VTimings[PAL][VRes][VPHASE__COUNT - 1]);
 else
  ret = VCounter;

 if(InterlaceMode == IM_DOUBLE)
  ret = (ret << 1) | !Odd;

 return ret;
}

static void LatchHV(void)
{
 Latched_VCNT = GetNLVCounter();

 if(HPhase >= HPHASE_HSYNC)
  Latched_HCNT = (HCounter + (0x200 - HTimings[HRes & 1][HPHASE__COUNT - 1])) << 1;
 else
  Latched_HCNT = HCounter << 1;
}

//
//
void VDP2_GetGunXTranslation(const bool clock28m, float* scale, float* offs)
{
 VDP2REND_GetGunXTranslation(clock28m, scale, offs);
}

void VDP2_StartFrame(struct EmulateSpecStruct* espec, const bool clock28m)
{
 Clock28M = clock28m;
 VDP2REND_StartFrame(espec, clock28m, SurfInterlaceField);
 CRTLineCounter = 0;
}

//
//
static INLINE void IncVCounter(const sscpu_timestamp_t event_timestamp)
{
 const unsigned prev_nlvc = GetNLVCounter();
 //
 VCounter = (VCounter + 1) & 0x1FF;

 if(VCounter == (VTimings[PAL][VRes][VPHASE__COUNT - 1] - 1))
 {
  Out_VB = false;
  Window[0].YEndMet = Window[1].YEndMet = false;
 }

 if(MDFN_UNLIKELY(ss_horrible_hacks & HORRIBLEHACK_NOSH2DMALINE106))
 {
  const bool s = (VCounter == (VTimings[PAL][VRes][VPHASE__COUNT - 1] - 1));

  for(size_t i = 0; i < 2; i++)
   SH7095_SetExtHaltDMAKludge(i, s);
 }

 // - 1, so the CPU loop will  have plenty of time to exit before we reach non-hblank top border area
 // (exit granularity could be large if program is executing from SCSP RAM space, for example).
 if(VCounter == (VTimings[PAL][VRes][VPHASE_TOP_BLANKING] - 1))
 {

  SS_RequestMLExit();
  VDP2REND_EndFrame();
 }

 while(VCounter >= VTimings[PAL][VRes][VPhase] - ((VPhase == VPHASE_VSYNC - 1) && InterlaceMode))
 {
  VPhase++;

  if(VPhase == VPHASE__COUNT)
  {
   VPhase = 0;
   VCounter -= VTimings[PAL][VRes][VPHASE__COUNT - 1];
  }

  if(VPhase == VPHASE_ACTIVE)
   InternalVB = !DisplayOn;
  else if(VPhase == VPHASE_BOTTOM_BORDER)
  {
   SS_SetEventNT(&events[SS_EVENT_MIDSYNC], event_timestamp + 1);
   InternalVB = true;
   Out_VB = true;
  }
  else if(VPhase == VPHASE_BOTTOM_BLANKING)
  {
   CRTLineCounter = 0x80000000U;
  }
  else if(VPhase == VPHASE_VSYNC)
  {
   if(InterlaceMode)
   {
    Odd = !Odd;
    VCounter += Odd;
    SurfInterlaceField = !Odd;
   }
   else
   {
    SurfInterlaceField = -1;
    Odd = true;
   }
  }
 }

 //
 //
 {
  const unsigned nlvc = GetNLVCounter();
  const unsigned mask = (InterlaceMode == IM_DOUBLE) ? 0x1FE : 0x1FF;

  for(unsigned d = 0; d < 2; d++)
  {
   if((nlvc & mask) == (Window[d].YStart & mask))
    Window[d].YIn = true;

   if((prev_nlvc & mask) == (Window[d].YEnd & mask))
    Window[d].YEndMet = true;

   Window[d].YIn &= !Window[d].YEndMet;
  }
 }
 //
 //

 RecalcVRAMPenalty();

 SMPC_SetVBVS(event_timestamp, Out_VB, VPhase == VPHASE_VSYNC);
}

static INLINE int32_t AddHCounter(const sscpu_timestamp_t event_timestamp, int32_t count)
{
 HCounter += count;

 while(HCounter >= HTimings[HRes & 1][HPhase])
 {
  HPhase++;

  if(HPhase == HPHASE__COUNT)
  {
   HPhase = 0;
   HCounter -= HTimings[HRes & 1][HPHASE__COUNT - 1];
  }
  //
  //
  //
  if(HPhase == HPHASE_ACTIVE)
  {
   {
    const int32_t div = Clock28M ? 61 : 65;
    const int32_t coord_adj = 6832 - 80 * div;

    SMPC_LineHook(event_timestamp, CRTLineCounter, div, coord_adj);
   }

   if(VPhase == VPHASE_ACTIVE)
   {
    struct VDP2Rend_LIB* lib = VDP2REND_GetLIB(VCounter);

    lib->win_ymet[0] = Window[0].YIn;
    lib->win_ymet[1] = Window[1].YIn;

    if(!InternalVB)
    {
     // The rotation-parameter fetch AND the per-line matrix-multiply
     // that populates lib->rv[0..1] are both only useful when the
     // VDP2 is actually rendering an RBG (rotating background) layer.
     // SetupRotVars in vdp2_render.c's consumer DrawLine is the
     // sole reader of LIB[].rv, and it's gated on the same
     // BGON & 0x30 check. Because BGON updates are synchronous from
     // the SH-2 side (RegsWrite at register 0x20 writes BGON
     // directly, line 639 of this file) and AddHCounter doesn't
     // interleave with SH-2 execution, the producer's BGON here is
     // identical to what the consumer will see for this same
     // scanline (the COMMAND_WRITE16 carrying any BGON change is
     // ordered behind COMMAND_DRAW_LINE in the queue only if it
     // arrived after, otherwise it's already been applied). So
     // skipping the rv compute when BGON & 0x30 == 0 is observably
     // identical to running it -- LIB[].rv just goes unread.
     //
     // For non-RBG games (the vast majority of 2D titles) this
     // saves ~30 multiplications + 2 int64_t shifts per scanline x
     // 240 lines x 60 fps ~= 0.5 M ops/s of producer-thread work
     // that was producing dead data.
     if(BGON & 0x30)
     {
      if(VCounter == 0)
       RPRCTL[0] = RPRCTL[1] = 0x07;
      FetchRotParams(false/*field*/);
      RPRCTL[0] = RPRCTL[1] = 0;

      for(unsigned i = 0; i < 2; i++)
      {
       const __typeof__(RotParams[i]) *rp = &RotParams[i];
       __typeof__(lib->rv[i]) *r = &lib->rv[i];

       r->Xsp = ((int64_t)rp->RotMatrix[0] * ((int32_t)rp->XstAccum - (rp->Px * 1024)) +
	       (int64_t)rp->RotMatrix[1] * ((int32_t)rp->YstAccum - (rp->Py * 1024)) +
	       (int64_t)rp->RotMatrix[2] * (rp->Zst      - (rp->Pz * 1024))) >> 10;
       r->Ysp = ((int64_t)rp->RotMatrix[3] * ((int32_t)rp->XstAccum - (rp->Px * 1024)) +
	       (int64_t)rp->RotMatrix[4] * ((int32_t)rp->YstAccum - (rp->Py * 1024)) +
	       (int64_t)rp->RotMatrix[5] * (rp->Zst      - (rp->Pz * 1024))) >> 10;

       r->Xp = rp->RotMatrix[0] * (rp->Px - rp->Cx) +
	     rp->RotMatrix[1] * (rp->Py - rp->Cy) +
	     rp->RotMatrix[2] * (rp->Pz - rp->Cz) +
	     (rp->Cx * 1024) + rp->Mx;

       r->Yp = rp->RotMatrix[3] * (rp->Px - rp->Cx) +
	     rp->RotMatrix[4] * (rp->Py - rp->Cy) +
	     rp->RotMatrix[5] * (rp->Pz - rp->Cz) +
	     (rp->Cy * 1024) + rp->My;

       r->dX = (rp->RotMatrix[0] * rp->DX + rp->RotMatrix[1] * rp->DY) >> 10;
       r->dY = (rp->RotMatrix[3] * rp->DX + rp->RotMatrix[4] * rp->DY) >> 10;

       r->kx = rp->kx;
       r->ky = rp->ky;

       r->KAstAccum = rp->KAstAccum;
       r->DKAx = rp->DKAx;
      }
     }
    }
    lib->vdp1_hires8 = VDP1_GetLine(VCounter, lib->vdp1_line, lib->vdp1_mesh_line, (HRes & 1) ? 352 : 320, (int32_t)RotParams[0].XstAccum >> 1, (int32_t)RotParams[0].YstAccum >> 1, (int32_t)RotParams[0].DX >> 1, (int32_t)RotParams[0].DY >> 1); // Always call, has side effects.
    VDP2REND_DrawLine(InternalVB ? -1 : VCounter, CRTLineCounter, !Odd);
    CRTLineCounter++;
   }
   else if(VPhase == VPHASE_TOP_BORDER || VPhase == VPHASE_BOTTOM_BORDER)
   {
    VDP2REND_DrawLine(-1, CRTLineCounter, !Odd);
    CRTLineCounter++;
   }
  }
  else if(HPhase == HPHASE_HSYNC)
  {
   IncVCounter(event_timestamp);
  }
 }

 return (HTimings[HRes & 1][HPhase] - HCounter);
}

sscpu_timestamp_t VDP2_Update(sscpu_timestamp_t timestamp)
{
 int32_t clocks;

 if(MDFN_UNLIKELY(timestamp < lastts))
  clocks = 0;
 else
  clocks = (timestamp - lastts) >> 2;

 lastts += clocks << 2;
 //
 //
 int32_t tmp;
 int32_t ne = AddHCounter(timestamp, clocks);
 VDP1_SetHBVB(timestamp, HPhase > HPHASE_ACTIVE, Out_VB);
 tmp = SCU_SetHBVB(clocks, HPhase > HPHASE_ACTIVE, Out_VB);
 if(tmp < ne)
  ne = tmp;

 //
 //
 //
 if(MDFN_UNLIKELY(ExLatchPending))
 {
  LatchHV();
  HVIsExLatched = true;
  ExLatchPending = false;
 }

 return lastts + (ne << 2);
}

//
// Register writes seem to always be 16-bit
//
static INLINE void RegsWrite(uint32_t A, uint16_t V)
{
 A &= 0x1FE;

 RawRegs[A >> 1] = V;

 switch(A)
 {
  default:
	break;

  case 0x00:
	VDP2_Update(SH7095_mem_timestamp);
	//
	DisplayOn = (V >> 15) & 0x1;
	BorderMode = (V >> 8) & 0x1;
	InterlaceMode = (V >> 6) & 0x3;
	VRes = (V >> 4) & 0x3;
	HRes = (V >> 0) & 0x7;
	//
	InternalVB |= !DisplayOn;
	//
	SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	break;

  case 0x02:
	ExLatchEnable = (V >> 9) & 0x1;
	ExSyncEnable = (V >> 8) & 0x1;

	DispAreaSelect = (V >> 1) & 0x1;
	ExBGEnable = (V >> 0) & 0x1;
	break;

  case 0x06:
	VRAMSize = (V >> 15) & 0x1;

	break;

  case 0x0E:
	RAMCTL_Raw = V & 0xB3FF;
	CRAM_Mode = (V >> 12) & 0x3;
	break;

  case 0x10:
  case 0x12:
  case 0x14:
  case 0x16:
  case 0x18:
  case 0x1A:
  case 0x1C:
  case 0x1E:
	{
	 uint8_t* const b = &VCPRegs[(A >> 2) & 3][(A & 0x2) << 1];
	 b[0] = (V >> 12) & 0xF;
	 b[1] = (V >>  8) & 0xF;
	 b[2] = (V >>  4) & 0xF;
	 b[3] = (V >>  0) & 0xF;
	}
	break;

  case 0x20:
	BGON = V & 0x1F3F;
	break;

  case 0xB2:
	RPRCTL[0] = (V >> 0) & 0x7;
	RPRCTL[1] = (V >> 8) & 0x7;
	break;

  case 0xB6:
	KTAOF[0] = (V >> 0) & 0x7;
	KTAOF[1] = (V >> 8) & 0x7;
	break;

  case 0xBC:
	RPTA = (RPTA & 0xFFFF) | ((V & 0x7) << 16);
	break;

  case 0xBE:
	RPTA = (RPTA & ~0xFFFF) | (V & 0xFFFE);
	break;

  case 0xC2: Window[0].YStart = V & 0x1FF; break;
  case 0xC6: Window[0].YEnd = V & 0x1FF; break;

  case 0xCA: Window[1].YStart = V & 0x1FF; break;
  case 0xCE: Window[1].YEnd = V & 0x1FF; break;
 }
}

static INLINE uint16_t RegsRead(uint32_t A)
{
 switch(A & 0x1FE)
 {
  case 0x00:
	return (DisplayOn << 15) | (BorderMode << 8) | (InterlaceMode << 6) | (VRes << 4) | (HRes << 0);
  case 0x02:
	if(!ExLatchEnable)
	{
	 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
 	 LatchHV();
	}
	return (ExLatchEnable << 9) | (ExSyncEnable << 8) | (DispAreaSelect << 1) | (ExBGEnable << 0);
  case 0x04:
	SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
	{
	 // TODO: EXSYFG
	 uint16_t ret = (HVIsExLatched << 9) | (InternalVB << 3) | ((HPhase > HPHASE_ACTIVE) << 2) | (Odd << 1) | (PAL << 0);

	 HVIsExLatched = false;

	 return ret;
	}

  case 0x06:
	return VRAMSize << 15;
  case 0x08:
	return Latched_HCNT;
  case 0x0A:
	return Latched_VCNT;
  case 0x0E:
	return RAMCTL_Raw;
  default:
        break;
 }
 return 0;
}

/* Was `template<typename T, bool IsWrite> static INLINE uint32_t
 * RW(uint32_t A, uint16_t* DB)` with a static_assert blocking
 * `RW<uint8_t, false>` (8-bit reads forbidden -- SH7095 always
 * reads VDP2 as 16-bit).  Three instantiations were used in
 * practice: RW<uint16_t, false>, RW<uint8_t, true>, RW<uint16_t,
 * true>.  Monomorphized into three plain INLINE functions; the
 * branch-on-IsWrite is gone (function pick is at the callsite)
 * and the `sizeof(T) == 2` mask check is folded to constants. */

/* Read 16-bit (only valid read mode). */
static INLINE uint32_t RW_R16(uint32_t A, uint16_t* DB)
{
 A &= 0x1FFFFF;

 //
 // VRAM
 //
 if(A < 0x100000)
 {
  const size_t vri = (A & 0x7FFFF) >> 1;

  *DB = VRAM[vri];

  return VRAMPenalty[vri >> 16];
 }

 //
 // CRAM
 //
 if(A < 0x180000)
 {
  const unsigned cri = (A & 0xFFF) >> 1;

  switch(CRAM_Mode)
  {
   case CRAM_MODE_RGB555_1024:
   case CRAM_MODE_RGB555_2048:
	*DB = CRAM[cri];
	break;

   case CRAM_MODE_RGB888_1024:
   case CRAM_MODE_ILLEGAL:
   default:
	*DB = CRAM[((cri >> 1) & 0x3FF) | ((cri & 1) << 10)];
	break;
  }

  return 0;
 }

 //
 // Registers
 //
 if(A < 0x1C0000)
 {
  *DB = RegsRead(A);

  return 0;
 }

 *DB = 0;

 return 0;
}

/* Write 8-bit (mask = 0xFF00 >> ((A & 1) << 3)).  Only the VRAM
 * write path uses the mask; CRAM and register writes are 16-bit
 * regardless. */
static INLINE uint32_t RW_W8(uint32_t A, uint16_t* DB)
{
 A &= 0x1FFFFF;

 //
 // VRAM
 //
 if(A < 0x100000)
 {
  const size_t vri = (A & 0x7FFFF) >> 1;
  const unsigned mask = 0xFF00 >> ((A & 1) << 3);

  VRAM[vri] = (VRAM[vri] &~ mask) | (*DB & mask);

  return VRAMPenalty[vri >> 16];
 }

 //
 // CRAM
 //
 if(A < 0x180000)
 {
  const unsigned cri = (A & 0xFFF) >> 1;

  switch(CRAM_Mode)
  {
   case CRAM_MODE_RGB555_1024:
	(CRAM + 0x000)[cri & 0x3FF] = *DB;
	(CRAM + 0x400)[cri & 0x3FF] = *DB;
	break;

   case CRAM_MODE_RGB555_2048:
	CRAM[cri] = *DB;
	break;

   case CRAM_MODE_RGB888_1024:
   case CRAM_MODE_ILLEGAL:
   default:
	CRAM[((cri >> 1) & 0x3FF) | ((cri & 1) << 10)] = *DB;
	break;
  }

  return 0;
 }

 //
 // Registers
 //
 if(A < 0x1C0000)
 {
  RegsWrite(A, *DB);

  return 0;
 }

 return 0;
}

/* Write 16-bit (mask = 0xFFFF, which folds the VRAM masked write
 * to a plain assignment). */
static INLINE uint32_t RW_W16(uint32_t A, uint16_t* DB)
{
 A &= 0x1FFFFF;

 //
 // VRAM
 //
 if(A < 0x100000)
 {
  const size_t vri = (A & 0x7FFFF) >> 1;

  VRAM[vri] = *DB;

  return VRAMPenalty[vri >> 16];
 }

 //
 // CRAM
 //
 if(A < 0x180000)
 {
  const unsigned cri = (A & 0xFFF) >> 1;

  switch(CRAM_Mode)
  {
   case CRAM_MODE_RGB555_1024:
	(CRAM + 0x000)[cri & 0x3FF] = *DB;
	(CRAM + 0x400)[cri & 0x3FF] = *DB;
	break;

   case CRAM_MODE_RGB555_2048:
	CRAM[cri] = *DB;
	break;

   case CRAM_MODE_RGB888_1024:
   case CRAM_MODE_ILLEGAL:
   default:
	CRAM[((cri >> 1) & 0x3FF) | ((cri & 1) << 10)] = *DB;
	break;
  }

  return 0;
 }

 //
 // Registers
 //
 if(A < 0x1C0000)
 {
  RegsWrite(A, *DB);

  return 0;
 }

 return 0;
}

uint16_t VDP2_Read16_DB(uint32_t A)
{
 uint16_t DB;

 RW_R16(A, &DB);

 return DB;
}

uint32_t VDP2_Write8_DB(uint32_t A, uint16_t DB)
{
 VDP2REND_Write8_DB(A, DB);

 return RW_W8(A, &DB);
}

uint32_t VDP2_Write16_DB(uint32_t A, uint16_t DB)
{
 VDP2REND_Write16_DB(A, DB);

 return RW_W16(A, &DB);
}

uint32_t VDP2_Write16Burst_DB(uint32_t base, uint32_t n16, uint32_t add_mode, const uint16_t* words)
{
 VDP2REND_WriteBurst16_DB(base, n16, add_mode, words);

 const uint32_t stride = (1u << add_mode) &~ 1u;
 uint32_t a = base;
 uint32_t penalty_sum = 0;

 for(uint32_t i = 0; i < n16; i++)
 {
  uint16_t w = words[i];
  penalty_sum += RW_W16(a, &w);
  a += stride;
 }

 return penalty_sum;
}

//
//
//

void VDP2_AdjustTS(const int32_t delta)
{
 lastts += delta;
}

void VDP2_Init(const bool IsPAL, const uint64_t affinity)
{
 SurfInterlaceField = -1;
 PAL = IsPAL;
 lastts = 0;
 CRTLineCounter = 0x80000000U;
 Clock28M = false;

 SS_SetPhysMemMap(0x05E00000, 0x05EFFFFF, VRAM, 0x80000, true);

 ExLatchIn = false;

 VDP2REND_Init(IsPAL, affinity);
}

void VDP2_SetGetVideoParams(struct MDFNGI* gi, const bool caspect, const int sls, const int sle, const bool show_h_overscan, const bool dohblend)
{
 VDP2REND_SetGetVideoParams(gi, caspect, sls, sle, show_h_overscan, dohblend);
}

void VDP2_Kill(void)
{
 VDP2REND_Kill();
}

//
// TODO: Check reset versus power on values.
//
void VDP2_Reset(bool powering_up)
{
 memset(RawRegs, 0, sizeof(RawRegs));

 DisplayOn = false;
 BorderMode = false;
 ExLatchEnable = false;
 ExSyncEnable = false;
 ExBGEnable = false;
 DispAreaSelect = false;
 HRes = 0;
 VRes = 0;
 InterlaceMode = 0;

 VRAMSize = 0;

 InternalVB = true;
 Out_VB = false;
 VPhase = VPHASE_ACTIVE;
 VCounter = 0;
 Odd = true;

 for(size_t i = 0; i < 2; i++)
  SH7095_SetExtHaltDMAKludge(i, false);

 RAMCTL_Raw = 0;
 CRAM_Mode = 0;

 BGON = 0;

 memset(VCPRegs, 0, sizeof(VCPRegs));

 for(unsigned i = 0; i < 2; i++)
 {
  KTAOF[i] = 0;
  RPRCTL[i] = 0;
 }
 RPTA = 0;
 memset(RotParams, 0, sizeof(RotParams));

 for(unsigned w = 0; w < 2; w++)
 {
  Window[w].YStart = 0;
  Window[w].YEnd = 0;
  Window[w].YEndMet = false;
  Window[w].YIn = false;
 }
 //
 //
 //
 if(powering_up)
 {
  HPhase = 0;
  HCounter = 0;

  Latched_VCNT = 0;
  Latched_HCNT = 0;
  HVIsExLatched = false;
  ExLatchPending = false;
 }
 //
 // FIXME(init values), also in VDP2REND.
 if(powering_up)
 {
  memset(VRAM, 0, sizeof(VRAM));
  memset(CRAM, 0, sizeof(CRAM));
 }
 //
 RecalcVRAMPenalty();
 //
 //
 VDP2REND_Reset(powering_up);
}

//
//
//
//

void VDP2_SetDeinterlaceOff(bool off)
{
 VDP2REND_SetDeinterlaceOff(off);
}

void VDP2_StateAction(StateMem* sm, const unsigned load, const bool data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(lastts),

  SFPTR16N(&(RawRegs)[0], (sizeof(RawRegs) / sizeof(uint16_t)), "RawRegs"),
  SFVAR(DisplayOn),
  SFVAR(BorderMode),
  SFVAR(ExLatchEnable),
  SFVAR(ExSyncEnable),
  SFVAR(ExBGEnable),
  SFVAR(DispAreaSelect),

  SFVAR(VRAMSize),

  SFVAR(HRes),
  SFVAR(VRes),
  SFVAR(InterlaceMode),

  SFVAR(RAMCTL_Raw),
  SFVAR(CRAM_Mode),

  SFVAR(BGON),
  SFPTR8N(&(VCPRegs)[0][0], (sizeof(VCPRegs) / sizeof(uint8_t)), "&VCPRegs[0][0]"),
  SFPTR32N(&(VRAMPenalty)[0], (sizeof(VRAMPenalty) / sizeof(uint32_t)), "VRAMPenalty"),

  SFVAR(RPTA),
  SFPTR8N(&(RPRCTL)[0], (sizeof(RPRCTL) / sizeof(uint8_t)), "RPRCTL"),
  SFPTR8N(&(KTAOF)[0], (sizeof(KTAOF) / sizeof(uint8_t)), "KTAOF"),

  SFPTR16N(&(VRAM)[0], (sizeof(VRAM) / sizeof(uint16_t)), "VRAM"),
  SFPTR16N(&(CRAM)[0], (sizeof(CRAM) / sizeof(uint16_t)), "CRAM"),

  SFVAR(RotParams->Xst, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->Yst, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->Zst, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->DXst, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->DYst, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->DX, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->DY, 2, sizeof(*RotParams), RotParams),
  SFPTR32N(&(RotParams->RotMatrix)[0], (sizeof(RotParams->RotMatrix) / sizeof(int32_t)), 2, sizeof(*RotParams), RotParams, "RotParams->RotMatrix"),
  SFVAR(RotParams->Px, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->Py, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->Pz, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->Cx, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->Cy, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->Cz, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->Mx, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->My, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->kx, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->ky, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->KAst, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->DKAst, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->DKAx, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->XstAccum, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->YstAccum, 2, sizeof(*RotParams), RotParams),
  SFVAR(RotParams->KAstAccum, 2, sizeof(*RotParams), RotParams),

  SFVAR(Out_VB),

  SFVAR(VPhase),
  SFVAR(VCounter),
  SFVAR(InternalVB),
  SFVAR(Odd),

  SFVAR(CRTLineCounter),
  SFVAR(Clock28M),
//
  SFVAR(SurfInterlaceField),

  SFVAR(HPhase),
  SFVAR(HCounter),

  SFVAR(Latched_VCNT),
  SFVAR(Latched_HCNT),
  SFVAR(HVIsExLatched),
  SFVAR(ExLatchIn),
  SFVAR(ExLatchPending),

  SFVAR(Window->YStart, 2, sizeof(*Window), Window),
  SFVAR(Window->YEnd, 2, sizeof(*Window), Window),
  SFVAR(Window->YEndMet, 2, sizeof(*Window), Window),
  SFVAR(Window->YIn, 2, sizeof(*Window), Window),

  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, "VDP2", false);

 if(load)
 {
  if(load < 0x00102100)
  {
   Window[0].YStart = RawRegs[0xC2 >> 1] & 0x1FF;
   Window[0].YEnd = RawRegs[0xC6 >> 1] & 0x1FF;

   Window[1].YStart = RawRegs[0xCA >> 1] & 0x1FF;
   Window[1].YEnd = RawRegs[0xCE >> 1] & 0x1FF;

   for(unsigned d = 0; d < 2; d++)
   {
    Window[d].YEndMet = false;
    Window[d].YIn = false;
   }
  }
  //
  //
  InterlaceMode &= 0x3;
  VRes &= 0x3;
  HRes &= 0x7;

  CRAM_Mode &= 0x3;
  InterlaceMode &= 0x3;

  HCounter &= 0x1FF;
  VCounter &= 0x1FF;

  HPhase %= HPHASE__COUNT;
  VPhase %= VPHASE__COUNT;

  //
  // Register-value masks that RegsWrite applies on bus writes but
  // that StateAction had not been applying on state load. Each is a
  // 4-bit (or 3-bit) bitfield packed into a 16-bit register; the
  // runtime path masks each byte before storing it, but the load
  // path just took whatever was in the save file.
  //
  // VCPRegs is the load-bearing one. Inside RecalcVRAMPenalty
  // (line ~272) values feed an index into a local 16-entry
  // vcp_type_penalty[] array:
  //
  //   tmp += vcp_type_penalty[VCPRegs[esb][i]];   x8
  //   VRAMPenalty[bank] = tab[tmp];               (tab is 9 bytes)
  //
  // A crafted state with VCPRegs[i][j] > 0xF produces an OOB read
  // of adjacent stack memory; the byte value is then accumulated
  // into tmp eight times, which can push tmp far past 8 and turn
  // the final tab[tmp] read into a second OOB into the rodata
  // segment. The output is consumed as a memory-access penalty, so
  // the symptom is corrupt VRAM timing rather than memory
  // corruption, but the read primitive is real. Match the
  // RegsWrite mask (4 bits per nibble).
  //
  // RPRCTL is bit-tested only (& 0x01, 0x02, 0x04) -- functionally
  // safe with any value -- but we mask for consistency with
  // RegsWrite (3 bits).
  //
  // KTAOF feeds `KTAOF[i] << 26` (line ~174). KTAOF is uint8_t, so
  // the operand is promoted to signed int; values >= 0x20 then
  // produce a left shift whose result doesn't fit in int, which is
  // UB in C++. The result becomes part of KAstAccum so the symptom
  // is mostly "garbage rotation parameters", but cleaner to clamp.
  for(unsigned r = 0; r < 4; r++)
   for(unsigned c = 0; c < 8; c++)
    VCPRegs[r][c] &= 0xF;

  for(unsigned i = 0; i < 2; i++)
  {
   RPRCTL[i] &= 0x7;
   KTAOF[i]  &= 0x7;
  }
 }

 VDP2REND_StateAction(sm, load, data_only, RawRegs, CRAM, VRAM);
}

/* The trailing extern "C" proxy block (5 forwarders into the
 * namespace) was removed when the `namespace VDP2 { ... }` wrap
 * was lifted -- the renamed functions ARE C-linkage natively now,
 * so the proxies are redundant.
 *
 * The one INLINE entry point (VDP2_SetExtLatch) moved entirely
 * into vdp2.h's C-compat block, which means smpc.c now gets it
 * inlined directly from the header at zero call cost -- an
 * improvement over the previous proxy form.  See vdp2.h. */
