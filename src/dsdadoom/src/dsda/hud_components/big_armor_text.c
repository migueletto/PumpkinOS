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
//	DSDA Big Armor Text HUD Component
//

#include "base.h"

#include "big_armor_text.h"

typedef struct {
  dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

static int patch_delta_x;

static void dsda_DrawComponent(void) {
  player_t* player;
  int cm;
  int armor;

  player = &players[displayplayer];

  if (hexen) {
    armor = dsda_HexenArmor(player);
    cm = dsda_TextCR(dsda_tc_exhud_armor_zero);
  }
  else {
    armor = player->armorpoints[ARMOR_ARMOR];
    if (armor <= 0)
      cm = dsda_TextCR(dsda_tc_exhud_armor_zero);
    else if (player->armortype < 2)
      cm = dsda_TextCR(dsda_tc_exhud_armor_one);
    else
      cm = dsda_TextCR(dsda_tc_exhud_armor_two);
  }

  dsda_DrawBigNumber(local->component.x, local->component.y, patch_delta_x, 0,
                     cm, local->component.vpt, 3, armor);
}

void dsda_InitBigArmorTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  if (heretic)
    patch_delta_x = 10;
  else if (hexen)
    patch_delta_x = 10;
  else
    patch_delta_x = 14;

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArmorTextHC(void* data) {
  local = data;
}

void dsda_DrawBigArmorTextHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
