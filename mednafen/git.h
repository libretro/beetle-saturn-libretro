#ifndef __MDFN_GIT_H
#define __MDFN_GIT_H

#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <libretro.h>

#include "video/surface.h"

#include "state.h"
#include "settings-common.h"

struct MemoryPatch;

struct CheatFormatStruct
{
 const char *FullName;		//"Game Genie", "GameShark", "Pro Action Catplay", etc.
 const char *Description;	// Whatever?

 bool (*DecodeCheat)(const std::string& cheat_string, MemoryPatch* patch);	// *patch should be left as initialized by MemoryPatch::MemoryPatch(), unless this is the
										// second(or third or whatever) part of a multipart cheat.
										//
										// Will throw an std::exception(or derivative) on format error.
										//
										// Will return true if this is part of a multipart cheat.
};

MDFN_HIDE extern const std::vector<CheatFormatStruct> CheatFormatInfo_Empty;

struct CheatInfoStruct
{
 //
 // InstallReadPatch and RemoveReadPatches should be non-NULL(even if only pointing to dummy functions) if the emulator module supports
 // read-substitution and read-substitution-with-compare style(IE Game Genie-style) cheats.
 //
 // See also "SubCheats" global stuff in mempatcher.h.
 //
 void (*InstallReadPatch)(uint32_t address, uint8_t value, int compare); // Compare is >= 0 when utilized.
 void (*RemoveReadPatches)(void);
 uint8_t (*MemRead)(uint32_t addr);
 void (*MemWrite)(uint32_t addr, uint8_t val);

 const std::vector<CheatFormatStruct>& CheatFormatInfo;

 bool BigEndian;	// UI default for cheat search and new cheats.
};

MDFN_HIDE extern const CheatInfoStruct CheatInfo_Empty;

/* EmulateSpecStruct now lives in mednafen/emuspec.h so it can be
   included from C TUs (the libretro entry-point is converted to C
   in the same commit that introduced emuspec.h).  This include
   gives C++ TUs the same POD typedef they had before; the only
   semantic change is that the C++-only default member initializers
   are gone -- callers that previously did `EmulateSpecStruct spec;`
   relying on those defaults now zero-init explicitly. */
#include "emuspec.h"

struct GameDB_Entry
{
 std::string GameID;
 bool GameIDIsHash = false;
 std::string Name;
 std::string Setting;
 std::string Purpose;
};

struct GameDB_Database
{
 std::string ShortName;
 std::string FullName;
 std::string Description;

 std::vector<GameDB_Entry> Entries;
};

//===========================================

#include "mdfn_gameinfo.h"

//===========================================

int StateAction(StateMem *sm, int load, int data_only);

extern retro_log_printf_t log_cb;

#endif
