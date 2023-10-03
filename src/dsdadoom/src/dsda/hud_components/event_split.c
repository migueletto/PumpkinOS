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
//	DSDA Event Split HUD Component
//

#include "base.h"

#include "event_split.h"

typedef struct {
  const char* msg;
  int default_delay;
  int delay;
} dsda_split_state_t;

static dsda_split_state_t dsda_split_state[DSDA_SPLIT_CLASS_COUNT] = {
  [DSDA_SPLIT_BLUE_KEY] = { "Blue Key", 0, 0 },
  [DSDA_SPLIT_YELLOW_KEY] = { "Yellow Key", 0, 0 },
  [DSDA_SPLIT_RED_KEY] = { "Red Key", 0, 0 },
  [DSDA_SPLIT_USE] = { "Use", 2, 0 },
  [DSDA_SPLIT_SECRET] = { "Secret", 0, 0 },
};

typedef struct {
  dsda_text_t component;
} local_component_t;

static local_component_t* local;

static int ticks;

void dsda_InitEventSplitHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_AddSplit(dsda_split_class_t split_class, int lifetime) {
  int minutes;
  float seconds;
  dsda_split_state_t* split_state;

  split_state = &dsda_split_state[split_class];

  if (split_state->delay > 0) {
    split_state->delay = split_state->default_delay;
    return;
  }

  split_state->delay = split_state->default_delay;

  ticks = lifetime;

  // To match the timer, we use the leveltime value at the end of the frame
  minutes = (leveltime + 1) / 35 / 60;
  seconds = (float)((leveltime + 1) % (60 * 35)) / 35;
  snprintf(
    local->component.msg, sizeof(local->component.msg), "%s%d:%05.2f - %s",
    dsda_TextColor(dsda_tc_exhud_event_split),
    minutes, seconds, split_state->msg
  );

  dsda_RefreshHudText(&local->component);
}

void dsda_UpdateEventSplitHC(void* data) {
  int i;

  local = data;

  if (ticks > 0)
    --ticks;

  for (i = 0; i < DSDA_SPLIT_CLASS_COUNT; ++i)
    if (dsda_split_state[i].delay > 0)
      --dsda_split_state[i].delay;
}

void dsda_DrawEventSplitHC(void* data) {
  local = data;

  if (ticks > 0)
    dsda_DrawBasicText(&local->component);
}
