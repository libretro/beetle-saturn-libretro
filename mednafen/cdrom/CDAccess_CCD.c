/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* CDAccess_CCD.c:
**  Copyright (C) 2013-2016 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <boolean.h>

#include <streams/file_stream.h>

#include <libretro.h>

#include "CDAccess_CCD.h"

extern retro_log_printf_t log_cb;

/* ----------------------------------------------------------------
 *
 * In-memory parsed-CCD representation.
 *
 * The CCD descriptor is an INI-like text file: sections in [name]
 * headers, each with key=value lines.  Used to be parsed into
 * std::map<std::string, std::map<std::string, std::string>>; now
 * uses two parallel fixed-size arrays.  Parsing is one-shot at
 * disc-load time, so the linear-scan section/property lookups are
 * fine performance-wise (max ~120 sections for a 99-track disc
 * plus DISC + Session 1 + a few entries).
 *
 * Sizing rationale:
 *  - CCD_MAX_SECTIONS: a 99-track disc has 99 ENTRY sections plus
 *    DISC, Session 1, and lead-in/lead-out (TOC entries 0xA0-0xA2),
 *    so ~103 in the worst legal case.  Cap at 256 for slack.
 *  - CCD_MAX_PROPS:  each section has at most ~10 properties
 *    (POINT, ADR, CONTROL, TRACKNO, AMIN, ASEC, AFRAME, ALBA, ZERO,
 *    PMIN, PSEC, PFRAME, PLBA - 13 in the worst-case ENTRY).
 *  - Name and value buffers are 64 bytes each, plenty for any real
 *    CCD content.
 * ---------------------------------------------------------------- */

#define CCD_MAX_SECTIONS  256
#define CCD_MAX_PROPS      16
#define CCD_NAME_MAX       64
#define CCD_VAL_MAX        64

struct CCD_Property
{
   char key[CCD_NAME_MAX];
   char val[CCD_VAL_MAX];
};

struct CCD_Section
{
   char  name[CCD_NAME_MAX];
   struct CCD_Property props[CCD_MAX_PROPS];
   unsigned n_props;
};

struct CCD_File
{
   struct CCD_Section sections[CCD_MAX_SECTIONS];
   unsigned n_sections;
};

/* In-place ASCII uppercase.  Replaces MDFN_strtoupper(std::string&)
 * from the C++ version - which itself was just a wrapper around
 * std::transform with ::toupper. */
static void str_to_upper(char *s)
{
   for (; *s; s++)
      *s = (char)toupper((unsigned char)*s);
}

/* Bounded copy with explicit truncation + null termination.  Used
 * instead of strncpy() to silence -Wstringop-truncation warnings
 * gcc raises around strncpy(dst, src, N-1) + dst[N-1]='\0'
 * idioms. */
static void copy_str(char *dst, size_t dst_sz, const char *src)
{
   size_t n = strlen(src);
   if (n >= dst_sz)
      n = dst_sz - 1;
   memcpy(dst, src, n);
   dst[n] = '\0';
}

/* Trim leading/trailing ASCII whitespace in place.  Replaces the
 * MDFN_ltrim / MDFN_rtrim pair the C++ parser called separately. */
static void str_trim(char *s)
{
   char *start = s;
   char *end;
   size_t len;

   while (*start && isspace((unsigned char)*start))
      start++;

   if (start != s)
      memmove(s, start, strlen(start) + 1);

   len = strlen(s);
   if (len == 0)
      return;

   end = s + len - 1;
   while (end >= s && isspace((unsigned char)*end))
   {
      *end = '\0';
      end--;
   }
}

/* Return pointer to the named section, or NULL.  Insert-or-find
 * semantics; pass create_if_missing=true to match the std::map's
 * operator[] upsert pattern, false to mimic std::map::find. */
static struct CCD_Section *ccd_section_lookup(struct CCD_File *f, const char *name, bool create_if_missing)
{
   unsigned i;
   for (i = 0; i < f->n_sections; i++)
   {
      if (strcmp(f->sections[i].name, name) == 0)
         return &f->sections[i];
   }
   if (!create_if_missing || f->n_sections >= CCD_MAX_SECTIONS)
      return NULL;

   struct CCD_Section *s = &f->sections[f->n_sections++];
   copy_str(s->name, CCD_NAME_MAX, name);

   s->n_props = 0;
   return s;
}

/* Upsert key=value in a section. */
static void ccd_section_set(struct CCD_Section *s, const char *key, const char *val)
{
   unsigned i;
   for (i = 0; i < s->n_props; i++)
   {
      if (strcmp(s->props[i].key, key) == 0)
      {
         copy_str(s->props[i].val, CCD_VAL_MAX, val);

         return;
      }
   }
   if (s->n_props >= CCD_MAX_PROPS)
      return;

   copy_str(s->props[s->n_props].key, CCD_NAME_MAX, key);

   copy_str(s->props[s->n_props].val, CCD_VAL_MAX, val);

   s->n_props++;
}

static const char *ccd_section_get(const struct CCD_Section *s, const char *key)
{
   unsigned i;
   for (i = 0; i < s->n_props; i++)
   {
      if (strcmp(s->props[i].key, key) == 0)
         return s->props[i].val;
   }
   return NULL;
}

/* ----------------------------------------------------------------
 * Property->integer helpers.  The C++ version had one template
 * function CCD_ReadInt<T> that picked strtoul vs strtol via
 * std::numeric_limits<T>::is_signed.  In C we get two flavours
 * (signed long / unsigned long) plus a result flag - callers cast
 * down to uint8_t / unsigned / signed at the call site.  All
 * call sites in Load already do the cast.
 *
 * The C++ throw-on-missing-or-malformed is replaced with a return
 * flag; Load aborts (return false) on any failure. */

static bool ccd_read_uint(const struct CCD_Section *s, const char *key, unsigned long *out)
{
   const char *v = ccd_section_get(s, key);
   char       *ep;
   int         base   = 10;
   size_t      offset = 0;

   if (!v)
      return false;

   if (v[0] == '0' && v[1] == 'x')
   {
      base   = 16;
      offset = 2;
   }

   *out = strtoul(v + offset, &ep, base);
   /* Whole string must have parsed as a number (no trailing junk). */
   return v[offset] != '\0' && *ep == '\0';
}

/* ----------------------------------------------------------------
 * Path helpers - C versions of MDFN_GetFilePathComponents and
 * MDFN_EvalFIP from general.cpp.  Inline because pulling C++ utilities
 * across the language boundary is awkward and the logic is short. */

#ifdef _WIN32
# define PATH_SEP '\\'
#else
# define PATH_SEP '/'
#endif

static const char *find_last_path_sep(const char *path)
{
   const char *p = strrchr(path, '/');
#ifdef _WIN32
   const char *q = strrchr(path, '\\');
   if (q && (!p || q > p))
      p = q;
#endif
   return p;
}

static void split_path(const char *path, char *dir_out, size_t dir_sz,
      char *base_out, size_t base_sz)
{
   const char *sep = find_last_path_sep(path);
   const char *file;
   const char *dot;
   size_t      base_len;

   if (!sep)
   {
      copy_str(dir_out, dir_sz, ".");
      file = path;
   }
   else
   {
      size_t dlen = (size_t)(sep - path);
      if (dlen >= dir_sz)
         dlen = dir_sz - 1;
      memcpy(dir_out, path, dlen);
      dir_out[dlen] = '\0';
      file = sep + 1;
   }

   dot = strrchr(file, '.');
   base_len = dot ? (size_t)(dot - file) : strlen(file);
   if (base_len >= base_sz)
      base_len = base_sz - 1;
   memcpy(base_out, file, base_len);
   base_out[base_len] = '\0';
}

/* Build dir + sep + name into out_buf.  Equivalent to MDFN_EvalFIP
 * with skip_safety_check=true (the only mode CDAccess_CCD ever used). */
static void join_path(const char *dir, const char *name, char *out_buf, size_t out_sz)
{
   snprintf(out_buf, out_sz, "%s%c%s", dir, PATH_SEP, name);
}

/* ----------------------------------------------------------------
 * The CCD descriptor file extension matters for finding the
 * matching .img / .sub on case-sensitive filesystems: a .CCD
 * descriptor expects .IMG / .SUB beside it, .Ccd expects .Img /
 * .Sub, and so on.  Derive the expected img/sub extension casing
 * from the descriptor's extension.  Direct port of the loop the
 * old C++ Load() ran. */
static void derive_companion_ext(const char *ccd_path, char img_ext[4], char sub_ext[4])
{
   const char *dot = strrchr(ccd_path, '.');
   signed char extupt[3] = { -1, -1, -1 };
   signed char av        = -1;
   int         i;

   strcpy(img_ext, "img");
   strcpy(sub_ext, "sub");

   /* Need a 4-char extension including the dot. */
   if (!dot || strlen(dot) != 4)
      return;

   for (i = 1; i < 4; i++)
   {
      char c = dot[i];
      if (c >= 'A' && c <= 'Z')
         extupt[i - 1] = 'A' - 'a';
      else if (c >= 'a' && c <= 'z')
         extupt[i - 1] = 0;
   }

   for (i = 0; i < 3; i++)
   {
      if (extupt[i] != -1)
         av = extupt[i];
      else
         extupt[i] = av;
   }
   if (av == -1)
      av = 0;
   for (i = 0; i < 3; i++)
   {
      if (extupt[i] == -1)
         extupt[i] = av;
   }

   for (i = 0; i < 3; i++)
   {
      img_ext[i] = (char)(img_ext[i] + extupt[i]);
      sub_ext[i] = (char)(sub_ext[i] + extupt[i]);
   }
}

/* ----------------------------------------------------------------
 * Read the descriptor text into the CCD_File structure.
 * One-pass line scanner; sections are inserted lazily via
 * ccd_section_lookup(..., create_if_missing=true). */
static bool ccd_parse(cdstream *cf, struct CCD_File *out)
{
   char line[256];
   char cur_section[CCD_NAME_MAX] = "";

   out->n_sections = 0;

   while (cdstream_get_line(cf, line, sizeof(line)) >= 0)
   {
      char *eq;
      char *k, *v;

      str_trim(line);

      if (line[0] == '\0')
         continue;

      if (line[0] == '[')
      {
         size_t len = strlen(line);
         if (len < 3 || line[len - 1] != ']')
            return false;

         /* Drop brackets in place and uppercase. */
         line[len - 1] = '\0';
         copy_str(cur_section, CCD_NAME_MAX, line + 1);

         str_to_upper(cur_section);
         continue;
      }

      eq = strchr(line, '=');
      /* Reject lines with no '=' OR more than one '=' (the C++
       * parser used feqpos == leqpos to enforce exactly-one). */
      if (!eq || strchr(eq + 1, '=') != NULL)
         return false;

      *eq = '\0';
      k   = line;
      v   = eq + 1;
      str_trim(k);
      str_trim(v);
      str_to_upper(k);

      struct CCD_Section *s = ccd_section_lookup(out, cur_section, true);
      if (!s)
         return false;
      ccd_section_set(s, k, v);
   }

   return true;
}

/* ----------------------------------------------------------------
 * CDAccess_CCD vtable implementation. */

static void CDAccess_CCD_cleanup(CDAccess_CCD *self)
{
   if (self->img_stream)
   {
      cdstream_destroy(self->img_stream);
      self->img_stream = NULL;
   }
   if (self->sub_data)
   {
      free(self->sub_data);
      self->sub_data = NULL;
   }
}

/* Checks for Q subchannel mode 1 (current-time) data that has a
 * valid checksum but is nonetheless nonsensical.  Some bad rips
 * floating around the Internet hit this.  Allowing those rips
 * through causes problems during emulation, so we sanity-check
 * here.  This check is not exhaustive - it won't catch every kind
 * of broken rip and should not be used as a "rip repair" tool.
 *
 * The check pattern, the constants, and the threshold ratios are
 * preserved exactly from the C++ implementation. */
static bool CDAccess_CCD_CheckSubQSanity(CDAccess_CCD *self)
{
   size_t      checksum_pass_counter = 0;
   int         prev_lba              = INT32_MAX;
   uint8_t     prev_track            = 0;
   size_t      s;

   for (s = 0; s < self->img_numsectors; s++)
   {
      union
      {
         uint8_t full[96];
         struct
         {
            uint8_t pbuf[12];
            uint8_t qbuf[12];
         };
      } buf;

      memcpy(buf.full, &self->sub_data[s * 96], 96);

      if (subq_check_checksum(buf.qbuf))
      {
         uint8_t adr = buf.qbuf[0] & 0xF;

         if (adr == 0x01)
         {
            uint8_t track_bcd  = buf.qbuf[1];
            uint8_t index_bcd  = buf.qbuf[2];
            uint8_t rm_bcd     = buf.qbuf[3];
            uint8_t rs_bcd     = buf.qbuf[4];
            uint8_t rf_bcd     = buf.qbuf[5];
            uint8_t am_bcd     = buf.qbuf[7];
            uint8_t as_bcd     = buf.qbuf[8];
            uint8_t af_bcd     = buf.qbuf[9];

            /* Reject obviously bad BCD digits. */
            if (!BCD_is_valid(track_bcd) || !BCD_is_valid(index_bcd)
                  || !BCD_is_valid(rm_bcd) || !BCD_is_valid(rs_bcd)
                  || !BCD_is_valid(rf_bcd) || !BCD_is_valid(am_bcd)
                  || !BCD_is_valid(as_bcd) || !BCD_is_valid(af_bcd)
                  /* Track 00 lead-in subQ doesn't show up in CCD rips this far in. */
                  || track_bcd == 0x00
                  /* Seconds and frames have hard limits. */
                  || rs_bcd >= 0x60 || rf_bcd >= 0x75
                  || as_bcd >= 0x60 || af_bcd >= 0x75
                  /* AMSF must match the sector LBA being read. */
                  || AMSF_to_LBA(BCD_to_U8(am_bcd), BCD_to_U8(as_bcd), BCD_to_U8(af_bcd)) != (int32_t)s)
            {
               log_cb(RETRO_LOG_ERROR, "Garbage subQ data detected.\n");
               return false;
            }

            checksum_pass_counter++;
         }
      }
   }

   /* If less than 0.5% of sectors had a checksum-valid subQ
    * mode-1 entry, declare the rip too damaged to trust. */
   if (self->img_numsectors >= 90)
   {
      if (checksum_pass_counter < (self->img_numsectors + 199) / 200)
      {
         log_cb(RETRO_LOG_ERROR, "Garbage subQ data detected; not enough passing entries.\n");
         return false;
      }
   }

   (void)prev_lba;
   (void)prev_track;
   return true;
}

static bool CDAccess_CCD_Load(CDAccess_CCD *self, const char *path, bool image_memcache)
{
   struct CCD_File   ccd;
   struct CCD_Section *ds;
   cdstream          cf;
   unsigned long     toc_entries;
   unsigned long     num_sessions;
   unsigned long     data_tracks_scrambled;
   unsigned          te;
   char              dir_path[1024];
   char              file_base[256];
   char              img_ext[4];
   char              sub_ext[4];
   char              tmp_path[2048];
   char              tmp_name[260];
   RFILE            *sub_stream;
   uint64_t          ss;

   /* CCD descriptor is tiny; memcache-open the whole thing so the
    * line parser runs in-memory. */
   if (!cdstream_open_memcached(&cf, path))
      return false;

   if (!ccd_parse(&cf, &ccd))
   {
      cdstream_close(&cf);
      return false;
   }
   cdstream_close(&cf);

   /* Pull required keys out of the [DISC] section.  All are
    * mandatory; missing/malformed any one of them aborts. */
   ds = ccd_section_lookup(&ccd, "DISC", false);
   if (!ds)
      return false;
   if (!ccd_read_uint(ds, "TOCENTRIES", &toc_entries))
      return false;
   if (!ccd_read_uint(ds, "SESSIONS", &num_sessions))
      return false;
   if (!ccd_read_uint(ds, "DATATRACKSSCRAMBLED", &data_tracks_scrambled))
      return false;

   /* Multi-session not supported; scrambled data tracks not
    * supported (they need an unscramble pass we don't do). */
   if (num_sessions != 1 || data_tracks_scrambled != 0)
      return false;

   for (te = 0; te < toc_entries; te++)
   {
      char                tmpbuf[64];
      struct CCD_Section *ts;
      unsigned long       session, point, adr, control, pmin, psec;
      long                plba_signed;
      unsigned long       plba_u;

      snprintf(tmpbuf, sizeof(tmpbuf), "ENTRY %u", te);
      ts = ccd_section_lookup(&ccd, tmpbuf, false);
      if (!ts)
         return false;

      if (!ccd_read_uint(ts, "SESSION", &session)
            || !ccd_read_uint(ts, "POINT",   &point)
            || !ccd_read_uint(ts, "ADR",     &adr)
            || !ccd_read_uint(ts, "CONTROL", &control)
            || !ccd_read_uint(ts, "PMIN",    &pmin)
            || !ccd_read_uint(ts, "PSEC",    &psec))
         return false;

      /* PLBA is signed in the C++ template instantiation - so we
       * parse it as unsigned and cast.  CCD files store negative
       * LBA as the two's-complement uint32_t representation. */
      if (!ccd_read_uint(ts, "PLBA", &plba_u))
         return false;
      plba_signed = (long)(int32_t)plba_u;

      if (session != 1)
         return false;

      /* Reference: ECMA-394, page 5-14. */
      if (point >= 1 && point <= 99)
      {
         self->tocd.tracks[point].adr     = (uint8_t)adr;
         self->tocd.tracks[point].control = (uint8_t)control;
         self->tocd.tracks[point].lba     = (int32_t)plba_signed;
         self->tocd.tracks[point].valid   = true;
      }
      else switch (point)
      {
      default:
         return false;
      case 0xA0:
         self->tocd.first_track = (uint8_t)pmin;
         self->tocd.disc_type   = (uint8_t)psec;
         break;
      case 0xA1:
         self->tocd.last_track  = (uint8_t)pmin;
         break;
      case 0xA2:
         self->tocd.tracks[100].adr     = (uint8_t)adr;
         self->tocd.tracks[100].control = (uint8_t)control;
         self->tocd.tracks[100].lba     = (int32_t)plba_signed;
         self->tocd.tracks[100].valid   = true;
         break;
      }
   }

   /* Derive companion .img and .sub names from the descriptor path. */
   split_path(path, dir_path, sizeof(dir_path), file_base, sizeof(file_base));
   derive_companion_ext(path, img_ext, sub_ext);

   /* Open image stream (raw 2352-byte sectors). */
   snprintf(tmp_name, sizeof(tmp_name), "%s.%s", file_base, img_ext);
   join_path(dir_path, tmp_name, tmp_path, sizeof(tmp_path));

   if (image_memcache)
      self->img_stream = cdstream_new_memcached(tmp_path);
   else
      self->img_stream = cdstream_new(tmp_path);

   if (!self->img_stream)
      return false;

   ss = cdstream_size(self->img_stream);
   if (ss % 2352)
      return false;
   if (ss > 0x7FFFFFFF)
      return false;

   self->img_numsectors = (size_t)(ss / 2352);

   /* Open subchannel stream (96 raw bytes per sector). */
   snprintf(tmp_name, sizeof(tmp_name), "%s.%s", file_base, sub_ext);
   join_path(dir_path, tmp_name, tmp_path, sizeof(tmp_path));

   sub_stream = filestream_open(tmp_path,
         RETRO_VFS_FILE_ACCESS_READ,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);

   if (!sub_stream
         || filestream_get_size(sub_stream) != (int64_t)((uint64_t)self->img_numsectors * 96))
   {
      if (sub_stream)
         filestream_close(sub_stream);
      return false;
   }

   self->sub_data = (uint8_t *)malloc((size_t)((uint64_t)self->img_numsectors * 96));
   if (!self->sub_data)
   {
      filestream_close(sub_stream);
      return false;
   }
   filestream_read(sub_stream, self->sub_data, (uint64_t)self->img_numsectors * 96);
   filestream_close(sub_stream);

   if (!CDAccess_CCD_CheckSubQSanity(self))
      return false;

   return true;
}

static bool CDAccess_CCD_Read_Raw_Sector(CDAccess_CCD *self, uint8_t *buf, int32_t lba)
{
   if (lba < 0)
   {
      synth_udapp_sector_lba(0xFF, &self->tocd, lba, 0, buf);
      return true; /* TODO/FIXME - see if we need to return false here? */
   }

   if ((size_t)lba >= self->img_numsectors)
   {
      synth_leadout_sector_lba(0xFF, &self->tocd, lba, buf);
      return true; /* TODO/FIXME - see if we need to return false here? */
   }

   cdstream_seek(self->img_stream, lba * 2352, SEEK_SET);
   cdstream_read(self->img_stream, buf, 2352);

   subpw_interleave(&self->sub_data[lba * 96], buf + 2352);

   return true;
}

static bool CDAccess_CCD_Fast_Read_Raw_PW_TSRE(CDAccess_CCD *self, uint8_t *pwbuf, int32_t lba)
{
   if (lba < 0)
   {
      subpw_synth_udapp_lba(&self->tocd, lba, 0, pwbuf);
      return true;
   }

   if ((size_t)lba >= self->img_numsectors)
   {
      subpw_synth_leadout_lba(&self->tocd, lba, pwbuf);
      return true;
   }

   subpw_interleave(&self->sub_data[lba * 96], pwbuf);

   return true;
}

static bool CDAccess_CCD_Read_TOC(CDAccess_CCD *self, TOC *out_toc)
{
   *out_toc = self->tocd;
   return true;
}

/* ----------------------------------------------------------------
 * CDAccess vtable adapters and factory.  Same shape as Phase-3
 * established for the three backends. */

static bool CDAccess_CCD_RRS_vt(CDAccess *base, uint8_t *buf, int32_t lba)
{
   return CDAccess_CCD_Read_Raw_Sector((CDAccess_CCD *)base, buf, lba);
}

static bool CDAccess_CCD_FRPT_vt(CDAccess *base, uint8_t *pwbuf, int32_t lba)
{
   return CDAccess_CCD_Fast_Read_Raw_PW_TSRE((CDAccess_CCD *)base, pwbuf, lba);
}

static bool CDAccess_CCD_RTOC_vt(CDAccess *base, TOC *out_toc)
{
   return CDAccess_CCD_Read_TOC((CDAccess_CCD *)base, out_toc);
}

static void CDAccess_CCD_destroy_vt(CDAccess *base)
{
   CDAccess_CCD *self = (CDAccess_CCD *)base;
   CDAccess_CCD_cleanup(self);
   free(self);
}

CDAccess *CDAccess_CCD_New(const char *path, bool image_memcache)
{
   CDAccess_CCD *self = (CDAccess_CCD *)calloc(1, sizeof(*self));
   if (!self)
      return NULL;

   TOC_Clear(&self->tocd);

   if (!CDAccess_CCD_Load(self, path, image_memcache))
   {
      CDAccess_CCD_cleanup(self);
      free(self);
      return NULL;
   }

   self->base.Read_Raw_Sector       = CDAccess_CCD_RRS_vt;
   self->base.Fast_Read_Raw_PW_TSRE = CDAccess_CCD_FRPT_vt;
   self->base.Read_TOC              = CDAccess_CCD_RTOC_vt;
   self->base.destroy               = CDAccess_CCD_destroy_vt;
   return &self->base;
}
