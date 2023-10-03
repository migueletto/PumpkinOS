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
//	DSDA Health Text HUD Component
//

#include "base.h"

#include "health_text.h"

typedef struct {
  dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  player_t* player;

  player = &players[displayplayer];

  snprintf(
    str,
    max_size,
    "%sHEL %3d%%",
    player->health <= hud_health_red ? dsda_TextColor(dsda_tc_exhud_health_bad) :
      player->health <= hud_health_yellow ? dsda_TextColor(dsda_tc_exhud_health_warning) :
      player->health <= hud_health_green ? dsda_TextColor(dsda_tc_exhud_health_ok) :
      dsda_TextColor(dsda_tc_exhud_health_super),
    player->health
  );
}

void dsda_InitHealthTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateHealthTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawHealthTextHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
