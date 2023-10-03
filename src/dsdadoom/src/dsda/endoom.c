//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Endoom
//

#include "doomdef.h"
#include "doomtype.h"
#include "lprintf.h"
#include "w_wad.h"

#include "dsda/configuration.h"

#include "endoom.h"

static const char* cp437_to_utf8[256] = {
  " ",
  "\xe2\x98\xba",
  "\xe2\x98\xbb",
  "\xe2\x99\xa5",
  "\xe2\x99\xa6",
  "\xe2\x99\xa3",
  "\xe2\x99\xa0",
  "\xe2\x80\xa2",
  "\xe2\x97\x98",
  "\xe2\x97\x8b",
  "\xe2\x97\x99",
  "\xe2\x99\x82",
  "\xe2\x99\x80",
  "\xe2\x99\xaa",
  "\xe2\x99\xab",
  "\xe2\x98\xbc",

  "\xe2\x96\xba",
  "\xe2\x97\x84",
  "\xe2\x86\x95",
  "\xe2\x80\xbc",
  "\xc2\xb6",
  "\xc2\xa7",
  "\xe2\x96\xac",
  "\xe2\x86\xa8",
  "\xe2\x86\x91",
  "\xe2\x86\x93",
  "\xe2\x86\x92",
  "\xe2\x86\x90",
  "\xe2\x88\x9f",
  "\xe2\x86\x94",
  "\xe2\x96\xb2",
  "\xe2\x96\xbc",

  " ",
  "!",
  "\"",
  "#",
  "$",
  "%",
  "&",
  "'",
  "(",
  ")",
  "*",
  "+",
  ",",
  "-",
  ".",
  "/",

  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  ":",
  ";",
  "<",
  "=",
  ">",
  "?",

  "@",
  "A",
  "B",
  "C",
  "D",
  "E",
  "F",
  "G",
  "H",
  "I",
  "J",
  "K",
  "L",
  "M",
  "N",
  "O",

  "P",
  "Q",
  "R",
  "S",
  "T",
  "U",
  "V",
  "W",
  "X",
  "Y",
  "Z",
  "[",
  "\\",
  "]",
  "^",
  "_",

  "`",
  "a",
  "b",
  "c",
  "d",
  "e",
  "f",
  "g",
  "h",
  "i",
  "j",
  "k",
  "l",
  "m",
  "n",
  "o",

  "p",
  "q",
  "r",
  "s",
  "t",
  "u",
  "v",
  "w",
  "x",
  "y",
  "z",
  "{",
  "|",
  "}",
  "~",
  "\xe2\x8c\x82",

  "\xc3\x87",
  "\xc3\xbc",
  "\xc3\xa9",
  "\xc3\xa2",
  "\xc3\xa4",
  "\xc3\xa0",
  "\xc3\xa5",
  "\xc3\xa7",
  "\xc3\xaa",
  "\xc3\xab",
  "\xc3\xa8",
  "\xc3\xaf",
  "\xc3\xae",
  "\xc3\xac",
  "\xc3\x84",
  "\xc3\x85",

  "\xc3\x89",
  "\xc3\xa6",
  "\xc3\x86",
  "\xc3\xb4",
  "\xc3\xb6",
  "\xc3\xb2",
  "\xc3\xbb",
  "\xc3\xb9",
  "\xc3\xbf",
  "\xc3\x96",
  "\xc3\x9c	",
  "\xc2\xa2",
  "\xc2\xa3",
  "\xc2\xa5",
  "\xe2\x82\xa7",
  "\xc6\x92",

  "\xc3\xa1",
  "\xc3\xad",
  "\xc3\xb3",
  "\xc3\xba",
  "\xc3\xb1",
  "\xc3\x91",
  "\xc2\xaa",
  "\xc2\xba",
  "\xc2\xbf",
  "\xe2\x8c\x90",
  "\xc2\xac",
  "\xc2\xbd",
  "\xc2\xbc",
  "\xc2\xa1",
  "\xc2\xab",
  "\xc2\xbb",

  "\xe2\x96\x91",
  "\xe2\x96\x92",
  "\xe2\x96\x93",
  "\xe2\x94\x82",
  "\xe2\x94\xa4",
  "\xe2\x95\xa1",
  "\xe2\x95\xa2",
  "\xe2\x95\x96",
  "\xe2\x95\x95",
  "\xe2\x95\xa3",
  "\xe2\x95\x91",
  "\xe2\x95\x97",
  "\xe2\x95\x9d",
  "\xe2\x95\x9c",
  "\xe2\x95\x9b",
  "\xe2\x94\x90",

  "\xe2\x94\x94",
  "\xe2\x94\xb4",
  "\xe2\x94\xac",
  "\xe2\x94\x9c",
  "\xe2\x94\x80",
  "\xe2\x94\xbc",
  "\xe2\x95\x9e",
  "\xe2\x95\x9f",
  "\xe2\x95\x9a",
  "\xe2\x95\x94",
  "\xe2\x95\xa9",
  "\xe2\x95\xa6",
  "\xe2\x95\xa0",
  "\xe2\x95\x90",
  "\xe2\x95\xac",
  "\xe2\x95\xa7",

  "\xe2\x95\xa8",
  "\xe2\x95\xa4",
  "\xe2\x95\xa5",
  "\xe2\x95\x99",
  "\xe2\x95\x98",
  "\xe2\x95\x92",
  "\xe2\x95\x93",
  "\xe2\x95\xab",
  "\xe2\x95\xaa",
  "\xe2\x94\x98",
  "\xe2\x94\x8c",
  "\xe2\x96\x88",
  "\xe2\x96\x84",
  "\xe2\x96\x8c",
  "\xe2\x96\x90",
  "\xe2\x96\x80",

  "\xce\xb1",
  "\xc3\x9f",
  "\xce\x93",
  "\xcf\x80",
  "\xce\xa3",
  "\xcf\x83",
  "\xc2\xb5",
  "\xcf\x84",
  "\xce\xa6",
  "\xce\x98",
  "\xce\xa9",
  "\xce\xb4",
  "\xe2\x88\x9e",
  "\xcf\x86",
  "\xce\xb5",
  "\xe2\x88\xa9",

  "\xe2\x89\xa1",
  "\xc2\xb1",
  "\xe2\x89\xa5",
  "\xe2\x89\xa4",
  "\xe2\x8c\xa0",
  "\xe2\x8c\xa1",
  "\xc3\xb7",
  "\xe2\x89\x88",
  "\xc2\xb0",
  "\xe2\x88\x99",
  "\xc2\xb7",
  "\xe2\x88\x9a",
  "\xe2\x81\xbf",
  "\xc2\xb2",
  "\xe2\x96\xa0",
  "\xc2\xa0",
};

typedef enum {
  format_null,
  format_cp437,
  format_utf8,
} output_format_t;

static byte* endoom;
static output_format_t output_format;

void dsda_CacheEndoom(void) {
  int lump;

  output_format = dsda_IntConfig(dsda_config_ansi_endoom);

  if (!output_format)
    return;

  if (hexen)
    return;

  if (heretic)
    lump = W_CheckNumForName("ENDTEXT");
  else {
    lump = W_CheckNumForName("ENDBOOM");
    if (lump == LUMP_NOT_FOUND)
      lump = W_CheckNumForName("ENDOOM");
  }

  if (lump == LUMP_NOT_FOUND || W_LumpLength(lump) != 4000)
    return;

  endoom = Z_Malloc(4000);
  memcpy(endoom, W_LumpByNum(lump), 4000);
}

void dsda_DumpEndoom(void) {
  if (endoom) {
    int i;
    const char* color_lookup[] = {
      "0", "4", "2", "6", "1", "5", "3", "7",
      "0;1", "4;1", "2;1", "6;1", "1;1", "5;1", "3;1", "7;1",
    };

    for (i = 0; i < 2000; ++i) {
      byte character;
      byte data;
      const char *foreground;
      const char *background;
      const char *blink;

      character = endoom[i * 2];
      data = endoom[i * 2 + 1];
      foreground = color_lookup[data & 0x0f];
      background = color_lookup[(data >> 4) & 0x07];
      blink = ((data >> 7) & 0x01) ? "5" : "25";

      if (!character)
        character = ' ';

      if (output_format == format_utf8)
        lprintf(LO_INFO, "\033[3%sm\033[4%sm\033[%sm%s\033[0m",
                foreground, background, blink, cp437_to_utf8[character]);
      else
        lprintf(LO_INFO, "\033[3%sm\033[4%sm\033[%sm%c\033[0m",
                foreground, background, blink, character);

      if ((i + 1) % 80 == 0)
        lprintf(LO_INFO, "\n");
    }

    lprintf(LO_INFO, "\n");

    Z_Free(endoom);
    endoom = NULL;
  }
}
