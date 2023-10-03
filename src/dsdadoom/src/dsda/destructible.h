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
//	DSDA Destructible
//

#ifndef __DSDA_DESTRUCTIBLE__
#define __DSDA_DESTRUCTIBLE__

#include "p_mobj.h"

void dsda_AddLineToHealthGroup(line_t* line);
void dsda_ResetHealthGroups(void);
void dsda_DamageLinedef(line_t* line, mobj_t* source, int damage);
void dsda_RadiusAttackDestructibles(int xl, int xh, int yl, int yh);

#endif
