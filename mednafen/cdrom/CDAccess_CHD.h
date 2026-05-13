/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* CDAccess_CHD.h:
**  Copyright (C) 2017 Romain Tisserand
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

#ifndef __MDFN_CDACCESS_CHD_H
#define __MDFN_CDACCESS_CHD_H

#include <stdint.h>
#include <boolean.h>

#include "CDAccess.h"
#include <libchdr/chd.h>
#include <libchdr/cdrom.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CHDFILE_TRACK_INFO
{
   int32_t LBA;

   uint32_t DIFormat;
   uint8_t subq_control;

   int32_t pregap;
   int32_t pregap_dv;

   int32_t postgap;

   int32_t chd_offset;

   int32_t index[100];

   int32_t sectors; /* Not including pregap sectors! */
   bool FirstFileInstance;
   bool RawAudioMSBFirst;
   unsigned int SubchannelMode;

   uint32_t LastSamplePos;
};

typedef struct CHDFILE_TRACK_INFO CHDFILE_TRACK_INFO;

/* Concrete backend for the CDAccess vtable defined in CDAccess.h.
 * Was a C++ class until the Phase-3 / Phase-4 conversion of the CD
 * layer to C; the layout still matches the old class so the
 * vtable cast in CDAccess_CHD_New() is layout-compatible.
 *
 * CDAccess base MUST be the first member - the dispatch path in
 * cdromif casts a CDAccess* back to this type via a cast from the
 * embedded `base` pointer's address. */
struct CDAccess_CHD
{
   CDAccess base;

   int32_t NumTracks;
   int32_t FirstTrack;
   int32_t LastTrack;
   int32_t total_sectors;
   uint8_t disc_type;
   TOC     toc;
   CHDFILE_TRACK_INFO Tracks[CD_MAX_TRACKS + 1];

   int num_sessions;
   int num_tracks;

   chd_file *chd;
   /* hunk data cache */
   uint8_t *hunkmem;
   /* last hunknum read */
   int oldhunk;
};

typedef struct CDAccess_CHD CDAccess_CHD;

CDAccess *CDAccess_CHD_New(const char *path, bool image_memcache);

#ifdef __cplusplus
}
#endif

#endif
