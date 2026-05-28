/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* stv_5881.c - Sega 315-5881 encryption/compression chip (ST-V)
**  Copyright (C) 2025 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
*/

/* C89 port of MAME's sega_315_5881_crypt_device.
** Original (BSD-3-Clause): Andreas Naive, Olivier Galibert, David Haywood.
**
** The encryption is a stream cipher in counter mode built from a 16-bit block
** cipher (two 4-round Feistel networks), optionally followed by a Huffman/RLE
** decompression pass. The S-box tables, key-scheduling tables, Huffman trees
** and the bit-permutations are reproduced verbatim from MAME; the control flow
** of block_decrypt / enc_start / enc_fill / line_fill / get_compressed_bit /
** get_decrypted_16 / do_decrypt is a 1:1 translation. See the long comment in
** MAME's 315-5881_crypt.cpp for the algorithm description.
**
** Differences vs the C++ original, all behaviour-preserving:
**   - device_t / save_item plumbing -> file-static state + STV5881_StateAction
**   - std::unique_ptr<uint8_t[]> buffers -> fixed file-static arrays
**     (BUFFER_SIZE == 2, LINE_SIZE == 512, exactly the upstream sizes)
**   - line_buffer.swap() -> pointer swap over a 2-entry array pool
**   - bitswap<16>()/BIT() -> local helpers
**   - debug printf/logerror -> removed
*/

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <boolean.h>

#include "stv_5881.h"

#define STV5881_BIT(x, n)        (((x) >> (n)) & 1)

enum { BUFFER_SIZE = 2 };
enum { LINE_SIZE   = 512 };
enum { FLAG_COMPRESSED = 0x20000 };

enum { FN1GK = 38 };
enum { FN2GK = 32 };

struct sbox
{
   uint8_t table[64];
   int inputs[6];      /* positions of the input bits, -1 means no input except from key */
   int outputs[2];     /* positions of the output bits */
};

/* ------------------------------------------------------------------ state */

static uint32_t          key;
static stv_5881_read_cb  m_read;

static uint8_t           buffer[BUFFER_SIZE];
static uint8_t           lbuf[2][LINE_SIZE];   /* line_buffer pool (swapped) */
static uint8_t          *line_buffer;          /* -> lbuf[lb_sel]            */
static uint8_t          *line_buffer_prev;     /* -> lbuf[lb_sel ^ 1]        */
static uint8_t           lb_sel;               /* which lbuf is "current"    */

static uint32_t          prot_cur_address;
static uint16_t          subkey;
static uint16_t          dec_hist;
static uint32_t          dec_header;

static int               enc_ready;
static int               first_read;

static int               buffer_pos;
static int               line_buffer_pos;
static int               line_buffer_size;
static int               buffer_bit;
static int               buffer_bit2;
static uint8_t           buffer2[2];
static uint16_t          buffer2a;

static int               block_size;
static int               block_pos;
static int               block_numlines;
static int               done_compression;

/* ------------------------------------------------------------- bitswap16 */
/* MAME bitswap<16>(v, b15, b14, ..., b0): output bit15 = bit b15 of v, etc. */
static uint16_t bitswap16(uint16_t v,
   int b15, int b14, int b13, int b12, int b11, int b10, int b9, int b8,
   int b7,  int b6,  int b5,  int b4,  int b3,  int b2,  int b1, int b0)
{
   return (uint16_t)(
      (STV5881_BIT(v, b15) << 15) | (STV5881_BIT(v, b14) << 14) |
      (STV5881_BIT(v, b13) << 13) | (STV5881_BIT(v, b12) << 12) |
      (STV5881_BIT(v, b11) << 11) | (STV5881_BIT(v, b10) << 10) |
      (STV5881_BIT(v, b9)  <<  9) | (STV5881_BIT(v, b8)  <<  8) |
      (STV5881_BIT(v, b7)  <<  7) | (STV5881_BIT(v, b6)  <<  6) |
      (STV5881_BIT(v, b5)  <<  5) | (STV5881_BIT(v, b4)  <<  4) |
      (STV5881_BIT(v, b3)  <<  3) | (STV5881_BIT(v, b2)  <<  2) |
      (STV5881_BIT(v, b1)  <<  1) | (STV5881_BIT(v, b0)  <<  0));
}

/* ----------------------------------------------------------------- tables */

static const struct sbox fn1_sboxes[4][4] =
{
   {  /* 1st round */
      {
         {
            0,3,2,2,1,3,1,2,3,2,1,2,1,2,3,1,3,2,2,0,2,1,3,0,0,3,2,3,2,1,2,0,
            2,3,1,1,2,2,1,1,1,0,2,3,3,0,2,1,1,1,1,1,3,0,3,2,1,0,1,2,0,3,1,3,
         },
         {3,4,5,7,-1,-1},
         {0,4}
      },
      {
         {
            2,2,2,0,3,3,0,1,2,2,3,2,3,0,2,2,1,1,0,3,3,2,0,2,0,1,0,1,2,3,1,1,
            0,1,3,3,1,3,3,1,2,3,2,0,0,0,2,2,0,3,1,3,0,3,2,2,0,3,0,3,1,1,0,2,
         },
         {0,1,2,5,6,7},
         {1,6}
      },
      {
         {
            0,1,3,0,3,1,1,1,1,2,3,1,3,0,2,3,3,2,0,2,1,1,2,1,1,3,1,0,0,2,0,1,
            1,3,1,0,0,3,2,3,2,0,3,3,0,0,0,0,1,2,3,3,2,0,3,2,1,0,0,0,2,2,3,3,
         },
         {0,2,5,6,7,-1},
         {2,3}
      },
      {
         {
            3,2,1,2,1,2,3,2,0,3,2,2,3,1,3,3,0,2,3,0,3,3,2,1,1,1,2,0,2,2,0,1,
            1,3,3,0,0,3,0,3,0,2,1,3,2,1,0,0,0,1,1,2,0,1,0,0,0,1,3,3,2,0,3,3,
         },
         {1,2,3,4,6,7},
         {5,7}
      },
   },
   {  /* 2nd round */
      {
         {
            3,3,1,2,0,0,2,2,2,1,2,1,3,1,1,3,3,0,0,3,0,3,3,2,1,1,3,2,3,2,1,3,
            2,3,0,1,3,2,0,1,2,1,3,1,2,2,3,3,3,1,2,2,0,3,1,2,2,1,3,0,3,0,1,3,
         },
         {0,1,3,4,5,7},
         {0,4}
      },
      {
         {
            2,0,1,0,0,3,2,0,3,3,1,2,1,3,0,2,0,2,0,0,0,2,3,1,3,1,1,2,3,0,3,0,
            3,0,2,0,0,2,2,1,0,2,3,3,1,3,1,0,1,3,3,0,0,1,3,1,0,2,0,3,2,1,0,1,
         },
         {0,1,3,4,6,-1},
         {1,5}
      },
      {
         {
            2,2,2,3,1,1,0,1,3,3,1,1,2,2,2,0,0,3,2,3,3,0,2,1,2,2,3,0,1,3,0,0,
            3,2,0,3,2,0,1,0,0,1,2,2,3,3,0,2,2,1,3,1,1,1,1,2,0,3,1,0,0,2,3,2,
         },
         {1,2,5,6,7,6},
         {2,7}
      },
      {
         {
            0,1,3,3,3,1,3,3,1,0,2,0,2,0,0,3,1,2,1,3,1,2,3,2,2,0,1,3,0,3,3,3,
            0,0,0,2,1,1,2,3,2,2,3,1,1,2,0,2,0,2,1,3,1,1,3,3,1,1,3,0,2,3,0,0,
         },
         {2,3,4,5,6,7},
         {3,6}
      },
   },
   {  /* 3rd round */
      {
         {
            0,0,1,0,1,0,0,3,2,0,0,3,0,1,0,2,0,3,0,0,2,0,3,2,2,1,3,2,2,1,1,2,
            0,0,0,3,0,1,1,0,0,2,1,0,3,1,2,2,2,0,3,1,3,0,1,2,2,1,1,1,0,2,3,1,
         },
         {1,2,3,4,5,7},
         {0,5}
      },
      {
         {
            1,2,1,0,3,1,1,2,0,0,2,3,2,3,1,3,2,0,3,2,2,3,1,1,1,1,0,3,2,0,0,1,
            1,0,0,1,3,1,2,3,0,0,2,3,3,0,1,0,0,2,3,0,1,2,0,1,3,3,3,1,2,0,2,1,
         },
         {0,2,4,5,6,7},
         {1,6}
      },
      {
         {
            0,3,0,2,1,2,0,0,1,1,0,0,3,1,1,0,0,3,0,0,2,3,3,2,3,1,2,0,0,2,3,0,
            255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
         },
         {0,2,4,6,7,-1},
         {2,3}
      },
      {
         {
            0,0,1,0,0,1,0,2,3,3,0,3,3,2,3,0,2,2,2,0,3,2,0,3,1,0,0,3,3,0,0,0,
            2,2,1,0,2,0,3,2,0,0,3,1,3,3,0,0,2,1,1,2,1,0,1,1,0,3,1,2,0,2,0,3,
         },
         {0,1,2,3,6,-1},
         {4,7}
      },
   },
   {  /* 4th round */
      {
         {
            0,3,3,3,3,3,2,0,0,1,2,0,2,2,2,2,1,1,0,2,2,1,3,2,3,2,0,1,2,3,2,1,
            3,2,2,3,1,0,1,0,0,2,0,1,2,1,2,3,1,2,1,1,2,2,1,0,1,3,2,3,2,0,3,1,
         },
         {0,1,3,4,5,6},
         {0,5}
      },
      {
         {
            0,3,0,0,2,0,3,1,1,1,2,2,2,1,3,1,2,2,1,3,2,2,3,3,0,3,1,0,3,2,0,1,
            3,0,2,0,1,0,2,1,3,3,1,2,2,0,2,3,3,2,3,0,1,1,3,3,0,2,1,3,0,2,2,3,
         },
         {0,1,2,3,5,7},
         {1,7}
      },
      {
         {
            0,1,2,3,3,3,3,1,2,0,2,3,2,1,0,1,2,2,1,2,0,3,2,0,1,1,0,1,3,1,3,1,
            3,1,0,0,1,0,0,0,0,1,2,2,1,1,3,3,1,2,3,3,3,2,3,0,2,2,1,3,3,0,2,0,
         },
         {2,3,4,5,6,7},
         {2,3}
      },
      {
         {
            0,2,1,1,3,2,0,3,1,0,1,0,3,2,1,1,2,2,0,3,1,0,1,2,2,2,3,3,0,0,0,0,
            1,2,1,0,2,1,2,2,2,3,2,3,0,1,3,0,0,1,3,0,0,1,1,0,1,0,0,0,0,2,0,1,
         },
         {0,1,2,4,6,7},
         {4,6}
      },
   },
};

static const struct sbox fn2_sboxes[4][4] =
{
   {  /* 1st round */
      {
         {
            3,3,0,1,0,1,0,0,0,3,0,0,1,3,1,2,0,3,3,3,2,1,0,1,1,1,2,2,2,3,2,2,
            2,1,3,3,1,3,1,1,0,0,1,2,0,2,2,1,1,2,3,1,2,1,3,1,2,2,0,1,3,0,2,2,
         },
         {1,3,4,5,6,7},
         {0,7}
      },
      {
         {
            0,1,3,0,1,1,2,3,2,0,0,3,2,1,3,1,3,3,0,0,1,0,0,3,0,3,3,2,3,2,0,1,
            3,2,3,2,2,1,3,1,1,1,0,3,3,2,2,1,1,2,0,2,0,1,1,0,1,0,1,1,2,0,3,0,
         },
         {0,3,5,6,5,0},
         {1,2}
      },
      {
         {
            0,2,2,1,0,1,2,1,2,0,1,2,3,3,0,1,3,1,1,2,1,2,1,3,3,2,3,3,2,1,0,1,
            0,1,0,2,0,1,1,3,2,0,3,2,1,1,1,3,2,3,0,2,3,0,2,2,1,3,0,1,1,2,2,2,
         },
         {0,2,3,4,7,-1},
         {3,4}
      },
      {
         {
            2,3,1,3,2,0,1,2,0,0,3,3,3,3,3,1,2,0,2,1,2,3,0,2,0,1,0,3,0,2,1,0,
            2,3,0,1,3,0,3,2,3,1,2,0,3,1,1,2,0,3,0,0,2,0,2,1,2,2,3,2,1,2,3,1,
         },
         {1,2,5,6,-1,-1},
         {5,6}
      },
   },
   {  /* 2nd round */
      {
         {
            2,3,1,3,1,0,3,3,3,2,3,3,2,0,0,3,2,3,0,3,1,1,2,3,1,1,2,2,0,1,0,0,
            2,1,0,1,2,0,1,2,0,3,1,1,2,3,1,2,0,2,0,1,3,0,1,0,2,2,3,0,3,2,3,0,
         },
         {0,1,4,5,6,7},
         {0,7}
      },
      {
         {
            0,2,2,0,2,2,0,3,2,3,2,1,3,2,3,3,1,1,0,0,3,0,2,1,1,3,3,2,3,2,0,1,
            1,2,3,0,1,0,3,0,3,1,0,2,1,2,0,3,2,3,1,2,2,0,3,2,3,0,0,1,2,3,3,3,
         },
         {0,2,3,6,7,-1},
         {1,5}
      },
      {
         {
            1,0,3,0,0,1,2,1,0,0,1,0,0,0,2,3,2,2,0,2,0,1,3,0,2,0,1,3,2,3,0,1,
            1,2,2,2,1,3,0,3,0,1,1,0,3,2,3,3,2,0,0,3,1,2,1,3,3,2,1,0,2,1,2,3,
         },
         {2,3,4,6,7,2},
         {2,3}
      },
      {
         {
            2,3,1,3,1,1,2,3,3,1,1,0,1,0,2,3,2,1,0,0,2,2,0,1,0,2,2,2,0,2,1,0,
            3,1,2,3,1,3,0,2,1,0,1,0,0,1,2,2,3,2,3,1,3,2,1,1,2,0,2,1,3,3,1,0,
         },
         {1,2,3,4,5,6},
         {4,6}
      },
   },
   {  /* 3rd round */
      {
         {
            0,3,0,1,3,0,0,2,1,0,1,3,2,2,2,0,3,3,3,0,2,2,0,3,0,0,2,3,0,3,2,1,
            3,3,0,3,0,2,3,3,1,1,1,0,2,2,1,1,3,0,3,1,2,0,2,0,0,0,3,2,1,1,0,0,
         },
         {1,4,5,6,7,5},
         {0,5}
      },
      {
         {
            0,3,0,1,3,0,3,1,3,2,2,2,3,0,3,2,2,1,2,2,0,3,2,2,0,0,2,1,1,3,2,3,
            2,3,3,1,2,0,1,2,2,1,0,0,0,0,2,3,1,2,0,3,1,3,1,2,3,2,1,0,3,0,0,2,
         },
         {0,2,3,4,6,7},
         {1,7}
      },
      {
         {
            2,2,0,3,0,3,1,0,1,1,2,3,2,3,1,0,0,0,3,2,2,0,2,3,1,3,2,0,3,3,1,3,
            255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
         },
         {1,2,4,7,2,-1},
         {2,4}
      },
      {
         {
            0,2,3,1,3,1,1,0,0,1,3,0,2,1,3,3,2,0,2,1,1,2,3,3,0,0,0,2,0,2,3,0,
            3,3,3,3,2,3,3,2,3,0,1,0,2,3,3,2,0,1,3,1,0,1,2,3,3,0,2,0,3,0,3,3,
         },
         {0,1,2,3,5,7},
         {3,6}
      },
   },
   {  /* 4th round */
      {
         {
            0,1,1,0,0,1,0,2,3,3,0,1,2,3,0,2,1,0,3,3,2,0,3,0,0,2,1,0,1,0,1,3,
            0,3,3,1,2,0,3,0,1,3,2,0,3,3,1,3,0,2,3,3,2,1,1,2,2,1,2,1,2,0,1,1,
         },
         {0,1,2,4,7,-1},
         {0,5}
      },
      {
         {
            2,0,0,2,3,0,2,3,3,1,1,1,2,1,1,0,0,2,1,0,0,3,1,0,0,3,3,0,1,0,1,2,
            0,2,0,2,0,1,2,3,2,1,1,0,3,3,3,3,3,3,1,0,3,0,0,2,0,3,2,0,2,2,0,1,
         },
         {0,1,3,5,6,-1},
         {1,3}
      },
      {
         {
            0,1,1,2,1,3,1,1,0,0,3,1,1,1,2,0,3,2,0,1,1,2,3,3,3,0,3,0,0,2,0,3,
            3,2,0,0,3,2,3,1,2,3,0,3,2,0,1,2,2,2,0,2,0,1,2,2,3,1,2,2,1,1,1,1,
         },
         {0,2,3,4,5,7},
         {2,7}
      },
      {
         {
            0,1,2,0,3,3,0,3,2,1,3,3,0,3,1,1,3,2,3,2,3,0,0,0,3,0,2,2,3,2,2,3,
            2,2,3,1,2,3,1,2,0,3,0,2,3,1,0,0,3,2,1,2,1,2,1,3,1,0,2,3,3,1,3,2,
         },
         {2,3,4,5,6,7},
         {4,6}
      },
   },
};

static const int fn1_game_key_scheduling[FN1GK][2] =
{
   {1,29},  {1,71},  {2,4},   {2,54},  {3,8},   {4,56},  {4,73},  {5,11},
   {6,51},  {7,92},  {8,89},  {9,9},   {9,39},  {9,58},  {10,90}, {11,6},
   {12,64}, {13,49}, {14,44}, {15,40}, {16,69}, {17,15}, {18,23}, {18,43},
   {19,82}, {20,81}, {21,32}, {22,5},  {23,66}, {24,13}, {24,45}, {25,12},
   {25,35}, {26,61}, {27,10}, {27,59}, {28,25}, {29,86}
};

static const int fn2_game_key_scheduling[FN2GK][2] =
{
   {0,0},   {1,3},   {2,11},  {3,20},  {4,22},  {5,23},  {6,29},  {7,38},
   {8,39},  {9,55},  {9,86},  {9,87},  {10,50}, {11,57}, {12,59}, {13,61},
   {14,63}, {15,67}, {16,72}, {17,83}, {18,88}, {19,94}, {20,35}, {21,17},
   {22,6},  {23,85}, {24,16}, {25,25}, {26,92}, {27,47}, {28,28}, {29,90}
};

static const int fn1_sequence_key_scheduling[20][2] =
{
   {0,52},  {1,34},  {2,17},  {3,36}, {4,84},  {4,88},  {5,57},  {6,48},
   {6,68},  {7,76},  {8,83},  {9,30}, {10,22}, {10,41}, {11,38}, {12,55},
   {13,74}, {14,19}, {14,80}, {15,26}
};

static const int fn2_sequence_key_scheduling[16] =
   {77,34,8,42,36,27,69,66,13,9,79,31,49,7,24,64};

static const int fn2_middle_result_scheduling[16] =
   {1,10,44,68,74,78,81,95,2,4,30,40,41,51,53,58};

/* node format
   0xxxxxxx - next node index
   1a0bbccc - end node  (a: 0 repeat / 1 fetch; b: fetch delta; c: count-1)
   11111111 - empty node */
static const uint8_t trees[9][2][32] =
{
   {
      {0x01,0x10,0x0f,0x05,0xc4,0x13,0x87,0x0a,0xcc,0x81,0xce,0x0c,0x86,0x0e,0x84,0xc2,
         0x11,0xc1,0xc3,0xcf,0x15,0xc8,0xcd,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      {0xc7,0x02,0x03,0x04,0x80,0x06,0x07,0x08,0x09,0xc9,0x0b,0x0d,0x82,0x83,0x85,0xc0,
         0x12,0xc6,0xc5,0x14,0x16,0xca,0xcb,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
   },
   {
      {0x02,0x80,0x05,0x04,0x81,0x10,0x15,0x82,0x09,0x83,0x0b,0x0c,0x0d,0xdc,0x0f,0xde,
         0x1c,0xcf,0xc5,0xdd,0x86,0x16,0x87,0x18,0x19,0x1a,0xda,0xca,0xc9,0x1e,0xce,0xff,},
      {0x01,0x17,0x03,0x0a,0x08,0x06,0x07,0xc2,0xd9,0xc4,0xd8,0xc8,0x0e,0x84,0xcb,0x85,
         0x11,0x12,0x13,0x14,0xcd,0x1b,0xdb,0xc7,0xc0,0xc1,0x1d,0xdf,0xc3,0xc6,0xcc,0xff,},
   },
   {
      {0xc6,0x80,0x03,0x0b,0x05,0x07,0x82,0x08,0x15,0xdc,0xdd,0x0c,0xd9,0xc2,0x14,0x10,
         0x85,0x86,0x18,0x16,0xc5,0xc4,0xc8,0xc9,0xc0,0xcc,0xff,0xff,0xff,0xff,0xff,0xff,},
      {0x01,0x02,0x12,0x04,0x81,0x06,0x83,0xc3,0x09,0x0a,0x84,0x11,0x0d,0x0e,0x0f,0x19,
         0xca,0xc1,0x13,0xd8,0xda,0xdb,0x17,0xde,0xcd,0xcb,0xff,0xff,0xff,0xff,0xff,0xff,},
   },
   {
      {0x01,0x80,0x0d,0x04,0x05,0x15,0x83,0x08,0xd9,0x10,0x0b,0x0c,0x84,0x0e,0xc0,0x14,
         0x12,0xcb,0x13,0xca,0xc8,0xc2,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      {0xc5,0x02,0x03,0x07,0x81,0x06,0x82,0xcc,0x09,0x0a,0xc9,0x11,0xc4,0x0f,0x85,0xd8,
         0xda,0xdb,0xc3,0xdc,0xdd,0xc1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
   },
   {
      {0x01,0x80,0x06,0x0c,0x05,0x81,0xd8,0x84,0x09,0xdc,0x0b,0x0f,0x0d,0x0e,0x10,0xdb,
         0x11,0xca,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      {0xc4,0x02,0x03,0x04,0xcb,0x0a,0x07,0x08,0xd9,0x82,0xc8,0x83,0xc0,0xc1,0xda,0xc2,
         0xc9,0xc3,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
   },
   {
      {0x01,0x02,0x06,0x0a,0x83,0x0b,0x07,0x08,0x09,0x82,0xd8,0x0c,0xd9,0xda,0xff,0xff,
         0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      {0xc3,0x80,0x03,0x04,0x05,0x81,0xca,0xc8,0xdb,0xc9,0xc0,0xc1,0x0d,0xc2,0xff,0xff,
         0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
   },
   {
      {0x01,0x02,0x03,0x04,0x81,0x07,0x08,0xd8,0xda,0xd9,0xff,0xff,0xff,0xff,0xff,0xff,
         0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      {0xc2,0x80,0x05,0xc9,0xc8,0x06,0x82,0xc0,0x09,0xc1,0xff,0xff,0xff,0xff,0xff,0xff,
         0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
   },
   {
      {0x01,0x80,0x04,0xc8,0xc0,0xd9,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
         0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      {0xc1,0x02,0x03,0x81,0x05,0xd8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
         0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
   },
   {
      {0x01,0xd8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
         0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      {0xc0,0x80,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
         0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
   },
};

/* ---------------------------------------------------------------- cipher */

static int feistel_function(int input, const struct sbox *sboxes, uint32_t subkeys)
{
   int k, m;
   int aux;
   int result = 0;

   for (m = 0; m < 4; ++m)   /* 4 sboxes */
   {
      for (k = 0, aux = 0; k < 6; ++k)
         if (sboxes[m].inputs[k] != -1)
            aux |= STV5881_BIT(input, sboxes[m].inputs[k]) << k;

      aux = sboxes[m].table[(aux ^ subkeys) & 0x3f];

      for (k = 0; k < 2; ++k)
         result |= STV5881_BIT(aux, k) << sboxes[m].outputs[k];

      subkeys >>= 6;
   }

   return result;
}

static uint16_t block_decrypt(uint32_t game_key, uint16_t sequence_key,
   uint16_t counter, uint16_t data)
{
   int j;
   int aux, aux2;
   int A, B;
   int middle_result;
   uint32_t fn1_subkeys[4];
   uint32_t fn2_subkeys[4];

   /* Game-key scheduling (could be cached once per game). */
   memset(fn1_subkeys, 0, sizeof(fn1_subkeys));
   memset(fn2_subkeys, 0, sizeof(fn2_subkeys));

   for (j = 0; j < FN1GK; ++j)
      if (STV5881_BIT(game_key, fn1_game_key_scheduling[j][0]) != 0)
      {
         aux  = fn1_game_key_scheduling[j][1] % 24;
         aux2 = fn1_game_key_scheduling[j][1] / 24;
         fn1_subkeys[aux2] ^= (1 << aux);
      }

   for (j = 0; j < FN2GK; ++j)
      if (STV5881_BIT(game_key, fn2_game_key_scheduling[j][0]) != 0)
      {
         aux  = fn2_game_key_scheduling[j][1] % 24;
         aux2 = fn2_game_key_scheduling[j][1] / 24;
         fn2_subkeys[aux2] ^= (1 << aux);
      }

   /* Sequence-key scheduling (could be cached once per run). */
   for (j = 0; j < 20; ++j)
      if (STV5881_BIT(sequence_key, fn1_sequence_key_scheduling[j][0]) != 0)
      {
         aux  = fn1_sequence_key_scheduling[j][1] % 24;
         aux2 = fn1_sequence_key_scheduling[j][1] / 24;
         fn1_subkeys[aux2] ^= (1 << aux);
      }

   for (j = 0; j < 16; ++j)
      if (STV5881_BIT(sequence_key, j) != 0)
      {
         aux  = fn2_sequence_key_scheduling[j] % 24;
         aux2 = fn2_sequence_key_scheduling[j] / 24;
         fn2_subkeys[aux2] ^= (1 << aux);
      }

   /* First Feistel Network */
   aux = bitswap16(counter, 5,12,14,13,9,3,6,4,8,1,15,11,0,7,10,2);

   B = aux >> 8;
   A = (aux & 0xff) ^ feistel_function(B, fn1_sboxes[0], fn1_subkeys[0]);
   B ^= feistel_function(A, fn1_sboxes[1], fn1_subkeys[1]);
   A ^= feistel_function(B, fn1_sboxes[2], fn1_subkeys[2]);
   B ^= feistel_function(A, fn1_sboxes[3], fn1_subkeys[3]);

   middle_result = (B << 8) | A;

   /* Middle-result-key scheduling */
   for (j = 0; j < 16; ++j)
      if (STV5881_BIT(middle_result, j) != 0)
      {
         aux  = fn2_middle_result_scheduling[j] % 24;
         aux2 = fn2_middle_result_scheduling[j] / 24;
         fn2_subkeys[aux2] ^= (1 << aux);
      }

   /* Second Feistel Network */
   aux = bitswap16(data, 14,3,8,12,13,7,15,4,6,2,9,5,11,0,1,10);

   B = aux >> 8;
   A = (aux & 0xff) ^ feistel_function(B, fn2_sboxes[0], fn2_subkeys[0]);
   B ^= feistel_function(A, fn2_sboxes[1], fn2_subkeys[1]);
   A ^= feistel_function(B, fn2_sboxes[2], fn2_subkeys[2]);
   B ^= feistel_function(A, fn2_sboxes[3], fn2_subkeys[3]);

   aux = (B << 8) | A;
   aux = bitswap16((uint16_t)aux, 15,7,6,14,13,12,5,4,3,2,11,10,9,1,0,8);

   return (uint16_t)aux;
}

/* ----------------------------------------------------------- stream/comp */

static void enc_start(void);
static void enc_fill(void);
static void line_fill(void);

static uint16_t get_decrypted_16(void)
{
   uint16_t enc = m_read(prot_cur_address);
   uint16_t dec = block_decrypt(key, subkey, (uint16_t)prot_cur_address, enc);
   uint16_t res = (uint16_t)((dec & 3) | (dec_hist & 0xfffc));

   dec_hist = dec;
   prot_cur_address++;

   return res;
}

static int get_compressed_bit(void)
{
   int res;

   if (buffer_bit2 == 15)
   {
      buffer_bit2  = 0;
      buffer2a     = get_decrypted_16();
      buffer2[0]   = (uint8_t)buffer2a;
      buffer2[1]   = (uint8_t)(buffer2a >> 8);
      buffer_pos   = 0;
   }
   else
      buffer_bit2++;

   res = (buffer2[(buffer_pos & 1) ^ 1] >> buffer_bit) & 1;
   buffer_bit--;
   if (buffer_bit == -1)
   {
      buffer_bit = 7;
      buffer_pos++;
   }
   return res;
}

static void enc_start(void)
{
   int blocky;

   block_pos        = 0;
   done_compression = 0;
   buffer_pos       = BUFFER_SIZE;

   /* if there are still bits left in the decompression buffer we must use
      them rather than reading the next word (twcup98 needs this). */
   if (buffer_bit2 < 14)
      dec_header = (uint32_t)(buffer2a & 0x0003) << 16;
   else
   {
      dec_hist   = 0;
      dec_header = (uint32_t)get_decrypted_16() << 16;
   }

   dec_header |= get_decrypted_16();

   block_numlines = ((dec_header & 0x000000ff) >> 0) + 1;
   blocky         = ((dec_header & 0x0001ff00) >> 8) + 1;
   block_size     = block_numlines * blocky;

   if (dec_header & FLAG_COMPRESSED)
   {
      line_buffer_size = blocky;
      line_buffer_pos  = line_buffer_size;
      buffer_bit       = 7;
      buffer_bit2      = 15;
   }

   enc_ready = 1;
}

static void enc_fill(void)
{
   int i;

   assert(buffer_pos == BUFFER_SIZE);

   for (i = 0; i != BUFFER_SIZE; i += 2)
   {
      uint16_t val = get_decrypted_16();

      buffer[i]     = (uint8_t)val;
      buffer[i + 1] = (uint8_t)(val >> 8);
      block_pos    += 2;

      if (!(dec_header & FLAG_COMPRESSED))
         if (block_pos == block_size)
            enc_start();   /* need a new header at the size boundary */
   }

   buffer_pos = 0;
}

static void line_fill(void)
{
   static const int offsets[4] = {0, 1, 0, -1};
   uint8_t *lp;
   uint8_t *lc;
   uint8_t *t;
   int i;

   assert(line_buffer_pos == line_buffer_size);

   lp = line_buffer;        /* current (becomes prev after swap) */
   lc = line_buffer_prev;   /* prev    (becomes current after swap) */

   t                = line_buffer;
   line_buffer      = line_buffer_prev;
   line_buffer_prev = t;
   lb_sel          ^= 1;

   line_buffer_pos = 0;

   i = 0;
   while (i != line_buffer_size)
   {
      /* vlc 0: start of line, vlc 1: interior, vlc 2-9: 7-1 bytes from end */
      int slot = i ? (i < line_buffer_size - 7 ? 1 : (i & 7) + 1) : 0;
      uint32_t tmp = 0;

      while (!(tmp & 0x80))
      {
         if (get_compressed_bit())
            tmp = trees[slot][1][tmp];
         else
            tmp = trees[slot][0][tmp];
      }

      if (tmp != 0xff)
      {
         int count = (tmp & 7) + 1;

         if (tmp & 0x40)
         {
            /* copy from previous line */
            int offset = offsets[(tmp & 0x18) >> 3];
            int j;

            for (j = 0; j != count; j++)
            {
               lc[i ^ 1] = lp[((i + offset) % line_buffer_size) ^ 1];
               i++;
            }
         }
         else
         {
            /* fetch a byte and write it `count` times */
            uint8_t byte;
            int j;

            byte =        (uint8_t)(get_compressed_bit()  << 1);
            byte = (uint8_t)((byte | get_compressed_bit()) << 1);
            byte = (uint8_t)((byte | get_compressed_bit()) << 1);
            byte = (uint8_t)((byte | get_compressed_bit()) << 1);
            byte = (uint8_t)((byte | get_compressed_bit()) << 1);
            byte = (uint8_t)((byte | get_compressed_bit()) << 1);
            byte = (uint8_t)((byte | get_compressed_bit()) << 1);
            byte = (uint8_t)( byte | get_compressed_bit());

            for (j = 0; j != count; j++)
               lc[(i++) ^ 1] = byte;
         }
      }
   }

   block_pos++;
   if (block_numlines == block_pos)
      done_compression = 1;
}

/* ----------------------------------------------------------- public API */

uint16_t STV5881_DoDecrypt(void)
{
   uint8_t *base;

   if (!enc_ready)
      enc_start();

   if (dec_header & FLAG_COMPRESSED)
   {
      if (line_buffer_pos == line_buffer_size)   /* nothing left to read */
      {
         if (done_compression == 1)
            enc_start();
         line_fill();
      }
      base = line_buffer + line_buffer_pos;
      line_buffer_pos += 2;
   }
   else
   {
      if (buffer_pos == BUFFER_SIZE)
         enc_fill();
      base = buffer + buffer_pos;
      buffer_pos += 2;
   }

   return (uint16_t)((base[0] << 8) | base[1]);
}

void STV5881_SetAddrLow(uint16_t data)
{
   prot_cur_address = (prot_cur_address & 0xffff0000u) | data;
   enc_ready = 0;
}

void STV5881_SetAddrHigh(uint16_t data)
{
   prot_cur_address = (prot_cur_address & 0x0000ffffu) | ((uint32_t)data << 16);
   enc_ready = 0;

   buffer_bit  = 7;
   buffer_bit2 = 15;
}

void STV5881_SetSubkey(uint16_t data)
{
   subkey = data;
   enc_ready = 0;
}

void STV5881_Reset(void)
{
   memset(buffer, 0, sizeof(buffer));
   memset(lbuf,   0, sizeof(lbuf));

   lb_sel           = 0;
   line_buffer      = lbuf[0];
   line_buffer_prev = lbuf[1];

   prot_cur_address = 0;
   subkey           = 0;
   dec_hist         = 0;
   dec_header       = 0;

   enc_ready        = 0;
   first_read       = 0;

   buffer_pos       = 0;
   line_buffer_pos  = 0;
   line_buffer_size = 0;
   buffer_bit       = 0;
   buffer_bit2      = 0;
   buffer2[0]       = 0;
   buffer2[1]       = 0;
   buffer2a         = 0;

   block_size       = 0;
   block_pos        = 0;
   block_numlines   = 0;
   done_compression = 0;
}

void STV5881_Init(uint32_t game_key, stv_5881_read_cb read_cb)
{
   key    = game_key;
   m_read = read_cb;
   STV5881_Reset();
}

void STV5881_StateAction(StateMem *sm, const unsigned load, const bool data_only)
{
   SFORMAT StateRegs[] =
   {
      SFVAR(prot_cur_address),
      SFVAR(subkey),
      SFVAR(dec_hist),
      SFVAR(dec_header),
      SFVAR(enc_ready),
      SFVAR(first_read),
      SFVAR(buffer_pos),
      SFVAR(line_buffer_pos),
      SFVAR(line_buffer_size),
      SFVAR(buffer_bit),
      SFVAR(buffer_bit2),
      SFVAR(buffer2a),
      SFVAR(block_size),
      SFVAR(block_pos),
      SFVAR(block_numlines),
      SFVAR(done_compression),
      SFVAR(lb_sel),
      SFPTR8(buffer, BUFFER_SIZE),
      SFPTR8(buffer2, 2),
      SFPTR8(&lbuf[0][0], (uint32_t)sizeof(lbuf)),

      SFEND
   };

   MDFNSS_StateAction(sm, load, data_only, StateRegs, "STV_5881", false);

   if (load)
   {
      /* Rebind the swappable line-buffer pointers from the saved selector. */
      lb_sel          &= 1;
      line_buffer      = lbuf[lb_sel];
      line_buffer_prev = lbuf[lb_sel ^ 1];
   }
}
