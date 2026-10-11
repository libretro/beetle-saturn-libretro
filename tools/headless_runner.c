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
**   RUNNER_FRAME_HASH=1     print an FNV-1a hash of every pushed frame's
**                           pixels (a dupe prints "dupe"), to compare the
**                           picture between configurations.
**   RUNNER_LEND=N           answer GET_CURRENT_SOFTWARE_FRAMEBUFFER on
**                           every Nth frame with a buffer the exact size
**                           asked for, inside canary bands, and check
**                           that the push returns the buffer exactly as
**                           lent (pointer, width, height, pitch) and that
**                           nothing was written outside it. 1 lends
**                           every frame. RUNNER_LEND_PAD=P pads the pitch
**                           by P bytes. A violation is fatal.
**   RUNNER_SET_AT=N:k=v     change core option k to v before frame N (one
**                           change; a geometry change on a lent frame is
**                           the mismatch case).
**   RUNNER_PRESS=N:B        hold joypad button B (RETRO_DEVICE_ID_JOYPAD_*
**                           number) on port 0 for frames N..N+9.
**   RUNNER_VERBOSE=1        pass the core's info-level log through.
**
** Profiling: perf record -F 499 -D 40000 -- ./runner ... 7200 skips the
** BIOS boot and records the attract mode.
*/
#if defined(_WIN32)
 #include <windows.h>
 #define dlopen(p, f)  ((void*)LoadLibraryA(p))
 #define dlsym(h, n)   ((void*)GetProcAddress((HMODULE)(h), (n)))
 #define dlclose(h)    FreeLibrary((HMODULE)(h))
 #define dlerror()     "LoadLibrary failed"
 #define RTLD_NOW 0
 #define RTLD_LOCAL 0
#else
 #include <dlfcn.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "libretro.h"

/* Monotonic clock; MinGW lacks clock_gettime, clock() is fine at these lengths. */
static void now_mono(struct timespec* ts)
{
#if defined(_WIN32)
 clock_t c = clock(); ts->tv_sec = c / CLOCKS_PER_SEC; ts->tv_nsec = (long)((c % CLOCKS_PER_SEC) * (1000000000L / CLOCKS_PER_SEC));
#else
 clock_gettime(CLOCK_MONOTONIC, ts);
#endif
}

static const char* sysdir = "/home/claude/sys";   /* BIOS directory; override with RUNNER_SYSDIR */
static struct { const char* k; const char* v; } vars[32]; static int nvars;
static int  vars_updated;
static long set_at_frame = -1; static char* set_at_k; static char* set_at_v;
static retro_log_printf_t logcb;
static int quiet = 1;

/* RUNNER_LEND: the frame buffer lent this frame, with canary bands on
 * both sides. The push must hand it back untouched outside the window. */
#define LEND_CANARY 4096
static int      lend_every, lend_pad;
static uint8_t* lend_block;      /* canary + buffer + canary */
static size_t   lend_block_cap;
static void*    lend_data;       /* what was lent, NULL when nothing was */
static unsigned lend_w, lend_h;
static size_t   lend_pitch;
static unsigned long frames_run, lend_asked, lend_zero_copy, lend_dupes;

static bool lend_framebuffer(struct retro_framebuffer* fb)
{
 size_t pitch, need, i;
 if(!lend_every || (frames_run % lend_every)) return false;
 if(!fb->width || !fb->height) return false;
 pitch = (size_t)fb->width * 4 + lend_pad;
 need  = pitch * fb->height + 2 * LEND_CANARY;
 if(need > lend_block_cap) { lend_block = realloc(lend_block, need); lend_block_cap = need; }
 for(i = 0; i < need; i++) lend_block[i] = (uint8_t)(0xA5 ^ i);
 lend_data  = lend_block + LEND_CANARY;
 lend_w     = fb->width;
 lend_h     = fb->height;
 lend_pitch = pitch;
 fb->data         = lend_data;
 fb->pitch        = pitch;
 fb->format       = RETRO_PIXEL_FORMAT_XRGB8888;
 fb->memory_flags = RETRO_MEMORY_TYPE_CACHED;
 lend_asked++;
 return true;
}

/* The canaries, and the pitch padding of every row, must still carry the
 * pattern they were lent with. */
static void lend_check_bounds(void)
{
 size_t i, y, base = lend_pitch * lend_h + LEND_CANARY;
 for(i = 0; i < LEND_CANARY; i++)
  if(lend_block[i] != (uint8_t)(0xA5 ^ i) || lend_block[base + i] != (uint8_t)(0xA5 ^ (base + i)))
  { fprintf(stderr, "frame %lu: write outside the lent buffer (%s canary)\n", frames_run, i < LEND_CANARY && lend_block[i] != (uint8_t)(0xA5 ^ i) ? "front" : "back"); exit(3); }
 for(y = 0; y < lend_h; y++)
  for(i = (size_t)lend_w * 4; i < lend_pitch; i++)
  {
   size_t k = LEND_CANARY + y * lend_pitch + i;
   if(lend_block[k] != (uint8_t)(0xA5 ^ k))
   { fprintf(stderr, "frame %lu: write into the pitch padding of row %lu\n", frames_run, (unsigned long)y); exit(3); }
  }
}

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
   *(bool*)data = vars_updated != 0; vars_updated = 0;
   return true;
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
  case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
   return lend_framebuffer((struct retro_framebuffer*)data);
  default:
   return false;
 }
}

static const char* dump_path;
static int frame_hash;
static void video_cb(const void* d, unsigned w, unsigned h, size_t p)
{
 if(lend_data)
 {
  /* A lent buffer comes back exactly as lent, or as a dupe. */
  if(d)
  {
   if(d != lend_data || w != lend_w || h != lend_h || p != lend_pitch)
   { fprintf(stderr, "frame %lu: lent %p %ux%u pitch %lu, pushed %p %ux%u pitch %lu\n", frames_run, lend_data, lend_w, lend_h, (unsigned long)lend_pitch, d, w, h, (unsigned long)p); exit(3); }
   lend_zero_copy++;
  }
  else
   lend_dupes++;
  lend_check_bounds();
  lend_data = NULL;
 }
 if(frame_hash)
 {
  if(!d) printf("frame %6lu dupe\n", frames_run);
  else
  {
   uint64_t fh = 1469598103934665603ull; unsigned y, x;
   for(y = 0; y < h; y++) { const uint32_t* row = (const uint32_t*)((const char*)d + y * p);
    for(x = 0; x < w; x++) { fh ^= row[x] & 0xFFFFFF; fh *= 1099511628211ull; } }
   printf("frame %6lu %ux%u %016llx\n", frames_run, w, h, (unsigned long long)fh);
  }
 }
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
static long press_frame = -1; static unsigned press_id;
static int16_t input_state_cb(unsigned port, unsigned dev, unsigned idx, unsigned id)
{
 (void)dev;(void)idx;
 if(port || press_frame < 0 || (long)frames_run < press_frame || (long)frames_run >= press_frame + 10) return 0;
 if(id == RETRO_DEVICE_ID_JOYPAD_MASK) return (int16_t)(1 << press_id);
 return id == press_id;
}

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
#if defined(_WIN32)
 if(!getenv("RUNNER_SYSDIR")) sysdir = "Z:\\home\\claude\\sys";
#endif
 dump_path = getenv("RUNNER_DUMP");
 frame_hash = getenv("RUNNER_FRAME_HASH") != NULL;
 lend_every = getenv("RUNNER_LEND") ? atoi(getenv("RUNNER_LEND")) : 0;
 lend_pad   = getenv("RUNNER_LEND_PAD") ? atoi(getenv("RUNNER_LEND_PAD")) : 0;
 if(getenv("RUNNER_PRESS")) { press_frame = atol(getenv("RUNNER_PRESS")); press_id = (unsigned)atoi(strchr(getenv("RUNNER_PRESS"), ':') ? strchr(getenv("RUNNER_PRESS"), ':') + 1 : "3"); }
 if(getenv("RUNNER_SET_AT"))
 {
  char* spec = strdup(getenv("RUNNER_SET_AT")); char* colon = strchr(spec, ':'); char* eq = colon ? strchr(colon, '=') : NULL;
  if(colon && eq) { *colon = 0; *eq = 0; set_at_frame = atol(spec); set_at_k = colon + 1; set_at_v = eq + 1; }
 }
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
 now_mono(&t0);
 {
  const char* he = getenv("RUNNER_HASH_EVERY"); int every = he ? atoi(he) : 0;
  void* sbuf = NULL; size_t scap = 0;
  for(i = 0; i < frames; i++)
  {
   if(i == set_at_frame)
   {
    int k, found = 0;
    for(k = 0; k < nvars; k++) if(!strcmp(vars[k].k, set_at_k)) { vars[k].v = set_at_v; found = 1; }
    if(!found && nvars < 32) { vars[nvars].k = set_at_k; vars[nvars].v = set_at_v; nvars++; }
    vars_updated = 1;
   }
   p_retro_run();
   frames_run++;
   if(lend_data) { fprintf(stderr, "frame %lu: lent buffer never pushed\n", frames_run); return 3; }
   if(every && ((i + 1) % every) == 0)
    printf("frame %6d state %016llx\n", i + 1, (unsigned long long)hash_state(p_retro_serialize_size, p_retro_serialize, &sbuf, &scap));
  }
  free(sbuf);
 }
 now_mono(&t1);
 {
  double s = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
  fprintf(stderr, "%d frames in %.2f s = %.1f fps (%.2fx realtime at %.2f fps)\n", frames, s, frames / s, (frames / s) / av.timing.fps, av.timing.fps);
  if(lend_every)
   fprintf(stderr, "lent %lu: %lu pushed zero-copy, %lu duped\n", lend_asked, lend_zero_copy, lend_dupes);
 }
 p_retro_unload_game(); p_retro_deinit(); dlclose(h);
 free(lend_block);
 return 0;
}
