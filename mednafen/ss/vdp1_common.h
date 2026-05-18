/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* vdp1_common.h:
**  Copyright (C) 2015-2020 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
*/

#ifndef __MDFN_SS_VDP1_COMMON_H
#define __MDFN_SS_VDP1_COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <boolean.h>
#include <retro_inline.h>
#include "../mednafen-types.h"
#include "vdp1.h"

/* Internal shorthand: map unqualified names to VDP1_-prefixed globals.
   Only visible within the vdp1*.c files that include this header. */
#define VRAM VDP1_VRAM
#define FB VDP1_FB
#define FBDrawWhichPtr VDP1_FBDrawWhichPtr
#define MeshFB VDP1_MeshFB
#define MeshFBDrawWhichPtr VDP1_MeshFBDrawWhichPtr
#define SysClipX VDP1_SysClipX
#define SysClipY VDP1_SysClipY
#define UserClipX0 VDP1_UserClipX0
#define UserClipY0 VDP1_UserClipY0
#define UserClipX1 VDP1_UserClipX1
#define UserClipY1 VDP1_UserClipY1
#define LocalX VDP1_LocalX
#define LocalY VDP1_LocalY
#define TexFetchTab VDP1_TexFetchTab
#define TVMR VDP1_TVMR
#define FBCR VDP1_FBCR
#define spr_w_shift_tab VDP1_spr_w_shift_tab
#define gouraud_lut VDP1_gouraud_lut
#define LineData VDP1_LineData
#define LineInnerData VDP1_LineInnerData
#define PrimData VDP1_PrimData
#define DTACounter VDP1_DTACounter
#define SetupDrawLine VDP1_SetupDrawLine
#define AdjustDrawTiming VDP1_AdjustDrawTiming
#define TVMR_8BPP VDP1_TVMR_8BPP
#define TVMR_ROTATE VDP1_TVMR_ROTATE
#define TVMR_HDTV VDP1_TVMR_HDTV
#define TVMR_VBE VDP1_TVMR_VBE
#define FBCR_FCT VDP1_FBCR_FCT
#define FBCR_FCM VDP1_FBCR_FCM
#define FBCR_DIL VDP1_FBCR_DIL
#define FBCR_DIE VDP1_FBCR_DIE
#define FBCR_EOS VDP1_FBCR_EOS

enum { VDP1_SuspendResumeThreshold = 1000 };

int32_t VDP1_CMD_NormalSprite(const uint16_t*);
int32_t VDP1_CMD_ScaledSprite(const uint16_t*);
int32_t VDP1_CMD_DistortedSprite(const uint16_t*);
int32_t VDP1_RESUME_Sprite(const uint16_t*);
int32_t VDP1_CMD_Polygon(const uint16_t*);
int32_t VDP1_RESUME_Polygon(const uint16_t*);
int32_t VDP1_CMD_Line(const uint16_t*);
int32_t VDP1_RESUME_Line(const uint16_t*);

MDFN_HIDE extern uint16_t* VDP1_FBDrawWhichPtr;
MDFN_HIDE extern uint16_t VDP1_MeshFB[2][0x20000];
MDFN_HIDE extern uint16_t* VDP1_MeshFBDrawWhichPtr;
MDFN_HIDE extern int32_t VDP1_SysClipX, VDP1_SysClipY;
MDFN_HIDE extern int32_t VDP1_UserClipX0, VDP1_UserClipY0, VDP1_UserClipX1, VDP1_UserClipY1;
MDFN_HIDE extern int32_t VDP1_LocalX, VDP1_LocalY;
MDFN_HIDE extern uint32_t (MDFN_FASTCALL *const VDP1_TexFetchTab[0x20])(uint32_t x);

enum { VDP1_TVMR_8BPP   = 0x1 };
enum { VDP1_TVMR_ROTATE = 0x2 };
enum { VDP1_TVMR_HDTV   = 0x4 };
enum { VDP1_TVMR_VBE    = 0x8 };
MDFN_HIDE extern uint8_t VDP1_TVMR;

enum { VDP1_FBCR_FCT = 0x01 };
enum { VDP1_FBCR_FCM = 0x02 };
enum { VDP1_FBCR_DIL = 0x04 };
enum { VDP1_FBCR_DIE = 0x08 };
enum { VDP1_FBCR_EOS = 0x10 };
MDFN_HIDE extern uint8_t VDP1_FBCR;

MDFN_HIDE extern uint8_t VDP1_spr_w_shift_tab[8];
MDFN_HIDE extern uint8_t VDP1_gouraud_lut[0x40];

typedef struct {
 uint32_t g; uint32_t intinc;
 int32_t ginc[4]; int32_t error[4]; int32_t error_inc[4]; int32_t error_adj[4];
} GourauderTheTerrible;

static INLINE void Gourauder_Setup(GourauderTheTerrible *self, const unsigned length, const uint16_t gstart, const uint16_t gend)
{
 unsigned cc; self->g = gstart & 0x7FFF; self->intinc = 0;
 for(cc = 0; cc < 3; cc++) {
  const int dg = ((gend >> (cc * 5)) & 0x1F) - ((gstart >> (cc * 5)) & 0x1F);
  const unsigned abs_dg = abs(dg);
  self->ginc[cc] = (uint32_t)((dg >= 0) ? 1 : -1) << (cc * 5);
  if(length <= abs_dg) {
   self->error_inc[cc] = (abs_dg + 1) * 2; self->error_adj[cc] = (length * 2);
   self->error[cc] = abs_dg + 1 - (length * 2 + ((dg < 0) ? 1 : 0));
   while(self->error[cc] >= 0) { self->g += self->ginc[cc]; self->error[cc] -= self->error_adj[cc]; }
   while(self->error_inc[cc] >= self->error_adj[cc]) { self->intinc += self->ginc[cc]; self->error_inc[cc] -= self->error_adj[cc]; }
  } else {
   self->error_inc[cc] = abs_dg * 2; self->error_adj[cc] = ((length - 1) * 2);
   self->error[cc] = length - (length * 2 - ((dg < 0) ? 1 : 0));
   if(self->error[cc] >= 0) { self->g += self->ginc[cc]; self->error[cc] -= self->error_adj[cc]; }
   if(self->error_inc[cc] >= self->error_adj[cc]) { self->intinc += self->ginc[cc]; self->error_inc[cc] -= self->error_adj[cc]; }
  }
  self->error[cc] = ~self->error[cc];
 }
 self->ginc[3] = 0; self->error[3] = 0; self->error_inc[3] = 0; self->error_adj[3] = 0;
}
static INLINE uint32_t Gourauder_Current(GourauderTheTerrible *self) { return self->g; }
static INLINE uint16_t Gourauder_Apply(const GourauderTheTerrible *self, uint16_t pix) {
 uint16_t ret = pix & 0x8000;
 ret |= VDP1_gouraud_lut[((pix & (0x1F <<  0)) + (self->g & (0x1F <<  0))) >>  0] <<  0;
 ret |= VDP1_gouraud_lut[((pix & (0x1F <<  5)) + (self->g & (0x1F <<  5))) >>  5] <<  5;
 ret |= VDP1_gouraud_lut[((pix & (0x1F << 10)) + (self->g & (0x1F << 10))) >> 10] << 10;
 return ret;
}
static INLINE void Gourauder_Step(GourauderTheTerrible *self) {
 int32_t g_delta = 0; unsigned cc;
 for(cc = 0; cc < 4; cc++) {
  self->error[cc] -= self->error_inc[cc];
  { const int32_t mask = self->error[cc] >> 31;
    g_delta += self->ginc[cc] & mask; self->error[cc] += self->error_adj[cc] & mask; }
 }
 self->g += self->intinc + (uint32_t)g_delta;
}

typedef struct { int32_t t, tinc, error, error_inc, error_adj; } VileTex;

static INLINE bool VileTex_Setup(VileTex *self, const unsigned length, const int32_t tstart, const int32_t tend, const int32_t sf, const int32_t tfudge) {
 int dt = tend - tstart; unsigned abs_dt = abs(dt);
 self->t = (tstart * sf) | tfudge; self->tinc = (dt >= 0) ? sf : -sf;
 if(length <= abs_dt) { self->error_inc = (abs_dt + 1) * 2; self->error_adj = (length * 2); self->error = abs_dt + 1 - (length * 2 + ((dt < 0) ? 1 : 0)); }
 else { self->error_inc = abs_dt * 2; self->error_adj = ((length - 1) * 2); self->error = length - (length * 2 - ((dt < 0) ? 1 : 0)); }
 return false;
}
static INLINE bool VileTex_IncPending(VileTex *self) { return self->error >= 0; }
static INLINE int32_t VileTex_DoPendingInc(VileTex *self) { self->t += self->tinc; self->error -= self->error_adj; return self->t; }
static INLINE void VileTex_AddError(VileTex *self) { self->error += self->error_inc; }
static INLINE int32_t VileTex_PreStep(VileTex *self) { while(self->error >= 0) { self->t += self->tinc; self->error -= self->error_adj; } self->error += self->error_inc; return self->t; }
static INLINE int32_t VileTex_Current(VileTex *self) { return self->t; }

typedef struct { int32_t x, y; uint16_t g; int32_t t; } line_vertex;

typedef struct {
 uint32_t d_error, d_error_inc, d_error_adj; int32_t d_error_cmp;
 uint32_t x, x_inc, x_error, x_error_inc, x_error_adj; int32_t x_error_cmp;
 uint32_t y, y_inc, y_error, y_error_inc, y_error_adj; int32_t y_error_cmp;
 GourauderTheTerrible g;
} EdgeStepper;

void EdgeStepper_Setup(EdgeStepper *self, const bool gourauden, const line_vertex *p0, const line_vertex *p1, const int32_t dmax);

static INLINE void EdgeStepper_GetVertex_gouraud(EdgeStepper *s, line_vertex *p) { p->x = s->x; p->y = s->y; p->g = Gourauder_Current(&s->g); }
static INLINE void EdgeStepper_GetVertex_nogouraud(EdgeStepper *s, line_vertex *p) { p->x = s->x; p->y = s->y; }

static INLINE void EdgeStepper_Step_gouraud(EdgeStepper *s) {
 s->d_error += s->d_error_inc;
 if((int32_t)s->d_error >= s->d_error_cmp) { s->d_error += s->d_error_adj;
  s->x_error += s->x_error_inc; { const uint32_t m = -((int32_t)s->x_error >= s->x_error_cmp); s->x += s->x_inc & m; s->x_error += s->x_error_adj & m; }
  s->y_error += s->y_error_inc; { const uint32_t m = -((int32_t)s->y_error >= s->y_error_cmp); s->y += s->y_inc & m; s->y_error += s->y_error_adj & m; }
  Gourauder_Step(&s->g); }
}
static INLINE void EdgeStepper_Step_nogouraud(EdgeStepper *s) {
 s->d_error += s->d_error_inc;
 if((int32_t)s->d_error >= s->d_error_cmp) { s->d_error += s->d_error_adj;
  s->x_error += s->x_error_inc; { const uint32_t m = -((int32_t)s->x_error >= s->x_error_cmp); s->x += s->x_inc & m; s->x_error += s->x_error_adj & m; }
  s->y_error += s->y_error_inc; { const uint32_t m = -((int32_t)s->y_error >= s->y_error_cmp); s->y += s->y_inc & m; s->y_error += s->y_error_adj & m; } }
}

/* Unified dispatch wrappers: gourauden is a compile-time constant at every
   call site, so the compiler folds the branch and inlines the right variant. */
static INLINE void EdgeStepper_GetVertex(EdgeStepper *s, const int gourauden, line_vertex *p)
{ if(gourauden) EdgeStepper_GetVertex_gouraud(s, p); else EdgeStepper_GetVertex_nogouraud(s, p); }
static INLINE void EdgeStepper_Step(EdgeStepper *s, const int gourauden)
{ if(gourauden) EdgeStepper_Step_gouraud(s); else EdgeStepper_Step_nogouraud(s); }

typedef struct { uint32_t xy, error; bool drawn_ac; uint32_t texel; VileTex t; GourauderTheTerrible g;
 int32_t xy_inc[2]; uint32_t aa_xy_inc, term_xy; int32_t error_cmp; uint32_t error_inc, error_adj; uint16_t color; } line_inner_data;
typedef struct { line_vertex p[2]; uint16_t color; int32_t ec_count;
 uint32_t (MDFN_FASTCALL *tffn)(uint32_t); uint16_t CLUT[0x10]; uint32_t cb_or, tex_base; } line_data;
MDFN_HIDE extern line_data VDP1_LineData;
MDFN_HIDE extern line_inner_data VDP1_LineInnerData;
typedef struct { EdgeStepper e[2]; VileTex big_t; uint32_t tex_base; int32_t iter; bool need_line_resume; } prim_data;
MDFN_HIDE extern prim_data VDP1_PrimData;

static INLINE int32_t VDP1_AdjustDrawTiming(const int32_t cycles) {
 MDFN_HIDE extern uint32_t VDP1_DTACounter; uint32_t extra_cycles;
 VDP1_DTACounter += cycles * ((VDP1_TVMR & VDP1_TVMR_8BPP) ? 24 : 48);
 extra_cycles = VDP1_DTACounter >> 8; VDP1_DTACounter &= 0xFF; return cycles + extra_cycles;
}

bool VDP1_SetupDrawLine(int32_t* const cycle_counter, const bool AA, const bool Textured, const uint16_t mode);

/* ---------------------------------------------------------------------------
 * VDP1_ASSUME_FOLDED -- compile-time guarantee that a template-engine
 * parameter constant-folds.
 *
 * VDP1_DrawLine_impl / VDP1_PlotPixel / TexFetch_impl are the C replacements
 * for C++ templates: each is MDFN_FORCE_INLINE and every dispatch wrapper
 * calls it with literal constant args, so the if(param) branches dead-strip
 * exactly as a macro-monomorphized form did. VDP1_ASSUME_FOLDED() turns that
 * from an assumption into a hard requirement -- if a parameter ever stops
 * folding (engine downgraded from MDFN_FORCE_INLINE to plain INLINE, or
 * called with a runtime argument) the build FAILS at that source line
 * instead of silently regressing into per-pixel runtime branching.
 * tools/check_engine_inlined.py is the same check from the outside; this is
 * the same check at the source site.
 *
 * Gated on __OPTIMIZE__: at -O0, __builtin_constant_p() is conservatively 0
 * before constant propagation runs, so this is a no-op for debug builds and
 * active for every optimized build (the core ships -O2).
 * ------------------------------------------------------------------------- */
#if defined(__OPTIMIZE__)
 #if defined(__GNUC__) && !defined(__clang__)
  extern void VDP1_PARAM_NOT_FOLDED(void)
    __attribute__((error(
      "VDP1 template-engine parameter did not constant-fold: the engine is "
      "not being force-inlined into its wrapper -- check MDFN_FORCE_INLINE")));
 #else
  /* Portable fallback (clang/others that don't honor the error attribute):
     left undefined on purpose, so a surviving call is an unmistakable
     'undefined reference to VDP1_PARAM_NOT_FOLDED' link error. */
  extern void VDP1_PARAM_NOT_FOLDED(void);
 #endif
 #define VDP1_ASSUME_FOLDED(x) \
   do { if(!__builtin_constant_p(x)) VDP1_PARAM_NOT_FOLDED(); } while(0)
#else
 #define VDP1_ASSUME_FOLDED(x) ((void)0)
#endif

/* MDFN_FORCE_INLINE, not INLINE: this is the C replacement for a C++ template.
   Each wrapper calls it with all-constant former-template args; forcing the
   inline is what lets the compiler dead-strip the if(param) branches, exactly
   as macro-monomorphized form did. Plain INLINE is only a hint and the compiler
   refuses it here (function too large), leaving one generic out-of-line copy
   with every branch live -- a real per-pixel slowdown vs the C++ original. */
static MDFN_FORCE_INLINE int32_t VDP1_PlotPixel(const int die, const unsigned bpp8, const int MSBOn,
 const int UserClipEn, const int UserClipMode, const int MeshEn,
 const int GouraudEn, const int HalfFGEn, const int HalfBGEn,
 int32_t x, int32_t y, uint16_t pix, bool transparent, GourauderTheTerrible* g)
{
 /* Former C++ template parameters -- must reach this body as compile-time
    constants (see VDP1_ASSUME_FOLDED above). */
 VDP1_ASSUME_FOLDED(die);    VDP1_ASSUME_FOLDED(bpp8);   VDP1_ASSUME_FOLDED(MSBOn);
 VDP1_ASSUME_FOLDED(UserClipEn); VDP1_ASSUME_FOLDED(UserClipMode);
 VDP1_ASSUME_FOLDED(MeshEn); VDP1_ASSUME_FOLDED(GouraudEn);
 VDP1_ASSUME_FOLDED(HalfFGEn); VDP1_ASSUME_FOLDED(HalfBGEn);
 {
 int32_t ret = 0;
 uint16_t* fbyptr;
 if(die) { fbyptr = &VDP1_FBDrawWhichPtr[((y >> 1) & 0xFF) << 9]; transparent |= ((y & 1) != (bool)(VDP1_FBCR & VDP1_FBCR_DIL)); }
 else fbyptr = &VDP1_FBDrawWhichPtr[(y & 0xFF) << 9];
 if(MeshEn) { if(bpp8 || !VDP1_MeshImproved) transparent |= (x ^ y) & 1; }
 if(bpp8) {
  if(MSBOn) { pix = (fbyptr[((x >> 1) & 0x1FF)] | 0x8000) >> (((x & 1) ^ 1) << 3); ret += 5; }
  else if(HalfBGEn) ret += 5;
  if(!transparent) { const uint32_t boff__ = (bpp8 == 2) ? ((x & 0x1FF) | ((y & 0x100) << 1)) : (x & 0x3FF);
#ifdef MSB_FIRST
   ((uint8_t*)fbyptr)[boff__] = pix;
#else
   ((uint8_t*)fbyptr)[boff__ ^ 1] = pix;
#endif
  } ret++;
 } else {
  uint16_t* const p = &fbyptr[x & 0x1FF];
  if(MSBOn) { pix = *p | 0x8000; ret += 5; }
  else {
   if(HalfBGEn) { uint16_t bg_pix = *p; ret += 5;
    if(bg_pix & 0x8000) {
     if(HalfFGEn) { if(GouraudEn) pix = Gourauder_Apply(g, pix); pix = ((pix + bg_pix) - ((pix ^ bg_pix) & 0x8421)) >> 1; }
     else { if(GouraudEn) pix = 0; else pix = ((bg_pix & 0x7BDE) >> 1) | (bg_pix & 0x8000); }
    } else {
     if(HalfFGEn) { if(GouraudEn) pix = Gourauder_Apply(g, pix); }
     else { if(GouraudEn) pix = 0; else pix = bg_pix; }
    }
   } else { if(GouraudEn) pix = Gourauder_Apply(g, pix); if(HalfFGEn) pix = ((pix & 0x7BDE) >> 1) | (pix & 0x8000); }
  }
  if(MeshEn && !MSBOn && !HalfBGEn && !HalfFGEn && VDP1_MeshImproved) {
   if(!transparent) { const uint32_t row = die ? ((y >> 1) & 0xFF) : (y & 0xFF); VDP1_MeshFBDrawWhichPtr[(row << 9) + (x & 0x1FF)] = pix; }
  } else {
   if(!transparent) *p = pix;
   if(!MeshEn && VDP1_MeshImproved && !transparent) { const uint32_t row = die ? ((y >> 1) & 0xFF) : (y & 0xFF); VDP1_MeshFBDrawWhichPtr[(row << 9) + (x & 0x1FF)] = 0; }
  }
  ret++;
 }
 return ret;
 } /* VDP1_ASSUME_FOLDED block */
}

#define VDP1_PBODY(pxy, dl_die, dl_bpp8, dl_MSBOn, dl_UCE, dl_UCM, dl_ME, dl_GE, dl_HFE, dl_HBE) \
 { const uint32_t px = (uint16_t)(pxy); const uint32_t py = (pxy) >> 16; bool clipped; \
  if(dl_UCE && !dl_UCM) clipped = ((uclipo1 - (pxy)) | ((pxy) - uclipo0)) & 0x80008000; \
  else clipped = (clipo - (pxy)) & 0x80008000; \
  if(MDFN_UNLIKELY((clipped ^ lid.drawn_ac) & clipped)) return ret; \
  lid.drawn_ac &= clipped; \
  if(dl_UCE) { if(!dl_UCM) clipped |= (clipo - (pxy)) & 0x80008000; \
   else clipped |= !(((uclipo1 - (pxy)) | ((pxy) - uclipo0)) & 0x80008000); } \
  ret += VDP1_PlotPixel(dl_die, dl_bpp8, dl_MSBOn, dl_UCE, dl_UCM, dl_ME, dl_GE, dl_HFE, dl_HBE, px, py, pix, transparent | clipped, (dl_GE ? &lid.g : NULL)); }

/* MDFN_FORCE_INLINE, not INLINE -- see the note on VDP1_PlotPixel above.
   This is the body of the former DrawLine<...> template; it MUST be inlined
   into each VDP1_GEN_WRAPPER slot so the per-slot constant args fold away.
   With plain INLINE the optimizer leaves it out-of-line at -O2 (too big to
   inline into 1728 sites) and every slot becomes a thunk into one generic
   copy with all 13 branches live. */
static MDFN_FORCE_INLINE int32_t VDP1_DrawLine_impl(const int AA, const int Textured,
 const int die, const unsigned bpp8, const int MSBOn,
 const int UserClipEn, const int UserClipMode, const int MeshEn,
 const int ECD, const int SPD, const int GouraudEn,
 const int HalfFGEn, const int HalfBGEn, bool* need_line_resume)
{
 /* Former C++ template parameters -- must reach this body as compile-time
    constants (see VDP1_ASSUME_FOLDED above). */
 VDP1_ASSUME_FOLDED(AA);     VDP1_ASSUME_FOLDED(Textured); VDP1_ASSUME_FOLDED(die);
 VDP1_ASSUME_FOLDED(bpp8);   VDP1_ASSUME_FOLDED(MSBOn);    VDP1_ASSUME_FOLDED(UserClipEn);
 VDP1_ASSUME_FOLDED(UserClipMode); VDP1_ASSUME_FOLDED(MeshEn); VDP1_ASSUME_FOLDED(ECD);
 VDP1_ASSUME_FOLDED(SPD);    VDP1_ASSUME_FOLDED(GouraudEn);
 VDP1_ASSUME_FOLDED(HalfFGEn); VDP1_ASSUME_FOLDED(HalfBGEn);
 {
 const uint32_t clipo = ((VDP1_SysClipY & 0x3FF) << 16) | (VDP1_SysClipX & 0x3FF);
 const uint32_t uclipo0 = ((VDP1_UserClipY0 & 0x3FF) << 16) | (VDP1_UserClipX0 & 0x3FF);
 const uint32_t uclipo1 = ((VDP1_UserClipY1 & 0x3FF) << 16) | (VDP1_UserClipX1 & 0x3FF);
 line_inner_data lid = VDP1_LineInnerData;
 int32_t ret = 0;
 do {
  bool transparent; uint16_t pix;
  if(Textured) {
   while(VileTex_IncPending(&lid.t)) { int32_t tx = VileTex_DoPendingInc(&lid.t);
    lid.texel = VDP1_LineData.tffn(tx);
    if(!ECD && MDFN_UNLIKELY(VDP1_LineData.ec_count <= 0)) return ret; }
   VileTex_AddError(&lid.t);
   transparent = (SPD && ECD) ? false : (lid.texel >> 31); pix = lid.texel;
  } else { pix = lid.color; transparent = !SPD; }
  lid.xy = (lid.xy + lid.xy_inc[0]) & 0x07FF07FF;
  lid.error += lid.error_inc;
  if((int32_t)lid.error >= lid.error_cmp) { lid.error += lid.error_adj;
   if(AA) { const uint32_t aa_xy = (lid.xy + lid.aa_xy_inc) & 0x07FF07FF;
    VDP1_PBODY(aa_xy, die, bpp8, MSBOn, UserClipEn, UserClipMode, MeshEn, GouraudEn, HalfFGEn, HalfBGEn); }
   lid.xy = (lid.xy + lid.xy_inc[1]) & 0x07FF07FF; }
  VDP1_PBODY(lid.xy, die, bpp8, MSBOn, UserClipEn, UserClipMode, MeshEn, GouraudEn, HalfFGEn, HalfBGEn);
  if(GouraudEn) Gourauder_Step(&lid.g);
  if(MDFN_UNLIKELY(ret >= VDP1_SuspendResumeThreshold) && lid.xy != lid.term_xy) {
   VDP1_LineInnerData.xy = lid.xy; VDP1_LineInnerData.error = lid.error; VDP1_LineInnerData.drawn_ac = lid.drawn_ac;
   if(Textured) { VDP1_LineInnerData.texel = lid.texel; VDP1_LineInnerData.t = lid.t; }
   if(GouraudEn) VDP1_LineInnerData.g = lid.g;
   *need_line_resume = true; return ret; }
 } while(MDFN_LIKELY(lid.xy != lid.term_xy));
 return ret;
 } /* VDP1_ASSUME_FOLDED block */
}

/* Wrapper function name: token-pastes (die,bpp8,b,c) into a unique C identifier. */
#define VDP1_DL_NAME(die,bpp8,b,c) VDP1_DL_##die##_##bpp8##_##b##_##c

/* Generate one thin wrapper function that calls DrawLine_impl with constant args.
   The compiler force-inlines DrawLine_impl and constant-folds the branches. */
#define VDP1_GEN_WRAPPER(FNAME, AA,TEX,DIE,BPP8,MSBON,UCE,UCM,ME,ECD,SPD,GE,HFE,HBE) \
 static int32_t FNAME(bool* nlr) { return VDP1_DrawLine_impl(AA,TEX,DIE,BPP8,MSBON,UCE,UCM,ME,ECD,SPD,GE,HFE,HBE,nlr); }

/* --- Table initializer helpers (comma-separated, for LineFuncTab[][][][]) --- */
#define VDP1_LINEFN_B_TAB(M, die, bpp8, b) \
 { M(die,bpp8,b,0x0), M(die,bpp8,b,0x1), M(die,bpp8,b,0x2), M(die,bpp8,b,0x3), \
   M(die,bpp8,b,0x4), M(die,bpp8,b,0x5), M(die,bpp8,b,0x6), M(die,bpp8,b,0x7), \
   M(die,bpp8,b,0x8) }
#define VDP1_LINEFN_BPP8_TAB(M, die, bpp8) \
 { VDP1_LINEFN_B_TAB(M,die,bpp8,0x00), VDP1_LINEFN_B_TAB(M,die,bpp8,0x01), VDP1_LINEFN_B_TAB(M,die,bpp8,0x02), VDP1_LINEFN_B_TAB(M,die,bpp8,0x03), \
   VDP1_LINEFN_B_TAB(M,die,bpp8,0x04), VDP1_LINEFN_B_TAB(M,die,bpp8,0x05), VDP1_LINEFN_B_TAB(M,die,bpp8,0x06), VDP1_LINEFN_B_TAB(M,die,bpp8,0x07), \
   VDP1_LINEFN_B_TAB(M,die,bpp8,0x08), VDP1_LINEFN_B_TAB(M,die,bpp8,0x09), VDP1_LINEFN_B_TAB(M,die,bpp8,0x0A), VDP1_LINEFN_B_TAB(M,die,bpp8,0x0B), \
   VDP1_LINEFN_B_TAB(M,die,bpp8,0x0C), VDP1_LINEFN_B_TAB(M,die,bpp8,0x0D), VDP1_LINEFN_B_TAB(M,die,bpp8,0x0E), VDP1_LINEFN_B_TAB(M,die,bpp8,0x0F), \
   VDP1_LINEFN_B_TAB(M,die,bpp8,0x10), VDP1_LINEFN_B_TAB(M,die,bpp8,0x11), VDP1_LINEFN_B_TAB(M,die,bpp8,0x12), VDP1_LINEFN_B_TAB(M,die,bpp8,0x13), \
   VDP1_LINEFN_B_TAB(M,die,bpp8,0x14), VDP1_LINEFN_B_TAB(M,die,bpp8,0x15), VDP1_LINEFN_B_TAB(M,die,bpp8,0x16), VDP1_LINEFN_B_TAB(M,die,bpp8,0x17), \
   VDP1_LINEFN_B_TAB(M,die,bpp8,0x18), VDP1_LINEFN_B_TAB(M,die,bpp8,0x19), VDP1_LINEFN_B_TAB(M,die,bpp8,0x1A), VDP1_LINEFN_B_TAB(M,die,bpp8,0x1B), \
   VDP1_LINEFN_B_TAB(M,die,bpp8,0x1C), VDP1_LINEFN_B_TAB(M,die,bpp8,0x1D), VDP1_LINEFN_B_TAB(M,die,bpp8,0x1E), VDP1_LINEFN_B_TAB(M,die,bpp8,0x1F) }
#define VDP1_LINEFUNCTAB_INIT(M) \
 { VDP1_LINEFN_BPP8_TAB(M,0,0), VDP1_LINEFN_BPP8_TAB(M,0,1), VDP1_LINEFN_BPP8_TAB(M,0,2) }, \
 { VDP1_LINEFN_BPP8_TAB(M,1,0), VDP1_LINEFN_BPP8_TAB(M,1,1), VDP1_LINEFN_BPP8_TAB(M,1,2) }

#define VDP1_LINEFN_B_DEF(M, die, bpp8, b) \
 M(die,bpp8,b,0x0) M(die,bpp8,b,0x1) M(die,bpp8,b,0x2) M(die,bpp8,b,0x3) \
 M(die,bpp8,b,0x4) M(die,bpp8,b,0x5) M(die,bpp8,b,0x6) M(die,bpp8,b,0x7) M(die,bpp8,b,0x8)
#define VDP1_LINEFN_BPP8_DEF(M, die, bpp8) \
 VDP1_LINEFN_B_DEF(M,die,bpp8,0x00) VDP1_LINEFN_B_DEF(M,die,bpp8,0x01) VDP1_LINEFN_B_DEF(M,die,bpp8,0x02) VDP1_LINEFN_B_DEF(M,die,bpp8,0x03) \
 VDP1_LINEFN_B_DEF(M,die,bpp8,0x04) VDP1_LINEFN_B_DEF(M,die,bpp8,0x05) VDP1_LINEFN_B_DEF(M,die,bpp8,0x06) VDP1_LINEFN_B_DEF(M,die,bpp8,0x07) \
 VDP1_LINEFN_B_DEF(M,die,bpp8,0x08) VDP1_LINEFN_B_DEF(M,die,bpp8,0x09) VDP1_LINEFN_B_DEF(M,die,bpp8,0x0A) VDP1_LINEFN_B_DEF(M,die,bpp8,0x0B) \
 VDP1_LINEFN_B_DEF(M,die,bpp8,0x0C) VDP1_LINEFN_B_DEF(M,die,bpp8,0x0D) VDP1_LINEFN_B_DEF(M,die,bpp8,0x0E) VDP1_LINEFN_B_DEF(M,die,bpp8,0x0F) \
 VDP1_LINEFN_B_DEF(M,die,bpp8,0x10) VDP1_LINEFN_B_DEF(M,die,bpp8,0x11) VDP1_LINEFN_B_DEF(M,die,bpp8,0x12) VDP1_LINEFN_B_DEF(M,die,bpp8,0x13) \
 VDP1_LINEFN_B_DEF(M,die,bpp8,0x14) VDP1_LINEFN_B_DEF(M,die,bpp8,0x15) VDP1_LINEFN_B_DEF(M,die,bpp8,0x16) VDP1_LINEFN_B_DEF(M,die,bpp8,0x17) \
 VDP1_LINEFN_B_DEF(M,die,bpp8,0x18) VDP1_LINEFN_B_DEF(M,die,bpp8,0x19) VDP1_LINEFN_B_DEF(M,die,bpp8,0x1A) VDP1_LINEFN_B_DEF(M,die,bpp8,0x1B) \
 VDP1_LINEFN_B_DEF(M,die,bpp8,0x1C) VDP1_LINEFN_B_DEF(M,die,bpp8,0x1D) VDP1_LINEFN_B_DEF(M,die,bpp8,0x1E) VDP1_LINEFN_B_DEF(M,die,bpp8,0x1F)
#define VDP1_EMIT_ALL_WRAPPERS(M) \
 VDP1_LINEFN_BPP8_DEF(M,0,0) VDP1_LINEFN_BPP8_DEF(M,0,1) VDP1_LINEFN_BPP8_DEF(M,0,2) \
 VDP1_LINEFN_BPP8_DEF(M,1,0) VDP1_LINEFN_BPP8_DEF(M,1,1) VDP1_LINEFN_BPP8_DEF(M,1,2)

#endif
