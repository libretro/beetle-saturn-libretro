/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* CDAccess_CHD.cpp:
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <boolean.h>


#include <stdio.h>

#include <libretro.h>

#include "CDAccess_CHD.h"

extern retro_log_printf_t log_cb;

/* Forward declarations - methods reference each other regardless of
 * source order. */
static bool    CDAccess_CHD_Load_internal(CDAccess_CHD *self, const char *path, bool image_memcache);
static int32_t CDAccess_CHD_MakeSubPQ    (const CDAccess_CHD *self, int32_t lba, uint8_t *SubPWBuf);
static void    CDAccess_CHD_cleanup      (CDAccess_CHD *self);

// Disk-image(rip) track/sector formats
enum
{
  DI_FORMAT_AUDIO = 0x00,
  DI_FORMAT_MODE1 = 0x01,
  DI_FORMAT_MODE1_RAW = 0x02,
  DI_FORMAT_MODE2 = 0x03,
  DI_FORMAT_MODE2_FORM1 = 0x04,
  DI_FORMAT_MODE2_FORM2 = 0x05,
  DI_FORMAT_MODE2_RAW = 0x06,
  DI_FORMAT_CDI_RAW = 0x07,
  _DI_FORMAT_COUNT
};

static const int32_t DI_Size_Table[8] =
{
  2352, // Audio
  2048, // MODE1
  2352, // MODE1 RAW
  2336, // MODE2
  2048, // MODE2 Form 1
  2324, // MODE2 Form 2
  2352, // MODE2 RAW
  2352  // CD-I RAW
};

/* Init helper.  Was the C++ ctor; now an explicit function called
 * from CDAccess_CHD_New.  Zeroes the struct's variable state (the
 * fields that the C++ ctor's member-initializer list used to handle)
 * and clears the embedded TOC before Load populates it. */
static bool CDAccess_CHD_init(CDAccess_CHD *self, const char *path, bool image_memcache)
{
  self->NumTracks     = 0;
  self->FirstTrack    = 0;
  self->LastTrack     = 0;
  self->total_sectors = 0;
  self->disc_type     = 0;
  self->chd           = NULL;
  self->hunkmem       = NULL;
  self->oldhunk       = -1;
  self->num_sessions  = 0;
  self->num_tracks    = 0;
  memset(self->Tracks, 0, sizeof(self->Tracks));
  TOC_Clear(&self->toc);
  return CDAccess_CHD_Load_internal(self, path, image_memcache);
}

static bool CDAccess_CHD_Load_internal(CDAccess_CHD *self, const char *path, bool image_memcache)
{
  chd_error err = chd_open(path, CHD_OPEN_READ, NULL, &self->chd);
  if (err != CHDERR_NONE)
  {
    log_cb(RETRO_LOG_ERROR, "Failed to load CHD image: %s", path);
    return false;
  }

  if (image_memcache)
  {
    err = chd_precache(self->chd);

    if (err != CHDERR_NONE)
    {
      log_cb(RETRO_LOG_ERROR, "Failed to pre-cache CHD image: %s", path);
      return false;
    }
  }

  /* allocate storage for sector reads */
  const chd_header *head = chd_get_header(self->chd);
  self->hunkmem = (uint8_t *)malloc(head->hunkbytes);
  self->oldhunk = -1;

  int plba = -150;
  int numsectors = 0;
  int chd_offset = 0;
  while (1)
  {
    int tkid = 0, frames = 0, pad = 0, pregap = 0, postgap = 0;
    char type[64], subtype[32], pgtype[32], pgsub[32];
    char tmp[512];

    err = chd_get_metadata(self->chd, CDROM_TRACK_METADATA2_TAG, self->NumTracks, tmp, sizeof(tmp), NULL, NULL, NULL);
    if (err == CHDERR_NONE)
    {
      sscanf(tmp, CDROM_TRACK_METADATA2_FORMAT, &tkid, type, subtype, &frames, &pregap, pgtype, pgsub, &postgap);
    }
    else
    {
      /* try to read the old v3/v4 metadata tag */
      err = chd_get_metadata(self->chd, CDROM_TRACK_METADATA_TAG,
                             self->NumTracks, tmp, sizeof(tmp), NULL, NULL,
                             NULL);
      if (err == CHDERR_NONE)
      {
        sscanf(tmp, CDROM_TRACK_METADATA_FORMAT, &tkid, type, subtype,
               &frames);
      }
      else
      {
        /* if there's no valid metadata, this is the end of the TOC */
        break;
      }
    }

    if (strcmp(type, "MODE1") && strcmp(type, "MODE1_RAW") && strcmp(type, "MODE2_RAW") &&
        strcmp(type, "AUDIO"))
    {
      log_cb(RETRO_LOG_ERROR, "chd_parse track type %s unsupported\n", type);
      return 0;
    }

    if (strcmp(subtype, "NONE"))
    {
      log_cb(RETRO_LOG_ERROR, "chd_parse track subtype %s unsupported\n", subtype);
      return 0;
    }

    /* add track */
    self->NumTracks++;
    self->toc.tracks[self->NumTracks].adr = 1;
    self->toc.tracks[self->NumTracks].control = strcmp(type, "AUDIO") == 0 ? 0 : 4;
    self->toc.tracks[self->NumTracks].valid = true;

    self->Tracks[self->NumTracks].pregap = (self->NumTracks == 1) ? 150 : 0;
    self->Tracks[self->NumTracks].pregap_dv = pregap;
    plba += self->Tracks[self->NumTracks].pregap + self->Tracks[self->NumTracks].pregap_dv;
    self->Tracks[self->NumTracks].LBA = self->toc.tracks[self->NumTracks].lba = plba;
    self->Tracks[self->NumTracks].postgap = postgap;
    self->Tracks[self->NumTracks].chd_offset = chd_offset;
    self->Tracks[self->NumTracks].sectors = frames - self->Tracks[self->NumTracks].pregap_dv;
    self->Tracks[self->NumTracks].SubchannelMode = 0;
    self->Tracks[self->NumTracks].index[0] = -1;
    self->Tracks[self->NumTracks].index[1] = 0;
    for (int32_t i = 2; i < 100; i++)
      self->Tracks[self->NumTracks].index[i] = -1;

    self->toc.tracks[self->NumTracks].lba = plba;

    if (strcmp(type, "AUDIO") == 0)
    {
      self->Tracks[self->NumTracks].DIFormat = DI_FORMAT_AUDIO;
      self->Tracks[self->NumTracks].RawAudioMSBFirst = 1;
    }
    else if (strcmp(type, "MODE1_RAW") == 0)
      self->Tracks[self->NumTracks].DIFormat = DI_FORMAT_MODE1_RAW;
    else if (strcmp(type, "MODE2_RAW") == 0)
      self->Tracks[self->NumTracks].DIFormat = DI_FORMAT_MODE2_RAW;
    else if (strcmp(type, "MODE1") == 0)
      self->Tracks[self->NumTracks].DIFormat = DI_FORMAT_MODE1;

    self->Tracks[self->NumTracks].subq_control = strcmp(type, "AUDIO") == 0 ? 0 : 4;

    //log_cb(RETRO_LOG_INFO, "chd_parse '%s' track=%d lba=%d, pregap=%d pregap_dv=%d postgap=%d sectors=%d\n", tmp, self->NumTracks, self->Tracks[self->NumTracks].LBA, self->Tracks[self->NumTracks].pregap, self->Tracks[self->NumTracks].pregap_dv, self->Tracks[self->NumTracks].postgap, self->Tracks[self->NumTracks].sectors);

    plba += frames - self->Tracks[self->NumTracks].pregap_dv;
    plba += self->Tracks[self->NumTracks].postgap;

    // tracks are padded to a 4-frame boundary in chds, calculate the
    // next track's offset to generate correct block addresses
    if (frames % CD_TRACK_PADDING > 0)
      chd_offset += (frames + (CD_TRACK_PADDING - frames % CD_TRACK_PADDING)) - frames;

    numsectors += frames;
    self->toc.first_track = 1;
    self->toc.last_track = self->NumTracks;
  }

  self->FirstTrack = 1;
  self->LastTrack = self->NumTracks;
  self->total_sectors = numsectors;
  //log_cb(RETRO_LOG_INFO, "self->chd self->total_sectors '%d'\n", self->total_sectors);

  /* add track */
  self->toc.tracks[100].adr = 1;
  self->toc.tracks[100].control = 0;
  self->toc.tracks[100].lba = numsectors; // HACK
  self->toc.tracks[100].valid = true;

  //
  // Adjust indexes for MakeSubPQ()
  //
  for (int x = self->FirstTrack; x < (self->FirstTrack + self->NumTracks); x++)
  {
    const int32_t base = self->Tracks[x].index[1];
    for (int32_t i = 0; i < 100; i++)
    {
      if (i == 0 || self->Tracks[x].index[i] == -1)
        self->Tracks[x].index[i] = INT32_MAX;
      else
        self->Tracks[x].index[i] = self->Tracks[x].LBA + (self->Tracks[x].index[i] - base);

      assert(self->Tracks[x].index[i] >= 0);
    }
  }

  return true;
}

static void CDAccess_CHD_cleanup(CDAccess_CHD *self)
{
  if (self->chd != NULL)
    chd_close(self->chd);

  if (self->hunkmem)
    free(self->hunkmem);
}

static bool CDAccess_CHD_Read_Raw_Sector(CDAccess_CHD *self, uint8_t *buf, int32_t lba)
{
  uint8_t SimuQ[0xC];
  int32_t track;
  CHDFILE_TRACK_INFO *ct;

  //
  // Leadout synthesis
  //
  if (lba >= self->total_sectors)
  {
    uint8_t data_synth_mode = 0x01; // Default for DISC_TYPE_CDDA_OR_M1, would be 0x02 for DISC_TYPE_CD_XA

    switch (self->Tracks[self->LastTrack].DIFormat)
    {
    case DI_FORMAT_AUDIO:
      break;

    case DI_FORMAT_MODE1_RAW:
    case DI_FORMAT_MODE1:
      data_synth_mode = 0x01;
      break;

    case DI_FORMAT_MODE2_RAW:
    case DI_FORMAT_MODE2_FORM1:
    case DI_FORMAT_MODE2_FORM2:
    case DI_FORMAT_MODE2:
    case DI_FORMAT_CDI_RAW:
      data_synth_mode = 0x02;
      break;
    }

    synth_leadout_sector_lba(data_synth_mode, &self->toc, lba, buf);
    return true;
  }

  memset(buf + 2352, 0, 96);
  track = CDAccess_CHD_MakeSubPQ(self, lba, buf + 2352);
  /* MakeSubPQ used to throw on track-not-found; now it returns -1.
   * Treat as a synthesised-failure -> zeroed sector + false return so
   * callers fall back to their error path the same way the C++
   * exception used to. */
  if (track < 0)
    return false;
  subq_deinterleave(buf + 2352, SimuQ);

  ct = &self->Tracks[track];

  //
  // Handle pregap and postgap reading
  //
  if (lba < (ct->LBA - ct->pregap_dv) || lba >= (ct->LBA + ct->sectors))
  {
    int32_t pg_offset = lba - ct->LBA;
    CHDFILE_TRACK_INFO *et = ct;

    if (pg_offset < -150)
    {
      if ((self->Tracks[track].subq_control & SUBQ_CTRLF_DATA) && (self->FirstTrack < track) && !(self->Tracks[track - 1].subq_control & SUBQ_CTRLF_DATA))
        et = &self->Tracks[track - 1];
    }

    memset(buf, 0, 2352);
    switch (et->DIFormat)
    {
    case DI_FORMAT_AUDIO:
      break;

    case DI_FORMAT_MODE1_RAW:
    case DI_FORMAT_MODE1:
      encode_mode1_sector(lba + 150, buf);
      break;

    case DI_FORMAT_MODE2_RAW:
    case DI_FORMAT_MODE2_FORM1:
    case DI_FORMAT_MODE2_FORM2:
    case DI_FORMAT_MODE2:
    case DI_FORMAT_CDI_RAW:
      buf[12 + 6] = 0x20;
      buf[12 + 10] = 0x20;
      encode_mode2_form2_sector(lba + 150, buf);
      // TODO: Zero out optional(?) checksum bytes?
      break;
    }
  }
  else
  {
    const chd_header *head = chd_get_header(self->chd);
    int cad                = lba + ct->chd_offset;
    int hunkid             = (cad * CD_FRAME_SIZE) / head->hunkbytes;
    int hunkofs            = (cad * CD_FRAME_SIZE) % head->hunkbytes;
    int err                = CHDERR_NONE;

    /* each hunk holds ~8 sectors, optimize when reading contiguous sectors */
    if (hunkid != self->oldhunk)
    {
      err = chd_read(self->chd, hunkid, self->hunkmem);
      if (err == CHDERR_NONE)
        self->oldhunk = hunkid;
    }

    if (ct->DIFormat == DI_FORMAT_MODE1 || ct->DIFormat == DI_FORMAT_MODE2) {
        memcpy(buf + 16, self->hunkmem + hunkofs, DI_Size_Table[ct->DIFormat]);
    } else {
        memcpy(buf, self->hunkmem + hunkofs, DI_Size_Table[ct->DIFormat]);
    }

    switch(ct->DIFormat)
    {
      case DI_FORMAT_AUDIO:
        if (ct->RawAudioMSBFirst)
        {
          /* Endian_A16_Swap folded: unconditional byte-pair swap. */
          uint32_t k;
          for (k = 0; k < 588 * 2; k++)
          {
            uint8_t t = buf[k * 2];
            buf[k * 2]     = buf[k * 2 + 1];
            buf[k * 2 + 1] = t;
          }
        }
        break;

      case DI_FORMAT_MODE1:
        encode_mode1_sector(lba + 150, buf);
        break;

      case DI_FORMAT_MODE2:
        encode_mode2_sector(lba + 150, buf);
        break;
    }

  } // end if audible part of audio track read.

  return true;
}

//
// Note: this function makes use of the current contents(as in |=) in SubPWBuf.
//
static int32_t CDAccess_CHD_MakeSubPQ(const CDAccess_CHD *self, int32_t lba, uint8_t *SubPWBuf)
{
  uint8_t buf[0xC];
  int32_t track;
  uint32_t lba_relative;
  uint32_t ma, sa, fa;
  uint32_t m, s, f;
  uint8_t pause_or = 0x00;
  bool track_found = false;

  for (track = self->FirstTrack; track < (self->FirstTrack + self->NumTracks); track++)
  {
    if (lba >= (self->Tracks[track].LBA - self->Tracks[track].pregap_dv - self->Tracks[track].pregap) && lba < (self->Tracks[track].LBA + self->Tracks[track].sectors + self->Tracks[track].postgap))
    {
      track_found = true;
      break;
    }
  }

  if (!track_found)
    return -1;

  if (lba < self->Tracks[track].LBA)
    lba_relative = self->Tracks[track].LBA - 1 - lba;
  else
    lba_relative = lba - self->Tracks[track].LBA;

  f = (lba_relative % 75);
  s = ((lba_relative / 75) % 60);
  m = (lba_relative / 75 / 60);

  fa = (lba + 150) % 75;
  sa = ((lba + 150) / 75) % 60;
  ma = ((lba + 150) / 75 / 60);

  uint8_t adr = 0x1; // Q channel data encodes position
  uint8_t control = self->Tracks[track].subq_control;

  // Handle pause(D7 of interleaved subchannel byte) bit, should be set to 1 when in pregap or postgap.
  if ((lba < self->Tracks[track].LBA) || (lba >= self->Tracks[track].LBA + self->Tracks[track].sectors))
    pause_or = 0x80;

  // Handle pregap between audio->data track
  {
    int32_t pg_offset = (int32_t)lba - self->Tracks[track].LBA;

    // If we're more than 2 seconds(150 sectors) from the real "start" of the track/INDEX 01, and the track is a data track,
    // and the preceding track is an audio track, encode it as audio(by taking the SubQ control field from the preceding track).
    //
    // TODO: Look into how we're supposed to handle subq control field in the four combinations of track types(data/audio).
    //
    if (pg_offset < -150)
    {
      if ((self->Tracks[track].subq_control & SUBQ_CTRLF_DATA) && (self->FirstTrack < track) && !(self->Tracks[track - 1].subq_control & SUBQ_CTRLF_DATA))
        control = self->Tracks[track - 1].subq_control;
    }
  }

  memset(buf, 0, 0xC);
  buf[0] = (adr << 0) | (control << 4);
  buf[1] = U8_to_BCD(track);

  // Index
  //if(lba < self->Tracks[track].LBA) // Index is 00 in pregap
  // buf[2] = U8_to_BCD(0x00);
  //else
  // buf[2] = U8_to_BCD(0x01);
  {
    int index = 0;

    for (int32_t i = 0; i < 100; i++)
    {
      if (lba >= self->Tracks[track].index[i])
        index = i;
    }
    buf[2] = U8_to_BCD(index);
  }

  // Track relative MSF address
  buf[3] = U8_to_BCD(m);
  buf[4] = U8_to_BCD(s);
  buf[5] = U8_to_BCD(f);

  buf[6] = 0; // Zerroooo

  // Absolute MSF address
  buf[7] = U8_to_BCD(ma);
  buf[8] = U8_to_BCD(sa);
  buf[9] = U8_to_BCD(fa);

  subq_generate_checksum(buf);

  for (int i = 0; i < 96; i++)
    SubPWBuf[i] |= (((buf[i >> 3] >> (7 - (i & 0x7))) & 1) ? 0x40 : 0x00) | pause_or;

  return track;
}

static bool CDAccess_CHD_Fast_Read_Raw_PW_TSRE(CDAccess_CHD *self, uint8_t *pwbuf, int32_t lba)
{
  int32_t track;

  if (lba >= self->total_sectors)
  {
    subpw_synth_leadout_lba(&self->toc, lba, pwbuf);
    return (true);
  }

  memset(pwbuf, 0, 96);
  track = CDAccess_CHD_MakeSubPQ(self, lba, pwbuf);
  if (track < 0)
    return false;
  //
  // If TOC+BIN has embedded subchannel data, we can't fast-read(synthesize) it...
  //
  if (self->Tracks[track].SubchannelMode && lba >= (self->Tracks[track].LBA - self->Tracks[track].pregap_dv) && (lba < self->Tracks[track].LBA + self->Tracks[track].sectors))
    return (false);

  return (true);
}

static bool CDAccess_CHD_Read_TOC(CDAccess_CHD *self, TOC *out_toc)
{
  *out_toc = self->toc;
  return true;
}

/* ---------------------------------------------------------------- */
/* CDAccess vtable adapters.  Each forwards to the static C method  */
/* of the same name above; the wrapper exists to bridge the         */
/* (CDAccess*,...) function-pointer shape declared in CDAccess.h    */
/* to the (CDAccess_CHD*,...) static-method shape.                  */
/* ---------------------------------------------------------------- */

static bool CDAccess_CHD_RRS_vt(CDAccess *base, uint8_t *buf, int32_t lba)
{
   return CDAccess_CHD_Read_Raw_Sector((CDAccess_CHD *)base, buf, lba);
}

static bool CDAccess_CHD_FRPT_vt(CDAccess *base, uint8_t *pwbuf, int32_t lba)
{
   return CDAccess_CHD_Fast_Read_Raw_PW_TSRE((CDAccess_CHD *)base, pwbuf, lba);
}

static bool CDAccess_CHD_RTOC_vt(CDAccess *base, TOC *out_toc)
{
   return CDAccess_CHD_Read_TOC((CDAccess_CHD *)base, out_toc);
}

static void CDAccess_CHD_destroy_vt(CDAccess *base)
{
   CDAccess_CHD *self = (CDAccess_CHD *)base;
   CDAccess_CHD_cleanup(self);
   free(self);
}

CDAccess *CDAccess_CHD_New(const char *path, bool image_memcache)
{
   CDAccess_CHD *self = (CDAccess_CHD *)calloc(1, sizeof(*self));
   if (!self)
      return NULL;

   if (!CDAccess_CHD_init(self, path, image_memcache))
   {
      /* init's Load may have partially populated chd / hunkmem
       * before failing - run cleanup so we don't leak. */
      CDAccess_CHD_cleanup(self);
      free(self);
      return NULL;
   }

   self->base.Read_Raw_Sector       = CDAccess_CHD_RRS_vt;
   self->base.Fast_Read_Raw_PW_TSRE = CDAccess_CHD_FRPT_vt;
   self->base.Read_TOC              = CDAccess_CHD_RTOC_vt;
   self->base.destroy               = CDAccess_CHD_destroy_vt;
   return &self->base;
}
