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
//	DSDA Level Splits HUD Component
//

#include "dsda/split_tracker.h"

#include "base.h"

#include "level_splits.h"

typedef struct {
  dsda_text_t time_component;
  dsda_text_t total_component;
} local_component_t;

static local_component_t* local;

extern int leveltime, totalleveltimes;

static int dsda_SplitComparisonDelta(dsda_split_time_t* split_time) {
  return split_time->ref != -1 ? split_time->ref_delta : split_time->best_delta;
}

static void dsda_UpdateIntermissionTime(dsda_split_t* split) {
  char delta[18];
  const char* color;

  delta[0] = '\0';
  color = dsda_TextColor(dsda_tc_inter_split_normal);

  if (split && !split->first_time) {
    const char* sign;
    int diff;

    diff = dsda_SplitComparisonDelta(&split->leveltime);
    sign = diff >= 0 ? "+" : "-";
    color = diff >= 0 ? dsda_TextColor(dsda_tc_inter_split_normal) :
                        dsda_TextColor(dsda_tc_inter_split_good);
    diff = abs(diff);

    if (diff >= 2100) {
      snprintf(
        delta, sizeof(delta),
        " (%s%d:%05.2f)",
        sign, diff / 35 / 60, (float)(diff % (60 * 35)) / 35
      );
    }
    else {
      snprintf(
        delta, sizeof(delta),
        " (%s%04.2f)",
        sign, (float)(diff % (60 * 35)) / 35
      );
    }
  }

  snprintf(
    local->time_component.msg,
    sizeof(local->time_component.msg),
    "%s%d:%05.2f",
    color, leveltime / 35 / 60,
    (float)(leveltime % (60 * 35)) / 35
  );

  strcat(local->time_component.msg, delta);

  dsda_RefreshHudText(&local->time_component);
}

static void dsda_UpdateIntermissionTotal(dsda_split_t* split) {
  char delta[16];
  const char* color;

  delta[0] = '\0';
  color = dsda_TextColor(dsda_tc_inter_split_normal);

  if (split && !split->first_time) {
    const char* sign;
    int diff;

    diff = dsda_SplitComparisonDelta(&split->totalleveltimes) / 35;
    sign = diff >= 0 ? "+" : "-";
    color = diff >= 0 ? dsda_TextColor(dsda_tc_inter_split_normal) :
                        dsda_TextColor(dsda_tc_inter_split_good);
    diff = abs(diff);

    if (diff >= 60) {
      snprintf(
        delta, sizeof(delta),
        " (%s%d:%02d)",
        sign, diff / 60, diff % 60
      );
    }
    else {
      snprintf(
        delta, sizeof(delta),
        " (%s%d)",
        sign, diff % 60
      );
    }
  }

  snprintf(
    local->total_component.msg,
    sizeof(local->total_component.msg),
    "%s%d:%02d",
    color, totalleveltimes / 35 / 60,
    (totalleveltimes / 35) % 60
  );

  strcat(local->total_component.msg, delta);

  dsda_RefreshHudText(&local->total_component);
}

void dsda_InitLevelSplitsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitTextHC(&local->time_component, x_offset, y_offset, vpt);
  dsda_InitTextHC(&local->total_component, x_offset, y_offset + 8, vpt);
}

void dsda_UpdateLevelSplitsHC(void* data) {
  local = data;
}

void dsda_DrawLevelSplitsHC(void* data) {
  dsda_split_t* split;

  local = data;

  split = dsda_CurrentSplit();

  dsda_UpdateIntermissionTime(split);
  dsda_UpdateIntermissionTotal(split);

  dsda_DrawBasicText(&local->time_component);
  dsda_DrawBasicText(&local->total_component);
}
