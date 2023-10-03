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
//	DSDA Line Distance Tracker HUD Component
//

#include "base.h"

#include "line_distance_tracker.h"

void dsda_LineDistanceTrackerHC(char* str, size_t max_size, int id) {
  line_t* line;
  mobj_t* mo;
  double distance;
  double radius;

  line = &lines[id];
  mo = players[displayplayer].mo;
  radius = (double) mo->radius / FRACUNIT;
  distance = dsda_DistancePointToLine(line->v1->x, line->v1->y, line->v2->x, line->v2->y,
                                      mo->x, mo->y);

  snprintf(
    str,
    max_size,
    "%sld %d: %.03f",
    distance < radius ? dsda_TextColor(dsda_tc_exhud_line_close) :
                        dsda_TextColor(dsda_tc_exhud_line_far),
    id,
    distance
  );
}
