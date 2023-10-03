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
//	DSDA Speed Text HUD Component
//

#include "base.h"

#include "speed_text.h"

typedef struct {
  dsda_text_t component;
  char label[9];
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int speed;

  speed = dsda_GameSpeed();

  snprintf(
    str,
    max_size,
    "%s%s%d%%",
    local->label,
    speed < 100 ? dsda_TextColor(dsda_tc_exhud_speed_slow)
                : speed == 100 ? dsda_TextColor(dsda_tc_exhud_speed_normal)
                               : dsda_TextColor(dsda_tc_exhud_speed_fast),
    speed
  );
}

void dsda_InitSpeedTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  if (arg_count < 1 || args[0])
    snprintf(local->label, sizeof(local->label), "%sSPEED ", dsda_TextColor(dsda_tc_exhud_speed_label));
  else
    local->label[0] = '\0';

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateSpeedTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawSpeedTextHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
