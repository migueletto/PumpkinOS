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
//	DSDA CR Table
//

#include "doomdef.h"
#include "lprintf.h"
#include "w_wad.h"
#include "v_video.h"

#include "dsda/palette.h"
#include "dsda/utility.h"

#include "cr_table.h"

typedef struct {
  int r1, g1, b1;
  int r2, g2, b2;
} cr_range_t;

// Default values - overridden by DSDACR lump
cr_range_t cr_range[CR_LIMIT] = {
  [CR_DEFAULT]   = { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF },
  [CR_BRICK]     = { 0x47, 0x00, 0x00, 0xFF, 0xB8, 0xB8 },
  [CR_TAN]       = { 0x33, 0x2B, 0x13, 0xFF, 0xEB, 0xDF },
  [CR_GRAY]      = { 0x27, 0x27, 0x27, 0xEF, 0xEF, 0xEF },
  [CR_GREEN]     = { 0x0B, 0x17, 0x07, 0x77, 0xFF, 0x6F },
  [CR_BROWN]     = { 0x53, 0x3F, 0x2F, 0xBF, 0xA7, 0x8F },
  [CR_GOLD]      = { 0x73, 0x2B, 0x00, 0xFF, 0xFF, 0x73 },
  [CR_RED]       = { 0x3F, 0x00, 0x00, 0xFF, 0x00, 0x00 },
  [CR_BLUE]      = { 0x00, 0x00, 0x27, 0x00, 0x00, 0xFF },
  [CR_ORANGE]    = { 0x20, 0x00, 0x00, 0xFF, 0x80, 0x00 },
  [CR_YELLOW]    = { 0x77, 0x77, 0x00, 0xFF, 0xFF, 0x00 },
  [CR_LIGHTBLUE] = { 0x00, 0x00, 0x73, 0xB4, 0xB4, 0xFF },
  [CR_BLACK]     = { 0x13, 0x13, 0x13, 0x50, 0x50, 0x50 },
  [CR_PURPLE]    = { 0x23, 0x00, 0x23, 0xCF, 0x00, 0xCF },
  [CR_WHITE]     = { 0x24, 0x24, 0x24, 0xFF, 0xFF, 0xFF },
};

static char ref_lump_doom[9] = "STCFN065";
static char ref_lump_heretic[9] = "FONTA33";
static char ref_lump_hexen[9] = "FONTA33";

typedef struct {
  double light_lower_bound;
  double light_upper_bound;
  double multiplier;
} cr_font_t;

static cr_font_t cr_font = {
  .light_lower_bound = 1.0,
  .light_upper_bound = 0.0,
  .multiplier = 1.0,
};

static void dsda_RegisterFontLightness(double lightness) {
  if (cr_font.light_lower_bound > lightness)
    cr_font.light_lower_bound = lightness;

  if (cr_font.light_upper_bound < lightness)
    cr_font.light_upper_bound = lightness;
}

static void dsda_CalculateFontBounds(const char *playpal) {
  int i, j;
  const byte* lump;
  const byte* p;
  short width;
  byte length;
  byte entry;
  double lightness;

  if (heretic)
    lump = W_LumpByName(ref_lump_heretic);
  else if (hexen)
    lump = W_LumpByName(ref_lump_hexen);
  else
    lump = W_LumpByName(ref_lump_doom);

  width = *((const int16_t *) lump);
  width = LittleShort(width);

  for (i = 0; i < width; ++i) {
    int32_t offset;
    p = lump + 8 + 4 * i;
    offset = *((const int32_t *) p);
    p = lump + LittleLong(offset);

    while (*p != 0xff) {
      p++;
      length = *p++;
      p++;

      for (j = 0; j < length; ++j) {
        entry = *p++;
        lightness = dsda_PaletteEntryLightness((const char *)playpal, entry) / 100.0;
        dsda_RegisterFontLightness(lightness);
      }

      p++;
    }
  }

  cr_font.multiplier = 1.0 / (cr_font.light_upper_bound - cr_font.light_lower_bound);

  lprintf(LO_DEBUG, "Font Bounds: %lf:%lf x%lf\n",
          cr_font.light_lower_bound, cr_font.light_upper_bound, cr_font.multiplier);
}

static void dsda_LoadCRLump(void) {
  char* lump;
  char** lines;
  const char* line;
  int line_i;
  int i, r1, g1, b1, r2, g2, b2;

  lump = W_ReadLumpToString(W_GetNumForName("DSDACR"));

  lines = dsda_SplitString(lump, "\n\r");

  if (lines[0])
    sscanf(lines[0], "%8s %8s %8s", ref_lump_doom, ref_lump_heretic, ref_lump_hexen);

  for (line_i = 1; lines[line_i]; ++line_i) {
    line = lines[line_i];

    if (sscanf(line, "%d %i %i %i %i %i %i", &i, &r1, &g1, &b1, &r2, &g2, &b2) != 7)
      I_Error("DSDACR lump has unknown format!");

    if (i < 1 || i >= CR_LIMIT)
      I_Error("DSDACR index %d is out of bounds!", i);

    if (r1 < 0 || g1 < 0 || b1 < 0 || r2 < 0 || g2 < 0 || b2 < 0 ||
        r1 > 255 || g1 > 255 || b1 > 255 || r2 > 255 || g2 > 255 || b2 > 255)
      I_Error("DSDACR index %d has color out of range (0-255)", i);

    cr_range[i].r1 = r1;
    cr_range[i].g1 = g1;
    cr_range[i].b1 = b1;
    cr_range[i].r2 = r2;
    cr_range[i].g2 = g2;
    cr_range[i].b2 = b2;
  }

  Z_Free(lines);
  Z_Free(lump);
}

byte* dsda_GenerateCRTable(void) {
  int cr_i;
  int orig_i;
  int check_i;
  byte* buffer;
  const byte* playpal;

  dsda_LoadCRLump();

  playpal = W_LumpByName("PLAYPAL");

  buffer = Z_Malloc(256 * CR_LIMIT);

  dsda_CalculateFontBounds((const char *)playpal);

  for (orig_i = 0; orig_i < 256; ++orig_i) {
    double length;

    length = dsda_PaletteEntryLightness((const char *)playpal, orig_i) / 100.0;
    length -= cr_font.light_lower_bound;
    length *= cr_font.multiplier;
    if (length < 0)
      length = 0;
    if (length > 1)
      length = 1;

    // This is the exhud font bright value
    if (orig_i == 176)
      length = 1;

    for (cr_i = 0; cr_i < CR_LIMIT; ++cr_i) {
      int target_r, target_g, target_b;
      int best_i = 0;
      int best_dist = INT_MAX;

      target_r = cr_range[cr_i].r1 +
                 (int) (length * (cr_range[cr_i].r2 - cr_range[cr_i].r1));
      target_g = cr_range[cr_i].g1 +
                 (int) (length * (cr_range[cr_i].g2 - cr_range[cr_i].g1));
      target_b = cr_range[cr_i].b1 +
                 (int) (length * (cr_range[cr_i].b2 - cr_range[cr_i].b1));

      for (check_i = 0; check_i < 768; check_i += 3) {
        int dist;
        int dist_r, dist_g, dist_b;
        int avg_r;

        avg_r = (target_r + playpal[check_i + 0]) / 2;
        dist_r = target_r - playpal[check_i + 0];
        dist_g = target_g - playpal[check_i + 1];
        dist_b = target_b - playpal[check_i + 2];

        // This equation seems to fix issues with red-dominant translation,
        // e.g., aaliens CR_BRICK, which has artifacts in the second equation.
        //
        // I experimented with more "sophisticated" approaches,
        // but they don't seem to do well with common palettes.
        if (target_r > target_g && target_r > target_b)
          dist = (((512 + avg_r) * dist_r * dist_r) >> 8) +
                 4 * dist_g * dist_g +
                 (((767 - avg_r) * dist_b * dist_b) >> 8);
        else
          dist = dist_r * dist_r +
                 dist_g * dist_g +
                 dist_b * dist_b;

        if (dist < best_dist) {
          best_dist = dist;
          best_i = check_i / 3;
        }
      }

      buffer[cr_i * 256 + orig_i] = best_i;
    }
  }

  return buffer;
}
