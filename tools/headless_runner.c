/* headless_runner.c - minimal libretro frontend for profiling and bit-exactness checks
**
** Build:  gcc -O2 -I../libretro-common/include headless_runner.c -o runner -ldl
**         gcc -O2 -shared -fPIC faketime.c -o faketime.so
** Run:    ./runner core.so game.cue FRAMES [option=value ...]
**
** Options given as key=value answer RETRO_ENVIRONMENT_GET_VARIABLE, so
** e.g. beetle_saturn_jit_scu=disabled switches a core option.  The BIOS
** is looked up in /home/claude/sys (edit sysdir) as sega_101.bin /
** mpr-17933.bin.  Input is never pressed, so what runs is the BIOS boot
** followed by the game's attract mode, which is deterministic once the
** host clock is pinned: LD_PRELOAD=./faketime.so fixes time() so the
** SMPC auto-RTC is reproducible.
**
** Environment:
**   RUNNER_HASH_EVERY=N     print an FNV-1a hash of retro_serialize()
**                           every N frames -- the whole machine state.
**                           Two configurations that agree on every hash
**                           are bit-exact as far as the emulator knows.
**   RUNNER_HASH_SECTIONS=1  also hash each savestate section by name, to
**                           see which subsystem diverged first.
**   RUNNER_DUMP=file.ppm    write every frame to file.ppm (last one wins).
**   RUNNER_VERBOSE=1        pass the core's info-level log through.
**
** Profiling: perf record -F 499 -D 40000 -- ./runner ... 7200 skips the
** BIOS boot and records the attract mode.
*/
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "libretro.h"

static const char* sysdir = "/home/claude/sys";   /* BIOS directory; override with RUNNER_SYSDIR */
static struct { const char* k; const char* v; } vars[32]; static int nvars;
static retro_log_printf_t logcb;
static int quiet = 1;

static void log_printf(enum retro_log_level lvl, const char* fmt, ...)
{
 va_list ap; if(quiet && lvl < RETRO_LOG_WARN) return;
 va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
}

static bool env_cb(unsigned cmd, void* data)
{
 switch(cmd)
 {
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
  case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
   *(const char**)data = sysdir; return true;
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
   return *(enum retro_pixel_format*)data == RETRO_PIXEL_FORMAT_XRGB8888;
  case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
   ((struct retro_log_callback*)data)->log = log_printf; return true;
  case RETRO_ENVIRONMENT_GET_VARIABLE:
  {
   struct retro_variable* v = (struct retro_variable*)data; int i;
   v->value = NULL;
   for(i = 0; i < nvars; i++) if(!strcmp(vars[i].k, v->key)) { v->value = vars[i].v; return true; }
   return false;
  }
  case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
   *(bool*)data = false; return true;
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
   *(bool*)data = true; return true;
  case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
   *(unsigned*)data = 2; return true;
  case RETRO_ENVIRONMENT_SET_VARIABLES:
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2:
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL:
  case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
  case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
  case RETRO_ENVIRONMENT_SET_GEOMETRY:
  case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
  case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
  case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
  case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
  case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK:
  case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
  case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:
  case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
  case RETRO_ENVIRONMENT_SET_MESSAGE:
  case RETRO_ENVIRONMENT_SET_MESSAGE_EXT:
   return true;
  case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
   return true;
  default:
   return false;
 }
}

static const char* dump_path;
static void video_cb(const void* d, unsigned w, unsigned h, size_t p)
{
 if(!dump_path || !d) return;
 FILE* f = fopen(dump_path, "wb"); unsigned y, x;
 if(!f) return;
 fprintf(f, "P6\n%u %u\n255\n", w, h);
 for(y = 0; y < h; y++) { const uint32_t* row = (const uint32_t*)((const char*)d + y * p);
  for(x = 0; x < w; x++) { unsigned char rgb[3] = { row[x] >> 16, row[x] >> 8, row[x] }; fwrite(rgb, 1, 3, f); } }
 fclose(f);
}
static size_t audio_batch_cb(const int16_t* d, size_t f) { (void)d; return f; }
static void audio_cb(int16_t l, int16_t r) { (void)l;(void)r; }
static void input_poll_cb(void) {}
static int16_t input_state_cb(unsigned port, unsigned dev, unsigned idx, unsigned id) { (void)port;(void)dev;(void)idx;(void)id; return 0; }

#define SYM(name) name##_t p_##name = (name##_t)dlsym(h, #name); if(!p_##name) { fprintf(stderr, "missing %s\n", #name); return 1; }
typedef void (*retro_set_environment_t)(retro_environment_t);
typedef void (*retro_set_video_refresh_t)(retro_video_refresh_t);
typedef void (*retro_set_audio_sample_t)(retro_audio_sample_t);
typedef void (*retro_set_audio_sample_batch_t)(retro_audio_sample_batch_t);
typedef void (*retro_set_input_poll_t)(retro_input_poll_t);
typedef void (*retro_set_input_state_t)(retro_input_state_t);
typedef void (*retro_init_t)(void);
typedef void (*retro_deinit_t)(void);
typedef bool (*retro_load_game_t)(const struct retro_game_info*);
typedef void (*retro_run_t)(void);
typedef void (*retro_unload_game_t)(void);
typedef void (*retro_get_system_av_info_t)(struct retro_system_av_info*);
typedef size_t (*retro_serialize_size_t)(void);
typedef bool (*retro_serialize_t)(void*, size_t);

/* FNV-1a over a savestate: the whole machine, so two runs that agree on
 * every hash agree on everything the emulator considers state. */
static uint64_t hash_state(retro_serialize_size_t ssz, retro_serialize_t ser, void** buf, size_t* cap)
{
 size_t n = ssz(); uint64_t h = 1469598103934665603ull; size_t i;
 if(n > *cap) { *buf = realloc(*buf, n); *cap = n; }
 if(!ser(*buf, n)) return 0;
 if(getenv("RUNNER_HASH_SECTIONS"))
 {
  /* Mednafen savestate: 32-byte header, then sections of [name 32][size u32 LE][data]. */
  const unsigned char* p = (const unsigned char*)*buf; size_t off = 32;
  while(off + 36 <= n)
  {
   char name[33]; uint32_t sz; uint64_t sh = 1469598103934665603ull;
   memcpy(name, p + off, 32); name[32] = 0;
   sz = p[off+32] | (p[off+33] << 8) | (p[off+34] << 16) | ((uint32_t)p[off+35] << 24);
   if(off + 36 + sz > n) break;
   for(i = 0; i < sz; i++) { sh ^= p[off + 36 + i]; sh *= 1099511628211ull; }
   printf("   section %-24s %7u %016llx\n", name, sz, (unsigned long long)sh);
   off += 36 + sz;
  }
 }
 for(i = 0; i < n; i++) { h ^= ((const unsigned char*)*buf)[i]; h *= 1099511628211ull; }
 return h;
}

int main(int argc, char** argv)
{
 void* h; int frames, i; struct retro_game_info gi; struct retro_system_av_info av; struct timespec t0, t1;
 if(argc < 4) { fprintf(stderr, "usage: %s core.so game frames [k=v...]\n", argv[0]); return 1; }
 for(i = 4; i < argc && nvars < 32; i++)
 {
  char* eq = strchr(argv[i], '='); if(!eq) continue;
  *eq = 0; vars[nvars].k = argv[i]; vars[nvars].v = eq + 1; nvars++;
 }
 if(getenv("RUNNER_VERBOSE")) quiet = 0;
 if(getenv("RUNNER_SYSDIR")) sysdir = getenv("RUNNER_SYSDIR");
 dump_path = getenv("RUNNER_DUMP");
 h = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
 if(!h) { fprintf(stderr, "dlopen: %s\n", dlerror()); return 1; }
 SYM(retro_set_environment) SYM(retro_set_video_refresh) SYM(retro_set_audio_sample) SYM(retro_set_audio_sample_batch)
 SYM(retro_set_input_poll) SYM(retro_set_input_state) SYM(retro_init) SYM(retro_deinit) SYM(retro_load_game)
 SYM(retro_run) SYM(retro_unload_game) SYM(retro_get_system_av_info) SYM(retro_serialize_size) SYM(retro_serialize)
 p_retro_set_environment(env_cb);
 p_retro_set_video_refresh(video_cb);
 p_retro_set_audio_sample(audio_cb);
 p_retro_set_audio_sample_batch(audio_batch_cb);
 p_retro_set_input_poll(input_poll_cb);
 p_retro_set_input_state(input_state_cb);
 p_retro_init();
 memset(&gi, 0, sizeof gi); gi.path = argv[2];
 if(!p_retro_load_game(&gi)) { fprintf(stderr, "load_game failed\n"); return 2; }
 p_retro_get_system_av_info(&av);
 frames = atoi(argv[3]);
 clock_gettime(CLOCK_MONOTONIC, &t0);
 {
  const char* he = getenv("RUNNER_HASH_EVERY"); int every = he ? atoi(he) : 0;
  void* sbuf = NULL; size_t scap = 0;
  for(i = 0; i < frames; i++)
  {
   p_retro_run();
   if(every && ((i + 1) % every) == 0)
    printf("frame %6d state %016llx\n", i + 1, (unsigned long long)hash_state(p_retro_serialize_size, p_retro_serialize, &sbuf, &scap));
  }
  free(sbuf);
 }
 clock_gettime(CLOCK_MONOTONIC, &t1);
 {
  double s = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
  fprintf(stderr, "%d frames in %.2f s = %.1f fps (%.2fx realtime at %.2f fps)\n", frames, s, frames / s, (frames / s) / av.timing.fps, av.timing.fps);
 }
 p_retro_unload_game(); p_retro_deinit(); dlclose(h);
 return 0;
}
