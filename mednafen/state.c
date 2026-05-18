/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <boolean.h>
#include <libretro.h>

#include "general.h"
#include "state.h"

/* log_cb's defining TU is libretro.c.  Forward-declare here so this
   TU keeps its include surface minimal -- git.h would otherwise need
   to be pulled in just to chase the retro_log_printf_t typedef, and
   git.h carries a chunk of unrelated state-action / EmulateSpecStruct
   surface this state.c TU doesn't use. */
extern retro_log_printf_t log_cb;

#define RLSB MDFNSTATE_RLSB	//0x80000000

int32_t smem_read(StateMem *st, void *buffer, uint32_t len)
{
   if ((len + st->loc) > st->len)
      return 0;

   memcpy(buffer, st->data + st->loc, len);
   st->loc += len;

   return(len);
}

int32_t smem_write(StateMem *st, void *buffer, uint32_t len)
{
   if ((len + st->loc) > st->malloced)
   {
      uint32_t newsize = (st->malloced >= 32768) ? st->malloced : (st->initial_malloc ? st->initial_malloc : 32768);
      uint8_t *new_data;

      while(newsize < (len + st->loc))
         newsize *= 2;

      // Don't realloc data_frontend memory: it is owned by the caller.
      // Switch to our own heap allocation, copying the existing
      // contents over.
      if (st->data == st->data_frontend && st->data != NULL)
      {
         new_data = (uint8_t *)malloc(newsize);
         if (!new_data)
            return 0;                /* allocation failure -- nothing written */
         memcpy(new_data, st->data_frontend, st->malloced);
         st->data = new_data;
      }
      else
      {
         /* realloc-via-temp so we don't leak the previous allocation
          * if realloc returns NULL. The previous code did
          * st->data = realloc(st->data, ...) which lost the old
          * pointer on failure and segfaulted on the memcpy below. */
         new_data = (uint8_t *)realloc(st->data, newsize);
         if (!new_data)
            return 0;
         st->data = new_data;
      }

      st->malloced = newsize;
   }
   memcpy(st->data + st->loc, buffer, len);
   st->loc += len;

   if (st->loc > st->len)
      st->len = st->loc;

   return(len);
}

int32_t smem_putc(StateMem *st, int value)
{
   uint8_t tmpval = value;
   if(smem_write(st, &tmpval, 1) != 1)
      return(-1);
   return(1);
}

int32_t smem_seek(StateMem *st, uint32_t offset, int whence)
{
   switch(whence)
   {
      case SEEK_SET: st->loc = offset; break;
      case SEEK_END: st->loc = st->len - offset; break;
      case SEEK_CUR: st->loc += offset; break;
   }

   if(st->loc > st->len)
   {
      st->loc = st->len;
      return(-1);
   }

   /* st->loc is uint32_t; the prior `if (st->loc < 0)` branch was
    * unreachable. SEEK_END with offset > len wraps to a huge value
    * which is correctly clamped by the > st->len check above. */

   return(0);
}

int smem_write32le(StateMem *st, uint32_t b)
{
   uint8_t s[4];
   s[0]=b;
   s[1]=b>>8;
   s[2]=b>>16;
   s[3]=b>>24;
   return((smem_write(st, s, 4)<4)?0:4);
}

int smem_read32le(StateMem *st, uint32_t *b)
{
   uint8_t s[4];

   if(smem_read(st, s, 4) < 4)
      return(0);

   *b = s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);

   return(4);
}

static void SubWrite(StateMem *st, const SFORMAT *sf)
{
   while(sf->size || sf->name)	// Size can sometimes be zero, so also check for the text name.  These two should both be zero only at the end of a struct.
   {
      int32_t bytesize;
      uintptr_t p;
      uint32_t repcount;
      size_t repstride;
      char nameo[1 + 255];
      int slen;

      if(!sf->size || !sf->data)
      {
         sf++;
         continue;
      }

      if(sf->size == ~0U)		/* Link to another struct.	*/
      {
         SubWrite(st, (const SFORMAT *)sf->data);

         sf++;
         continue;
      }

      bytesize = sf->size;
      p = (uintptr_t)sf->data;
      repcount = sf->repcount;
      repstride = sf->repstride;
      slen = strlen(sf->name);

      memcpy(&nameo[1], sf->name, slen);
      nameo[0] = slen;

      smem_write(st, nameo, 1 + nameo[0]);
      smem_write32le(st, bytesize * (repcount + 1));

	do
	{
		// Special case for the evil bool type, to convert bool to 1-byte elements.
		if(!sf->type)
		{
			int32_t bool_monster;
			for(bool_monster = 0; bool_monster < bytesize; bool_monster++)
			{
				uint8_t tmp_bool = ((bool *)p)[bool_monster];
				smem_write(st, &tmp_bool, 1);
			}
		}
		else
			smem_write(st, (void*)p, bytesize);
	} while(p += repstride, repcount--);

      sf++;
   }
}

/* Name -> SFORMAT* lookup table. Was a
 * std::map<const char*, const SFORMAT*, compare_cstr>; the SFORMAT
 * lists this is built from are small (at most a few dozen live
 * entries per chunk), so a flat array with a linear strcmp scan is
 * simpler and, with no per-node allocation, at least as fast and far
 * more cache-friendly. This is purely an in-memory lookup structure
 * built fresh in ReadStateChunk -- it has no bearing on the
 * serialized savestate format. */
typedef struct
{
 const char    *name;
 const SFORMAT *sf;
} SFMapEntry;

typedef struct
{
 SFMapEntry *entries;
 size_t      count;
 size_t      cap;
} SFMap_t;

static const SFORMAT *SFMap_Find(const SFMap_t *m, const char *name)
{
 size_t i;
 for(i = 0; i < m->count; i++)
  if(!strcmp(m->entries[i].name, name))
   return m->entries[i].sf;
 return NULL;
}

/* Insert-or-overwrite, matching std::map's operator[] = assignment. */
static void SFMap_Set(SFMap_t *m, const char *name, const SFORMAT *sf)
{
 size_t i;
 for(i = 0; i < m->count; i++)
 {
  if(!strcmp(m->entries[i].name, name))
  {
   m->entries[i].sf = sf;
   return;
  }
 }
 if(m->count >= m->cap)
 {
  size_t newcap     = m->cap ? m->cap * 2 : 32;
  SFMapEntry *np    = (SFMapEntry *)realloc(m->entries, newcap * sizeof(SFMapEntry));
  if(!np)
   return;
  m->entries = np;
  m->cap     = newcap;
 }
 m->entries[m->count].name = name;
 m->entries[m->count].sf   = sf;
 m->count++;
}

static void MakeSFMap(const SFORMAT *sf, SFMap_t *sfmap)
{
 while(sf->size || sf->name) // Size can sometimes be zero, so also check for the text name.  These two should both be zero only at the end of a struct.
 {
  if(!sf->size || !sf->data)
  {
   sf++;
   continue;
  }

  if(sf->size == ~0U)            /* Link to another SFORMAT structure. */
   MakeSFMap((const SFORMAT *)sf->data, sfmap);
  else
  {
   assert(sf->name);

   if(SFMap_Find(sfmap, sf->name))
    log_cb( RETRO_LOG_WARN, "Duplicate save state variable in internal emulator structures(CLUB THE PROGRAMMERS WITH BREADSTICKS): %s\n", sf->name);

   SFMap_Set(sfmap, sf->name, sf);
  }

  sf++;
 }
}

static int ReadStateChunk(StateMem *st, const SFORMAT *sf, uint32_t size)
{
	SFMap_t sfmap = { NULL, 0, 0 };
	int temp;

	MakeSFMap(sf, &sfmap);

	temp = st->loc;

	while (st->loc < (temp + size))
	{
		uint32_t recorded_size;	// In bytes
		uint8_t toa[1 + 256];	// Don't change to char unless cast toa[0] to unsigned to smem_read() and other places.
		const SFORMAT *tmp;

		if(smem_read(st, toa, 1) != 1)
		{
			free(sfmap.entries);
			return(0);
		}

		if(smem_read(st, toa + 1, toa[0]) != toa[0])
		{
			free(sfmap.entries);
			return 0;
		}

		toa[1 + toa[0]] = 0;

		smem_read32le(st, &recorded_size);

		tmp = SFMap_Find(&sfmap, (char *)toa + 1);

		if(tmp)
		{
			if(recorded_size != tmp->size * (1 + tmp->repcount))
			{
				if(smem_seek(st, recorded_size, SEEK_CUR) < 0)
				{
					free(sfmap.entries);
					return(0);
				}
			}
			else
			{
				const uint32_t type        = tmp->type;
				const uint32_t expected_size = tmp->size;	// In bytes
				uintptr_t p                = (uintptr_t)tmp->data;
				uint32_t repcount            = tmp->repcount;
				const size_t repstride     = tmp->repstride;

				do
				{
					smem_read(st, (void*)p, expected_size);

					if(!type)
					{
						int32_t bool_monster;
						// Converting downwards is necessary for the case of sizeof(bool) > 1
						for(bool_monster = expected_size - 1; bool_monster >= 0; bool_monster--)
							((bool *)p)[bool_monster] = ((uint8_t *)p)[bool_monster];
					}
				} while(p += repstride, repcount--);
			}
		}
		else
		{
			if(smem_seek(st, recorded_size, SEEK_CUR) < 0)
			{
				free(sfmap.entries);
				return(0);
			}
		}
	}

	free(sfmap.entries);
	return 1;
}

static int WriteStateChunk(StateMem *st, const char *sname, SFORMAT *sf)
{
	int32_t data_start_pos;
	int32_t end_pos;

	uint8_t sname_tmp[32];
	size_t sname_len = strlen(sname);

	memset(sname_tmp, 0, sizeof(sname_tmp));
        memcpy((char *)sname_tmp, sname, (sname_len < 32) ? sname_len : 32);

	smem_write(st, sname_tmp, 32);

	smem_write32le(st, 0);                // We'll come back and write this later.

	data_start_pos = st->loc;

	SubWrite(st, sf);

	end_pos = st->loc;

	smem_seek(st, data_start_pos - 4, SEEK_SET);
	smem_write32le(st, end_pos - data_start_pos);
	smem_seek(st, end_pos, SEEK_SET);

	return(end_pos - data_start_pos);
}

/* This function is called by the game driver(NES, GB, GBA) to save a state. */
static int MDFNSS_StateAction_internal( void *st_p, int load, int data_only, SSDescriptor *section)
{
	StateMem *st = (StateMem*)st_p;

	if ( load )
	{
		char sname[32];

		int found = 0;
		uint32_t tmp_size;
		uint32_t total = 0;

		while ( smem_read(st, (uint8_t *)sname, 32 ) == 32 )
		{
			if(smem_read32le(st, &tmp_size) != 4)
				return(0);

			total += tmp_size + 32 + 4;

			// Yay, we found the section
			if ( !strncmp(sname, section->name, 32 ) )
			{
				if(!ReadStateChunk(st, section->sf, tmp_size))
					return(0);

				found = 1;
				break;
			}
			else
			{
				if ( smem_seek(st, tmp_size, SEEK_CUR ) < 0 )
					return(0);
			}
		}

		if ( smem_seek(st, -total, SEEK_CUR) < 0 )
			return(0);

		if( !found && !section->optional ) // Not found.  We are sad!
			return(0);
	}
	else
	{
		// Write all the chunks.
		if ( !WriteStateChunk(st, section->name, section->sf ) )
			return(0);
	}

	return(1);
}

int MDFNSS_StateAction(void *st_p, int load, int data_only, SFORMAT *sf, const char *name, bool optional)
{
   SSDescriptor love;
   StateMem *st  = (StateMem*)st_p;

   love.sf       = sf;
   love.name     = name;
   love.optional = optional;

   return(MDFNSS_StateAction_internal(st, load, 0, &love));
}

extern int LibRetro_StateAction( StateMem* sm, const unsigned load);

int MDFNSS_SaveSM(void *st_p, uint32_t ver, const void *unused0, const void *unused1, const void *unused2)
{
	int success;
	(void)unused0;
	(void)unused1;
	(void)unused2;
	uint8_t header[32];
	StateMem *st = (StateMem*)st_p;
	static const char *header_magic = "MDFNSVST";
	int neowidth = 0, neoheight = 0;
	uint32_t sizy;

	// Write header.
	memset( header, 0, sizeof(header) );
	memcpy( header, header_magic, 8 );
	/* MDFN_en32lsb folded inline: 4 LE byte stores per uint32_t. */
	header[16] = ver;       header[17] = ver >> 8;       header[18] = ver >> 16;       header[19] = ver >> 24;
	header[24] = neowidth;  header[25] = neowidth >> 8;  header[26] = neowidth >> 16;  header[27] = neowidth >> 24;
	header[28] = neoheight; header[29] = neoheight >> 8; header[30] = neoheight >> 16; header[31] = neoheight >> 24;
	smem_write(st, header, 32);

	// Call out to main save state function.
	success = LibRetro_StateAction( st, 0 /*SAVE*/);

	// Circle back and fill in the file size.
	sizy = st->loc;
	smem_seek(st, 16 + 4, SEEK_SET);
	smem_write32le(st, sizy);

	// Success!
	return success;
}

int MDFNSS_LoadSM(void *st_p, uint32_t ver)
{
	uint8_t header[32];
	uint32_t stateversion;
	StateMem *st = (StateMem*)st_p;

	smem_read( st, header, 32 );

	// Invalid header?
	if ( memcmp( header, "MDFNSVST", 8 ) )
		return(0);

	// Unsupported state version?
	/* MDFN_de32lsb folded inline: build host-endian uint32_t from 4 LE bytes. */
	stateversion = (uint32_t)header[16] | ((uint32_t)header[17] << 8) | ((uint32_t)header[18] << 16) | ((uint32_t)header[19] << 24);
	if ( stateversion < 0x900 )
		return(0);

	// Call out to main save state function.
	return LibRetro_StateAction( st, stateversion /*LOAD*/);
}
