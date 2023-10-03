//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Analysis
//

#ifndef __DSDA_ANALYSIS__
#define __DSDA_ANALYSIS__

#include "doomtype.h"

extern int dsda_analysis;

extern dboolean dsda_pacifist;
extern dboolean dsda_reality;
extern dboolean dsda_almost_reality;
extern dboolean dsda_reborn;
extern int dsda_missed_monsters;
extern int dsda_missed_secrets;
extern int dsda_missed_weapons;
extern dboolean dsda_tyson_weapons;
extern dboolean dsda_100k;
extern dboolean dsda_100s;
extern dboolean dsda_any_counted_monsters;
extern dboolean dsda_any_monsters;
extern dboolean dsda_any_secrets;
extern dboolean dsda_any_weapons;
extern dboolean dsda_stroller;
extern dboolean dsda_nomo;
extern dboolean dsda_respawn;
extern dboolean dsda_fast;
extern dboolean dsda_turbo;
extern dboolean dsda_weapon_collector;

extern int dsda_kills_on_map;
extern dboolean dsda_100k_on_map;
extern dboolean dsda_100k_note_shown;
extern dboolean dsda_pacifist_note_shown;

void dsda_ResetAnalysis(void);
void dsda_WriteAnalysis(void);
const char* dsda_DetectCategory(void);

#endif
