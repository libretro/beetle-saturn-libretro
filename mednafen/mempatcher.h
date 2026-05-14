#ifndef __MDFN_MEMPATCHER_H
#define __MDFN_MEMPATCHER_H

#include "mempatcher-driver.h"
#include <vector>

typedef struct __SUBCHEAT
{
	uint32_t addr;
	uint8_t value;
	int compare; // < 0 on no compare
} SUBCHEAT;

extern std::vector<SUBCHEAT> SubCheats[8];
extern bool SubCheatsOn;

bool MDFNMP_Init(uint32_t ps, uint32_t numpages);
void MDFNMP_AddRAM(uint32_t size, uint32_t address, uint8_t *RAM);
void MDFNMP_Kill(void);


void MDFNMP_InstallReadPatches(void);
void MDFNMP_RemoveReadPatches(void);

void MDFNMP_ApplyPeriodicCheats(void);
void MDFNMP_RegSearchable(uint32_t addr, uint32_t size);

extern MDFNSetting MDFNMP_Settings[];

#endif
