/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* jitdump.c - shared Linux perf jitdump writer for the DSP JITs
**  Copyright (C) 2026 pstef
*/

#include "jitdump.h"

#if defined(WANT_DSP_JIT_PERF_DUMP) && (defined(__aarch64__) || defined(__arm64__) || defined(__x86_64__))

/*
 * Produces /tmp/jit-<pid>.dump in the Linux perf jitdump v1 format.
 * `perf record` captures the marker mmap, then `perf inject --jit`
 * reads the dump and emits ELF stubs so `perf report` resolves samples
 * in the code segments to per-slot symbols.
 *
 * Both DSP JITs compile on the emulator thread, so the fd and the index
 * counter are unserialized; the atexit handler fires after that thread
 * exits.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#define JITDUMP_MAGIC      0x4A695444u /* "JiTD" */
#define JITDUMP_VERSION    1u
#define JIT_CODE_LOAD      0u
#define JIT_CODE_CLOSE     3u
#define ELF_MACH_AARCH64   183u
#define ELF_MACH_X86_64    62u

struct JitdumpHeader
{
 uint32_t magic;
 uint32_t version;
 uint32_t total_size;
 uint32_t elf_mach;
 uint32_t pad1;
 uint32_t pid;
 uint64_t timestamp;
 uint64_t flags;
};

struct JitdumpRecPrefix
{
 uint32_t id;
 uint32_t total_size;
 uint64_t timestamp;
};

struct JitdumpRecCodeLoad
{
 struct JitdumpRecPrefix p;
 uint32_t pid;
 uint32_t tid;
 uint64_t vma;
 uint64_t code_addr;
 uint64_t code_size;
 uint64_t code_index;
 /* followed by NUL-terminated name, then code_size bytes of code */
};

static int      g_jitdump_fd          = -1;
static void*    g_jitdump_marker      = NULL;
static size_t   g_jitdump_marker_size = 0;
static uint64_t g_jitdump_index       = 0;

static uint64_t jitdump_now_ns(void)
{
 struct timespec ts;
 clock_gettime(CLOCK_MONOTONIC, &ts);
 return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static void jitdump_close(void)
{
 struct JitdumpRecPrefix close_rec = {0};
 if(g_jitdump_fd < 0) return;
 close_rec.id         = JIT_CODE_CLOSE;
 close_rec.total_size = sizeof(close_rec);
 close_rec.timestamp  = jitdump_now_ns();
 (void)write(g_jitdump_fd, &close_rec, sizeof(close_rec));
 if(g_jitdump_marker) (void)munmap(g_jitdump_marker, g_jitdump_marker_size);
 (void)close(g_jitdump_fd);
 g_jitdump_fd          = -1;
 g_jitdump_marker      = NULL;
 g_jitdump_marker_size = 0;
}

void SS_JitDump_Open(void)
{
 char path[64];
 int fd;
 struct JitdumpHeader hdr = {0};
 long pagesz;

 if(g_jitdump_fd >= 0) return;
 snprintf(path, sizeof(path), "/tmp/jit-%d.dump", (int)getpid());
 fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0644);
 if(fd < 0) return;

 hdr.magic      = JITDUMP_MAGIC;
 hdr.version    = JITDUMP_VERSION;
 hdr.total_size = sizeof(hdr);
#if defined(__x86_64__)
 hdr.elf_mach   = ELF_MACH_X86_64;
#else
 hdr.elf_mach   = ELF_MACH_AARCH64;
#endif
 hdr.pid        = (uint32_t)getpid();
 hdr.timestamp  = jitdump_now_ns();
 if(write(fd, &hdr, sizeof(hdr)) != (ssize_t)sizeof(hdr))
 {
  (void)close(fd);
  return;
 }

 /* The marker mmap is what `perf record` sees in its MMAP events; that
  * is how `perf inject --jit` discovers our dump file.  One page of
  * PROT_READ|PROT_EXEC at file offset 0 is the documented contract. */
 pagesz = sysconf(_SC_PAGESIZE);
 g_jitdump_marker_size = (pagesz > 0) ? (size_t)pagesz : 4096u;
 g_jitdump_marker = mmap(NULL, g_jitdump_marker_size,
                         PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);
 if(g_jitdump_marker == MAP_FAILED) g_jitdump_marker = NULL;

 g_jitdump_fd = fd;
 atexit(jitdump_close);
}

void SS_JitDump_Emit(const char* name, const void* code_addr, size_t code_size)
{
 size_t name_len;
 struct JitdumpRecCodeLoad rec = {0};
 struct iovec iov[3];

 if(g_jitdump_fd < 0 || !code_addr || code_size == 0) return;

 name_len = strlen(name) + 1;
 rec.p.id         = JIT_CODE_LOAD;
 rec.p.total_size = (uint32_t)(sizeof(rec) + name_len + code_size);
 rec.p.timestamp  = jitdump_now_ns();
 rec.pid          = (uint32_t)getpid();
 rec.tid          = (uint32_t)syscall(SYS_gettid);
 rec.vma          = (uint64_t)(uintptr_t)code_addr;
 rec.code_addr    = rec.vma;
 rec.code_size    = code_size;
 rec.code_index   = ++g_jitdump_index;

 iov[0].iov_base = &rec;
 iov[0].iov_len  = sizeof(rec);
 iov[1].iov_base = (char*)name;
 iov[1].iov_len  = name_len;
 iov[2].iov_base = (void*)code_addr;
 iov[2].iov_len  = code_size;
 (void)writev(g_jitdump_fd, iov, 3);
}

#endif
