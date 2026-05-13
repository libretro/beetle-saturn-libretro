/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __MDFN_CDROM_CDROMIF_H
#define __MDFN_CDROM_CDROMIF_H

#include <stdint.h>
#include <boolean.h>

#include "CDUtility.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CDIF: thin wrapper around a CDAccess backend with optional
 * background read-ahead.  Previously a C++ class hierarchy
 * (CDIF abstract base + CDIF_MT for the threaded reader +
 * CDIF_ST for the synchronous reader); now a single opaque
 * struct tagged by an internal is_mt flag.  All callers go
 * through the function API below; no field-level access. */
struct CDIF;
typedef struct CDIF CDIF;

/* LBA bounds the readers will service.  Outside this range
 * the read functions return false without touching the
 * backend - same behaviour as the C++ readers. */
#define CDIF_LBA_Read_Minimum  (-150)
#define CDIF_LBA_Read_Maximum  (449849) /* 100 * 75 * 60 - 150 - 1 */

/* Construct a CDIF for the given disc image path.  Picks MT
 * (background read-ahead) when image_memcache is false, ST
 * (synchronous) when true.  Returns NULL if the underlying
 * CDAccess_Open fails or if the loaded TOC has bad
 * first/last-track numbers. */
CDIF *CDIF_Open(const char *path, bool image_memcache);

/* Tear down a CDIF: joins the read thread (MT only), frees the
 * underlying CDAccess via its destroy slot, releases the ring
 * buffer and sync primitives. */
void CDIF_Close(CDIF *cdif);

/* Copy the cached disc TOC into *out.  Was an inline accessor on
 * the C++ class; now a free function so the header doesn't have to
 * expose the struct layout. */
void CDIF_ReadTOC(CDIF *cdif, TOC *out);

/* MT-only: hint the read thread to read-ahead from lba.  No-op
 * in ST mode. */
void CDIF_HintReadSector(CDIF *cdif, int32_t lba);

/* Read 2352 main + 96 subchannel = 2448 bytes for the sector at
 * lba into buf.  Blocking. Returns false on out-of-range lba
 * (and zeros buf in that case). */
bool CDIF_ReadRawSector(CDIF *cdif, uint8_t *buf, int32_t lba);

/* Read just the 96 bytes of P+W subchannel for the sector at lba
 * into pwbuf.  Uses the backend's fast-synth path when available;
 * falls back to a full ReadRawSector otherwise.  hint_fullread
 * additionally nudges the read-ahead thread in MT mode. */
bool CDIF_ReadRawSectorPWOnly(CDIF *cdif, uint8_t *pwbuf, int32_t lba, bool hint_fullread);

/* Validate a 2352-byte sector via EDC/ECC.  Mode-1 / mode-2-form-1
 * only.  Returns true if intact (or correctable). */
bool CDIF_ValidateRawSector(uint8_t *buf);

/* Read sector_count mode-1 / mode-2-form-1 user-data sectors
 * (2048 B each) into buf starting at lba.  Returns the mode of the
 * first sector (1 or 2) on success, 0 on error. */
int CDIF_ReadSector(CDIF *cdif, uint8_t *buf, int32_t lba, uint32_t sector_count);

#ifdef __cplusplus
}
#endif

#endif
