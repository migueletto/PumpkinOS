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
//	DSDA Line Tracker HUD Component
//

#include "base.h"

#include "line_tracker.h"

void dsda_LineTrackerHC(char* str, size_t max_size, int id) {
  snprintf(
    str,
    max_size,
    "%sl %d: %d %d",
    lines[id].special ? dsda_TextColor(dsda_tc_exhud_line_special) :
                        dsda_TextColor(dsda_tc_exhud_line_normal),
    id,
    lines[id].special,
    lines[id].player_activations
  );
}
