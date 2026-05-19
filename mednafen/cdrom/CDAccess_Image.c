/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 Notes and TODO:

  POSTGAP in CUE sheets may not be handled properly, should the directive automatically increment the index number?

  INDEX nn where 02 <= nn <= 99 is not supported in CUE sheets.

  TOC reading code is extremely barebones, leaving out support for more esoteric features.

  A PREGAP statement in the first track definition in a CUE sheet may not work properly(depends on what is proper);
  it will be added onto the implicit default 00:02:00 of pregap.
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <boolean.h>

#include <streams/file_stream.h>
#include <libretro.h>

#include "../cdstream.h"

#include "CDAccess.h"
#include "CDAccess_Image.h"

#include "audioreader.h"

extern retro_log_printf_t log_cb;

enum
{
   CDRF_SUBM_NONE = 0,
   CDRF_SUBM_RW = 1,
   CDRF_SUBM_RW_RAW = 2
};

/* Disk-image (rip) track / sector formats. */
enum
{
   DI_FORMAT_AUDIO       = 0x00,
   DI_FORMAT_MODE1       = 0x01,
   DI_FORMAT_MODE1_RAW   = 0x02,
   DI_FORMAT_MODE2       = 0x03,
   DI_FORMAT_MODE2_FORM1 = 0x04,
   DI_FORMAT_MODE2_FORM2 = 0x05,
   DI_FORMAT_MODE2_RAW   = 0x06,
   DI_FORMAT_CDI_RAW     = 0x07,
   _DI_FORMAT_COUNT
};

static const int32_t DI_Size_Table[8] =
{
   2352, /* Audio */
   2048, /* MODE1 */
   2352, /* MODE1 RAW */
   2336, /* MODE2 */
   2048, /* MODE2 Form 1 */
   2324, /* Mode 2 Form 2 */
   2352, /* MODE2 RAW */
   2352  /* CD-I RAW */
};

/* ---- sscanf-replacement helpers ---------------------------------
 *
 * Cold-path TOC/CUE parser sites used to call sscanf with format
 * strings like "%u:%u:%u" and "%ld".  glibc sscanf re-parses the
 * format string on every call (and on at least some libc versions
 * does an internal alloc), which is needless overhead even for
 * cold paths; replacement strtoul/strtol-based parsers run in a
 * few cycles and let us thread proper failure returns through
 * sites that previously discarded sscanf's count.  Defensive
 * NULL-check on the head pointer is for robustness in the face of
 * sscanf-style 'maybe-NULL on optional CUE field' call patterns. */

static bool parse_uint(const char *s, unsigned *out)
{
   char *e;
   unsigned long v;
   if (!s) return false;
   v = strtoul(s, &e, 10);
   if (e == s) return false;
   *out = (unsigned)v;
   return true;
}

static bool parse_long(const char *s, long *out)
{
   char *e;
   long v;
   if (!s) return false;
   v = strtol(s, &e, 10);
   if (e == s) return false;
   *out = v;
   return true;
}

/* Parse "M:S:F" (BCD-ish minutes:seconds:frames triple as text).
 * Replaces sscanf with %u:%u:%u and %d:%d:%d, both unified to
 * unsigned -- MSF values are conceptually non-negative.
 *
 * Bounds m <= 99, s <= 59, f <= 74 (standard CD-ROM MSF range; m=99
 * covers extended-runtime CD-Rs).  Without these, a malformed CUE
 * or TOC with absurd MSF values (e.g. "999999:0:0") propagates as
 * an absurd byte offset through ((m*60+s)*75+f)*sector_mult, which
 * has no real-world meaning and risks integer wrap when added to
 * 32-bit FileOffset / sectors fields downstream.  Saturn discs are
 * all standard 74-min CD-ROM Mode 2 Form 1, so legitimate inputs
 * never exceed m <= 80 or so in practice; the 99 cap is the
 * standard CD spec limit.  Upstream Mednafen's StringToMSF enforces
 * the same bounds but only at that single sscanf-replacement site;
 * we apply it at every MSF parse for defence in depth. */
static bool parse_msf(const char *str, unsigned *m, unsigned *sec, unsigned *f)
{
   char *e;
   unsigned long v;
   if (!str) return false;

   v = strtoul(str, &e, 10);
   if (e == str || *e != ':') return false;
   *m = (unsigned)v;
   str = e + 1;

   v = strtoul(str, &e, 10);
   if (e == str || *e != ':') return false;
   *sec = (unsigned)v;
   str = e + 1;

   v = strtoul(str, &e, 10);
   if (e == str) return false;
   *f = (unsigned)v;

   if (*m > 99 || *sec > 59 || *f > 74) return false;
   return true;
}

/* ---- subq_map operations ----------------------------------------
 * Replaces std::map<uint32_t, stl_array<uint8_t,12>> with a sorted-
 * array binary search.  SBI tables are small (a few dozen entries
 * typical), so the red-black tree overhead of std::map was pure
 * cost.  Binary search on a packed array is both faster (fewer
 * cache misses - the whole array typically fits in 1-3 cache lines)
 * and avoids the per-entry heap allocation std::map does. */

static void subq_map_clear(subq_map *m)
{
   m->count  = 0;
   m->sorted = false;
}

static bool subq_map_empty(const subq_map *m)
{
   return m->count == 0;
}

static void subq_map_insert(subq_map *m, uint32_t aba, const uint8_t data[12])
{
   unsigned i;
   /* Overwrite-on-duplicate to match the std::map::operator[]= semantics
    * the pre-conversion code had: a malformed SBI with duplicate aba
    * entries should retain the last write, not an arbitrary one after
    * qsort. SBI sizes are tiny (a few dozen entries) so the linear scan
    * here is cheaper than maintaining a side index. Overwrite preserves
    * sort order (the aba doesn't change), so m->sorted stays as-is. */
   for (i = 0; i < m->count; i++)
   {
      if (m->entries[i].aba == aba)
      {
         memcpy(m->entries[i].data, data, 12);
         return;
      }
   }
   if (m->count >= SUBQ_MAP_MAX)
      return;
   m->entries[m->count].aba = aba;
   memcpy(m->entries[m->count].data, data, 12);
   m->count++;
   m->sorted = false;
}

static int subq_entry_cmp(const void *a, const void *b)
{
   uint32_t aa = ((const struct subq_map_entry *)a)->aba;
   uint32_t bb = ((const struct subq_map_entry *)b)->aba;
   if (aa < bb) return -1;
   if (aa > bb) return  1;
   return 0;
}

static void subq_map_finalize(subq_map *m)
{
   if (m->sorted)
      return;
   if (m->count > 1)
      qsort(m->entries, m->count, sizeof(struct subq_map_entry), subq_entry_cmp);
   m->sorted = true;
}

static const uint8_t *subq_map_find(const subq_map *m, uint32_t aba)
{
   if (!m->sorted)
   {
      unsigned i;
      for (i = 0; i < m->count; i++)
         if (m->entries[i].aba == aba)
            return m->entries[i].data;
      return NULL;
   }
   else
   {
      int lo = 0;
      int hi = (int)m->count - 1;
      while (lo <= hi)
      {
         int      mid = lo + ((hi - lo) >> 1);
         uint32_t mid_aba = m->entries[mid].aba;
         if (mid_aba == aba)
            return m->entries[mid].data;
         if (mid_aba < aba)
            lo = mid + 1;
         else
            hi = mid - 1;
      }
      return NULL;
   }
}

/* ---- toc_streamcache operations ---------------------------------
 * Replaces std::map<std::string, cdstream*> for the CUE/TOC parser's
 * per-disc-image file dedup.  Linear scan on insert/find - parser
 * is one-shot at disc load. */

static void toc_streamcache_init(toc_streamcache *c)
{
   c->count = 0;
}

static cdstream *toc_streamcache_find(const toc_streamcache *c, const char *filename)
{
   unsigned i;
   for (i = 0; i < c->count; i++)
      if (strcmp(c->entries[i].filename, filename) == 0)
         return c->entries[i].fp;
   return NULL;
}

static void toc_streamcache_insert(toc_streamcache *c, const char *filename, cdstream *fp)
{
   size_t n;
   if (c->count >= TOC_STREAMCACHE_MAX)
      return;
   n = strlen(filename);
   if (n >= TOC_STREAMCACHE_NAME)
      n = TOC_STREAMCACHE_NAME - 1;
   memcpy(c->entries[c->count].filename, filename, n);
   c->entries[c->count].filename[n] = '\0';
   c->entries[c->count].fp          = fp;
   c->count++;
}

/* ---- Generic string helpers -------------------------------------
 * Bounded char-buffer equivalents of the C++ string operations the
 * old parser used.  Same names as the CCD conversion's helpers so
 * they're easy to recognize across the CD layer. */

static void copy_str(char *dst, size_t dst_sz, const char *src)
{
   size_t n = strlen(src);
   if (n >= dst_sz)
      n = dst_sz - 1;
   memcpy(dst, src, n);
   dst[n] = '\0';
}

/* In-place ASCII uppercase.  Replaces the file-local
 * MDFN_strtoupper(std::string&) helper. */
static void str_to_upper(char *s)
{
   for (; *s; s++)
   {
      if (*s >= 'a' && *s <= 'z')
         *s = (char)(*s - 'a' + 'A');
   }
}

/* Trim leading/trailing ASCII whitespace in place.  Replaces
 * MDFN_ltrim + MDFN_rtrim from mednafen/general.c (which were
 * upstream Mednafen helpers; this fork inlines them here so the
 * CDAccess_Image TU doesn't depend on general.c for two short
 * string functions). */
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

/* ---- Path helpers - C versions of MDFN_GetFilePathComponents /
 *      MDFN_EvalFIP from mednafen/general.c.  Same logic as the
 *      CCD conversion. */

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
      char *base_out, size_t base_sz, char *ext_out, size_t ext_sz)
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
   if (dot)
   {
      base_len = (size_t)(dot - file);
      copy_str(ext_out, ext_sz, dot);
   }
   else
   {
      base_len = strlen(file);
      ext_out[0] = '\0';
   }

   if (base_len >= base_sz)
      base_len = base_sz - 1;
   memcpy(base_out, file, base_len);
   base_out[base_len] = '\0';
}

/* Equivalent to MDFN_EvalFIP(dir, name, skip_safety=true): if name
 * is absolute, use as-is; else join dir + sep + name. */
static bool join_path(const char *dir, const char *name, char *out_buf, size_t out_sz)
{
   int n;
   /* Treat names with absolute path indicators as already absolute. */
   if (name[0] == '/'
#ifdef _WIN32
         || name[0] == '\\'
         || (name[0] && name[1] == ':')
#endif
      )
   {
      size_t name_len = strlen(name);
      if (name_len >= out_sz)
         return false;
      copy_str(out_buf, out_sz, name);
      return true;
   }
   /* snprintf returns the length that *would* have been written, not
    * what was written.  A return >= out_sz means truncation.  Pre-
    * conversion code used std::string and grew the buffer; here a
    * pathological path silently truncated and produced a not-found
    * error one layer down.  Surface the failure here for a clearer
    * error path. */
   n = snprintf(out_buf, out_sz, "%s%c%s", dir, PATH_SEP, name);
   if (n < 0 || (size_t)n >= out_sz)
      return false;
   return true;
}

/* ---- CUE/TOC parser tables ------------------------------------- */

static const char *DI_CDRDAO_Strings[8] =
{
   "AUDIO",
   "MODE1",
   "MODE1_RAW",
   "MODE2",
   "MODE2_FORM1",
   "MODE2_FORM2",
   "MODE2_RAW",
   "CDI_RAW"
};

static const char *DI_CUE_Strings[8] =
{
   "AUDIO",
   "MODE1/2048",
   "MODE1/2352",
   "MODE2/2336",
   "MODE2/2048",
   "MODE2/2324",
   "MODE2/2352",
   "CDI/2352"
};

/* UnQuotify: read one whitespace-separated (optionally quoted)
 * token from src starting at source_offset; write it to dest
 * (capped at dest_sz - 1).  Returns the offset to the start of the
 * next token, or src_len if exhausted.
 *
 * Was static size_t UnQuotify(const std::string&, size_t,
 * std::string&, bool) in the C++ source. */
static size_t UnQuotify(const char *src, size_t src_len, size_t source_offset,
      char *dest, size_t dest_sz, bool parse_quotes)
{
   bool   in_quote        = false;
   bool   already_normal  = false;
   size_t dest_pos        = 0;

   if (dest_sz > 0)
      dest[0] = '\0';

   while (source_offset < src_len)
   {
      char c = src[source_offset];

      if (c == ' ' || c == '\t')
      {
         if (!in_quote)
         {
            if (already_normal)
               break;
            source_offset++;
            continue;
         }
      }

      if (c == '"' && parse_quotes)
      {
         if (in_quote)
         {
            source_offset++;
            break;
         }
         else
         {
            in_quote = true;
         }
      }
      else
      {
         if (dest_pos + 1 < dest_sz)
         {
            dest[dest_pos++] = c;
            dest[dest_pos]   = '\0';
         }
         already_normal = true;
      }
      source_offset++;
   }

   while (source_offset < src_len)
   {
      if (src[source_offset] != ' ' && src[source_offset] != '\t')
         break;
      source_offset++;
   }

   return source_offset;
}

static bool StringToMSF(const char *str, unsigned *m, unsigned *s, unsigned *f)
{
   /* parse_msf now does the m<=99 / s<=59 / f<=74 bounds check
    * itself, so this wrapper is just an alias kept for naming
    * symmetry with the rest of the parser code. */
   return parse_msf(str, m, s, f);
}

/* ---- Forward declarations ---------------------------------------
 * Methods reference each other regardless of source order. */

static bool     CDAccess_Image_ImageOpen(CDAccess_Image *self, const char *path, bool image_memcache);
static bool     CDAccess_Image_LoadSBI  (CDAccess_Image *self, const char *sbi_path);
static void     CDAccess_Image_Cleanup  (CDAccess_Image *self);
static void     CDAccess_Image_GenerateTOC(CDAccess_Image *self);
static int32_t  CDAccess_Image_MakeSubPQ(const CDAccess_Image *self, int32_t lba, uint8_t *SubPWBuf);
static bool     CDAccess_Image_ParseTOCFileLineInfo(CDAccess_Image *self,
      CDRFILE_TRACK_INFO *track, const int tracknum, const char *filename,
      const char *binoffset, const char *msfoffset, const char *length,
      bool image_memcache, toc_streamcache *cache);
static uint32_t CDAccess_Image_GetSectorCount(CDRFILE_TRACK_INFO *track);

/* ---- Method implementations ------------------------------------- */

static uint32_t CDAccess_Image_GetSectorCount(CDRFILE_TRACK_INFO *track)
{
   /* (size - FileOffset) is signed int64 throughout; a malformed
    * CUE that pushed FileOffset past the actual file size used to
    * underflow into a huge negative int64, which the (uint32_t)
    * cast then wrapped into a giant sector count.  Clamp to zero
    * on FileOffset > content_size so downstream length validation
    * (`tmp_long > sectors`) trips cleanly rather than passing
    * through to track->sectors = (int32_t)<garbage>. */
   if (track->DIFormat == DI_FORMAT_AUDIO)
   {
      if (track->AReader)
      {
         const int64_t content = (int64_t)AR_FrameCount(track->AReader) * 4;
         if (track->FileOffset >= content) return 0;
         return (uint32_t)((content - track->FileOffset) / 2352);
      }
      else
      {
         const int64_t size = cdstream_size(track->fp);
         if (track->FileOffset >= size) return 0;
         if (track->SubchannelMode)
            return (uint32_t)((size - track->FileOffset) / (2352 + 96));
         return (uint32_t)((size - track->FileOffset) / 2352);
      }
   }
   else
   {
      const int64_t size = cdstream_size(track->fp);
      if (track->FileOffset >= size) return 0;
      return (uint32_t)((size - track->FileOffset) / DI_Size_Table[track->DIFormat]);
   }
}

static bool CDAccess_Image_ParseTOCFileLineInfo(CDAccess_Image *self,
      CDRFILE_TRACK_INFO *track, const int tracknum, const char *filename,
      const char *binoffset, const char *msfoffset, const char *length,
      bool image_memcache, toc_streamcache *cache)
{
   /* Byte-offset accumulator is int64 (rather than the storage
    * field's `long`, which is 32-bit on MinGW/Windows) so the two
    * += additions below can't trip signed overflow UB when a
    * malformed TOC supplies a huge binoffset.  We then validate
    * the accumulated total fits in `long` before storing it back
    * to track->FileOffset. */
   int64_t   offset_acc = 0;
   long      tmp_long;
   unsigned  m, s, f;
   uint32_t  sector_mult;
   long      sectors;
   cdstream *cached_fp = toc_streamcache_find(cache, filename);
   size_t    fname_len = strlen(filename);

   if (cached_fp)
   {
      track->FirstFileInstance = 0;
      track->fp                = cached_fp;
   }
   else
   {
      char efn[2048];
      track->FirstFileInstance = 1;
      if (!join_path(self->base_dir, filename, efn, sizeof(efn)))
         return false;

      if (image_memcache)
         track->fp = cdstream_new_memcached(efn);
      else
         track->fp = cdstream_new(efn);

      if (!track->fp)
         return false;

      toc_streamcache_insert(cache, filename, track->fp);
   }

   if (fname_len >= 4 && !strcasecmp(filename + fname_len - 4, ".wav"))
   {
      track->AReader = AR_Open(track->fp);
      if (!track->AReader)
         return false;
   }

   sector_mult = DI_Size_Table[track->DIFormat];

   if (track->SubchannelMode)
      sector_mult += 96;

   if (parse_long(binoffset, &tmp_long))
      offset_acc += (int64_t)tmp_long;

   /* MSF post-bounds (m<=99, s<=59, f<=74) is at most 449999 sectors
    * * 2448 bytes/sector = ~1.1 GB; the int64 accumulator absorbs
    * any in-range addition without wrap. */
   if (parse_msf(msfoffset, &m, &s, &f))
      offset_acc += (int64_t)(((m * 60 + s) * 75 + f) * sector_mult);

   /* Reject if the accumulated offset is negative or wouldn't fit
    * into the storage field on this platform (long is 32-bit under
    * MinGW even for 64-bit Windows builds). */
   if (offset_acc < 0 || offset_acc > (int64_t)LONG_MAX)
      return false;

   track->FileOffset = (long)offset_acc; /* Set this before GetSectorCount! */
   sectors = CDAccess_Image_GetSectorCount(track);

   if (length)
   {
      tmp_long = sectors;

      if (parse_msf(length, &m, &s, &f))
         tmp_long = (m * 60 + s) * 75 + f;
      else if (track->DIFormat == DI_FORMAT_AUDIO)
      {
         char *endptr = NULL;
         tmp_long = strtol(length, &endptr, 10);
         if (endptr == length)
            tmp_long = sectors;
         else
            tmp_long /= 588;
      }

      if (tmp_long > sectors)
         return false;
      sectors = tmp_long;
   }

   track->sectors = (int32_t)sectors;
   return true;
}

static bool CDAccess_Image_LoadSBI(CDAccess_Image *self, const char *sbi_path)
{
   uint8_t header[4];
   uint8_t ed[4 + 10];
   uint8_t tmpq[12];
   RFILE  *sbis = NULL;

   /* SBI file not available, but don't error out. */
   if (!filestream_exists(sbi_path))
      return true;

   sbis = filestream_open(sbi_path,
         RETRO_VFS_FILE_ACCESS_READ,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);

   if (!sbis)
      return true;

   /* Short or failed read leaves header[] with stack garbage; the
    * pre-fix code then ran memcmp against that garbage, which only
    * happened to fail the magic check by luck of the stack bytes.
    * Fold the read-success check into the magic check. */
   if (filestream_read(sbis, header, 4) != 4
         || memcmp(header, "SBI\0", 4))
      goto error;

   while (filestream_read(sbis, ed, sizeof(ed)) == sizeof(ed))
   {
      uint32_t aba;

      if (!BCD_is_valid(ed[0]) || !BCD_is_valid(ed[1]) || !BCD_is_valid(ed[2]))
         goto error;

      if (ed[3] != 0x01)
         goto error;

      memcpy(tmpq, &ed[4], 10);

      subq_generate_checksum(tmpq);
      tmpq[10] ^= 0xFF;
      tmpq[11] ^= 0xFF;

      aba = AMSF_to_ABA(BCD_to_U8(ed[0]), BCD_to_U8(ed[1]), BCD_to_U8(ed[2]));

      subq_map_insert(&self->SubQReplaceMap, aba, tmpq);
   }

   /* Sort once after all inserts so subsequent MakeSubPQ lookups
    * can binary-search. */
   subq_map_finalize(&self->SubQReplaceMap);

   filestream_close(sbis);
   return true;

error:
   if (sbis)
      filestream_close(sbis);
   return false;
}

/* ---- ImageOpen: parse CUE / TOC, populate Tracks[] ------------- */

#define UNQ_MAX_ARGS  4
#define UNQ_ARG_LEN   512   /* args[] entry capacity */
#define UNQ_LINE_LEN  1024  /* maximum CUE/TOC line length */
#define UNQ_CMD_LEN   64

static bool CDAccess_Image_ImageOpen(CDAccess_Image *self, const char *path, bool image_memcache)
{
   cdstream    fp;
   bool        opened   = false;
   bool        success  = false;
   bool        IsTOC    = false;
   int32_t     active_track = -1;
   int32_t     AutoTrackInc = 1;        /* For TOC */
   CDRFILE_TRACK_INFO TmpTrack;
   char        file_base[256];
   char        file_ext[64];
   toc_streamcache cache;
   char        line_c[UNQ_LINE_LEN];
   char        linebuf[UNQ_LINE_LEN];
   char        cmdbuf[UNQ_CMD_LEN];
   char        args[UNQ_MAX_ARGS][UNQ_ARG_LEN];
   int32_t     RunningLBA;
   /* Cross-track byte-offset accumulator: int64 to absorb the
    * cumulative `sectors * sector_size` additions across all
    * tracks that share a single backing file.  The per-track
    * storage field track->FileOffset is `long` (32-bit on MinGW
    * even for 64-bit builds), so we explicitly bounds-check this
    * accumulator at every store and fail the load cleanly on
    * overflow rather than silently wrapping into a negative offset
    * that the cdstream_seek downstream would reject. */
   int64_t     FileOffset;
   int         x;

   /* Cue / toc file is small; slurp it into RAM up front so the
    * line-by-line parser hits an in-memory tight loop instead of
    * byte-at-a-time filestream reads.  Closed on every return path
    * via the `opened` flag and the cleanup_close label. */
   if (!cdstream_open_memcached(&fp, path))
      return false;
   opened = true;

   memset(&TmpTrack, 0, sizeof(TmpTrack));
   toc_streamcache_init(&cache);
   self->disc_type = DISC_TYPE_CDDA_OR_M1;

   /* Split out base_dir / file_base / file_ext. */
   split_path(path, self->base_dir, sizeof(self->base_dir),
         file_base, sizeof(file_base), file_ext, sizeof(file_ext));

   if (!strcasecmp(file_ext, ".toc"))
      IsTOC = true;

   /* Check for annoying UTF-8 BOM. */
   if (!IsTOC)
   {
      uint8_t bom_tmp[3];
      if (cdstream_read(&fp, bom_tmp, 3) == 3
            && bom_tmp[0] == 0xEF && bom_tmp[1] == 0xBB && bom_tmp[2] == 0xBF)
      {
         /* Print an annoying error message, but don't actually error out. */
      }
      else
         cdstream_seek(&fp, 0, SEEK_SET);
   }

   /* Assign opposite maximum values so our tests will work! */
   self->FirstTrack = 99;
   self->LastTrack  = 0;

   while (cdstream_get_line(&fp, line_c, sizeof(line_c)) >= 0)
   {
      size_t   linelen;
      size_t   offs;
      unsigned argcount = 0;
      unsigned ai;

      copy_str(linebuf, sizeof(linebuf), line_c);

      if (IsTOC)
      {
         /* Handle TOC format comments */
         char *ss = strstr(linebuf, "//");
         if (ss)
            *ss = '\0';
      }

      /* Trim AFTER stripping TOC comments so trailing whitespace
       * in lines like `MONKEY  // BABIES` gets cleared. */
      str_trim(linebuf);

      linelen = strlen(linebuf);
      if (linelen == 0)
         continue;

      /* Grab command and arguments. */
      offs = 0;
      offs = UnQuotify(linebuf, linelen, offs, cmdbuf, sizeof(cmdbuf), false);
      for (argcount = 0; argcount < UNQ_MAX_ARGS && offs < linelen; argcount++)
         offs = UnQuotify(linebuf, linelen, offs, args[argcount], UNQ_ARG_LEN, true);

      /* Clear unused args so we don't have inter-line leaks. */
      for (ai = argcount; ai < UNQ_MAX_ARGS; ai++)
         args[ai][0] = '\0';

      str_to_upper(cmdbuf);

      if (IsTOC)
      {
         if (!strcmp(cmdbuf, "TRACK"))
         {
            int format_lookup;
            int32_t i;

            if (active_track >= 0)
            {
               memcpy(&self->Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
               memset(&TmpTrack, 0, sizeof(TmpTrack));
               active_track = -1;
            }

            for (i = 2; i < 100; i++)
               TmpTrack.index[i] = -1;

            if (AutoTrackInc > 99)
               goto cleanup_close;

            active_track = AutoTrackInc++;
            if (active_track < self->FirstTrack)
               self->FirstTrack = active_track;
            if (active_track > self->LastTrack)
               self->LastTrack  = active_track;

            for (format_lookup = 0; format_lookup < _DI_FORMAT_COUNT; format_lookup++)
            {
               if (!strcasecmp(args[0], DI_CDRDAO_Strings[format_lookup]))
               {
                  TmpTrack.DIFormat = format_lookup;
                  break;
               }
            }

            if (format_lookup == _DI_FORMAT_COUNT)
               goto cleanup_close;

            if (TmpTrack.DIFormat == DI_FORMAT_AUDIO)
               TmpTrack.RawAudioMSBFirst = true; /* Silly cdrdao... */

            if (!strcasecmp(args[1], "RW"))
               TmpTrack.SubchannelMode = CDRF_SUBM_RW;
            else if (!strcasecmp(args[1], "RW_RAW"))
               TmpTrack.SubchannelMode = CDRF_SUBM_RW_RAW;
         }
         else if (!strcmp(cmdbuf, "FIFO"))
            goto cleanup_close;
         else if (!strcmp(cmdbuf, "FILE") || !strcmp(cmdbuf, "AUDIOFILE"))
         {
            const char *binoffset = NULL;
            const char *msfoffset = NULL;
            const char *length    = NULL;

            if (args[1][0] == '#')
            {
               binoffset = args[1] + 1;
               msfoffset = args[2];
               length    = args[3];
            }
            else
            {
               msfoffset = args[1];
               length    = args[2];
            }
            if (!CDAccess_Image_ParseTOCFileLineInfo(self, &TmpTrack, active_track,
                     args[0], binoffset, msfoffset, length, image_memcache, &cache))
               goto cleanup_close;
         }
         else if (!strcmp(cmdbuf, "DATAFILE"))
         {
            const char *binoffset = NULL;
            const char *length    = NULL;

            if (args[1][0] == '#')
            {
               binoffset = args[1] + 1;
               length    = args[2];
            }
            else
               length = args[1];

            if (!CDAccess_Image_ParseTOCFileLineInfo(self, &TmpTrack, active_track,
                     args[0], binoffset, NULL, length, image_memcache, &cache))
               goto cleanup_close;
         }
         else if (!strcmp(cmdbuf, "INDEX"))
            goto cleanup_close;
         else if (!strcmp(cmdbuf, "PREGAP"))
         {
            unsigned int m, s, f;
            if (active_track < 0)
               goto cleanup_close;
            if (!StringToMSF(args[0], &m, &s, &f))
               goto cleanup_close;
            TmpTrack.pregap = (m * 60 + s) * 75 + f;
         }
         else if (!strcmp(cmdbuf, "START"))
         {
            unsigned int m, s, f;
            if (active_track < 0)
               goto cleanup_close;
            if (!StringToMSF(args[0], &m, &s, &f))
               goto cleanup_close;
            TmpTrack.pregap = (m * 60 + s) * 75 + f;
         }
         else if (!strcmp(cmdbuf, "TWO_CHANNEL_AUDIO"))
            TmpTrack.subq_control &= ~SUBQ_CTRLF_4CH;
         else if (!strcmp(cmdbuf, "FOUR_CHANNEL_AUDIO"))
            TmpTrack.subq_control |=  SUBQ_CTRLF_4CH;
         else if (!strcmp(cmdbuf, "NO"))
         {
            str_to_upper(args[0]);
            if (!strcmp(args[0], "COPY"))
               TmpTrack.subq_control &= ~SUBQ_CTRLF_DCP;
            else if (!strcmp(args[0], "PRE_EMPHASIS"))
               TmpTrack.subq_control &= ~SUBQ_CTRLF_PRE;
            else
               goto cleanup_close;
         }
         else if (!strcmp(cmdbuf, "COPY"))
            TmpTrack.subq_control |= SUBQ_CTRLF_DCP;
         else if (!strcmp(cmdbuf, "PRE_EMPHASIS"))
            TmpTrack.subq_control |= SUBQ_CTRLF_PRE;
         else if (!strcmp(cmdbuf, "CD_DA"))
            self->disc_type = DISC_TYPE_CDDA_OR_M1;
         else if (!strcmp(cmdbuf, "CD_ROM"))
            self->disc_type = DISC_TYPE_CDDA_OR_M1;
         else if (!strcmp(cmdbuf, "CD_ROM_XA"))
            self->disc_type = DISC_TYPE_CD_XA;
         /* TODO: CATALOG */
      }
      else /* CUE sheet handling */
      {
         if (!strcmp(cmdbuf, "FILE"))
         {
            char efn[2048];

            if (active_track >= 0)
            {
               memcpy(&self->Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
               memset(&TmpTrack, 0, sizeof(TmpTrack));
               active_track = -1;
            }

            if (strstr(args[0], "cdrom://") == NULL)
            {
               if (!join_path(self->base_dir, args[0], efn, sizeof(efn)))
                  goto cleanup_close;
            }
            else
            {
               /* cdrom:// URL: copied verbatim.  Detect truncation
                * explicitly (copy_str silently truncates). */
               if (strlen(args[0]) >= sizeof(efn))
                  goto cleanup_close;
               copy_str(efn, sizeof(efn), args[0]);
            }

            if (image_memcache)
               TmpTrack.fp = cdstream_new_memcached(efn);
            else
               TmpTrack.fp = cdstream_new(efn);

            if (!TmpTrack.fp)
               goto cleanup_close;

            TmpTrack.FirstFileInstance = 1;

            if (!strcasecmp(args[1], "BINARY"))
            {
               /* nothing extra */
            }
            else if (!strcasecmp(args[1], "WAVE") || !strcasecmp(args[1], "WAV")
                  || !strcasecmp(args[1], "PCM"))
            {
               /* Treat as raw audio - same path as BINARY. */
            }
            else if (!strcasecmp(args[1], "OGG") || !strcasecmp(args[1], "VORBIS"))
            {
               /* Beetle-Saturn's audioreader is Vorbis-only.  Upstream
                * Mednafen historically listed MPC / MP+ here, but the
                * libretro fork has never shipped a Musepack decoder
                * (no libmpcdec in deps/, no HAVE_MPC code path).  An
                * MPC-track CUE would reach this arm, hand the MPC byte
                * stream to ov_open_callbacks, and fail at the Ogg
                * magic check inside tremor; dropping the keywords
                * makes that failure happen one branch earlier with a
                * more honest path through the parser. */
               TmpTrack.AReader = AR_Open(TmpTrack.fp);
               if (!TmpTrack.AReader)
                  goto cleanup_close;
            }
            else
               goto cleanup_close;
         }
         else if (!strcmp(cmdbuf, "TRACK"))
         {
            int format_lookup;
            int32_t i;

            if (active_track >= 0)
            {
               memcpy(&self->Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
               TmpTrack.FirstFileInstance = 0;
               TmpTrack.pregap            = 0;
               TmpTrack.pregap_dv         = 0;
               TmpTrack.postgap           = 0;
               TmpTrack.index[0]          = -1;
               TmpTrack.index[1]          = 0;
            }

            for (i = 2; i < 100; i++)
               TmpTrack.index[i] = -1;

            active_track = atoi(args[0]);

            if (active_track < 1 || active_track > 99)
               goto cleanup_close;

            if (active_track < self->FirstTrack)
               self->FirstTrack = active_track;
            if (active_track > self->LastTrack)
               self->LastTrack  = active_track;

            for (format_lookup = 0; format_lookup < _DI_FORMAT_COUNT; format_lookup++)
            {
               if (!strcasecmp(args[1], DI_CUE_Strings[format_lookup]))
               {
                  TmpTrack.DIFormat = format_lookup;
                  break;
               }
            }

            if (format_lookup == _DI_FORMAT_COUNT)
               goto cleanup_close;
         }
         else if (!strcmp(cmdbuf, "INDEX"))
         {
            if (active_track >= 0)
            {
               unsigned wi;
               unsigned int m, s, f;

               if (!StringToMSF(args[1], &m, &s, &f))
                  goto cleanup_close;

               if (parse_uint(args[0], &wi) && wi < 100)
                  TmpTrack.index[wi] = (m * 60 + s) * 75 + f;
               else
                  goto cleanup_close;
            }
         }
         else if (!strcmp(cmdbuf, "PREGAP"))
         {
            if (active_track >= 0)
            {
               unsigned int m, s, f;
               if (!StringToMSF(args[0], &m, &s, &f))
                  goto cleanup_close;
               TmpTrack.pregap = (m * 60 + s) * 75 + f;
            }
         }
         else if (!strcmp(cmdbuf, "POSTGAP"))
         {
            if (active_track >= 0)
            {
               unsigned int m, s, f;
               if (!StringToMSF(args[0], &m, &s, &f))
                  goto cleanup_close;
               TmpTrack.postgap = (m * 60 + s) * 75 + f;
            }
         }
         else if (!strcmp(cmdbuf, "REM"))
         {
            /* comment, skip */
         }
         else if (!strcmp(cmdbuf, "FLAGS"))
         {
            unsigned i;
            TmpTrack.subq_control &= ~(SUBQ_CTRLF_PRE | SUBQ_CTRLF_DCP | SUBQ_CTRLF_4CH);
            for (i = 0; i < argcount; i++)
            {
               if (!strcmp(args[i], "DCP"))
                  TmpTrack.subq_control |= SUBQ_CTRLF_DCP;
               else if (!strcmp(args[i], "4CH"))
                  TmpTrack.subq_control |= SUBQ_CTRLF_4CH;
               else if (!strcmp(args[i], "PRE"))
                  TmpTrack.subq_control |= SUBQ_CTRLF_PRE;
               else if (!strcmp(args[i], "SCMS"))
               {
                  /* Not implemented; probably pointless. */
               }
               else
                  goto cleanup_close;
            }
         }
         else if (!strcmp(cmdbuf, "CDTEXTFILE") || !strcmp(cmdbuf, "CATALOG")
               || !strcmp(cmdbuf, "ISRC")       || !strcmp(cmdbuf, "TITLE")
               || !strcmp(cmdbuf, "PERFORMER")  || !strcmp(cmdbuf, "SONGWRITER"))
         {
            /* recognized but ignored */
         }
         else
            goto cleanup_close;
      }
   }

   if (active_track >= 0)
   {
      memcpy(&self->Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
      /* The TRACK and FILE handlers commit-then-zero TmpTrack to
       * maintain the invariant "TmpTrack.FirstFileInstance==1 iff
       * TmpTrack holds uncommitted resources".  cleanup_close's
       * TmpTrack-cleanup block (added in 2f74753) relies on that
       * invariant.  This post-loop commit was the lone exception:
       * without the zero, TmpTrack.fp / AReader remained aliased to
       * self->Tracks[active_track]'s, and any subsequent fall-through
       * to cleanup_close (success path) or goto cleanup_close (the
       * sites below: FirstTrack > LastTrack, the second processing
       * loop's fp/AReader check, SBI failure) would free those via
       * TmpTrack while leaving self->Tracks[active_track] holding
       * dangling pointers.  CDAccess_Image_Cleanup at content-close
       * time then walked them and crashed. */
      memset(&TmpTrack, 0, sizeof(TmpTrack));
   }

   if (self->FirstTrack > self->LastTrack)
      goto cleanup_close;

   self->NumTracks = 1 + self->LastTrack - self->FirstTrack;

   RunningLBA  = 0;
   FileOffset  = 0;

   RunningLBA -= 150;
   self->Tracks[self->FirstTrack].pregap += 150;

   for (x = self->FirstTrack; x < (self->FirstTrack + self->NumTracks); x++)
   {
      if (!self->Tracks[x].fp && !self->Tracks[x].AReader)
         goto cleanup_close;

      if (self->Tracks[x].DIFormat == DI_FORMAT_AUDIO)
         self->Tracks[x].subq_control &= ~SUBQ_CTRLF_DATA;
      else
         self->Tracks[x].subq_control |=  SUBQ_CTRLF_DATA;

      if (!IsTOC) /* TOC-format disc_type calculation is handled differently. */
      {
         if (self->disc_type != DISC_TYPE_CD_I)
         {
            switch (self->Tracks[x].DIFormat)
            {
            default: break;
            case DI_FORMAT_MODE2:
            case DI_FORMAT_MODE2_FORM1:
            case DI_FORMAT_MODE2_FORM2:
            case DI_FORMAT_MODE2_RAW:
               self->disc_type = DISC_TYPE_CD_XA;
               break;
            case DI_FORMAT_CDI_RAW:
               self->disc_type = DISC_TYPE_CD_I;
               break;
            }
         }
      }

      if (IsTOC)
      {
         RunningLBA            += self->Tracks[x].pregap;
         self->Tracks[x].LBA    = RunningLBA;
         RunningLBA            += self->Tracks[x].sectors;
         RunningLBA            += self->Tracks[x].postgap;
      }
      else /* CUE sheet */
      {
         int idx;
         int32_t prev_idx;

         if (self->Tracks[x].FirstFileInstance)
            FileOffset = 0;

         RunningLBA            += self->Tracks[x].pregap;

         self->Tracks[x].pregap_dv = 0;
         if (self->Tracks[x].index[0] != -1)
            self->Tracks[x].pregap_dv = self->Tracks[x].index[1] - self->Tracks[x].index[0];

         /* Well-formed CUE has INDEX 01 >= INDEX 00 within a track;
          * pregap_dv >= 0.  A malformed sheet that has them swapped
          * would propagate negative pregap_dv into FileOffset and
          * RunningLBA, eventually feeding a negative seek to
          * cdstream_seek.  Reject the load instead. */
         if (self->Tracks[x].pregap_dv < 0)
            goto cleanup_close;

         /* Within-track INDEX 02..99 must be ascending and >= INDEX 01.
          * parse_msf bounded each INDEX value's MSF range, but never
          * checked the relationship between indices.  An out-of-order
          * INDEX produces wrong SubQ-index reporting in MakeSubPQ
          * (which scans `if (lba >= track.index[i])`); for malformed
          * input where index[i] is negative-after-rebasing at line
          * 1297, the comparison's signed semantics flip the index
          * detection entirely. */
         prev_idx = self->Tracks[x].index[1];
         for (idx = 2; idx < 100; idx++)
         {
            if (self->Tracks[x].index[idx] == -1)
               continue;
            if (self->Tracks[x].index[idx] < prev_idx)
               goto cleanup_close;
            prev_idx = self->Tracks[x].index[idx];
         }

         FileOffset            += (int64_t)self->Tracks[x].pregap_dv * DI_Size_Table[self->Tracks[x].DIFormat];
         RunningLBA            += self->Tracks[x].pregap_dv;

         self->Tracks[x].LBA    = RunningLBA;

         /* FileOffset must fit the per-track storage field (long,
          * 32-bit on MinGW).  If the cumulative byte offset across
          * tracks-sharing-one-file overflows, the (long) cast below
          * would wrap to a negative value and cdstream_seek would
          * reject every subsequent track read. */
         if (FileOffset < 0 || FileOffset > (int64_t)LONG_MAX)
            goto cleanup_close;

         /* Set FileOffset before GetSectorCount. */
         self->Tracks[x].FileOffset = (long)FileOffset;
         self->Tracks[x].sectors    = CDAccess_Image_GetSectorCount(&self->Tracks[x]);

         if ((x + 1) >= (self->FirstTrack + self->NumTracks)
               || self->Tracks[x + 1].FirstFileInstance)
         {
            /* nothing */
         }
         else
         {
            /* Multiple tracks share one binary image - fix the sector count. */
            if (self->Tracks[x + 1].index[0] == -1)
               self->Tracks[x].sectors = self->Tracks[x + 1].index[1] - self->Tracks[x].index[1];
            else
               self->Tracks[x].sectors = self->Tracks[x + 1].index[0] - self->Tracks[x].index[1];

            /* Adjacent-track index subtraction must be non-negative
             * for well-formed CUE (tracks listed in ascending order).
             * Negative sectors propagates into RunningLBA and the
             * cumulative FileOffset arithmetic below; reject. */
            if (self->Tracks[x].sectors < 0)
               goto cleanup_close;
         }

         RunningLBA += self->Tracks[x].sectors;
         RunningLBA += self->Tracks[x].postgap;

         FileOffset += (int64_t)self->Tracks[x].sectors * DI_Size_Table[self->Tracks[x].DIFormat];
      }
   }

   self->total_sectors = RunningLBA;

   /* Adjust indexes for MakeSubPQ. */
   for (x = self->FirstTrack; x < (self->FirstTrack + self->NumTracks); x++)
   {
      const int32_t base = self->Tracks[x].index[1];
      int32_t i;

      for (i = 0; i < 100; i++)
      {
         if (i == 0 || self->Tracks[x].index[i] == -1)
            self->Tracks[x].index[i] = INT32_MAX;
         else
            self->Tracks[x].index[i] = self->Tracks[x].LBA + (self->Tracks[x].index[i] - base);

         assert(self->Tracks[x].index[i] >= 0);
      }
   }

   /* Load SBI file, if present. */
   if (!IsTOC)
   {
      char sbi_ext[4] = { 's', 'b', 'i', 0 };
      char sbi_name[260];
      char sbi_path[2048];
      size_t flen = strlen(file_ext);
      unsigned i;

      /* Match the case of the user-supplied extension on case-
       * sensitive filesystems. */
      if (flen == 4 && file_ext[0] == '.')
      {
         for (i = 0; i < 3; i++)
         {
            if (file_ext[1 + i] >= 'A' && file_ext[1 + i] <= 'Z')
               sbi_ext[i] = (char)(sbi_ext[i] + ('A' - 'a'));
         }
      }

      /* Construct sbi_path = base_dir + '/' + file_base + '.' + sbi_ext.
       * On a pathological file_base / base_dir length that overflows
       * either fixed buffer, skip SBI loading rather than failing the
       * image -- LoadSBI itself treats "file doesn't exist" as success
       * because most games ship no .sbi, and a truncated lookup path
       * could otherwise match an unrelated file on disk. */
      {
         int n = snprintf(sbi_name, sizeof(sbi_name), "%s.%s", file_base, sbi_ext);
         if (n < 0 || (size_t)n >= sizeof(sbi_name))
            goto skip_sbi;
         if (!join_path(self->base_dir, sbi_name, sbi_path, sizeof(sbi_path)))
            goto skip_sbi;
      }

      if (!CDAccess_Image_LoadSBI(self, sbi_path))
         goto cleanup_close;
skip_sbi: ;
   }

   CDAccess_Image_GenerateTOC(self);

   success = true;

cleanup_close:
   /* TmpTrack holds an uncommitted file handle (and possibly AReader)
    * between a CUE FILE-line open and the next commit -- which can be
    * a TRACK or FILE line during parsing, or the post-loop final
    * commit after the parse loop ends.  All three commit sites
    * memcpy TmpTrack into self->Tracks[active_track] and then
    * memset(&TmpTrack, 0, ...), so TmpTrack.FirstFileInstance==1 here
    * implies TmpTrack still owns resources that nobody else does.
    * If parsing fails in the open-to-commit window we goto here
    * directly and TmpTrack goes out of scope as a stack local; the
    * pre-conversion C++ used a unique_ptr that handled this in its
    * destructor.  Mirror that here.  Gate on FirstFileInstance: a
    * shared-fp track (FFI==0, fp aliased to a cache entry) must NOT
    * close the fp -- the owner does that. */
   if (TmpTrack.FirstFileInstance)
   {
      if (TmpTrack.AReader)
      {
         AR_Close(TmpTrack.AReader);
         TmpTrack.AReader = NULL;
      }
      if (TmpTrack.fp)
      {
         cdstream_destroy(TmpTrack.fp);
         TmpTrack.fp = NULL;
      }
   }
   if (opened)
      cdstream_close(&fp);
   return success;
}

/* ---- Teardown -------------------------------------------------- */

static void CDAccess_Image_Cleanup(CDAccess_Image *self)
{
   int32_t track;
   for (track = 0; track < 100; track++)
   {
      CDRFILE_TRACK_INFO *this_track = &self->Tracks[track];

      if (this_track->FirstFileInstance)
      {
         if (this_track->AReader)
         {
            AR_Close(this_track->AReader);
            this_track->AReader = NULL;
         }
         if (this_track->fp)
         {
            cdstream_destroy(this_track->fp);
            this_track->fp = NULL;
         }
      }
   }
}

/* ---- Read hot path -------------------------------------------- */

static bool CDAccess_Image_Read_Raw_Sector(CDAccess_Image *self, uint8_t *buf, int32_t lba)
{
   uint8_t SimuQ[0xC];
   int32_t track;
   CDRFILE_TRACK_INFO *ct;

   /* Leadout synthesis. */
   if (lba >= self->total_sectors)
   {
      uint8_t data_synth_mode = (self->disc_type == DISC_TYPE_CD_XA ? 0x02 : 0x01);

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
   track = CDAccess_Image_MakeSubPQ(self, lba, buf + 2352);
   /* MakeSubPQ used to throw on track-not-found; now it returns -1.
    * Treat as a hard failure same as the C++ exception did. */
   if (track < 0)
      return false;
   subq_deinterleave(buf + 2352, SimuQ);

   ct = &self->Tracks[track];

   /* Handle pregap / postgap. */
   if (lba < (ct->LBA - ct->pregap_dv) || lba >= (ct->LBA + ct->sectors))
   {
      int32_t pg_offset = lba - ct->LBA;
      CDRFILE_TRACK_INFO *et = ct;

      if (pg_offset < -150)
      {
         if ((self->Tracks[track].subq_control & SUBQ_CTRLF_DATA)
               && (self->FirstTrack < track)
               && !(self->Tracks[track - 1].subq_control & SUBQ_CTRLF_DATA))
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
         buf[12 +  6] = 0x20;
         buf[12 + 10] = 0x20;
         encode_mode2_form2_sector(lba + 150, buf);
         break;
      }
   }
   else
   {
      if (ct->AReader)
      {
         int16_t  AudioBuf[588 * 2];
         uint64_t frames_read = AR_Read(ct->AReader, (ct->FileOffset / 4) + (lba - ct->LBA) * 588, AudioBuf, 588);
         int      i;

         ct->LastSamplePos += frames_read;

         if (frames_read > 588)
            frames_read = 0;

         if (frames_read < 588)
            memset((uint8_t *)AudioBuf + frames_read * 2 * sizeof(int16_t), 0,
                  (588 - frames_read) * 2 * sizeof(int16_t));

         for (i = 0; i < 588 * 2; i++)
         {
            /* MDFN_en16lsb folded: write host int16_t as 2 LE bytes. */
            uint16_t v__ = (uint16_t)AudioBuf[i];
            buf[i * 2 + 0] = (uint8_t)v__;
            buf[i * 2 + 1] = (uint8_t)(v__ >> 8);
         }
      }
      else /* Binary */
      {
         long SeekPos   = ct->FileOffset;
         long LBARelPos = lba - ct->LBA;
         bool ok        = true;

         SeekPos += LBARelPos * DI_Size_Table[ct->DIFormat];

         if (ct->SubchannelMode)
            SeekPos += 96 * (lba - ct->LBA);

         cdstream_seek(ct->fp, SeekPos, SEEK_SET);

         /* Pre-fix: every cdstream_read in this switch was unchecked,
          * the optional sub-channel read after it was unchecked, and
          * the function returned true unconditionally even when a
          * read truncated.  The MT read thread in cdromif.c was
          * already plumbed to surface a per-sector error flag
          * (SectorBuffers[].error) up through CDIF_MT_ReadRawSector's
          * `return !error_condition;`, but with this layer always
          * reporting success the flag was always false and any short
          * read just got served as garbage to the Saturn CDB.
          *
          * Catch each read's result individually so the per-format
          * post-processing (audio byte-swap, mode1/2 sector encoding)
          * only runs on a successful read -- encoding over garbage
          * could produce a sector that *looks* like a valid mode-1
          * sector with random user-data, harder to debug than an
          * obviously-zeroed one. */
         switch (ct->DIFormat)
         {
         case DI_FORMAT_AUDIO:
            ok = (cdstream_read(ct->fp, buf, 2352) == 2352);
            if (ok && ct->RawAudioMSBFirst)
            {
               /* Endian_A16_Swap folded: byte-pair swap, unconditional
                * (RawAudioMSBFirst means the on-disk samples are stored
                * MSB-first; the Saturn expects native LSB). */
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
            ok = (cdstream_read(ct->fp, buf + 12 + 3 + 1, 2048) == 2048);
            if (ok)
               encode_mode1_sector(lba + 150, buf);
            break;
         case DI_FORMAT_MODE1_RAW:
         case DI_FORMAT_MODE2_RAW:
         case DI_FORMAT_CDI_RAW:
            ok = (cdstream_read(ct->fp, buf, 2352) == 2352);
            break;
         case DI_FORMAT_MODE2:
            ok = (cdstream_read(ct->fp, buf + 16, 2336) == 2336);
            if (ok)
               encode_mode2_sector(lba + 150, buf);
            break;
         /* FIXME: M2F1, M2F2 - does sub-header come before or after
          * user data? Standards say before. */
         case DI_FORMAT_MODE2_FORM1:
            ok = (cdstream_read(ct->fp, buf + 24, 2048) == 2048);
            break;
         case DI_FORMAT_MODE2_FORM2:
            ok = (cdstream_read(ct->fp, buf + 24, 2324) == 2324);
            break;
         }

         if (ok && ct->SubchannelMode)
            ok = (cdstream_read(ct->fp, buf + 2352, 96) == 96);

         if (!ok)
         {
            log_cb(RETRO_LOG_ERROR,
                  "CDAccess_Image: short read at lba=%d, fmt=%d\n",
                  lba, (int)ct->DIFormat);
            memset(buf, 0, 2352 + 96);
            return false;
         }
      }
   }

   return true;
}

static bool CDAccess_Image_Fast_Read_Raw_PW_TSRE(CDAccess_Image *self, uint8_t *pwbuf, int32_t lba)
{
   int32_t track;

   if (lba >= self->total_sectors)
   {
      subpw_synth_leadout_lba(&self->toc, lba, pwbuf);
      return true;
   }

   memset(pwbuf, 0, 96);
   track = CDAccess_Image_MakeSubPQ(self, lba, pwbuf);
   if (track < 0)
      return false;

   /* If TOC+BIN has embedded subchannel data, we can't fast-read it. */
   if (self->Tracks[track].SubchannelMode
         && lba >= (self->Tracks[track].LBA - self->Tracks[track].pregap_dv)
         && (lba <  self->Tracks[track].LBA + self->Tracks[track].sectors))
      return false;

   return true;
}

/* Note: makes use of the current contents (|=) of SubPWBuf. */
static int32_t CDAccess_Image_MakeSubPQ(const CDAccess_Image *self, int32_t lba, uint8_t *SubPWBuf)
{
   uint8_t   buf[0xC];
   int32_t   track;
   uint32_t  lba_relative;
   uint32_t  ma, sa, fa;
   uint32_t  m, s, f;
   uint8_t   pause_or    = 0x00;
   bool      track_found = false;
   uint8_t   adr         = 0x1; /* Q channel data encodes position */
   uint8_t   control;
   int       index;
   int32_t   i;

   for (track = self->FirstTrack; track < (self->FirstTrack + self->NumTracks); track++)
   {
      if (lba >= (self->Tracks[track].LBA - self->Tracks[track].pregap_dv - self->Tracks[track].pregap)
            && lba < (self->Tracks[track].LBA + self->Tracks[track].sectors + self->Tracks[track].postgap))
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

   control = self->Tracks[track].subq_control;

   /* Pause bit set in pre/postgap. */
   if ((lba < self->Tracks[track].LBA) || (lba >= self->Tracks[track].LBA + self->Tracks[track].sectors))
      pause_or = 0x80;

   /* Pregap between audio and data track. */
   {
      int32_t pg_offset = (int32_t)lba - self->Tracks[track].LBA;
      if (pg_offset < -150)
      {
         if ((self->Tracks[track].subq_control & SUBQ_CTRLF_DATA)
               && (self->FirstTrack < track)
               && !(self->Tracks[track - 1].subq_control & SUBQ_CTRLF_DATA))
            control = self->Tracks[track - 1].subq_control;
      }
   }

   memset(buf, 0, 0xC);
   buf[0] = (adr << 0) | (control << 4);
   buf[1] = U8_to_BCD(track);

   index = 0;
   for (i = 0; i < 100; i++)
   {
      if (lba >= self->Tracks[track].index[i])
         index = i;
   }
   buf[2] = U8_to_BCD(index);

   /* Track-relative MSF */
   buf[3] = U8_to_BCD(m);
   buf[4] = U8_to_BCD(s);
   buf[5] = U8_to_BCD(f);

   buf[6] = 0;

   /* Absolute MSF */
   buf[7] = U8_to_BCD(ma);
   buf[8] = U8_to_BCD(sa);
   buf[9] = U8_to_BCD(fa);

   subq_generate_checksum(buf);

   if (!subq_map_empty(&self->SubQReplaceMap))
   {
      const uint8_t *replace = subq_map_find(&self->SubQReplaceMap, LBA_to_ABA(lba));
      if (replace)
         memcpy(buf, replace, 12);
   }

   {
      int j;
      for (j = 0; j < 96; j++)
         SubPWBuf[j] |= (((buf[j >> 3] >> (7 - (j & 0x7))) & 1) ? 0x40 : 0x00) | pause_or;
   }

   return track;
}

static bool CDAccess_Image_Read_TOC(CDAccess_Image *self, TOC *rtoc)
{
   *rtoc = self->toc;
   return true;
}

static void CDAccess_Image_GenerateTOC(CDAccess_Image *self)
{
   int i;
   TOC_Clear(&self->toc);

   self->toc.first_track = self->FirstTrack;
   self->toc.last_track  = self->FirstTrack + self->NumTracks - 1;
   self->toc.disc_type   = self->disc_type;

   for (i = self->FirstTrack; i < self->FirstTrack + self->NumTracks; i++)
   {
      if (self->Tracks[i].DIFormat == DI_FORMAT_CDI_RAW)
      {
         self->toc.first_track = (99 < (i + 1)) ? 99 : (i + 1);
         self->toc.last_track  = (self->toc.first_track > self->toc.last_track)
            ? self->toc.first_track : self->toc.last_track;
      }

      self->toc.tracks[i].lba     = self->Tracks[i].LBA;
      self->toc.tracks[i].adr     = ADR_CURPOS;
      self->toc.tracks[i].control = self->Tracks[i].subq_control;
      self->toc.tracks[i].valid   = true;
   }

   self->toc.tracks[100].lba     = self->total_sectors;
   self->toc.tracks[100].adr     = ADR_CURPOS;
   self->toc.tracks[100].control = self->Tracks[self->FirstTrack + self->NumTracks - 1].subq_control;
   self->toc.tracks[100].valid   = true;
}

/* ---------------------------------------------------------------- */
/* CDAccess vtable adapters and factory.                            */
/* ---------------------------------------------------------------- */

static bool CDAccess_Image_RRS_vt(CDAccess *base, uint8_t *buf, int32_t lba)
{
   return CDAccess_Image_Read_Raw_Sector((CDAccess_Image *)base, buf, lba);
}

static bool CDAccess_Image_FRPT_vt(CDAccess *base, uint8_t *pwbuf, int32_t lba)
{
   return CDAccess_Image_Fast_Read_Raw_PW_TSRE((CDAccess_Image *)base, pwbuf, lba);
}

static bool CDAccess_Image_RTOC_vt(CDAccess *base, TOC *toc)
{
   return CDAccess_Image_Read_TOC((CDAccess_Image *)base, toc);
}

static void CDAccess_Image_destroy_vt(CDAccess *base)
{
   CDAccess_Image *self = (CDAccess_Image *)base;
   CDAccess_Image_Cleanup(self);
   free(self);
}

CDAccess *CDAccess_Image_New(const char *path, bool image_memcache)
{
   CDAccess_Image *self = (CDAccess_Image *)calloc(1, sizeof(*self));
   if (!self)
      return NULL;

   /* Defensive zero of the toc member - GenerateTOC clears before
    * populating, but if Load fails partway through and an accessor
    * runs anyway, an uninitialized TOC could surface.  subq_map
    * needs explicit clear too - plain POD struct, no default ctor. */
   TOC_Clear(&self->toc);
   subq_map_clear(&self->SubQReplaceMap);

   if (!CDAccess_Image_ImageOpen(self, path, image_memcache))
   {
      /* ImageOpen may have partially populated cdstream/AReader
       * across multiple Tracks[] entries before failing - run
       * cleanup so we don't leak. */
      CDAccess_Image_Cleanup(self);
      free(self);
      return NULL;
   }

   self->base.Read_Raw_Sector       = CDAccess_Image_RRS_vt;
   self->base.Fast_Read_Raw_PW_TSRE = CDAccess_Image_FRPT_vt;
   self->base.Read_TOC              = CDAccess_Image_RTOC_vt;
   self->base.destroy               = CDAccess_Image_destroy_vt;
   return &self->base;
}
