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
//	DSDA Ghost
//

#ifndef __DSDA_GHOST__
#define __DSDA_GHOST__

void dsda_InitGhostExport(const char* name);
void dsda_InitGhostImport(const char** ghost_names, int count);
void dsda_ExportGhostFrame(void);
void dsda_SpawnGhost(void);
void dsda_UpdateGhosts(void* _void);

#endif
