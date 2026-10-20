/* Fixture-driven CHD/ZIP and mapped/short-read frontend I/O checks. */
#include "../mednafen/cdrom/CDAccess_CHD.c"
#include "../zip_reader.h"
#include "../mednafen/cdstream.h"
#include <stdarg.h>
#ifndef _WIN32
#define _ftelli64 ftello
#define _fseeki64 fseeko
#endif
static void test_log(enum retro_log_level level, const char *fmt, ...)
{ (void)level; (void)fmt; }
static bool short_reads;
/* One-shot seek failure that leaves the handle at EOF, as a frontend
 * VFS may do when it fails partway through repositioning. */
static bool fail_next_seek;
static struct retro_vfs_file_handle *vopen(const char *p, unsigned mode, unsigned hints)
{ (void)mode; (void)hints; return (struct retro_vfs_file_handle*)fopen(p, "rb"); }
static int vclose(struct retro_vfs_file_handle *h) { return fclose((FILE*)h); }
static int64_t vtell(struct retro_vfs_file_handle *h) { return _ftelli64((FILE*)h); }
static int64_t vseek(struct retro_vfs_file_handle *h, int64_t off, int whence)
{
   if (fail_next_seek)
   {
      fail_next_seek = false;
      _fseeki64((FILE*)h, 0, SEEK_END);
      return -1;
   }
   return _fseeki64((FILE*)h, off, whence) ? -1 : 0;
}
static bool sector_ok(CDAccess *cd, uint8_t *buf, unsigned frame)
{
   unsigned j;
   if (!cd->Read_Raw_Sector(cd, buf, frame)) return false;
   for (j = 0; j < 2352; j++)
      if (buf[j] != (uint8_t)(frame * 17 + (j ^ 1) * 13)) return false;
   return true;
}
static int64_t vsize(struct retro_vfs_file_handle *h)
{
   int64_t p = vtell(h), n;
   if (p < 0 || vseek(h, 0, SEEK_END) < 0) return -1;
   n = vtell(h);
   return vseek(h, p, SEEK_SET) < 0 ? -1 : n;
}
static int64_t vread(struct retro_vfs_file_handle *h, void *p, uint64_t n)
{ if (short_reads && n > 7) n = 7; return fread(p, 1, (size_t)n, (FILE*)h); }
static void frontend_vfs(void)
{
   static struct retro_vfs_interface api;
   struct retro_vfs_interface_info info;
   memset(&api, 0, sizeof(api));
   api.open = vopen; api.close = vclose; api.size = vsize;
   api.tell = vtell; api.seek = vseek; api.read = vread;
   info.required_interface_version = 2; info.iface = &api;
   filestream_vfs_init(&info);
}
int main(int argc, char **argv)
{
   unsigned mode, i;
   log_cb = test_log;
   if (argc != 3) return 1;
   for (mode = 0; mode < 3; mode++)
   {
      CDAccess *cd;
      CDAccess_CHD *ch;
      uint8_t buf[2448];
      if (mode) { frontend_vfs(); short_reads = true; }
      {
         cdstream stream;
         uint8_t header[16];
         if (!cdstream_open(&stream, argv[1])) return 13;
         if (cdstream_read(&stream, header, sizeof(header)) != sizeof(header)
               || memcmp(header, "MComprHD", 8)) return 14;
         cdstream_seek(&stream, 0, SEEK_SET);
         if (cdstream_tell(&stream) != 0) return 15;
         cdstream_seek(&stream, -8, SEEK_END);
         if (cdstream_read(&stream, header, sizeof(header)) != 8
               || cdstream_read(&stream, header, 1) != 0) return 16;
         cdstream_close(&stream);
      }
      cd = CDAccess_CHD_New(argv[1], mode == 2);
      if (!cd) { printf("CHD open failed: %s mode %u\n", argv[1], mode); return 2; }
      ch = (CDAccess_CHD*)cd;
      if (mode && ch->mapped) return 3;
      if (mode == 2 && !ch->precache) return 4;
      if (mode == 1)
      {
         /* Hunk 0 leaves the stream where hunk 1's data starts. A failed
          * seek for hunk 2 moves the handle, so hunk 1 must re-seek
          * rather than trust the position it last recorded. */
         if (!sector_ok(cd, buf, 0)) return 17;
         fail_next_seek = true;
         if (cd->Read_Raw_Sector(cd, buf, 16)) return 18;
         if (fail_next_seek) return 19;
         if (!sector_ok(cd, buf, 8))
         { printf("stale position after failed seek: %s\n", argv[1]); return 20; }
      }
      for (i = 0; i < 96; i++)
      {
         unsigned frame = (i * 17) % 32, j;
         if (!cd->Read_Raw_Sector(cd, buf, frame)) return 5;
         for (j = 0; j < 2352; j++)
            if (buf[j] != (uint8_t)(frame * 17 + (j ^ 1) * 13))
            { printf("bad sector frame=%u byte=%u mode=%u\n", frame, j, mode); return 6; }
      }
      cd->destroy(cd);
      {
         zip_archive za;
         if (!zip_open(&za, argv[2])) return 7;
         for (i = 0; i < za.entry_count; i++)
         {
            const struct zip_entry *e = &za.entries[i];
            uint8_t *buf = malloc((size_t)e->uncompressed_size + 1);
            if (!zip_extract(&za, e, buf))
            { printf("ZIP extract failed: %s\n", e->name); return 8; }
            if (e->uncompressed_size)
            {
               unsigned j, seed = 12345;
               for (j = 0; j < e->uncompressed_size; j++)
               {
                  uint8_t expected = (uint8_t)((j * 13) ^ (j >> 7));
                  if (!strcmp(e->name, "random.bin"))
                  {
                     seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
                     expected = (uint8_t)seed;
                  }
                  if (buf[j] != expected) return 9;
               }
            }
            /* Wrong CRC, output length, and truncated DEFLATE must fail. */
            {
               struct zip_entry bad = *e;
               bad.crc32 ^= 1;
               if (zip_extract(&za, &bad, buf)) return 10;
               if (e->method == 8 && e->compressed_size)
               {
                  bad = *e;
                  bad.compressed_size--;
                  if (zip_extract(&za, &bad, buf)) return 11;
                  bad = *e;
                  bad.uncompressed_size++;
                  if (zip_extract(&za, &bad, buf)) return 12;
               }
            }
            free(buf);
         }
         zip_close(&za);
      }

   }
   printf("I/O PASS: %s; local/mapped, 7-byte frontend reads, precache, ZIP CRCs\n", argv[1]);
   return 0;
}
