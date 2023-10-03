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
//	DSDA Composite Time HUD Component
//

#include "base.h"

#include "composite_time.h"

typedef struct {
  dsda_text_t component;
  char label[8];
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  extern dboolean dsda_reborn;

  int total_time;
  int length;

  total_time = hexen ?
               players[consoleplayer].worldTimer :
               totalleveltimes + leveltime;

  if (total_time != leveltime)
    length = snprintf(
      str,
      max_size,
      "%s%s%d:%02d %s%d:%05.2f ",
      local->label,
      dsda_TextColor(dsda_tc_exhud_total_time),
      total_time / 35 / 60,
      (total_time % (60 * 35)) / 35,
      dsda_TextColor(dsda_tc_exhud_level_time),
      leveltime / 35 / 60,
      (float) (leveltime % (60 * 35)) / 35
    );
  else
    length = snprintf(
      str,
      max_size,
      "%s%s%d:%05.2f ",
      local->label,
      dsda_TextColor(dsda_tc_exhud_level_time),
      leveltime / 35 / 60,
      (float) (leveltime % (60 * 35)) / 35
    );

  if (dsda_reborn && (demorecording || demoplayback)) {
    int demo_tic = dsda_DemoTic();

    snprintf(
      str + length,
      max_size - length,
      "%s%d:%02d ",
      dsda_TextColor(dsda_tc_exhud_demo_length),
      demo_tic / 35 / 60,
      (demo_tic % (60 * 35)) / 35
    );
  }
}

void dsda_InitCompositeTimeHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  if (arg_count < 1 || args[0])
    snprintf(local->label, sizeof(local->label), "%stime ", dsda_TextColor(dsda_tc_exhud_time_label));
  else
    local->label[0] = '\0';

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateCompositeTimeHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawCompositeTimeHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
