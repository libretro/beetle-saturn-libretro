/* See save_writer.h. C89. */
#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <streams/file_stream.h>

#include "save_writer.h"

extern retro_log_printf_t log_cb;

#ifdef WANT_THREADING
#include <rthreads/rthreads.h>
#include "thread_eventcount.h"
#endif

struct save_job
{
   size_t   len;
   bool    *failed;   /* the saver's flag */
   bool     ok;       /* the write's outcome */
   uint8_t *data;     /* the bytes, then the path */
};

save_job_t *save_job_new(const char *path, size_t len)
{
   const size_t plen = strlen(path) + 1;
   save_job_t  *job  = (save_job_t *)malloc(sizeof(*job) + len + plen);

   if (!job)
      return NULL;
   job->len    = len;
   job->failed = NULL;
   job->ok     = false;
   job->data   = (uint8_t *)(job + 1);
   memcpy(job->data + len, path, plen);
   return job;
}

uint8_t *save_job_data(save_job_t *job)
{
   return job->data;
}

void save_job_fill_be16(save_job_t *job, const void *src, size_t len)
{
   const uint8_t *in  = (const uint8_t *)src;
   uint8_t       *out = job->data;
   size_t         i;

   for (i = 0; i + 1 < len; i += 2)
   {
      uint16_t v;
      memcpy(&v, in + i, 2);
      out[i]     = (uint8_t)(v >> 8);
      out[i + 1] = (uint8_t)v;
   }
   if (len & 1)
      out[len - 1] = 0;
}

static void save_job_run(save_job_t *job)
{
   RFILE *f = filestream_open((const char *)job->data + job->len,
         RETRO_VFS_FILE_ACCESS_WRITE, RETRO_VFS_FILE_ACCESS_HINT_NONE);

   job->ok = false;
   if (!f)
      return;
   job->ok = filestream_write(f, job->data, (int64_t)job->len) == (int64_t)job->len;
   if (filestream_close(f) != 0)
      job->ok = false;
}

/* On the emulation thread: the outcome reported, the job freed. */
static void save_job_finish(save_job_t *job)
{
   if (!job->ok)
   {
      const char *path = (const char *)job->data + job->len;
      if (job->failed)
         *job->failed = true;
      log_cb(RETRO_LOG_ERROR, "Save to \"%s\" failed (disk full or I/O error).\n", path);
   }
   free(job);
}

#ifdef WANT_THREADING

/* At most this many jobs out at once: the queues hold that many
 * pointers, so the writer's hand-back never finds its queue full. */
#define SAVE_WRITER_JOBS 16

static struct
{
   sthread_t            *thread;
   retro_waitable_spsc_t jobs;  /* emulation thread -> writer */
   retro_waitable_spsc_t done;  /* writer -> emulation thread */
   unsigned              out;   /* handed over, not yet back */
} writer;

static void save_writer_thread(void *unused)
{
   (void)unused;
   for (;;)
   {
      save_job_t *job;

      if (!retro_waitable_spsc_wait_readable(&writer.jobs, sizeof(job), 1000000))
      {
         if (retro_waitable_spsc_cancelled(&writer.jobs))
            return;
         continue;
      }
      retro_waitable_spsc_read(&writer.jobs, &job, sizeof(job));
      save_job_run(job);
      retro_waitable_spsc_write(&writer.done, &job, sizeof(job));
   }
}

/* One job back, waited for. */
static void save_writer_reap_one(void)
{
   save_job_t *job;

   while (!retro_waitable_spsc_wait_readable(&writer.done, sizeof(job), 1000000)) { }
   retro_waitable_spsc_read(&writer.done, &job, sizeof(job));
   writer.out--;
   save_job_finish(job);
}

/* The jobs that are back, finished; with @wait, until none is out. */
static void save_writer_reap(bool wait)
{
   save_job_t *job;

   while (writer.out)
   {
      if (wait)
      {
         save_writer_reap_one();
         continue;
      }
      if (!retro_waitable_spsc_wait_readable(&writer.done, sizeof(job), 0))
         return;
      retro_waitable_spsc_read(&writer.done, &job, sizeof(job));
      writer.out--;
      save_job_finish(job);
   }
}

static bool save_writer_start(void)
{
   if (writer.thread)
      return true;
   memset(&writer, 0, sizeof(writer));
   if (!retro_waitable_spsc_init(&writer.jobs, SAVE_WRITER_JOBS * sizeof(save_job_t *)))
      return false;
   if (!retro_waitable_spsc_init(&writer.done, SAVE_WRITER_JOBS * sizeof(save_job_t *)))
   {
      retro_waitable_spsc_free(&writer.jobs);
      return false;
   }
   if (!(writer.thread = sthread_create(save_writer_thread, NULL)))
   {
      retro_waitable_spsc_free(&writer.done);
      retro_waitable_spsc_free(&writer.jobs);
      return false;
   }
   return true;
}

void save_writer_submit(save_job_t *job, bool *failed)
{
   if (!job)
      return;
   job->failed = failed;
   if (save_writer_start())
   {
      save_writer_reap(false);
      /* (the writer is a whole queue behind: the oldest is waited for) */
      if (writer.out >= SAVE_WRITER_JOBS)
         save_writer_reap_one();
      writer.out++;
      retro_waitable_spsc_write(&writer.jobs, &job, sizeof(job));
      return;
   }
   /* no thread to be had: written here */
   save_job_run(job);
   save_job_finish(job);
}

void save_writer_drain(void)
{
   if (writer.thread)
      save_writer_reap(true);
}

void save_writer_stop(void)
{
   if (!writer.thread)
      return;
   save_writer_reap(true);
   retro_waitable_spsc_cancel(&writer.jobs);
   sthread_join(writer.thread);
   writer.thread = NULL;
   retro_waitable_spsc_free(&writer.done);
   retro_waitable_spsc_free(&writer.jobs);
}

#else

void save_writer_submit(save_job_t *job, bool *failed)
{
   if (!job)
      return;
   job->failed = failed;
   save_job_run(job);
   save_job_finish(job);
}

void save_writer_drain(void) { }
void save_writer_stop(void) { }

#endif
