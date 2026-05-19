/******************************************************************************/
/* Beetle Saturn libretro -- minimal ZIP reader                               */
/******************************************************************************/
/* zip_reader.h:
**
** Small in-tree ZIP reader for the ST-V cart loading path.  Parses the
** ZIP file format (PKWARE APPNOTE) and decompresses entries using the
** existing in-tree zlib's inflate() primitive.  No new third-party code
** is introduced -- only the ZIP container parsing is original.
**
** Limitations (matching ST-V ROM zip realities, not general ZIP support):
**   - Reads from a regular file (RFILE) only -- no in-memory or
**     streaming sources.
**   - Compression methods supported: 0 (stored) and 8 (deflate).
**     LZMA / BZIP2 / WavPack / etc. return failure; ST-V ROM zips
**     never use these.
**   - ZIP64 (>4 GiB files) not supported; ST-V ROM zips are well
**     under that.
**   - Encrypted entries fail; ST-V ROM zips are never encrypted.
**   - Multi-volume / spanned archives not supported.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
*/

#ifndef __BEETLE_SATURN_ZIP_READER_H
#define __BEETLE_SATURN_ZIP_READER_H

#include <stdint.h>
#include <stddef.h>
#include <boolean.h>

#include <streams/file_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Filename cap matches the central-directory filename length field
 * (uint16_t) but in practice ST-V ROM names are <32 chars; 256 leaves
 * headroom while keeping struct size bounded for inline arrays. */
#define ZIP_MAX_NAME 256

struct zip_entry
{
   char     name[ZIP_MAX_NAME];
   uint32_t compressed_size;
   uint32_t uncompressed_size;
   uint32_t local_header_offset;  /* offset into the file where the LFH lives */
   uint32_t crc32;
   uint16_t method;               /* 0 = stored, 8 = deflate */
};

typedef struct zip_archive
{
   RFILE            *fp;
   struct zip_entry *entries;
   size_t            entry_count;
} zip_archive;

/* Open `path` and parse its central directory.  On success returns
 * true with `out` populated; on failure returns false and `out` is
 * zeroed (safe to call zip_close on a failed-open archive). */
bool zip_open(zip_archive *out, const char *path);

/* Tear down: closes the file and frees the entry table.  Safe to
 * call on a zip_archive that's already zeroed (no-op). */
void zip_close(zip_archive *za);

/* Find an entry by case-insensitive filename match (the common ZIP
 * usage convention -- frontends and tools treat zip-internal names
 * as case-insensitive even though the format technically allows
 * case-sensitive names).  Returns NULL if not found. */
const struct zip_entry *zip_find(const zip_archive *za, const char *name);

/* Extract `e` into the caller-supplied `dst` buffer of size
 * `e->uncompressed_size`.  Verifies the entry's CRC-32 matches what
 * the central directory recorded.  Returns true on success. */
bool zip_extract(zip_archive *za, const struct zip_entry *e, uint8_t *dst);

#ifdef __cplusplus
}
#endif

#endif
