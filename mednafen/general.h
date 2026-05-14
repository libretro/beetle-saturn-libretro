#ifndef _GENERAL_H
#define _GENERAL_H

#include <stddef.h>

extern uint32_t MDFN_RoundUpPow2(uint32_t);

void GetFileBase(const char *f);

typedef enum
{
 MDFNMKF_STATE = 0,
 MDFNMKF_SNAP,
 MDFNMKF_SAV,
 MDFNMKF_CART,
 MDFNMKF_CHEAT,
 MDFNMKF_PALETTE,
 MDFNMKF_IPS,
 MDFNMKF_MOVIE,
 MDFNMKF_AUX,
 MDFNMKF_SNAP_DAT,
 MDFNMKF_CHEAT_TMP,
 MDFNMKF_FIRMWARE
} MakeFName_Type;

// Caller-allocated buffer (buf, buflen). Returns buf for chaining.
// See libretro.cpp definition for the migration rationale.
char *MDFN_MakeFName(char *buf, size_t buflen, MakeFName_Type type, int id1, const char *cd1);

const char * GetFNComponent(const char *str);

// Split file_path into directory / base name / extension. Any of the
// three output buffers may be NULL to skip that component; out_size
// applies to whichever buffers are non-NULL.
void MDFN_GetFilePathComponents(const char *file_path,
      char *dir_path_out, char *file_base_out,
      char *file_ext_out, size_t out_size);

// Resolve rel_path against dir_path into the caller-supplied buffer.
// Absolute rel_path values are copied through unchanged.
void MDFN_EvalFIP(char *out, size_t out_size, const char *dir_path, const char *rel_path);
#endif
