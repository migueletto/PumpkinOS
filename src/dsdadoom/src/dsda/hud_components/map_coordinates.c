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
//	DSDA Map Coordinates HUD Component
//

#include "base.h"

#include "map_coordinates.h"

typedef struct {
  dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  snprintf(
    str,
    max_size,
    "%sX: %-5d\n"
    "Y: %-5d\n"
    "Z: %-5d",
    dsda_TextColor(dsda_tc_map_coords),
    (players[displayplayer].mo->x >> FRACBITS),
    (players[displayplayer].mo->y >> FRACBITS),
    (players[displayplayer].mo->z >> FRACBITS)
  );
}

void dsda_InitMapCoordinatesHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateMapCoordinatesHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawMapCoordinatesHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
