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
//	DSDA Tracker
//

#include "doomstat.h"
#include "p_tick.h"
#include "r_state.h"

#include "dsda/args.h"
#include "dsda/features.h"
#include "dsda/settings.h"
#include "dsda/tracker.h"

#include "tracker.h"

dsda_tracker_t dsda_tracker[TRACKER_LIMIT];

static int tracker_map;
static int tracker_episode;

static int dsda_FindTracker(int type, int id) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    if (dsda_tracker[i].type == type && dsda_tracker[i].id == id)
      return i;

  return -1;
}

mobj_t* dsda_FindMobj(int id) {
  thinker_t* th;
  mobj_t* mobj;

  for (th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function != P_MobjThinker)
      continue;

    mobj = (mobj_t*) th;

    if (mobj->index == id)
      return mobj;
  }

  return NULL;
}

static void dsda_WipeTracker(int i) {
  dsda_tracker[i].type = dsda_tracker_nothing;
  dsda_tracker[i].id = 0;
  dsda_tracker[i].mobj = NULL;
}

void dsda_WipeTrackers(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    if (dsda_tracker[i].type != dsda_tracker_player)
      dsda_WipeTracker(i);
}

static void dsda_ConsolidateTrackers(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    if (dsda_tracker[i].type == dsda_tracker_nothing) {
      int j;

      for (j = i + 1; j < TRACKER_LIMIT; ++j)
        if (dsda_tracker[j].type != dsda_tracker_nothing) {
          dsda_tracker[i] = dsda_tracker[j];
          dsda_WipeTracker(j);
          break;
        }

      if (j == TRACKER_LIMIT)
        return;
    }
}

static void dsda_RefreshTrackers(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i) {
    switch (dsda_tracker[i].type) {
      default:
        break;
      case dsda_tracker_mobj:
        dsda_tracker[i].mobj = dsda_FindMobj(dsda_tracker[i].id);
        if (!dsda_tracker[i].mobj)
          dsda_WipeTracker(i);
        break;
    }

    if (dsda_tracker[i].type != dsda_tracker_nothing)
      dsda_TrackFeature(uf_tracker);
  }
}

static void dsda_ParseCommandlineTrackers(int arg_id, dboolean (*track)(int)) {
  dsda_arg_t* arg;

  arg = dsda_Arg(arg_id);
  if (arg->found) {
    int i;

    for (i = 0; i < arg->count; ++i)
      track(arg->value.v_int_array[i]);
  }
}

void dsda_ResetTrackers(void) {
  static dboolean first_time = true;

  if (first_time) {
    first_time = false;

    dsda_ParseCommandlineTrackers(dsda_arg_track_line, dsda_TrackLine);
    dsda_ParseCommandlineTrackers(dsda_arg_track_line_distance, dsda_TrackLineDistance);
    dsda_ParseCommandlineTrackers(dsda_arg_track_sector, dsda_TrackSector);
    dsda_ParseCommandlineTrackers(dsda_arg_track_mobj, dsda_TrackMobj);

    if (dsda_Flag(dsda_arg_track_player))
      dsda_TrackPlayer(0);

    return;
  }

  if (gamemap != tracker_map || gameepisode != tracker_episode)
    dsda_WipeTrackers();
  else
    dsda_RefreshTrackers();

  dsda_ConsolidateTrackers();
}

static dboolean dsda_AddTracker(int type, int id, mobj_t* mobj) {
  int i;

  tracker_map = gamemap;
  tracker_episode = gameepisode;

  if (dsda_FindTracker(type, id) >= 0)
    return false;

  if ((i = dsda_FindTracker(dsda_tracker_nothing, 0)) >= 0) {
    dsda_TrackFeature(uf_tracker);

    dsda_tracker[i].type = type;
    dsda_tracker[i].id = id;
    dsda_tracker[i].mobj = mobj;

    return true;
  }

  return false;
}

static dboolean dsda_RemoveTracker(int type, int id) {
  int i;

  if (dsda_StrictMode())
    return false;

  if ((i = dsda_FindTracker(type, id)) >= 0) {
    dsda_WipeTracker(i);
    dsda_ConsolidateTrackers();

    return true;
  }

  return false;
}

dboolean dsda_TrackLine(int id) {
  if (dsda_StrictMode())
    return false;

  if (id >= numlines || id < 0)
    return false;

  return dsda_AddTracker(dsda_tracker_line, id, NULL);
}

dboolean dsda_UntrackLine(int id) {
  return dsda_RemoveTracker(dsda_tracker_line, id);
}

dboolean dsda_TrackLineDistance(int id) {
  if (dsda_StrictMode())
    return false;

  if (id >= numlines || id < 0)
    return false;

  return dsda_AddTracker(dsda_tracker_line_distance, id, NULL);
}

dboolean dsda_UntrackLineDistance(int id) {
  return dsda_RemoveTracker(dsda_tracker_line_distance, id);
}

dboolean dsda_TrackSector(int id) {
  if (dsda_StrictMode())
    return false;

  if (id >= numsectors || id < 0)
    return false;

  return dsda_AddTracker(dsda_tracker_sector, id, NULL);
}

dboolean dsda_UntrackSector(int id) {
  return dsda_RemoveTracker(dsda_tracker_sector, id);
}

dboolean dsda_TrackMobj(int id) {
  mobj_t* mobj = NULL;

  if (dsda_StrictMode())
    return false;

  mobj = dsda_FindMobj(id);

  if (!mobj)
    return false;

  {
    mobj_t* target = NULL;

    // While a mobj is targeted, its address is preserved
    P_SetTarget(&target, mobj);
  }

  return dsda_AddTracker(dsda_tracker_mobj, id, mobj);
}

dboolean dsda_UntrackMobj(int id) {
  return dsda_RemoveTracker(dsda_tracker_mobj, id);
}

dboolean dsda_TrackPlayer(int id) {
  if (dsda_StrictMode())
    return false;

  return dsda_AddTracker(dsda_tracker_player, id, NULL);
}

dboolean dsda_UntrackPlayer(int id) {
  return dsda_RemoveTracker(dsda_tracker_player, id);
}
