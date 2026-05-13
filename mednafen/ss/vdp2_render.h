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


void VDP2REND_Init(const bool IsPAL, const uint64 affinity) MDFN_COLD;
void VDP2REND_SetGetVideoParams(MDFNGI* gi, const bool caspect, const int sls, const int sle, const bool show_h_overscan, const bool dohblend) MDFN_COLD;
void VDP2REND_Kill(void) MDFN_COLD;
void VDP2REND_GetGunXTranslation(const bool clock28m, float* scale, float* offs);
void VDP2REND_StartFrame(EmulateSpecStruct* espec, const bool clock28m, const int SurfInterlaceField);
void VDP2REND_EndFrame(void);
void VDP2REND_Reset(bool powering_up) MDFN_COLD;
void VDP2REND_SetLayerEnableMask(uint64 mask) MDFN_COLD;
void VDP2REND_SetDeinterlaceOff(bool off) MDFN_COLD;

void VDP2REND_StateAction(StateMem* sm, const unsigned load, const bool data_only, uint16 (&rr)[0x100], uint16 (&cr)[2048], uint16 (&vr)[262144]) MDFN_COLD;

struct VDP2Rend_LIB
{
 struct
 {
  uint32 Xsp, Ysp;// .10
  uint32 Xp, Yp; // .10
  uint32 dX, dY; // .10
  int32 kx, ky;	 // .16
  uint32 KAstAccum;
  uint32 DKAx;
 } rv[2];
 bool vdp1_hires8;
 bool win_ymet[2];
 uint16 vdp1_line[352];
 // Mesh side-buffer scanline staged here by VDP1::GetLine when the
 // improved-mesh-transparency option is on. Per-pixel: raw VDP1 texel
 // (CRAM offset / colour-bank-OR / priority-CC bits for paletted types
 // 0-4, or RGB555 for direct-colour types 5-7); 0 = no mesh. The
 // composite tail (ApplyMeshOverlay) decodes each entry the same way
 // T_DrawSpriteData would and 50%-blends the result onto the surface.
 // All zeros when the option is off.
 uint16 vdp1_mesh_line[352];
 // Source pointers populated by VDP1::GetLine. T_DrawSpriteData and
 // ApplyMeshOverlay read from these rather than from vdp1_line /
 // vdp1_mesh_line directly. In the rotation path the pointers point
 // at the scratch buffers above (which GetLine fills via scattered
 // per-pixel FB reads); in the contiguous-row path they point
 // straight at &FB[!FBDrawWhich][...] / &MeshFB[!FBDrawWhich][...],
 // skipping the redundant row memcpy that used to populate the
 // scratch from the FB row byte-for-byte. The display side of FB
 // and MeshFB is not written during a scanline's render (VDP1 draws
 // into the other side), so reading those rows in place is safe for
 // the duration of T_DrawSpriteData / ApplyMeshOverlay.
 const uint16* vdp1_line_src;
 const uint16* vdp1_mesh_line_src;
 // Winning-layer priority per output pixel, captured at T_MixIt's
 // terminal store. ApplyMeshOverlay reads this to gate the mesh blend:
 // a mesh pixel is suppressed where a higher-priority VDP2 layer
 // already occludes the would-be VDP1 sprite at that position
 // (matches Kronos's `if (i <= FBMeshPrio)` rule, using the mesh
 // texel's own priority bits + SpritePrioNum[] lookup as FBMeshPrio).
 // Sized to the hires output width.
 uint8 vdp1_winprio[704];
};

VDP2Rend_LIB* VDP2REND_GetLIB(unsigned line);
void VDP2REND_DrawLine(int vdp2_line, const uint32 crt_line, const bool field);

void VDP2REND_Write8_DB(uint32 A, uint16 DB) MDFN_HOT;
void VDP2REND_Write16_DB(uint32 A, uint16 DB) MDFN_HOT;
void VDP2REND_WriteBurst16_DB(uint32 base, uint32 n16, uint32 add_mode, const uint16* words) MDFN_HOT;


#endif
