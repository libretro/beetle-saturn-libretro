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
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <sys/types.h>

#include <boolean.h>

#include "mednafen.h"
#include "general.h"
#include "state.h"

#include "hash/md5.h"

static bool IsAbsolutePath(const char *path)
{
   if (
#ifdef _WIN32
         path[0] == '\\' ||
#endif
         path[0] == '/'
      )
         return true;

#if defined(_WIN32) || defined(DOS)
   if((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z'))
   {
      if(path[1] == ':')
         return true;
   }
#endif

 return(false);
}

/* Split file_path into directory / base name / extension.
 * Each of the three output buffers may be NULL to skip that component.
 * The caller owns the buffers; out_size applies to whichever buffers are
 * non-NULL. */
void MDFN_GetFilePathComponents(const char *file_path,
      char *dir_path_out, char *file_base_out,
      char *file_ext_out, size_t out_size)
{
   const char *final_ds;
   const char *fn;            /* points into file_path, the base name */
   const char *fn_final_dot;

#ifdef _WIN32
   {
      const char *bs = strrchr(file_path, '\\');
      const char *fs = strrchr(file_path, '/');
      final_ds = (bs > fs) ? bs : fs;
   }
#else
   final_ds = strrchr(file_path, '/');
#endif

   if(!final_ds)
   {
      if(dir_path_out && out_size)
      {
         dir_path_out[0] = '.';
         dir_path_out[1] = '\0';
      }
      fn = file_path;
   }
   else
   {
      if(dir_path_out && out_size)
      {
         size_t dl = (size_t)(final_ds - file_path);
         if(dl >= out_size)
            dl = out_size - 1;
         memcpy(dir_path_out, file_path, dl);
         dir_path_out[dl] = '\0';
      }
      fn = final_ds + 1;
   }

   fn_final_dot = strrchr(fn, '.');

   if(fn_final_dot)
   {
      if(file_base_out && out_size)
      {
         size_t bl = (size_t)(fn_final_dot - fn);
         if(bl >= out_size)
            bl = out_size - 1;
         memcpy(file_base_out, fn, bl);
         file_base_out[bl] = '\0';
      }
      if(file_ext_out && out_size)
      {
         strncpy(file_ext_out, fn_final_dot, out_size - 1);
         file_ext_out[out_size - 1] = '\0';
      }
   }
   else
   {
      if(file_base_out && out_size)
      {
         strncpy(file_base_out, fn, out_size - 1);
         file_base_out[out_size - 1] = '\0';
      }
      if(file_ext_out && out_size)
         file_ext_out[0] = '\0';
   }
}

/* Resolve rel_path against dir_path, writing the result into the
 * caller-supplied buffer (out, out_size). Absolute rel_path values are
 * copied through unchanged; relative ones are joined with the platform
 * path separator. */
void MDFN_EvalFIP(char *out, size_t out_size, const char *dir_path, const char *rel_path)
{
#ifdef _WIN32
   const char slash = '\\';
#else
   const char slash = '/';
#endif

   if(!out_size)
      return;

   if(IsAbsolutePath(rel_path))
   {
      strncpy(out, rel_path, out_size - 1);
      out[out_size - 1] = '\0';
   }
   else
      snprintf(out, out_size, "%s%c%s", dir_path, slash, rel_path);
}

const char * GetFNComponent(const char *str)
{
   const char *tp1;
#ifdef _WIN32
   const char *tp3;
#endif

#ifdef _WIN32
   tp1 = ((char *)strrchr(str,'\\'));

   tp3 = ((char *)strrchr(str,'/'));

   if (tp1<tp3)
      tp1 = tp3;
#else
   tp1 = ((char *)strrchr(str,'/'));
#endif

   if (tp1)
      return (tp1+1);
   else
      return (str);
}
