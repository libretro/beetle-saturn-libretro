/******************************************************************************/
/* Beetle Saturn libretro -- minimal ZIP reader                               */
/******************************************************************************/
/* zip_reader.c:
**
** See zip_reader.h for scope notes.  This TU is the entire ZIP-format
** parser; it consumes only zlib's inflate primitive and libretro-common's
** filestream wrappers.
**
** ZIP format references:
**   PKWARE APPNOTE 6.3.x (the format spec) -- field offsets and the
**   EOCD search-from-end protocol used here both come from there.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
*/

#include "zip_reader.h"

#include <stdlib.h>
#include <string.h>

#include <streams/file_stream.h>

#include "zlib.h"

/* ---- zlib allocator hooks ---- */

/* This in-tree zlib build's inflateInit2_ returns Z_STREAM_ERROR if
 * strm->zalloc / strm->zfree are not set (rather than auto-defaulting
 * to system malloc/free as some other builds do).  Provide trivial
 * libc wrappers; libchdr (the other in-tree zlib consumer) uses the
 * same pattern with its own arena allocator -- a plain libc wrap is
 * sufficient for ST-V ROM extraction since the lifetime is bounded
 * to one zip_extract call and the working-set is the inflate state
 * struct (~7 KiB) plus the per-block sliding window. */

static voidpf z_alloc_fn(voidpf opaque, uInt items, uInt size)
{
   (void)opaque;
   return (voidpf)calloc(items, size);
}

static void z_free_fn(voidpf opaque, voidpf addr)
{
   (void)opaque;
   free(addr);
}

/* ---- ZIP record signatures ---- */
#define ZIP_SIG_EOCD  0x06054b50u   /* End of Central Directory record */
#define ZIP_SIG_CD    0x02014b50u   /* Central Directory file header */
#define ZIP_SIG_LFH   0x04034b50u   /* Local File Header */

/* EOCD search window: the EOCD record is at the very end of the file,
 * but may be followed by a comment of up to 65535 bytes.  Plus 22 bytes
 * for the fixed EOCD record itself = 65557 byte max scan window. */
#define EOCD_MAX_SEARCH 65557

/* Compression methods.  Anything outside this set returns failure. */
#define METHOD_STORED   0
#define METHOD_DEFLATE  8

/* ---- Little-endian field readers ---- */

static uint16_t read_le16(const uint8_t *p)
{
   return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t read_le32(const uint8_t *p)
{
   return (uint32_t)p[0]
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
}

/* ---- EOCD location ---- */

/* Scan backward from the end of `fp` looking for the EOCD signature.
 * Returns the absolute file offset of the signature on success, or
 * (int64_t)-1 on not-found / read error.
 *
 * The PKWARE spec doesn't guarantee a unique EOCD signature in the
 * comment bytes that follow it, so we accept the *last* match within
 * the search window -- that's where the genuine EOCD lives. */
static int64_t find_eocd(RFILE *fp, int64_t fsize)
{
   int64_t   scan_start;
   int64_t   scan_len;
   uint8_t  *buf;
   int64_t   read_n;
   int64_t   match = -1;
   int64_t   i;

   if (fsize < 22)
      return -1;

   /* Compute scan window: last EOCD_MAX_SEARCH bytes, or the whole
    * file if smaller. */
   scan_start = fsize - EOCD_MAX_SEARCH;
   if (scan_start < 0)
      scan_start = 0;
   scan_len = fsize - scan_start;

   buf = (uint8_t*)malloc((size_t)scan_len);
   if (!buf)
      return -1;

   if (filestream_seek(fp, scan_start, SEEK_SET) < 0)
   {
      free(buf);
      return -1;
   }

   read_n = filestream_read(fp, buf, scan_len);
   if (read_n != scan_len)
   {
      free(buf);
      return -1;
   }

   /* Scan forward looking for the signature, keeping the LAST hit.
    * 4-byte signature + 18 trailing fixed bytes; need at least 22
    * bytes after the signature for it to be a valid EOCD. */
   for (i = 0; i + 22 <= read_n; i++)
   {
      if (read_le32(buf + i) == ZIP_SIG_EOCD)
         match = scan_start + i;
   }

   free(buf);
   return match;
}

/* ---- Central directory parse ---- */

/* Read the central directory at `cd_off` (size `cd_size`, `entry_count`
 * entries) and populate `out_entries` (heap-allocated).  Caller frees.
 * Returns true on success.
 *
 * Notes on robustness:
 *  - Filename length is bounded to ZIP_MAX_NAME-1 (truncation makes the
 *    entry effectively un-locatable by name, but we keep it in the
 *    table so the entry_count matches the header).
 *  - The "general purpose bit flag" bit 0 (encryption) is rejected.
 *  - Method must be STORED or DEFLATE; other methods are kept in the
 *    table with their method value preserved (zip_extract returns
 *    failure when extracting them). */
static bool read_central_directory(RFILE *fp,
      int64_t cd_off, uint32_t cd_size, uint16_t entry_count,
      struct zip_entry **out_entries, size_t *out_count)
{
   uint8_t          *cd_buf;
   struct zip_entry *entries;
   size_t            i;
   size_t            cursor = 0;

   *out_entries = NULL;
   *out_count   = 0;

   if (cd_size == 0 || entry_count == 0)
      return true;  /* empty CD is valid */

   cd_buf = (uint8_t*)malloc(cd_size);
   if (!cd_buf)
      return false;

   if (filestream_seek(fp, cd_off, SEEK_SET) < 0
    || filestream_read(fp, cd_buf, cd_size) != (int64_t)cd_size)
   {
      free(cd_buf);
      return false;
   }

   entries = (struct zip_entry*)calloc(entry_count, sizeof(*entries));
   if (!entries)
   {
      free(cd_buf);
      return false;
   }

   /* CD entry layout (46-byte fixed header, then variable):
    *   off  size  field
    *   0    4     signature 0x02014b50
    *   8    2     general purpose bit flag
    *  10    2     compression method
    *  16    4     CRC-32
    *  20    4     compressed size
    *  24    4     uncompressed size
    *  28    2     file name length
    *  30    2     extra field length
    *  32    2     file comment length
    *  42    4     local header offset
    *  46    n     file name
    *  46+n m     extra field
    *  46+n+m k  file comment
    */
   for (i = 0; i < entry_count; i++)
   {
      uint16_t flags, method, nlen, xlen, clen;
      uint32_t crc, csize, usize, lho;
      size_t   copy_len;

      if (cursor + 46 > cd_size)
         goto fail;
      if (read_le32(cd_buf + cursor) != ZIP_SIG_CD)
         goto fail;

      flags  = read_le16(cd_buf + cursor + 8);
      method = read_le16(cd_buf + cursor + 10);
      crc    = read_le32(cd_buf + cursor + 16);
      csize  = read_le32(cd_buf + cursor + 20);
      usize  = read_le32(cd_buf + cursor + 24);
      nlen   = read_le16(cd_buf + cursor + 28);
      xlen   = read_le16(cd_buf + cursor + 30);
      clen   = read_le16(cd_buf + cursor + 32);
      lho    = read_le32(cd_buf + cursor + 42);

      /* Encrypted entries (bit 0 of general purpose flag): reject. */
      if (flags & 0x1)
         goto fail;

      if (cursor + 46 + nlen + xlen + clen > cd_size)
         goto fail;

      copy_len = nlen;
      if (copy_len >= ZIP_MAX_NAME)
         copy_len = ZIP_MAX_NAME - 1;
      memcpy(entries[i].name, cd_buf + cursor + 46, copy_len);
      entries[i].name[copy_len]      = '\0';
      entries[i].method              = method;
      entries[i].compressed_size     = csize;
      entries[i].uncompressed_size   = usize;
      entries[i].crc32               = crc;
      entries[i].local_header_offset = lho;

      cursor += 46 + nlen + xlen + clen;
   }

   free(cd_buf);
   *out_entries = entries;
   *out_count   = entry_count;
   return true;

fail:
   free(cd_buf);
   free(entries);
   return false;
}

/* ---- Public API ---- */

bool zip_open(zip_archive *out, const char *path)
{
   uint8_t  eocd_rec[22];
   int64_t  fsize;
   int64_t  eocd_off;
   uint16_t entry_count;
   uint32_t cd_size;
   uint32_t cd_off;

   memset(out, 0, sizeof(*out));

   out->fp = filestream_open(path,
         RETRO_VFS_FILE_ACCESS_READ,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);
   if (!out->fp)
      return false;

   /* filestream_get_size returns the underlying file size directly --
    * cleaner than seek-to-end + tell and avoids any seek-position
    * ambiguity in the VFS implementation. */
   fsize = filestream_get_size(out->fp);
   if (fsize < 22)
      goto fail;

   eocd_off = find_eocd(out->fp, fsize);
   if (eocd_off < 0)
      goto fail;

   if (filestream_seek(out->fp, eocd_off, SEEK_SET) < 0
    || filestream_read(out->fp, eocd_rec, sizeof(eocd_rec)) != (int64_t)sizeof(eocd_rec))
      goto fail;

   /* EOCD fields (after the 4-byte signature):
    *   off  size  field
    *   4    2     disk number
    *   6    2     disk where CD starts
    *   8    2     entries on this disk
    *  10    2     total entries
    *  12    4     CD size
    *  16    4     CD offset (from start of disk)
    *  20    2     comment length
    */
   entry_count = read_le16(eocd_rec + 10);
   cd_size     = read_le32(eocd_rec + 12);
   cd_off      = read_le32(eocd_rec + 16);

   /* Multi-disk archives: refuse.  ST-V ROM zips are always single-volume. */
   if (read_le16(eocd_rec + 4) != 0 || read_le16(eocd_rec + 6) != 0)
      goto fail;
   if (read_le16(eocd_rec + 8) != entry_count)
      goto fail;

   if ((int64_t)cd_off + (int64_t)cd_size > fsize)
      goto fail;

   if (!read_central_directory(out->fp,
         (int64_t)cd_off, cd_size, entry_count,
         &out->entries, &out->entry_count))
      goto fail;

   return true;

fail:
   zip_close(out);
   return false;
}

void zip_close(zip_archive *za)
{
   if (!za)
      return;
   if (za->fp)
   {
      filestream_close(za->fp);
      za->fp = NULL;
   }
   if (za->entries)
   {
      free(za->entries);
      za->entries = NULL;
   }
   za->entry_count = 0;
}

/* Case-insensitive ASCII compare (no locale assumptions; ZIP names
 * are typically pure ASCII per APPNOTE, and even when UTF-8 the
 * ASCII subset is what we care about for ST-V ROM filename matching). */
static int ascii_strcasecmp(const char *a, const char *b)
{
   while (*a && *b)
   {
      int ca = (unsigned char)*a;
      int cb = (unsigned char)*b;
      if (ca >= 'A' && ca <= 'Z') ca += 32;
      if (cb >= 'A' && cb <= 'Z') cb += 32;
      if (ca != cb) return ca - cb;
      a++; b++;
   }
   return (unsigned char)*a - (unsigned char)*b;
}

const struct zip_entry *zip_find(const zip_archive *za, const char *name)
{
   size_t i;
   if (!za || !za->entries || !name)
      return NULL;
   for (i = 0; i < za->entry_count; i++)
   {
      if (!ascii_strcasecmp(za->entries[i].name, name))
         return &za->entries[i];
   }
   return NULL;
}

/* ---- Extraction ---- */

/* Skip the variable-length suffix (file name + extra field) of an
 * already-validated local file header so the stream is positioned at
 * the start of the compressed data.  The LFH's lengths can differ
 * from the CD's (the spec only ties the *file name* contents, not
 * lengths) but in practice they match; we use the LFH's own values
 * here, the source of truth at the data boundary. */
static bool seek_to_compressed_data(RFILE *fp, uint32_t lho)
{
   uint8_t  lfh[30];
   uint16_t nlen;
   uint16_t xlen;

   if (filestream_seek(fp, lho, SEEK_SET) < 0)
      return false;
   if (filestream_read(fp, lfh, sizeof(lfh)) != (int64_t)sizeof(lfh))
      return false;
   if (read_le32(lfh) != ZIP_SIG_LFH)
      return false;

   /* LFH fields we care about for stream-positioning only:
    *   off  size  field
    *  26    2     file name length
    *  28    2     extra field length
    */
   nlen = read_le16(lfh + 26);
   xlen = read_le16(lfh + 28);

   return filestream_seek(fp, (int64_t)lho + 30 + nlen + xlen, SEEK_SET) >= 0;
}

static bool extract_stored(RFILE *fp,
      const struct zip_entry *e, uint8_t *dst)
{
   /* STORED: compressed_size == uncompressed_size; just copy. */
   if (e->compressed_size != e->uncompressed_size)
      return false;
   if (filestream_read(fp, dst, e->uncompressed_size)
         != (int64_t)e->uncompressed_size)
      return false;
   return true;
}

static bool extract_deflate(RFILE *fp,
      const struct zip_entry *e, uint8_t *dst)
{
   /* Use raw deflate (no zlib header / no gzip header): inflateInit2
    * with negative windowBits.  Stream the compressed data in chunks
    * so we don't need to allocate the entire compressed payload.
    *
    * Chunk size is a balance: too small means lots of syscalls; too
    * large wastes memory for tiny files.  64 KiB is comfortable for
    * ST-V ROM sizes (typically 256 KiB - 4 MiB compressed). */
#define CHUNK 65536
   uint8_t   in_buf[CHUNK];
   z_stream  strm;
   uint32_t  remaining = e->compressed_size;
   int       zret;
   bool      ok = false;

   memset(&strm, 0, sizeof(strm));
   strm.zalloc = z_alloc_fn;
   strm.zfree  = z_free_fn;
   if (inflateInit2(&strm, -15) != Z_OK)
      return false;

   strm.next_out  = dst;
   strm.avail_out = e->uncompressed_size;

   while (remaining > 0 || strm.avail_in > 0)
   {
      if (strm.avail_in == 0 && remaining > 0)
      {
         uint32_t want = (remaining < CHUNK) ? remaining : CHUNK;
         int64_t  got  = filestream_read(fp, in_buf, want);
         if (got <= 0)
            goto done;
         strm.next_in  = in_buf;
         strm.avail_in = (uInt)got;
         remaining    -= (uint32_t)got;
      }

      zret = inflate(&strm, Z_NO_FLUSH);
      if (zret == Z_STREAM_END)
      {
         ok = (strm.total_out == e->uncompressed_size);
         goto done;
      }
      if (zret != Z_OK)
         goto done;
      if (strm.avail_out == 0 && strm.total_out < e->uncompressed_size)
         goto done;  /* would overflow output */
   }
   /* Ran out of input without Z_STREAM_END: bad stream. */

done:
   inflateEnd(&strm);
   return ok;
#undef CHUNK
}

bool zip_extract(zip_archive *za, const struct zip_entry *e, uint8_t *dst)
{
   if (!za || !za->fp || !e || !dst)
      return false;

   if (!seek_to_compressed_data(za->fp, e->local_header_offset))
      return false;

   if (e->method == METHOD_STORED)
   {
      if (!extract_stored(za->fp, e, dst))
         return false;
   }
   else if (e->method == METHOD_DEFLATE)
   {
      if (!extract_deflate(za->fp, e, dst))
         return false;
   }
   else
   {
      /* LZMA / BZIP2 / etc.: refuse.  Real ST-V zips never use these. */
      return false;
   }

   /* Verify CRC-32 against what the central directory recorded.  This
    * is the canonical integrity check the ZIP spec defines; a mismatch
    * means corruption or truncation between the time the archive was
    * built and now. */
   {
      uLong got = crc32(0L, dst, e->uncompressed_size);
      if ((uint32_t)got != e->crc32)
         return false;
   }

   return true;
}
