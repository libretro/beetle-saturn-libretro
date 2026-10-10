/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* CD reader: one emulation producer and one I/O worker. Requests use
 * an atomic mailbox (only one demand can be outstanding); advisory
 * hints coalesce. Sector ownership moves through SPSC index queues.
 * The worker alone touches the backend and the emulation thread alone
 * owns the direct-mapped cache. No shared sector is concurrently copied
 * and rewritten, and no sequence-counter retry over non-atomic data is
 * needed. */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include <boolean.h>
#ifdef WANT_THREADING
#include <rthreads/rthreads.h>
#include "../thread_eventcount.h"
#endif
#include <retro_miscellaneous.h>
#include <libretro.h>

#include "CDUtility.h"
#include "CDAccess.h"
#include "cdromif.h"

extern retro_log_printf_t log_cb;

/* ------------------------------------------------------------------
 * Sector ring buffer (MT mode only). */

#define CDIF_SBSIZE 256

typedef struct CDIF_Sector_Buffer
{
   bool    valid;
   bool    error;
   int32_t lba;
   uint8_t data[2352 + 96];
} CDIF_Sector_Buffer;

/* ------------------------------------------------------------------
 * CDIF.  Common state at the top; MT-only state at the bottom. */

struct CDIF
{
   bool      is_mt;
   CDAccess *disc_cdaccess;
   TOC       disc_toc;

#ifdef WANT_THREADING
   /* MT-only.  Untouched in ST instances. */
   sthread_t *CDReadThread;

   /* Indices transfer ownership of these fixed pool slots. */
   CDIF_Sector_Buffer SectorPool[32];
   CDIF_Sector_Buffer SectorBuffers[CDIF_SBSIZE]; /* emulation-owned */
   retro_waitable_spsc_t ready_sectors; /* worker -> emulation */
   retro_waitable_spsc_t free_sectors;  /* emulation -> worker */
   retro_eventcount_t work;
   retro_atomic_int_t request_lba, request_seq, hint_lba, hint_seq;
   retro_atomic_int_t stopping;
   unsigned request_local, hint_local; /* emulation-owned */

   /* Read-thread-only state.  Touched only by the background
    * thread; no synchronization needed. */
   int32_t ra_lba;
   int32_t ra_count;
   int32_t last_read_lba;
#endif
};

/* Forward declaration for the read-thread entry point. */
#ifdef WANT_THREADING
static void CDIF_ReadThreadStart(void *v_arg);
#endif

/* ------------------------------------------------------------------
 * Public API. */

void CDIF_ReadTOC(CDIF *cdif, TOC *out)
{
   *out = cdif->disc_toc;
}

static bool CDIF_ValidateRawSector(uint8_t *buf)
{
   int mode = buf[12 + 3];

   if (mode != 0x1 && mode != 0x2)
      return false;

   if (!edc_lec_check_and_correct(buf, mode == 2))
      return false;

   return true;
}

int CDIF_ReadSector(CDIF *cdif, uint8_t *buf, int32_t lba, uint32_t sector_count)
{
   int ret = 0;

   while (sector_count--)
   {
      uint8_t tmpbuf[2352 + 96];
      int     mode;

      if (!CDIF_ReadRawSector(cdif, tmpbuf, lba))
      {
         log_cb(RETRO_LOG_ERROR, "CDIF Raw Read error\n");
         return 0;
      }

      if (!CDIF_ValidateRawSector(tmpbuf))
         return 0;

      mode = tmpbuf[12 + 3];

      if (!ret)
         ret = mode;

      if (mode == 1)
         memcpy(buf, &tmpbuf[12 + 4], 2048);
      else if (mode == 2)
         memcpy(buf, &tmpbuf[12 + 4 + 8], 2048);
      else
         return 0;

      buf += 2048;
      lba++;
   }

   return ret;
}

/* ------------------------------------------------------------------
 * MT path: ReadThread + the three method bodies. */

#ifdef WANT_THREADING
static bool CDIF_TakeSector(CDIF *self, bool blocking)
{
   unsigned slot;
   CDIF_Sector_Buffer *source, *cache;
   if (blocking)
   {
      while (!retro_waitable_spsc_wait_readable(&self->ready_sectors, sizeof(slot), 1000000))
         if (retro_atomic_load_acquire_int(&self->stopping)) return false;
   }
   else if (retro_spsc_read_avail(&self->ready_sectors.queue) < sizeof(slot))
      return false;
   retro_waitable_spsc_read(&self->ready_sectors, &slot, sizeof(slot));
   source = &self->SectorPool[slot];
   cache = &self->SectorBuffers[(uint32_t)source->lba & (CDIF_SBSIZE - 1)];
   *cache = *source;
   /* The read of ready_sectors acquired the worker's payload; returning
    * the index releases it only after the snapshot is complete. */
   retro_waitable_spsc_write(&self->free_sectors, &slot, sizeof(slot));
   return true;
}

static void CDIF_MT_Hint(CDIF *self, int32_t lba)
{
   if (lba < CDIF_LBA_Read_Minimum || lba > CDIF_LBA_Read_Maximum) return;
   while (CDIF_TakeSector(self, false)) {}
   retro_atomic_store_release_int(&self->hint_lba, lba);
   retro_atomic_store_release_int(&self->hint_seq, (int)++self->hint_local);
   retro_eventcount_notify(&self->work);
}

static void CDIF_MT_ReadThreadBody(CDIF *self)
{
   unsigned request_seen = 0, hint_seen = 0;
   bool demanded = false;
   self->ra_count = 0;
   self->last_read_lba = CDIF_LBA_Read_Maximum + 1;
   while (!retro_atomic_load_acquire_int(&self->stopping))
   {
      unsigned request = (unsigned)retro_atomic_load_acquire_int(&self->request_seq);
      unsigned hint = (unsigned)retro_atomic_load_acquire_int(&self->hint_seq);
      if (request != request_seen)
      {
         request_seen = request;
         hint_seen = hint;
         self->ra_lba = retro_atomic_load_acquire_int(&self->request_lba);
         self->last_read_lba = self->ra_lba;
         self->ra_count = 17; /* demanded sector followed by bounded read-ahead */
         demanded = true;
      }
      else if (hint != hint_seen)
      {
         int32_t lba = retro_atomic_load_acquire_int(&self->hint_lba);
         hint_seen = hint;
         if (lba != self->last_read_lba)
         {
            if (lba == self->last_read_lba + 1 && self->ra_lba > lba && self->ra_lba <= lba + 16)
               self->ra_count = lba + 17 - self->ra_lba;
            else
            {
               self->ra_lba = lba;
               self->ra_count = 17;
            }
            self->last_read_lba = lba;
         }
      }
      if (self->ra_count && self->ra_lba <= CDIF_LBA_Read_Maximum)
      {
         unsigned slot;
         CDIF_Sector_Buffer *sector;
         if (demanded)
         {
            while (!retro_waitable_spsc_wait_readable(&self->free_sectors, sizeof(slot), 1000000))
               if (retro_atomic_load_acquire_int(&self->stopping)) return;
         }
         else if (retro_spsc_read_avail(&self->free_sectors.queue) < sizeof(slot))
         {
            self->ra_count = 0; /* speculative work must not obstruct a demand */
            continue;
         }
         retro_waitable_spsc_read(&self->free_sectors, &slot, sizeof(slot));
         sector = &self->SectorPool[slot];
         memset(sector->data, 0, sizeof(sector->data));
         sector->error = !self->disc_cdaccess->Read_Raw_Sector(self->disc_cdaccess,
               sector->data, self->ra_lba);
         sector->lba = self->ra_lba++;
         sector->valid = true;
         self->ra_count--;
         demanded = false;
         /* At most 32 pool indices exist; this queue always fits the
          * index whose slot we just took from free_sectors. */
         retro_waitable_spsc_write(&self->ready_sectors, &slot, sizeof(slot));
      }
      else
      {
         int key;
         self->ra_count = 0;
         key = retro_eventcount_prepare_wait(&self->work);
         if (retro_atomic_load_acquire_int(&self->stopping) ||
             (unsigned)retro_atomic_load_acquire_int(&self->request_seq) != request_seen ||
             (unsigned)retro_atomic_load_acquire_int(&self->hint_seq) != hint_seen)
            retro_eventcount_cancel_wait(&self->work);
         else
            retro_eventcount_commit_wait(&self->work, key);
      }
   }
}

static void CDIF_ReadThreadStart(void *v_arg)
{
   CDIF_MT_ReadThreadBody((CDIF *)v_arg);
}

static bool CDIF_MT_ReadRawSector(CDIF *self, uint8_t *buf, int32_t lba)
{
   CDIF_Sector_Buffer *cache = &self->SectorBuffers[(uint32_t)lba & (CDIF_SBSIZE - 1)];
   if (lba < CDIF_LBA_Read_Minimum || lba > CDIF_LBA_Read_Maximum)
   {
      memset(buf, 0, 2352 + 96);
      return false;
   }
   while (CDIF_TakeSector(self, false)) {}
   if (!(cache->valid && cache->lba == lba))
   {
      retro_atomic_store_release_int(&self->request_lba, lba);
      retro_atomic_store_release_int(&self->request_seq, (int)++self->request_local);
      retro_eventcount_notify(&self->work);
      while (!cache->valid || cache->lba != lba)
         if (!CDIF_TakeSector(self, true)) return false;
   }
   {
      bool ok = !cache->error;
      memcpy(buf, cache->data, sizeof(cache->data));
      CDIF_MT_Hint(self, lba);
      return ok;
   }
}

static bool CDIF_MT_ReadRawSectorPWOnly(CDIF *self, uint8_t *pwbuf, int32_t lba, bool hint_fullread)
{
   if (lba < CDIF_LBA_Read_Minimum || lba > CDIF_LBA_Read_Maximum)
   {
      memset(pwbuf, 0, 96);
      return false;
   }

   if (self->disc_cdaccess->Fast_Read_Raw_PW_TSRE(self->disc_cdaccess, pwbuf, lba))
   {
      if (hint_fullread)
         CDIF_MT_Hint(self, lba);
      return true;
   }
   else
   {
      uint8_t tmpbuf[2352 + 96];
      bool    ret;

      ret = CDIF_MT_ReadRawSector(self, tmpbuf, lba);
      memcpy(pwbuf, tmpbuf + 2352, 96);
      return ret;
   }
}

#endif

/* ------------------------------------------------------------------
 * ST path: synchronous reads, no thread.  Identical bodies to the
 * C++ CDIF_ST methods. */

static bool CDIF_ST_ReadRawSector(CDIF *self, uint8_t *buf, int32_t lba)
{
   if (lba < CDIF_LBA_Read_Minimum || lba > CDIF_LBA_Read_Maximum)
   {
      memset(buf, 0, 2352 + 96);
      return false;
   }

   /* Pre-fix: ignored Read_Raw_Sector's return and unconditionally
    * returned true.  The MT counterpart routes failure through
    * SectorBuffers[].error; the ST path has no sector buffer to
    * stash it in, so just propagate the bool directly. */
   return self->disc_cdaccess->Read_Raw_Sector(self->disc_cdaccess, buf, lba);
}

static bool CDIF_ST_ReadRawSectorPWOnly(CDIF *self, uint8_t *pwbuf, int32_t lba, bool hint_fullread)
{
   (void)hint_fullread;  /* ST has no read thread to hint. */

   if (lba < CDIF_LBA_Read_Minimum || lba > CDIF_LBA_Read_Maximum)
   {
      memset(pwbuf, 0, 96);
      return false;
   }

   if (self->disc_cdaccess->Fast_Read_Raw_PW_TSRE(self->disc_cdaccess, pwbuf, lba))
      return true;
   else
   {
      uint8_t tmpbuf[2352 + 96];
      bool    ret;

      ret = CDIF_ST_ReadRawSector(self, tmpbuf, lba);
      memcpy(pwbuf, tmpbuf + 2352, 96);
      return ret;
   }
}

/* ------------------------------------------------------------------
 * Dispatch on is_mt.  At -O2 the runtime branch is well-predicted
 * (a given CDIF instance only ever takes one side) and cheaper than
 * the C++ virtual call it replaces. */

void CDIF_HintReadSector(CDIF *cdif, int32_t lba)
{
#ifdef WANT_THREADING
   if (cdif->is_mt)
      CDIF_MT_Hint(cdif, lba);
#else
   (void)cdif; (void)lba;
#endif
   /* ST: no-op (would require asynchronous I/O we don't do). */
}

bool CDIF_ReadRawSector(CDIF *cdif, uint8_t *buf, int32_t lba)
{
#ifdef WANT_THREADING
   if (cdif->is_mt)
      return CDIF_MT_ReadRawSector(cdif, buf, lba);
#endif
   return CDIF_ST_ReadRawSector(cdif, buf, lba);
}

bool CDIF_ReadRawSectorPWOnly(CDIF *cdif, uint8_t *pwbuf, int32_t lba, bool hint_fullread)
{
#ifdef WANT_THREADING
   if (cdif->is_mt)
      return CDIF_MT_ReadRawSectorPWOnly(cdif, pwbuf, lba, hint_fullread);
#endif
   return CDIF_ST_ReadRawSectorPWOnly(cdif, pwbuf, lba, hint_fullread);
}

/* ------------------------------------------------------------------
 * Construction / destruction. */

CDIF *CDIF_Open(const char *path, bool image_memcache)
{
   CDAccess *cda = CDAccess_Open(path, image_memcache);
   CDIF     *cdif;

   if (!cda)
      return NULL;

   cdif = (CDIF *)calloc(1, sizeof(*cdif));
   if (!cdif)
   {
      cda->destroy(cda);
      return NULL;
   }

   cdif->disc_cdaccess = cda;
   cdif->is_mt         = false;
   /* Finish startup I/O before transferring sole backend ownership to
    * the worker. The work event then has only the worker as its waiter. */
   if (!cda->Read_TOC(cda, &cdif->disc_toc))
   {
      CDIF_Close(cdif);
      return NULL;
   }
#ifdef WANT_THREADING
   cdif->is_mt         = !image_memcache;

   if (cdif->is_mt)
   {
      unsigned i;
      bool ok;
      retro_atomic_int_init(&cdif->request_lba, 0);
      retro_atomic_int_init(&cdif->request_seq, 0);
      retro_atomic_int_init(&cdif->hint_lba, 0);
      retro_atomic_int_init(&cdif->hint_seq, 0);
      retro_atomic_int_init(&cdif->stopping, 0);
      ok = retro_eventcount_init(&cdif->work);
      ok = ok && retro_waitable_spsc_init(&cdif->ready_sectors, 32 * sizeof(unsigned));
      ok = ok && retro_waitable_spsc_init(&cdif->free_sectors, 32 * sizeof(unsigned));
      if (ok)
      {
         for (i = 0; i < 32; i++)
            retro_waitable_spsc_write(&cdif->free_sectors, &i, sizeof(i));
         cdif->CDReadThread = sthread_create(CDIF_ReadThreadStart, cdif);
         ok = cdif->CDReadThread != NULL;
      }
      if (!ok)
      {
         retro_waitable_spsc_free(&cdif->ready_sectors);
         retro_waitable_spsc_free(&cdif->free_sectors);
         retro_eventcount_free(&cdif->work);
         cdif->disc_cdaccess->destroy(cdif->disc_cdaccess);
         free(cdif);
         return NULL;
      }
   }
#endif

   /* Validate both synchronous and threaded images. */
   if (cdif->disc_toc.first_track < 1 || cdif->disc_toc.last_track > 99 ||
       cdif->disc_toc.first_track > cdif->disc_toc.last_track)
   {
      log_cb(RETRO_LOG_ERROR, "TOC first(%d)/last(%d) track numbers bad.\n",
            cdif->disc_toc.first_track, cdif->disc_toc.last_track);
      CDIF_Close(cdif);
      return NULL;
   }

   return cdif;
}

void CDIF_Close(CDIF *cdif)
{
   if (!cdif)
      return;

#ifdef WANT_THREADING
   if (cdif->is_mt)
   {
      retro_atomic_store_release_int(&cdif->stopping, 1);
      retro_eventcount_notify(&cdif->work);
      retro_waitable_spsc_cancel(&cdif->free_sectors);
      retro_waitable_spsc_cancel(&cdif->ready_sectors);
      sthread_join(cdif->CDReadThread);
      retro_waitable_spsc_free(&cdif->ready_sectors);
      retro_waitable_spsc_free(&cdif->free_sectors);
      retro_eventcount_free(&cdif->work);
   }

#endif

   if (cdif->disc_cdaccess)
      cdif->disc_cdaccess->destroy(cdif->disc_cdaccess);

   free(cdif);
}
