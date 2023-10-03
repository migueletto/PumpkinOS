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

#include "doomstat.h"
#include "lprintf.h"
#include "z_zone.h"

#include "dsda/mapinfo.h"
#include "dsda/mapinfo/doom/parser.h"

#include "episode.h"

dsda_episode_t* episodes;
size_t num_episodes;

static void dsda_DetermineEpisodeMap(dsda_episode_t* episode) {
  if (!dsda_NameToMap(episode->map_lump, &episode->start_episode, &episode->start_map))
    I_Error("Cannot evaluate start map for episode %s", episode->name ? episode->name :
                                                        episode->pic_name ? episode->pic_name :
                                                        "UNKNOWN");
}

void dsda_AddOriginalEpisodes(void) {
  if (heretic) {
    dsda_AddEpisode("e1m1", "CITY OF THE DAMNED", NULL, 'c', true);
    dsda_AddEpisode("e2m1", "HELL'S MAW", NULL, 'h', true);
    dsda_AddEpisode("e3m1", "THE DOME OF D'SPARIL", NULL, 't', true);

    if (gamemode == retail) {
      dsda_AddEpisode("e4m1", "THE OSSUARY", NULL, 't', true);
      dsda_AddEpisode("e5m1", "THE STAGNANT DEMESNE", NULL, 't', true);
    }
  }
  else if (hexen) {
    dsda_AddEpisode("map01", "FIGHTER", NULL, 'f', true);
    dsda_AddEpisode("map01", "CLERIC", NULL, 'c', true);
    dsda_AddEpisode("map01", "MAGE", NULL, 'm', true);
  }
  else if (gamemode != commercial && gamemission != chex) {
    dsda_AddEpisode("e1m1", NULL, "M_EPI1", 'k', true);
    dsda_AddEpisode("e2m1", NULL, "M_EPI2", 't', true);
    dsda_AddEpisode("e3m1", NULL, "M_EPI3", 'i', true);

    if (gamemode == retail)
      dsda_AddEpisode("e4m1", NULL, "M_EPI4", 't', true);
  }
}

void dsda_AddCustomEpisodes(void) {
  int i;

  if (!doom_mapinfo.loaded)
    return;

  // TODO: umapinfo edits episodes while parsing the lump,
  // so we add the originals before loading info lumps,
  // but we want to validate existence in the scope of mapinfo,
  // so we start from scratch again...
  dsda_ClearEpisodes();

  if (!doom_mapinfo.episodes_cleared)
    dsda_AddOriginalEpisodes();

  for (i = 0; i < doom_mapinfo.num_episodes; ++i)
    dsda_AddEpisode(
      doom_mapinfo.episodes[i].map_lump,
      doom_mapinfo.episodes[i].name,
      doom_mapinfo.episodes[i].pic_name,
      doom_mapinfo.episodes[i].key,
      false
    );
}

void dsda_ClearEpisodes(void) {
  int i;

  for (i = 0; i < num_episodes; ++i) {
    Z_Free(episodes[i].map_lump);
    Z_Free(episodes[i].name);
    Z_Free(episodes[i].pic_name);
  }

  Z_Free(episodes);
  episodes = NULL;
  num_episodes = 0;
}

void dsda_AddEpisode(const char* map_lump, const char* name,
                     const char* pic_name, char key, dboolean vanilla) {
  ++num_episodes;
  episodes = Z_Realloc(episodes, num_episodes * sizeof(*episodes));

  episodes[num_episodes - 1].map_lump = map_lump ? Z_Strdup(map_lump) : NULL;
  episodes[num_episodes - 1].name = name ? Z_Strdup(name) : NULL;
  episodes[num_episodes - 1].pic_name = pic_name ? Z_Strdup(pic_name) : NULL;
  episodes[num_episodes - 1].key = key;
  episodes[num_episodes - 1].vanilla = vanilla;

  dsda_DetermineEpisodeMap(&episodes[num_episodes - 1]);
}
