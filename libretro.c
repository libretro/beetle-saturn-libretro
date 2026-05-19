#include <stdarg.h>
#include <compat/msvc.h>
#include <libretro.h>
#include <rthreads/rthreads.h>
#include <string/stdstring.h>
#include <streams/file_stream.h>
#include <file/file_path.h>
#include <vfs/vfs_implementation.h>

#include "mednafen/ss/db.h"
#include "zip_reader.h"

#include <ctype.h>
#include <time.h>

#include "mednafen/mednafen-types.h"
#include "mednafen/settings.h"
/* MDFNGI and EmulateSpecStruct were the only types this TU needed
 * from the C++-only mednafen/git.h.  Both have C-compat POD homes
 * now -- mdfn_gameinfo.h and emuspec.h respectively.  Pull those
 * in directly; git.h would drag in <algorithm>/<string>/<vector>/
 * <map> through its CheatFormatStruct/GameDB_Entry/etc. surface,
 * which fails in a C TU. */
#include "mednafen/mdfn_gameinfo.h"
#include "mednafen/emuspec.h"
#include "mednafen/general.h"
#include "mednafen/mempatcher.h"
#include "mednafen/video/surface.h"
#ifdef NEED_DEINTERLACER
#include "mednafen/video/Deinterlacer.h"
#endif
#include "mednafen/cdrom/CDUtility.h"

#include "mednafen/ss/ss.h"
#include "mednafen/ss/cart.h"
#include "mednafen/ss/db.h"
#include "mednafen/ss/smpc.h"
#include "mednafen/ss/stvio.h"
#include "mednafen/ss/vdp1.h"
/* vdp2.h's one entry point used here is forward-declared below
 * to keep this TU's include surface light -- vdp2.h would
 * transitively pull in ss.h's full event-system surface for a
 * single function call.  Same pattern as vdp1.c uses for
 * VDP2_Update. */
#include "mednafen/ss/sound.h"

#include "libretro_core_options.h"
#include "libretro_settings.h"
#include "input.h"
#include "disc.h"

#define MEDNAFEN_CORE_NAME                   "Beetle Saturn"
#define MEDNAFEN_CORE_VERSION                "v1.32.1"
#define MEDNAFEN_CORE_VERSION_NUMERIC        0x00103201
#define MEDNAFEN_CORE_EXTENSIONS             "cue|ccd|chd|toc|m3u|zip"
/* MAX_W / MAX_H are the framebuffer ceiling reported to the
 * frontend at retro_get_system_av_info / SET_GEOMETRY time.
 * The matching BASE_W / BASE_H / ASPECT_RATIO constants that
 * used to live alongside them were unreferenced: the base
 * geometry is computed at runtime in retro_get_system_av_info
 * and the SET_GEOMETRY call site from h_mask + linevisfirst /
 * linevislast / is_pal, not from a compile-time default. */
#define MEDNAFEN_CORE_GEOMETRY_MAX_W         704
#define MEDNAFEN_CORE_GEOMETRY_MAX_H         576
#define FB_WIDTH                             MEDNAFEN_CORE_GEOMETRY_MAX_W

struct retro_perf_callback perf_cb;
retro_get_cpu_features_t perf_get_cpu_features_cb = NULL;
// fallback_log is defined below; we forward-declare here so log_cb has a
// safe default before retro_init runs. The previous default of NULL would
// have crashed if anything had logged before retro_init -- only safe by
// virtue of frontend call order.
static void fallback_log(enum retro_log_level level, const char *fmt, ...);
retro_log_printf_t log_cb                         = fallback_log;
static retro_audio_sample_t audio_cb              = NULL;
static retro_audio_sample_batch_t audio_batch_cb  = NULL;
static retro_input_poll_t input_poll_cb           = NULL;
static retro_input_state_t input_state_cb         = NULL;
static retro_environment_t environ_cb             = NULL;
static retro_video_refresh_t video_cb             = NULL;

static unsigned frame_count = 0;
static unsigned image_offset = 0;
static unsigned image_crop = 0;

// Geometry tracking. These used to be retro_run-local function-statics,
// which meant they persisted across retro_unload_game / retro_load_game
// and could prevent SET_GEOMETRY from firing for a new game that happened
// to match the previous game's dimensions. File-scope + reset in
// retro_load_game makes the lifetime explicit.
static unsigned cur_width = 0;
static unsigned cur_height = 0;
static unsigned game_width = 0;
static unsigned game_height = 0;

static unsigned h_mask = 0;
static unsigned first_sl = 0;
static unsigned last_sl = 239;
static unsigned first_sl_pal = 0;
static unsigned last_sl_pal = 287;
bool is_pal = false;

int setting_crosshair_color_p1 = 0xFF0000;
int setting_crosshair_color_p2 = 0x0080FF;

bool cdimagecache = false;

// shared internal memory support
bool shared_intmemory = false;
bool shared_intmemory_toggle = false;

// shared backup memory support
bool shared_backup = false;
bool shared_backup_toggle = false;

char retro_save_directory[4096];
char retro_base_directory[4096];
static char retro_cd_base_directory[4096];
static char retro_cd_path[4096];
char retro_cd_base_name[4096];

#ifndef RETRO_SLASH
#ifdef _WIN32
#define RETRO_SLASH "\\"
#else
#define RETRO_SLASH "/"
#endif
#endif

static bool libretro_supports_bitmasks = false;

MDFNGI *MDFNGameInfo = NULL;

extern MDFNGI EmulatedSS;

/* Local forward decl for the single VDP2 entry point this TU
 * reaches into.  See the include block above for why vdp2.h isn't
 * pulled in directly. */
extern void VDP2_SetDeinterlaceOff(bool off);

#ifdef NEED_DEINTERLACER
static bool PrevInterlaced;
static Deinterlacer deint;
#else
/* Two uses of PrevInterlaced in retro_run's frame-emit path
 * (cur_height calculation and the pix pointer offset) are intentionally
 * not gated, because the height/offset math needs a value either way.
 * Without the SW deinterlacer, fields are never combined, so the
 * single-field semantics (<< 0 = identity) are correct.  Declaring as
 * a const false keeps every call site source-compatible. */
static const bool PrevInterlaced = false;
#endif

static MDFN_Surface *surf = NULL;

static void alloc_surface(void)
{
  uint32_t width  = MEDNAFEN_CORE_GEOMETRY_MAX_W;
  uint32_t height = MEDNAFEN_CORE_GEOMETRY_MAX_H;

  // The MDFN_PixelFormat construction is gone; the new surface API
  // (ported in from beetle-psx-libretro) hard-codes a single
  // 32bpp RGBA layout via the RED_SHIFT/GREEN_SHIFT/etc macros in
  // surface.h. Every call site was already passing the same
  // R=16/G=8/B=0/A=24 shifts anyway.
  MDFN_Surface_Delete(surf);
  surf = MDFN_Surface_New(width, height, width);
}

static void check_system_specs(void)
{
   // Hints that we need a fairly powerful system to run this.
   unsigned level = 15;
   environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
   (void)level;
   va_list va;
   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

/* LED interface */
extern int Running; // variable in ss.c
extern uint8_t GetDriveStatus(void);
static retro_set_led_state_t led_state_cb = NULL;
static unsigned int retro_led_state[2] = {0};
static void retro_led_interface(void)
{
   /* 0: Power
    * 1: CD */

   unsigned int led_state[2] = {0};
   unsigned int l            = 0;

   unsigned int drive_status = GetDriveStatus();
   /* Active values:
    * STATUS_BUSY = 0x00,
    * STATUS_PLAY = 0x03,
    * STATUS_SEEK = 0x04,
    * STATUS_SCAN = 0x05, */

   led_state[0] = (!Running) ? 1 : 0;
   led_state[1] = (drive_status == 0 || drive_status == 3 || drive_status == 4 || drive_status == 5) ? 1 : 0;

   for (l = 0; l < sizeof(led_state)/sizeof(led_state[0]); l++)
   {
      if (retro_led_state[l] != led_state[l])
      {
         retro_led_state[l] = led_state[l];
         led_state_cb(l, led_state[l]);
      }
   }
}

void retro_init(void)
{
   const char *dir = NULL;
   struct retro_log_callback log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = fallback_log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
   {
      snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", dir);
   }

   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) && dir)
   {
      /* If save directory is defined use it, otherwise use system directory */
      if (dir)
         snprintf(retro_save_directory, sizeof(retro_save_directory), "%s", dir);
      else
         snprintf(retro_save_directory, sizeof(retro_save_directory), "%s", retro_base_directory);
   }
   else
   {
      snprintf(retro_save_directory, sizeof(retro_save_directory), "%s", retro_base_directory);
   }

   CDUtility_Init();
   disc_init();

#ifdef NEED_DEINTERLACER
   // C struct now, not a C++ class with a constructor. Init zeroes
   // StateValid/PrevDRect_* and sets DeintType = DEINT_WEAVE, matching
   // the previous default-constructed Saturn Deinterlacer behaviour.
   Deinterlacer_Init(&deint);
#endif

   if (environ_cb(RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf_cb))
      perf_get_cpu_features_cb = perf_cb.get_cpu_features;
   else
      perf_get_cpu_features_cb = NULL;

   setting_region = 0; // auto
   setting_smpc_autortc = true;
   setting_smpc_autortc_lang = 0;
   setting_initial_scanline = 0;
   setting_last_scanline = 239;
   setting_initial_scanline_pal = 0;
   setting_last_scanline_pal = 287;

   /* The save-state size emitted by MDFNSS_SaveSM is data-driven --
    * the SMPC sub-state writes one SFORMAT block per attached I/O
    * device, and each device subclass (Control Pad / 3D Control Pad
    * / Mission Stick / Arcade Racer / Mouse / Twin Stick / Keyboard)
    * has a different SFORMAT.  Switching device type via
    * retro_set_controller_port_device therefore grows or shrinks
    * the serialized state size from one frame to the next.
    *
    * Without this hint, RetroArch's rewind buffer (which allocates
    * `retro_serialize_size()` bytes per frame at start-up) treats
    * a mid-session size change as "save state failed", invalidates
    * the buffer entries that pre-date the device switch, and
    * effectively rewinds only as far back as the moment the input
    * device was changed.  CORE_VARIABLE_SIZE tells the front-end
    * to size each rewind slot from the live serialize_size() rather
    * than the cached startup value, which keeps the rewind chain
    * intact across device changes.  See issue #21. */
   {
      uint64_t serial_quirks = RETRO_SERIALIZATION_QUIRK_CORE_VARIABLE_SIZE;
      environ_cb(RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS, &serial_quirks);
   }

   /* GET_INPUT_BITMASKS hint: when the front-end supports it,
    * input_update_with_bitmasks() can fetch all 14+ digital
    * JOYPAD buttons for a port in a single retro_input_state_t
    * call (passing RETRO_DEVICE_ID_JOYPAD_MASK as the id) instead
    * of one call per button.  At 12 ports * ~14 buttons per port,
    * that's the difference between ~12 and ~168 dispatch hops
    * across the libretro state-callback boundary per frame; the
    * win matters most on front-ends where that boundary is
    * non-trivial (netplay, replay-recording, runahead). */
   if (environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL))
      libretro_supports_bitmasks = true;

   check_system_specs();
}

void retro_reset(void)
{
   SS_Reset(true);
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   return false;
}

static void check_variables(bool startup)
{
   struct retro_variable var = {0};

   if (startup)
   {
      var.key = "beetle_saturn_cdimagecache";
      var.value = NULL;
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
      {
         cdimagecache = false;
         if (!strcmp(var.value, "enabled"))
            cdimagecache = true;
      }

      var.key = "beetle_saturn_shared_int";
      var.value = NULL;
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
      {
         if (!strcmp(var.value, "enabled"))
            shared_intmemory_toggle = true;
         else if (!strcmp(var.value, "disabled"))
            shared_intmemory_toggle = false;
      }

      var.key = "beetle_saturn_shared_ext";
      var.value = NULL;
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
      {
         if (!strcmp(var.value, "enabled"))
            shared_backup_toggle = true;
         else if (!strcmp(var.value, "disabled"))
            shared_backup_toggle = false;
      }
   }

   var.key = "beetle_saturn_region";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "Auto Detect") || !strcmp(var.value, "auto"))
         setting_region = 0;
      else if (!strcmp(var.value, "Japan") || !strcmp(var.value, "jp"))
         setting_region = SMPC_AREA_JP;
      else if (!strcmp(var.value, "North America") || !strcmp(var.value, "na"))
         setting_region = SMPC_AREA_NA;
      else if (!strcmp(var.value, "Europe") || !strcmp(var.value, "eu"))
         setting_region = SMPC_AREA_EU_PAL;
      else if (!strcmp(var.value, "South Korea") || !strcmp(var.value, "kr"))
         setting_region = SMPC_AREA_KR;
      else if (!strcmp(var.value, "Asia (NTSC)") || !strcmp(var.value, "tw"))
         setting_region = SMPC_AREA_ASIA_NTSC;
      else if (!strcmp(var.value, "Asia (PAL)") || !strcmp(var.value, "as"))
         setting_region = SMPC_AREA_ASIA_PAL;
      else if (!strcmp(var.value, "Brazil") || !strcmp(var.value, "br"))
         setting_region = SMPC_AREA_CSA_NTSC;
      else if (!strcmp(var.value, "Latin America") || !strcmp(var.value, "la"))
         setting_region = SMPC_AREA_CSA_PAL;
   }

   var.key = "beetle_saturn_cart";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "Auto Detect") || !strcmp(var.value, "auto"))
         setting_cart = CART__RESERVED;
      else if (!strcmp(var.value, "None") || !strcmp(var.value, "none"))
         setting_cart = CART_NONE;
      else if (!strcmp(var.value, "Backup Memory") || !strcmp(var.value, "backup"))
         setting_cart = CART_BACKUP_MEM;
      else if (!strcmp(var.value, "Extended RAM (1MB)") || !strcmp(var.value, "extram1"))
         setting_cart = CART_EXTRAM_1M;
      else if (!strcmp(var.value, "Extended RAM (4MB)") || !strcmp(var.value, "extram4"))
         setting_cart = CART_EXTRAM_4M;
      else if (!strcmp(var.value, "The King of Fighters '95") || !strcmp(var.value, "kof95"))
         setting_cart = CART_KOF95;
      else if (!strcmp(var.value, "Ultraman: Hikari no Kyojin Densetsu") || !strcmp(var.value, "ultraman"))
         setting_cart = CART_ULTRAMAN;
   }

   var.key = "beetle_saturn_multitap_port1";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      bool connected = false;
      if (!strcmp(var.value, "enabled"))
         connected = true;
      else if (!strcmp(var.value, "disabled"))
         connected = false;

      input_multitap(1, connected);
   }

   var.key = "beetle_saturn_multitap_port2";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      bool connected = false;
      if (!strcmp(var.value, "enabled"))
         connected = true;
      else if (!strcmp(var.value, "disabled"))
         connected = false;

      input_multitap(2, connected);
   }

   var.key = "beetle_saturn_opposite_directions";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         opposite_directions = true;
      else if (!strcmp(var.value, "disabled"))
         opposite_directions = false;
   }

   var.key = "beetle_saturn_midsync";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         setting_midsync = true;
      else if (!strcmp(var.value, "disabled"))
         setting_midsync = false;
   }

   var.key = "beetle_saturn_autortc";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         setting_smpc_autortc = 1;
      else if (!strcmp(var.value, "disabled"))
         setting_smpc_autortc = 0;
   }

   var.key = "beetle_saturn_autortc_lang";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "english"))
         setting_smpc_autortc_lang = 0;
      else if (!strcmp(var.value, "german"))
         setting_smpc_autortc_lang = 1;
      else if (!strcmp(var.value, "french"))
         setting_smpc_autortc_lang = 2;
      else if (!strcmp(var.value, "spanish"))
         setting_smpc_autortc_lang = 3;
      else if (!strcmp(var.value, "italian"))
         setting_smpc_autortc_lang = 4;
      else if (!strcmp(var.value, "japanese"))
         setting_smpc_autortc_lang = 5;
   }

   var.key = "beetle_saturn_horizontal_overscan";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      h_mask = atoi(var.value);
   }

   var.key = "beetle_saturn_initial_scanline";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      first_sl = atoi(var.value);
   }

   var.key = "beetle_saturn_last_scanline";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      last_sl = atoi(var.value);
   }

   var.key = "beetle_saturn_initial_scanline_pal";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      first_sl_pal = atoi(var.value);
   }

   var.key = "beetle_saturn_last_scanline_pal";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      last_sl_pal = atoi(var.value);
   }

   var.key = "beetle_saturn_horizontal_blend";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      bool newval = (!strcmp(var.value, "enabled"));
      DoHBlend = newval;
   }

   var.key = "beetle_saturn_deinterlacer";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      // The "off" mode pairs DEINT_OFF on the deinterlacer (which then
      // does nothing) with VDP2_SetDeinterlaceOff(true) on the VDP2
      // renderer side, which causes each rendered scanline to also be
      // memcpy'd to the opposite-field row of the libretro surface.
      // Every emulated frame thus produces a stable, full-resolution
      // progressive image. All other modes leave the renderer in its
      // default single-field-per-frame behaviour and let the
      // deinterlacer combine fields after the fact.
      const bool off = (strcmp(var.value, "off") == 0);
      VDP2_SetDeinterlaceOff(off);

#ifdef NEED_DEINTERLACER
      /* The 'deint' instance is itself #ifdef NEED_DEINTERLACER (declared
       * at file scope above), as are every other Deinterlacer_* call in
       * this TU (Init, ClearState, Process, GetType, Cleanup).  This
       * setting-handler chain was the lone exception, breaking the
       * NEED_DEINTERLACER=0 build.  VDP2_SetDeinterlaceOff stays
       * outside the gate -- VDP2 is the GPU subsystem and independent
       * of the SW deinterlacer. */
      if (strcmp(var.value, "bob") == 0)
         Deinterlacer_SetType(&deint, DEINT_BOB);
      else if (strcmp(var.value, "bob_offset") == 0)
         Deinterlacer_SetType(&deint, DEINT_BOB_OFFSET);
      else if (strcmp(var.value, "fastmad") == 0)
         Deinterlacer_SetType(&deint, DEINT_FASTMAD);
      else if (off)
         Deinterlacer_SetType(&deint, DEINT_OFF);
      else
         Deinterlacer_SetType(&deint, DEINT_WEAVE);
#endif
   }

   var.key = "beetle_saturn_mesh_transparency";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      // Forwarded to VDP1_SetMeshImproved which sets a module-level
      // bool. PlotPixel reads it inside its MeshEn=true template
      // path. Called from the same thread that runs SH-2 / VDP1
      // rasterisation, so no synchronisation needed.
      VDP1_SetMeshImproved(strcmp(var.value, "enabled") == 0);
   }

   var.key = "beetle_saturn_analog_stick_deadzone";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      input_set_deadzone_stick(atoi(var.value));
   }

   var.key = "beetle_saturn_trigger_deadzone";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      input_set_deadzone_trigger(atoi(var.value));
   }

   var.key = "beetle_saturn_mouse_sensitivity";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      input_set_mouse_sensitivity(atoi(var.value));
   }

   var.key   = "beetle_saturn_virtuagun_input";
   var.value = NULL;

   if ( environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value )
   {
      if ( !strcmp(var.value, "Touchscreen") ) {
         setting_gun_input = SETTING_GUN_INPUT_POINTER;
      } else {
         setting_gun_input = SETTING_GUN_INPUT_LIGHTGUN;
      }
   }

   var.key = "beetle_saturn_virtuagun_crosshair";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "Off"))
         setting_gun_crosshair = SETTING_GUN_CROSSHAIR_OFF;
      else if (!strcmp(var.value, "Cross"))
         setting_gun_crosshair = SETTING_GUN_CROSSHAIR_CROSS;
      else if (!strcmp(var.value, "Dot"))
         setting_gun_crosshair = SETTING_GUN_CROSSHAIR_DOT;
   }

   var.key = "beetle_saturn_crosshair_color_p1";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (strcmp(var.value, "red") == 0)
         setting_crosshair_color_p1 = 0xFF0000;
      else if (strcmp(var.value, "blue") == 0)
         setting_crosshair_color_p1 = 0x0080FF;
      else if (strcmp(var.value, "green") == 0)
         setting_crosshair_color_p1 = 0x00FF00;
      else if (strcmp(var.value, "orange") == 0)
         setting_crosshair_color_p1 = 0xFF8000;
      else if (strcmp(var.value, "yellow") == 0)
         setting_crosshair_color_p1 = 0xFFFF00;
      else if (strcmp(var.value, "cyan") == 0)
         setting_crosshair_color_p1 = 0x00FFFF;
      else if (strcmp(var.value, "pink") == 0)
         setting_crosshair_color_p1 = 0xFF00FF;
      else if (strcmp(var.value, "purple") == 0)
         setting_crosshair_color_p1 = 0x8000FF;
      else if (strcmp(var.value, "black") == 0)
         setting_crosshair_color_p1 = 0x000000;
      else if (strcmp(var.value, "white") == 0)
         setting_crosshair_color_p1 = 0xFFFFFF;
   }

   var.key = "beetle_saturn_crosshair_color_p2";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (strcmp(var.value, "red") == 0)
         setting_crosshair_color_p2 = 0xFF0000;
      else if (strcmp(var.value, "blue") == 0)
         setting_crosshair_color_p2 = 0x0080FF;
      else if (strcmp(var.value, "green") == 0)
         setting_crosshair_color_p2 = 0x00FF00;
      else if (strcmp(var.value, "orange") == 0)
         setting_crosshair_color_p2 = 0xFF8000;
      else if (strcmp(var.value, "yellow") == 0)
         setting_crosshair_color_p2 = 0xFFFF00;
      else if (strcmp(var.value, "cyan") == 0)
         setting_crosshair_color_p2 = 0x00FFFF;
      else if (strcmp(var.value, "pink") == 0)
         setting_crosshair_color_p2 = 0xFF00FF;
      else if (strcmp(var.value, "purple") == 0)
         setting_crosshair_color_p2 = 0x8000FF;
      else if (strcmp(var.value, "black") == 0)
         setting_crosshair_color_p2 = 0x000000;
      else if (strcmp(var.value, "white") == 0)
         setting_crosshair_color_p2 = 0xFFFFFF;
   }

   SMPC_SetCrosshairsColor(0, setting_crosshair_color_p1);
   SMPC_SetCrosshairsColor(1, setting_crosshair_color_p2);

   /* ST-V's hammer/gun-control scheme owns its own IODevice on stvio's
    * side (separate from SMPC's per-port virtual ports), so the same
    * p1 colour needs to be forwarded to the stvio gun.  Safe to call
    * unconditionally -- STVIO_SetCrosshairsColor no-ops when the gun
    * isn't allocated (non-ST-V games, or before STVIO_Init has run). */
   STVIO_SetCrosshairsColor(0, setting_crosshair_color_p1);
}

/* =====================================================================
 * ST-V .zip content support
 * =====================================================================
 *
 * MAME-style multi-file ROM archives are the universal distribution
 * format for ST-V games.  The frontend (RetroArch) may auto-extract
 * the archive, but its extractor matches files inside the zip against
 * the core's valid_extensions -- ST-V ROM filenames (`epr20825.13`,
 * `mpr20826.1`, ...) match nothing in our list, so the frontend errors
 * out with "Failed to extract content from compressed file" before the
 * core sees anything.
 *
 * Handle .zip content paths in-core via zip_reader:
 *
 *   1. Detect `.zip` extension on the supplied path.
 *   2. Open the archive, enumerate its entries.
 *   3. Use DB_LookupSTV_ByPredicate to find which ST-V game these
 *      ROMs belong to -- the predicate consults zip_find() per
 *      candidate rom_layout filename.
 *   4. Extract the matched game's rom_layout files to a per-archive
 *      cache directory under retro_save_directory.
 *   5. Re-enter the bare-file ST-V load path with that cache dir as
 *      rom_dir, so cart/stv.c's existing JoinPath / filestream_open
 *      loop sees the extracted files as if the user had unpacked
 *      them by hand.
 *
 * Caching: extracted files persist in retro_save_directory/stv_extract/
 * <archive-basename>/ across launches.  A cache hit is detected by
 * checking the on-disk file size against the zip's central directory
 * uncompressed size; mismatch -> re-extract.  CRC validation is left
 * to zip_extract on the extract path (we don't checksum the cache
 * file every launch -- file-size match is the cheap discriminator). */

/* Predicate context for DB_LookupSTV_ByPredicate -- pass a zip_archive
 * and zip_find() per candidate filename. */
struct stv_zip_lookup_ctx
{
   const zip_archive *za;
};

static bool stv_zip_has_file(void *ctx, const char *fname)
{
   const struct stv_zip_lookup_ctx *c = (const struct stv_zip_lookup_ctx*)ctx;
   return zip_find(c->za, fname) != NULL;
}

/* mkdir-p: create `dir` and any missing parents.  libretro-common's
 * file_path.h declares path_mkdir() with recursive-parent semantics
 * but its file_path.c never defines the function -- both the in-tree
 * subset and current upstream master are missing the body.  Roll our
 * own on top of retro_vfs_mkdir_impl (which does a single mkdir):
 * try the leaf first; on parent-missing failure, peel the last path
 * component off and recurse on the parent before retrying the leaf.
 *
 * retro_vfs_mkdir_impl return codes:
 *   0   created
 *  -1   error (could be parent-missing, permissions, etc.)
 *  -2   already exists (path_mkdir_error macro family)
 *
 * "Already exists" is success for our purposes -- the leaf or one of
 * the parents being there is fine.  Failure to distinguish "parent
 * missing" from "permissions" / "disk full" / etc. is acceptable
 * since we recurse on -1 anyway; if the parent is also unmakeable
 * we eventually return false. */
static bool make_dir_recursive(const char *path)
{
   int ret;
   char parent[4096];
   char *last_sep;
   char *last_bsep;
   size_t plen;

   if (!path || !*path)
      return false;

   ret = retro_vfs_mkdir_impl(path);
   if (ret == 0 || ret == -2)
      return true;

   /* Peel last component to recurse on parent.  Handle both forward
    * and back slashes since the path may be a mix on Windows. */
   plen = strlen(path);
   if (plen >= sizeof(parent))
      return false;
   memcpy(parent, path, plen + 1);

   last_sep  = strrchr(parent, '/');
   last_bsep = strrchr(parent, '\\');
   if (last_bsep && (!last_sep || last_bsep > last_sep))
      last_sep = last_bsep;
   if (!last_sep || last_sep == parent)
      return false;  /* no parent to peel, or already at root */

   *last_sep = '\0';

   if (!make_dir_recursive(parent))
      return false;

   /* Parent created (or pre-existed); retry the leaf. */
   ret = retro_vfs_mkdir_impl(path);
   return (ret == 0 || ret == -2);
}

/* Strip ".zip" (case-insensitive) from `basename` in place.  Used to
 * derive the per-archive cache subdir name. */
static void strip_zip_ext(char *basename)
{
   size_t len = strlen(basename);
   if (len >= 4)
   {
      char *suffix = basename + len - 4;
      if (   (suffix[0] == '.')
          && (suffix[1] == 'z' || suffix[1] == 'Z')
          && (suffix[2] == 'i' || suffix[2] == 'I')
          && (suffix[3] == 'p' || suffix[3] == 'P'))
         *suffix = '\0';
   }
}

/* Check whether `path` exists with size exactly `expected`.  Used by
 * the cache-hit detector to skip extraction when the file is already
 * unpacked. */
static bool file_size_matches(const char *path, uint32_t expected)
{
   RFILE  *fp = filestream_open(path,
         RETRO_VFS_FILE_ACCESS_READ,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);
   int64_t sz;
   if (!fp)
      return false;
   sz = filestream_get_size(fp);
   filestream_close(fp);
   return sz == (int64_t)expected;
}

/* Open `zip_path`, identify the ST-V game inside via the predicate
 * lookup, and extract its rom_layout files to a per-archive cache dir
 * under retro_save_directory.  On success: writes the cache dir into
 * `out_dir` and returns the matched STVGameInfo.  On failure: returns
 * NULL and leaves out_dir undefined. */
static const struct STVGameInfo* prepare_stv_zip_content(
      const char *zip_path, char *out_dir, size_t out_dir_size)
{
   zip_archive                  za;
   struct stv_zip_lookup_ctx    pred_ctx;
   const struct STVGameInfo    *sgi;
   char                         arch_base[256];
   size_t                       i;

   if (!zip_open(&za, zip_path))
   {
      log_cb(RETRO_LOG_ERROR, "ST-V zip: failed to open archive \"%s\".\n", zip_path);
      return NULL;
   }

   pred_ctx.za = &za;
   sgi         = DB_LookupSTV_ByPredicate(stv_zip_has_file, &pred_ctx);
   if (!sgi)
   {
      log_cb(RETRO_LOG_ERROR,
            "ST-V zip: no ST-V game in DB matches the contents of \"%s\".\n",
            zip_path);
      zip_close(&za);
      return NULL;
   }

   log_cb(RETRO_LOG_INFO, "ST-V zip: identified as \"%s\".\n", sgi->name);

   /* Build the cache subdir path: <save>/stv_extract/<archive_base>/  */
   {
      const char *base = path_basename(zip_path);
      if (!base)
         base = zip_path;
      strncpy(arch_base, base, sizeof(arch_base) - 1);
      arch_base[sizeof(arch_base) - 1] = '\0';
      strip_zip_ext(arch_base);
   }
   snprintf(out_dir, out_dir_size,
         "%s" RETRO_SLASH "stv_extract" RETRO_SLASH "%s",
         retro_save_directory, arch_base);

   /* mkdir -p semantics: create the leaf plus any missing parents.
    * libretro-common's path_mkdir declaration has no matching body
    * in this fork's subset; make_dir_recursive wraps retro_vfs_mkdir_impl
    * with a peel-and-recurse parent walk. */
   if (!make_dir_recursive(out_dir))
   {
      log_cb(RETRO_LOG_ERROR,
            "ST-V zip: cannot create cache dir \"%s\".\n", out_dir);
      zip_close(&za);
      return NULL;
   }

   /* Extract every rom_layout file for this game (skipping mirrored
    * slots that point to the same source file, which cart/stv.c's
    * loader detects via the prev_match test). */
   for (i = 0;
        i < sizeof(sgi->rom_layout) / sizeof(sgi->rom_layout[0])
              && sgi->rom_layout[i].size;
        i++)
   {
      const struct STVROMLayout *rle = &sgi->rom_layout[i];
      const struct zip_entry    *ze;
      char                       out_path[4096];
      RFILE                     *out_fp;
      uint8_t                   *buf;

      /* Mirror-slot dedup: same fname as the previous slot means the
       * loader will copy from the in-memory previous mapping without
       * re-opening the file, so we don't need to write it twice. */
      if (i > 0 && !strcmp(rle->fname, sgi->rom_layout[i - 1].fname))
         continue;

      ze = zip_find(&za, rle->fname);
      if (!ze)
      {
         log_cb(RETRO_LOG_ERROR,
               "ST-V zip: archive missing required ROM \"%s\".\n",
               rle->fname);
         zip_close(&za);
         return NULL;
      }

      snprintf(out_path, sizeof(out_path),
            "%s" RETRO_SLASH "%s", out_dir, rle->fname);

      /* Cache hit: skip if the on-disk file already has the right
       * uncompressed size.  We trust the size as a fingerprint --
       * a torn write or partial copy would mismatch; a deliberate
       * corruption is the user's problem. */
      if (file_size_matches(out_path, ze->uncompressed_size))
         continue;

      buf = (uint8_t*)malloc(ze->uncompressed_size);
      if (!buf)
      {
         log_cb(RETRO_LOG_ERROR,
               "ST-V zip: out of memory extracting \"%s\".\n", rle->fname);
         zip_close(&za);
         return NULL;
      }
      if (!zip_extract(&za, ze, buf))
      {
         log_cb(RETRO_LOG_ERROR,
               "ST-V zip: extract failed for \"%s\" (CRC mismatch?).\n",
               rle->fname);
         free(buf);
         zip_close(&za);
         return NULL;
      }

      out_fp = filestream_open(out_path,
            RETRO_VFS_FILE_ACCESS_WRITE,
            RETRO_VFS_FILE_ACCESS_HINT_NONE);
      if (!out_fp)
      {
         log_cb(RETRO_LOG_ERROR,
               "ST-V zip: cannot write to cache file \"%s\".\n", out_path);
         free(buf);
         zip_close(&za);
         return NULL;
      }
      if (filestream_write(out_fp, buf, ze->uncompressed_size)
            != (int64_t)ze->uncompressed_size)
      {
         log_cb(RETRO_LOG_ERROR,
               "ST-V zip: short write to cache file \"%s\".\n", out_path);
         filestream_close(out_fp);
         free(buf);
         zip_close(&za);
         return NULL;
      }
      filestream_close(out_fp);
      free(buf);
   }

   zip_close(&za);
   return sgi;
}

static bool MDFNI_LoadGame(const char *name)
{
   unsigned horrible_hacks   = 0;
   // .. safe defaults
   unsigned region           = SMPC_AREA_NA;
   int cart_type             = CART_BACKUP_MEM;
   unsigned cpucache_emumode = CPUCACHE_EMUMODE_DATA;

   // always set this.
   MDFNGameInfo = &EmulatedSS;

   size_t name_len = strlen(name);

   // check for a valid file extension
   if (name_len > 3)
   {
      const char *ext = name + name_len - 3;

      // supported extension?
      if ((!strcasecmp( ext, "ccd" )) ||
          (!strcasecmp( ext, "chd" )) ||
          (!strcasecmp( ext, "cue" )) ||
          (!strcasecmp( ext, "toc" )) ||
          (!strcasecmp( ext, "m3u" )) )
      {
         uint8_t fd_id[16];
         char sgid[16 + 1]     = { 0 };
         char sgname[0x70 + 1] = { 0 };
         char sgarea[0x10 + 1] = { 0 };

         if (disc_load_content(name, fd_id, sgid, sgname, sgarea, cdimagecache))
         {
            /* CD/m3u content successfully loaded.  Register the
             * disc-control vtable with RetroArch now that we know
             * we actually have discs to control.  ST-V .zip content
             * never reaches this branch, so its boot stays free of
             * the spurious 'Failed to set last used disc' toast. */
            disc_register_environment(environ_cb);

            log_cb(RETRO_LOG_INFO, "Game ID is: %s\n", sgid);

            // test discs?
            bool discs_ok = true;
            if (setting_disc_test)
               discs_ok = DiscSanityChecks();

            if (discs_ok)
            {
               DetectRegion(&region);

               DB_Lookup(NULL, sgid, sgname, sgarea, fd_id, &region, &cart_type, &cpucache_emumode);
               horrible_hacks = DB_LookupHH(sgid, fd_id);

               // forced region setting?
               if (setting_region != 0)
                  region = setting_region;

               // forced cartridge setting?
               if (setting_cart != CART__RESERVED)
                  cart_type = setting_cart;

               // GO!
               if (InitCommon(cpucache_emumode,
                    horrible_hacks, cart_type, region,
                    NULL, NULL, NULL))
               {
                  MDFN_LoadGameCheats();
                  return true;
               }

               // OK it's really bad. Probably don't 
               // have a BIOS if InitCommon
               // fails. We can't continue as an 
               // emulator and will show a blank
               // screen.

               disc_cleanup();
               return false;
            } // discs okay?

         } // load content

      } // supported extension?

   } // valid name?

   //
   // Not a disc image. Try ST-V: open the file and ask the ST-V game DB
   // whether the filename + first 128 bytes match a known ROM set. The
   // STV game database (DB_LookupSTV) handles the file-extension and
   // CRC32 disambiguation internally -- much simpler than maintaining
   // a separate filename-list check here.
   //
   // .zip content takes a different sub-path: prepare_stv_zip_content
   // identifies the game via DB_LookupSTV_ByPredicate (zip_find as the
   // resolver) and extracts the rom_layout files to a cache dir under
   // retro_save_directory.  After successful extraction we synthesise
   // a (rom_dir, base_filename) pair pointing into that cache dir and
   // hand it to InitCommon exactly as the bare-file path does.
   //
   if (name && name[0])
   {
      /* zip path: detect ".zip" suffix, extract, and InitCommon
       * with the cache dir.  No fall-through to the cdstream-based
       * bare-file detection -- if the archive opens but isn't ST-V,
       * we want a clear "no game matched" error rather than a
       * second pass that tries to interpret the zip-as-binary. */
      size_t nlen = strlen(name);
      if (nlen >= 4 && !strcasecmp(name + nlen - 4, ".zip"))
      {
         char                       cache_dir[4096];
         const struct STVGameInfo  *sgi =
            prepare_stv_zip_content(name, cache_dir, sizeof(cache_dir));

         if (sgi)
         {
            log_cb(RETRO_LOG_INFO, "ST-V game: %s\n", sgi->name);

            region            = sgi->area;
            cart_type         = CART_STV;
            cpucache_emumode  = CPUCACHE_EMUMODE_FULL;
            horrible_hacks    = 0;

            /* base_filename for the InitCommon contract -- the loader
             * uses it for log/error context only (cart/stv.c walks
             * rom_layout independently); pass rom_layout[0].fname
             * as the natural "lead" file the cache dir contains. */
            if (InitCommon(cpucache_emumode, horrible_hacks, cart_type,
                           region, cache_dir, sgi->rom_layout[0].fname, sgi))
            {
               MDFN_LoadGameCheats();
               return true;
            }
            log_cb(RETRO_LOG_ERROR, "ST-V InitCommon failed.\n");
            return false;
         }
         /* zip opened but no ST-V game matched, OR open failed: fall
          * through to BIOS drop.  prepare_stv_zip_content has already
          * logged the specific reason. */
      }
      else
      {
         /* Bare-file path: the historic detection.  cdstream_open
          * the file, DB_LookupSTV with the first 128 bytes for
          * head_crc32 disambiguation. */
         cdstream stvfs;
         if (cdstream_open(&stvfs, name))
         {
            // Extract directory + basename. path_basedir mutates its
            // argument so work on a scratch copy.
            char dir_buf[4096];
            strncpy(dir_buf, name, sizeof(dir_buf) - 1);
            dir_buf[sizeof(dir_buf) - 1] = '\0';
            fill_pathname_basedir(dir_buf, name, sizeof(dir_buf));
            // path_basedir leaves a trailing slash; trim it so cart/stv.c's
            // JoinPath doesn't double up.
            size_t dirlen = strlen(dir_buf);
            while (dirlen && (dir_buf[dirlen - 1] == '/' || dir_buf[dirlen - 1] == '\\'))
               dir_buf[--dirlen] = '\0';

            const char* base = path_basename(name);

            const struct STVGameInfo* sgi = DB_LookupSTV(base ? base : "", &stvfs);

            cdstream_close(&stvfs);

            if (sgi)
            {
               // ST-V game recognised. Region comes from the game DB
               // (cabinet region; affects BIOS selection); cart_type is
               // forced to CART_STV. cpucache_emumode and horrible_hacks
               // are pinned to FULL / VDP1RWDRAWSLOWDOWN since upstream
               // hardcodes these for ST-V (the per-game CemuDB / HHDB
               // tables don't cover ST-V).
               log_cb(RETRO_LOG_INFO, "ST-V game: %s\n", sgi->name);

               region            = sgi->area;
               cart_type         = CART_STV;
               cpucache_emumode  = CPUCACHE_EMUMODE_FULL;
               horrible_hacks    = 0; // HORRIBLEHACK_VDP1RWDRAWSLOWDOWN if exposed

               if (InitCommon(cpucache_emumode, horrible_hacks, cart_type,
                              region, dir_buf, base, sgi))
               {
                  MDFN_LoadGameCheats();
                  return true;
               }
               log_cb(RETRO_LOG_ERROR, "ST-V InitCommon failed.\n");
               return false;
            }
         }
      }
      // Open or lookup failed; fall through to BIOS drop below.
   }

   //
   // Drop to BIOS

   disc_cleanup();

   // forced region setting?
   if (setting_region != 0)
      region = setting_region;

   // forced cartridge setting?
   if (setting_cart != CART__RESERVED)
      cart_type = setting_cart;

   // Initialise with safe parameters
   if (!InitCommon(cpucache_emumode, horrible_hacks, cart_type, region, NULL, NULL, NULL))
      return false;

   MDFN_LoadGameCheats();

   return true;
}

bool retro_load_game(const struct retro_game_info *info)
{
   char tocbasepath[4096];

   if (!info)
      return false;

   input_init_env(environ_cb);

   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
      return false;

   extract_basename(retro_cd_base_name,       info->path, sizeof(retro_cd_base_name));
   extract_directory(retro_cd_base_directory, info->path, sizeof(retro_cd_base_directory));

   snprintf(tocbasepath, sizeof(tocbasepath), "%s" RETRO_SLASH "%s.toc", retro_cd_base_directory, retro_cd_base_name);

   if (!strstr(tocbasepath, "cdrom://") && filestream_exists(tocbasepath))
      snprintf(retro_cd_path, sizeof(retro_cd_path), "%s", tocbasepath);
   else
      snprintf(retro_cd_path, sizeof(retro_cd_path), "%s", info->path);

   check_variables(true);
   //make sure shared memory cards and save states are enabled only at startup
   shared_intmemory = shared_intmemory_toggle;
   shared_backup = shared_backup_toggle;

   // Let's try to load the game. If this fails then things are very wrong.
   if (MDFNI_LoadGame(retro_cd_path) == false)
      return false;

   // MDFN_LoadGameCheats() was called here previously, but MDFNI_LoadGame
   // already invokes it at the end of its success path. The duplicate
   // call was redundant (and potentially harmful if it ever becomes
   // non-idempotent).

   alloc_surface();

#ifdef NEED_DEINTERLACER
   PrevInterlaced = false;
   Deinterlacer_ClearState(&deint);
#endif

   input_init();

   disc_select(disk_get_image_index());

   frame_count = 0;

   // Reset geometry trackers so the first SET_GEOMETRY of the new game
   // always fires; otherwise leftover values from a previous game could
   // accidentally match and the frontend would keep using the wrong
   // base width/height from retro_get_system_av_info.
   cur_width = 0;
   cur_height = 0;
   game_width = 0;
   game_height = 0;

   struct retro_core_option_display option_display;
   option_display.visible = false;
   if (is_pal)
   {
      option_display.key = "beetle_saturn_initial_scanline";
      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY, &option_display);
      option_display.key = "beetle_saturn_last_scanline";
      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY, &option_display);
   }
   else
   {
      option_display.key = "beetle_saturn_initial_scanline_pal";
      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY, &option_display);
      option_display.key = "beetle_saturn_last_scanline_pal";
      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY, &option_display);
   }

   return true;
}

void retro_unload_game(void)
{
   if(!MDFNGameInfo)
      return;

   MDFN_FlushGameCheats();

   CloseGame();

   MDFNMP_Kill();

   MDFNGameInfo = NULL;

   disc_cleanup();

   retro_cd_base_directory[0] = '\0';
   retro_cd_path[0]           = '\0';
   retro_cd_base_name[0]      = '\0';

   // The save-state size cache is keyed on the loaded cart config
   // (cart NV layout, multitap setup, etc.). Different games can yield
   // different sizes, so invalidate on unload to force a fresh
   // measurement on the next load.
   extern size_t serialize_size;
   serialize_size = 0;
}

// current_frame_is_sim is set at the top of retro_run() to indicate
// whether this retro_run call is a run-ahead simulation pass (frame
// the frontend will throw away) vs a real visible frame. MidSync
// consults this to decide whether to re-poll input mid-frame -- see
// MDFN_MidSync below for the full rationale.
static bool current_frame_is_sim = false;

void retro_run(void)
{
   bool updated = false;
   bool hires_h_mode;
   unsigned overscan_mask;
   unsigned linevisfirst, linevislast;
   // width/height/game_width/game_height are file-scope statics now;
   // see comment near their declaration. The previous function-local
   // statics persisted across retro_unload_game / retro_load_game and
   // could prevent a needed SET_GEOMETRY when a new game happened to
   // start with the same dimensions as the previous one.

   // Decide once, up front, whether this retro_run is a run-ahead
   // simulation pass (video disabled => the frame will be discarded).
   // The decision is used in two places: by MDFN_MidSync to decide
   // whether to re-poll input mid-frame (skip on sim frames so the
   // simulated input timeline matches the eventual visible frame's
   // timeline), and by the BRAM / cart-NV flush block at the bottom
   // of this function to skip disk writes during simulation.
   {
      int av_enable = 0x3; /* default: video + audio enabled */
      if (environ_cb(RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, &av_enable))
         current_frame_is_sim = !(av_enable & 1);
      else
         current_frame_is_sim = false;
   }

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      check_variables(false);

   linevisfirst = is_pal ? first_sl_pal : first_sl;
   linevislast  = is_pal ? last_sl_pal : last_sl;

   // Keep the counters at 0 so that they don't display a bogus
   // value if this option is enabled later on
   frame_count = 0;

   input_poll_cb();

   if (libretro_supports_bitmasks)
      input_update_with_bitmasks(input_state_cb);
   else
      input_update(input_state_cb);

   // rects is 2.3KB; previously declared 'static' for no good reason
   // (it's fully written by Emulate each frame). Plain stack-local now.
   int32_t rects[MEDNAFEN_CORE_GEOMETRY_MAX_H];
   rects[0] = ~0;

   /* Was `EmulateSpecStruct spec;` relying on the C++ default
    * member initializers in git.h's struct EmulateSpecStruct.  After
    * the factor-out to the C-compat mednafen/emuspec.h those
    * defaults are gone; this explicit zero-init produces the same
    * semantics: surface/LineWidths NULL, InterlaceOn/Field/skip
    * false, sizes/cycles 0. */
   EmulateSpecStruct spec = {0};
   spec.surface = surf;
   spec.LineWidths = rects;
   spec.SoundBufSize = 0;

   EmulateSpecStruct *espec = (EmulateSpecStruct*)&spec;

   Emulate(espec);

#ifdef NEED_DEINTERLACER
   if (spec.InterlaceOn)
   {
      if (!PrevInterlaced)
         Deinterlacer_ClearState(&deint);

      Deinterlacer_Process(&deint, spec.surface, &spec.DisplayRect, spec.LineWidths, spec.InterlaceField);

      // PrevInterlaced governs whether the libretro output presents at
      // full interlaced height (true) or half-height (false). WEAVE,
      // BOB_OFFSET, FASTMAD, and OFF all produce a full-height surface;
      // only BOB compacts to half-height.
      {
         const unsigned dt = Deinterlacer_GetType(&deint);
         PrevInterlaced = (dt == DEINT_WEAVE
                        || dt == DEINT_BOB_OFFSET
                        || dt == DEINT_FASTMAD
                        || dt == DEINT_OFF);
      }

      spec.InterlaceOn = false;
      spec.InterlaceField = 0;
   }
   else
      PrevInterlaced = false;

#endif
   const void *fb      = NULL;
   const uint32_t *pix = surf->pixels;
   size_t pitch        = FB_WIDTH * sizeof(uint32_t);

   hires_h_mode  = (rects[0] == 704) ? true : false;
   overscan_mask = (h_mask >> 1) << hires_h_mode;
   cur_width     = rects[0] - (h_mask << hires_h_mode);
   cur_height    = (linevislast + 1 - linevisfirst) << PrevInterlaced;

   if (cur_width != game_width || cur_height != game_height)
   {
      struct retro_system_av_info av_info;

      // Change frontend resolution using base width/height (+ overscan adjustments).
      // This avoids inconsistent frame scales when game switches between interlaced and non-interlaced modes.
      av_info.geometry.base_width    = 352 - h_mask;
      av_info.geometry.base_height   = linevislast + 1 - linevisfirst;
      av_info.geometry.max_width     = MEDNAFEN_CORE_GEOMETRY_MAX_W;
      av_info.geometry.max_height    = MEDNAFEN_CORE_GEOMETRY_MAX_H;
      av_info.geometry.aspect_ratio  = 352.0f / ((is_pal) ? 256.0f : 240.0f);
      av_info.geometry.aspect_ratio *= 6.0f / 7.0f;

      /* Corrections for croppings */
      av_info.geometry.aspect_ratio *= ((is_pal) ? 288.0f : 240.0f) / av_info.geometry.base_height;
      av_info.geometry.aspect_ratio /= 352.0f / (352 - h_mask);

      environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);

      game_width  = cur_width;
      game_height = cur_height;

      input_set_geometry(cur_width, cur_height);
   }

   pix += surf->pitchinpix * (linevisfirst << PrevInterlaced) + overscan_mask;

   fb = pix;

   video_cb(fb, game_width, game_height, pitch);
   audio_batch_cb((int16_t*)&IBuffer, spec.SoundBufSize);

   //
   // Periodic Backup-RAM / cart-NV flush, moved out of Emulate().
   //
   // Previously SaveBackupRAM / SaveCartNV were called from inside
   // Emulate() under a master-cycle countdown. That made run-ahead /
   // rewind / netplay fire spurious disk writes on every simulated
   // frame. The replacement: maintain dirty flags inside Emulate(),
   // and flush from here -- once per real retro_run, skipped during
   // run-ahead simulation passes (current_frame_is_sim was set at the
   // top of this function from RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE
   // bit 0). The 3-second batching delay is preserved via a frame
   // counter; ~180 frames at NTSC's 59.76 Hz and PAL's 49.92 Hz both
   // round to roughly 3 seconds, close enough for save batching.
   //
   {
      static unsigned bram_save_counter = 0;
      static unsigned cart_save_counter = 0;
      static const unsigned SAVE_INTERVAL_FRAMES = 180;
      static const unsigned RETRY_INTERVAL_FRAMES = 60; /* used as backoff after a failed write */

      if (!current_frame_is_sim)
      {
         if (BackupRAM_Dirty)
         {
            if (++bram_save_counter >= SAVE_INTERVAL_FRAMES)
            {
               if (SS_FlushBackupRAM())
               {
                  BackupRAM_Dirty = false;
                  bram_save_counter = 0;
               }
               else
               {
                  /* retry sooner; clamp counter so we don't write every frame */
                  bram_save_counter = SAVE_INTERVAL_FRAMES - RETRY_INTERVAL_FRAMES;
               }
            }
         }
         else
         {
            bram_save_counter = 0;
         }

         if (CartNV_Dirty)
         {
            if (++cart_save_counter >= SAVE_INTERVAL_FRAMES)
            {
               if (SS_FlushCartNV())
               {
                  CartNV_Dirty = false;
                  cart_save_counter = 0;
               }
               else
               {
                  cart_save_counter = SAVE_INTERVAL_FRAMES - RETRY_INTERVAL_FRAMES;
               }
            }
         }
         else
         {
            cart_save_counter = 0;
         }
      }
   }

   /* LED interface */
   if (led_state_cb)
      retro_led_interface();
}

void retro_get_system_info(struct retro_system_info *info)
{
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
   memset(info, 0, sizeof(*info));
   info->library_name     = MEDNAFEN_CORE_NAME;
   info->library_version  = MEDNAFEN_CORE_VERSION GIT_VERSION;
   info->need_fullpath    = true;
   info->valid_extensions = MEDNAFEN_CORE_EXTENSIONS;
   /* block_extract: tell the frontend NOT to auto-extract .zip
    * content -- our libretro.c handles ST-V .zip directly via the
    * in-tree zip_reader (see prepare_stv_zip_content above).
    *
    * With block_extract = false, RetroArch's content layer sees the
    * .zip, decides "core requires uncompressed content", and tries
    * to extract by matching files inside against valid_extensions.
    * MAME-style ST-V ROM names (fpr18914.13, mpr18915.1, ...) don't
    * match any of the cue/ccd/chd/toc/m3u extensions, so RetroArch
    * fails with "Failed to extract content from compressed file"
    * before retro_load_game is called.  block_extract = true bypasses
    * that probe entirely: the .zip path arrives at retro_load_game
    * intact, our zip handler identifies the game via STVGI[], and
    * extracts to the per-archive cache dir under retro_save_directory.
    *
    * Bare-file content (.cue/.chd/etc. -- non-zip) is unaffected:
    * block_extract only governs archive handling, and non-archives
    * are passed through to retro_load_game with both flag settings. */
   info->block_extract    = true;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   const bool pal_region = (retro_get_region() == RETRO_REGION_PAL);

   memset(info, 0, sizeof(*info));
   info->timing.sample_rate    = 44100;

   // Report the same base geometry that retro_run's SET_GEOMETRY will
   // converge on, instead of the old 320x240. The frontend uses these
   // initial values for window sizing, aspect ratio, the first
   // screenshot, etc., before SET_GEOMETRY is observed. Matching them
   // to retro_run's runtime values avoids one-frame flashes of wrong
   // aspect / scale at game start.
   info->geometry.base_width   = 352 - h_mask;
   info->geometry.base_height  = pal_region
      ? (last_sl_pal + 1 - first_sl_pal)
      : (last_sl     + 1 - first_sl);
   info->geometry.max_width    = MEDNAFEN_CORE_GEOMETRY_MAX_W;
   info->geometry.max_height   = MEDNAFEN_CORE_GEOMETRY_MAX_H;
   {
      float ar = 352.0f / (pal_region ? 256.0f : 240.0f);
      ar *= 6.0f / 7.0f;
      ar *= (pal_region ? 288.0f : 240.0f) / (float)info->geometry.base_height;
      ar /= 352.0f / (352 - h_mask);
      info->geometry.aspect_ratio = ar;
   }

   if (pal_region)
      info->timing.fps            = 49.92012779552716;
   else
   {
      /* NTSC refresh derived from MAME's saturn.cpp / stv.cpp screen
       * configuration: set_raw(MASTER_CLOCK_320/8, 427, 0, ..., 263, 0, 224),
       * where MASTER_CLOCK_320 = 53,693,174 Hz (15x NTSC color subcarrier,
       * the real-world crystal value Sega used). That gives
       *   53693174 / 8 / (427 * 263) = 59.764800 Hz (== MAME's reported
       *   59.764802 Hz at saturnjp and every stv romset).
       *
       * Previously hardcoded as 59.82650314089141, which was derived from
       * beetle's own internal 352-dot-mode crystal switch (28.636 MHz
       * CPU clock / 478660 timestamps per frame). That value is strictly
       * more hardware-accurate for 352-dot mode (real Saturn does switch
       * to MASTER_CLOCK_352 in 352 mode, MAME does not), but every
       * cross-reference (MAME, arcadeitalia / mdk.cab, datomatic) lists
       * 59.764802, and the frontend / display-sync use cases (VRR target,
       * RetroArch's set_video_refresh, sub-frame audio resampling) all
       * benefit more from agreement with the established reference than
       * from the 352-mode internal-crystal precision.
       *
       * The internal emulation timing (HTimings/VTimings, cur_clock_div
       * 61/65 selection, ts_freq -> sample-clock ratio) is unchanged.
       * The 0.10% gap between the reported rate and beetle's actual
       * 352-mode internal rate means the frontend will believe one frame
       * is 16.730 ms while the emulator produces 16.7150 ms of game time
       * worth of audio per frame; the libretro audio_batch_cb path
       * absorbs this as a constant ~0.1% audio-clock skew, well below
       * any audible pitch shift threshold and identical to what every
       * other Saturn emulator that reports MAME's value already has. */
      info->timing.fps            = 53693174.0 / 8.0 / (427.0 * 263.0);
   }
}

void retro_deinit(void)
{
   MDFN_Surface_Delete(surf);
   surf = NULL;

#ifdef NEED_DEINTERLACER
   Deinterlacer_Cleanup(&deint);
#endif

   libretro_supports_bitmasks          = false;
}

unsigned retro_get_region(void)
{
   return (is_pal) ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_environment(retro_environment_t cb)
{
   struct retro_vfs_interface_info vfs_iface_info;
   struct retro_led_interface led_interface;
   /* libretro_set_core_options early-returns on NULL out-arg, so we
    * pass a function-local instead of a real out-arg.  The boolean
    * it writes (frontend supports v2 categorised core options) is
    * not consumed by anything in this fork. */
   bool option_categories_supported = false;
   environ_cb = cb;

   libretro_set_core_options(environ_cb,
           &option_categories_supported);

   vfs_iface_info.required_interface_version = 2;
   vfs_iface_info.iface                      = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_iface_info))
      filestream_vfs_init(&vfs_iface_info);

   if(environ_cb(RETRO_ENVIRONMENT_GET_LED_INTERFACE, &led_interface))
   if (led_interface.set_led_state && !led_state_cb)
      led_state_cb = led_interface.set_led_state;

   input_set_env(cb);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

// serialize_size is cached at file scope (not function-static or static
// file-scope) so retro_unload_game can invalidate it across games.
size_t serialize_size = 0;

size_t retro_serialize_size(void)
{
   // Don't know yet?
   if (serialize_size == 0)
   {
      // Do a fake save to see.
      StateMem st;

      st.data_frontend  = NULL;
      st.data           = NULL;
      st.loc            = 0;
      st.len            = 0;
      st.malloced       = 0;
      st.initial_malloc = 0;

      if (MDFNSS_SaveSM(&st, MEDNAFEN_CORE_VERSION_NUMERIC))
      {
         // Cache and tidy up.
         serialize_size = st.len;
         if (st.data)
            free(st.data);
      }
   }

   // Return cached value.
   return serialize_size;
}

bool retro_serialize(void *data, size_t size)
{
   StateMem st;
   bool ret          = false;

   st.data_frontend  = (uint8_t*)data;
   st.data           = st.data_frontend;
   st.loc            = 0;
   st.len            = 0;
   st.malloced       = size;
   st.initial_malloc = 0;

   /* MDFNSS_SaveSM will malloc separate memory for st.data to complete
    * the save if the passed-in size is too small */
   ret               = MDFNSS_SaveSM(&st, MEDNAFEN_CORE_VERSION_NUMERIC);

   if (st.len != size)
   {
      log_cb(RETRO_LOG_WARN, "Save state size has changed.\n");

      if (st.data != st.data_frontend)
      {
         free(st.data);
         serialize_size = st.len;
         ret            = false;
      }
   }

   return ret;
}

bool retro_unserialize(const void *data, size_t size)
{
   StateMem st;

   st.data_frontend  = (uint8_t*)data;
   st.data           = st.data_frontend;
   st.loc            = 0;
   st.len            = size;
   st.malloced       = 0;
   st.initial_malloc = 0;

   return MDFNSS_LoadSM(&st, MEDNAFEN_CORE_VERSION_NUMERIC);
}

void *retro_get_memory_data(unsigned type)
{
   switch (type & RETRO_MEMORY_MASK)
   {
      case RETRO_MEMORY_SYSTEM_RAM:
         return WorkRAM;
      // Exposing internal Backup RAM via RETRO_MEMORY_SAVE_RAM lets the
      // frontend manage save persistence (.srm) instead of (or in
      // addition to) the core's own .bkr file. Critically, this allows
      // libretro features like cloud saves, achievement-save-protect,
      // and run-ahead to work correctly: the periodic flush in
      // retro_run still writes the .bkr for backward compatibility,
      // but the frontend now has its own view of the same memory.
      case RETRO_MEMORY_SAVE_RAM:
         return BackupRAM;
   }

   // not supported
   return NULL;
}

size_t retro_get_memory_size(unsigned type)
{
   switch (type & RETRO_MEMORY_MASK)
   {
      case RETRO_MEMORY_SYSTEM_RAM:
         return sizeof(WorkRAM);
      case RETRO_MEMORY_SAVE_RAM:
         return sizeof(BackupRAM);
   }

   // not supported
   return 0;
}

/* Maximum number of write operations that can be packed into a
 * single libretro slot via '+'-joined multi-line codes.  Saturn
 * databases occasionally chain ~16 ops onto one cheat; 32 is
 * comfortable headroom without making the on-stack buffer huge. */
#define SS_CHEAT_OPS_MAX 32

/* Lex two hex fields (the address part and the value part) separated
 * by space, tab, or colon (any combination, repeated).  Leading and
 * trailing whitespace are tolerated.  Anything trailing the value
 * field other than whitespace causes a failure.  On success:
 *   *addr_out / *val_out  - the two 64-bit accumulators
 *   *addr_n / *val_n      - number of hex digits actually consumed
 *                           in each field (used by the caller to
 *                           disambiguate PAR-type-prefix codes vs
 *                           raw addresses).
 * Both fields cap at 8 hex digits; longer fields are rejected. */
static bool ss_cheat_lex(const char *s,
                         uint64_t *addr_out, int *addr_n,
                         uint64_t *val_out,  int *val_n)
{
   const char *p = s;
   uint64_t acc;
   int n;

   if (!p)
      return false;

   while (*p == ' ' || *p == '\t')
      p++;

   acc = 0; n = 0;
   while (*p)
   {
      int d;
      if      (*p >= '0' && *p <= '9') d = *p - '0';
      else if (*p >= 'a' && *p <= 'f') d = *p - 'a' + 10;
      else if (*p >= 'A' && *p <= 'F') d = *p - 'A' + 10;
      else break;
      if (++n > 8) return false;
      acc = (acc << 4) | (uint64_t)d;
      p++;
   }
   if (n == 0)
      return false;
   *addr_out = acc;
   *addr_n   = n;

   if (*p != ' ' && *p != '\t' && *p != ':')
      return false;
   while (*p == ' ' || *p == '\t' || *p == ':')
      p++;

   acc = 0; n = 0;
   while (*p)
   {
      int d;
      if      (*p >= '0' && *p <= '9') d = *p - '0';
      else if (*p >= 'a' && *p <= 'f') d = *p - 'a' + 10;
      else if (*p >= 'A' && *p <= 'F') d = *p - 'A' + 10;
      else break;
      if (++n > 8) return false;
      acc = (acc << 4) | (uint64_t)d;
      p++;
   }
   if (n == 0)
      return false;
   *val_out = acc;
   *val_n   = n;

   while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
      p++;
   if (*p != '\0')
      return false;

   return true;
}

/* Decode one cheat sub-code (no '+' inside) into a write operation.
 *
 * Two accepted forms, distinguished by inspecting the address field:
 *
 *   1. Saturn Pro Action Replay (PAR) type-prefix
 *      ----------------------------------------------
 *      "TTAAAAAA VVVV[VVVV]"   - 8-hex address whose high byte
 *                                TT is a known PAR type encoding
 *                                width + workram region:
 *
 *        0x14 -> 8-bit  write at 0x06000000 + AAAAAA  (WorkRAM High)
 *        0x16 -> 16-bit write at 0x06000000 + AAAAAA
 *        0x18 -> 32-bit write at 0x06000000 + AAAAAA
 *        0x34 -> 8-bit  write at 0x00200000 + AAAAAA  (WorkRAM Low)
 *        0x36 -> 16-bit write at 0x00200000 + AAAAAA
 *        0x38 -> 32-bit write at 0x00200000 + AAAAAA
 *
 *      Bit 5 of TT selects the region (set -> Low at 0x00200000,
 *      clear -> High at 0x06000000); bits 1-2 encode the byte width
 *      (1/2/3 -> 1/2/4 bytes).  Other PAR type bytes (master codes,
 *      conditionals, ROM patches, find-and-replace, disable-when,
 *      etc.) are not supported and fall through to raw mode, which
 *      will then reject them as out-of-range raw addresses.
 *
 *      For 8-bit writes the value field is masked to the low byte
 *      (PAR databases write 8-bit codes both as "00YY" and as
 *      "YY"; either parses identically here).
 *
 *   2. Raw 32-bit address
 *      ----------------------------------------------
 *      "AAAAAAAA VVVV"        - up to 8-hex address (any value,
 *                                so long as it fits in 32 bits)
 *                                and up to 4-hex 16-bit value.
 *      The 16-bit width is the default for the raw path; users
 *      who want 8-bit or 32-bit access should use the PAR form.
 *
 * Saturn cheats are big-endian by community convention; that flag
 * is set unconditionally on the output op. */
static bool ss_cheat_parse_one(const char *s, MDFNCheatOp *out)
{
   uint64_t a64, v64;
   int      a_n,  v_n;

   if (!ss_cheat_lex(s, &a64, &a_n, &v64, &v_n))
      return false;

   /* PAR type-prefix: address must be a full 8 hex digits so the
    * high byte sits where the type code is expected. */
   if (a_n == 8)
   {
      uint8_t  tt   = (uint8_t)(a64 >> 24);
      uint32_t aoff = (uint32_t)(a64 & 0xFFFFFFu);
      uint32_t base = 0;
      unsigned len  = 0;

      switch (tt)
      {
         case 0x14: base = 0x06000000u; len = 1; break;
         case 0x16: base = 0x06000000u; len = 2; break;
         case 0x18: base = 0x06000000u; len = 4; break;
         case 0x34: base = 0x00200000u; len = 1; break;
         case 0x36: base = 0x00200000u; len = 2; break;
         case 0x38: base = 0x00200000u; len = 4; break;
         default:   break;
      }

      if (len != 0)
      {
         if (len == 1)
            v64 &= 0xFFu;
         else if (len == 2 && v64 > 0xFFFFull)
            return false;
         else if (len == 4 && v64 > 0xFFFFFFFFull)
            return false;

         out->addr      = base + aoff;
         out->val       = v64;
         out->length    = len;
         out->bigendian = true;
         return true;
      }
   }

   /* Raw 32-bit address + 16-bit value fallback. */
   if (a64 > 0xFFFFFFFFull)
      return false;
   if (v64 > 0xFFFFull)
      return false;
   out->addr      = (uint32_t)a64;
   out->val       = v64;
   out->length    = 2;
   out->bigendian = true;
   return true;
}

/* Split `code` on '+' separators, parse each sub-code, accumulate
 * into ops[].  Whitespace surrounding the '+' is tolerated, as are
 * trailing '+' (which produce an empty trailing segment that's
 * silently skipped).  Returns the number of ops parsed (>= 1) on
 * success, or -1 if any sub-code fails to parse or the op count
 * would exceed ops_max. */
static int ss_cheat_parse(const char *code,
                          MDFNCheatOp *ops, size_t ops_max)
{
   size_t count = 0;
   const char *p = code;

   if (!p)
      return -1;

   while (*p)
   {
      char        scratch[64];
      const char *q = p;
      size_t      len;
      const char *trim;

      /* Run to the next '+' or end of string. */
      while (*q && *q != '+')
         q++;

      len = (size_t)(q - p);
      if (len >= sizeof(scratch))
         return -1;
      memcpy(scratch, p, len);
      scratch[len] = '\0';

      /* Tolerate whitespace-only / empty segments (e.g. "a + + b"
       * or a trailing '+').  The lexer would reject them as
       * malformed, so screen them out here. */
      trim = scratch;
      while (*trim == ' ' || *trim == '\t')
         trim++;
      if (*trim != '\0')
      {
         if (count >= ops_max)
            return -1;
         if (!ss_cheat_parse_one(scratch, &ops[count]))
            return -1;
         count++;
      }

      p = (*q == '+') ? q + 1 : q;
   }

   if (count == 0)
      return -1;
   return (int)count;
}

void retro_cheat_reset(void)
{
   /* Flush frees all per-cheat strings and resets cheats_count to 0;
    * the subsequent RecomputeCheatsActive call inside Flush flips
    * CheatsActive back to false. */
   MDFN_FlushGameCheats();
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   MDFNCheatOp ops[SS_CHEAT_OPS_MAX];
   int n = ss_cheat_parse(code, ops, SS_CHEAT_OPS_MAX);

   if (n < 0)
   {
      if (log_cb)
         log_cb(RETRO_LOG_WARN,
                "[Cheats] Ignoring malformed code at slot %u: '%s'\n"
                "[Cheats] Accepted forms (sub-codes joined with '+'):\n"
                "[Cheats]   TTAAAAAA VVVV         PAR type-prefix\n"
                "[Cheats]     TT = 0x14/16/18 -> 8/16/32-bit @ 0x06000000+AAAAAA\n"
                "[Cheats]     TT = 0x34/36/38 -> 8/16/32-bit @ 0x00200000+AAAAAA\n"
                "[Cheats]   AAAAAAAA VVVV         raw 32-bit addr, 16-bit val\n",
                index, code ? code : "(null)");
      /* Drop any prior occupants of the slot so the frontend's view
       * of "slot is set" matches our view of "slot is empty". */
      MDFNMP_SetCheat(index, false, NULL, 0);
      return;
   }

   /* MDFNMP_SetCheat handles slot bookkeeping: it compact-removes any
    * prior entries that belonged to this libretro slot and then
    * appends the n new ones, all tagged with frontend_slot == index.
    * RecomputeCheatsActive runs at the tail so the per-frame apply
    * path picks up the new state on the next VBlank-In. */
   MDFNMP_SetCheat(index, enabled, ops, (size_t)n);
}

// Use a simpler approach to make sure that things go right for libretro.
// Caller-allocated buffer rather than a function-static return area.
// The previous signature returned a pointer to a 4KB function-static
// buffer that was clobbered by every subsequent call from any thread.
// All current callers used the result immediately so the bug was
// latent, but it was the kind of footgun that bites the moment a
// future caller stashes the pointer or another thread calls in
// between. Caller-allocated buffer eliminates the class of bug; we
// return the buf parameter so the call still flows into a single
// expression (`cdstream_open_write(&f, MDFN_MakeFName(buf, sizeof buf, ...))`).
//
// On unknown MakeFName_Type, log and return an empty string in buf.
char *MDFN_MakeFName(char *buf, size_t buflen, MakeFName_Type type, int id1, const char *cd1)
{
   if (buflen)
      buf[0] = '\0';

   switch (type)
   {
      case MDFNMKF_SAV:
         snprintf(buf, buflen, "%s" RETRO_SLASH "%s.%s",
               retro_save_directory,
               (!shared_intmemory) ? retro_cd_base_name : "mednafen_saturn_libretro_shared",
               cd1);
         break;
      case MDFNMKF_CART:
         snprintf(buf, buflen, "%s" RETRO_SLASH "%s.%s",
               retro_save_directory,
               (!shared_backup) ? retro_cd_base_name : "mednafen_saturn_libretro_shared",
               cd1);
         break;
      case MDFNMKF_FIRMWARE:
         snprintf(buf, buflen, "%s" RETRO_SLASH "%s", retro_base_directory, cd1);
         break;
      default:
         log_cb(RETRO_LOG_WARN, "MDFN_MakeFName called with unknown type %d\n", (int)type);
         break;
   }

   return buf;
}

void MDFN_MidSync(void)
{
   // Mid-frame input re-poll. This is what hooks the freshest available
   // user input into the SMPC cache right before the emulated VBLANK
   // (where most Saturn games run their INTBACK), shaving roughly half
   // a frame off input-to-response latency.
   //
   // Mid-frame polling is, however, fundamentally at odds with features
   // that re-execute frames to compute input-to-pixel paths -- run-ahead,
   // rewind, netplay rollback, movie replay. Those features all assume
   // a given retro_run() produces the same output for the same inputs;
   // a fresh poll mid-frame can return different OS-side input state on
   // the simulation pass versus the visible pass, breaking that
   // assumption.
   //
   // Resolution: skip the re-poll on simulation frames (video disabled,
   // detected via RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE bit 0 at the
   // top of retro_run). Visible frames still get the latency reduction;
   // run-ahead simulation frames see exactly the same input timeline
   // the visible frame will, so input determinism holds.
   if (current_frame_is_sim)
      return;

   input_poll_cb();
   if (libretro_supports_bitmasks)
      input_update_with_bitmasks(input_state_cb);
   else
      input_update(input_state_cb);
}
