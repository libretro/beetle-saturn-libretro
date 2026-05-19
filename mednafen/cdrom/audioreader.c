/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* AudioReader is a thin shim that wraps a cdstream in Tremor's
 * ov_callbacks interface so an Ogg Vorbis-encoded CDDA track can
 * be decoded on-demand by the CDB layer.
 *
 * Ownership contract: AR_Open / AR_Close do NOT take ownership of
 * the cdstream object.  Caller retains ownership and is responsible
 * for cdstream_destroy() after AR_Close().  This is enforced by
 * deliberately passing a NULL close_func to ov_open_callbacks --
 * Tremor's ov_clear() then skips the close hook entirely (see
 * `if(vf->datasource && vf->callbacks.close_func)` in
 * tremor/vorbisfile.c).  If you reintroduce a close callback here,
 * audit every AR_Close call site for double-close: today
 * CDAccess_Image_Cleanup() calls AR_Close() then cdstream_destroy()
 * on the same cdstream, and a non-NULL close_func would turn that
 * into a close-twice (currently saved only by cdstream_close()'s
 * idempotence). */

#include <stdlib.h>
#include <string.h>

#include "audioreader.h"

#include "../tremor/ivorbisfile.h"

struct AudioReader
{
   OggVorbis_File ovfile;
   int64_t        last_read_pos;
};

static size_t iov_read_func(void *ptr, size_t size, size_t nmemb, void *user_data)
{
   cdstream *fw = (cdstream *)user_data;

   if(!size || !fw)
      return 0;

   return cdstream_read(fw, ptr, size * nmemb) / size;
}

static int iov_seek_func(void *user_data, int64_t offset, int whence)
{
   cdstream *fw = (cdstream *)user_data;

   if (fw)
      cdstream_seek(fw, offset, whence);
   return 0;
}

static long iov_tell_func(void *user_data)
{
   cdstream *fw = (cdstream *)user_data;

   if (!fw)
      return -1;

   /* Tremor's tell_func signature returns long (see
    * tremor/ivorbisfile.h ov_callbacks).  cdstream_tell returns
    * uint64_t.  On LLP64 (Win64) `long` is 32-bit, so this
    * truncates above 2 GiB.  Not fixable at our layer without
    * forking Tremor; in practice Vorbis-encoded CDDA tracks are
    * orders of magnitude smaller than that. */
   return (long)cdstream_tell(fw);
}

AudioReader *AR_Open(cdstream *fp)
{
   ov_callbacks cb;
   AudioReader *r;

   if (!fp)
      return NULL;

   r = (AudioReader *)calloc(1, sizeof(*r));
   if (!r)
      return NULL;

   memset(&cb, 0, sizeof(cb));
   cb.read_func  = iov_read_func;
   cb.seek_func  = iov_seek_func;
   cb.close_func = NULL;             /* see ownership contract above */
   cb.tell_func  = iov_tell_func;

   cdstream_seek(fp, 0, SEEK_SET);
   if (ov_open_callbacks(fp, &r->ovfile, NULL, 0, cb) != 0)
   {
      free(r);
      return NULL;
   }

   return r;
}

void AR_Close(AudioReader *r)
{
   if (!r)
      return;
   ov_clear(&r->ovfile);
   free(r);
}

int64_t AR_FrameCount(AudioReader *r)
{
   if (!r)
      return 0;
   return ov_pcm_total(&r->ovfile, -1);
}

int64_t AR_Read(AudioReader *r, int64_t frame_offset,
      int16_t *buffer, int64_t frames)
{
   uint8_t *tw_buf;
   long     toread;
   int      cursection = 0;
   int64_t  ret;

   if (!r)
      return 0;

   /* The pre-collapse CDAFReader had this seek shortcut as an INLINE
    * wrapper in CDAFReader.h, then dispatched Seek_ / Read_ virtually.
    * Folded inline here since Vorbis is the only backend. */
   if (r->last_read_pos != frame_offset)
   {
      ov_pcm_seek(&r->ovfile, frame_offset);
      r->last_read_pos = frame_offset;
   }

   tw_buf = (uint8_t *)buffer;
   toread = frames * (long)sizeof(int16_t) * 2;

   while (toread > 0)
   {
      long didread = ov_read(&r->ovfile, (char *)tw_buf, toread, &cursection);

      if (didread == 0)
         break;                       /* EOF */
      if (didread < 0)
      {
         /* Tremor signals decode trouble through negative returns:
          *   OV_HOLE     (-3)   gap in the bitstream; docs say to
          *                      just keep calling, but a pathological
          *                      file could spin us indefinitely here
          *   OV_EBADLINK (-137) inconsistent logical bitstream
          *   OV_EINVAL   (-131) initial file headers couldn't be read
          *
          * The pre-fix code blindly added a negative didread to the
          * uint8_t* write pointer (pointer arithmetic going backwards
          * past the start of the buffer -- UB) and subtracted it from
          * `toread` (negative subtract = grow), so a single decode
          * error would either corrupt arbitrary memory or loop
          * forever.  Treat any negative as terminal: CDDA cutting out
          * a few samples early is vastly preferable to either of
          * those outcomes, and the caller (CDB) handles short-read
          * returns from this function gracefully. */
         break;
      }

      tw_buf += didread;
      toread -= didread;
   }

   ret = frames - toread / (int64_t)sizeof(int16_t) / 2;
   r->last_read_pos += ret;
   return ret;
}
