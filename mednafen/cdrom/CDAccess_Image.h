#ifndef __MDFN_CDACCESS_IMAGE_H
#define __MDFN_CDACCESS_IMAGE_H

#include "CDUtility.h"
#include "../cdstream.h"
#include "audioreader.h"

/* Sorted-array SubQ replacement table, replaces the former
 * std::map<uint32_t, stl_array<uint8_t, 12>> SubQReplaceMap.
 * Typical SBI files have a few dozen entries; the cap of 1024 is
 * generous (largest known SBI files in the wild are well under
 * 200).  Insertion order is arbitrary; subq_map_finalize() sorts
 * by aba so subq_map_find() can binary-search.  Same shape as
 * beetle-psx-libretro's subq_map in mednafen/cdrom/cdaccess_track.h. */
#define SUBQ_MAP_MAX 1024

struct subq_map_entry
{
   uint32_t aba;
   uint8_t  data[12];
};

struct subq_map
{
   struct subq_map_entry entries[SUBQ_MAP_MAX];
   unsigned              count;
   bool                  sorted;
};
typedef struct subq_map subq_map;

struct CDRFILE_TRACK_INFO
{
   int32_t LBA;

   uint32_t DIFormat;
   uint8_t subq_control;

   int32_t pregap;
   int32_t pregap_dv;

   int32_t postgap;

   int32_t index[100];

   int32_t sectors;	// Not including pregap sectors!
   cdstream *fp;
   bool FirstFileInstance;
   bool RawAudioMSBFirst;
   long FileOffset;
   unsigned int SubchannelMode;

   uint32_t LastSamplePos;

   AudioReader *AReader;
};

/* Per-disc-image cdstream dedupe table.  CUE / TOC files often
 * reference the same backing .bin / .wav across multiple track
 * lines (track-in-file layout); the cache lets us open each file
 * once and share the cdstream pointer between tracks instead of
 * re-opening per reference.
 *
 * Was std::map<std::string, cdstream*> in the C++ source.  Replaced
 * here with a fixed-size insert-once-find-many sorted array, same
 * shape as subq_map above.  Lookups happen once per track-line
 * during parse and never on the gameplay hot path, so the linear
 * insert cost is irrelevant - what matters is dropping the last
 * std::map from the CD layer and getting rid of std::string keys.
 *
 * Filename strings up to 255 chars; cap 100 unique files per disc
 * image (any real Saturn rip is well under that). */
#define TOC_STREAMCACHE_MAX     100
#define TOC_STREAMCACHE_NAME    256

struct toc_streamcache_entry
{
   char       filename[TOC_STREAMCACHE_NAME];
   cdstream  *fp;
};

struct toc_streamcache
{
   struct toc_streamcache_entry entries[TOC_STREAMCACHE_MAX];
   unsigned                     count;
};
typedef struct toc_streamcache toc_streamcache;

class CDAccess_Image
{
   public:

      CDAccess_Image(const std::string& path, bool image_memcache);
      ~CDAccess_Image();

      bool Read_Raw_Sector(uint8_t *buf, int32_t lba);

      bool Fast_Read_Raw_PW_TSRE(uint8_t* pwbuf, int32_t lba);

      bool Read_TOC(TOC *toc);

      /* CDAccess vtable base.  MUST be the first member so the
       * dispatch function pointers in CDAccess.h can be reached
       * via a CDAccess* alias of this object's address. */
      CDAccess base;

   private:

      int32_t NumTracks;
      int32_t FirstTrack;
      int32_t LastTrack;
      int32_t total_sectors;
      uint8_t disc_type;
      CDRFILE_TRACK_INFO Tracks[100]; // Track #0(HMM?) through 99
      TOC toc;

      subq_map SubQReplaceMap;

      std::string base_dir;

      bool ImageOpen(const std::string& path, bool image_memcache);
      bool LoadSBI(const std::string& sbi_path);
      void GenerateTOC(void);
      void Cleanup(void);

      // MakeSubPQ will OR the simulated P and Q subchannel data into SubPWBuf.
      int32_t MakeSubPQ(int32_t lba, uint8_t *SubPWBuf) const;

      bool ParseTOCFileLineInfo(CDRFILE_TRACK_INFO *track, const int tracknum, const std::string &filename, const char *binoffset, const char *msfoffset, const char *length, bool image_memcache, toc_streamcache *cache);
      uint32_t GetSectorCount(CDRFILE_TRACK_INFO *track);
};

/* C-callable factory used by CDAccess.c.  Returns NULL on failure
 * (catches the ImageOpen exception). */
extern "C" CDAccess *CDAccess_Image_New(const char *path, bool image_memcache);

#endif
