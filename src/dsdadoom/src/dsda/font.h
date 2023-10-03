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

#ifndef __DSDA_FONT__
#define __DSDA_FONT__

#include "r_defs.h"

#define HU_FONTSTART '!'  /* the first font characters */
#define HU_FONTEND (0x7f) /*jff 2/16/98 '_' the last font characters */
#define HU_FONTSIZE (HU_FONTEND - HU_FONTSTART + 1)

typedef struct {
  const patchnum_t *font;
  int height;
  int line_height;
  int space_width;
  int start;
} dsda_font_t;

extern dsda_font_t hud_font;
extern dsda_font_t exhud_font;

void dsda_InitFont(void);

#endif
