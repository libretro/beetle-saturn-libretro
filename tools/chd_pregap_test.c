/* CHD pregaps that are not stored in the image (PGTYPE without 'V').
 *
 * Reads tools/make_io_fixtures.py's pregap/pregap.chd: three tracks of
 * 8 stored frames each, frame f filled with (f * 29 + j * 7 + 1) & 255.
 *   track 1  MODE1_RAW  PREGAP 0                 stored frames 0..7
 *   track 2  MODE2_RAW  PREGAP 3, PGTYPE MODE1   stored frames 8..15
 *   track 3  AUDIO      PREGAP 2, PGTYPE VAUDIO  stored frames 16..23
 *                       (frames 16..17 are its stored pregap)
 * The disc layout below is worked out by hand from that, not from the
 * reader: track 2's 3-sector pregap is on the disc (LBA 8..10) but not in
 * the image, so track 2 starts at LBA 11 on stored frame 8, and every
 * sector from LBA 11 on is stored frame lba - 3. */
#include "../mednafen/cdrom/CDAccess_CHD.c"

static void test_log(enum retro_log_level level, const char *fmt, ...)
{ (void)level; (void)fmt; }

static uint8_t pattern(unsigned frame, unsigned j)
{ return (uint8_t)(frame * 29 + j * 7 + 1); }

/* Data tracks are copied raw; audio is byte-swapped per sample. */
static bool stored_frame(const uint8_t *buf, unsigned frame, bool audio)
{
   unsigned j;
   for (j = 0; j < 2352; j++)
      if (buf[j] != pattern(frame, audio ? (j ^ 1) : j)) return false;
   return true;
}

static uint8_t bcd(unsigned v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); }

int main(int argc, char **argv)
{
   CDAccess *cd;
   TOC toc;
   uint8_t buf[2448];
   int32_t lba;
   unsigned j;

   log_cb = test_log;
   if (argc != 2) return 1;
   cd = CDAccess_CHD_New(argv[1], false);
   if (!cd) { printf("CHD open failed: %s\n", argv[1]); return 2; }

   if (!cd->Read_TOC(cd, &toc)) return 3;
   if (toc.first_track != 1 || toc.last_track != 3) return 4;
   if (toc.tracks[1].lba != 0 || toc.tracks[2].lba != 11 || toc.tracks[3].lba != 21)
   {
      printf("track LBAs %d %d %d, want 0 11 21\n",
            (int)toc.tracks[1].lba, (int)toc.tracks[2].lba, (int)toc.tracks[3].lba);
      return 5;
   }
   if (toc.tracks[100].lba != 27)
   { printf("lead-out LBA %d, want 27\n", (int)toc.tracks[100].lba); return 6; }

   /* Track 1. */
   for (lba = 0; lba < 8; lba++)
      if (!cd->Read_Raw_Sector(cd, buf, lba) || !stored_frame(buf, (unsigned)lba, false))
      { printf("bad sector at LBA %d\n", (int)lba); return 7; }

   /* Track 2's pregap is not in the image: synthesised Mode 2 sectors. */
   for (lba = 8; lba < 11; lba++)
   {
      const unsigned aba = (unsigned)lba + 150;
      if (!cd->Read_Raw_Sector(cd, buf, lba)) return 8;
      if (buf[0] != 0x00 || buf[11] != 0x00) return 9;
      for (j = 1; j < 11; j++)
         if (buf[j] != 0xFF) return 9;
      if (buf[12] != bcd(aba / 75 / 60) || buf[13] != bcd(aba / 75 % 60) ||
          buf[14] != bcd(aba % 75) || buf[15] != 0x02)
      { printf("bad pregap header at LBA %d\n", (int)lba); return 10; }
   }

   /* Track 2 data, then track 3's stored pregap and data. */
   for (lba = 11; lba < 27; lba++)
      if (!cd->Read_Raw_Sector(cd, buf, lba) || !stored_frame(buf, (unsigned)lba - 3, lba >= 19))
      { printf("bad sector at LBA %d\n", (int)lba); return 11; }

   cd->destroy(cd);
   printf("CHD pregap layout OK.\n");
   return 0;
}
