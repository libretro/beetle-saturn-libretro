#ifndef _MDFN_SETTINGS_COMMON_H
#define _MDFN_SETTINGS_COMMON_H

#include <stdint.h>
#include <boolean.h>

typedef enum
{
   MDFNST_INT = 0,
   MDFNST_UINT,
   MDFNST_BOOL,
   MDFNST_FLOAT,
   MDFNST_STRING,
   MDFNST_ENUM,
   MDFNST_ALIAS
} MDFNSettingType;

#define MDFNSF_NOFLAGS		      0

typedef struct
{
   const char *string;
   int number;
   const char *description;
   const char *description_extra;
} MDFNSetting_EnumList;

typedef struct
{
   const char *name;
   uint32_t flags;
   const char *description;
   const char *description_extra;

   MDFNSettingType type;
   const char *default_value;
   const char *minimum;
   const char *maximum;
   bool (*validate_func)(const char *name, const char *value);
   void (*ChangeNotification)(const char *name);
   const MDFNSetting_EnumList *enum_list;
} MDFNSetting;

#endif
