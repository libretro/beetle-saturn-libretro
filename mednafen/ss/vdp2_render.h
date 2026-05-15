/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* vdp2_render.h:
**  Copyright (C) 2016-2017 Mednafen Team
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

#ifndef __MDFN_SS_VDP2_RENDER_H
#define __MDFN_SS_VDP2_RENDER_H

#include <mednafen/state.h>
/* git.h is C++-only (CheatFormatStruct's std::exception,
 * GameDB_Entry's std::vector, etc.).  This header now needs to
 * parse as C because vdp2.c (formerly vdp2.cpp) includes it.
 * MDFNGI and EmulateSpecStruct are used only as pointers below,
 * so forward decls are sufficient. */
struct MDFNGI;
struct EmulateSpecStruct;

#ifdef __cplusplus
extern "C" {
#endif

void VDP2REND_Init(const bool IsPAL, const uint64_t affinity) MDFN_COLD;
void VDP2REND_SetGetVideoParams(struct MDFNGI* gi, const bool caspect, const int sls, const int sle, const bool show_h_overscan, const bool dohblend) MDFN_COLD;
void VDP2REND_Kill(void) MDFN_COLD;
void VDP2REND_GetGunXTranslation(const bool clock28m, float* scale, float* offs);
void VDP2REND_StartFrame(struct EmulateSpecStruct* espec, const bool clock28m, const int SurfInterlaceField);
void VDP2REND_EndFrame(void);
void VDP2REND_Reset(bool powering_up) MDFN_COLD;
void VDP2REND_SetLayerEnableMask(uint64_t mask) MDFN_COLD;
void VDP2REND_SetDeinterlaceOff(bool off) MDFN_COLD;

/* Array reference parameters (uint16_t (&rr)[0x100] etc.) replaced
 * with plain pointers: the body indexes rr[i] (works with the
 * pointer) and memcpy(VRAM, vr, sizeof(VRAM)) (size taken from the
 * destination, not vr's compile-time array size).  No runtime
 * semantics depend on the reference-to-array form. */
void VDP2REND_StateAction(StateMem* sm, const unsigned load, const bool data_only, uint16_t* rr, uint16_t* cr, uint16_t* vr) MDFN_COLD;

#ifdef __cplusplus
}
#endif

struct VDP2Rend_LIB
{
 struct
 {
  uint32_t Xsp, Ysp;// .10
  uint32_t Xp, Yp; // .10
  uint32_t dX, dY; // .10
  int32_t kx, ky;	 // .16
  uint32_t KAstAccum;
  uint32_t DKAx;
 } rv[2];
 bool vdp1_hires8;
 bool win_ymet[2];
 uint16_t vdp1_line[352];
 // Mesh side-buffer scanline staged here by VDP1_GetLine when the
 // improved-mesh-transparency option is on. Per-pixel: raw VDP1 texel
 // (CRAM offset / colour-bank-OR / priority-CC bits for paletted types
 // 0-4, or RGB555 for direct-colour types 5-7); 0 = no mesh. The
 // composite tail (ApplyMeshOverlay) decodes each entry the same way
 // T_DrawSpriteData would and 50%-blends the result onto the surface.
 // All zeros when the option is off.
 uint16_t vdp1_mesh_line[352];
 // Winning-layer priority per output pixel, captured at T_MixIt's
 // terminal store. ApplyMeshOverlay reads this to gate the mesh blend:
 // a mesh pixel is suppressed where a higher-priority VDP2 layer
 // already occludes the would-be VDP1 sprite at that position
 // (matches Kronos's `if (i <= FBMeshPrio)` rule, using the mesh
 // texel's own priority bits + SpritePrioNum[] lookup as FBMeshPrio).
 // Sized to the hires output width.
 uint8_t vdp1_winprio[704];
};

#ifdef __cplusplus
extern "C" {
#endif

struct VDP2Rend_LIB* VDP2REND_GetLIB(unsigned line);
void VDP2REND_DrawLine(int vdp2_line, const uint32_t crt_line, const bool field);

void VDP2REND_Write8_DB(uint32_t A, uint16_t DB) MDFN_HOT;
void VDP2REND_Write16_DB(uint32_t A, uint16_t DB) MDFN_HOT;
void VDP2REND_WriteBurst16_DB(uint32_t base, uint32_t n16, uint32_t add_mode, const uint16_t* words) MDFN_HOT;

#ifdef __cplusplus
}
#endif


#endif
