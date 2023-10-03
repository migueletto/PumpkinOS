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
//	DSDA Weapon Text HUD Component
//

#include "base.h"

#include "weapon_text.h"

typedef struct {
  dsda_text_t component;
  dboolean grid;
  const char* label_w;
  const char* label_p;
  const char* label_n;
  const char* label_wpn;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  player_t* player;

  player = &players[displayplayer];

  if (local->grid)
    snprintf(
      str,
      max_size,
      "%s%s%s%c %s%c %c\n"
      "%s%s%s%c %c %c\n"
      "%s%s%s%c %c %c",
      dsda_TextColor(dsda_tc_exhud_weapon_label),
      local->label_w,
      player->powers[pw_strength] ? dsda_TextColor(dsda_tc_exhud_weapon_berserk) :
                                    dsda_TextColor(dsda_tc_exhud_weapon_owned),
      player->weaponowned[0] ? '1' : ' ',
      dsda_TextColor(dsda_tc_exhud_weapon_owned),
      player->weaponowned[1] ? '2' : ' ',
      player->weaponowned[2] ? '3' : ' ',
      dsda_TextColor(dsda_tc_exhud_weapon_label),
      local->label_p,
      dsda_TextColor(dsda_tc_exhud_weapon_owned),
      player->weaponowned[3] ? '4' : ' ',
      player->weaponowned[4] ? '5' : ' ',
      player->weaponowned[5] ? '6' : ' ',
      dsda_TextColor(dsda_tc_exhud_weapon_label),
      local->label_n,
      dsda_TextColor(dsda_tc_exhud_weapon_owned),
      player->weaponowned[6] ? '7' : ' ',
      player->weaponowned[7] ? '8' : ' ',
      player->weaponowned[8] ? '9' : ' '
    );
  else
    snprintf(
      str,
      max_size,
      "%s%s%s%c %s%c %c %c %c %c %c %c %c",
      dsda_TextColor(dsda_tc_exhud_weapon_label),
      local->label_wpn,
      player->powers[pw_strength] ? dsda_TextColor(dsda_tc_exhud_weapon_berserk) :
                                    dsda_TextColor(dsda_tc_exhud_weapon_owned),
      player->weaponowned[0] ? '1' : ' ',
      dsda_TextColor(dsda_tc_exhud_weapon_owned),
      player->weaponowned[1] ? '2' : ' ',
      player->weaponowned[2] ? '3' : ' ',
      player->weaponowned[3] ? '4' : ' ',
      player->weaponowned[4] ? '5' : ' ',
      player->weaponowned[5] ? '6' : ' ',
      player->weaponowned[6] ? '7' : ' ',
      player->weaponowned[7] ? '8' : ' ',
      player->weaponowned[8] ? '9' : ' '
    );
}

void dsda_InitWeaponTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->grid = args[0];

  if (arg_count < 2 || args[1]) {
    local->label_w = "W ";
    local->label_p = "P ";
    local->label_n = "N ";
    local->label_wpn = "WPN ";
  }
  else {
    local->label_w = "";
    local->label_p = "";
    local->label_n = "";
    local->label_wpn = "";
  }

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateWeaponTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawWeaponTextHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
