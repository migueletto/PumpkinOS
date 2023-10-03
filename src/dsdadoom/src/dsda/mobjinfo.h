//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Mobj Info
//

#ifndef __DSDA_MOBJ_INFO__
#define __DSDA_MOBJ_INFO__

#include "info.h"

#include "dsda/deh_hash.h"

typedef struct {
  mobjinfo_t* info;
  byte* edited_bits;
} dsda_deh_mobjinfo_t;

int dsda_FindDehMobjIndex(int index);
int dsda_TranslateDehMobjIndex(int index);
int dsda_GetDehMobjIndex(int index);
dsda_deh_mobjinfo_t dsda_GetDehMobjInfo(int index);
void dsda_InitializeMobjInfo(int zero, int max, int count);
void dsda_FreeDehMobjInfo(void);
void dsda_AppendZDoomMobjInfo(void);

#endif
