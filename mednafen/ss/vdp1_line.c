/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* vdp1_line.c - VDP1 Line Drawing Commands Emulation
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

/* Line: AA=0, Textured=0, ECD=0, HalfFGEn=(c&0x2)
   This matches the original DrawLine<false, false, ...> instantiation. */
#define VDP1_DL_LINE_GEN(die, bpp8, b, c) \
 VDP1_GEN_WRAPPER(VDP1_DL_NAME(die,bpp8,b,c), \
   /*AA*/0, /*Tex*/0, die, bpp8, \
   (c)==0x8, !!((b)&0x10), ((b)&0x10)&&((b)&0x08), \
   !!((b)&0x04), /*ECD*/0, !!((b)&0x01), \
   !!((c)&0x4), !!((c)&0x2), !!((c)&0x1))

/* Generate all 1728 wrapper functions */
VDP1_EMIT_ALL_WRAPPERS(VDP1_DL_LINE_GEN)
#undef VDP1_DL_LINE_GEN

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

int32_t VDP1_RESUME_Line(const uint16_t* cmd_data)
{
 const uint16_t mode = cmd_data[0x2];
 /* Abusing the SPD bit passed to the line draw function to denote non-transparency when == 1, transparent when == 0. */
 const bool SPD_Opaque = (((mode >> 3) & 0x7) < 0x6) ? ((int32_t)(TexFetchTab[(mode >> 3) & 0x1F](0xFFFFFFFF)) >= 0) : true;
 int32_t (*const fnptr)(bool* need_line_resume) = LineFuncTab[(bool)(FBCR & FBCR_DIE)][(TVMR & TVMR_8BPP) ? ((TVMR & TVMR_ROTATE) ? 2 : 1) : 0][((mode >> 6) & 0x1E) | SPD_Opaque /*(mode >> 6) & 0x1F*/][(mode & 0x8000) ? 8 : (mode & 0x7)];
 const uint32_t num_lines = (cmd_data[0] & 0x1) ? 4 : 1;
 uint32_t iter = PrimData.iter;
 int32_t ret = 0;

 if(MDFN_UNLIKELY(PrimData.need_line_resume))
 {
  PrimData.need_line_resume = false;
  goto ResumeLine;
 }

 if(iter < num_lines)
 {
  do
  {
   LineData.p[0].x = sign_x_to_s32(13, cmd_data[0x6 + (((iter << 1) + 0) & 0x7)] & 0x1FFF) + LocalX;
   LineData.p[0].y = sign_x_to_s32(13, cmd_data[0x7 + (((iter << 1) + 0) & 0x7)] & 0x1FFF) + LocalY;
   LineData.p[1].x = sign_x_to_s32(13, cmd_data[0x6 + (((iter << 1) + 2) & 0x7)] & 0x1FFF) + LocalX;
   LineData.p[1].y = sign_x_to_s32(13, cmd_data[0x7 + (((iter << 1) + 2) & 0x7)] & 0x1FFF) + LocalY;

   if(mode & 0x4) /* Gouraud */
   {
    const uint16_t* gtb = &VRAM[cmd_data[0xE] << 2];

    ret += 2;
    LineData.p[0].g = gtb[(iter + 0) & 0x3];
    LineData.p[1].g = gtb[(iter + 1) & 0x3];
   }

   VDP1_SetupDrawLine(&ret, false, false, mode);
   /**/
   ResumeLine:;
   ret += VDP1_AdjustDrawTiming(fnptr(&PrimData.need_line_resume));
   if(MDFN_UNLIKELY(PrimData.need_line_resume))
    break;
  } while(++iter < num_lines && ret < VDP1_SuspendResumeThreshold);
 }

 PrimData.iter = iter;

 return ret;
}

int32_t VDP1_CMD_Line(const uint16_t* cmd_data)
{
 /**/
 /**/
 LineData.tex_base         = 0;
 LineData.color            = cmd_data[0x3];
 /**/
 /**/
 PrimData.iter             = 0;
 PrimData.need_line_resume = false;

 return 1;
}
