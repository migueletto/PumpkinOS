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
//	DSDA FPS HUD Component
//

#include "base.h"

#include "fps.h"

typedef struct {
  dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  extern int dsda_render_stats_fps;

  snprintf(
    str,
    max_size,
    "%s%4d",
    dsda_render_stats_fps < 35 ? dsda_TextColor(dsda_tc_exhud_fps_bad) :
                                 dsda_TextColor(dsda_tc_exhud_fps_fine),
    dsda_render_stats_fps
  );
}

void dsda_InitFPSHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateFPSHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawFPSHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
