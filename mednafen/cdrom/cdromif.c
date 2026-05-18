/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * cdromif: thin layer between the emulated CD-ROM block and the
 * disc-format-specific CDAccess backends (Image / CCD / CHD).
 *
 * Two modes selected at CDIF_Open time based on image_memcache:
 *
 *   MT (multi-threaded): background thread reads ahead of the
 *      emulated drive, smoothing out disk-I/O latency.  Sectors
 *      land in a 256-slot ring buffer; ReadRawSector blocks until
 *      the requested LBA appears.
 *
 *   ST (single-threaded): synchronous; ReadRawSector calls the
 *      CDAccess backend directly on the caller's thread.  Used when
 *      image_memcache is true (whole disc fits in RAM).
 *
 * Historical baggage cleared in this C conversion:
 *
 *   - class CDIF (abstract base) + CDIF_MT / CDIF_ST (concrete).
 *     Replaced with one struct CDIF tagged by `is_mt`; only one of
 *     the two field-sets is live per instance, `is_mt` selects.
 *     Same dispatch outcome as the C++ vtable, one fewer indirection.
 *
 *   - CDIF_Message had three constructors including one taking a
 *     std::string for FATAL_ERROR diagnostic text.  No code path
 *     ever constructed a string-bearing message in Beetle Saturn;
 *     the CDIF_MSG_FATAL_ERROR code was wired up but never sent.
 *     The std::string field is gone; messages are 5-uint32_t PODs.
 *
 *   - CDIF_Queue was std::queue<CDIF_Message>.  In practice the
 *     in-flight depth is 1-3 messages (DIE + a handful of READ
 *     hints).  Replaced with a fixed-size 16-slot ring buffer with
 *     mutex+cond.  No heap alloc per push, no dynamic resizing.
 *
 *   - The CDIF_Open factory returned a heap-allocated polymorphic
 *     object; the destructor was a virtual function.  Now
 *     CDIF_Open calloc's the concrete struct, init's it, returns
 *     a pointer; CDIF_Close mirror-tears-down.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include <boolean.h>
#include <rthreads/rthreads.h>
#include <retro_miscellaneous.h>
#include <libretro.h>

#include "CDUtility.h"
#include "CDAccess.h"
#include "cdromif.h"

extern retro_log_printf_t log_cb;

/* ------------------------------------------------------------------
 * CDIF_Message - inter-thread protocol message.
 * ------------------------------------------------------------------ */

enum
{
   CDIF_MSG_DONE = 0,        /* Read -> emu. Read thread is ready. */
   CDIF_MSG_DIEDIEDIE,       /* Emu  -> read.  Time to exit. */
   CDIF_MSG_READ_SECTOR      /* Emu  -> read.  args[0] = lba. */
};

typedef struct CDIF_Message
{
   unsigned message;
   uint32_t args[4];
} CDIF_Message;

/* ------------------------------------------------------------------
 * CDIF_Queue - fixed-capacity SPSC ring buffer of CDIF_Message.
 * ------------------------------------------------------------------ */

#define CDIF_QUEUE_SIZE 16

typedef struct CDIF_Queue
{
   CDIF_Message ring[CDIF_QUEUE_SIZE];
   unsigned     head;     /* dequeue position */
   unsigned     tail;     /* enqueue position */
   unsigned     count;
   slock_t     *mutex;
   scond_t     *cond;
} CDIF_Queue;

static bool CDIF_Queue_Init(CDIF_Queue *q)
{
   q->head  = 0;
   q->tail  = 0;
   q->count = 0;
   q->mutex = slock_new();
   q->cond  = scond_new();
   /* slock_new / scond_new can return NULL on OOM.  Report up so
    * CDIF_Open's MT-path init can roll back rather than deadlocking
    * later inside CDIF_Queue_Read on a half-initialised queue. */
   return q->mutex && q->cond;
}

static void CDIF_Queue_Free(CDIF_Queue *q)
{
   if (q->mutex)
      slock_free(q->mutex);
   if (q->cond)
      scond_free(q->cond);
   q->mutex = NULL;
   q->cond  = NULL;
}

/* Pop the next message into *out.  blocking=true waits for one if
 * the queue is empty; blocking=false returns immediately with
 * return value false in that case.  Was throw-on-FATAL_ERROR in
 * the C++ version but FATAL_ERROR was never sent, so the dead
 * branch is dropped. */
static bool CDIF_Queue_Read(CDIF_Queue *q, CDIF_Message *out, bool blocking)
{
   bool ret = false;

   slock_lock(q->mutex);

   if (blocking)
   {
      while (q->count == 0)
         scond_wait(q->cond, q->mutex);
   }

   if (q->count != 0)
   {
      *out     = q->ring[q->head];
      q->head  = (q->head + 1) % CDIF_QUEUE_SIZE;
      q->count--;
      ret      = true;
   }

   slock_unlock(q->mutex);
   return ret;
}

static void CDIF_Queue_Write(CDIF_Queue *q, const CDIF_Message *msg)
{
   slock_lock(q->mutex);

   if (q->count < CDIF_QUEUE_SIZE)
   {
      q->ring[q->tail] = *msg;
      q->tail          = (q->tail + 1) % CDIF_QUEUE_SIZE;
      q->count++;
   }
   /* If the ring is full the message is silently dropped.  In
    * practice DIEDIEDIE (sent once) and READ_SECTOR (rate-limited
    * by HintReadSector calls from the emu thread) can't fill a
    * 16-slot ring in any realistic timing. */

   scond_signal(q->cond);
   slock_unlock(q->mutex);
}

/* Push a bare message-code with no args (DONE / DIEDIEDIE). */
static void CDIF_Queue_Write_Code(CDIF_Queue *q, unsigned code)
{
   CDIF_Message m;
   memset(&m, 0, sizeof(m));
   m.message = code;
   CDIF_Queue_Write(q, &m);
}

/* Push a READ_SECTOR message carrying lba in args[0]. */
static void CDIF_Queue_Write_ReadLBA(CDIF_Queue *q, uint32_t lba)
{
   CDIF_Message m;
   memset(&m, 0, sizeof(m));
   m.message = CDIF_MSG_READ_SECTOR;
   m.args[0] = lba;
   CDIF_Queue_Write(q, &m);
}

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

   /* MT-only.  Untouched in ST instances. */
   sthread_t *CDReadThread;

   CDIF_Queue ReadThreadQueue;   /* emu -> read */
   CDIF_Queue EmuThreadQueue;    /* read -> emu */

   CDIF_Sector_Buffer SectorBuffers[CDIF_SBSIZE];
   uint32_t SBWritePos;

   slock_t *SBMutex;
   scond_t *SBCond;

   /* Read-thread-only state.  Touched only by the background
    * thread; no synchronization needed. */
   int32_t ra_lba;
   int32_t ra_count;
   int32_t last_read_lba;
};

/* Forward declaration for the read-thread entry point. */
static void CDIF_ReadThreadStart(void *v_arg);

/* ------------------------------------------------------------------
 * Public API. */

void CDIF_ReadTOC(CDIF *cdif, TOC *out)
{
   *out = cdif->disc_toc;
}

bool CDIF_ValidateRawSector(uint8_t *buf)
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

static void CDIF_MT_ReadThreadBody(CDIF *self)
{
   bool Running = true;

   self->SBWritePos     = 0;
   self->ra_lba         = 0;
   self->ra_count       = 0;
   self->last_read_lba  = CDIF_LBA_Read_Maximum + 1;

   self->disc_cdaccess->Read_TOC(self->disc_cdaccess, &self->disc_toc);

   if (self->disc_toc.first_track < 1
         || self->disc_toc.last_track > 99
         || self->disc_toc.first_track > self->disc_toc.last_track)
   {
      log_cb(RETRO_LOG_ERROR, "TOC first(%d)/last(%d) track numbers bad.\n",
            self->disc_toc.first_track, self->disc_toc.last_track);
   }

   self->SBWritePos     = 0;
   self->ra_lba         = 0;
   self->ra_count       = 0;
   self->last_read_lba  = CDIF_LBA_Read_Maximum + 1;
   memset(self->SectorBuffers, 0, sizeof(self->SectorBuffers));

   CDIF_Queue_Write_Code(&self->EmuThreadQueue, CDIF_MSG_DONE);

   while (Running)
   {
      CDIF_Message msg;

      /* Only block waiting for a message when no read-ahead work
       * is pending. */
      if (CDIF_Queue_Read(&self->ReadThreadQueue, &msg, self->ra_count == 0))
      {
         switch (msg.message)
         {
         case CDIF_MSG_DIEDIEDIE:
            Running = false;
            break;

         case CDIF_MSG_READ_SECTOR:
            {
               static const int max_ra       = 16;
               static const int initial_ra   = 1;
               static const int speedmult_ra = 2;
               int32_t new_lba = (int32_t)msg.args[0];

               assert((unsigned)max_ra < (CDIF_SBSIZE / 4));

               if (new_lba == (self->last_read_lba + 1))
               {
                  int how_far_ahead = self->ra_lba - new_lba;
                  int candidate;

                  if (how_far_ahead <= max_ra)
                  {
                     candidate = 1 + max_ra - how_far_ahead;
                     /* The old C++ code used ((speedmult_ra) < (candidate) ? (speedmult_ra) : (candidate)). */
                     self->ra_count = (speedmult_ra < candidate) ? speedmult_ra : candidate;
                  }
                  else
                  {
                     self->ra_count++;
                  }
               }
               else if (new_lba != self->last_read_lba)
               {
                  self->ra_lba   = new_lba;
                  self->ra_count = initial_ra;
               }

               self->last_read_lba = new_lba;
            }
            break;
         }
      }

      /* Don't read beyond what the readers can handle sanely. */
      if (self->ra_count && self->ra_lba == CDIF_LBA_Read_Maximum)
         self->ra_count = 0;

      if (self->ra_count)
      {
         /* Read directly into the ring slot.  Flip valid=false
          * outside the mutex so a racing reader won't snapshot a
          * half-written buffer; the Read_Raw_Sector call writes
          * the 2448-byte payload in place, then we relock to
          * publish valid=true + signal.  Saves a 2448-byte memcpy
          * per read-ahead sector versus the previous tmpbuf+copy
          * pattern.  Same shape as beetle-psx-libretro's
          * cdromif.c read thread. */
         const unsigned slot = self->SBWritePos;

         slock_lock(self->SBMutex);
         self->SectorBuffers[slot].valid = false;
         slock_unlock(self->SBMutex);

         self->disc_cdaccess->Read_Raw_Sector(self->disc_cdaccess,
               self->SectorBuffers[slot].data, self->ra_lba);

         slock_lock(self->SBMutex);
         self->SectorBuffers[slot].lba   = self->ra_lba;
         self->SectorBuffers[slot].error = false;
         self->SectorBuffers[slot].valid = true;
         self->SBWritePos = (slot + 1) % CDIF_SBSIZE;
         scond_signal(self->SBCond);
         slock_unlock(self->SBMutex);

         self->ra_lba++;
         self->ra_count--;
      }
   }
}

static void CDIF_ReadThreadStart(void *v_arg)
{
   CDIF_MT_ReadThreadBody((CDIF *)v_arg);
}

static bool CDIF_MT_ReadRawSector(CDIF *self, uint8_t *buf, int32_t lba)
{
   bool found            = false;
   bool error_condition  = false;
   int  i;

   if (lba < CDIF_LBA_Read_Minimum || lba > CDIF_LBA_Read_Maximum)
   {
      memset(buf, 0, 2352 + 96);
      return false;
   }

   CDIF_Queue_Write_ReadLBA(&self->ReadThreadQueue, (uint32_t)lba);

   slock_lock(self->SBMutex);

   do
   {
      for (i = 0; i < CDIF_SBSIZE; i++)
      {
         if (self->SectorBuffers[i].valid && self->SectorBuffers[i].lba == lba)
         {
            error_condition = self->SectorBuffers[i].error;
            memcpy(buf, self->SectorBuffers[i].data, 2352 + 96);
            found = true;
         }
      }

      if (!found)
         scond_wait(self->SBCond, self->SBMutex);
   } while (!found);

   slock_unlock(self->SBMutex);

   return !error_condition;
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
         CDIF_Queue_Write_ReadLBA(&self->ReadThreadQueue, (uint32_t)lba);
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

   self->disc_cdaccess->Read_Raw_Sector(self->disc_cdaccess, buf, lba);
   return true;
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
   if (cdif->is_mt)
      CDIF_Queue_Write_ReadLBA(&cdif->ReadThreadQueue, (uint32_t)lba);
   /* ST: no-op (would require asynchronous I/O we don't do). */
}

bool CDIF_ReadRawSector(CDIF *cdif, uint8_t *buf, int32_t lba)
{
   if (cdif->is_mt)
      return CDIF_MT_ReadRawSector(cdif, buf, lba);
   return CDIF_ST_ReadRawSector(cdif, buf, lba);
}

bool CDIF_ReadRawSectorPWOnly(CDIF *cdif, uint8_t *pwbuf, int32_t lba, bool hint_fullread)
{
   if (cdif->is_mt)
      return CDIF_MT_ReadRawSectorPWOnly(cdif, pwbuf, lba, hint_fullread);
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
   cdif->is_mt         = !image_memcache;

   if (cdif->is_mt)
   {
      CDIF_Message msg;
      /* Step every MT-init allocation under a single ok flag.  Any
       * NULL return from slock_new / scond_new / sthread_create means
       * the read thread will never reach the DONE-message signal, and
       * the CDIF_Queue_Read below would block forever waiting on it.
       * Roll back partial state and fail the open instead. */
      bool ok = true;

      ok = ok && CDIF_Queue_Init(&cdif->ReadThreadQueue);
      ok = ok && CDIF_Queue_Init(&cdif->EmuThreadQueue);

      if (ok)
      {
         cdif->SBMutex = slock_new();
         cdif->SBCond  = scond_new();
         if (!cdif->SBMutex || !cdif->SBCond)
            ok = false;
      }

      if (ok)
      {
         cdif->CDReadThread = sthread_create(CDIF_ReadThreadStart, cdif);
         if (!cdif->CDReadThread)
            ok = false;
      }

      if (!ok)
      {
         /* CDIF_Queue_Free is NULL-safe on the mutex/cond it owns;
          * slock_free / scond_free on NULL is also safe per libretro-
          * common's rthreads contract. */
         CDIF_Queue_Free(&cdif->ReadThreadQueue);
         CDIF_Queue_Free(&cdif->EmuThreadQueue);
         if (cdif->SBMutex) slock_free(cdif->SBMutex);
         if (cdif->SBCond)  scond_free(cdif->SBCond);
         cdif->disc_cdaccess->destroy(cdif->disc_cdaccess);
         free(cdif);
         return NULL;
      }

      /* Wait for the read thread to fill disc_toc and signal DONE. */
      CDIF_Queue_Read(&cdif->EmuThreadQueue, &msg, true);
   }
   else
   {
      /* ST: populate disc_toc inline, no thread spawned. */
      cdif->disc_cdaccess->Read_TOC(cdif->disc_cdaccess, &cdif->disc_toc);

      if (cdif->disc_toc.first_track < 1
            || cdif->disc_toc.last_track > 99
            || cdif->disc_toc.first_track > cdif->disc_toc.last_track)
      {
         log_cb(RETRO_LOG_ERROR, "TOC first(%d)/last(%d) track numbers bad.\n",
               cdif->disc_toc.first_track, cdif->disc_toc.last_track);
         /* The C++ ST ctor used to throw on this; the call chain in
          * disc.c / libretro.c didn't actually catch it, so the
          * emulator would terminate.  Mirror that behaviour with a
          * NULL return - callers already null-check. */
         cdif->disc_cdaccess->destroy(cdif->disc_cdaccess);
         free(cdif);
         return NULL;
      }
   }

   return cdif;
}

void CDIF_Close(CDIF *cdif)
{
   if (!cdif)
      return;

   if (cdif->is_mt)
   {
      CDIF_Queue_Write_Code(&cdif->ReadThreadQueue, CDIF_MSG_DIEDIEDIE);
      sthread_join(cdif->CDReadThread);

      CDIF_Queue_Free(&cdif->ReadThreadQueue);
      CDIF_Queue_Free(&cdif->EmuThreadQueue);

      if (cdif->SBMutex)
         slock_free(cdif->SBMutex);
      if (cdif->SBCond)
         scond_free(cdif->SBCond);
   }

   if (cdif->disc_cdaccess)
      cdif->disc_cdaccess->destroy(cdif->disc_cdaccess);

   free(cdif);
}
