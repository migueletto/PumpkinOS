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
//	DSDA Player Tracker HUD Component
//

#include "base.h"

#include "player_tracker.h"

void dsda_PlayerTrackerHC(char* str, size_t max_size) {
  extern int player_damage_last_tic;

  snprintf(
    str,
    max_size,
    "%sp: %d",
    player_damage_last_tic > 0 ? dsda_TextColor(dsda_tc_exhud_player_damage)
                               : dsda_TextColor(dsda_tc_exhud_player_neutral),
    player_damage_last_tic
  );
}
