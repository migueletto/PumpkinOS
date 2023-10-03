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
//	DSDA Ready Ammo Text HUD Component
//

#include "base.h"

#include "ready_ammo_text.h"

typedef struct {
  dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  player_t* player;

  player = &players[displayplayer];

  if (hexen) {
    snprintf(str, max_size, "%sAMM %s%3d %s%3d",
             dsda_TextColor(dsda_tc_exhud_ammo_label),
             dsda_TextColor(dsda_tc_exhud_ammo_mana1), player->ammo[0],
             dsda_TextColor(dsda_tc_exhud_ammo_mana2), player->ammo[1]);
  }
  else {
    ammotype_t ammo_type;

    ammo_type = weaponinfo[player->readyweapon].ammo;

    if (ammo_type == am_noammo || !player->maxammo[ammo_type])
      snprintf(str, max_size, "%sAMM %sN/A",
               dsda_TextColor(dsda_tc_exhud_ammo_label),
               dsda_TextColor(dsda_tc_exhud_ammo_value));
    else
      snprintf(str, max_size, "%sAMM %s%3d",
               dsda_TextColor(dsda_tc_exhud_ammo_label),
               dsda_TextColor(dsda_AmmoColor(player)), player->ammo[ammo_type]);
  }
}

void dsda_InitReadyAmmoTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateReadyAmmoTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawReadyAmmoTextHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
