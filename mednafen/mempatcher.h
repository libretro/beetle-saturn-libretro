#ifndef __MDFN_MEMPATCHER_H
#define __MDFN_MEMPATCHER_H

#include <stdint.h>
#include <stddef.h>
#include <boolean.h>

#include "settings-common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __SUBCHEAT
{
	uint32_t addr;
	uint8_t value;
	int compare; // < 0 on no compare
} SUBCHEAT;

bool MDFNMP_Init(uint32_t ps, uint32_t numpages);
void MDFNMP_AddRAM(uint32_t size, uint32_t address, uint8_t *RAM);
void MDFNMP_Kill(void);

void MDFNMP_InstallReadPatches(void);

void MDFNMP_ApplyPeriodicCheats(void);

void MDFN_LoadGameCheats(void);
void MDFN_FlushGameCheats(void);

/* Set or replace the cheat at libretro frontend slot `index`.
 *   - If index < current cheat count, the existing slot is overwritten
 *     in place (its name / conditions strings are freed first).
 *   - If index == current count, a new slot is appended.
 *   - If index > current count, the array grows with disabled
 *     placeholders filling the gap so the new cheat lands at index.
 * All cheats added through this API are type 'R' (periodic write).
 * Saturn cheats are conventionally big-endian: pass bigendian = true
 * for WorkRAM patches.  length is 1, 2, or 4 bytes; val carries the
 * value padded out to 64 bits.  CheatsActive is recomputed after the
 * update. */
void MDFNMP_SetCheat(unsigned index, bool enabled, uint32_t addr,
                     uint64_t val, unsigned length, bool bigendian);

/* Walk the cheats[] array and set CheatsActive = (>=1 enabled cheat).
 * Then RebuildSubCheats so the per-bucket SubCheats arrays are
 * consistent with the new flag.  Called automatically by SetCheat
 * and by the LoadGameCheats / FlushGameCheats path. */
void MDFNMP_RecomputeCheatsActive(void);

#ifdef __cplusplus
}
#endif

#endif
