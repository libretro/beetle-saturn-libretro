/* Battery-backed saves (backup RAM, the cartridge's NV memory) go to
 * their files from a thread of their own: the emulation thread copies
 * the bytes - the one copy a snapshot needs - and hands them over, and
 * the writer opens, writes and closes. Without threads the write happens
 * before the hand-over returns.
 *
 * One thread saves: the emulation thread. C89. */
#ifndef __MDFN_SAVE_WRITER_H
#define __MDFN_SAVE_WRITER_H

#include <stddef.h>
#include <stdint.h>
#include <boolean.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct save_job save_job_t;

/* A job writing @len bytes as the whole file at @path; its bytes are
 * filled in through save_job_data(). NULL without memory. */
save_job_t *save_job_new(const char *path, size_t len);
uint8_t *save_job_data(save_job_t *job);
/* Its bytes from @len bytes of host-endian 16-bit words at @src, written
 * big-endian (a trailing odd byte as 0). */
void save_job_fill_be16(save_job_t *job, const void *src, size_t len);

/* Hands @job over (it is the writer's from here). @failed is set to true
 * on the emulation thread, by a later call here, if the write failed, so
 * it has to outlive the job: a static of the saver's. */
void save_writer_submit(save_job_t *job, bool *failed);

/* Returns when everything handed over is written (or failed), with the
 * outcomes reported. */
void save_writer_drain(void);

/* The same, and the thread is ended; the next hand-over starts it. */
void save_writer_stop(void);

#ifdef __cplusplus
}
#endif

#endif
