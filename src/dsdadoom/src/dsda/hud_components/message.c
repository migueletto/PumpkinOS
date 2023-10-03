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
//	DSDA Message HUD Component
//

#include "base.h"

#include "message.h"

typedef struct {
  dsda_text_t component;
  dboolean center;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  char* dsda_PlayerMessage(void);

  char* message;

  message = dsda_PlayerMessage();

  if (message)
    snprintf(
      str,
      max_size,
      "%s%s",
      dsda_TextColor(dsda_tc_hud_message),
      message
    );
  else
    str[0] = '\0';
}

void dsda_InitMessageHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->center = arg_count > 0 ? !!args[0] : false;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateMessageHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);

  if (local->center)
    HUlib_setTextXCenter(&local->component.text);
}

void dsda_DrawMessageHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
