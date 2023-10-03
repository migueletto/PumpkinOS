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

#ifndef __DSDA_TRACKER__
#define __DSDA_TRACKER__

#include "p_mobj.h"

#define TRACKER_LIMIT 16

typedef enum {
  dsda_tracker_nothing,
  dsda_tracker_line,
  dsda_tracker_line_distance,
  dsda_tracker_sector,
  dsda_tracker_mobj,
  dsda_tracker_player,
} dsda_tracker_type_t;

typedef struct {
  dsda_tracker_type_t type;
  int id;
  mobj_t* mobj;
} dsda_tracker_t;

dboolean dsda_TrackLine(int id);
dboolean dsda_UntrackLine(int id);
dboolean dsda_TrackLineDistance(int id);
dboolean dsda_UntrackLineDistance(int id);
dboolean dsda_TrackSector(int id);
dboolean dsda_UntrackSector(int id);
dboolean dsda_TrackMobj(int id);
dboolean dsda_UntrackMobj(int id);
dboolean dsda_TrackPlayer(int id);
dboolean dsda_UntrackPlayer(int id);
void dsda_WipeTrackers(void);
void dsda_ResetTrackers(void);
mobj_t* dsda_FindMobj(int id);

#endif
