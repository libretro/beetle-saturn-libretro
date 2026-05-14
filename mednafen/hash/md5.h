#ifndef _MEDNAFEN_MD5_H
#define _MEDNAFEN_MD5_H

#include <stdint.h>
#include <string.h>
#include <retro_inline.h>

/* RFC 1321 MD5. Formerly a C++ class (md5_context); converted to a
   plain struct + free functions. The static std::string asciistr()
   helpers the class used to carry were unused anywhere in the tree
   and were dropped along with the <string> dependency, which is what
   let this become a C header. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   uint32_t total[2];
   uint32_t state[4];
   uint8_t  buffer[64];
} md5_context;

void mdfn_md5_starts(md5_context *ctx);
void mdfn_md5_update(md5_context *ctx, const uint8_t *input, uint32_t length);
void mdfn_md5_finish(md5_context *ctx, uint8_t digest[16]);

static INLINE void mdfn_md5_update_u32_as_lsb(md5_context *ctx, const uint32_t input)
{
   uint8_t buf[4];

   buf[0] = input >> 0;
   buf[1] = input >> 8;
   buf[2] = input >> 16;
   buf[3] = input >> 24;

   mdfn_md5_update(ctx, buf, 4);
}

static INLINE void mdfn_md5_update_string(md5_context *ctx, const char *string)
{
   mdfn_md5_update(ctx, (const uint8_t *)string, strlen(string));
}

#ifdef __cplusplus
}
#endif

#endif /* md5.h */
