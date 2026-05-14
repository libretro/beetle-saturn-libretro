
#include "libretro.h"
#include <string/stdstring.h>
#include <streams/file_stream.h>

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
// warning at the < CDInterfaces.size() check and required an `(int)` cast
// where a derived index was assigned. The value is never negative -- every
// assignment is from a non-negative source (0, an unsigned index, or the
// frontend's image index, all u32 in practice) -- so unsigned is the type
// the variable actually wants to be.
static unsigned g_current_disc;

static unsigned g_initial_disc;
static std::string g_initial_disc_path;

static std::vector<CDIF *> CDInterfaces;

static std::vector<std::string> disk_image_paths;
static std::vector<std::string> disk_image_labels;

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


static void ReadM3U( std::vector<std::string> &file_list, std::string path, unsigned depth = 0 )
{
	std::string dir_path;
	char linebuf[ 2048 ];
	RFILE *fp = rfopen(path.c_str(), "rb");
	if (!fp)
		return;

	MDFN_GetFilePathComponents(path, &dir_path);

	while(rfgets(linebuf, sizeof(linebuf), fp) != NULL)
	{
		std::string efp;

		if(linebuf[0] == '#')
			continue;
		string_trim_whitespace_right(linebuf);
		if(linebuf[0] == 0)
			continue;

		efp = MDFN_EvalFIP(dir_path, std::string(linebuf));
		if(efp.size() >= 4 && efp.substr(efp.size() - 4) == ".m3u")
		{
			if(efp == path)
			{
				log_cb(RETRO_LOG_ERROR, "M3U at \"%s\" references self.\n", efp.c_str());
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
			ReadM3U(file_list, efp, depth + 1);
		}
		else
		{
			file_list.push_back(efp);
		}
	}

end:
	rfclose(fp);
}

static bool IsSaturnDisc( const uint8_t* sa32k )
{
	if(sha256(&sa32k[0x100], 0xD00) != "96b8ea48819cfa589f24c40aa149c224c420dccf38b730f00156efe25c9bbc8f"_sha256)
		return false;

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
			if ( g_current_disc < CDInterfaces.size() ) {
				CDB_SetDisc( false, CDInterfaces[g_current_disc] );
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
		if ( index < CDInterfaces.size() ) {
			g_current_disc = index;
			return true;
		}
	}

	return false;
}

static unsigned disk_get_num_images(void)
{
	return CDInterfaces.size();
}

static bool disk_replace_image_index(unsigned index, const struct retro_game_info *info)
{
	// index is unsigned; the previous check `index < 0` was dead code.
	if (index >= CDInterfaces.size())
		return false;

	if (info != NULL)
	{
		char image_label[512];

		image_label[0] = '\0';

		CDIF *image  = CDIF_Open(info->path, cdimagecache);
		// CDIF_Open can either throw or return NULL on failure. The
		// throw is handled by the caller via the outer try/catch;
		// NULL was previously assigned without a check, leaving a
		// NULL in CDInterfaces[] that would crash the later
		// ReadTOC() loop. Also: the prior CDIF was leaked here on
		// every swap (the old pointer was overwritten without
		// deletion); now we delete it before reassignment.
		if (image == NULL)
			return false;

		CDIF_Close(CDInterfaces[index]);
		CDInterfaces[index] = image;

		extract_basename(image_label,
			info->path,
			sizeof(image_label));

		disk_image_paths[index] = info->path;
		disk_image_labels[index] = image_label;
		return true;
	}
	return false;
}

static bool disk_add_image_index(void)
{
	CDInterfaces.push_back(NULL);
	disk_image_paths.push_back("");
	disk_image_labels.push_back("");
	return true;
}

static bool disk_set_initial_image(unsigned index, const char *path)
{
	if (string_is_empty(path))
		return false;

	g_initial_disc      = index;
	g_initial_disc_path = path;

	return true;
}

static bool disk_get_image_path(unsigned index, char *path, size_t len)
{
	if (len < 1)
		return false;

	if ((index < CDInterfaces.size()) &&
		 (index < disk_image_paths.size()))
	{
		if (!string_is_empty(disk_image_paths[index].c_str()))
		{
			strlcpy(path, disk_image_paths[index].c_str(), len);
			return true;
		}
	}

	return false;
}

static bool disk_get_image_label(unsigned index, char *label, size_t len)
{
	if (len < 1)
		return false;

	if ((index < CDInterfaces.size()) &&
		 (index < disk_image_labels.size()))
	{
		if (!string_is_empty(disk_image_labels[index].c_str()))
		{
			strlcpy(label, disk_image_labels[index].c_str(), len);
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
	g_initial_disc_path.clear();

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

	log_cb(RETRO_LOG_INFO, "Calculating game ID (%d discs)\n", CDInterfaces.size() );

	mctx.starts();

	for(x = 0; x < CDInterfaces.size(); x++)
	{
		CDIF *c = CDInterfaces[x];
		TOC toc;
		unsigned i;

		CDIF_ReadTOC(c, &toc);

		mctx.update_u32_as_lsb(toc.first_track);
		mctx.update_u32_as_lsb(toc.last_track);
		mctx.update_u32_as_lsb(toc.disc_type);

		for(i = 1; i <= 100; i++)
		{
			const TOC_Track* t = &toc.tracks[i];

			mctx.update_u32_as_lsb(t->adr);
			mctx.update_u32_as_lsb(t->control);
			mctx.update_u32_as_lsb(t->lba);
			mctx.update_u32_as_lsb(t->valid);
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

				mctx.update(&buf[0], 2048);
			}
		}

		if(x == 0)
		{
			md5_context fd_mctx = mctx;
			fd_mctx.finish(fd_id_out16);
		}
	}

	mctx.finish(id_out16);
}

void disc_cleanup(void)
{
	unsigned i;
	for(i = 0; i < CDInterfaces.size(); i++) {
		CDIF_Close(CDInterfaces[i]);
	}
	CDInterfaces.clear();

	disk_image_paths.clear();
	disk_image_labels.clear();

	g_current_disc = 0;
}

bool DetectRegion( unsigned* region )
{
	// std::vector replaces a raw `new uint8_t[]` here so that a throw
	// from ReadSector() / IsSaturnDisc() can't leak the 32 KiB buffer.
	std::vector<uint8_t> buf(2048 * 16);
	uint64_t possible_regions = 0;
	size_t ci;
	size_t rsi;
	const size_t region_strings_count = sizeof(region_strings) / sizeof(region_strings[0]);

	for(ci = 0; ci < CDInterfaces.size(); ci++)
	{
		CDIF* c = CDInterfaces[ci];
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
	for( i = 0; i < CDInterfaces.size(); i++ )
	{
		TOC toc;
		int32_t track;

		CDIF_ReadTOC(CDInterfaces[i], &toc);

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

				if(!CDIF_ReadRawSectorPWOnly(CDInterfaces[i], pwbuf, lba, false))
				{
					log_cb(RETRO_LOG_ERROR,
						"Testing Disc %zu of %zu: Error reading sector at LBA %d.\n",
							i + 1, CDInterfaces.size(), lba );
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
								i + 1, CDInterfaces.size(),
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
					  	i + 1, CDInterfaces.size(),
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
	if ( disc_num < CDInterfaces.size() ) {
		g_current_disc = disc_num;
		CDB_SetDisc( false, CDInterfaces[ g_current_disc ] );
	}
}

bool disc_load_content( MDFNGI* game_interface, const char* content_name, uint8_t* fd_id, char* sgid, char* sgname, char* sgarea, bool image_memcache )
{
	disc_cleanup();

	if ( !content_name )
		return false;

	uint8_t LayoutMD5[ 16 ];

	log_cb( RETRO_LOG_INFO, "Loading \"%s\"\n", content_name );

	try
	{
		size_t content_name_len = strlen( content_name );
		if ( content_name_len > 4 )
		{
			const char* content_ext = content_name + content_name_len - 4;
			if ( !strcasecmp( content_ext, ".m3u" ) )
			{
				// multiple discs
				unsigned i;
				ReadM3U(disk_image_paths, content_name);
				for(i = 0; i < disk_image_paths.size(); i++)
				{
					char image_label[4096];
					image_label[0] = '\0';
					log_cb(RETRO_LOG_INFO, "Adding CD: \"%s\".\n", disk_image_paths[i].c_str());

					CDIF *image = CDIF_Open(disk_image_paths[i].c_str(), image_memcache);
					// CDIF_Open is documented to throw on failure but
					// historically has also returned NULL in some
					// build configurations. Treat NULL as a hard
					// load failure rather than pushing a NULL onto
					// CDInterfaces (which the ReadTOC() loop below
					// would dereference).
					if (image == NULL)
					{
						log_cb(RETRO_LOG_ERROR, "Failed to open CD: \"%s\".\n", disk_image_paths[i].c_str());
						return false;
					}
					CDInterfaces.push_back(image);

					extract_basename(image_label,
						disk_image_paths[i].c_str(),
						sizeof(image_label));
					disk_image_labels.push_back(image_label);
				}
			}
			else
			{
				// single disc
				char image_label[4096];

				image_label[0] = '\0';

				disk_image_paths.push_back(content_name);
				CDIF *image  = CDIF_Open(content_name, image_memcache);
				if (image == NULL)
				{
					log_cb(RETRO_LOG_ERROR, "Failed to open CD: \"%s\".\n", content_name);
					return false;
				}
				CDInterfaces.push_back(image);

				extract_basename(image_label,
					content_name,
					sizeof(image_label));
				disk_image_labels.push_back(image_label);
			}

			/* Attempt to set initial disk index */
			if ((g_initial_disc > 0) &&
				(g_initial_disc 
				 < CDInterfaces.size()))
				if (g_initial_disc 
				< disk_image_paths.size())
					if (string_is_equal(
					disk_image_paths[
					g_initial_disc].c_str(),
					g_initial_disc_path.c_str()))
						g_current_disc =
							g_initial_disc;
		}
	}
	catch( std::exception &e )
	{
		log_cb(RETRO_LOG_ERROR, "Loading failed.\n");
		return false;
	}

	// Print out a track list for all discs.
	{
		unsigned i;
		for(i = 0; i < CDInterfaces.size(); i++)
		{
			TOC toc;
			int32_t track;
			CDIF_ReadTOC(CDInterfaces[i], &toc);
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
		layout_md5.starts();

		for( i = 0; i < CDInterfaces.size(); i++ )
		{
			TOC toc;
			uint32_t track;

			CDIF_ReadTOC(CDInterfaces[i], &toc);

			layout_md5.update_u32_as_lsb(toc.first_track);
			layout_md5.update_u32_as_lsb(toc.last_track);
			layout_md5.update_u32_as_lsb(toc.tracks[100].lba);

			for(track = toc.first_track; track <= toc.last_track; track++)
			{
				layout_md5.update_u32_as_lsb(toc.tracks[track].lba);
				layout_md5.update_u32_as_lsb(toc.tracks[track].control & 0x4);
			}
		}

		layout_md5.finish(LayoutMD5);
	}
	log_cb(RETRO_LOG_DEBUG, "Done calculating layout MD5.\n");
	// TODO: include module name in hash

	memcpy( game_interface->MD5, LayoutMD5, 16 );

	CalcGameID( game_interface->MD5, fd_id, sgid, sgname, sgarea );

	return true;
}

//==============================================================================
