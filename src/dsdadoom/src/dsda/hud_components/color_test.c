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
//	DSDA Color Test HUD Component
//

#include "base.h"

#include "color_test.h"

typedef struct {
  dsda_text_t component;
  dsda_text_t component_blocky;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  snprintf(
    str,
    max_size,
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n"
    "\x1b%c %02d TEST\n",
    HUlib_Color(0), 0,
    HUlib_Color(1), 1,
    HUlib_Color(2), 2,
    HUlib_Color(3), 3,
    HUlib_Color(4), 4,
    HUlib_Color(5), 5,
    HUlib_Color(6), 6,
    HUlib_Color(7), 7,
    HUlib_Color(8), 8,
    HUlib_Color(9), 9,
    HUlib_Color(10), 10,
    HUlib_Color(11), 11,
    HUlib_Color(12), 12,
    HUlib_Color(13), 13,
    HUlib_Color(14), 14
  );
}

void dsda_InitColorTestHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
  dsda_InitBlockyHC(&local->component_blocky, x_offset + 64, y_offset, vpt);

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);

  dsda_UpdateComponentText(local->component_blocky.msg, sizeof(local->component_blocky.msg));
  dsda_RefreshHudText(&local->component_blocky);
}

void dsda_UpdateColorTestHC(void* data) {
  // nothing to do
}

void dsda_DrawColorTestHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
  dsda_DrawBasicText(&local->component_blocky);
}
