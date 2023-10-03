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
//	DSDA SFX
//

#ifndef __DSDA_SFX__
#define __DSDA_SFX__

#include "sounds.h"

int dsda_GetDehSFXIndex(const char* key, size_t length);
int dsda_GetOriginalSFXIndex(const char* key);
sfxinfo_t* dsda_GetDehSFX(int index);
void dsda_InitializeSFX(sfxinfo_t* source, int count);
void dsda_FreeDehSFX(void);
dboolean dsda_BlockSFX(sfxinfo_t *sfx);

#endif
