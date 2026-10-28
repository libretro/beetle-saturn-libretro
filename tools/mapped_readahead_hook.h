/* Force-included into the off-thread build of mapped_readahead_test,
 * which points libretro-common's PrefetchVirtualMemory call at a
 * stand-in (-DVFS_WIN32_PREFETCH_HOOK=saturn_prefetch_hook). */
#ifndef SATURN_MAPPED_READAHEAD_HOOK_H
#define SATURN_MAPPED_READAHEAD_HOOK_H
#include <stddef.h>
int saturn_prefetch_hook(void *addr, size_t len);
#endif
