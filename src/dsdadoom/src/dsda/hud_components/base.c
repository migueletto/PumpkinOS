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
//	DSDA HUD Component Base
//

#include <math.h>

#include "base.h"

static char digit_lump[9];
static const char* digit_lump_format;

int dsda_HudComponentY(int y_offset, int vpt, double ratio) {
  int dsda_ExHudVerticalOffset(void);

  int y = 0;
  int vpt_align;

  if (ratio)
    y_offset *= ratio;

  vpt_align = vpt & VPT_ALIGN_MASK;
  if (BOTTOM_ALIGNMENT(vpt_align)) {
    y = 200;
    y_offset = -y_offset;

    y -= dsda_ExHudVerticalOffset();
  }

  return y + y_offset;
}

void dsda_InitTextHC(dsda_text_t* component, int x_offset, int y_offset, int vpt) {
  static double ratio;
  int x, y;

  DO_ONCE
    if (exhud_font.line_height != 8)
      ratio = (double) exhud_font.line_height / 8.0;
  END_ONCE

  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt, ratio);

  HUlib_initTextLine(&component->text, x, y, &exhud_font, CR_GRAY, vpt);
}

void dsda_InitBlockyHC(dsda_text_t* component, int x_offset, int y_offset, int vpt) {
  static double ratio;
  int x, y;

  DO_ONCE
    if (hud_font.line_height != 8)
      ratio = (double) hud_font.line_height / 8.0;
  END_ONCE

  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt, ratio);

  HUlib_initTextLine(&component->text, x, y, &hud_font, CR_GRAY, vpt);
}

void dsda_InitPatchHC(dsda_patch_component_t* component, int x_offset, int y_offset, int vpt) {
  int x, y;

  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt, 0);

  component->x = x;
  component->y = y;
  component->vpt = vpt;

  if (raven)
    digit_lump_format = "IN%.1d";
  else
    digit_lump_format = "STTNUM%.1d";
}

int dsda_HexenArmor(player_t* player) {
  return (pclass[player->pclass].auto_armor_save
          + player->armorpoints[ARMOR_ARMOR]
          + player->armorpoints[ARMOR_SHIELD]
          + player->armorpoints[ARMOR_HELMET]
          + player->armorpoints[ARMOR_AMULET]) >> FRACBITS;
}

static void dsda_DrawBigDigit(int x, int y, int cm, int vpt, int digit) {
  if (digit > 9 || digit < 0)
    return;

  snprintf(digit_lump, sizeof(digit_lump), digit_lump_format, digit);
  V_DrawNamePatch(x, y, FG, digit_lump, cm, vpt | VPT_TRANS);
}

static int digit_mod[6] = { 1, 10, 100, 1000, 10000, 100000 };
static int digit_div[6] = { 1,  1,  10,  100,  1000,  10000 };

void dsda_DrawBigNumber(int x, int y, int delta_x, int delta_y, int cm, int vpt, int count, int n) {
  int i;
  int digit, any_digit;

  if (count > 5)
    return;

  any_digit = 0;

  for (i = count; i > 0; --i) {
    digit = (n % digit_mod[i]) / digit_div[i];
    any_digit |= digit;

    if (any_digit || i == 1)
      dsda_DrawBigDigit(x, y, cm, vpt, digit);

    x += delta_x;
  }
}

void dsda_DrawBasicText(dsda_text_t* component) {
  HUlib_drawTextLine(&component->text, false);
}

void dsda_RefreshHudText(dsda_text_t* component) {
  const char* s;

  HUlib_clearTextLine(&component->text);

  s = component->msg;
  while (*s) HUlib_addCharToTextLine(&component->text, *(s++));
}
