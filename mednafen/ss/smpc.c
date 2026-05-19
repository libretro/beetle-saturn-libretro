/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* smpc.c - SMPC Emulation
**  Copyright (C) 2015-2021 Mednafen Team
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

/*
  TODO:
	CD On/Off

	Cleaner handling of waiting on conditions(see PendingCommand, PendingVB, IR0WX, IR0WA),
	and maybe proper handling of JR_WAIT() on JR_BS.
*/

#include "ss.h"
#include "../emuspec.h"			/* EmulateSpecStruct full layout */

#include "smpc.h"
#include "smpc_iodevice.h"
#include "sound.h"
#include "vdp1.h"
#include "vdp2.h"			/* VDP2_* declarations including the
					   static INLINE VDP2_SetExtLatch -- if
					   this isn't included, VDP2_SetExtLatch
					   doesn't get inlined and there is no
					   external symbol to link against
					   (the ss/vdp2: convert from C++ to C
					   commit collapsed the extern "C"
					   proxy in vdp2.c into the header
					   INLINE form). */
#include "cdb.h"
#include "scu.h"

/* SH7095 is still a struct in sh7095.h, so smpc.c reaches the two
 * methods it needs (SetActive / SetNMI) via the matching extern "C"
 * proxies in ss.c (which is where the CPU[2] global lives).  Local
 * forward decls here cover them. */
extern void SH7095_S_SetActive(bool active);
extern void SH7095_M_SetNMI(bool level);

enum
{
 CLOCK_DIVISOR_26M = 65,
 CLOCK_DIVISOR_28M = 61
};

enum
{
 CMD_MSHON = 0x00,
 CMD_SSHON = 0x02,
 CMD_SSHOFF = 0x03,

 CMD_SNDON = 0x06,
 CMD_SNDOFF = 0x07,

 CMD_CDON = 0x08,
 CMD_CDOFF = 0x09,

 // A, B, C do something...

 CMD_SYSRES = 0x0D,

 CMD_CKCHG352 = 0x0E,
 CMD_CKCHG320 = 0x0F,

 CMD_INTBACK = 0x10,
 CMD_SETTIME = 0x16,
 CMD_SETSMEM = 0x17,

 CMD_NMIREQ = 0x18,
 CMD_RESENAB = 0x19,
 CMD_RESDISA = 0x1A
};

static uint8_t AreaCode;
static int32_t MasterClock;

static struct
{
 uint64_t ClockAccum;

 bool Valid;

 union
 {
  uint8_t raw[7];
  struct
  {
   uint8_t year[2];		// BCD; [0] = xx00, [1] = 00xx
   uint8_t wday_mon;	// 0x0-0x6(upper; 6=Saturday), 0x1-0xC(lower)
   uint8_t mday;		// BCD; 0x01-0x31
   uint8_t hour;		// BCD; 0x00-0x23
   uint8_t minute;		// BCD; 0x00-0x59
   uint8_t second;		// BCD; 0x00-0x59
  };
 };
} RTC;

static uint8_t SaveMem[4];

static uint8_t IREG[7];
static uint8_t OREG[0x20];
static uint8_t SR;
static bool SF;

enum
{
 PMODE_15BYTE = 0,
 PMODE_255BYTE = 1,
 PMODE_ILL = 2,
 PMODE_0BYTE = 3
};

enum
{
 SR_RESB = 0x10,
 SR_NPE = 0x20,
 SR_PDL = 0x40,
};

static bool ResetNMIEnable;

static bool ResetButtonPhysStatus;
static int32_t ResetButtonCount;
static bool ResetPending;
static int32_t PendingCommand;
static int32_t ExecutingCommand;
static int32_t PendingClockDivisor;
static int32_t CurrentClockDivisor;

static bool PendingVB;

static uint8_t IR0WX, IR0WA;

static int32_t SubPhase;
static int64_t ClockCounter;
static uint32_t SMPC_ClockRatio;

static bool SoundCPUOn;
static bool SlaveSH2On;
static int SlaveSH2Pending;
static bool CDOn;

static uint8_t BusBuffer;
//
//
static struct
{
 int64_t TimeCounter;
 int32_t StartTime;
 int32_t OptWaitUntilTime;
 int32_t OptEatTime;

 int32_t OptReadTime;

 uint8_t Mode[2];
 bool TimeOptEn;
 bool NextContBit;

 uint8_t CurPort;
 uint8_t ID1;
 uint8_t ID2;
 uint8_t IDTap;

 uint8_t CommMode;

 uint8_t OWP;

 uint8_t work[8];
 //
 //
 uint8_t TapCounter;
 uint8_t TapCount;
 uint8_t ReadCounter;
 uint8_t ReadCount;
 uint8_t ReadBuffer[256];	// Maybe should only be 255, but +1 for save state sanitization simplification.
 uint8_t WriteCounter;
 uint8_t PDCounter;
} JRS;
//
//
static bool vb;
static bool vsync;
static sscpu_timestamp_t lastts;
//
//
static uint8_t DataOut[2][2];
static uint8_t DataDir[2][2];
static bool DirectModeEn[2];
static bool ExLatchEn[2];

static uint8_t IOBusState[2];
static IODevice* IOPorts[2];

static struct
{
 IODevice* none;
 IODevice* gamepad;
 IODevice* threedpad;
 IODevice* mouse;
 IODevice* wheel;
 IODevice* mission;
 IODevice* dualmission;
 IODevice* gun;
 IODevice* keyboard;
 IODevice* jpkeyboard;
} PossibleDevices[12];

static IODevice* PossibleMultitaps[2];

static IODevice* SPorts[2];
static IODevice* VirtualPorts[12];
static uint8_t* VirtualPortsDPtr[12];
static uint8_t* MiscInputPtr;

/* Per-port lightgun crosshair colour. The libretro option-update path
   (check_variables) can push these in before SMPC_Init has created
   the device objects -- in the old code PossibleDevices held by-value
   objects so the gun always existed, but now it holds pointers that
   are NULL until SMPC_Init's creation loop runs. Remember the colour
   here so SMPC_SetCrosshairsColor can no-op safely when the gun does
   not exist yet, and so SMPC_Init can (re)apply it to each freshly
   created gun. 0xFFFFFFFF means "never set". */
static uint32_t CrosshairsColor[12] =
{
   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
};

/* The base IODevice implementations (the empty Power/UpdateInput/etc.,
   the smpc_out-passthrough UpdateBus, the TS-rebasing ResetTS) now
   live in smpc_iodevice.c as IODevice_base_* / IODevice_base_vtable. */
//
//

static void UpdateIOBus(unsigned port, const sscpu_timestamp_t timestamp)
{
 IOBusState[port] = IOPorts[port]->vt->UpdateBus(IOPorts[port], timestamp, (DataOut[port][DirectModeEn[port]] | ~DataDir[port][DirectModeEn[port]]) & 0x7F, DataDir[port][DirectModeEn[port]]);
 assert(!(IOBusState[port] & 0x80));

 {
  bool tmp = (!(IOBusState[0] & 0x40) & ExLatchEn[0]) | (!(IOBusState[1] & 0x40) & ExLatchEn[1]);

  SCU_SetInt(SCU_INT_PAD, tmp);
  VDP2_SetExtLatch(timestamp, tmp);
 }
}

static void MapPorts(void)
{
 for(unsigned sp = 0, vp = 0; sp < 2; sp++)
 {
  IODevice* nd;

  if(SPorts[sp])
  {
   for(unsigned i = 0; i < 6; i++)
   {
    IODevice* const tsd = VirtualPorts[vp++];
    if (!tsd) continue; // libretro fix - patch for multi-tap set on startup.

    if(IODevice_Multitap_GetSubDevice(SPorts[sp], i) != tsd)
     tsd->vt->Power(tsd);

    IODevice_Multitap_SetSubDevice(SPorts[sp], i, tsd);
   }

   nd = SPorts[sp];
  }
  else
   nd = VirtualPorts[vp++];

  // The original code assumed nd != NULL here. That's true when
  // every VirtualPort has been populated by SMPC_SetInput before any
  // call to MapPorts() that touches that port. Init carefully arranges
  // for this to hold by calling SetInput in order, so the (vp - 1)
  // VirtualPort is always the freshly-set one. But the assumption is
  // fragile: any future code path that disables a multitap before
  // populating the corresponding VirtualPort would hit a null Power()
  // call here. The in-multitap-loop branch already has an analogous
  // `if (!tsd) continue;` guard; mirror it on the non-multitap branch.
  if(nd && IOPorts[sp] != nd)
   nd->vt->Power(nd);

  IOPorts[sp] = nd;
 }
}

void SMPC_SetMultitap(unsigned sport, bool enabled)
{
#ifndef NDEBUG
 assert(sport < 2);
#endif

 SPorts[sport] = (enabled ? PossibleMultitaps[sport] : NULL);
 MapPorts();
}

void SMPC_SetCrosshairsColor(unsigned port, uint32_t color)
{
#ifndef NDEBUG
 assert(port < 12);
#endif

 /* Remember the colour unconditionally: this can be called from the
    libretro option path before SMPC_Init has created the gun device.
    SMPC_Init applies CrosshairsColor[] to each gun it creates. */
 CrosshairsColor[port] = color;

 if(PossibleDevices[port].gun)
  IODevice_Gun_SetCrosshairsColor(PossibleDevices[port].gun, color);
}

void SMPC_SetInput(unsigned port, const char* type, uint8_t* ptr)
{
#ifndef NDEBUG
 assert(port < 13);
#endif

 if(port == 12)
 {
  MiscInputPtr = ptr;
  return;
 }
 //
 //
 //
 IODevice* nd = NULL;

 if(!strcmp(type, "none"))
  nd = PossibleDevices[port].none;
 else if(!strcmp(type, "gamepad"))
  nd = PossibleDevices[port].gamepad;
 else if(!strcmp(type, "3dpad"))
  nd = PossibleDevices[port].threedpad;
 else if(!strcmp(type, "mouse"))
  nd = PossibleDevices[port].mouse;
 else if(!strcmp(type, "wheel"))
  nd = PossibleDevices[port].wheel;
 else if(!strcmp(type, "mission") || !strcmp(type, "missionwoa"))
  nd = PossibleDevices[port].mission;
 else if(!strcmp(type, "dmission") || !strcmp(type, "dmissionwoa"))
  nd = PossibleDevices[port].dualmission;
 else if(!strcmp(type, "gun"))
  nd = PossibleDevices[port].gun;
 else if(!strcmp(type, "keyboard"))
  nd = PossibleDevices[port].keyboard;
 else if(!strcmp(type, "jpkeyboard"))
  nd = PossibleDevices[port].jpkeyboard;
 else if(!strcmp(type, "extern"))
 {
  // ST-V uses this to inject its own IODevice (the SMPC port shim that
  // talks to the AK93C45 EEPROM and forwards sound CPU reset to STV I/O).
  // ptr here is an IODevice*, not the usual raw input-state buffer; clear
  // it so the DPtr slot below doesn't treat it as one.
  nd = (IODevice*)ptr;
  ptr = NULL;
 }
 else
  abort();

 VirtualPorts[port] = nd;
 VirtualPortsDPtr[port] = ptr;

 MapPorts();
}

void SMPC_LoadNV(cdstream* s)
{
 /* File layout: 1 byte Valid, then 7 bytes RTC.raw, then 4 bytes
  * SaveMem.  On a short or failed read the pre-fix code silently
  * loaded partial data plus whatever stack bytes happened to sit
  * behind the uninitialised tail -- the BIOS would then see a
  * frankenstein RTC + SaveMem on cold boot and the SMPC clock
  * could drift, the language byte in SaveMem[3] could read as
  * a stale value, etc.
  *
  * Fall back to factory defaults: RTC.Valid = false (SMPC will
  * resynthesise from host time at the next read), RTC.raw zeroed
  * (paranoia in case Valid=false isn't honoured everywhere), and
  * SaveMem zeroed so the BIOS sees a fresh power-on state. */
 const uint8_t  valid_byte = cdstream_read_u8(s);
 const uint64_t r1         = cdstream_read(s, RTC.raw, sizeof(RTC.raw));
 const uint64_t r2         = cdstream_read(s, SaveMem, sizeof(SaveMem));

 if (r1 != sizeof(RTC.raw) || r2 != sizeof(SaveMem))
 {
  RTC.Valid = false;
  memset(RTC.raw, 0, sizeof(RTC.raw));
  memset(SaveMem, 0, sizeof(SaveMem));
  return;
 }

 RTC.Valid = valid_byte;
}

bool SMPC_SaveNV(cdstream* s)
{
 /* Three writes, total 12 bytes (1 + 7 + 4).  Tiny, but on a
  * full disk / quota-exceeded filesystem / write-protected media
  * any of them can come up short.  Pre-fix that was silently
  * dropped on the floor and the next SMPC_LoadNV would see a
  * truncated file (and, post-c05df2e, fall back to factory
  * defaults -- but only because we hardened the load path).
  *
  * Return failure so SS_SaveRTC (the caller in ss.c, which has
  * log_cb already extern-declared) can surface the error to the
  * user.  Function was void pre-fix; libretro frontend doesn't
  * call SMPC_SaveNV directly so this isn't an ABI concern. */
 bool ok = true;
 if (cdstream_write_u8(s, RTC.Valid) != 1)                          ok = false;
 if (cdstream_write(s, RTC.raw, sizeof(RTC.raw)) != sizeof(RTC.raw))  ok = false;
 if (cdstream_write(s, SaveMem, sizeof(SaveMem)) != sizeof(SaveMem))  ok = false;
 return ok;
}

void SMPC_SetRTC(const struct tm* ht, const uint8_t lang)
{
 RTC.ClockAccum = 0;

 if(!ht)
 {
  RTC.Valid = false;
  RTC.year[0] = 0x19;
  RTC.year[1] = 0x93;
  RTC.wday_mon = 0x5C;
  RTC.mday = 0x31;
  RTC.hour = 0x23;
  RTC.minute = 0x59;
  RTC.second = 0x59;

  for(unsigned i = 0; i < 4; i++)
   SaveMem[i] = 0x00;
 }
 else
 {
  int year_adj = ht->tm_year;
  //if(year_adj >= 100)
  // year_adj = 100 + ((year_adj - 100) % 28);

  RTC.Valid = true; //false;
  RTC.year[0] = U8_to_BCD(19 + year_adj / 100);
  RTC.year[1] = U8_to_BCD(year_adj % 100);
  RTC.wday_mon = (((unsigned)(6) < (unsigned)(ht->tm_wday) ? (unsigned)(6) : (unsigned)(ht->tm_wday)) << 4) | ((((unsigned)(11) < (unsigned)(ht->tm_mon) ? (unsigned)(11) : (unsigned)(ht->tm_mon)) + 1) << 0);
  RTC.mday = U8_to_BCD(((unsigned)(31) < (unsigned)(ht->tm_mday) ? (unsigned)(31) : (unsigned)(ht->tm_mday)));
  RTC.hour = U8_to_BCD(((unsigned)(23) < (unsigned)(ht->tm_hour) ? (unsigned)(23) : (unsigned)(ht->tm_hour)));
  RTC.minute = U8_to_BCD(((unsigned)(59) < (unsigned)(ht->tm_min) ? (unsigned)(59) : (unsigned)(ht->tm_min)));
  RTC.second = U8_to_BCD(((unsigned)(59) < (unsigned)(ht->tm_sec) ? (unsigned)(59) : (unsigned)(ht->tm_sec)));

  //if((SaveMem[3] & 0x0F) <= 0x05 || (SaveMem[3] & 0x0F) == 0xF)
  SaveMem[3] = (SaveMem[3] & 0xF0) | lang;
 }
}

// When the SMPC is running inside ST-V (Sega Titan Video) arcade hardware,
// the sound CPU control bus is wired directly to the STV's I/O board instead
// of being slaved to SMPC like on Saturn. SMPC therefore must not assert
// reset / 68K-active changes when STV is the host. Set true via the
// `block_soundcpu_control` argument to SMPC_Init.
static bool BlockSoundCPUControl;

void SMPC_Init(const uint8_t area_code_arg, const int32_t master_clock_arg, bool block_soundcpu_control)
{
 AreaCode = area_code_arg;
 MasterClock = master_clock_arg;
 SMPC_ClockRatio = 0;
 BlockSoundCPUControl = block_soundcpu_control;

 SlaveSH2Pending = false;

 ResetButtonPhysStatus = false;
 ResetPending = false;
 vb = false;
 vsync = false;
 lastts = 0;

 /* (Re)create the input devices.  SMPC_Init runs on every game load,
    so free any from a previous load first; IODevice_Free is NULL-safe,
    so the first call (zeroed statics, or a prior SMPC_Kill) is fine.
    The devices are owned here; IOPorts/VirtualPorts/SPorts hold
    borrowed pointers into these arrays and must not free them. */
 for(unsigned port = 0; port < 12; port++)
 {
  IODevice_Free(PossibleDevices[port].none);
  IODevice_Free(PossibleDevices[port].gamepad);
  IODevice_Free(PossibleDevices[port].threedpad);
  IODevice_Free(PossibleDevices[port].mouse);
  IODevice_Free(PossibleDevices[port].wheel);
  IODevice_Free(PossibleDevices[port].mission);
  IODevice_Free(PossibleDevices[port].dualmission);
  IODevice_Free(PossibleDevices[port].gun);
  IODevice_Free(PossibleDevices[port].keyboard);
  IODevice_Free(PossibleDevices[port].jpkeyboard);

  PossibleDevices[port].none        = IODevice_None_Create();
  PossibleDevices[port].gamepad     = IODevice_Gamepad_Create();
  PossibleDevices[port].threedpad   = IODevice_3DPad_Create();
  PossibleDevices[port].mouse       = IODevice_Mouse_Create();
  PossibleDevices[port].wheel       = IODevice_Wheel_Create();
  PossibleDevices[port].mission     = IODevice_Mission_Create(false);
  PossibleDevices[port].dualmission = IODevice_Mission_Create(true);
  PossibleDevices[port].gun         = IODevice_Gun_Create();
  PossibleDevices[port].keyboard    = IODevice_Keyboard_Create();
  PossibleDevices[port].jpkeyboard  = IODevice_JPKeyboard_Create();

  /* Re-apply any crosshair colour pushed in before this point (the
     libretro option path runs check_variables() before SMPC_Init).
     0xFFFFFFFF is the "never set" sentinel -- leave the gun at its
     own default in that case. */
  if(CrosshairsColor[port] != 0xFFFFFFFF)
   IODevice_Gun_SetCrosshairsColor(PossibleDevices[port].gun, CrosshairsColor[port]);
 }

 for(unsigned sp = 0; sp < 2; sp++)
 {
  IODevice_Free(PossibleMultitaps[sp]);
  PossibleMultitaps[sp] = IODevice_Multitap_Create();
 }

 for(unsigned sp = 0; sp < 2; sp++)
 {
  SPorts[sp]  = NULL;
  IOPorts[sp] = NULL; /* beetle/libretro: added to fix crash when two multi-taps are used */
 }

 for(unsigned i = 0; i < 12; i++)
 {
  VirtualPorts[i] = NULL;
  SMPC_SetInput(i, "none", NULL);
 }

 SMPC_SetRTC(NULL, 0);
}

void SMPC_Kill(void)
{
 /* Release the heap-allocated IODevice pool.  Pre-conversion these
    were by-value members of static PossibleDevices, released at
    static destructor time.  Post-conversion they are pointers from
    IODevice_*_Create() and were leaked on the final game close --
    SMPC_Init's IODevice_Free preamble only released them when there
    was a *next* game load to run it.

    IODevice_Free is NULL-safe, so this is also safe to call when
    SMPC_Init never ran (zeroed statics).

    SPorts/IOPorts/VirtualPorts hold borrowed pointers into the just-
    freed slots, so NULL them out so a subsequent SMPC_Init starts
    from a clean slate.  VirtualPortsDPtr and MiscInputPtr are
    caller-supplied (from SMPC_SetInput / the misc-port path) and
    are not owned here -- leave them alone. */
 for(unsigned port = 0; port < 12; port++)
 {
  IODevice_Free(PossibleDevices[port].none);
  IODevice_Free(PossibleDevices[port].gamepad);
  IODevice_Free(PossibleDevices[port].threedpad);
  IODevice_Free(PossibleDevices[port].mouse);
  IODevice_Free(PossibleDevices[port].wheel);
  IODevice_Free(PossibleDevices[port].mission);
  IODevice_Free(PossibleDevices[port].dualmission);
  IODevice_Free(PossibleDevices[port].gun);
  IODevice_Free(PossibleDevices[port].keyboard);
  IODevice_Free(PossibleDevices[port].jpkeyboard);
 }
 memset(PossibleDevices, 0, sizeof(PossibleDevices));

 for(unsigned sp = 0; sp < 2; sp++)
  IODevice_Free(PossibleMultitaps[sp]);
 memset(PossibleMultitaps, 0, sizeof(PossibleMultitaps));

 memset(SPorts,       0, sizeof(SPorts));
 memset(IOPorts,      0, sizeof(IOPorts));
 memset(VirtualPorts, 0, sizeof(VirtualPorts));
}

static void TurnSoundCPUOn(void)
{
 if(BlockSoundCPUControl)
 {
  SoundCPUOn = true;
  return;
 }
 SOUND_Reset68K();
 SoundCPUOn = true;
 SOUND_Set68KActive(true);
}

static void TurnSoundCPUOff(void)
{
 if(BlockSoundCPUControl)
 {
  SoundCPUOn = false;
  return;
 }
 SOUND_Reset68K();
 SoundCPUOn = false;
 SOUND_Set68KActive(false);
}

void SMPC_Reset(bool powering_up)
{
 SlaveSH2Pending = 0;
 SlaveSH2On = false;
 SH7095_S_SetActive(SlaveSH2On);
 //
 TurnSoundCPUOff();
 CDOn = true; // ? false;

 ResetButtonCount = 0;
 ResetNMIEnable = false;	// or only on powering_up?

 SH7095_M_SetNMI(true);

 memset(IREG, 0, sizeof(IREG));
 memset(OREG, 0, sizeof(OREG));
 PendingCommand = -1;
 ExecutingCommand = -1;
 SR = 0x00;
 SF = 0;

 BusBuffer = 0x00;

 for(unsigned port = 0; port < 2; port++)
 {
  for(unsigned sel = 0; sel < 2; sel++)
  {
   DataOut[port][sel] = 0;
   DataDir[port][sel] = 0;
  }
  DirectModeEn[port] = false;
  ExLatchEn[port] = false;
  UpdateIOBus(port, SH7095_mem_timestamp);
  //
  if(powering_up)
  {
   IOPorts[port]->vt->Power(IOPorts[port]);
   UpdateIOBus(port, SH7095_mem_timestamp);
  }
 }
 //
 //
 //
 ResetPending = false;

 PendingClockDivisor = 0;
 CurrentClockDivisor = CLOCK_DIVISOR_26M;

 SubPhase = 0;
 PendingVB = false;
 ClockCounter = 0;
 IR0WX = 0;
 IR0WA = 0;
 //
 memset(&JRS, 0, sizeof(JRS));
}

void SMPC_StateAction(StateMem* sm, const unsigned load, const bool data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(RTC.ClockAccum),
  SFVAR(RTC.Valid),
  SFPTR8N(&(RTC.raw)[0], (sizeof(RTC.raw) / sizeof(uint8_t)), "RTC.raw"),

  SFPTR8N(&(SaveMem)[0], (sizeof(SaveMem) / sizeof(uint8_t)), "SaveMem"),

  SFPTR8N(&(IREG)[0], (sizeof(IREG) / sizeof(uint8_t)), "IREG"),
  SFPTR8N(&(OREG)[0], (sizeof(OREG) / sizeof(uint8_t)), "OREG"),
  SFVAR(SR),
  SFVAR(SF),

  SFVAR(ResetNMIEnable),
  SFVAR(ResetButtonPhysStatus),
  SFVAR(ResetButtonCount),
  SFVAR(ResetPending),
  SFVAR(PendingCommand),
  SFVAR(ExecutingCommand),
  SFVAR(PendingClockDivisor),
  SFVAR(CurrentClockDivisor),

  SFVAR(PendingVB),

  SFVAR(IR0WX),
  SFVAR(IR0WA),

  SFVAR(SubPhase),
  SFVAR(ClockCounter),
  SFVAR(SMPC_ClockRatio),

  SFVAR(SoundCPUOn),
  SFVAR(SlaveSH2On),
  SFVAR(SlaveSH2Pending),
  SFVAR(CDOn),

  SFVAR(BusBuffer),

  SFVAR(JRS.TimeCounter),
  SFVAR(JRS.StartTime),
  SFVAR(JRS.OptWaitUntilTime),
  SFVAR(JRS.OptEatTime),
  SFVAR(JRS.OptReadTime),

  SFPTR8N(&(JRS.Mode)[0], (sizeof(JRS.Mode) / sizeof(uint8_t)), "JRS.Mode"),
  SFVAR(JRS.TimeOptEn),
  SFVAR(JRS.NextContBit),

  SFVAR(JRS.CurPort),
  SFVAR(JRS.ID1),
  SFVAR(JRS.ID2),
  SFVAR(JRS.IDTap),

  SFVAR(JRS.CommMode),

  SFVAR(JRS.OWP),

  SFPTR8N(&(JRS.work)[0], (sizeof(JRS.work) / sizeof(uint8_t)), "JRS.work"),

  SFVAR(JRS.TapCounter),
  SFVAR(JRS.TapCount),
  SFVAR(JRS.ReadCounter),
  SFVAR(JRS.ReadCount),
  SFPTR8N(&(JRS.ReadBuffer)[0], (sizeof(JRS.ReadBuffer) / sizeof(uint8_t)), "JRS.ReadBuffer"),
  SFVAR(JRS.WriteCounter),
  SFVAR(JRS.PDCounter),

  SFPTR8N(&(DataOut)[0][0], (sizeof(DataOut) / sizeof(uint8_t)), "&DataOut[0][0]"),
  SFPTR8N(&(DataDir)[0][0], (sizeof(DataDir) / sizeof(uint8_t)), "&DataDir[0][0]"),
  SFPTRBN(&(DirectModeEn)[0], (sizeof(DirectModeEn) / sizeof(bool)), "DirectModeEn"),
  SFPTRBN(&(ExLatchEn)[0], (sizeof(ExLatchEn) / sizeof(bool)), "ExLatchEn"),

  SFPTR8N(&(IOBusState)[0], (sizeof(IOBusState) / sizeof(uint8_t)), "IOBusState"),

  SFVAR(vb),
  SFVAR(vsync),

  SFVAR(lastts),

  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, "SMPC", false);

 for(unsigned port = 0; port < 2; port++)
 {
  const char snp[] = { 'S', 'M', 'P', 'C', '_', 'P', (char)('0' + port), 0 };

  IOPorts[port]->vt->StateAction(IOPorts[port], sm, load, data_only, snp);
 }

 if(load)
 {
  JRS.CurPort &= 0x1;
  JRS.OWP &= 0x3F;

  for(unsigned vp = 0; vp < 12; vp++)
  {
   IODevice* const p = VirtualPorts[vp];

   if(load < 0x00102600 && p->NextEventTS >= 0x40000000)
    p->NextEventTS = SS_EVENT_DISABLED_TS;
   else
    p->NextEventTS = ((sscpu_timestamp_t)(0) > (sscpu_timestamp_t)(p->NextEventTS) ? (sscpu_timestamp_t)(0) : (sscpu_timestamp_t)(p->NextEventTS));
  }

  if(load < 0x00103100)
  {
   //printf("%u --- %u\n", SubPhase, SubPhase + 1); //SubPhaseBias);

   switch(SubPhase)
   {
    case 29:
    case 27:
	JRS.NextContBit = true;
	if(SR & SR_NPE)
	{
	 IR0WX = (!JRS.NextContBit << 7);
	 IR0WA = 0xC0;
	 SubPhase = 27;
	}
	else
	{
	 IR0WA = 0;
	 SubPhase = 29;
	}
	break;
   }
  }
 }
}

void SMPC_TransformInput(void)
{
 float gun_x_scale, gun_x_offs;

 VDP2_GetGunXTranslation(((PendingClockDivisor > 0) ? PendingClockDivisor : CurrentClockDivisor) == CLOCK_DIVISOR_28M, &gun_x_scale, &gun_x_offs);

 for(unsigned vp = 0; vp < 12; vp++)
  VirtualPorts[vp]->vt->TransformInput(VirtualPorts[vp], VirtualPortsDPtr[vp], gun_x_scale, gun_x_offs);
}

void SMPC_ProcessSlaveOffOn(void)
{
 if(SlaveSH2Pending)
 {
  SlaveSH2On = (SlaveSH2Pending > 0);
  SH7095_S_SetActive(SlaveSH2On);
  SlaveSH2Pending = 0;
  //
 }
}

int32_t SMPC_StartFrame(void)
{
 if(ResetPending)
 {
  SS_Reset(false);

  // TODO: Fix SMPC_Reset(false) instead ?
  OREG[0x1F] = CMD_SYSRES;
 }

 if(PendingClockDivisor > 0)
 {
  CurrentClockDivisor = PendingClockDivisor;
  PendingClockDivisor = 0;
 }

 SMPC_ClockRatio = (1ULL << 32) * 4000000 * CurrentClockDivisor / MasterClock;
 SOUND_SetClockRatio((1ULL << 32) * 11289600 * CurrentClockDivisor / MasterClock);
 CDB_SetClockRatio((1ULL << 32) * 11289600 * CurrentClockDivisor / MasterClock);

 /* Per-frame TS-freq update for every virtual port.  Mirrors upstream
  * Mednafen's SMPC_StartFrame -- dropped during the C++ -> C source-
  * fold pass along with its (unused) EmulateSpecStruct* espec param.
  *
  * Without this, each VirtualPort's internal timestamp counter stays
  * pinned to whatever rate it got at SMPC_Init time.  If
  * CurrentClockDivisor changes mid-game (e.g. region/clock switch),
  * device-internal time-elapsed bookkeeping (mouse delta sampling,
  * gun-port crosshair latency, AK93C45 EEPROM bit timings on ST-V)
  * drifts versus master-clock time.
  *
  * No-op for IODevice_None / IODevice_base; meaningful for AK93C45
  * (via STVSMPC override on ST-V) and any future devices that
  * propagate ts-freq into substructures. */
 {
  const int32_t ts_freq = (2 * (int64_t)MasterClock + CurrentClockDivisor) / (2 * CurrentClockDivisor);
  unsigned vp;
  for(vp = 0; vp < 12; vp++)
   if(VirtualPorts[vp])
    VirtualPorts[vp]->vt->SetTSFreq(VirtualPorts[vp], ts_freq);
 }

 return CurrentClockDivisor;
}

void SMPC_EndFrame(EmulateSpecStruct* espec, const sscpu_timestamp_t timestamp)
{
 for(unsigned i = 0; i < 2; i++)
 {
  if(SPorts[i])
   IODevice_Multitap_ForceSubUpdate(SPorts[i], timestamp);
 }

 if(!espec->skip)
 {
  float gun_x_scale, gun_x_offs;

  VDP2_GetGunXTranslation(CurrentClockDivisor == CLOCK_DIVISOR_28M, &gun_x_scale, &gun_x_offs);

  for(unsigned i = 0; i < 2; i++)
  {
   IOPorts[i]->vt->Draw(IOPorts[i], espec->surface, &espec->DisplayRect, espec->LineWidths, espec->InterlaceOn ? espec->InterlaceField : -1, gun_x_scale, gun_x_offs);
  }
 }
}

void SMPC_UpdateOutput(void)
{
 for(unsigned vp = 0; vp < 12; vp++)
 {
  VirtualPorts[vp]->vt->UpdateOutput(VirtualPorts[vp], VirtualPortsDPtr[vp]);
 }
}

void SMPC_UpdateInput(const int32_t time_elapsed)
{
 if (MiscInputPtr)
  ResetButtonPhysStatus = (bool)(*MiscInputPtr & 0x1);
 for(unsigned vp = 0; vp < 12; vp++)
 {
  if (VirtualPorts[vp])
   VirtualPorts[vp]->vt->UpdateInput(VirtualPorts[vp], VirtualPortsDPtr[vp], time_elapsed);
 }
}

void SMPC_Write(const sscpu_timestamp_t timestamp, uint8_t A, uint8_t V)
{
 BusBuffer = V;
 A &= 0x3F;

 //
 // Call VDP2_Update() to prevent out-of-temporal-order calls to SMPC_Update() from here and the event system.
 //
 SS_SetEventNT(&events[SS_EVENT_VDP2], VDP2_Update(timestamp));	// TODO: conditionalize so we don't consume so much CPU time if a game writes continuously to SMPC ports
 sscpu_timestamp_t nt = SMPC_Update(timestamp);
 switch(A)
 {
  case 0x00:
	if((V ^ IR0WX) & IR0WA)	// For handling intback break and continue bits.
	 nt = timestamp + 1;
	// fall-through
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
  case 0x06:
	IREG[A] = V;
	break;

  case 0x0F:
	PendingCommand = V;
	break;

  case 0x31:
	SF = true;
	break;

  //
  //
  //
  case 0x3A:
	DataOut[0][1] = V & 0x7F;
	UpdateIOBus(0, SH7095_mem_timestamp);
	break;

  case 0x3B:
	DataOut[1][1] = V & 0x7F;
	UpdateIOBus(1, SH7095_mem_timestamp);
	break;

  case 0x3C:
	DataDir[0][1] = V & 0x7F;
	UpdateIOBus(0, SH7095_mem_timestamp);
	break;

  case 0x3D:
	DataDir[1][1] = V & 0x7F;
	UpdateIOBus(1, SH7095_mem_timestamp);
	break;

  case 0x3E:
	DirectModeEn[0] = (bool)(V & 0x1);
	UpdateIOBus(0, SH7095_mem_timestamp);

	DirectModeEn[1] = (bool)(V & 0x2);
	UpdateIOBus(1, SH7095_mem_timestamp);
	break;

  case 0x3F:
	ExLatchEn[0] = (bool)(V & 0x1);
	UpdateIOBus(0, SH7095_mem_timestamp);

	ExLatchEn[1] = (bool)(V & 0x2);
	UpdateIOBus(1, SH7095_mem_timestamp);
	break;

  default:
	break;

 }

 if(PendingCommand >= 0)
  nt = timestamp + 1;

 if((((sscpu_timestamp_t)(IOPorts[0]->NextEventTS) < (sscpu_timestamp_t)(IOPorts[1]->NextEventTS) ? (sscpu_timestamp_t)(IOPorts[0]->NextEventTS) : (sscpu_timestamp_t)(IOPorts[1]->NextEventTS))) < nt) nt = (((sscpu_timestamp_t)(IOPorts[0]->NextEventTS) < (sscpu_timestamp_t)(IOPorts[1]->NextEventTS) ? (sscpu_timestamp_t)(IOPorts[0]->NextEventTS) : (sscpu_timestamp_t)(IOPorts[1]->NextEventTS)));

 SS_SetEventNT(&events[SS_EVENT_SMPC], nt);
}

uint8_t SMPC_Read(const sscpu_timestamp_t timestamp, uint8_t A)
{
 uint8_t ret = BusBuffer;

 A &= 0x3F;

 switch(A)
 {
  default:
	break;

  case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
  case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
  case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
  case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:

	ret = OREG[(size_t)A - 0x10];
	break;

  case 0x30:
	ret = SR;
	break;

  case 0x31:
	ret &= ~0x01;
	ret |= SF;
	break;

  case 0x3A:
	ret = (ret & 0x80) | IOBusState[0];
	break;

  case 0x3B:
	ret = (ret & 0x80) | IOBusState[1];
	break;

 }

 return ret;
}

void SMPC_ResetTS(void)
{
 for(unsigned p = 0; p < 2; p++)
  IOPorts[p]->vt->ResetTS(IOPorts[p]);

 lastts = 0;
}

#define SMPC_WAIT_UNTIL_COND_SHORT(cond)  {				\
			    case __COUNTER__:				\
			    ClockCounter = 0; /* before if(), not after, otherwise the variable will overflow eventually. */	\
			    if(!(cond))					\
			    {						\
			     SubPhase = __COUNTER__ - SubPhaseBias - 1;	\
			     next_event_ts = timestamp + 8;		\
			     goto Breakout;				\
			    }						\
			   }

#define SMPC_WAIT_UNTIL_COND(cond)  {					\
			    case __COUNTER__:				\
			    ClockCounter = 0; /* before if(), not after, otherwise the variable will overflow eventually. */	\
			    if(!(cond))					\
			    {						\
			     SubPhase = __COUNTER__ - SubPhaseBias - 1;	\
			     next_event_ts = timestamp + 1000;		\
			     goto Breakout;				\
			    }						\
			   }

#define SMPC_WAIT_UNTIL_COND_TIMEOUT(cond, n)							\
		{										\
		 ClockCounter -= (int64_t)(n) << 32;						\
		 case __COUNTER__:								\
		 if(!(cond) && ClockCounter < 0)						\
		 {										\
		  SubPhase = __COUNTER__ - SubPhaseBias - 1;					\
		  next_event_ts = timestamp + (-ClockCounter + SMPC_ClockRatio - 1) / SMPC_ClockRatio;		\
		  goto Breakout;								\
		 }										\
		 ClockCounter = 0;								\
		}

#define SMPC_EAT_CLOCKS(n)									\
		{										\
		 ClockCounter -= (int64_t)(n) << 32;						\
		 case __COUNTER__:								\
		 if(ClockCounter < 0)								\
		 {										\
		  SubPhase = __COUNTER__ - SubPhaseBias - 1;					\
		  next_event_ts = timestamp + (-ClockCounter + SMPC_ClockRatio - 1) / SMPC_ClockRatio;		\
		  goto Breakout;								\
		 }										\
		}										\

static unsigned RTC_BCDInc(uint8_t v)
{
 unsigned tmp = v & 0xF;

 tmp++;

 if(tmp >= 0xA)
  tmp += 0x06;

 tmp += v & 0xF0;

 if(tmp >= 0xA0)
  tmp += 0x60;

 return tmp;
}

static void RTC_IncTime(void)
{
 // Seconds
 if(RTC.second == 0x59)
 {
  RTC.second = 0x00;

  // Minutes
  if(RTC.minute == 0x59)
  {
   RTC.minute = 0x00;

   // Hours
   if(RTC.hour == 0x23)
   {
    RTC.hour = 0x00;

    // Day of week
    if(RTC.wday_mon >= 0x60)
     RTC.wday_mon &= 0x0F;
    else
     RTC.wday_mon += 0x10;

    //
    static const uint8_t mdtab[0x10] = {
    //         Jan,  Feb,  Mar,  Apr,   May, June, July,  Aug, Sept, Oct,  Nov,  Dec
	0x10, 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31, 0xC1, 0xF5, 0xFF
    };
    const uint8_t day_compare = mdtab[RTC.wday_mon & 0x0F] + ((RTC.wday_mon & 0x0F) == 0x02 && ((RTC.year[1] & 0x1F) < 0x1A) && !((RTC.year[1] + ((RTC.year[1] & 0x10) >> 3)) & 0x3));

    // Day of month
    if(RTC.mday >= day_compare)
    {
     RTC.mday = 0x01;

     // Month of year
     if((RTC.wday_mon & 0x0F) == 0x0C)
     {
      RTC.wday_mon &= 0xF0;
      RTC.wday_mon |= 0x01;

      // Year
      unsigned tmp = RTC_BCDInc(RTC.year[1]);
      RTC.year[1] = tmp;

      if(tmp >= 0x100)
       RTC.year[0] = RTC_BCDInc(RTC.year[0]);
     }
     else
      RTC.wday_mon++;
    }
    else
     RTC.mday = RTC_BCDInc(RTC.mday);
   }
   else
    RTC.hour = RTC_BCDInc(RTC.hour);
  }
  else
   RTC.minute = RTC_BCDInc(RTC.minute);
 }
 else
  RTC.second = RTC_BCDInc(RTC.second);
}

enum { SubPhaseBias = __COUNTER__ + 1 };
sscpu_timestamp_t SMPC_Update(sscpu_timestamp_t timestamp)
{
 int64_t clocks;

 if(MDFN_UNLIKELY(timestamp < lastts))
  clocks = 0;
 else
 {
  clocks = (int64_t)(timestamp - lastts) * SMPC_ClockRatio;
  lastts = timestamp;
 }

 ClockCounter += clocks;
 RTC.ClockAccum += clocks;
 JRS.TimeCounter += clocks;

 UpdateIOBus(0, timestamp);
 UpdateIOBus(1, timestamp);

 //
 sscpu_timestamp_t next_event_ts;

 switch(SubPhase + SubPhaseBias)
 {
  for(;;)
  {
   default:
   case __COUNTER__:

   SMPC_WAIT_UNTIL_COND(PendingCommand >= 0 || PendingVB);

   if(PendingVB && PendingCommand < 0)
   {
    PendingVB = false;

    if(JRS.OptReadTime)
     JRS.OptWaitUntilTime = ((int32_t)(0) > (int32_t)((JRS.TimeCounter >> 32) - JRS.OptReadTime - 5000) ? (int32_t)(0) : (int32_t)((JRS.TimeCounter >> 32) - JRS.OptReadTime - 5000));
    else
     JRS.OptWaitUntilTime = 0;
    JRS.TimeCounter = 0;
    SMPC_EAT_CLOCKS(234);

    SR &= ~SR_RESB;
    if(ResetButtonPhysStatus)	// FIXME: Semantics may not be right in regards to CMD_RESENAB timing.
    {
     SR |= SR_RESB;
     if(ResetButtonCount >= 0)
     {
      ResetButtonCount++;

      if(ResetButtonCount >= 3)
      {
       ResetButtonCount = 3;

       if(ResetNMIEnable)
       {
        SH7095_M_SetNMI(false);
        SH7095_M_SetNMI(true);

        ResetButtonCount = -1;
       }
      }
     }
    }
    else
     ResetButtonCount = 0;

    //
    // Do RTC increment here
    //
    while(MDFN_UNLIKELY(RTC.ClockAccum >= (4000000ULL << 32)))
    {
     RTC_IncTime();
     RTC.ClockAccum -= (4000000ULL << 32);
    }

    continue;
   }

   ExecutingCommand = PendingCommand;
   PendingCommand = -1;

   SMPC_EAT_CLOCKS(92);
   if(ExecutingCommand < 0x20)
   {
    OREG[0x1F] = ExecutingCommand;

    if(ExecutingCommand == CMD_MSHON)
    {

    }
    else if(ExecutingCommand == CMD_SNDON)
    {
     if(!SoundCPUOn)
      TurnSoundCPUOn();
    }
    else if(ExecutingCommand == CMD_SNDOFF)
    {
     if(SoundCPUOn)
      TurnSoundCPUOff();
    }
    else if(ExecutingCommand == CMD_CDON)
    {
     CDOn = true;
    }
    else if(ExecutingCommand == CMD_CDOFF)
    {
     CDOn = false;
    }
    else if(ExecutingCommand == CMD_SYSRES)
    {
     ResetPending = true;
     SMPC_WAIT_UNTIL_COND(!ResetPending);
     // TODO/FIXME(unreachable currently?):
    }
    else if(ExecutingCommand == CMD_CKCHG352 || ExecutingCommand == CMD_CKCHG320)
    {
     // Devour some time

     if(SlaveSH2On)
     {
      SlaveSH2Pending = -1;
      SS_RequestEHLExit();
     }

     if(SoundCPUOn)
      TurnSoundCPUOff();

     SOUND_Reset(false);
     VDP1_Reset(false);
     VDP2_Reset(false);
     SCU_Reset(false);

     // Change clock
     PendingClockDivisor = (ExecutingCommand == CMD_CKCHG352) ? CLOCK_DIVISOR_28M : CLOCK_DIVISOR_26M;

     // Wait for a few vblanks
     SMPC_WAIT_UNTIL_COND(!vb);
     SMPC_WAIT_UNTIL_COND(vb);
     SMPC_WAIT_UNTIL_COND(!vb);
     SMPC_WAIT_UNTIL_COND(vb);
     SMPC_WAIT_UNTIL_COND(!PendingClockDivisor);
     SMPC_WAIT_UNTIL_COND(!vb);
     SMPC_WAIT_UNTIL_COND(vb);
     SMPC_WAIT_UNTIL_COND(vsync);

     // Send NMI to master SH-2
     SH7095_M_SetNMI(false);
     SH7095_M_SetNMI(true);
    }
    else if(ExecutingCommand == CMD_INTBACK)
    {
     SR &= ~SR_NPE;
     if(IREG[0] & 0xF)
     {
      SMPC_EAT_CLOCKS(952);

      OREG[0] = (RTC.Valid << 7) | (!ResetNMIEnable << 6);

      for(unsigned i = 0; i < 7; i++)
       OREG[1 + i] = RTC.raw[i];

      OREG[0x8] = 0; // TODO FIXME: Cartridge code?
      OREG[0x9] = AreaCode;
      OREG[0xA] = 0x24 |
		 ((CurrentClockDivisor == CLOCK_DIVISOR_28M) << 6) |
		 (SlaveSH2On << 4) |
		 (true << 3) | 	// TODO?: Master NMI
		 (true << 1) |	// TODO?: sysres
		 (SoundCPUOn << 0);	// sndres

      OREG[0xB] = (CDOn << 6) | (1 << 1);	// cdres, TODO?: bit1

      for(unsigned i = 0; i < 4; i++)
       OREG[0xC + i] = SaveMem[i];

      if(IREG[1] & 0x8)
       SR |= SR_NPE;

      SR &= ~0x80;
      SR |= 0x0F;

      SCU_SetInt(SCU_INT_SMPC, true);
      SCU_SetInt(SCU_INT_SMPC, false);
     }

     if(IREG[1] & 0x8)
     {
      #define JR_WAIT(cond)	{ SMPC_WAIT_UNTIL_COND((cond) || PendingVB); if(PendingVB) { goto AbortJR; } }
      #define JR_EAT(n)		{ SMPC_EAT_CLOCKS(n); if(PendingVB) { goto AbortJR; } }
      #define JR_WRNYB(val)															\
	{																	\
	 if(!JRS.OWP)																\
	 {																	\
	  if(JRS.PDCounter > 0)															\
	  {																	\
	   SR = (SR & ~SR_PDL) | ((JRS.PDCounter < 0x2) ? SR_PDL : 0);										\
      	   SR = (SR & ~0xF) | (JRS.Mode[0] << 0) | (JRS.Mode[1] << 2);										\
	   SR |= SR_NPE;															\
	   SR |= 0x80;																\
	   SCU_SetInt(SCU_INT_SMPC, true);													\
	   SCU_SetInt(SCU_INT_SMPC, false);													\
	   IR0WX = (!JRS.NextContBit << 7);													\
	   IR0WA = 0xC0;															\
	   JR_WAIT((bool)(IREG[0] & 0x80) == JRS.NextContBit || (IREG[0] & 0x40));								\
           if(IREG[0] & 0x40)															\
           {																	\
            goto AbortJR;															\
	   }																	\
	   IR0WA = 0;																\
	   JRS.NextContBit = !JRS.NextContBit;													\
	  }																	\
          if(JRS.PDCounter < 0xFF)														\
           JRS.PDCounter++;															\
	 }																	\
																		\
	 OREG[(JRS.OWP >> 1)] &= 0x0F << ((JRS.OWP & 1) << 2);								\
	 OREG[(JRS.OWP >> 1)] |= ((val) & 0xF) << (((JRS.OWP & 1) ^ 1) << 2);						\
	 JRS.OWP = (JRS.OWP + 1) & 0x3F;										\
	}

      #define JR_BS	IOBusState[JRS.CurPort]

      #define JR_TH_TR(th, tr)											\
	{													\
	 DataDir[JRS.CurPort][0] = ((th >= 0) << 6) | ((tr >= 0) << 5);						\
	 DataOut[JRS.CurPort][0] = (DataOut[JRS.CurPort][0] & 0x1F) | (((th) > 0) << 6) | (((tr) > 0) << 5);	\
	 UpdateIOBus(JRS.CurPort, timestamp);									\
	}

      // Wait until Continue or Break condition(if having previously returned SMPC status).
      // Wait until end of vblank.
      // Time optimization wait.

      JRS.NextContBit = true;
      if(SR & SR_NPE)
      {
       IR0WX = (!JRS.NextContBit << 7);
       IR0WA = 0xC0;
       JR_WAIT((bool)(IREG[0] & 0x80) == JRS.NextContBit || (IREG[0] & 0x40));
       if((IREG[0] & 0x40) && (bool)(IREG[0] & 0x80) != JRS.NextContBit)
       {
        goto AbortJR;
       }
       IR0WA = 0;
       JRS.NextContBit = !JRS.NextContBit;
      }
      //
      JR_WAIT(!vb);
      //
      //
      //
      JRS.PDCounter = 0;
      JRS.TimeOptEn = !(IREG[1] & 0x2);
      JRS.Mode[0] = (IREG[1] >> 4) & 0x3;
      JRS.Mode[1] = (IREG[1] >> 6) & 0x3;

      JRS.OptReadTime = 0;
      JRS.OptEatTime = ((int32_t)(0) > (int32_t)((JRS.OptWaitUntilTime - (JRS.TimeCounter >> 32))) ? (int32_t)(0) : (int32_t)((JRS.OptWaitUntilTime - (JRS.TimeCounter >> 32))));
      JRS.OptWaitUntilTime = 0;

      if(JRS.TimeOptEn)
      {
       SMPC_WAIT_UNTIL_COND_TIMEOUT(PendingVB, JRS.OptEatTime);
       if(PendingVB)
       {
	goto AbortJR;
       }
       SS_SetEventNT(&events[SS_EVENT_MIDSYNC], timestamp + 1);
      }

      JRS.StartTime = JRS.TimeCounter >> 32;
      JR_EAT(120);
      JRS.OWP = 0;
      for(JRS.CurPort = 0; JRS.CurPort < 2; JRS.CurPort++)
      {
       JR_EAT(380);

       if(JRS.Mode[JRS.CurPort] & 0x2)
	continue;

       // TODO: 255-byte read size mode.

       JRS.ID1 = 0;
       JR_TH_TR(1, 1);
       JR_EAT(50);
       JRS.work[0] = JR_BS;
       JRS.ID1 |= ((((JRS.work[0] >> 3) | (JRS.work[0] >> 2)) & 1) << 3) | ((((JRS.work[0] >> 1) | (JRS.work[0] >> 0)) & 1) << 2);

       JR_TH_TR(0, 1);
       JR_EAT(50);
       JRS.work[1] = JR_BS;
       JRS.ID1 |= ((((JRS.work[1] >> 3) | (JRS.work[1] >> 2)) & 1) << 1) | ((((JRS.work[1] >> 1) | (JRS.work[1] >> 0)) & 1) << 0);

       if(JRS.ID1 == 0xB)
       {
	// Saturn digital pad.
	JR_TH_TR(1, 0)
	JR_EAT(50);
	JRS.work[2] = JR_BS;

	JR_TH_TR(0, 0)
	JR_EAT(50);
	JRS.work[3] = JR_BS;

	JR_EAT(30);

	JR_WRNYB(0xF);	// Multitap ID
	JR_EAT(21);

	JR_WRNYB(0x1);	// Number of connected devices behind multitap
	JR_EAT(21);

	JR_WRNYB(0x0);	// Peripheral ID-2.
	JR_EAT(21);

	JR_WRNYB(0x2);	// Data size.
	JR_EAT(21);

	JR_WRNYB(JRS.work[1] & 0xF);
	JR_EAT(21);

	JR_WRNYB(JRS.work[2] & 0xF);
	JR_EAT(21);

	JR_WRNYB(JRS.work[3] & 0xF);
	JR_EAT(21);

	JR_WRNYB((JRS.work[0] & 0xF) | 0x7);
	JR_EAT(21);

	//JR_EAT();

	//
	//
	//
       }
       else if(JRS.ID1 == 0x3 || JRS.ID1 == 0x5)
       {
	JR_TH_TR(0, 0)
	JR_EAT(50);
	JR_WAIT(!(JR_BS & 0x10));
	JRS.ID2 = ((JR_BS & 0xF) << 4);

	JR_TH_TR(0, 1)
	JR_EAT(50);
	JR_WAIT(JR_BS & 0x10);
	JRS.ID2 |= ((JR_BS & 0xF) << 0);

	if(JRS.ID1 == 0x3)
	 JRS.ID2 = 0xE3;

        if((JRS.ID2 & 0xF0) == 0x40) // Multitap
        {
	 JR_TH_TR(0, 0)
	 JR_EAT(50);
	 JR_WAIT(!(JR_BS & 0x10));
	 JRS.IDTap = ((JRS.ID2 & 0xF) << 4) | (JR_BS & 0xF);

	 JR_TH_TR(0, 1)
	 JR_EAT(50);
	 JR_WAIT(JR_BS & 0x10);
        }
	else
	 JRS.IDTap = 0xF1;

        JRS.TapCounter = 0;
        JRS.TapCount = (JRS.IDTap & 0xF);
        while(JRS.TapCounter < JRS.TapCount)
        {
         if(JRS.TapCount > 1)
         {
	  JR_TH_TR(0, 0)
	  JR_EAT(50);
	  JR_WAIT(!(JR_BS & 0x10));
	  JRS.ID2 = ((JR_BS & 0xF) << 4);

	  JR_TH_TR(0, 1)
	  JR_EAT(50);
	  JR_WAIT(JR_BS & 0x10);
	  JRS.ID2 |= ((JR_BS & 0xF) << 0);
         }
	 JRS.ReadCounter = 0;
         JRS.ReadCount = ((JRS.ID2 & 0xF0) == 0xF0) ? 0 : (JRS.ID2 & 0xF);
	 while(JRS.ReadCounter < JRS.ReadCount)
	 {
	  JR_TH_TR(0, 0)
	  JR_EAT(50);
	  JR_WAIT(!(JR_BS & 0x10));
	  JRS.ReadBuffer[JRS.ReadCounter] = ((JR_BS & 0xF) << 4);

	  JR_TH_TR(0, 1)
	  JR_EAT(50);
	  JR_WAIT(JR_BS & 0x10);
	  JRS.ReadBuffer[JRS.ReadCounter] |= ((JR_BS & 0xF) << 0);
	  JRS.ReadCounter++;
	 }

         if(!JRS.TapCounter)
         {
	  JR_WRNYB(JRS.IDTap >> 4);
	  JR_EAT(21);

	  JR_WRNYB(JRS.IDTap >> 0);
	  JR_EAT(21);
         }

	 JR_WRNYB(JRS.ID2 >> 4);
	 JR_EAT(21);

	 JR_WRNYB(JRS.ID2 >> 0);
	 JR_EAT(21);

	 JRS.WriteCounter = 0;
	 while(JRS.WriteCounter < JRS.ReadCounter)
	 {
	  JR_WRNYB(JRS.ReadBuffer[JRS.WriteCounter] >> 4);
 	  JR_EAT(21);

	  JR_WRNYB(JRS.ReadBuffer[JRS.WriteCounter] >> 0);
 	  JR_EAT(21);

          JRS.WriteCounter++;
	 }
	 JRS.TapCounter++;
	}
	// Saturn analog joystick, keyboard, multitap
        // OREG[0x0] = 0xF1;	// Upper nybble, multitap ID.  Lower nybble, number of connected devices behind multitap.
        // OREG[0x1] = 0x02;	// Upper nybble, peripheral ID 2.  Lower nybble, data size.
       }
       else
       {
	JR_WRNYB(JRS.ID1);
	JR_WRNYB(0x0);
       }
       JR_EAT(26);
       JR_TH_TR(-1, -1);
      }
      JRS.CurPort = 0; // For save state sanitization consistency.

      SR = (SR & ~SR_NPE);
      SR = (SR & ~0xF) | (JRS.Mode[0] << 0) | (JRS.Mode[1] << 2);
      SR = (SR & ~SR_PDL) | ((JRS.PDCounter < 0x2) ? SR_PDL : 0);
      SR |= 0x80;
      SCU_SetInt(SCU_INT_SMPC, true);
      SCU_SetInt(SCU_INT_SMPC, false);

      if(JRS.TimeOptEn)
       JRS.OptReadTime = ((int32_t)(0) > (int32_t)((JRS.TimeCounter >> 32) - JRS.StartTime) ? (int32_t)(0) : (int32_t)((JRS.TimeCounter >> 32) - JRS.StartTime));
     }
    }
    else if(ExecutingCommand == CMD_SETTIME)	// Warning: Execute RTC setting atomically(all values or none) in regards to emulator exit/power toggle.
    {
     SMPC_EAT_CLOCKS(380);

     RTC.ClockAccum = 0;	// settime resets sub-second count.
     RTC.Valid = true;

     for(unsigned i = 0; i < 7; i++)
      RTC.raw[i] = IREG[i];
    }
    else if(ExecutingCommand == CMD_SETSMEM)	// Warning: Execute save mem setting(all values or none) atomically in regards to emulator exit/power toggle.
    {
     SMPC_EAT_CLOCKS(234);

     for(unsigned i = 0; i < 4; i++)
      SaveMem[i] = IREG[i];
    }
    else if(ExecutingCommand == CMD_NMIREQ)
    {
     SH7095_M_SetNMI(false);
     SH7095_M_SetNMI(true);
    }
    else if(ExecutingCommand == CMD_RESENAB)
    {
     ResetNMIEnable = true;
    }
    else if(ExecutingCommand == CMD_RESDISA)
    {
     ResetNMIEnable = false;
    }
    else if(ExecutingCommand == CMD_SSHON)
    {
     if(!SlaveSH2On)
     {
      SlaveSH2Pending = 1;
      SS_RequestEHLExit();
      SMPC_WAIT_UNTIL_COND_SHORT(!SlaveSH2Pending);
     }
    }
    else if(ExecutingCommand == CMD_SSHOFF)
    {
     if(SlaveSH2On)
     {
      SlaveSH2Pending = -1;
      SS_RequestEHLExit();
      SMPC_WAIT_UNTIL_COND_SHORT(!SlaveSH2Pending);
     }
    }
   }

   ExecutingCommand = -1;
   SF = false;
   continue;
   //
   //
   //
   AbortJR:;

   SMPC_EAT_CLOCKS(87); // Conservatively low, may be higher in some contexts, hard to measure since timing is variable, possibly due to timing alignment
   IR0WA = 0;
   // TODO: Set TH TR to inputs?
   ExecutingCommand = -1;
   SF = false;
   continue;
  }
 }
 Breakout:;

 return ((sscpu_timestamp_t)(next_event_ts) < (sscpu_timestamp_t)(((sscpu_timestamp_t)(IOPorts[0]->NextEventTS) < (sscpu_timestamp_t)(IOPorts[1]->NextEventTS) ? (sscpu_timestamp_t)(IOPorts[0]->NextEventTS) : (sscpu_timestamp_t)(IOPorts[1]->NextEventTS))) ? (sscpu_timestamp_t)(next_event_ts) : (sscpu_timestamp_t)(((sscpu_timestamp_t)(IOPorts[0]->NextEventTS) < (sscpu_timestamp_t)(IOPorts[1]->NextEventTS) ? (sscpu_timestamp_t)(IOPorts[0]->NextEventTS) : (sscpu_timestamp_t)(IOPorts[1]->NextEventTS))));
}

void SMPC_SetVBVS(sscpu_timestamp_t event_timestamp, bool vb_status, bool vsync_status)
{
 if(vb ^ vb_status)
 {
  if(vb_status)	// Going into vblank
  {
   PendingVB = true;

   if(events[SS_EVENT_MIDSYNC].event_time == SS_EVENT_DISABLED_TS)
    SS_SetEventNT(&events[SS_EVENT_MIDSYNC], event_timestamp + 1);
  }

  SS_SetEventNT(&events[SS_EVENT_SMPC], event_timestamp + 1);
 }

 vb = vb_status;
 vsync = vsync_status;
}

void SMPC_LineHook(sscpu_timestamp_t event_timestamp, int32_t out_line, int32_t div, int32_t coord_adj)
{
 IOPorts[0]->vt->LineHook(IOPorts[0], event_timestamp, out_line, div, coord_adj);
 IOPorts[1]->vt->LineHook(IOPorts[1], event_timestamp, out_line, div, coord_adj);
 //
 //
 sscpu_timestamp_t nets = ((sscpu_timestamp_t)(events[SS_EVENT_SMPC].event_time) < (sscpu_timestamp_t)(((sscpu_timestamp_t)(IOPorts[0]->NextEventTS) < (sscpu_timestamp_t)(IOPorts[1]->NextEventTS) ? (sscpu_timestamp_t)(IOPorts[0]->NextEventTS) : (sscpu_timestamp_t)(IOPorts[1]->NextEventTS))) ? (sscpu_timestamp_t)(events[SS_EVENT_SMPC].event_time) : (sscpu_timestamp_t)(((sscpu_timestamp_t)(IOPorts[0]->NextEventTS) < (sscpu_timestamp_t)(IOPorts[1]->NextEventTS) ? (sscpu_timestamp_t)(IOPorts[0]->NextEventTS) : (sscpu_timestamp_t)(IOPorts[1]->NextEventTS))));

 SS_SetEventNT(&events[SS_EVENT_SMPC], nets);
}
