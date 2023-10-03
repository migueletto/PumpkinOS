//
// Copyright(C) 2023 by Ryan Krafnick
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DSDA TRANMAP
//

#include <string.h>

#include "md5.h"
#include "lprintf.h"
#include "m_file.h"
#include "w_wad.h"
#include "z_zone.h"

#include "dsda/data_organizer.h"
#include "dsda/utility.h"

#include "tranmap.h"

static char* tranmap_base_dir;
static char* tranmap_palette_dir;
static dsda_cksum_t playpal_cksum;
static const int default_tranmap_alpha = 66;
static const int tranmap_length = 256 * 256;
static const byte* tranmap_data[100];

static void dsda_CalculatePlaypalCksum(void) {
  struct MD5Context md5;
  int lump;

  lump = W_GetNumForName("PLAYPAL");

  MD5Init(&md5);
  MD5Update(&md5, W_LumpByNum(lump), W_LumpLength(lump));
  MD5Final(playpal_cksum.bytes, &md5);
  dsda_TranslateCheckSum(&playpal_cksum);
}

static void dsda_InitTranMapBaseDir(void) {
  int length;
  const char* data_root;

  data_root = dsda_DataRoot();

  length = strlen(data_root) + 10; // "/tranmaps\0"
  tranmap_base_dir = Z_Malloc(length);
  snprintf(tranmap_base_dir, length, "%s/tranmaps", data_root);

  M_MakeDir(tranmap_base_dir, true);
}

static void dsda_InitTranMapPaletteDir(void) {
  int length;

  if (!tranmap_base_dir)
    dsda_InitTranMapBaseDir();

  if (!playpal_cksum.string[0])
    dsda_CalculatePlaypalCksum();

  length = strlen(tranmap_base_dir) + 34; // "/<cksum (32)>\0"
  tranmap_palette_dir = Z_Malloc(length);
  snprintf(tranmap_palette_dir, length, "%s/%s", tranmap_base_dir, playpal_cksum.string);

  M_MakeDir(tranmap_palette_dir, true);
}

//
// R_InitTranMap
//
// Initialize translucency filter map
//
// By Lee Killough 2/21/98
//

#define TSC 12 /* number of fixed point digits in filter percent */

static byte* dsda_GenerateTranMap(unsigned int alpha) {
  byte* buffer;
  const byte* playpal;
  int pal[3][256];
  int tot[256];
  int pal_w1[3][256];
  int w1, w2;

  playpal = W_LumpByName("PLAYPAL");

  w1 = (alpha << TSC) / 100;
  w2 = (1l << TSC) - w1;

  buffer = Z_Malloc(tranmap_length);

  // First, convert playpal into long int type, and transpose array,
  // for fast inner-loop calculations. Precompute tot array.
  {
    register int i = 255;
    register const unsigned char *p = playpal + 255 * 3;

    do {
      register int t, d;
      pal_w1[0][i] = (pal[0][i] = t = p[0]) * w1;
      d = t * t;
      pal_w1[1][i] = (pal[1][i] = t = p[1]) * w1;
      d += t * t;
      pal_w1[2][i] = (pal[2][i] = t = p[2]) * w1;
      d += t * t;
      p -= 3;
      tot[i] = d << (TSC - 1);
    }
    while (--i >= 0);
  }

  // Next, compute all entries using minimum arithmetic.
  {
    int i, j;
    byte *tp = buffer;

    for (i = 0; i < 256; i++) {
      int r1 = pal[0][i] * w2;
      int g1 = pal[1][i] * w2;
      int b1 = pal[2][i] * w2;

      for (j = 0; j < 256; j++, tp++) {
        register int color = 255;
        register int err;
        int r = pal_w1[0][j] + r1;
        int g = pal_w1[1][j] + g1;
        int b = pal_w1[2][j] + b1;
        int best = INT_MAX;

        do {
          err = tot[color] - pal[0][color] * r - pal[1][color] * g - pal[2][color] * b;
          if (err < best) {
            best = err;
            *tp = color;
          }
        }
        while (--color >= 0);
      }
    }
  }

  return buffer;
}

const byte* dsda_TranMap(unsigned int alpha) {
  int length;
  byte *buffer = NULL;

  if (alpha > 99)
    return NULL;

  if (!tranmap_data[alpha]) {
    char* filename;

    if (!tranmap_palette_dir)
      dsda_InitTranMapPaletteDir();

    length = strlen(tranmap_palette_dir) + 16; // "/tranmap_99.dat\0"
    filename = Z_Malloc(length);
    snprintf(filename, length, "%s/tranmap_%02d.dat", tranmap_palette_dir, alpha);

    length = M_ReadFile(filename, &buffer);
    if (buffer && length != tranmap_length) {
      Z_Free(buffer);
      buffer = NULL;
    }

    if (!buffer) {
      buffer = dsda_GenerateTranMap(alpha);

      M_WriteFile(filename, buffer, tranmap_length);
    }

    tranmap_data[alpha] = buffer;
  }

  return tranmap_data[alpha];
}

const byte* dsda_DefaultTranMap(void) {
  int lump;

  lump = W_CheckNumForName("TRANMAP");

  if (lump != LUMP_NOT_FOUND)
    return W_LumpByNum(lump);

  return dsda_TranMap(default_tranmap_alpha);
}
