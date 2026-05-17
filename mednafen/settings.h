#ifndef MDFN_SETTINGS_H
#define MDFN_SETTINGS_H

#include <stdint.h>
#include <boolean.h>

#ifdef __cplusplus
extern "C" {
#endif

extern bool DoHBlend;

// This should assert() or something if the setting isn't found, since it would
// be a totally tubular error!
uint64_t MDFN_GetSettingUI(const char *name);
int64_t MDFN_GetSettingI(const char *name);
bool MDFN_GetSettingB(const char *name);
const char *MDFN_GetSettingS(const char *name);

#ifdef __cplusplus
}
#endif

#endif
