#ifndef __MDFN_MEMPATCHER_DRIVER_H
#define __MDFN_MEMPATCHER_DRIVER_H

struct MemoryPatch
{
   MemoryPatch();
   ~MemoryPatch();
   std::string name;
   std::string conditions;
   uint32_t addr;
   uint64_t val;
   uint64_t compare;
   uint32_t mltpl_count;
   uint32_t mltpl_addr_inc;
   uint64_t mltpl_val_inc;
   uint32_t copy_src_addr;
   uint32_t copy_src_addr_inc;
   unsigned length;
   bool bigendian;
   bool status; // (in)active
   unsigned icount;
   char type; /* 'R' for replace, 'S' for substitute(GG), 'C' for substitute with compare */
   /* 'T' for copy/transfer data, 'A' for add(variant of type R) */
   //enum { TypeReplace, TypeSubst, TypeCompSubst };
   //int type;
};

int MDFNI_DecodePAR(const char *code, uint32_t *a, uint8_t *v, uint8_t *c, char *type);
int MDFNI_DecodeGG(const char *str, uint32_t *a, uint8_t *v, uint8_t *c, char *type);
int MDFNI_AddCheat(const char *name, uint32_t addr, uint64_t val, uint64_t compare, char type, unsigned int length, bool bigendian);
int MDFNI_DelCheat(uint32_t which);
int MDFNI_ToggleCheat(uint32_t which);

int32_t MDFNI_CheatSearchGetCount(void);
void MDFNI_CheatSearchGetRange(uint32_t first, uint32_t last, int (*callb)(uint32_t a, uint8_t last, uint8_t current));
void MDFNI_CheatSearchGet(int (*callb)(uint32_t a, uint64_t last, uint64_t current, void *data), void *data);
void MDFNI_CheatSearchBegin(void);
void MDFNI_CheatSearchEnd(int type, uint64_t v1, uint64_t v2, unsigned int bytelen, bool bigendian);
void MDFNI_ListCheats(int (*callb)(char *name, uint32_t a, uint64_t v, uint64_t compare, int s, char type, unsigned int length, bool bigendian, void *data), void *data);

int MDFNI_GetCheat(uint32_t which, char **name, uint32_t *a, uint64_t *v, uint64_t *compare, int *s, char *type, unsigned int *length, bool *bigendian);
int MDFNI_SetCheat(uint32_t which, const char *name, uint32_t a, uint64_t v, uint64_t compare, int s, char type, unsigned int length, bool bigendian);

void MDFNI_CheatSearchShowExcluded(void);
void MDFNI_CheatSearchSetCurrentAsOriginal(void);

#endif
