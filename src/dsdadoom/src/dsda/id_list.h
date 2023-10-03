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
//	DSDA ID List
//

#ifndef __DSDA_ID_LIST__
#define __DSDA_ID_LIST__

#include "r_main.h"

void dsda_AddLineID(int id, int value);
void dsda_AddSectorID(int id, int value);
const int* dsda_FindLinesFromID(int id);
const int* dsda_FindSectorsFromID(int id);
const int* dsda_FindSectorsFromIDOrLine(int id, const line_t *line);
void dsda_ResetLineIDList(int size);
void dsda_ResetSectorIDList(int size);

#define FIND_SECTORS(id_p, tag) for (id_p = dsda_FindSectorsFromID(tag); *id_p >= 0; id_p++)
#define FIND_SECTORS2(id_p, tag, line) for (id_p = dsda_FindSectorsFromIDOrLine(tag, line); *id_p >= 0; id_p++)

#endif
