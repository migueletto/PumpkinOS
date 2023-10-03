//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

#ifndef __HEXEN_A_ACTION__
#define __HEXEN_A_ACTION__

#include "p_mobj.h"

extern int localQuakeHappening[MAX_MAXPLAYERS];

dboolean A_LocalQuake(byte * args, mobj_t * victim);
void P_SpawnDirt(mobj_t * actor, fixed_t radius);
void A_BridgeRemove(mobj_t * actor);

#endif
