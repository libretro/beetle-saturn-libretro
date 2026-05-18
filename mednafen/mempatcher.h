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

/* One memory-write operation as decoded from a libretro cheat code.
 * MDFNMP_SetCheat takes an array of these; a single retro_cheat_set
 * call may decode to multiple ops when the user supplies a '+'-joined
 * multi-line code.  length is 1, 2, or 4 bytes.  Saturn cheats use
 * bigendian = true (the SH-2 reads workram big-endian; the apply
 * path inside mempatcher handles host-endian byte ordering of the
 * underlying storage). */
typedef struct __MDFNCheatOp
{
	uint32_t addr;
	uint64_t val;
	unsigned length;
	bool     bigendian;
} MDFNCheatOp;

bool MDFNMP_Init(uint32_t ps, uint32_t numpages);
void MDFNMP_AddRAM(uint32_t size, uint32_t address, uint8_t *RAM);
void MDFNMP_Kill(void);

void MDFNMP_InstallReadPatches(void);

void MDFNMP_ApplyPeriodicCheats(void);

void MDFN_LoadGameCheats(void);
void MDFN_FlushGameCheats(void);

/* Replace the set of cheats associated with libretro frontend slot
 * `slot` with the `op_count` operations in `ops[]`.  All ops share
 * the single `enabled` flag (libretro's API carries one enable bit
 * per slot).  Each op becomes its own CHEATF row of type 'R' (the
 * periodic-write replace path) tagged with the slot index so the
 * next SetCheat at the same slot can find and remove the prior
 * occupants before appending the new ones.
 *
 * op_count == 0 is valid: it drops this slot's prior entries
 * without adding any.  Useful when the libretro frontend
 * intentionally disables a slot, or when a malformed code led
 * the parser to fail before producing any ops.
 *
 * CheatsActive is recomputed at the tail of this call. */
void MDFNMP_SetCheat(unsigned slot, bool enabled,
                     const MDFNCheatOp *ops, size_t op_count);

/* Walk the cheats[] array and set CheatsActive = (>=1 enabled cheat).
 * Then RebuildSubCheats so the per-bucket SubCheats arrays are
 * consistent with the new flag.  Called automatically by SetCheat
 * and by the LoadGameCheats / FlushGameCheats path. */
void MDFNMP_RecomputeCheatsActive(void);

#ifdef __cplusplus
}
#endif

#endif
