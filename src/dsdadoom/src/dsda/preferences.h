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
//	DSDA Preferences
//

#ifndef __DSDA_PREFERENCES__
#define __DSDA_PREFERENCES__

#include "doomtype.h"

void dsda_LoadWadPreferences(void);
void dsda_HandleMapPreferences(void);
void dsda_PreferOpenGL(void);
void dsda_PreferSoftware(void);
dboolean dsda_UseMapinfo(void);

#endif
