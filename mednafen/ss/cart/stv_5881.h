/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* stv_5881.h - Sega 315-5881 encryption/compression chip (ST-V)
**  Copyright (C) 2025 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* C89 port of MAME's sega_315_5881_crypt_device (src/mame/sega/315-5881_crypt.cpp,
   license BSD-3-Clause, copyright Andreas Naive, Olivier Galibert, David Haywood).
   The cipher core (two 4-round Feistel networks + Huffman/RLE decompression) is
   reproduced verbatim; only the C++ scaffolding (device_t, std::unique_ptr,
   bitswap<>, BIT(), templates) has been rewritten in C. Functional behaviour is
   intended to be bit-identical to MAME -- see tools cross-check harness. */

#ifndef __MDFN_SS_CART_STV_5881_H
#define __MDFN_SS_CART_STV_5881_H

#include <stdint.h>
#include <boolean.h>

#include "../../state.h"   /* StateMem, SFORMAT, MDFNSS_StateAction */

#ifdef __cplusplus
extern "C" {
#endif

/* Source-data fetch callback: given a word index `addr`, return the raw
   (still-encrypted) 16-bit word the chip is to decrypt next. The ST-V wiring
   reads it from cart ROM at 0x02000000 + 2*addr. */
typedef uint16_t (*stv_5881_read_cb)(uint32_t addr);

/* One-time setup. `game_key` is the per-game 315-5881 key (FFR = 0x0524AC01).
   Calls STV5881_Reset() internally. */
void STV5881_Init(uint32_t game_key, stv_5881_read_cb read_cb);

void STV5881_Reset(void);

/* Register interface, mirrors MAME's set_addr_low/set_addr_high/set_subkey. */
void STV5881_SetAddrLow(uint16_t data);
void STV5881_SetAddrHigh(uint16_t data);
void STV5881_SetSubkey(uint16_t data);

/* Returns one decrypted 16-bit word from the stream, exactly as MAME's
   do_decrypt(): i.e. (buf[0] << 8) | buf[1] over the internal byte buffer.
   The ST-V cart layer applies the same outer byteswap MAME's common_prot_r
   does before handing the value to the SH-2. */
uint16_t STV5881_DoDecrypt(void);

void STV5881_StateAction(StateMem *sm, const unsigned load, const bool data_only);

#ifdef __cplusplus
}
#endif

#endif
