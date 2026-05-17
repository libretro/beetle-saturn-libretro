/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* vdp1_poly.c - VDP1 Polygon Drawing Commands Emulation
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
#include "../math_ops.h"

/* Polygon: AA=1, Textured=0, ECD=0, HalfFGEn=(c&0x2) */
#define VDP1_DL_POLY_GEN(die, bpp8, b, c) \
 VDP1_GEN_WRAPPER(VDP1_DL_NAME(die,bpp8,b,c), \
   /*AA*/1, /*Tex*/0, die, bpp8, \
   (c)==0x8, !!((b)&0x10), ((b)&0x10)&&((b)&0x08), \
   !!((b)&0x04), /*ECD*/0, !!((b)&0x01), \
   !!((c)&0x4), !!((c)&0x2), !!((c)&0x1))

VDP1_EMIT_ALL_WRAPPERS(VDP1_DL_POLY_GEN)
#undef VDP1_DL_POLY_GEN

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

static int32_t PolygonResumeBase(const uint16_t* cmd_data, const int gourauden)
{
 const uint16_t mode = cmd_data[0x2];
 /* Abusing the SPD bit passed to the line draw function to denote non-transparency when == 1, transparent when == 0. */
 const bool SPD_Opaque = (((mode >> 3) & 0x7) < 0x6) ? ((int32_t)(TexFetchTab[(mode >> 3) & 0x1F](0xFFFFFFFF)) >= 0) : true;
 int32_t (*const fnptr)(bool* need_line_resume) = LineFuncTab[(bool)(FBCR & FBCR_DIE)][(TVMR & TVMR_8BPP) ? ((TVMR & TVMR_ROTATE) ? 2 : 1) : 0][((mode >> 6) & 0x1E) | SPD_Opaque /*(mode >> 6) & 0x1F*/][(mode & 0x8000) ? 8 : (mode & 0x7)];
 /*
 ** Don't merge e0 and e1 into a single array, keeping them separate is a workaround for gcc bug #113255
 */
 EdgeStepper e0 = PrimData.e[0];
 EdgeStepper e1 = PrimData.e[1];
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

   if(!VDP1_SetupDrawLine(&ret, true, false, mode) || !iter)
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
 PrimData.iter = iter;

 return ret;
}

int32_t VDP1_RESUME_Polygon(const uint16_t* cmd_data)
{
 if(cmd_data[0x2] & 0x4) /* gouraud */
  return PolygonResumeBase(cmd_data, 1);
 return PolygonResumeBase(cmd_data, 0);
}


static INLINE int32_t CMD_PolygonG_T(const uint16_t* cmd_data, const int gourauden)
{
 line_vertex p[4];
 int32_t ret = 0;
 unsigned i;
 int32_t dmax;
 /**/
 LineData.tex_base = 0;
 LineData.color = cmd_data[0x3];
 /**/
 for(i = 0; i < 4; i++)
 {
  p[i].x = sign_x_to_s32(13, cmd_data[0x6 + (i << 1)]) + LocalX;
  p[i].y = sign_x_to_s32(13, cmd_data[0x7 + (i << 1)]) + LocalY;
 }

 if(gourauden)
 {
  const uint16_t* gtb = &VRAM[cmd_data[0xE] << 2];
  ret += 4;
  for(i = 0; i < 4; i++)
   p[i].g = gtb[i];
 }
 /**/
 dmax =                    abs(sign_x_to_s32(13, p[3].x - p[0].x));
 if((abs(sign_x_to_s32(13, p[3].y - p[0].y))) > dmax) dmax = (abs(sign_x_to_s32(13, p[3].y - p[0].y)));
 if((abs(sign_x_to_s32(13, p[2].x - p[1].x))) > dmax) dmax = (abs(sign_x_to_s32(13, p[2].x - p[1].x)));
 if((abs(sign_x_to_s32(13, p[2].y - p[1].y))) > dmax) dmax = (abs(sign_x_to_s32(13, p[2].y - p[1].y)));
 dmax &= 0xFFF;

 EdgeStepper_Setup(&PrimData.e[0], gourauden, &p[0], &p[3], dmax);
 EdgeStepper_Setup(&PrimData.e[1], gourauden, &p[1], &p[2], dmax);
 PrimData.iter = dmax;
 PrimData.need_line_resume = false;

 return ret;
}

int32_t VDP1_CMD_Polygon(const uint16_t* cmd_data)
{
 if(cmd_data[0x2] & 0x4) /* gouraud */
  return CMD_PolygonG_T(cmd_data, 1);
 return CMD_PolygonG_T(cmd_data, 0);
}
