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
void MDFNMP_RemoveReadPatches(void);

void MDFNMP_ApplyPeriodicCheats(void);
void MDFNMP_RegSearchable(uint32_t addr, uint32_t size);

void MDFN_LoadGameCheats(void);
void MDFN_FlushGameCheats(void);

/* Cheat code decoders and the cheat-list interface. Defined in
 * mempatcher.c; currently no in-tree caller, but kept declared so the
 * definitions are prototyped and remain reachable. */
int MDFNI_DecodePAR(const char *str, uint32_t *a, uint8_t *v, uint8_t *c, char *type);
int MDFNI_DecodeGG(const char *str, uint32_t *a, uint8_t *v, uint8_t *c, char *type);
bool MDFNI_DecodeGBGG(const char *instr, uint32_t *a, uint8_t *v, uint8_t *c, char *type);
int MDFNI_AddCheat(const char *name, uint32_t addr, uint64_t val, uint64_t compare, char type, unsigned int length, bool bigendian);
int MDFNI_DelCheat(uint32_t which);
int MDFNI_ToggleCheat(uint32_t which);
int MDFNI_GetCheat(uint32_t which, char **name, uint32_t *a, uint64_t *v, uint64_t *compare, int *s, char *type, unsigned int *length, bool *bigendian);
int MDFNI_SetCheat(uint32_t which, const char *name, uint32_t a, uint64_t v, uint64_t compare, int s, char type, unsigned int length, bool bigendian);
void MDFNI_ListCheats(int (*callb)(char *name, uint32_t a, uint64_t v, uint64_t compare, int s, char type, unsigned int length, bool bigendian, void *data), void *data);

extern MDFNSetting MDFNMP_Settings[];

#ifdef __cplusplus
}
#endif

#endif
