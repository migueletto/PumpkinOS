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
//	DSDA Font
//

#include "r_data.h"

#include "font.h"

static patchnum_t hu_font[HU_FONTSIZE];
static patchnum_t hu_font2[HU_FONTSIZE];

dsda_font_t hud_font;
dsda_font_t exhud_font;

void dsda_InitFont(void) {
  int i;
  int j;
  char buffer[9];

  j = HU_FONTSTART;
  for (i = 0; i < HU_FONTSIZE - 1; i++, j++) {
    sprintf(buffer, "DIG%.3d", j);
    R_SetPatchNum(&hu_font2[i], buffer);
  }

  j = HU_FONTSTART;
  for (i = 0; i < HU_FONTSIZE; ++i, ++j) {
    if ('0' <= j && j <= '9') {
      if (raven)
        sprintf(buffer, "FONTA%.2d", j - 32);
      else
        sprintf(buffer, "STCFN%.3d", j);
      R_SetPatchNum(&hu_font[i], buffer);
    }
    else if ('A' <= j && j <= 'Z') {
      if (raven)
        sprintf(buffer, "FONTA%.2d", j - 32);
      else
        sprintf(buffer, "STCFN%.3d", j);
      R_SetPatchNum(&hu_font[i], buffer);
    }
    else if (!raven && j < 97) {
      sprintf(buffer, "STCFN%.3d", j);
      R_SetPatchNum(&hu_font[i], buffer);
      //jff 2/23/98 make all font chars defined, useful or not
    }
    else if (raven && j < 91) {
      sprintf(buffer, "FONTA%.2d", j - 32);
      R_SetPatchNum(&hu_font[i], buffer);
      //jff 2/23/98 make all font chars defined, useful or not
    }
    else {
      hu_font[i] = hu_font[0]; //jff 2/16/98 account for gap
    }
  }

  hud_font.font = hu_font;
  hud_font.height = hu_font['0' - HU_FONTSTART].height;
  hud_font.line_height = hud_font.height + 1;
  hud_font.space_width = 4;
  hud_font.start = HU_FONTSTART;

  exhud_font.font = hu_font2;
  exhud_font.height = hu_font2['0' - HU_FONTSTART].height;
  exhud_font.line_height = exhud_font.height + 1;
  exhud_font.space_width = 5;
  exhud_font.start = HU_FONTSTART;
}
