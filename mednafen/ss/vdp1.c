/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* vdp1.c - VDP1 Emulation
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

// TODO: Draw timing for small lines(somewhere between 2 and 15 pixels wide) on the VDP1 seems weird; investigate further
// before making timing changes to the drawing code.

// TODO: Investigate and more accurately model the 10-20% draw overhead currently approximated in AdjustDrawTiming().

// TODO: Draw timing for shrunken(even just slightly) sprite lines with HSS disabled should be 100% higher.

// TODO: 32-bit writes from the SH-2 CPUs to VDP1 registers seem to be broken on a Saturn; test, and implement here.

// TODO: Check to see what registers are reset on reset.

// TODO: SS_SetPhysMemMap(0x05C80000, 0x05CFFFFF, VDP1_FB[FBDrawWhich], sizeof(VDP1_FB[0]));
//  (...but goes weird in 8bpp rotated mode...)

// TODO: Test 1x1 line, polyline, sprite, and polygon.

// TODO: Framebuffer swap/auto drawing start happens a bit too early, should happen near
//       end of hblank instead of the beginning.

#include <string.h>
#include "vdp1.h"
#include "vdp1_common.h"

/* ss.h and scu.h became C-includable in ba93ea6 (the SS-core
   omnibus), so include them directly instead of localising the
   handful of cross-TU symbols inline.  Six previously-inlined
   externs (ss_horrible_hacks, event_list_entry events[],
   SS_SetEventNT, SS_SetPhysMemMap, SH7095_mem_timestamp, SCU_SetInt,
   SCU_CheckVDP1HaltKludge) now come from the real headers, which
   removes a per-TU ledger to keep in sync. */
#include "ss.h"
#include "scu.h"

/* SS_EVENT_*, SCU_INT_*, HORRIBLEHACK_* and event_list_entry come
   from the shared C/C++ leaf header -- the single source of truth.
   They were previously re-typed by hand here and miscounted
   (SS_EVENT_VDP1/VDP2 and SCU_INT_VDP1 were all wrong), which
   broke VDP1 event scheduling.  Never transcribe these again --
   add to ss_c_abi.h instead.  ss.h pulls this in transitively but
   vdp1.c uses these symbols directly so the explicit include
   reflects actual usage and stays stable across upstream
   reorganisations. */
#include "ss_c_abi.h"

/* vdp2.h is still no longer used (namespace VDP2 wrap), so the one
   remaining VDP2 entry point that vdp1.c needs gets a localised
   forward decl until vdp2.h is itself made C-includable.
   vdp2.c:1474 has the matching `extern "C" VDP2_Update` proxy
   that forwards into VDP2::Update. */
extern int32_t VDP2_Update(int32_t timestamp);

/* sign_x_to_s32(n_bits, value) is provided as a macro in math_ops.h,
   which now reaches vdp1.c transitively via ss.h.  The previous
   local static inline function was a verbatim duplicate kept here
   only because ss.h (and therefore math_ops.h) was no longer used at the
   original conversion time. */

enum { VDP1_UpdateTimingGran = 263 };
enum { VDP1_IdleTimingGran = 1019 };

/* namespace VDP1 removed */

uint8_t VDP1_spr_w_shift_tab[8];
uint8_t VDP1_gouraud_lut[0x40];
line_data VDP1_LineData;
line_inner_data VDP1_LineInnerData;
prim_data VDP1_PrimData;

int32_t VDP1_SysClipX, VDP1_SysClipY;
int32_t VDP1_UserClipX0, VDP1_UserClipY0, VDP1_UserClipX1, VDP1_UserClipY1;

int32_t VDP1_LocalX, VDP1_LocalY;

uint8_t VDP1_TVMR;
uint8_t VDP1_FBCR;
static uint8_t PTMR;
static uint8_t EDSR;

uint16_t* VDP1_FBDrawWhichPtr;
static bool FBDrawWhich;

static bool DrawingActive;
static uint32_t CurCommandAddr;
static int32_t RetCommandAddr;
static uint16_t LOPR;

static sscpu_timestamp_t lastts;
static int32_t CycleCounter;
static int32_t CommandPhase;
static uint16_t CommandData[0x10];
uint32_t VDP1_DTACounter;

static bool vb_status, hb_status;
static bool vbcdpending;
static bool FBManualPending;
static bool FBVBErasePending;
static bool FBVBEraseActive;
static sscpu_timestamp_t FBVBEraseLastTS;
static sscpu_timestamp_t LastRWTS;

static uint16_t EWDR;	// Erase/Write Data
static uint16_t EWLR;	// Erase/Write Upper Left Coordinate
static uint16_t EWRR;	// Erase/Write Lower Right Coordinate

static struct
{
 bool rot8;
 uint32_t fb_x_mask;

 uint32_t y_start;
 uint32_t x_start;

 uint32_t y_end;
 uint32_t x_bound;

 uint16_t fill_data;
} EraseParams;

static uint32_t EraseYCounter;

static uint32_t InstantDrawSanityLimit; // ss_horrible_hacks

uint16_t VDP1_VRAM[0x40000];
uint16_t VDP1_FB[2][0x20000];

// Side-buffer for "improved mesh transparency" mode. When VDP1_MeshImproved
// is true, VDP1 mesh-bit primitives write their colour here (with MSB
// set as the "mesh pixel present" marker) instead of the main FB.
// Non-mesh primitives in improved mode clear VDP1_MeshFB at their pixel
// position so opaque writes properly cover earlier mesh content.
// VDP2's MixIt path reads this via VDP1_GetLine and 50%-blends the
// mesh pixel over the final composited surface, matching Kronos's
// "outMeshSurface" side-buffer + late composite mechanism.
//
// Double-buffered in lockstep with FB; VDP1_MeshFBDrawWhichPtr tracks
// VDP1_MeshFB[FBDrawWhich]. Erased in the same loops that erase FB so the
// per-frame clear matches the game's intent for the main framebuffer.
uint16_t VDP1_MeshFB[2][0x20000];
uint16_t* VDP1_MeshFBDrawWhichPtr;

// Module-level toggle for the "improved mesh transparency" mode.
// Read by PlotPixel in vdp1_common.h when its MeshEn template
// arg is true. Default false = hardware-accurate stipple.
bool VDP1_MeshImproved = false;

void VDP1_SetMeshImproved(bool improved)
{
 VDP1_MeshImproved = improved;
}

void VDP1_Init(void)
{
 vbcdpending = false;

 for(int i = 0; i < 0x40; i++)
 {
  VDP1_gouraud_lut[i] = ((int)(31) < (int)(((int)(0) > (int)(i - 16) ? (int)(0) : (int)(i - 16))) ? (int)(31) : (int)(((int)(0) > (int)(i - 16) ? (int)(0) : (int)(i - 16))));
 }

 for(int i = 0; i < 8; i++)
 {
  VDP1_spr_w_shift_tab[i] = (7 - i) / 3;
 }

 //
 //
 SS_SetPhysMemMap(0x05C00000, 0x05C7FFFF, VDP1_VRAM, sizeof(VDP1_VRAM), true);

 vb_status = false;
 hb_status = false;
 lastts = 0;
 FBVBEraseLastTS = 0;
 LastRWTS = 0;
}

void VDP1_Kill(void)
{

}

void VDP1_Reset(bool powering_up)
{
 if(powering_up)
 {
  for(unsigned i = 0; i < 0x40000; i++)
  {
   uint16_t val;

   if((i & 0xF) == 0)
    val = 0x8000;
   else if(i & 0x1)
    val = 0x5555;
   else
    val = 0xAAAA;

   VDP1_VRAM[i] = val;
  }

  for(unsigned fb = 0; fb < 2; fb++)
   for(unsigned i = 0; i < 0x20000; i++)
    VDP1_FB[fb][i] = 0xFFFF;

  memset(&VDP1_LineData, 0, sizeof(VDP1_LineData));
  memset(&VDP1_LineInnerData, 0, sizeof(VDP1_LineInnerData));
  memset(&VDP1_PrimData, 0, sizeof(VDP1_PrimData));

  //
  // Registers with somewhat undefined state on power-on:
  //
  EWDR = 0;
  EWLR = 0;
  EWRR = 0;

  VDP1_UserClipX0 = 0;
  VDP1_UserClipY0 = 0;
  VDP1_UserClipX1 = 0;
  VDP1_UserClipY1 = 0;

  VDP1_SysClipX = 0;
  VDP1_SysClipY = 0;

  VDP1_LocalX = 0;
  VDP1_LocalY = 0;
 }

 FBDrawWhich = 0;
 VDP1_FBDrawWhichPtr = VDP1_FB[FBDrawWhich];
 VDP1_MeshFBDrawWhichPtr = VDP1_MeshFB[FBDrawWhich];
 memset(VDP1_MeshFB, 0, sizeof(VDP1_MeshFB));
 //SS_SetPhysMemMap(0x05C80000, 0x05CFFFFF, VDP1_FB[FBDrawWhich], sizeof(VDP1_FB[0]), true);

 FBManualPending = false;
 FBVBErasePending = false;
 FBVBEraseActive = false;

 LOPR = 0;
 CurCommandAddr = 0;
 RetCommandAddr = -1;
 DrawingActive = false;
 CycleCounter = 0;
 CommandPhase = 0;
 memset(CommandData, 0, sizeof(CommandData));
 InstantDrawSanityLimit = 0;
 VDP1_DTACounter = 0;

 //
 // Begin registers/variables confirmed to be initialized on reset.
 VDP1_TVMR = 0;
 VDP1_FBCR = 0;
 PTMR = 0;
 EDSR = 0;
 // End confirmed.
 //

 memset(&EraseParams, 0, sizeof(EraseParams));
 EraseYCounter = ~0U;
}

static int32_t CMD_SetUserClip(const uint16_t* cmd_data)
{
 VDP1_UserClipX0 = cmd_data[0x6] & 0x1FFF;
 VDP1_UserClipY0 = cmd_data[0x7] & 0x1FFF;

 VDP1_UserClipX1 = cmd_data[0xA] & 0x1FFF;
 VDP1_UserClipY1 = cmd_data[0xB] & 0x1FFF;

 return 0;
}

int32_t CMD_SetSystemClip(const uint16_t* cmd_data)
{
 VDP1_SysClipX = cmd_data[0xA] & 0x1FFF;
 VDP1_SysClipY = cmd_data[0xB] & 0x1FFF;

 return 0;
}

int32_t CMD_SetLocalCoord(const uint16_t* cmd_data)
{
 VDP1_LocalX = sign_x_to_s32(11, cmd_data[0x6] & 0x7FF);
 VDP1_LocalY = sign_x_to_s32(11, cmd_data[0x7] & 0x7FF);

 return 0;
}

/* MDFN_FORCE_INLINE, not INLINE: replaces the C++ TexFetch<ECDSPDMode>
   template. Must inline into each TexFetch_0xNN wrapper so the ColorMode /
   ECD / SPD switch folds to one arm, as the macro-monomorphized form did. */
static MDFN_FORCE_INLINE uint32_t TexFetch_impl(const unsigned ECDSPDMode, uint32_t x)
{
 /* Former C++ template parameter -- must reach this body as a compile-time
    constant (see VDP1_ASSUME_FOLDED in vdp1_common.h). */
 VDP1_ASSUME_FOLDED(ECDSPDMode);
 {
 const uint32_t base = VDP1_LineData.tex_base;
 const bool ECD = ECDSPDMode & 0x10;
 const bool SPD = ECDSPDMode & 0x08;
 const unsigned ColorMode = ECDSPDMode & 0x07;

 uint32_t rtd;
 uint32_t ret_or = 0;

 switch(ColorMode)
 {
  case 0:	// 16 colors, color bank
	rtd = (VDP1_VRAM[(base + (x >> 2)) & 0x3FFFF] >> (((x & 0x3) ^ 0x3) << 2)) & 0xF;

	if(!ECD && rtd == 0xF)
	{
	 VDP1_LineData.ec_count--;	
	 return -1;
	}
	ret_or = VDP1_LineData.cb_or;
	
	if(!SPD) ret_or |= (int32_t)(rtd - 1) >> 31;

	return rtd | ret_or;

  case 1:	// 16 colors, LUT
	rtd = (VDP1_VRAM[(base + (x >> 2)) & 0x3FFFF] >> (((x & 0x3) ^ 0x3) << 2)) & 0xF;

	if(!ECD && rtd == 0xF)
	{
	 VDP1_LineData.ec_count--;
	 return -1;
	}

	if(!SPD) ret_or |= (int32_t)(rtd - 1) >> 31;

	return VDP1_LineData.CLUT[rtd] | ret_or;

  case 2:	// 64 colors, color bank
	rtd = (VDP1_VRAM[(base + (x >> 1)) & 0x3FFFF] >> (((x & 0x1) ^ 0x1) << 3)) & 0xFF;

	if(!ECD && rtd == 0xFF)
	{
	 VDP1_LineData.ec_count--;
	 return -1;
	}

	ret_or = VDP1_LineData.cb_or;

	if(!SPD) ret_or |= (int32_t)(rtd - 1) >> 31;

	return (rtd & 0x3F) | ret_or;

  case 3:	// 128 colors, color bank
	rtd = (VDP1_VRAM[(base + (x >> 1)) & 0x3FFFF] >> (((x & 0x1) ^ 0x1) << 3)) & 0xFF;

	if(!ECD && rtd == 0xFF)
	{
	 VDP1_LineData.ec_count--;
	 return -1;
	}

	ret_or = VDP1_LineData.cb_or;

	if(!SPD) ret_or |= (int32_t)(rtd - 1) >> 31;

	return (rtd & 0x7F) | ret_or;

  case 4:	// 256 colors, color bank
	rtd = (VDP1_VRAM[(base + (x >> 1)) & 0x3FFFF] >> (((x & 0x1) ^ 0x1) << 3)) & 0xFF;

	if(!ECD && rtd == 0xFF)
	{
	 VDP1_LineData.ec_count--;
	 return -1;
	}

	ret_or = VDP1_LineData.cb_or;

	if(!SPD) ret_or |= (int32_t)(rtd - 1) >> 31;

	return rtd | ret_or;

  case 5:	// 32K colors, RGB
  case 6:
  case 7:
	if(ColorMode >= 6)
	 rtd = VDP1_VRAM[0];
	else
	 rtd = VDP1_VRAM[(base + x) & 0x3FFFF];

	if(!ECD && (rtd & 0xC000) == 0x4000)
	{
	 VDP1_LineData.ec_count--;
	 return -1;
	}

	if(!SPD) ret_or |= (int32_t)(rtd - 0x4000) >> 31;

	return rtd | ret_or;
 }

 return 0; /* unreachable: ColorMode is always 0-7 */
 } /* VDP1_ASSUME_FOLDED block */
}

 static uint32_t MDFN_FASTCALL TexFetch_0x00(uint32_t x) { return TexFetch_impl(0x00, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x01(uint32_t x) { return TexFetch_impl(0x01, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x02(uint32_t x) { return TexFetch_impl(0x02, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x03(uint32_t x) { return TexFetch_impl(0x03, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x04(uint32_t x) { return TexFetch_impl(0x04, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x05(uint32_t x) { return TexFetch_impl(0x05, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x06(uint32_t x) { return TexFetch_impl(0x06, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x07(uint32_t x) { return TexFetch_impl(0x07, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x08(uint32_t x) { return TexFetch_impl(0x08, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x09(uint32_t x) { return TexFetch_impl(0x09, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x0A(uint32_t x) { return TexFetch_impl(0x0A, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x0B(uint32_t x) { return TexFetch_impl(0x0B, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x0C(uint32_t x) { return TexFetch_impl(0x0C, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x0D(uint32_t x) { return TexFetch_impl(0x0D, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x0E(uint32_t x) { return TexFetch_impl(0x0E, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x0F(uint32_t x) { return TexFetch_impl(0x0F, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x10(uint32_t x) { return TexFetch_impl(0x10, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x11(uint32_t x) { return TexFetch_impl(0x11, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x12(uint32_t x) { return TexFetch_impl(0x12, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x13(uint32_t x) { return TexFetch_impl(0x13, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x14(uint32_t x) { return TexFetch_impl(0x14, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x15(uint32_t x) { return TexFetch_impl(0x15, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x16(uint32_t x) { return TexFetch_impl(0x16, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x17(uint32_t x) { return TexFetch_impl(0x17, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x18(uint32_t x) { return TexFetch_impl(0x18, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x19(uint32_t x) { return TexFetch_impl(0x19, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x1A(uint32_t x) { return TexFetch_impl(0x1A, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x1B(uint32_t x) { return TexFetch_impl(0x1B, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x1C(uint32_t x) { return TexFetch_impl(0x1C, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x1D(uint32_t x) { return TexFetch_impl(0x1D, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x1E(uint32_t x) { return TexFetch_impl(0x1E, x); }
 static uint32_t MDFN_FASTCALL TexFetch_0x1F(uint32_t x) { return TexFetch_impl(0x1F, x); }

uint32_t (MDFN_FASTCALL *const VDP1_TexFetchTab[0x20])(uint32_t x) =
{
  #define TF(a) (TexFetch_##a)

 TF(0x00), TF(0x01), TF(0x02), TF(0x03),
 TF(0x04), TF(0x05), TF(0x06), TF(0x07),

 TF(0x08), TF(0x09), TF(0x0A), TF(0x0B),
 TF(0x0C), TF(0x0D), TF(0x0E), TF(0x0F),

 TF(0x10), TF(0x11), TF(0x12), TF(0x13),
 TF(0x14), TF(0x15), TF(0x16), TF(0x17),

 TF(0x18), TF(0x19), TF(0x1A), TF(0x1B),
 TF(0x1C), TF(0x1D), TF(0x1E), TF(0x1F),

 #undef TF
};

bool VDP1_SetupDrawLine(int32_t* const cycle_counter, const bool AA, const bool Textured, const uint16_t mode)
{
 const bool HSS = (mode & 0x1000);
 const bool PCD = (mode & 0x800);
 const bool UserClipEn = (mode & 0x400);
 const bool UserClipMode = (mode & 0x200);
 //const bool ECD = (mode & 0x80);
 //const bool SPD = (mode & 0x40);
 const bool GouraudEn = (mode & 0x8004) == 0x4;
 line_vertex p0 = VDP1_LineData.p[0];
 line_vertex p1 = VDP1_LineData.p[1];
 line_inner_data* const lidp = &VDP1_LineInnerData;
 bool clipped = false;

 p0.x &= 0x1FFF;
 p0.y &= 0x1FFF;
 p1.x &= 0x1FFF;
 p1.y &= 0x1FFF;

 if(!PCD)
 {
  bool swapped = false;

  *cycle_counter += 4;

  if(UserClipEn && !UserClipMode)
  {
   // Ignore system clipping WRT pre-clip for UserClipEn == 1 && UserClipMode == 0
   clipped |= (((VDP1_UserClipX1 - p0.x) & (VDP1_UserClipX1 - p1.x)) | ((p0.x - VDP1_UserClipX0) & (p1.x - VDP1_UserClipX0))) & 0x1000;
   clipped |= (((VDP1_UserClipY1 - p0.y) & (VDP1_UserClipY1 - p1.y)) | ((p0.y - VDP1_UserClipY0) & (p1.y - VDP1_UserClipY0))) & 0x1000;

   swapped = (p0.y == p1.y) & ((p0.x < VDP1_UserClipX0) | (p0.x > VDP1_UserClipX1));
  }
  else
  {
   clipped |= (((VDP1_SysClipX - p0.x) & (VDP1_SysClipX - p1.x)) | (p0.x & p1.x)) & 0x1000;
   clipped |= (((VDP1_SysClipY - p0.y) & (VDP1_SysClipY - p1.y)) | (p0.y & p1.y)) & 0x1000;

   swapped = (p0.y == p1.y) & (p0.x > VDP1_SysClipX);
  }
  //
  // VDP1 reduces the line into a point to clip it, and it can be seen in the framebuffer under
  // certain conditions relating to coordinate precision.
  //
  if(clipped)
   p1 = p0;
  else if(swapped)
  {
   /* std::swap(p0, p1) folded; braces are required -- this is an
    * unbraced else-if body, so the three swap statements must be a
    * block or only the first would bind to the else-if. */
   line_vertex tmp_v = p0;
   p0 = p1;
   p1 = tmp_v;
  }
 }

 *cycle_counter += 8;

 //
 //
 const int32_t dx = sign_x_to_s32(13, p1.x - p0.x);
 const int32_t dy = sign_x_to_s32(13, p1.y - p0.y);
 const int32_t abs_dx = abs(dx); // & 0xFFF;
 const int32_t abs_dy = abs(dy); // & 0xFFF;
 const int32_t max_adx_ady = ((int32_t)(abs_dx) > (int32_t)(abs_dy) ? (int32_t)(abs_dx) : (int32_t)(abs_dy));
 const int32_t x_inc = (dx >= 0) ? 1 : -1;
 const int32_t y_inc = (dy >= 0) ? 1 : -1;
 const int32_t lid_x_inc = (x_inc & 0x7FF) <<  0;
 const int32_t lid_y_inc = (y_inc & 0x7FF) << 16;

 lidp->xy = (p0.x & 0x7FF) + ((p0.y & 0x7FF) << 16);
 lidp->term_xy = (p1.x & 0x7FF) + ((p1.y & 0x7FF) << 16);
 lidp->drawn_ac = true;	// Drawn all-clipped
 lidp->color = VDP1_LineData.color;

 if(GouraudEn)
  Gourauder_Setup(&lidp->g, max_adx_ady + 1, p0.g, p1.g);

 if(Textured)
 {
  VDP1_LineData.ec_count = 2;	// Call before tffn()

  if(MDFN_UNLIKELY(max_adx_ady < abs(p1.t - p0.t) && HSS))
  {
   VDP1_LineData.ec_count = 0x7FFFFFFF;
   VileTex_Setup(&lidp->t, max_adx_ady + 1, p0.t >> 1, p1.t >> 1, 2, (bool)(VDP1_FBCR & VDP1_FBCR_EOS));
  }
  else
   VileTex_Setup(&lidp->t, max_adx_ady + 1, p0.t, p1.t, 1, 0);

  lidp->texel = VDP1_LineData.tffn(VileTex_Current(&lidp->t));
 }

 {
  int32_t aa_x_inc;
  int32_t aa_y_inc;

  if(abs_dy > abs_dx)
  {
   if(y_inc < 0)
   {
    aa_x_inc =  (x_inc >> 31);
    aa_y_inc = -(x_inc >> 31);
   }
   else
   {
    aa_x_inc = -(~x_inc >> 31);
    aa_y_inc =  (~x_inc >> 31);
   }
  }
  else
  {
   if(x_inc < 0)
   {
    aa_x_inc = -(~y_inc >> 31);
    aa_y_inc = -(~y_inc >> 31);
   }
   else
   {
    aa_x_inc =  (y_inc >> 31);
    aa_y_inc =  (y_inc >> 31);
   }
  }
  lidp->aa_xy_inc = (aa_x_inc & 0x7FF) + ((aa_y_inc & 0x7FF) << 16);
 }

 // x, y, x_inc, y_inc, aa_x_inc, aa_y_inc, term_x, term_y, error, error_inc, error_adj, t, g, color
 if(abs_dy > abs_dx)
 {
  lidp->error_inc =  (2 * abs_dx);
  lidp->error_adj = -(2 * abs_dy);
  lidp->error = (abs_dy - (2 * abs_dy)) - 1;
  lidp->error_cmp = 0;

  if(dy < 0 && !AA)
   lidp->error_cmp--;

  lidp->error -= lidp->error_inc;
  lidp->xy = (lidp->xy + (0x8000000 - lid_y_inc)) & 0x07FF07FF;
  lidp->xy_inc[0] = lid_y_inc;
  lidp->xy_inc[1] = lid_x_inc;
 }
 else
 {
  lidp->error_inc =  (2 * abs_dy);
  lidp->error_adj = -(2 * abs_dx);
  lidp->error = (abs_dx - (2 * abs_dx)) - 1;
  lidp->error_cmp = 0;

  if(dx < 0 && !AA)
   lidp->error_cmp--;

  lidp->error -= lidp->error_inc;
  lidp->xy = (lidp->xy + (0x800 - lid_x_inc)) & 0x07FF07FF;
  lidp->xy_inc[0] = lid_x_inc;
  lidp->xy_inc[1] = lid_y_inc;
 }
 if(AA)
 {
  lidp->error++;
  lidp->error_cmp++;
 }

 //
 lidp->error_inc <<= 32 - 13;
 lidp->error_adj <<= 32 - 13;
 lidp->error <<= 32 - 13;
 lidp->error_cmp = (uint32_t)lidp->error_cmp << (32 - 13);

 return clipped;
}

void EdgeStepper_Setup(EdgeStepper *self, const bool gourauden, const line_vertex *p0, const line_vertex *p1, const int32_t dmax)
{
  int32_t dx = sign_x_to_s32(13, p1->x - p0->x);
  int32_t dy = sign_x_to_s32(13, p1->y - p0->y);
  int32_t abs_dx = abs(dx);
  int32_t abs_dy = abs(dy);
  int32_t max_adxdy = ((int32_t)(abs_dx) > (int32_t)(abs_dy) ? (int32_t)(abs_dx) : (int32_t)(abs_dy));

  self->x = p0->x;
  self->x_inc = (dx >= 0) ? 1 : -1;
  self->x_error_inc =  (2 * abs_dx);
  self->x_error_adj = -(2 * max_adxdy);
  self->x_error = (max_adxdy - (2 * max_adxdy)) - 1;
  self->x_error_cmp = (dy < 0) ? -1 : 0;

  self->y = p0->y;
  self->y_inc = (dy >= 0) ? 1 : -1;
  self->y_error_inc =  (2 * abs_dy);
  self->y_error_adj = -(2 * max_adxdy);
  self->y_error = (max_adxdy - (2 * max_adxdy)) - 1;
  self->y_error_cmp = (dx < 0) ? -1 : 0;

  self->d_error = dmax - (2 * dmax) - 1;
  self->d_error_inc =  (2 * max_adxdy);
  self->d_error_adj = -(2 * dmax);
  self->d_error_cmp = (((abs_dy > abs_dx) ? dy : dx) < 0) ? -1 : 0;
  //
  self->x_error <<= (32 - 13);
  self->x_error_inc <<= (32 - 13);
  self->x_error_adj <<= (32 - 13);
  self->x_error_cmp = (uint32_t)self->x_error_cmp << (32 - 13);

  self->y_error <<= (32 - 13);
  self->y_error_inc <<= (32 - 13);
  self->y_error_adj <<= (32 - 13);
  self->y_error_cmp = (uint32_t)self->y_error_cmp << (32 - 13);

  self->d_error <<= (32 - 13);
  self->d_error_inc <<= (32 - 13);
  self->d_error_adj <<= (32 - 13);
  self->d_error_cmp = (uint32_t)self->d_error_cmp << (32 - 13);
  //
  if(gourauden)
   Gourauder_Setup(&self->g, max_adxdy + 1, p0->g, p1->g);
}

enum { CommandPhaseBias = __COUNTER__ + 1 };
#define VDP1_EAT_CLOCKS(n)									\
		{										\
		 CycleCounter -= (n);								\
		 case __COUNTER__:								\
		 if(CycleCounter <= 0)								\
		 {										\
		  CommandPhase = __COUNTER__ - CommandPhaseBias - 1;				\
		  goto Breakout;								\
		 }										\
		}										\

static INLINE void DoDrawing(void)
{
 if(MDFN_UNLIKELY(ss_horrible_hacks & HORRIBLEHACK_VDP1INSTANT))
  CycleCounter = InstantDrawSanityLimit;

 switch(CommandPhase + CommandPhaseBias)
 {
  for(;;)
  {
   default:
   VDP1_EAT_CLOCKS(0);

   // Fetch command data
   memcpy(CommandData, &VDP1_VRAM[CurCommandAddr], sizeof(CommandData));

   VDP1_EAT_CLOCKS(16);

   if(MDFN_LIKELY(!(CommandData[0] & 0xC000)))
   {
    if(MDFN_UNLIKELY((CommandData[0] & 0xF) >= 0xC))
    {
     DrawingActive = false;
     goto Breakout;
    }
    else
    {
     static int32_t (*const command_table[0xC])(const uint16_t* cmd_data) =
     {
      /* 0x0 */         /* 0x1 */           /* 0x2 */            /* 0x3 */
      VDP1_CMD_NormalSprite, VDP1_CMD_ScaledSprite,   VDP1_CMD_DistortedSprite, VDP1_CMD_DistortedSprite,

      /* 0x4 */         /* 0x5 (polyline) *//* 0x6 */            /* 0x7 (polyline) */
      VDP1_CMD_Polygon,      VDP1_CMD_Line,	    VDP1_CMD_Line,            VDP1_CMD_Line,

      /* 0x8*/          /* 0x9 */           /* 0xA */            /* 0xB */
      CMD_SetUserClip,  CMD_SetSystemClip,  CMD_SetLocalCoord,   CMD_SetUserClip
     };

     static int32_t (*const resume_table[0x8])(const uint16_t* cmd_data) =
     {
      /* 0x0 */         /* 0x1 */         /* 0x2 */            /* 0x3 */
      VDP1_RESUME_Sprite, VDP1_RESUME_Sprite, VDP1_RESUME_Sprite, VDP1_RESUME_Sprite,

      /* 0x4 */    /* 0x5 */     /* 0x6 */ /* 0x7 */
      VDP1_RESUME_Polygon, VDP1_RESUME_Line, VDP1_RESUME_Line, VDP1_RESUME_Line,
     };

     VDP1_EAT_CLOCKS(command_table[CommandData[0] & 0xF](CommandData));
     if(!(CommandData[0] & 0x8))
     {
      for(;;)
      {
       int32_t cycles;

       cycles = resume_table[CommandData[0] & 0x7](CommandData);

       if(!cycles)
        break;

       VDP1_EAT_CLOCKS(cycles);
      }
     }
    }
   }
   else if(MDFN_UNLIKELY(CommandData[0] & 0x8000))
   {
    DrawingActive = false;

    EDSR |= 0x2;	// TODO: Does EDSR reflect IRQ out status?

    SCU_SetInt(SCU_INT_VDP1, true);
    SCU_SetInt(SCU_INT_VDP1, false);
    goto Breakout;
   }

   CurCommandAddr = (CurCommandAddr + 0x10) & 0x3FFFF;
   switch((CommandData[0] >> 12) & 0x3)
   {
    case 0:
	break;

    case 1:
	CurCommandAddr = (CommandData[1] << 2) &~ 0xF;
	break;

    case 2:
	if(RetCommandAddr < 0)
	 RetCommandAddr = CurCommandAddr;

	CurCommandAddr = (CommandData[1] << 2) &~ 0xF;
	break;

    case 3:
	if(RetCommandAddr >= 0)
	{
	 CurCommandAddr = RetCommandAddr;
	 RetCommandAddr = -1;
	}
	break;
   }
  //
  //
  //
  }
 }
 Breakout:;

 if(MDFN_UNLIKELY(ss_horrible_hacks & HORRIBLEHACK_VDP1INSTANT))
  InstantDrawSanityLimit = CycleCounter;
}

sscpu_timestamp_t VDP1_Update(sscpu_timestamp_t timestamp)
{
 if(MDFN_UNLIKELY(timestamp < lastts))
 {
  // Don't else { } normal execution, since this bug condition miiight occur in the call from SetHBVB(),
  // and we need drawing to start ASAP before silly games overwrite the beginning of the command table.
  //
  timestamp = lastts;
 }
 //
 //
 //
 int32_t cycles = timestamp - lastts;
 lastts = timestamp;

 CycleCounter += cycles;
 if(CycleCounter > VDP1_UpdateTimingGran)
  CycleCounter = VDP1_UpdateTimingGran;

 if(CycleCounter > 0 && SCU_CheckVDP1HaltKludge())
  CycleCounter = 0;
 else if(DrawingActive)
  DoDrawing();

 return timestamp + (DrawingActive ? ((int32_t)(VDP1_UpdateTimingGran) > (int32_t)(0 - CycleCounter) ? (int32_t)(VDP1_UpdateTimingGran) : (int32_t)(0 - CycleCounter)) : VDP1_IdleTimingGran);
}

// Draw-clear minimum x amount is 2(16-bit units) for normal and 8bpp, and 8 for rotate...actually, seems like
// rotate being enabled forces vblank erase mode somehow.

static void StartDrawing(void)
{
 // On draw start, clear CEF.
 EDSR &= ~0x2;

 CurCommandAddr = 0;
 RetCommandAddr = -1;
 DrawingActive = true;
 CommandPhase = 0;

 CycleCounter = VDP1_UpdateTimingGran;
}

void VDP1_SetHBVB(const sscpu_timestamp_t event_timestamp, const bool new_hb_status, const bool new_vb_status)
{
 const bool old_hb_status = hb_status;
 const bool old_vb_status = vb_status;

 hb_status = new_hb_status;
 vb_status = new_vb_status;

 if(MDFN_UNLIKELY(vbcdpending & hb_status & (old_hb_status ^ hb_status)))
 {
  vbcdpending = false;

  if(vb_status) // Going into v-blank
  {
   //
   // v-blank erase
   //
   if((VDP1_TVMR & VDP1_TVMR_VBE) || FBVBErasePending)
   {

    FBVBErasePending = false;
    FBVBEraseActive = true;
    FBVBEraseLastTS = event_timestamp;
   }
  }
  else // Leaving v-blank
  {
   InstantDrawSanityLimit = 1000000;

   // Run vblank erase at end of vblank all at once(not strictly accurate, but should only have visible side effects wrt the debugger and reset).
   if(FBVBEraseActive)
   {
    int32_t count = event_timestamp - FBVBEraseLastTS;
    //
    //
    //
    uint32_t y = EraseParams.y_start;

    do
    {
     uint16_t* fbyptr;
     uint16_t* mfbyptr;
     uint32_t x = EraseParams.x_start;

     fbyptr = &VDP1_FB[!FBDrawWhich][(y & 0xFF) << 9];
     mfbyptr = &VDP1_MeshFB[!FBDrawWhich][(y & 0xFF) << 9];
     if(EraseParams.rot8)
     {
      fbyptr += (y & 0x100);
      mfbyptr += (y & 0x100);
     }

     count -= 8;
     do
     {
      for(unsigned sub = 0; sub < 8; sub++)
      {
       fbyptr[x & EraseParams.fb_x_mask] = EraseParams.fill_data;
       // Clear the side-buffer in lockstep with the main FB so the
       // new draw side starts with no stale mesh pixels from the
       // previous frame. Mesh side-buffer always erases to 0
       // regardless of the game's chosen FB fill colour, since 0
       // is the "no mesh pixel here" marker.
       mfbyptr[x & EraseParams.fb_x_mask] = 0;
       x++;
      }
      count -= 8;
      if(MDFN_UNLIKELY(count <= 0))
       goto AbortVBErase;
     } while(x < EraseParams.x_bound);
    } while(++y <= EraseParams.y_end);

    AbortVBErase:;
    //
    FBVBEraseActive = false;
   }
   //
   //
   //
   //
   if(!(VDP1_FBCR & VDP1_FBCR_FCM) || (FBManualPending && (VDP1_FBCR & VDP1_FBCR_FCT)))	// Swap framebuffers
   {
    if((ss_horrible_hacks & HORRIBLEHACK_VDP1VRAM5000FIX) && DrawingActive && VDP1_VRAM[0] == 0x5000 && VDP1_VRAM[1] == 0x0000)
     VDP1_VRAM[0] = 0x8000;

    if(DrawingActive)
     DrawingActive = false;

    FBDrawWhich = !FBDrawWhich;
    VDP1_FBDrawWhichPtr = VDP1_FB[FBDrawWhich];
    VDP1_MeshFBDrawWhichPtr = VDP1_MeshFB[FBDrawWhich];

    // Unconditionally clear the new draw side of VDP1_MeshFB. Game-driven erase
    // (VBErase, per-scanline GetLine erase) is gated on the game's VDP1_FBCR/
    // EraseParams settings, which won't fire if the game uses manual buffer
    // management. Mesh data is transient by nature - we always want a fresh
    // slate per frame, regardless of the game's main-FB persistence choices.
    if(VDP1_MeshImproved)
     memset(VDP1_MeshFBDrawWhichPtr, 0, sizeof(VDP1_MeshFB[0]));

    // On fb swap, copy CEF to BEF, clear CEF, and copy COPR to LOPR.
    EDSR = EDSR >> 1;
    LOPR = CurCommandAddr >> 2;

    //
    EraseParams.rot8 = (VDP1_TVMR & (VDP1_TVMR_8BPP | VDP1_TVMR_ROTATE)) == (VDP1_TVMR_8BPP | VDP1_TVMR_ROTATE);
    EraseParams.fb_x_mask = EraseParams.rot8 ? 0xFF : 0x1FF;

    EraseParams.y_start = EWLR & 0x1FF;
    EraseParams.x_start = ((EWLR >> 9) & 0x3F) << 3;

    EraseParams.y_end = EWRR & 0x1FF;
    EraseParams.x_bound = ((EWRR >> 9) & 0x7F) << 3;

    EraseParams.fill_data = EWDR;
    //

    if(PTMR & 0x2)	// Start drawing(but only if we swapped the frame)
    {
     StartDrawing();
     SS_SetEventNT(&events[SS_EVENT_VDP1], VDP1_Update(event_timestamp));
    }
   }

   EraseYCounter = ~0U;
   if(!(VDP1_FBCR & VDP1_FBCR_FCM) || (FBManualPending && !(VDP1_FBCR & VDP1_FBCR_FCT)))
   {
    if(VDP1_TVMR & VDP1_TVMR_ROTATE)
     FBVBErasePending = true;
    else
     EraseYCounter = EraseParams.y_start;
   }

   FBManualPending = false;
  }
 }
 vbcdpending |= old_vb_status ^ vb_status;
}

bool VDP1_GetLine(const int line, uint16_t* buf, uint16_t* mesh_buf, unsigned w, uint32_t rot_x, uint32_t rot_y, uint32_t rot_xinc, uint32_t rot_yinc)
{
 bool ret = false;
 //
 //
 //
 if(VDP1_TVMR & VDP1_TVMR_ROTATE)
 {
  const uint16_t* fbptr = VDP1_FB[!FBDrawWhich];
  const uint16_t* mfbptr = VDP1_MeshFB[!FBDrawWhich];

  if(VDP1_TVMR & VDP1_TVMR_8BPP)
  {
   for(unsigned i = 0; MDFN_LIKELY(i < w); i++)
   {
    const uint32_t fb_x = rot_x >> 9;
    const uint32_t fb_y = rot_y >> 9;

    if((fb_x | fb_y) &~ 0x1FF)
    {
     buf[i] = 0;	// Not 0xFF00
     mesh_buf[i] = 0;
    }
    else
    {
     const uint16_t* fbyptr = fbptr + ((fb_y & 0xFF) << 9);
     /* ne16_rbo_be<uint8_t>(base_u16, byte_off) folded:
      * byte read from BE bus over uint16_t-array. */
     const uint32_t boff_ = (fb_x & 0x1FF) | ((fb_y & 0x100) << 1);
#ifdef MSB_FIRST
     uint8_t tmp = ((const uint8_t*)fbyptr)[boff_];
#else
     uint8_t tmp = ((const uint8_t*)fbyptr)[boff_ ^ 1];
#endif

     buf[i] = 0xFF00 | tmp;
     // 8bpp paletted mode doesn't use mesh-improved (PlotPixel
     // gates improved mesh on the 16bpp path only), so VDP1_MeshFB
     // is guaranteed clear in 8bpp mode -- but read it anyway
     // in case mode bits changed mid-frame.
     const uint16_t* mfbyptr = mfbptr + ((fb_y & 0xFF) << 9);
#ifdef MSB_FIRST
     uint8_t mtmp = ((const uint8_t*)mfbyptr)[boff_];
#else
     uint8_t mtmp = ((const uint8_t*)mfbyptr)[boff_ ^ 1];
#endif
     mesh_buf[i] = mtmp;
    }

    rot_x += rot_xinc;
    rot_y += rot_yinc;
   }
  }
  else
  {
   for(unsigned i = 0; MDFN_LIKELY(i < w); i++)
   {
    const uint32_t fb_x = rot_x >> 9;
    const uint32_t fb_y = rot_y >> 9;

    if((fb_x &~ 0x1FF) | (fb_y &~ 0xFF))
    {
     buf[i] = 0;
     mesh_buf[i] = 0;
    }
    else
    {
     buf[i] = fbptr[(fb_y << 9) + fb_x];
     mesh_buf[i] = mfbptr[(fb_y << 9) + fb_x];
    }

    rot_x += rot_xinc;
    rot_y += rot_yinc;
   }
  }
 }
 else
 {
  const uint16_t* fbyptr = &VDP1_FB[!FBDrawWhich][(line & 0xFF) << 9];
  const uint16_t* mfbyptr = &VDP1_MeshFB[!FBDrawWhich][(line & 0xFF) << 9];

  if(VDP1_TVMR & VDP1_TVMR_8BPP)
   ret = true;

  // Plain contiguous copy of w uint16_t framebuffer cells into the
  // per-scanline sprite buffer. Same job in both 16bpp (each cell =
  // one pixel) and 8bpp (each cell = two pixels) modes, since the
  // copy is byte-for-byte either way. memcpy guarantees a vectorised
  // path on every backend (rep-movsq on x86-64, LDP/STP on AArch64);
  // the previous scalar for-loop with MDFN_LIKELY was at the mercy
  // of the compiler's autovectorisation heuristics.
  memcpy(buf, fbyptr, (size_t)w * sizeof(uint16_t));
  /* MeshFB row is only ever read by ApplyMeshOverlay (vdp2_render),
   * which is itself gated on VDP1_MeshImproved.  When the option is
   * off this memcpy lands in a buffer nothing reads -- skip it.  At
   * 352-wide x 224 lines x 60Hz that saves ~9.5 MB/s of dead copy
   * traffic in the default state. */
  if(VDP1_MeshImproved)
   memcpy(mesh_buf, mfbyptr, (size_t)w * sizeof(uint16_t));
 }

 //
 //
 //
 if(EraseYCounter <= EraseParams.y_end)
 {
  uint16_t* fbyptr;
  uint16_t* mfbyptr;
  uint32_t x = EraseParams.x_start;

  fbyptr = &VDP1_FB[!FBDrawWhich][(EraseYCounter & 0xFF) << 9];
  mfbyptr = &VDP1_MeshFB[!FBDrawWhich][(EraseYCounter & 0xFF) << 9];
  if(EraseParams.rot8)
  {
   fbyptr += (EraseYCounter & 0x100);
   mfbyptr += (EraseYCounter & 0x100);
  }

  do
  {
   for(unsigned sub = 0; sub < 2; sub++)
   {
    fbyptr[x & EraseParams.fb_x_mask] = EraseParams.fill_data;
    mfbyptr[x & EraseParams.fb_x_mask] = 0;
    x++;
   }
  } while(x < EraseParams.x_bound);

  EraseYCounter++;
 }

 return ret;
}

void VDP1_AdjustTS(const int32_t delta)
{
 lastts += delta;
 if(FBVBEraseActive)
  FBVBEraseLastTS += delta;

 LastRWTS = ((sscpu_timestamp_t)(-1000000) > (sscpu_timestamp_t)(LastRWTS + delta) ? (sscpu_timestamp_t)(-1000000) : (sscpu_timestamp_t)(LastRWTS + delta));
}

static INLINE void WriteReg(const unsigned which, const uint16_t value)
{
 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(SH7095_mem_timestamp));
 sscpu_timestamp_t nt = VDP1_Update(SH7095_mem_timestamp);

 switch(which)
 {
  default:
	break;

  case 0x0:	// VDP1_TVMR
	VDP1_TVMR = value & 0xF;
	break;

  case 0x1:	// VDP1_FBCR
	VDP1_FBCR = value & 0x1F;
	FBManualPending |= value & 0x2;
	break;

  case 0x2:	// PTMR
	PTMR = (value & 0x3);
	if(value & 0x1)
	{
	 StartDrawing();
	 nt = SH7095_mem_timestamp + 1;
	}
	break;

  case 0x3:	// EWDR
	EWDR = value;
	break;

  case 0x4:	// EWLR
	EWLR = value & 0x7FFF;
	break;

  case 0x5:	// EWRR
	EWRR = value;
	break;

  case 0x6:	// ENDR
	if(DrawingActive)
	{
	 DrawingActive = false;
	 if(CycleCounter < 0)
	  CycleCounter = 0;
	 nt = SH7095_mem_timestamp + VDP1_IdleTimingGran;
	}
	break;

 }

 SS_SetEventNT(&events[SS_EVENT_VDP1], nt);
}

static INLINE uint16_t ReadReg(const unsigned which)
{
 switch(which)
 {
  default:
        break;

  case 0x8:	// EDSR
	return EDSR;

  case 0x9:	// LOPR
	return LOPR;

  case 0xA:	// COPR
	return CurCommandAddr >> 2;

  case 0xB:	// MODR
	return (0x1 << 12) | ((PTMR & 0x2) << 7) | ((VDP1_FBCR & 0x1E) << 3) | (VDP1_TVMR << 0);
 }

 return 0;
}

//
// Due to the emulated CPUs running faster than they should(due to lack of instruction cache emulation, lack of emulation of some pipeline details,
// lack of memory refresh cycle emulation), there is the potential that a game may write too much data too fast, causing drawing to timeout and abort,
// and the game could subsequently hang waiting for drawing to complete.  With this in mind, only selectively enable it for games that are known
// to benefit, via the horrible hacks mechanism.
//
MDFN_FASTCALL void VDP1_Write_CheckDrawSlowdown(uint32_t A, sscpu_timestamp_t time_thing)
{
 if(DrawingActive && time_thing > LastRWTS && (ss_horrible_hacks & HORRIBLEHACK_VDP1RWDRAWSLOWDOWN))
 {
  const int32_t count = (A & 0x100000) ? 22 : 25;
  const uint32_t a = ((uint32_t)(count) < (uint32_t)(time_thing - LastRWTS) ? (uint32_t)(count) : (uint32_t)(time_thing - LastRWTS));

  CycleCounter -= a;
  LastRWTS = time_thing;
 }
}

MDFN_FASTCALL void VDP1_Read_CheckDrawSlowdown(uint32_t A, sscpu_timestamp_t time_thing)
{
 if(!(A & 0x100000) && time_thing > LastRWTS && DrawingActive && (ss_horrible_hacks & HORRIBLEHACK_VDP1RWDRAWSLOWDOWN))
 {
  const int32_t count = (A & 0x80000) ? 44 : 41;
  const uint32_t a = ((uint32_t)(count) < (uint32_t)(time_thing - LastRWTS) ? (uint32_t)(count) : (uint32_t)(time_thing - LastRWTS));

  CycleCounter -= a;
  LastRWTS = time_thing;
 }
}

MDFN_FASTCALL void VDP1_Write8_DB(uint32_t A, uint16_t DB)
{
 A &= 0x1FFFFF;

 if(A < 0x80000)
 {
  /* ne16_wbo_be<uint8_t>(VDP1_VRAM, A, val) folded. */
  const uint8_t val_ = DB >> (((A & 1) ^ 1) << 3);
#ifdef MSB_FIRST
  ((uint8_t*)VDP1_VRAM)[A] = val_;
#else
  ((uint8_t*)VDP1_VRAM)[A ^ 1] = val_;
#endif
  return;
 }

 if(A < 0x100000)
 {
  uint32_t FBA = A;

  if((VDP1_TVMR & (VDP1_TVMR_8BPP | VDP1_TVMR_ROTATE)) == (VDP1_TVMR_8BPP | VDP1_TVMR_ROTATE))
   FBA = (FBA & 0x1FF) | ((FBA << 1) & 0x3FC00) | ((FBA >> 8) & 0x200);

  /* ne16_wbo_be<uint8_t>(VDP1_FB[..], offs, val) folded. */
  const uint32_t fbo_ = FBA & 0x3FFFF;
  const uint8_t val_ = DB >> (((A & 1) ^ 1) << 3);
#ifdef MSB_FIRST
  ((uint8_t*)VDP1_FB[FBDrawWhich])[fbo_] = val_;
#else
  ((uint8_t*)VDP1_FB[FBDrawWhich])[fbo_ ^ 1] = val_;
#endif
  return;
 }

 WriteReg((A - 0x100000) >> 1, DB);
}

MDFN_FASTCALL void VDP1_Write16_DB(uint32_t A, uint16_t DB)
{
 A &= 0x1FFFFE;

 if(A < 0x80000)
 {
  VDP1_VRAM[A >> 1] = DB;
  return;
 }

 if(A < 0x100000)
 {
  uint32_t FBA = A;

  if((VDP1_TVMR & (VDP1_TVMR_8BPP | VDP1_TVMR_ROTATE)) == (VDP1_TVMR_8BPP | VDP1_TVMR_ROTATE))
   FBA = (FBA & 0x1FF) | ((FBA << 1) & 0x3FC00) | ((FBA >> 8) & 0x200);

  VDP1_FB[FBDrawWhich][(FBA >> 1) & 0x1FFFF] = DB;
  return;
 }

 WriteReg((A - 0x100000) >> 1, DB);
}

MDFN_FASTCALL uint16_t VDP1_Read16_DB(uint32_t A)
{
 A &= 0x1FFFFE;

 if(A < 0x080000)
  return VDP1_VRAM[A >> 1];

 if(A < 0x100000)
 {
  uint32_t FBA = A;

  if((VDP1_TVMR & (VDP1_TVMR_8BPP | VDP1_TVMR_ROTATE)) == (VDP1_TVMR_8BPP | VDP1_TVMR_ROTATE))
   FBA = (FBA & 0x1FF) | ((FBA << 1) & 0x3FC00) | ((FBA >> 8) & 0x200);

  return VDP1_FB[FBDrawWhich][(FBA >> 1) & 0x1FFFF];
 }

 return ReadReg((A - 0x100000) >> 1);
}

void VDP1_StateAction(StateMem* sm, const unsigned load, const bool data_only)
{
 bool tmp_abs_dy_gt_abs_dx = false;

 /* On-disk savestate keys deliberately use the short, unqualified names
    (VRAM, PrimData.*, LineData.*, ...) so that states written before the
    C-conversion of VDP1 load with the same keys. vdp1_common.h defines
    short-name aliases for every VDP1_-prefixed extern; the # operator
    captures argument tokens before macro expansion, so SFVAR(VRAM)
    stringifies to "VRAM" while the expression (VRAM) expands to
    (VDP1_VRAM). */
 SFORMAT Prim_StateRegs[] =
 {
  SFVAR(PrimData.e->d_error, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->d_error_inc, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->d_error_adj, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->d_error_cmp, 0x2, sizeof(*PrimData.e), PrimData.e),

  SFVAR(PrimData.e->x, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->x_inc, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->x_error, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->x_error_inc, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->x_error_adj, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->x_error_cmp, 0x2, sizeof(*PrimData.e), PrimData.e),

  SFVAR(PrimData.e->y, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->y_inc, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->y_error, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->y_error_inc, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->y_error_adj, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->y_error_cmp, 0x2, sizeof(*PrimData.e), PrimData.e),

  SFVAR(PrimData.e->g.g, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFVAR(PrimData.e->g.intinc, 0x2, sizeof(*PrimData.e), PrimData.e),
  SFPTR32N(&(PrimData.e->g.ginc)[0], (sizeof(PrimData.e->g.ginc) / sizeof(int32_t)), 0x2, sizeof(*PrimData.e), PrimData.e, "PrimData.e->g.ginc"),
  SFPTR32N(&(PrimData.e->g.error)[0], (sizeof(PrimData.e->g.error) / sizeof(int32_t)), 0x2, sizeof(*PrimData.e), PrimData.e, "PrimData.e->g.error"),
  SFPTR32N(&(PrimData.e->g.error_inc)[0], (sizeof(PrimData.e->g.error_inc) / sizeof(int32_t)), 0x2, sizeof(*PrimData.e), PrimData.e, "PrimData.e->g.error_inc"),
  SFPTR32N(&(PrimData.e->g.error_adj)[0], (sizeof(PrimData.e->g.error_adj) / sizeof(int32_t)), 0x2, sizeof(*PrimData.e), PrimData.e, "PrimData.e->g.error_adj"),

  SFVAR(PrimData.big_t.t),
  SFVAR(PrimData.big_t.tinc),
  SFVAR(PrimData.big_t.error),
  SFVAR(PrimData.big_t.error_inc),
  SFVAR(PrimData.big_t.error_adj),

  SFVAR(PrimData.iter),
  SFVAR(PrimData.tex_base),
  SFVAR(PrimData.need_line_resume),
  //
  //
  //
  SFVAR(LineInnerData.xy),
  SFVAR(LineInnerData.error),
  SFVAR(LineInnerData.drawn_ac),

  SFVAR(LineInnerData.texel),

  SFVAR(LineInnerData.t.t),
  SFVAR(LineInnerData.t.tinc),
  SFVAR(LineInnerData.t.error),
  SFVAR(LineInnerData.t.error_inc),
  SFVAR(LineInnerData.t.error_adj),

  SFVAR(LineInnerData.g.g),
  SFVAR(LineInnerData.g.intinc),
  SFPTR32N(&(LineInnerData.g.ginc)[0], (sizeof(LineInnerData.g.ginc) / sizeof(int32_t)), "LineInnerData.g.ginc"),
  SFPTR32N(&(LineInnerData.g.error)[0], (sizeof(LineInnerData.g.error) / sizeof(int32_t)), "LineInnerData.g.error"),
  SFPTR32N(&(LineInnerData.g.error_inc)[0], (sizeof(LineInnerData.g.error_inc) / sizeof(int32_t)), "LineInnerData.g.error_inc"),
  SFPTR32N(&(LineInnerData.g.error_adj)[0], (sizeof(LineInnerData.g.error_adj) / sizeof(int32_t)), "LineInnerData.g.error_adj"),

  SFVARN(LineInnerData.xy_inc[0], "LineInnerData.x_inc"),
  SFVARN(LineInnerData.xy_inc[1], "LineInnerData.y_inc"),
  SFVAR(LineInnerData.aa_xy_inc),
  SFVAR(LineInnerData.term_xy),

  SFVAR(LineInnerData.error_cmp),
  SFVAR(LineInnerData.error_inc),
  SFVAR(LineInnerData.error_adj),

  SFVAR(LineInnerData.color),

  SFVARN(tmp_abs_dy_gt_abs_dx, "LineInnerData.abs_dy_gt_abs_dx"),
  //
  //
  //
  SFVAR(LineData.p->t, 0x2, sizeof(*LineData.p), LineData.p),
  SFVAR(LineData.color),
  SFVAR(LineData.ec_count),
  //uint32_t (MDFN_FASTCALL *tffn)(uint32_t);
  SFPTR16N(&(LineData.CLUT)[0], (sizeof(LineData.CLUT) / sizeof(uint16_t)), "LineData.CLUT"),
  SFVAR(LineData.cb_or),
  SFVAR(LineData.tex_base),
  //
  //
  //
  SFEND
 };

 SFORMAT StateRegs[] =
 {
  SFPTR16N(&(VRAM)[0], (sizeof(VRAM) / sizeof(uint16_t)), "VRAM"),
  SFPTR16N(&(FB)[0][0], (sizeof(FB) / sizeof(uint16_t)), "&FB[0][0]"),
  SFPTR16N(&(MeshFB)[0][0], (sizeof(MeshFB) / sizeof(uint16_t)), "&MeshFB[0][0]"),
  SFVAR(FBDrawWhich),

  SFVAR(FBManualPending),

  SFVAR(FBVBErasePending),
  SFVAR(FBVBEraseActive),
  SFVAR(FBVBEraseLastTS),

  SFVAR(SysClipX),
  SFVAR(SysClipY),
  SFVAR(UserClipX0),
  SFVAR(UserClipY0),
  SFVAR(UserClipX1),
  SFVAR(UserClipY1),
  SFVAR(LocalX),
  SFVAR(LocalY),

  SFVAR(CurCommandAddr),
  SFVAR(RetCommandAddr),
  SFVAR(DrawingActive),

  SFVAR(LOPR),

  SFVAR(EWDR),
  SFVAR(EWLR),
  SFVAR(EWRR),	// Erase/Write Lower Right Coordinate

  SFVAR(EraseParams.rot8),
  // Recovered in if(load): SFVAR(EraseParams.fb_x_mask),

  SFVAR(EraseParams.y_start),
  SFVAR(EraseParams.x_start),

  SFVAR(EraseParams.y_end),
  SFVAR(EraseParams.x_bound),

  SFVAR(EraseParams.fill_data),

  SFVAR(EraseYCounter),

  SFVAR(TVMR),
  SFVAR(FBCR),
  SFVAR(PTMR),
  SFVAR(EDSR),

  SFVAR(vb_status),
  SFVAR(hb_status),
  SFVAR(lastts),
  SFVAR(CycleCounter),
  SFVAR(CommandPhase),
  SFVAR(CommandData),
  SFVAR(DTACounter),

  SFVAR(vbcdpending),

  SFVAR(LastRWTS),

  SFVAR(InstantDrawSanityLimit),

  SFLINK(Prim_StateRegs),

  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, "VDP1", false);

 if(load)
 {
  // The drawing loop fetches 16 uint16s from VDP1_VRAM[CurCommandAddr]
  // via memcpy (search for "Fetch command data"). VDP1_VRAM has 0x40000
  // entries, so the largest in-bounds index for a 16-element read
  // is 0x3FFF0. The previous mask of 0x3FFFF let a hostile save
  // state push CurCommandAddr into [0x3FFF1, 0x3FFFF], producing a
  // 30-byte out-of-bounds read of adjacent memory into the next
  // command's data buffer.
  //
  // Mask to 0x3FFF0 instead. This also enforces the 0x10-alignment
  // invariant maintained by every other writer (init=0, increment
  // by 0x10, jumps masked with `& ~0xF`), which the drawing loop
  // implicitly assumes.
  CurCommandAddr &= 0x3FFF0;
  if(RetCommandAddr >= 0)
   RetCommandAddr &= 0x3FFF0;

  VDP1_DTACounter &= 0xFF;

  EraseParams.fb_x_mask = EraseParams.rot8 ? 0xFF : 0x1FF;

  EraseParams.y_start &= 0x1FF;
  EraseParams.x_start &= 0x3F << 3;

  EraseParams.y_end &= 0x1FF;
  EraseParams.x_bound &= 0x7F << 3;
  //
  VDP1_FBDrawWhichPtr = VDP1_FB[FBDrawWhich];
  VDP1_MeshFBDrawWhichPtr = VDP1_MeshFB[FBDrawWhich];

  if(load < 0x00102500)
  {
   CommandPhase = 0;
  }

  if(tmp_abs_dy_gt_abs_dx)
  {
   /* std::swap(xy_inc[0], xy_inc[1]) folded; braced because this is
    * an unbraced if body. */
   int32_t tmp_xy = VDP1_LineInnerData.xy_inc[0];
   VDP1_LineInnerData.xy_inc[0] = VDP1_LineInnerData.xy_inc[1];
   VDP1_LineInnerData.xy_inc[1] = tmp_xy;
  }
 }
}

uint32_t VDP1_GetRegister(const unsigned id, char* const special, const uint32_t special_len)
{
 uint32_t ret = 0xDEADBEEF;

 switch(id)
 {
  case VDP1_GSREG_SYSCLIPX:
	ret = VDP1_SysClipX;
	break;

  case VDP1_GSREG_SYSCLIPY:
	ret = VDP1_SysClipY;
	break;

  case VDP1_GSREG_USERCLIPX0:
	ret = VDP1_UserClipX0;
	break;

  case VDP1_GSREG_USERCLIPY0:
	ret = VDP1_UserClipY0;
	break;

  case VDP1_GSREG_USERCLIPX1:
	ret = VDP1_UserClipX1;
	break;

  case VDP1_GSREG_USERCLIPY1:
	ret = VDP1_UserClipY1;
	break;

  case VDP1_GSREG_LOCALX:
	ret = VDP1_LocalX;
	break;

  case VDP1_GSREG_LOCALY:
	ret = VDP1_LocalY;
 	break;

  case VDP1_GSREG_TVMR:
	ret = VDP1_TVMR;
	break;

  case VDP1_GSREG_FBCR:
	ret = VDP1_FBCR;
	break;

  case VDP1_GSREG_EWDR:
	ret = EWDR;
	break;

  case VDP1_GSREG_EWLR:
	ret = EWLR;
	break;

  case VDP1_GSREG_EWRR:
	ret = EWRR;
	break;
 }

 return ret;
}

void VDP1_SetRegister(const unsigned id, const uint32_t value)
{
 // TODO
 switch(id)
 {
  case VDP1_GSREG_SYSCLIPX:
	VDP1_SysClipX = value & 0x1FFF;
	break;

  case VDP1_GSREG_SYSCLIPY:
	VDP1_SysClipY = value & 0x1FFF;
	break;

  case VDP1_GSREG_USERCLIPX0:
	VDP1_UserClipX0 = value & 0x1FFF;
	break;

  case VDP1_GSREG_USERCLIPY0:
	VDP1_UserClipY0 = value & 0x1FFF;
	break;

  case VDP1_GSREG_USERCLIPX1:
	VDP1_UserClipX1 = value & 0x1FFF;
	break;

  case VDP1_GSREG_USERCLIPY1:
	VDP1_UserClipY1 = value & 0x1FFF;
	break;

/*
  case VDP1_GSREG_LOCALX:
	ret = VDP1_LocalX;
	break;

  case VDP1_GSREG_LOCALY:
	ret = VDP1_LocalY;
 	break;

  case VDP1_GSREG_TVMR:
	ret = VDP1_TVMR;
	break;

  case VDP1_GSREG_FBCR:
	ret = VDP1_FBCR;
	break;

  case VDP1_GSREG_EWDR:
	ret = EWDR;
	break;

  case VDP1_GSREG_EWLR:
	ret = EWLR;
	break;

  case VDP1_GSREG_EWRR:
	ret = EWRR;
	break;
*/
 }
}

/* end namespace VDP1 */
