
#include "libretro.h"
#include <string/stdstring.h>
#include <streams/file_stream.h>

#include <stdlib.h>

#include "mednafen/mednafen-types.h"
#include "mednafen/git.h"
#include "mednafen/general.h"
#include "mednafen/cdrom/cdromif.h"
#include "mednafen/hash/md5.h"
#include "mednafen/hash/sha256.h"
#include "mednafen/ss/ss.h"
#include "mednafen/ss/cdb.h"
#include "mednafen/ss/smpc.h"

// Forward declarations
extern "C"{
RFILE* rfopen(const char *path, const char *mode);
char *rfgets(char *buffer, int maxCount, RFILE* stream);
int rfclose(RFILE* stream);
}

extern bool cdimagecache;
//------------------------------------------------------------------------------
// Locals
//------------------------------------------------------------------------------

static bool g_eject_state;

// Was previously `static int g_current_disc;` which created a sign-compare
// warning at the < num_discs check and required an `(int)` cast
// where a derived index was assigned. The value is never negative -- every
// assignment is from a non-negative source (0, an unsigned index, or the
// frontend's image index, all u32 in practice) -- so unsigned is the type
// the variable actually wants to be.
static unsigned g_current_disc;

static unsigned g_initial_disc;
// Was std::string. Fixed buffer: every assignment is a frontend-supplied
// path, which is already bounded everywhere else in this file at 4096.
static char g_initial_disc_path[4096];

// The disc list. Was three std::vectors -- std::vector<CDIF*> plus two
// std::vector<std::string> -- always pushed, cleared and indexed
// together with identical length. They are now three parallel C arrays
// behind a shared count/capacity, grown with realloc-doubling. The
// path/label entries are heap-duplicated C strings owned by this module
// (freed in disc_cleanup / overwritten in place by disk_replace_image_index).
static CDIF  **disc_cdif    = NULL;   // was CDInterfaces
static char  **disc_paths   = NULL;   // was disk_image_paths
static char  **disc_labels  = NULL;   // was disk_image_labels
static size_t  num_discs    = 0;
static size_t  disc_cap     = 0;

// Append one disc entry. On success the list takes ownership of cdif
// (freed later by disc_cleanup) and copies path/label. On failure
// (allocation error) cdif is closed here -- "push, or it's gone" --
// so callers can ignore the return value without leaking the CDIF.
static bool disc_list_push(CDIF *cdif, const char *path, const char *label)
{
	char *path_dup;
	char *label_dup;

	if(num_discs >= disc_cap)
	{
		size_t newcap   = disc_cap ? disc_cap * 2 : 8;
		CDIF  **nc      = (CDIF  **)realloc(disc_cdif,   newcap * sizeof(CDIF *));
		char  **np      = (char  **)realloc(disc_paths,  newcap * sizeof(char *));
		char  **nl      = (char  **)realloc(disc_labels, newcap * sizeof(char *));
		if(nc) disc_cdif   = nc;
		if(np) disc_paths  = np;
		if(nl) disc_labels = nl;
		if(!nc || !np || !nl)
		{
			if(cdif)
				CDIF_Close(cdif);
			return false;
		}
		disc_cap = newcap;
	}

	path_dup  = strdup(path  ? path  : "");
	label_dup = strdup(label ? label : "");
	if(!path_dup || !label_dup)
	{
		free(path_dup);
		free(label_dup);
		if(cdif)
			CDIF_Close(cdif);
		return false;
	}

	disc_cdif[num_discs]   = cdif;
	disc_paths[num_discs]  = path_dup;
	disc_labels[num_discs] = label_dup;
	num_discs++;
	return true;
}

//
// Remember to rebuild region database in db.cpp if changing the order of
// entries in this table(and be careful about game id collisions,
// e.g. with some Korean games).
//
static const struct
{
	const char c;
	const char* str;	// Community-defined region string that may appear in filename.
	unsigned region;
}
region_strings[] =
{
	// Listed in order of preference for multi-region games.
	{ 'U', "USA", SMPC_AREA_NA },
	{ 'J', "Japan", SMPC_AREA_JP },
	{ 'K', "Korea", SMPC_AREA_KR },

	{ 'E', "Europe", SMPC_AREA_EU_PAL },
	{ 'E', "Germany", SMPC_AREA_EU_PAL },
	{ 'E', "France", SMPC_AREA_EU_PAL },
	{ 'E', "Spain", SMPC_AREA_EU_PAL },

	{ 'B', "Brazil", SMPC_AREA_CSA_NTSC },

	{ 'T', "Asia_NTSC", SMPC_AREA_ASIA_NTSC },
	{ 'A', "Asia_PAL", SMPC_AREA_ASIA_PAL },
	{ 'L', "CSA_PAL", SMPC_AREA_CSA_PAL },
};


//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

void extract_basename(char *buf, const char *path, size_t size)
{
   const char *base = strrchr(path, '/');
   if (!base)
      base = strrchr(path, '\\');
   if (!base)
      base = path;

   if (*base == '\\' || *base == '/')
      base++;

   strncpy(buf, base, size - 1);
   buf[size - 1] = '\0';

   char *ext = strrchr(buf, '.');
   if (ext)
      *ext = '\0';
}

void extract_directory(char *buf, const char *path, size_t size)
{
   strncpy(buf, path, size - 1);
   buf[size - 1] = '\0';

   char *base = strrchr(buf, '/');
   if (!base)
      base = strrchr(buf, '\\');

   if (base)
      *base = '\0';
   else
      buf[0] = '\0';
}


// Growable list of plain C strings; the M3U reader's output. Was
// std::vector<std::string>. The path entries are heap-duplicated and
// owned by the list; m3u_list_free releases them.
typedef struct
{
	char  **items;
	size_t  count;
	size_t  cap;
} m3u_list;

static bool m3u_list_push(m3u_list *l, const char *s)
{
	char *dup;
	if(l->count >= l->cap)
	{
		size_t newcap = l->cap ? l->cap * 2 : 8;
		char **ni     = (char **)realloc(l->items, newcap * sizeof(char *));
		if(!ni)
			return false;
		l->items = ni;
		l->cap   = newcap;
	}
	dup = strdup(s);
	if(!dup)
		return false;
	l->items[l->count++] = dup;
	return true;
}

static void m3u_list_free(m3u_list *l)
{
	size_t i;
	for(i = 0; i < l->count; i++)
		free(l->items[i]);
	free(l->items);
	l->items = NULL;
	l->count = 0;
	l->cap   = 0;
}

static void ReadM3U( m3u_list *file_list, const char *path, unsigned depth )
{
	char dir_path[4096];
	char linebuf[ 2048 ];
	RFILE *fp = rfopen(path, "rb");
	if (!fp)
		return;

	MDFN_GetFilePathComponents(path, dir_path, NULL, NULL, sizeof(dir_path));

	while(rfgets(linebuf, sizeof(linebuf), fp) != NULL)
	{
		char efp_buf[4096];
		size_t efp_len;

		if(linebuf[0] == '#')
			continue;
		string_trim_whitespace_right(linebuf);
		if(linebuf[0] == 0)
			continue;

		MDFN_EvalFIP(efp_buf, sizeof(efp_buf), dir_path, linebuf);
		efp_len = strlen(efp_buf);
		if(efp_len >= 4 && !strcmp(efp_buf + efp_len - 4, ".m3u"))
		{
			if(!strcmp(efp_buf, path))
			{
				log_cb(RETRO_LOG_ERROR, "M3U at \"%s\" references self.\n", efp_buf);
				goto end;
			}

			if(depth == 99)
			{
				log_cb(RETRO_LOG_ERROR, "M3U load recursion too deep!\n");
				goto end;
			}

			// Pre-increment so the recursive call actually sees a
			// deeper depth. Previously this was 'depth++', which is
			// post-increment on a value parameter -- the recursive
			// call always received the same value, making the
			// depth==99 guard above unreachable and allowing
			// stack-blowing recursion via crafted m3u chains.
			ReadM3U(file_list, efp_buf, depth + 1);
		}
		else
		{
			m3u_list_push(file_list, efp_buf);
		}
	}

end:
	rfclose(fp);
}

static bool IsSaturnDisc( const uint8_t* sa32k )
{
	{
		static const sha256_digest saturn_disc_hash =
		{ { 0x96, 0xb8, 0xea, 0x48, 0x81, 0x9c, 0xfa, 0x58, 0x9f, 0x24, 0xc4, 0x0a, 0xa1, 0x49, 0xc2, 0x24, 0xc4, 0x20, 0xdc, 0xcf, 0x38, 0xb7, 0x30, 0xf0, 0x01, 0x56, 0xef, 0xe2, 0x5c, 0x9b, 0xbc, 0x8f } };
		const sha256_digest h = sha256(&sa32k[0x100], 0xD00);

		if(!sha256_digest_eq(&h, &saturn_disc_hash))
			return false;
	}

	if(memcmp(&sa32k[0], "SEGA SEGASATURN ", 16))
		return false;

	log_cb(RETRO_LOG_INFO, "This is a Saturn disc.\n");
	return true;
}

static bool disk_set_eject_state( bool ejected )
{
	if ( ejected == g_eject_state )
	{
		// no change
		return false;
	}
	else
	{
		// store
		g_eject_state = ejected;

		if ( ejected )
		{
			// open disc tray
			CDB_SetDisc( true, NULL );
		}
		else
		{
			// close the tray - with a disc inside
			if ( g_current_disc < num_discs ) {
				CDB_SetDisc( false, disc_cdif[g_current_disc] );
			} else {
				CDB_SetDisc( false, NULL );
			}
		}

		return true;
	}
}

static bool disk_get_eject_state(void)
{
	return g_eject_state;
}

static bool disk_set_image_index(unsigned index)
{
	// only listen if the tray is open
	if ( g_eject_state == true )
	{
		if ( index < num_discs ) {
			g_current_disc = index;
			return true;
		}
	}

	return false;
}

static unsigned disk_get_num_images(void)
{
	return num_discs;
}

static bool disk_replace_image_index(unsigned index, const struct retro_game_info *info)
{
	// index is unsigned; the previous check `index < 0` was dead code.
	if (index >= num_discs)
		return false;

	if (info != NULL)
	{
		char image_label[512];
		char *path_dup;
		char *label_dup;

		image_label[0] = '\0';

		CDIF *image  = CDIF_Open(info->path, cdimagecache);
		// CDIF_Open returns NULL on failure. NULL was previously
		// assigned without a check, leaving a NULL in
		// disc_cdif[] that would crash the later ReadTOC()
		// loop. Also: the prior CDIF was leaked here on
		// every swap (the old pointer was overwritten without
		// deletion); now we delete it before reassignment.
		if (image == NULL)
			return false;

		extract_basename(image_label,
			info->path,
			sizeof(image_label));

		// Duplicate the new path/label before touching anything,
		// so a strdup failure leaves the slot untouched.
		path_dup  = strdup(info->path);
		label_dup = strdup(image_label);
		if(!path_dup || !label_dup)
		{
			free(path_dup);
			free(label_dup);
			CDIF_Close(image);
			return false;
		}

		CDIF_Close(disc_cdif[index]);
		disc_cdif[index] = image;

		free(disc_paths[index]);
		free(disc_labels[index]);
		disc_paths[index]  = path_dup;
		disc_labels[index] = label_dup;
		return true;
	}
	return false;
}

static bool disk_add_image_index(void)
{
	return disc_list_push(NULL, "", "");
}

static bool disk_set_initial_image(unsigned index, const char *path)
{
	if (string_is_empty(path))
		return false;

	g_initial_disc      = index;
	strlcpy(g_initial_disc_path, path, sizeof(g_initial_disc_path));

	return true;
}

static bool disk_get_image_path(unsigned index, char *path, size_t len)
{
	if (len < 1)
		return false;

	if (index < num_discs)
	{
		if (!string_is_empty(disc_paths[index]))
		{
			strlcpy(path, disc_paths[index], len);
			return true;
		}
	}

	return false;
}

static bool disk_get_image_label(unsigned index, char *label, size_t len)
{
	if (len < 1)
		return false;

	if (index < num_discs)
	{
		if (!string_is_empty(disc_labels[index]))
		{
			strlcpy(label, disc_labels[index], len);
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------

/* This has to be 'global', since we need to
 * access the current disk index inside
 * libretro.cpp */
unsigned disk_get_image_index(void)
{
	return g_current_disc;
}

static struct retro_disk_control_callback disk_interface =
{
	disk_set_eject_state,
	disk_get_eject_state,
	disk_get_image_index,
	disk_set_image_index,
	disk_get_num_images,
	disk_replace_image_index,
	disk_add_image_index,
};

static struct retro_disk_control_ext_callback disk_interface_ext =
{
	disk_set_eject_state,
	disk_get_eject_state,
	disk_get_image_index,
	disk_set_image_index,
	disk_get_num_images,
	disk_replace_image_index,
	disk_add_image_index,
	disk_set_initial_image,
	disk_get_image_path,
	disk_get_image_label,
};


void disc_init( retro_environment_t environ_cb )
{
	unsigned dci_version = 0;

	// start closed
	g_eject_state = false;

	g_initial_disc = 0;
	g_initial_disc_path[0] = '\0';

	// register vtable with environment
	if (environ_cb(RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION, &dci_version) && (dci_version >= 1))
		environ_cb(RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE, &disk_interface_ext);
	else
		environ_cb(RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE, &disk_interface);
}

static INLINE bool MDFN_isspace(const char c) { return c == ' ' || c == '\f' || c == '\r' || c == '\n' || c == '\t' || c == '\v'; }

// Remove whitespace from beginning of s
static void MDFN_ltrim(char* s)
{
 const char* si = s;
 char* di = s;
 bool InWhitespace = true;

 while(*si)
 {
  if(!InWhitespace || !MDFN_isspace(*si))
  {
   InWhitespace = false;
   *di = *si;
   di++;
  }
  si++;
 }

 *di = 0;
}

// Remove whitespace from end of s
static void MDFN_rtrim(char* s)
{
 const size_t len = strlen(s);

 if(!len)
  return;
 //
 size_t x = len;

 do
 {
  x--;

  if(!MDFN_isspace(s[x]))
   break;
 
  s[x] = 0;
 } while(x);
}

static void MDFN_trim(char* s)
{
 MDFN_rtrim(s);
 MDFN_ltrim(s);
}

static void MDFN_zapctrlchars(char* s)
{
 if(!s)
  return;

 while(*s)
 {
  if((unsigned char)*s < 0x20)
   *s = ' ';

  s++;
 }
}

static void CalcGameID( uint8_t* id_out16, uint8_t* fd_id_out16, char* sgid, char* sgname, char* sgarea )
{
	md5_context mctx;
	uint8_t buf[2048];
	size_t x;

	log_cb(RETRO_LOG_INFO, "Calculating game ID (%d discs)\n", num_discs );

	mdfn_md5_starts(&mctx);

	for(x = 0; x < num_discs; x++)
	{
		CDIF *c = disc_cdif[x];
		TOC toc;
		unsigned i;

		CDIF_ReadTOC(c, &toc);

		mdfn_md5_update_u32_as_lsb(&mctx, toc.first_track);
		mdfn_md5_update_u32_as_lsb(&mctx, toc.last_track);
		mdfn_md5_update_u32_as_lsb(&mctx, toc.disc_type);

		for(i = 1; i <= 100; i++)
		{
			const TOC_Track* t = &toc.tracks[i];

			mdfn_md5_update_u32_as_lsb(&mctx, t->adr);
			mdfn_md5_update_u32_as_lsb(&mctx, t->control);
			mdfn_md5_update_u32_as_lsb(&mctx, t->lba);
			mdfn_md5_update_u32_as_lsb(&mctx, t->valid);
		}

		for(i = 0; i < 512; i++)
		{
			if(CDIF_ReadSector(c, (uint8_t*)&buf[0], i, 1) >= 0x1)
			{
				if(i == 0)
				{
					char* tmp;
					memcpy(sgid, (void*)(&buf[0x20]), 16);
					sgid[16] = 0;
					if((tmp = strrchr(sgid, 'V')))
					{
						do
						{
						*tmp = 0;
						} while(tmp-- != sgid && (signed char)*tmp <= 0x20);
						memcpy(sgname, &buf[0x60], 0x70);
						sgname[0x70] = 0;
						MDFN_zapctrlchars(sgname);
						MDFN_trim(sgname);

						memcpy(sgarea, &buf[0x40], 0x10);
						sgarea[0x10] = 0;
						MDFN_zapctrlchars(sgarea);
						MDFN_trim(sgarea);
					}
				}

				mdfn_md5_update(&mctx, &buf[0], 2048);
			}
		}

		if(x == 0)
		{
			md5_context fd_mctx = mctx;
			mdfn_md5_finish(&fd_mctx, fd_id_out16);
		}
	}

	mdfn_md5_finish(&mctx, id_out16);
}

void disc_cleanup(void)
{
	size_t i;
	for(i = 0; i < num_discs; i++) {
		CDIF_Close(disc_cdif[i]);
		free(disc_paths[i]);
		free(disc_labels[i]);
	}
	free(disc_cdif);
	free(disc_paths);
	free(disc_labels);
	disc_cdif   = NULL;
	disc_paths  = NULL;
	disc_labels = NULL;
	num_discs   = 0;
	disc_cap    = 0;

	g_current_disc = 0;
}

bool DetectRegion( unsigned* region )
{
	// 32 KiB scratch buffer. Was a std::vector<uint8_t> (and before
	// that a raw new[]) for throw-safety; with exceptions gone from
	// the tree a plain stack array is simpler and allocation-free.
	uint8_t buf[2048 * 16];
	uint64_t possible_regions = 0;
	size_t ci;
	size_t rsi;
	const size_t region_strings_count = sizeof(region_strings) / sizeof(region_strings[0]);

	for(ci = 0; ci < num_discs; ci++)
	{
		CDIF* c = disc_cdif[ci];
		unsigned i;

		if(CDIF_ReadSector(c, &buf[0], 0, 16) != 0x1)
			continue;

		if(!IsSaturnDisc(&buf[0]))
			continue;

		for(i = 0; i < 16; i++)
		{
			for(rsi = 0; rsi < region_strings_count; rsi++)
			{
				if(region_strings[rsi].c == buf[0x40 + i])
				{
					possible_regions |= (uint64_t)1 << region_strings[rsi].region;
					break;
				}
			}
		}

		break;
	}

	for(rsi = 0; rsi < region_strings_count; rsi++)
	{
		if(possible_regions & ((uint64_t)1 << region_strings[rsi].region))
		{
			log_cb(RETRO_LOG_INFO, "Disc Region: \"%s\"\n", region_strings[rsi].str );
			*region = region_strings[rsi].region;
			return true;
		}
	}

	return false;
}

bool DiscSanityChecks(void)
{
	size_t i;

	// For each disc
	for( i = 0; i < num_discs; i++ )
	{
		TOC toc;
		int32_t track;

		CDIF_ReadTOC(disc_cdif[i], &toc);

		// For each track
		for( track = 1; track <= 99; track++)
		{
			const int32_t start_lba = toc.tracks[track].lba;
			const int32_t end_lba = start_lba + 32 - 1;
			bool any_subq_curpos = false;
			int32_t lba;

			if(!toc.tracks[track].valid)
				continue;

			if(toc.tracks[track].control & SUBQ_CTRLF_DATA)
				continue;

			//
			//
			//

			for(lba = start_lba; lba <= end_lba; lba++)
			{
				uint8_t pwbuf[96];
				uint8_t qbuf[12];

				if(!CDIF_ReadRawSectorPWOnly(disc_cdif[i], pwbuf, lba, false))
				{
					log_cb(RETRO_LOG_ERROR,
						"Testing Disc %zu of %zu: Error reading sector at LBA %d.\n",
							i + 1, num_discs, lba );
					return false;
				}

				subq_deinterleave(pwbuf, qbuf);
				if(subq_check_checksum(qbuf) && (qbuf[0] & 0xF) == ADR_CURPOS)
				{
					const uint8_t qm = qbuf[7];
					const uint8_t qs = qbuf[8];
					const uint8_t qf = qbuf[9];
					uint8_t lm, ls, lf;

					any_subq_curpos = true;

					LBA_to_AMSF(lba, &lm, &ls, &lf);
					lm = U8_to_BCD(lm);
					ls = U8_to_BCD(ls);
					lf = U8_to_BCD(lf);

					if(lm != qm || ls != qs || lf != qf)
					{
						log_cb(RETRO_LOG_ERROR,
							"Testing Disc %zu of %zu: Time mismatch at LBA=%d(%02x:%02x:%02x); Q subchannel: %02x:%02x:%02x\n",
								i + 1, num_discs,
								lba,
								lm, ls, lf,
								qm, qs, qf);

						return false;
					}
				}
			}

			if(!any_subq_curpos)
			{
				log_cb(RETRO_LOG_ERROR,
					  "Testing Disc %zu of %zu: No valid Q subchannel ADR_CURPOS data present at LBA %d-%d?!\n",
					  	i + 1, num_discs,
					  	start_lba, end_lba );
				return false;
			}

			break;

		} // for each track

	} // for each disc

	return true;
}

void disc_select( unsigned disc_num )
{
	if ( disc_num < num_discs ) {
		g_current_disc = disc_num;
		CDB_SetDisc( false, disc_cdif[ g_current_disc ] );
	}
}

bool disc_load_content( MDFNGI* game_interface, const char* content_name, uint8_t* fd_id, char* sgid, char* sgname, char* sgarea, bool image_memcache )
{
	uint8_t LayoutMD5[ 16 ];
	size_t content_name_len;

	disc_cleanup();

	if ( !content_name )
		return false;

	log_cb( RETRO_LOG_INFO, "Loading \"%s\"\n", content_name );

	content_name_len = strlen( content_name );
	if ( content_name_len > 4 )
	{
		const char* content_ext = content_name + content_name_len - 4;
		if ( !strcasecmp( content_ext, ".m3u" ) )
		{
			// multiple discs
			m3u_list m3u = { NULL, 0, 0 };
			size_t i;
			ReadM3U(&m3u, content_name, 0);
			for(i = 0; i < m3u.count; i++)
			{
				char image_label[4096];
				CDIF *image;
				image_label[0] = '\0';
				log_cb(RETRO_LOG_INFO, "Adding CD: \"%s\".\n", m3u.items[i]);

				image = CDIF_Open(m3u.items[i], image_memcache);
				// CDIF_Open returns NULL on failure. Treat NULL as a hard
				// load failure rather than pushing a NULL onto
				// disc_cdif (which the ReadTOC() loop below
				// would dereference).
				if (image == NULL)
				{
					log_cb(RETRO_LOG_ERROR, "Failed to open CD: \"%s\".\n", m3u.items[i]);
					m3u_list_free(&m3u);
					return false;
				}

				extract_basename(image_label,
					m3u.items[i],
					sizeof(image_label));
				// disc_list_push closes 'image' itself if it
				// fails, so there's nothing to clean up here --
				// just treat it as a hard load failure.
				if (!disc_list_push(image, m3u.items[i], image_label))
				{
					log_cb(RETRO_LOG_ERROR, "Failed to add CD: \"%s\".\n", m3u.items[i]);
					m3u_list_free(&m3u);
					return false;
				}
			}
			m3u_list_free(&m3u);
		}
		else
		{
			// single disc
			char image_label[4096];
			CDIF *image;

			image_label[0] = '\0';

			image = CDIF_Open(content_name, image_memcache);
			if (image == NULL)
			{
				log_cb(RETRO_LOG_ERROR, "Failed to open CD: \"%s\".\n", content_name);
				return false;
			}

			extract_basename(image_label,
				content_name,
				sizeof(image_label));
			// disc_list_push closes 'image' itself on failure.
			if (!disc_list_push(image, content_name, image_label))
			{
				log_cb(RETRO_LOG_ERROR, "Failed to add CD: \"%s\".\n", content_name);
				return false;
			}
		}

		/* Attempt to set initial disk index */
		if ((g_initial_disc > 0) &&
			(g_initial_disc < num_discs))
			if (string_is_equal(
				disc_paths[g_initial_disc],
				g_initial_disc_path))
					g_current_disc =
						g_initial_disc;
	}

	// Print out a track list for all discs.
	{
		unsigned i;
		for(i = 0; i < num_discs; i++)
		{
			TOC toc;
			int32_t track;
			CDIF_ReadTOC(disc_cdif[i], &toc);
			log_cb(RETRO_LOG_DEBUG, "Disc %d\n", i + 1);
			for(track = toc.first_track; track <= toc.last_track; track++) {
				log_cb(RETRO_LOG_DEBUG, "- Track %2d, LBA: %6d  %s\n", track, toc.tracks[track].lba, (toc.tracks[track].control & 0x4) ? "DATA" : "AUDIO");
			}
			log_cb(RETRO_LOG_DEBUG, "Leadout: %6d\n", toc.tracks[100].lba);
		}
	}

	log_cb(RETRO_LOG_DEBUG, "Calculating layout MD5.\n");
	// Calculate layout MD5.  The system emulation LoadCD() code is free to ignore this value and calculate
	// its own, or to use it to look up a game in its database.
	{
		md5_context layout_md5;
		unsigned i;
		mdfn_md5_starts(&layout_md5);

		for( i = 0; i < num_discs; i++ )
		{
			TOC toc;
			uint32_t track;

			CDIF_ReadTOC(disc_cdif[i], &toc);

			mdfn_md5_update_u32_as_lsb(&layout_md5, toc.first_track);
			mdfn_md5_update_u32_as_lsb(&layout_md5, toc.last_track);
			mdfn_md5_update_u32_as_lsb(&layout_md5, toc.tracks[100].lba);

			for(track = toc.first_track; track <= toc.last_track; track++)
			{
				mdfn_md5_update_u32_as_lsb(&layout_md5, toc.tracks[track].lba);
				mdfn_md5_update_u32_as_lsb(&layout_md5, toc.tracks[track].control & 0x4);
			}
		}

		mdfn_md5_finish(&layout_md5, LayoutMD5);
	}
	log_cb(RETRO_LOG_DEBUG, "Done calculating layout MD5.\n");
	// TODO: include module name in hash

	memcpy( game_interface->MD5, LayoutMD5, 16 );

	CalcGameID( game_interface->MD5, fd_id, sgid, sgname, sgarea );

	return true;
}

//==============================================================================
