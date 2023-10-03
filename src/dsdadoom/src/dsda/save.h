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
//	DSDA Save
//

#ifndef __DSDA_SAVE__
#define __DSDA_SAVE__

void dsda_ArchiveAll(void);
void dsda_UnArchiveAll(void);
void dsda_InitSaveDir(void);
char* dsda_SaveGameName(int slot, dboolean via_excmd);
void dsda_ResetDemoSaveSlots(void);
void dsda_SetLastLoadSlot(int slot);
void dsda_SetLastSaveSlot(int slot);
int dsda_LastSaveSlot(void);
void dsda_ResetLastSaveSlot(void);
int dsda_AllowMenuLoad(int slot);
int dsda_AllowAnyMenuLoad(void);

#endif
