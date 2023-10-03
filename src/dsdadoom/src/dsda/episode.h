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
//	DSDA Episode
//

#ifndef __DSDA_EPISODE__
#define __DSDA_EPISODE__

#include "doomtype.h"

typedef struct {
  char* map_lump;
  char* name;
  char* pic_name;
  char key;
  dboolean vanilla;
  int start_map;
  int start_episode;
} dsda_episode_t;

extern dsda_episode_t* episodes;
extern size_t num_episodes;

void dsda_AddOriginalEpisodes(void);
void dsda_AddCustomEpisodes(void);
void dsda_ClearEpisodes(void);
void dsda_AddEpisode(const char* map_lump, const char* name,
                     const char* pic_name, char key, dboolean vanilla);

#endif
