/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss_init.c - FastMemMap + SH-2 event-system orchestration.
**             Phase-7c extraction from ss.cpp.  Holds the page-mapped
**             SH7095_FastMap table that every SH-2 memory access goes
**             through, the FMIsWriteable bitmap, the event ring, the
**             event-handler dispatch loop, and the public-ABI entry
**             points that fire events from outside ss.cpp.
**
**             Nothing here touches the SH-2 / SCU / SCSP / M68K class
**             interiors -- the few cross-language entry points needed
**             (CART_GetEventHandler, the various subsystem _Update
**             handlers stuffed into event_handlers[] at boot) are
**             already extern "C" function pointers.
**
**  Copyright (C) 2015-2023 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include <mednafen/mednafen-types.h>
/* mednafen.h pulls in git.h which #includes <algorithm> -- C++-only.
 * The only thing ss_init.c needs from mednafen.h is the _() identity
 * macro for translation strings.  Define it directly. */
#ifndef _
#define _(String) (String)
#endif
#include "ss.h"
#include "ss_init.h"

#include "scu.h"     /* SCU_UpdateDMA, SCU_UpdateDSP, SS_EVENT_SCU_* */
#include "smpc.h"    /* SMPC_Update */
#include "vdp1.h"    /* VDP1_Update */
#include "vdp2.h"    /* VDP2_Update */
#include "cdb.h"     /* CDB_Update */
#include "sound.h"   /* SOUND_Update */
#include "cart.h"    /* CART_GetEventHandler */
#include "stvio.h"   /* STVIO_UpdateInput */
#include "db.h"      /* CPUCACHE_EMUMODE_* */
#include "ss_state.h" /* BRAM_Init_Data, SS_Load{RTC,BackupRAM,CartNV},
                       * SS_Backup{BackupRAM,CartNV} */

#include "../mempatcher.h"
#include "../settings.h"
#include "../hash/sha256.h"
#include <mednafen/emuspec.h>
#include <mednafen/mdfn_gameinfo.h>
#include "../general.h"           /* MDFN_MidSync, log_cb (via cdstream.h) */
#include "../../libretro_settings.h" /* setting_midsync, setting_multitap_port*,
                                      * retro_base_directory */
#include "../../disc.h"           /* disc_cleanup */
#include <streams/file_stream.h>  /* filestream_open/read/close/get_size */
#include <libretro.h>             /* RFILE, RETRO_VFS_*, RETRO_LOG_* */
#include <retro_miscellaneous.h>  /* ARRAY_SIZE */
#include <time.h>                 /* time, localtime, struct tm */

/* Some libretro front-end log pointer + RETRO_SLASH choice -- match
 * ss.cpp's earlier conditional. */
#ifdef _WIN32
#define RETRO_SLASH "\\"
#else
#define RETRO_SLASH "/"
#endif

extern retro_log_printf_t log_cb;

/* ===================================================================
 * FastMemMap
 * =================================================================== */

uintptr_t SH7095_FastMap[1U << (32 - SH7095_EXT_MAP_GRAN_BITS)];
uint32_t  FMIsWriteable[FMISWRITEABLE_BITS / 32];

static uint16_t fmap_dummy[(1U << SH7095_EXT_MAP_GRAN_BITS) / sizeof(uint16_t)];

static void SetFastMemMap(uint32_t Astart, uint32_t Aend, uint16_t* ptr, uint32_t length, bool is_writeable)
{
 const uint64_t Abound = (uint64_t)Aend + 1;
 uint64_t A;

 assert((Astart & ((1U << SH7095_EXT_MAP_GRAN_BITS) - 1)) == 0);
 assert((Abound & ((1U << SH7095_EXT_MAP_GRAN_BITS) - 1)) == 0);
 assert((length & ((1U << SH7095_EXT_MAP_GRAN_BITS) - 1)) == 0);
 assert(length > 0);
 assert(length <= (Abound - Astart));

 for(A = Astart; A < Abound; A += (1U << SH7095_EXT_MAP_GRAN_BITS))
 {
  uintptr_t tmp = (uintptr_t)ptr + ((A - Astart) % length);

  if(A < (1U << 27))
   FMIsWriteable_set(A >> SH7095_EXT_MAP_GRAN_BITS, is_writeable);

  SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS] = tmp - A;
 }
}

bool InitFastMemMap(void)
{
 unsigned i;
 uint64_t A;

 for(i = 0; i < sizeof(fmap_dummy) / sizeof(fmap_dummy[0]); i++)
 {
  fmap_dummy[i] = 0;
 }

 FMIsWriteable_reset();

 /* MDFNMP_Init returns false on RAMPtrs calloc failure; the rest of
  * InitFastMemMap and InitCommon downstream (MDFNMP_RegSearchable,
  * MDFNMP_AddRAM, the cheat search machinery) assume RAMPtrs is a
  * live array, so a NULL there would crash on the first patch /
  * cheat install.  Propagate the failure instead. */
 if(!MDFNMP_Init(1ULL << SH7095_EXT_MAP_GRAN_BITS, (1ULL << 27) / (1ULL << SH7095_EXT_MAP_GRAN_BITS)))
  return false;

 for(A = 0; A < 1ULL << 32; A += (1U << SH7095_EXT_MAP_GRAN_BITS))
 {
  SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS] = (uintptr_t)fmap_dummy - A;
 }

 return true;
}

void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t* ptr, uint32_t length, bool is_writeable)
{
 uint32_t Abase;

 assert(Astart < 0x20000000);
 assert(Aend < 0x20000000);

 if(!ptr)
 {
  ptr = fmap_dummy;
  length = sizeof(fmap_dummy);
 }

 for(Abase = 0; Abase < 0x40000000; Abase += 0x20000000)
  SetFastMemMap(Astart + Abase, Aend + Abase, ptr, length, is_writeable);
}

uint8_t CheatMemRead(uint32_t A)
{
 A &= (1U << 27) - 1;

 /* ne16_rbo_be<uint8_t>(base, A) folded - byte read from BE bus
  * over uint16_t fast-map slot. */
#ifdef MSB_FIRST
 return ((const uint8_t*)SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS])[A];
#else
 return ((const uint8_t*)SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS])[A ^ 1];
#endif
}

/* ===================================================================
 * Event system
 * =================================================================== */

int Running;
__attribute__((aligned(16))) event_list_entry events[SS_EVENT__SIMD_COUNT];
ss_event_handler event_handlers[SS_EVENT__COUNT];
sscpu_timestamp_t next_event_ts;

/* NO_INLINE keeps the body out of any caller's no-unroll pragma scope
 * so -O2 auto-vectorizes the reduction (smin/sminv on aarch64). */
NO_INLINE sscpu_timestamp_t FindNextEventTS(void)
{
 sscpu_timestamp_t m = SS_EVENT_DISABLED_TS;
 unsigned i;
 for(i = 0; i < SS_EVENT__SIMD_COUNT; i++)
  m = ((m) < (events[i].event_time) ? (m) : (events[i].event_time));
 return m;
}

/* Phase-7a: was `template<unsigned c> static sscpu_timestamp_t
 * SH_DMA_EventHandler(sscpu_timestamp_t et)` with c instantiated
 * to 0 (master SH-2) and 1 (slave SH-2).  The C++ body called
 * CPU[c].DMA_Update(et); the C-side rewrite reaches the same
 * dispatch through the extern "C" wrappers SH7095_M_DMA_Update /
 * SH7095_S_DMA_Update (defined in ss.cpp, where the SH7095 class
 * type still lives -- this gets retired once the SH7095 class is
 * fully converted to a C struct in a later phase). */
extern int32_t SH7095_M_DMA_Update(int32_t et);
extern int32_t SH7095_S_DMA_Update(int32_t et);
extern uint32_t SH7095_BusLock;

#define SH_DMA_EVENT_HANDLER_BODY(UPDATE_FN)                                                       \
{                                                                                                  \
 if(et < SH7095_mem_timestamp)                                                                     \
  return SH7095_mem_timestamp;                                                                     \
                                                                                                   \
 /* Must come after the (et < SH7095_mem_timestamp) check. */                                      \
 if(MDFN_UNLIKELY(SH7095_BusLock))                                                                 \
  return et + 1;                                                                                   \
                                                                                                   \
 return UPDATE_FN(et);                                                                             \
}

static sscpu_timestamp_t SH_DMA_EventHandler_M(sscpu_timestamp_t et) SH_DMA_EVENT_HANDLER_BODY(SH7095_M_DMA_Update)
static sscpu_timestamp_t SH_DMA_EventHandler_S(sscpu_timestamp_t et) SH_DMA_EVENT_HANDLER_BODY(SH7095_S_DMA_Update)

#undef SH_DMA_EVENT_HANDLER_BODY

void InitEvents(void)
{
 unsigned i;

 /* SYNFIRST/SYNLAST and padding slots stay disabled so the min-reduction
  * ignores them; only [SYNFIRST+1, SYNLAST) hold real events. */
 for(i = 0; i < SS_EVENT__SIMD_COUNT; i++)
 {
  if(i == SS_EVENT__SYNFIRST || i == SS_EVENT__SYNLAST || i >= SS_EVENT__COUNT)
   events[i].event_time = SS_EVENT_DISABLED_TS;
  else
   events[i].event_time = 0;
 }

 for(i = 0; i < SS_EVENT__COUNT; i++)
  event_handlers[i] = NULL;

 event_handlers[SS_EVENT_SH2_M_DMA] = &SH_DMA_EventHandler_M;
 event_handlers[SS_EVENT_SH2_S_DMA] = &SH_DMA_EventHandler_S;

 event_handlers[SS_EVENT_SCU_DMA] = SCU_UpdateDMA;
 event_handlers[SS_EVENT_SCU_DSP] = SCU_UpdateDSP;
 /*event_handlers[SS_EVENT_SCU_INT] = SCU_UpdateInt;*/

 event_handlers[SS_EVENT_SMPC] = SMPC_Update;

 event_handlers[SS_EVENT_VDP1] = VDP1_Update;
 event_handlers[SS_EVENT_VDP2] = VDP2_Update;

 event_handlers[SS_EVENT_CDB] = CDB_Update;

 event_handlers[SS_EVENT_SOUND] = SOUND_Update;

 event_handlers[SS_EVENT_CART] = CART_GetEventHandler();

 event_handlers[SS_EVENT_MIDSYNC] = MidSync;
 /*  */
 SS_SetEventNT(&events[SS_EVENT_MIDSYNC], SS_EVENT_DISABLED_TS);
}

void RebaseTS(const sscpu_timestamp_t timestamp)
{
 unsigned i;
 for(i = SS_EVENT__SYNFIRST + 1; i < SS_EVENT__SYNLAST; i++)
 {
  assert(events[i].event_time > timestamp);

  if(events[i].event_time != SS_EVENT_DISABLED_TS)
   events[i].event_time -= timestamp;
 }

 next_event_ts = FindNextEventTS();
}

void SS_SetEventNT(event_list_entry* e, const sscpu_timestamp_t next_timestamp)
{
 const sscpu_timestamp_t old_t = e->event_time;
 e->event_time = next_timestamp;

 if(MDFN_UNLIKELY(Running <= 0))
  next_event_ts = 0;
 else if(old_t == next_event_ts)
  next_event_ts = FindNextEventTS();
 else if(next_timestamp < next_event_ts)
  next_event_ts = next_timestamp;
}

/* EventHandler was static INLINE; promoted to TU-external in phase 7c
 * because ss.cpp's RunLoop template body calls it.  Keeping it INLINE
 * (declared as such in ss_init.h via the prototype) lets gcc/LTO
 * fold it back into the hot loop at link time. */
bool EventHandler(const sscpu_timestamp_t timestamp)
{
 sscpu_timestamp_t best_t;
 /* next_event_ts is forced to 0 (sentinel) when Running <= 0 to make
  * CheckEventsByMemTS trip and unwind RunLoop. Don't enter the dispatch
  * loop in that state -- best_t = 0 wouldn't match any
  * events[i].event_time and the inner scan would walk off the end. */
 if(MDFN_UNLIKELY(Running <= 0))
  return false;
 best_t = next_event_ts;
 while(best_t <= timestamp)
 {
  unsigned best_i = SS_EVENT__SYNFIRST + 1;
  while(events[best_i].event_time != best_t)
   best_i++;
  events[best_i].event_time = event_handlers[best_i](best_t);
  best_t = FindNextEventTS();
 }

 next_event_ts = (Running > 0) ? best_t : 0;
 return Running > 0;
}

static void CheckEventsByMemTS_Sub(void)
{
 EventHandler(SH7095_mem_timestamp);
}

void CheckEventsByMemTS(void)
{
 if(MDFN_UNLIKELY(SH7095_mem_timestamp >= next_event_ts))
  CheckEventsByMemTS_Sub();
}

void SS_RequestEHLExit(void)
{
 if(Running)
 {
  Running = -1;
  next_event_ts = 0;
 }
}

void SS_RequestMLExit(void)
{
 Running = 0;
 next_event_ts = 0;
}

/* ===================================================================
 * Phase-7e: per-frame Emulate() loop + MidSync helper
 * =================================================================== */

/* Externs into ss.cpp -- promoted to TU-external in phase 7d. */
extern bool          NeedEmuICache;
extern int           ActiveCartType;
extern int64_t       UpdateInputLastBigTS;

/* Externs for the libretro front-end / game-info structs. */
extern MDFNGI        EmulatedSS;
extern uint32_t      IBufferCount;

/* C wrappers that ss.cpp publishes for our use (extern "C" defined
 * there); these forward into the SH7095 class instances and the
 * template-parameterised RunLoop body that still live in C++.
 * Each retires once the SH7095 class becomes a C struct. */
int32_t SS_RunLoop_ICache(struct EmulateSpecStruct* espec);
int32_t SS_RunLoop_NoICache(struct EmulateSpecStruct* espec);
void    SS_ForceEventUpdates(int32_t timestamp);
void    SH7095_M_AdjustTS(int32_t delta);
void    SH7095_S_AdjustTS(int32_t delta);

/* Frame-scoped state.  espec is the active EmulateSpecStruct
 * pointer Emulate received this frame -- shared with MidSync via
 * file-static visibility. AllowMidSync gates whether MidSync's
 * one-shot mid-frame callback fires (front-end flag); reset on
 * each frame, cleared on first fire. cur_clock_div is SMPC's
 * frame-start clock divisor (carried for the per-tick input
 * elapsed-time conversion). */
static struct EmulateSpecStruct* espec;
static bool                      AllowMidSync;
static int32_t                   cur_clock_div;

static INLINE void UpdateSMPCInput(const sscpu_timestamp_t timestamp)
{
 int32_t elapsed_time;

 SMPC_TransformInput();

 elapsed_time = (((int64_t)timestamp * cur_clock_div * 1000 * 1000) - UpdateInputLastBigTS) / (EmulatedSS.MasterClock / MDFN_MASTERCLOCK_FIXED(1));

 UpdateInputLastBigTS += (int64_t)elapsed_time * (EmulatedSS.MasterClock / MDFN_MASTERCLOCK_FIXED(1));

 /* ST-V samples gamepad/gun/coin state into its own DataIn buffer on
  * the same cadence SMPC samples virtual ports. */
 if(ActiveCartType == CART_STV)
   STVIO_UpdateInput(elapsed_time);

 SMPC_UpdateInput(elapsed_time);
}

sscpu_timestamp_t MidSync(const sscpu_timestamp_t timestamp)
{
 if(AllowMidSync)
 {
    SMPC_UpdateOutput();

    MDFN_MidSync();

    UpdateSMPCInput(timestamp);

    AllowMidSync = false;
 }

 return SS_EVENT_DISABLED_TS;
}

void Emulate(struct EmulateSpecStruct* espec_arg)
{
 int32_t end_ts;
 unsigned c;

 espec = espec_arg;
 AllowMidSync = setting_midsync;

 cur_clock_div = SMPC_StartFrame();
 UpdateSMPCInput(0);
 VDP2_StartFrame(espec, cur_clock_div == 61);
 CART_SetCPUClock(EmulatedSS.MasterClock / MDFN_MASTERCLOCK_FIXED(1), cur_clock_div);
 espec->SoundBufSize = 0;
 espec->MasterCycles = 0;

 if (NeedEmuICache)
  end_ts = SS_RunLoop_ICache(espec);
 else
  end_ts = SS_RunLoop_NoICache(espec);
 assert(end_ts >= 0);

 SS_ForceEventUpdates(end_ts);

 SMPC_EndFrame(espec, end_ts);

 RebaseTS(end_ts);

 CDB_ResetTS();
 SOUND_AdjustTS(-end_ts);
 VDP1_AdjustTS(-end_ts);
 VDP2_AdjustTS(-end_ts);
 SMPC_ResetTS();
 SCU_AdjustTS(-end_ts);
 CART_AdjustTS(-end_ts);

 UpdateInputLastBigTS -= (int64_t)end_ts * cur_clock_div * 1000 * 1000;

 SH7095_mem_timestamp -= end_ts; /* Update before SH7095 AdjustTS calls. */

 /* CPU[c].AdjustTS(-end_ts) for c in {0,1} via extern "C" wrappers. */
 SH7095_M_AdjustTS(-end_ts);
 SH7095_S_AdjustTS(-end_ts);
 (void)c;

 espec->MasterCycles  = (int64_t)end_ts * cur_clock_div;
 espec->SoundBufSize += IBufferCount;
 IBufferCount         = 0;

 SMPC_UpdateOutput();

 /* Backup-RAM and cart-NV dirty tracking.
  *
  * Previously this block performed synchronous file I/O from inside
  * Emulate() (SaveBackupRAM/SaveCartNV under a master-cycle countdown).
  * That had two big problems for a libretro core:
  *   1. Run-ahead / rewind / netplay re-emulate frames repeatedly. The
  *      cycle-counted delay fires identically on each pass, so a single
  *      real frame could produce two or three full SaveBackupRAM disk
  *      writes -- visible as stutter and unstable frame pacing.
  *   2. The save is fully synchronous (FileStream::write+close) on the
  *      emulation thread, so the duration is unpredictable under load.
  *
  * The fix is to keep BackupRAM_Dirty (and the cart-NV dirty bit) as
  * pure flags here, and let libretro.cpp flush them from retro_run --
  * outside Emulate, with awareness of RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE
  * so run-ahead simulation frames don't trigger writes. The frontend
  * can also manage Backup RAM directly via RETRO_MEMORY_SAVE_RAM. */
 if(CART_GetClearNVDirty())
  CartNV_Dirty = true;
}

/* ===================================================================
 * Phase-7f: InitCommon / SS_Reset / Cleanup / CloseGame /
 *           MDFN_BackupSavFile
 *
 * These are the boot-time orchestration entry points the libretro
 * front-end calls (InitCommon from retro_load_game, SS_Reset from
 * retro_reset, CloseGame from retro_unload_game).  Each reaches
 * into the SH7095 CPU[2] instances that still live in ss.cpp via
 * extern "C" wrappers added there in this phase
 * (SH7095_{M,S}_{Init,SetMD5,TruePowerOn,Reset}).  The wrappers
 * retire once the SH7095 class becomes a C struct.
 * =================================================================== */

/* Externs into ss.cpp (TU-external definitions live there). */
extern uint8_t       WorkRAM[];
extern uint16_t*     WorkRAML;
extern uint16_t*     WorkRAMH;
extern uint16_t      BIOSROM[];   /* 524288 / sizeof(uint16_t) entries */
extern uint32_t      SH7095_DB;
extern uint32_t      ss_horrible_hacks;
extern bool          is_pal;
extern sha256_digest BIOS_SHA256;

/* Defined in libretro.c (no header to include for this -- ss.cpp and
 * settings.c each redeclare it locally; match that pattern). */
extern char retro_base_directory[4096];

/* SH7095 method dispatch wrappers (extern "C" defined in ss.cpp). */
void SH7095_M_Init(const bool emumode_full, const bool emumode_cb_only) MDFN_COLD;
void SH7095_S_Init(const bool emumode_full, const bool emumode_cb_only) MDFN_COLD;
void SH7095_M_SetMD5(bool level);
void SH7095_S_SetMD5(bool level);
void SH7095_M_TruePowerOn(void) MDFN_COLD;
void SH7095_S_TruePowerOn(void) MDFN_COLD;
void SH7095_M_Reset(bool power_on_reset)    MDFN_COLD;

typedef struct
{
   const unsigned type;
   const char *name;
} CartName;

static MDFN_COLD void Cleanup(void)
{
 CART_Kill();

 VDP1_Kill();
 VDP2_Kill();
 SOUND_Kill();
 CDB_Kill();
 STVIO_Kill();
 SMPC_Kill();

 disc_cleanup();
}

bool MDFN_COLD InitCommon(const unsigned cpucache_emumode, const unsigned horrible_hacks, const unsigned cart_type, const unsigned smpc_area, const char* rom_dir, const char* main_fname, const struct STVGameInfo* sgi)
{
   unsigned i;
   /* C99 hoists block-scoped declarations; in C99 they can live inside
    * the if-bodies they belong to, matching the original C++ style. */
   {
      typedef struct {
         unsigned mode;
         const char* name;
      } CPUCacheEmuMode;
      static const CPUCacheEmuMode CPUCacheEmuModes[] =
      {
         { CPUCACHE_EMUMODE_DATA_CB, _("Data only, with high-level bypass") },
         { CPUCACHE_EMUMODE_DATA,    _("Data only") },
         { CPUCACHE_EMUMODE_FULL,    _("Full") },
      };
      const char* cem = _("Unknown");
      unsigned k;

      for(k = 0; k < ARRAY_SIZE(CPUCacheEmuModes); k++)
      {
         if(CPUCacheEmuModes[k].mode == cpucache_emumode)
         {
            cem = CPUCacheEmuModes[k].name;
            break;
         }
      }
      log_cb(RETRO_LOG_INFO, "CPU Cache Emulation Mode: %s\n", cem);
   }

   if(horrible_hacks)
      log_cb(RETRO_LOG_INFO, "Horrible hacks: 0x%08x\n", horrible_hacks);

   {
      const CartName CartNames[] =
      {
         { CART_NONE,       "None" },
         { CART_BACKUP_MEM, "Backup Memory" },
         { CART_EXTRAM_1M,  "1MiB Extended RAM" },
         { CART_EXTRAM_4M,  "4MiB Extended RAM" },
         { CART_KOF95,      "King of Fighters '95 ROM" },
         { CART_ULTRAMAN,   "Ultraman ROM" },
         { CART_CS1RAM_16M, _("16MiB CS1 RAM") },
         { CART_NLMODEM,    _("Netlink Modem") },
         { CART_STV,        _("Sega Titan Video (ST-V)") },
         { CART_BOOTROM,    _("Bootable ROM") }
      };
      const char* cn = NULL;

      log_cb(RETRO_LOG_INFO, "Region: 0x%01x.\n", smpc_area);

      for(i = 0; i < ARRAY_SIZE(CartNames); i++)
      {
         if(CartNames[i].type != cart_type)
            continue;
         cn = CartNames[i].name;
         break;
      }
      if(cn)
         log_cb(RETRO_LOG_INFO, "Cart: %s.\n", cn);
      else
         log_cb(RETRO_LOG_INFO, "Cart: Unknown (%d).\n", cart_type);
   }

   NeedEmuICache = (cpucache_emumode == CPUCACHE_EMUMODE_FULL);
   SH7095_M_Init((cpucache_emumode == CPUCACHE_EMUMODE_FULL), (cpucache_emumode == CPUCACHE_EMUMODE_DATA_CB));
   SH7095_S_Init((cpucache_emumode == CPUCACHE_EMUMODE_FULL), (cpucache_emumode == CPUCACHE_EMUMODE_DATA_CB));
   SH7095_M_SetMD5(false);
   SH7095_S_SetMD5(true);

   SH7095_mem_timestamp = 0;
   SH7095_DB = 0;

   ss_horrible_hacks = horrible_hacks;

   /* Initialise backup memory. */
   memset(BackupRAM, 0x00, sizeof(BackupRAM));
   for(i = 0; i < 0x40; i++)
      BackupRAM[i] = BRAM_Init_Data[i & 0x0F];

   /* Call InitFastMemMap() before functions like SOUND_Init(). */
   if(!InitFastMemMap())
   {
      Cleanup();
      return false;
   }
   SS_SetPhysMemMap(0x00000000, 0x000FFFFF, BIOSROM, 512 * 1024, false);
   SS_SetPhysMemMap(0x00200000, 0x003FFFFF, WorkRAML, WORKRAM_BANK_SIZE_BYTES, true);
   SS_SetPhysMemMap(0x06000000, 0x07FFFFFF, WorkRAMH, WORKRAM_BANK_SIZE_BYTES, true);
   MDFNMP_RegSearchable(0x00200000, WORKRAM_BANK_SIZE_BYTES);
   MDFNMP_RegSearchable(0x06000000, WORKRAM_BANK_SIZE_BYTES);

   if(!CART_Init(cart_type, rom_dir, main_fname, sgi))
   {
      Cleanup();
      return false;
   }
   ActiveCartType = cart_type;

   {
      const bool is_stv = (cart_type == CART_STV);
      /* ST-V runs on a monitor, not on a TV; force 60 Hz timing
       * regardless of region. PAL ST-V cabinets exist but the video
       * signal is still 60 Hz. */
      const bool PAL = is_pal = (!is_stv) && (smpc_area & SMPC_AREA__PAL_MASK);
      const int32_t MasterClock = PAL ? 1734687500 : 1746818182; /* NTSC: 1746818181.818..., PAL: 1734687500-ish */
      const char* bios_filename;
      int sls = MDFN_GetSettingI(PAL ? "ss.slstartp" : "ss.slstart");
      int sle = MDFN_GetSettingI(PAL ? "ss.slendp"   : "ss.slend");
      const uint64_t vdp2_affinity = 0; /* LibRetro: unused */

      if(sls > sle)
      {
         /* std::swap(sls, sle) folded; braced because this is an
          * unbraced if body. */
         int tmp_sl = sls;
         sls = sle;
         sle = tmp_sl;
      }

      if(is_stv)
      {
         /* ST-V BIOSes are 128 KiB (vs Saturn's 512 KiB) and live in their
          * own filename namespace. Hardcoded to match the fork's existing
          * BIOS-filename convention for sega_101.bin / mpr-17933.bin.
          * Users supply the actual files in retro_base_directory.
          *   JP / Asia-NTSC:                              epr-20091.ic8
          *   NA / CSA-NTSC / CSA-PAL:                     epr-17952a.ic8
          *   EU / Asia-PAL / Korea / everything else:     epr-17954a.ic8 */
         if(smpc_area == SMPC_AREA_JP || smpc_area == SMPC_AREA_ASIA_NTSC)
            bios_filename = "epr-20091.ic8";
         else if(smpc_area == SMPC_AREA_NA || smpc_area == SMPC_AREA_CSA_NTSC || smpc_area == SMPC_AREA_CSA_PAL)
            bios_filename = "epr-17952a.ic8";
         else
            bios_filename = "epr-17954a.ic8";
      }
      else if(smpc_area == SMPC_AREA_JP || smpc_area == SMPC_AREA_ASIA_NTSC)
         bios_filename = "sega_101.bin"; /* Japan */
      else
         bios_filename = "mpr-17933.bin"; /* North America and Europe */

      {
         char bios_path[4096];
         RFILE *BIOSFile;
         int64_t bios_size;
         unsigned bw;

         snprintf(bios_path, sizeof(bios_path), "%s" RETRO_SLASH "%s", retro_base_directory, bios_filename);

         BIOSFile = filestream_open(bios_path,
               RETRO_VFS_FILE_ACCESS_READ,
               RETRO_VFS_FILE_ACCESS_HINT_NONE);

         if(!BIOSFile)
         {
            log_cb(RETRO_LOG_ERROR, "Cannot open BIOS file \"%s\".\n", bios_path);
            Cleanup();
            return false;
         }

         /* Saturn BIOSes are 512 KiB; ST-V BIOSes are 128 KiB
          * (mapped into the upper half of the 512 KiB BIOSROM[] array
          * and read by the SH-2 from the same 0x00000000-0x000FFFFF
          * window). Accept either size.
          *
          * filestream_get_size must come AFTER the BIOSFile NULL check
          * -- on filestream_open failure BIOSFile is NULL and passing
          * it to get_size would be undefined. */
         bios_size = filestream_get_size(BIOSFile);
         if(bios_size != 524288 && !(is_stv && bios_size == 131072))
         {
            log_cb(RETRO_LOG_ERROR, "BIOS file \"%s\" is of an incorrect size.\n", bios_path);
            filestream_close(BIOSFile);
            Cleanup();
            return false;
         }

         memset(BIOSROM, 0xFF, 512 * 1024);
         /* Short read between get_size and the actual read would
          * leave BIOSROM half-loaded (head: partial BIOS bytes,
          * tail: 0xFF from the memset above), BIOS_SHA256 would
          * hash the corrupted data, and the byte-swap loop below
          * would scramble it further.  Fail init with a clear
          * error rather than silently emulating with a corrupted
          * BIOS image. */
         if(filestream_read(BIOSFile, BIOSROM, bios_size) != bios_size)
         {
            log_cb(RETRO_LOG_ERROR, "BIOS file \"%s\" could not be fully read (short or failed read).\n", bios_path);
            filestream_close(BIOSFile);
            Cleanup();
            return false;
         }
         filestream_close(BIOSFile);
         BIOS_SHA256 = sha256(BIOSROM, 512 * 1024);

         /* swap endian-ness */
         for(bw = 0; bw < (unsigned)(bios_size / 2); bw++)
         {
            /* MDFN_de16msb folded: BE-on-disk to host-endian. */
#ifndef MSB_FIRST
            BIOSROM[bw] = (uint16_t)((BIOSROM[bw] << 8) | (BIOSROM[bw] >> 8));
#endif
         }
      }

      EmulatedSS.MasterClock = MDFN_MASTERCLOCK_FIXED(MasterClock);

      SCU_Init();
      SMPC_Init(smpc_area, MasterClock, is_stv);
      VDP1_Init();
      VDP2_Init(PAL, vdp2_affinity);
      VDP2_SetGetVideoParams(&EmulatedSS, true, sls, sle, true, DoHBlend);
      CDB_Init();
      SOUND_Init();

      InitEvents();
      UpdateInputLastBigTS = 0;

      /* Apply multi-tap state to SMPC. */
      SMPC_SetMultitap(0, setting_multitap_port1);
      SMPC_SetMultitap(1, setting_multitap_port2);

      if(is_stv)
      {
         /* ST-V provides its own SMPC port shim (handles AK93C45 EEPROM
          * and 68K sound-CPU control), and routes player input through
          * STVIO_SetInput / STVIO_UpdateInput rather than the standard
          * per-virtual-port path. Init the I/O board first, then inject
          * the port shims into SMPC super-ports 0 and 1. */
         unsigned sp;
         STVIO_Init(sgi);
         for(sp = 0; sp < 2; sp++)
            SMPC_SetInput(sp, "extern", (uint8_t*)STVIO_GetSMPCDevice(sp));
      }
   }

   SS_LoadRTC();
   SS_LoadBackupRAM();
   SS_LoadCartNV();

   SS_BackupBackupRAM();
   SS_BackupCartNV();

   /* Just-loaded state is by definition clean. The cycle-counted
    * SaveDelay variables are gone -- see comment in Emulate(). */
   BackupRAM_Dirty = false;
   CART_GetClearNVDirty();
   CartNV_Dirty = false;

   if(MDFN_GetSettingB("ss.smpc.autortc"))
   {
      time_t ut;
      struct tm* ht;

      if((ut = time(NULL)) == (time_t)-1)
      {
         log_cb(RETRO_LOG_ERROR, "AutoRTC error #1\n");
         /* Previously this just returned false, leaving the VDP2
          * render thread, semaphore, queue, SCU, SMPC, etc. fully
          * initialised. A subsequent load would then double-init
          * and race with the orphaned render thread. */
         Cleanup();
         return false;
      }

      if((ht = localtime(&ut)) == NULL)
      {
         log_cb(RETRO_LOG_ERROR, "AutoRTC error #2\n");
         Cleanup();
         return false;
      }

      SMPC_SetRTC(ht, MDFN_GetSettingUI("ss.smpc.autortc.lang"));
   }

   SS_Reset(true);

   return true;
}

/* SS_Reset is the public-ABI reset entry called from libretro.c's
 * retro_reset and from InitCommon's final step.  Reaches into both
 * SH7095 CPU instances via the extern "C" wrappers. */
void SS_Reset(bool powering_up)
{
 SH7095_BusLock = 0;

 if(powering_up)
 {
   memset(WorkRAM, 0x00, sizeof(WorkRAM[0]) * (2 * WORKRAM_BANK_SIZE_BYTES));
   /* TODO: Check real hardware. */
 }

 if(powering_up)
 {
  SH7095_M_TruePowerOn();
  SH7095_S_TruePowerOn();
 }

 SCU_Reset(powering_up);
 SH7095_M_Reset(powering_up);

 /* ST-V's I/O board must reset before SMPC -- SMPC's port shim
  * (IODevice_STVSMPC) consults state that STVIO_Reset re-initialises. */
 if(ActiveCartType == CART_STV)
   STVIO_Reset(powering_up);

 SMPC_Reset(powering_up);

 VDP1_Reset(powering_up);
 VDP2_Reset(powering_up);

 CDB_Reset(powering_up);

 SOUND_Reset(powering_up);

 CART_Reset(powering_up);
}

void MDFN_COLD CloseGame(void)
{
#ifdef SH7095_OP_PAIR_PROFILE
 SS_DumpOpPairProfile();
#endif

 SS_SaveBackupRAM();
 SS_SaveCartNV();
 SS_SaveRTC();

 Cleanup();
}

void MDFN_BackupSavFile(const uint8_t max_backup_count, const char* sav_ext)
{
   /* stub for libretro port */
   (void)max_backup_count;
   (void)sav_ext;
}
