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
//	DSDA Map Time HUD Component
//

#include "base.h"

#include "map_time.h"

typedef struct {
  dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int length;
  int level_time;
  int total_time;

  total_time = hexen ?
               players[consoleplayer].worldTimer :
               totalleveltimes + leveltime;
  level_time = leveltime;

  total_time /= 35;
  level_time /= 35;

  length = snprintf(
    str,
    max_size,
    "%s%02d:%02d:%02d\n",
    dsda_TextColor(dsda_tc_map_time_level),
    level_time / 3600,
    (level_time % 3600) / 60,
    level_time % 60
  );

  if (total_time != level_time)
    snprintf(
      str + length,
      max_size - length,
      "%s%02d:%02d:%02d\n",
      dsda_TextColor(dsda_tc_map_time_total),
      total_time / 3600,
      (total_time % 3600) / 60,
      total_time % 60
    );
}

void dsda_InitMapTimeHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateMapTimeHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawMapTimeHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
