/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* vdp1_sprite.c - VDP1 Sprite Drawing Commands Emulation
**  Copyright (C) 2015-2020 Mednafen Team
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

#include "vdp1_common.h"
#include <mednafen/math_ops.h>

//#pragma GCC optimize("Os,no-crossjumping")

/* Sprite: AA=1, Textured=1, ECD=(b&0x02), HalfFGEn=(!bpp8)&&(c&0x2) */
#define VDP1_DL_SPRITE_GEN(die, bpp8, b, c) \
 VDP1_GEN_WRAPPER(VDP1_DL_NAME(die,bpp8,b,c), \
   /*AA*/1, /*Tex*/1, die, bpp8, \
   (c)==0x8, !!((b)&0x10), ((b)&0x10)&&((b)&0x08), \
   !!((b)&0x04), !!((b)&0x02), !!((b)&0x01), \
   !!((c)&0x4), !(bpp8)&&!!((c)&0x2), !!((c)&0x1))

VDP1_EMIT_ALL_WRAPPERS(VDP1_DL_SPRITE_GEN)
#undef VDP1_DL_SPRITE_GEN

static int32_t (*LineFuncTab[2][3][0x20][8 + 1])(bool* need_line_resume) =
{
 #define LINEFN_BC(die, bpp8, b, c)	VDP1_DL_NAME(die,bpp8,b,c)

 #define LINEFN_B(die, bpp8, b)							\
	{								\
	 LINEFN_BC(die, bpp8, b, 0x0), LINEFN_BC(die, bpp8, b, 0x1), LINEFN_BC(die, bpp8, b, 0x2), LINEFN_BC(die, bpp8, b, 0x3),	\
	 LINEFN_BC(die, bpp8, b, 0x4), LINEFN_BC(die, bpp8, b, 0x5), LINEFN_BC(die, bpp8, b, 0x6), LINEFN_BC(die, bpp8, b, 0x7), 	\
	 LINEFN_BC(die, bpp8, b, 0x8), 	/* msb on */						\
	}

 #define LINEFN_BPP8(die, bpp8)							\
 {										\
  LINEFN_B(die, bpp8, 0x00), LINEFN_B(die, bpp8, 0x01), LINEFN_B(die, bpp8, 0x02), LINEFN_B(die, bpp8, 0x03),	\
  LINEFN_B(die, bpp8, 0x04), LINEFN_B(die, bpp8, 0x05), LINEFN_B(die, bpp8, 0x06), LINEFN_B(die, bpp8, 0x07),	\
  LINEFN_B(die, bpp8, 0x08), LINEFN_B(die, bpp8, 0x09), LINEFN_B(die, bpp8, 0x0A), LINEFN_B(die, bpp8, 0x0B),	\
  LINEFN_B(die, bpp8, 0x0C), LINEFN_B(die, bpp8, 0x0D), LINEFN_B(die, bpp8, 0x0E), LINEFN_B(die, bpp8, 0x0F),	\
									\
  LINEFN_B(die, bpp8, 0x10), LINEFN_B(die, bpp8, 0x11), LINEFN_B(die, bpp8, 0x12), LINEFN_B(die, bpp8, 0x13),	\
  LINEFN_B(die, bpp8, 0x14), LINEFN_B(die, bpp8, 0x15), LINEFN_B(die, bpp8, 0x16), LINEFN_B(die, bpp8, 0x17),	\
  LINEFN_B(die, bpp8, 0x18), LINEFN_B(die, bpp8, 0x19), LINEFN_B(die, bpp8, 0x1A), LINEFN_B(die, bpp8, 0x1B),	\
  LINEFN_B(die, bpp8, 0x1C), LINEFN_B(die, bpp8, 0x1D), LINEFN_B(die, bpp8, 0x1E), LINEFN_B(die, bpp8, 0x1F),	\
 }

 {
  LINEFN_BPP8(0, 0),
  LINEFN_BPP8(0, 1),
  LINEFN_BPP8(0, 2),
 },
 {
  LINEFN_BPP8(1, 0),
  LINEFN_BPP8(1, 1),
  LINEFN_BPP8(1, 2),
 }

 #undef LINEFN_BPP8
 #undef LINEFN_B
 #undef LINEFN_BC
};

enum
{
 FORMAT_NORMAL = 0,
 FORMAT_SCALED,
 FORMAT_DISTORTED
};

static int32_t SpriteResumeBase(const uint16_t* cmd_data, const int gourauden)
{
 const uint16_t mode = cmd_data[0x2];
 int32_t (*const fnptr)(bool* need_line_resume) = LineFuncTab[(bool)(FBCR & FBCR_DIE)][(TVMR & TVMR_8BPP) ? ((TVMR & TVMR_ROTATE) ? 2 : 1) : 0][(mode >> 6) & 0x1F][(mode & 0x8000) ? 8 : (mode & 0x7)];
 LineData.tffn = TexFetchTab[(mode >> 3) & 0x1F];
 /*
 ** Don't merge e0 and e1 into a single array, keeping them separate is a workaround for gcc bug #113255
 */
 EdgeStepper e0 = PrimData.e[0];
 EdgeStepper e1 = PrimData.e[1];
 VileTex big_t = PrimData.big_t;
 const uint32_t tex_base = PrimData.tex_base;
 int32_t iter = PrimData.iter;
 int32_t ret = 0;
 /**/
 if(MDFN_UNLIKELY(PrimData.need_line_resume))
 {
  PrimData.need_line_resume = false;
  goto ResumeLine;
 }

 if(iter >= 0)
 {
  do
  {
   EdgeStepper_GetVertex(&e0, gourauden, &LineData.p[0]);
   EdgeStepper_GetVertex(&e1, gourauden, &LineData.p[1]);

   LineData.tex_base = tex_base + VileTex_PreStep(&big_t);
   /**/
   if(!VDP1_SetupDrawLine(&ret, true, true, mode) || !iter)
   {
    ResumeLine:;
    ret += VDP1_AdjustDrawTiming(fnptr(&PrimData.need_line_resume));
    if(MDFN_UNLIKELY(PrimData.need_line_resume))
     break;
   }

   EdgeStepper_Step(&e0, gourauden);
   EdgeStepper_Step(&e1, gourauden);
  } while(MDFN_LIKELY(--iter >= 0 && ret < VDP1_SuspendResumeThreshold));
 }
 /**/
 PrimData.e[0] = e0;
 PrimData.e[1] = e1;
 PrimData.big_t = big_t;
 PrimData.iter = iter;

 return ret;
}

int32_t VDP1_RESUME_Sprite(const uint16_t* cmd_data)
{
 if(cmd_data[0x2] & 0x4) /* gouraud */
  return SpriteResumeBase(cmd_data, 1);
 else
  return SpriteResumeBase(cmd_data, 0);
}

static INLINE int32_t SpriteBase(const uint16_t* cmd_data, const unsigned format)
{
 const unsigned dir = (cmd_data[0] >> 4) & 0x3;
 const uint16_t mode = cmd_data[0x2];
 const unsigned cm = (mode >> 3) & 0x7;
 const uint16_t color = cmd_data[0x3];
 const uint32_t w = ((cmd_data[0x5] >> 8) & 0x3F) << 3;
 const uint32_t h = cmd_data[0x5] & 0xFF;
 line_vertex p[4];
 int32_t ret = 0;
 unsigned i;
 int32_t dmax;
 bool h_inv;
 int32_t tex_base;
 bool v_inv;
 int32_t tv[2];

 (void)color;
 LineData.color = cmd_data[0x3];

 if(format == FORMAT_DISTORTED)
 {
  for(i = 0; i < 4; i++)
  {
   p[i].x = sign_x_to_s32(13, cmd_data[0x6 + (i << 1)]) + LocalX;
   p[i].y = sign_x_to_s32(13, cmd_data[0x7 + (i << 1)]) + LocalY;
  }
 }
 else if(format == FORMAT_NORMAL)
 {
  p[0].x = sign_x_to_s32(13, cmd_data[0x6]) + LocalX;
  p[0].y = sign_x_to_s32(13, cmd_data[0x7]) + LocalY;

  p[1].x = p[0].x + (((uint32_t)(w) > (uint32_t)(1) ? (uint32_t)(w) : (uint32_t)(1)) - 1);
  p[1].y = p[0].y;

  p[2].x = p[1].x;
  p[2].y = p[0].y + (((uint32_t)(h) > (uint32_t)(1) ? (uint32_t)(h) : (uint32_t)(1)) - 1);

  p[3].x = p[0].x;
  p[3].y = p[2].y;
 }
 else if(format == FORMAT_SCALED)
 {
  const unsigned zp = (cmd_data[0] >> 8) & 0xF;
  {
   int32_t zp_x = sign_x_to_s32(13, cmd_data[0x6]);
   int32_t zp_y = sign_x_to_s32(13, cmd_data[0x7]);
   int32_t disp_w = sign_x_to_s32(13, cmd_data[0x8]);
   int32_t disp_h = sign_x_to_s32(13, cmd_data[0x9]);
   int32_t alt_x = sign_x_to_s32(13, cmd_data[0xA]);
   int32_t alt_y = sign_x_to_s32(13, cmd_data[0xB]);

   for(i = 0; i < 4; i++)
   {
    p[i].x = zp_x;
    p[i].y = zp_y;
   }

   switch(zp >> 2)
   {
    case 0x0: p[2].y = alt_y; p[3].y = alt_y; break;
    case 0x1: p[2].y += disp_h; p[3].y += disp_h; break;
    case 0x2: p[0].y -= disp_h >> 1; p[1].y -= disp_h >> 1; p[2].y += (disp_h + 1) >> 1; p[3].y += (disp_h + 1) >> 1; break;
    case 0x3: p[0].y -= disp_h; p[1].y -= disp_h; break;
   }

   switch(zp & 0x3)
   {
    case 0x0: p[1].x = alt_x; p[2].x = alt_x; break;
    case 0x1: p[1].x += disp_w; p[2].x += disp_w; break;
    case 0x2: p[0].x -= disp_w >> 1; p[1].x += (disp_w + 1) >> 1; p[2].x += (disp_w + 1) >> 1; p[3].x -= disp_w >> 1; break;
    case 0x3: p[0].x -= disp_w; p[3].x -= disp_w; break;
   }

   for(i = 0; i < 4; i++)
   {
    p[i].x += LocalX;
    p[i].y += LocalY;
   }
  }
 }

 if(cmd_data[0x2] & 0x4) /* gouraud */
 {
  const uint16_t* gtb = &VRAM[cmd_data[0xE] << 2];
  ret += 4;
  for(i = 0; i < 4; i++)
   p[i].g = gtb[i];
 }
 /**/
 {
  h_inv = dir & 1;
  LineData.p[0 ^ h_inv].t = 0;
  LineData.p[1 ^ h_inv].t = w ? (w - 1) : 0;
 }

 switch(cm)
 {
  case 0: LineData.cb_or = color &~ 0xF;   break;
  case 1: for(i = 0; i < 16; i++) LineData.CLUT[i] = VRAM[((color &~ 0x3) << 2) | i]; ret += 16; break;
  case 2: LineData.cb_or = color &~ 0x3F;  break;
  case 3: LineData.cb_or = color &~ 0x7F;  break;
  case 4: LineData.cb_or = color &~ 0xFF;  break;
  case 5: break;
  case 6: break;
  case 7: break;
 }
 /**/
 dmax =                    abs(sign_x_to_s32(13, p[3].x - p[0].x));
 if((abs(sign_x_to_s32(13, p[3].y - p[0].y))) > dmax) dmax = (abs(sign_x_to_s32(13, p[3].y - p[0].y)));
 if((abs(sign_x_to_s32(13, p[2].x - p[1].x))) > dmax) dmax = (abs(sign_x_to_s32(13, p[2].x - p[1].x)));
 if((abs(sign_x_to_s32(13, p[2].y - p[1].y))) > dmax) dmax = (abs(sign_x_to_s32(13, p[2].y - p[1].y)));
 dmax &= 0xFFF;

 tex_base = cmd_data[0x4] << 2;
 {
  const bool gourauden = (bool)(cmd_data[0x2] & 0x4);
  if(cm == 5) /* RGB */
   tex_base &= ~0x7;

  EdgeStepper_Setup(&PrimData.e[0], gourauden, &p[0], &p[3], dmax);
  EdgeStepper_Setup(&PrimData.e[1], gourauden, &p[1], &p[2], dmax);
 }
 PrimData.iter = dmax;
 PrimData.tex_base = tex_base;
 PrimData.need_line_resume = false;

 {
  v_inv = dir & 2;
  tv[0 ^ v_inv] = 0;
  tv[1 ^ v_inv] = h ? (h - 1) : 0;

  /* VileTex_Setup: C++ had default sf=1, tfudge=0; now pass explicitly */
  VileTex_Setup(&PrimData.big_t, dmax + 1, tv[0], tv[1], w >> spr_w_shift_tab[cm], 0);
 }

 return ret;
}

int32_t VDP1_CMD_DistortedSprite(const uint16_t* cmd_data)
{
 return SpriteBase(cmd_data, FORMAT_DISTORTED);
}

int32_t VDP1_CMD_NormalSprite(const uint16_t* cmd_data)
{
 return SpriteBase(cmd_data, FORMAT_NORMAL);
}

int32_t VDP1_CMD_ScaledSprite(const uint16_t* cmd_data)
{
 return SpriteBase(cmd_data, FORMAT_SCALED);
}
