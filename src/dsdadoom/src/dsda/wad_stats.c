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
//	DSDA Wad Stats
//

#include <stdio.h>

#include "doomstat.h"
#include "lprintf.h"
#include "m_file.h"
#include "w_wad.h"
#include "z_zone.h"

#include "dsda/data_organizer.h"
#include "dsda/skill_info.h"
#include "dsda/utility.h"

#include "wad_stats.h"

static const char* filename = "stats.txt";
static const int current_version = 1;
static map_stats_t* current_map_stats;

wad_stats_t wad_stats;

static void dsda_EnsureMapCount(int count) {
  wad_stats.map_count = count;

  if (wad_stats.maps_size < wad_stats.map_count) {
    int old_size;

    old_size = wad_stats.maps_size;
    while (wad_stats.maps_size < wad_stats.map_count)
      wad_stats.maps_size = wad_stats.maps_size ? wad_stats.maps_size * 2 : 32;
    wad_stats.maps = Z_Realloc(wad_stats.maps, sizeof(*wad_stats.maps) * wad_stats.maps_size);
    memset(wad_stats.maps + old_size,
           0, (wad_stats.maps_size - old_size) * sizeof(*wad_stats.maps));
  }
}

static const char* dsda_WadStatsPath(void) {
  static dsda_string_t path;

  if (!path.string)
    dsda_StringPrintF(&path, "%s/%s", dsda_DataDir(), filename);

  return path.string;
}

static dboolean dsda_MapStatsExist(const char* lump) {
  int i;

  for (i = 0; i < wad_stats.maps_size && wad_stats.maps[i].lump[0]; ++i)
    if (!strncasecmp(wad_stats.maps[i].lump, lump, 8))
      return true;

  return false;
}

static int C_DECL dicmp_map_stats(const void* a, const void* b) {
  const map_stats_t* m1 = (const map_stats_t *) a;
  const map_stats_t* m2 = (const map_stats_t *) b;

  if (m1->episode == -1 && m2->episode == -1)
    return m1->lump[0] - m2->lump[0];

  if (m1->episode == -1)
    return 1;

  if (m2->episode == -1)
    return -1;

  return (m1->episode == m2->episode) ?
         (m1->map == m2->map) ?
         m1->lump[0] - m2->lump[0] : m1->map - m2->map : m1->episode - m2->episode;
}

static void dsda_CreateWadStats(void) {
  int i;
  const char* map_name;
  int map_count = 0;
  dboolean any_pwad_map = false;

  for (i = numlumps - 1; i > 0; --i) {
    if (any_pwad_map && lumpinfo[i].source == source_iwad)
      break;

    if (lumpinfo[i].source != source_iwad &&
        lumpinfo[i].source != source_pwad)
      continue;

    if (strncasecmp(lumpinfo[i].name, "THINGS", 8) &&
        strncasecmp(lumpinfo[i].name, "TEXTMAP", 8))
      continue;

    map_name = lumpinfo[i - 1].name;
    if (dsda_MapStatsExist(map_name))
      continue;

    if (lumpinfo[i - 1].source == source_pwad)
      any_pwad_map = true;

    {
      int episode, map;
      map_stats_t* ms;

      map_count += 1;
      dsda_EnsureMapCount(map_count);
      ms = &wad_stats.maps[map_count - 1];
      memset(ms, 0, sizeof(*wad_stats.maps));

      strcpy(ms->lump, map_name);

      if (sscanf(map_name, "MAP%d", &map) == 1) {
        ms->episode = 1;
        ms->map = map;
      }
      else if (sscanf(map_name, "E%dM%d", &episode, &map) == 2) {
        ms->episode = episode;
        ms->map = map;
      }
      else {
        ms->episode = -1;
        ms->map = -1;
      }

      ms->best_time = -1;
      ms->best_max_time = -1;
      ms->best_sk5_time = -1;
      ms->max_kills = -1;
      ms->max_items = -1;
      ms->max_secrets = -1;
    }
  }

  qsort(wad_stats.maps, wad_stats.map_count, sizeof(*wad_stats.maps), dicmp_map_stats);
}

static void dsda_LoadWadStats(void) {
  const char* path;
  char* buffer;
  char** lines;
  int version;
  int map_count = 0;
  int i;

  path = dsda_WadStatsPath();

  if (M_ReadFileToString(path, &buffer) != -1) {
    lines = dsda_SplitString(buffer, "\n\r");

    if (lines) {
      if (!lines[0] || !lines[1])
        I_Error("Encountered invalid wad stats: %s", path);

      if (sscanf(lines[0], "%d", &version) != 1)
        I_Error("Encountered invalid wad stats: %s", path);

      if (version > current_version)
        I_Error("Encountered unsupported wad stats version: %s", path);

      if (sscanf(lines[1], "%d", &wad_stats.total_kills) != 1)
        I_Error("Encountered invalid wad stats: %s", path);

      for (i = 2; lines[i] && *lines[i]; ++i) {
        map_stats_t ms;

        if (
          sscanf(
            lines[i], "%8s %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
            ms.lump, &ms.episode, &ms.map,
            &ms.best_skill, &ms.best_time, &ms.best_max_time, &ms.best_sk5_time,
            &ms.total_exits, &ms.total_kills,
            &ms.best_kills, &ms.best_items, &ms.best_secrets,
            &ms.max_kills, &ms.max_items, &ms.max_secrets
          ) == 15
        ) {
          map_count += 1;
          dsda_EnsureMapCount(map_count);
          wad_stats.maps[map_count - 1] = ms;
        }
      }

      Z_Free(lines);
    }

    Z_Free(buffer);
  }
}

static dboolean forget_wad_stats;

void M_ForgetWadStats(void)
{
  forget_wad_stats = true;
}

void M_RememberWadStats(void)
{
  forget_wad_stats = false;
}

void dsda_SaveWadStats(void) {
  const char* path;
  dg_file_t* file;
  int i;

  if (forget_wad_stats)
    return;

  if (!wad_stats.map_count)
    return;

  path = dsda_WadStatsPath();

  file = M_OpenFile(path, "wb");
  if (!file)
    lprintf(LO_WARN, "dsda_SaveWadStats: Failed to save wad stats file \"%s\".", path);

  DG_printf(file, "%d\n", current_version);
  DG_printf(file, "%d\n", wad_stats.total_kills);

  for (i = 0; i < wad_stats.map_count; ++i) {
    map_stats_t* ms;

    ms = &wad_stats.maps[i];
    DG_printf(file, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            ms->lump, ms->episode, ms->map,
            ms->best_skill, ms->best_time, ms->best_max_time, ms->best_sk5_time,
            ms->total_exits, ms->total_kills,
            ms->best_kills, ms->best_items, ms->best_secrets,
            ms->max_kills, ms->max_items, ms->max_secrets);
  }

  DG_close(file);
}

static map_stats_t* dsda_MapStats(int episode, int map) {
  int i;

  for (i = 0; i < wad_stats.map_count; ++i)
    if (wad_stats.maps[i].episode == episode && wad_stats.maps[i].map == map)
      return &wad_stats.maps[i];

  return NULL;
}

void dsda_WadStatsEnterMap(void) {
  current_map_stats = dsda_MapStats(gameepisode, gamemap);
}

void dsda_WadStatsExitMap(int missed_monsters) {
  int skill;

  // TODO: remap 5 -> num_skills and 4 -> num_skills - 1

  if (!current_map_stats || demoplayback)
    return;

  if (!nomonsters) {
    skill = gameskill + 1;
    if (skill > current_map_stats->best_skill) {
      if (current_map_stats->best_skill < 4) {
        current_map_stats->best_time = -1;
        current_map_stats->best_max_time = -1;
      }

      current_map_stats->best_skill = skill;
    }

    if (skill >= current_map_stats->best_skill || skill == 4) {
      if (levels_completed == 1)
        if (current_map_stats->best_time == -1 || current_map_stats->best_time > leveltime)
          current_map_stats->best_time = leveltime;

      if (levels_completed == 1 && skill == 5)
        if (current_map_stats->best_sk5_time == -1 || current_map_stats->best_sk5_time > leveltime)
          current_map_stats->best_sk5_time = leveltime;

      current_map_stats->max_kills = totalkills;
      current_map_stats->max_items = totalitems;
      current_map_stats->max_secrets = totalsecret;

      if (!skill_info.respawn_time) {
        if (totalkills - missed_monsters > current_map_stats->best_kills)
          current_map_stats->best_kills = totalkills - missed_monsters;

        if (levels_completed == 1)
          if (missed_monsters == 0 && players[consoleplayer].secretcount == totalsecret &&
              (current_map_stats->best_max_time == -1 || current_map_stats->best_max_time > leveltime))
            current_map_stats->best_max_time = leveltime;
      }

      if (players[consoleplayer].itemcount > current_map_stats->best_items)
        current_map_stats->best_items = players[consoleplayer].itemcount;

      if (players[consoleplayer].secretcount > current_map_stats->best_secrets)
        current_map_stats->best_secrets = players[consoleplayer].secretcount;
    }
  }

  ++current_map_stats->total_exits;
}

void dsda_WadStatsKill(void) {
  if (!current_map_stats || demoplayback)
    return;

  ++current_map_stats->total_kills;
  ++wad_stats.total_kills;
}

void dsda_InitWadStats(void) {
  dsda_LoadWadStats();

  if (!wad_stats.map_count)
    dsda_CreateWadStats();
}
