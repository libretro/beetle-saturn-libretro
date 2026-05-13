/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* CDAccess_CCD.h:
**  Copyright (C) 2013-2016 Mednafen Team
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

#include "../cdstream.h"

#include "CDAccess.h"

class CDAccess_CCD
{
 public:

 CDAccess_CCD(const std::string& path, bool image_memcache);
 ~CDAccess_CCD();

 bool Read_Raw_Sector(uint8 *buf, int32 lba);

 bool Fast_Read_Raw_PW_TSRE(uint8* pwbuf, int32 lba);

 bool Read_TOC(TOC *toc);

 /* CDAccess vtable base.  MUST be the first member - see comment
  * in CDAccess.h. */
 CDAccess base;

 private:

 bool Load(const std::string& path, bool image_memcache);
 void Cleanup(void);

 bool CheckSubQSanity(void);

 cdstream *img_stream;
 uint8_t *sub_data;

 size_t img_numsectors;
 TOC tocd;
};

extern "C" CDAccess *CDAccess_CCD_New(const char *path, bool image_memcache);
