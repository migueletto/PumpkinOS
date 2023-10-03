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
//	DSDA Music
//

#ifndef __DSDA_MUSIC__
#define __DSDA_MUSIC__

#include "sounds.h"

int dsda_GetDehMusicIndex(const char* key, size_t length);
int dsda_GetOriginalMusicIndex(const char* key);
void dsda_InitializeMusic(musicinfo_t* source, int count);
void dsda_FreeDehMusic(void);

void dsda_ArchiveMusic(void);
void dsda_UnArchiveMusic(void);
dboolean dsda_StartQueuedMusic(void);

#endif
