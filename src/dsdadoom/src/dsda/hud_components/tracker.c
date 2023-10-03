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
//	DSDA Tracker HUD Component
//

#include "dsda/tracker.h"

#include "base.h"

#include "line_distance_tracker.h"
#include "line_tracker.h"
#include "mobj_tracker.h"
#include "null.h"
#include "player_tracker.h"
#include "sector_tracker.h"

#include "tracker.h"

extern dsda_tracker_t dsda_tracker[TRACKER_LIMIT];

typedef struct {
  dsda_text_t component[TRACKER_LIMIT];
} local_component_t;

static local_component_t* local;

void dsda_InitTrackerHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  int i;

  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    dsda_InitTextHC(&local->component[i], x_offset, y_offset + i * 8, vpt);
}

void dsda_UpdateTrackerHC(void* data) {
  int i;

  local = data;

  for (i = 0; i < TRACKER_LIMIT; ++i) {
    switch (dsda_tracker[i].type) {
      case dsda_tracker_nothing:
        dsda_NullHC(local->component[i].msg, sizeof(local->component[i].msg));
        break;
      case dsda_tracker_line:
        dsda_LineTrackerHC(local->component[i].msg, sizeof(local->component[i].msg), dsda_tracker[i].id);
        break;
      case dsda_tracker_line_distance:
        dsda_LineDistanceTrackerHC(local->component[i].msg, sizeof(local->component[i].msg), dsda_tracker[i].id);
        break;
      case dsda_tracker_sector:
        dsda_SectorTrackerHC(local->component[i].msg, sizeof(local->component[i].msg), dsda_tracker[i].id);
        break;
      case dsda_tracker_mobj:
        dsda_MobjTrackerHC(local->component[i].msg, sizeof(local->component[i].msg), dsda_tracker[i].id, dsda_tracker[i].mobj);
        break;
      case dsda_tracker_player:
        dsda_PlayerTrackerHC(local->component[i].msg, sizeof(local->component[i].msg));
        break;
    }

    dsda_RefreshHudText(&local->component[i]);
  }
}

void dsda_DrawTrackerHC(void* data) {
  int i;

  local = data;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    dsda_DrawBasicText(&local->component[i]);
}
