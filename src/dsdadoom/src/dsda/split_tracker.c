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
//	DSDA Split Tracker
//

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "m_file.h"
#include "lprintf.h"
#include "e6y.h"

#include "dsda/demo.h"
#include "dsda/settings.h"
#include "dsda/data_organizer.h"

#include "split_tracker.h"

#define SPLIT_VERSION 1

static dsda_split_t* dsda_splits;
static size_t dsda_splits_count;
static int attempts;
static int current_split;
static char* dsda_split_tracker_dir;
static char* dsda_split_tracker_path;

extern int gameskill, gamemap, gameepisode, leveltime, totalleveltimes;
extern int respawnparm, fastparm, nomonsters;

static char* dsda_SplitTrackerDir(void) {
  if (!dsda_split_tracker_dir)
    dsda_split_tracker_dir = dsda_DataDir();

  return dsda_split_tracker_dir;
}

static char* dsda_SplitTrackerPath(void) {
  if (!dsda_split_tracker_path) {
    int length;
    const char* name_base;
    char* dir;
    char params[4];

    name_base = dsda_DemoNameBase();
    if (!name_base)
      return NULL;

    name_base = PathFindFileName(name_base);

    params[0] = respawnparm ? 'r' : 'x';
    params[1] = fastparm    ? 'f' : 'x';
    params[2] = nomonsters  ? 'o' : 'x';
    params[3] = '\0';

    dir = dsda_SplitTrackerDir();

    length = strlen(dir) + strlen(name_base) + 28;
    dsda_split_tracker_path = Z_Malloc(length);

    snprintf(
      dsda_split_tracker_path, length, "%s/%s_%i_%i_%i_%s_splits.txt",
      dir, name_base, gameskill + 1, gameepisode, gamemap, params
    );
  }

  return dsda_split_tracker_path;
}

static void dsda_InitSplitTime(dsda_split_time_t* split_time) {
  split_time->current = 0;
  split_time->best = -1;
  split_time->best_delta = 0;
  split_time->session_best = -1;
  split_time->session_best_delta = 0;
  split_time->ref = -1;
  split_time->ref_delta = 0;
}

static void dsda_LoadSplits(void) {
  char* path;
  char* buffer;
  int version;
  static int loaded = false;

  if (loaded)
    return;

  loaded = true;
  path = dsda_SplitTrackerPath();

  if (!path)
    return;

  if (M_ReadFileToString(path, &buffer) != -1) {
    int episode, map, tics, total_tics, exits, count, i, ref_tics, ref_total_tics;
    char* line;

    line = strtok(buffer, "\n");

    if (line) {
      count = sscanf(line, "%d %d", &attempts, &version);

      if (count < 1)
        attempts = 0;

      if (count < 2)
        version = 0;

      line = strtok(NULL, "\n");
    }

    while (line) {
      ref_tics = ref_total_tics = 0;
      count = sscanf(
        line, "%i %i %i %i %i %i %i",
        &episode, &map, &tics, &total_tics, &exits,
        &ref_tics, &ref_total_tics
      );
      if (count < 5)
        break;

      i = dsda_splits_count;
      dsda_splits = Z_Realloc(dsda_splits, (++dsda_splits_count) * sizeof(dsda_split_t));
      dsda_InitSplitTime(&dsda_splits[i].leveltime);
      dsda_InitSplitTime(&dsda_splits[i].totalleveltimes);
      dsda_splits[i].first_time = 0;
      dsda_splits[i].episode = episode;
      dsda_splits[i].map = map;
      dsda_splits[i].leveltime.best = tics;
      dsda_splits[i].totalleveltimes.best = total_tics;
      dsda_splits[i].leveltime.ref = ref_tics;
      dsda_splits[i].totalleveltimes.ref = ref_total_tics;
      dsda_splits[i].exits = exits;
      dsda_splits[i].run_counter = 0;

      // in version 0, a time of 0 was considered unset
      if (version == 0) {
        if (!dsda_splits[i].leveltime.best)
          dsda_splits[i].leveltime.best = -1;

        if (!dsda_splits[i].totalleveltimes.best)
          dsda_splits[i].totalleveltimes.best = -1;

        if (!dsda_splits[i].leveltime.ref)
          dsda_splits[i].leveltime.ref = -1;

        if (!dsda_splits[i].totalleveltimes.ref)
          dsda_splits[i].totalleveltimes.ref = -1;
      }

      line = strtok(NULL, "\n");
    }

    Z_Free(buffer);
  }
}

void dsda_WriteSplits(void) {
  char* path;
  byte  buffer[32 * 100];
  byte* p = buffer;
  int i;

  if (!attempts)
    return;

  path = dsda_SplitTrackerPath();

  p += sprintf((char *)p, "%d %d\n", attempts, SPLIT_VERSION);

  for (i = 0; i < dsda_splits_count; ++i) {
    p += sprintf(
      (char *)p, "%i %i %i %i %i %i %i\n",
      dsda_splits[i].episode,
      dsda_splits[i].map,
      dsda_splits[i].leveltime.best,
      dsda_splits[i].totalleveltimes.best,
      dsda_splits[i].exits,
      dsda_splits[i].leveltime.ref,
      dsda_splits[i].totalleveltimes.ref
    );
  }

  if (!M_WriteFile(path, buffer, p - buffer))
    I_Warn("dsda_WriteSplits: Failed to write splits file \"%s\". (%d)", path, errno);
}

static void dsda_TrackSplitTime(dsda_split_time_t* split_time, int current) {
  split_time->current = current;
  split_time->best_delta = current - split_time->best;
  split_time->session_best_delta = current - split_time->session_best;
  split_time->ref_delta = current - split_time->ref;

  if (current < split_time->best || split_time->best == -1)
    split_time->best = current;

  if (current < split_time->session_best || split_time->session_best == -1)
    split_time->session_best = current;
}

void dsda_RecordSplit(void) {
  int i;

  if (!dsda_TrackSplits()) return;

  dsda_LoadSplits();

  for (i = 0; i < dsda_splits_count; ++i)
    if (dsda_splits[i].episode == gameepisode && dsda_splits[i].map == gamemap) {
      dsda_splits[i].first_time = 0;
      break;
    }

  if (i == dsda_splits_count) {
    dsda_splits = Z_Realloc(dsda_splits, (++dsda_splits_count) * sizeof(dsda_split_t));
    dsda_splits[i].first_time = 1;
    dsda_InitSplitTime(&dsda_splits[i].leveltime);
    dsda_InitSplitTime(&dsda_splits[i].totalleveltimes);
    dsda_splits[i].episode = gameepisode;
    dsda_splits[i].map = gamemap;
    dsda_splits[i].exits = 0;
  }

  current_split = i;
  dsda_splits[i].run_counter = attempts;
  dsda_splits[i].exits++;
  dsda_TrackSplitTime(&dsda_splits[i].leveltime, leveltime);
  dsda_TrackSplitTime(&dsda_splits[i].totalleveltimes, totalleveltimes);
}

dsda_split_t* dsda_CurrentSplit(void) {
  if (!dsda_ShowSplitData()) return NULL;

  return &dsda_splits[current_split];
}

void dsda_ResetSplits(void) {
  if (!dsda_TrackSplits()) return;

  dsda_LoadSplits();
  ++attempts;
}

int dsda_DemoAttempts(void) {
  return attempts;
}
